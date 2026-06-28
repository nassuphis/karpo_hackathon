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
import pipeline_programs

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP_CLI = os.path.join(LAMBDA_DIR, "sweep_cli.c")
PROGRAM_PROFILES = os.path.join(LAMBDA_DIR, "program_profiles.json")
PARAM_LEGACY_REGISTRY = os.path.join(LAMBDA_DIR, "param_legacy_registry.json")
PARAM_VOCAB_JS = os.path.join(os.path.dirname(__file__), "..", "param_vocab_js.js")


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


def _dispatch_pt_block(source, name):
    marker = f'if (strcmp(e->name, "{name}") == 0) {{'
    start = source.find(marker)
    assert start >= 0, f"dispatchPt block missing for {name}"
    next_start = source.find('\n    if (strcmp(e->name, "', start + len(marker))
    fallback_start = source.find("\n    if (pt_is_targetable_independent", start + len(marker))
    candidates = [idx for idx in (next_start, fallback_start) if idx >= 0]
    end = min(candidates) if candidates else source.find("\n}", start + len(marker))
    assert end > start, f"could not bound dispatchPt block for {name}"
    return source[start:end]


def _c_dispatch_default_lanes(source, name):
    block = _dispatch_pt_block(source, name)
    defaults = {}
    for match in re.finditer(
        r"e->nArgs\s*>\s*(\d+)\s*\?\s*(?:\([^)]*\))?\s*e->args\[\1\]\s*:\s*([-+]?\d+(?:\.\d+)?)",
        block,
    ):
        defaults[int(match.group(1))] = float(match.group(2))
    return defaults


def _program_profiles():
    with open(PROGRAM_PROFILES, "r", encoding="utf-8") as fh:
        return json.load(fh)["profiles"]


def _param_legacy_registry_payload():
    with open(PARAM_LEGACY_REGISTRY, "r", encoding="utf-8") as fh:
        return json.load(fh)


def _param_vocab_js_payload():
    with open(PARAM_VOCAB_JS, "r", encoding="utf-8") as fh:
        text = fh.read()
    return json.loads(text[text.index("{"): text.rindex("}") + 1])


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


def test_param_profile_source_grammar_matches_python():
    profile = _program_profiles()["param"]
    source = profile["source"]
    assert source["stack_op_aliases"] == {
        "dup": "duplicate",
        "duplicate": "duplicate",
        "swap": "swap",
        "pop": "pop",
        "flush": "flush",
    }
    assert set(source["binary_ops"]) == set(chain._BINARY_OPS)
    assert set(source["unary_ops"]) == set(chain._UNARY_OPS)
    assert set(source["targetable_unary"]) == set(chain._TARGETABLE_UNARY_SOURCE)
    assert source["push_sources"] == ["t1", "t2"]
    assert source["emit_aliases"] == {"emit_p1": "p1", "emit_p2": "p2"}
    assert {item["form"]: item["code"] for item in source["rejected_forms"]} == {
        "push(both)": "bad_selector",
        "emit(p1)": "noncanonical_emit",
    }


def test_generated_param_vocab_exposes_full_registry():
    vocab = _param_vocab_js_payload()
    registry_payload = _param_legacy_registry_payload()
    compat = registry_payload["compat"]
    ui = registry_payload["ui"]
    registry_functions = registry_payload["functions"]
    registry_names = sorted(chain.legacy_registry()["by_name"])
    assert sorted(vocab["names"]) == registry_names
    assert len(vocab["names"]) == 70
    for missing_from_old_js in ["add", "cadd", "scale", "zzold", "scdth"]:
        assert missing_from_old_js in vocab["names"]
    assert vocab["categoryMeta"] == ui["categories"]
    assert vocab["uiFunctions"] == {
        fn["name"]: {key: value for key, value in (fn.get("ui") or {}).items() if key != "params"}
        for fn in registry_functions
    }
    assert vocab["targetArgIndexes"] == {name: idx for name, idx in sorted(compat["target_arg_indexes"].items())}
    assert vocab["targetArgIndexes"] == {name: idx for name, idx in sorted(chain._LEGACY_TARGET_ARG_INDEXES.items())}
    assert sorted(compat["target_first"]) == sorted(chain._LEGACY_TARGET_FIRST_CHIPS)
    assert sorted(compat["target_last"]) == sorted(chain._LEGACY_TARGET_LAST_CHIPS)
    assert sorted(compat["dither_target_first"]) == sorted(chain._LEGACY_DITHER_TARGET_FIRST_CHIPS)
    assert vocab["independentTargets"] == sorted(compat["independent_targets"])
    assert vocab["independentTargets"] == sorted(chain._REDUNDANT_LEGACY_TARGET_ARG_NAMES)
    assert vocab["variableArgCounts"]["moebius"] == [0, 4, 8]
    assert vocab["variableArgCounts"]["inv_t_plus_2"] == [0, 1, 2, 3, 4]
    assert vocab["variableArgCounts"]["add"] == [0, 1, 2]
    assert vocab["variableArgCounts"] == {name: sorted(counts) for name, counts in compat["variable_arg_counts"].items()}
    assert len(vocab["argSpecs"]["moebius"]) == 4
    assert len(vocab["argSpecs"]["inv_t_plus_2"]) == 2
    for fn in registry_functions:
        local_ui = fn.get("ui") or {}
        assert local_ui.get("desc"), f"{fn['name']} missing function-local ui.desc"
        runtime_args = fn.get("args") or []
        if not runtime_args:
            continue
        ui_params = local_ui.get("params") or []
        assert len(ui_params) == len(runtime_args), f"{fn['name']} ui.params must match runtime args"
        generated = vocab["argSpecs"].get(fn["name"]) or []
        assert len(generated) == len(runtime_args), f"{fn['name']} generated argSpecs missing args"
        for runtime_arg, ui_arg, generated_arg in zip(runtime_args, ui_params, generated):
            name = runtime_arg["name"]
            assert (ui_arg.get("ph") or ui_arg.get("name")) == name
            assert str(ui_arg.get("title") or "").strip(), f"{fn['name']}.{name} missing help text"
            assert generated_arg["ph"] == name
            assert str(generated_arg.get("title") or "").strip(), f"{fn['name']}.{name} missing generated help text"
    for dead_payload in [
        "fnIndexByName",
        "targetFirst",
        "targetLast",
        "ditherTargetFirst",
        "specs",
        "programProfiles",
    ]:
        assert dead_payload not in vocab


def test_param_registry_runtime_defaults_match_c_dispatch_fallbacks():
    source = _c_source()
    payload = _param_legacy_registry_payload()
    compat = payload["compat"]
    target_indexes = {name: int(idx) for name, idx in compat["target_arg_indexes"].items()}
    variable_arg_names = set(compat["variable_arg_counts"])

    for fn in payload["functions"]:
        name = fn["name"]
        args = list(fn.get("args") or [])
        if not args or name in variable_arg_names:
            continue
        c_defaults = _c_dispatch_default_lanes(source, name)
        for arg_index, arg in enumerate(args):
            lane_index = arg_index
            if name in target_indexes and arg_index >= target_indexes[name]:
                lane_index += 1
            assert lane_index in c_defaults, f"{name}.{arg['name']} C fallback missing lane {lane_index}"
            assert c_defaults[lane_index] == float(arg["default"]), (
                f"{name}.{arg['name']} registry default {arg['default']} "
                f"does not match C fallback lane {lane_index}={c_defaults[lane_index]}"
            )


def test_param_independent_target_runtime_set_matches_registry():
    source = _c_source()
    match = re.search(
        r"static int pt_is_targetable_independent\(const char \*name\) \{(?P<body>.*?)\n\}",
        source,
        re.S,
    )
    assert match, "pt_is_targetable_independent missing from sweep_cli.c"
    c_names = sorted(re.findall(r'strcmp\(name,\s*"([^"]+)"\)\s*==\s*0', match.group("body")))
    compat = _param_legacy_registry_payload()["compat"]
    assert c_names == sorted(compat["independent_targets"])


def test_param_boundary_translator_target_compat_matches_registry():
    compat = _param_legacy_registry_payload()["compat"]

    for name in compat["target_first"]:
        translated = pipeline_programs.param_transforms_to_program_chain([[name, "p1", "9", "10"]])
        assert translated == [["legacy", name, "p1", "p1", "9", "10"]], name

    for name in compat["target_last"]:
        translated = pipeline_programs.param_transforms_to_program_chain([[name, "9", "p2"]])
        assert translated == [["legacy", name, "p2", "p2", "9"]], name

    for name in compat["dither_target_first"]:
        translated = pipeline_programs.param_transforms_to_program_chain([[name, "p1", "0.5"]])
        assert translated == [["legacy", name, "both", "both", "0", "0.5"]], name


def test_param_compiler_reads_runtime_compat_from_registry():
    registry = chain.legacy_registry()
    compat = registry["compat"]
    assert compat["target_arg_indexes"] == {
        name: idx for name, idx in sorted(chain._LEGACY_TARGET_ARG_INDEXES.items())
    }
    assert compat["target_first"] == frozenset(chain._LEGACY_TARGET_FIRST_CHIPS)
    assert compat["target_last"] == frozenset(chain._LEGACY_TARGET_LAST_CHIPS)
    assert compat["dither_target_first"] == frozenset(chain._LEGACY_DITHER_TARGET_FIRST_CHIPS)
    assert compat["independent_targets"] == frozenset(chain._REDUNDANT_LEGACY_TARGET_ARG_NAMES)
    assert compat["variable_arg_counts"] == {
        name: frozenset(counts) for name, counts in chain._VARIABLE_LEGACY_ARG_COUNTS.items()
    }
