"""
Raw-only DeepZoom export Lambda.

Strict wrapper over handler_deepzoom_export: this path is only valid for
artifacts that already carry greyscale raw sidecars.
"""
from handler_deepzoom_export import handle_deepzoom_export_request
from shared import parse_body, assert_safe_id


def handler(event, context):
    params = parse_body(event)
    # Caller-threaded operation identity (code-review-29 F1), mirroring
    # handler_deepzoom_export: overlapping runs must not share a status row.
    task_id = assert_safe_id(str(params.get("task_id") or "deepzoom_from_raw"), "task_id")
    return handle_deepzoom_export_request(
        params,
        require_raw_sidecar=True,
        task_id=task_id,
    )
