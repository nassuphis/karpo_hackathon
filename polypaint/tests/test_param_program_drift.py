"""Cross-layer drift guards for the Param Program VM metadata.

Coeff Program already had C/Python drift checks. Stage 1 adds the same
coverage for Param Program so the shared-profile work starts from pinned
opcode, expression, selector, and cap values instead of reviewer memory.
"""
import json
import os
import re
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

import param_program_chain as chain

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP_CLI = os.path.join(LAMBDA_DIR, "sweep_cli.c")
PROGRAM_PROFILES = os.path.join(LAMBDA_DIR, "program_profiles.json")


def _c_source():
    with open(SWEEP_CLI, "r", encoding="utf-8") as fh:
        return fh.read()


def _c_enum_values(source, prefix):
    return {
        name: int(value)
        for name, value in re.findall(rf"\b{prefix}(\w+)\s*=\s*(\d+)", source)
    }


def _c_defines(source, prefix="PARAM_PROGRAM_"):
    return {
        name: int(value)
        for name, value in re.findall(rf"#define\s+({prefix}\w+)\s+(\d+)", source)
    }


def _program_profiles():
    with open(PROGRAM_PROFILES, "r", encoding="utf-8") as fh:
        return json.load(fh)["profiles"]


def _param_expr_token_cap(source):
    match = re.search(
        r"#define\s+PARAM_PROGRAM_MAX_EXPR_NUMS\s+\((\d+)\s*\*\s*PARAM_PROGRAM_EXPR_STRIDE\)",
        source,
    )
    assert match, "PARAM_PROGRAM_MAX_EXPR_NUMS expression missing"
    return int(match.group(1))


def test_param_op_enum_matches_python():
    c_ops = _c_enum_values(_c_source(), "PARAM_OP_")
    py_ops = {
        name[len("PARAM_OP_"):]: value
        for name, value in vars(chain).items()
        if name.startswith("PARAM_OP_")
    }
    assert py_ops, "no PARAM_OP_ constants found in param_program_chain"
    assert c_ops == py_ops


def test_param_expr_enum_matches_python():
    c_exprs = _c_enum_values(_c_source(), "PARAM_EXPR_")
    py_exprs = {
        name[len("EXPR_"):]: value
        for name, value in vars(chain).items()
        if name.startswith("EXPR_")
    }
    assert py_exprs, "no EXPR_ constants found in param_program_chain"
    assert c_exprs == py_exprs


def test_param_selector_enum_matches_python():
    c_sel = _c_enum_values(_c_source(), "PARAM_SEL_")
    py_sel = {
        name[len("PARAM_SEL_"):]: value
        for name, value in vars(chain).items()
        if name.startswith("PARAM_SEL_")
    }
    assert c_sel == py_sel


def test_param_profile_caps_match_python_and_c():
    profile = _program_profiles()["param"]
    caps = profile["value_caps"]
    source = _c_source()
    defines = _c_defines(source)
    assert caps["program_tokens"] == chain.MAX_PROGRAM_TOKENS == defines["PARAM_PROGRAM_MAX_TOKENS"]
    assert caps["total_stack"] == chain.MAX_STACK == defines["PARAM_PROGRAM_MAX_STACK"]
    assert caps["vector_len"] == 0
    assert caps["vector_capable_stack"] == 0
    assert caps["max_args"] == chain.MAX_ARGS == defines["PARAM_PROGRAM_MAX_ARGS"]
    assert caps["scalar_exprs"] == defines["PARAM_PROGRAM_MAX_SCALAR_EXPRS"]
    assert caps["expr_tokens"] == chain.MAX_SCALAR_EXPR_TOKENS == _param_expr_token_cap(source)
    assert caps["lowered_expr_tokens"] == chain.MAX_LOWERED_PARAM_TOKENS_PER_EXPR


def test_param_profile_selectors_match_python():
    profile = _program_profiles()["param"]
    selectors = profile["selectors"]
    aliases = profile["selector_aliases"]
    assert selectors["legacy_src"] == list(chain._SOURCE_SELECTORS)
    assert selectors["legacy_tgt"] == list(chain._TARGET_SELECTORS)
    assert selectors["push"] == ["t1", "t2"]
    assert selectors["emit"] == ["p1", "p2"]
    assert aliases["push"] == {"0": "t1", "1": "t2"}
    assert aliases["emit"] == {"0": "p1", "1": "p2"}

    for name in selectors["push"]:
        assert name in chain._PUSH_TARGETS
    for alias, target in aliases["push"].items():
        assert chain._PUSH_TARGETS[alias] == chain._PUSH_TARGETS[target]
    for name in selectors["emit"]:
        assert name in chain._EMIT_TARGETS
    for alias, target in aliases["emit"].items():
        assert chain._EMIT_TARGETS[alias] == chain._EMIT_TARGETS[target]
