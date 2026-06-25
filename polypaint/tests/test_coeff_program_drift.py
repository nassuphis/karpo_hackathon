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
import json

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

import coeff_program_chain as chain
import coeff_program_source as source
from coeff_program_chain import legacy_registry

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP_CLI = os.path.join(LAMBDA_DIR, "sweep_cli.c")
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")
STRUCTURAL_CHIPS = os.path.join(LAMBDA_DIR, "structural_chips.json")
PROGRAM_PROFILES = os.path.join(LAMBDA_DIR, "program_profiles.json")
COEFF_VOCAB_JS = os.path.join(os.path.dirname(__file__), "..", "coeff_vocab_js.js")


def _c_source():
    with open(SWEEP_CLI, "r", encoding="utf-8") as fh:
        return fh.read()


def _c_enum_values(source, prefix):
    return {
        name: int(value)
        for name, value in re.findall(rf"\b{prefix}(\w+)\s*=\s*(\d+)", source)
    }


def _c_defines(source, prefix="COEFF_PROGRAM_"):
    return {
        name: int(value)
        for name, value in re.findall(rf"#define\s+({prefix}\w+)\s+(\d+)", source)
    }


def _json_payload(path):
    with open(path, "r", encoding="utf-8") as fh:
        return json.load(fh)


def _structural_payload():
    return _json_payload(STRUCTURAL_CHIPS)


def _program_profiles():
    return _json_payload(PROGRAM_PROFILES)["profiles"]


def _coeff_vocab_js_payload():
    with open(COEFF_VOCAB_JS, "r", encoding="utf-8") as fh:
        text = fh.read()
    return json.loads(text[text.index("{"): text.rindex("}") + 1])


def _structural_chips_by_name():
    return {chip["name"]: chip for chip in _structural_payload()["chips"]}


def _structural_chain_names():
    names = set()
    for chip in _structural_payload()["chips"]:
        names.add(chip["name"])
        names.update(chip.get("aliases") or [])
        for sub in chip.get("sub_ops") or []:
            names.add(sub["name"])
    return names


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


def test_structural_chip_opcodes_match_coeff_python_and_c():
    source = _c_source()
    c_ops = _c_enum_values(source, "COEFF_OP_")
    seen_ops = set()
    for chip in _structural_payload()["chips"]:
        symbol = chip.get("op_symbol")
        if not symbol:
            assert chip.get("compile_time_only"), chip
            continue
        short = symbol[len("COEFF_OP_"):]
        assert chip["op"] == getattr(chain, symbol), chip["name"]
        assert chip["op"] == c_ops[short], chip["name"]
        seen_ops.add(chip["op"])
    py_ops = {
        value for name, value in vars(chain).items()
        if name.startswith("COEFF_OP_")
    }
    assert py_ops <= seen_ops


def test_structural_chip_inventory_covers_chain_compiler_names():
    names = _structural_chain_names()
    compiler_names = (
        set(chain._ZERO_ARG_CHIP_OPS)
        | set(chain._CHIP_COMPILERS)
        | set(chain.VECTOR_BINARY_OPS)
        | set(chain.VECTOR_UNARY_OPS)
        | set(chain.VECTOR_ROLL_OPS)
    )
    assert compiler_names - names == set()


def test_structural_subop_tables_match_python_and_c():
    chips = _structural_chips_by_name()
    c_vec = _c_enum_values(_c_source(), "COEFF_VEC_")
    families = {
        "vector_binary": chain.VECTOR_BINARY_OPS,
        "vector_unary": chain.VECTOR_UNARY_OPS,
        "vector_roll": chain.VECTOR_ROLL_OPS,
    }
    for family_name, py_table in families.items():
        sub_ops = {entry["name"]: entry for entry in chips[family_name]["sub_ops"]}
        assert {name: entry["value"] for name, entry in sub_ops.items()} == py_table
        for name, entry in sub_ops.items():
            assert c_vec[entry["symbol"][len("COEFF_VEC_"):]] == entry["value"], name


def test_coeff_source_parser_vector_vocab_comes_from_structural_metadata():
    chips = _structural_chips_by_name()
    binary_aliases = {}
    for entry in chips["vector_binary"]["sub_ops"]:
        binary_aliases[entry["name"]] = entry["name"]
        for alias in entry.get("source_aliases") or []:
            binary_aliases[alias] = entry["name"]
    assert {k: v for k, v in source._VECTOR_BINARY_ALIASES.items() if k in binary_aliases} == binary_aliases
    assert set(source._VECTOR_UNARY_NAMES) == {entry["name"] for entry in chips["vector_unary"]["sub_ops"]}


def test_structural_selector_slots_resolve_through_coeff_profile():
    payload = _structural_payload()
    slot_defs = payload["selector_slots"]
    coeff_selectors = _program_profiles()["coeff"]["selectors"]
    referenced = set()
    for chip in payload["chips"]:
        referenced.update((chip.get("selector_slots") or {}).values())
        for arg in chip.get("args") or []:
            if arg.get("slot"):
                referenced.add(arg["slot"])
    for slot in referenced:
        assert slot in slot_defs, slot
        profile_name, selector_name = slot_defs[slot].split(".", 1)
        assert profile_name == "coeff", slot
        assert selector_name in coeff_selectors, slot


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


def test_coeff_profile_caps_and_selectors_match_python_and_c():
    profile = _program_profiles()["coeff"]
    caps = profile["value_caps"]
    defines = _c_defines(_c_source())
    assert caps["program_tokens"] == chain.MAX_PROGRAM_TOKENS == defines["COEFF_PROGRAM_MAX_TOKENS"]
    assert caps["total_stack"] == chain.MAX_VECTOR_STACK == defines["COEFF_PROGRAM_MAX_VECTOR_STACK"]
    assert caps["vector_capable_stack"] == chain.MAX_VECTOR_STACK
    assert caps["vector_len"] == chain.MAX_VECTOR_LEN == defines["COEFF_PROGRAM_MAX_VECTOR_LEN"]
    assert caps["max_args"] == chain.MAX_ARGS == defines["COEFF_PROGRAM_MAX_ARGS"]
    assert caps["scalar_exprs"] == chain.MAX_SCALAR_EXPRS == defines["COEFF_PROGRAM_MAX_SCALAR_EXPRS"]
    assert caps["expr_tokens"] == chain.MAX_SCALAR_EXPR_TOKENS
    assert caps["legacy_int_arg"] == chain.MAX_LEGACY_INT_ARG

    selectors = profile["selectors"]
    assert selectors["src"] == list(chain._SOURCE_SELECTORS)
    assert selectors["tgt"] == list(chain._TARGET_SELECTORS)
    assert selectors["push_src"] == ["cf", "poly"]
    assert selectors["set_tgt"] == ["poly"]
    assert selectors["vector_src"] == list(chain._VECTOR_SOURCE_SELECTORS)
    assert set(selectors["typed_vector_src"]) == set(chain._SOURCE_SELECTORS) | {"tos"}


def test_generated_coeff_vocab_carries_effective_args_and_andy():
    vocab = _coeff_vocab_js_payload()
    for name, spec in legacy_registry()["by_name"].items():
        generated = vocab["effectiveArgs"][name]
        assert generated == list(spec["effective_args"])
        if spec["supports_andy"]:
            assert generated[-1]["name"] == "andy"
            assert generated[-1]["optional"] is True
            assert generated[-1]["role"] == "andy"
            ui_params = vocab["ctCatalog"][name].get("params") or []
            assert any(param.get("kind") == "andy" for param in ui_params), name


def test_generated_coeff_vocab_exposes_all_vector_unary_subops():
    vocab = _coeff_vocab_js_payload()
    chips = {chip["name"]: chip for chip in vocab["structuralChips"]["chips"]}
    unary_names = {entry["name"] for entry in chips["vector_unary"]["sub_ops"]}
    assert set(chain.VECTOR_UNARY_OPS) == unary_names
    assert {"real", "imag"} <= unary_names


def test_coeff_source_parser_uses_profile_selectors_and_symbols():
    import coeff_program_source as source

    profile = _program_profiles()["coeff"]
    selectors = profile["selectors"]
    index_bases = {
        name
        for name, spec in profile["symbols"].items()
        if "expr_index_base" in (spec.get("contexts") or [])
    }
    writable_lhs = {
        name
        for name, spec in profile["symbols"].items()
        if spec.get("access") == "read_write" and "lhs" in (spec.get("contexts") or [])
    }
    assert source._SOURCE_NAMES == set(selectors["src"])
    assert source._VECTOR_SOURCE_NAMES == set(selectors["vector_src"])
    assert source._TYPED_VECTOR_SOURCE_NAMES == set(selectors["typed_vector_src"])
    assert source._TARGET_NAMES == set(selectors["tgt"])
    assert source._PUSH_SOURCE_NAMES == set(selectors["push_src"])
    assert source._INDEX_BASE_NAMES == index_bases
    assert source._WRITABLE_LHS_NAMES == writable_lhs


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


# Aliases are wire format: saved chip rows carry them, so the set is pinned
# literally — entries may be added but never removed or remapped. The chain
# and source tables and the generated JS vocab all derive from
# coeff_legacy_registry.json; these tests fail if any consumer drifts from
# the registry or the registry drifts from this pin.
EXPECTED_ALIASES = {
    "exp_affine": "exp",
    "pow_affine": "pow",
    "power_series": "power",
    "scale100": "linear",
}
EXPECTED_TEXT_ALIASES = {
    "exp_affine": "exp",
    "pow_affine": "pow",
    "power_series": "power",
}
EXPECTED_CHIP_NAMES = {"exp": "exp_affine", "power": "power_series"}


def test_registry_aliases_are_pinned_wire_format():
    assert chain.LEGACY_NAME_ALIASES == EXPECTED_ALIASES
    assert chain.TEXT_NAME_ALIASES == EXPECTED_TEXT_ALIASES
    registry = legacy_registry()
    assert registry["alias_to_canonical"] == EXPECTED_ALIASES
    assert registry["text_alias_to_canonical"] == EXPECTED_TEXT_ALIASES
    chip_names = {
        spec["name"]: spec["chip_name"]
        for spec in registry["by_name"].values()
        if spec["chip_name"] != spec["name"]
    }
    assert chip_names == EXPECTED_CHIP_NAMES


def test_source_transform_aliases_are_mirrored_in_chain():
    # pow_affine/power_series/exp_affine must be accepted by BOTH frontends;
    # a source-only alias produced chips the chain compiler rejected (CR2-1).
    from coeff_program_source import _NATIVE_TRANSFORM_ALIASES

    assert _NATIVE_TRANSFORM_ALIASES == EXPECTED_TEXT_ALIASES
    for alias, target in _NATIVE_TRANSFORM_ALIASES.items():
        assert chain.LEGACY_NAME_ALIASES.get(alias) == target, alias


def test_coeff_registry_has_no_unpinned_generic_complex_args():
    # Complex literal support for linear/pow/exp/round is intentionally
    # special-cased and fingerprint-golden-tested. A future registry-level
    # type:"complex" arg must update the packer and wire corpus explicitly.
    offenders = [
        (name, arg.get("name") or arg.get("ph") or idx)
        for name, spec in legacy_registry()["by_name"].items()
        for idx, arg in enumerate(spec.get("args") or ())
        if arg.get("type") == "complex"
    ]
    assert offenders == []


def test_coeff_legacy_enum_inverse_uses_registry_map_and_trims_default():
    spec = legacy_registry()["by_name"]["roots"]
    base = {
        "op": chain.COEFF_OP_LEGACY,
        "fn_index": spec["fn_index"],
        "src": chain.COEFF_SEL_POLY,
        "tgt": chain.COEFF_SEL_POLY,
    }
    assert chain._legacy_transforms([{**base, "args": [8, chain._ENUM_ARG_VALUES["hi"]]}]) == [["roots"]]
    assert chain._legacy_transforms([{**base, "args": [8, chain._ENUM_ARG_VALUES["lo"]]}]) == [["roots", "8", "lo"]]
    assert chain._legacy_transforms([{**base, "args": [8, 2.0]}]) == []


def test_coeff_compat_signature_transforms_are_pinned():
    registry = legacy_registry()["by_name"]
    signature_names = {
        name
        for name, spec in registry.items()
        if spec.get("compat_signatures")
    }
    assert signature_names == {"linear", "exp", "round", "pow"}
    assert set(signature_names) == {
        spec["name"]
        for spec in registry.values()
        if spec["fn_index"] in {chain.FN_LINEAR, chain.FN_EXP, chain.FN_ROUND, chain.FN_POW}
    }
    for name in signature_names:
        for signature in registry[name]["compat_signatures"]:
            assert signature["wire"] in {"complex_lanes", "flat_complex_components", "real_lanes"}
            assert signature.get("arg_counts") or signature.get("andy_arg_counts")


def test_coeff_forward_packer_uses_signature_interpreter_not_fn_specific_helpers():
    with open(os.path.join(LAMBDA_DIR, "coeff_program_chain.py"), "r", encoding="utf-8") as fh:
        py = fh.read()
    forbidden_defs = [
        "def _linear_legacy_args",
        "def _pow_legacy_args",
        "def _exp_legacy_args",
        "def _round_legacy_args",
        "def _affine_pair_legacy_args",
    ]
    assert [name for name in forbidden_defs if name in py] == []
    assert "def _compat_signature_args" in py


def test_generated_js_vocab_matches_registry():
    # coeff_vocab_js.js is what the browser loads; the frontend harness runs
    # against the file on disk, so a stale or hand-edited artifact must fail
    # here (and in gen_coeff_vocab.py --check / the predeploy gate).
    sys.path.insert(0, LAMBDA_DIR)
    from gen_coeff_vocab import build_vocab, render_js, JS_OUT

    vocab = build_vocab()
    assert vocab["structuralChips"] == _structural_payload()
    assert vocab["programProfiles"] == _json_payload(PROGRAM_PROFILES)
    assert vocab["aliasToCanonical"] == EXPECTED_ALIASES
    assert vocab["sourceAliasByName"] == {v: k for k, v in EXPECTED_TEXT_ALIASES.items()}
    assert vocab["chipNameByRegistryName"] == EXPECTED_CHIP_NAMES
    assert vocab["fnIndexByName"] == {
        name: spec["fn_index"] for name, spec in legacy_registry()["by_name"].items()
    }
    # The chip catalog (param shapes/descs/UI hints) is part of the vocab:
    # every registry function must have a ui block with a desc, categories
    # must match the registry, and the andy flag stays universal (the JS
    # hydrator appends the shared andy param to every transform).
    registry = legacy_registry()["by_name"]
    assert set(vocab["ctCatalog"]) == set(registry)
    for name, entry in vocab["ctCatalog"].items():
        assert entry.get("desc"), f"{name}: missing ui desc"
        assert entry["category"] == registry[name]["category"], name
        assert registry[name]["supports_andy"], name
    assert set(vocab["programParamDefs"]) == {"exp", "round"}
    # Ordering is UI contract: ctCatalog keys appear in the transform picker
    # in insertion order (fn_index order), categories in registry order.
    assert list(vocab["ctCatalog"]) == vocab["names"]
    assert list(vocab["categoryMeta"]) == ["structural", "accumulation", "elementwise", "roots"]
    with open(JS_OUT, "r", encoding="utf-8") as fh:
        assert fh.read() == render_js(), "coeff_vocab_js.js is stale; run lambda/gen_coeff_vocab.py"


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
