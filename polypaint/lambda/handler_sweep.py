"""
Sweep Lambda handler — runs sweep subprocesses on row ranges.

Single route:
  POST /compute-only-stripe  — run sweep on a row range, upload .bin to S3
"""
import json
import multiprocessing
import os
import subprocess
import time

import boto3

from shared import BUCKET, parse_body, ok_response

s3 = boto3.client("s3")
SWEEP = os.path.join(os.path.dirname(__file__), "sweep")


def handler(event, context):
    params = parse_body(event)
    if "coeffs_key" in params:
        return handle_solve_from_coeffs(params)
    return handle_compute_only_stripe(event)


def handle_compute_only_stripe(event):
    """Per-stripe worker: compute roots via sweep, upload .bin to S3.
    Spawns multiple sweep subprocesses to use all available vCPUs.

    Optional param: s3_key — override default upload path (used for lores sweep).
    """
    params = parse_body(event)
    job_id = params["job_id"]
    stripe_idx = params["stripe_idx"]
    i1_start = params["i1_start"]
    i1_end = params["i1_end"]
    total_rows = i1_end - i1_start

    n_cpus = max(1, multiprocessing.cpu_count())
    n_procs = min(n_cpus, total_rows) if total_rows > 1 and n_cpus > 1 else 1

    t0 = time.time()

    if n_procs == 1:
        bin_path = "/tmp/stripe.bin"
        spec = {
            "mode": "grid",
            "function": params["function"],
            "n1": params["n1"], "n2": params["n2"],
            "i1_start": i1_start, "i1_end": i1_end,
            "match_roots": False,
        }
        result = subprocess.run(
            [SWEEP, bin_path],
            input=json.dumps(spec),
            capture_output=True, text=True, timeout=840
        )
        if result.returncode != 0:
            raise RuntimeError(f"sweep failed: {result.stderr.strip()}")
        compute_meta = json.loads(result.stdout)
    else:
        rows_per = total_rows // n_procs
        sub_ranges = []
        for c in range(n_procs):
            sub_start = i1_start + c * rows_per
            sub_end = i1_start + (c + 1) * rows_per if c < n_procs - 1 else i1_end
            if sub_start < sub_end:
                sub_ranges.append((c, sub_start, sub_end))

        procs = []
        for c, sub_start, sub_end in sub_ranges:
            sub_bin = f"/tmp/sub_{c}.bin"
            spec = {
                "mode": "grid",
                "function": params["function"],
                "n1": params["n1"], "n2": params["n2"],
                "i1_start": sub_start, "i1_end": sub_end,
                "match_roots": False,
            }
            proc = subprocess.Popen(
                [SWEEP, sub_bin],
                stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, text=True
            )
            proc.stdin.write(json.dumps(spec))
            proc.stdin.close()
            proc.stdin = None  # prevent communicate() from flushing closed fd
            procs.append((c, sub_bin, proc))

        sub_metas = []
        for c, sub_bin, proc in procs:
            stdout, stderr = proc.communicate(timeout=840)
            if proc.returncode != 0:
                raise RuntimeError(f"sweep sub-{c} failed: {stderr.strip()}")
            sub_metas.append(json.loads(stdout))

        bin_path = "/tmp/stripe.bin"
        with open(bin_path, "wb") as out:
            for c, sub_bin, _ in procs:
                with open(sub_bin, "rb") as f:
                    out.write(f.read())
                try:
                    os.remove(sub_bin)
                except OSError:
                    pass

        total_n_t = sum(m["n_t"] for m in sub_metas)
        total_weighted_iters = sum(
            m["avg_iterations"] * m["n_t"] for m in sub_metas)
        compute_meta = {
            "n_t": total_n_t,
            "degree": sub_metas[0]["degree"],
            "avg_iterations": (total_weighted_iters / total_n_t
                               if total_n_t > 0 else 0),
        }

    compute_us = int((time.time() - t0) * 1e6)

    # Upload .bin to S3 (use s3_key override if provided, e.g. for lores sweep)
    s3_key = params.get("s3_key", f"renders/{job_id}/stripe_{stripe_idx}.bin")
    with open(bin_path, "rb") as f:
        bin_data = f.read()
    s3.put_object(Bucket=BUCKET, Key=s3_key,
                  Body=bin_data, ContentType="application/octet-stream")

    try:
        os.remove(bin_path)
    except OSError:
        pass

    return ok_response({
        "stripe_idx": stripe_idx,
        "s3_key": s3_key,
        "bin_size": len(bin_data),
        "compute_us": compute_us,
        "n_t": compute_meta["n_t"],
        "degree": compute_meta["degree"],
        "avg_iterations": compute_meta["avg_iterations"],
        "n_procs": n_procs,
    })


def handle_solve_from_coeffs(params):
    """Solve roots from pre-computed coefficient file (coeffs.bin).
    Downloads the relevant byte range of coeffs.bin from S3,
    runs sweep in 'solve' mode, uploads root .bin to S3.
    """
    job_id = params["job_id"]
    stripe_idx = params["stripe_idx"]
    coeffs_key = params["coeffs_key"]
    n_coeffs = params["n_coeffs"]
    n1 = params["n1"]
    n2 = params["n2"]
    i1_start = params["i1_start"]
    i1_end = params["i1_end"]

    t0 = time.time()

    # S3 range read for this stripe's coefficients
    bytes_per_row = n2 * n_coeffs * 2 * 4  # n2 points × n_coeffs × (re,im) × float32
    start_byte = i1_start * bytes_per_row
    end_byte = i1_end * bytes_per_row - 1

    resp = s3.get_object(
        Bucket=BUCKET, Key=coeffs_key,
        Range=f"bytes={start_byte}-{end_byte}")
    coeffs_data = resp["Body"].read()

    coeffs_file = "/tmp/coeffs_stripe.bin"
    with open(coeffs_file, "wb") as f:
        f.write(coeffs_data)

    bin_path = "/tmp/stripe.bin"
    spec = {
        "mode": "solve",
        "coeffs_file": coeffs_file,
        "n_coeffs": n_coeffs,
        "n1": n1, "n2": n2,
        "i1_start": 0,  # offset already handled by range read
        "i1_end": i1_end - i1_start,
        "match_roots": False,
    }
    result = subprocess.run(
        [SWEEP, bin_path],
        input=json.dumps(spec),
        capture_output=True, text=True, timeout=840
    )
    if result.returncode != 0:
        raise RuntimeError(f"solve failed: {result.stderr.strip()}")
    compute_meta = json.loads(result.stdout)

    compute_us = int((time.time() - t0) * 1e6)

    s3_key = params.get("s3_key", f"renders/{job_id}/stripe_{stripe_idx}.bin")
    with open(bin_path, "rb") as f:
        bin_data = f.read()
    s3.put_object(Bucket=BUCKET, Key=s3_key,
                  Body=bin_data, ContentType="application/octet-stream")

    for p in [coeffs_file, bin_path]:
        try:
            os.remove(p)
        except OSError:
            pass

    return ok_response({
        "stripe_idx": stripe_idx,
        "s3_key": s3_key,
        "bin_size": len(bin_data),
        "compute_us": compute_us,
        "n_t": compute_meta["n_t"],
        "degree": compute_meta["degree"],
        "avg_iterations": compute_meta["avg_iterations"],
        "n_procs": 1,
    })
