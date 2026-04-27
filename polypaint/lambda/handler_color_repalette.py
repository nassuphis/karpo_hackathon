"""
Color RePalette Lambda.

Active Color artifacts carry a raw sidecar. RePalette reuses those bytes and
renders them through the requested scalar or channel-LUT palette mapping; the
removed legacy pixel-bin reconstruction path is intentionally not supported.
"""
from color_recolor_raw import handle_color_recolor_from_raw_request
from shared import parse_body


def handler(event, context):
    params = parse_body(event)
    return handle_color_recolor_from_raw_request(params)
