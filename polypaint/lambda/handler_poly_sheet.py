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
    begin   — SYNCHRONOUS admission (the /sheet-begin route): validate
              the whole config, compile+probe frame 0 (real degree, real
              per-frame cost -> honest budget), mint the server-owned
              run GENERATION, pre-write every worker/stitch status row,
              persist sheets/{id}/run.json, return the dispatch plan
    frames  — FAN-OUT worker: render an assigned subset of frames,
              upload each as sheets/{id}/tiles/{generation}/{k}.bin+.json
    stitch  — assemble ONE generation's tiles into sheet.png+sheet.json
              (publication is the commit point; cleanup is best-effort
              garbage collection and can never turn a published sheet
              into an error)
    run     — single-invocation render (small sheets, tests)
    cancel  — write the GENERATION-scoped cancel marker

Durability contract (CR35-F4/F5/F19): the browser only relays an
admission the server recorded first. Every status row exists BEFORE
any async invocation, so a worker that dies pre-report leaves an
'accepted' row a poll deadline can see — never an empty poll loop.
All temporary keys, cancel markers, and status identities carry the
generation, so replays and concurrent runs cannot mix state. Frozen
viewport under fan-out: every worker derives frame 0's bounds itself
(the pipeline is deterministic, so all workers agree exactly).
"""
import hashlib
import json
import math
import os
import re
import struct
import subprocess
import time

import boto3
from botocore.exceptions import ClientError, ParamValidationError

from compute_fused import _solve_us_per_step
from cp437_font import FONT_ROWS
from handler_compute_preview import _compile_compute_inputs
from shared import (
    BUCKET,
    REF_SIZE,
    compute_viewport_from_bin,
    encode_png_gray,
    is_missing_s3_error,
    ok_response,
    parse_body,
    read_task_status,
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
GENERATION_RE = re.compile(r"^g[0-9a-f]{12}$")
COMMENT_RE = re.compile(r"#[^\n]*")

MAX_STEPS = 256
MIN_N, MAX_N = 8, 256
MIN_TILE, MAX_TILE = 32, 1024
MAX_CANVAS_PX = 150_000_000   # stitched mosaic pixel cap (~150MB gray buffer)
MAX_COLS = 32
BUDGET_US = 720_000_000  # ~12 of the lambda's 15 minutes

# fields the scan token may appear in (textual substitution BEFORE compile)
SOURCE_FIELDS = ("coeff_program_source_text", "param_program_source_text")


MAX_FANOUT = 16
SHEET_WORKERS = 8


def handler(event, context):
    params = parse_body(event)
    action = str(params.get("action") or "").strip().lower()
    if action == "begin":
        return handle_begin(params)
    if action == "cancel":
        return handle_cancel(params)
    if action == "abandon":
        return handle_abandon(params)
    if action == "frames":
        return handle_frames(params)
    if action == "stitch":
        return handle_stitch(params)
    # round-3 finding 1: action="run" bypassed admission entirely — the
    # single-shot path is a test/dev helper, NOT a public entry point.
    # handle_run stays callable in-process (the byte-parity suite uses
    # it); the dispatch surface only accepts admitted actions.
    raise RuntimeError(
        f"poly-sheet action must be one of begin/frames/stitch/cancel/abandon, got {action!r}")


def _cancel_key(sheet_id, generation):
    return f"sheets/{sheet_id}/cancel_{generation}"


def _validated_sheet_id(params):
    sheet_id = str(params.get("sheet_id") or "").strip()
    if not SHEET_ID_RE.match(sheet_id):
        raise RuntimeError(f"sheet_id must match {SHEET_ID_RE.pattern}, got {sheet_id!r}")
    return sheet_id


def _validated_generation(params):
    generation = str(params.get("generation") or "").strip()
    if not GENERATION_RE.match(generation):
        raise RuntimeError(
            f"generation must match {GENERATION_RE.pattern}, got {generation!r} "
            "(obtain one from the begin action)")
    return generation


def handle_cancel(params):
    sheet_id = _validated_sheet_id(params)
    generation = _validated_generation(params)
    s3.put_object(Bucket=BUCKET, Key=_cancel_key(sheet_id, generation), Body=b"1")
    _mark_run_terminal(sheet_id, generation, "cancelled")
    return ok_response({"cancelled": sheet_id, "generation": generation})


def handle_abandon(params):
    """Client gave up resuming (round-3 finding 5): mark the run terminal
    so /list-sheets discovery stops rediscovering it. Generation-guarded,
    so it cannot abandon a newer run that took over the id."""
    sheet_id = _validated_sheet_id(params)
    generation = _validated_generation(params)
    _mark_run_terminal(sheet_id, generation, "abandoned")
    return ok_response({"abandoned": sheet_id, "generation": generation})


def _cancel_requested(sheet_id, generation):
    """Generation-scoped cancel check. Only a genuinely-absent marker
    means 'not cancelled'; an operational S3 error (throttle, 5xx) is
    retried once and then treated as not-cancelled — a transient blip
    must not kill legitimate work, and the next frame re-checks
    (CR35-F17: the old code swallowed EVERY ClientError as absence)."""
    for attempt in range(3):
        try:
            s3.head_object(Bucket=BUCKET, Key=_cancel_key(sheet_id, generation))
            return True
        except ClientError as exc:
            if is_missing_s3_error(exc):
                return False
            time.sleep(0.2 * (attempt + 1))
    # review round-2 finding 9: only a CONFIRMED-missing marker means
    # "not cancelled". Persistent operational failure fails CLOSED —
    # honoring a possible user cancel beats continuing blind (and with
    # S3 unreachable the tiles could not upload anyway).
    return True


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


def substitute_tokens(params, mapping):
    """ONE lexical substitution pass for every scan token (CR35-F14).
    Longest-match alternation, so $T never corrupts $T2; a token whose
    only occurrences sit inside # comments does not count as present
    (identical frames are a configuration mistake, not a sheet)."""
    if not mapping:
        raise RuntimeError("substitute_tokens requires at least one token")
    ordered = sorted(mapping, key=len, reverse=True)
    pattern = re.compile(
        "|".join(re.escape(token) for token in ordered) + r"(?![A-Za-z0-9_])")
    literals = {token: _value_literal(value) for token, value in mapping.items()}
    out = dict(params)
    for field in SOURCE_FIELDS:
        text = out.get(field)
        if not isinstance(text, str) or not text:
            continue
        out[field] = pattern.sub(lambda m: literals[m.group(0)], text)
    # presence: comment-stripped, boundary-aware, per token
    for token in ordered:
        present = 0
        for field in SOURCE_FIELDS:
            text = params.get(field)
            if not isinstance(text, str) or not text:
                continue
            live = COMMENT_RE.sub("", text)
            present += len(re.findall(
                re.escape(token) + r"(?![A-Za-z0-9_])", live))
        if present == 0:
            raise RuntimeError(
                f"scan token {token!r} does not appear (outside comments) "
                f"in any of {SOURCE_FIELDS}")
    return out


def substitute_token(params, token, value):
    """Single-token compatibility wrapper over substitute_tokens."""
    return substitute_tokens(params, {token: value})


def _run_binary(binary, out_path, spec, label, timeout_s=300, deadline_s=None):
    """deadline_s (epoch seconds): a hard budget wall. The subprocess
    timeout is clamped to the remaining budget so ONE oversized frame is
    killed with a clear error instead of running until Lambda kills the
    whole worker (round-3 finding 3: the loop-top check could admit a
    degree-255 frame that then ran unbounded)."""
    if deadline_s is not None:
        remaining = deadline_s - time.time()
        if remaining < 5:
            raise RuntimeError(
                f"{label} refused: {remaining:.0f}s left of the worker "
                "budget — per-frame cost outgrew the admission estimate")
        timeout_s = min(timeout_s, remaining)
    try:
        proc = subprocess.run(
            [binary, out_path], input=json.dumps(spec),
            capture_output=True, text=True, timeout=timeout_s)
    except subprocess.TimeoutExpired:
        raise RuntimeError(
            f"{label} exceeded its {timeout_s:.0f}s budget slice — the "
            "frame's degree/cost is beyond this worker's remaining time")
    if proc.returncode != 0:
        raise RuntimeError(f"{label} failed: {proc.stderr.strip()[:400]}")
    return json.loads(proc.stdout)


def _solve_frame(compiled, params, n, solver_mode, solver_iters=0, deadline_s=None):
    """One frame: grid coeffgen (+fused solve in-process) or the split
    solver from the f32 file. Returns (roots_bytes, degree).
    solver_iters mirrors the preview exactly: a cap for solve_mt and
    solve_newton; jt/cm and the fused trio ignore it (CR35-F22)."""
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
    meta = _run_binary(SWEEP_COEFFGEN, TMP_COEFFS, coeff_spec, "sheet coeffgen",
                       deadline_s=deadline_s)
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
            if solver_mode == "newton" and solver_iters:
                solve_spec["max_iter"] = solver_iters
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
            if solver_iters:
                solve_spec["max_iter"] = solver_iters
            binary = SWEEP_MT
        _run_binary(binary, TMP_ROOTS, solve_spec, "sheet solve",
                     deadline_s=deadline_s)

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
    raw_scans = params.get("scans")
    if not raw_scans:
        raw_scans = [params.get("scan") or {}]
    if not isinstance(raw_scans, list) or not 1 <= len(raw_scans) <= 2:
        raise RuntimeError(f"scans must be a list of 1..2 scan specs, got {raw_scans!r}")
    axes = [_parse_scan_axis(spec) for spec in raw_scans]
    if len(axes) == 2 and axes[0]["token"] == axes[1]["token"]:
        raise RuntimeError(f"scan tokens must be distinct, both are {axes[0]['token']!r}")
    steps = 1
    for axis in axes:
        steps *= axis["steps"]
    if not 1 <= steps <= MAX_STEPS:
        raise RuntimeError(
            f"total frames (product of scan steps) must be in 1..{MAX_STEPS}, got {steps}")

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
    solver_iters = int(frame.get("solver_iters") or 0)
    if not 0 <= solver_iters <= 64:
        raise RuntimeError(f"solver_iters must be in 0..64, got {solver_iters}")
    if solver_mode == "newton" and solver_iters > 50:
        raise RuntimeError(
            f"newton solver_iters must be <= 50 (native ceiling), got {solver_iters}")

    viewport = frame.get("viewport") or {}
    vp_mode = str(viewport.get("mode") or "quantile").strip().lower()
    if vp_mode not in ("quantile", "explicit", "frozen"):
        raise RuntimeError(f"viewport mode must be quantile/explicit/frozen, got {vp_mode!r}")
    # presence, not truthiness: an explicit 0 is a valid shim/quantile
    # (CR35-F15: `or 0.05` silently turned zero into 5 percent)
    quantile = float(viewport["quantile"]) if viewport.get("quantile") is not None else 0.0
    shim = float(viewport["shim"]) if viewport.get("shim") is not None else 0.05
    explicit_bounds = None
    if vp_mode == "explicit":
        try:
            explicit_bounds = _square_fit((
                viewport["min_re"], viewport["max_re"],
                viewport["min_im"], viewport["max_im"]))
        except KeyError as missing:
            raise RuntimeError(f"explicit viewport requires min/max re/im, missing {missing}")

    if len(axes) == 2:
        # cross product: axis 0 walks the columns, axis 1 walks the rows
        cols = axes[0]["steps"]
        if cols > MAX_COLS:
            raise RuntimeError(
                f"2-D sheet: first scan's steps become the columns and must be "
                f"<= {MAX_COLS}, got {cols}")
        rows = axes[1]["steps"]
    else:
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
        "sheet_id": sheet_id, "axes": axes, "steps": steps,
        "n": n, "tile_px": tile_px,
        "solver_mode": solver_mode, "rotate": rotate, "polarity": polarity,
        "margin_px": margin_px, "label": label, "solver_iters": solver_iters,
        "vp_mode": vp_mode, "quantile": quantile,
        "shim": shim, "explicit_bounds": explicit_bounds, "cols": cols,
        "rows": rows, "canvas_w": canvas_w, "canvas_h": canvas_h,
        "fg": fg, "bg": bg,
    }


def _parse_scan_axis(spec):
    """One scan line -> validated axis with resolved per-frame values.
    The step is resolved ONCE and that same value is executed, stored in
    the manifest, and therefore restored by Populate (CR35-F16: an
    omitted step used to execute as 1 but persist as 0)."""
    if not isinstance(spec, dict):
        raise RuntimeError(f"scan spec must be an object, got {spec!r}")
    token = str(spec.get("token") or "").strip()
    if not TOKEN_RE.match(token):
        raise RuntimeError(f"scan token must match {TOKEN_RE.pattern}, got {token!r}")
    steps = int(spec.get("steps") or 0)
    if not 1 <= steps <= MAX_STEPS:
        raise RuntimeError(f"scan steps must be in 1..{MAX_STEPS}, got {steps}")
    spacing = str(spec.get("spacing") or "linear").strip().lower()
    step = None
    if spacing == "step":
        step = float(spec["step"]) if spec.get("step") is not None else 1.0
    values = scan_values(spec.get("from"), spec.get("to") or 0.0, steps, spacing,
                         step=step)
    return {
        "token": token, "steps": steps, "spacing": spacing, "values": values,
        "from": float(spec.get("from")), "to": float(spec.get("to") or 0.0),
        "step": step,
    }


def _frame_values(cfg, k):
    """Frame k's value per axis. 2-D is row-major over the mosaic grid:
    axis 0 = column index (fastest), axis 1 = row index."""
    axes = cfg["axes"]
    if len(axes) == 1:
        return (axes[0]["values"][k],)
    s0 = axes[0]["steps"]
    return (axes[0]["values"][k % s0], axes[1]["values"][k // s0])


def _substitute_frame(params, cfg, k):
    """Substitute every axis token for frame k in ONE lexical pass."""
    values = _frame_values(cfg, k)
    return substitute_tokens(
        params, {axis["token"]: value
                 for axis, value in zip(cfg["axes"], values)})


def _require_job_task(params, action):
    job_id = str(params.get("job_id") or "").strip()
    task_id = str(params.get("task_id") or "").strip()
    if not job_id or not task_id:
        raise RuntimeError(f"poly-sheet {action} requires job_id and task_id")
    return job_id, task_id


def _budget_check(cfg, n_frames, what, degree=40):
    spp = _solve_us_per_step(solver_mode=cfg["solver_mode"], degree=degree,
                             fused_threads=2)
    est_us = int(n_frames * cfg["n"] * cfg["n"] * (spp + 3.0) * 1.4)
    if est_us > BUDGET_US:
        raise RuntimeError(
            f"{what} too large for one invocation: {n_frames} frames x "
            f"{cfg['n']}x{cfg['n']} rows with {cfg['solver_mode']} estimates "
            f"{est_us / 1e6:.0f}s > {BUDGET_US / 1e6:.0f}s budget — "
            f"reduce frames, N, or fan out wider")


def _render_frame_tile(cfg, params, k, frozen_cache, deadline_s=None):
    """Solve one frame and bin it. Returns (tile_bytes, record)."""
    values = _frame_values(cfg, k)
    frame_params = _substitute_frame(params, cfg, k)
    compiled = _compile_compute_inputs(frame_params)
    roots, degree = _solve_frame(compiled, frame_params, cfg["n"],
                                 cfg["solver_mode"], cfg["solver_iters"],
                                 deadline_s=deadline_s)
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
                fp0 = _substitute_frame(params, cfg, 0)
                roots0, _ = _solve_frame(_compile_compute_inputs(fp0), fp0,
                                         cfg["n"], cfg["solver_mode"],
                                         cfg["solver_iters"],
                                         deadline_s=deadline_s)
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
        tile = draw_tile_label(tile, cfg["tile_px"],
                               ",".join(f"{v:.6g}" for v in values),
                               cfg["fg"], cfg["bg"])
    record = {"frame": k,
              "value": values[0] if len(values) == 1 else list(values),
              "values": list(values), "degree": degree,
              # full binary64 precision: Populate Frame reconstructs the
              # viewport from these (round-3 finding 8)
              "bounds": [float(b) for b in bounds]}
    return tile, record


def _blit_tile(canvas, cfg, k, tile):
    row, col = divmod(k, cfg["cols"])
    tile_px, margin_px, canvas_w = cfg["tile_px"], cfg["margin_px"], cfg["canvas_w"]
    y0 = margin_px + row * (tile_px + margin_px)
    x0 = margin_px + col * (tile_px + margin_px)
    for y in range(tile_px):
        start = (y0 + y) * canvas_w + x0
        canvas[start:start + tile_px] = tile[y * tile_px:(y + 1) * tile_px]


def _tile_key(sheet_id, generation, k):
    return f"sheets/{sheet_id}/tiles/{generation}/{k:05d}.bin"


def _run_json_key(sheet_id):
    return f"sheets/{sheet_id}/run.json"


def _worker_task_id(sheet_id, generation, index):
    return f"sheet_tiles_{sheet_id}_{generation}_w{index}"


def _stitch_task_id(sheet_id, generation):
    return f"sheet_stitch_{sheet_id}_{generation}"


def _worker_ranges(steps, workers):
    ranges = [[] for _ in range(workers)]
    for k in range(steps):
        ranges[k * workers // steps].append(k)
    return [r for r in ranges if r]


def _sheet_manifest(cfg, params, t0, degree, frame_records, render_mode):
    scans = [{"token": a["token"], "from": a["from"], "to": a["to"],
              "step": a["step"], "steps": a["steps"], "spacing": a["spacing"],
              "values": [round(v, 12) for v in a["values"]]}
             for a in cfg["axes"]]
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
        "solver_iters": cfg["solver_iters"],
        "render_mode": render_mode,
        "pipeline": {key: params[key] for key in (
            "function", "cfpv", "param_transforms", "coeff_transforms",
            "param_program_chain", "coeff_program_chain",
            "param_program_source_text", "coeff_program_source_text")
            if params.get(key) not in (None, "", [])},
        "scan": scans[0],
        "scans": scans,
        "viewport": {"mode": cfg["vp_mode"], "quantile": cfg["quantile"],
                     "shim": cfg["shim"],
                     "explicit": (list(cfg["explicit_bounds"])
                                  if cfg["explicit_bounds"] else None)},
        "function": str(params.get("function") or ""),
        "frame_records": frame_records,
    }


def _gen_prefix(sheet_id, generation):
    return f"sheets/{sheet_id}/{generation}/"


def _write_generation_artifacts(cfg, generation, manifest, canvas):
    """Write the IMMUTABLE, generation-scoped mosaic + manifest. These
    keys are never overwritten by another generation, so any interleaving
    of stitches leaves each generation's bytes intact (round-3 finding 4:
    two mutable fixed writes could not be made atomic)."""
    prefix = _gen_prefix(cfg["sheet_id"], generation)
    png = encode_png_gray(cfg["canvas_w"], cfg["canvas_h"], bytes(canvas))
    s3.put_object(Bucket=BUCKET, Key=prefix + "sheet.png", Body=png,
                  ContentType="image/png")
    s3.put_object(Bucket=BUCKET, Key=prefix + "sheet.json",
                  Body=json.dumps(manifest, indent=1).encode("utf-8"),
                  ContentType="application/json")
    return prefix


def _cas_put_run(sheet_id, expected_etag, run):
    """Conditional run.json write (round-3 findings 2/3). The If-Match on
    the ETag captured in the SAME read is the atomic commit; there is NO
    unconditional fallback — an environment without S3 conditional writes
    FAILS CLOSED rather than silently clobbering a newer begin. Returns
    True on commit; raises 'superseded' when the precondition fails."""
    body = json.dumps(run, indent=1).encode("utf-8")
    kwargs = {"Bucket": BUCKET, "Key": _run_json_key(sheet_id), "Body": body,
              "ContentType": "application/json"}
    if expected_etag is None:
        kwargs["IfNoneMatch"] = "*"          # create-only
    else:
        kwargs["IfMatch"] = expected_etag
    try:
        s3.put_object(**kwargs)
        return True
    except ClientError as exc:
        code = (exc.response or {}).get("Error", {}).get("Code", "")
        if code in ("PreconditionFailed", "412", "ConditionalRequestConflict", "409"):
            raise RuntimeError(
                "publish refused: the run record changed under this stitch "
                "(a newer begin took over)")
        raise
    except (TypeError, ParamValidationError) as exc:
        raise RuntimeError(
            "publish refused: this runtime lacks S3 conditional writes, so a "
            "race-free commit is impossible — refusing to publish rather than "
            f"risk clobbering a newer run ({exc})")


def _commit_run_publication(sheet_id, generation, gen_prefix):
    """CAS run.json to point at the generation's immutable artifacts. The
    pointer is the ONLY authoritative publish; consumers resolve the
    current sheet through it (never the mutable fixed keys, which no
    longer exist for pointer-published sheets)."""
    resp = s3.get_object(Bucket=BUCKET, Key=_run_json_key(sheet_id))
    etag = resp.get("ETag")
    run = json.loads(resp["Body"].read())
    if run.get("generation") != generation:
        raise RuntimeError(
            f"publish refused: run superseded by {run.get('generation')}")
    run["status"] = "done"
    run["finished_at_s"] = time.time()
    run["published_generation"] = generation
    run["published_png_key"] = gen_prefix + "sheet.png"
    run["published_manifest_key"] = gen_prefix + "sheet.json"
    _cas_put_run(sheet_id, etag, run)


def _mark_run_terminal(sheet_id, generation, status):
    """Best-effort CAS to move run.json to a terminal state on worker
    error / cancel (round-3 finding 5: terminal runs stayed 'running'
    and were rediscovered forever). Generation-guarded; a lost race
    against a newer begin is fine — that newer run owns the record."""
    try:
        resp = s3.get_object(Bucket=BUCKET, Key=_run_json_key(sheet_id))
        etag = resp.get("ETag")
        run = json.loads(resp["Body"].read())
        if run.get("generation") != generation or run.get("status") != "running":
            return
        run["status"] = status
        run["finished_at_s"] = time.time()
        _cas_put_run(sheet_id, etag, run)
    except (ClientError, RuntimeError):
        pass


def _published_keys(run):
    """Resolve a run record to its current published (png_key, manifest_key),
    or None when nothing is published yet."""
    if not isinstance(run, dict):
        return None
    if run.get("published_png_key") and run.get("published_manifest_key"):
        return run["published_png_key"], run["published_manifest_key"]
    return None


# Fields that define the sheet's computation: the admission hash binds a
# dispatched worker/stitch payload to exactly what begin validated.
_HASHED_PARAM_FIELDS = (
    "sheet_id", "function", "cfpv", "scans", "scan", "frame", "grid_cols",
    "param_transforms", "coeff_transforms",
    "param_program_chain", "coeff_program_chain",
    "param_program_source_text", "coeff_program_source_text",
)


def _params_hash(params):
    payload = {k: params.get(k) for k in _HASHED_PARAM_FIELDS if params.get(k) is not None}
    canonical = json.dumps(payload, sort_keys=True, separators=(",", ":"))
    return hashlib.sha256(canonical.encode("utf-8")).hexdigest()[:16]


def _worker_already_done(job_id, task_id):
    try:
        return read_task_status(job_id, task_id) == "done"
    except Exception:
        return False


def _load_run(sheet_id):
    try:
        body = s3.get_object(Bucket=BUCKET, Key=_run_json_key(sheet_id))["Body"].read()
        run = json.loads(body)
        return run if isinstance(run, dict) else None
    except ClientError as exc:
        if is_missing_s3_error(exc):
            return None
        raise


def _bind_to_run(params, sheet_id, generation, task_id, *, frame_indices=None,
                 stitch=False):
    """Review round-2 finding 4: admission must be ENFORCED, not a client
    convention. A frames/stitch request is only valid if it matches the
    persisted run record: same generation, an allocated task id, the
    exact allocated frame range, and the admitted payload hash."""
    run = _load_run(sheet_id)
    if run is None:
        raise RuntimeError(
            f"no admitted run for sheet {sheet_id}; call begin first")
    if run.get("generation") != generation:
        raise RuntimeError(
            f"generation {generation} is not the admitted run "
            f"({run.get('generation')}); superseded or forged request")
    # round-3 finding 7: job identity is part of the admission — a
    # correctly-hashed payload must not redirect status writes to a
    # different DDB partition, and the hash itself is mandatory
    if str(params.get("job_id") or "") != str(run.get("job_id") or ""):
        raise RuntimeError(
            f"job_id {params.get('job_id')!r} does not match the admitted run")
    if not run.get("params_hash"):
        raise RuntimeError("admitted run carries no params_hash; re-run begin")
    if _params_hash(params) != run.get("params_hash"):
        raise RuntimeError(
            "request payload does not match the admitted configuration")
    if stitch:
        if task_id != run.get("stitch_task_id"):
            raise RuntimeError(f"task {task_id} is not the admitted stitch task")
    else:
        allocation = {w.get("task_id"): list(w.get("frames") or [])
                      for w in run.get("workers") or []}
        if task_id not in allocation:
            raise RuntimeError(f"task {task_id} is not an admitted worker task")
        if frame_indices is not None and sorted(frame_indices) != sorted(allocation[task_id]):
            raise RuntimeError(
                f"frame range for {task_id} does not match the admission")
    return run


def _probe_frame_cost(params, cfg):
    """Compile + 1x1-coeffgen probe of frame 0: the REAL degree and the
    measured per-frame compile cost feed the budget (CR35-F6: the old
    estimate hardcoded degree 40 and ignored compile entirely)."""
    t0 = time.time()
    frame_params = _substitute_frame(params, cfg, 0)
    compiled = _compile_compute_inputs(frame_params)
    compile_ms = (time.time() - t0) * 1000.0
    probe_spec = {
        "mode": "coeffgen",
        "function": str(frame_params["function"]),
        "param_transforms": compiled["param_transforms"],
        "coeff_transforms": compiled["coeff_transforms"],
        "n1": 1, "n2": 1, "i1_start": 0, "i1_end": 1, "times": 1,
    }
    for key, src in (("param_program", "param_program"),
                     ("coeff_program", "coeff_program"),
                     ("cfpv", "cfpv")):
        if compiled[src]:
            probe_spec[key] = compiled[src]
    meta = _run_binary(SWEEP_COEFFGEN, TMP_COEFFS, probe_spec, "sheet probe")
    return int(meta["degree"]), compile_ms


def handle_begin(params):
    """Synchronous admission: validate everything, probe frame 0,
    budget against the measured degree/compile cost, mint the run
    generation, pre-write every status row, persist run.json, and
    return the dispatch plan the client relays to the async workers."""
    job_id = str(params.get("job_id") or "").strip()
    if not job_id:
        raise RuntimeError("poly-sheet begin requires job_id")
    cfg = _parse_sheet_config(params)
    sheet_id = cfg["sheet_id"]
    steps = cfg["steps"]

    degree, compile_ms = _probe_frame_cost(params, cfg)
    spp = _solve_us_per_step(solver_mode=cfg["solver_mode"],
                             degree=max(2, degree), fused_threads=2)
    raster_us = cfg["tile_px"] * cfg["tile_px"] * 0.02 + degree * cfg["n"] * cfg["n"] * 0.05
    frame_us = compile_ms * 1000.0 + cfg["n"] * cfg["n"] * (spp + 3.0) + raster_us
    workers = min(SHEET_WORKERS, steps)
    frames_per_worker = math.ceil(steps / workers)
    est_worker_us = int(frames_per_worker * frame_us * 1.4)
    if est_worker_us > BUDGET_US:
        raise RuntimeError(
            f"sheet too large: {steps} frames at measured degree {degree} "
            f"({frames_per_worker} frames/worker x {frame_us / 1e6:.1f}s "
            f"estimated) exceeds the {BUDGET_US / 1e6:.0f}s worker budget — "
            "reduce frames or N, or switch solver")

    generation = "g" + os.urandom(6).hex()
    # stale-state hygiene: previous generations' tiles and markers can
    # only waste listing space now (keys are generation-scoped)
    ranges = _worker_ranges(steps, workers)
    worker_tasks = [_worker_task_id(sheet_id, generation, i)
                    for i in range(len(ranges))]
    stitch_task = _stitch_task_id(sheet_id, generation)
    # every status row exists BEFORE any async dispatch (CR35-F5): a
    # worker that dies pre-report leaves this row for the poll deadline
    for i, task in enumerate(worker_tasks):
        report_status(job_id, task, "started", result_data={
            "phase": "accepted", "phase_label": "Accepted",
            "sheet_id": sheet_id, "generation": generation,
            "frames": len(ranges[i]), "frame": 0,
        })
    report_status(job_id, stitch_task, "started", result_data={
        "phase": "accepted", "phase_label": "Waiting for workers",
        "sheet_id": sheet_id, "generation": generation,
    })
    run = {
        "sheet_id": sheet_id,
        "generation": generation,
        "job_id": job_id,
        "steps": steps,
        "workers": [{"task_id": t, "frames": r}
                    for t, r in zip(worker_tasks, ranges)],
        "stitch_task_id": stitch_task,
        "degree_probe": degree,
        "compile_ms_probe": round(compile_ms, 3),
        "est_worker_us": est_worker_us,
        "created_at_s": time.time(),
        "status": "running",
        # the admitted payload, verbatim: a resume (or any client) can
        # rebuild the exact dispatch jobs from the server record alone
        "params_hash": _params_hash(params),
        "payload": {k: params.get(k) for k in _HASHED_PARAM_FIELDS
                    if params.get(k) is not None},
    }
    s3.put_object(Bucket=BUCKET, Key=_run_json_key(sheet_id),
                  Body=json.dumps(run, indent=1).encode("utf-8"),
                  ContentType="application/json")
    return ok_response(run)


def handle_frames(params):
    """Fan-out worker: render the assigned frame subset, upload tiles.
    EVERYTHING after identity extraction runs inside the status guard:
    any failure — including config validation — writes a terminal error
    row (CR35-F5; the begin action already wrote the accepted row)."""
    job_id = str(params.get("job_id") or "").strip()
    task_id = str(params.get("task_id") or "").strip()
    if not job_id or not task_id:
        raise RuntimeError("poly-sheet frames requires job_id and task_id")
    # round-3 finding 7: bind BEFORE any status write. A rejected
    # (unauthenticated / mismatched) request must not be able to poison
    # a legitimate task's status row — validation failures here raise
    # without touching DDB.
    generation = _validated_generation(params)
    cfg = _parse_sheet_config(params)
    sheet_id = cfg["sheet_id"]
    indices = params.get("frame_indices")
    if not isinstance(indices, list) or not indices:
        raise RuntimeError("frames action requires a nonempty frame_indices list")
    indices = sorted({int(k) for k in indices})
    if indices[0] < 0 or indices[-1] >= cfg["steps"]:
        raise RuntimeError(
            f"frame_indices out of range 0..{cfg['steps'] - 1}: {indices}")
    run = _bind_to_run(params, sheet_id, generation, task_id,
                       frame_indices=indices)
    # duplicate/replayed worker: if this exact task already finished this
    # generation, do not regress its row (round-3 finding 7 secondary)
    if _worker_already_done(job_id, task_id):
        return ok_response({"sheet_id": sheet_id, "frames_done": len(indices),
                            "already_done": True})
    try:
        _budget_check(cfg, len(indices) + (1 if cfg["vp_mode"] == "frozen" else 0),
                      "worker share",
                      degree=max(2, int(run.get("degree_probe") or 40)))
        _substitute_frame(params, cfg, indices[0])
        worker_deadline_s = time.time() + BUDGET_US / 1e6

        report_status(job_id, task_id, "running", result_data={
            "phase": "sheet", "phase_label": "Sheet frames", "task_id": task_id,
            "sheet_id": sheet_id, "generation": generation,
            "frames": len(indices), "frame": 0,
        })
        frozen_cache = {}
        done = 0
        for k in indices:
            if _cancel_requested(sheet_id, generation):
                report_status(job_id, task_id, "error", "Cancelled by user",
                              result_data={"phase": "error",
                                           "phase_label": "Cancelled", "task_id": task_id,
                                           "sheet_id": sheet_id, "frame": done})
                _mark_run_terminal(sheet_id, generation, "cancelled")
                return ok_response({"cancelled": sheet_id, "frames_done": done})
            if time.time() > worker_deadline_s:
                raise RuntimeError(
                    f"worker exceeded its {BUDGET_US / 1e6:.0f}s budget at "
                    f"frame {k} ({done}/{len(indices)} done) — per-frame cost "
                    "grew beyond the admission estimate (scan tokens can "
                    "raise the degree); reduce frames or N")
            tile, record = _render_frame_tile(cfg, params, k, frozen_cache,
                                              deadline_s=worker_deadline_s)
            key = _tile_key(sheet_id, generation, k)
            s3.put_object(Bucket=BUCKET, Key=key,
                          Body=bytes(tile), ContentType="application/octet-stream")
            s3.put_object(Bucket=BUCKET, Key=key.replace(".bin", ".json"),
                          Body=json.dumps(record).encode("utf-8"),
                          ContentType="application/json")
            done += 1
            report_status(job_id, task_id, "running", result_data={
                "phase": "sheet", "phase_label": f"{done}/{len(indices)} frames",
                "task_id": task_id, "sheet_id": sheet_id, "generation": generation,
                "frames": len(indices), "frame": done,
            })
        report_status(job_id, task_id, "done", result_data={
            "phase": "done", "phase_label": "Worker done", "task_id": task_id,
            "sheet_id": sheet_id, "generation": generation,
            "frames": len(indices), "frame": done,
        })
        return ok_response({"sheet_id": sheet_id, "frames_done": done})
    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data={
            "phase": "error", "phase_label": "Sheet worker failed", "task_id": task_id,
        })
        _mark_run_terminal(sheet_id, generation, "failed")
        raise
    finally:
        for path in (TMP_COEFFS, TMP_ROOTS):
            try:
                os.remove(path)
            except OSError:
                pass


def handle_stitch(params):
    """Assemble ONE generation's tiles into the final mosaic + manifest.
    PUBLICATION IS THE COMMIT POINT (CR35-F18): once sheet.png and
    sheet.json are durably written the run is done — tile cleanup is
    best-effort garbage collection whose failure is recorded, never an
    error that contradicts the published artifact."""
    job_id = str(params.get("job_id") or "").strip()
    task_id = str(params.get("task_id") or "").strip()
    if not job_id or not task_id:
        raise RuntimeError("poly-sheet stitch requires job_id and task_id")
    # bind before any status write (round-3 finding 7)
    generation = _validated_generation(params)
    cfg = _parse_sheet_config(params)
    sheet_id = cfg["sheet_id"]
    _bind_to_run(params, sheet_id, generation, task_id, stitch=True)
    if _worker_already_done(job_id, task_id):
        return ok_response({"sheet_id": sheet_id, "already_done": True})
    try:
        t0 = float(params.get("started_at_s") or time.time())

        report_status(job_id, task_id, "running", result_data={
            "phase": "stitch", "phase_label": "Stitching",
            "sheet_id": sheet_id, "generation": generation,
            "frames": cfg["steps"],
        })
        if _cancel_requested(sheet_id, generation):
            report_status(job_id, task_id, "error", "Cancelled by user",
                          result_data={"phase": "error", "phase_label": "Cancelled",
                                       "sheet_id": sheet_id})
            return ok_response({"cancelled": sheet_id, "frames_done": 0})

        tile_bytes = cfg["tile_px"] * cfg["tile_px"]
        canvas = bytearray(bytes([cfg["bg"]]) * (cfg["canvas_w"] * cfg["canvas_h"]))
        frame_records = []
        missing = []
        for k in range(cfg["steps"]):
            key = _tile_key(sheet_id, generation, k)
            try:
                tile = s3.get_object(Bucket=BUCKET, Key=key)["Body"].read()
                record = json.loads(
                    s3.get_object(Bucket=BUCKET,
                                  Key=key.replace(".bin", ".json"))["Body"].read())
            except ClientError as exc:
                if is_missing_s3_error(exc):
                    missing.append(k)
                    continue
                raise
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

        # the sheet-wide degree is the MAX over frames: scan tokens can
        # change the degree per frame (review round-2 finding 5)
        degrees = [int(r.get("degree") or 0) for r in frame_records]
        degree = max(degrees) if degrees else None
        manifest = _sheet_manifest(cfg, params, t0, degree, frame_records, "fanout")
        manifest["generation"] = generation
        # round-3 finding 4: publish to GENERATION-scoped keys first
        # (collision-free), win the conditional commit on run.json, and
        # only then copy to the fixed public keys — a superseded stitch
        # fails the commit and never touches what viewers read
        gen_prefix = _write_generation_artifacts(cfg, generation, manifest, canvas)
        manifest["png_key"] = gen_prefix + "sheet.png"
        manifest["manifest_key"] = gen_prefix + "sheet.json"
        # re-write the manifest now that it carries its own final keys,
        # then the pointer commit makes it authoritative (round-3 f4)
        s3.put_object(Bucket=BUCKET, Key=gen_prefix + "sheet.json",
                      Body=json.dumps(manifest, indent=1).encode("utf-8"),
                      ContentType="application/json")
        _commit_run_publication(sheet_id, generation, gen_prefix)   # <- COMMIT

        cleanup_ok = True
        try:
            delete_keys = []
            for k in range(cfg["steps"]):
                key = _tile_key(sheet_id, generation, k)
                delete_keys.append({"Key": key})
                delete_keys.append({"Key": key.replace(".bin", ".json")})
            delete_keys.append({"Key": _cancel_key(sheet_id, generation)})
            for i in range(0, len(delete_keys), 1000):
                resp = s3.delete_objects(
                    Bucket=BUCKET,
                    Delete={"Objects": delete_keys[i:i + 1000], "Quiet": True})
                if resp.get("Errors"):
                    cleanup_ok = False
        except Exception:
            cleanup_ok = False

        report_status(job_id, task_id, "done", result_data={
            "phase": "done", "phase_label": "Done",
            "sheet_id": sheet_id, "generation": generation,
            "png_key": manifest["png_key"],
            "frames": cfg["steps"], "elapsed_ms": manifest["elapsed_ms"],
            "cleanup_ok": cleanup_ok,
        })
        return ok_response(manifest)
    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data={
            "phase": "error", "phase_label": "Stitch failed", "task_id": task_id,
        })
        _mark_run_terminal(sheet_id, generation, "failed")
        raise


def handle_run(params):
    job_id, task_id = _require_job_task(params, "run")
    cfg = _parse_sheet_config(params)
    sheet_id = cfg["sheet_id"]
    steps = cfg["steps"]
    generation = str(params.get("generation") or "").strip() or "g000000000000"
    _budget_check(cfg, steps, "sheet")

    # every token must hit at least once (validated on frame 0's values)
    _substitute_frame(params, cfg, 0)

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
            if _cancel_requested(sheet_id, generation):
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
        # in-process single-shot path (test/dev only — not dispatchable):
        # writes the fixed keys directly, no pointer commit
        png = encode_png_gray(cfg["canvas_w"], cfg["canvas_h"], bytes(canvas))
        s3.put_object(Bucket=BUCKET, Key=f"sheets/{sheet_id}/sheet.png",
                      Body=png, ContentType="image/png")
        s3.put_object(Bucket=BUCKET, Key=f"sheets/{sheet_id}/sheet.json",
                      Body=json.dumps(manifest, indent=1).encode("utf-8"),
                      ContentType="application/json")
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
