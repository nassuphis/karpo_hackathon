#!/usr/bin/env python3
"""
Stage 0 benchmark for solve-score histogram batching.

This runs locally against a real compute job's chunk objects in S3 and answers:

- does batching multiple chunks into one worker session help?
- how much wall time drops when we reduce per-batch setup overhead?

It does NOT require deploying a Lambda.
It uses the current tmpfile + solve_proximity_stats flow and can simulate one
worker process per batch with configurable local concurrency.

Example:
  uv run python scripts/bench_solve_hist_batch.py \
    --job-id compute_mmw2ilf7 \
    --metric centroid_re \
    --batch-sizes 1,2,4,8 \
    --max-chunks 80 \
    --concurrency 10
"""

from __future__ import annotations

import argparse
import concurrent.futures
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path
from typing import Iterable

ROOT = Path(__file__).resolve().parents[1]
LAMBDA_DIR = ROOT / "lambda"
SOLVE_PROX_SRC = LAMBDA_DIR / "solve_proximity_stats.c"
HOST_SOLVE_PROX_BIN = Path(tempfile.gettempdir()) / "polypaint_solve_proximity_stats_host"
DEFAULT_BUCKET = os.environ.get("BUCKET", "polypaint")
DEFAULT_REGION = os.environ.get("AWS_REGION", "us-east-1")


def extract_chunk_items(calc: dict, job_id: str) -> list[dict]:
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


def batch_chunk_items(items: list[dict], batch_size: int) -> list[list[dict]]:
    if batch_size < 1:
        raise ValueError(f"batch_size must be >= 1, got {batch_size}")
    return [items[i:i + batch_size] for i in range(0, len(items), batch_size)]


def parse_batch_sizes(value: str) -> list[int]:
    sizes = []
    for raw in str(value).split(","):
        raw = raw.strip()
        if not raw:
            continue
        try:
            size = int(raw)
        except ValueError as e:
            raise argparse.ArgumentTypeError(f"invalid batch size {raw!r}") from e
        if size < 1:
            raise argparse.ArgumentTypeError(f"batch size must be >= 1, got {size}")
        sizes.append(size)
    if not sizes:
        raise argparse.ArgumentTypeError("at least one batch size is required")
    return sorted(set(sizes))


def ensure_host_solve_prox_binary(binary_path: Path = HOST_SOLVE_PROX_BIN) -> Path:
    if binary_path.exists() and binary_path.stat().st_mtime >= SOLVE_PROX_SRC.stat().st_mtime:
        return binary_path
    cc = shutil.which("cc")
    if not cc:
        raise RuntimeError("cc not found; cannot build host solve_proximity_stats benchmark binary")
    cmd = [
        cc,
        "-O3",
        "-pthread",
        "-I",
        str(LAMBDA_DIR),
        "-o",
        str(binary_path),
        str(SOLVE_PROX_SRC),
        "-lm",
    ]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"failed to compile host solve_proximity_stats: {result.stderr.strip()}")
    return binary_path


def s3_client(region: str):
    try:
        import boto3
    except ModuleNotFoundError as e:
        raise RuntimeError(
            "boto3 is required to run the benchmark against S3. "
            "Use the project environment, e.g. `uv run python ...`."
        ) from e
    return boto3.client("s3", region_name=region)


def download_s3_to_file(s3, bucket: str, key: str, dest_path: Path) -> tuple[int, int]:
    t0 = time.perf_counter()
    obj = s3.get_object(Bucket=bucket, Key=key)
    size = 0
    with dest_path.open("wb") as f:
        for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
            if not chunk:
                continue
            f.write(chunk)
            size += len(chunk)
    dl_ms = int((time.perf_counter() - t0) * 1000)
    return size, dl_ms


def load_calc_from_s3(s3, bucket: str, job_id: str) -> dict:
    obj = s3.get_object(Bucket=bucket, Key=f"renders/{job_id}/calc.json")
    return json.loads(obj["Body"].read())


def run_clip(binary_path: Path, input_path: Path, degree: int, metric: str, quantile: float,
             threads: int = 1, root_xforms_json: str | None = None) -> tuple[dict, int]:
    cmd = [
        str(binary_path),
        str(input_path),
        "--mode=clip",
        f"--degree={degree}",
        f"--metric={metric}",
        f"--quantile_lo={quantile}",
        f"--quantile_hi={1.0 - quantile}",
        f"--threads={threads}",
    ]
    if root_xforms_json:
        cmd.append(f"--root_xforms={root_xforms_json}")
    t0 = time.perf_counter()
    result = subprocess.run(cmd, capture_output=True, text=True)
    compute_ms = int((time.perf_counter() - t0) * 1000)
    if result.returncode != 0:
        raise RuntimeError(f"solve_proximity_stats clip failed: {result.stderr.strip()}")
    return json.loads(result.stdout), compute_ms


def run_hist(binary_path: Path, input_path: Path, degree: int, metric: str, clip_lo: float, clip_hi: float,
             hist_bins: int, threads: int, omega: float, omega_enabled: bool,
             root_xforms_json: str | None = None) -> tuple[dict, int]:
    cmd = [
        str(binary_path),
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
    if root_xforms_json:
        cmd.append(f"--root_xforms={root_xforms_json}")
    t0 = time.perf_counter()
    result = subprocess.run(cmd, capture_output=True, text=True)
    compute_ms = int((time.perf_counter() - t0) * 1000)
    if result.returncode != 0:
        raise RuntimeError(f"solve_proximity_stats hist failed: {result.stderr.strip()}")
    return json.loads(result.stdout), compute_ms


def process_batch(manifest: dict) -> dict:
    bucket = manifest["bucket"]
    region = manifest["region"]
    degree = int(manifest["degree"])
    metric = manifest["metric"]
    clip_lo = float(manifest["clip_lo"])
    clip_hi = float(manifest["clip_hi"])
    hist_bins = int(manifest["hist_bins"])
    threads = int(manifest["threads"])
    omega = float(manifest["omega"])
    omega_enabled = bool(manifest["omega_enabled"])
    binary_path = Path(manifest["binary_path"])
    root_xforms_json = manifest.get("root_xforms_json") or None
    chunks = list(manifest["chunks"])

    s3 = s3_client(region)
    totals = {
        "batch_index": int(manifest["batch_index"]),
        "batch_size": len(chunks),
        "chunks_processed": 0,
        "bytes_downloaded": 0,
        "download_ms": 0,
        "compute_ms": 0,
        "n_solves_total": 0,
        "hist_checksum": 0,
    }

    with tempfile.TemporaryDirectory(prefix="solve_hist_batch_") as tmpdir:
        tmpdir_path = Path(tmpdir)
        for chunk in chunks:
            input_path = tmpdir_path / f"chunk_{chunk['chunk_idx']:04d}.bin"
            size, dl_ms = download_s3_to_file(s3, bucket, chunk["bin_key"], input_path)
            hist_data, compute_ms = run_hist(
                binary_path=binary_path,
                input_path=input_path,
                degree=degree,
                metric=metric,
                clip_lo=clip_lo,
                clip_hi=clip_hi,
                hist_bins=hist_bins,
                threads=threads,
                omega=omega,
                omega_enabled=omega_enabled,
                root_xforms_json=root_xforms_json,
            )
            totals["chunks_processed"] += 1
            totals["bytes_downloaded"] += size
            totals["download_ms"] += dl_ms
            totals["compute_ms"] += compute_ms
            totals["n_solves_total"] += int(hist_data["n_solves"])
            totals["hist_checksum"] += sum((i + 1) * int(v) for i, v in enumerate(hist_data["hist"]))
            input_path.unlink(missing_ok=True)
    return totals


def run_worker_subprocess(manifest: dict) -> dict:
    with tempfile.NamedTemporaryFile("w", suffix=".json", delete=False, prefix="bench_hist_manifest_") as f:
        json.dump(manifest, f)
        manifest_path = Path(f.name)
    try:
        cmd = [sys.executable, str(Path(__file__).resolve()), "--worker-manifest", str(manifest_path)]
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            raise RuntimeError(result.stderr.strip() or result.stdout.strip() or "worker failed")
        return json.loads(result.stdout)
    finally:
        manifest_path.unlink(missing_ok=True)


def aggregate_results(rows: Iterable[dict]) -> dict:
    rows = list(rows)
    return {
        "batches": len(rows),
        "chunks_processed": sum(int(r["chunks_processed"]) for r in rows),
        "bytes_downloaded": sum(int(r["bytes_downloaded"]) for r in rows),
        "download_ms": sum(int(r["download_ms"]) for r in rows),
        "compute_ms": sum(int(r["compute_ms"]) for r in rows),
        "n_solves_total": sum(int(r["n_solves_total"]) for r in rows),
        "hist_checksum": sum(int(r["hist_checksum"]) for r in rows),
    }


def build_progress_line(*, batch_size: int, completed_batches: int, total_batches: int,
                        chunks_processed: int, total_chunks: int, wall_ms: int,
                        download_ms: int, compute_ms: int, worker_mode: str) -> str:
    return (
        f"[progress] batch_size={batch_size} mode={worker_mode} "
        f"batches={completed_batches}/{total_batches} "
        f"chunks={chunks_processed}/{total_chunks} "
        f"wall={fmt_seconds_ms(wall_ms)} "
        f"download={fmt_seconds_ms(download_ms)} "
        f"compute={fmt_seconds_ms(compute_ms)}"
    )


def run_batch_benchmark(base_manifest: dict, chunk_items: list[dict], batch_size: int, *,
                        concurrency: int, worker_mode: str) -> dict:
    batches = batch_chunk_items(chunk_items, batch_size)
    manifests = []
    for batch_index, batch in enumerate(batches):
        manifest = dict(base_manifest)
        manifest["batch_index"] = batch_index
        manifest["chunks"] = batch
        manifests.append(manifest)

    wall_t0 = time.perf_counter()
    rows = []
    fn = run_worker_subprocess if worker_mode == "subprocess" else process_batch
    print(
        f"[start] batch_size={batch_size} mode={worker_mode} "
        f"batches={len(batches)} chunks={len(chunk_items)} concurrency={max(1, concurrency)}",
        flush=True,
    )
    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, concurrency)) as pool:
        futures = [pool.submit(fn, manifest) for manifest in manifests]
        for future in concurrent.futures.as_completed(futures):
            rows.append(future.result())
            agg_so_far = aggregate_results(rows)
            wall_ms_so_far = int((time.perf_counter() - wall_t0) * 1000)
            print(
                build_progress_line(
                    batch_size=batch_size,
                    completed_batches=len(rows),
                    total_batches=len(batches),
                    chunks_processed=agg_so_far["chunks_processed"],
                    total_chunks=len(chunk_items),
                    wall_ms=wall_ms_so_far,
                    download_ms=agg_so_far["download_ms"],
                    compute_ms=agg_so_far["compute_ms"],
                    worker_mode=worker_mode,
                ),
                flush=True,
            )
    wall_ms = int((time.perf_counter() - wall_t0) * 1000)
    agg = aggregate_results(rows)
    agg["wall_ms"] = wall_ms
    agg["batch_size"] = batch_size
    agg["concurrency"] = concurrency
    agg["worker_mode"] = worker_mode
    return agg


def fmt_seconds_ms(ms: int | float) -> str:
    return f"{(float(ms) / 1000.0):.1f}s"


def fmt_mb(bytes_count: int) -> str:
    return f"{bytes_count / (1024 * 1024):.1f}MB"


def fmt_bytes(bytes_count: int | None) -> str:
    if bytes_count is None or bytes_count < 0:
        return "-"
    if bytes_count < 1024:
        return f"{bytes_count}B"
    if bytes_count < 1024 * 1024:
        return f"{bytes_count / 1024.0:.1f}KB"
    if bytes_count < 1024 * 1024 * 1024:
        return f"{bytes_count / (1024.0 * 1024.0):.1f}MB"
    return f"{bytes_count / (1024.0 * 1024.0 * 1024.0):.2f}GB"


def solve_bytes_for_degree(degree: int) -> int:
    return max(1, int(degree) * 2 * 4)


def solves_for_bin_bytes(byte_count: int | None, degree: int) -> int | None:
    if byte_count is None or byte_count < 0:
        return None
    solve_bytes = solve_bytes_for_degree(degree)
    if solve_bytes <= 0:
        return None
    return int(byte_count) // solve_bytes


def head_object_size(s3, bucket: str, key: str) -> int:
    meta = s3.head_object(Bucket=bucket, Key=key)
    return int(meta.get("ContentLength") or 0)


def print_describe_summary(job_id: str, degree: int, lores_key: str, lores_size: int | None,
                           total_chunks: int, selected_chunks: list[dict], chunk_rows: list[dict]):
    print(f"Job: {job_id}")
    print(f"Degree: {degree}")
    print(f"Solve bytes: {solve_bytes_for_degree(degree)}")
    print(f"Total chunks in calc: {total_chunks}")
    print(f"Selected chunks: {len(selected_chunks)}")
    print(f"Lores key: {lores_key}")
    print(
        "Lores size: "
        f"{fmt_bytes(lores_size)}"
        + (
            f" ({solves_for_bin_bytes(lores_size, degree)} solves)"
            if solves_for_bin_bytes(lores_size, degree) is not None
            else ""
        )
    )
    print("")
    print("Chunk  Declared      Actual        Solves      Key")
    for row in chunk_rows:
        solves = row["actual_solves"]
        solves_str = str(solves) if solves is not None else "-"
        print(
            f"{row['chunk_idx']:>5}  "
            f"{fmt_bytes(row['declared_size']):>11}  "
            f"{fmt_bytes(row['actual_size']):>11}  "
            f"{solves_str:>10}  "
            f"{row['bin_key']}"
        )


def describe_job(args) -> int:
    s3 = s3_client(args.region)
    calc = load_calc_from_s3(s3, args.bucket, args.job_id)
    degree = int(calc.get("degree") or 0)
    if degree < 1:
        raise RuntimeError(f"calc.json missing valid degree for {args.job_id}")
    lores_key = (calc.get("lores") or {}).get("bin_key")
    if not lores_key:
        raise RuntimeError(f"calc.json missing lores.bin_key for {args.job_id}")

    all_chunk_items = extract_chunk_items(calc, args.job_id)
    selected_chunks = all_chunk_items[args.chunk_offset:]
    if args.max_chunks:
        selected_chunks = selected_chunks[:args.max_chunks]
    if not selected_chunks:
        raise RuntimeError("No chunks selected for description")

    lores_size = head_object_size(s3, args.bucket, lores_key)
    describe_count = min(len(selected_chunks), max(1, args.describe_chunks))
    chunk_rows = []
    for chunk in selected_chunks[:describe_count]:
        actual_size = head_object_size(s3, args.bucket, chunk["bin_key"])
        chunk_rows.append({
            "chunk_idx": int(chunk["chunk_idx"]),
            "bin_key": chunk["bin_key"],
            "declared_size": int(chunk["bin_size"]) if chunk.get("bin_size") is not None else None,
            "actual_size": actual_size,
            "actual_solves": solves_for_bin_bytes(actual_size, degree),
        })

    print_describe_summary(
        job_id=args.job_id,
        degree=degree,
        lores_key=lores_key,
        lores_size=lores_size,
        total_chunks=len(all_chunk_items),
        selected_chunks=selected_chunks,
        chunk_rows=chunk_rows,
    )
    if args.json:
        print("")
        print(json.dumps({
            "job_id": args.job_id,
            "degree": degree,
            "solve_bytes": solve_bytes_for_degree(degree),
            "total_chunks": len(all_chunk_items),
            "selected_chunks": len(selected_chunks),
            "lores_key": lores_key,
            "lores_size": lores_size,
            "lores_solves": solves_for_bin_bytes(lores_size, degree),
            "chunk_rows": chunk_rows,
        }, indent=2))
    return 0


def print_summary(job_id: str, metric: str, degree: int, chunk_count: int, concurrency: int,
                  clip_info: dict, results: list[dict]):
    print(f"Job: {job_id}")
    print(f"Metric: {metric}")
    print(f"Degree: {degree}")
    print(f"Chunks tested: {chunk_count}")
    print(f"Concurrency: {concurrency}")
    print(
        "Clip: "
        f"lo={clip_info['clip_lo']:.6g} hi={clip_info['clip_hi']:.6g} "
        f"download={fmt_seconds_ms(clip_info['download_ms'])} "
        f"compute={fmt_seconds_ms(clip_info['compute_ms'])}"
    )
    print("")
    print("Batch  Batches  Wall     Download   Compute   Bytes     AvgWall/chunk  AvgDl/chunk  AvgCpu/chunk")
    baseline_wall = results[0]["wall_ms"] if results else 0
    for row in results:
        chunks = max(1, row["chunks_processed"])
        avg_wall = row["wall_ms"] / chunks
        avg_dl = row["download_ms"] / chunks
        avg_cpu = row["compute_ms"] / chunks
        speedup = baseline_wall / row["wall_ms"] if row["wall_ms"] > 0 and baseline_wall > 0 else 1.0
        print(
            f"{row['batch_size']:>5}  "
            f"{row['batches']:>7}  "
            f"{fmt_seconds_ms(row['wall_ms']):>7}  "
            f"{fmt_seconds_ms(row['download_ms']):>9}  "
            f"{fmt_seconds_ms(row['compute_ms']):>8}  "
            f"{fmt_mb(row['bytes_downloaded']):>8}  "
            f"{fmt_seconds_ms(avg_wall):>13}  "
            f"{fmt_seconds_ms(avg_dl):>12}  "
            f"{fmt_seconds_ms(avg_cpu):>13}  "
            f"x{speedup:.2f}"
        )


def benchmark(args) -> int:
    s3 = s3_client(args.region)
    calc = load_calc_from_s3(s3, args.bucket, args.job_id)
    degree = int(calc.get("degree") or 0)
    if degree < 1:
        raise RuntimeError(f"calc.json missing valid degree for {args.job_id}")
    lores_key = (calc.get("lores") or {}).get("bin_key")
    if not lores_key:
        raise RuntimeError(f"calc.json missing lores.bin_key for {args.job_id}")

    chunk_items = extract_chunk_items(calc, args.job_id)
    if args.chunk_offset:
        chunk_items = chunk_items[args.chunk_offset:]
    if args.max_chunks:
        chunk_items = chunk_items[:args.max_chunks]
    if not chunk_items:
        raise RuntimeError("No chunks selected for benchmark")

    binary_path = ensure_host_solve_prox_binary()

    with tempfile.TemporaryDirectory(prefix="solve_hist_clip_") as tmpdir:
        lores_path = Path(tmpdir) / "lores.bin"
        lores_size, lores_dl_ms = download_s3_to_file(s3, args.bucket, lores_key, lores_path)
        clip_data, clip_compute_ms = run_clip(
            binary_path=binary_path,
            input_path=lores_path,
            degree=degree,
            metric=args.metric,
            quantile=args.quantile,
            threads=args.threads,
            root_xforms_json=args.root_xforms_json,
        )

    base_manifest = {
        "bucket": args.bucket,
        "region": args.region,
        "degree": degree,
        "metric": args.metric,
        "clip_lo": clip_data["clip_lo"],
        "clip_hi": clip_data["clip_hi"],
        "hist_bins": args.hist_bins,
        "threads": args.threads,
        "omega": args.omega,
        "omega_enabled": args.omega_enabled,
        "binary_path": str(binary_path),
        "root_xforms_json": os.path.abspath(args.root_xforms_json) if args.root_xforms_json else "",
    }

    results = []
    for batch_size in args.batch_sizes:
        row = run_batch_benchmark(
            base_manifest=base_manifest,
            chunk_items=chunk_items,
            batch_size=batch_size,
            concurrency=args.concurrency,
            worker_mode=args.worker_mode,
        )
        results.append(row)

    clip_info = {
        "download_ms": lores_dl_ms,
        "compute_ms": clip_compute_ms,
        "clip_lo": clip_data["clip_lo"],
        "clip_hi": clip_data["clip_hi"],
        "lores_size": lores_size,
    }

    print_summary(args.job_id, args.metric, degree, len(chunk_items), args.concurrency, clip_info, results)
    if args.json:
        print("")
        print(json.dumps({
            "job_id": args.job_id,
            "metric": args.metric,
            "degree": degree,
            "chunk_count": len(chunk_items),
            "concurrency": args.concurrency,
            "worker_mode": args.worker_mode,
            "clip": clip_info,
            "results": results,
        }, indent=2))
    return 0


def worker_entry(args) -> int:
    manifest = json.loads(Path(args.worker_manifest).read_text())
    result = process_batch(manifest)
    sys.stdout.write(json.dumps(result))
    return 0


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description="Local Stage 0 solve-score hist batching benchmark")
    p.add_argument("--job-id", help="Compute job id, e.g. compute_mmw2ilf7")
    p.add_argument("--bucket", default=DEFAULT_BUCKET)
    p.add_argument("--region", default=DEFAULT_REGION)
    p.add_argument("--describe-only", action="store_true",
                   help="Only inspect calc/lores/chunk sizes; do not run clip or hist")
    p.add_argument("--describe-chunks", type=int, default=8,
                   help="In describe-only mode, inspect at most this many selected chunks")
    p.add_argument("--metric", default="proximity")
    p.add_argument("--quantile", type=float, default=0.001, help="Solve-score clip quantile q")
    p.add_argument("--hist-bins", type=int, default=100)
    p.add_argument("--threads", type=int, default=1, help="Native solve_proximity_stats threads per chunk")
    p.add_argument("--omega", type=float, default=1.0)
    p.add_argument("--omega-enabled", action="store_true", default=True)
    p.add_argument("--no-omega-enabled", dest="omega_enabled", action="store_false")
    p.add_argument("--batch-sizes", type=parse_batch_sizes, default=[1, 2, 4, 8],
                   help="Comma-separated batch sizes, e.g. 1,2,4,8")
    p.add_argument("--concurrency", type=int, default=10,
                   help="How many batch workers to run concurrently locally")
    p.add_argument("--max-chunks", type=int, default=80,
                   help="Benchmark at most this many chunks")
    p.add_argument("--chunk-offset", type=int, default=0,
                   help="Skip this many initial chunks before benchmarking")
    p.add_argument("--worker-mode", choices=("subprocess", "inline"), default="subprocess",
                   help="subprocess more closely simulates one worker session per batch")
    p.add_argument("--root-xforms-json", default="",
                   help="Optional JSON file with root transforms to pass through to the native binary")
    p.add_argument("--json", action="store_true", help="Also print machine-readable JSON summary")
    p.add_argument("--worker-manifest", help=argparse.SUPPRESS)
    return p


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)

    if args.worker_manifest:
        return worker_entry(args)

    if not args.job_id:
        parser.error("--job-id is required")
    if args.concurrency < 1:
        parser.error("--concurrency must be >= 1")
    if args.max_chunks is not None and args.max_chunks < 1:
        parser.error("--max-chunks must be >= 1")
    if args.chunk_offset < 0:
        parser.error("--chunk-offset must be >= 0")
    if args.describe_chunks < 1:
        parser.error("--describe-chunks must be >= 1")
    if not (0.001 <= args.quantile <= 0.05):
        parser.error("--quantile must be in [0.001, 0.05]")
    if args.threads < 1:
        parser.error("--threads must be >= 1")
    if args.root_xforms_json and not Path(args.root_xforms_json).exists():
        parser.error(f"--root-xforms-json not found: {args.root_xforms_json}")

    if args.describe_only:
        return describe_job(args)
    return benchmark(args)


if __name__ == "__main__":
    raise SystemExit(main())
