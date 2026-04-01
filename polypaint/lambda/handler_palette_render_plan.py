"""
Palette render plan Lambda — computes the workflow plan for exact full-resolution palette artifacts.

Loads calc metadata, validates palette-tab params, derives exact pass-0 chunk spans,
and produces compact chunk arrays and output keys for Step Functions.
"""
import hashlib
import json
import time

import boto3

from palette_names import VALID_PALETTE_NAMES
from shared import BUCKET, parse_body, ok_response

s3 = boto3.client("s3")

MAX_PLAN_BYTES = 200 * 1024  # fail fast before 256KB Step Functions limit
VALID_METRICS = {
    "proximity", "crowding", "spread", "anisotropy", "area",
    "clusteriness", "shelliness", "outlierness", "nn_variation", "real_axis_proximity",
}
def _palette_variant_id(metric, palette, q, root_transforms):
    q_label = f"{q * 100:.1f}".replace(".", "p")
    rt_json = json.dumps(root_transforms or [], separators=(",", ":"))
    rt_hash = hashlib.sha1(rt_json.encode("utf-8")).hexdigest()[:8]
    return f"pal_{int(time.time() * 1000)}_{metric}_{palette}_q{q_label}_rt{rt_hash}"


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    run_id = params["run_id"]
    task_id = params["task_id"]
    pp = dict(params.get("params", {}))

    metric = pp.get("metric", "proximity")
    palette = pp.get("palette", "inferno")
    root_transforms = pp.get("root_transforms", [])
    try:
        q = float(pp.get("solve_score_quantile", 0.001))
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_quantile must be numeric, got {pp.get('solve_score_quantile')!r}")
    if metric not in VALID_METRICS:
        raise RuntimeError(f"Invalid metric: {metric}")
    if palette not in VALID_PALETTE_NAMES:
        raise RuntimeError(f"Invalid palette: {palette}")
    if not (0.001 <= q <= 0.05):
        raise RuntimeError(f"solve_score_quantile must be in [0.001, 0.05], got {q}")

    calc = _load_calc(job_id)
    degree = calc.get("degree")
    full_n = calc.get("N", calc.get("n1"))
    times = int(calc.get("times", 1) or 1)
    lores_bin_key = (calc.get("lores") or {}).get("bin_key", "")
    if not degree or not full_n or not lores_bin_key:
        raise RuntimeError(f"calc.json missing degree, N, or lores.bin_key for {job_id}")

    chunks = list(calc.get("chunks", calc.get("stripes", [])) or [])
    if not chunks:
        raise RuntimeError(f"calc.json missing chunk metadata for {job_id}")
    chunks.sort(key=lambda c: c.get("idx", c.get("chunk_idx", c.get("stripe_idx", 0))))

    pass0_steps = full_n * full_n
    record_bytes = degree * 2 * 4
    chunk_items = []
    palette_items = []
    step_start = 0
    for raw in chunks:
        idx = raw.get("idx", raw.get("chunk_idx", raw.get("stripe_idx")))
        bin_key = raw.get("bin_key", raw.get("s3_key"))
        step_count = raw.get("step_count", raw.get("n_t"))
        if step_count is None:
            bin_size = raw.get("bin_size")
            if not bin_size:
                raise RuntimeError(f"Chunk {idx} missing step_count/n_t/bin_size")
            step_count = int(bin_size // record_bytes)
        step_count = int(step_count)
        if idx is None or not bin_key or step_count <= 0:
            raise RuntimeError(f"Invalid chunk metadata: idx={idx} bin_key={bin_key!r} step_count={step_count}")

        item = {
            "chunk_idx": int(idx),
            "bin_key": bin_key,
            "step_start": step_start,
            "step_count": step_count,
        }
        chunk_items.append(item)

        pass0_count = max(0, min(step_count, pass0_steps - step_start))
        if pass0_count > 0:
            palette_items.append({
                "chunk_idx": int(idx),
                "bin_key": bin_key,
                "step_start": step_start,
                "step_count": pass0_count,
            })
        step_start += step_count

    if step_start < pass0_steps:
        raise RuntimeError(f"Full solve metadata too small: only {step_start} solves, expected at least {pass0_steps}")

    palette_id = _palette_variant_id(metric, palette, q, root_transforms)
    prefix = f"renders/{job_id}/palettes/{palette_id}/"
    solve_prefix = prefix + "solve_score/"
    chunks_prefix = prefix + "chunks/"

    plan = {
        "job_id": job_id,
        "run_id": run_id,
        "task_id": task_id,
        "mode": "palette",
        "params": {
            "metric": metric,
            "palette": palette,
            "solve_score_quantile": q,
            "root_transforms": root_transforms,
        },
        "palette_id": palette_id,
        "prefix": prefix,
        "calc": {
            "degree": degree,
            "N": full_n,
            "times": times,
            "n_chunks": len(chunk_items),
            "n_palette_chunks": len(palette_items),
            "pass0_steps": pass0_steps,
            "lores_bin_key": lores_bin_key,
        },
        "chunk_items": chunk_items,
        "palette_items": palette_items,
        "solve_score": {
            "metric": metric,
            "quantile": q,
            "clip_key": prefix + f"solve_score/{metric}_clip.json",
            "hist_prefix": solve_prefix,
            "bins_key": prefix + f"solve_score/{metric}_bins.json",
        },
        "outputs": {
            "image_key": prefix + "image.jpeg",
            "preview_key": prefix + "preview.png",
            "score_key": prefix + f"score_{metric}.bin",
            "palette_bins_key": prefix + "palette_bins.bin",
            "meta_key": prefix + "meta.json",
            "chunks_prefix": chunks_prefix,
        },
    }

    plan_json = json.dumps(plan)
    if len(plan_json) > MAX_PLAN_BYTES:
        raise RuntimeError(
            f"Plan too large: {len(plan_json)} bytes > {MAX_PLAN_BYTES} limit. "
            f"Reduce chunk count."
        )
    return ok_response(plan)


def _load_calc(job_id):
    obj = s3.get_object(Bucket=BUCKET, Key=f"renders/{job_id}/calc.json")
    return json.loads(obj["Body"].read())
