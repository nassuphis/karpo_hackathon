"""
Coeffgen Lambda handler — generates coefficient vectors.

Three modes, routed by 'phase' parameter:

  phase=param_gen (or absent with params_key absent):
    Old-style stripe coeffgen or new param_gen stage.

  phase=coeffgen_chunked:
    Reads a slice of params.bin from S3, generates coefficients for that chunk.

  Default (no phase, no params_key): legacy stripe coeffgen.
"""
import json
import os
import subprocess
import time

import boto3

from shared import BUCKET, parse_body, ok_response, report_status

s3 = boto3.client("s3")
SWEEP = os.path.join(os.path.dirname(__file__), "sweep")


def handler(event, context):
    params = parse_body(event)
    phase = params.get("phase", "")

    if phase == "param_gen":
        return handle_param_gen(params)
    elif phase == "coeffgen_chunked":
        return handle_coeffgen_chunked(params)
    else:
        return handle_legacy_coeffgen(params)


def handle_param_gen(params):
    """Generate the full unrolled parameter stream and upload to S3.

    Streams binary output from the sweep binary directly to S3 via multipart
    upload — no /tmp materialization. Supports arbitrarily large params.bin.
    """
    job_id = params["job_id"]
    task_id = params.get("task_id", "param_gen")

    try:
        report_status(job_id, task_id, "started")

        grid_n = params.get("N", params.get("n1"))
        times = params.get("times", 1)
        params_key = params.get("params_key", f"renders/{job_id}/params.bin")
        t0 = time.time()

        spec = {
            "mode": "param_gen",
            "n1": grid_n,
            "n2": grid_n,
            "times": times,
            "param_transforms": params.get("param_transforms", []),
        }

        # Launch sweep with "-" as output path → binary goes to stdout
        proc = subprocess.Popen(
            [SWEEP, "-"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        proc.stdin.write(json.dumps(spec).encode())
        proc.stdin.close()

        # Stream stdout to S3 via multipart upload (no /tmp needed)
        PART_SIZE = 8 * 1024 * 1024  # 8 MB per part (S3 minimum is 5 MB)
        mpu = s3.create_multipart_upload(
            Bucket=BUCKET, Key=params_key,
            ContentType="application/octet-stream")
        upload_id = mpu["UploadId"]

        parts = []
        part_num = 0
        total_bytes = 0
        buf = b""

        try:
            while True:
                chunk = proc.stdout.read(1024 * 1024)  # 1 MB reads
                if not chunk:
                    break
                buf += chunk
                while len(buf) >= PART_SIZE:
                    part_num += 1
                    part_data = buf[:PART_SIZE]
                    buf = buf[PART_SIZE:]
                    resp = s3.upload_part(
                        Bucket=BUCKET, Key=params_key,
                        UploadId=upload_id, PartNumber=part_num,
                        Body=part_data)
                    parts.append({"ETag": resp["ETag"], "PartNumber": part_num})
                    total_bytes += len(part_data)

            # Upload remaining buffer
            if buf:
                part_num += 1
                resp = s3.upload_part(
                    Bucket=BUCKET, Key=params_key,
                    UploadId=upload_id, PartNumber=part_num,
                    Body=buf)
                parts.append({"ETag": resp["ETag"], "PartNumber": part_num})
                total_bytes += len(buf)

            s3.complete_multipart_upload(
                Bucket=BUCKET, Key=params_key,
                UploadId=upload_id,
                MultipartUpload={"Parts": parts})

        except Exception:
            s3.abort_multipart_upload(
                Bucket=BUCKET, Key=params_key, UploadId=upload_id)
            raise

        proc.wait(timeout=10)
        stderr_out = proc.stderr.read().decode()
        if proc.returncode != 0:
            raise RuntimeError(f"param_gen failed: {stderr_out.strip()}")

        # Metadata is on stderr (stdout was binary data)
        meta = json.loads(stderr_out.strip())

        if total_bytes != meta["data_bytes"]:
            raise RuntimeError(
                f"params.bin size mismatch: expected {meta['data_bytes']}, uploaded {total_bytes}")

        result_data = {
            "params_key": params_key,
            "n_steps": meta["n_steps"],
            "data_bytes": meta["data_bytes"],
            "elapsed_us": int((time.time() - t0) * 1e6),
        }
        report_status(job_id, task_id, "done", result_data=result_data)
        return ok_response({"job_id": job_id, **result_data})

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        raise


def handle_coeffgen_chunked(params):
    """Read a slice of params.bin from S3, generate coefficients for that chunk."""
    job_id = params["job_id"]
    chunk_idx = params["chunk_idx"]
    step_start = params["step_start"]
    step_count = params["step_count"]
    params_key = params["params_key"]
    task_id = f"coeffgen_{chunk_idx}"

    try:
        report_status(job_id, task_id, "started")

        # Range-read our slice of params.bin from S3
        record_bytes = 16  # 4 × float32
        byte_start = step_start * record_bytes
        byte_end = (step_start + step_count) * record_bytes - 1
        t0 = time.time()

        resp = s3.get_object(
            Bucket=BUCKET, Key=params_key,
            Range=f"bytes={byte_start}-{byte_end}")
        params_data = resp["Body"].read()

        params_file = "/tmp/params_chunk.bin"
        with open(params_file, "wb") as f:
            f.write(params_data)

        bin_path = f"/tmp/coeffs_chunk_{chunk_idx}.bin"

        spec = {
            "mode": "coeffgen_chunked",
            "function": params["function"],
            "coeff_transforms": params.get("coeff_transforms", []),
            "params_file": params_file,
            "step_start": 0,  # file contains only our slice
            "step_count": step_count,
        }

        result = subprocess.run(
            [SWEEP, bin_path],
            input=json.dumps(spec),
            capture_output=True, text=True, timeout=840
        )
        if result.returncode != 0:
            raise RuntimeError(f"coeffgen_chunked failed: {result.stderr.strip()}")

        meta = json.loads(result.stdout)

        # Verify file size
        actual_size = os.path.getsize(bin_path)
        if actual_size != meta["data_bytes"]:
            raise RuntimeError(
                f"coeffs chunk size mismatch: expected {meta['data_bytes']}, got {actual_size}")

        coeffs_key = params.get("s3_key",
                                f"renders/{job_id}/coeffs_{chunk_idx:04d}.bin")
        with open(bin_path, "rb") as f:
            s3.put_object(Bucket=BUCKET, Key=coeffs_key,
                          Body=f, ContentType="application/octet-stream")

        for p in [params_file, bin_path]:
            try:
                os.remove(p)
            except OSError:
                pass

        report_status(job_id, task_id, "done")
        return ok_response({
            "job_id": job_id,
            "chunk_idx": chunk_idx,
            "coeffs_key": coeffs_key,
            "coeffs_size": meta["data_bytes"],
            "n_coeffs": meta["n_coeffs"],
            "degree": meta["degree"],
            "elapsed_us": int((time.time() - t0) * 1e6),
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        for p in ["/tmp/params_chunk.bin", f"/tmp/coeffs_chunk_{chunk_idx}.bin"]:
            try:
                os.remove(p)
            except OSError:
                pass
        raise


def handle_legacy_coeffgen(params):
    """Legacy stripe-based coeffgen (backward compatible)."""
    job_id = params["job_id"]
    stripe_idx = params["stripe_idx"]
    grid_n = params.get("N", params.get("n1"))
    i1_start = params.get("row_start", params.get("i1_start"))
    i1_end = params.get("row_end", params.get("i1_end"))
    task_id = f"coeffgen_{stripe_idx}"

    try:
        report_status(job_id, task_id, "started")

        bin_path = f"/tmp/coeffs_{stripe_idx}.bin"
        t0 = time.time()

        spec = {
            "mode": "coeffgen",
            "param_transforms": params.get("param_transforms", []),
            "function": params["function"],
            "coeff_transforms": params.get("coeff_transforms", []),
            "n1": grid_n,
            "n2": grid_n,
            "i1_start": i1_start,
            "i1_end": i1_end,
            "times": params.get("times", 1),
        }

        result = subprocess.run(
            [SWEEP, bin_path],
            input=json.dumps(spec),
            capture_output=True, text=True, timeout=840
        )
        if result.returncode != 0:
            raise RuntimeError(f"coeffgen failed: {result.stderr.strip()}")

        meta = json.loads(result.stdout)
        elapsed_us = int((time.time() - t0) * 1e6)

        actual_size = os.path.getsize(bin_path)
        expected_size = meta["data_bytes"]
        if actual_size != expected_size:
            try:
                os.remove(bin_path)
            except OSError:
                pass
            raise RuntimeError(
                f"coeffs.bin size mismatch: expected {expected_size}, got {actual_size}")

        coeffs_key = params.get("s3_key", f"renders/{job_id}/coeffs_{stripe_idx:04d}.bin")
        with open(bin_path, "rb") as f:
            s3.put_object(Bucket=BUCKET, Key=coeffs_key,
                          Body=f, ContentType="application/octet-stream")

        try:
            os.remove(bin_path)
        except OSError:
            pass

        report_status(job_id, task_id, "done")
        return ok_response({
            "job_id": job_id,
            "stripe_idx": stripe_idx,
            "coeffs_key": coeffs_key,
            "coeffs_size": meta["data_bytes"],
            "n_coeffs": meta["n_coeffs"],
            "degree": meta["degree"],
            "elapsed_us": elapsed_us,
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        raise
