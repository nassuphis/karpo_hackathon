from __future__ import annotations


INTERPRETATION_ALIASES = {
    "": "scalar_lut",
    "scalar": "scalar_lut",
    "scalar_lut": "scalar_lut",
    "scalar_palette": "scalar_lut",
    "palette": "scalar_lut",
    "rgb": "rgb",
    "direct_rgb": "rgb",
    "hsv": "hsv",
    "rgb_lut": "rgb_lut",
    "rgb-lut": "rgb_lut",
    "hsv_lut": "hsv_lut",
    "hsv-lut": "hsv_lut",
}

VALID_COLOR_INTERPRETATIONS = {"scalar_lut", "rgb", "hsv", "rgb_lut", "hsv_lut"}
CHANNEL_LUT_INTERPRETATIONS = {"rgb_lut", "hsv_lut"}


def normalize_color_interpretation(value, *, default="scalar_lut"):
    raw = default if value in (None, "") else value
    key = str(raw or "").strip().lower()
    try:
        return INTERPRETATION_ALIASES[key]
    except KeyError:
        allowed = ", ".join(sorted(VALID_COLOR_INTERPRETATIONS))
        raise RuntimeError(f"color_interpretation must be one of {allowed}, got {value!r}")


def channel_names_for_interpretation(interpretation, channel_count):
    mode = normalize_color_interpretation(interpretation)
    count = int(channel_count or 0)
    if mode == "scalar_lut":
        return ["score"] if count == 1 else [f"channel_{idx}" for idx in range(count)]
    if mode == "rgb" and count == 3:
        return ["r", "g", "b"]
    if mode == "hsv" and count == 3:
        return ["h", "s", "v"]
    if mode == "rgb_lut" and count == 3:
        return ["r_lookup", "g_lookup", "b_lookup"]
    if mode == "hsv_lut" and count == 3:
        return ["h_lookup", "s_lookup", "v_lookup"]
    return [f"channel_{idx}" for idx in range(count)]


def apply_channel_names(output_channels, interpretation):
    rows = [dict(row or {}) for row in (output_channels or [])]
    names = channel_names_for_interpretation(interpretation, len(rows))
    for idx, row in enumerate(rows):
        channel = int(row.get("channel", idx))
        name = names[idx] if idx < len(names) else f"channel_{channel}"
        row["channel"] = channel
        row["name"] = name
        row["display_name"] = name
    return rows


def validate_color_output_contract(*, interpretation, output_channel_count, output_channels=None):
    mode = normalize_color_interpretation(interpretation)
    count = int(output_channel_count or 1)
    if mode == "scalar_lut" and count != 1:
        raise RuntimeError(f"Scalar LUT requires 1 solve-score output, got {count}")
    if mode == "rgb" and count != 3:
        raise RuntimeError(f"RGB requires 3 solve-score outputs, got {count}")
    if mode == "hsv" and count != 3:
        raise RuntimeError(f"HSV requires 3 solve-score outputs, got {count}")
    if mode == "rgb_lut" and count != 3:
        raise RuntimeError(f"RGB LUT requires 3 solve-score outputs, got {count}")
    if mode == "hsv_lut" and count != 3:
        raise RuntimeError(f"HSV LUT requires 3 solve-score outputs, got {count}")

    channels = apply_channel_names(output_channels or [
        {"channel": idx, "name": f"channel_{idx}", "range_normalized": False}
        for idx in range(count)
    ], mode)
    warnings = []
    if mode == "hsv" and channels:
        first = channels[0]
        emit = str(first.get("emit") or "").strip()
        if bool(first.get("range_normalized")) or emit == "emit_norm":
            warnings.append(
                "HSV hue channel uses emit(norm); normalized hue is non-cyclic. "
                "Consider sawtooth + emit(raw)."
            )
    return {
        "interpretation": mode,
        "channel_count": count,
        "channels": channels,
        "warnings": warnings,
    }


def require_scalar_raw(sidecar_or_validated, *, feature):
    channels = int((sidecar_or_validated or {}).get("channels", 1) or 1)
    if channels != 1:
        raise RuntimeError(f"{feature} requires a scalar (channels=1) raw artifact; got channels={channels}")
    return sidecar_or_validated


def repalette_target_for_source(*, source_channels, requested_interpretation=None):
    channels = int(source_channels or 1)
    requested = "" if requested_interpretation in ("", None) else requested_interpretation
    if channels == 1:
        mode = normalize_color_interpretation(requested or "scalar_lut")
        if mode != "scalar_lut":
            raise RuntimeError(
                "Color RePalette on a scalar (channels=1) artifact requires "
                f"interpretation=scalar_lut, got {requested_interpretation!r}"
            )
        return "scalar_lut"
    if channels == 3:
        if not requested:
            raise RuntimeError(
                "Color RePalette on a 3-channel artifact requires "
                "new_interpretation=rgb_lut or new_interpretation=hsv_lut"
            )
        mode = normalize_color_interpretation(requested)
        if mode not in CHANNEL_LUT_INTERPRETATIONS:
            raise RuntimeError(
                "Color RePalette on a 3-channel artifact requires "
                f"interpretation=rgb_lut or hsv_lut, got {requested_interpretation!r}"
            )
        return mode
    raise RuntimeError(f"Color RePalette does not support channels={channels}")
