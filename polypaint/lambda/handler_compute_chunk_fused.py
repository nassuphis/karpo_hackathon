import json
import os
import platform
import subprocess
import time
from concurrent.futures import ThreadPoolExecutor

import boto3

from roots_stream_upload import RootsStreamUploader
from shared import BUCKET, build_identity, is_enospc, ok_response, parse_body, report_status

s3 = boto3.client("s3")
SWEEP_COEFFGEN = os.path.join(os.path.dirname(__file__), "sweep_coeffgen")
SWEEP_MT = os.path.join(os.path.dirname(__file__), "sweep_mt")
SWEEP_CM = os.path.join(os.path.dirname(__file__), "sweep_cm")
STAGE_META_PREFIX = "pp"


def handler(event, context):
    params = parse_body(event)
    return handle_fused_chunk(params)


def _require_int(params, key, *, minimum=None):
    if key not in params:
        raise RuntimeError(f"fused compute chunk requires {key}")
    value = params.get(key)
    try:
        n = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"fused compute chunk requires integer {key}, got {value!r}")
    if minimum is not None and n < minimum:
        raise RuntimeError(f"fused compute chunk requires {key} >= {minimum}, got {n}")
    return n


def _require_str(params, key):
    if key not in params:
        raise RuntimeError(f"fused compute chunk requires {key}")
    value = str(params.get(key) or "").strip()
    if not value:
        raise RuntimeError(f"fused compute chunk requires non-empty {key}")
    return value


def handle_fused_chunk(params):
    t_handler = time.time()   # post-mortem F13: complete handler wall
    job_id = _require_str(params, "job_id")
    chunk_idx = _require_int(params, "chunk_idx", minimum=0)
    step_start = _require_int(params, "step_start", minimum=0)
    step_count = _require_int(params, "step_count", minimum=1)
    n = _require_int(params, "N", minimum=1)
    times = _require_int(params, "times", minimum=1) if "times" in params else 1
    n_coeffs = _require_int(params, "n_coeffs", minimum=1)
    degree = _require_int(params, "degree", minimum=1)
    fused_threads = _require_int(params, "fused_threads", minimum=1) if "fused_threads" in params else 4
    solver_mode = str(params.get("solver_mode") or "aberth_mt").strip().lower() or "aberth_mt"
    if solver_mode not in {"aberth_mt", "companion_matrix"}:
        raise RuntimeError(f"fused compute solver_mode must be one of aberth_mt, companion_matrix; got {solver_mode!r}")
    task_id = str(params.get("task_id") or f"compute_fused_{chunk_idx}")
    function_name = _require_str(params, "function")
    param_transforms = params.get("param_transforms") or []
    param_program = params.get("param_program") or None
    coeff_transforms = params.get("coeff_transforms") or []
    coeff_program = params.get("coeff_program") or None
    cfpv = params.get("cfpv") or []

    params_key = _require_str(params, "params_key")
    coeffs_key = _require_str(params, "coeffs_key")
    bin_key = _require_str(params, "bin_key")

    params_expected = step_count * 16
    coeffs_expected = step_count * n_coeffs * 8
    roots_expected = step_count * degree * 8

    params_path = f"/tmp/fused_params_{chunk_idx}.bin"
    coeffs_path = f"/tmp/fused_coeffs_{chunk_idx}.bin"
    roots_path = f"/tmp/fused_roots_{chunk_idx}.bin"
    progress_path = roots_path + ".progress"

    # CR34 §12-1a: params/coeffs uploads run on background threads so solve
    # starts the moment coeffgen finishes; the futures are joined (and their
    # failures re-raised) before the task can report success.
    pre_uploads = ThreadPoolExecutor(max_workers=2)
    params_upload_future = None
    coeffs_upload_future = None

    progress = {
        "phase": "compute_chunk_fused",
        "chunk_idx": chunk_idx,
        "step_start": step_start,
        "step_count": step_count,
        "fused_threads": fused_threads,
        "execution_method": "fused_chunk_pipeline",
        "params_key": params_key,
        "coeffs_key": coeffs_key,
        "bin_key": bin_key,
    }

    try:
        report_status(job_id, task_id, "started", result_data=progress)

        reused_params = False
        reused_coeffs = False

        if _s3_size_matches(
            params_key,
            params_expected,
            expected_metadata=_stage_metadata(
                stage="params",
                step_start=step_start,
                step_count=step_count,
            ),
        ):
            _download_file(params_key, params_path)
            reused_params = True
            param_meta = {"threads": fused_threads}
            param_gen_us = 0
            upload_params_us = 0
        else:
            param_meta = _run_param_gen_local(
                output_path=params_path,
                n=n,
                times=times,
                step_start=step_start,
                step_count=step_count,
                param_transforms=param_transforms,
                param_program=param_program,
                fused_threads=fused_threads,
            )
            param_gen_us = int(param_meta.get("elapsed_us", 0) or 0)
            params_upload_future = pre_uploads.submit(
                _timed_upload,
                params_path,
                params_key,
                _stage_metadata(
                    stage="params",
                    step_start=step_start,
                    step_count=step_count,
                ),
            )

        params_size = os.path.getsize(params_path)
        if params_size != params_expected:
            raise RuntimeError(
                f"fused param_gen size mismatch for chunk {chunk_idx}: expected {params_expected}, got {params_size}"
            )

        if _s3_size_matches(
            coeffs_key,
            coeffs_expected,
            expected_metadata=_stage_metadata(
                stage="coeffs",
                step_start=step_start,
                step_count=step_count,
                n_coeffs=n_coeffs,
                degree=degree,
            ),
        ):
            _download_file(coeffs_key, coeffs_path)
            reused_coeffs = True
            coeff_meta = {
                "degree": degree,
                "n_coeffs": n_coeffs,
                "data_bytes": coeffs_expected,
                "threads": fused_threads,
            }
            coeffgen_us = 0
            upload_coeffs_us = 0
        else:
            coeff_t0 = time.time()
            coeff_meta = _run_coeffgen_local(
                output_path=coeffs_path,
                function_name=function_name,
                coeff_transforms=coeff_transforms,
                coeff_program=coeff_program,
                cfpv=cfpv,
                params_path=params_path,
                n=n,
                source_step_start=step_start,
                step_count=step_count,
                fused_threads=fused_threads,
            )
            coeffgen_us = int((time.time() - coeff_t0) * 1e6)
            coeffs_upload_future = pre_uploads.submit(
                _timed_upload,
                coeffs_path,
                coeffs_key,
                _stage_metadata(
                    stage="coeffs",
                    step_start=step_start,
                    step_count=step_count,
                    n_coeffs=n_coeffs,
                    degree=degree,
                ),
            )

        coeffs_size = os.path.getsize(coeffs_path)
        if coeffs_size != coeffs_expected:
            raise RuntimeError(
                f"fused coeffgen size mismatch for chunk {chunk_idx}: expected {coeffs_expected}, got {coeffs_size}"
            )

        # params_path/coeffs_path stay on disk until their background uploads
        # are joined; the finally block removes every tmp file.

        # CR34 §12-1b: the roots bin uploads as S3 multipart parts WHILE the
        # solver writes it — sweep_mt flushes completed slices and publishes
        # durable watermarks in the progress sidecar — so after solve only
        # the tail parts and the multipart complete remain on the critical
        # path. Any streaming failure falls back to the plain serial upload.
        streamer = RootsStreamUploader(
            s3, BUCKET, bin_key, roots_path, progress_path, roots_expected)
        streamer.start()
        solve_t0 = time.time()
        try:
            solve_meta = _run_solve_local(
                output_path=roots_path,
                coeffs_path=coeffs_path,
                solver_mode=solver_mode,
                n_coeffs=n_coeffs,
                n_steps=step_count,
                fused_threads=fused_threads,
                progress_path=progress_path if solver_mode == "aberth_mt" else None,
            )
            solve_us = int((time.time() - solve_t0) * 1e6)

            roots_size = os.path.getsize(roots_path)
            if roots_size != roots_expected:
                raise RuntimeError(
                    f"fused solve size mismatch for chunk {chunk_idx}: expected {roots_expected}, got {roots_size}"
                )
        except BaseException:
            streamer.abort()
            raise
        tail_t0 = time.time()
        stream_ok = streamer.finish()
        upload_roots_us = None
        if not stream_ok:
            put_t0 = time.time()
            _upload_file(roots_path, bin_key)
            upload_roots_us = int((time.time() - put_t0) * 1e6)
        upload_roots_tail_us = int((time.time() - tail_t0) * 1e6)

        # Join the pre-solve uploads: their failures surface HERE, before the
        # task can report success. The wait is the only critical-path cost
        # they retain (expected ~0 — solve far exceeds the upload spans).
        wait_t0 = time.time()
        if params_upload_future is not None:
            upload_params_us = params_upload_future.result()
        if coeffs_upload_future is not None:
            upload_coeffs_us = coeffs_upload_future.result()
        pre_solve_upload_wait_us = int((time.time() - wait_t0) * 1e6)

        result_data = {
            "chunk_idx": chunk_idx,
            "stripe_idx": chunk_idx,
            "s3_key": bin_key,
            "bin_size": roots_size,
            "compute_us": solve_us,
            "n_t": int(solve_meta["n_t"]),
            "degree": int(solve_meta["degree"]),
            "avg_iterations": float(solve_meta.get("avg_iterations", 0) or 0),
            "params_key": params_key,
            "params_size": int(params_size),
            "params_step_start": 0,
            "params_step_count": int(step_count),
            "coeffs_key": coeffs_key,
            "coeffs_size": int(coeffs_size),
            "param_gen_us": int(param_gen_us),
            "coeffgen_us": int(coeffgen_us),
            "solve_us": int(solve_us),
            "upload_params_us": 0 if reused_params else int(upload_params_us),
            "upload_coeffs_us": int(upload_coeffs_us),
            # CR34 §12-1 truthful names: upload_params/coeffs_us are now
            # SPANS that overlap the solve, not serial critical-path stages;
            # the serial remainder is the join wait + the roots tail.
            # upload_roots_us keeps its original serial-PUT meaning and is
            # emitted only on the fallback path (added below).
            "pre_solve_upload_wait_us": int(pre_solve_upload_wait_us),
            "upload_roots_tail_us": int(upload_roots_tail_us),
            "upload_roots_span_us": int(streamer.span_us) if stream_ok else int(upload_roots_tail_us),
            "roots_parts_during_solve": int(streamer.parts_during_solve),
            "roots_upload_fallback": 0 if stream_ok else 1,
            "param_gen_threads": int(fused_threads),
            "coeffgen_threads": int(coeff_meta.get("threads", fused_threads) or fused_threads),
            "fused_threads": int(fused_threads),
            "execution_method": "fused_chunk_pipeline",
            "reused_params": int(reused_params),
            "reused_coeffs": int(reused_coeffs),
            # CR33 telemetry: one structured stage summary per chunk (never
            # per row) — the fields code-review-33 asked production to
            # report so future optimization is ranked against real programs.
            "stage_telemetry": {
                "param_scheduler": str(param_meta.get("param_scheduler") or param_meta.get("scheduler") or ""),
                "param_native_us": int(param_meta.get("native_elapsed_us", 0) or 0),
                "param_tokens": int(param_meta.get("param_program_tokens", 0) or 0),
                "param_legacy_static": int(param_meta.get("legacy_static_tokens", 0) or 0),
                "param_legacy_dynamic": int(param_meta.get("legacy_dynamic_tokens", 0) or 0),
                "param_legacy_prepared": int(param_meta.get("legacy_prepared_tokens", 0) or 0),
                "online_cpus": int(param_meta.get("online_cpus", 0) or 0),
                "coeff_native_us": int(coeff_meta.get("native_elapsed_us", 0) or 0),
                "coeff_tokens": int(coeff_meta.get("coeff_program_tokens", 0) or 0),
                "coeff_tok_typed_scalar": int(coeff_meta.get("tok_typed_scalar", 0) or 0),
                "coeff_tok_typed_vector": int(coeff_meta.get("tok_typed_vector", 0) or 0),
                "coeff_tok_selector": int(coeff_meta.get("tok_selector", 0) or 0),
                "coeff_tok_native": int(coeff_meta.get("tok_native", 0) or 0),
                "coeff_fused_regions": int(coeff_meta.get("fused_regions", 0) or 0),
                "coeff_fused_tokens": int(coeff_meta.get("fused_tokens", 0) or 0),
                "roots_size": int(roots_size),
                # post-mortem F13: native solve elapsed separately from the
                # outer solve_us wall, plus a complete handler wall
                "solve_native_us": int(solve_meta.get("elapsed_us", 0) or 0),
                "handler_wall_us": int((time.time() - t_handler) * 1e6),
                "lambda_memory_mb": int(os.environ.get("AWS_LAMBDA_FUNCTION_MEMORY_SIZE", 0) or 0),
                "arch": platform.machine(),
                **build_identity(),
            },
        }
        if upload_roots_us is not None:
            result_data["upload_roots_us"] = int(upload_roots_us)
        if "skipped_overflow" in solve_meta:
            result_data["skipped_overflow"] = int(solve_meta.get("skipped_overflow", 0) or 0)
        report_status(job_id, task_id, "done", result_data=result_data)
        return ok_response(result_data)
    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        if is_enospc(e):
            raise RuntimeError(f"fused compute chunk {chunk_idx} ran out of /tmp: {e}") from e
        raise
    finally:
        # Background uploads must be joined before their source files go
        # away (this shutdown only waits; failures were surfaced by the
        # .result() calls on the success path).
        pre_uploads.shutdown(wait=True)
        for path in (params_path, coeffs_path, roots_path, progress_path):
            try:
                os.remove(path)
            except OSError:
                pass


def _run_param_gen_local(*, output_path, n, times, step_start, step_count, param_transforms, fused_threads, param_program=None):
    spec = {
        "mode": "param_gen",
        "n1": n,
        "n2": n,
        "times": times,
        "param_transforms": list(param_transforms or []),
        "step_start": step_start,
        "step_count": step_count,
        "n_threads": fused_threads,
    }
    if param_program:
        spec["param_program"] = param_program
    t0 = time.time()
    # CR33 F1: pass the REAL output path (like _run_coeffgen_local). The "-"
    # argument forced the native binary onto the ordered-ring stdout scheduler
    # (its only cue is the literal argument), silently bypassing the static
    # pwrite scheduler for every multithreaded fused chunk: measured +14-39%
    # at 2-4 workers and pathological at 8, byte-identical output. In file
    # mode the run metadata JSON arrives on stdout; stderr carries errors.
    result = subprocess.run(
        [SWEEP_COEFFGEN, output_path],
        input=json.dumps(spec),
        capture_output=True,
        text=True,
        timeout=840,
    )
    if result.returncode != 0:
        raise RuntimeError(f"fused param_gen failed: {result.stderr.strip()}")
    meta = json.loads(result.stdout.strip())
    # CR33 telemetry: native elapsed_us stays visible next to the wall time —
    # their difference is process startup + JSON plumbing, one of the fields
    # the review asked production to report.
    meta["native_elapsed_us"] = int(meta.get("elapsed_us", 0) or 0)
    meta["param_scheduler"] = meta.get("scheduler") or ("static_file" if fused_threads > 1 else "serial")
    meta["elapsed_us"] = int((time.time() - t0) * 1e6)
    return meta


def _run_coeffgen_local(*, output_path, function_name, coeff_transforms, cfpv, params_path, n, source_step_start, step_count, fused_threads, coeff_program=None):
    spec = {
        "mode": "coeffgen_chunked",
        "function": function_name,
        "coeff_transforms": list(coeff_transforms or []),
        "params_file": params_path,
        "step_start": 0,
        "source_step_start": int(source_step_start),
        "source_n1": int(n),
        "source_n2": int(n),
        "step_count": step_count,
        "n_threads": fused_threads,
    }
    if coeff_program:
        spec["coeff_program"] = coeff_program
    if cfpv:
        spec["cfpv"] = list(cfpv)
    t0 = time.time()
    result = subprocess.run(
        [SWEEP_COEFFGEN, output_path],
        input=json.dumps(spec),
        capture_output=True,
        text=True,
        timeout=840,
    )
    if result.returncode != 0:
        raise RuntimeError(f"fused coeffgen failed: {result.stderr.strip()}")
    meta = json.loads(result.stdout)
    meta["native_elapsed_us"] = int(meta.get("elapsed_us", 0) or 0)
    meta["wall_elapsed_us"] = int((time.time() - t0) * 1e6)
    return meta


def _timed_upload(local_path, key, metadata=None):
    """Upload wrapper for background futures: returns the upload span in µs
    so the joined future carries its own honest timing."""
    t0 = time.time()
    _upload_file(local_path, key, metadata=metadata)
    return int((time.time() - t0) * 1e6)


def _run_solve_local(*, output_path, coeffs_path, solver_mode, n_coeffs, n_steps, fused_threads, progress_path=None):
    if solver_mode == "companion_matrix":
        spec = {
            "mode": "solve_cm",
            "coeffs_file": coeffs_path,
            "n_coeffs": n_coeffs,
            "n_steps": n_steps,
        }
        binary = SWEEP_CM
    elif solver_mode == "aberth_mt":
        spec = {
            "mode": "solve_mt",
            "coeffs_file": coeffs_path,
            "n_coeffs": n_coeffs,
            "n2": n_steps,
            "i1_start": 0,
            "i1_end": 1,
            "match_roots": False,
            "n_threads": fused_threads,
        }
        if progress_path:
            # sweep_mt publishes durable flush watermarks here so the
            # streaming uploader can ship parts while the solve runs
            spec["progress_file"] = progress_path
        binary = SWEEP_MT
    result = subprocess.run(
        [binary, output_path],
        input=json.dumps(spec),
        capture_output=True,
        text=True,
        timeout=840,
    )
    if result.returncode != 0:
        raise RuntimeError(f"fused solve failed: {result.stderr.strip()}")
    return json.loads(result.stdout)


def _stage_metadata(*, stage, step_start, step_count, n_coeffs=None, degree=None):
    metadata = {
        f"{STAGE_META_PREFIX}-stage": str(stage),
        f"{STAGE_META_PREFIX}-step-start": str(int(step_start)),
        f"{STAGE_META_PREFIX}-step-count": str(int(step_count)),
    }
    if n_coeffs is not None:
        metadata[f"{STAGE_META_PREFIX}-n-coeffs"] = str(int(n_coeffs))
    if degree is not None:
        metadata[f"{STAGE_META_PREFIX}-degree"] = str(int(degree))
    return metadata


def _upload_file(local_path, key, metadata=None):
    with open(local_path, "rb") as fh:
        extra_args = {"Metadata": dict(metadata or {})} if metadata else None
        if extra_args:
            s3.upload_fileobj(fh, BUCKET, key, ExtraArgs=extra_args)
        else:
            s3.upload_fileobj(fh, BUCKET, key)


def _download_file(key, local_path):
    with open(local_path, "wb") as fh:
        s3.download_fileobj(BUCKET, key, fh)


def _s3_size_matches(key, expected_size, expected_metadata=None):
    try:
        head = s3.head_object(Bucket=BUCKET, Key=key)
    except Exception:
        return False
    if int(head.get("ContentLength", -1)) != int(expected_size):
        return False
    if expected_metadata:
        got_meta = {str(k).lower(): str(v) for k, v in (head.get("Metadata") or {}).items()}
        for key_name, expected_value in expected_metadata.items():
            if got_meta.get(str(key_name).lower()) != str(expected_value):
                return False
    return True
