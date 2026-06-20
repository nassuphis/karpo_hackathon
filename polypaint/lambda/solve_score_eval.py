"""Pure-Python solve-score evaluator used as the DD5 oracle.

The production path evaluates solve-score programs in C through
``solve_score.h``. This module mirrors that header's metric normalization and
postfix VM closely enough to compare Python-vs-native numerics in unit tests
and to provide a stable oracle before the CR18 VM merge.
"""
from __future__ import annotations

import math
import struct
from dataclasses import dataclass

from solve_score_chain import compile_solve_score_chain_or_legacy, strip_solve_score_version


EPS2 = 1e-300
EPS = 1e-150
MAX_PROGRAM_TOKENS = 32
MAX_OUTPUT_CHANNELS = 8


def _f32(value):
    return struct.unpack("<f", struct.pack("<f", float(value)))[0]


def _finite(value):
    return math.isfinite(float(value))


def _finite_or_zero(value):
    value = float(value)
    return value if math.isfinite(value) else 0.0


def clamp_unit(value):
    value = float(value)
    if not math.isfinite(value):
        return 0.0
    if value < 0.0:
        return 0.0
    if value > 1.0:
        return 1.0
    return value


def apply_omega(value, omega, phase=0.0):
    value = clamp_unit(value)
    omega = float(omega) if math.isfinite(float(omega)) else 1.0
    phase = float(phase) if math.isfinite(float(phase)) else 0.0
    return 0.5 * (math.cos(omega * 2.0 * math.pi * value + phase) + 1.0)


def apply_transfer(value, omega_enabled=True, omega=1.0):
    value = clamp_unit(value)
    return apply_omega(value, omega, 0.0) if omega_enabled else value


def _root_pairs(values):
    if values is None:
        return []
    pairs = []
    for item in values:
        if isinstance(item, complex):
            pairs.append((_f32(item.real), _f32(item.imag)))
        else:
            re, im = item
            pairs.append((_f32(re), _f32(im)))
    return pairs


def _flat_param_pairs(values):
    if values is None:
        return []
    if len(values) == 0:
        return []
    if isinstance(values[0], complex):
        pairs = [(value.real, value.imag) for value in values]
    elif isinstance(values[0], (tuple, list)):
        pairs = [(value[0], value[1]) for value in values]
    else:
        if len(values) % 2 != 0:
            raise RuntimeError("param values must be pairs or an even flat list")
        pairs = [(values[i], values[i + 1]) for i in range(0, len(values), 2)]
    return [(_f32(re), _f32(im)) for re, im in pairs]


def _finite_roots(roots, min_roots):
    roots = _root_pairs(roots)
    finite = [(re, im) for re, im in roots if _finite(re) and _finite(im)]
    if len(finite) < min_roots:
        return []
    return finite


def _median(values):
    if not values:
        return 0.0
    vals = sorted(float(v) for v in values)
    n = len(vals)
    mid = n // 2
    if n % 2:
        return vals[mid]
    return 0.5 * (vals[mid - 1] + vals[mid])


def _mean(values):
    return sum(values) / len(values) if values else 0.0


def _stddev(values, mean):
    if len(values) <= 1:
        return 0.0
    return math.sqrt(sum((v - mean) * (v - mean) for v in values) / len(values))


def _angle_0_2pi(re, im):
    angle = math.atan2(im, re)
    return angle + 2.0 * math.pi if angle < 0.0 else angle


def _centroid(roots):
    return (
        sum(re for re, _im in roots) / len(roots),
        sum(im for _re, im in roots) / len(roots),
    )


def _metric_min_roots(metric):
    if metric in {
        "centroid_re", "centroid_im", "centroid_dist", "dist_unit_circle",
        "asymmetry_re", "max_re", "min_re", "max_im", "min_im",
        "min_mod", "max_mod", "mean_log_mod", "inside_unit_fraction",
        "unit_annulus_fraction_01", "imag_axis_proximity",
        "diagonal_proximity", "sector_max_share_16",
    }:
        return 1
    if metric in {
        "area", "min_angular_separation", "sd_log_mod",
        "angular_entropy_16", "angular_order_2", "angular_order_3",
        "angular_order_4",
    }:
        return 2 if metric != "area" else 3
    return 2


def _param_metric_score(params, metric):
    params = _flat_param_pairs(params)
    if not params:
        return 0.0
    t1_re, t1_im = params[0] if len(params) >= 1 else (0.0, 0.0)
    t2_re, t2_im = params[1] if len(params) >= 2 else (0.0, 0.0)
    if metric == "max_re":
        return max(t1_re, t2_re)
    if metric == "min_re":
        return min(t1_re, t2_re)
    if metric == "max_im":
        return max(t1_im, t2_im)
    if metric == "min_im":
        return min(t1_im, t2_im)
    if metric == "min_mod":
        t1_mod = math.hypot(t1_re, t1_im)
        t2_mod = math.hypot(t2_re, t2_im)
        if t1_mod == 0.0 and t2_mod == 0.0:
            return 0.0
        if t1_mod == 0.0:
            return t2_mod
        if t2_mod == 0.0:
            return t1_mod
        return min(t1_mod, t2_mod)
    if metric == "max_mod":
        return max(math.hypot(t1_re, t1_im), math.hypot(t2_re, t2_im))
    if metric == "t1_re":
        return t1_re
    if metric == "t1_im":
        return t1_im
    if metric == "t1_abs":
        return math.hypot(t1_re, t1_im)
    if metric == "t1_phase":
        return _angle_0_2pi(t1_re, t1_im)
    if metric == "t2_re":
        return t2_re
    if metric == "t2_im":
        return t2_im
    if metric == "t2_abs":
        return math.hypot(t2_re, t2_im)
    if metric == "t2_phase":
        return _angle_0_2pi(t2_re, t2_im)
    return 0.0


def _nearest_neighbor_scores(roots):
    scores = []
    for i, (ri_re, ri_im) in enumerate(roots):
        d2_min = 1e300
        for j, (rj_re, rj_im) in enumerate(roots):
            if i == j:
                continue
            dr = ri_re - rj_re
            di = ri_im - rj_im
            d2 = dr * dr + di * di
            if d2 < d2_min:
                d2_min = d2
        scores.append(-0.5 * math.log10(d2_min if d2_min > EPS2 else EPS2))
    return scores


def _angle_histogram_16(roots):
    counts = [0] * 16
    angle_count = 0
    scale = 16.0 / (2.0 * math.pi)
    for re, im in roots:
        if re == 0.0 and im == 0.0:
            continue
        bin_index = int(_angle_0_2pi(re, im) * scale)
        if bin_index < 0:
            bin_index = 0
        if bin_index > 15:
            bin_index = 15
        counts[bin_index] += 1
        angle_count += 1
    return counts, angle_count


def _angular_order(roots, order):
    c = 0.0
    s = 0.0
    angle_count = 0
    for re, im in roots:
        if re == 0.0 and im == 0.0:
            continue
        theta = _angle_0_2pi(re, im)
        c += math.cos(order * theta)
        s += math.sin(order * theta)
        angle_count += 1
    if angle_count < 2:
        return 0.0
    return math.hypot(c, s) / angle_count


def compute_solve_metric_score(roots, metric):
    metric = str(metric)
    if metric.startswith("t1_") or metric.startswith("t2_"):
        return _param_metric_score(roots, metric)
    roots = _finite_roots(roots, _metric_min_roots(metric))
    if not roots:
        return 0.0
    degree = len(roots)

    if metric == "proximity":
        d2_min = 1e300
        for i, (ri_re, ri_im) in enumerate(roots):
            for rj_re, rj_im in roots[i + 1:]:
                dr = ri_re - rj_re
                di = ri_im - rj_im
                d2_min = min(d2_min, dr * dr + di * di)
        return -0.5 * math.log10(d2_min if d2_min > EPS2 else EPS2)

    if metric == "crowding":
        total = 0.0
        count = 0
        for i, (ri_re, ri_im) in enumerate(roots):
            for rj_re, rj_im in roots[i + 1:]:
                dr = ri_re - rj_re
                di = ri_im - rj_im
                d2 = dr * dr + di * di
                total += -0.5 * math.log10(d2 if d2 > EPS2 else EPS2)
                count += 1
        return total / count if count else 0.0

    if metric in {"clusteriness", "nn_variation"}:
        scores = _nearest_neighbor_scores(roots)
        if metric == "clusteriness":
            return max(scores) - _median(scores)
        return _stddev(scores, _mean(scores))

    if metric == "real_axis_proximity":
        return -math.log10(_median([abs(im) for _re, im in roots]) + EPS)
    if metric == "imag_axis_proximity":
        return -math.log10(_median([abs(re) for re, _im in roots]) + EPS)
    if metric == "diagonal_proximity":
        return -math.log10(_median([abs(abs(re) - abs(im)) for re, im in roots]) + EPS)

    if metric == "mean_log_mod":
        return sum(math.log(math.hypot(re, im) + EPS) for re, im in roots) / degree
    if metric == "sd_log_mod":
        logs = [math.log(math.hypot(re, im) + EPS) for re, im in roots]
        return math.sqrt(sum((v - _mean(logs)) ** 2 for v in logs) / degree)
    if metric == "inside_unit_fraction":
        return sum(1 for re, im in roots if math.hypot(re, im) < 1.0) / degree
    if metric == "unit_annulus_fraction_01":
        return sum(1 for re, im in roots if abs(math.hypot(re, im) - 1.0) < 0.1) / degree

    if metric in {"angular_entropy_16", "sector_max_share_16"}:
        counts, angle_count = _angle_histogram_16(roots)
        if angle_count <= 0:
            return 0.0
        if metric == "sector_max_share_16":
            return max(counts) / angle_count
        if angle_count < 2:
            return 0.0
        entropy = 0.0
        for count in counts:
            if count <= 0:
                continue
            p = count / angle_count
            entropy -= p * math.log(p)
        return entropy / math.log(16.0)

    if metric == "angular_order_2":
        return _angular_order(roots, 2)
    if metric == "angular_order_3":
        return _angular_order(roots, 3)
    if metric == "angular_order_4":
        return _angular_order(roots, 4)

    mean_re, mean_im = _centroid(roots)
    if metric == "centroid_re":
        return mean_re
    if metric == "centroid_im":
        return mean_im
    if metric == "centroid_dist":
        return math.log10(math.hypot(mean_re, mean_im) + EPS)
    if metric == "dist_unit_circle":
        return math.log10(sum(abs(math.hypot(re, im) - 1.0) for re, im in roots) / degree + EPS)
    if metric == "asymmetry_re":
        mean_abs_re = sum(abs(re) for re, _im in roots) / degree
        return abs(mean_re) / (mean_abs_re + EPS)
    if metric == "max_re":
        return max(re for re, _im in roots)
    if metric == "min_re":
        return min(re for re, _im in roots)
    if metric == "max_im":
        return max(im for _re, im in roots)
    if metric == "min_im":
        return min(im for _re, im in roots)
    if metric == "min_mod":
        mods = [math.hypot(re, im) for re, im in roots if not (re == 0.0 and im == 0.0)]
        return min(mods) if mods else 0.0
    if metric == "max_mod":
        return max(math.hypot(re, im) for re, im in roots)
    if metric == "min_angular_separation":
        angles = sorted(_angle_0_2pi(re, im) for re, im in roots if not (re == 0.0 and im == 0.0))
        if len(angles) < 2:
            return 0.0
        gaps = [angles[i] - angles[i - 1] for i in range(1, len(angles))]
        gaps.append(2.0 * math.pi - angles[-1] + angles[0])
        return min(gaps)

    if metric == "spread":
        r2 = sum((re - mean_re) ** 2 + (im - mean_im) ** 2 for re, im in roots) / degree
        return 0.5 * math.log10(r2 if r2 > EPS2 else EPS2)

    if metric in {"shelliness", "outlierness"}:
        radii = [math.hypot(re - mean_re, im - mean_im) for re, im in roots]
        if metric == "shelliness":
            rho_mean = _mean(radii)
            rho_std = _stddev(radii, rho_mean)
            return -math.log10(rho_std / (rho_mean + EPS) + EPS)
        rho_med = _median(radii)
        return math.log10((max(radii) + EPS) / (rho_med + EPS))

    if degree < 2:
        return 0.0
    sxx = sum((re - mean_re) ** 2 for re, _im in roots) / degree
    syy = sum((im - mean_im) ** 2 for _re, im in roots) / degree
    sxy = sum((re - mean_re) * (im - mean_im) for re, im in roots) / degree
    trace = sxx + syy
    det = sxx * syy - sxy * sxy
    disc = math.sqrt(max(trace * trace - 4.0 * det, 0.0))
    lambda_max = max(0.5 * (trace + disc), 0.0)
    lambda_min = max(0.5 * (trace - disc), 0.0)
    if metric == "anisotropy":
        return math.log10(lambda_max + EPS2) - math.log10(lambda_min + EPS2)
    if metric == "area":
        product = lambda_max * lambda_min
        return 0.5 * math.log10(product if product > EPS2 else EPS2)
    return 0.0


def _metric_roots_for_source(source, roots, coeff_roots, param_values):
    if source == "pm":
        return _flat_param_pairs(param_values)
    if source == "cf":
        return _root_pairs(coeff_roots)
    return _root_pairs(roots)


def eval_metric_slot_normalized(metric_spec, roots, coeff_roots=None, param_values=None):
    source = str(metric_spec.get("source", "slv"))
    metric = str(metric_spec["metric"])
    if source == "pm":
        score = _param_metric_score(param_values, metric)
    else:
        score_roots = _metric_roots_for_source(source, roots, coeff_roots, param_values)
        score = compute_solve_metric_score(score_roots, metric) if score_roots else 0.0
    lo = float(metric_spec["clip_lo"])
    hi = float(metric_spec["clip_hi"])
    return clamp_unit((score - lo) / (hi - lo))


def eval_metric_slots(compiled, roots, coeff_roots=None, param_values=None):
    return [
        eval_metric_slot_normalized(metric, roots, coeff_roots=coeff_roots, param_values=param_values)
        for metric in compiled.get("metrics", [])
    ]


@dataclass(frozen=True)
class SolveScoreOutputs:
    metrics: list[float]
    outputs: list[float]


def eval_program_outputs_from_buffers(current_metrics, program_spec, recent_metrics=None):
    current_metrics = [float(v) for v in current_metrics]
    recent_metrics = None if recent_metrics is None else [float(v) for v in recent_metrics]
    stack = []
    outputs = []
    has_explicit_outputs = False
    _, program_body = strip_solve_score_version(program_spec)
    for raw in program_body.split(";"):
        token = raw.strip()
        if not token:
            continue
        if token.startswith("m"):
            slot_text, _, lag_text = token[1:].partition("-")
            slot = int(slot_text)
            lag = int(lag_text or "0")
            if lag == 0:
                stack.append(current_metrics[slot])
            elif lag == 1:
                if recent_metrics is None:
                    raise RuntimeError(f"program references {token} but recent metrics are unavailable")
                stack.append(recent_metrics[slot])
            else:
                raise RuntimeError(f"unsupported lag depth in {token}")
            continue
        if token.startswith("const:"):
            stack.append(float(token[6:]))
            continue
        if token == "dup":
            stack.append(stack[-1])
            continue
        if token == "flush":
            stack.clear()
            continue
        if token in {"emit", "emit_norm", "emit_none"}:
            value = stack.pop()
            has_explicit_outputs = True
            if token != "emit_none":
                outputs.append(value)
            continue
        if token.startswith("omega_cosine:"):
            parts = token.split(":")
            stack[-1] = apply_omega(stack[-1], float(parts[1]), float(parts[2]) if len(parts) > 2 else 0.0)
            continue
        if token.startswith("sawtooth:"):
            x = stack[-1] * float(token.split(":", 1)[1])
            stack[-1] = clamp_unit(x - math.floor(x))
            continue
        if token.startswith("weighted_sum:"):
            _name, a_text, b_text = token.split(":")
            b = stack.pop()
            a = stack[-1]
            stack[-1] = clamp_unit(float(a_text) * a + float(b_text) * b)
            continue
        if token.startswith("ema:"):
            alpha = float(token.split(":", 1)[1])
            b = stack.pop()
            a = stack[-1]
            stack[-1] = _finite_or_zero(alpha * a + (1.0 - alpha) * b)
            continue
        if token.startswith("pow:"):
            stack[-1] = _finite_or_zero(math.pow(stack[-1], float(token.split(":", 1)[1])))
            continue

        if token in {"avg", "min", "max", "mul", "add", "mult", "subtract", "ratio", "abs_diff", "geometric_mean"}:
            b = stack.pop()
            a = stack[-1]
            if token == "avg":
                stack[-1] = clamp_unit(0.5 * (a + b))
            elif token == "min":
                stack[-1] = min(a, b)
            elif token == "max":
                stack[-1] = max(a, b)
            elif token == "mul":
                stack[-1] = clamp_unit(a * b)
            elif token == "add":
                stack[-1] = _finite_or_zero(a + b)
            elif token == "mult":
                stack[-1] = _finite_or_zero(a * b)
            elif token == "subtract":
                stack[-1] = _finite_or_zero(a - b)
            elif token == "ratio":
                stack[-1] = 0.0 if (not math.isfinite(b) or b == 0.0) else _finite_or_zero(a / b)
            elif token == "abs_diff":
                stack[-1] = clamp_unit(abs(a - b))
            elif token == "geometric_mean":
                stack[-1] = clamp_unit(math.sqrt(clamp_unit(a) * clamp_unit(b)))
            continue

        if token == "flip":
            stack[-1] = clamp_unit(1.0 - stack[-1])
        elif token == "clamp":
            stack[-1] = clamp_unit(stack[-1])
        elif token == "sin":
            stack[-1] = _finite_or_zero(math.sin(stack[-1]))
        elif token == "cos":
            stack[-1] = _finite_or_zero(math.cos(stack[-1]))
        elif token == "log":
            stack[-1] = 0.0 if (not math.isfinite(stack[-1]) or stack[-1] <= 0.0) else _finite_or_zero(math.log(stack[-1]))
        elif token == "exp":
            stack[-1] = _finite_or_zero(math.exp(stack[-1]))
        else:
            raise RuntimeError(f"unsupported solve-score program token: {token!r}")

    if has_explicit_outputs:
        if stack:
            raise RuntimeError("explicit-output program ended with non-empty stack")
        return outputs
    if len(stack) != 1:
        raise RuntimeError(f"solve-score program ended with stack depth {len(stack)}")
    return [stack[0]]


def eval_solve_score(compiled, roots, coeff_roots=None, param_values=None, recent_metrics=None):
    metrics = eval_metric_slots(compiled, roots, coeff_roots=coeff_roots, param_values=param_values)
    outputs = eval_program_outputs_from_buffers(metrics, compiled["program_spec"], recent_metrics=recent_metrics)
    return SolveScoreOutputs(metrics=metrics, outputs=outputs)


def eval_solve_score_chain(
    chain,
    roots,
    *,
    metric=None,
    quantile=None,
    omega=None,
    omega_enabled=None,
    default_metric=None,
    clips=None,
    coeff_roots=None,
    param_values=None,
    recent_metrics=None,
):
    compiled = compile_solve_score_chain_or_legacy(
        chain,
        metric,
        quantile,
        omega,
        omega_enabled,
        default_metric=default_metric,
    )
    clips = clips or {}
    metrics = []
    for spec in compiled["metrics"]:
        key = spec["metric"]
        lo, hi = clips.get(key, (0.0, 1.0))
        metrics.append({**spec, "clip_lo": lo, "clip_hi": hi})
    compiled = {**compiled, "metrics": metrics}
    return eval_solve_score(
        compiled,
        roots,
        coeff_roots=coeff_roots,
        param_values=param_values,
        recent_metrics=recent_metrics,
    )
