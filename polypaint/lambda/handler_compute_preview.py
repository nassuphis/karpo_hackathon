"""
Compute preview Lambda handler — fast, ephemeral compute preview.

Runs coeffgen + solve + viewport + PNG raster in one synchronous Lambda call.
No S3, no DynamoDB, no Step Functions. Uses /tmp only.
"""
import base64
import json
import math
import os
import struct
import subprocess
import time

from shared import (
    BUCKET,
    REF_SIZE,
    compute_viewport_from_bin,
    encode_png_gray,
    format_bytes,
    parse_body,
    tmp_space_stats,
)
from coeff_program_chain import compile_coeff_program_chain
from coeff_program_source import coeff_source_text_from_payload, parse_coeff_program_source
from pipeline_programs import (
    CoeffSourceCompileError,
    coeff_source_text_for_run,
    parse_coeff_source_for_run,
    pipeline_mode_from_params,
)
from param_program_chain import compile_param_program_chain


SWEEP_COEFFGEN = os.path.join(os.path.dirname(__file__), "sweep_coeffgen")
SWEEP_MT = os.path.join(os.path.dirname(__file__), "sweep_mt")
SWEEP_CM = os.path.join(os.path.dirname(__file__), "sweep_cm")
TMP_COEFFS = "/tmp/preview_coeffs.bin"
TMP_ROOTS = "/tmp/preview_roots.bin"
MAX_PREVIEW_N = 1024
MAX_PREVIEW_PIX = 4096
MAX_COEFFS_EST = 256
TMP_HEADROOM = 0.8
ROOTS_CM_SYNC_MAX_N = int(os.environ.get("COMPUTE_PREVIEW_ROOTS_CM_MAX_N", "128"))
PARAM_PROGRAMS_PREFIX = "polypaint/param-programs/"
COEFF_PROGRAMS_PREFIX = "polypaint/coeff-programs/"
_s3 = None


def _s3_client():
    global _s3
    if _s3 is None:
        import boto3

        _s3 = boto3.client("s3")
    return _s3


def _json_response(status_code, body):
    return {
        "statusCode": status_code,
        "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
        "body": json.dumps(body),
    }


def _solver_tag(solver_mode):
    return "CM" if solver_mode == "companion_matrix" else "AE-MT"


def _format_chain(chain):
    if not isinstance(chain, list) or not chain:
        return "none"
    parts = []
    for item in chain:
        if isinstance(item, list) and item:
            name = str(item[0])
            args = ",".join(str(v) for v in item[1:])
            parts.append(f"{name}({args})" if args else name)
        else:
            parts.append(str(item))
    return ",".join(parts) if parts else "none"


def _preview_context(*, solver_mode, n_preview, function_name, coeff_transforms, param_transforms,
                     param_program_chain=None, coeff_program_chain=None, pipeline_mode="chain"):
    param_label = _format_chain(param_program_chain) if param_program_chain else _format_chain(param_transforms)
    coeff_label = _format_chain(coeff_program_chain) if coeff_program_chain else _format_chain(coeff_transforms)
    return (
        f"solver={_solver_tag(solver_mode)}, N_preview={n_preview}, function={function_name}, "
        f"use={pipeline_mode}, param={param_label}, coeff={coeff_label}"
    )


# _pipeline_mode_from_params moved to pipeline_programs (shared with plan
# and coeffgen so the mode rules cannot drift — CR14).


def _compiled_coeff_program_payload(compiled):
    return {
        "version": compiled["version"],
        "fingerprint": compiled["fingerprint"],
        "display": compiled["display"],
        "stack_max": compiled["stack_max"],
        "token_count": compiled["token_count"],
        "scalar_expr_count": compiled["scalar_expr_count"],
        "uses_legacy_chain_equivalent": compiled["uses_legacy_chain_equivalent"],
        "tokens": compiled["tokens"],
        "scalar_exprs": compiled["scalar_exprs"],
    }


def _compiled_param_program_payload(compiled):
    payload = {
        "version": compiled["version"],
        "fingerprint": compiled["fingerprint"],
        "display": compiled["display"],
        "stack_max": compiled["stack_max"],
        "token_count": compiled["token_count"],
        "uses_legacy_fast_path": compiled["uses_legacy_fast_path"],
        "tokens": compiled["tokens"],
    }
    scalar_exprs = compiled.get("scalar_exprs") or []
    if scalar_exprs:
        payload["scalar_exprs"] = scalar_exprs
    return payload


def _is_missing_s3_error(exc):
    response = getattr(exc, "response", {}) or {}
    code = str((response.get("Error") or {}).get("Code") or "")
    return code in {"NoSuchKey", "NoSuchBucket", "404", "NotFound"}


def _read_saved_program_source_chain(prefix, program_kind, program_id):
    macro_id = str(program_id or "").strip()
    if not macro_id:
        raise RuntimeError(f"{program_kind} macro name is required")
    key = f"{prefix}{macro_id}.json"
    try:
        obj = _s3_client().get_object(Bucket=BUCKET, Key=key)
    except Exception as exc:
        if _is_missing_s3_error(exc):
            raise RuntimeError(f"{program_kind} macro not found: {macro_id}") from None
        raise
    raw = obj["Body"].read()
    try:
        payload = json.loads(raw) if raw else {}
    except Exception as exc:
        raise RuntimeError(f"{program_kind} macro is not valid JSON: {macro_id}") from exc
    if not isinstance(payload, dict):
        raise RuntimeError(f"{program_kind} macro must be a JSON object: {macro_id}")
    if program_kind == "coeff program":
        source_text = coeff_source_text_from_payload(payload)
        if source_text is not None:
            parsed = parse_coeff_program_source(source_text)
            return parsed["chain"]
    chain = payload.get("chain")
    if not isinstance(chain, list):
        raise RuntimeError(f"{program_kind} macro chain must be a JSON array: {macro_id}")
    return chain


def _param_program_macro_resolver():
    return lambda macro_id: _read_saved_program_source_chain(
        PARAM_PROGRAMS_PREFIX,
        "param program",
        macro_id,
    )


def _coeff_program_macro_resolver():
    return lambda macro_id: _read_saved_program_source_chain(
        COEFF_PROGRAMS_PREFIX,
        "coeff program",
        macro_id,
    )


def _chain_has_transform(chain, name):
    if not isinstance(chain, list):
        return False
    needle = str(name)
    for item in chain:
        if isinstance(item, list) and item and str(item[0]) == needle:
            return True
        if isinstance(item, str) and item == needle:
            return True
    return False


def _sync_preview_budget_error(*, n_preview, coeff_transforms):
    if _chain_has_transform(coeff_transforms, "roots_cm") and n_preview > ROOTS_CM_SYNC_MAX_N:
        return (
            "compute preview refused before coeffgen: roots_cm coefficient transform is too slow "
            f"for the synchronous HTTP preview at N-preview={n_preview}; "
            f"use N-preview <= {ROOTS_CM_SYNC_MAX_N}, remove roots_cm, or run the full Compute pipeline"
        )
    return None


def _opt_lib_env():
    env = dict(os.environ)
    ld = env.get("LD_LIBRARY_PATH", "")
    if "/opt/lib" not in ld:
        env["LD_LIBRARY_PATH"] = "/opt/lib" + (":" + ld if ld else "")
    return env


def _cleanup_tmp():
    for path in (TMP_COEFFS, TMP_ROOTS):
        try:
            os.remove(path)
        except OSError:
            pass


def _ensure_preview_n(value):
    try:
        n_preview = int(value)
    except (TypeError, ValueError):
        raise ValueError(f"invalid N-preview: {value!r}") from None
    if n_preview < 8 or n_preview > MAX_PREVIEW_N:
        raise ValueError(f"N-preview must be between 8 and {MAX_PREVIEW_N}, got {n_preview}")
    return n_preview


def _ensure_preview_size(value):
    try:
        preview_size = int(value)
    except (TypeError, ValueError):
        raise ValueError(f"invalid preview size: {value!r}") from None
    if preview_size < 64 or preview_size > MAX_PREVIEW_PIX:
        raise ValueError(f"preview size must be between 64 and {MAX_PREVIEW_PIX}, got {preview_size}")
    return preview_size


def _validate_cfpv(values):
    if values is None:
        return []
    if not isinstance(values, list):
        raise ValueError("cfpv must be a list")
    out = []
    for idx, value in enumerate(values):
        try:
            parsed = float(value)
        except (TypeError, ValueError):
            raise ValueError(f"cfpv[{idx}] is not numeric: {value!r}") from None
        if not math.isfinite(parsed):
            raise ValueError(f"cfpv[{idx}] is not finite: {value!r}")
        out.append(parsed)
    return out


def _validate_quantile(value):
    try:
        q = float(value)
    except (TypeError, ValueError):
        raise ValueError(f"invalid preview quantile: {value!r}") from None
    if not math.isfinite(q):
        raise ValueError(f"invalid preview quantile: {value!r}")
    if q < 0.0 or q >= 0.5:
        raise ValueError(f"preview quantile must be in [0, 0.5), got {q}")
    return q


def _validate_shim(value):
    try:
        shim = float(value)
    except (TypeError, ValueError):
        raise ValueError(f"invalid preview shim: {value!r}") from None
    if not math.isfinite(shim):
        raise ValueError(f"invalid preview shim: {value!r}")
    if shim < 0.0 or shim > 1.0:
        raise ValueError(f"preview shim must be in [0, 1], got {shim}")
    return shim


def _validate_debug_coord(value, label):
    try:
        parsed = float(value)
    except (TypeError, ValueError):
        raise ValueError(f"compute debug {label} must be numeric, got {value!r}") from None
    if not math.isfinite(parsed) or parsed < 0.0 or parsed > 1.0:
        raise ValueError(f"compute debug {label} must be in [0, 1], got {value!r}")
    return parsed


def _compile_compute_inputs(params):
    pipeline_mode = pipeline_mode_from_params(params)
    coeff_transforms = params.get("coeff_transforms") or []
    param_transforms = params.get("param_transforms") or []
    param_program_chain = params.get("param_program_chain") or []
    coeff_program_source_text = coeff_source_text_for_run(params, pipeline_mode)
    if coeff_program_source_text is not None:
        parsed_coeff_source = parse_coeff_source_for_run(coeff_program_source_text)
        coeff_program_chain = parsed_coeff_source["chain"]
    else:
        coeff_program_chain = params.get("coeff_program_chain") or []
    param_program = None
    coeff_program = None
    compiled_param_program = None
    compiled_coeff_program = None

    if pipeline_mode == "program":
        param_transforms = []
        coeff_transforms = []
    else:
        param_program_chain = []
        coeff_program_chain = []
        coeff_program_source_text = None

    if param_program_chain:
        if not isinstance(param_program_chain, list):
            raise ValueError("param_program_chain must be a list")
        try:
            compiled_param_program = compile_param_program_chain(
                param_program_chain,
                macro_resolver=_param_program_macro_resolver(),
            )
        except RuntimeError as e:
            raise ValueError(f"invalid param_program_chain: {e}") from None
        if compiled_param_program["legacy_transforms"]:
            param_transforms = compiled_param_program["legacy_transforms"]
        else:
            param_transforms = []
            param_program = _compiled_param_program_payload(compiled_param_program)

    if coeff_program_chain:
        if not isinstance(coeff_program_chain, list):
            raise ValueError("coeff_program_chain must be a list")
        try:
            compiled_coeff_program = compile_coeff_program_chain(
                coeff_program_chain,
                macro_resolver=_coeff_program_macro_resolver(),
            )
        except RuntimeError as e:
            raise ValueError(f"invalid coeff_program_chain: {e}") from None
        if compiled_coeff_program["legacy_coeff_transforms"]:
            coeff_transforms = compiled_coeff_program["legacy_coeff_transforms"]
        else:
            coeff_transforms = []
            coeff_program = _compiled_coeff_program_payload(compiled_coeff_program)

    return {
        "pipeline_mode": pipeline_mode,
        "param_transforms": param_transforms,
        "coeff_transforms": coeff_transforms,
        "param_program_chain": param_program_chain,
        "coeff_program_chain": coeff_program_chain,
        "coeff_program_source_text": coeff_program_source_text,
        "param_program": param_program,
        "coeff_program": coeff_program,
        "compiled_param_program": compiled_param_program,
        "compiled_coeff_program": compiled_coeff_program,
        "cfpv": _validate_cfpv(params.get("cfpv")),
    }


def _complex_pairs_from_file(path):
    with open(path, "rb") as fh:
        data = fh.read()
    usable = (len(data) // 8) * 8
    return [[float(re), float(im)] for re, im in struct.iter_unpack("<ff", data[:usable])]


def _handle_compute_debug(params):
    function_name = str(params.get("function") or "").strip()
    if not function_name:
        return _json_response(400, {"message": "compute debug missing function"})
    stage = str(params.get("debug_stage") or params.get("stage") or "param").strip().lower()
    stage = {"solveae": "solve_ae", "ae": "solve_ae", "solvecm": "solve_cm", "cm": "solve_cm"}.get(stage, stage)
    if stage not in {"param", "poly", "solve_ae", "solve_cm"}:
        return _json_response(400, {"message": f"unsupported compute debug stage: {stage}"})

    u = _validate_debug_coord(params.get("u", 0.0), "u")
    v = _validate_debug_coord(params.get("v", 0.0), "v")
    grid_n = _ensure_preview_n(params.get("N_preview") or params.get("grid_n") or 256)
    compiled = _compile_compute_inputs(params)
    ctx = _preview_context(
        solver_mode="aberth_mt" if stage != "solve_cm" else "companion_matrix",
        n_preview=grid_n,
        function_name=function_name,
        coeff_transforms=compiled["coeff_transforms"],
        param_transforms=compiled["param_transforms"],
        param_program_chain=compiled["param_program_chain"],
        coeff_program_chain=compiled["coeff_program_chain"],
        pipeline_mode=compiled["pipeline_mode"],
    )

    _cleanup_tmp()
    spec = {
        "mode": "compute_debug",
        "function": function_name,
        "u": u,
        "v": v,
        "grid_n": grid_n,
        "param_transforms": compiled["param_transforms"],
        "coeff_transforms": compiled["coeff_transforms"],
    }
    if compiled["param_program"]:
        spec["param_program"] = compiled["param_program"]
    if compiled["coeff_program"]:
        spec["coeff_program"] = compiled["coeff_program"]
    if compiled["cfpv"]:
        spec["cfpv"] = compiled["cfpv"]

    debug_meta = _run_json_binary(SWEEP_COEFFGEN, TMP_COEFFS, spec, phase="compute_debug", timeout_s=10)
    coeff = debug_meta.get("coeff") or {}
    n_coeffs = int(coeff.get("n_coeffs") or 0)
    degree = int(coeff.get("degree") or max(0, n_coeffs - 1))
    if n_coeffs < 1:
        raise RuntimeError(f"compute_debug produced invalid coefficient count {n_coeffs} ({ctx})")

    response = {
        "stage": stage,
        "pipeline_mode": compiled["pipeline_mode"],
        "context": ctx,
        "debug": debug_meta,
        "param_program": {
            "token_count": int((compiled["compiled_param_program"] or {}).get("token_count") or 0),
            "stack_max": int((compiled["compiled_param_program"] or {}).get("stack_max") or 0),
            "fingerprint": (compiled["compiled_param_program"] or {}).get("fingerprint") or "",
        },
        "coeff_program": {
            "token_count": int((compiled["compiled_coeff_program"] or {}).get("token_count") or 0),
            "stack_max": int((compiled["compiled_coeff_program"] or {}).get("stack_max") or 0),
            "fingerprint": (compiled["compiled_coeff_program"] or {}).get("fingerprint") or "",
            "diagnostics": (compiled["compiled_coeff_program"] or {}).get("diagnostics") or [],
        },
    }

    if stage in {"solve_ae", "solve_cm"}:
        solve_binary = SWEEP_CM if stage == "solve_cm" else SWEEP_MT
        solve_spec = {
            "mode": "solve_cm" if stage == "solve_cm" else "solve_mt",
            "coeffs_file": TMP_COEFFS,
            "n_coeffs": n_coeffs,
        }
        if stage == "solve_cm":
            solve_spec["n_steps"] = 1
        else:
            solve_spec.update({"n2": 1, "i1_start": 0, "i1_end": 1, "match_roots": False})
        solve_meta = _run_json_binary(solve_binary, TMP_ROOTS, solve_spec, phase=stage, timeout_s=10)
        response["solve"] = solve_meta
        response["roots"] = _complex_pairs_from_file(TMP_ROOTS)[:degree]
    return _json_response(200, response)


def _preflight_tmp_capacity(n_preview):
    n_steps = n_preview * n_preview
    coeffs_est = n_steps * MAX_COEFFS_EST * 8
    roots_est = n_steps * max(0, MAX_COEFFS_EST - 1) * 8
    peak_est = coeffs_est + roots_est
    stats = tmp_space_stats("/tmp")
    if peak_est > int(stats["free_bytes"] * TMP_HEADROOM):
        raise RuntimeError(
            "compute preview refused before coeffgen: "
            f"estimated_peak_tmp={format_bytes(peak_est)}, free={format_bytes(stats['free_bytes'])}, "
            f"total={format_bytes(stats['total_bytes'])}, device={stats['path']}"
        )


def _exact_tmp_capacity(coeffs_size, degree, n_steps):
    roots_est = n_steps * max(0, degree) * 8
    peak_est = int(coeffs_size) + roots_est
    stats = tmp_space_stats("/tmp")
    if peak_est > int(stats["free_bytes"] * 0.9):
        raise RuntimeError(
            "compute preview refused before solve: "
            f"coeffs_size={format_bytes(coeffs_size)}, estimated_roots_size={format_bytes(roots_est)}, "
            f"estimated_peak_tmp={format_bytes(peak_est)}, free={format_bytes(stats['free_bytes'])}, "
            f"total={format_bytes(stats['total_bytes'])}, device={stats['path']}"
        )


def _run_json_binary(binary, out_path, spec, *, phase, timeout_s):
    result = subprocess.run(
        [binary, out_path],
        input=json.dumps(spec),
        capture_output=True,
        text=True,
        timeout=timeout_s,
        env=_opt_lib_env(),
    )
    if result.returncode != 0:
        raise RuntimeError(f"{phase} failed: {result.stderr.strip()}")
    stdout = (result.stdout or "").strip()
    if not stdout.startswith("{"):
        stderr = (result.stderr or "").strip()
        raise RuntimeError(f"{phase} produced non-JSON stdout: {stdout[:200]!r} stderr: {stderr[:200]!r}")
    return json.loads(stdout)


def _raster_gray_preview(bin_data, width, height, viewport):
    scale = viewport["scale"] * width / float(REF_SIZE)
    cx = viewport["center_re"]
    cy = viewport["center_im"]
    half_w = width / 2.0
    half_h = height / 2.0
    gray = bytearray(width * height)
    in_view = 0
    usable = (len(bin_data) // 8) * 8
    for re, im in struct.iter_unpack("<ff", bin_data[:usable]):
        if not (math.isfinite(re) and math.isfinite(im)):
            continue
        px = int(half_w + (re - cx) * scale)
        py = int(half_h - (im - cy) * scale)
        if 0 <= px < width and 0 <= py < height:
            gray[py * width + px] = 255
            in_view += 1
    return gray, in_view


def _viewport_bounds_for_preview(viewport, width, height):
    scale = float(viewport["scale"]) * float(width) / float(REF_SIZE)
    if scale <= 0.0 or not math.isfinite(scale):
        raise RuntimeError(f"auto viewport returned non-positive scale: {viewport.get('scale')!r}")
    center_re = float(viewport["center_re"])
    center_im = float(viewport["center_im"])
    half_w_world = (float(width) / 2.0) / scale
    half_h_world = (float(height) / 2.0) / scale
    return {
        "min_re": center_re - half_w_world,
        "max_re": center_re + half_w_world,
        "min_im": center_im - half_h_world,
        "max_im": center_im + half_h_world,
        "center_re": center_re,
        "center_im": center_im,
        "scale_ref": float(viewport["scale"]),
    }


def handler(event, context):
    try:
        params = parse_body(event)
        if params.get("debug_stage") or params.get("stage") in {"param", "poly", "solve_ae", "solve_cm", "solveae", "solvecm", "ae", "cm"}:
            try:
                return _handle_compute_debug(params)
            except ValueError as e:
                return _json_response(400, {"message": str(e)})

        function_name = str(params.get("function") or "").strip()
        if not function_name:
            return _json_response(400, {"message": "compute preview missing function"})

        try:
            # Client-input validation: bad request values are 400s, not 500s.
            n_preview = _ensure_preview_n(params.get("N_preview"))
            preview_size = _ensure_preview_size(params.get("preview_size", 1000))
            quantile = _validate_quantile(params.get("quantile", 0.0))
            shim = _validate_shim(params.get("shim", 0.05))
        except ValueError as e:
            return _json_response(400, {"message": str(e)})
        solver_mode = str(params.get("solver_mode") or "aberth_mt").strip() or "aberth_mt"
        if solver_mode not in {"aberth_mt", "companion_matrix"}:
            return _json_response(400, {"message": f"unsupported preview solver_mode: {solver_mode}"})

        try:
            compiled = _compile_compute_inputs(params)
        except CoeffSourceCompileError as e:
            # Keep the editor-grade structure: line/column per diagnostic.
            return _json_response(400, {"message": str(e), "diagnostics": e.diagnostics})
        except ValueError as e:
            return _json_response(400, {"message": str(e)})
        pipeline_mode = compiled["pipeline_mode"]
        coeff_transforms = compiled["coeff_transforms"]
        param_transforms = compiled["param_transforms"]
        param_program_chain = compiled["param_program_chain"]
        coeff_program_chain = compiled["coeff_program_chain"]
        param_program = compiled["param_program"]
        coeff_program = compiled["coeff_program"]
        cfpv = compiled["cfpv"]
        ctx = _preview_context(
            solver_mode=solver_mode,
            n_preview=n_preview,
            function_name=function_name,
            coeff_transforms=coeff_transforms,
            param_transforms=param_transforms,
            param_program_chain=param_program_chain,
            coeff_program_chain=coeff_program_chain,
            pipeline_mode=pipeline_mode,
        )
        budget_error = _sync_preview_budget_error(n_preview=n_preview, coeff_transforms=coeff_transforms)
        if budget_error:
            return _json_response(400, {"message": f"{budget_error} ({ctx})"})
        n_steps = n_preview * n_preview
        _cleanup_tmp()
        _preflight_tmp_capacity(n_preview)

        t_total = time.time()

        t0 = time.time()
        coeff_spec = {
            "mode": "coeffgen",
            "function": function_name,
            "param_transforms": param_transforms,
            "coeff_transforms": coeff_transforms,
            "n1": n_preview,
            "n2": n_preview,
            "i1_start": 0,
            "i1_end": n_preview,
            "times": 1,
        }
        if param_program:
            coeff_spec["param_program"] = param_program
        if coeff_program:
            coeff_spec["coeff_program"] = coeff_program
        if cfpv:
            coeff_spec["cfpv"] = cfpv
        coeff_meta = _run_json_binary(SWEEP_COEFFGEN, TMP_COEFFS, coeff_spec, phase="coeffgen", timeout_s=25)
        coeffgen_ms = int((time.time() - t0) * 1000)

        coeffs_size = os.path.getsize(TMP_COEFFS)
        if coeffs_size != int(coeff_meta.get("data_bytes", -1)):
            raise RuntimeError(
                f"coeffgen size mismatch: expected {coeff_meta.get('data_bytes')}, got {coeffs_size} ({ctx})"
            )
        n_coeffs = int(coeff_meta["n_coeffs"])
        degree = int(coeff_meta["degree"])
        _exact_tmp_capacity(coeffs_size, degree, n_steps)

        if solver_mode == "companion_matrix":
            solve_binary = SWEEP_CM
            solve_spec = {
                "mode": "solve_cm",
                "coeffs_file": TMP_COEFFS,
                "n_coeffs": n_coeffs,
                "n_steps": n_steps,
            }
        elif solver_mode == "aberth_mt":
            solve_binary = SWEEP_MT
            solve_spec = {
                "mode": "solve_mt",
                "coeffs_file": TMP_COEFFS,
                "n_coeffs": n_coeffs,
                "n2": n_steps,
                "i1_start": 0,
                "i1_end": 1,
                "match_roots": False,
            }

        t0 = time.time()
        solve_meta = _run_json_binary(solve_binary, TMP_ROOTS, solve_spec, phase="solve", timeout_s=25)
        solve_ms = int((time.time() - t0) * 1000)

        t0 = time.time()
        with open(TMP_ROOTS, "rb") as fh:
            roots_data = fh.read()
        viewport = compute_viewport_from_bin(roots_data, quantile=quantile, shim=shim)
        viewport_ms = int((time.time() - t0) * 1000)

        t0 = time.time()
        gray, n_roots_in_view = _raster_gray_preview(roots_data, preview_size, preview_size, viewport)
        raster_ms = int((time.time() - t0) * 1000)
        viewport_bounds = _viewport_bounds_for_preview(viewport, preview_size, preview_size)

        t0 = time.time()
        png_data = encode_png_gray(preview_size, preview_size, gray)
        encode_ms = int((time.time() - t0) * 1000)

        total_ms = int((time.time() - t_total) * 1000)
        response = {
            "solver_mode": solver_mode,
            "N_preview": n_preview,
            "preview_size": preview_size,
            "degree": degree,
            "n_coeffs": n_coeffs,
            "n_roots_total": len(roots_data) // 8,
            "n_roots_in_view": n_roots_in_view,
            "coeffgen_ms": coeffgen_ms,
            "solve_ms": solve_ms,
            "viewport_ms": viewport_ms,
            "raster_ms": raster_ms,
            "encode_ms": encode_ms,
            "total_ms": total_ms,
            "coeffs_size": coeffs_size,
            "roots_size": len(roots_data),
            "image_width": preview_size,
            "image_height": preview_size,
            "quantile": quantile,
            "shim": shim,
            "image_png_base64": base64.b64encode(png_data).decode("ascii"),
            "q_re": viewport["q_re"],
            "q_im": viewport["q_im"],
            "viewport": viewport_bounds,
            "avg_iterations": solve_meta.get("avg_iterations", 0),
        }
        if "n_threads" in solve_meta:
            response["n_threads"] = solve_meta["n_threads"]
        if "skipped_overflow" in solve_meta:
            response["skipped_overflow"] = solve_meta["skipped_overflow"]
        return _json_response(200, response)

    except subprocess.TimeoutExpired as e:
        msg = f"compute preview timeout at subprocess: {e.cmd[0]} after {e.timeout}s"
        if "ctx" in locals():
            msg += f" ({ctx})"
        return _json_response(500, {"message": msg})
    except Exception as e:
        msg = str(e)
        if "ctx" in locals() and ctx not in msg:
            msg += f" ({ctx})"
        return _json_response(500, {"message": msg})
    finally:
        _cleanup_tmp()
