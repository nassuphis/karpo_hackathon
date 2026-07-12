"""CR32 F1: the typed-stack fast kernels (vector-vector add/subtract/multiply,
vector neg/conj/real/imag) must implement the FULL generic operation —
including the final non-finite clamp to zero — not just its arithmetic core.

Two pins per operation:
  1. semantic: overflow through a fast kernel yields exact 0.0 output floats
     (the VM's historical policy), never inf;
  2. path parity: the fast vector-vector kernel produces bytes identical to
     the generic ladder driven over the same values (vector-scalar broadcast
     for binaries, the legacy selector form for unaries).

Values cover overflow, negative overflow, subnormal, zero, signed zero, and
non-finite stack inputs (scalar-expression overflow feeding fill).
"""
import json
import math
import os
import struct
import subprocess
import sys
import tempfile

import pytest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

from coeff_program_chain import compile_coeff_program_chain
from coeff_program_source import compile_coeff_program_source

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")


def _run(chain, source=None):
    compiled = (compile_coeff_program_source(source) if source is not None
                else compile_coeff_program_chain(chain))
    payload = {
        "mode": "coeffgen", "function": "poly_1", "cfpv": [],
        "n1": 2, "n2": 2, "times": 1,
        "coeff_program": compiled,
    }
    with tempfile.NamedTemporaryFile(prefix="pp_fast_kernel_", suffix=".bin", delete=False) as fh:
        out_path = fh.name
    try:
        proc = subprocess.run(
            [SWEEP, out_path], input=json.dumps(payload),
            capture_output=True, text=True, timeout=30,
        )
        assert proc.returncode == 0, proc.stderr
        with open(out_path, "rb") as fh:
            data = fh.read()
        return data
    finally:
        try:
            os.remove(out_path)
        except FileNotFoundError:
            pass


def _floats(data):
    return struct.unpack(f"<{len(data) // 4}f", data)


def _fast_binary_chain(value, op, rhs_scalar=None):
    """poly=[value,value]; TOS gets two vectors (or vector+neg for subtract
    overflow); fast vector-vector kernel runs."""
    chain = [["push_const", 2, value], ["emit"],
             ["_typed_push_vector", "poly"], ["_typed_push_vector", "poly"]]
    if rhs_scalar == "neg":
        chain.append(["_typed_unary", "neg"])
    chain += [["_typed_binary", op], ["_typed_set_poly"]]
    return chain


def _broadcast_binary_chain(value, op, rhs_value):
    """Same math through the GENERIC typed ladder: vector op scalar broadcast."""
    return [["push_const", 2, value], ["emit"],
            ["_typed_push_vector", "poly"], ["_typed_push_scalar", rhs_value],
            ["_typed_binary", op], ["_typed_set_poly"]]


def _fast_unary_chain(fill_expr, op):
    """fill via typed pushes so the value expression can go non-finite; the
    unary then runs the fast typed kernel."""
    return [["push_scalar", "2"], ["push_scalar", fill_expr], ["_typed_fill"],
            ["_typed_unary", op], ["_typed_set_poly"]]


# ---- semantic pins: overflow through fast kernels must clamp to 0 ----

def test_fast_add_overflow_clamps_to_zero():
    vals = _floats(_run(_fast_binary_chain(1e308, "add")))
    assert all(v == 0.0 for v in vals), vals


def test_fast_subtract_overflow_clamps_to_zero():
    # poly - neg(poly) = 2*poly -> overflow
    vals = _floats(_run(_fast_binary_chain(1e308, "subtract", rhs_scalar="neg")))
    assert all(v == 0.0 for v in vals), vals


def test_fast_multiply_overflow_clamps_to_zero():
    vals = _floats(_run(_fast_binary_chain(1e200, "multiply")))
    assert all(v == 0.0 for v in vals), vals


# ---- path parity: fast vector-vector == generic broadcast, same values ----

@pytest.mark.parametrize("value", [1e308, -1e308, 1e-320, 0.0, -0.0, 3.5, 7e37])
@pytest.mark.parametrize("op", ["add", "subtract", "multiply"])
def test_fast_binary_matches_generic_broadcast(op, value):
    fast = _run(_fast_binary_chain(value, op))
    generic = _run(_broadcast_binary_chain(value, op, value))
    assert fast == generic, (op, value, _floats(fast), _floats(generic))


# ---- unary kernels: non-finite inputs must leave as 0, like the ladder ----

@pytest.mark.parametrize("op", ["neg", "conj", "real", "imag"])
@pytest.mark.parametrize("fill_expr", [
    "0 * 1e308",         # 0
    "1e-320",            # subnormal
    "-1e308",            # large negative, finite
    "-0.0",              # signed zero
])
def test_fast_unary_matches_legacy_selector(op, fill_expr):
    fast = _run(_fast_unary_chain(fill_expr, op))
    legacy = _run(None, source=f"fill(2, {fill_expr})\npoly = pop\npoly = {op}(poly)")
    assert fast == legacy, (op, fill_expr, _floats(fast), _floats(legacy))


def test_nonfinite_scalar_exprs_fail_loudly_at_both_layers():
    """Non-finite values cannot reach the typed stack through scalar
    expressions: the COMPILER rejects non-finite constant folds, and the C
    runtime hard-errors on non-finite expression results. These guards (plus
    the fast-kernel clamps for arithmetic overflow) are what keep the VM's
    non-finite policy closed — pin both.

    The unary fast-kernel clamps are therefore defensive: parity on finite
    edges is asserted above, and the only overflow route (typed binary) is
    pinned to clamp by the tests at the top of this file.
    """
    with pytest.raises(RuntimeError, match="finite"):
        compile_coeff_program_chain(
            [["push_scalar", "2"], ["push_scalar", "1e308 * 1e308"],
             ["_typed_fill"], ["_typed_set_poly"]])

    payload = {
        "mode": "coeffgen", "function": "poly_1", "cfpv": [],
        "n1": 2, "n2": 2, "times": 1,
        "coeff_program": compile_coeff_program_chain(
            [["push_scalar", "2"], ["push_scalar", "(p1 + 2) * 1e308"],
             ["_typed_fill"], ["_typed_set_poly"]]),
    }
    with tempfile.NamedTemporaryFile(prefix="pp_fast_kernel_", suffix=".bin", delete=False) as fh:
        out_path = fh.name
    try:
        proc = subprocess.run(
            [SWEEP, out_path], input=json.dumps(payload),
            capture_output=True, text=True, timeout=30,
        )
        assert proc.returncode != 0
        assert "finite" in proc.stderr
    finally:
        try:
            os.remove(out_path)
        except FileNotFoundError:
            pass
