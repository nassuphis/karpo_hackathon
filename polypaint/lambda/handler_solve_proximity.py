"""
Solve score Lambda — async 3-phase prepass for solve-level color modes.

Supports multiple metrics: proximity, crowding, spread, anisotropy, area.
Phases: clip, hist, merge.

Dispatched async via dispatch handler, reports status to DynamoDB.
"""
import json
import os
import subprocess
import time

import boto3

from shared import BUCKET, parse_body, ok_response, report_status

s3 = boto3.client("s3")
BINARY = os.path.join(os.path.dirname(__file__), "solve_proximity_stats")

VALID_METRICS = {"proximity", "crowding", "spread", "anisotropy", "area",
                 "clusteriness", "shelliness", "outlierness", "nn_variation", "real_axis_proximity"}

_TMP_INPUT = "/tmp/solve_prox_input.bin"
_TMP_XFORMS = "/tmp/solve_prox_root_xforms.json"
_TMP_CLIP = "/tmp/solve_prox_clip.json"
_TMP_HIST = "/tmp/solve_prox_hist.json"


def _cleanup_tmp():
    for p in [_TMP_INPUT, _TMP_XFORMS, _TMP_CLIP, _TMP_HIST]:
        try:
            os.remove(p)
        except OSError:
            pass


def _download(key, path):
    obj = s3.get_object(Bucket=BUCKET, Key=key)
    with open(path, "wb") as f:
        for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
            f.write(chunk)
    return os.path.getsize(path)


def _write_xforms(root_transforms):
    if root_transforms:
        with open(_TMP_XFORMS, "w") as f:
            json.dump(root_transforms, f)
        return _TMP_XFORMS
    return None


def _validate_metric(metric):
    if metric not in VALID_METRICS:
        raise RuntimeError(f"Invalid metric: {metric} (valid: {', '.join(sorted(VALID_METRICS))})")


def handler(event, context):
    params = parse_body(event)
    phase = params["phase"]
    if phase == "clip":
        return handle_clip(params)
    elif phase == "hist":
        return handle_hist(params)
    elif phase == "merge":
        return handle_merge(params)
    elif phase == "summary":
        return handle_summary(params)
    else:
        raise RuntimeError(f"Unknown phase: {phase}")


def _validate_quantile(q):
    """Validate solve_score_quantile is in [0.001, 0.05]."""
    try:
        q = float(q)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_quantile must be numeric, got {q!r}")
    if not (0.001 <= q <= 0.05):
        raise RuntimeError(f"solve_score_quantile must be in [0.001, 0.05], got {q}")
    return q


def handle_clip(params):
    job_id = params["job_id"]
    task_id = params["task_id"]
    degree = params["degree"]
    metric = params.get("metric", "proximity")
    _validate_metric(metric)
    solve_score_quantile = _validate_quantile(params.get("solve_score_quantile", 0.001))
    lores_bin_key = params["lores_bin_key"]
    root_transforms = params.get("root_transforms")
    out_key = params["out_key"]
    progress = {"phase": "clip", "metric": metric, "source_key": lores_bin_key}

    try:
        _cleanup_tmp()
        report_status(job_id, task_id, "started", result_data=progress)

        t0 = time.time()
        size = _download(lores_bin_key, _TMP_INPUT)
        dl_ms = int((time.time() - t0) * 1000)
        progress["dl_ms"] = dl_ms
        progress["source_size"] = size

        report_status(job_id, task_id, "bin_downloaded", result_data=progress)

        quantile_lo = solve_score_quantile
        quantile_hi = 1.0 - solve_score_quantile
        cmd = [BINARY, _TMP_INPUT, "--mode=clip", f"--degree={degree}",
               f"--metric={metric}", f"--quantile_lo={quantile_lo}", f"--quantile_hi={quantile_hi}"]
        xf_path = _write_xforms(root_transforms)
        if xf_path:
            cmd.append(f"--root_xforms={xf_path}")

        t1 = time.time()
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
        compute_ms = int((time.time() - t1) * 1000)
        if result.returncode != 0:
            raise RuntimeError(f"solve_proximity_stats clip failed: {result.stderr.strip()}")

        clip_data = json.loads(result.stdout)
        progress["compute_ms"] = compute_ms
        progress["n_solves"] = clip_data["n_solves"]
        progress["clip_lo"] = clip_data["clip_lo"]
        progress["clip_hi"] = clip_data["clip_hi"]

        report_status(job_id, task_id, "computed", result_data=progress)

        artifact = {
            "family": "solve_score",
            "version": 1,
            "job_id": job_id,
            "metric": metric,
            "clip_quantile": solve_score_quantile,
            "clip_lo": clip_data["clip_lo"],
            "clip_hi": clip_data["clip_hi"],
            "n_solves": clip_data["n_solves"],
            "degree": degree,
            "lores_bin_key": lores_bin_key,
            "root_transforms": root_transforms or [],
        }

        s3.put_object(Bucket=BUCKET, Key=out_key,
                      Body=json.dumps(artifact),
                      ContentType="application/json")

        progress["out_key"] = out_key
        report_status(job_id, task_id, "done", result_data=progress)
        return ok_response(progress)

    except Exception as e:
        progress["error"] = str(e)
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        _cleanup_tmp()


def handle_hist(params):
    job_id = params["job_id"]
    task_id = params["task_id"]
    chunk_idx = params.get("chunk_idx", params.get("stripe_idx"))
    if chunk_idx is None:
        raise RuntimeError("hist requires chunk_idx")
    metric = params.get("metric", "proximity")
    _validate_metric(metric)
    solve_score_quantile = _validate_quantile(params.get("solve_score_quantile", 0.001))
    bin_key = params["bin_key"]
    degree = params["degree"]
    clip_key = params["clip_key"]
    hist_bins = params.get("hist_bins", 100)
    root_transforms = params.get("root_transforms")
    out_key = params["out_key"]
    progress = {"phase": "hist", "metric": metric, "chunk_idx": chunk_idx}

    try:
        _cleanup_tmp()
        report_status(job_id, task_id, "started", result_data=progress)

        t0 = time.time()
        size = _download(bin_key, _TMP_INPUT)
        progress["source_size"] = size

        clip_obj = s3.get_object(Bucket=BUCKET, Key=clip_key)
        clip_data = json.loads(clip_obj["Body"].read())
        dl_ms = int((time.time() - t0) * 1000)
        progress["dl_ms"] = dl_ms

        report_status(job_id, task_id, "bin_downloaded", result_data=progress)

        cmd = [BINARY, _TMP_INPUT, "--mode=hist", f"--degree={degree}",
               f"--metric={metric}",
               f"--clip_lo={clip_data['clip_lo']}", f"--clip_hi={clip_data['clip_hi']}",
               f"--hist_bins={hist_bins}"]
        xf_path = _write_xforms(root_transforms)
        if xf_path:
            cmd.append(f"--root_xforms={xf_path}")

        t1 = time.time()
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
        compute_ms = int((time.time() - t1) * 1000)
        if result.returncode != 0:
            raise RuntimeError(f"solve_proximity_stats hist failed: {result.stderr.strip()}")

        hist_data = json.loads(result.stdout)
        progress["compute_ms"] = compute_ms
        progress["n_solves"] = hist_data["n_solves"]

        artifact = {
            "family": "solve_score",
            "version": 1,
            "job_id": job_id,
            "metric": metric,
            "clip_quantile": solve_score_quantile,
            "chunk_idx": chunk_idx,
            "hist_bins": hist_bins,
            "clip_lo": clip_data["clip_lo"],
            "clip_hi": clip_data["clip_hi"],
            "n_solves": hist_data["n_solves"],
            "hist": hist_data["hist"],
        }

        s3.put_object(Bucket=BUCKET, Key=out_key,
                      Body=json.dumps(artifact),
                      ContentType="application/json")

        progress["out_key"] = out_key
        report_status(job_id, task_id, "done", result_data=progress)
        return ok_response(progress)

    except Exception as e:
        progress["error"] = str(e)
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        _cleanup_tmp()


def handle_merge(params):
    job_id = params["job_id"]
    task_id = params["task_id"]
    metric = params.get("metric", "proximity")
    _validate_metric(metric)
    solve_score_quantile = _validate_quantile(params.get("solve_score_quantile", 0.001))
    n_chunks = params.get("n_chunks", params.get("n_stripes"))
    if n_chunks is None:
        raise RuntimeError("merge requires n_chunks")
    hist_prefix = params["hist_prefix"]
    clip_key = params["clip_key"]
    out_key = params["out_key"]
    progress = {"phase": "merge", "metric": metric, "n_chunks": n_chunks}

    try:
        _cleanup_tmp()
        report_status(job_id, task_id, "started", result_data=progress)

        # Load and validate clip data
        clip_obj = s3.get_object(Bucket=BUCKET, Key=clip_key)
        clip_data = json.loads(clip_obj["Body"].read())
        if clip_data.get("family") == "solve_score" and clip_data.get("metric") != metric:
            raise RuntimeError(f"Clip metric mismatch: expected {metric}, got {clip_data.get('metric')}")
        if clip_data.get("family") == "solve_score" and clip_data.get("clip_quantile") != solve_score_quantile:
            raise RuntimeError(f"Clip quantile mismatch: expected {solve_score_quantile}, got {clip_data.get('clip_quantile')}")
        hist_bins = 100

        total_hist = [0] * hist_bins
        total_solves = 0
        for c in range(n_chunks):
            key = f"{hist_prefix}chunk_{c}_hist.json"
            try:
                try:
                    obj = s3.get_object(Bucket=BUCKET, Key=key)
                except s3.exceptions.NoSuchKey:
                    # Backward compatibility for old stripe-named hist artifacts
                    legacy_key = f"{hist_prefix}stripe_{c}_hist.json"
                    obj = s3.get_object(Bucket=BUCKET, Key=legacy_key)
                    key = legacy_key
                data = json.loads(obj["Body"].read())
                # Validate metric match
                if data.get("family") == "solve_score" and data.get("metric") != metric:
                    raise RuntimeError(f"Chunk {c} metric mismatch: expected {metric}, got {data.get('metric')}")
                if data.get("family") == "solve_score" and data.get("clip_quantile") != solve_score_quantile:
                    raise RuntimeError(f"Chunk {c} quantile mismatch: expected {solve_score_quantile}, got {data.get('clip_quantile')}")
                chunk_hist = data["hist"]
                if len(chunk_hist) != hist_bins:
                    raise RuntimeError(f"Chunk {c} histogram has {len(chunk_hist)} bins, expected {hist_bins}")
                for i in range(hist_bins):
                    total_hist[i] += chunk_hist[i]
                total_solves += data["n_solves"]
            except s3.exceptions.NoSuchKey:
                raise RuntimeError(f"Missing histogram: {key}")

        progress["n_solves_total"] = total_solves
        report_status(job_id, task_id, "merged", result_data=progress)

        # Derive 10 equal-density bins
        final_bins = 10
        total_count = sum(total_hist)
        cuts_norm = []
        for k in range(1, final_bins):
            target = total_count * k / final_bins
            cum = 0
            cut = 1.0
            for i in range(hist_bins):
                cum_before = cum
                cum += total_hist[i]
                if cum >= target:
                    bucket_count = total_hist[i]
                    if bucket_count > 0:
                        frac = (target - cum_before) / bucket_count
                    else:
                        frac = 1.0
                    cut = (i + frac) / hist_bins
                    break
            cut = max(0.0, min(1.0, cut))
            if cuts_norm and cut < cuts_norm[-1]:
                cut = cuts_norm[-1]
            cuts_norm.append(cut)

        artifact = {
            "family": "solve_score",
            "version": 1,
            "job_id": job_id,
            "metric": metric,
            "clip_quantile": solve_score_quantile,
            "hist_bins": hist_bins,
            "final_bins": final_bins,
            "clip_lo": clip_data["clip_lo"],
            "clip_hi": clip_data["clip_hi"],
            "cuts_norm": cuts_norm,
            "n_solves_total": total_solves,
            "root_transforms": clip_data.get("root_transforms", []),
        }

        s3.put_object(Bucket=BUCKET, Key=out_key,
                      Body=json.dumps(artifact),
                      ContentType="application/json")

        progress["out_key"] = out_key
        progress["cuts_norm"] = cuts_norm
        report_status(job_id, task_id, "done", result_data=progress)
        return ok_response(progress)

    except Exception as e:
        progress["error"] = str(e)
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        _cleanup_tmp()


def handle_summary(params):
    """Synchronous debug summary — no side effects, no DDB writes, no S3 artifacts."""
    degree = params["degree"]
    metric = params.get("metric", "proximity")
    _validate_metric(metric)
    solve_score_quantile = _validate_quantile(params.get("solve_score_quantile", 0.001))
    lores_bin_key = params["lores_bin_key"]
    root_transforms = params.get("root_transforms")

    try:
        _cleanup_tmp()

        t0 = time.time()
        size = _download(lores_bin_key, _TMP_INPUT)
        dl_ms = int((time.time() - t0) * 1000)

        quantile_lo = solve_score_quantile
        quantile_hi = 1.0 - solve_score_quantile
        cmd = [BINARY, _TMP_INPUT, "--mode=summary", f"--degree={degree}",
               f"--metric={metric}",
               f"--quantile_lo={quantile_lo}", f"--quantile_hi={quantile_hi}"]
        xf_path = _write_xforms(root_transforms)
        if xf_path:
            cmd.append(f"--root_xforms={xf_path}")

        t1 = time.time()
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
        compute_ms = int((time.time() - t1) * 1000)
        print(f"solve_prox_summary rc={result.returncode} stdout={repr(result.stdout[:300])} stderr={repr(result.stderr[:500])}")
        if result.returncode != 0:
            raise RuntimeError(f"solve_proximity_stats summary failed (rc={result.returncode}): {result.stderr.strip()}")

        summary = json.loads(result.stdout)
        summary["dl_ms"] = dl_ms
        summary["compute_ms"] = compute_ms
        summary["source_size"] = size

        return ok_response(summary)

    finally:
        _cleanup_tmp()
