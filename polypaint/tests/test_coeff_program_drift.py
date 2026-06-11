"""Cross-layer drift guards for the Coeff Program system.

The opcode/expression/selector enums are hand-duplicated between
lambda/coeff_program_chain.py and lambda/sweep_cli.c, and the legacy
registry fn_index values are special-cased in three places. Nothing else
in the suite pins them, so drift in any untested opcode would pass every
test. This file parses the C enums and compares them constant-by-constant
to the Python side, pins the registry ids, and checks limit parity.
"""
import os
import re
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

import coeff_program_chain as chain
from coeff_program_chain import legacy_registry

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP_CLI = os.path.join(LAMBDA_DIR, "sweep_cli.c")
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")


def _c_source():
    with open(SWEEP_CLI, "r", encoding="utf-8") as fh:
        return fh.read()


def _c_enum_values(source, prefix):
    return {
        name: int(value)
        for name, value in re.findall(rf"\b{prefix}(\w+)\s*=\s*(\d+)", source)
    }


def _c_defines(source):
    return {
        name: int(value)
        for name, value in re.findall(r"#define\s+(COEFF_PROGRAM_\w+)\s+(\d+)", source)
    }


def test_coeff_op_enum_matches_python():
    c_ops = _c_enum_values(_c_source(), "COEFF_OP_")
    py_ops = {
        name[len("COEFF_OP_"):]: value
        for name, value in vars(chain).items()
        if name.startswith("COEFF_OP_")
    }
    assert py_ops, "no COEFF_OP_ constants found in coeff_program_chain"
    assert c_ops == py_ops


def test_coeff_expr_enum_matches_python():
    c_exprs = _c_enum_values(_c_source(), "COEFF_EXPR_")
    py_exprs = {
        name[len("EXPR_"):]: value
        for name, value in vars(chain).items()
        if name.startswith("EXPR_")
    }
    assert py_exprs, "no EXPR_ constants found in coeff_program_chain"
    assert c_exprs == py_exprs


def test_selector_and_scalar_source_enums_match_python():
    source = _c_source()
    c_sel = _c_enum_values(source, "COEFF_SEL_")
    py_sel = {
        name[len("COEFF_SEL_"):]: value
        for name, value in vars(chain).items()
        if name.startswith("COEFF_SEL_")
    }
    assert c_sel == py_sel
    c_scalar = _c_enum_values(source, "COEFF_SCALAR_SRC_")
    py_scalar = {
        name[len("COEFF_SCALAR_SRC_"):]: value
        for name, value in vars(chain).items()
        if name.startswith("COEFF_SCALAR_SRC_")
    }
    assert c_scalar == py_scalar


def test_vector_op_tables_match_python():
    source = _c_source()
    c_vec = _c_enum_values(source, "COEFF_VEC_")
    for name, fn_index in chain.VECTOR_BINARY_OPS.items():
        assert c_vec[name.upper()] == fn_index, f"vector binary {name}"
    for name, fn_index in chain.VECTOR_UNARY_OPS.items():
        assert c_vec[name.upper()] == fn_index, f"vector unary {name}"
    assert c_vec["ROLL_LEFT"] == chain.VECTOR_ROLL_OPS["roll"]
    assert c_vec["ROLL_RIGHT"] == chain.VECTOR_ROLL_OPS["rolr"]


def test_limits_match_c_defines():
    defines = _c_defines(_c_source())
    assert defines["COEFF_PROGRAM_MAX_TOKENS"] == chain.MAX_PROGRAM_TOKENS
    assert defines["COEFF_PROGRAM_MAX_VECTOR_STACK"] == chain.MAX_VECTOR_STACK
    assert defines["COEFF_PROGRAM_MAX_VECTOR_LEN"] == chain.MAX_VECTOR_LEN
    assert defines["COEFF_PROGRAM_MAX_ARGS"] == chain.MAX_ARGS
    assert defines["COEFF_PROGRAM_MAX_SCALAR_EXPRS"] == chain.MAX_SCALAR_EXPRS
    assert defines["COEFF_PROGRAM_MAX_EXPR_NUMS"] == (
        chain.MAX_SCALAR_EXPR_TOKENS * defines["COEFF_PROGRAM_EXPR_STRIDE"]
    )


def test_legacy_int_arg_clamp_matches_c():
    source = _c_source()
    match = re.search(r"#define\s+COEFF_LEGACY_MAX_INT_ARG\s+(\d+)", source)
    assert match, "COEFF_LEGACY_MAX_INT_ARG missing from sweep_cli.c"
    assert int(match.group(1)) == chain.MAX_LEGACY_INT_ARG


def test_registry_fn_indices_are_pinned():
    # These ids are wire format: they appear in compiled tokens, are
    # special-cased in coeff_program_source / coeff_program_chain /
    # sweep_cli.c, and persist in saved programs. Renumbering breaks all
    # three in lockstep-invisible ways, so pin every one.
    expected = {
        "rev": 1, "conj": 2, "normalize": 3, "deriv": 4, "safe": 5,
        "negate_odd": 6, "max2one": 7, "sort_mod_keep_angle": 8,
        "sort_angle_keep_mod": 9, "sort_abs": 10, "cumsum": 11,
        "cummax": 12, "sort_cumsum": 13, "linear": 14, "swirler": 15,
        "exp": 16, "cos": 17, "sin": 18, "tan": 19, "cosh": 20,
        "sinh": 21, "tanh": 22, "round": 23, "pow": 24, "power": 25,
        "invpower": 26, "roots_cm": 27, "roots": 28,
    }
    registry = legacy_registry()["by_name"]
    actual = {name: spec["fn_index"] for name, spec in registry.items()}
    assert actual == expected


def test_sweep_test_binary_is_not_older_than_source():
    # Native parity tests validate whatever binary is on disk; a C edit
    # without a rebuild silently turns them into stale-code checks.
    assert os.path.exists(SWEEP_TEST), (
        "lambda/sweep_test missing; build with: "
        "cc -O2 -pthread -o lambda/sweep_test lambda/sweep_cli.c -lm"
    )
    src_mtime = os.path.getmtime(SWEEP_CLI)
    bin_mtime = os.path.getmtime(SWEEP_TEST)
    assert bin_mtime >= src_mtime - 1.0, (
        "lambda/sweep_test is older than sweep_cli.c; rebuild with: "
        "cc -O2 -pthread -o lambda/sweep_test lambda/sweep_cli.c -lm"
    )


def test_source_transform_aliases_are_mirrored_in_chain():
    # pow_affine/power_series/exp_affine must be accepted by BOTH frontends;
    # a source-only alias produced chips the chain compiler rejected (CR2-1).
    from coeff_program_source import _NATIVE_TRANSFORM_ALIASES

    for alias, target in _NATIVE_TRANSFORM_ALIASES.items():
        assert chain.LEGACY_NAME_ALIASES.get(alias) == target, alias


def test_native_transform_packing_parity_between_source_and_chain():
    # The fn 14/16/23/24 andy-split/packing rules live in coeff_program_source,
    # coeff_program_chain, and sweep_cli.c; pin the source-vs-chain halves so
    # a drift (like the old round(a, b, andy) break) fails loudly.
    from coeff_program_source import parse_coeff_program_source

    cases = [
        # (source text, chain row, fn_index)
        ("poly = linear(poly, 2, 3, 0.5)", ["legacy", "linear", "poly", "poly", "2", "3", "0.5"], 14),
        ("poly = exp_affine(poly, 2, 3, 0.5)", ["legacy", "exp", "poly", "poly", "2", "3", "0.5"], 16),
        ("poly = round(poly, 2, 0.5)", ["legacy", "round", "poly", "poly", "2", "0.5"], 23),
        ("poly = round(poly, 1, 2, 0.5)", ["legacy", "round", "poly", "poly", "1", "2", "0.5"], 23),
        ("poly = pow_affine(poly, 2, 3, 0.5)", ["legacy", "pow", "poly", "poly", "2", "3", "0.5"], 24),
    ]
    for source_text, legacy_row, fn_index in cases:
        parsed = parse_coeff_program_source(source_text, strict=False)
        assert parsed["diagnostics"] == [], (source_text, parsed["diagnostics"])
        source_compiled = chain.compile_coeff_program_chain(parsed["chain"])
        source_tok = next(
            t for t in source_compiled["tokens"]
            if t["op"] in (chain.COEFF_OP_NATIVE_TRANSFORM, chain.COEFF_OP_LEGACY)
        )
        chain_tok = chain.compile_coeff_program_chain([legacy_row])["tokens"][0]
        assert source_tok["fn_index"] == fn_index == chain_tok["fn_index"], source_text
        assert source_tok.get("andy") == chain_tok.get("andy") == 0.5, source_text
        # Stack-arg forms carry args on the stack; full-args forms pack them.
        if source_tok.get("stack_arg_count"):
            assert source_tok["stack_arg_count"] <= chain_tok.get("n_args", 8), source_text
        else:
            assert source_tok.get("args") == chain_tok.get("args"), source_text
