"""
Storage Lambda handler — S3 metadata operations + DynamoDB status tracking.

Routes:
  POST /list           — list all computed jobs with metadata
  POST /delete         — delete all S3 objects for a job
  POST /save-metadata  — save calc.json to S3
  POST /cleanup        — delete a list of S3 keys
  POST /clean-render   — delete render artifacts (.raw/.jpeg/.png) for a job, preserving .bin
  POST /check-keys     — check which S3 keys exist (for polling render completion)
  POST /check-status   — query DynamoDB for task completion counts (replaces check-keys)
  POST /presign        — generate a presigned URL for an S3 key
"""
import json
import time

import boto3

from shared import BUCKET, JOBS_TABLE, PRESIGN_EXPIRY, parse_body, ok_response, _get_ddb

s3 = boto3.client("s3")


def handler(event, context):
    path = event.get("rawPath", event.get("path", "/"))
    if path.endswith("/list"):
        return handle_list(event)
    elif path.endswith("/delete"):
        return handle_delete(event)
    elif path.endswith("/save-metadata"):
        return handle_save_metadata(event)
    elif path.endswith("/cleanup"):
        return handle_cleanup(event)
    elif path.endswith("/clean-render"):
        return handle_clean_render(event)
    elif path.endswith("/check-keys"):
        return handle_check_keys(event)
    elif path.endswith("/check-status"):
        return handle_check_status(event)
    elif path.endswith("/detail"):
        return handle_detail(event)
    elif path.endswith("/presign"):
        return handle_presign(event)
    elif path.endswith("/list-prefix"):
        return handle_list_prefix(event)
    elif path.endswith("/head-keys"):
        return handle_head_keys(event)
    elif path.endswith("/delete-task"):
        return handle_delete_task(event)
    return {
        "statusCode": 400,
        "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
        "body": json.dumps({"error": f"Unknown route: {path}"}),
    }


def handle_list(event):
    """List all computed results in S3.
    Uses Delimiter='/' to get just folder names (O(n_jobs)),
    then reads calc.json per job in parallel for metadata.
    """
    import concurrent.futures

    t0 = time.time()

    # List folder prefixes under renders/ — O(n_jobs), not O(all_objects)
    job_ids = []
    paginator = s3.get_paginator('list_objects_v2')
    for page in paginator.paginate(Bucket=BUCKET, Prefix='renders/',
                                   Delimiter='/'):
        for prefix in page.get('CommonPrefixes', []):
            # prefix['Prefix'] = 'renders/job_id/'
            job_id = prefix['Prefix'].split('/')[1]
            if job_id:
                job_ids.append(job_id)

    # Read calc.json for each job (parallelized)
    def read_calc(job_id):
        entry = {"job_id": job_id}
        try:
            obj = s3.get_object(Bucket=BUCKET,
                                Key=f"renders/{job_id}/calc.json")
            calc = json.loads(obj["Body"].read())
            entry["function"] = calc.get("function", "?")
            entry["degree"] = calc.get("degree", 0)
            entry["N"] = calc.get("N", calc.get("n1", 0))
            entry["n1"] = calc.get("n1", entry["N"])  # backward compat
            entry["n_stripes"] = calc.get("n_stripes", 0)
            entry["times"] = calc.get("times", 1)
            # Pipeline info
            pipeline = calc.get("pipeline", {})
            entry["param_transforms"] = pipeline.get("param_transforms", [])
            entry["param_transforms_display"] = pipeline.get("param_transforms_display", [])
            entry["coeff_transforms"] = pipeline.get("coeff_transforms", [])
            entry["pipeline"] = pipeline
            # Compute total bin size and root count from chunk/stripe metadata
            stripes = calc.get("chunks", calc.get("stripes", []))
            entry["total_size"] = sum(s.get("bin_size", 0) for s in stripes)
            entry["total_size"] += calc.get("total_coeffs_size", 0)
            # total_roots: stored directly, or derive from stripe bin sizes (8 bytes per root)
            entry["total_roots"] = calc.get("total_roots",
                sum(s.get("bin_size", 0) for s in stripes) // 8)
        except Exception:
            entry["function"] = "?"
            entry["total_size"] = 0

        # Check for preview/image with lightweight HEAD requests
        # Check .png (new) then .jpg (legacy) for preview
        preview_key = None
        if _key_exists(f"renders/{job_id}/preview.png"):
            preview_key = f"renders/{job_id}/preview.png"
        elif _key_exists(f"renders/{job_id}/preview.jpg"):
            preview_key = f"renders/{job_id}/preview.jpg"
        entry["has_preview"] = preview_key is not None
        if preview_key:
            entry["preview_url"] = s3.generate_presigned_url(
                "get_object",
                Params={"Bucket": BUCKET, "Key": preview_key},
                ExpiresIn=PRESIGN_EXPIRY)

        entry["has_image"] = (_key_exists(f"renders/{job_id}/image.jpeg")
                              or _key_exists(f"renders/{job_id}/image.png"))

        return entry

    with concurrent.futures.ThreadPoolExecutor(max_workers=20) as pool:
        results = list(pool.map(read_calc, job_ids))

    # Sort by job_id descending (job_ids contain timestamps)
    results.sort(key=lambda r: r["job_id"], reverse=True)

    return ok_response({
        "results": results,
        "count": len(results),
        "list_us": int((time.time() - t0) * 1e6),
    })


def _key_exists(key):
    """Check if an S3 key exists via HEAD (fast, no data transfer)."""
    try:
        s3.head_object(Bucket=BUCKET, Key=key)
        return True
    except Exception:
        return False


def handle_delete(event):
    """Delete all S3 objects for a given job_id."""
    params = parse_body(event)
    job_id = params["job_id"]

    # List all objects for this job
    prefix = f"renders/{job_id}/"
    objects = []
    paginator = s3.get_paginator('list_objects_v2')
    for page in paginator.paginate(Bucket=BUCKET, Prefix=prefix):
        objects.extend(page.get('Contents', []))

    if not objects:
        return ok_response({"job_id": job_id, "deleted": 0})

    # Delete in batches of 1000
    total_deleted = 0
    errors = []
    for i in range(0, len(objects), 1000):
        batch = objects[i:i + 1000]
        resp = s3.delete_objects(
            Bucket=BUCKET,
            Delete={"Objects": [{"Key": obj["Key"]} for obj in batch]},
        )
        total_deleted += len(resp.get("Deleted", []))
        for err in resp.get("Errors", []):
            errors.append(f"{err['Key']}: {err['Code']}")

    result = {"job_id": job_id, "deleted": total_deleted}
    if errors:
        result["errors"] = errors
    return ok_response(result)


def handle_list_prefix(event):
    """List S3 keys under a prefix, optionally filtered by suffix.
    Input: {prefix, suffix (optional), delimiter (optional), max_keys (optional, default 1000)}
    When delimiter is set, returns {prefixes: [...]} (CommonPrefixes) instead of keys.
    Returns: {keys: [...]} or {prefixes: [...]}
    """
    params = parse_body(event)
    prefix = params["prefix"]
    suffix = params.get("suffix", "")
    delimiter = params.get("delimiter", "")
    max_keys = params.get("max_keys", 1000)

    paginator = s3.get_paginator('list_objects_v2')
    paginate_kwargs = {"Bucket": BUCKET, "Prefix": prefix}
    if delimiter:
        paginate_kwargs["Delimiter"] = delimiter

    if delimiter:
        # Return CommonPrefixes (folder-level listing)
        prefixes = []
        for page in paginator.paginate(**paginate_kwargs):
            for cp in page.get('CommonPrefixes', []):
                prefixes.append(cp['Prefix'])
                if len(prefixes) >= max_keys:
                    break
            if len(prefixes) >= max_keys:
                break
        return ok_response({"prefixes": prefixes, "count": len(prefixes)})
    else:
        keys = []
        for page in paginator.paginate(**paginate_kwargs):
            for obj in page.get('Contents', []):
                if not suffix or obj['Key'].endswith(suffix):
                    keys.append(obj['Key'])
                    if len(keys) >= max_keys:
                        break
            if len(keys) >= max_keys:
                break
        return ok_response({"keys": keys, "count": len(keys)})


def handle_check_keys(event):
    """Check how many expected S3 keys exist under a prefix.
    Input: {prefix: "renders/jobid/", expected: 4000, suffix: ".raw"}
    Uses S3 list (fast) instead of per-key HEAD.
    Kept for backward compatibility — prefer /check-status for new code.
    """
    params = parse_body(event)
    prefix = params["prefix"]
    expected = params["expected"]
    suffix = params.get("suffix", ".raw")

    found = 0
    paginator = s3.get_paginator('list_objects_v2')
    for page in paginator.paginate(Bucket=BUCKET, Prefix=prefix):
        for obj in page.get('Contents', []):
            if obj['Key'].endswith(suffix):
                found += 1

    return ok_response({"found": found, "expected": expected, "done": found >= expected})


def handle_check_status(event):
    """Query DynamoDB for task completion status.
    Input: {job_id, task_prefix, expected}
    task_prefix examples: "stripe_" (render), "merge_0_" (merge round 0)
    Returns: {done, errors, error_details, total, expected, complete}
    """
    params = parse_body(event)
    job_id = params["job_id"]
    task_prefix = params["task_prefix"]
    expected = params["expected"]

    return_ids = params.get("return_ids", False)

    ddb = _get_ddb()
    done = 0
    found_ids = []  # all task_ids seen in DynamoDB (when return_ids=True)
    error_details = []
    stuck_tasks = []  # tasks in non-terminal status, with their actual status
    status_counts = {}  # track all statuses: started, tiles_read, tiles_merged, done, error
    results = []  # result_data from completed tasks (for sweep metadata)
    kwargs = {
        "TableName": JOBS_TABLE,
        "KeyConditionExpression": "job_id = :jid AND begins_with(task_id, :pfx)",
        "ExpressionAttributeValues": {
            ":jid": {"S": job_id},
            ":pfx": {"S": task_prefix},
        },
        "ProjectionExpression": "task_id, task_status, error_msg, result_data",
    }
    while True:
        resp = ddb.query(**kwargs)
        for item in resp["Items"]:
            status = item["task_status"]["S"]
            status_counts[status] = status_counts.get(status, 0) + 1
            if return_ids:
                found_ids.append(item["task_id"]["S"])
            if status == "done":
                done += 1
                rd = item.get("result_data", {}).get("S")
                if rd:
                    try:
                        results.append(json.loads(rd))
                    except Exception:
                        pass
            elif status == "error":
                error_details.append({
                    "task_id": item["task_id"]["S"],
                    "error_msg": item.get("error_msg", {}).get("S", "unknown"),
                })
            else:
                stuck_tasks.append({
                    "task_id": item["task_id"]["S"],
                    "status": status,
                })
        if "LastEvaluatedKey" not in resp:
            break
        kwargs["ExclusiveStartKey"] = resp["LastEvaluatedKey"]

    total = done + len(error_details)
    resp_body = {
        "done": done,
        "errors": len(error_details),
        "error_details": error_details[:20],
        "stuck": stuck_tasks[:50],
        "status_counts": status_counts,
        "total": total,
        "expected": expected,
        "complete": total >= expected,
    }
    if results:
        resp_body["results"] = results
    if return_ids:
        resp_body["found_ids"] = found_ids
    return ok_response(resp_body)


def handle_clean_render(event):
    """Delete intermediate render artifacts for a job, preserving final images.
    Deletes: pix_*, raw_*, tile_*, bilevel_t*, coeff_t* (intermediates)
    Preserves: image.jpeg, image.png, image_bilevel.tif, *_preview.png, calc.json, *.bin
    Also clears DynamoDB status entries for the job.
    """
    params = parse_body(event)
    job_id = params["job_id"]
    prefix = f"renders/{job_id}/"

    # Only delete known intermediate file prefixes
    intermediate_prefixes = ['pix_', 'raw_', 'tile_', 'bilevel_t', 'coeff_t']
    objects = []
    paginator = s3.get_paginator('list_objects_v2')
    for rp in intermediate_prefixes:
        for page in paginator.paginate(Bucket=BUCKET, Prefix=prefix + rp):
            for obj in page.get('Contents', []):
                objects.append(obj)

    total_deleted = 0
    if objects:
        for i in range(0, len(objects), 1000):
            batch = objects[i:i + 1000]
            resp = s3.delete_objects(
                Bucket=BUCKET,
                Delete={"Objects": [{"Key": obj["Key"]} for obj in batch]},
            )
            total_deleted += len(resp.get("Deleted", []))

    # Also clear DynamoDB status entries for this job
    ddb_deleted = 0
    ddb_errors = []
    try:
        ddb = _get_ddb()
        kwargs = {
            "TableName": JOBS_TABLE,
            "KeyConditionExpression": "job_id = :jid",
            "ExpressionAttributeValues": {":jid": {"S": job_id}},
            "ProjectionExpression": "job_id, task_id",
        }
        items_to_delete = []
        while True:
            resp = ddb.query(**kwargs)
            items_to_delete.extend(resp["Items"])
            if "LastEvaluatedKey" not in resp:
                break
            kwargs["ExclusiveStartKey"] = resp["LastEvaluatedKey"]

        import time
        for i in range(0, len(items_to_delete), 25):
            batch = items_to_delete[i:i + 25]
            request_items = {
                JOBS_TABLE: [
                    {"DeleteRequest": {"Key": {
                        "job_id": item["job_id"],
                        "task_id": item["task_id"],
                    }}}
                    for item in batch
                ]
            }
            # Retry unprocessed items up to 3 times with backoff
            for attempt in range(4):
                resp = ddb.batch_write_item(RequestItems=request_items)
                unprocessed = resp.get("UnprocessedItems", {}).get(JOBS_TABLE, [])
                processed = len(request_items[JOBS_TABLE]) - len(unprocessed)
                ddb_deleted += processed
                if not unprocessed:
                    break
                request_items = {JOBS_TABLE: unprocessed}
                if attempt < 3:
                    time.sleep(0.1 * (2 ** attempt))
            else:
                ddb_errors.append(f"{len(unprocessed)} items unprocessed after retries")
    except Exception as e:
        ddb_errors.append(str(e))

    result = {
        "job_id": job_id,
        "deleted": total_deleted,
        "ddb_deleted": ddb_deleted,
    }
    if ddb_errors:
        result["ddb_errors"] = ddb_errors
    return ok_response(result)


def handle_save_metadata(event):
    """Save calc.json metadata to S3.
    Input: {job_id, metadata} where metadata is the calc.json content.
    """
    params = parse_body(event)
    job_id = params["job_id"]
    metadata = params["metadata"]

    s3.put_object(Bucket=BUCKET,
                  Key=f"renders/{job_id}/calc.json",
                  Body=json.dumps(metadata),
                  ContentType="application/json")

    return ok_response({"job_id": job_id, "saved": "calc.json"})


def handle_presign(event):
    """Generate a presigned URL for an S3 key.
    Input: {key: "renders/job_id/image.jpeg", filename: "optional_download_name.jpeg"}
    If filename is provided, the URL forces a download with that filename.
    Returns: {url: presigned_url, key: key}
    """
    params = parse_body(event)
    key = params["key"]
    s3_params = {"Bucket": BUCKET, "Key": key}
    filename = params.get("filename")
    if filename:
        s3_params["ResponseContentDisposition"] = f'attachment; filename="{filename}"'
    url = s3.generate_presigned_url(
        "get_object",
        Params=s3_params,
        ExpiresIn=PRESIGN_EXPIRY)
    return ok_response({"url": url, "key": key})


def handle_cleanup(event):
    """Delete a list of S3 keys (temp .raw and merge files).
    Input: {keys: ["renders/job/stripe_0.raw", ...]}
    """
    params = parse_body(event)
    keys = params.get("keys", [])

    if not keys:
        return ok_response({"deleted": 0})

    total_deleted = 0
    for i in range(0, len(keys), 1000):
        batch = keys[i:i + 1000]
        try:
            resp = s3.delete_objects(Bucket=BUCKET, Delete={
                "Objects": [{"Key": k} for k in batch],
                "Quiet": True,
            })
            total_deleted += len(batch)
        except Exception:
            pass

    return ok_response({"deleted": total_deleted})


def handle_detail(event):
    """Return file_count and viewport for a single job (called on selection)."""
    params = parse_body(event)
    job_id = params["job_id"]
    prefix = f"renders/{job_id}/"
    result = {"job_id": job_id}

    # Count files
    try:
        n_files = 0
        for page in s3.get_paginator('list_objects_v2').paginate(
                Bucket=BUCKET, Prefix=prefix):
            n_files += page.get('KeyCount', 0)
        result["file_count"] = n_files
    except Exception:
        result["file_count"] = 0

    # Read view.json for viewport coordinates
    try:
        vobj = s3.get_object(Bucket=BUCKET,
                             Key=f"renders/{job_id}/view.json")
        view = json.loads(vobj["Body"].read())
        result["q_re"] = view.get("q_re")
        result["q_im"] = view.get("q_im")
    except Exception:
        pass

    return ok_response(result)


def handle_head_keys(event):
    """Check which S3 keys exist via HEAD (batch), return metadata.
    Input: {keys: ["renders/job/image_bilevel.tif", ...]}
    Returns: {exists: ["key", ...], meta: {"key": {"size": 12345, "type": "image/tiff"}, ...}}
    """
    import concurrent.futures
    params = parse_body(event)
    keys = params["keys"]

    if not keys:
        return ok_response({"exists": [], "meta": {}})

    def check(key):
        try:
            resp = s3.head_object(Bucket=BUCKET, Key=key)
            info = {
                "size": resp.get("ContentLength", 0),
                "type": resp.get("ContentType", ""),
            }
            # Include image dimensions if stored in user metadata
            user_meta = resp.get("Metadata", {})
            if "width" in user_meta and "height" in user_meta:
                info["width"] = int(user_meta["width"])
                info["height"] = int(user_meta["height"])
            return key, info
        except Exception:
            return None, None

    meta = {}
    exists = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=min(len(keys), 20)) as pool:
        results = list(pool.map(check, keys))

    for key, info in results:
        if key:
            exists.append(key)
            meta[key] = info

    return ok_response({"exists": exists, "meta": meta})


def handle_delete_task(event):
    """Delete a single DynamoDB task status row.
    Input: {job_id, task_id}
    Used to clear stale status before re-dispatching a task with a fixed task_id.
    """
    params = parse_body(event)
    job_id = params["job_id"]
    task_id = params["task_id"]
    ddb = _get_ddb()
    ddb.delete_item(
        TableName=JOBS_TABLE,
        Key={"job_id": {"S": job_id}, "task_id": {"S": task_id}},
    )
    return ok_response({"deleted": f"{job_id}/{task_id}"})
