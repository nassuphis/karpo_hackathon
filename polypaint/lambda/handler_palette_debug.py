"""
Palette debug Lambda — generates a solve-score palette JPEG from lores data.

Synchronous, produces one S3 artifact: renders/{job_id}/image_palette.jpeg.
No DDB writes. No render orchestrator. No active render run.
"""
import hashlib
import json
import os
import subprocess
import time

import boto3

from palette_names import VALID_PALETTE_NAMES
from root_pipeline_programs import root_program_for_run
from shared import BUCKET, CACHE_IMMUTABLE, parse_body, ok_response, imgpipe_env

s3 = boto3.client("s3")
BINARY = os.path.join(os.path.dirname(__file__), "solve_palette_debug")
RAW2JPEG = os.path.join(os.path.dirname(__file__), "raw2jpeg")
PRESIGN_EXPIRY = 3600
S3_USER_METADATA_LIMIT_BYTES = 2048


def _utc_now_iso():
    return time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())


def _validate_omega(value):
    try:
        omega = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_omega must be numeric, got {value!r}")
    if not (omega == omega and abs(omega) != float("inf")):
        raise RuntimeError(f"solve_score_omega must be finite, got {value!r}")
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


def _omega_display(enabled, omega):
    return f"w={omega:g}" if enabled else "w=off"


def _palette_variant_id(metric, palette, q, omega, omega_enabled, root_transforms):
    q_label = f"{q * 100:.1f}".replace(".", "p")
    omega_label = (
        f"{omega:.0f}" if float(omega).is_integer() else str(omega).replace(".", "p")
    ) if omega_enabled else "off"
    rt_json = json.dumps(root_transforms or [], separators=(",", ":"))
    rt_hash = hashlib.sha1(rt_json.encode("utf-8")).hexdigest()[:8]
    return f"pal_{int(time.time() * 1000)}_{metric}_{palette}_q{q_label}_w{omega_label}_rt{rt_hash}"


def _metadata_size_bytes(metadata):
    total = 0
    for key, value in (metadata or {}).items():
        total += len(str(key).encode("utf-8"))
        total += len(str(value).encode("utf-8"))
    return total


def _palette_image_metadata(full_n, palette):
    return {
        "pix": str(full_n),
        "width": str(full_n),
        "height": str(full_n),
        "palette": str(palette),
    }


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    degree = params["degree"]
    full_n = params["N"]
    lores_n = params["lores_N"]
    times = params.get("times", 1)
    lores_bin_key = params["lores_bin_key"]
    metric = params.get("metric", "proximity")
    palette = params.get("palette", "inferno")
    solve_score_quantile = params.get("solve_score_quantile", 0.001)
    solve_score_omega = _validate_omega(params.get("solve_score_omega", 1.0))
    solve_score_omega_enabled = _validate_omega_enabled(params.get("solve_score_omega_enabled", True))
    root_transforms = root_program_for_run(params)["chain"]
    persistent = bool(params.get("persistent", False))

    try:
        q = float(solve_score_quantile)
        if not (0.001 <= q <= 0.05):
            raise ValueError()
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_quantile must be in [0.001, 0.05], got {solve_score_quantile}")
    if palette not in VALID_PALETTE_NAMES:
        raise RuntimeError(f"Invalid palette: {palette}")

    tmp_bin = "/tmp/palette_lores.bin"
    tmp_raw = "/tmp/palette_out.raw"
    tmp_jpeg = "/tmp/palette_out.jpeg"
    tmp_xforms = "/tmp/palette_xforms.json"
    tmp_scores = "/tmp/palette_scores.bin"
    tmp_bins = "/tmp/palette_bins.bin"

    try:
        # Download lores bin
        t0 = time.time()
        obj = s3.get_object(Bucket=BUCKET, Key=lores_bin_key)
        with open(tmp_bin, "wb") as f:
            for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                f.write(chunk)
        dl_ms = int((time.time() - t0) * 1000)

        # Build binary command
        quantile_lo = q
        quantile_hi = 1.0 - q
        cmd = [
            BINARY, tmp_bin, tmp_raw,
            f"--degree={degree}",
            f"--lores_n={lores_n}",
            f"--full_n={full_n}",
            f"--times={times}",
            f"--metric={metric}",
            f"--palette={palette}",
            f"--quantile_lo={quantile_lo}",
            f"--quantile_hi={quantile_hi}",
            f"--omega={solve_score_omega}",
            f"--omega_enabled={1 if solve_score_omega_enabled else 0}",
        ]
        if persistent:
            cmd.append(f"--scores_out={tmp_scores}")
            cmd.append(f"--palette_bins_out={tmp_bins}")
        if root_transforms:
            with open(tmp_xforms, "w") as xf:
                json.dump(root_transforms, xf)
            cmd.append(f"--root_xforms={tmp_xforms}")

        # Run binary
        t1 = time.time()
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
        compute_ms = int((time.time() - t1) * 1000)
        if result.returncode != 0:
            raise RuntimeError(f"solve_palette_debug failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)

        # Encode to JPEG via raw2jpeg
        t2 = time.time()
        env = imgpipe_env()
        enc_result = subprocess.run(
            [RAW2JPEG, tmp_raw, tmp_jpeg, "--quality=90"],
            capture_output=True, text=True, timeout=120, env=env,
        )
        encode_ms = int((time.time() - t2) * 1000)
        if enc_result.returncode != 0:
            raise RuntimeError(f"raw2jpeg failed: {enc_result.stderr.strip()}")

        created_at = _utc_now_iso()
        if persistent:
            palette_id = _palette_variant_id(metric, palette, q, solve_score_omega, solve_score_omega_enabled, root_transforms)
            prefix = f"renders/{job_id}/palettes/{palette_id}/"
            out_key = prefix + "image.jpeg"
            preview_key = prefix + "preview.png"
            score_key = prefix + f"score_{metric}.bin"
            palette_bins_key = prefix + "palette_bins.bin"
            meta_key = prefix + "meta.json"
        else:
            palette_id = None
            prefix = None
            score_key = None
            palette_bins_key = None
            meta_key = None
            # Delete stale cached preview so browser regenerates from new source
            preview_key = f"renders/{job_id}/preview_palette.png"
            try:
                s3.delete_object(Bucket=BUCKET, Key=preview_key)
            except Exception:
                pass
            out_key = f"renders/{job_id}/image_palette.jpeg"

        file_size = os.path.getsize(tmp_jpeg)
        s3_metadata = _palette_image_metadata(full_n, palette)
        metadata_size = _metadata_size_bytes(s3_metadata)
        if metadata_size > S3_USER_METADATA_LIMIT_BYTES:
            raise RuntimeError(
                f"Palette image metadata too large before upload: {metadata_size} bytes > "
                f"{S3_USER_METADATA_LIMIT_BYTES} limit"
            )
        with open(tmp_jpeg, "rb") as fh:
            s3.upload_fileobj(fh, BUCKET, out_key,
                              ExtraArgs={"ContentType": "image/jpeg", "Metadata": s3_metadata})

        # Generate preview eagerly (tabs are passive viewers, no lazy generation)
        tmp_preview = "/tmp/palette_preview.png"
        vt_path = "/opt/bin/vipsthumbnail"
        env = imgpipe_env()
        prev_result = subprocess.run(
            [vt_path, tmp_jpeg, "-s", "512x512", "-o", tmp_preview + "[strip]"],
            capture_output=True, text=True, timeout=30, env=env,
        )
        if prev_result.returncode != 0:
            raise RuntimeError(f"Preview generation failed: {prev_result.stderr.strip()}")
        with open(tmp_preview, "rb") as pfh:
            s3.upload_fileobj(pfh, BUCKET, preview_key,
                              ExtraArgs={"ContentType": "image/png", "CacheControl": CACHE_IMMUTABLE})
        try:
            os.remove(tmp_preview)
        except OSError:
            pass

        # Presign URL
        image_url = s3.generate_presigned_url(
            "get_object", Params={"Bucket": BUCKET, "Key": out_key},
            ExpiresIn=PRESIGN_EXPIRY,
        )

        if persistent:
            with open(tmp_scores, "rb") as sfh:
                s3.upload_fileobj(
                    sfh, BUCKET, score_key,
                    ExtraArgs={"ContentType": "application/octet-stream"},
                )
            with open(tmp_bins, "rb") as bfh:
                s3.upload_fileobj(
                    bfh, BUCKET, palette_bins_key,
                    ExtraArgs={"ContentType": "application/octet-stream"},
                )
            preview_url = s3.generate_presigned_url(
                "get_object", Params={"Bucket": BUCKET, "Key": preview_key},
                ExpiresIn=PRESIGN_EXPIRY,
            )
            meta_body = {
                "job_id": job_id,
                "palette_id": palette_id,
                "created_at": created_at,
                "display_name": f"{metric} q={(q*100):.1f}% {_omega_display(solve_score_omega_enabled, solve_score_omega)} {palette} {created_at}",
                "metric": metric,
                "palette": palette,
                "solve_score_quantile": q,
                "solve_score_omega": solve_score_omega,
                "solve_score_omega_enabled": solve_score_omega_enabled,
                "root_transforms": root_transforms or [],
                "degree": degree,
                "N": full_n,
                "lores_N": lores_n,
                "times": times,
                "using_pass": 0,
                "clip_lo": meta.get("clip_lo"),
                "clip_hi": meta.get("clip_hi"),
                "cuts_norm": meta.get("cuts_norm", []),
                "clip_fallback": meta.get("clip_fallback", False),
                "clip_fallback_reason": meta.get("clip_fallback_reason"),
                "file_size": file_size,
                "image_key": out_key,
                "preview_key": preview_key,
                "score_key": score_key,
                "palette_bins_key": palette_bins_key,
            }
            s3.put_object(
                Bucket=BUCKET,
                Key=meta_key,
                Body=json.dumps(meta_body),
                ContentType="application/json",
            )
        else:
            preview_url = None

        body = {
            "job_id": job_id,
            "out_key": out_key,
            "image_url": image_url,
            "width": full_n,
            "height": full_n,
            "metric": metric,
            "palette": palette,
            "solve_score_quantile": q,
            "solve_score_omega": solve_score_omega,
            "solve_score_omega_enabled": solve_score_omega_enabled,
            "lores_N": lores_n,
            "N": full_n,
            "times": times,
            "n_samples_used": meta.get("n_samples_used", lores_n * lores_n),
            "clip_lo": meta.get("clip_lo"),
            "clip_hi": meta.get("clip_hi"),
            "cuts_norm": meta.get("cuts_norm", []),
            "clip_fallback": meta.get("clip_fallback", False),
            "clip_fallback_reason": meta.get("clip_fallback_reason"),
            "dl_ms": dl_ms,
            "compute_ms": compute_ms,
            "encode_ms": encode_ms,
            "file_size": file_size,
            "using_pass": 0,
        }
        if persistent:
            body.update({
                "persistent": True,
                "palette_id": palette_id,
                "created_at": created_at,
                "display_name": f"{metric} q={(q*100):.1f}% {_omega_display(solve_score_omega_enabled, solve_score_omega)} {palette} {created_at}",
                "image_key": out_key,
                "preview_key": preview_key,
                "preview_url": preview_url,
                "score_key": score_key,
                "palette_bins_key": palette_bins_key,
                "meta_key": meta_key,
            })
        return ok_response(body)

    finally:
        for p in [tmp_bin, tmp_raw, tmp_jpeg, tmp_xforms, tmp_scores, tmp_bins]:
            try:
                os.remove(p)
            except OSError:
                pass
