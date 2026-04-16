import math
import os


MB = 1024 * 1024
VALID_SECTION_MODES = {"physical_chunks", "logical_sections", "logical_sections_auto"}
MAX_LOGICAL_SECTIONS = 4096

DEFAULT_SOLVE_SCORE_MEMORY_MB = int(os.environ.get("SOLVE_PROXIMITY_MEMORY_MB", "4096") or 4096)
DEFAULT_PALETTE_CHUNK_MEMORY_MB = int(os.environ.get("PALETTE_CHUNK_MEMORY_MB", "1769") or 1769)
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


def _chunk_overlap_spans(chunk_items, solve_start, solve_count, *, source_key_name, row_bytes_value, step_start_name="step_start", step_count_name="step_count", source_start_solve_name=None):
    section_end = int(solve_start) + int(solve_count)
    spans = []
    for item in chunk_items:
        item_step_start = int(item.get(step_start_name) or 0)
        item_step_count = int(item.get(step_count_name) or 0)
        item_end = item_step_start + item_step_count
        overlap_start = max(int(solve_start), item_step_start)
        overlap_end = min(section_end, item_end)
        if overlap_end <= overlap_start:
            continue
        overlap_count = overlap_end - overlap_start
        key = str(item.get(source_key_name) or "").strip()
        if not key:
            raise RuntimeError(f"Missing {source_key_name} for logical sectioning")
        source_origin = int(item.get(source_start_solve_name) or 0) if source_start_solve_name else 0
        source_offset_solves = source_origin + (overlap_start - item_step_start)
        spans.append({
            "key": key,
            "solve_start": overlap_start,
            "solve_count": overlap_count,
            "local_solve_start": overlap_start - int(solve_start),
            "byte_start": source_offset_solves * int(row_bytes_value),
            "byte_length": overlap_count * int(row_bytes_value),
        })
    return spans


def build_logical_section_items(chunk_items, *, section_count, degree, n_coeffs, include_coeff=False, include_param=False):
    items = [dict(item) for item in (chunk_items or [])]
    if not items:
        return []
    if not all(int(item.get("step_count") or 0) > 0 for item in items):
        raise RuntimeError("logical sectioning requires step_count on every chunk item")
    items.sort(key=lambda item: (int(item.get("step_start") or 0), int(item.get("chunk_idx") or 0)))
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
            "chunk_idx": section_idx,
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
            "root_spans": _chunk_overlap_spans(
                items,
                cursor,
                solve_count,
                source_key_name="bin_key",
                row_bytes_value=root_bytes,
            ),
        }
        if include_coeff:
            section["coeff_spans"] = _chunk_overlap_spans(
                items,
                cursor,
                solve_count,
                source_key_name="coeffs_key",
                row_bytes_value=coeff_bytes,
            )
            if section["coeff_spans"]:
                section["coeffs_key"] = section["coeff_spans"][0]["key"]
        if include_param:
            section["param_spans"] = _chunk_overlap_spans(
                items,
                cursor,
                solve_count,
                source_key_name="params_key",
                row_bytes_value=param_bytes,
                source_start_solve_name="params_step_start",
            )
            if section["param_spans"]:
                section["params_key"] = section["param_spans"][0]["key"]
            section["params_step_start"] = 0
            section["params_step_count"] = solve_count
        if section["root_spans"]:
            section["bin_key"] = section["root_spans"][0]["key"]
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
