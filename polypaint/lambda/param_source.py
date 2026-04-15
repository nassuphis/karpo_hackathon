PARAM_RECORD_BYTES = 16


def fallback_params_key(job_id, calc):
    key = str((calc or {}).get("params_key") or "").strip()
    if key:
        return key
    if str((calc or {}).get("param_storage_mode") or "").strip().lower() == "chunked":
        return ""
    job = str(job_id or "").strip()
    return f"renders/{job}/params.bin" if job else ""


def enrich_chunk_item_with_params(item, raw, calc, job_id):
    """Attach the param source needed for param-metric chips.

    Legacy calc.json files have one global params.bin, so a chunk reads from its
    global step offset. New calc.json files store one params_####.bin per chunk,
    so consumers read from offset zero inside that chunk object.
    """
    raw = raw or {}
    step_count = int(item.get("step_count") or raw.get("step_count") or raw.get("n_t") or 0)
    if step_count <= 0:
        return item

    raw_key = str(raw.get("params_key") or "").strip()
    if raw_key:
        item["params_key"] = raw_key
        item["params_step_start"] = int(raw.get("params_step_start") or 0)
        item["params_step_count"] = int(raw.get("params_step_count") or step_count)
        if raw.get("params_bin_size") not in ("", None):
            item["params_bin_size"] = int(raw.get("params_bin_size") or 0)
        else:
            item["params_bin_size"] = int(item["params_step_count"]) * PARAM_RECORD_BYTES
        return item

    key = fallback_params_key(job_id, calc)
    if key:
        item["params_key"] = key
        item["params_step_start"] = int(item.get("step_start") or raw.get("step_start") or 0)
        item["params_step_count"] = step_count
        item["params_bin_size"] = step_count * PARAM_RECORD_BYTES
    return item


def chunk_items_have_params(chunk_items):
    for item in chunk_items or []:
        key = str(item.get("params_key") or "").strip()
        if not key:
            return False
        try:
            if int(item.get("params_step_count") or 0) < 1:
                return False
        except (TypeError, ValueError):
            return False
    return bool(chunk_items)
