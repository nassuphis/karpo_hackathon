"""
Poly-Sheet Lambda — parameter-scan mosaic artifacts (poly-sheet.md).

One async invocation renders every frame of a sheet: the standard
pipeline triple with a $-token substituted per frame, each frame a
preview-class grid solved on this machine (fused trio in-process, the
split solvers from the f32 file), binned to a BILEVEL tile, all tiles
stitched into one PNG. Exactly two objects are uploaded:

    sheets/{sheet_id}/sheet.png     the mosaic
    sheets/{sheet_id}/sheet.json    the manifest

Actions:
    run     — render (invoked async through the dispatch fan-out)
    cancel  — write the cancel marker the frame loop checks

No Step Functions, no chunk fan-out, no stored roots.
"""
import json
import math
import os
import re
import struct
import subprocess
import time

import boto3
from botocore.exceptions import ClientError

from compute_fused import _solve_us_per_step
from handler_compute_preview import _compile_compute_inputs
from shared import (
    BUCKET,
    REF_SIZE,
    compute_viewport_from_bin,
    encode_png_gray,
    ok_response,
    parse_body,
    report_status,
)

s3 = boto3.client("s3")
SWEEP_COEFFGEN = os.path.join(os.path.dirname(__file__), "sweep_coeffgen")
SWEEP_MT = os.path.join(os.path.dirname(__file__), "sweep_mt")
SWEEP_CM = os.path.join(os.path.dirname(__file__), "sweep_cm")

TMP_COEFFS = "/tmp/sheet_coeffs.bin"
TMP_ROOTS = "/tmp/sheet_roots.bin"

FUSED_MODES = ("jt64", "cm64", "ae64")
SOLVER_MODES = ("aberth_mt", "companion_matrix", "jenkins_traub", "newton") + FUSED_MODES
SPACINGS = ("linear", "log", "angle")

SHEET_ID_RE = re.compile(r"^[A-Za-z0-9][A-Za-z0-9_-]{2,63}$")
TOKEN_RE = re.compile(r"^\$[A-Za-z][A-Za-z0-9_]*$")

MAX_STEPS = 256
MIN_N, MAX_N = 8, 256
MIN_TILE, MAX_TILE = 32, 1024
MAX_CANVAS_PX = 150_000_000   # stitched mosaic pixel cap (~150MB gray buffer)
MAX_COLS = 32
BUDGET_US = 720_000_000  # ~12 of the lambda's 15 minutes

# fields the scan token may appear in (textual substitution BEFORE compile)
SOURCE_FIELDS = ("coeff_program_source_text", "param_program_source_text")


def handler(event, context):
    params = parse_body(event)
    action = str(params.get("action") or "run").strip().lower()
    if action == "cancel":
        return handle_cancel(params)
    return handle_run(params)


def _cancel_key(sheet_id):
    return f"sheets/{sheet_id}/cancel"


def _validated_sheet_id(params):
    sheet_id = str(params.get("sheet_id") or "").strip()
    if not SHEET_ID_RE.match(sheet_id):
        raise RuntimeError(f"sheet_id must match {SHEET_ID_RE.pattern}, got {sheet_id!r}")
    return sheet_id


def handle_cancel(params):
    sheet_id = _validated_sheet_id(params)
    s3.put_object(Bucket=BUCKET, Key=_cancel_key(sheet_id), Body=b"1")
    return ok_response({"cancelled": sheet_id})


def _cancel_requested(sheet_id):
    try:
        s3.head_object(Bucket=BUCKET, Key=_cancel_key(sheet_id))
        return True
    except ClientError:
        return False


def scan_values(lo, hi, steps, spacing):
    """The per-frame parameter values. linear/log include both ends;
    angle excludes the upper end (periodic parameters: last != first)."""
    lo, hi, steps = float(lo), float(hi), int(steps)
    if steps < 1:
        raise RuntimeError(f"scan steps must be >= 1, got {steps}")
    if spacing not in SPACINGS:
        raise RuntimeError(f"scan spacing must be one of {SPACINGS}, got {spacing!r}")
    if steps == 1:
        return [lo]
    if spacing == "linear":
        return [lo + (hi - lo) * k / (steps - 1) for k in range(steps)]
    if spacing == "angle":
        return [lo + (hi - lo) * k / steps for k in range(steps)]
    if lo == 0 or hi == 0 or (lo < 0) != (hi < 0):
        raise RuntimeError("log spacing needs nonzero, same-sign endpoints")
    ratio = hi / lo
    return [lo * ratio ** (k / (steps - 1)) for k in range(steps)]


def _value_literal(value):
    """A grammar-safe literal: negatives spell as (0-x) — the source
    language has no unary minus in every position."""
    if value < 0:
        return f"(0-{repr(abs(float(value)))})"
    return repr(float(value))


def substitute_token(params, token, value):
    """Textual substitution of the scan token in the source fields.
    Returns a shallow-copied params dict; raises if the token appears
    nowhere (a sheet whose frames are all identical is a mistake)."""
    literal = _value_literal(value)
    out = dict(params)
    hits = 0
    for field in SOURCE_FIELDS:
        text = out.get(field)
        if isinstance(text, str) and token in text:
            hits += text.count(token)
            out[field] = text.replace(token, literal)
    if hits == 0:
        raise RuntimeError(
            f"scan token {token!r} does not appear in any of {SOURCE_FIELDS}")
    return out


def _run_binary(binary, out_path, spec, label, timeout_s=300):
    proc = subprocess.run(
        [binary, out_path], input=json.dumps(spec),
        capture_output=True, text=True, timeout=timeout_s)
    if proc.returncode != 0:
        raise RuntimeError(f"{label} failed: {proc.stderr.strip()[:400]}")
    return json.loads(proc.stdout)


def _solve_frame(compiled, params, n, solver_mode):
    """One frame: grid coeffgen (+fused solve in-process) or the split
    solver from the f32 file. Returns (roots_bytes, degree)."""
    coeff_spec = {
        "mode": "coeffgen",
        "function": str(params["function"]),
        "param_transforms": compiled["param_transforms"],
        "coeff_transforms": compiled["coeff_transforms"],
        "n1": n, "n2": n, "i1_start": 0, "i1_end": n, "times": 1,
    }
    if compiled["param_program"]:
        coeff_spec["param_program"] = compiled["param_program"]
    if compiled["coeff_program"]:
        coeff_spec["coeff_program"] = compiled["coeff_program"]
    if compiled["cfpv"]:
        coeff_spec["cfpv"] = compiled["cfpv"]
    if solver_mode in FUSED_MODES:
        coeff_spec["fused_solver"] = solver_mode
        coeff_spec["roots_file"] = TMP_ROOTS
    meta = _run_binary(SWEEP_COEFFGEN, TMP_COEFFS, coeff_spec, "sheet coeffgen")
    n_coeffs = int(meta["n_coeffs"])
    degree = int(meta["degree"])
    n_steps = n * n

    if solver_mode not in FUSED_MODES:
        if solver_mode in ("companion_matrix", "jenkins_traub", "newton"):
            solve_spec = {
                "mode": {"companion_matrix": "solve_cm",
                         "jenkins_traub": "solve_jt",
                         "newton": "solve_newton"}[solver_mode],
                "coeffs_file": TMP_COEFFS,
                "n_coeffs": n_coeffs,
                "n_steps": n_steps,
                "n_threads": 2,
            }
            binary = SWEEP_CM
        else:
            solve_spec = {
                "mode": "solve_mt",
                "coeffs_file": TMP_COEFFS,
                "n_coeffs": n_coeffs,
                "n2": n_steps,
                "i1_start": 0,
                "i1_end": 1,
                "match_roots": False,
            }
            binary = SWEEP_MT
        _run_binary(binary, TMP_ROOTS, solve_spec, "sheet solve")

    with open(TMP_ROOTS, "rb") as fh:
        roots = fh.read()
    expected = n_steps * degree * 8
    if len(roots) != expected:
        raise RuntimeError(
            f"sheet roots size mismatch: expected {expected}, got {len(roots)}")
    return roots, degree


def _bounds_from_viewport(viewport):
    """center/scale (REF_SIZE convention) -> square world bounds."""
    scale = float(viewport.get("scale") or 1.0)
    world = REF_SIZE / scale if scale > 0 else 4.0
    cre = float(viewport.get("center_re") or 0.0)
    cim = float(viewport.get("center_im") or 0.0)
    return (cre - world / 2, cre + world / 2, cim - world / 2, cim + world / 2)


def _square_fit(bounds):
    """Fit the larger span isotropically around the rect center
    (mirrors the preview's explicit-viewport behavior)."""
    xmin, xmax, ymin, ymax = (float(b) for b in bounds)
    if not (xmax > xmin and ymax > ymin):
        raise RuntimeError(f"viewport bounds must be a nonempty rect, got {bounds}")
    world = max(xmax - xmin, ymax - ymin)
    cre, cim = (xmin + xmax) / 2, (ymin + ymax) / 2
    return (cre - world / 2, cre + world / 2, cim - world / 2, cim + world / 2)


def bin_bilevel_tile(roots_bytes, bounds, tile_px):
    """Hit-mask: pixel black (0) iff >= 1 finite root lands in it;
    white (255) background. Row 0 is the TOP of the imaginary axis."""
    xmin, xmax, ymin, ymax = bounds
    world_x = xmax - xmin
    world_y = ymax - ymin
    tile = bytearray(b"\xff" * (tile_px * tile_px))
    isfinite = math.isfinite
    usable = (len(roots_bytes) // 8) * 8
    for re_, im_ in struct.iter_unpack("<ff", roots_bytes[:usable]):
        if not (isfinite(re_) and isfinite(im_)):
            continue
        if not (xmin <= re_ < xmax and ymin < im_ <= ymax):
            continue
        px = int((re_ - xmin) / world_x * tile_px)
        py = int((ymax - im_) / world_y * tile_px)
        if px >= tile_px:
            px = tile_px - 1
        if py >= tile_px:
            py = tile_px - 1
        tile[py * tile_px + px] = 0
    return tile


def rotate_tile(tile, tile_px, rotate):
    """Quarter-turn rotation (counter-clockwise, like np.rot90)."""
    turns = (int(rotate) // 90) % 4
    for _ in range(turns):
        out = bytearray(len(tile))
        for y in range(tile_px):
            for x in range(tile_px):
                out[(tile_px - 1 - x) * tile_px + y] = tile[y * tile_px + x]
        tile = out
    return tile


def handle_run(params):
    job_id = str(params.get("job_id") or "").strip()
    task_id = str(params.get("task_id") or "").strip()
    sheet_id = _validated_sheet_id(params)
    if not job_id or not task_id:
        raise RuntimeError("poly-sheet run requires job_id and task_id")

    scan = params.get("scan") or {}
    token = str(scan.get("token") or "").strip()
    if not TOKEN_RE.match(token):
        raise RuntimeError(f"scan token must match {TOKEN_RE.pattern}, got {token!r}")
    steps = int(scan.get("steps") or 0)
    if not 1 <= steps <= MAX_STEPS:
        raise RuntimeError(f"scan steps must be in 1..{MAX_STEPS}, got {steps}")
    spacing = str(scan.get("spacing") or "linear").strip().lower()
    values = scan_values(scan.get("from"), scan.get("to"), steps, spacing)

    frame = params.get("frame") or {}
    n = int(frame.get("n") or 0)
    if not MIN_N <= n <= MAX_N:
        raise RuntimeError(f"frame n must be in {MIN_N}..{MAX_N}, got {n}")
    tile_px = int(frame.get("tile_px") or 256)
    if not MIN_TILE <= tile_px <= MAX_TILE:
        raise RuntimeError(f"tile_px must be in {MIN_TILE}..{MAX_TILE}, got {tile_px}")
    solver_mode = str(frame.get("solver_mode") or "ae64").strip().lower()
    if solver_mode not in SOLVER_MODES:
        raise RuntimeError(f"solver_mode must be one of {SOLVER_MODES}, got {solver_mode!r}")
    rotate = int(frame.get("rotate") or 0)
    if rotate not in (0, 90, 180, 270):
        raise RuntimeError(f"rotate must be one of 0/90/180/270, got {rotate}")

    viewport = frame.get("viewport") or {}
    vp_mode = str(viewport.get("mode") or "quantile").strip().lower()
    if vp_mode not in ("quantile", "explicit", "frozen"):
        raise RuntimeError(f"viewport mode must be quantile/explicit/frozen, got {vp_mode!r}")
    quantile = float(viewport.get("quantile") or 0.0)
    shim = float(viewport.get("shim") or 0.05)
    explicit_bounds = None
    if vp_mode == "explicit":
        try:
            explicit_bounds = _square_fit((
                viewport["min_re"], viewport["max_re"],
                viewport["min_im"], viewport["max_im"]))
        except KeyError as missing:
            raise RuntimeError(f"explicit viewport requires min/max re/im, missing {missing}")

    cols = int(params.get("grid_cols") or math.ceil(math.sqrt(steps)))
    if not 1 <= cols <= MAX_COLS:
        raise RuntimeError(f"grid_cols must be in 1..{MAX_COLS}, got {cols}")
    rows = math.ceil(steps / cols)
    canvas_px = cols * tile_px * rows * tile_px
    if canvas_px > MAX_CANVAS_PX:
        raise RuntimeError(
            f"mosaic too large: {cols * tile_px}x{rows * tile_px} = {canvas_px / 1e6:.0f}MP "
            f"> {MAX_CANVAS_PX / 1e6:.0f}MP — reduce tile_px, frames, or columns")

    # budget guard: refuse sheets that cannot finish in one invocation
    spp = _solve_us_per_step(solver_mode=solver_mode, degree=40, fused_threads=2)
    est_us = int(steps * n * n * (spp + 3.0) * 1.4)
    if est_us > BUDGET_US:
        raise RuntimeError(
            f"sheet too large for one invocation: {steps} frames x {n}x{n} rows "
            f"with {solver_mode} estimates {est_us / 1e6:.0f}s > {BUDGET_US / 1e6:.0f}s "
            f"budget — reduce frames, N, or switch solver")

    # substitution must hit at least once (validated on frame 0's value)
    substitute_token(params, token, values[0])

    t0 = time.time()
    report_status(job_id, task_id, "started", result_data={
        "phase": "sheet", "phase_label": "Sheet frames",
        "sheet_id": sheet_id, "frames": steps, "frame": 0,
    })

    canvas_w = cols * tile_px
    canvas_h = rows * tile_px
    canvas = bytearray(b"\xff" * (canvas_w * canvas_h))
    frozen_bounds = None
    frame_records = []
    degree = None

    try:
        for k, value in enumerate(values):
            if _cancel_requested(sheet_id):
                report_status(job_id, task_id, "error", "Cancelled by user",
                              result_data={"phase": "error", "phase_label": "Cancelled",
                                           "sheet_id": sheet_id, "frame": k})
                return ok_response({"cancelled": sheet_id, "frames_done": k})

            frame_params = substitute_token(params, token, value)
            compiled = _compile_compute_inputs(frame_params)
            roots, degree = _solve_frame(compiled, frame_params, n, solver_mode)

            if vp_mode == "explicit":
                bounds = explicit_bounds
            elif vp_mode == "frozen":
                if frozen_bounds is None:
                    frozen_bounds = _bounds_from_viewport(
                        compute_viewport_from_bin(roots, quantile=quantile, shim=shim))
                bounds = frozen_bounds
            else:
                bounds = _bounds_from_viewport(
                    compute_viewport_from_bin(roots, quantile=quantile, shim=shim))

            tile = bin_bilevel_tile(roots, bounds, tile_px)
            if rotate:
                tile = rotate_tile(tile, tile_px, rotate)

            row, col = divmod(k, cols)
            y0, x0 = row * tile_px, col * tile_px
            for y in range(tile_px):
                start = (y0 + y) * canvas_w + x0
                canvas[start:start + tile_px] = tile[y * tile_px:(y + 1) * tile_px]

            frame_records.append({"frame": k, "value": value,
                                  "bounds": [round(b, 12) for b in bounds]})
            report_status(job_id, task_id, "running", result_data={
                "phase": "sheet", "phase_label": f"Sheet frame {k + 1}/{steps}",
                "sheet_id": sheet_id, "frames": steps, "frame": k + 1,
            })

        png = encode_png_gray(canvas_w, canvas_h, bytes(canvas))
        png_key = f"sheets/{sheet_id}/sheet.png"
        manifest = {
            "sheet_id": sheet_id,
            "created_at_ms": int(t0 * 1000),
            "elapsed_ms": int((time.time() - t0) * 1000),
            "png_key": png_key,
            "frames": steps,
            "grid": {"cols": cols, "rows": rows},
            "tile_px": tile_px,
            "n": n,
            "degree": degree,
            "solver_mode": solver_mode,
            "rotate": rotate,
            "scan": {"token": token, "from": float(scan.get("from")),
                     "to": float(scan.get("to")), "steps": steps, "spacing": spacing},
            "viewport": {"mode": vp_mode, "quantile": quantile, "shim": shim,
                         "explicit": list(explicit_bounds) if explicit_bounds else None},
            "function": str(params.get("function") or ""),
            "frame_records": frame_records,
        }
        s3.put_object(Bucket=BUCKET, Key=png_key, Body=png,
                      ContentType="image/png")
        s3.put_object(Bucket=BUCKET, Key=f"sheets/{sheet_id}/sheet.json",
                      Body=json.dumps(manifest, indent=1).encode("utf-8"),
                      ContentType="application/json")
        report_status(job_id, task_id, "done", result_data={
            "phase": "done", "phase_label": "Done",
            "sheet_id": sheet_id, "png_key": png_key,
            "frames": steps, "elapsed_ms": manifest["elapsed_ms"],
        })
        return ok_response(manifest)
    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data={
            "phase": "error", "phase_label": "Sheet failed", "sheet_id": sheet_id,
        })
        raise
    finally:
        for path in (TMP_COEFFS, TMP_ROOTS):
            try:
                os.remove(path)
            except OSError:
                pass
