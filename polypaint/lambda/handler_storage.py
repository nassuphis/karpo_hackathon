"""
Storage Lambda handler — S3 metadata operations + DynamoDB status tracking.

Routes:
  POST /list           — list all computed jobs with metadata
  POST /list-favorites — list persisted favorite Color artifact refs
  POST /add-favorite   — add one favorite Color artifact ref
  POST /delete-favorite — delete one favorite Color artifact ref
  POST /list-palettes  — list saved palette variants for one job
  POST /delete-palette — delete one saved palette variant
  POST /delete-render-artifact — delete one immutable render artifact variant
  POST /delete         — delete all S3 objects for a job
  POST /save-metadata  — save calc.json to S3
  POST /cleanup        — delete a list of S3 keys
  POST /clean-render   — delete render artifacts (.raw/.jpeg/.png) for a job, preserving .bin
  POST /check-keys     — check which S3 keys exist (for polling render completion)
  POST /check-status   — query DynamoDB for task completion counts (replaces check-keys)
  POST /presign        — generate a presigned URL for an S3 key
"""
import json
import re
import time

import boto3
from botocore.config import Config
from botocore.exceptions import ClientError

from color_artifact_meta import color_artifact_meta_key
from logical_sections import (
    AUTO_FIXED_OVERHEAD_MB,
    AUTO_PER_THREAD_OVERHEAD_MB,
    AUTO_USABLE_FRACTION,
    DEFAULT_PALETTE_CHUNK_MEMORY_MB,
    DEFAULT_RASTER_MEMORY_MB,
    DEFAULT_SOLVE_SCORE_MEMORY_MB,
    summarize_chunk_items,
)
from shared import BUCKET, JOBS_TABLE, PRESIGN_EXPIRY, parse_body, ok_response, _get_ddb
from solve_score_chain import compile_solve_score_chain_or_legacy, serialize_solve_score_chain

s3 = boto3.client("s3")
FAVORITES_KEY = "polypaint/favorites/color_artifacts.json"
FAVORITES_DDB_JOB_ID = "favorites#color"
FAVORITES_DDB_META_TASK_ID = "__meta__"
FAVORITES_DDB_TASK_PREFIX = "favorite#"
SOLVE_SCORE_PROGRAMS_PREFIX = "polypaint/solve-score-programs/"
SOLVE_SCORE_PROGRAM_VERSION = 1
SOLVE_SCORE_PROGRAM_META_NAME = "solve_score_name"
SOLVE_SCORE_PROGRAM_META_STATEMENT_COUNT = "solve_score_statement_count"
SOLVE_SCORE_PROGRAM_META_SAVED_AT = "solve_score_saved_at"
MAX_SOLVE_SCORE_PROGRAM_NAME_LEN = 120
MAX_SOLVE_SCORE_PROGRAM_STATEMENTS = 256
MAX_SOLVE_SCORE_PROGRAM_CHAIN_BYTES = 16 * 1024
MAX_SOLVE_SCORE_PROGRAM_TOKEN_LEN = 128
DEFAULT_RESULTS_LIST_WORKERS = 32
MAX_RESULTS_LIST_WORKERS = 64


class _SolveScoreProgramNotFound(RuntimeError):
    pass


def _validate_results_list_workers(value):
    if value in (None, ""):
        value = DEFAULT_RESULTS_LIST_WORKERS
    try:
        workers = int(value)
    except (TypeError, ValueError):
        raise ValueError(f"list_workers must be an integer, got {value!r}")
    if not (1 <= workers <= MAX_RESULTS_LIST_WORKERS):
        raise ValueError(
            f"list_workers must be in [1, {MAX_RESULTS_LIST_WORKERS}], got {workers}"
        )
    return workers


def _results_list_pool_size(workers):
    return max(16, int(workers) * 2)


def _error_response(status_code, message):
    return {
        "statusCode": int(status_code),
        "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
        "body": json.dumps({"error": str(message)[:1000]}),
    }


def _handle_storage_route(fn, event):
    try:
        return fn(event)
    except _SolveScoreProgramNotFound as exc:
        return _error_response(404, exc)
    except (ValueError, KeyError, TypeError, RuntimeError, json.JSONDecodeError) as exc:
        return _error_response(400, exc)


def _utc_now_iso():
    return time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())


def _slugify_solve_score_program_id(name):
    text = str(name or "").strip().lower()
    slug = re.sub(r"[^a-z0-9]+", "-", text).strip("-")
    slug = slug[:64].strip("-")
    return slug or "solve-score-program"


def _solve_score_program_key(program_id):
    return f"{SOLVE_SCORE_PROGRAMS_PREFIX}{program_id}.json"


def _validate_solve_score_program_name(name):
    text = str(name or "").strip()
    if not text:
        raise ValueError("solve-score program name is required")
    if len(text) > MAX_SOLVE_SCORE_PROGRAM_NAME_LEN:
        raise ValueError(
            f"solve-score program name must be at most {MAX_SOLVE_SCORE_PROGRAM_NAME_LEN} characters"
        )
    if any(ch in "\r\n\t" for ch in text) or not all(ch.isprintable() for ch in text):
        raise ValueError("solve-score program name must contain printable single-line text")
    return text


def _validate_solve_score_program_chain_value(value, path):
    if isinstance(value, str):
        if len(value) > MAX_SOLVE_SCORE_PROGRAM_TOKEN_LEN:
            raise ValueError(
                f"{path} string token must be at most {MAX_SOLVE_SCORE_PROGRAM_TOKEN_LEN} characters"
            )
        return
    if isinstance(value, (int, float)):
        return
    if isinstance(value, list):
        for idx, item in enumerate(value):
            _validate_solve_score_program_chain_value(item, f"{path}[{idx}]")
        return
    raise ValueError(f"{path} must contain only arrays, strings, and numbers")


def _compile_solve_score_program_payload(name, chain, *, saved_at=None, version=SOLVE_SCORE_PROGRAM_VERSION):
    validated_name = _validate_solve_score_program_name(name)
    if not isinstance(chain, list) or not chain:
        raise ValueError("solve-score program chain must be a non-empty JSON array")
    if len(chain) > MAX_SOLVE_SCORE_PROGRAM_STATEMENTS:
        raise ValueError(
            f"solve-score program chain must contain at most {MAX_SOLVE_SCORE_PROGRAM_STATEMENTS} statements"
        )
    _validate_solve_score_program_chain_value(chain, "chain")
    chain_json = json.dumps(chain, separators=(",", ":"), ensure_ascii=False)
    if len(chain_json.encode("utf-8")) > MAX_SOLVE_SCORE_PROGRAM_CHAIN_BYTES:
        raise ValueError(
            f"solve-score program chain JSON must be at most {MAX_SOLVE_SCORE_PROGRAM_CHAIN_BYTES} bytes"
        )

    compiled = compile_solve_score_chain_or_legacy(
        chain,
        "",
        default_metric="proximity",
    )
    canonical_chain = json.loads(serialize_solve_score_chain(compiled["chain"]))
    saved_at_text = _utc_now_iso() if saved_at is None else str(saved_at or "").strip()
    try:
        version_num = int(version)
    except (TypeError, ValueError):
        version_num = SOLVE_SCORE_PROGRAM_VERSION
    return {
        "version": version_num,
        "id": _slugify_solve_score_program_id(validated_name),
        "name": validated_name,
        "chain": canonical_chain,
        "metric": compiled["metric"],
        "display": compiled["display"],
        "program_spec": compiled["program_spec"],
        "statement_count": len(canonical_chain),
        "saved_at": saved_at_text,
    }


def _read_solve_score_program_object(program_id):
    key = _solve_score_program_key(program_id)
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=key)
    except Exception as exc:
        if _is_missing_s3_error(exc):
            raise _SolveScoreProgramNotFound(f"solve-score program not found: {program_id}")
        raise
    raw = obj["Body"].read()
    try:
        payload = json.loads(raw) if raw else {}
    except Exception as exc:
        raise RuntimeError(f"saved solve-score program is not valid JSON: {program_id}") from exc
    if not isinstance(payload, dict):
        raise RuntimeError(f"saved solve-score program must be a JSON object: {program_id}")
    program = _compile_solve_score_program_payload(
        payload.get("name"),
        payload.get("chain"),
        saved_at=payload.get("saved_at", ""),
        version=payload.get("version", SOLVE_SCORE_PROGRAM_VERSION),
    )
    program["id"] = str(program_id)
    return program


def _solve_score_program_put_metadata(program):
    return {
        SOLVE_SCORE_PROGRAM_META_NAME: str(program.get("name") or ""),
        SOLVE_SCORE_PROGRAM_META_STATEMENT_COUNT: str(int(program.get("statement_count") or 0)),
        SOLVE_SCORE_PROGRAM_META_SAVED_AT: str(program.get("saved_at") or ""),
    }


def _solve_score_program_summary_from_head(program_id):
    resp = s3.head_object(Bucket=BUCKET, Key=_solve_score_program_key(program_id))
    meta = resp.get("Metadata") or {}
    name = str(meta.get(SOLVE_SCORE_PROGRAM_META_NAME) or "").strip()
    saved_at = str(meta.get(SOLVE_SCORE_PROGRAM_META_SAVED_AT) or "").strip()
    statement_count_raw = str(meta.get(SOLVE_SCORE_PROGRAM_META_STATEMENT_COUNT) or "").strip()
    if not name or not saved_at or not statement_count_raw:
        raise RuntimeError(f"solve-score program summary metadata missing for {program_id}")
    try:
        statement_count = int(statement_count_raw)
    except (TypeError, ValueError) as exc:
        raise RuntimeError(
            f"solve-score program summary metadata invalid statement_count for {program_id}"
        ) from exc
    return {
        "id": str(program_id),
        "name": name,
        "statement_count": statement_count,
        "saved_at": saved_at,
    }


def _results_list_s3_client(max_workers):
    return boto3.client(
        "s3",
        config=Config(max_pool_connections=_results_list_pool_size(max_workers)),
    )


def _is_missing_s3_error(exc):
    code = getattr(exc, "response", {}).get("Error", {}).get("Code")
    if code in {"NoSuchKey", "404", "NotFound"}:
        return True
    msg = str(exc)
    return "NoSuchKey" in msg or "NotFound" in msg


def _favorite_task_id(job_id, artifact_id):
    return f"{FAVORITES_DDB_TASK_PREFIX}{job_id}#{artifact_id}"


def _load_legacy_favorites():
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=FAVORITES_KEY)
    except Exception as exc:
        if _is_missing_s3_error(exc):
            return []
        raise
    raw = obj["Body"].read()
    data = json.loads(raw) if raw else []
    if isinstance(data, list):
        return data
    if isinstance(data, dict) and isinstance(data.get("favorites"), list):
        return data["favorites"]
    return []


def _list_favorite_rows():
    ddb = _get_ddb()
    kwargs = {
        "TableName": JOBS_TABLE,
        "KeyConditionExpression": "job_id = :jid",
        "ExpressionAttributeValues": {
            ":jid": {"S": FAVORITES_DDB_JOB_ID},
        },
    }
    rows = []
    while True:
        resp = ddb.query(**kwargs)
        rows.extend(resp.get("Items", []))
        if "LastEvaluatedKey" not in resp:
            break
        kwargs["ExclusiveStartKey"] = resp["LastEvaluatedKey"]
    return rows


def _favorite_store_initialized():
    ddb = _get_ddb()
    resp = ddb.get_item(
        TableName=JOBS_TABLE,
        Key={
            "job_id": {"S": FAVORITES_DDB_JOB_ID},
            "task_id": {"S": FAVORITES_DDB_META_TASK_ID},
        },
        ProjectionExpression="job_id, task_id",
    )
    return "Item" in resp


def _favorite_from_row(row):
    entry = {
        "job_id": row["favorite_job_id"]["S"],
        "artifact_id": row["favorite_artifact_id"]["S"],
        "family": row.get("family", {}).get("S", "color"),
        "added_at": row.get("added_at", {}).get("S", ""),
    }
    for field in ("display_name", "image_key", "preview_key"):
        value = row.get(field, {}).get("S")
        if value:
            entry[field] = value
    return entry


def _sort_favorites(items):
    return sorted(items, key=lambda item: item.get("added_at", ""), reverse=True)


def _read_favorites_from_ddb():
    rows = _list_favorite_rows()
    favorites = []
    for row in rows:
        task_id = row.get("task_id", {}).get("S")
        if task_id == FAVORITES_DDB_META_TASK_ID:
            continue
        if task_id and task_id.startswith(FAVORITES_DDB_TASK_PREFIX):
            favorites.append(_favorite_from_row(row))
    return _sort_favorites(favorites)


def _put_favorite_meta():
    try:
        _get_ddb().put_item(
            TableName=JOBS_TABLE,
            Item={
                "job_id": {"S": FAVORITES_DDB_JOB_ID},
                "task_id": {"S": FAVORITES_DDB_META_TASK_ID},
                "family": {"S": "color"},
                "updated_at_ms": {"N": str(int(time.time() * 1000))},
            },
            ConditionExpression="attribute_not_exists(job_id) AND attribute_not_exists(task_id)",
        )
    except ClientError as exc:
        code = exc.response.get("Error", {}).get("Code")
        if code != "ConditionalCheckFailedException":
            raise


def _put_favorite_entry(entry, allow_existing=False):
    item = {
        "job_id": {"S": FAVORITES_DDB_JOB_ID},
        "task_id": {"S": _favorite_task_id(entry["job_id"], entry["artifact_id"])},
        "favorite_job_id": {"S": entry["job_id"]},
        "favorite_artifact_id": {"S": entry["artifact_id"]},
        "family": {"S": "color"},
        "added_at": {"S": entry["added_at"]},
        "updated_at_ms": {"N": str(int(time.time() * 1000))},
    }
    for field in ("display_name", "image_key", "preview_key"):
        value = entry.get(field)
        if value:
            item[field] = {"S": value}
    try:
        _get_ddb().put_item(
            TableName=JOBS_TABLE,
            Item=item,
            ConditionExpression="attribute_not_exists(job_id) AND attribute_not_exists(task_id)",
        )
        return True
    except ClientError as exc:
        code = exc.response.get("Error", {}).get("Code")
        if code == "ConditionalCheckFailedException" and allow_existing:
            return False
        if code == "ConditionalCheckFailedException":
            return False
        raise


def _delete_favorite_entry(job_id, artifact_id):
    resp = _get_ddb().delete_item(
        TableName=JOBS_TABLE,
        Key={
            "job_id": {"S": FAVORITES_DDB_JOB_ID},
            "task_id": {"S": _favorite_task_id(job_id, artifact_id)},
        },
        ReturnValues="ALL_OLD",
    )
    return "Attributes" in resp


def _ensure_favorites_store_ready():
    if _favorite_store_initialized():
        return
    legacy = _load_legacy_favorites()
    _put_favorite_meta()
    for entry in legacy:
        if entry.get("job_id") and entry.get("artifact_id"):
            _put_favorite_entry({
                "job_id": entry["job_id"],
                "artifact_id": entry["artifact_id"],
                "family": "color",
                "added_at": entry.get("added_at") or time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
                "display_name": entry.get("display_name"),
                "image_key": entry.get("image_key"),
                "preview_key": entry.get("preview_key"),
            }, allow_existing=True)


def _read_favorites():
    if _favorite_store_initialized():
        return _read_favorites_from_ddb()
    return _load_legacy_favorites()


def handler(event, context):
    path = event.get("rawPath", event.get("path", "/"))
    if path.endswith("/list"):
        return handle_list(event)
    elif path.endswith("/list-solve-score-programs"):
        return _handle_storage_route(handle_list_solve_score_programs, event)
    elif path.endswith("/fetch-solve-score-program"):
        return _handle_storage_route(handle_fetch_solve_score_program, event)
    elif path.endswith("/save-solve-score-program"):
        return _handle_storage_route(handle_save_solve_score_program, event)
    elif path.endswith("/delete-solve-score-program"):
        return _handle_storage_route(handle_delete_solve_score_program, event)
    elif path.endswith("/list-favorites"):
        return handle_list_favorites(event)
    elif path.endswith("/add-favorite"):
        return handle_add_favorite(event)
    elif path.endswith("/delete-favorite"):
        return handle_delete_favorite(event)
    elif path.endswith("/list-palettes"):
        return handle_list_palettes(event)
    elif path.endswith("/delete-palette"):
        return handle_delete_palette(event)
    elif path.endswith("/delete-render-artifact"):
        return handle_delete_render_artifact(event)
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
    elif path.endswith("/render-summary"):
        return handle_render_summary(event)
    elif path.endswith("/delete-task"):
        return handle_delete_task(event)
    elif path.endswith("/delete-prefix"):
        return handle_delete_prefix(event)
    elif path.endswith("/list-deepzoom"):
        return handle_list_deepzoom(event)
    return {
        "statusCode": 400,
        "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
        "body": json.dumps({"error": f"Unknown route: {path}"}),
    }


def handle_list_favorites(event):
    favorites = _read_favorites()
    return ok_response({"favorites": favorites, "count": len(favorites)})


def handle_add_favorite(event):
    params = parse_body(event)
    job_id = params["job_id"]
    artifact_id = params["artifact_id"]
    family = params.get("family", "color")
    if family != "color":
        raise ValueError("Only color favorites are supported")
    _ensure_favorites_store_ready()
    entry = {
        "job_id": job_id,
        "artifact_id": artifact_id,
        "family": "color",
        "added_at": params.get("added_at") or time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
    }
    for field in ("display_name", "image_key", "preview_key"):
        value = params.get(field)
        if value:
            entry[field] = value
    added = _put_favorite_entry(entry, allow_existing=True)
    favorites = _read_favorites_from_ddb()
    return ok_response({"added": added, "favorites": favorites, "count": len(favorites)})


def handle_delete_favorite(event):
    params = parse_body(event)
    job_id = params["job_id"]
    artifact_id = params["artifact_id"]
    _ensure_favorites_store_ready()
    deleted = _delete_favorite_entry(job_id, artifact_id)
    favorites = _read_favorites_from_ddb()
    return ok_response({"deleted": deleted, "favorites": favorites, "count": len(favorites)})


def handle_list_solve_score_programs(event):
    parse_body(event)
    programs = []
    errors = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=SOLVE_SCORE_PROGRAMS_PREFIX):
        for obj in page.get("Contents", []):
            key = obj.get("Key", "")
            if not key.endswith(".json") or key.endswith("/"):
                continue
            program_id = key[len(SOLVE_SCORE_PROGRAMS_PREFIX):-5]
            if not program_id:
                continue
            try:
                program = _solve_score_program_summary_from_head(program_id)
            except _SolveScoreProgramNotFound:
                continue
            except Exception as exc:
                try:
                    program = _read_solve_score_program_object(program_id)
                except _SolveScoreProgramNotFound:
                    continue
                except Exception as read_exc:
                    err_text = str(read_exc)
                    print(f"solve-score program list skipped {key}: {err_text}")
                    errors.append({
                        "id": program_id,
                        "error": err_text[:240],
                    })
                    continue
                else:
                    print(
                        f"solve-score program list used full read fallback for {key}: "
                        f"{type(exc).__name__}: {exc}"
                    )
            programs.append(program)
    # Tie-break id ascending for identical timestamps.
    programs = sorted(programs, key=lambda row: row["id"])
    programs = sorted(programs, key=lambda row: row.get("saved_at") or "", reverse=True)
    return ok_response({
        "programs": programs,
        "count": len(programs),
        "order": "saved_at_desc",
        "errors": errors,
        "error_count": len(errors),
    })


def handle_fetch_solve_score_program(event):
    params = parse_body(event)
    program_id = str(params.get("id") or "").strip()
    if not program_id:
        raise ValueError("solve-score program fetch requires id")
    return ok_response({"program": _read_solve_score_program_object(program_id)})


def handle_save_solve_score_program(event):
    params = parse_body(event)
    program = _compile_solve_score_program_payload(
        params.get("name"),
        params.get("chain"),
    )
    key = _solve_score_program_key(program["id"])
    overwritten = _key_exists(key)
    s3.put_object(
        Bucket=BUCKET,
        Key=key,
        Body=(json.dumps(program, indent=2) + "\n").encode("utf-8"),
        ContentType="application/json",
        Metadata=_solve_score_program_put_metadata(program),
    )
    return ok_response({"program": program, "overwritten": overwritten})


def handle_delete_solve_score_program(event):
    params = parse_body(event)
    program_id = str(params.get("id") or "").strip()
    if not program_id:
        raise ValueError("solve-score program delete requires id")
    key = _solve_score_program_key(program_id)
    if not _key_exists(key):
        raise _SolveScoreProgramNotFound(f"solve-score program not found: {program_id}")
    s3.delete_object(Bucket=BUCKET, Key=key)
    return ok_response({"id": program_id, "deleted": 1})


def handle_list(event):
    """List all computed results in S3.
    Uses Delimiter='/' to get just folder names (O(n_jobs)),
    then reads calc.json per job in parallel for metadata.
    """
    import concurrent.futures

    params = parse_body(event)
    requested_workers = _validate_results_list_workers(params.get("list_workers"))
    list_s3 = _results_list_s3_client(requested_workers)
    t0 = time.time()

    # List folder prefixes under renders/ — O(n_jobs), not O(all_objects)
    job_ids = []
    t_prefix_0 = time.time()
    paginator = list_s3.get_paginator('list_objects_v2')
    for page in paginator.paginate(Bucket=BUCKET, Prefix='renders/',
                                   Delimiter='/'):
        for prefix in page.get('CommonPrefixes', []):
            # prefix['Prefix'] = 'renders/job_id/'
            job_id = prefix['Prefix'].split('/')[1]
            if job_id:
                job_ids.append(job_id)
    prefix_list_us = int((time.time() - t_prefix_0) * 1e6)

    # Read calc.json for each job (parallelized) — table fields only
    def read_calc(job_id):
        entry = {"job_id": job_id}
        try:
            obj = list_s3.get_object(Bucket=BUCKET,
                                     Key=f"renders/{job_id}/calc.json")
            calc = json.loads(obj["Body"].read())
            entry["function"] = calc.get("function", "?")
            entry["degree"] = calc.get("degree", 0)
            entry["N"] = calc.get("N", calc.get("n1", 0))
            entry["n1"] = calc.get("n1", entry["N"])
            entry["n_chunks"] = calc.get("n_chunks", calc.get("n_stripes", 0))
            entry["times"] = calc.get("times", 1)
            chunks = calc.get("chunks", calc.get("stripes", []))
            entry["total_size"] = sum(s.get("bin_size", 0) for s in chunks)
            entry["total_size"] += calc.get("total_coeffs_size", 0)
            entry["total_roots"] = calc.get("total_roots",
                sum(s.get("bin_size", 0) for s in chunks) // 8)
        except Exception as exc:
            entry["function"] = "?"
            entry["total_size"] = 0
            entry["_metadata_error"] = f"{type(exc).__name__}: {exc}"

        return entry

    list_workers = min(requested_workers, max(1, len(job_ids) or 1))
    t_calc_0 = time.time()
    with concurrent.futures.ThreadPoolExecutor(max_workers=list_workers) as pool:
        results = list(pool.map(read_calc, job_ids))
    calc_fetch_us = int((time.time() - t_calc_0) * 1e6)

    metadata_errors = []
    for entry in results:
        err = entry.pop("_metadata_error", None)
        if err:
            metadata_errors.append({"job_id": entry["job_id"], "error": err[:200]})

    # Sort by job_id descending (job_ids contain timestamps)
    t_sort_0 = time.time()
    results.sort(key=lambda r: r["job_id"], reverse=True)
    sort_us = int((time.time() - t_sort_0) * 1e6)

    return ok_response({
        "results": results,
        "count": len(results),
        "list_us": int((time.time() - t0) * 1e6),
        "prefix_list_us": prefix_list_us,
        "calc_fetch_us": calc_fetch_us,
        "sort_us": sort_us,
        "list_workers": list_workers,
        "s3_pool_connections": _results_list_pool_size(requested_workers),
        "metadata_error_count": len(metadata_errors),
        "metadata_errors": metadata_errors[:20],
    })


def handle_list_palettes(event):
    """List immutable palette artifacts for a single job.
    Returns newest-first entries from renders/{job_id}/palettes/*/meta.json.
    """
    params = parse_body(event)
    job_id = params["job_id"]
    palettes = _list_saved_palettes(job_id)
    palettes = _order_palette_variants(palettes)
    return ok_response({"job_id": job_id, "palettes": palettes, "count": len(palettes)})


def handle_delete_palette(event):
    """Delete one immutable palette variant under renders/{job_id}/palettes/{palette_id}/."""
    params = parse_body(event)
    job_id = params["job_id"]
    palette_id = params["palette_id"]
    prefix = f"renders/{job_id}/palettes/{palette_id}/"

    objects = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=prefix):
        objects.extend(page.get("Contents", []))

    if not objects:
        return ok_response({"job_id": job_id, "palette_id": palette_id, "deleted": 0})

    total_deleted = 0
    for i in range(0, len(objects), 1000):
        batch = objects[i:i + 1000]
        resp = s3.delete_objects(
            Bucket=BUCKET,
            Delete={"Objects": [{"Key": obj["Key"]} for obj in batch]},
        )
        total_deleted += len(resp.get("Deleted", []))

    return ok_response({"job_id": job_id, "palette_id": palette_id, "deleted": total_deleted})


RENDER_FAMILY_DIRS = {
    "color": "color",
    "bilevel": "bilevel",
    "coeffs": "coeffs",
    "pdf": "pdf",
}

RENDER_FAMILY_SHAPES = {
    "color": {
        "image_candidates": ["image.jpeg", "image.png"],
        "preview_candidates": ["preview.png"],
        "legacy_image_candidates": ["image.jpeg", "image.png"],
        "legacy_preview_candidates": ["preview_color.png"],
    },
    "bilevel": {
        "image_candidates": ["image.tif", "image.png"],
        "preview_candidates": ["preview.png"],
        "legacy_image_candidates": ["image_bilevel.tif"],
        "legacy_preview_candidates": ["preview_bilevel.png", "image_bilevel_preview.png"],
    },
    "coeffs": {
        "image_candidates": ["image.tif"],
        "preview_candidates": ["preview.png"],
        "legacy_image_candidates": ["image_coeffs_bilevel.tif"],
        "legacy_preview_candidates": ["preview_coeffs.png", "image_coeffs_bilevel_preview.png"],
    },
    "palette": {
        "legacy_image_candidates": ["image_palette.jpeg"],
        "legacy_preview_candidates": ["preview_palette.png"],
    },
    "pdf": {
        "image_candidates": ["document.pdf"],
        "preview_candidates": [],
        "legacy_image_candidates": [],
        "legacy_preview_candidates": [],
    },
}


def _parse_root_transforms(raw):
    if not raw:
        return []
    try:
        return json.loads(raw)
    except Exception:
        return []


def _list_saved_palettes(job_id):
    import concurrent.futures

    base_prefix = f"renders/{job_id}/palettes/"
    palette_prefixes = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=base_prefix, Delimiter="/"):
        palette_prefixes.extend(p["Prefix"] for p in page.get("CommonPrefixes", []))

    def read_meta(prefix):
        try:
            obj = s3.get_object(Bucket=BUCKET, Key=prefix + "meta.json")
            meta = json.loads(obj["Body"].read())
            metric = meta.get("metric", "proximity")
            image_key = meta.get("image_key", prefix + "image.jpeg")
            preview_key = meta.get("preview_key", prefix + "preview.png")
            score_key = meta.get("score_key")
            palette_bins_key = meta.get("palette_bins_key")
            section_scores_prefix = meta.get("section_scores_prefix", meta.get("chunk_scores_prefix", prefix + "chunks/score_section_"))
            section_bins_prefix = meta.get("section_bins_prefix", meta.get("chunk_bins_prefix", prefix + "chunks/palette_bins_section_"))
            section_meta_prefix = meta.get("section_meta_prefix", meta.get("chunk_meta_prefix", prefix + "chunks/meta_section_"))
            render_reusable = _parse_bool(meta.get("render_reusable"), False)
            data_layout = meta.get("data_layout", "")
            meta["family"] = "palette"
            meta["artifact_id"] = meta.get("palette_id")
            meta["image_key"] = image_key
            meta["preview_key"] = preview_key
            if score_key:
                meta["score_key"] = score_key
            if palette_bins_key:
                meta["palette_bins_key"] = palette_bins_key
            meta["section_scores_prefix"] = section_scores_prefix
            meta["section_bins_prefix"] = section_bins_prefix
            meta["section_meta_prefix"] = section_meta_prefix
            meta["chunk_scores_prefix"] = section_scores_prefix
            meta["chunk_bins_prefix"] = section_bins_prefix
            meta["chunk_meta_prefix"] = section_meta_prefix
            meta["render_reusable"] = render_reusable
            meta["data_layout"] = data_layout
            meta["solve_score_omega_enabled"] = _parse_bool(meta.get("solve_score_omega_enabled"), True)
            meta["solve_score_chain"] = meta.get("solve_score_chain", [])
            meta["derived_from_palette_id"] = meta.get("derived_from_palette_id", "")
            meta["image_url"] = s3.generate_presigned_url(
                "get_object", Params={"Bucket": BUCKET, "Key": image_key},
                ExpiresIn=PRESIGN_EXPIRY,
            )
            meta["preview_url"] = s3.generate_presigned_url(
                "get_object", Params={"Bucket": BUCKET, "Key": preview_key},
                ExpiresIn=PRESIGN_EXPIRY,
            )
            meta["viewer_url"] = meta["preview_url"] or meta["image_url"]
            meta["file_size"] = meta.get("file_size", 0)
            meta["size"] = meta.get("file_size", 0)
            meta["format"] = "jpeg"
            return meta
        except Exception:
            return None

    with concurrent.futures.ThreadPoolExecutor(max_workers=min(len(palette_prefixes), 20) or 1) as pool:
        return [m for m in pool.map(read_meta, palette_prefixes) if m]


def _first_existing(head_results, keys):
    for key in keys:
        info = head_results.get(key)
        if info and info.get("exists"):
            return info
    return None


def _parse_float(value):
    if value in ("", None):
        return None
    try:
        return float(value)
    except Exception:
        return None


def _parse_int(value):
    if value in ("", None):
        return None
    try:
        return int(value)
    except Exception:
        return None


def _parse_bool(value, default=False):
    if value in ("", None):
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    return str(value).strip().lower() in ("1", "true", "yes", "on")


def _parse_json(value):
    if value in ("", None):
        return None
    if isinstance(value, (dict, list)):
        return value
    try:
        return json.loads(value)
    except Exception:
        return None


def _load_color_artifact_overlay(job_id, artifact_id):
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=color_artifact_meta_key(job_id, artifact_id))
    except Exception:
        return None
    try:
        body = obj["Body"].read()
        data = json.loads(body)
    except Exception:
        return None
    return data if isinstance(data, dict) else None


def _render_artifact_entry(family, artifact_id, image_info, preview_info=None, fallback_meta=None, legacy=False):
    meta = {}
    if image_info:
        meta.update(image_info.get("user_meta", {}) or {})
    if fallback_meta:
        meta.update(fallback_meta)

    image_key = image_info["key"]
    image_url = image_info.get("url")
    preview_key = preview_info["key"] if preview_info and preview_info.get("exists") else None
    preview_url = preview_info.get("url") if preview_info and preview_info.get("exists") else None
    created_at = meta.get("created_at") or image_info.get("modified_at")
    entry = {
        "family": family,
        "artifact_id": meta.get("artifact_id", artifact_id),
        "created_at": created_at,
        "image_key": image_key,
        "image_url": image_url,
        "preview_key": preview_key,
        "preview_url": preview_url,
        "viewer_url": preview_url or image_url,
        "width": image_info.get("width"),
        "height": image_info.get("height"),
        "file_size": image_info.get("size", 0),
        "size": image_info.get("size", 0),
        "content_type": image_info.get("type", ""),
        "format": meta.get("format") or image_key.rsplit(".", 1)[-1].lower(),
        "root_transforms": _parse_root_transforms(meta.get("root_transforms")),
        "rotation": _parse_float(meta.get("rotation")),
        "degree": meta.get("degree"),
        "pix": meta.get("pix"),
        "quality": _parse_float(meta.get("quality")),
        "view_mode": meta.get("view_mode", ""),
        "quantile": _parse_float(meta.get("quantile")),
        "shim": _parse_float(meta.get("shim")),
        "square_extent": _parse_float(meta.get("square_extent")),
        "min_re": _parse_float(meta.get("min_re")),
        "max_re": _parse_float(meta.get("max_re")),
        "min_im": _parse_float(meta.get("min_im")),
        "max_im": _parse_float(meta.get("max_im")),
        "legacy": legacy,
    }

    if family == "color":
        repalette_capable = str(meta.get("repalette_capable", "")).strip().lower() == "true"
        entry["color_mode"] = meta.get("color_mode", "")
        entry["match_mode"] = meta.get("match_mode", "")
        entry["palette"] = meta.get("palette", "")
        entry["constant_color"] = meta.get("constant_color", "")
        entry["background_color"] = meta.get("background_color", "")
        entry["background_threshold"] = _parse_float(meta.get("background_threshold"))
        entry["solve_score_chain"] = _parse_json(meta.get("solve_score_chain"))
        entry["palette_source_id"] = meta.get("palette_source_id", "")
        entry["palette_source_display_name"] = meta.get("palette_source_display_name", "")
        entry["palette_source_palette"] = meta.get("palette_source_palette", "")
        entry["palette_source_metric"] = meta.get("palette_source_metric", "")
        entry["palette_source_score_chain"] = _parse_json(meta.get("palette_source_score_chain"))
        src_q = meta.get("palette_source_quantile", "")
        entry["palette_source_quantile"] = float(src_q) if src_q not in ("", None) else None
        src_omega = meta.get("palette_source_omega", "")
        entry["palette_source_omega"] = float(src_omega) if src_omega not in ("", None) else None
        entry["palette_source_omega_enabled"] = _parse_bool(meta.get("palette_source_omega_enabled"), True)
        entry["associated_palette_mode"] = meta.get("associated_palette_mode", "")
        entry["associated_palette_id"] = meta.get("associated_palette_id", "")
        entry["associated_palette_display_name"] = meta.get("associated_palette_display_name", "")
        entry["associated_palette_image_key"] = meta.get("associated_palette_image_key", "")
        entry["associated_palette_preview_key"] = meta.get("associated_palette_preview_key", "")
        entry["associated_palette_palette"] = meta.get("associated_palette_palette", "")
        entry["associated_palette_metric"] = meta.get("associated_palette_metric", "")
        entry["associated_palette_score_chain"] = _parse_json(meta.get("associated_palette_score_chain"))
        assoc_q = meta.get("associated_palette_quantile", "")
        entry["associated_palette_quantile"] = float(assoc_q) if assoc_q not in ("", None) else None
        assoc_omega = meta.get("associated_palette_omega", "")
        entry["associated_palette_omega"] = float(assoc_omega) if assoc_omega not in ("", None) else None
        entry["associated_palette_omega_enabled"] = _parse_bool(
            meta.get("associated_palette_omega_enabled"),
            bool(entry["associated_palette_mode"]),
        )
        entry["render_execution"] = _parse_json(meta.get("render_execution"))
        entry["derived_from_artifact_id"] = meta.get("derived_from_artifact_id", "")
        entry["derivation_kind"] = meta.get("derivation_kind", "")
        entry["postprocess_kind"] = meta.get("postprocess_kind", "")
        entry["postprocess_profile"] = meta.get("postprocess_profile", "")
        entry["autolevels_params"] = _parse_json(meta.get("autolevels_params"))
        entry["resize_params"] = _parse_json(meta.get("resize_params"))
        entry["repalette_capable"] = repalette_capable
        entry["pixel_bins_prefix"] = meta.get("pixel_bins_prefix", "")
        pbe = meta.get("pixel_bins_empty", "")
        entry["pixel_bins_empty"] = int(pbe) if pbe not in ("", None) else None
        entry["pixel_bins_layout"] = meta.get("pixel_bins_layout", "")
        entry["raw_key"] = meta.get("raw_key", "")
        entry["raw_meta_key"] = meta.get("raw_meta_key", "")
        entry["step_scores_key"] = meta.get("step_scores_key", "")
        entry["step_count"] = _parse_int(meta.get("step_count"))
        entry["step_scores_grid_n"] = _parse_int(meta.get("step_scores_grid_n"))
    elif family in ("bilevel", "coeffs"):
        entry["derived_from_artifact_id"] = meta.get("derived_from_artifact_id", "")
        entry["derived_from_image_key"] = meta.get("derived_from_image_key", "")
        entry["postprocess_kind"] = meta.get("postprocess_kind", "")
        entry["postprocess_profile"] = meta.get("postprocess_profile", "")
        entry["threshold"] = _parse_int(meta.get("threshold"))
        entry["render_execution"] = _parse_json(meta.get("render_execution"))
        entry["bilevel_pipeline"] = meta.get("bilevel_pipeline", "")
        entry["bilevel_section_mode"] = meta.get("bilevel_section_mode", "")
        entry["bilevel_section_count"] = _parse_int(meta.get("bilevel_section_count"))
    elif family == "pdf":
        page_count = meta.get("page_count")
        entry["pdf_kind"] = meta.get("pdf_kind", "")
        entry["source_family"] = meta.get("source_family", "")
        entry["source_artifact_id"] = meta.get("source_artifact_id", "")
        entry["source_image_key"] = meta.get("source_image_key", "")
        entry["source_display_name"] = meta.get("source_display_name", "")
        entry["source_color_mode"] = meta.get("source_color_mode", "")
        entry["source_palette"] = meta.get("source_palette", "")
        entry["source_solve_metric"] = meta.get("source_solve_metric", "")
        entry["source_solve_score_quantile"] = _parse_float(meta.get("source_solve_score_quantile"))
        entry["source_solve_score_omega"] = _parse_float(meta.get("source_solve_score_omega"))
        entry["source_solve_score_omega_enabled"] = _parse_bool(meta.get("source_solve_score_omega_enabled"), True)
        entry["page_count"] = int(page_count) if page_count not in ("", None) else None
    return entry


def _sort_variants_by_created_desc(items):
    items.sort(key=lambda a: a.get("created_at", ""), reverse=True)
    return items


def _order_color_variants(variants):
    by_id = {v.get("artifact_id"): v for v in variants if v.get("artifact_id")}
    children = {}
    top = []
    for art in variants:
        parent_id = art.get("derived_from_artifact_id") or ""
        if parent_id and parent_id in by_id and parent_id != art.get("artifact_id"):
            children.setdefault(parent_id, []).append(art)
        else:
            top.append(art)

    for art_list in children.values():
        _sort_variants_by_created_desc(art_list)
    _sort_variants_by_created_desc(top)

    ordered = []
    seen = set()

    def append_with_children(art):
        aid = art.get("artifact_id")
        if aid in seen:
            return
        if aid:
            seen.add(aid)
        for child in children.get(aid, []):
            append_with_children(child)
        ordered.append(art)

    for art in top:
        append_with_children(art)
    for art in variants:
        append_with_children(art)
    return ordered


def _order_palette_variants(variants):
    by_id = {v.get("palette_id") or v.get("artifact_id"): v for v in variants if v.get("palette_id") or v.get("artifact_id")}
    children = {}
    top = []
    for art in variants:
        aid = art.get("palette_id") or art.get("artifact_id")
        parent_id = art.get("derived_from_palette_id") or ""
        if parent_id and parent_id in by_id and parent_id != aid:
            children.setdefault(parent_id, []).append(art)
        else:
            top.append(art)

    for art_list in children.values():
        _sort_variants_by_created_desc(art_list)
    _sort_variants_by_created_desc(top)

    ordered = []
    seen = set()

    def append_with_children(art):
        aid = art.get("palette_id") or art.get("artifact_id")
        if aid in seen:
            return
        if aid:
            seen.add(aid)
        for child in children.get(aid, []):
            append_with_children(child)
        ordered.append(art)

    for art in top:
        append_with_children(art)
    for art in variants:
        append_with_children(art)
    return ordered


def _list_render_family_variants(job_id, family):
    import concurrent.futures

    shape = RENDER_FAMILY_SHAPES[family]
    base_prefix = f"renders/{job_id}/{RENDER_FAMILY_DIRS[family]}/"
    artifact_prefixes = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=base_prefix, Delimiter="/"):
        artifact_prefixes.extend(p["Prefix"] for p in page.get("CommonPrefixes", []))

    def read_prefix(prefix):
        artifact_id = prefix.rstrip("/").split("/")[-1]
        keys = [prefix + k for k in shape["image_candidates"] + shape["preview_candidates"]]
        head_results = _head_artifact_keys(keys, presign=True)
        image_info = _first_existing(head_results, [prefix + k for k in shape["image_candidates"]])
        if not image_info:
            return None
        preview_info = _first_existing(head_results, [prefix + k for k in shape["preview_candidates"]])
        fallback_meta = _load_color_artifact_overlay(job_id, artifact_id) if family == "color" else None
        return _render_artifact_entry(family, artifact_id, image_info, preview_info, fallback_meta=fallback_meta)

    variants = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=min(len(artifact_prefixes), 20) or 1) as pool:
        variants.extend(v for v in pool.map(read_prefix, artifact_prefixes) if v)

    return variants


def _legacy_render_variant(job_id, family):
    shape = RENDER_FAMILY_SHAPES[family]
    prefix = f"renders/{job_id}/"
    keys = [prefix + k for k in shape["legacy_image_candidates"] + shape["legacy_preview_candidates"]]
    head_results = _head_artifact_keys(keys, presign=True)
    image_info = _first_existing(head_results, [prefix + k for k in shape["legacy_image_candidates"]])
    if not image_info:
        return None
    preview_info = _first_existing(head_results, [prefix + k for k in shape["legacy_preview_candidates"]])
    fallback_meta = _load_color_artifact_overlay(job_id, f"legacy_{family}") if family == "color" else None
    return _render_artifact_entry(family, f"legacy_{family}", image_info, preview_info, fallback_meta=fallback_meta, legacy=True)


def _delete_prefix_objects(prefix):
    objects = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=prefix):
        objects.extend(page.get("Contents", []))

    if not objects:
        return 0

    total_deleted = 0
    for i in range(0, len(objects), 1000):
        batch = objects[i:i + 1000]
        resp = s3.delete_objects(
            Bucket=BUCKET,
            Delete={"Objects": [{"Key": obj["Key"]} for obj in batch]},
        )
        total_deleted += len(resp.get("Deleted", []))
    return total_deleted


def handle_delete_render_artifact(event):
    params = parse_body(event)
    job_id = params["job_id"]
    family = params["family"]
    artifact_id = params["artifact_id"]

    if family == "palette":
        if artifact_id == "legacy_palette":
            keys = [f"renders/{job_id}/image_palette.jpeg", f"renders/{job_id}/preview_palette.png"]
            deleted = 0
            for key in keys:
                try:
                    s3.delete_object(Bucket=BUCKET, Key=key)
                    deleted += 1
                except Exception:
                    pass
            return ok_response({"job_id": job_id, "family": family, "artifact_id": artifact_id, "deleted": deleted})
        return handle_delete_palette({"body": json.dumps({"job_id": job_id, "palette_id": artifact_id})})

    if artifact_id == f"legacy_{family}":
        deleted = 0
        for suffix in RENDER_FAMILY_SHAPES[family]["legacy_image_candidates"] + RENDER_FAMILY_SHAPES[family]["legacy_preview_candidates"]:
            try:
                s3.delete_object(Bucket=BUCKET, Key=f"renders/{job_id}/{suffix}")
                deleted += 1
            except Exception:
                pass
        return ok_response({"job_id": job_id, "family": family, "artifact_id": artifact_id, "deleted": deleted})

    family_dir = RENDER_FAMILY_DIRS.get(family)
    if not family_dir:
        raise RuntimeError(f"Unknown render family: {family}")
    prefix = f"renders/{job_id}/{family_dir}/{artifact_id}/"
    deleted = _delete_prefix_objects(prefix)
    return ok_response({"job_id": job_id, "family": family, "artifact_id": artifact_id, "deleted": deleted})


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

    now_ms = int(time.time() * 1000)
    ddb = _get_ddb()
    done = 0
    found_ids = []
    error_details = []
    stuck_tasks = []
    status_counts = {}
    results = []
    # Freshness tracking
    latest_update_ms = None
    latest_done_ms = None
    latest_nonterminal_ms = None
    newest_task = None

    kwargs = {
        "TableName": JOBS_TABLE,
        "KeyConditionExpression": "job_id = :jid AND begins_with(task_id, :pfx)",
        "ExpressionAttributeValues": {
            ":jid": {"S": job_id},
            ":pfx": {"S": task_prefix},
        },
        "ProjectionExpression": "task_id, task_status, error_msg, result_data, updated_at_ms",
    }
    while True:
        resp = ddb.query(**kwargs)
        for item in resp["Items"]:
            status = item["task_status"]["S"]
            task_id_val = item["task_id"]["S"]
            status_counts[status] = status_counts.get(status, 0) + 1
            if return_ids:
                found_ids.append(task_id_val)

            # Parse updated_at_ms (may be absent on old rows)
            row_ms = None
            if "updated_at_ms" in item:
                try:
                    row_ms = int(item["updated_at_ms"]["N"])
                except (ValueError, KeyError):
                    pass

            # Track freshness
            if row_ms is not None:
                if latest_update_ms is None or row_ms > latest_update_ms:
                    latest_update_ms = row_ms
                    newest_task = {"task_id": task_id_val, "status": status, "updated_at_ms": row_ms}
                if status == "done" and (latest_done_ms is None or row_ms > latest_done_ms):
                    latest_done_ms = row_ms
                if status not in ("done", "error") and (latest_nonterminal_ms is None or row_ms > latest_nonterminal_ms):
                    latest_nonterminal_ms = row_ms

            # Collect result_data
            parsed_rd = None
            rd = item.get("result_data", {}).get("S")
            if rd:
                try:
                    parsed_rd = json.loads(rd)
                    results.append(parsed_rd)
                except Exception:
                    pass

            if status == "done":
                done += 1
            elif status == "error":
                detail = {
                    "task_id": task_id_val,
                    "error_msg": item.get("error_msg", {}).get("S", "unknown"),
                }
                if parsed_rd is not None:
                    detail["result_data"] = parsed_rd
                error_details.append(detail)
            else:
                entry = {"task_id": task_id_val, "status": status}
                if row_ms is not None:
                    entry["updated_at_ms"] = row_ms
                    entry["age_ms"] = now_ms - row_ms
                stuck_tasks.append(entry)

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
        "latest_update_ms": latest_update_ms,
        "latest_done_ms": latest_done_ms,
        "latest_nonterminal_ms": latest_nonterminal_ms,
        "stale_for_ms": (now_ms - latest_update_ms) if latest_update_ms else None,
    }
    if newest_task:
        resp_body["newest_task"] = newest_task
    if results:
        resp_body["results"] = results
    if return_ids:
        resp_body["found_ids"] = found_ids
    return ok_response(resp_body)


# Canonical ownership mapping: each family owns its own intermediates, previews, and stale siblings
ARTIFACT_FAMILIES = {
    "color": {
        "intermediate_prefixes": ["pix_", "pixbin_chunk_", "pixbin_group_", "raw_", "tile_", "solve_proximity/", "solve_scores/"],
        "intermediate_keys": ["solve_proximity_clip.json", "solve_proximity_bins.json"],
        "preview": [],
        "same_family_stale": [],
    },
    "bilevel": {
        "intermediate_prefixes": ["bilevel_t", "bits_chunk_", "bilevel_section_"],
        "intermediate_keys": [],
        "preview": [],
        "same_family_stale": [],
    },
    "coeff_bilevel": {
        "intermediate_prefixes": ["coeff_bilevel_section_", "coeff_t", "coeff_bits_chunk_"],
        "intermediate_keys": [],
        "preview": [],
        "same_family_stale": [],
    },
    "palette": {
        "intermediate_prefixes": [],
        "intermediate_keys": [],
        "preview": [],
        "same_family_stale": [],
    },
}


def handle_clean_render(event):
    """Family-scoped cleanup: delete only the specified family's intermediates,
    previews, and stale same-family siblings. Never touches other families."""
    params = parse_body(event)
    job_id = params["job_id"]
    prefix = f"renders/{job_id}/"
    pipeline = params.get("pipeline", "color")

    family = ARTIFACT_FAMILIES.get(pipeline, ARTIFACT_FAMILIES["color"])

    objects = []
    paginator = s3.get_paginator('list_objects_v2')
    for rp in family["intermediate_prefixes"]:
        for page in paginator.paginate(Bucket=BUCKET, Prefix=prefix + rp):
            for obj in page.get('Contents', []):
                objects.append(obj)
    for key_suffix in family["intermediate_keys"]:
        objects.append({"Key": prefix + key_suffix})
    for key_suffix in family["preview"]:
        objects.append({"Key": prefix + key_suffix})
    for key_suffix in family["same_family_stale"]:
        objects.append({"Key": prefix + key_suffix})

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
    """Return file_count and compute-job viewport/metadata for a single job."""
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

    # Read compute view.json coordinates.
    try:
        vobj = s3.get_object(Bucket=BUCKET,
                             Key=f"renders/{job_id}/view.json")
        view = json.loads(vobj["Body"].read())
        result["compute_q_re"] = view.get("q_re")
        result["compute_q_im"] = view.get("q_im")
        # Compatibility alias for older /detail consumers.
        result["q_re"] = result["compute_q_re"]
        result["q_im"] = result["compute_q_im"]
    except Exception:
        pass

    # Read calc.json for full compute metadata + pipeline info
    try:
        cobj = s3.get_object(Bucket=BUCKET,
                             Key=f"renders/{job_id}/calc.json")
        calc = json.loads(cobj["Body"].read())
        result["calc"] = calc
        # Extract pipeline info for the info panel
        pipeline = calc.get("pipeline", {})
        result["times"] = calc.get("times", 1)
        result["param_transforms"] = pipeline.get("param_transforms", [])
        result["param_transforms_display"] = pipeline.get("param_transforms_display", [])
        result["coeff_transforms"] = pipeline.get("coeff_transforms", [])
        result["pipeline"] = pipeline
    except Exception:
        pass

    # Check for preview (presign if found)
    preview_key = None
    if _key_exists(f"renders/{job_id}/preview.png"):
        preview_key = f"renders/{job_id}/preview.png"
    elif _key_exists(f"renders/{job_id}/preview.jpg"):
        preview_key = f"renders/{job_id}/preview.jpg"
    result["has_preview"] = preview_key is not None
    if preview_key:
        result["preview_url"] = s3.generate_presigned_url(
            "get_object",
            Params={"Bucket": BUCKET, "Key": preview_key},
            ExpiresIn=PRESIGN_EXPIRY)

    # Load preview stats only if preview exists (avoid stale stats)
    if preview_key:
        try:
            ps_obj = s3.get_object(Bucket=BUCKET,
                                   Key=f"renders/{job_id}/preview_stats.json")
            result["preview_stats"] = json.loads(ps_obj["Body"].read())
        except Exception:
            pass

    return ok_response(result)


def _head_artifact_keys(keys, presign=True):
    """HEAD-check a list of S3 keys in parallel."""
    import concurrent.futures
    if not keys:
        return {}

    def check(key):
        try:
            resp = s3.head_object(Bucket=BUCKET, Key=key)
            info = {
                "exists": True,
                "key": key,
                "size": resp.get("ContentLength", 0),
                "type": resp.get("ContentType", ""),
                "width": None,
                "height": None,
                "url": None,
                "modified_at": resp.get("LastModified").strftime("%Y-%m-%dT%H:%M:%SZ") if resp.get("LastModified") else None,
                "user_meta": {},
            }
            user_meta = resp.get("Metadata", {})
            info["user_meta"] = user_meta
            if "width" in user_meta and "height" in user_meta:
                info["width"] = int(user_meta["width"])
                info["height"] = int(user_meta["height"])
            if presign:
                info["url"] = s3.generate_presigned_url(
                    "get_object",
                    Params={"Bucket": BUCKET, "Key": key},
                    ExpiresIn=3600)
            return key, info
        except Exception:
            return key, {"exists": False, "key": key, "size": 0, "type": "", "width": None, "height": None, "url": None, "modified_at": None, "user_meta": {}}

    with concurrent.futures.ThreadPoolExecutor(max_workers=min(len(keys), 20)) as pool:
        results = dict(pool.map(check, keys))
    return results


def handle_head_keys(event):
    """Check which S3 keys exist via HEAD (batch), return metadata."""
    params = parse_body(event)
    keys = params.get("keys", [])
    presign = params.get("presign", False)

    if not keys:
        return ok_response({"exists": [], "meta": {}})

    result = _head_artifact_keys(keys, presign=presign)
    exists = [k for k, v in result.items() if v["exists"]]
    meta = {k: v for k, v in result.items() if v["exists"]}
    return ok_response({"exists": exists, "meta": meta})


def _calc_chunk_items_for_summary(calc):
    calc = calc or {}
    degree = int(calc.get("degree", 1) or 1)
    n_coeffs = int(calc.get("n_coeffs", degree + 1) or (degree + 1))
    record_bytes = degree * 2 * 4
    items = []
    step_start = 0
    chunks = list(calc.get("chunks", calc.get("stripes", [])) or [])
    for raw in sorted(chunks, key=lambda row: row.get("idx", row.get("chunk_idx", row.get("stripe_idx", 0)))):
        step_count = raw.get("step_count", raw.get("n_t"))
        bin_size = raw.get("bin_size")
        if step_count in ("", None) and bin_size not in ("", None) and record_bytes > 0:
            step_count = int(bin_size) // record_bytes
        if step_count in ("", None):
            continue
        step_count = int(step_count)
        if step_count < 1:
            continue
        items.append({
            "chunk_idx": int(raw.get("idx", raw.get("chunk_idx", raw.get("stripe_idx", len(items))))),
            "step_start": step_start,
            "step_count": step_count,
            "bin_size": int(bin_size) if bin_size not in ("", None) else step_count * record_bytes,
            "coeffs_bin_size": int(raw.get("coeffs_size") or 0) or (step_count * n_coeffs * 2 * 4),
            "params_bin_size": int(raw.get("params_bin_size") or 0) or (step_count * 16),
        })
        step_start += step_count
    return items


def _render_summary_calc(calc_data):
    calc = {
        "exists": True,
        "N": calc_data.get("N", calc_data.get("n1")),
        "n1": calc_data.get("n1", calc_data.get("N")),
        "degree": calc_data.get("degree"),
    }
    chunk_items = _calc_chunk_items_for_summary(calc_data)
    if chunk_items:
        degree = int(calc_data.get("degree", 1) or 1)
        n_coeffs = int(calc_data.get("n_coeffs", degree + 1) or (degree + 1))
        summary = summarize_chunk_items(chunk_items, degree, n_coeffs)
        calc.update({
            "n_chunks": len(chunk_items),
            "n_coeffs": n_coeffs,
            "job_size": {
                **summary,
                "solve_hist_memory_mb": DEFAULT_SOLVE_SCORE_MEMORY_MB,
                "palette_chunk_memory_mb": DEFAULT_PALETTE_CHUNK_MEMORY_MB,
                "raster_memory_mb": DEFAULT_RASTER_MEMORY_MB,
                "auto_usable_fraction": AUTO_USABLE_FRACTION,
                "auto_fixed_overhead_mb": AUTO_FIXED_OVERHEAD_MB,
                "auto_per_thread_overhead_mb": AUTO_PER_THREAD_OVERHEAD_MB,
                "lores_root_bytes": int(((calc_data.get("lores") or {}).get("bin_size") or 0) or 0),
                "lores_coeff_bytes": int(((calc_data.get("lores") or {}).get("coeffs_size") or 0) or 0),
                "lores_param_bytes": int(((calc_data.get("lores") or {}).get("params_size") or 0) or 0),
            },
        })
    return calc


def handle_render_summary(event):
    """Single-call Render refresh.
    Returns immutable per-family artifact catalogs plus legacy top-level artifacts
    for compatibility with older jobs."""
    params = parse_body(event)
    job_id = params["job_id"]
    prefix = f"renders/{job_id}/"

    # Legacy top-level artifacts kept for compatibility with older jobs.
    artifact_map = {
        "color_jpeg": prefix + "image.jpeg",
        "color_png": prefix + "image.png",
        "bilevel_tif": prefix + "image_bilevel.tif",
        "bilevel_preview_png": prefix + "image_bilevel_preview.png",
        "bilevel_compat_tif": prefix + "image_bilevel_compat.tif",
        "bilevel_png": prefix + "image_bilevel.png",
        "coeff_tif": prefix + "image_coeffs_bilevel.tif",
        "coeff_preview_png": prefix + "image_coeffs_bilevel_preview.png",
        "preview_coeffs_png": prefix + "preview_coeffs.png",
        "preview_color_png": prefix + "preview_color.png",
        "preview_bilevel_png": prefix + "preview_bilevel.png",
        "palette_jpeg": prefix + "image_palette.jpeg",
        "preview_palette_png": prefix + "preview_palette.png",
    }
    head_results = _head_artifact_keys(list(artifact_map.values()), presign=True)
    artifacts = {}
    for logical_name, s3_key in artifact_map.items():
        artifacts[logical_name] = head_results.get(s3_key, {
            "exists": False, "key": s3_key, "size": 0, "type": "",
            "width": None, "height": None, "url": None, "modified_at": None, "user_meta": {}
        })

    # 2. Read calc.json server-side
    calc = {"exists": False, "N": None, "n1": None, "degree": None}
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=prefix + "calc.json")
        calc_data = json.loads(obj["Body"].read())
        calc = _render_summary_calc(calc_data)
    except Exception:
        pass

    # 3. Read deepzoom pointer server-side (no list/scan)
    deepzoom_latest = {"exists": False}
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=prefix + "deepzoom_latest.json")
        dz_data = json.loads(obj["Body"].read())
        # Presign the DZI URL for browser access
        dzi_key = dz_data.get("dzi_key", "")
        dzi_url = dz_data.get("dzi_url", "")
        deepzoom_latest = {
            "exists": True,
            "export_id": dz_data.get("export_id", ""),
            "created_at": dz_data.get("created_at", ""),
            "source_key": dz_data.get("source_key", ""),
            "source_artifact_id": dz_data.get("source_artifact_id", ""),
            "source_family": dz_data.get("source_family", ""),
            "source_rotation": dz_data.get("source_rotation"),
            "viewport_min_re": dz_data.get("viewport_min_re"),
            "viewport_max_re": dz_data.get("viewport_max_re"),
            "viewport_min_im": dz_data.get("viewport_min_im"),
            "viewport_max_im": dz_data.get("viewport_max_im"),
            "dzi_key": dzi_key,
            "dzi_url": dzi_url,
            "share_url": dz_data.get("share_url", ""),
            "tile_prefix": dz_data.get("tile_prefix", ""),
            "width": dz_data.get("width"),
            "height": dz_data.get("height"),
            "tiles_uploaded": dz_data.get("tiles_uploaded"),
        }
    except Exception:
        pass

    families = {
        "color": _list_render_family_variants(job_id, "color"),
        "bilevel": _list_render_family_variants(job_id, "bilevel"),
        "coeffs": _list_render_family_variants(job_id, "coeffs"),
        "palette": _list_saved_palettes(job_id),
        "pdf": _list_render_family_variants(job_id, "pdf"),
    }

    for family in ("color", "bilevel", "coeffs", "palette", "pdf"):
        legacy = _legacy_render_variant(job_id, family)
        if legacy:
            families[family].append(legacy)
        if family == "color":
            families[family] = _order_color_variants(families[family])
        elif family == "palette":
            families[family] = _order_palette_variants(families[family])
        else:
            families[family].sort(key=lambda a: a.get("created_at", ""), reverse=True)

    return ok_response({
        "job_id": job_id,
        "schema_version": 2,
        "calc": calc,
        "artifacts": artifacts,
        "families": families,
        "deepzoom_latest": deepzoom_latest,
    })


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


def handle_delete_prefix(event):
    """Delete all S3 objects under a prefix.
    Input: {prefix}
    Prefix must start with 'deepzoom/' (safety guard).
    """
    params = parse_body(event)
    prefix = params["prefix"]
    if not prefix.startswith("deepzoom/"):
        return {
            "statusCode": 400,
            "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
            "body": json.dumps({"error": "delete-prefix only allowed under deepzoom/"}),
        }

    objects = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=prefix):
        objects.extend(page.get("Contents", []))

    if not objects:
        return ok_response({"prefix": prefix, "deleted": 0})

    total_deleted = 0
    for i in range(0, len(objects), 1000):
        batch = objects[i:i + 1000]
        resp = s3.delete_objects(
            Bucket=BUCKET,
            Delete={"Objects": [{"Key": obj["Key"]} for obj in batch]},
        )
        total_deleted += len(resp.get("Deleted", []))

    return ok_response({"prefix": prefix, "deleted": total_deleted})


def handle_list_deepzoom(event):
    """List all DeepZoom exports server-side in one call.
    Two-level prefix scan + parallel meta.json reads.
    Returns: {exports: [{job_id, export_id, ...}, ...]}
    """
    import concurrent.futures

    # Level 1: job prefixes under deepzoom/
    job_prefixes = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix="deepzoom/",
                                   Delimiter="/"):
        job_prefixes.extend(p["Prefix"] for p in page.get("CommonPrefixes", []))

    # Level 2: export prefixes under each job (parallelized)
    def list_exports(job_prefix):
        prefixes = []
        for page in paginator.paginate(Bucket=BUCKET, Prefix=job_prefix,
                                       Delimiter="/"):
            prefixes.extend(p["Prefix"] for p in page.get("CommonPrefixes", []))
        return prefixes

    export_prefixes = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=20) as pool:
        for batch in pool.map(list_exports, job_prefixes):
            export_prefixes.extend(batch)

    # Level 3: read meta.json for each export (parallelized)
    def read_meta(prefix):
        try:
            obj = s3.get_object(Bucket=BUCKET, Key=prefix + "meta.json")
            return json.loads(obj["Body"].read())
        except Exception:
            return None

    with concurrent.futures.ThreadPoolExecutor(max_workers=20) as pool:
        exports = [m for m in pool.map(read_meta, export_prefixes) if m]

    exports.sort(key=lambda e: e.get("created_at", ""), reverse=True)
    return ok_response({"exports": exports, "count": len(exports)})
