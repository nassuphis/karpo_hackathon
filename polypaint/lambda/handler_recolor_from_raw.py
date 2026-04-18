"""
Raw-only Color recolor Lambda.

Standalone raw-sidecar recolor path for fused Color artifacts.
"""
from color_recolor_raw import handle_color_recolor_from_raw_request
from shared import parse_body


def handler(event, context):
    params = parse_body(event)
    return handle_color_recolor_from_raw_request(params)
