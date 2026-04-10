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
    compute_viewport_from_bin,
    encode_png_gray,
    format_bytes,
    parse_body,
    tmp_space_stats,
)


SWEEP_COEFFGEN = os.path.join(os.path.dirname(__file__), "sweep_coeffgen")
SWEEP_AE = os.path.join(os.path.dirname(__file__), "sweep")
SWEEP_MT = os.path.join(os.path.dirname(__file__), "sweep_mt")
SWEEP_CM = os.path.join(os.path.dirname(__file__), "sweep_cm")
TMP_COEFFS = "/tmp/preview_coeffs.bin"
TMP_ROOTS = "/tmp/preview_roots.bin"
MAX_PREVIEW_N = 1024
MAX_COEFFS_EST = 256
TMP_HEADROOM = 0.8


def _json_response(status_code, body):
    return {
        "statusCode": status_code,
        "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
        "body": json.dumps(body),
    }


def _solver_tag(solver_mode):
    return "CM" if solver_mode == "companion_matrix" else "AE-MT" if solver_mode == "aberth_mt" else "AE"


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


def _preview_context(*, solver_mode, n_preview, function_name, coeff_transforms, param_transforms):
    return (
        f"solver={_solver_tag(solver_mode)}, N_preview={n_preview}, function={function_name}, "
        f"param={_format_chain(param_transforms)}, coeff={_format_chain(coeff_transforms)}"
    )


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
    scale = viewport["scale"] * width / 4096.0
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


def handler(event, context):
    try:
        params = parse_body(event)
        function_name = str(params.get("function") or "").strip()
        if not function_name:
            return _json_response(400, {"message": "compute preview missing function"})

        n_preview = _ensure_preview_n(params.get("N_preview"))
        solver_mode = str(params.get("solver_mode") or "aberth").strip() or "aberth"
        if solver_mode not in {"aberth", "aberth_mt", "companion_matrix"}:
            return _json_response(400, {"message": f"unsupported preview solver_mode: {solver_mode}"})
        quantile = _validate_quantile(params.get("quantile", 0.0))
        shim = _validate_shim(params.get("shim", 0.05))

        coeff_transforms = params.get("coeff_transforms") or []
        param_transforms = params.get("param_transforms") or []
        cfpv = _validate_cfpv(params.get("cfpv"))
        ctx = _preview_context(
            solver_mode=solver_mode,
            n_preview=n_preview,
            function_name=function_name,
            coeff_transforms=coeff_transforms,
            param_transforms=param_transforms,
        )
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
        else:
            solve_binary = SWEEP_AE
            solve_spec = {
                "mode": "solve",
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
        gray, n_roots_in_view = _raster_gray_preview(roots_data, n_preview, n_preview, viewport)
        raster_ms = int((time.time() - t0) * 1000)

        t0 = time.time()
        png_data = encode_png_gray(n_preview, n_preview, gray)
        encode_ms = int((time.time() - t0) * 1000)

        total_ms = int((time.time() - t_total) * 1000)
        response = {
            "solver_mode": solver_mode,
            "N_preview": n_preview,
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
            "image_width": n_preview,
            "image_height": n_preview,
            "quantile": quantile,
            "shim": shim,
            "image_png_base64": base64.b64encode(png_data).decode("ascii"),
            "q_re": viewport["q_re"],
            "q_im": viewport["q_im"],
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
