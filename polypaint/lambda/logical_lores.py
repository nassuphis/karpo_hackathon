"""
Ephemeral logical-lores materialization helpers.

Logical lores samples a compact low-resolution view from hires solve artifacts.
The output files are contiguous and ordered like a normal low-res solve stream:
pass-major, row-major, serpentine columns.
"""
import concurrent.futures
import os
import time

from calc_chunks import build_chunk_items
from logical_sections import (
    build_solve_source_manifest,
    build_source_spans,
    coeff_row_bytes,
    param_row_bytes,
    root_row_bytes,
)


DEFAULT_LOGICAL_LORES_N = 60


def logical_lores_default_n(calc, fallback=DEFAULT_LOGICAL_LORES_N):
    lores = (calc or {}).get("lores") or {}
    for value in (lores.get("N"), (calc or {}).get("lores_N"), fallback):
        try:
            parsed = int(value)
        except (TypeError, ValueError):
            continue
        if parsed > 0:
            return parsed
    return int(fallback)


def calc_square_grid(calc):
    calc = calc or {}
    n1 = int(calc.get("N") or calc.get("n1") or 0)
    n2 = int(calc.get("n2") or n1 or 0)
    times = int(calc.get("times") or 1)
    if n1 < 1 or n2 < 1:
        raise RuntimeError(f"calc.json missing positive N/n1/n2 for logical lores: n1={n1} n2={n2}")
    if n1 != n2:
        raise RuntimeError(f"logical lores preview requires a square source grid, got n1={n1} n2={n2}")
    if times < 1:
        raise RuntimeError(f"calc.json has invalid times={times} for logical lores")
    return n1, times


def _read_body_bytes(body):
    if hasattr(body, "read"):
        return body.read()
    if hasattr(body, "iter_chunks"):
        return b"".join(body.iter_chunks(chunk_size=1024 * 1024))
    raise RuntimeError("S3 body does not expose read() or iter_chunks()")


def _range_get(s3_client, bucket, key, start, length):
    if length <= 0:
        return b""
    end = int(start) + int(length) - 1
    obj = s3_client.get_object(Bucket=bucket, Key=key, Range=f"bytes={int(start)}-{end}")
    data = _read_body_bytes(obj["Body"])
    if len(data) != int(length):
        raise RuntimeError(
            f"short S3 range read for {key}: bytes={int(start)}-{end} "
            f"got={len(data)} expected={int(length)}"
        )
    return data


def _logical_row_mapping(full_n, view_n, logical_row):
    physical_i1 = min(full_n - 1, (int(logical_row) * full_n) // view_n)
    physical_js = []
    for logical_j in range(view_n):
        logical_i2 = (view_n - 1 - logical_j) if (logical_row & 1) else logical_j
        physical_i2 = min(full_n - 1, (logical_i2 * full_n) // view_n)
        physical_j = (full_n - 1 - physical_i2) if (physical_i1 & 1) else physical_i2
        physical_js.append(physical_j)
    return physical_i1, physical_js


def _fetch_source_row(s3_client, bucket, solve_manifest, family, solve_start, solve_count, row_bytes):
    spans = build_source_spans(
        solve_manifest,
        source_family=family,
        solve_start=solve_start,
        solve_count=solve_count,
    )
    if not spans:
        raise RuntimeError(
            f"logical lores materialization resolved no {family} spans "
            f"for solve_start={solve_start} solve_count={solve_count}"
        )

    row = bytearray(int(solve_count) * int(row_bytes))
    bytes_read = 0
    range_gets = 0
    for span in spans:
        length = int(span.get("byte_length") or 0)
        if length <= 0:
            continue
        local_solve_start = int(span.get("local_solve_start") or 0)
        dst_start = local_solve_start * int(row_bytes)
        dst_end = dst_start + length
        if dst_start < 0 or dst_end > len(row):
            raise RuntimeError(
                f"logical lores {family} span writes outside row buffer: "
                f"dst={dst_start}:{dst_end} row_bytes={len(row)}"
            )
        data = _range_get(
            s3_client,
            bucket,
            str(span["key"]),
            int(span.get("byte_start") or 0),
            length,
        )
        row[dst_start:dst_end] = data
        bytes_read += length
        range_gets += 1
    return row, bytes_read, range_gets


def _family_row_bytes(family, degree, n_coeffs):
    if family == "slv":
        return root_row_bytes(degree)
    if family == "cf":
        return coeff_row_bytes(n_coeffs)
    if family == "pm":
        return param_row_bytes()
    raise RuntimeError(f"unknown logical lores source family: {family}")


def _materialize_family(
    *,
    s3_client,
    bucket,
    solve_manifest,
    family,
    full_n,
    view_n,
    times,
    row_bytes,
    out_path,
    fetch_workers=16,
):
    t0 = time.time()
    output_rows = int(view_n) * int(view_n) * int(times)
    output_bytes = output_rows * int(row_bytes)
    bytes_read = 0
    range_gets = 0
    source_rows = 0

    # PARALLEL row fan-out: the sequential loop was latency-bound (one
    # ranged fetch per logical row, back to back — 512 round trips WAS the
    # user-felt minute at 512²). Every logical row compacts into an
    # independent fixed-size block at a known offset, so rows fetch
    # concurrently and assemble in memory byte-identically to the
    # sequential order. Tiny jobs (and tiny test fixtures) stay sequential
    # and deterministic below the threshold.
    tasks = []
    for pass_idx in range(int(times)):
        pass_base = pass_idx * int(full_n) * int(full_n)
        for logical_row in range(int(view_n)):
            tasks.append((pass_base, logical_row))
    row_out_bytes = int(view_n) * int(row_bytes)
    buf = bytearray(len(tasks) * row_out_bytes)

    def fetch_one(task_idx):
        pass_base, logical_row = tasks[task_idx]
        physical_i1, physical_js = _logical_row_mapping(int(full_n), int(view_n), logical_row)
        solve_start = pass_base + physical_i1 * int(full_n)
        row, row_bytes_read, row_range_gets = _fetch_source_row(
            s3_client,
            bucket,
            solve_manifest,
            family,
            solve_start,
            int(full_n),
            int(row_bytes),
        )
        chunk = bytearray(row_out_bytes)
        for j, physical_j in enumerate(physical_js):
            rec_start = int(physical_j) * int(row_bytes)
            chunk[j * int(row_bytes):(j + 1) * int(row_bytes)] = row[rec_start:rec_start + int(row_bytes)]
        return task_idx, bytes(chunk), row_bytes_read, row_range_gets

    workers = min(int(fetch_workers or 1), len(tasks))
    if workers > 1 and len(tasks) > 8:
        with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
            results = pool.map(fetch_one, range(len(tasks)))
            for task_idx, chunk, row_bytes_read, row_range_gets in results:
                buf[task_idx * row_out_bytes:(task_idx + 1) * row_out_bytes] = chunk
                bytes_read += row_bytes_read
                range_gets += row_range_gets
                source_rows += 1
    else:
        for task_idx in range(len(tasks)):
            _, chunk, row_bytes_read, row_range_gets = fetch_one(task_idx)
            buf[task_idx * row_out_bytes:(task_idx + 1) * row_out_bytes] = chunk
            bytes_read += row_bytes_read
            range_gets += row_range_gets
            source_rows += 1
    with open(out_path, "wb") as out:
        out.write(buf)

    actual_size = os.path.getsize(out_path)
    if actual_size != output_bytes:
        raise RuntimeError(
            f"logical lores {family} output size mismatch: got={actual_size} expected={output_bytes}"
        )
    return {
        "family": family,
        "path": out_path,
        "row_bytes": int(row_bytes),
        "source_rows": int(source_rows),
        "range_gets": int(range_gets),
        "bytes_read": int(bytes_read),
        "output_bytes": int(output_bytes),
        "elapsed_ms": int((time.time() - t0) * 1000),
    }


def materialize_logical_lores(
    *,
    s3_client,
    bucket,
    calc,
    job_id,
    degree,
    n_coeffs,
    view_n,
    out_paths,
    include_coeff=False,
    include_param=False,
    fetch_workers=16,
):
    t0 = time.time()
    full_n, times = calc_square_grid(calc)
    view_n = int(view_n)
    if view_n < 1:
        raise RuntimeError(f"logical_lores_size must be >= 1, got {view_n}")
    if view_n > full_n:
        raise RuntimeError(f"logical_lores_size={view_n} exceeds source N={full_n}")

    chunk_items = build_chunk_items(calc or {}, job_id, require_chunks=True)
    expected_solves = int(full_n) * int(full_n) * int(times)
    covered_solves = sum(int(item.get("step_count") or 0) for item in chunk_items)
    if covered_solves < expected_solves:
        raise RuntimeError(
            f"calc.json chunk metadata covers {covered_solves} solves, "
            f"but logical lores needs {expected_solves}"
        )

    solve_manifest = build_solve_source_manifest(
        chunk_items,
        job_id=job_id,
        degree=degree,
        n_coeffs=n_coeffs,
        include_solve=True,
        include_coeff=include_coeff,
        include_param=include_param,
    )

    families = ["slv"]
    if include_coeff:
        families.append("cf")
    if include_param:
        families.append("pm")

    family_stats = {}
    for family in families:
        out_path = str(out_paths.get(family) or "").strip()
        if not out_path:
            raise RuntimeError(f"missing output path for logical lores family {family}")
        family_stats[family] = _materialize_family(
            s3_client=s3_client,
            bucket=bucket,
            solve_manifest=solve_manifest,
            family=family,
            full_n=full_n,
            view_n=view_n,
            times=times,
            row_bytes=_family_row_bytes(family, degree, n_coeffs),
            out_path=out_path,
            fetch_workers=fetch_workers,
        )

    total_bytes_read = sum(row["bytes_read"] for row in family_stats.values())
    total_output_bytes = sum(row["output_bytes"] for row in family_stats.values())
    total_range_gets = sum(row["range_gets"] for row in family_stats.values())
    return {
        "mode": "logical",
        "full_N": int(full_n),
        "view_N": int(view_n),
        "times": int(times),
        "n_solves": int(view_n) * int(view_n) * int(times),
        "covered_solves": int(covered_solves),
        "chunk_count": len(chunk_items),
        "families": family_stats,
        "bytes_read": int(total_bytes_read),
        "output_bytes": int(total_output_bytes),
        "range_gets": int(total_range_gets),
        "elapsed_ms": int((time.time() - t0) * 1000),
        "mapping": "pass-major logical grid, floor(i*full_N/view_N), serpentine compact order",
        "fetch_workers": int(fetch_workers or 1),
    }


def estimate_logical_lores_bytes(*, calc, degree, n_coeffs, view_n, include_coeff=False, include_param=False):
    full_n, times = calc_square_grid(calc)
    view_n = int(view_n)
    families = ["slv"]
    if include_coeff:
        families.append("cf")
    if include_param:
        families.append("pm")
    source_row_bytes = 0
    compact_row_bytes = 0
    for family in families:
        rb = _family_row_bytes(family, degree, n_coeffs)
        source_row_bytes += int(times) * int(view_n) * int(full_n) * rb
        compact_row_bytes += int(times) * int(view_n) * int(view_n) * rb
    return {
        "estimated_source_bytes": int(source_row_bytes),
        "estimated_output_bytes": int(compact_row_bytes),
    }
