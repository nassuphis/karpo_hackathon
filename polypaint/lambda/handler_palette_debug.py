"""
Palette debug Lambda — generates a solve-score palette JPEG from lores data.

Synchronous, produces one S3 artifact: renders/{job_id}/image_palette.jpeg.
No DDB writes. No render orchestrator. No active render run.
"""
import json
import os
import subprocess
import time

import boto3

from shared import BUCKET, parse_body, ok_response, imgpipe_env

s3 = boto3.client("s3")
BINARY = os.path.join(os.path.dirname(__file__), "solve_palette_debug")
RAW2JPEG = os.path.join(os.path.dirname(__file__), "raw2jpeg")
PRESIGN_EXPIRY = 3600


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
    root_transforms = params.get("root_transforms", [])

    try:
        q = float(solve_score_quantile)
        if not (0.001 <= q <= 0.05):
            raise ValueError()
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_quantile must be in [0.001, 0.05], got {solve_score_quantile}")

    tmp_bin = "/tmp/palette_lores.bin"
    tmp_raw = "/tmp/palette_out.raw"
    tmp_jpeg = "/tmp/palette_out.jpeg"
    tmp_xforms = "/tmp/palette_xforms.json"

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
        ]
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

        # Upload to S3
        # Delete stale cached preview so browser regenerates from new source
        preview_key = f"renders/{job_id}/preview_palette.png"
        try:
            s3.delete_object(Bucket=BUCKET, Key=preview_key)
        except Exception:
            pass

        out_key = f"renders/{job_id}/image_palette.jpeg"
        file_size = os.path.getsize(tmp_jpeg)
        s3_metadata = {
            "width": str(full_n),
            "height": str(full_n),
            "metric": metric,
            "palette": palette,
            "solve_score_quantile": str(q),
            "lores_n": str(lores_n),
            "full_n": str(full_n),
            "times": str(times),
            "using_pass": "0",
            "clip_lo": str(meta.get("clip_lo", "")),
            "clip_hi": str(meta.get("clip_hi", "")),
            "clip_fallback": str(meta.get("clip_fallback", False)).lower(),
            "clip_fallback_reason": str(meta.get("clip_fallback_reason", "none")),
        }
        with open(tmp_jpeg, "rb") as fh:
            s3.upload_fileobj(fh, BUCKET, out_key,
                              ExtraArgs={"ContentType": "image/jpeg", "Metadata": s3_metadata})

        # Generate preview eagerly (tabs are passive viewers, no lazy generation)
        tmp_preview = "/tmp/palette_preview.png"
        try:
            vt_path = "/opt/bin/vipsthumbnail"
            env = imgpipe_env()
            prev_result = subprocess.run(
                [vt_path, tmp_jpeg, "-s", "512x512", "-o", tmp_preview + "[strip]"],
                capture_output=True, text=True, timeout=30, env=env,
            )
            if prev_result.returncode == 0 and os.path.exists(tmp_preview):
                with open(tmp_preview, "rb") as pfh:
                    s3.upload_fileobj(pfh, BUCKET, preview_key,
                                      ExtraArgs={"ContentType": "image/png"})
        except Exception:
            pass  # preview generation is best-effort, don't fail the whole action
        finally:
            try:
                os.remove(tmp_preview)
            except OSError:
                pass

        # Presign URL
        image_url = s3.generate_presigned_url(
            "get_object", Params={"Bucket": BUCKET, "Key": out_key},
            ExpiresIn=PRESIGN_EXPIRY,
        )

        return ok_response({
            "job_id": job_id,
            "out_key": out_key,
            "image_url": image_url,
            "width": full_n,
            "height": full_n,
            "metric": metric,
            "palette": palette,
            "solve_score_quantile": q,
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
        })

    finally:
        for p in [tmp_bin, tmp_raw, tmp_jpeg, tmp_xforms]:
            try:
                os.remove(p)
            except OSError:
                pass
