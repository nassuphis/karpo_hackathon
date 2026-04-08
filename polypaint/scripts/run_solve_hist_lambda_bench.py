#!/usr/bin/env python3
"""
Invoke the AWS-side solve-score histogram benchmark Lambda and print a compact summary.

Example:
  uv run python scripts/run_solve_hist_lambda_bench.py \
    --job-id compute_mmw2ilf7 \
    --metric centroid_re \
    --concurrency-values 1,2,4 \
    --max-chunks 4
"""

from __future__ import annotations

import argparse
import json
import os


DEFAULT_FUNCTION = os.environ.get("SOLVE_HIST_BENCH_FUNCTION", "polypaint-solve-proximity-bench")
DEFAULT_REGION = os.environ.get("AWS_REGION", "us-east-1")


def parse_concurrency_values(value: str) -> list[int]:
    values = []
    for raw in str(value).split(","):
        raw = raw.strip()
        if not raw:
            continue
        try:
            parsed = int(raw)
        except ValueError as e:
            raise argparse.ArgumentTypeError(f"invalid concurrency value {raw!r}") from e
        if parsed < 1:
            raise argparse.ArgumentTypeError(f"concurrency must be >= 1, got {parsed}")
        values.append(parsed)
    if not values:
        raise argparse.ArgumentTypeError("at least one concurrency value is required")
    return sorted(set(values))


def fmt_seconds(ms: int | float) -> str:
    return f"{float(ms) / 1000.0:.1f}s"


def fmt_mb(num_bytes: int | float) -> str:
    return f"{float(num_bytes) / (1024.0 * 1024.0):.1f}MB"


def print_summary(result: dict):
    print(f"Job: {result['job_id']}")
    print(f"Metric: {result['metric']}")
    print(f"Degree: {result['degree']}")
    print(f"Lambda memory: {result.get('memory_mb', 0)} MB")
    clip = result["clip"]
    print(
        f"Clip: lo={clip['clip_lo']:.6g} hi={clip['clip_hi']:.6g} "
        f"download={fmt_seconds(clip['download_ms'])} compute={fmt_seconds(clip['compute_ms'])}"
    )
    sel = result["chunk_selection"]
    print(
        f"Chunks: selected={sel['selected_chunks']} total={sel['total_chunks']} "
        f"indices={sel['selected_chunk_indices']}"
    )
    print("")
    print("Conc  Wall     Download   Compute   Bytes     AvgWall/chunk  AvgDl/chunk  AvgCpu/chunk")
    for row in result["results"]:
        print(
            f"{row['concurrency']:>4}  "
            f"{fmt_seconds(row['wall_ms']):>7}  "
            f"{fmt_seconds(row['download_ms']):>9}  "
            f"{fmt_seconds(row['compute_ms']):>8}  "
            f"{fmt_mb(row['bytes_downloaded']):>8}  "
            f"{fmt_seconds(row['avg_wall_ms_per_chunk']):>13}  "
            f"{fmt_seconds(row['avg_download_ms_per_chunk']):>12}  "
            f"{fmt_seconds(row['avg_compute_ms_per_chunk']):>13}"
        )


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description="Invoke the AWS solve-score hist benchmark Lambda")
    p.add_argument("--job-id", required=True)
    p.add_argument("--function-name", default=DEFAULT_FUNCTION)
    p.add_argument("--region", default=DEFAULT_REGION)
    p.add_argument("--metric", default="proximity")
    p.add_argument("--quantile", type=float, default=0.001)
    p.add_argument("--threads", type=int, default=1)
    p.add_argument("--hist-bins", type=int, default=100)
    p.add_argument("--omega", type=float, default=1.0)
    p.add_argument("--omega-enabled", action="store_true", default=True)
    p.add_argument("--no-omega-enabled", dest="omega_enabled", action="store_false")
    p.add_argument("--max-chunks", type=int, default=4)
    p.add_argument("--chunk-offset", type=int, default=0)
    p.add_argument("--concurrency-values", type=parse_concurrency_values, default=[1, 2, 4])
    p.add_argument("--json", action="store_true")
    return p


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        import boto3
    except ModuleNotFoundError as e:
        raise RuntimeError(
            "boto3 is required to invoke the benchmark Lambda. "
            "Use the project environment, e.g. `uv run python ...`."
        ) from e
    client = boto3.client("lambda", region_name=args.region)
    payload = {
        "job_id": args.job_id,
        "metric": args.metric,
        "solve_score_quantile": args.quantile,
        "solve_score_threads": args.threads,
        "hist_bins": args.hist_bins,
        "solve_score_omega": args.omega,
        "solve_score_omega_enabled": args.omega_enabled,
        "max_chunks": args.max_chunks,
        "chunk_offset": args.chunk_offset,
        "concurrency_values": args.concurrency_values,
    }
    resp = client.invoke(
        FunctionName=args.function_name,
        InvocationType="RequestResponse",
        Payload=json.dumps({"body": json.dumps(payload)}).encode(),
    )
    payload_bytes = resp["Payload"].read()
    if resp.get("FunctionError"):
        raise RuntimeError(payload_bytes.decode("utf-8", errors="replace"))
    outer = json.loads(payload_bytes or b"{}")
    if int(outer.get("statusCode", 500)) != 200:
        raise RuntimeError(payload_bytes.decode("utf-8", errors="replace"))
    body = json.loads(outer["body"])
    print_summary(body)
    if args.json:
        print("")
        print(json.dumps(body, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
