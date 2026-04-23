import json
import math
import os


MB = 1024 * 1024
VALID_SECTION_MODES = {"physical_chunks", "logical_sections", "logical_sections_auto"}
MAX_LOGICAL_SECTIONS = 4096

DEFAULT_SOLVE_SCORE_MEMORY_MB = int(os.environ.get("SOLVE_PROXIMITY_MEMORY_MB", "4096") or 4096)
DEFAULT_PALETTE_CHUNK_MEMORY_MB = int(os.environ.get("PALETTE_CHUNK_MEMORY_MB", "1769") or 1769)
DEFAULT_RASTER_MEMORY_MB = int(os.environ.get("RASTER_MT_MEMORY_MB", "4096") or 4096)
AUTO_USABLE_FRACTION = float(os.environ.get("RENDER_SECTION_AUTO_USABLE_FRACTION", "0.40") or 0.40)
AUTO_FIXED_OVERHEAD_MB = int(os.environ.get("RENDER_SECTION_AUTO_FIXED_OVERHEAD_MB", "96") or 96)
AUTO_PER_THREAD_OVERHEAD_MB = int(os.environ.get("RENDER_SECTION_AUTO_PER_THREAD_MB", "8") or 8)


def normalize_section_mode(value, default="physical_chunks"):
    mode = str(value or default).strip().lower()
    if mode not in VALID_SECTION_MODES:
        raise RuntimeError(
            f"section_mode must be one of {', '.join(sorted(VALID_SECTION_MODES))}, got {value!r}"
        )
    return mode


def validate_section_count(value, field_name, default=""):
    if value in (None, ""):
        return default
    try:
        count = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{field_name} must be an integer, got {value!r}")
    if not (1 <= count <= MAX_LOGICAL_SECTIONS):
        raise RuntimeError(f"{field_name} must be in [1, {MAX_LOGICAL_SECTIONS}], got {count}")
    return count


def root_row_bytes(degree):
    return int(degree) * 2 * 4


def coeff_row_bytes(n_coeffs):
    return int(n_coeffs) * 2 * 4


def param_row_bytes():
    return 16


def _normalize_int(value, default=0):
    try:
        return int(value)
    except (TypeError, ValueError):
        return int(default)


def choose_representative_chunk(items, size_key="bin_size"):
    rows = [dict(item) for item in (items or [])]
    if not rows:
        return {"size_bytes": 0, "step_count": 0, "kind": "missing"}
    max_steps = max(int(row.get("step_count") or 0) for row in rows)
    if max_steps > 0:
        for row in rows:
            if int(row.get("step_count") or 0) == max_steps and int(row.get(size_key) or 0) > 0:
                return {
                    "size_bytes": int(row.get(size_key) or 0),
                    "step_count": int(row.get("step_count") or 0),
                    "chunk_idx": int(row.get("chunk_idx") or 0),
                    "kind": "first_full",
                }
    row = max(rows, key=lambda r: int(r.get(size_key) or 0))
    return {
        "size_bytes": int(row.get(size_key) or 0),
        "step_count": int(row.get("step_count") or 0),
        "chunk_idx": int(row.get("chunk_idx") or 0),
        "kind": "largest",
    }


def summarize_chunk_items(chunk_items, degree, n_coeffs):
    items = [dict(item) for item in (chunk_items or [])]
    total_solves = sum(int(item.get("step_count") or 0) for item in items)
    total_root_bytes = sum(int(item.get("bin_size") or 0) for item in items)
    total_coeff_bytes = sum(int(item.get("coeffs_bin_size") or 0) for item in items)
    total_param_bytes = sum(int(item.get("params_bin_size") or 0) for item in items)
    root_rep = choose_representative_chunk(items, "bin_size")
    coeff_rep = choose_representative_chunk(items, "coeffs_bin_size")
    param_rep = choose_representative_chunk(items, "params_bin_size")
    return {
        "chunk_count": len(items),
        "total_solves": total_solves,
        "chunk_step_metadata_complete": bool(items) and all(int(item.get("step_count") or 0) > 0 for item in items),
        "root_row_bytes": root_row_bytes(degree) if degree else 0,
        "coeff_row_bytes": coeff_row_bytes(n_coeffs) if n_coeffs else 0,
        "param_row_bytes": param_row_bytes(),
        "representative_root_chunk_size": root_rep["size_bytes"],
        "representative_root_chunk_kind": root_rep["kind"],
        "representative_coeff_chunk_size": coeff_rep["size_bytes"],
        "representative_coeff_chunk_kind": coeff_rep["kind"],
        "representative_param_chunk_size": param_rep["size_bytes"],
        "representative_param_chunk_kind": param_rep["kind"],
        "total_root_bytes": total_root_bytes,
        "total_coeff_bytes": total_coeff_bytes,
        "total_param_bytes": total_param_bytes,
    }


def _phase_memory_mb(phase):
    phase_key = str(phase or "").strip().lower()
    if phase_key in ("hist", "solve_score", "solve_score_hist"):
        return DEFAULT_SOLVE_SCORE_MEMORY_MB
    if phase_key in ("palette", "associated_palette", "palette_chunk"):
        return DEFAULT_PALETTE_CHUNK_MEMORY_MB
    if phase_key in ("raster", "color_raster", "raster_mt"):
        return DEFAULT_RASTER_MEMORY_MB
    return DEFAULT_SOLVE_SCORE_MEMORY_MB


def section_budget_bytes(memory_mb, threads):
    mem_bytes = max(0, int(memory_mb) * MB)
    usable_bytes = int(mem_bytes * AUTO_USABLE_FRACTION)
    fixed_bytes = AUTO_FIXED_OVERHEAD_MB * MB
    thread_bytes = max(1, int(threads)) * AUTO_PER_THREAD_OVERHEAD_MB * MB
    return max(0, usable_bytes - fixed_bytes - thread_bytes)


def section_source_row_bytes(degree, n_coeffs, include_coeff=False, include_param=False):
    total = root_row_bytes(degree)
    if include_coeff:
        total += coeff_row_bytes(n_coeffs)
    if include_param:
        total += param_row_bytes()
    return total


def compute_safe_sectioning(total_solves, degree, n_coeffs, threads, phase, include_coeff=False, include_param=False):
    total_solves = int(total_solves or 0)
    if total_solves < 1:
        return {
            "memory_mb": _phase_memory_mb(phase),
            "budget_bytes": 0,
            "row_bytes": 0,
            "max_solves_per_section": 0,
            "min_safe_sections": 1,
            "computed_section_count": 1,
            "auto_available": False,
            "reason": "no_solves",
        }
    memory_mb = _phase_memory_mb(phase)
    budget = section_budget_bytes(memory_mb, threads)
    row_bytes = section_source_row_bytes(degree, n_coeffs, include_coeff, include_param)
    if row_bytes <= 0 or budget <= 0:
        return {
            "memory_mb": memory_mb,
            "budget_bytes": budget,
            "row_bytes": row_bytes,
            "max_solves_per_section": 0,
            "min_safe_sections": MAX_LOGICAL_SECTIONS,
            "computed_section_count": MAX_LOGICAL_SECTIONS,
            "auto_available": False,
            "reason": "invalid_budget",
        }
    max_solves = max(1, budget // row_bytes)
    min_sections = max(1, int(math.ceil(float(total_solves) / float(max_solves))))
    min_sections = min(MAX_LOGICAL_SECTIONS, min_sections)
    return {
        "memory_mb": memory_mb,
        "budget_bytes": budget,
        "row_bytes": row_bytes,
        "max_solves_per_section": int(max_solves),
        "min_safe_sections": int(min_sections),
        "computed_section_count": int(min_sections),
        "auto_available": True,
        "reason": "",
    }


def _partition_sizes(total_solves, section_count):
    base = total_solves // section_count
    rem = total_solves % section_count
    return [base + (1 if idx < rem else 0) for idx in range(section_count)]


def _sorted_chunk_items(chunk_items):
    items = [dict(item) for item in (chunk_items or [])]
    items.sort(key=lambda item: (int(item.get("step_start") or 0), int(item.get("chunk_idx") or 0)))
    return items


def build_physical_section_items(chunk_items):
    items = _sorted_chunk_items(chunk_items)
    section_count = len(items)
    out = []
    for section_idx, item in enumerate(items):
        out.append({
            "section_idx": section_idx,
            "section_count": section_count,
            "step_start": _normalize_int(item.get("step_start")),
            "step_count": _normalize_int(item.get("step_count")),
            "bin_key": str(item.get("bin_key") or ""),
            "coeffs_key": str(item.get("coeffs_key") or ""),
            "params_key": str(item.get("params_key") or ""),
            "bin_size": _normalize_int(item.get("bin_size")),
            "coeffs_bin_size": _normalize_int(item.get("coeffs_bin_size")),
            "params_bin_size": _normalize_int(item.get("params_bin_size")),
            "params_step_start": _normalize_int(item.get("params_step_start"), _normalize_int(item.get("step_start"))),
            "params_step_count": _normalize_int(item.get("params_step_count"), _normalize_int(item.get("step_count"))),
        })
    return out


def _source_segments_from_chunk_items(items, *, family, degree, n_coeffs):
    if family == "slv":
        row_bytes = root_row_bytes(degree)
        key_field = "bin_key"
        size_field = "bin_size"
        source_start_field = None
        solve_count_field = "step_count"
    elif family == "cf":
        row_bytes = coeff_row_bytes(n_coeffs)
        key_field = "coeffs_key"
        size_field = "coeffs_bin_size"
        source_start_field = None
        solve_count_field = "step_count"
    elif family == "pm":
        row_bytes = param_row_bytes()
        key_field = "params_key"
        size_field = "params_bin_size"
        source_start_field = "params_step_start"
        solve_count_field = "params_step_count"
    else:
        raise RuntimeError(f"Unknown source family: {family}")

    segments = []
    for idx, item in enumerate(items):
        key = str(item.get(key_field) or "").strip()
        if not key:
            continue
        solve_start = item.get("step_start")
        solve_count = item.get(solve_count_field)
        if solve_start in ("", None) or solve_count in ("", None):
            continue
        solve_start = int(solve_start)
        solve_count = int(solve_count)
        if solve_count <= 0:
            continue
        source_solve_start = int(item.get(source_start_field) or 0) if source_start_field else 0
        byte_size = item.get(size_field)
        if byte_size in ("", None):
            byte_size = solve_count * row_bytes
        segments.append({
            "storage_id": f"{family}_{idx:04d}",
            "key": key,
            "solve_start": solve_start,
            "solve_count": solve_count,
            "source_solve_start": source_solve_start,
            "byte_size": int(byte_size),
        })
    return {
        "row_bytes": row_bytes,
        "segments": segments,
    }


def _compact_manifest_source(items, *, family, degree, n_coeffs, include=True):
    if family == "slv":
        row_bytes = root_row_bytes(degree)
        key_field = "bin_key"
        solve_count_field = "step_count"
        source_start_field = None
    elif family == "cf":
        row_bytes = coeff_row_bytes(n_coeffs)
        key_field = "coeffs_key"
        solve_count_field = "step_count"
        source_start_field = None
    elif family == "pm":
        row_bytes = param_row_bytes()
        key_field = "params_key"
        solve_count_field = "params_step_count"
        source_start_field = "params_step_start"
    else:
        raise RuntimeError(f"Unknown source family: {family}")

    if not include:
        return {"r": row_bytes, "k": [], "g": []}

    keys = []
    key_ids = {}
    segments = []
    for item in items:
        step_start = _normalize_int(item.get("step_start"))
        step_count = _normalize_int(item.get(solve_count_field), _normalize_int(item.get("step_count")))
        if step_count <= 0:
            continue
        key = str(item.get(key_field) or "").strip()
        if not key:
            continue
        key_idx = key_ids.get(key)
        if key_idx is None:
            key_idx = len(keys)
            key_ids[key] = key_idx
            keys.append(key)
        source_start = _normalize_int(item.get(source_start_field)) if source_start_field else 0
        segments.append([key_idx, step_start, step_count, source_start])
    return {
        "r": row_bytes,
        "k": keys,
        "g": segments,
    }


def build_solve_source_manifest(chunk_items, *, job_id, degree, n_coeffs, include_solve=True, include_coeff=True, include_param=True):
    items = _sorted_chunk_items(chunk_items)
    return {
        "v": 2,
        "j": str(job_id or ""),
        "t": sum(_normalize_int(item.get("step_count")) for item in items),
        "d": _normalize_int(degree),
        "n": _normalize_int(n_coeffs),
        "s": {
            "slv": _compact_manifest_source(items, family="slv", degree=degree, n_coeffs=n_coeffs, include=include_solve),
            "cf": _compact_manifest_source(items, family="cf", degree=degree, n_coeffs=n_coeffs, include=include_coeff),
            "pm": _compact_manifest_source(items, family="pm", degree=degree, n_coeffs=n_coeffs, include=include_param),
        },
    }


def build_source_spans(solve_source_manifest, *, source_family, solve_start, solve_count):
    manifest = dict(solve_source_manifest or {})
    sources = dict(manifest.get("sources") or manifest.get("s") or {})
    source = dict(sources.get(source_family) or {})
    row_bytes_value = int(source.get("row_bytes") or source.get("r") or 0)
    if row_bytes_value <= 0:
        return []
    segments = list(source.get("segments") or source.get("g") or [])
    segment_keys = list(source.get("keys") or source.get("k") or [])
    section_end = int(solve_start) + int(solve_count)
    out = []
    if segments and isinstance(segments[0], dict):
        ordered_segments = sorted(
            segments,
            key=lambda row: (int(row.get("solve_start") or 0), str(row.get("storage_id") or row.get("key") or "")),
        )
    else:
        ordered_segments = segments
    for segment in ordered_segments:
        if isinstance(segment, dict):
            key = str(segment.get("key") or "")
            segment_start = int(segment.get("solve_start") or 0)
            segment_count = int(segment.get("solve_count") or 0)
            source_origin = int(segment.get("source_solve_start") or 0)
        else:
            try:
                key_idx, segment_start, segment_count, source_origin = segment[:4]
            except (TypeError, ValueError):
                raise RuntimeError(f"Invalid compact solve_source_manifest segment for {source_family}: {segment!r}")
            key = str(segment_keys[int(key_idx)] or "")
        segment_end = segment_start + segment_count
        overlap_start = max(int(solve_start), segment_start)
        overlap_end = min(section_end, segment_end)
        if overlap_end <= overlap_start:
            continue
        overlap_count = overlap_end - overlap_start
        source_offset_solves = source_origin + (overlap_start - segment_start)
        out.append({
            "storage_id": str(segment.get("storage_id") or "") if isinstance(segment, dict) else "",
            "key": key,
            "solve_start": overlap_start,
            "solve_count": overlap_count,
            "local_solve_start": overlap_start - int(solve_start),
            "byte_start": source_offset_solves * row_bytes_value,
            "byte_length": overlap_count * row_bytes_value,
        })
    return out


def build_native_multispan_manifest(solve_source_manifest, *, source_family, solve_start, solve_count, url_by_key):
    manifest = dict(solve_source_manifest or {})
    source = dict(((manifest.get("sources") or manifest.get("s") or {}).get(source_family) or {}))
    row_bytes_value = int(source.get("row_bytes") or source.get("r") or 0)
    if row_bytes_value <= 0:
        raise RuntimeError(f"solve_source_manifest missing row_bytes for source family {source_family}")
    spans = build_source_spans(
        solve_source_manifest,
        source_family=source_family,
        solve_start=solve_start,
        solve_count=solve_count,
    )
    if not spans:
        raise RuntimeError(
            f"solve_source_manifest produced no spans for source family {source_family} "
            f"(solve_start={solve_start}, solve_count={solve_count})"
        )
    sources = []
    source_ids = {}
    native_spans = []
    for span in spans:
        key = str(span["key"])
        if key not in url_by_key:
            raise RuntimeError(f"Missing presigned URL for source family {source_family} key {key}")
        if key not in source_ids:
            source_id = len(sources)
            source_ids[key] = source_id
            sources.append({
                "id": source_id,
                "url": str(url_by_key[key]),
                "key": key,
            })
        native_spans.append({
            "source_id": source_ids[key],
            "logical_byte_start": int(span["local_solve_start"]) * row_bytes_value,
            "byte_start": int(span["byte_start"]),
            "byte_length": int(span["byte_length"]),
        })
    return {
        "source_family": source_family,
        "logical_size": int(solve_count) * row_bytes_value,
        "row_bytes": row_bytes_value,
        "solve_start": int(solve_start),
        "solve_count": int(solve_count),
        "sources": sources,
        "spans": native_spans,
    }


def build_native_manifest_urls(s3_client, bucket, spans, *, expires_in=900):
    urls = {}
    for span in spans or []:
        key = str(span.get("key") or "").strip()
        if not key or key in urls:
            continue
        urls[key] = s3_client.generate_presigned_url(
            "get_object",
            Params={"Bucket": bucket, "Key": key},
            ExpiresIn=int(expires_in),
        )
    return urls


def write_native_multispan_manifest(path, manifest):
    with open(path, "w", encoding="utf-8") as f:
        json.dump(manifest, f, separators=(",", ":"))
    return path


def build_logical_section_items(chunk_items, *, section_count, degree, n_coeffs, include_coeff=False, include_param=False):
    items = _sorted_chunk_items(chunk_items)
    if not items:
        return []
    if not all(int(item.get("step_count") or 0) > 0 for item in items):
        raise RuntimeError("logical sectioning requires step_count on every chunk item")
    total_solves = sum(int(item.get("step_count") or 0) for item in items)
    if total_solves < 1:
        return []
    section_count = max(1, min(int(section_count), total_solves, MAX_LOGICAL_SECTIONS))
    section_sizes = _partition_sizes(total_solves, section_count)
    out = []
    cursor = 0
    root_bytes = root_row_bytes(degree)
    coeff_bytes = coeff_row_bytes(n_coeffs) if include_coeff else 0
    param_bytes = param_row_bytes() if include_param else 0
    for section_idx, solve_count in enumerate(section_sizes):
        if solve_count <= 0:
            continue
        section = {
            "section_idx": section_idx,
            "section_count": section_count,
            "step_start": cursor,
            "step_count": solve_count,
            "bin_key": "",
            "coeffs_key": "",
            "params_key": "",
            "bin_size": solve_count * root_bytes,
            "coeffs_bin_size": solve_count * coeff_bytes if include_coeff else 0,
            "params_bin_size": solve_count * param_bytes if include_param else 0,
            "params_step_start": 0,
            "params_step_count": solve_count if include_param else 0,
        }
        out.append(section)
        cursor += solve_count
    return out


def stitch_spans_to_file(s3_client, bucket, spans, path):
    total = 0
    with open(path, "wb") as out:
        for span in spans or []:
            length = int(span.get("byte_length") or 0)
            if length <= 0:
                continue
            start = int(span.get("byte_start") or 0)
            end = start + length - 1
            obj = s3_client.get_object(Bucket=bucket, Key=span["key"], Range=f"bytes={start}-{end}")
            for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                out.write(chunk)
            total += length
    return total
