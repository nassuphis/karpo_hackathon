"""
Coeffgen Lambda handler — generates coefficient vectors.

Three modes, routed by 'phase' parameter:

  phase=param_gen:
    Generate and upload the parameter stream.

  phase=coeffgen_chunked:
    Reads a slice of params.bin from S3, generates coefficients for that chunk.

  phase=legacy_coeffgen:
    Legacy chunkless coeffgen path for old callers only.

  phase=degree_probe:
    Tiny coeffgen probe that returns degree/n_coeffs and optional fused sizing.
"""
import json
import os
import subprocess
import time

import boto3

from compute_fused import (
    PROBE_N,
    PROBE_SIGNATURE_SPEC_VERSION,
    build_probe_signature,
    estimate_fused_chunking,
    validate_fused_threads,
)
from coeff_program_chain import compile_coeff_program_chain
from pipeline_programs import (
    coeff_source_text_for_run,
    coeff_transforms_to_program_chain,
    param_source_text_for_run,
    param_transforms_to_program_chain,
    parse_coeff_source_for_run,
    parse_param_source_for_run,
    pipeline_mode_from_params,
)
from param_program_chain import compile_param_program_chain
from program_compile_helpers import (
    coeff_program_macro_resolver,
    compiled_coeff_program_payload as _compiled_coeff_program_payload,
    compiled_param_program_payload as _compiled_param_program_payload,
    param_program_macro_resolver,
)
from shared import BUCKET, attach_contract_warnings, contract_param, parse_body, ok_response, report_status

s3 = boto3.client("s3")
SWEEP = os.path.join(os.path.dirname(__file__), "sweep_coeffgen")


def _param_program_macro_resolver():
    return param_program_macro_resolver(s3_client=s3)


def _coeff_program_macro_resolver():
    return coeff_program_macro_resolver(s3_client=s3)


def _resolve_coeff_program(params, coeff_transforms, *, pipeline_mode=None):
    coeff_program = params.get("coeff_program") or None
    allow_program_fields = pipeline_mode != "chain"
    if not allow_program_fields:
        coeff_program = None
    # Probe payloads are mode-filtered upstream, so the precedence helper
    # runs without the mode gate; compile failures carry line/column.
    source_text = coeff_source_text_for_run(params, None) if allow_program_fields and coeff_program is None else None
    if source_text is not None:
        parsed = parse_coeff_source_for_run(source_text)
        coeff_program_chain = parsed["chain"]
    else:
        coeff_program_chain = params.get("coeff_program_chain") if allow_program_fields else None
    if coeff_program is None and not coeff_program_chain and coeff_transforms:
        coeff_program_chain = coeff_transforms_to_program_chain(coeff_transforms)
    if coeff_program is None and coeff_program_chain:
        if not isinstance(coeff_program_chain, list):
            raise RuntimeError("coeff_program_chain must be an array")
        compiled = compile_coeff_program_chain(
            coeff_program_chain,
            macro_resolver=_coeff_program_macro_resolver(),
        )
        coeff_transforms = []
        coeff_program = _compiled_coeff_program_payload(compiled)
    return coeff_transforms, coeff_program


def _resolve_param_program(params, param_transforms, *, pipeline_mode=None):
    param_program = params.get("param_program") or None
    allow_program_fields = pipeline_mode != "chain"
    if not allow_program_fields:
        param_program = None
    source_text = param_source_text_for_run(params, None) if allow_program_fields and param_program is None else None
    if source_text is not None:
        parsed = parse_param_source_for_run(source_text)
        param_program_chain = parsed["chain"]
    else:
        param_program_chain = params.get("param_program_chain") if allow_program_fields else None
    if param_program is None and not param_program_chain and param_transforms:
        param_program_chain = param_transforms_to_program_chain(param_transforms)
    if param_program is None and param_program_chain:
        if not isinstance(param_program_chain, list):
            raise RuntimeError("param_program_chain must be an array")
        compiled = compile_param_program_chain(
            param_program_chain,
            macro_resolver=_param_program_macro_resolver(),
        )
        param_transforms = []
        param_program = _compiled_param_program_payload(compiled)
    return param_transforms, param_program


# _pipeline_mode_from_params moved to pipeline_programs (shared — CR14).


def handler(event, context):
    params = parse_body(event)
    phase = str(params.get("phase", "") or "").strip()

    if phase == "param_gen":
        return handle_param_gen(params)
    if phase == "coeffgen_chunked":
        return handle_coeffgen_chunked(params)
    if phase == "legacy_coeffgen":
        return handle_legacy_coeffgen(params)
    if phase == "degree_probe":
        return handle_degree_probe(params)
    raise ValueError(
        "Unknown coeffgen phase. Expected one of: param_gen, coeffgen_chunked, legacy_coeffgen, degree_probe"
    )


def handle_param_gen(params):
    """Generate the full unrolled parameter stream and upload to S3.

    Streams binary output from the sweep binary directly to S3 via multipart
    upload — no /tmp materialization. Supports arbitrarily large params.bin.
    """
    job_id = params["job_id"]
    task_id = params.get("task_id", "param_gen")
    contract_warnings = []

    try:
        grid_n = params.get("N", params.get("n1"))
        times = contract_param(params, "times", 1, contract_warnings)
        params_key = params.get("params_key", f"renders/{job_id}/params.bin")
        t0 = time.time()

        # gridN: override for dither scaling (lores uses full N, not loresN)
        grid_n_override = params.get("gridN")
        raw_threads = params.get("n_threads")
        spec = {
            "mode": "param_gen",
            "n1": grid_n,
            "n2": grid_n,
            "times": times,
            "param_transforms": contract_param(params, "param_transforms", [], contract_warnings),
        }
        pipeline_mode = pipeline_mode_from_params(params)
        spec["param_transforms"], param_program = _resolve_param_program(
            params, spec["param_transforms"], pipeline_mode=pipeline_mode
        )
        if param_program:
            spec["param_program"] = param_program
        if grid_n_override:
            spec["gridN"] = grid_n_override
        if raw_threads not in (None, ""):
            try:
                n_threads = int(raw_threads)
            except (TypeError, ValueError):
                raise RuntimeError(f"n_threads must be an integer, got {raw_threads!r}")
            if n_threads < 1:
                raise RuntimeError(f"n_threads must be >= 1, got {n_threads}")
            spec["n_threads"] = n_threads
        threads = int(spec.get("n_threads", 1) or 1)
        total_steps = int(grid_n) * int(grid_n) * int(times)
        raw_step_start = params.get("step_start")
        raw_step_count = params.get("step_count")
        step_start = 0
        step_count = total_steps
        if raw_step_start not in (None, ""):
            try:
                step_start = int(raw_step_start)
            except (TypeError, ValueError):
                raise RuntimeError(f"step_start must be an integer, got {raw_step_start!r}")
            spec["step_start"] = step_start
        if raw_step_count not in (None, ""):
            try:
                step_count = int(raw_step_count)
            except (TypeError, ValueError):
                raise RuntimeError(f"step_count must be an integer, got {raw_step_count!r}")
            spec["step_count"] = step_count
        elif raw_step_start not in (None, ""):
            step_count = total_steps - step_start
            spec["step_count"] = step_count
        if step_start < 0 or step_count < 1 or step_start > total_steps or step_count > total_steps - step_start:
            raise RuntimeError(
                f"invalid param_gen range: step_start={step_start} step_count={step_count} total_steps={total_steps}"
            )
        expected_steps = step_count
        expected_bytes = expected_steps * 16
        phase_meta = {
            "phase": "param_gen",
            "phase_label": "Param gen",
            "params_key": params_key,
            "n_steps": expected_steps,
            "total_steps": total_steps,
            "step_start": step_start,
            "step_count": step_count,
            "data_bytes": expected_bytes,
            "threads": threads,
            "elapsed_us": 0,
            "uploaded_bytes": 0,
            "uploaded_steps_est": 0,
            "progress": 0.0,
        }
        report_status(job_id, task_id, "started", result_data=attach_contract_warnings(dict(phase_meta), contract_warnings))

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
        last_progress_report_at = t0

        def _report_progress():
            elapsed_us = int((time.time() - t0) * 1e6)
            uploaded_steps_est = int(total_bytes // 16)
            progress = float(total_bytes / expected_bytes) if expected_bytes > 0 else 1.0
            progress = max(0.0, min(1.0, progress))
            progress_data = dict(phase_meta)
            progress_data.update({
                "elapsed_us": elapsed_us,
                "uploaded_bytes": total_bytes,
                "uploaded_steps_est": uploaded_steps_est,
                "progress": progress,
            })
            report_status(job_id, task_id, "started", result_data=attach_contract_warnings(progress_data, contract_warnings))

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
                    now = time.time()
                    if now - last_progress_report_at >= 2.0:
                        _report_progress()
                        last_progress_report_at = now

            # Upload remaining buffer
            if buf:
                part_num += 1
                resp = s3.upload_part(
                    Bucket=BUCKET, Key=params_key,
                    UploadId=upload_id, PartNumber=part_num,
                    Body=buf)
                parts.append({"ETag": resp["ETag"], "PartNumber": part_num})
                total_bytes += len(buf)
                now = time.time()
                if now - last_progress_report_at >= 2.0:
                    _report_progress()
                    last_progress_report_at = now

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
                f"params output size mismatch for {params_key}: expected {meta['data_bytes']}, uploaded {total_bytes}")

        result_data = {
            "phase": "param_gen",
            "phase_label": "Param gen",
            "params_key": params_key,
            "n_steps": meta["n_steps"],
            "total_steps": int(meta.get("total_steps", total_steps) or total_steps),
            "step_start": int(meta.get("step_start", step_start) or 0),
            "step_count": int(meta.get("step_count", step_count) or step_count),
            "data_bytes": meta["data_bytes"],
            "threads": int(meta.get("threads", 1) or 1),
            "elapsed_us": int((time.time() - t0) * 1e6),
        }
        report_status(job_id, task_id, "done", result_data=attach_contract_warnings(result_data, contract_warnings))
        return ok_response({"job_id": job_id, **result_data})

    except Exception as e:
        error_data = dict(locals().get("phase_meta", {}) or {})
        error_data.setdefault("phase", "param_gen")
        error_data.setdefault("phase_label", "Param gen")
        error_data.setdefault("job_id", job_id)
        error_data.setdefault("params_key", params.get("params_key", f"renders/{job_id}/params.bin"))
        report_status(job_id, task_id, "error", str(e), result_data=attach_contract_warnings(error_data, contract_warnings))
        raise


def handle_coeffgen_chunked(params):
    """Read a slice of params.bin from S3, generate coefficients for that chunk."""
    job_id = params["job_id"]
    chunk_idx = params["chunk_idx"]
    step_start = params["step_start"]
    step_count = params["step_count"]
    params_key = params["params_key"]
    params_step_start = params.get("params_step_start", step_start)
    params_step_count = params.get("params_step_count", step_count)
    task_id = params.get("task_id", f"coeffgen_{chunk_idx}")
    contract_warnings = []
    raw_threads = params.get("n_threads")
    phase_meta = {
        "phase": "coeffgen_chunked",
        "phase_label": "Coeffgen chunk",
        "job_id": job_id,
        "chunk_idx": chunk_idx,
        "params_key": params_key,
        "step_start": step_start,
        "step_count": step_count,
        "params_step_start": params_step_start,
        "params_step_count": params_step_count,
    }

    try:
        step_start = int(step_start)
        step_count = int(step_count)
        params_step_start = int(params_step_start)
        params_step_count = int(params_step_count)
        phase_meta.update({
            "step_start": step_start,
            "step_count": step_count,
            "params_step_start": params_step_start,
            "params_step_count": params_step_count,
        })
        if step_start < 0 or step_count < 1:
            raise RuntimeError(f"step_start/step_count must be valid, got {step_start}/{step_count}")
        if params_step_start < 0 or params_step_count < 1:
            raise RuntimeError(f"params_step_start/params_step_count must be valid, got {params_step_start}/{params_step_count}")
        if params_step_count != step_count:
            raise RuntimeError(
                f"params_step_count must match step_count for coeffgen_chunked, got {params_step_count}/{step_count}"
            )
        coeffgen_threads = 1
        if raw_threads not in (None, ""):
            try:
                coeffgen_threads = int(raw_threads)
            except (TypeError, ValueError):
                raise RuntimeError(f"n_threads must be an integer, got {raw_threads!r}")
            if coeffgen_threads < 1:
                raise RuntimeError(f"n_threads must be >= 1, got {coeffgen_threads}")

        report_status(job_id, task_id, "started", result_data=attach_contract_warnings({
            "phase": "coeffgen_chunked",
            "chunk_idx": chunk_idx,
            "threads": coeffgen_threads,
            "params_key": params_key,
            "params_step_start": params_step_start,
            "params_step_count": params_step_count,
        }, contract_warnings))

        # Range-read our slice of params.bin from S3
        record_bytes = 16  # 4 × float32
        byte_start = params_step_start * record_bytes
        byte_end = (params_step_start + params_step_count) * record_bytes - 1
        t0 = time.time()

        resp = s3.get_object(
            Bucket=BUCKET, Key=params_key,
            Range=f"bytes={byte_start}-{byte_end}")
        params_data = resp["Body"].read()

        params_file = "/tmp/params_chunk.bin"
        with open(params_file, "wb") as f:
            f.write(params_data)

        bin_path = f"/tmp/coeffs_chunk_{chunk_idx}.bin"
        grid_n = int(params.get("N") or params.get("n1") or 0)

        spec = {
            "mode": "coeffgen_chunked",
            "function": params["function"],
            "coeff_transforms": contract_param(params, "coeff_transforms", [], contract_warnings),
            "params_file": params_file,
            "step_start": 0,  # file contains only our slice
            "source_step_start": params_step_start,
            "step_count": step_count,
            "n_threads": coeffgen_threads,
        }
        if grid_n > 0:
            spec["source_n1"] = grid_n
            spec["source_n2"] = grid_n
        pipeline_mode = pipeline_mode_from_params(params)
        spec["coeff_transforms"], coeff_program = _resolve_coeff_program(
            params, spec["coeff_transforms"], pipeline_mode=pipeline_mode
        )
        if coeff_program:
            spec["coeff_program"] = coeff_program
        if params.get("cfpv"):
            spec["cfpv"] = params["cfpv"]

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

        result_data = {
            "job_id": job_id,
            "chunk_idx": chunk_idx,
            "coeffs_key": coeffs_key,
            "params_key": params_key,
            "params_step_start": params_step_start,
            "params_step_count": params_step_count,
            "coeffs_size": meta["data_bytes"],
            "n_coeffs": meta["n_coeffs"],
            "degree": meta["degree"],
            "threads": int(meta.get("threads", coeffgen_threads) or coeffgen_threads),
            "coeff_program_tokens": int(meta.get("coeff_program_tokens", 0) or 0),
            "elapsed_us": int((time.time() - t0) * 1e6),
        }
        report_status(job_id, task_id, "done", result_data=attach_contract_warnings(result_data, contract_warnings))
        return ok_response({
            "chunk_idx": chunk_idx,
            "coeffs_size": meta["data_bytes"],
            "n_coeffs": meta["n_coeffs"],
            "degree": meta["degree"],
            "threads": int(meta.get("threads", coeffgen_threads) or coeffgen_threads),
            "coeff_program_tokens": int(meta.get("coeff_program_tokens", 0) or 0),
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=attach_contract_warnings(dict(phase_meta), contract_warnings))
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
    task_id = params.get("task_id", f"coeffgen_{stripe_idx}")
    phase_meta = {
        "phase": "legacy_coeffgen",
        "phase_label": "Legacy coeffgen",
        "job_id": job_id,
        "stripe_idx": stripe_idx,
        "grid_n": grid_n,
        "row_start": i1_start,
        "row_end": i1_end,
    }

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
        pipeline_mode = pipeline_mode_from_params(params)
        spec["param_transforms"], param_program = _resolve_param_program(
            params, spec["param_transforms"], pipeline_mode=pipeline_mode
        )
        if param_program:
            spec["param_program"] = param_program
        spec["coeff_transforms"], coeff_program = _resolve_coeff_program(
            params, spec["coeff_transforms"], pipeline_mode=pipeline_mode
        )
        if coeff_program:
            spec["coeff_program"] = coeff_program
        if params.get("cfpv"):
            spec["cfpv"] = params["cfpv"]

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

        result_data = {
            "job_id": job_id,
            "stripe_idx": stripe_idx,
            "coeffs_key": coeffs_key,
            "coeffs_size": meta["data_bytes"],
            "n_coeffs": meta["n_coeffs"],
            "degree": meta["degree"],
            "elapsed_us": elapsed_us,
        }
        report_status(job_id, task_id, "done", result_data=result_data)
        return ok_response(result_data)

    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=dict(phase_meta))
        raise


def handle_degree_probe(params):
    pipeline_mode = pipeline_mode_from_params(params)
    function_name = str(params.get("function", "") or "").strip()
    if not function_name:
        raise RuntimeError("function is required")
    coeff_transforms = params.get("coeff_transforms")
    if coeff_transforms is None:
        coeff_transforms = []
    if not isinstance(coeff_transforms, list):
        raise RuntimeError("coeff_transforms must be an array")
    coeff_program = None
    if pipeline_mode == "program":
        coeff_transforms = []
    coeff_transforms, coeff_program = _resolve_coeff_program(
        params, coeff_transforms, pipeline_mode=pipeline_mode
    )
    param_transforms = params.get("param_transforms")
    if param_transforms is None:
        param_transforms = []
    if not isinstance(param_transforms, list):
        raise RuntimeError("param_transforms must be an array")
    if pipeline_mode == "program":
        param_transforms = []
    param_transforms, param_program = _resolve_param_program(
        params, param_transforms, pipeline_mode=pipeline_mode
    )
    cfpv = params.get("cfpv")
    if cfpv in (None, ""):
        cfpv = []
    if not isinstance(cfpv, list):
        raise RuntimeError("cfpv must be an array")

    probe_n = int(params.get("probe_n") or PROBE_N)
    if probe_n < 2:
        raise RuntimeError(f"probe_n must be >= 2, got {probe_n}")
    rows = [(0, 1), (probe_n - 1, probe_n)]
    samples = []
    total_coeffgen_us = 0
    t0 = time.time()
    contract_warnings = []
    try:
        for idx, (i1_start, i1_end) in enumerate(rows):
            bin_path = f"/tmp/degree_probe_{idx}.bin"
            spec = {
                "mode": "coeffgen",
                "param_transforms": param_transforms,
                "function": function_name,
                "coeff_transforms": coeff_transforms,
                "n1": probe_n,
                "n2": probe_n,
                "i1_start": i1_start,
                "i1_end": i1_end,
                "times": 1,
            }
            if param_program:
                spec["param_program"] = param_program
            if coeff_program:
                spec["coeff_program"] = coeff_program
            if cfpv:
                spec["cfpv"] = cfpv
            sample_t0 = time.time()
            result = subprocess.run(
                [SWEEP, bin_path],
                input=json.dumps(spec),
                capture_output=True,
                text=True,
                timeout=120,
            )
            if result.returncode != 0:
                raise RuntimeError(f"degree_probe failed: {result.stderr.strip()}")
            meta = json.loads(result.stdout)
            elapsed_us = int((time.time() - sample_t0) * 1e6)
            sample = {
                "degree": int(meta["degree"]),
                "n_coeffs": int(meta["n_coeffs"]),
                "coeffs_size": int(meta["data_bytes"]),
                "elapsed_us": elapsed_us,
                "row_start": int(i1_start),
                "row_end": int(i1_end),
            }
            samples.append(sample)
            total_coeffgen_us += elapsed_us
            try:
                os.remove(bin_path)
            except OSError:
                pass

        degree = samples[0]["degree"]
        n_coeffs = samples[0]["n_coeffs"]
        stable = all(s["degree"] == degree and s["n_coeffs"] == n_coeffs for s in samples[1:])
        probe_signature = build_probe_signature(
            function_name=function_name,
            param_transforms=param_transforms,
            coeff_transforms=coeff_transforms,
            cfpv=cfpv,
            param_program=param_program,
            coeff_program=coeff_program,
        )
        body = {
            "probe_n": probe_n,
            "probe_step_count": probe_n * probe_n,
            "probe_signature": probe_signature,
            "probe_signature_spec_version": PROBE_SIGNATURE_SPEC_VERSION,
            "probe_stable": bool(stable),
            "degree": int(degree),
            "n_coeffs": int(n_coeffs),
            "coeffgen_us": int(total_coeffgen_us),
            "samples": samples,
            "elapsed_us": int((time.time() - t0) * 1e6),
        }
        if not stable:
            return ok_response(attach_contract_warnings(body, contract_warnings))

        if params.get("N") is not None and params.get("n_chunks") is not None:
            solver_mode = str(params.get("solver_mode") or "aberth_mt").strip().lower() or "aberth_mt"
            fused_threads = validate_fused_threads(params.get("fused_threads", 4))
            estimate = estimate_fused_chunking(
                n=int(params.get("N")),
                times=int(params.get("times", 1) or 1),
                requested_chunks=int(params.get("n_chunks")),
                degree=degree,
                n_coeffs=n_coeffs,
                fused_threads=fused_threads,
                solver_mode=solver_mode,
                auto_hires_chunks=bool(params.get("auto_hires_chunks")),
            )
            body["fused_estimate"] = estimate
        return ok_response(attach_contract_warnings(body, contract_warnings))
    finally:
        for idx in range(len(rows)):
            try:
                os.remove(f"/tmp/degree_probe_{idx}.bin")
            except OSError:
                pass
