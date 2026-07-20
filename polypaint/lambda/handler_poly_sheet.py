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
import logging
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
    claim_task,
    finalize_task,
    LeaseHeartbeat,
    read_task_status,
    renew_claim,
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
ATTEMPT_RE = re.compile(r"^[A-Za-z0-9_-]{1,128}$")
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

RUN_ACTIVE = "active"
RUN_INACTIVE = "inactive"
RUN_UNKNOWN = "unknown"
RUN_TERMINAL_STATUSES = frozenset(("done", "failed", "cancelled", "abandoned"))

# SQS is the durable quiescence boundary for generation cleanup. Lambda's
# longest invocation is 900 seconds; cleanup waits longer than that after a
# terminal/superseding run.json write so no pre-transition writer can still be
# alive. SQS delays top out at 900 seconds, so messages retain not_before_s and
# requeue themselves for the remainder.
POLY_SHEET_GC_QUEUE_URL = os.environ.get("POLY_SHEET_GC_QUEUE_URL", "")
SHEET_GC_QUIESCENCE_S = 930
SHEET_GC_MAX_DELAY_S = 900
SHEET_GC_RUNNING_RECHECK_S = 120
SHEET_GC_MESSAGE_TYPE = "poly-sheet-generation-gc-v1"
_sqs = None

logger = logging.getLogger(__name__)


def handler(event, context):
    if _is_sqs_gc_event(event):
        for record in event["Records"]:
            _handle_generation_gc_message(json.loads(record["body"]))
        return {"processed": len(event["Records"])}

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
    # the marker is a best-effort FAST hint so a worker stops between frames
    # without waiting for run.json; it is NOT the authoritative decision
    # (round-9 finding 1) — that is the run.json CAS below. round-10 finding
    # 3: swallow ANY error here (not just ClientError — an
    # EndpointConnectionError is NOT a ClientError and previously aborted
    # cancel before the authoritative CAS ever ran).
    try:
        s3.put_object(Bucket=BUCKET, Key=_cancel_key(sheet_id, generation), Body=b"1")
    except Exception:
        pass
    # cancellation competes with publication through the SAME run.json CAS.
    # The returned status is the ACTUAL outcome: 'cancelled' if we won,
    # 'done' if a publish beat us (cancel came too late), etc.
    final, run = _mark_run_terminal(sheet_id, generation, "cancelled")
    if final is None:
        # round-9 finding 2: could NOT confirm the transition — fail loudly
        # (the async invocation errors, the client's run.json poll stays
        # 'running', and it retries) rather than reporting a false success.
        raise RuntimeError(
            "cancel could not be confirmed: run.json unreachable — retry")
    _schedule_terminal_gc_from_run(sheet_id, generation, run)
    return ok_response({"sheet_id": sheet_id, "generation": generation,
                        "status": final, "cancelled": final == "cancelled"})


def handle_abandon(params):
    """Client gave up resuming (round-3 finding 5): mark the run terminal
    so /list-sheets discovery stops rediscovering it. Generation-guarded,
    so it cannot abandon a newer run that took over the id."""
    sheet_id = _validated_sheet_id(params)
    generation = _validated_generation(params)
    final, run = _mark_run_terminal(sheet_id, generation, "abandoned")
    if final is None:
        raise RuntimeError(
            "abandon could not be confirmed: run.json unreachable — retry")
    _schedule_terminal_gc_from_run(sheet_id, generation, run)
    return ok_response({"sheet_id": sheet_id, "generation": generation,
                        "status": final})


def _cancel_requested(sheet_id, generation):
    """Generation-scoped best-effort cancel HINT (a fast worker-stop signal
    between frames; the authoritative cancel decision is the run.json CAS).
    Only a genuinely-absent marker means 'not cancelled'. An operational S3
    error (throttle, 5xx) is retried a couple times and then FAILS CLOSED —
    returns True — because with S3 unreachable the tiles could not upload
    anyway and honoring a possible cancel beats blind work (round-9 finding
    7: the docstring previously CLAIMED 'not-cancelled' but the code has
    returned True on persistent failure since CR35-F17)."""
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


def _put_object_once(key, body, content_type):
    """Write-once create-only PUT (round-10 finding 4). If-None-Match='*'
    makes the object immutable once written, so a delayed STALE worker
    cannot overwrite a successor's tile — the second write is rejected. The
    render is deterministic, so an already-present object is byte-equivalent
    and 'already exists' is a benign success. Round-12 finding 6: FAIL
    CLOSED on a runtime without conditional writes (the old fallback to an
    unconditional put reintroduced the very overwrite this guards against);
    admission already implies conditional-write support, so this never
    fires in production but the helper no longer degrades silently."""
    try:
        s3.put_object(Bucket=BUCKET, Key=key, Body=body,
                      ContentType=content_type, IfNoneMatch="*")
    except ClientError as exc:
        code = (exc.response or {}).get("Error", {}).get("Code", "")
        if code in ("PreconditionFailed", "412", "ConditionalRequestConflict", "409"):
            return False                 # already written by another attempt
        raise
    except (TypeError, ParamValidationError) as exc:
        raise RuntimeError(
            "write-once tile refused: this runtime lacks S3 conditional "
            f"writes, so a stale worker could overwrite a successor ({exc})")
    return True


def _run_json_key(sheet_id):
    return f"sheets/{sheet_id}/run.json"


def _is_sqs_gc_event(event):
    records = event.get("Records") if isinstance(event, dict) else None
    return bool(records) and all(
        isinstance(record, dict)
        and record.get("eventSource") == "aws:sqs"
        and isinstance(record.get("body"), str)
        for record in records)


def _get_sqs():
    global _sqs
    if _sqs is None:
        _sqs = boto3.client("sqs")
    return _sqs


def _validated_gc_identity(message):
    if not isinstance(message, dict) or message.get("type") != SHEET_GC_MESSAGE_TYPE:
        raise RuntimeError("invalid poly-sheet GC message type")
    sheet_id = str(message.get("sheet_id") or "").strip()
    generation = str(message.get("generation") or "").strip()
    if not SHEET_ID_RE.fullmatch(sheet_id):
        raise RuntimeError("invalid poly-sheet GC sheet_id")
    if not GENERATION_RE.fullmatch(generation):
        raise RuntimeError("invalid poly-sheet GC generation")
    try:
        steps = int(message.get("steps"))
        not_before_s = float(message.get("not_before_s"))
    except (TypeError, ValueError):
        raise RuntimeError("invalid poly-sheet GC steps/deadline")
    if not 1 <= steps <= MAX_STEPS or not math.isfinite(not_before_s):
        raise RuntimeError("invalid poly-sheet GC steps/deadline")
    return sheet_id, generation, steps, not_before_s


def _send_generation_gc(message):
    if not POLY_SHEET_GC_QUEUE_URL:
        # Local tests call admission directly without deployment resources.
        # A deployed Lambda must never silently lose its crash-cleanup path.
        if os.environ.get("AWS_LAMBDA_FUNCTION_NAME"):
            raise RuntimeError("POLY_SHEET_GC_QUEUE_URL is not configured")
        return False
    _, _, _, not_before_s = _validated_gc_identity(message)
    delay = max(0, min(SHEET_GC_MAX_DELAY_S,
                       int(math.ceil(not_before_s - time.time()))))
    _get_sqs().send_message(
        QueueUrl=POLY_SHEET_GC_QUEUE_URL,
        DelaySeconds=delay,
        MessageBody=json.dumps(message, separators=(",", ":"), sort_keys=True),
    )
    return True


def _schedule_generation_gc(sheet_id, generation, steps, *, not_before_s=None):
    message = {
        "type": SHEET_GC_MESSAGE_TYPE,
        "sheet_id": sheet_id,
        "generation": generation,
        "steps": int(steps),
        "not_before_s": (float(not_before_s) if not_before_s is not None
                         else time.time() + SHEET_GC_QUIESCENCE_S),
    }
    _validated_gc_identity(message)
    return _send_generation_gc(message)


def _schedule_terminal_gc_from_run(sheet_id, generation, run):
    if not isinstance(run, dict) or run.get("generation") != generation:
        return False
    try:
        steps = int(run.get("steps"))
    except (TypeError, ValueError):
        return False
    try:
        finished_at_s = float(run.get("finished_at_s"))
    except (TypeError, ValueError):
        finished_at_s = time.time()
    if not math.isfinite(finished_at_s):
        finished_at_s = time.time()
    return _schedule_generation_gc(
        sheet_id, generation, steps,
        not_before_s=finished_at_s + SHEET_GC_QUIESCENCE_S)


def _gc_requeue(message, not_before_s):
    message = dict(message)
    message["not_before_s"] = float(not_before_s)
    _send_generation_gc(message)
    return {"state": "deferred", "not_before_s": float(not_before_s)}


def _object_modified_s(response):
    modified = response.get("LastModified") if isinstance(response, dict) else None
    if modified is not None and hasattr(modified, "timestamp"):
        try:
            value = float(modified.timestamp())
            if math.isfinite(value):
                return value
        except (TypeError, ValueError, OverflowError):
            pass
    return None


def _handle_generation_gc_message(message):
    """Reap one generation only after all pre-terminal writers must be dead.

    The durable run record decides whether the generation is active and which
    attempt, if any, is still published. Any unreadable/malformed state raises
    so SQS retries; destructive cleanup is never based on a guess.
    """
    sheet_id, generation, message_steps, not_before_s = _validated_gc_identity(message)
    now = time.time()
    if now < not_before_s:
        return _gc_requeue(message, not_before_s)

    response = None
    run = None
    try:
        response = s3.get_object(Bucket=BUCKET, Key=_run_json_key(sheet_id))
        run = json.loads(response["Body"].read())
        if not isinstance(run, dict):
            raise RuntimeError("run.json must contain an object")
    except ClientError as exc:
        if not is_missing_s3_error(exc):
            raise

    winner = None
    steps = message_steps
    inactive_at_s = None
    if run is None:
        # No admitted run can authorize a writer. The initial message already
        # waited one complete Lambda lifetime from admission.
        inactive_at_s = not_before_s - SHEET_GC_QUIESCENCE_S
    elif run.get("generation") == generation:
        status = run.get("status")
        if status == "running":
            return _gc_requeue(message, now + SHEET_GC_RUNNING_RECHECK_S)
        if status not in RUN_TERMINAL_STATUSES:
            raise RuntimeError(f"poly-sheet GC found unknown run status {status!r}")
        try:
            run_steps = int(run.get("steps"))
        except (TypeError, ValueError):
            raise RuntimeError("poly-sheet GC found invalid run steps")
        if run_steps != message_steps:
            raise RuntimeError("poly-sheet GC message/run step count mismatch")
        steps = run_steps
        try:
            inactive_at_s = float(run.get("finished_at_s"))
        except (TypeError, ValueError):
            inactive_at_s = _object_modified_s(response)
        if status == "done":
            winner = _winner_prefix_from_run(run, sheet_id, generation)
            if winner is None:
                raise RuntimeError("poly-sheet GC cannot validate published winner")
    else:
        # The current record was written by the superseding begin. Its
        # creation time (or S3 LastModified) is the earliest safe quiescence
        # anchor for the old generation.
        try:
            inactive_at_s = float(run.get("created_at_s"))
        except (TypeError, ValueError):
            inactive_at_s = _object_modified_s(response)
        if run.get("published_generation") == generation:
            winner = _published_winner_prefix(run, sheet_id, generation)
            if winner is None:
                raise RuntimeError("poly-sheet GC found inconsistent carried winner")

    if inactive_at_s is None or not math.isfinite(inactive_at_s):
        raise RuntimeError("poly-sheet GC cannot establish the quiescence timestamp")
    quiescent_at_s = inactive_at_s + SHEET_GC_QUIESCENCE_S
    if now < quiescent_at_s:
        return _gc_requeue(message, quiescent_at_s)

    if not _reap_sheet_scaffolding(sheet_id, generation, steps, winner):
        raise RuntimeError("poly-sheet generation GC was incomplete")
    logger.info("poly-sheet GC reaped %s/%s", sheet_id, generation)
    return {"state": "reaped", "sheet_id": sheet_id, "generation": generation}


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


def _attempt_token(owner):
    """The per-claim random suffix of an owner id (owner = task_id ':' hex),
    used as a filesystem-safe, collision-free attempt prefix segment."""
    return str(owner).rsplit(":", 1)[-1] or "a"


def _attempt_prefix(sheet_id, generation, owner):
    return f"sheets/{sheet_id}/{generation}/{_attempt_token(owner)}/"


def _write_generation_artifacts(cfg, generation, manifest, canvas, owner,
                                *, prefix=None):
    """Write the mosaic + manifest to an ATTEMPT-scoped prefix
    (round-8 finding 2). Two stitchers of the SAME generation (the
    original plus a lease-expiry redispatch) each own a distinct random
    attempt segment, so a stale stitch writes ONLY under its own prefix
    and can never overwrite the winner's supposedly-immutable PNG or
    manifest. The winning attempt is the one whose CAS on run.json lands;
    losing attempts leave orphaned objects that family-scoped GC reaps.
    (Round-3 finding 4 established generation scoping; attempt scoping
    closes the same-generation two-stitcher overwrite the reviewer found.)"""
    prefix = prefix or _attempt_prefix(cfg["sheet_id"], generation, owner)
    png = encode_png_gray(cfg["canvas_w"], cfg["canvas_h"], bytes(canvas))
    try:
        s3.put_object(Bucket=BUCKET, Key=prefix + "sheet.png", Body=png,
                      ContentType="image/png")
        s3.put_object(Bucket=BUCKET, Key=prefix + "sheet.json",
                      Body=json.dumps(manifest, indent=1).encode("utf-8"),
                      ContentType="application/json")
    except Exception:
        # The prefix is attempt-private and no publication CAS has run yet,
        # so both keys are unambiguously ours. Clean a one-object partial
        # write immediately; durable generation GC is the crash backstop.
        _prune_own_attempt(cfg["sheet_id"], prefix)
        raise
    return prefix


def _prune_own_attempt(sheet_id, gen_prefix):
    """Round-9 finding 5: a stitch that WROTE its attempt artifacts but then
    lost the lease or the commit CAS cleans its OWN prefix immediately,
    rather than relying solely on the winner's one-shot sweep (which can
    miss a late writer). Best-effort."""
    try:
        s3.delete_objects(Bucket=BUCKET, Delete={"Objects": [
            {"Key": gen_prefix + "sheet.png"},
            {"Key": gen_prefix + "sheet.json"},
        ], "Quiet": True})
    except Exception:
        pass


def _prune_own_attempt_if_not_published(sheet_id, generation, gen_prefix):
    """Prune this attempt's artifacts on a FAILED/ambiguous commit — but
    ONLY when our own conditional CAS can no longer land (round-12 finding
    2). A single re-read that sees 'running' does NOT prove a timed-out CAS
    will not subsequently publish these keys, so deleting then would leave
    the pointer dangling. Delete only when run.json has DEMONSTRABLY moved
    past the point our If-Match could still succeed:

      - the pointer already references OUR prefix -> we won; keep.
      - a NEWER generation owns the record -> our CAS will 412; safe to prune.
      - this generation is TERMINAL (done-by-another/cancelled/abandoned/
        failed) -> the ETag has moved, our CAS will 412; safe to prune.
      - still 'running' for this generation, or unconfirmable -> our CAS
        might still land; DEFER to the winner's family sweep (GC), do not
        delete."""
    try:
        resp = s3.get_object(Bucket=BUCKET, Key=_run_json_key(sheet_id))
        run = json.loads(resp["Body"].read())
    except Exception:
        return                                  # cannot confirm -> defer to GC
    if not isinstance(run, dict):
        return                                  # malformed -> never delete
    # round-16 finding 2 discipline: pointer-ours is checked against BOTH
    # published keys (OR = conservative KEEP) — if either key references our
    # prefix, deleting would break a live reference (possibly carried
    # forward by a superseding begin).
    if (run.get("published_png_key") == gen_prefix + "sheet.png"
            or run.get("published_manifest_key") == gen_prefix + "sheet.json"):
        return                                  # our (ambiguous) CAS actually won
    if run.get("generation") != generation:
        _prune_own_attempt(sheet_id, gen_prefix)    # a newer generation owns it
        return
    if run.get("status") in ("done", "cancelled", "abandoned", "failed"):
        _prune_own_attempt(sheet_id, gen_prefix)    # terminal -> our CAS can't land
        return
    # status == 'running' (or unknown) for THIS generation: the timed-out
    # CAS could still publish -> leave the objects for the winner's sweep


def _run_write_state(sheet_id, generation, attempts=3):
    """Return (active|inactive|unknown, run) for a shared-write fence.

    A confirmed terminal state or generation mismatch is INACTIVE. A
    transient S3/decode failure is UNKNOWN, never terminal: callers must not
    delete data or return a successful "run ended" response from an unknown
    observation. Confirmed NoSuchKey is inactive because S3 is strongly
    consistent and no admitted generation remains to authorize the write.
    """
    attempts = max(1, int(attempts))
    for attempt in range(attempts):
        try:
            resp = s3.get_object(Bucket=BUCKET, Key=_run_json_key(sheet_id))
            run = json.loads(resp["Body"].read())
            if not isinstance(run, dict):
                raise ValueError("run.json must contain an object")
        except ClientError as exc:
            if is_missing_s3_error(exc):
                return RUN_INACTIVE, None
            if attempt + 1 < attempts:
                time.sleep(0.05 * (attempt + 1))
            continue
        except Exception:
            if attempt + 1 < attempts:
                time.sleep(0.05 * (attempt + 1))
            continue

        if run.get("generation") != generation:
            return RUN_INACTIVE, run
        status = run.get("status")
        if status == "running":
            return RUN_ACTIVE, run
        if status in RUN_TERMINAL_STATUSES:
            return RUN_INACTIVE, run
        return RUN_UNKNOWN, run
    return RUN_UNKNOWN, None


def _confirmed_run_inactive_response(sheet_id, generation, run, **extra):
    status = (run.get("status") if isinstance(run, dict)
              and run.get("generation") == generation else SUPERSEDED)
    return ok_response({"sheet_id": sheet_id, "generation": generation,
                        "run_terminal": True, "status": status, **extra})


def _delete_keys_best_effort(keys):
    """Round-16 findings 1/3: a fenced-out writer deletes the objects IT
    wrote in this invocation. Once the run is inactive (terminal or
    superseded) those keys are provably garbage — generation-keyed and
    referenced by nothing — so the writer self-cleans instead of relying on
    a cleanup pass that cannot know about in-flight writes. NEVER called on
    a lost-lease exit: there a successor legitimately shares these keys."""
    if not keys:
        return
    try:
        s3.delete_objects(Bucket=BUCKET, Delete={
            "Objects": [{"Key": k} for k in keys], "Quiet": True})
    except Exception:
        pass


def _reap_sheet_scaffolding(sheet_id, generation, steps, winner_prefix):
    """Cleanup for a generation: delete the frame tiles + cancel marker and
    prune attempt artifacts. winner_prefix=<prefix> keeps the winner and
    prunes the losers (post-publication); winner_prefix=None means NO winner
    (a failed/cancelled run) and prunes ALL attempts (round-14 finding 4).
    Callers must first establish quiescence unless publication proved all
    workers complete. Idempotent; returns True on a clean sweep and False on
    any delete/list error."""
    ok = True
    try:
        delete_keys = []
        for k in range(steps):
            key = _tile_key(sheet_id, generation, k)
            delete_keys.append({"Key": key})
            delete_keys.append({"Key": key.replace(".bin", ".json")})
        delete_keys.append({"Key": _cancel_key(sheet_id, generation)})
        for i in range(0, len(delete_keys), 1000):
            resp = s3.delete_objects(
                Bucket=BUCKET,
                Delete={"Objects": delete_keys[i:i + 1000], "Quiet": True})
            if resp.get("Errors"):
                ok = False
        # round-9 finding 8: reap the losing stitch attempts' orphaned
        # png/manifest objects (the winner is winner_prefix)
        if not _prune_losing_attempts(sheet_id, generation, winner_prefix):
            ok = False
    except Exception:
        ok = False
    return ok


def _prune_losing_attempts(sheet_id, generation, winning_prefix):
    """Round-9 finding 8: reap the artifacts of LOSING same-generation
    stitch attempts. Each attempt writes under sheets/{id}/{gen}/{token}/
    and exactly one token wins the run.json CAS; the others are orphans.
    Delete every object under the generation prefix that is NOT the winner's.
    Round-14 finding 4: winning_prefix=None means there is NO winner (a
    failed/cancelled run) — delete ALL attempt objects under the generation.
    Best-effort (part of cleanup_ok): returns True on a clean sweep, False on
    any list/delete error."""
    gen_root = _gen_prefix(sheet_id, generation)     # sheets/{id}/{gen}/
    legacy_winner_keys = ({gen_root + "sheet.png", gen_root + "sheet.json"}
                          if winning_prefix == gen_root else set())

    def is_winner(key):
        if winning_prefix is None:
            return False
        if legacy_winner_keys:
            return key in legacy_winner_keys
        return key.startswith(winning_prefix)

    ok = True
    try:
        token = None
        while True:
            kwargs = {"Bucket": BUCKET, "Prefix": gen_root}
            if token:
                kwargs["ContinuationToken"] = token
            resp = s3.list_objects_v2(**kwargs)
            victims = [{"Key": o["Key"]} for o in resp.get("Contents", [])
                       if not is_winner(o["Key"])]
            for i in range(0, len(victims), 1000):
                d = s3.delete_objects(
                    Bucket=BUCKET,
                    Delete={"Objects": victims[i:i + 1000], "Quiet": True})
                if d.get("Errors"):
                    ok = False
            if not resp.get("IsTruncated"):
                break
            token = resp.get("NextContinuationToken")
    except Exception:
        ok = False
    return ok


class _CASConflict(RuntimeError):
    """The run.json If-Match precondition failed — a concurrent writer
    changed the record. RETRYABLE: the caller re-reads and re-evaluates.
    Distinct from the fatal no-conditional-writes RuntimeError, which must
    propagate (fail closed), never be swallowed as a retryable conflict
    (round-9 finding: the commit loop was masking the fail-closed error)."""


def _cas_put_run(sheet_id, expected_etag, run):
    """Conditional run.json write (round-3 findings 2/3). The If-Match on
    the ETag captured in the SAME read is the atomic commit; there is NO
    unconditional fallback — an environment without S3 conditional writes
    FAILS CLOSED rather than silently clobbering a newer begin. Returns
    True on commit; raises _CASConflict when the precondition fails."""
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
            raise _CASConflict(
                "run record changed under this write (concurrent transition)")
        raise
    except (TypeError, ParamValidationError) as exc:
        raise RuntimeError(
            "publish refused: this runtime lacks S3 conditional writes, so a "
            "race-free commit is impossible — refusing to publish rather than "
            f"risk clobbering a newer run ({exc})")


def _commit_run_publication(sheet_id, generation, gen_prefix):
    """CAS run.json to point at the generation's immutable artifacts.
    Publication competes with cancel/abandon/failure through the SAME
    run.json CAS (round-9 finding 1) so exactly one terminal outcome wins.
    Returns the RESOLVED status:
      - 'done' if we published (or this generation was already published), OR
      - 'cancelled'/'abandoned'/'failed' if a terminal decision beat us —
        the stitch then reports that outcome instead of publishing.
    Raises 'superseded' only when a NEWER generation owns the record.

    The loop re-reads on every CAS conflict and re-evaluates, so a cancel
    that lands between our read and our CAS is observed (not blindly
    overwritten) — the reproduced marker/publish race can no longer resolve
    to a wrongly-published 'done'.

    Returns (status, won_by_this_attempt): `won` is decided by whether
    run.json's POINTER references THIS attempt's prefix, NOT by which
    invocation observed the CAS success (round-12 finding 1). A
    network-ambiguous CAS — S3 applies the write but the SDK retries and
    observes 412 — re-reads to find OUR own publication; comparing the
    pointer keys correctly reports won=True there, so the caller does not
    delete the winner it just wrote."""
    def _pointer_is_ours(run):
        return (run.get("published_png_key") == gen_prefix + "sheet.png"
                and run.get("published_manifest_key") == gen_prefix + "sheet.json")

    for _ in range(6):
        resp = s3.get_object(Bucket=BUCKET, Key=_run_json_key(sheet_id))
        etag = resp.get("ETag")
        run = json.loads(resp["Body"].read())
        if not isinstance(run, dict):
            raise RuntimeError("publish refused: run.json must contain an object")
        if run.get("generation") != generation:
            raise RuntimeError(
                f"publish refused: run superseded by {run.get('generation')}")
        status = run.get("status")
        if status in ("cancelled", "abandoned", "failed"):
            return status, False             # a terminal decision won
        if status == "done":
            # idempotent 'done': won iff the pointer is OURS (an ambiguous
            # CAS that actually landed) — otherwise ANOTHER attempt published
            if _winner_prefix_from_run(run, sheet_id, generation) is None:
                raise RuntimeError(
                    "publish refused: done run has no valid published pointer")
            return "done", _pointer_is_ours(run)
        if status != "running":
            raise RuntimeError(
                f"publish refused: unknown run status {status!r}")
        run["status"] = "done"
        run["finished_at_s"] = time.time()
        run["published_generation"] = generation
        run["published_png_key"] = gen_prefix + "sheet.png"
        run["published_manifest_key"] = gen_prefix + "sheet.json"
        try:
            _cas_put_run(sheet_id, etag, run)
            return "done", True              # WE published — the winner
        except _CASConflict:
            continue                         # conflict: re-read + re-evaluate
        # a NON-conflict RuntimeError (no conditional writes) propagates —
        # fail closed, never masked as a retryable conflict
    raise RuntimeError("publish refused: run.json contended — retry")


SUPERSEDED = "superseded"


def _mark_run_terminal(sheet_id, generation, status):
    """CAS run.json to a terminal state (round-3 finding 5). run.json is
    the SINGLE serialization point for the run's terminal decision — cancel,
    abandon, worker-failure and publish all transition it through this CAS,
    so exactly one terminal outcome wins and the others observe it.

    Returns (resolved_status, run_record):
      - resolved_status:
          `status` if we won the CAS (we set it), OR the EXISTING terminal
          status if a concurrent writer reached a different terminal first
          (round-9 finding 1: a publish that beat a cancel leaves 'done'),
          OR SUPERSEDED if a newer begin owns the record, OR None if it
          could NOT be confirmed after retries (round-9 finding 2).
      - run_record: the run.json dict at the RESOLVING read (carries the
          published_* pointer when a publish won), or None for
          SUPERSEDED/None. Round-14 finding 1: cleanup reconciliation MUST
          key the winner off THIS record — a second read can catch a
          concurrent new generation and mis-identify the winner, deleting
          the real published artifacts."""
    for _ in range(5):
        try:
            resp = s3.get_object(Bucket=BUCKET, Key=_run_json_key(sheet_id))
            etag = resp.get("ETag")
            run = json.loads(resp["Body"].read())
            if not isinstance(run, dict):
                return None, None
            if run.get("generation") != generation:
                return SUPERSEDED, None    # a newer begin owns the record
            cur = run.get("status")
            if cur != "running":
                if cur in RUN_TERMINAL_STATUSES:
                    if (cur == "done"
                            and _winner_prefix_from_run(
                                run, sheet_id, generation) is None):
                        return None, None      # done without a winner is corrupt
                    return cur, run        # already terminal — record carries winner
                return None, None          # malformed/unknown is not a decision
            run["status"] = status
            run["finished_at_s"] = time.time()
            _cas_put_run(sheet_id, etag, run)
            return status, run
        except _CASConflict:
            continue                 # concurrent transition — re-read + re-evaluate
        except ClientError:
            continue                 # transient S3 error — retry
    return None, None                # UNCONFIRMED — caller must not claim success


def _owns_for_write(job_id, task_id, owner, *, result_data=None):
    """Fail-closed ownership proof for the instant before a shared S3 write
    (round-9 finding 1). A write fence must be a synchronous check-then-
    write: an async heartbeat flag can lag reality (a persistent DDB outage
    never latches `lost` yet the real lease still expires and a successor
    reclaims). This does an owner-conditional renew RIGHT HERE and returns
    True ONLY when ownership is freshly CONFIRMED. A lost lease (renew
    False) OR any DDB uncertainty (renew raises) returns False, so the
    caller SKIPS the shared write — the only safe action when we cannot
    prove we still hold the lease."""
    try:
        return bool(renew_claim(job_id, task_id, owner=owner,
                                result_data=result_data))
    except Exception:
        return False


def _task_status_for(resolved):
    """Round-12 finding 4: map the AUTHORITATIVE run status to the DDB task
    status. A run that resolved to 'done' (a concurrent publication won) is
    a task SUCCESS, never an error — only failed/cancelled/abandoned (and
    superseded) are task errors."""
    return "done" if resolved == "done" else "error"


def _finalize_failure_or_exit(job_id, task_id, owner, sheet_id, generation,
                              exc, *, phase_label, own_attempt_prefix=None,
                              own_written_keys=None):
    """Shared error path for the worker and the stitch. Ordered so no shared
    state is mutated on unconfirmed ownership, and the DDB task is finalized
    only AFTER the authoritative run.json transition is confirmed (round-8
    finding 3 + round-10 finding 2):

      1. Confirm ownership via a renew (does NOT transition the task
         terminal). renew False -> a successor owns it: exit benignly. renew
         RAISES (DDB uncertain) -> touch NOTHING, re-raise the original error.
      2. We own it: CAS run.json -> failed. If that returns None (could not
         be confirmed after retries) leave BOTH the DDB task and run.json as
         they are and re-raise loudly — do not finalize the task against an
         unconfirmed run (that manufactured ghost 'running' runs).
      3. run.json confirmed terminal -> finalize the DDB task, then re-raise."""
    try:
        owns = renew_claim(job_id, task_id, owner=owner)
    except Exception:
        raise exc                      # ownership UNKNOWN -> mutate nothing
    if not owns:
        return ok_response({"sheet_id": sheet_id, "lost_lease": True})
    final, run = _mark_run_terminal(sheet_id, generation, "failed")
    if final is None:
        raise exc                      # unconfirmed run -> don't finalize; raise
    # Never run a family-wide sweep here: other invocations can remain alive
    # for a full Lambda lifetime after the terminal CAS. This invocation may
    # clean only objects it knows it created; durable delayed GC owns the
    # generation-wide sweep after quiescence.
    _schedule_terminal_gc_from_run(sheet_id, generation, run)
    _delete_keys_best_effort(own_written_keys)
    if own_attempt_prefix is not None:
        _prune_own_attempt_if_not_published(sheet_id, generation, own_attempt_prefix)
    # round-12 finding 4: record the DDB task at the ACTUAL resolved status.
    # If a concurrent attempt published (final == 'done') the run SUCCEEDED,
    # so the task is 'done' — not a false 'error'.
    task_status = _task_status_for(final)
    try:
        finalize_task(
            job_id, task_id, owner=owner, status=task_status,
            error_msg=(None if task_status == "done" else str(exc)),
            result_data={"phase": task_status,
                         "phase_label": ("Published" if final == "done" else phase_label),
                         "task_id": task_id})
    except Exception:
        pass
    if final == "done":
        # the run is published (by a concurrent attempt) — our exception is
        # moot; report the success instead of failing loudly
        return ok_response({"sheet_id": sheet_id, "generation": generation,
                            "status": "done", "published_by_this_attempt": False})
    raise exc


def _published_winner_prefix(run, sheet_id, generation):
    """Validate and return a published attempt prefix for ``generation``.

    The two pointer keys must agree and must resolve to exactly one attempt
    segment below this sheet/generation. This validator is also used when a
    newer run carries an older generation's winner forward.
    """
    if not isinstance(run, dict) or run.get("published_generation") != generation:
        return None
    png = run.get("published_png_key")
    man = run.get("published_manifest_key")
    if not (isinstance(png, str) and isinstance(man, str)):
        return None
    if not (png.endswith("/sheet.png") and man.endswith("/sheet.json")):
        return None
    p_prefix = png[:-len("sheet.png")]
    m_prefix = man[:-len("sheet.json")]
    if p_prefix != m_prefix:
        return None
    root = _gen_prefix(sheet_id, generation)
    if p_prefix == root:
        return root                 # legacy generation-scoped publication
    if not p_prefix.startswith(root):
        return None
    attempt = p_prefix[len(root):].rstrip("/")
    if not ATTEMPT_RE.fullmatch(attempt) or p_prefix != root + attempt + "/":
        return None
    return p_prefix


def _winner_prefix_from_run(run, sheet_id, generation):
    """The WINNING attempt prefix (sheets/{id}/{gen}/{token}/) derived from a
    run RECORD — NOT a fresh read (round-14 finding 1: a second read can
    catch a concurrent new generation and mis-identify the winner). Requires:
    the record is for THIS generation, and BOTH published keys are present,
    correctly suffixed, and agree on the prefix. Returns None when the winner
    is not DEFINITIVELY known — the caller must then SKIP the family sweep
    rather than guess (never substitute the caller's own prefix)."""
    if not isinstance(run, dict) or run.get("generation") != generation:
        return None
    return _published_winner_prefix(run, sheet_id, generation)


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


def _safe_status(job_id, task_id, status, error_msg=None, result_data=None):
    """report_status that never raises (round-6 finding 3): a transient
    DDB failure on an error/done write must not prevent the terminal
    run-state mark that precedes it, nor mask the real exception."""
    try:
        report_status(job_id, task_id, status, error_msg, result_data)
    except Exception:
        pass


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
    # round-5 finding 1: a terminal run is CLOSED. A late worker or stitch
    # for a cancelled/abandoned/failed/done generation must be refused —
    # otherwise a straggler stitch could undo a cancellation by driving
    # the run back to 'done'.
    if run.get("status") in ("cancelled", "abandoned", "failed", "done"):
        raise RuntimeError(
            f"run {sheet_id}/{generation} is terminal ({run.get('status')}); "
            "no further work is accepted")
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
    # Schedule before publication of run.json. If admission later loses every
    # CAS, this message finds no matching generation and reaps nothing; if the
    # client or Lambda dies immediately after admission, crash cleanup is
    # already durable and does not depend on another request reaching us.
    _schedule_generation_gc(sheet_id, generation, steps)
    # round-5 finding 7 + round-6 finding 5: carry forward the PREVIOUS
    # publication pointer so the last-good sheet stays reachable until
    # this new generation commits, and do the read+write as a CAS loop —
    # an unconditional put could erase a pointer another generation
    # committed between the read and the write (lost update).
    for _ in range(6):
        resp = None
        try:
            resp = s3.get_object(Bucket=BUCKET, Key=_run_json_key(sheet_id))
        except ClientError as exc:
            if not is_missing_s3_error(exc):
                raise
        etag = resp.get("ETag") if resp else None
        prior = None
        if resp:
            try:
                prior = json.loads(resp["Body"].read())
            except (ValueError, TypeError):
                prior = None
        if isinstance(prior, dict) and prior.get("published_png_key"):
            run["published_generation"] = prior.get("published_generation")
            run["published_png_key"] = prior.get("published_png_key")
            run["published_manifest_key"] = prior.get("published_manifest_key")
        else:
            run.pop("published_generation", None)
            run.pop("published_png_key", None)
            run.pop("published_manifest_key", None)
        try:
            # This timestamp is the supersession fence used by delayed GC;
            # stamp it immediately before the CAS, not before potentially
            # slow SQS/status work.
            run["created_at_s"] = time.time()
            _cas_put_run(sheet_id, etag, run)
            # A pre-feature prior generation may not have its own queued GC.
            # Enqueue one now, but never reap inline: an old worker can remain
            # alive for a full Lambda lifetime after this superseding CAS.
            prior_gen = prior.get("generation") if isinstance(prior, dict) else None
            if (isinstance(prior_gen, str)
                    and prior_gen != generation
                    and GENERATION_RE.fullmatch(prior_gen)):
                try:
                    prior_steps = int(prior.get("steps"))
                except (TypeError, ValueError):
                    prior_steps = 0
                if 1 <= prior_steps <= MAX_STEPS:
                    _schedule_generation_gc(sheet_id, prior_gen, prior_steps)
            return ok_response(run)
        except _CASConflict:
            continue   # a concurrent commit changed run.json; re-read
    raise RuntimeError("begin could not admit the run (persistent write contention)")


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
    # round-6 finding 1: claim a LEASE, not a permanent flag. Only one
    # invocation holds a live lease; a duplicate/redispatch of a live
    # worker loses the claim and no-ops, but a CRASHED worker's lease
    # expires so a later redispatch can reclaim (crash recovery).
    owner = task_id + ":" + os.urandom(6).hex()
    if not claim_task(job_id, task_id, owner=owner):
        return ok_response({"sheet_id": sheet_id, "frames_done": len(indices),
                            "not_claimed": True})
    # round-8 finding 1: a single frame chains coeffgen + solve (+ a frozen
    # solve), up to ~900s of native ops between the inline per-tile renews —
    # longer than the 420s lease. The heartbeat renews in the background so
    # a HEALTHY worker keeps its lease across those ops; if a renewal shows
    # the lease was taken over, hb.lost latches and we exit before writing
    # any tile a successor may also be writing.
    hb = LeaseHeartbeat(job_id, task_id, owner=owner).start()
    written = []   # keys this invocation definitively created
    try:
        _budget_check(cfg, len(indices) + (1 if cfg["vp_mode"] == "frozen" else 0),
                      "worker share",
                      degree=max(2, int(run.get("degree_probe") or 40)))
        _substitute_frame(params, cfg, indices[0])
        worker_deadline_s = time.time() + BUDGET_US / 1e6

        frozen_cache = {}
        done = 0
        for k in indices:
            if _cancel_requested(sheet_id, generation):
                # round-10 finding 2: do NOT finalize the DDB task or report
                # terminal success until the run.json CAS is CONFIRMED — an
                # unconfirmable transition (None) must raise, not 200 with a
                # null status while run.json stays 'running'.
                final, terminal_run = _mark_run_terminal(
                    sheet_id, generation, "cancelled")
                if final is None:
                    raise RuntimeError(
                        "cancel could not be confirmed: run.json unreachable")
                _schedule_terminal_gc_from_run(sheet_id, generation, terminal_run)
                _delete_keys_best_effort(written)
                # round-12 finding 4: record the DDB task at the ACTUAL
                # resolved status — if a publish beat the cancel (final ==
                # 'done') the task succeeded, not errored.
                task_status = _task_status_for(final)
                try:
                    finalize_task(
                        job_id, task_id, owner=owner, status=task_status,
                        error_msg=(None if task_status == "done" else "Cancelled by user"),
                        result_data={"phase": task_status,
                                     "phase_label": ("Published" if final == "done" else "Cancelled"),
                                     "task_id": task_id,
                                     "sheet_id": sheet_id, "frame": done})
                except Exception:
                    pass
                return ok_response({"sheet_id": sheet_id, "generation": generation,
                                    "status": final, "frames_done": done,
                                    "cancelled": final == "cancelled"})
            if time.time() > worker_deadline_s:
                raise RuntimeError(
                    f"worker exceeded its {BUDGET_US / 1e6:.0f}s budget at "
                    f"frame {k} ({done}/{len(indices)} done) — per-frame cost "
                    "grew beyond the admission estimate (scan tokens can "
                    "raise the degree); reduce frames or N")
            tile, record = _render_frame_tile(cfg, params, k, frozen_cache,
                                              deadline_s=worker_deadline_s)
            done_after = done + 1
            # round-9 finding 1: the FENCE is a synchronous owner-conditional
            # renew IMMEDIATELY before the shared write, not the async
            # heartbeat's `lost` flag (which a persistent DDB outage never
            # latches even after the lease has really expired). This proves
            # ownership NOW and fails CLOSED on any uncertainty — a lost
            # lease OR a DDB error both skip the write, so a reclaimed worker
            # can never overwrite a successor's tile keys. The heartbeat is
            # only a keep-alive so a healthy long frame's fence still passes.
            if not _owns_for_write(job_id, task_id, owner, result_data={
                    "phase": "sheet", "phase_label": f"{done_after}/{len(indices)} frames",
                    "task_id": task_id, "sheet_id": sheet_id, "generation": generation,
                    "frames": len(indices), "frame": done_after}):
                return ok_response({"sheet_id": sheet_id, "frames_done": done,
                                    "lost_lease": True})
            # round-16 finding 1: the lease is ORTHOGONAL to cancellation —
            # a cancelled/superseded run's worker still holds a valid lease,
            # so the write must ALSO be fenced against the run record. A
            # worker that missed the marker mid-render stops HERE: tiles can
            # never be recreated after terminal cleanup. Self-clean what we
            # wrote (same-gen terminal is re-reaped by the owner; a
            # superseded gen's reap fails closed, so own-key deletion is the
            # only cleaner for that case). NOT on lost-lease exits.
            run_state, fenced_run = _run_write_state(sheet_id, generation)
            if run_state == RUN_UNKNOWN:
                raise RuntimeError(
                    "run.json state could not be confirmed before tile write")
            if run_state == RUN_INACTIVE:
                _delete_keys_best_effort(written)
                return _confirmed_run_inactive_response(
                    sheet_id, generation, fenced_run, frames_done=done)
            # round-10 finding 4: WRITE-ONCE tiles (create-only). Even though
            # the fence proved ownership an instant ago, a sufficiently
            # delayed stale worker could still reach here after losing the
            # lease; create-only makes the tile object immutable once
            # written, so a stale write is REJECTED (not an overwrite). The
            # render is deterministic, so an existing tile is byte-equivalent.
            key = _tile_key(sheet_id, generation, k)
            record_key = key.replace(".bin", ".json")
            if _put_object_once(key, bytes(tile), "application/octet-stream"):
                written.append(key)
            if _put_object_once(record_key, json.dumps(record).encode("utf-8"),
                                "application/json"):
                written.append(record_key)
            done = done_after
        # round-16 finding 1: final quiescence check — a cancel that lands
        # during the LAST frame's write window has no later loop iteration
        # to catch it; without this, that straggler tile would outlive the
        # terminal reap with the client no longer watching.
        run_state, fenced_run = _run_write_state(sheet_id, generation)
        if run_state == RUN_UNKNOWN:
            raise RuntimeError(
                "run.json state could not be confirmed after tile writes")
        if run_state == RUN_INACTIVE:
            _delete_keys_best_effort(written)
            return _confirmed_run_inactive_response(
                sheet_id, generation, fenced_run, frames_done=done)
        # round-7 finding 2: the terminal 'done' is OWNER-CONDITIONAL and
        # best-effort — a lost lease means a successor owns it (don't
        # overwrite), and a transient DDB throttle on the done write must
        # not fail a run whose tiles are already durably written.
        try:
            finalize_task(job_id, task_id, owner=owner, status="done", result_data={
                "phase": "done", "phase_label": "Worker done", "task_id": task_id,
                "sheet_id": sheet_id, "generation": generation,
                "frames": len(indices), "frame": done})
        except Exception:
            pass
        return ok_response({"sheet_id": sheet_id, "frames_done": done})
    except Exception as e:
        return _finalize_failure_or_exit(
            job_id, task_id, owner, sheet_id, generation, e,
            phase_label="Sheet worker failed", own_written_keys=written)
    finally:
        hb.stop()
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
    owner = task_id + ":" + os.urandom(6).hex()
    if not claim_task(job_id, task_id, owner=owner):
        return ok_response({"sheet_id": sheet_id, "not_claimed": True})
    # round-8 finding 1/2: the final section (PNG encode + uploads) can run
    # long; the heartbeat keeps a live stitch's lease alive, and hb.lost
    # gates the artifact write so a reclaimed stitch never publishes.
    hb = LeaseHeartbeat(job_id, task_id, owner=owner).start()
    # Establish identity before the first PUT so every exception path knows
    # which attempt-private prefix may contain a partial upload.
    gen_prefix = _attempt_prefix(sheet_id, generation, owner)
    try:
        t0 = float(params.get("started_at_s") or time.time())

        if _cancel_requested(sheet_id, generation):
            # honor the ACTUAL resolved status (round-9 finding 1): if a
            # successor already published, run.json is 'done' and we must
            # NOT report a false cancellation. round-10 finding 2: an
            # unconfirmable transition (None) RAISES — never a 200 with a
            # null status while run.json stays 'running'.
            final, terminal_run = _mark_run_terminal(
                sheet_id, generation, "cancelled")
            if final is None:
                raise RuntimeError(
                    "cancel could not be confirmed: run.json unreachable")
            _schedule_terminal_gc_from_run(sheet_id, generation, terminal_run)
            # round-12 finding 4: DDB task at the ACTUAL resolved status
            task_status = _task_status_for(final)
            try:
                finalize_task(
                    job_id, task_id, owner=owner, status=task_status,
                    error_msg=(None if task_status == "done" else "Cancelled by user"),
                    result_data={"phase": task_status,
                                 "phase_label": ("Published" if final == "done" else "Cancelled"),
                                 "sheet_id": sheet_id})
            except Exception:
                pass
            return ok_response({"sheet_id": sheet_id, "generation": generation,
                                "status": final, "frames_done": 0,
                                "cancelled": final == "cancelled"})

        tile_bytes = cfg["tile_px"] * cfg["tile_px"]
        canvas = bytearray(bytes([cfg["bg"]]) * (cfg["canvas_w"] * cfg["canvas_h"]))
        frame_records = []
        missing = []
        for k in range(cfg["steps"]):
            # renew the stitch lease every 32 tiles so a large sheet's read
            # loop cannot let the lease lapse under a live stitch
            if k and k % 32 == 0:
                if not renew_claim(job_id, task_id, owner=owner, result_data={
                        "phase": "stitch", "phase_label": f"Stitching {k}/{cfg['steps']}",
                        "sheet_id": sheet_id, "generation": generation,
                        "frames": cfg["steps"]}):
                    # round-7 finding 1: lost lease -> a successor stitch
                    # owns it; exit benignly rather than failing the run
                    return ok_response({"sheet_id": sheet_id, "lost_lease": True})
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
        # round-9 finding 1/3: FENCE (synchronous fail-closed renew) before
        # the expensive encode+upload — a reclaimed stitch produces no
        # artifacts. The heartbeat above was only a keep-alive.
        if not _owns_for_write(job_id, task_id, owner, result_data={
                "phase": "stitch", "phase_label": "Publishing",
                "sheet_id": sheet_id, "generation": generation,
                "frames": cfg["steps"]}):
            return ok_response({"sheet_id": sheet_id, "lost_lease": True})
        # round-16 finding 3: fence the artifact write against the RUN RECORD
        # too — a superseded/terminal run's stitch must not create attempt
        # artifacts that the begin-GC (which already ran) can never revisit.
        run_state, fenced_run = _run_write_state(sheet_id, generation)
        if run_state == RUN_UNKNOWN:
            raise RuntimeError(
                "run.json state could not be confirmed before publication")
        if run_state == RUN_INACTIVE:
            return _confirmed_run_inactive_response(
                sheet_id, generation, fenced_run)
        manifest["png_key"] = gen_prefix + "sheet.png"
        manifest["manifest_key"] = gen_prefix + "sheet.json"
        _write_generation_artifacts(cfg, generation, manifest, canvas, owner,
                                    prefix=gen_prefix)
        # round-9 finding 3: FENCE AGAIN immediately before the commit — the
        # encode + three writes above are an unbounded section during which
        # the lease could have expired, so re-prove ownership so the only
        # remaining unfenced step is the CAS itself.
        if not _owns_for_write(job_id, task_id, owner):
            _prune_own_attempt(sheet_id, gen_prefix)
            return ok_response({"sheet_id": sheet_id, "lost_lease": True})
        # THE COMMIT: publication competes with cancel/abandon through the
        # SAME run.json CAS (round-9 finding 1). Returns (status, won): won
        # is True ONLY when THIS attempt set 'done'.
        outcome, won = _commit_run_publication(sheet_id, generation, gen_prefix)
        if outcome != "done":
            # a cancel/abandon/failure won the CAS: clean our OWN orphaned
            # attempt and report the real outcome — never a false 'done'.
            _prune_own_attempt(sheet_id, gen_prefix)
            try:
                finalize_task(job_id, task_id, owner=owner, status="error",
                              error_msg=f"run was {outcome} before publish",
                              result_data={"phase": "error",
                                           "phase_label": outcome.title(),
                                           "sheet_id": sheet_id})
            except Exception:
                pass
            return ok_response({"sheet_id": sheet_id, "generation": generation,
                                "status": outcome,
                                "cancelled": outcome == "cancelled"})
        if not won:
            # round-10 finding 1: ANOTHER attempt already published — WE are
            # the orphan. Prune ONLY our own prefix; NEVER run the family-wide
            # sweep below (it would delete the actual winner's artifacts).
            _prune_own_attempt(sheet_id, gen_prefix)
            try:
                finalize_task(job_id, task_id, owner=owner, status="done",
                              result_data={"phase": "done",
                                           "phase_label": "Published by another attempt",
                                           "sheet_id": sheet_id,
                                           "generation": generation})
            except Exception:
                pass
            return ok_response({"sheet_id": sheet_id, "generation": generation,
                                "status": "done",
                                "published_by_this_attempt": False})
    except Exception as e:
        # PRE-commit failures mark the run failed — but only when ownership
        # is CONFIRMED (round-8 finding 3). A stale stitch that lost its
        # lease exits benignly; DDB-uncertain ownership never authorizes
        # failing the successor's run. Cleanup is reconciled inside
        # _finalize_failure_or_exit through the single _reap_terminal_
        # generation owner (round-15); own_attempt_prefix lets the SUPERSEDED
        # case self-clean the attempt the fail-closed owner cannot touch
        # (round-16 finding 3).
        return _finalize_failure_or_exit(
            job_id, task_id, owner, sheet_id, generation, e,
            phase_label="Stitch failed", own_attempt_prefix=gen_prefix)
    finally:
        hb.stop()

    # round-5 finding 2: past the commit the sheet IS published. Cleanup
    # and the done-report are best-effort — a failed DDB status write (or
    # a delete error) must NEVER mark a published sheet as failed. The
    # authoritative 'done' already lives in run.json.
    cleanup_ok = _reap_sheet_scaffolding(sheet_id, generation, cfg["steps"], gen_prefix)
    try:
        finalize_task(job_id, task_id, owner=owner, status="done", result_data={
            "phase": "done", "phase_label": "Done",
            "sheet_id": sheet_id, "generation": generation,
            "png_key": manifest["png_key"],
            "frames": cfg["steps"], "elapsed_ms": manifest["elapsed_ms"],
            "cleanup_ok": cleanup_ok,
        })
    except Exception:
        pass   # published; the status row is a convenience, not the truth
    return ok_response(manifest)


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
