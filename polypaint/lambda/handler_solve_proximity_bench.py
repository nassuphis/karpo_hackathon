"""
AWS-side solve-score histogram benchmark Lambda.

Purpose:
- benchmark solve-score hist download/scan behavior inside Lambda, in-region with S3
- sweep chunk-worker concurrency over a selected subset of chunks
- return wall/download/compute timings for comparison

This is intentionally separate from the render path. It is a direct-invoke benchmark tool,
not a production render handler.
"""
from __future__ import annotations

import concurrent.futures
import json
import logging
import os
import subprocess
import tempfile
import time
from pathlib import Path

import boto3

import handler_solve_proximity as hsp
from shared import BUCKET, ok_response, parse_body

logger = logging.getLogger()
logger.setLevel(logging.INFO)

s3 = boto3.client("s3")
BINARY = os.path.join(os.path.dirname(__file__), "solve_proximity_stats")
MAX_SWEEP_CONCURRENCY = 32
TIME_GUARD_MS = 60_000


def _extract_chunk_items(calc: dict, job_id: str) -> list[dict]:
    chunks = list(calc.get("chunks", calc.get("stripes", [])) or [])
    if chunks:
        items = []
        for raw in chunks:
            idx = raw.get("idx", raw.get("chunk_idx", raw.get("stripe_idx")))
            bin_key = raw.get("bin_key", raw.get("s3_key"))
            if idx is None or not bin_key:
                raise RuntimeError(f"Invalid chunk metadata: idx={idx} bin_key={bin_key!r}")
            item = {"chunk_idx": int(idx), "bin_key": str(bin_key)}
            if raw.get("bin_size") is not None:
                item["bin_size"] = int(raw["bin_size"])
            items.append(item)
        items.sort(key=lambda item: item["chunk_idx"])
        return items

    n_chunks = int(calc.get("n_chunks", calc.get("n_stripes", 0)) or 0)
    if n_chunks <= 0:
        raise RuntimeError(f"calc.json missing chunk metadata for {job_id}")
    return [{"chunk_idx": c, "bin_key": f"renders/{job_id}/chunk_{c}.bin"} for c in range(n_chunks)]


def _parse_concurrency_values(value) -> list[int]:
    if value in (None, "", []):
        return [1, 2, 4]
    if isinstance(value, int):
        values = [value]
    elif isinstance(value, (list, tuple)):
        values = list(value)
    else:
        values = [raw.strip() for raw in str(value).split(",") if raw.strip()]

    parsed = []
    for raw in values:
        try:
            val = int(raw)
        except (TypeError, ValueError):
            raise RuntimeError(f"concurrency_values must contain integers, got {raw!r}")
        if not (1 <= val <= MAX_SWEEP_CONCURRENCY):
            raise RuntimeError(f"concurrency must be in [1, {MAX_SWEEP_CONCURRENCY}], got {val}")
        parsed.append(val)
    if not parsed:
        raise RuntimeError("concurrency_values must not be empty")
    return sorted(set(parsed))


def _download_to_file(key: str, path: Path) -> int:
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=key)
    except Exception as e:
        raise RuntimeError(f"Failed to download s3://{BUCKET}/{key}: {e}") from e
    size = 0
    with path.open("wb") as f:
        for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
            if not chunk:
                continue
            f.write(chunk)
            size += len(chunk)
    return size


def _write_root_xforms(tmpdir: Path, root_transforms) -> str | None:
    if not root_transforms:
        return None
    path = tmpdir / "root_xforms.json"
    path.write_text(json.dumps(root_transforms))
    return str(path)


def _run_clip(binary_path: str, input_path: Path, *, degree: int, metric: str, quantile: float,
              omega: float, omega_enabled: bool, threads: int, xf_path: str | None) -> dict:
    cmd = [
        binary_path,
        str(input_path),
        "--mode=clip",
        f"--degree={degree}",
        f"--metric={metric}",
        f"--quantile_lo={quantile}",
        f"--quantile_hi={1.0 - quantile}",
        f"--omega={omega}",
        f"--omega_enabled={1 if omega_enabled else 0}",
        f"--threads={threads}",
    ]
    if xf_path:
        cmd.append(f"--root_xforms={xf_path}")
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
    if result.returncode != 0:
        raise RuntimeError(f"solve_proximity_stats clip failed: {result.stderr.strip()}")
    return json.loads(result.stdout)


def _run_hist(binary_path: str, input_path: Path, *, degree: int, metric: str, clip_lo: float, clip_hi: float,
              hist_bins: int, omega: float, omega_enabled: bool, threads: int, xf_path: str | None) -> dict:
    cmd = [
        binary_path,
        str(input_path),
        "--mode=hist",
        f"--degree={degree}",
        f"--metric={metric}",
        f"--clip_lo={clip_lo}",
        f"--clip_hi={clip_hi}",
        f"--hist_bins={hist_bins}",
        f"--omega={omega}",
        f"--omega_enabled={1 if omega_enabled else 0}",
        f"--threads={threads}",
    ]
    if xf_path:
        cmd.append(f"--root_xforms={xf_path}")
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
    if result.returncode != 0:
        raise RuntimeError(f"solve_proximity_stats hist failed: {result.stderr.strip()}")
    return json.loads(result.stdout)


def _process_hist_chunk(chunk: dict, *, tmpdir: Path, degree: int, metric: str, clip_lo: float, clip_hi: float,
                        hist_bins: int, omega: float, omega_enabled: bool, threads: int, xf_path: str | None) -> dict:
    input_path = tmpdir / f"chunk_{int(chunk['chunk_idx']):04d}.bin"
    dl_t0 = time.perf_counter()
    size = _download_to_file(chunk["bin_key"], input_path)
    dl_ms = int((time.perf_counter() - dl_t0) * 1000)

    compute_t0 = time.perf_counter()
    hist_data = _run_hist(
        BINARY,
        input_path,
        degree=degree,
        metric=metric,
        clip_lo=clip_lo,
        clip_hi=clip_hi,
        hist_bins=hist_bins,
        omega=omega,
        omega_enabled=omega_enabled,
        threads=threads,
        xf_path=xf_path,
    )
    compute_ms = int((time.perf_counter() - compute_t0) * 1000)
    input_path.unlink(missing_ok=True)
    return {
        "chunk_idx": int(chunk["chunk_idx"]),
        "bytes_downloaded": size,
        "download_ms": dl_ms,
        "compute_ms": compute_ms,
        "n_solves": int(hist_data["n_solves"]),
        "hist_checksum": sum((i + 1) * int(v) for i, v in enumerate(hist_data["hist"])),
    }


def _aggregate_rows(rows: list[dict]) -> dict:
    return {
        "chunks_processed": sum(int(r["chunks_processed"]) if "chunks_processed" in r else 1 for r in rows),
        "bytes_downloaded": sum(int(r["bytes_downloaded"]) for r in rows),
        "download_ms": sum(int(r["download_ms"]) for r in rows),
        "compute_ms": sum(int(r["compute_ms"]) for r in rows),
        "n_solves_total": sum(int(r["n_solves_total"]) if "n_solves_total" in r else int(r["n_solves"]) for r in rows),
        "hist_checksum": sum(int(r["hist_checksum"]) for r in rows),
    }


def _run_concurrency_benchmark(chunk_items: list[dict], *, concurrency: int, degree: int, metric: str,
                               clip_lo: float, clip_hi: float, hist_bins: int, omega: float,
                               omega_enabled: bool, threads: int, root_transforms) -> dict:
    wall_t0 = time.perf_counter()
    rows = []
    with tempfile.TemporaryDirectory(prefix=f"solve_hist_bench_c{concurrency}_") as tmpdir:
        tmpdir_path = Path(tmpdir)
        xf_path = _write_root_xforms(tmpdir_path, root_transforms)
        with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, concurrency)) as pool:
            futures = [
                pool.submit(
                    _process_hist_chunk,
                    chunk,
                    tmpdir=tmpdir_path,
                    degree=degree,
                    metric=metric,
                    clip_lo=clip_lo,
                    clip_hi=clip_hi,
                    hist_bins=hist_bins,
                    omega=omega,
                    omega_enabled=omega_enabled,
                    threads=threads,
                    xf_path=xf_path,
                )
                for chunk in chunk_items
            ]
            for future in concurrent.futures.as_completed(futures):
                row = future.result()
                rows.append(row)
                agg = _aggregate_rows(rows)
                wall_ms = int((time.perf_counter() - wall_t0) * 1000)
                logger.info(
                    "SOLVE_HIST_BENCH progress concurrency=%d chunks=%d/%d wall_ms=%d download_ms=%d compute_ms=%d",
                    concurrency,
                    len(rows),
                    len(chunk_items),
                    wall_ms,
                    agg["download_ms"],
                    agg["compute_ms"],
                )

    wall_ms = int((time.perf_counter() - wall_t0) * 1000)
    agg = _aggregate_rows(rows)
    return {
        "concurrency": concurrency,
        "chunk_count": len(chunk_items),
        "wall_ms": wall_ms,
        "download_ms": agg["download_ms"],
        "compute_ms": agg["compute_ms"],
        "bytes_downloaded": agg["bytes_downloaded"],
        "n_solves_total": agg["n_solves_total"],
        "hist_checksum": agg["hist_checksum"],
        "avg_wall_ms_per_chunk": wall_ms / max(1, len(chunk_items)),
        "avg_download_ms_per_chunk": agg["download_ms"] / max(1, len(chunk_items)),
        "avg_compute_ms_per_chunk": agg["compute_ms"] / max(1, len(chunk_items)),
    }


def _load_calc(job_id: str) -> dict:
    obj = s3.get_object(Bucket=BUCKET, Key=f"renders/{job_id}/calc.json")
    return json.loads(obj["Body"].read())


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    metric = params.get("metric", "proximity")
    hsp._validate_metric(metric)
    quantile = hsp._validate_quantile(params.get("solve_score_quantile", 0.001))
    omega = hsp._validate_omega(params.get("solve_score_omega", 1.0))
    omega_enabled = hsp._validate_omega_enabled(params.get("solve_score_omega_enabled", True))
    solve_score_threads = hsp._validate_threads(params.get("solve_score_threads", 1), default=1)
    concurrency_values = _parse_concurrency_values(params.get("concurrency_values", [1, 2, 4]))
    max_chunks = int(params.get("max_chunks", 4) or 4)
    chunk_offset = int(params.get("chunk_offset", 0) or 0)
    hist_bins = int(params.get("hist_bins", 100) or 100)
    root_transforms = params.get("root_transforms") or []
    if max_chunks < 1:
        raise RuntimeError("max_chunks must be >= 1")
    if chunk_offset < 0:
        raise RuntimeError("chunk_offset must be >= 0")
    if hist_bins < 1:
        raise RuntimeError("hist_bins must be >= 1")

    calc = _load_calc(job_id)
    degree = int(calc.get("degree") or 0)
    if degree < 1:
        raise RuntimeError(f"calc.json missing valid degree for {job_id}")
    lores_bin_key = (calc.get("lores") or {}).get("bin_key")
    if not lores_bin_key:
        raise RuntimeError(f"calc.json missing lores.bin_key for {job_id}")
    all_chunk_items = _extract_chunk_items(calc, job_id)
    chunk_items = all_chunk_items[chunk_offset:]
    if max_chunks:
        chunk_items = chunk_items[:max_chunks]
    if not chunk_items:
        raise RuntimeError("No chunks selected for benchmark")

    with tempfile.TemporaryDirectory(prefix="solve_hist_bench_clip_") as tmpdir:
        tmpdir_path = Path(tmpdir)
        lores_path = tmpdir_path / "lores.bin"
        xf_path = _write_root_xforms(tmpdir_path, root_transforms)
        clip_dl_t0 = time.perf_counter()
        lores_size = _download_to_file(lores_bin_key, lores_path)
        clip_dl_ms = int((time.perf_counter() - clip_dl_t0) * 1000)
        clip_compute_t0 = time.perf_counter()
        clip_data = _run_clip(
            BINARY,
            lores_path,
            degree=degree,
            metric=metric,
            quantile=quantile,
            omega=omega,
            omega_enabled=omega_enabled,
            threads=solve_score_threads,
            xf_path=xf_path,
        )
        clip_compute_ms = int((time.perf_counter() - clip_compute_t0) * 1000)

    logger.info(
        "SOLVE_HIST_BENCH start job_id=%s degree=%d metric=%s chunks=%d concurrency_values=%s threads=%d",
        job_id, degree, metric, len(chunk_items), concurrency_values, solve_score_threads,
    )

    results = []
    stopped_early = False
    for concurrency in concurrency_values:
        remaining_ms = context.get_remaining_time_in_millis() if context else None
        if remaining_ms is not None and remaining_ms < TIME_GUARD_MS and results:
            stopped_early = True
            logger.warning(
                "SOLVE_HIST_BENCH stopping early before concurrency=%d remaining_ms=%d",
                concurrency, remaining_ms,
            )
            break
        result = _run_concurrency_benchmark(
            chunk_items,
            concurrency=concurrency,
            degree=degree,
            metric=metric,
            clip_lo=float(clip_data["clip_lo"]),
            clip_hi=float(clip_data["clip_hi"]),
            hist_bins=hist_bins,
            omega=omega,
            omega_enabled=omega_enabled,
            threads=solve_score_threads,
            root_transforms=root_transforms,
        )
        logger.info(
            "SOLVE_HIST_BENCH done concurrency=%d wall_ms=%d download_ms=%d compute_ms=%d bytes=%d",
            concurrency,
            result["wall_ms"],
            result["download_ms"],
            result["compute_ms"],
            result["bytes_downloaded"],
        )
        results.append(result)

    body = {
        "mode": "solve_hist_benchmark",
        "job_id": job_id,
        "metric": metric,
        "degree": degree,
        "bucket": BUCKET,
        "memory_mb": int(os.environ.get("AWS_LAMBDA_FUNCTION_MEMORY_SIZE", "0") or 0),
        "clip": {
            "lores_bin_key": lores_bin_key,
            "lores_size": lores_size,
            "clip_lo": clip_data["clip_lo"],
            "clip_hi": clip_data["clip_hi"],
            "n_solves": clip_data["n_solves"],
            "download_ms": clip_dl_ms,
            "compute_ms": clip_compute_ms,
        },
        "chunk_selection": {
            "total_chunks": len(all_chunk_items),
            "chunk_offset": chunk_offset,
            "max_chunks": max_chunks,
            "selected_chunks": len(chunk_items),
            "selected_chunk_indices": [int(c["chunk_idx"]) for c in chunk_items],
            "declared_chunk_sizes": {
                str(int(c["chunk_idx"])): int(c["bin_size"])
                for c in chunk_items
                if c.get("bin_size") is not None
            },
        },
        "threads": solve_score_threads,
        "hist_bins": hist_bins,
        "concurrency_values": concurrency_values,
        "stopped_early": stopped_early,
        "results": results,
    }
    return ok_response(body)
