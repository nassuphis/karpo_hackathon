"""
Shared helpers for normalizing calc.json chunk metadata.
"""
from __future__ import annotations

from param_source import enrich_chunk_item_with_params, fallback_params_key


def fallback_lores_coeffs_key(job_id, calc):
    lores = calc.get("lores", {}) or {}
    key = str(lores.get("coeffs_key") or "").strip()
    if key:
        return key
    job = str(job_id or "").strip()
    return f"renders/{job}/lores_coeffs.bin" if job else ""


def fallback_lores_params_key(job_id, calc):
    lores = calc.get("lores", {}) or {}
    key = str(lores.get("params_key") or "").strip()
    if key:
        return key
    job = str(job_id or "").strip()
    return f"renders/{job}/lores_params.bin" if job else ""


def fallback_params_global_key(job_id, calc):
    return fallback_params_key(job_id, calc)


def chunk_coeffs_key(calc, job_id, chunk_idx):
    coeffs_keys = list(calc.get("coeffs_keys") or [])
    idx = int(chunk_idx)
    if 0 <= idx < len(coeffs_keys):
        key = str(coeffs_keys[idx] or "").strip()
        if key:
            return key
    return f"renders/{job_id}/coeffs_{idx:04d}.bin"


def build_chunk_items(calc, job_id, *, require_chunks=False):
    chunks = list(calc.get("chunks", calc.get("stripes", [])) or [])
    if chunks:
        degree = int(calc.get("degree", 1) or 1)
        n_coeffs = int(calc.get("n_coeffs", degree + 1) or (degree + 1))
        record_bytes = degree * 2 * 4
        chunk_items = []
        step_start = 0
        ordered_chunks = sorted(
            chunks,
            key=lambda raw: raw.get("idx", raw.get("chunk_idx", raw.get("stripe_idx", 0))),
        )
        for raw in ordered_chunks:
            idx = raw.get("idx", raw.get("chunk_idx", raw.get("stripe_idx")))
            bin_key = raw.get("bin_key", raw.get("s3_key"))
            if idx is None or not bin_key:
                raise RuntimeError(f"Invalid chunk metadata: idx={idx} bin_key={bin_key!r}")
            item = {
                "chunk_idx": int(idx),
                "bin_key": str(bin_key),
                "coeffs_key": chunk_coeffs_key(calc, job_id, int(idx)),
            }
            bin_size = raw.get("bin_size")
            step_count = raw.get("step_count", raw.get("n_t"))
            if step_count in ("", None):
                if bin_size not in ("", None):
                    step_count = int(bin_size) // record_bytes
            if step_count not in ("", None):
                step_count = int(step_count)
                if step_count <= 0:
                    raise RuntimeError(f"Invalid chunk metadata: idx={idx} step_count={step_count}")
                item["step_start"] = step_start
                item["step_count"] = step_count
                item["bin_size"] = int(bin_size) if bin_size not in ("", None) else int(step_count) * record_bytes
                item["coeffs_bin_size"] = int(step_count) * n_coeffs * 2 * 4
                enrich_chunk_item_with_params(item, raw, calc, job_id)
                step_start += step_count
            elif bin_size not in ("", None):
                item["bin_size"] = int(bin_size)
            chunk_items.append(item)
        return chunk_items

    if require_chunks:
        raise RuntimeError("calc.json missing chunk metadata")

    n_chunks = calc.get("n_chunks", calc.get("n_stripes", 10))
    return [
        {
            "chunk_idx": c,
            "bin_key": f"renders/{job_id}/chunk_{c}.bin",
            "coeffs_key": chunk_coeffs_key(calc, job_id, c),
        }
        for c in range(n_chunks)
    ]
