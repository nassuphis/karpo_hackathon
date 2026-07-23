"""
Palette finalize Lambda — assemble pass-0 palette images while preserving durable
all-pass chunk-local palette data for later reuse.
"""
import json
import os
import subprocess
import time
import hashlib

import boto3

from color_artifact_meta import load_color_artifact_head
from color_render_contract import normalize_color_interpretation
from palette_names import normalize_palette_display_name
from raw_score_render import histogram_from_raw_path_channel0, render_score_raw, write_equalization_lut
from raw_sidecar import build_raw_sidecar
from solve_score_chain import (
    SOLVE_SCORE_SPEC_VERSION,
    compiled_solve_score_fingerprint,
    emit_solve_score_metadata,
)
from solve_score_pipeline_programs import solve_score_program_for_run
from shared import (
    BUCKET,
    CACHE_IMMUTABLE,
    attach_contract_warnings,
    contract_param,
    parse_boolish,
    parse_body,
    ok_response,
    report_status,
    imgpipe_env,
)

s3 = boto3.client("s3")
PALETTE_VARIANT_SPEC_VERSION = 1
PALETTE_RENDER = os.path.join(os.path.dirname(__file__), "palette_bins_render")
RAW2JPEG = os.path.join(os.path.dirname(__file__), "raw2jpeg")
PRESIGN_EXPIRY = 3600
S3_USER_METADATA_LIMIT_BYTES = 2048

_TMP_BINS = "/tmp/palette_bins_full.bin"
_TMP_RAW = "/tmp/palette_image.raw"
_TMP_RAW_SCORE = "/tmp/palette_score.raw"
_TMP_EQ_LUT = "/tmp/palette_eq.lut"
_TMP_JPEG = "/tmp/palette_image.jpeg"
_TMP_PREVIEW = "/tmp/palette_preview.png"
VIEWPORT_METADATA_KEYS = (
    "view_mode",
    "quantile",
    "shim",
    "square_extent",
    "min_re",
    "max_re",
    "min_im",
    "max_im",
    "rotation",
)


def _cleanup_tmp():
    for p in (_TMP_BINS, _TMP_RAW, _TMP_RAW_SCORE, _TMP_EQ_LUT, _TMP_JPEG, _TMP_PREVIEW):
        try:
            os.remove(p)
        except OSError:
            pass


def _utc_now_iso():
    return time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())


def _omega_display(enabled, omega):
    return f"w={omega:g}" if enabled else "w=off"


def _metadata_size_bytes(meta):
    total = 0
    for key, value in (meta or {}).items():
        total += len(str(key).encode("utf-8"))
        total += len(str(value).encode("utf-8"))
    return total


def _copy_viewport_metadata(target_metadata, source_metadata):
    for key in VIEWPORT_METADATA_KEYS:
        value = (source_metadata or {}).get(key)
        if value not in ("", None):
            target_metadata[key] = value


def _delete_keys(keys):
    keys = [k for k in keys if k]
    for i in range(0, len(keys), 1000):
        batch = keys[i:i + 1000]
        if not batch:
            continue
        s3.delete_objects(Bucket=BUCKET, Delete={"Objects": [{"Key": k} for k in batch]})


def _stable_digest(payload):
    return hashlib.sha256(
        json.dumps(payload, sort_keys=True, separators=(",", ":")).encode("utf-8")
    ).hexdigest()


def _write_identity_lut(path):
    with open(path, "wb") as fh:
        fh.write(bytes(range(256)))


def _list_keys(prefix):
    keys = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=prefix):
        keys.extend(obj["Key"] for obj in page.get("Contents", []))
    return keys


def _read_palette_bin_prefix(key, n_bytes):
    kwargs = {"Bucket": BUCKET, "Key": key}
    if n_bytes is not None:
        if n_bytes < 1:
            return b""
        kwargs["Range"] = f"bytes=0-{n_bytes - 1}"
    obj = s3.get_object(**kwargs)
    return obj["Body"].read()


def _copy_pass0_chunk_rows(dst, src, *, full_n, step_start, channels=1):
    """Copy a contiguous pass-0 chunk into the serpentine image buffer."""
    src_len = len(src)
    if src_len == 0:
        return 0
    channels = int(channels or 1)
    if channels < 1:
        raise RuntimeError(f"channels must be >= 1, got {channels}")
    if src_len % channels != 0:
        raise RuntimeError(f"Chunk byte length {src_len} is not divisible by channels={channels}")
    if step_start < 0:
        raise RuntimeError(f"Chunk writes negative solve index at {step_start}")

    g = step_start
    src_pos = 0
    sample_count = src_len // channels
    end = step_start + sample_count
    while g < end:
        row = g // full_n
        row_start = row * full_n
        row_end = row_start + full_n
        seg_end = min(end, row_end)
        seg_len = seg_end - g
        j0 = g - row_start
        j1 = j0 + seg_len
        byte_len = seg_len * channels
        segment = src[src_pos:src_pos + byte_len]
        if row % 2 == 0:
            dst0 = (row_start + j0) * channels
            dst[dst0:dst0 + byte_len] = segment
        else:
            dst0 = (row_start + (full_n - j1)) * channels
            dst1 = (row_start + (full_n - j0)) * channels
            for px in range(seg_len):
                src_px = seg_len - 1 - px
                dst[dst0 + px * channels:dst0 + (px + 1) * channels] = (
                    segment[src_px * channels:(src_px + 1) * channels]
                )
        src_pos += byte_len
        g = seg_end
    return sample_count


def handler(event, context):
    params = parse_body(event)
    contract_warnings = []
    job_id = params["job_id"]
    task_id = params["task_id"]
    palette_id = params["palette_id"]
    full_n = int(params["N"])
    times = int(contract_param(params, "times", 1, contract_warnings) or 1)
    degree = int(params["degree"])
    metric = params["metric"]
    palette = params["palette"]
    palette_display_name = normalize_palette_display_name(
        params.get("palette_display_name"),
        palette,
    )
    solve_score_chain = contract_param(params, "solve_score_chain", "", contract_warnings)
    compile_params = dict(params)
    compile_params.setdefault("metric", metric)
    compile_params.setdefault("solve_metric", metric)
    compiled = solve_score_program_for_run(compile_params)
    solve_score_chain = compiled.get("chain_public") or solve_score_chain
    solve_score_program_source_text = str(
        params.get("solve_score_program_source_text")
        or compiled.get("source_text")
        or ""
    )
    metric = compiled["metric"]
    q = compiled["quantile"]
    omega = compiled["omega"]
    omega_enabled = compiled["omega_enabled"]
    chain_fingerprint = compiled_solve_score_fingerprint(compiled)
    render_execution = contract_param(params, "render_execution", None, contract_warnings)
    root_transforms = contract_param(params, "root_transforms", [], contract_warnings)
    root_program = params.get("root_program") if isinstance(params.get("root_program"), dict) else {}
    root_program_source_text = str(
        params.get("root_program_source_text")
        or root_program.get("source_text")
        or ""
    )
    root_program_fingerprint = str(
        params.get("root_program_fingerprint")
        or root_program.get("fingerprint")
        or ""
    )
    root_spec_version = int(params.get("root_spec_version") or root_program.get("spec_version") or 1)
    image_key = params["image_key"]
    preview_key = params["preview_key"]
    meta_key = params["meta_key"]
    raw_key = str(params.get("raw_key") or "").strip()
    raw_meta_key = str(params.get("raw_meta_key") or "").strip()
    color_interpretation = normalize_color_interpretation(params.get("color_interpretation") or "scalar_lut")
    score_output_channel_count = int(params.get("score_output_channel_count") or params.get("raw_channels") or 1)
    if not (1 <= score_output_channel_count <= 8):
        raise RuntimeError(f"score_output_channel_count must be in [1, 8], got {score_output_channel_count}")
    score_output_channels = list(params.get("score_output_channels") or [])
    raw_layout = str(
        params.get("raw_layout")
        or ("u8_scalar_row_major" if score_output_channel_count == 1 else "u8_packed_channels_row_major")
    )
    raw_output_path = bool(raw_key or score_output_channel_count != 1 or compiled.get("has_explicit_outputs"))
    chunks_prefix = params["chunks_prefix"]
    solve_score_prefix = params["solve_score_prefix"]
    solve_score_clip_key = params["solve_score_clip_key"]
    solve_score_bins_key = params["solve_score_bins_key"]
    cleanup_solve_score_scratch = parse_boolish(
        contract_param(params, "cleanup_solve_score_scratch", True, contract_warnings),
        True,
    )
    section_scores_prefix = params.get("section_scores_prefix", params.get("chunk_scores_prefix", chunks_prefix + "score_section_"))
    section_bins_prefix = params.get("section_bins_prefix", params.get("chunk_bins_prefix", chunks_prefix + "palette_bins_section_"))
    section_meta_prefix = params.get("section_meta_prefix", params.get("chunk_meta_prefix", chunks_prefix + "meta_section_"))
    source_color_artifact_id = str(params.get("source_color_artifact_id") or "").strip()
    source_view_meta = {}
    if source_color_artifact_id:
        try:
            source_head = load_color_artifact_head(s3, BUCKET, job_id, source_color_artifact_id)
            source_view_meta = dict(source_head.get("metadata", {}) or {})
        except Exception:
            source_view_meta = {}

    progress = attach_contract_warnings({"phase": "palette_finalize", "palette_id": palette_id}, contract_warnings)
    try:
        _cleanup_tmp()
        report_status(job_id, task_id, "started", result_data=progress)

        t0 = time.time()
        meta_keys = [k for k in _list_keys(section_meta_prefix) if k.endswith(".json")]
        if not meta_keys:
            raise RuntimeError(f"No palette section metadata found under {chunks_prefix}")

        chunk_meta = []
        for key in meta_keys:
            obj = s3.get_object(Bucket=BUCKET, Key=key)
            chunk_meta.append(json.loads(obj["Body"].read()))
        chunk_meta.sort(key=lambda m: (m.get("step_start", 0), m.get("section_idx", m.get("chunk_idx", 0))))

        pass0_steps = full_n * full_n
        assembled_channels = score_output_channel_count if raw_output_path else 1
        bins = bytearray(pass0_steps * assembled_channels)

        filled = 0
        pass0_chunks_read = 0
        pass0_chunks_skipped = 0
        pass0_bytes_read = 0
        for meta in chunk_meta:
            step_start = int(meta["step_start"])
            step_count = int(meta["step_count"])
            if step_start < 0:
                raise RuntimeError(f"Section {meta.get('section_idx', meta.get('chunk_idx'))} writes negative solve index at {step_start}")
            pass0_count = max(0, min(step_count, pass0_steps - step_start))
            if pass0_count <= 0:
                pass0_chunks_skipped += 1
                continue
            bin_key = meta["palette_bins_key"]
            meta_channels = int(meta.get("raw_channels") or meta.get("score_output_channel_count") or 1)
            if meta_channels != assembled_channels:
                raise RuntimeError(
                    f"Section {meta.get('section_idx', meta.get('chunk_idx'))} channel count "
                    f"{meta_channels} != expected {assembled_channels}"
                )
            range_count = (pass0_count * assembled_channels) if pass0_count < step_count else None
            bin_bytes = _read_palette_bin_prefix(bin_key, range_count)
            expected_len = (pass0_count if range_count is not None else step_count) * assembled_channels
            if len(bin_bytes) != expected_len:
                raise RuntimeError(
                    f"Section {meta.get('section_idx', meta.get('chunk_idx'))} "
                    f"bin length {len(bin_bytes)} != {expected_len}"
                )
            if range_count is None:
                bin_bytes = bin_bytes[:pass0_count * assembled_channels]

            filled += _copy_pass0_chunk_rows(
                bins,
                bin_bytes,
                full_n=full_n,
                step_start=step_start,
                channels=assembled_channels,
            )
            pass0_chunks_read += 1
            pass0_bytes_read += len(bin_bytes)

        if filled != pass0_steps:
            raise RuntimeError(f"Palette finalize filled {filled} samples, expected {pass0_steps}")

        assembled_path = _TMP_RAW_SCORE if raw_output_path else _TMP_BINS
        with open(assembled_path, "wb") as bf:
            bf.write(bins)

        bins_obj = s3.get_object(Bucket=BUCKET, Key=solve_score_bins_key)
        bins_meta = json.loads(bins_obj["Body"].read())
        bins_channel_count = int(bins_meta.get("score_output_channel_count") or 1)
        if bins_channel_count != score_output_channel_count:
            raise RuntimeError(
                "Solve-score bins channel count mismatch: "
                f"expected {score_output_channel_count}, got {bins_channel_count}"
            )
        if bins_meta.get("score_output_channels"):
            score_output_channels = list(bins_meta.get("score_output_channels") or [])
        clip_obj = s3.get_object(Bucket=BUCKET, Key=solve_score_clip_key)
        clip_meta = json.loads(clip_obj["Body"].read())
        actual_bins_fingerprint = str(bins_meta.get("chain_fingerprint") or "").strip()
        if int(bins_meta.get("version", 1) or 1) >= 2:
            if not actual_bins_fingerprint:
                raise RuntimeError("Solve-score bins artifact missing chain_fingerprint")
            if actual_bins_fingerprint != chain_fingerprint:
                raise RuntimeError(
                    f"Solve-score bins fingerprint mismatch: expected {chain_fingerprint}, got {actual_bins_fingerprint}"
                )
        else:
            if float(bins_meta.get("omega", 1.0)) != omega:
                raise RuntimeError(f"Solve-score bins omega mismatch: expected {omega}, got {bins_meta.get('omega')}")
            if parse_boolish(bins_meta.get("omega_enabled", True), True) != omega_enabled:
                raise RuntimeError(
                    f"Solve-score bins omega_enabled mismatch: expected {omega_enabled}, got {bins_meta.get('omega_enabled')}"
                )
        actual_clip_fingerprint = str(clip_meta.get("chain_fingerprint") or "").strip()
        if int(clip_meta.get("version", 1) or 1) >= 2:
            if not actual_clip_fingerprint:
                raise RuntimeError("Solve-score clip artifact missing chain_fingerprint")
            if actual_clip_fingerprint != chain_fingerprint:
                raise RuntimeError(
                    f"Solve-score clip fingerprint mismatch: expected {chain_fingerprint}, got {actual_clip_fingerprint}"
                )
        assemble_ms = int((time.time() - t0) * 1000)
        assemble_stats = {
            "assemble_ms": assemble_ms,
            "pass0_chunks_read": pass0_chunks_read,
            "pass0_chunks_skipped": pass0_chunks_skipped,
            "pass0_chunk_count": len(chunk_meta),
            "pass0_bytes_read": pass0_bytes_read,
        }
        report_status(
            job_id,
            task_id,
            "assembled",
            result_data=attach_contract_warnings({**progress, **assemble_stats}, contract_warnings),
        )

        raw_histogram = None
        raw_file_size = 0
        raw_sidecar_body = None
        if raw_output_path:
            if not raw_key or not raw_meta_key:
                raise RuntimeError("Raw palette output requires raw_key and raw_meta_key")
            expected_raw_size = pass0_steps * score_output_channel_count
            raw_file_size = os.path.getsize(_TMP_RAW_SCORE)
            if raw_file_size != expected_raw_size:
                raise RuntimeError(f"Raw palette size mismatch: expected {expected_raw_size}, got {raw_file_size}")
            raw_histogram = histogram_from_raw_path_channel0(
                _TMP_RAW_SCORE,
                channels=score_output_channel_count,
                expected_size=expected_raw_size,
            )
            eq_lut_path = ""
            if score_output_channel_count == 1 and color_interpretation == "scalar_lut":
                _write_identity_lut(_TMP_EQ_LUT)
                eq_lut_path = _TMP_EQ_LUT
            t1 = time.time()
            encode_meta = render_score_raw(
                raw_path=_TMP_RAW_SCORE,
                out_path=_TMP_JPEG,
                preview_path=_TMP_PREVIEW,
                pix=full_n,
                eq_lut_path=eq_lut_path,
                palette=palette or "inferno",
                background_color="000000",
                quality=90,
                channels=score_output_channel_count,
                interpretation=color_interpretation,
                zero_background=False,
            )
            render_ms = int((time.time() - t1) * 1000)
            encode_ms = 0
            file_size = int(encode_meta["file_size"])
        else:
            t1 = time.time()
            env = imgpipe_env()
            result = subprocess.run(
                [PALETTE_RENDER, _TMP_BINS, _TMP_RAW, f"--n={full_n}", f"--palette={palette}"],
                capture_output=True, text=True, timeout=300,
            )
            render_ms = int((time.time() - t1) * 1000)
            if result.returncode != 0:
                raise RuntimeError(f"palette_bins_render failed: {result.stderr.strip()}")

            t2 = time.time()
            enc = subprocess.run(
                [RAW2JPEG, _TMP_RAW, _TMP_JPEG, "--quality=90"],
                capture_output=True, text=True, timeout=300, env=env,
            )
            encode_ms = int((time.time() - t2) * 1000)
            if enc.returncode != 0:
                raise RuntimeError(f"raw2jpeg failed: {enc.stderr.strip()}")

            vt_path = "/opt/bin/vipsthumbnail"
            prev = subprocess.run(
                [vt_path, _TMP_JPEG, "-s", "512x512", "-o", _TMP_PREVIEW + "[strip]"],
                capture_output=True, text=True, timeout=60, env=env,
            )
            if prev.returncode != 0:
                raise RuntimeError(f"Preview generation failed: {prev.stderr.strip()}")

            file_size = os.path.getsize(_TMP_JPEG)
        metadata = {
            "pix": str(full_n),
            "width": str(full_n),
            "height": str(full_n),
            "palette": palette,
            "color_interpretation": color_interpretation,
            "score_output_channel_count": str(score_output_channel_count),
            "raw_channels": str(score_output_channel_count if raw_output_path else 1),
            "raw_layout": raw_layout if raw_output_path else "",
            "full_n": str(full_n),
            "times": str(times),
            "using_pass": "0",
            "clip_lo": str(bins_meta.get("clip_lo", "")),
            "clip_hi": str(bins_meta.get("clip_hi", "")),
        }
        if root_program_fingerprint:
            metadata["root_program_fingerprint"] = root_program_fingerprint
            metadata["root_spec_version"] = str(root_spec_version)
        _copy_viewport_metadata(metadata, source_view_meta)
        metadata_size = _metadata_size_bytes(metadata)
        if metadata_size > S3_USER_METADATA_LIMIT_BYTES:
            raise RuntimeError(
                f"Palette image metadata too large before upload: {metadata_size} bytes > "
                f"{S3_USER_METADATA_LIMIT_BYTES} limit"
            )
        with open(_TMP_JPEG, "rb") as fh:
            s3.upload_fileobj(fh, BUCKET, image_key, ExtraArgs={"ContentType": "image/jpeg", "Metadata": metadata})
        with open(_TMP_PREVIEW, "rb") as pf:
            s3.upload_fileobj(pf, BUCKET, preview_key, ExtraArgs={"ContentType": "image/png", "CacheControl": CACHE_IMMUTABLE})
        if raw_output_path:
            with open(_TMP_RAW_SCORE, "rb") as rf:
                s3.upload_fileobj(rf, BUCKET, raw_key, ExtraArgs={"ContentType": "application/octet-stream"})

        created_at = _utc_now_iso()
        plan_params_digest = _stable_digest({
            "metric": metric,
            "palette": palette,
            "color_interpretation": color_interpretation,
            "score_output_channel_count": score_output_channel_count,
            "solve_score_chain": solve_score_chain,
            "solve_score_program_source_text": solve_score_program_source_text,
            "solve_score_quantile": q,
            "solve_score_omega": omega,
            "solve_score_omega_enabled": omega_enabled,
            "root_program_fingerprint": root_program_fingerprint,
            "root_spec_version": root_spec_version,
            "root_transforms": root_transforms or [],
        })
        if raw_output_path:
            raw_sidecar_body = build_raw_sidecar(
                job_id=job_id,
                run_id=params.get("run_id") or "",
                artifact_family="palette",
                artifact_id=palette_id,
                width=full_n,
                height=full_n,
                chain_fingerprint=chain_fingerprint,
                solve_score_spec_version=int(
                    params.get("solve_score_spec_version", SOLVE_SCORE_SPEC_VERSION) or SOLVE_SCORE_SPEC_VERSION
                ),
                score_chain=solve_score_chain,
                score_program=bins_meta.get("program") or compiled.get("program_spec") or "",
                score_source_text=solve_score_program_source_text,
                clip_slots=bins_meta.get("metrics") or [
                    {
                        "slot": 0,
                        "metric": metric,
                        "source": "slv",
                        "clip_lo": bins_meta.get("clip_lo", 0.0),
                        "clip_hi": bins_meta.get("clip_hi", 1.0),
                    }
                ],
                score_output_normalize=any(
                    bool(ch.get("range_normalized", ch.get("normalized", False)))
                    for ch in score_output_channels
                    if isinstance(ch, dict)
                ),
                score_output_clip_lo=(score_output_channels[0].get("clip_lo", 0.0) if score_output_channels and isinstance(score_output_channels[0], dict) else 0.0),
                score_output_clip_hi=(score_output_channels[0].get("clip_hi", 1.0) if score_output_channels and isinstance(score_output_channels[0], dict) else 1.0),
                background_color="000000",
                plan_params_digest=plan_params_digest,
                render_execution=render_execution if isinstance(render_execution, dict) else {},
                raw_key=raw_key,
                image_key=image_key,
                preview_key=preview_key,
                meta_key=meta_key,
                created_at=created_at,
                histogram=raw_histogram or [0] * 256,
                step_scores_key=raw_key,
                step_count=pass0_steps,
                step_scores_grid_n=full_n,
                channels=score_output_channel_count,
                raw_layout=raw_layout,
                interpretation=color_interpretation,
                output_channels=score_output_channels,
            )
            s3.put_object(
                Bucket=BUCKET,
                Key=raw_meta_key,
                Body=json.dumps(raw_sidecar_body),
                ContentType="application/json",
            )
        meta_body = {
            "job_id": job_id,
            "palette_id": palette_id,
            "created_at": created_at,
            "display_name": (
                f"{metric} q={(float(q) * 100):.1f}% "
                f"{_omega_display(omega_enabled, omega)} "
                f"{palette_display_name or palette} {created_at}"
            ),
            "palette": palette,
            "palette_display_name": palette_display_name,
            "root_transforms": root_transforms or [],
            "degree": degree,
            "N": full_n,
            "times": times,
            "using_pass": 0,
            "image_pass": 0,
            "base_grid_solves": pass0_steps,
            "total_solves": sum(int(m.get("step_count", 0)) for m in chunk_meta),
            "pass_count": times,
            "data_layout": raw_layout if raw_output_path else "chunk_all_pass_v1",
            "render_reusable": not raw_output_path,
            "color_interpretation": color_interpretation,
            "score_output_channel_count": score_output_channel_count,
            "score_output_channels": score_output_channels,
            "raw_channels": score_output_channel_count if raw_output_path else 1,
            "raw_layout": raw_layout if raw_output_path else "",
            "raw_key": raw_key if raw_output_path else "",
            "raw_meta_key": raw_meta_key if raw_output_path else "",
            "raw_file_size": raw_file_size,
            "raw_sidecar_version": (raw_sidecar_body or {}).get("version") if raw_output_path else None,
            "palette_variant_fingerprint": params.get("palette_variant_fingerprint") or "",
            "palette_variant_spec_version": int(
                params.get("palette_variant_spec_version", PALETTE_VARIANT_SPEC_VERSION)
                or PALETTE_VARIANT_SPEC_VERSION
            ),
            "content_fingerprint": params.get("palette_variant_fingerprint") or "",
            "clip_lo": bins_meta.get("clip_lo"),
            "clip_hi": bins_meta.get("clip_hi"),
            "cuts_norm": bins_meta.get("cuts_norm", []),
            "clip_fallback": clip_meta.get("clip_fallback"),
            "clip_fallback_reason": clip_meta.get("clip_fallback_reason"),
            "file_size": file_size,
            "image_key": image_key,
            "preview_key": preview_key,
            "section_scores_prefix": section_scores_prefix,
            "section_bins_prefix": section_bins_prefix,
            "section_meta_prefix": section_meta_prefix,
            "chunk_scores_prefix": section_scores_prefix,
            "chunk_bins_prefix": section_bins_prefix,
            "chunk_meta_prefix": section_meta_prefix,
        }
        if isinstance(render_execution, dict):
            meta_body["render_execution"] = render_execution
        if root_program_fingerprint:
            meta_body["root_program_source_text"] = root_program_source_text
            meta_body["root_program"] = root_program
            meta_body["root_program_fingerprint"] = root_program_fingerprint
            meta_body["root_spec_version"] = root_spec_version
        meta_body.update(
            {
                "metric": metric,
                "solve_score_quantile": float(q),
                "solve_score_omega": omega,
                "solve_score_omega_enabled": omega_enabled,
                "solve_score_chain": json.loads(
                    emit_solve_score_metadata(
                        "solve",
                        metric=metric,
                        quantile=q,
                        omega=omega,
                        omega_enabled=omega_enabled,
                        chain=solve_score_chain,
                    )["solve_score_chain"]
                ),
                "solve_score_chain_fingerprint": chain_fingerprint,
                "solve_score_program_source_text": solve_score_program_source_text,
                "solve_score_spec_version": int(
                    params.get("solve_score_spec_version", SOLVE_SCORE_SPEC_VERSION)
                    or SOLVE_SCORE_SPEC_VERSION
                ),
            }
        )
        if source_color_artifact_id:
            meta_body["derived_from_color_artifact_id"] = source_color_artifact_id
            meta_body["derivation_kind"] = "extract_palette"
        _copy_viewport_metadata(meta_body, source_view_meta)
        s3.put_object(Bucket=BUCKET, Key=meta_key, Body=json.dumps(meta_body), ContentType="application/json")

        # Cleanup only when this workflow owns the solve-score scratch.
        if cleanup_solve_score_scratch:
            _delete_keys(_list_keys(solve_score_prefix) + [solve_score_clip_key, solve_score_bins_key])

        result_data = attach_contract_warnings({
            "palette_id": palette_id,
            "image_key": image_key,
            "preview_key": preview_key,
            "file_size": file_size,
            "raw_key": raw_key if raw_output_path else "",
            "raw_meta_key": raw_meta_key if raw_output_path else "",
            "raw_channels": score_output_channel_count if raw_output_path else 1,
            "color_interpretation": color_interpretation,
            "render_reusable": not raw_output_path,
            "palette_display_name": palette_display_name,
            **assemble_stats,
            "render_ms": render_ms,
            "encode_ms": encode_ms,
        }, contract_warnings)
        report_status(job_id, task_id, "done", result_data=result_data)
        return ok_response(result_data)
    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        _cleanup_tmp()
