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

VALID_METRICS = {"proximity", "crowding", "spread", "anisotropy", "area"}

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
    else:
        raise RuntimeError(f"Unknown phase: {phase}")


def handle_clip(params):
    job_id = params["job_id"]
    task_id = params["task_id"]
    degree = params["degree"]
    metric = params.get("metric", "proximity")
    _validate_metric(metric)
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

        cmd = [BINARY, _TMP_INPUT, "--mode=clip", f"--degree={degree}",
               f"--metric={metric}", "--quantile_lo=0.001", "--quantile_hi=0.999"]
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
    stripe_idx = params["stripe_idx"]
    metric = params.get("metric", "proximity")
    _validate_metric(metric)
    bin_key = params["bin_key"]
    degree = params["degree"]
    clip_key = params["clip_key"]
    hist_bins = params.get("hist_bins", 100)
    root_transforms = params.get("root_transforms")
    out_key = params["out_key"]
    progress = {"phase": "hist", "metric": metric, "stripe_idx": stripe_idx}

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
            "stripe_idx": stripe_idx,
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
    n_stripes = params["n_stripes"]
    hist_prefix = params["hist_prefix"]
    clip_key = params["clip_key"]
    out_key = params["out_key"]
    progress = {"phase": "merge", "metric": metric, "n_stripes": n_stripes}

    try:
        _cleanup_tmp()
        report_status(job_id, task_id, "started", result_data=progress)

        # Load and validate clip data
        clip_obj = s3.get_object(Bucket=BUCKET, Key=clip_key)
        clip_data = json.loads(clip_obj["Body"].read())
        if clip_data.get("family") == "solve_score" and clip_data.get("metric") != metric:
            raise RuntimeError(f"Clip metric mismatch: expected {metric}, got {clip_data.get('metric')}")
        hist_bins = 100

        total_hist = [0] * hist_bins
        total_solves = 0
        for s in range(n_stripes):
            key = f"{hist_prefix}stripe_{s}_hist.json"
            try:
                obj = s3.get_object(Bucket=BUCKET, Key=key)
                data = json.loads(obj["Body"].read())
                # Validate metric match
                if data.get("family") == "solve_score" and data.get("metric") != metric:
                    raise RuntimeError(f"Stripe {s} metric mismatch: expected {metric}, got {data.get('metric')}")
                stripe_hist = data["hist"]
                if len(stripe_hist) != hist_bins:
                    raise RuntimeError(f"Stripe {s} histogram has {len(stripe_hist)} bins, expected {hist_bins}")
                for i in range(hist_bins):
                    total_hist[i] += stripe_hist[i]
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
