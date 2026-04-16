import hashlib
import json
import math
import os


PROBE_N = 5
MAX_FUSED_THREADS = 64

FUSED_WORKER_MEMORY_MB_DEFAULT = 10240
FUSED_WORKER_TMP_MB_DEFAULT = 10240
FUSED_WORKER_TIMEOUT_SEC_DEFAULT = 900

FUSED_FIXED_OVERHEAD_BYTES = 256 * 1024 * 1024
FUSED_PER_THREAD_BYTES = 48 * 1024 * 1024
FUSED_TMP_FIXED_BYTES = 64 * 1024 * 1024
FUSED_MEMORY_BUDGET_RATIO = 0.72
FUSED_TMP_BUDGET_RATIO = 0.82
FUSED_TIMEOUT_BUDGET_RATIO = 0.80
UPLOAD_THROUGHPUT_BYTES_PER_SEC = 200 * 1024 * 1024


def execution_method_from_params(run_params):
    raw = str(run_params.get("execution_method") or "").strip().lower()
    if not raw:
        raw = "classic_chunk_pipeline"
    if raw not in ("classic_chunk_pipeline", "fused_chunk_pipeline"):
        raise RuntimeError(
            "execution_method must be one of classic_chunk_pipeline, fused_chunk_pipeline"
        )
    return raw


def build_probe_signature(*, function_name, param_transforms, coeff_transforms, cfpv):
    payload = {
        "function": str(function_name or "").strip(),
        "param_transforms": list(param_transforms or []),
        "coeff_transforms": list(coeff_transforms or []),
        "cfpv": list(cfpv or []),
    }
    packed = json.dumps(payload, sort_keys=True, separators=(",", ":"))
    return hashlib.sha1(packed.encode("utf-8")).hexdigest()


def fused_worker_memory_mb():
    return _read_positive_int_env("FUSED_WORKER_MEMORY_MB", FUSED_WORKER_MEMORY_MB_DEFAULT)


def fused_worker_tmp_mb():
    return _read_positive_int_env("FUSED_WORKER_TMP_MB", FUSED_WORKER_TMP_MB_DEFAULT)


def fused_worker_timeout_sec():
    return _read_positive_int_env("FUSED_WORKER_TIMEOUT_SEC", FUSED_WORKER_TIMEOUT_SEC_DEFAULT)


def validate_fused_threads(value):
    try:
        n = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"fused_threads must be an integer, got {value!r}")
    if n < 1 or n > MAX_FUSED_THREADS:
        raise RuntimeError(f"fused_threads must be between 1 and {MAX_FUSED_THREADS}, got {n}")
    return n


def build_chunk_items(*, job_id, run_id, total_steps, n_chunks):
    chunk_size = int(math.ceil(total_steps / n_chunks))
    items = []
    for chunk_idx in range(n_chunks):
        step_start = chunk_idx * chunk_size
        step_count = min(chunk_size, total_steps - step_start)
        if step_count <= 0:
            break
        items.append({
            "chunk_idx": chunk_idx,
            "step_start": step_start,
            "step_count": step_count,
            "params_key": f"renders/{job_id}/params_{chunk_idx:04d}.bin",
            "params_bin_size": int(step_count) * 16,
            "params_step_start": 0,
            "params_step_count": step_count,
            "paramgen_task_id": f"compute_{run_id}_param_gen_{chunk_idx}",
            "coeffs_key": f"renders/{job_id}/coeffs_{chunk_idx:04d}.bin",
            "coeffgen_task_id": f"compute_{run_id}_coeffgen_{chunk_idx}",
            "solve_task_id": f"compute_{run_id}_solve_{chunk_idx}",
            "fused_task_id": f"compute_{run_id}_fused_{chunk_idx}",
            "bin_key": f"renders/{job_id}/chunk_{chunk_idx}.bin",
        })
    return items


def estimate_fused_chunking(*, n, times, requested_chunks, degree, n_coeffs, fused_threads, solver_mode, auto_hires_chunks=False):
    total_steps = int(n) * int(n) * int(times)
    requested_chunks = max(1, min(int(requested_chunks), total_steps))
    degree = int(degree)
    n_coeffs = int(n_coeffs)
    fused_threads = validate_fused_threads(fused_threads)

    params_bytes_per_step = 16
    coeff_bytes_per_step = max(1, n_coeffs) * 8
    roots_bytes_per_step = max(1, degree) * 8

    scratch_step_bytes = _solver_scratch_step_bytes(solver_mode, degree)
    peak_memory_step_bytes = max(
        params_bytes_per_step * 2,
        params_bytes_per_step + coeff_bytes_per_step * 2 + roots_bytes_per_step,
        coeff_bytes_per_step + roots_bytes_per_step * 2 + scratch_step_bytes,
    )
    peak_tmp_step_bytes = max(
        params_bytes_per_step + coeff_bytes_per_step,
        coeff_bytes_per_step + roots_bytes_per_step,
    )

    memory_budget_bytes = int(fused_worker_memory_mb() * 1024 * 1024 * FUSED_MEMORY_BUDGET_RATIO)
    memory_budget_bytes -= FUSED_FIXED_OVERHEAD_BYTES + fused_threads * FUSED_PER_THREAD_BYTES
    if memory_budget_bytes <= 0:
        memory_budget_bytes = 1
    tmp_budget_bytes = int(fused_worker_tmp_mb() * 1024 * 1024 * FUSED_TMP_BUDGET_RATIO) - FUSED_TMP_FIXED_BYTES
    if tmp_budget_bytes <= 0:
        tmp_budget_bytes = 1

    memory_floor_chunks = max(1, int(math.ceil((total_steps * peak_memory_step_bytes) / memory_budget_bytes)))
    tmp_floor_chunks = max(1, int(math.ceil((total_steps * peak_tmp_step_bytes) / tmp_budget_bytes)))

    timeout_per_step_us = _param_gen_us_per_step() + _coeffgen_us_per_step(n_coeffs) + _solve_us_per_step(
        solver_mode=solver_mode,
        degree=degree,
        fused_threads=fused_threads,
    )
    upload_per_step_us = ((params_bytes_per_step + coeff_bytes_per_step + roots_bytes_per_step) / UPLOAD_THROUGHPUT_BYTES_PER_SEC) * 1e6
    timeout_budget_us = int(fused_worker_timeout_sec() * 1_000_000 * FUSED_TIMEOUT_BUDGET_RATIO)
    fixed_timeout_us = 2_500_000
    timeout_floor_chunks = max(
        1,
        int(math.ceil((total_steps * (timeout_per_step_us + upload_per_step_us) + fixed_timeout_us) / max(1, timeout_budget_us))),
    )

    min_safe_chunks = max(memory_floor_chunks, tmp_floor_chunks, timeout_floor_chunks)
    limiting_floor = "memory"
    if min_safe_chunks == tmp_floor_chunks and tmp_floor_chunks >= memory_floor_chunks and tmp_floor_chunks >= timeout_floor_chunks:
        limiting_floor = "/tmp"
    if min_safe_chunks == timeout_floor_chunks and timeout_floor_chunks >= memory_floor_chunks and timeout_floor_chunks >= tmp_floor_chunks:
        limiting_floor = "timeout"

    actual_chunks = min_safe_chunks if auto_hires_chunks else requested_chunks
    if actual_chunks < min_safe_chunks:
        actual_chunks = min_safe_chunks
    actual_chunks = max(1, min(actual_chunks, total_steps))
    chunk_step_count = int(math.ceil(total_steps / actual_chunks))
    params_bytes = chunk_step_count * params_bytes_per_step
    coeff_bytes = chunk_step_count * coeff_bytes_per_step
    roots_bytes = chunk_step_count * roots_bytes_per_step
    peak_estimated_bytes = (
        FUSED_FIXED_OVERHEAD_BYTES
        + fused_threads * FUSED_PER_THREAD_BYTES
        + chunk_step_count * peak_memory_step_bytes
    )
    tmp_peak_bytes = FUSED_TMP_FIXED_BYTES + chunk_step_count * peak_tmp_step_bytes
    estimated_timeout_us = int(
        fixed_timeout_us
        + chunk_step_count * (timeout_per_step_us + upload_per_step_us)
    )

    return {
        "requested_chunks": requested_chunks,
        "min_safe_chunks": int(min_safe_chunks),
        "actual_chunks": int(actual_chunks),
        "chunk_step_count": int(chunk_step_count),
        "params_bytes": int(params_bytes),
        "coeff_bytes": int(coeff_bytes),
        "roots_bytes": int(roots_bytes),
        "estimated_peak_bytes": int(peak_estimated_bytes),
        "estimated_tmp_peak_bytes": int(tmp_peak_bytes),
        "estimated_chunk_timeout_us": int(estimated_timeout_us),
        "safe_chunk_limit_reason": limiting_floor,
        "memory_floor_chunks": int(memory_floor_chunks),
        "tmp_floor_chunks": int(tmp_floor_chunks),
        "timeout_floor_chunks": int(timeout_floor_chunks),
        "fused_worker_memory_mb": fused_worker_memory_mb(),
        "fused_worker_tmp_mb": fused_worker_tmp_mb(),
        "fused_worker_timeout_sec": fused_worker_timeout_sec(),
        "fused_threads": int(fused_threads),
    }


def _solver_scratch_step_bytes(solver_mode, degree):
    base = max(1, int(degree)) * 8
    if solver_mode == "companion_matrix":
        return base * 18
    if solver_mode == "aberth_mt":
        return base * 10
    return base * 8


def _param_gen_us_per_step():
    return 0.8


def _coeffgen_us_per_step(n_coeffs):
    return max(2.5, 0.16 * max(1, int(n_coeffs)))


def _solve_us_per_step(*, solver_mode, degree, fused_threads):
    degree = max(1.0, float(degree))
    if solver_mode == "companion_matrix":
        return 90.0 * (degree / 35.0)
    if solver_mode == "aberth_mt":
        return (28.0 * (degree / 35.0)) / max(1.0, min(float(fused_threads), 8.0) ** 0.55)
    return 42.0 * (degree / 35.0)


def _read_positive_int_env(name, default):
    raw = os.environ.get(name)
    if raw in (None, ""):
        return int(default)
    try:
        value = int(raw)
    except (TypeError, ValueError):
        return int(default)
    return int(default) if value < 1 else value
