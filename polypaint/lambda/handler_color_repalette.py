"""
Color RePalette Lambda.

Active Color artifacts carry a greyscale raw sidecar. RePalette reuses that
sidecar and renders it through the requested palette; the removed legacy
pixel-bin reconstruction path is intentionally not supported.
"""
from color_recolor_raw import handle_color_recolor_from_raw_request
from shared import parse_body


def handler(event, context):
    params = parse_body(event)
    return handle_color_recolor_from_raw_request(params)
