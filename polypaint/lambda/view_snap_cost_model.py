"""Deterministic ViewSnap geometry and resource admission.

This module intentionally contains no AWS, registry, or handler imports.  Its
inputs are normalized values and deployed limits supplied by the render plan.
Runtime timing is telemetry only; admission is based on deterministic bytes
and geometry.
"""

from __future__ import annotations

import math


MIB = 1024 * 1024
MAX_I64 = (1 << 63) - 1
CAMERA_STREAM_CARRY_BYTES = 64 * 1024
ENCODED_OUTPUT_FIXED_BYTES = 2 * MIB


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


def camera_projection_bound(camera, pix):
    """Validate the camera frustum and return the one-pixel root bound."""

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

    pixel_count = checked_mul(pix, pix, label="ViewSnap pixel count")
    return {
        "near": near,
        "far": far,
        "min_prism_depth": min_depth,
        "pricing_depth": pricing_depth,
        "pixels_per_root_upper": 1,
    }


def estimate_camera_sections(
    section_items,
    *,
    pix,
    degree,
    channels,
    times,
    source_row_bytes,
    camera,
):
    """Estimate deterministic resources for stored-score camera reprojection."""

    pix = _checked_nonnegative_int(pix, "ViewSnap pix")
    degree = _checked_nonnegative_int(degree, "ViewSnap degree")
    channels = _checked_nonnegative_int(channels, "ViewSnap channels")
    times = _checked_nonnegative_int(times, "ViewSnap times")
    source_row_bytes = _checked_nonnegative_int(
        source_row_bytes, "ViewSnap source row bytes"
    )
    if pix < 2 or degree < 1 or channels < 1 or times < 1:
        raise RuntimeError("ViewSnap pix/degree/channels/times must be positive")

    pixel_count = checked_mul(pix, pix, label="ViewSnap pixel count")
    record_size = checked_add(8, channels, label="ViewSnap fragment record size")
    projection_bound = camera_projection_bound(camera, pix)
    fixed_raster_bytes = checked_add(
        checked_mul(4, pixel_count, label="ViewSnap raster depth bytes"),
        checked_mul(channels, pixel_count, label="ViewSnap raster score bytes"),
        ((pixel_count + 63) // 64) * 8,
        label="ViewSnap raster fixed bytes",
    )

    rows = []
    total_fragment_bytes = 0
    total_candidate_roots = 0
    max_fragment_bytes = 0
    max_raster_memory_bytes = 0
    max_raster_tmp_bytes = 0
    max_candidate_roots = 0
    for item in section_items or []:
        step_start = _checked_nonnegative_int(
            item.get("step_start"), "ViewSnap section step_start"
        )
        step_count = _checked_nonnegative_int(
            item.get("step_count"), "ViewSnap section step_count"
        )
        candidate_roots = checked_mul(
            step_count, degree, label="ViewSnap candidate roots"
        )
        occupied = min(
            pixel_count,
            checked_mul(
                candidate_roots,
                projection_bound["pixels_per_root_upper"],
                label="ViewSnap occupied-pixel bound",
            ),
        )
        fragment_bytes = checked_mul(
            record_size, occupied, label="ViewSnap fragment bytes"
        )
        step_score_bytes = checked_mul(
            channels, step_count, label="ViewSnap section step-score bytes"
        )
        source_bytes = checked_mul(
            source_row_bytes,
            step_count,
            label="ViewSnap section source bytes",
        )
        raster_memory_bytes = checked_add(
            source_bytes,
            fixed_raster_bytes if step_count else 0,
            step_score_bytes,
            CAMERA_STREAM_CARRY_BYTES,
            label="ViewSnap raster peak bytes",
        )
        raster_tmp_bytes = checked_add(
            fragment_bytes,
            step_score_bytes,
            label="ViewSnap raster tmp bytes",
        )
        rows.append({
            "section_idx": int(item.get("section_idx") or 0),
            "step_start": step_start,
            "step_count": step_count,
            "candidate_roots": candidate_roots,
            "occupied_pixels_upper": occupied,
            "fragment_bytes_upper": fragment_bytes,
            "step_score_bytes": step_score_bytes,
            "source_bytes": source_bytes,
            "raster_memory_bytes": raster_memory_bytes,
            "raster_tmp_bytes": raster_tmp_bytes,
        })
        total_fragment_bytes = checked_add(
            total_fragment_bytes,
            fragment_bytes,
            label="ViewSnap total fragment bytes",
        )
        total_candidate_roots = checked_add(
            total_candidate_roots,
            candidate_roots,
            label="ViewSnap total candidate roots",
        )
        max_fragment_bytes = max(max_fragment_bytes, fragment_bytes)
        max_raster_memory_bytes = max(max_raster_memory_bytes, raster_memory_bytes)
        max_raster_tmp_bytes = max(max_raster_tmp_bytes, raster_tmp_bytes)
        max_candidate_roots = max(max_candidate_roots, candidate_roots)

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
        encoded_output_upper,
        label="ViewSnap final tmp bytes",
    )
    return {
        "pixel_count": pixel_count,
        "channels": channels,
        "record_size_bytes": record_size,
        "projection": projection_bound,
        "fixed_raster_bytes": fixed_raster_bytes,
        "sections": rows,
        "max_fragment_bytes": max_fragment_bytes,
        "total_fragment_bytes": total_fragment_bytes,
        "max_raster_memory_bytes": max_raster_memory_bytes,
        "max_raster_tmp_bytes": max_raster_tmp_bytes,
        "max_candidate_roots": max_candidate_roots,
        "total_candidate_roots": total_candidate_roots,
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
