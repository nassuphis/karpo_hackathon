"""
Palette render plan Lambda — computes the workflow plan for reusable full-resolution
palette artifacts.

The palette image/preview still uses pass 0 only, but the durable numeric payload is
all-pass chunk-local bin/score data so saved palettes can later drive Color renders.
"""
import hashlib
import json
import time

import boto3

from color_artifact_meta import load_color_artifact_head, parse_root_transforms
from palette_names import VALID_PALETTE_NAMES
from shared import BUCKET, parse_body, ok_response

s3 = boto3.client("s3")

MAX_PLAN_BYTES = 200 * 1024  # fail fast before 256KB Step Functions limit
VALID_METRICS = {
    "proximity", "crowding", "spread", "anisotropy", "area",
    "clusteriness", "shelliness", "outlierness", "nn_variation", "real_axis_proximity",
    "centroid_re", "centroid_im", "centroid_dist", "dist_unit_circle", "asymmetry_re",
}


def _validate_threads(value, default=1):
    if value in (None, ""):
        return default
    try:
        threads = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_threads must be an integer, got {value!r}")
    if not (1 <= threads <= 16):
        raise RuntimeError(f"solve_score_threads must be in [1, 16], got {threads}")
    return threads


def _validate_palette_chunk_threads(value, default=4):
    if value in (None, ""):
        value = default
    try:
        threads = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"palette_chunk_threads must be an integer, got {value!r}")
    if not (1 <= threads <= 16):
        raise RuntimeError(f"palette_chunk_threads must be in [1, 16], got {threads}")
    return threads


def _validate_merge_workers(value, default=16):
    if value in (None, ""):
        value = default
    try:
        workers = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_merge_workers must be an integer, got {value!r}")
    if not (1 <= workers <= 64):
        raise RuntimeError(f"solve_score_merge_workers must be in [1, 64], got {workers}")
    return workers


def _validate_palette_chunk_workers(value, default=16):
    if value in (None, ""):
        value = default
    try:
        workers = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"palette_chunk_workers must be an integer, got {value!r}")
    if not (1 <= workers <= 64):
        raise RuntimeError(f"palette_chunk_workers must be in [1, 64], got {workers}")
    return workers


def _validate_hist_input_mode(value):
    mode = str(value or "tmpfile").strip().lower()
    if mode not in {"tmpfile", "stdin", "sectioned"}:
        raise RuntimeError(
            "solve_score_hist_input_mode must be one of sectioned, stdin, tmpfile, "
            f"got {value!r}"
        )
    return mode


def _validate_palette_chunk_input_mode(value):
    mode = str(value or "sectioned").strip().lower()
    if mode not in {"tmpfile", "sectioned"}:
        raise RuntimeError(
            "palette_chunk_input_mode must be one of sectioned, tmpfile, "
            f"got {value!r}"
        )
    return mode


def _validate_sectioned_retries(value):
    if value in (None, ""):
        value = 2
    try:
        retries = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_hist_retries must be an integer, got {value!r}")
    if not (0 <= retries <= 10):
        raise RuntimeError(f"solve_score_hist_retries must be in [0, 10], got {retries}")
    return retries


def _validate_omega(value):
    try:
        omega = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_omega must be numeric, got {value!r}")
    if not (1.0 <= omega <= 10.0):
        raise RuntimeError(f"solve_score_omega must be in [1, 10], got {omega}")
    return omega


def _validate_omega_enabled(value):
    if value in (None, ""):
        return True
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    text = str(value).strip().lower()
    if text in ("1", "true", "yes", "on"):
        return True
    if text in ("0", "false", "no", "off"):
        return False
    raise RuntimeError(f"solve_score_omega_enabled must be boolean-like, got {value!r}")


def _palette_variant_id(metric, palette, q, omega, omega_enabled, root_transforms):
    q_label = f"{q * 100:.1f}".replace(".", "p")
    omega_label = (
        f"{omega:.0f}" if float(omega).is_integer() else str(omega).replace(".", "p")
    ) if omega_enabled else "off"
    rt_json = json.dumps(root_transforms or [], separators=(",", ":"))
    rt_hash = hashlib.sha1(rt_json.encode("utf-8")).hexdigest()[:8]
    return f"pal_{int(time.time() * 1000)}_{metric}_{palette}_q{q_label}_w{omega_label}_rt{rt_hash}"


def _omega_display(enabled, omega):
    return f"w={float(omega):g}" if enabled else "w=off"


def _palette_display_name(metric, palette, q, omega, omega_enabled):
    return f"{metric} q={(float(q) * 100):.1f}% {_omega_display(omega_enabled, omega)} {palette}"


def _load_calc(job_id):
    obj = s3.get_object(Bucket=BUCKET, Key=f"renders/{job_id}/calc.json")
    return json.loads(obj["Body"].read())


def _load_palette_meta(job_id, palette_id):
    key = f"renders/{job_id}/palettes/{palette_id}/meta.json"
    obj = s3.get_object(Bucket=BUCKET, Key=key)
    meta = json.loads(obj["Body"].read())
    if meta.get("job_id") and meta.get("job_id") != job_id:
        raise RuntimeError(f"Palette artifact {palette_id} belongs to {meta.get('job_id')}, not {job_id}")
    return meta


def _artifact_meta_quantile(meta, field_name):
    raw = meta.get(field_name, "")
    if raw in ("", None):
        return None
    return float(raw)


def _artifact_meta_omega_enabled(meta, field_name):
    if field_name not in meta:
        return None
    return _validate_omega_enabled(meta.get(field_name))


def _resolve_color_lineage(job_id, artifact_id):
    selected = _load_color_artifact(job_id, artifact_id)
    current = selected
    seen = {artifact_id}
    while True:
        if current.get("associated_palette_mode") and current.get("associated_palette_id"):
            return selected, current, "associated"
        if current.get("color_mode") == "saved_palette" and current.get("palette_source_id"):
            return selected, current, "saved_palette"
        if current.get("color_mode") == "solve_score":
            return selected, current, "solve_score"
        parent_id = str(current.get("derived_from_artifact_id") or "").strip()
        if not parent_id or parent_id in seen:
            break
        seen.add(parent_id)
        current = _load_color_artifact(job_id, parent_id)
    raise RuntimeError(
        f"ExtractPalette could not resolve reusable palette lineage from Color artifact {artifact_id}"
    )


def _load_color_artifact(job_id, artifact_id):
    head_info = load_color_artifact_head(s3, BUCKET, job_id, artifact_id)
    meta = dict(head_info["metadata"])
    meta["artifact_id"] = artifact_id
    meta["image_key"] = head_info["image_key"]
    meta["root_transforms"] = parse_root_transforms(meta.get("root_transforms"))
    return meta


def _scratch_matches(job_id, metric, quantile, omega, omega_enabled, root_transforms):
    clip_key = f"renders/{job_id}/solve_scores/{metric}_clip.json"
    bins_key = f"renders/{job_id}/solve_scores/{metric}_bins.json"
    try:
        clip = json.loads(s3.get_object(Bucket=BUCKET, Key=clip_key)["Body"].read())
        bins = json.loads(s3.get_object(Bucket=BUCKET, Key=bins_key)["Body"].read())
    except Exception:
        return False, clip_key, bins_key
    expected_xforms = root_transforms or []
    for obj in (clip, bins):
        if obj.get("family") != "solve_score":
            return False, clip_key, bins_key
        if obj.get("metric") != metric:
            return False, clip_key, bins_key
        if float(obj.get("clip_quantile", -1)) != float(quantile):
            return False, clip_key, bins_key
        if float(obj.get("omega", 1.0)) != float(omega):
            return False, clip_key, bins_key
        if _validate_omega_enabled(obj.get("omega_enabled", True)) != bool(omega_enabled):
            return False, clip_key, bins_key
        if parse_root_transforms(obj.get("root_transforms")) != expected_xforms:
            return False, clip_key, bins_key
    return True, clip_key, bins_key


def _build_chunk_items(calc):
    chunks = list(calc.get("chunks", calc.get("stripes", [])) or [])
    if not chunks:
        raise RuntimeError("calc.json missing chunk metadata")
    chunks.sort(key=lambda c: c.get("idx", c.get("chunk_idx", c.get("stripe_idx", 0))))
    degree = int(calc["degree"])
    record_bytes = degree * 2 * 4
    chunk_items = []
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
        chunk_items.append({
            "chunk_idx": int(idx),
            "bin_key": bin_key,
            "step_start": step_start,
            "step_count": step_count,
            "bin_size": int(raw.get("bin_size")) if raw.get("bin_size") not in ("", None) else int(step_count) * record_bytes,
        })
        step_start += step_count
    return chunk_items


def _base_extract_plan(job_id, run_id, task_id, selected, palette_id, display_name, image_key, preview_key, metric, palette, q, omega, omega_enabled):
    return {
        "job_id": job_id,
        "run_id": run_id,
        "task_id": task_id,
        "mode": "extract_palette",
        "params": {},
        "palette_id": palette_id,
        "extract": {},
        "attach": {
            "enabled": True,
            "artifact_id": selected["artifact_id"],
            "mode": "",
            "palette_id": palette_id,
            "display_name": display_name,
            "image_key": image_key,
            "preview_key": preview_key,
            "palette": palette,
            "metric": metric,
            "quantile": q,
            "omega": omega,
            "omega_enabled": omega_enabled,
        },
        "outputs": {
            "image_key": image_key,
            "preview_key": preview_key,
            "meta_key": f"renders/{job_id}/palettes/{palette_id}/meta.json" if palette_id else "",
            "chunks_prefix": "",
            "chunk_scores_prefix": "",
            "chunk_bins_prefix": "",
            "chunk_meta_prefix": "",
        },
    }


def _execution_params(raw_params):
    pp = dict(raw_params or {})
    return {
        "solve_score_threads": _validate_threads(pp.get("solve_score_threads", 1), default=1),
        "solve_score_hist_input_mode": _validate_hist_input_mode(pp.get("solve_score_hist_input_mode", "tmpfile")),
        "solve_score_hist_retries": _validate_sectioned_retries(pp.get("solve_score_hist_retries", 2)),
        "solve_score_merge_workers": _validate_merge_workers(pp.get("solve_score_merge_workers", 16), default=16),
        "palette_chunk_threads": _validate_palette_chunk_threads(pp.get("palette_chunk_threads", 4), default=4),
        "palette_chunk_input_mode": _validate_palette_chunk_input_mode(pp.get("palette_chunk_input_mode", "sectioned")),
        "palette_chunk_retries": _validate_sectioned_retries(pp.get("palette_chunk_retries", 2)),
        "palette_chunk_workers": _validate_palette_chunk_workers(pp.get("palette_chunk_workers", 16), default=16),
    }


def _build_extract_plan(job_id, run_id, task_id, artifact_id, raw_params=None):
    execution = _execution_params(raw_params)
    selected, source, source_kind = _resolve_color_lineage(job_id, artifact_id)

    if selected.get("associated_palette_mode") and selected.get("associated_palette_id"):
        palette_id = str(selected.get("associated_palette_id"))
        image_key = str(selected.get("associated_palette_image_key") or f"renders/{job_id}/palettes/{palette_id}/image.jpeg")
        preview_key = str(selected.get("associated_palette_preview_key") or f"renders/{job_id}/palettes/{palette_id}/preview.png")
        plan = _base_extract_plan(
            job_id, run_id, task_id, selected,
            palette_id=palette_id,
            display_name=str(selected.get("associated_palette_display_name") or ""),
            image_key=image_key,
            preview_key=preview_key,
            metric=str(selected.get("associated_palette_metric") or ""),
            palette=str(selected.get("associated_palette_palette") or ""),
            q=_artifact_meta_quantile(selected, "associated_palette_quantile"),
            omega=_artifact_meta_quantile(selected, "associated_palette_omega"),
            omega_enabled=_validate_omega_enabled(selected.get("associated_palette_omega_enabled", True)),
        )
        plan["params"].update(execution)
        plan["extract"] = {"action": "done", "reason": "already_associated", "source_artifact_id": selected["artifact_id"]}
        plan["attach"]["enabled"] = False
        return plan

    if source_kind == "associated":
        palette_id = str(source.get("associated_palette_id"))
        image_key = str(source.get("associated_palette_image_key") or f"renders/{job_id}/palettes/{palette_id}/image.jpeg")
        preview_key = str(source.get("associated_palette_preview_key") or f"renders/{job_id}/palettes/{palette_id}/preview.png")
        plan = _base_extract_plan(
            job_id, run_id, task_id, selected,
            palette_id=palette_id,
            display_name=str(source.get("associated_palette_display_name") or ""),
            image_key=image_key,
            preview_key=preview_key,
            metric=str(source.get("associated_palette_metric") or ""),
            palette=str(source.get("associated_palette_palette") or ""),
            q=_artifact_meta_quantile(source, "associated_palette_quantile"),
            omega=_artifact_meta_quantile(source, "associated_palette_omega"),
            omega_enabled=_validate_omega_enabled(source.get("associated_palette_omega_enabled", True)),
        )
        plan["params"].update(execution)
        plan["extract"] = {"action": "attach", "reason": "inherit_existing_association", "source_artifact_id": source["artifact_id"]}
        plan["attach"]["mode"] = str(source.get("associated_palette_mode") or "generated")
        return plan

    if source_kind == "saved_palette":
        palette_id = str(source.get("palette_source_id") or "").strip()
        if not palette_id:
            raise RuntimeError(f"Saved-palette Color artifact {source['artifact_id']} is missing palette_source_id")
        palette_meta = _load_palette_meta(job_id, palette_id)
        source_omega_enabled = _artifact_meta_omega_enabled(source, "palette_source_omega_enabled")
        plan = _base_extract_plan(
            job_id, run_id, task_id, selected,
            palette_id=palette_id,
            display_name=str(source.get("palette_source_display_name") or palette_meta.get("display_name") or ""),
            image_key=str(palette_meta.get("image_key") or f"renders/{job_id}/palettes/{palette_id}/image.jpeg"),
            preview_key=str(palette_meta.get("preview_key") or f"renders/{job_id}/palettes/{palette_id}/preview.png"),
            metric=str(source.get("palette_source_metric") or palette_meta.get("metric") or ""),
            palette=str(source.get("palette_source_palette") or palette_meta.get("palette") or ""),
            q=_artifact_meta_quantile(source, "palette_source_quantile"),
            omega=_artifact_meta_quantile(source, "palette_source_omega"),
            omega_enabled=source_omega_enabled
            if source_omega_enabled is not None
            else _validate_omega_enabled(palette_meta.get("solve_score_omega_enabled", True)),
        )
        plan["params"].update(execution)
        plan["extract"] = {"action": "attach", "reason": "saved_palette_dependency", "source_artifact_id": source["artifact_id"]}
        plan["attach"]["mode"] = "dependency"
        return plan

    metric = str(source.get("solve_metric") or "").strip()
    if not metric:
        raise RuntimeError(f"Solve-score Color artifact {source['artifact_id']} is missing solve_metric")
    palette = str(source.get("palette") or "").strip()
    if palette not in VALID_PALETTE_NAMES:
        raise RuntimeError(f"Solve-score Color artifact {source['artifact_id']} has invalid palette {palette!r}")
    q = _artifact_meta_quantile(source, "solve_score_quantile")
    if q is None:
        raise RuntimeError(f"Solve-score Color artifact {source['artifact_id']} is missing solve_score_quantile")
    omega = _artifact_meta_quantile(source, "solve_score_omega")
    if omega is None:
        omega = 1.0
    omega_enabled = _artifact_meta_omega_enabled(source, "solve_score_omega_enabled")
    if omega_enabled is None:
        omega_enabled = True
    root_transforms = list(source.get("root_transforms") or [])

    calc = _load_calc(job_id)
    degree = calc.get("degree")
    full_n = calc.get("N", calc.get("n1"))
    times = int(calc.get("times", 1) or 1)
    lores_bin_key = (calc.get("lores") or {}).get("bin_key", "")
    if not degree or not full_n or not lores_bin_key:
        raise RuntimeError(f"calc.json missing degree, N, or lores.bin_key for {job_id}")
    chunk_items = _build_chunk_items(calc)
    pass0_steps = int(full_n) * int(full_n)
    if sum(int(item["step_count"]) for item in chunk_items) < pass0_steps:
        raise RuntimeError("calc.json chunk metadata is too small for pass-0 palette extraction")

    scratch_ok, clip_key, bins_key = _scratch_matches(job_id, metric, q, omega, omega_enabled, root_transforms)
    palette_id = _palette_variant_id(metric, palette, q, omega, omega_enabled, root_transforms)
    prefix = f"renders/{job_id}/palettes/{palette_id}/"
    solve_prefix = prefix + "solve_score/"
    chunks_prefix = prefix + "chunks/"
    display_name = source.get("associated_palette_display_name") or _palette_display_name(metric, palette, q, omega, omega_enabled)

    plan = _base_extract_plan(
        job_id, run_id, task_id, selected,
        palette_id=palette_id,
        display_name=display_name,
        image_key=prefix + "image.jpeg",
        preview_key=prefix + "preview.png",
        metric=metric,
        palette=palette,
        q=q,
        omega=omega,
        omega_enabled=omega_enabled,
    )
    plan["mode"] = "extract_palette"
    plan["params"] = {
        "metric": metric,
        "palette": palette,
        "solve_score_quantile": q,
        "solve_score_omega": omega,
        "solve_score_omega_enabled": omega_enabled,
        "root_transforms": root_transforms,
        **execution,
    }
    plan["extract"] = {
        "action": "generate_reuse" if scratch_ok else "generate_full",
        "reason": "reuse_solve_score_scratch" if scratch_ok else "rerun_solve_score_prepass",
        "source_artifact_id": source["artifact_id"],
        "target_artifact_id": selected["artifact_id"],
    }
    plan["attach"]["mode"] = "generated"
    plan["calc"] = {
        "degree": degree,
        "N": full_n,
        "times": times,
        "n_chunks": len(chunk_items),
        "pass0_steps": pass0_steps,
        "lores_bin_key": lores_bin_key,
    }
    plan["chunk_items"] = chunk_items
    plan["solve_score"] = {
        "metric": metric,
        "quantile": q,
        "omega": omega,
        "omega_enabled": omega_enabled,
        "cleanup_scratch": not scratch_ok,
        "clip_key": clip_key if scratch_ok else prefix + f"solve_score/{metric}_clip.json",
        "hist_prefix": solve_prefix,
        "bins_key": bins_key if scratch_ok else prefix + f"solve_score/{metric}_bins.json",
    }
    plan["outputs"] = {
        "image_key": prefix + "image.jpeg",
        "preview_key": prefix + "preview.png",
        "meta_key": prefix + "meta.json",
        "chunks_prefix": chunks_prefix,
        "chunk_scores_prefix": chunks_prefix + "score_chunk_",
        "chunk_bins_prefix": chunks_prefix + "palette_bins_chunk_",
        "chunk_meta_prefix": chunks_prefix + "meta_chunk_",
    }
    return plan


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    run_id = params["run_id"]
    task_id = params["task_id"]
    artifact_id = str(params.get("artifact_id") or "").strip()
    if artifact_id:
        plan = _build_extract_plan(job_id, run_id, task_id, artifact_id, params.get("params", {}))
        plan_json = json.dumps(plan)
        if len(plan_json) > MAX_PLAN_BYTES:
            raise RuntimeError(
                f"Plan too large: {len(plan_json)} bytes > {MAX_PLAN_BYTES} limit. "
                f"Reduce chunk count."
            )
        return ok_response(plan)
    pp = dict(params.get("params", {}))

    execution = _execution_params(pp)
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
    omega = _validate_omega(pp.get("solve_score_omega", 1.0))
    omega_enabled = _validate_omega_enabled(pp.get("solve_score_omega_enabled", True))

    calc = _load_calc(job_id)
    degree = calc.get("degree")
    full_n = calc.get("N", calc.get("n1"))
    times = int(calc.get("times", 1) or 1)
    lores_bin_key = (calc.get("lores") or {}).get("bin_key", "")
    if not degree or not full_n or not lores_bin_key:
        raise RuntimeError(f"calc.json missing degree, N, or lores.bin_key for {job_id}")

    pass0_steps = full_n * full_n
    chunk_items = _build_chunk_items(calc)
    step_start = sum(int(item["step_count"]) for item in chunk_items)

    if step_start < pass0_steps:
        raise RuntimeError(f"Full solve metadata too small: only {step_start} solves, expected at least {pass0_steps}")

    palette_id = _palette_variant_id(metric, palette, q, omega, omega_enabled, root_transforms)
    prefix = f"renders/{job_id}/palettes/{palette_id}/"
    solve_prefix = prefix + "solve_score/"
    chunks_prefix = prefix + "chunks/"
    chunk_scores_prefix = chunks_prefix + "score_chunk_"
    chunk_bins_prefix = chunks_prefix + "palette_bins_chunk_"
    chunk_meta_prefix = chunks_prefix + "meta_chunk_"

    plan = {
        "job_id": job_id,
        "run_id": run_id,
        "task_id": task_id,
        "mode": "palette",
        "params": {
            "metric": metric,
            "palette": palette,
            "solve_score_quantile": q,
            "solve_score_omega": omega,
            "solve_score_omega_enabled": omega_enabled,
            "root_transforms": root_transforms,
            **execution,
        },
        "palette_id": palette_id,
        "extract": {
            "action": "generate_full",
            "reason": "normal_palette_generation",
            "source_artifact_id": "",
            "target_artifact_id": "",
        },
        "attach": {
            "enabled": False,
            "artifact_id": "",
            "mode": "",
            "palette_id": "",
            "display_name": "",
            "image_key": "",
            "preview_key": "",
            "palette": "",
            "metric": "",
            "quantile": "",
            "omega": "",
            "omega_enabled": True,
        },
        "prefix": prefix,
        "calc": {
            "degree": degree,
            "N": full_n,
            "times": times,
            "n_chunks": len(chunk_items),
            "pass0_steps": pass0_steps,
            "lores_bin_key": lores_bin_key,
        },
        "chunk_items": chunk_items,
        "solve_score": {
            "metric": metric,
            "quantile": q,
            "omega": omega,
            "omega_enabled": omega_enabled,
            "cleanup_scratch": True,
            "clip_key": prefix + f"solve_score/{metric}_clip.json",
            "hist_prefix": solve_prefix,
            "bins_key": prefix + f"solve_score/{metric}_bins.json",
        },
        "outputs": {
            "image_key": prefix + "image.jpeg",
            "preview_key": prefix + "preview.png",
            "meta_key": prefix + "meta.json",
            "chunks_prefix": chunks_prefix,
            "chunk_scores_prefix": chunk_scores_prefix,
            "chunk_bins_prefix": chunk_bins_prefix,
            "chunk_meta_prefix": chunk_meta_prefix,
        },
    }

    plan_json = json.dumps(plan)
    if len(plan_json) > MAX_PLAN_BYTES:
        raise RuntimeError(
            f"Plan too large: {len(plan_json)} bytes > {MAX_PLAN_BYTES} limit. "
            f"Reduce chunk count."
        )
    return ok_response(plan)
