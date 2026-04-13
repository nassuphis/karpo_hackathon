"""
Shared solve-score chain helpers.

V1 supports a very small grammar:

- exactly one metric chip
- optional final omega_cosine transfer chip

The backend still consumes scalar solve-score fields, so this module compiles
the chain down to the existing metric / omega / omega_enabled contract while
also emitting canonical chain metadata.
"""
from __future__ import annotations

import json


VALID_SOLVE_SCORE_METRICS = {
    "proximity",
    "crowding",
    "spread",
    "anisotropy",
    "area",
    "clusteriness",
    "shelliness",
    "outlierness",
    "nn_variation",
    "real_axis_proximity",
    "centroid_re",
    "centroid_im",
    "centroid_dist",
    "dist_unit_circle",
    "asymmetry_re",
}
TRANSFER_CHIP_NAME = "omega_cosine"

_FIELD_MAP = {
    "solve": {
        "chain": "solve_score_chain",
        "metric": "solve_metric",
        "quantile": "solve_score_quantile",
        "omega": "solve_score_omega",
        "omega_enabled": "solve_score_omega_enabled",
    },
    "palette_source": {
        "chain": "palette_source_score_chain",
        "metric": "palette_source_metric",
        "quantile": "palette_source_quantile",
        "omega": "palette_source_omega",
        "omega_enabled": "palette_source_omega_enabled",
    },
    "associated_palette": {
        "chain": "associated_palette_score_chain",
        "metric": "associated_palette_metric",
        "quantile": "associated_palette_quantile",
        "omega": "associated_palette_omega",
        "omega_enabled": "associated_palette_omega_enabled",
    },
}


def _scope_fields(scope):
    try:
        return _FIELD_MAP[scope]
    except KeyError:
        raise RuntimeError(f"Unknown solve-score metadata scope: {scope!r}")


def _parse_boolish(value, default=True):
    if value in ("", None):
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    text = str(value).strip().lower()
    if text in ("1", "true", "yes", "on"):
        return True
    if text in ("0", "false", "no", "off"):
        return False
    raise RuntimeError(f"solve_score_omega_enabled must be boolean-like, got {value!r}")


def _validate_metric(value):
    metric = str(value or "").strip()
    if metric not in VALID_SOLVE_SCORE_METRICS:
        raise RuntimeError(f"Invalid solve-score metric: {value!r}")
    return metric


def _validate_omega(value):
    try:
        omega = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_omega must be numeric, got {value!r}")
    if not (1.0 <= omega <= 10.0):
        raise RuntimeError(f"solve_score_omega must be in [1, 10], got {omega}")
    return omega


def _format_number(value):
    num = float(value)
    if num.is_integer():
        return str(int(num))
    return f"{num:g}"


def _normalize_chain_item(item):
    if isinstance(item, str):
        name = str(item).strip()
        params = []
    elif isinstance(item, (list, tuple)):
        if not item:
            raise RuntimeError("solve_score_chain contains an empty item")
        name = str(item[0]).strip()
        params = [str(v) for v in item[1:]]
    elif isinstance(item, dict):
        name = str(item.get("name") or "").strip()
        raw_params = item.get("params") or []
        if not isinstance(raw_params, list):
            raise RuntimeError(f"solve_score_chain params must be a list, got {raw_params!r}")
        params = [str(v) for v in raw_params]
    else:
        raise RuntimeError(f"Unsupported solve_score_chain item: {item!r}")
    if not name:
        raise RuntimeError(f"Invalid solve_score_chain item: {item!r}")
    return {"name": name, "params": params}


def normalize_solve_score_chain(raw_chain):
    if raw_chain in ("", None):
        return []
    payload = raw_chain
    if isinstance(payload, str):
        try:
            payload = json.loads(payload)
        except Exception as exc:
            raise RuntimeError(f"solve_score_chain must be valid JSON, got {raw_chain!r}") from exc
    if not isinstance(payload, list):
        raise RuntimeError(f"solve_score_chain must be a list, got {type(payload).__name__}")
    return [_normalize_chain_item(item) for item in payload]


def serialize_solve_score_chain(chain):
    return json.dumps(
        [
            ([item["name"], *item["params"]] if item.get("params") else item["name"])
            for item in normalize_solve_score_chain(chain)
        ],
        separators=(",", ":"),
    )


def solve_score_chain_from_scalars(metric, omega=1.0, omega_enabled=True):
    metric_name = _validate_metric(metric)
    chain = [{"name": metric_name, "params": []}]
    if _parse_boolish(omega_enabled, True):
        chain.append({"name": TRANSFER_CHIP_NAME, "params": [_format_number(_validate_omega(omega))]})
    return chain


def compile_solve_score_chain(raw_chain):
    chain = normalize_solve_score_chain(raw_chain)
    if not chain:
        raise RuntimeError("solve_score_chain must contain exactly one metric chip")
    if len(chain) > 2:
        raise RuntimeError("solve_score_chain v1 allows exactly one metric chip and at most one final transfer chip")

    metric_item = chain[0]
    if metric_item["name"] not in VALID_SOLVE_SCORE_METRICS:
        raise RuntimeError("solve_score_chain must start with a metric chip")
    if metric_item.get("params"):
        raise RuntimeError("metric chips do not take parameters in solve_score_chain v1")
    metric = _validate_metric(metric_item["name"])

    omega = 1.0
    omega_enabled = False
    if len(chain) == 2:
        transfer = chain[1]
        if transfer["name"] != TRANSFER_CHIP_NAME:
            raise RuntimeError(f"solve_score_chain final chip must be {TRANSFER_CHIP_NAME}")
        if len(transfer["params"]) != 1:
            raise RuntimeError(f"{TRANSFER_CHIP_NAME} requires exactly one omega parameter")
        omega = _validate_omega(transfer["params"][0])
        omega_enabled = True

    return {
        "chain": chain,
        "metric": metric,
        "omega": omega,
        "omega_enabled": omega_enabled,
        "display": f"{metric} {'w=' + _format_number(omega) if omega_enabled else 'w=off'}",
    }


def compile_solve_score_chain_or_legacy(raw_chain, metric, omega, omega_enabled, default_metric=None):
    omega_enabled_value = _parse_boolish(omega_enabled, True)
    legacy_omega = 1.0 if omega in ("", None) else _validate_omega(omega)
    if raw_chain not in ("", None, []):
        compiled = compile_solve_score_chain(raw_chain)
        if not compiled["omega_enabled"] and not omega_enabled_value:
            compiled = {
                **compiled,
                "omega": legacy_omega,
                "display": f"{compiled['metric']} w=off",
            }
        return compiled
    metric_value = str(metric or default_metric or "").strip()
    if not metric_value:
        raise RuntimeError("solve-score metadata is missing a metric")
    compiled_chain = solve_score_chain_from_scalars(
        metric_value,
        legacy_omega,
        omega_enabled_value,
    )
    compiled = compile_solve_score_chain(compiled_chain)
    if not compiled["omega_enabled"] and not omega_enabled_value:
        compiled = {
            **compiled,
            "omega": legacy_omega,
            "display": f"{compiled['metric']} w=off",
        }
    return compiled


def emit_solve_score_metadata(scope, metric, quantile, omega, omega_enabled, chain=None):
    fields = _scope_fields(scope)
    compiled = compile_solve_score_chain_or_legacy(
        chain,
        metric,
        omega,
        omega_enabled,
        default_metric=metric,
    )
    metadata = {
        fields["chain"]: serialize_solve_score_chain(compiled["chain"]),
        fields["metric"]: compiled["metric"],
        fields["quantile"]: "" if quantile in ("", None) else str(quantile),
        fields["omega"]: str(compiled["omega"]),
        fields["omega_enabled"]: "true" if compiled["omega_enabled"] else "false",
    }
    return metadata


def read_solve_score_metadata(scope, meta, default_metric=None, default_omega_enabled=True):
    fields = _scope_fields(scope)
    compiled = compile_solve_score_chain_or_legacy(
        (meta or {}).get(fields["chain"], ""),
        (meta or {}).get(fields["metric"], default_metric or ""),
        (meta or {}).get(fields["omega"], 1.0),
        (meta or {}).get(fields["omega_enabled"], default_omega_enabled),
        default_metric=default_metric,
    )
    quantile_raw = (meta or {}).get(fields["quantile"], "")
    quantile = None if quantile_raw in ("", None) else float(quantile_raw)
    return {
        "chain": compiled["chain"],
        "chain_json": serialize_solve_score_chain(compiled["chain"]),
        "metric": compiled["metric"],
        "quantile": quantile,
        "omega": compiled["omega"],
        "omega_enabled": compiled["omega_enabled"],
        "display": compiled["display"],
    }
