"""
Raw-only DeepZoom export Lambda.

Strict wrapper over handler_deepzoom_export: this path is only valid for
artifacts that already carry greyscale raw sidecars.
"""
from handler_deepzoom_export import handle_deepzoom_export_request
from shared import parse_body


def handler(event, context):
    params = parse_body(event)
    return handle_deepzoom_export_request(
        params,
        require_raw_sidecar=True,
        task_id="deepzoom_from_raw",
    )
