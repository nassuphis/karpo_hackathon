"""Deterministic ViewSnap geometry and resource admission.

This module intentionally contains no AWS, registry, or handler imports.  Its
inputs are normalized values and deployed limits supplied by the render plan.
Empirical wall-time rates belong to the separately versioned calibration
contract; byte and geometry safety do not depend on those measurements.
"""

from __future__ import annotations

import math


MIB = 1024 * 1024
MAX_I64 = (1 << 63) - 1
CAMERA_STREAM_CARRY_BYTES = 64 * 1024
ENCODED_OUTPUT_FIXED_BYTES = 2 * MIB
RASTER_NATIVE_TIMEOUT_SECONDS = 600.0
RASTER_HANDLER_TIMEOUT_SECONDS = 900.0
ASSEMBLER_TIMEOUT_SECONDS = 600.0
ENCODER_TIMEOUT_SECONDS = 600.0
FINALIZE_HANDLER_TIMEOUT_SECONDS = 900.0
FRAGMENT_URL_EXPIRY_SECONDS = 900.0


def _checked_nonnegative_int(value, label):
    if isinstance(value, bool):
        raise RuntimeError(f"{label} must be a non-negative integer")
    try:
        number = int(value)
    except (TypeError, ValueError) as exc:
        raise RuntimeError(f"{label} must be a non-negative integer") from exc
    if number < 0 or number > MAX_I64:
        raise RuntimeError(f"{label} is outside the supported integer range")
    return number


def checked_add(*values, label="resource estimate"):
    total = 0
    for value in values:
        number = _checked_nonnegative_int(value, label)
        if total > MAX_I64 - number:
            raise RuntimeError(f"{label} overflow")
        total += number
    return total


def checked_mul(*values, label="resource estimate"):
    product = 1
    for value in values:
        number = _checked_nonnegative_int(value, label)
        if number and product > MAX_I64 // number:
            raise RuntimeError(f"{label} overflow")
        product *= number
    return product


def _mul4(matrix, vector):
    return [
        matrix[row] * vector[0]
        + matrix[4 + row] * vector[1]
        + matrix[8 + row] * vector[2]
        + matrix[12 + row] * vector[3]
        for row in range(4)
    ]


def _perspective_planes(projection):
    near = projection[14] / (projection[10] - 1.0)
    far = projection[14] / (projection[10] + 1.0)
    if not math.isfinite(near) or not math.isfinite(far) or not (0.0 < near < far):
        raise RuntimeError("ViewSnap projection has invalid near/far planes")
    return near, far


def camera_footprint_bound(camera, pix):
    """Return a conservative footprint bound for the camera's local prism."""

    pix = _checked_nonnegative_int(pix, "ViewSnap pix")
    if pix < 2:
        raise RuntimeError("ViewSnap pix must be at least 2")
    model_view = camera["model_view_matrix"]
    projection = camera["projection_matrix"]
    tlo = float(camera["effective_tlo"])
    thi = float(camera["effective_thi"])
    corners = []
    for x in (-0.5, 0.5):
        for y in (tlo - 0.5, thi - 0.5):
            for z in (-0.5, 0.5):
                view = _mul4(model_view, (x, y, z, 1.0))
                clip = _mul4(projection, view)
                if not all(math.isfinite(value) for value in (*view, *clip)):
                    raise RuntimeError("ViewSnap camera prism produced non-finite coordinates")
                corners.append((view, clip))

    # A convex prism is outside a homogeneous clip half-space exactly when all
    # of its corners violate that same half-space. Boundary points are visible,
    # matching the native helper's strict comparisons.
    clip_rows = [clip for _view, clip in corners]
    outside = (
        all(row[0] < -row[3] for row in clip_rows)
        or all(row[0] > row[3] for row in clip_rows)
        or all(row[1] < -row[3] for row in clip_rows)
        or all(row[1] > row[3] for row in clip_rows)
        or all(row[2] < -row[3] for row in clip_rows)
        or all(row[2] > row[3] for row in clip_rows)
    )
    if outside:
        raise RuntimeError("empty camera frustum: nothing visible from this pose")

    near, far = _perspective_planes(projection)
    depths = [-view[2] for view, _clip in corners]
    min_depth = min(depths)
    pricing_depth = min_depth if min_depth >= near else near
    if not math.isfinite(pricing_depth) or not (pricing_depth > 0.0):
        raise RuntimeError("ViewSnap camera prism has no positive pricing depth")

    frame_fraction = (
        float(camera["point_world_size"])
        * float(camera["point_scale"])
        / pricing_depth
    )
    frame_fraction = max(
        float(camera["point_min_fraction"]),
        min(float(camera["point_max_fraction"]), frame_fraction),
    )
    if not math.isfinite(frame_fraction) or not (frame_fraction > 0.0):
        raise RuntimeError("ViewSnap camera footprint is invalid")
    point_side = max(1, min(pix, int(math.ceil(pix * frame_fraction))))
    pixel_count = checked_mul(pix, pix, label="ViewSnap pixel count")
    point_area = min(
        pixel_count,
        checked_mul(point_side, point_side, label="ViewSnap point footprint"),
    )
    return {
        "near": near,
        "far": far,
        "min_prism_depth": min_depth,
        "pricing_depth": pricing_depth,
        "point_fraction": frame_fraction,
        "max_point_side": point_side,
        "max_point_area": point_area,
    }


def _section_pass0_steps(step_start, step_count, pixel_count):
    start = _checked_nonnegative_int(step_start, "ViewSnap section step_start")
    count = _checked_nonnegative_int(step_count, "ViewSnap section step_count")
    end = checked_add(start, count, label="ViewSnap section end")
    return max(0, min(end, pixel_count) - min(max(start, 0), pixel_count))


def estimate_camera_sections(
    section_items,
    *,
    pix,
    degree,
    n_coeffs,
    channels,
    times,
    source_row_bytes,
    prelude_row_bytes=0,
    camera,
):
    """Estimate every deterministic ViewSnap resource from actual sections."""

    pix = _checked_nonnegative_int(pix, "ViewSnap pix")
    degree = _checked_nonnegative_int(degree, "ViewSnap degree")
    n_coeffs = _checked_nonnegative_int(n_coeffs, "ViewSnap n_coeffs")
    channels = _checked_nonnegative_int(channels, "ViewSnap channels")
    times = _checked_nonnegative_int(times, "ViewSnap times")
    source_row_bytes = _checked_nonnegative_int(
        source_row_bytes, "ViewSnap source row bytes"
    )
    prelude_row_bytes = _checked_nonnegative_int(
        prelude_row_bytes, "ViewSnap prelude row bytes"
    )
    if pix < 2 or degree < 1 or n_coeffs < 1 or channels < 1 or times < 1:
        raise RuntimeError("ViewSnap pix/degree/channels/times must be positive")

    pixel_count = checked_mul(pix, pix, label="ViewSnap pixel count")
    record_size = checked_add(8, channels, label="ViewSnap fragment record size")
    footprint = camera_footprint_bound(camera, pix)
    fixed_raster_bytes = checked_add(
        checked_mul(4, pixel_count, label="ViewSnap raster depth bytes"),
        checked_mul(channels, pixel_count, label="ViewSnap raster score bytes"),
        ((pixel_count + 63) // 64) * 8,
        label="ViewSnap raster fixed bytes",
    )

    rows = []
    total_fragment_bytes = 0
    total_work_units = 0
    max_fragment_bytes = 0
    max_raster_memory_bytes = 0
    max_raster_tmp_bytes = 0
    max_work_units = 0
    complexity_width = max(degree, n_coeffs)
    for item in section_items or []:
        step_start = _checked_nonnegative_int(
            item.get("step_start"), "ViewSnap section step_start"
        )
        step_count = _checked_nonnegative_int(
            item.get("step_count"), "ViewSnap section step_count"
        )
        pass0_steps = _section_pass0_steps(step_start, step_count, pixel_count)
        candidate_roots = checked_mul(
            pass0_steps, degree, label="ViewSnap candidate roots"
        )
        occupied = min(
            pixel_count,
            checked_mul(
                candidate_roots,
                footprint["max_point_area"],
                label="ViewSnap occupied-pixel bound",
            ),
        )
        fragment_bytes = checked_mul(
            record_size, occupied, label="ViewSnap fragment bytes"
        )
        step_score_bytes = checked_mul(
            channels, step_count, label="ViewSnap section step-score bytes"
        )
        prelude_steps = 1 if step_start > 0 and prelude_row_bytes else 0
        source_bytes = checked_add(
            checked_mul(
                source_row_bytes,
                step_count,
                label="ViewSnap section source bytes",
            ),
            checked_mul(
                prelude_row_bytes,
                prelude_steps,
                label="ViewSnap section prelude bytes",
            ),
            label="ViewSnap section source bytes",
        )
        raster_memory_bytes = checked_add(
            source_bytes,
            fixed_raster_bytes if pass0_steps else 0,
            step_score_bytes,
            CAMERA_STREAM_CARRY_BYTES,
            label="ViewSnap raster peak bytes",
        )
        raster_tmp_bytes = checked_add(
            fragment_bytes,
            step_score_bytes,
            label="ViewSnap raster tmp bytes",
        )
        scoring_units = checked_mul(
            checked_add(
                step_count,
                prelude_steps,
                label="ViewSnap scoring steps",
            ),
            complexity_width,
            complexity_width,
            label="ViewSnap scoring work units",
        )
        footprint_units = checked_mul(
            candidate_roots,
            footprint["max_point_area"],
            label="ViewSnap footprint work units",
        )
        work_units = checked_add(
            scoring_units, footprint_units, label="ViewSnap section work units"
        )
        rows.append({
            "section_idx": int(item.get("section_idx") or 0),
            "step_start": step_start,
            "step_count": step_count,
            "pass0_steps": pass0_steps,
            "candidate_roots": candidate_roots,
            "occupied_pixels_upper": occupied,
            "fragment_bytes_upper": fragment_bytes,
            "step_score_bytes": step_score_bytes,
            "prelude_steps": prelude_steps,
            "source_bytes": source_bytes,
            "raster_memory_bytes": raster_memory_bytes,
            "raster_tmp_bytes": raster_tmp_bytes,
            "work_units": work_units,
        })
        total_fragment_bytes = checked_add(
            total_fragment_bytes,
            fragment_bytes,
            label="ViewSnap total fragment bytes",
        )
        total_work_units = checked_add(
            total_work_units, work_units, label="ViewSnap total work units"
        )
        max_fragment_bytes = max(max_fragment_bytes, fragment_bytes)
        max_raster_memory_bytes = max(max_raster_memory_bytes, raster_memory_bytes)
        max_raster_tmp_bytes = max(max_raster_tmp_bytes, raster_tmp_bytes)
        max_work_units = max(max_work_units, work_units)

    raw_bytes = checked_mul(channels, pixel_count, label="ViewSnap raw bytes")
    step_scores_bytes = checked_mul(
        raw_bytes, times, label="ViewSnap combined step-score bytes"
    )
    final_memory_bytes = checked_add(
        checked_mul(4, pixel_count, label="ViewSnap final depth bytes"),
        raw_bytes,
        CAMERA_STREAM_CARRY_BYTES,
        label="ViewSnap final peak bytes",
    )
    # score_raw_render may hold the raw, encoded output, and preview on /tmp at
    # once.  Two raw-size equivalents plus a fixed envelope is conservative
    # for both PNG and JPEG without pretending to be a time calibration.
    encoded_output_upper = checked_add(
        checked_mul(2, raw_bytes, label="ViewSnap encoded-output bound"),
        ENCODED_OUTPUT_FIXED_BYTES,
        label="ViewSnap encoded-output bound",
    )
    final_tmp_bytes = checked_add(
        raw_bytes,
        step_scores_bytes,
        encoded_output_upper,
        label="ViewSnap final tmp bytes",
    )
    return {
        "pixel_count": pixel_count,
        "channels": channels,
        "record_size_bytes": record_size,
        "footprint": footprint,
        "fixed_raster_bytes": fixed_raster_bytes,
        "sections": rows,
        "max_fragment_bytes": max_fragment_bytes,
        "total_fragment_bytes": total_fragment_bytes,
        "max_raster_memory_bytes": max_raster_memory_bytes,
        "max_raster_tmp_bytes": max_raster_tmp_bytes,
        "max_work_units": max_work_units,
        "total_work_units": total_work_units,
        "final_memory_bytes": final_memory_bytes,
        "final_tmp_bytes": final_tmp_bytes,
        "raw_bytes": raw_bytes,
        "step_scores_bytes": step_scores_bytes,
        "encoded_output_bytes_upper": encoded_output_upper,
    }


def enforce_hard_resource_limits(
    estimate,
    *,
    raster_memory_bytes,
    raster_tmp_bytes,
    finalize_memory_bytes,
    finalize_tmp_bytes,
):
    limits = {
        "raster_memory_bytes": _checked_nonnegative_int(
            raster_memory_bytes, "ViewSnap raster memory limit"
        ),
        "raster_tmp_bytes": _checked_nonnegative_int(
            raster_tmp_bytes, "ViewSnap raster tmp limit"
        ),
        "finalize_memory_bytes": _checked_nonnegative_int(
            finalize_memory_bytes, "ViewSnap finalize memory limit"
        ),
        "finalize_tmp_bytes": _checked_nonnegative_int(
            finalize_tmp_bytes, "ViewSnap finalize tmp limit"
        ),
    }
    checks = (
        ("Raster memory", estimate["max_raster_memory_bytes"], limits["raster_memory_bytes"]),
        ("Raster /tmp", estimate["max_raster_tmp_bytes"], limits["raster_tmp_bytes"]),
        ("Finalize memory", estimate["final_memory_bytes"], limits["finalize_memory_bytes"]),
        ("Finalize /tmp", estimate["final_tmp_bytes"], limits["finalize_tmp_bytes"]),
    )
    for label, required, available in checks:
        if required > available:
            raise RuntimeError(
                f"ViewSnap {label} admission failed: "
                f"required_bytes={required}, available_bytes={available}"
            )
    return limits


def _encode_rate(calibration, fmt, quality):
    fmt = str(fmt or "").strip().lower()
    if fmt == "jpg":
        fmt = "jpeg"
    if isinstance(quality, bool):
        raise RuntimeError("ViewSnap quality must be an integer")
    try:
        quality_number = int(quality)
    except (TypeError, ValueError) as exc:
        raise RuntimeError("ViewSnap quality must be an integer") from exc
    try:
        if float(quality) != quality_number:
            raise ValueError
    except (TypeError, ValueError) as exc:
        raise RuntimeError("ViewSnap quality must be an integer") from exc
    for segment in calibration["rates_and_latencies"]["encode_segments"]:
        if (
            segment["format"] == fmt
            and segment["quality_min"] <= quality_number <= segment["quality_max"]
        ):
            return segment["bytes_per_second"]
    raise RuntimeError(
        f"ViewSnap calibration has no encode rate for {fmt}:{quality_number}"
    )


def estimate_camera_wall_times(
    estimate,
    *,
    section_count,
    fmt,
    quality,
    calibration,
):
    """Estimate all five deployed ViewSnap timeout gates."""

    rates = calibration["rates_and_latencies"]
    derations = calibration["derations"]
    throughput_factor = derations["throughput"]
    latency_factor = derations["latency"]

    def rate(name):
        return rates[name] * throughput_factor

    def latency(name):
        return rates[name] * latency_factor

    section_rows = list(estimate.get("sections") or [])
    if len(section_rows) != int(section_count):
        raise RuntimeError(
            "ViewSnap section count does not match the resource estimate"
        )

    raster_sections = []
    for row in section_rows:
        native = row["work_units"] / rate("work_units_per_second")
        upload_bytes = row["fragment_bytes_upper"] + row["step_score_bytes"]
        handler = (
            latency("raster_prep_seconds")
            + native
            + 2.0 * latency("raster_upload_setup_seconds")
            + upload_bytes / rate("raster_upload_bytes_per_second")
        )
        raster_sections.append({
            "section_idx": row["section_idx"],
            "native_seconds": native,
            "handler_seconds": handler,
        })

    total_fragment_bytes = estimate["total_fragment_bytes"]
    total_fragment_records = (
        total_fragment_bytes // estimate["record_size_bytes"]
    )
    assembler = (
        int(section_count) * latency("fragment_request_seconds")
        + total_fragment_bytes / rate("fragment_download_bytes_per_second")
        + total_fragment_records / rate("merge_records_per_second")
    )
    presign = int(section_count) * latency("presign_object_seconds")
    sidecar_bytes = estimate["step_scores_bytes"]
    sidecar = (
        int(section_count) * latency("sidecar_get_seconds")
        + sidecar_bytes / rate("sidecar_download_bytes_per_second")
        + sidecar_bytes / rate("tmp_write_bytes_per_second")
        + latency("sidecar_upload_setup_seconds")
        + sidecar_bytes / rate("sidecar_upload_bytes_per_second")
    )
    encode_rate = _encode_rate(calibration, fmt, quality) * throughput_factor
    encode = estimate["raw_bytes"] / encode_rate
    publication_objects = 5
    publication_bytes = (
        estimate["raw_bytes"] + estimate["encoded_output_bytes_upper"]
    )
    publication = (
        publication_objects * latency("publication_put_seconds")
        + publication_bytes / rate("publication_upload_bytes_per_second")
    )
    total_finalize = presign + assembler + sidecar + encode + publication

    max_native = max(
        (row["native_seconds"] for row in raster_sections),
        default=0.0,
    )
    max_raster_handler = max(
        (row["handler_seconds"] for row in raster_sections),
        default=0.0,
    )
    return {
        "raster_sections": raster_sections,
        "raster_native_seconds": max_native,
        "raster_handler_seconds": max_raster_handler,
        "presign_seconds": presign,
        "assembler_seconds": assembler,
        "sidecar_seconds": sidecar,
        "encode_seconds": encode,
        "publication_seconds": publication,
        "finalize_total_seconds": total_finalize,
        "fragment_records_upper": total_fragment_records,
    }


def enforce_wall_time_limits(wall, *, calibration, stages=None):
    """Enforce the timeout structure used by the deployed handlers."""

    headrooms = calibration["headrooms"]
    checks = (
        (
            "Raster native",
            wall["raster_native_seconds"],
            RASTER_NATIVE_TIMEOUT_SECONDS - headrooms["raster_native_seconds"],
        ),
        (
            "Raster handler",
            wall["raster_handler_seconds"],
            RASTER_HANDLER_TIMEOUT_SECONDS - headrooms["raster_handler_seconds"],
        ),
        (
            "Finalize assembler",
            wall["assembler_seconds"],
            ASSEMBLER_TIMEOUT_SECONDS - headrooms["assembler_seconds"],
        ),
        (
            "Finalize encoder",
            wall["encode_seconds"],
            ENCODER_TIMEOUT_SECONDS - headrooms["encoder_seconds"],
        ),
        (
            "Finalize total",
            wall["finalize_total_seconds"],
            FINALIZE_HANDLER_TIMEOUT_SECONDS - headrooms["finalize_total_seconds"],
        ),
    )
    selected = set(stages or ())
    gates = {}
    for label, required, available in checks:
        key = label.lower().replace(" ", "_")
        if selected and key not in selected:
            continue
        gates[key] = {
            "estimated_seconds": required,
            "available_seconds": available,
        }
        if required >= available:
            raise RuntimeError(
                f"ViewSnap {label} admission failed: "
                f"estimated_seconds={required:.6g}, "
                f"available_seconds={available:.6g}"
            )
    return gates
