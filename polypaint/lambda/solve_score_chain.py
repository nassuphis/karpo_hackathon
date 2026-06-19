"""
Shared solve-score chain helpers.

Canonical model:

- metric chips own their quantile parameter, e.g. spread(q=5.0%)
- binary combine chips operate on normalized score maps in postfix/RPN order
- omega_cosine is a unary postfix op and may appear anywhere the stack depth is >= 1

The chain is the canonical source of truth. Legacy scalar fields still exist in
some read paths for old artifacts, but new live contracts should not depend on
them.
"""
from __future__ import annotations

import hashlib
import json
import re

from shared import parse_boolish

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
    "max_re",
    "min_re",
    "max_im",
    "min_im",
    "min_mod",
    "max_mod",
    "min_angular_separation",
    "mean_log_mod",
    "sd_log_mod",
    "inside_unit_fraction",
    "unit_annulus_fraction_01",
    "imag_axis_proximity",
    "diagonal_proximity",
    "angular_entropy_16",
    "sector_max_share_16",
    "angular_order_2",
    "angular_order_3",
    "angular_order_4",
    "t1_re",
    "t1_im",
    "t1_abs",
    "t1_phase",
    "t2_re",
    "t2_im",
    "t2_abs",
    "t2_phase",
}
# Public persisted/wire name; changing this requires migrating saved S3 programs.
GENERIC_METRIC_PUBLIC_NAME = "metric"
GENERIC_METRIC_CHIP_NAME = "__metric"
GENERIC_METRIC_SOURCES = {"slv", "cf"}
VALID_SOLVE_SCORE_SOURCES = {"slv", "cf", "pm"}
VALID_SOLVE_SCORE_LAG_DEPTHS = {0, 1}
PARAM_SOLVE_SCORE_METRICS = {
    "t1_re",
    "t1_im",
    "t1_abs",
    "t1_phase",
    "t2_re",
    "t2_im",
    "t2_abs",
    "t2_phase",
}
PARAM_CAPABLE_SOLVE_SCORE_METRICS = {
    "max_re",
    "min_re",
    "max_im",
    "min_im",
    "min_mod",
    "max_mod",
}
_METRIC_ALLOWED_SOURCES = {
    **{
        metric: {"slv", "cf"}
        for metric in VALID_SOLVE_SCORE_METRICS - PARAM_SOLVE_SCORE_METRICS - PARAM_CAPABLE_SOLVE_SCORE_METRICS
    },
    **{metric: {"slv", "cf", "pm"} for metric in PARAM_CAPABLE_SOLVE_SCORE_METRICS},
    **{metric: {"pm"} for metric in PARAM_SOLVE_SCORE_METRICS},
}
TRANSFER_CHIP_NAME = "omega_cosine"
UNARY_CHIPS = {
    TRANSFER_CHIP_NAME: {"arity": 1, "params": 2},
    "sawtooth": {"arity": 1, "params": 1},
    "flip": {"arity": 1, "params": 0},
    "clamp": {"arity": 1, "params": 0},
    "sin": {"arity": 1, "params": 0},
    "cos": {"arity": 1, "params": 0},
    "log": {"arity": 1, "params": 0},
    "exp": {"arity": 1, "params": 0},
    "pow": {"arity": 1, "params": 1},
}
COMBINE_CHIPS = {
    "avg": {"arity": 2, "params": 0},
    "min": {"arity": 2, "params": 0},
    "max": {"arity": 2, "params": 0},
    "mul": {"arity": 2, "params": 0},
    "add": {"arity": 2, "params": 0},
    "mult": {"arity": 2, "params": 0},
    "subtract": {"arity": 2, "params": 0},
    "ratio": {"arity": 2, "params": 0},
    "ema": {"arity": 2, "params": 1},
    "weighted_sum": {"arity": 2, "params": 2},
    "abs_diff": {"arity": 2, "params": 0},
    "geometric_mean": {"arity": 2, "params": 0},
}
OUTPUT_CHIPS = {
    "emit": {"params": (0, 1)},
    "emit_norm": {"params": (0,), "legacy_alias": ("emit", "norm")},
    "emit_none": {"params": (0,), "legacy_alias": ("emit", "none")},
}
STACK_CHIPS = {
    "const": {"arity": 0, "params": 1, "delta": 1},
    "dup": {"arity": 1, "params": 0, "delta": 1},
    "flush": {"arity": 0, "params": 0, "delta": None},
}
MAX_METRIC_SLOTS = 16
MAX_PROGRAM_TOKENS = 32
MAX_OUTPUT_CHANNELS = 8

_FIELD_MAP = {
    "solve": {
        "chain": "solve_score_chain",
        "fingerprint": "solve_score_chain_fingerprint",
        "metric": "solve_metric",
        "quantile": "solve_score_quantile",
        "omega": "solve_score_omega",
        "omega_enabled": "solve_score_omega_enabled",
    },
    "palette_source": {
        "chain": "palette_source_score_chain",
        "fingerprint": "palette_source_chain_fingerprint",
        "metric": "palette_source_metric",
        "quantile": "palette_source_quantile",
        "omega": "palette_source_omega",
        "omega_enabled": "palette_source_omega_enabled",
    },
    "associated_palette": {
        "chain": "associated_palette_score_chain",
        "fingerprint": "associated_palette_chain_fingerprint",
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


def _validate_metric(value):
    metric = str(value or "").strip()
    if metric not in VALID_SOLVE_SCORE_METRICS:
        raise RuntimeError(f"Invalid solve-score metric: {value!r}")
    return metric


def _metric_allowed_sources(metric):
    metric_name = _validate_metric(metric)
    return _METRIC_ALLOWED_SOURCES[metric_name]


def _metric_allowed_for_generic_chip(metric):
    allowed = _metric_allowed_sources(metric)
    return GENERIC_METRIC_SOURCES.issubset(allowed)


def _is_generic_metric_chip_name(name):
    return name in {GENERIC_METRIC_CHIP_NAME, GENERIC_METRIC_PUBLIC_NAME}


def _validate_metric_source_for_metric(metric, source):
    metric_name = _validate_metric(metric)
    source_name = _validate_metric_source(source)
    allowed = _metric_allowed_sources(metric_name)
    if source_name not in allowed:
        allowed_text = ", ".join(sorted(allowed))
        raise RuntimeError(
            f"solve-score metric {metric_name} only supports source(s) {allowed_text}, got {source_name!r}"
        )
    return source_name


def _validate_metric_source(value):
    source = str(value or "").strip().lower()
    if source not in VALID_SOLVE_SCORE_SOURCES:
        raise RuntimeError(f"solve-score metric source must be one of cf, pm, slv, got {value!r}")
    return source


def _split_metric_source_lag(value):
    source = str(value or "").strip().lower()
    if source in VALID_SOLVE_SCORE_SOURCES:
        return source, 0
    if source.endswith("-1"):
        base = source[:-2]
        if base in VALID_SOLVE_SCORE_SOURCES:
            return base, 1
    if "-" in source:
        raise RuntimeError("solve-score lag suffix must be exactly -1 in v1")
    raise RuntimeError(f"solve-score metric source must be one of cf, pm, slv, got {value!r}")


def _format_metric_source_lag(source, lag_depth):
    base = _validate_metric_source(source)
    lag = int(lag_depth or 0)
    if lag not in VALID_SOLVE_SCORE_LAG_DEPTHS:
        raise RuntimeError(f"solve-score lag depth {lag} is unsupported")
    return base if lag == 0 else f"{base}-{lag}"


def _validate_omega(value):
    try:
        omega = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_omega must be numeric, got {value!r}")
    if not (omega == omega and abs(omega) != float("inf")):
        raise RuntimeError(f"solve_score_omega must be finite, got {value!r}")
    return omega


def _validate_omega_phase(value):
    try:
        phase = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_omega_phase must be numeric, got {value!r}")
    if not (phase == phase and abs(phase) != float("inf")):
        raise RuntimeError(f"solve_score_omega_phase must be finite, got {value!r}")
    return phase


def _validate_finite_number(value, label):
    try:
        number = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{label} must be numeric, got {value!r}")
    if not (number == number and abs(number) != float("inf")):
        raise RuntimeError(f"{label} must be finite, got {value!r}")
    return number


def _validate_emit_mode(params, *, legacy_name="emit"):
    values = list(params or [])
    if legacy_name == "emit_norm":
        if values:
            raise RuntimeError("emit_norm takes no parameters")
        return "norm"
    if legacy_name == "emit_none":
        if values:
            raise RuntimeError("emit_none takes no parameters")
        return "none"
    if len(values) == 0:
        return "raw"
    if len(values) != 1:
        raise RuntimeError("emit requires zero parameters or one mode parameter: raw, norm, or none")
    mode = str(values[0] or "").strip().lower()
    if mode in ("raw", "emit"):
        return "raw"
    if mode in ("norm", "normalize", "normalized", "emit_norm"):
        return "norm"
    if mode in ("none", "skip", "off", "emit_none", "emit-none"):
        return "none"
    raise RuntimeError(f"emit mode must be raw, norm, or none, got {values[0]!r}")


def _emit_token_for_mode(mode):
    normalized = str(mode).strip().lower()
    if normalized == "norm":
        return "emit_norm"
    if normalized == "none":
        return "emit_none"
    return "emit"


def _validate_quantile_fraction(value):
    try:
        quantile = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_quantile must be numeric, got {value!r}")
    if not (0.001 <= quantile <= 0.05):
        raise RuntimeError(f"solve_score_quantile must be in [0.001, 0.05], got {quantile}")
    return quantile


def _validate_quantile_percent(value):
    try:
        pct = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve-score metric q must be numeric percent, got {value!r}")
    if not (0.1 <= pct <= 5.0):
        raise RuntimeError(f"solve-score metric q must be in [0.1, 5.0] percent, got {pct}")
    return pct


def _format_number(value):
    num = float(value)
    if num.is_integer():
        return str(int(num))
    return f"{num:g}"


def _format_quantile_percent(value):
    return _format_number(_validate_quantile_percent(value))


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
    def _serialize_item(item):
        name = item["name"]
        params = list(item.get("params") or [])
        if _is_generic_metric_chip_name(name):
            return [GENERIC_METRIC_PUBLIC_NAME, *params]
        if name == "emit_norm":
            return ["emit", "norm"]
        if name == "emit_none":
            return ["emit", "none"]
        if name == "emit":
            return ["emit", _validate_emit_mode(params)]
        if name in VALID_SOLVE_SCORE_METRICS and len(params) == 2 and params[0] == "slv":
            params = [params[1]]
        return [name, *params] if params else name

    return json.dumps(
        [_serialize_item(item) for item in normalize_solve_score_chain(chain)],
        separators=(",", ":"),
    )


def public_solve_score_chain(chain):
    return json.loads(serialize_solve_score_chain(chain))


def _metric_item(metric, quantile, source="slv"):
    metric_name = _validate_metric(metric)
    q = _validate_quantile_fraction(quantile)
    source_name = _validate_metric_source_for_metric(metric_name, source)
    return {"name": metric_name, "params": [source_name, _format_quantile_percent(q * 100.0)]}


def solve_score_chain_from_scalars(metric, quantile=0.001, omega=1.0, omega_enabled=True):
    chain = [_metric_item(metric, quantile)]
    if parse_boolish(omega_enabled, True, strict=True, label="solve_score_omega_enabled"):
        chain.append({"name": TRANSFER_CHIP_NAME, "params": [_format_number(_validate_omega(omega))]})
    return chain


def _normalize_generic_metric_chip(item):
    params = list(item.get("params") or [])
    if len(params) != 3:
        raise RuntimeError("Generic metric chip requires metric, source, and q parameters")
    metric_name = _validate_metric(params[0])
    if not _metric_allowed_for_generic_chip(metric_name):
        raise RuntimeError(
            f"Generic metric chip requires a metric that supports both slv and cf sources, got {metric_name!r}"
        )
    source, lag_depth = _split_metric_source_lag(params[1])
    if source not in GENERIC_METRIC_SOURCES:
        raise RuntimeError("Generic metric chip source must be one of slv, cf, slv-1, cf-1")
    source = _validate_metric_source_for_metric(metric_name, source)
    q_pct = _validate_quantile_percent(params[2])
    normalized = {
        "name": GENERIC_METRIC_CHIP_NAME,
        "params": [
            metric_name,
            _format_metric_source_lag(source, lag_depth),
            _format_quantile_percent(q_pct),
        ],
    }
    expanded = {
        "name": metric_name,
        "params": [_format_metric_source_lag(source, lag_depth), _format_quantile_percent(q_pct)],
    }
    return normalized, expanded, metric_name, source, lag_depth, q_pct / 100.0


def _metric_items_with_fallback(chain, legacy_quantile):
    fallback = None if legacy_quantile in ("", None) else _validate_quantile_fraction(legacy_quantile)
    total_metric_chips = sum(
        1
        for item in chain
        if item["name"] in VALID_SOLVE_SCORE_METRICS or _is_generic_metric_chip_name(item["name"])
    )
    if fallback is None and total_metric_chips == 1:
        fallback = 0.001
    items = []
    for item in chain:
        if _is_generic_metric_chip_name(item["name"]):
            items.append(_normalize_generic_metric_chip(item))
            continue
        if item["name"] not in VALID_SOLVE_SCORE_METRICS:
            items.append((item, item, None, None, 0, None))
            continue
        params = list(item.get("params") or [])
        if not params:
            if fallback is None or total_metric_chips != 1:
                raise RuntimeError(
                    f"Metric chip {item['name']} requires q in percent, e.g. "
                    f"{item['name']}(slv,0.1)"
                )
            params = ["slv", _format_quantile_percent(fallback * 100.0)]
        elif len(params) == 1:
            try:
                source, lag_depth = _split_metric_source_lag(params[0])
            except RuntimeError:
                source = "slv"
                lag_depth = 0
                q_pct = _validate_quantile_percent(params[0])
                normalized = {"name": item["name"], "params": [source, _format_quantile_percent(q_pct)]}
                items.append((normalized, normalized, item["name"], source, lag_depth, q_pct / 100.0))
                continue
            if fallback is None or total_metric_chips != 1:
                raise RuntimeError(
                    f"Metric chip {item['name']} with source {_format_metric_source_lag(source, lag_depth)} "
                    f"requires q in percent, e.g. {item['name']}({_format_metric_source_lag(source, lag_depth)},0.1)"
                )
            params = [
                _format_metric_source_lag(_validate_metric_source_for_metric(item["name"], source), lag_depth),
                _format_quantile_percent(fallback * 100.0),
            ]
        elif len(params) != 2:
            raise RuntimeError(f"Metric chip {item['name']} requires source and q parameters")
        source, lag_depth = _split_metric_source_lag(params[0])
        source = _validate_metric_source_for_metric(item["name"], source)
        q_pct = _validate_quantile_percent(params[1])
        normalized = {
            "name": item["name"],
            "params": [_format_metric_source_lag(source, lag_depth), _format_quantile_percent(q_pct)],
        }
        items.append((normalized, normalized, item["name"], source, lag_depth, q_pct / 100.0))
    return items


def _build_program_spec(program_tokens):
    parts = []
    for token in program_tokens:
        kind = token["kind"]
        if kind == "metric":
            parts.append(f"m{int(token['slot'])}-{int(token.get('lag', 0) or 0)}")
        elif kind == "const":
            parts.append(f"const:{_format_number(token['value'])}")
        elif kind == "weighted_sum":
            parts.append(f"weighted_sum:{_format_number(token['a'])}:{_format_number(token['b'])}")
        elif kind == "ema":
            parts.append(f"ema:{_format_number(token['alpha'])}")
        elif kind == "pow":
            parts.append(f"pow:{_format_number(token['exponent'])}")
        elif kind == TRANSFER_CHIP_NAME:
            phase = float(token.get("phase", 0.0))
            if abs(phase) < 1e-12:
                parts.append(f"{TRANSFER_CHIP_NAME}:{_format_number(token['omega'])}")
            else:
                parts.append(
                    f"{TRANSFER_CHIP_NAME}:{_format_number(token['omega'])}:{_format_number(phase)}"
                )
        elif kind == "sawtooth":
            parts.append(f"sawtooth:{_format_number(token['mult'])}")
        elif kind == "emit":
            parts.append(_emit_token_for_mode(token.get("mode", "raw")))
        else:
            parts.append(kind)
    return ";".join(parts)


def _canonical_metric_ref_token(token):
    match = re.fullmatch(r"m([0-9]+)(?:-([0-9]+))?", token)
    if not match:
        if re.fullmatch(r"[mM](?:[0-9]|-).*", token):
            raise RuntimeError(f"Malformed solve-score metric reference token: {token!r}")
        return None

    slot_text = match.group(1)
    lag_text = match.group(2)
    if slot_text != str(int(slot_text)):
        raise RuntimeError(f"Malformed solve-score metric slot in token: {token!r}")
    if lag_text is None:
        lag = 0
    elif lag_text in ("0", "1"):
        lag = int(lag_text)
    else:
        raise RuntimeError(f"solve-score lag depth in token {token!r} must be 0 or 1")
    return f"m{int(slot_text)}-{lag}"


def canonicalize_solve_score_program_spec(program_spec):
    parts = []
    for raw in str(program_spec or "").split(";"):
        token = raw.strip()
        if not token:
            continue
        metric_ref = _canonical_metric_ref_token(token)
        if metric_ref is None:
            parts.append(token)
        else:
            parts.append(metric_ref)
    return ";".join(parts)


def solve_score_program_spec_uses_lag(program_spec):
    for raw in str(program_spec or "").split(";"):
        token = raw.strip()
        if not token:
            continue
        metric_ref = _canonical_metric_ref_token(token)
        if metric_ref and metric_ref.endswith("-1"):
            return True
    return False


def _metrics_csv(metrics, field):
    values = []
    for metric in metrics:
        if field == "metric":
            values.append(str(metric["metric"]))
            continue
        if field == "source":
            values.append(_validate_metric_source(metric.get("source", "slv")))
            continue
        value = metric.get(field)
        if value in ("", None):
            raise RuntimeError(f"solve-score metric slot {metric.get('slot')} is missing {field}")
        values.append(_format_number(value))
    return ",".join(values)


def _token_display(item):
    name = item["name"]
    params = item.get("params") or []
    if _is_generic_metric_chip_name(name):
        metric_name = params[0] if len(params) > 0 else "?"
        source = params[1] if len(params) > 1 else "slv"
        q = params[2] if len(params) > 2 else "?"
        try:
            base_source, lag_depth = _split_metric_source_lag(source)
            display_source = _format_metric_source_lag(base_source, lag_depth)
        except RuntimeError:
            display_source = str(source or "slv")
        return f"metric({metric_name},{display_source},q={q}%)"
    if name in VALID_SOLVE_SCORE_METRICS:
        source = params[0] if len(params) > 0 else "slv"
        q = params[1] if len(params) > 1 else (params[0] if params else "?")
        try:
            base_source, lag_depth = _split_metric_source_lag(source)
            display_source = _format_metric_source_lag(base_source, lag_depth)
        except RuntimeError:
            display_source = str(source or "slv")
        if display_source == "slv":
            return f"{name}(q={q}%)"
        return f"{name}({display_source},q={q}%)"
    if name == TRANSFER_CHIP_NAME:
        omega = params[0] if len(params) > 0 else "?"
        phase = params[1] if len(params) > 1 else "0"
        try:
            phase_value = float(phase)
        except (TypeError, ValueError):
            phase_value = None
        if phase_value is not None and abs(phase_value) < 1e-12:
            return f"ω-cos({omega})"
        return f"ω-cos({omega},{phase})"
    if name == "sawtooth":
        return f"sawtooth({params[0] if params else '?'})"
    if name == "weighted_sum":
        a = params[0] if len(params) > 0 else "?"
        b = params[1] if len(params) > 1 else "?"
        return f"weighted_sum({a},{b})"
    if name in OUTPUT_CHIPS:
        if name == "emit_norm":
            return "emit(norm)"
        if name == "emit_none":
            return "emit(none)"
        return f"emit({_validate_emit_mode(params)})"
    return f"{name}({','.join(params)})" if params else name


def format_solve_score_chain_display(chain, legacy_quantile=None):
    compiled = compile_solve_score_chain(chain, legacy_quantile=legacy_quantile)
    return compiled["display"]


def solve_score_chain_id(chain, legacy_quantile=None):
    compiled = compile_solve_score_chain(chain, legacy_quantile=legacy_quantile)
    semantic_chain = compiled.get("expanded_chain") or compiled["chain"]
    return hashlib.sha1(serialize_solve_score_chain(semantic_chain).encode("utf-8")).hexdigest()[:12]


def compiled_solve_score_fingerprint(compiled):
    payload = {
        "program_spec": str(compiled["program_spec"]),
        "metrics": [
            {
                "slot": int(metric["slot"]),
                "metric": str(metric["metric"]),
                "source": str(metric.get("source", "slv")),
                "quantile": float(metric["quantile"]),
            }
            for metric in (compiled.get("metrics") or [])
        ],
    }
    encoded = json.dumps(payload, sort_keys=True, separators=(",", ":"))
    return f"sha256:{hashlib.sha256(encoded.encode('utf-8')).hexdigest()}"


def solve_score_chain_fingerprint(raw_chain, metric=None, quantile=None, omega=None, omega_enabled=None, default_metric=None):
    compiled = compile_solve_score_chain_or_legacy(
        raw_chain,
        metric,
        quantile,
        omega,
        omega_enabled,
        default_metric=default_metric,
    )
    return compiled_solve_score_fingerprint(compiled)


def solve_score_program_cli_payload(compiled_or_metrics):
    if isinstance(compiled_or_metrics, dict) and "metrics" in compiled_or_metrics:
        metrics = compiled_or_metrics["metrics"]
        program_spec = compiled_or_metrics["program_spec"]
    else:
        raise RuntimeError("solve_score_program_cli_payload expects a compiled solve-score chain")
    payload = {
        "score_metrics": _metrics_csv(metrics, "metric"),
        "score_clip_los": _metrics_csv(metrics, "clip_lo"),
        "score_clip_his": _metrics_csv(metrics, "clip_hi"),
        "score_program": program_spec,
    }
    if any(_validate_metric_source(metric.get("source", "slv")) != "slv" for metric in metrics):
        payload["score_sources"] = _metrics_csv(metrics, "source")
    return payload


def solve_score_uses_source(compiled_or_metrics, source):
    metrics = compiled_or_metrics.get("metrics") if isinstance(compiled_or_metrics, dict) else compiled_or_metrics
    if not isinstance(metrics, list):
        return False
    wanted = _validate_metric_source(source)
    return any(_validate_metric_source(metric.get("source", "slv")) == wanted for metric in metrics)


def solve_score_uses_non_solve_sources(compiled_or_metrics):
    metrics = compiled_or_metrics.get("metrics") if isinstance(compiled_or_metrics, dict) else compiled_or_metrics
    if not isinstance(metrics, list):
        return False
    return any(_validate_metric_source(metric.get("source", "slv")) != "slv" for metric in metrics)


def solve_score_lag_prelude_by_source(compiled):
    if not isinstance(compiled, dict):
        return {"slv": 0, "cf": 0, "pm": 0}
    prelude = dict(compiled.get("prelude_by_source") or {})
    return {
        "slv": 1 if int(prelude.get("slv") or 0) > 0 else 0,
        "cf": 1 if int(prelude.get("cf") or 0) > 0 else 0,
        "pm": 1 if int(prelude.get("pm") or 0) > 0 else 0,
    }


def compile_solve_score_chain(raw_chain, legacy_quantile=None):
    raw_items = normalize_solve_score_chain(raw_chain)
    if not raw_items:
        raise RuntimeError("solve_score_chain must contain at least one metric chip")

    normalized_with_metrics = _metric_items_with_fallback(raw_items, legacy_quantile)
    chain = [item for item, _, _, _, _, _ in normalized_with_metrics]
    expanded_chain = [expanded for _, expanded, _, _, _, _ in normalized_with_metrics]
    current_slot_candidates = {}
    for item, _expanded, item_metric_name, metric_source, metric_lag, metric_quantile in normalized_with_metrics:
        if not item_metric_name or int(metric_lag or 0) != 0:
            continue
        metric_name = _validate_metric(item_metric_name)
        source_name = _validate_metric_source_for_metric(metric_name, metric_source)
        q = _validate_quantile_fraction(metric_quantile)
        family_key = (metric_name, source_name)
        slot_key = (metric_name, source_name, q)
        candidates = current_slot_candidates.setdefault(family_key, [])
        if slot_key not in candidates:
            candidates.append(slot_key)
    metrics = []
    base_slot_keys = {}
    program_tokens = []
    stack_depth = 0
    omega = 1.0
    omega_phase = 0.0
    omega_enabled = False
    output_channels = []
    has_explicit_output_token = False

    for item, _expanded, item_metric_name, metric_source, metric_lag, metric_quantile in normalized_with_metrics:
        name = item["name"]
        params = item.get("params") or []
        if item_metric_name:
            metric_name = _validate_metric(item_metric_name)
            source_name = _validate_metric_source_for_metric(metric_name, metric_source)
            lag_depth = int(metric_lag or 0)
            if lag_depth not in VALID_SOLVE_SCORE_LAG_DEPTHS:
                raise RuntimeError(f"solve-score lag depth {lag_depth} is unsupported")
            q = _validate_quantile_fraction(metric_quantile)
            if lag_depth == 0:
                slot_key = (metric_name, source_name, q)
            else:
                candidates = current_slot_candidates.get((metric_name, source_name), [])
                if len(candidates) == 1:
                    slot_key = candidates[0]
                elif len(candidates) == 0:
                    slot_key = (metric_name, source_name, q)
                else:
                    raise RuntimeError(
                        f"lagged metric reference {metric_name}({_format_metric_source_lag(source_name, lag_depth)}) "
                        "is ambiguous because multiple base slots exist for that metric/source"
                    )
            slot = base_slot_keys.get(slot_key)
            if slot is None:
                slot = len(metrics)
                if slot >= MAX_METRIC_SLOTS:
                    raise RuntimeError(f"solve_score_chain supports at most {MAX_METRIC_SLOTS} metric chips")
                slot_metric, slot_source, slot_q = slot_key
                metrics.append(
                    {
                        "slot": slot,
                        "source": slot_source,
                        "metric": slot_metric,
                        "quantile": slot_q,
                        "quantile_pct": slot_q * 100.0,
                        "clip_lo": None,
                        "clip_hi": None,
                    }
                )
                base_slot_keys[slot_key] = slot
            program_tokens.append({"kind": "metric", "slot": slot, "metric": metric_name, "lag": lag_depth})
            stack_depth += 1
            continue

        if name == "const":
            if len(params) != 1:
                raise RuntimeError("const requires exactly one finite numeric parameter")
            value = _validate_finite_number(params[0], "const")
            program_tokens.append({"kind": "const", "value": value})
            stack_depth += 1
            continue
        if name == "dup":
            if stack_depth < 1:
                raise RuntimeError("dup requires one score value on the stack")
            if len(params) != 0:
                raise RuntimeError("dup takes no parameters")
            program_tokens.append({"kind": "dup"})
            stack_depth += 1
            continue
        if name == "flush":
            if len(params) != 0:
                raise RuntimeError("flush takes no parameters")
            program_tokens.append({"kind": "flush"})
            stack_depth = 0
            continue

        if name == TRANSFER_CHIP_NAME:
            if stack_depth < 1:
                raise RuntimeError(f"{TRANSFER_CHIP_NAME} requires one score value on the stack")
            if len(params) == 1:
                params = [params[0], "0"]
            if len(params) != 2:
                raise RuntimeError(f"{TRANSFER_CHIP_NAME} requires exactly two parameters: omega and phase")
            omega = _validate_omega(params[0])
            omega_phase = _validate_omega_phase(params[1])
            program_tokens.append({"kind": TRANSFER_CHIP_NAME, "omega": omega, "phase": omega_phase})
            omega_enabled = True
            continue
        if name == "sawtooth":
            if stack_depth < 1:
                raise RuntimeError("sawtooth requires one score value on the stack")
            if len(params) != 1:
                raise RuntimeError("sawtooth requires exactly one multiplier parameter")
            try:
                mult = float(params[0])
            except (TypeError, ValueError):
                raise RuntimeError("sawtooth requires one numeric multiplier")
            if not (mult == mult and abs(mult) != float("inf")):
                raise RuntimeError("sawtooth requires one finite numeric multiplier")
            program_tokens.append({"kind": "sawtooth", "mult": mult})
            continue
        if name == "flip":
            if stack_depth < 1:
                raise RuntimeError("flip requires one score value on the stack")
            if len(params) != 0:
                raise RuntimeError("flip takes no parameters")
            program_tokens.append({"kind": "flip"})
            continue
        if name == "clamp":
            if stack_depth < 1:
                raise RuntimeError("clamp requires one score value on the stack")
            if len(params) != 0:
                raise RuntimeError("clamp takes no parameters")
            program_tokens.append({"kind": "clamp"})
            continue
        if name in ("sin", "cos", "log", "exp"):
            if stack_depth < 1:
                raise RuntimeError(f"{name} requires one score value on the stack")
            if len(params) != 0:
                raise RuntimeError(f"{name} takes no parameters")
            program_tokens.append({"kind": name})
            continue
        if name == "pow":
            if stack_depth < 1:
                raise RuntimeError("pow requires one score value on the stack")
            if len(params) != 1:
                raise RuntimeError("pow requires exactly one exponent parameter")
            exponent = _validate_finite_number(params[0], "pow exponent")
            program_tokens.append({"kind": "pow", "exponent": exponent})
            continue

        if name in OUTPUT_CHIPS:
            if stack_depth < 1:
                raise RuntimeError(f"{name} at chip {len(program_tokens)} requires stack depth at least 1")
            mode = _validate_emit_mode(params, legacy_name=name)
            has_explicit_output_token = True
            channel = len(output_channels)
            native_emit = _emit_token_for_mode(mode)
            program_tokens.append({"kind": "emit", "mode": mode, "channel": channel if mode != "none" else None})
            if mode != "none":
                if len(output_channels) >= MAX_OUTPUT_CHANNELS:
                    raise RuntimeError(f"solve_score_chain supports at most {MAX_OUTPUT_CHANNELS} output channels")
                output_channels.append(
                    {
                        "name": f"channel_{channel}",
                        "emit": native_emit,
                        "mode": mode,
                        "channel": channel,
                        "range_normalized": mode == "norm",
                    }
                )
            stack_depth -= 1
            continue

        spec = COMBINE_CHIPS.get(name)
        if not spec:
            raise RuntimeError(f"Invalid solve-score chip: {name!r}")
        if stack_depth < spec["arity"]:
            raise RuntimeError(f"{name} requires {spec['arity']} inputs but the current stack depth is {stack_depth}")
        if len(params) != spec["params"]:
            raise RuntimeError(f"{name} requires exactly {spec['params']} parameter(s)")
        token = {"kind": name}
        if name == "weighted_sum":
            try:
                a = float(params[0])
                b = float(params[1])
            except (TypeError, ValueError):
                raise RuntimeError(f"{name} requires two numeric weights")
            if not (abs(a) > 0 or abs(b) > 0):
                raise RuntimeError(f"{name} requires at least one non-zero weight")
            token["a"] = a
            token["b"] = b
        elif name == "ema":
            alpha = _validate_finite_number(params[0], "ema alpha")
            if not (0.0 <= alpha <= 1.0):
                raise RuntimeError(f"ema alpha must be in [0, 1], got {alpha}")
            token["alpha"] = alpha
        program_tokens.append(token)
        stack_depth -= spec["arity"] - 1

    if not metrics:
        raise RuntimeError("solve_score_chain must contain at least one metric chip")
    has_explicit_outputs = has_explicit_output_token
    if has_explicit_outputs:
        if not output_channels:
            raise RuntimeError("explicit-output solve_score_chain must emit at least one channel")
        if stack_depth != 0:
            raise RuntimeError(f"explicit-output solve_score_chain must end with stack depth 0, got {stack_depth}")
    else:
        if stack_depth != 1:
            raise RuntimeError(f"solve_score_chain must end with stack depth 1, got {stack_depth}")
        output_channels = [
            {
                "name": "score",
                "emit": "implicit",
                "channel": 0,
                "range_normalized": False,
            }
        ]
    if len(program_tokens) > MAX_PROGRAM_TOKENS:
        raise RuntimeError(f"solve_score_chain supports at most {MAX_PROGRAM_TOKENS} program tokens")

    primary_metric = metrics[0]["metric"]
    primary_quantile = metrics[0]["quantile"]
    display = " ".join(_token_display(item) for item in chain)
    program_spec = _build_program_spec(program_tokens)
    uses_lag = any(
        token.get("kind") == "metric" and int(token.get("lag", 0) or 0) > 0
        for token in program_tokens
    )
    lagged_metric_slots = sorted({
        int(token["slot"])
        for token in program_tokens
        if token.get("kind") == "metric" and int(token.get("lag", 0) or 0) > 0
    })
    lagged_sources = sorted({
        metrics[int(token["slot"])].get("source", "slv")
        for token in program_tokens
        if token.get("kind") == "metric" and int(token.get("lag", 0) or 0) > 0
    })
    prelude_by_source = {
        "slv": 1 if "slv" in lagged_sources else 0,
        "cf": 1 if "cf" in lagged_sources else 0,
        "pm": 1 if "pm" in lagged_sources else 0,
    }
    all_solve_sources = all(metric.get("source", "slv") == "slv" for metric in metrics)
    legacy_compatible = (not has_explicit_outputs) and all_solve_sources and (
        (
            not uses_lag
            and
            len(program_tokens) == 1
            and program_tokens[0]["kind"] == "metric"
        ) or (
            not uses_lag
            and
            len(program_tokens) == 2
            and program_tokens[0]["kind"] == "metric"
            and program_tokens[1]["kind"] == TRANSFER_CHIP_NAME
            and abs(program_tokens[1]["phase"]) < 1e-12
        )
    )
    return {
        "chain": chain,
        "expanded_chain": expanded_chain,
        "metrics": metrics,
        "metric_count": len(metrics),
        "metric": primary_metric,
        "quantile": primary_quantile,
        "omega": omega,
        "omega_phase": omega_phase,
        "omega_enabled": omega_enabled,
        "program_tokens": program_tokens,
        "program_spec": program_spec,
        "program_id": hashlib.sha1(program_spec.encode("utf-8")).hexdigest()[:12],
        "has_explicit_outputs": has_explicit_outputs,
        "output_channel_count": len(output_channels),
        "output_channels": output_channels,
        "output_interpretation": "scalar_palette" if not has_explicit_outputs else "",
        "uses_lag": uses_lag,
        "max_lag": 1 if uses_lag else 0,
        "lagged_metric_slots": lagged_metric_slots,
        "lagged_sources": lagged_sources,
        "prelude_by_source": prelude_by_source,
        "legacy_compatible": legacy_compatible,
        "display": display,
    }


def compile_solve_score_chain_or_legacy(
    raw_chain,
    metric,
    quantile=None,
    omega=None,
    omega_enabled=None,
    default_metric=None,
):
    # Backward-compatible call shape:
    #   (raw_chain, metric, omega, omega_enabled, default_metric=...)
    if omega_enabled is None:
        legacy_quantile = 0.001
        legacy_omega_input = quantile
        legacy_omega_enabled_input = omega
    else:
        legacy_quantile = _validate_quantile_fraction(quantile if quantile not in ("", None) else 0.001)
        legacy_omega_input = omega
        legacy_omega_enabled_input = omega_enabled
    omega_enabled_value = parse_boolish(
        legacy_omega_enabled_input,
        True,
        strict=True,
        label="solve_score_omega_enabled",
    )
    legacy_omega = 1.0 if legacy_omega_input in ("", None) else _validate_omega(legacy_omega_input)
    if raw_chain not in ("", None, []):
        compiled = compile_solve_score_chain(raw_chain, legacy_quantile=legacy_quantile)
        if not compiled["omega_enabled"] and not omega_enabled_value:
            compiled = {
                **compiled,
                "omega": legacy_omega,
            }
        return compiled
    metric_value = str(metric or default_metric or "").strip()
    if not metric_value:
        raise RuntimeError("solve-score metadata is missing a metric")
    compiled_chain = solve_score_chain_from_scalars(
        metric_value,
        legacy_quantile,
        legacy_omega,
        omega_enabled_value,
    )
    compiled = compile_solve_score_chain(compiled_chain)
    if not compiled["omega_enabled"] and not omega_enabled_value:
        compiled = {
            **compiled,
            "omega": legacy_omega,
        }
    return compiled


def emit_solve_score_metadata(scope, metric, quantile, omega, omega_enabled, chain=None, include_legacy_scalars=True):
    fields = _scope_fields(scope)
    compiled = compile_solve_score_chain_or_legacy(
        chain,
        metric,
        quantile,
        omega,
        omega_enabled,
        default_metric=metric,
    )
    metadata = {
        fields["chain"]: serialize_solve_score_chain(compiled["chain"]),
        fields["fingerprint"]: compiled_solve_score_fingerprint(compiled),
        fields["quantile"]: "" if compiled["quantile"] in ("", None) else str(compiled["quantile"]),
    }
    if include_legacy_scalars:
        metadata.update({
            fields["metric"]: compiled["metric"],
            fields["omega"]: str(compiled["omega"]),
            fields["omega_enabled"]: "true" if compiled["omega_enabled"] else "false",
        })
    return metadata


def read_solve_score_metadata(scope, meta, default_metric=None, default_omega_enabled=True):
    fields = _scope_fields(scope)
    compiled = compile_solve_score_chain_or_legacy(
        (meta or {}).get(fields["chain"], ""),
        (meta or {}).get(fields["metric"], default_metric or ""),
        (meta or {}).get(fields["quantile"], ""),
        (meta or {}).get(fields["omega"], 1.0),
        (meta or {}).get(fields["omega_enabled"], default_omega_enabled),
        default_metric=default_metric,
    )
    quantile = compiled["quantile"]
    chain_json = serialize_solve_score_chain(compiled["chain"])
    return {
        "chain": compiled["chain"],
        "chain_public": json.loads(chain_json),
        "chain_json": chain_json,
        "chain_fingerprint": (meta or {}).get(fields["fingerprint"]) or compiled_solve_score_fingerprint(compiled),
        "metric": compiled["metric"],
        "metrics": compiled["metrics"],
        "quantile": quantile,
        "omega": compiled["omega"],
        "omega_phase": compiled["omega_phase"],
        "omega_enabled": compiled["omega_enabled"],
        "program_spec": compiled["program_spec"],
        "has_explicit_outputs": compiled.get("has_explicit_outputs", False),
        "output_channel_count": compiled.get("output_channel_count", 1),
        "output_channels": list(compiled.get("output_channels") or []),
        "output_interpretation": compiled.get("output_interpretation", "scalar_palette"),
        "uses_lag": compiled["uses_lag"],
        "lagged_metric_slots": list(compiled.get("lagged_metric_slots") or []),
        "lagged_sources": list(compiled.get("lagged_sources") or []),
        "prelude_by_source": dict(compiled.get("prelude_by_source") or {}),
        "display": compiled["display"],
    }
