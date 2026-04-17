"""
Raw-only Color recolor Lambda.

Strict wrapper over handler_color_repalette: this path is only valid for
artifacts that already carry greyscale raw sidecars.
"""
from handler_color_repalette import handle_color_repalette_request
from shared import parse_body


def handler(event, context):
    params = parse_body(event)
    return handle_color_repalette_request(params, require_raw_sidecar=True)
