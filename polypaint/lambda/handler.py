"""
Lambda handler for polypaint render pipeline.

Routes:
  POST /render               — orchestrate server-side image rendering
  POST /compute-render-stripe — per-stripe worker (compute roots + render PNG)
  POST /reduce-pair          — merge two PNGs via additive blending
  POST /encode-upload        — encode final PNG to JPEG/PNG and upload
"""
import json
import os
import subprocess
import time
import uuid

import boto3
from botocore.config import Config

BUCKET = os.environ.get("BUCKET", "polypaint")
s3 = boto3.client("s3")
# Large connection pool for parallel Lambda invocations (stripe fan-out + tree-reduce)
lambda_client = boto3.client("lambda", config=Config(max_pool_connections=200))
SWEEP = os.path.join(os.path.dirname(__file__), "sweep")
LORES_VIEWPORT = os.path.join(os.path.dirname(__file__), "lores_viewport")
IMGPIPE = os.path.join(os.path.dirname(__file__), "imgpipe")

# Environment for imgpipe (needs libvips from /opt/lib)
def _imgpipe_env():
    env = dict(os.environ)
    ld = env.get("LD_LIBRARY_PATH", "")
    if "/opt/lib" not in ld:
        env["LD_LIBRARY_PATH"] = "/opt/lib:" + ld
    return env
PRESIGN_EXPIRY = 3600  # 1 hour
FUNCTION_NAME = os.environ.get("AWS_LAMBDA_FUNCTION_NAME", "polypaint-solver")


def handler(event, context):
    path = event.get("rawPath", event.get("path", "/"))
    if path.endswith("/compute-render-stripe"):
        return handle_compute_render_stripe(event)
    elif path.endswith("/reduce-pair"):
        return handle_reduce_pair(event)
    elif path.endswith("/encode-upload"):
        return handle_encode_upload(event)
    else:
        return handle_render_v2(event)


def parse_body(event):
    """Parse request body from various invocation formats."""
    if isinstance(event.get("body"), str):
        return json.loads(event["body"])
    elif "body" in event and event["body"] is not None:
        return event["body"]
    return event


def ok_response(body):
    return {
        "statusCode": 200,
        "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
        "body": json.dumps(body),
    }


def err_response(code, msg):
    return {
        "statusCode": code,
        "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
        "body": json.dumps({"error": msg}),
    }



# ---- Render pipeline v2: separated compute + libvips image ----


def handle_compute_render_stripe(event):
    """Per-stripe worker: compute roots via sweep binary, render to raw via imgpipe.
    1. Run sweep --mode=grid to produce /tmp/stripe.bin (f32 root positions)
    2. Run imgpipe --roots2image to produce /tmp/stripe.raw (12-byte header + pixels)
    3. Upload stripe.raw to S3
    4. Return metadata
    """
    params = parse_body(event)
    job_id = params["job_id"]
    stripe_idx = params["stripe_idx"]
    width = params["width"]
    height = params["height"]
    degree = params["degree"]

    bin_path = "/tmp/stripe.bin"
    raw_path = "/tmp/stripe.raw"

    # Step 1: compute roots
    spec = {
        "mode": "grid",
        "function": params["function"],
        "n1": params["n1"],
        "n2": params["n2"],
        "i1_start": params["i1_start"],
        "i1_end": params["i1_end"],
        "match_roots": False,  # no need for root tracking in render
    }
    t0 = time.time()
    result = subprocess.run(
        [SWEEP, bin_path],
        input=json.dumps(spec),
        capture_output=True, text=True,
        timeout=840
    )
    if result.returncode != 0:
        raise RuntimeError(f"sweep failed: {result.stderr.strip()}")
    compute_meta = json.loads(result.stdout)
    compute_us = int((time.time() - t0) * 1e6)

    # Step 2: render roots to raw image via imgpipe
    t1 = time.time()
    color_mode = params.get("color", "rainbow")
    match_mode = params.get("match", "none")
    palette = params.get("palette", "inferno")
    constant_color = params.get("constant_color", "ffffff")
    imgpipe_args = [
        IMGPIPE, "--roots2image", bin_path, raw_path,
        f"--width={width}", f"--height={height}",
        f"--center_re={params['center_re']}", f"--center_im={params['center_im']}",
        f"--scale={params['scale']}", f"--degree={degree}",
        f"--color={color_mode}", f"--match={match_mode}",
        f"--palette={palette}", f"--constant_color={constant_color}",
    ]
    result = subprocess.run(
        imgpipe_args,
        capture_output=True, text=True,
        timeout=300, env=_imgpipe_env()
    )
    if result.returncode != 0:
        raise RuntimeError(f"imgpipe roots2image failed: {result.stderr.strip()}")
    render_meta = json.loads(result.stdout)
    render_us = int((time.time() - t1) * 1e6)

    # Step 3: upload raw image to S3
    s3_key = f"renders/{job_id}/stripe_{stripe_idx}.raw"
    with open(raw_path, "rb") as f:
        raw_data = f.read()
    s3.put_object(Bucket=BUCKET, Key=s3_key,
                  Body=raw_data, ContentType="application/octet-stream")

    # Cleanup tmp
    for p in [bin_path, raw_path]:
        try:
            os.remove(p)
        except OSError:
            pass

    return ok_response({
        "stripe_idx": stripe_idx,
        "s3_key": s3_key,
        "raw_size": len(raw_data),
        "compute_us": compute_us,
        "render_us": render_us,
        "roots_plotted": render_meta["roots_plotted"],
        "roots_clipped": render_meta["roots_clipped"],
        "n_t": compute_meta["n_t"],
        "degree": compute_meta["degree"],
        "avg_iterations": compute_meta["avg_iterations"],
    })


def handle_reduce_pair(event):
    """Merge two raw images via imgpipe --reduce. Used by tree-reduce fan-out.
    Input: {job_id, left_key, right_key, out_key}
    Downloads left and right from S3, merges, uploads result.
    """
    params = parse_body(event)
    left_key = params["left_key"]
    right_key = params["right_key"]
    out_key = params["out_key"]

    left_path = "/tmp/left.raw"
    right_path = "/tmp/right.raw"
    out_path = "/tmp/merged.raw"

    # Download both images
    obj = s3.get_object(Bucket=BUCKET, Key=left_key)
    with open(left_path, "wb") as f:
        f.write(obj["Body"].read())
    obj = s3.get_object(Bucket=BUCKET, Key=right_key)
    with open(right_path, "wb") as f:
        f.write(obj["Body"].read())

    # Merge
    gamma = params.get("gamma", 2.2)
    reduce_cmd = [IMGPIPE, "--reduce", left_path, right_path, out_path,
                  f"--gamma={gamma}"]
    result = subprocess.run(
        reduce_cmd, capture_output=True, text=True,
        timeout=120, env=_imgpipe_env()
    )
    if result.returncode != 0:
        raise RuntimeError(f"imgpipe reduce failed: {result.stderr.strip()}")

    # Upload result
    with open(out_path, "rb") as f:
        raw_data = f.read()
    s3.put_object(Bucket=BUCKET, Key=out_key,
                  Body=raw_data, ContentType="application/octet-stream")

    for p in [left_path, right_path, out_path]:
        try:
            os.remove(p)
        except OSError:
            pass

    return ok_response({"out_key": out_key, "size": len(raw_data)})


def handle_encode_upload(event):
    """Encode a raw image in S3 to JPEG/PNG and upload the result.
    Runs on a worker Lambda so the coordinator never touches image data.
    Input: {raw_key, out_key, format, quality}
    Returns: {out_key, file_size, image_url}
    """
    params = parse_body(event)
    raw_key = params["raw_key"]
    out_key = params["out_key"]
    fmt = params.get("format", "jpeg")
    quality = params.get("quality", 90)

    in_path = "/tmp/encode_in.raw"
    ext = "jpeg" if fmt != "png" else "png"
    out_path = f"/tmp/encode_out.{ext}"

    # Download source raw image
    obj = s3.get_object(Bucket=BUCKET, Key=raw_key)
    with open(in_path, "wb") as f:
        f.write(obj["Body"].read())

    # Encode
    encode_args = [IMGPIPE, "--encode", in_path, out_path]
    if ext == "jpeg":
        encode_args.append(f"--quality={quality}")
    result = subprocess.run(encode_args, capture_output=True, text=True,
                            timeout=300, env=_imgpipe_env())
    if result.returncode != 0:
        raise RuntimeError(f"imgpipe encode failed: {result.stderr.strip()}")
    encode_meta = json.loads(result.stdout)

    # Upload
    content_type = "image/jpeg" if ext == "jpeg" else "image/png"
    with open(out_path, "rb") as f:
        s3.put_object(Bucket=BUCKET, Key=out_key,
                      Body=f, ContentType=content_type)

    image_url = s3.generate_presigned_url(
        "get_object",
        Params={"Bucket": BUCKET, "Key": out_key},
        ExpiresIn=PRESIGN_EXPIRY)

    for p in [in_path, out_path]:
        try:
            os.remove(p)
        except OSError:
            pass

    return ok_response({
        "out_key": out_key,
        "file_size": encode_meta["file_size"],
        "image_url": image_url,
    })


def handle_render_v2(event):
    """Render pipeline v2: lores_viewport + parallel compute+render stripes + tree-reduce.
    Uses libvips (via imgpipe binary) instead of Pillow."""
    import concurrent.futures

    params = parse_body(event)
    job_id = params.get("job_id", "render_" + str(uuid.uuid4())[:8])
    fmt = params.get("format", "jpeg").lower()
    quality = params.get("quality", 90)
    width = params.get("width", 4096)
    height = params.get("height", 4096)
    func_name = params.get("function", "giga_5")
    n1 = params.get("n1", 100)
    n2 = params.get("n2", 100)
    n_stripes = params.get("n_stripes", 1)
    color_mode = params.get("color", "rainbow")
    match_mode = params.get("match", "none")
    palette = params.get("palette", "inferno")
    constant_color = params.get("constant_color", "ffffff")
    gamma = params.get("gamma", 2.2)

    # Auto-decide stripe count
    if n_stripes <= 1 and n1 * n2 > 50000:
        n_stripes = min(max(n1 * n2 // 50000, 2), 10)
    n_stripes = max(1, min(n_stripes, 500))

    # Phase 1: viewport via lores_viewport binary
    t_vp = time.time()
    auto_scale = params.get("auto_scale", True)
    quantile = params.get("quantile", 0.0)
    shim = params.get("shim", 0.05)
    if auto_scale:
        vp_spec = json.dumps({"function": func_name, "n1": n1, "n2": n2,
                               "quantile": quantile, "shim": shim})
        vp_result = subprocess.run(
            [LORES_VIEWPORT],
            input=vp_spec,
            capture_output=True, text=True,
            timeout=120
        )
        if vp_result.returncode != 0:
            raise RuntimeError(f"lores_viewport failed: {vp_result.stderr.strip()}")
        vp = json.loads(vp_result.stdout)
        center_re = vp["center_re"]
        center_im = vp["center_im"]
        # lores_viewport computes scale for 4096x4096 reference
        scale = vp["scale"] * min(width, height) / 4096.0
        degree = vp["degree"]
        viewport_info = {
            "center_re": center_re, "center_im": center_im,
            "scale": scale, "scale_ref": vp["scale"],
            "n_roots": vp.get("n_roots", 0),
            "q_re": vp.get("q_re"), "q_im": vp.get("q_im"),
            "quantile": quantile, "shim": shim,
        }
    else:
        center_re = params.get("center_re", 0)
        center_im = params.get("center_im", 0)
        scale = params.get("scale", 1.0)
        # Probe degree by running lores_viewport anyway (fast)
        vp_spec = json.dumps({"function": func_name, "n1": 2, "n2": 2})
        vp_result = subprocess.run(
            [LORES_VIEWPORT], input=vp_spec,
            capture_output=True, text=True, timeout=30
        )
        degree = json.loads(vp_result.stdout)["degree"] if vp_result.returncode == 0 else 25
        viewport_info = {
            "center_re": center_re, "center_im": center_im,
            "scale": scale, "manual": True,
        }
    viewport_us = int((time.time() - t_vp) * 1e6)

    if n_stripes <= 1:
        # Single-pass: compute roots + render in one invocation on this Lambda
        bin_path = "/tmp/stripe.bin"
        raw_path = "/tmp/stripe.raw"
        final_path = "/tmp/final." + ("jpeg" if fmt != "png" else "png")

        spec = {
            "mode": "grid",
            "function": func_name,
            "n1": n1, "n2": n2,
            "match_roots": False,
        }
        result = subprocess.run(
            [SWEEP, bin_path],
            input=json.dumps(spec),
            capture_output=True, text=True,
            timeout=840
        )
        if result.returncode != 0:
            raise RuntimeError(f"sweep failed: {result.stderr.strip()}")
        compute_meta = json.loads(result.stdout)

        result = subprocess.run(
            [IMGPIPE, "--roots2image", bin_path, raw_path,
             f"--width={width}", f"--height={height}",
             f"--center_re={center_re}", f"--center_im={center_im}",
             f"--scale={scale}", f"--degree={degree}",
             f"--color={color_mode}", f"--match={match_mode}",
             f"--palette={palette}", f"--constant_color={constant_color}"],
            capture_output=True, text=True,
            timeout=300, env=_imgpipe_env()
        )
        if result.returncode != 0:
            raise RuntimeError(f"imgpipe roots2image failed: {result.stderr.strip()}")
        render_meta = json.loads(result.stdout)

        # Encode to final format
        encode_args = [IMGPIPE, "--encode", raw_path, final_path]
        if fmt != "png":
            encode_args.append(f"--quality={quality}")
        result = subprocess.run(encode_args, capture_output=True, text=True,
                                timeout=120, env=_imgpipe_env())
        if result.returncode != 0:
            raise RuntimeError(f"imgpipe encode failed: {result.stderr.strip()}")
        encode_meta = json.loads(result.stdout)

        # Upload
        ext = "jpeg" if fmt != "png" else "png"
        content_type = "image/jpeg" if ext == "jpeg" else "image/png"
        image_key = f"renders/{job_id}/image.{ext}"
        with open(final_path, "rb") as f:
            image_bytes = f.read()
        s3.put_object(Bucket=BUCKET, Key=image_key,
                      Body=image_bytes, ContentType=content_type)
        image_url = s3.generate_presigned_url(
            "get_object",
            Params={"Bucket": BUCKET, "Key": image_key},
            ExpiresIn=PRESIGN_EXPIRY)

        for p in [bin_path, raw_path, final_path]:
            try:
                os.remove(p)
            except OSError:
                pass

        return ok_response({
            "job_id": job_id, "status": "complete",
            "pipeline": "libvips",
            "width": width, "height": height,
            "degree": degree, "n1": n1, "n2": n2,
            "function": func_name,
            "roots_plotted": render_meta["roots_plotted"],
            "roots_clipped": render_meta["roots_clipped"],
            "elapsed_us": compute_meta["elapsed_us"],
            "avg_iterations": compute_meta["avg_iterations"],
            "format": ext, "file_size": encode_meta["file_size"],
            "image_url": image_url, "image_key": image_key,
        })

    # --- Parallel striped render (v2) ---
    # Coordinator is thin: all heavy work (compute, render, reduce, encode)
    # runs on worker Lambdas.  Coordinator only makes invoke() calls.

    # Phase 2: fan-out compute+render stripes
    rows_per = n1 // n_stripes
    stripes = []
    for s in range(n_stripes):
        i1_start = s * rows_per
        i1_end = (s + 1) * rows_per if s < n_stripes - 1 else n1
        stripes.append((s, i1_start, i1_end))

    def invoke_cr_stripe(stripe_info):
        idx, start, end = stripe_info
        payload = {
            "rawPath": "/compute-render-stripe",
            "body": json.dumps({
                "job_id": job_id,
                "stripe_idx": idx,
                "function": func_name,
                "n1": n1, "n2": n2,
                "i1_start": start, "i1_end": end,
                "width": width, "height": height,
                "degree": degree,
                "center_re": center_re,
                "center_im": center_im,
                "scale": scale,
                "color": color_mode,
                "match": match_mode,
                "palette": palette,
                "constant_color": constant_color,
            }),
        }
        resp = lambda_client.invoke(
            FunctionName=FUNCTION_NAME,
            InvocationType="RequestResponse",
            Payload=json.dumps(payload),
        )
        result = json.loads(resp["Payload"].read())
        if result.get("statusCode") != 200:
            raise RuntimeError(f"Stripe {idx} failed: {result.get('body', '')}")
        return json.loads(result["body"])

    t0 = time.time()
    with concurrent.futures.ThreadPoolExecutor(max_workers=n_stripes) as pool:
        results = list(pool.map(invoke_cr_stripe, stripes))
    compute_wall_us = int((time.time() - t0) * 1e6)

    # Phase 3: tree-reduce via parallel Lambda invocations
    t_reduce = time.time()
    all_temp_keys = []  # track intermediate keys for cleanup

    keys = [f"renders/{job_id}/stripe_{s}.raw" for s in range(n_stripes)]
    round_num = 0

    def invoke_reduce_pair(pair_info):
        left_key, right_key, out_key = pair_info
        payload = {
            "rawPath": "/reduce-pair",
            "body": json.dumps({
                "job_id": job_id,
                "left_key": left_key,
                "right_key": right_key,
                "out_key": out_key,
                "gamma": gamma,
            }),
        }
        resp = lambda_client.invoke(
            FunctionName=FUNCTION_NAME,
            InvocationType="RequestResponse",
            Payload=json.dumps(payload),
        )
        result = json.loads(resp["Payload"].read())
        if result.get("statusCode") != 200:
            raise RuntimeError(f"reduce-pair failed: {result.get('body', '')}")
        return json.loads(result["body"])

    while len(keys) > 1:
        pairs = []
        next_keys = []
        for i in range(0, len(keys), 2):
            if i + 1 < len(keys):
                out_key = f"renders/{job_id}/merge_{round_num}_{i // 2}.raw"
                pairs.append((keys[i], keys[i + 1], out_key))
                next_keys.append(out_key)
                all_temp_keys.append(out_key)
            else:
                next_keys.append(keys[i])

        with concurrent.futures.ThreadPoolExecutor(max_workers=len(pairs)) as pool:
            list(pool.map(invoke_reduce_pair, pairs))

        keys = next_keys
        round_num += 1

    reduce_us = int((time.time() - t_reduce) * 1e6)

    # Phase 4: encode + upload via worker Lambda (coordinator touches no image data)
    t_encode = time.time()
    ext = "jpeg" if fmt != "png" else "png"
    image_key = f"renders/{job_id}/image.{ext}"
    encode_payload = {
        "rawPath": "/encode-upload",
        "body": json.dumps({
            "raw_key": keys[0],
            "out_key": image_key,
            "format": ext,
            "quality": quality,
        }),
    }
    resp = lambda_client.invoke(
        FunctionName=FUNCTION_NAME,
        InvocationType="RequestResponse",
        Payload=json.dumps(encode_payload),
    )
    encode_result = json.loads(resp["Payload"].read())
    if encode_result.get("statusCode") != 200:
        raise RuntimeError(f"encode-upload failed: {encode_result.get('body', '')}")
    encode_body = json.loads(encode_result["body"])
    image_url = encode_body["image_url"]
    file_size = encode_body["file_size"]
    encode_us = int((time.time() - t_encode) * 1e6)

    # Aggregate stats
    total_plotted = sum(r["roots_plotted"] for r in results)
    total_clipped = sum(r["roots_clipped"] for r in results)
    total_compute = sum(r["compute_us"] for r in results)
    total_render = sum(r["render_us"] for r in results)
    total_steps = sum(r["n_t"] for r in results)
    avg_iters = (sum(r["avg_iterations"] * r["n_t"] for r in results)
                 / total_steps if total_steps > 0 else 0)

    # Phase 5: cleanup temp S3 keys (batch delete — single API call)
    t_cleanup = time.time()
    cleanup_keys = [f"renders/{job_id}/stripe_{s}.raw" for s in range(n_stripes)]
    cleanup_keys.extend(all_temp_keys)
    if keys[0] != image_key:
        cleanup_keys.append(keys[0])
    # S3 batch delete: up to 1000 objects per call
    for i in range(0, len(cleanup_keys), 1000):
        batch = cleanup_keys[i:i + 1000]
        try:
            s3.delete_objects(Bucket=BUCKET, Delete={
                "Objects": [{"Key": k} for k in batch],
                "Quiet": True,
            })
        except Exception:
            pass
    cleanup_us = int((time.time() - t_cleanup) * 1e6)

    return ok_response({
        "job_id": job_id, "status": "complete",
        "pipeline": "libvips",
        "width": width, "height": height,
        "degree": degree, "n1": n1, "n2": n2,
        "function": func_name,
        "roots_plotted": total_plotted,
        "roots_clipped": total_clipped,
        "n_stripes": n_stripes,
        "avg_iterations": avg_iters,
        "format": ext, "file_size": file_size,
        "image_url": image_url, "image_key": image_key,
        "viewport": viewport_info,
        "color": color_mode, "match": match_mode,
        "palette": palette, "gamma": gamma,
        # Per-phase timing (microseconds)
        "timing": {
            "viewport_us": viewport_us,
            "compute_wall_us": compute_wall_us,
            "reduce_us": reduce_us,
            "reduce_rounds": round_num,
            "encode_us": encode_us,
            "cleanup_us": cleanup_us,
            "total_compute_us": total_compute,
            "total_render_us": total_render,
        },
    })
