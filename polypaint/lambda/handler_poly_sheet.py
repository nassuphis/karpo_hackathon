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
    run     — render every frame in one invocation (small sheets, tests)
    frames  — FAN-OUT worker: render an assigned subset of frames,
              upload each as sheets/{id}/tiles/{k}.bin + .json
    stitch  — assemble uploaded tiles into sheet.png + sheet.json,
              then delete the tiles/ prefix
    cancel  — write the cancel marker the frame loops check

The client orchestrates the fan-out (dispatch W 'frames' jobs, poll,
then one 'stitch' job) — no Step Functions, no stored roots. Frozen
viewport under fan-out: every worker derives frame 0's bounds itself
(the pipeline is deterministic, so all workers agree exactly).
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
from cp437_font import FONT_ROWS
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
SPACINGS = ("linear", "log", "angle", "step")
POLARITIES = ("white_on_black", "black_on_white")
MAX_MARGIN = 64

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


MAX_FANOUT = 16


def handler(event, context):
    params = parse_body(event)
    action = str(params.get("action") or "run").strip().lower()
    if action == "cancel":
        return handle_cancel(params)
    if action == "frames":
        return handle_frames(params)
    if action == "stitch":
        return handle_stitch(params)
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


def scan_values(lo, hi, steps, spacing, step=None):
    """The per-frame parameter values. linear/log include both ends;
    angle excludes the upper end (periodic parameters: last != first);
    step is the arithmetic sequence lo + k*step — the discrete form
    (step=1 walks integers: degrees, counts)."""
    lo, steps = float(lo), int(steps)
    if steps < 1:
        raise RuntimeError(f"scan steps must be >= 1, got {steps}")
    if spacing not in SPACINGS:
        raise RuntimeError(f"scan spacing must be one of {SPACINGS}, got {spacing!r}")
    if spacing == "step":
        step = float(step if step is not None else 1.0)
        if not math.isfinite(step) or step == 0:
            raise RuntimeError(f"step spacing needs a nonzero finite step, got {step}")
        return [lo + step * k for k in range(steps)]
    hi = float(hi)
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
    language has no unary minus in every position. Integral values
    spell as integers so the token works in count positions
    (fill/scan lengths, degrees, poke indices)."""
    v = float(value)
    body = str(int(abs(v))) if v.is_integer() else repr(abs(v))
    return f"(0-{body})" if v < 0 else body


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


def bin_bilevel_tile(roots_bytes, bounds, tile_px, fg=255, bg=0):
    """Hit-mask: pixel fg iff >= 1 finite root lands in it; bg
    elsewhere (default white-on-black, matching the compute preview).
    Row 0 is the TOP of the imaginary axis."""
    xmin, xmax, ymin, ymax = bounds
    world_x = xmax - xmin
    world_y = ymax - ymin
    tile = bytearray(bytes([bg]) * (tile_px * tile_px))
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
        tile[py * tile_px + px] = fg
    return tile


def draw_tile_label(tile, tile_px, text, fg, bg):
    """Stamp the frame's scan value into the tile's top-left corner
    (CP437 8x8 glyphs on a bg backing box, scaled with the tile so the
    label stays readable). Called AFTER rotation: labels read upright
    whatever the tile orientation."""
    scale = max(1, min(4, tile_px // 128))
    pad = 2 * scale
    max_chars = max(0, (tile_px - 2 * pad) // (8 * scale))
    text = str(text)[:max_chars]
    if not text:
        return tile
    box_w = pad + len(text) * 8 * scale + pad
    box_h = pad + 8 * scale + pad
    for y in range(min(box_h, tile_px)):
        base = y * tile_px
        for x in range(min(box_w, tile_px)):
            tile[base + x] = bg
    for ci, ch in enumerate(text):
        rows = FONT_ROWS.get(ord(ch) + 1)
        if rows is None:
            continue
        gx0 = pad + ci * 8 * scale
        for gy in range(8):
            row = rows[gy]
            for gx in range(8):
                # LSB-leftmost packing (cp437_font header): bit gx IS column gx
                if not (row & (1 << gx)):
                    continue
                for sy in range(scale):
                    py = pad + gy * scale + sy
                    if py >= tile_px:
                        continue
                    base = py * tile_px
                    for sx in range(scale):
                        px = gx0 + gx * scale + sx
                        if px < tile_px:
                            tile[base + px] = fg
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


def _parse_sheet_config(params):
    """Validate the full sheet spec (shared by run/frames/stitch —
    every action re-derives the same geometry from the same params)."""
    sheet_id = _validated_sheet_id(params)
    scan = params.get("scan") or {}
    token = str(scan.get("token") or "").strip()
    if not TOKEN_RE.match(token):
        raise RuntimeError(f"scan token must match {TOKEN_RE.pattern}, got {token!r}")
    steps = int(scan.get("steps") or 0)
    if not 1 <= steps <= MAX_STEPS:
        raise RuntimeError(f"scan steps must be in 1..{MAX_STEPS}, got {steps}")
    spacing = str(scan.get("spacing") or "linear").strip().lower()
    values = scan_values(scan.get("from"), scan.get("to") or 0.0, steps, spacing,
                         step=scan.get("step"))

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
    label = bool(frame.get("label"))
    polarity = str(frame.get("polarity") or "white_on_black").strip().lower()
    if polarity not in POLARITIES:
        raise RuntimeError(f"polarity must be one of {POLARITIES}, got {polarity!r}")
    margin_px = int(frame.get("margin_px") or 0)
    if not 0 <= margin_px <= MAX_MARGIN:
        raise RuntimeError(f"margin_px must be in 0..{MAX_MARGIN}, got {margin_px}")

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
    canvas_w = cols * tile_px + (cols + 1) * margin_px
    canvas_h = rows * tile_px + (rows + 1) * margin_px
    canvas_px = canvas_w * canvas_h
    if canvas_px > MAX_CANVAS_PX:
        raise RuntimeError(
            f"mosaic too large: {canvas_w}x{canvas_h} = {canvas_px / 1e6:.0f}MP "
            f"> {MAX_CANVAS_PX / 1e6:.0f}MP — reduce tile_px, frames, or columns")

    fg, bg = (255, 0) if polarity == "white_on_black" else (0, 255)
    return {
        "sheet_id": sheet_id, "scan": scan, "token": token, "steps": steps,
        "spacing": spacing, "values": values, "n": n, "tile_px": tile_px,
        "solver_mode": solver_mode, "rotate": rotate, "polarity": polarity,
        "margin_px": margin_px, "label": label, "vp_mode": vp_mode, "quantile": quantile,
        "shim": shim, "explicit_bounds": explicit_bounds, "cols": cols,
        "rows": rows, "canvas_w": canvas_w, "canvas_h": canvas_h,
        "fg": fg, "bg": bg,
    }


def _require_job_task(params, action):
    job_id = str(params.get("job_id") or "").strip()
    task_id = str(params.get("task_id") or "").strip()
    if not job_id or not task_id:
        raise RuntimeError(f"poly-sheet {action} requires job_id and task_id")
    return job_id, task_id


def _budget_check(cfg, n_frames, what):
    spp = _solve_us_per_step(solver_mode=cfg["solver_mode"], degree=40,
                             fused_threads=2)
    est_us = int(n_frames * cfg["n"] * cfg["n"] * (spp + 3.0) * 1.4)
    if est_us > BUDGET_US:
        raise RuntimeError(
            f"{what} too large for one invocation: {n_frames} frames x "
            f"{cfg['n']}x{cfg['n']} rows with {cfg['solver_mode']} estimates "
            f"{est_us / 1e6:.0f}s > {BUDGET_US / 1e6:.0f}s budget — "
            f"reduce frames, N, or fan out wider")


def _render_frame_tile(cfg, params, k, frozen_cache):
    """Solve one frame and bin it. Returns (tile_bytes, record)."""
    value = cfg["values"][k]
    frame_params = substitute_token(params, cfg["token"], value)
    compiled = _compile_compute_inputs(frame_params)
    roots, degree = _solve_frame(compiled, frame_params, cfg["n"],
                                 cfg["solver_mode"])
    if cfg["vp_mode"] == "explicit":
        bounds = cfg["explicit_bounds"]
    elif cfg["vp_mode"] == "frozen":
        if "bounds" not in frozen_cache:
            if k == 0:
                frozen_cache["bounds"] = _bounds_from_viewport(
                    compute_viewport_from_bin(
                        roots, quantile=cfg["quantile"], shim=cfg["shim"]))
            else:
                # deterministic pipeline: every worker derives frame 0's
                # bounds identically, no cross-worker coordination
                fp0 = substitute_token(params, cfg["token"], cfg["values"][0])
                roots0, _ = _solve_frame(_compile_compute_inputs(fp0), fp0,
                                         cfg["n"], cfg["solver_mode"])
                frozen_cache["bounds"] = _bounds_from_viewport(
                    compute_viewport_from_bin(
                        roots0, quantile=cfg["quantile"], shim=cfg["shim"]))
        bounds = frozen_cache["bounds"]
    else:
        bounds = _bounds_from_viewport(compute_viewport_from_bin(
            roots, quantile=cfg["quantile"], shim=cfg["shim"]))

    tile = bin_bilevel_tile(roots, bounds, cfg["tile_px"],
                            fg=cfg["fg"], bg=cfg["bg"])
    if cfg["rotate"]:
        tile = rotate_tile(tile, cfg["tile_px"], cfg["rotate"])
    if cfg["label"]:
        tile = draw_tile_label(tile, cfg["tile_px"], f"{value:.6g}",
                               cfg["fg"], cfg["bg"])
    record = {"frame": k, "value": value, "degree": degree,
              "bounds": [round(b, 12) for b in bounds]}
    return tile, record


def _blit_tile(canvas, cfg, k, tile):
    row, col = divmod(k, cfg["cols"])
    tile_px, margin_px, canvas_w = cfg["tile_px"], cfg["margin_px"], cfg["canvas_w"]
    y0 = margin_px + row * (tile_px + margin_px)
    x0 = margin_px + col * (tile_px + margin_px)
    for y in range(tile_px):
        start = (y0 + y) * canvas_w + x0
        canvas[start:start + tile_px] = tile[y * tile_px:(y + 1) * tile_px]


def _tile_key(sheet_id, k):
    return f"sheets/{sheet_id}/tiles/{k:05d}.bin"


def _sheet_manifest(cfg, params, t0, degree, frame_records, render_mode):
    scan = cfg["scan"]
    return {
        "sheet_id": cfg["sheet_id"],
        "created_at_ms": int(t0 * 1000),
        "elapsed_ms": int((time.time() - t0) * 1000),
        "png_key": f"sheets/{cfg['sheet_id']}/sheet.png",
        "frames": cfg["steps"],
        "grid": {"cols": cfg["cols"], "rows": cfg["rows"]},
        "tile_px": cfg["tile_px"],
        "n": cfg["n"],
        "degree": degree,
        "solver_mode": cfg["solver_mode"],
        "rotate": cfg["rotate"],
        "polarity": cfg["polarity"],
        "margin_px": cfg["margin_px"],
        "label": cfg["label"],
        "render_mode": render_mode,
        "pipeline": {key: params[key] for key in (
            "function", "cfpv", "param_transforms", "coeff_transforms",
            "param_program_chain", "coeff_program_chain",
            "param_program_source_text", "coeff_program_source_text")
            if params.get(key) not in (None, "", [])},
        "scan": {"token": cfg["token"], "from": float(scan.get("from")),
                 "to": float(scan.get("to") or 0.0),
                 "step": (float(scan.get("step") or 0.0)
                          if cfg["spacing"] == "step" else None),
                 "steps": cfg["steps"], "spacing": cfg["spacing"],
                 "values": [round(v, 12) for v in cfg["values"]]},
        "viewport": {"mode": cfg["vp_mode"], "quantile": cfg["quantile"],
                     "shim": cfg["shim"],
                     "explicit": (list(cfg["explicit_bounds"])
                                  if cfg["explicit_bounds"] else None)},
        "function": str(params.get("function") or ""),
        "frame_records": frame_records,
    }


def _upload_sheet(cfg, manifest, canvas):
    png = encode_png_gray(cfg["canvas_w"], cfg["canvas_h"], bytes(canvas))
    s3.put_object(Bucket=BUCKET, Key=manifest["png_key"], Body=png,
                  ContentType="image/png")
    s3.put_object(Bucket=BUCKET, Key=f"sheets/{cfg['sheet_id']}/sheet.json",
                  Body=json.dumps(manifest, indent=1).encode("utf-8"),
                  ContentType="application/json")


def handle_frames(params):
    """Fan-out worker: render the assigned frame subset, upload tiles."""
    job_id, task_id = _require_job_task(params, "frames")
    cfg = _parse_sheet_config(params)
    sheet_id = cfg["sheet_id"]
    indices = params.get("frame_indices")
    if not isinstance(indices, list) or not indices:
        raise RuntimeError("frames action requires a nonempty frame_indices list")
    indices = sorted({int(k) for k in indices})
    if indices[0] < 0 or indices[-1] >= cfg["steps"]:
        raise RuntimeError(
            f"frame_indices out of range 0..{cfg['steps'] - 1}: {indices}")
    _budget_check(cfg, len(indices) + (1 if cfg["vp_mode"] == "frozen" else 0),
                  "worker share")
    substitute_token(params, cfg["token"], cfg["values"][indices[0]])

    report_status(job_id, task_id, "started", result_data={
        "phase": "sheet", "phase_label": "Sheet frames",
        "sheet_id": sheet_id, "frames": len(indices), "frame": 0,
    })
    frozen_cache = {}
    done = 0
    try:
        for k in indices:
            if _cancel_requested(sheet_id):
                report_status(job_id, task_id, "error", "Cancelled by user",
                              result_data={"phase": "error",
                                           "phase_label": "Cancelled",
                                           "sheet_id": sheet_id, "frame": done})
                return ok_response({"cancelled": sheet_id, "frames_done": done})
            tile, record = _render_frame_tile(cfg, params, k, frozen_cache)
            s3.put_object(Bucket=BUCKET, Key=_tile_key(sheet_id, k),
                          Body=bytes(tile), ContentType="application/octet-stream")
            s3.put_object(Bucket=BUCKET,
                          Key=_tile_key(sheet_id, k).replace(".bin", ".json"),
                          Body=json.dumps(record).encode("utf-8"),
                          ContentType="application/json")
            done += 1
            report_status(job_id, task_id, "running", result_data={
                "phase": "sheet", "phase_label": f"{done}/{len(indices)} frames",
                "sheet_id": sheet_id, "frames": len(indices), "frame": done,
            })
        report_status(job_id, task_id, "done", result_data={
            "phase": "done", "phase_label": "Worker done",
            "sheet_id": sheet_id, "frames": len(indices), "frame": done,
        })
        return ok_response({"sheet_id": sheet_id, "frames_done": done})
    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data={
            "phase": "error", "phase_label": "Sheet worker failed",
            "sheet_id": sheet_id,
        })
        raise
    finally:
        for path in (TMP_COEFFS, TMP_ROOTS):
            try:
                os.remove(path)
            except OSError:
                pass


def handle_stitch(params):
    """Assemble the workers' tiles into the final mosaic + manifest,
    then delete the tiles/ prefix."""
    job_id, task_id = _require_job_task(params, "stitch")
    cfg = _parse_sheet_config(params)
    sheet_id = cfg["sheet_id"]
    t0 = float(params.get("started_at_s") or time.time())

    report_status(job_id, task_id, "started", result_data={
        "phase": "stitch", "phase_label": "Stitching",
        "sheet_id": sheet_id, "frames": cfg["steps"],
    })
    try:
        if _cancel_requested(sheet_id):
            report_status(job_id, task_id, "error", "Cancelled by user",
                          result_data={"phase": "error", "phase_label": "Cancelled",
                                       "sheet_id": sheet_id})
            return ok_response({"cancelled": sheet_id, "frames_done": 0})

        tile_bytes = cfg["tile_px"] * cfg["tile_px"]
        canvas = bytearray(bytes([cfg["bg"]]) * (cfg["canvas_w"] * cfg["canvas_h"]))
        frame_records = []
        missing = []
        for k in range(cfg["steps"]):
            key = _tile_key(sheet_id, k)
            try:
                tile = s3.get_object(Bucket=BUCKET, Key=key)["Body"].read()
                record = json.loads(
                    s3.get_object(Bucket=BUCKET,
                                  Key=key.replace(".bin", ".json"))["Body"].read())
            except ClientError:
                missing.append(k)
                continue
            if len(tile) != tile_bytes:
                raise RuntimeError(
                    f"tile {k} size mismatch: expected {tile_bytes}, got {len(tile)}")
            _blit_tile(canvas, cfg, k, tile)
            frame_records.append(record)
        if missing:
            raise RuntimeError(
                f"stitch is missing {len(missing)} of {cfg['steps']} tiles "
                f"(frames {missing[:8]}{'...' if len(missing) > 8 else ''}) — "
                f"a worker failed or is still running")

        degree = frame_records[0].get("degree") if frame_records else None
        manifest = _sheet_manifest(cfg, params, t0, degree, frame_records, "fanout")
        _upload_sheet(cfg, manifest, canvas)

        # tiles are scaffolding — remove them so /list-sheets stays lean
        delete_keys = []
        for k in range(cfg["steps"]):
            delete_keys.append({"Key": _tile_key(sheet_id, k)})
            delete_keys.append({"Key": _tile_key(sheet_id, k).replace(".bin", ".json")})
        for i in range(0, len(delete_keys), 1000):
            s3.delete_objects(Bucket=BUCKET,
                              Delete={"Objects": delete_keys[i:i + 1000],
                                      "Quiet": True})

        report_status(job_id, task_id, "done", result_data={
            "phase": "done", "phase_label": "Done",
            "sheet_id": sheet_id, "png_key": manifest["png_key"],
            "frames": cfg["steps"], "elapsed_ms": manifest["elapsed_ms"],
        })
        return ok_response(manifest)
    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data={
            "phase": "error", "phase_label": "Stitch failed", "sheet_id": sheet_id,
        })
        raise


def handle_run(params):
    job_id, task_id = _require_job_task(params, "run")
    cfg = _parse_sheet_config(params)
    sheet_id = cfg["sheet_id"]
    steps = cfg["steps"]
    _budget_check(cfg, steps, "sheet")

    # substitution must hit at least once (validated on frame 0's value)
    substitute_token(params, cfg["token"], cfg["values"][0])

    t0 = time.time()
    report_status(job_id, task_id, "started", result_data={
        "phase": "sheet", "phase_label": "Sheet frames",
        "sheet_id": sheet_id, "frames": steps, "frame": 0,
    })

    canvas = bytearray(bytes([cfg["bg"]]) * (cfg["canvas_w"] * cfg["canvas_h"]))
    frozen_cache = {}
    frame_records = []
    degree = None

    try:
        for k in range(steps):
            if _cancel_requested(sheet_id):
                report_status(job_id, task_id, "error", "Cancelled by user",
                              result_data={"phase": "error", "phase_label": "Cancelled",
                                           "sheet_id": sheet_id, "frame": k})
                return ok_response({"cancelled": sheet_id, "frames_done": k})

            tile, record = _render_frame_tile(cfg, params, k, frozen_cache)
            degree = record["degree"]
            _blit_tile(canvas, cfg, k, tile)
            frame_records.append(record)
            report_status(job_id, task_id, "running", result_data={
                "phase": "sheet", "phase_label": f"Sheet frame {k + 1}/{steps}",
                "sheet_id": sheet_id, "frames": steps, "frame": k + 1,
            })

        manifest = _sheet_manifest(cfg, params, t0, degree, frame_records, "single")
        _upload_sheet(cfg, manifest, canvas)
        report_status(job_id, task_id, "done", result_data={
            "phase": "done", "phase_label": "Done",
            "sheet_id": sheet_id, "png_key": manifest["png_key"],
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
