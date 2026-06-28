"""Shared schema guards for Param/Coeff program registries.

These tests intentionally check only stable registry contracts. Profile-specific
wire compatibility belongs in the existing drift suites.
"""

import json
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

import coeff_program_chain as coeff_chain
import param_program_chain as param_chain


LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
PARAM_REGISTRY = os.path.join(LAMBDA_DIR, "param_legacy_registry.json")
COEFF_REGISTRY = os.path.join(LAMBDA_DIR, "coeff_legacy_registry.json")
PROGRAM_PROFILES = os.path.join(LAMBDA_DIR, "program_profiles.json")

PARAM_ALLOWED_ARG_TYPES = {"real", "complex"}
COEFF_ALLOWED_ARG_TYPES = {"real", "complex", "int", "enum"}
PARAM_VARIABLE_ARITY_EXEMPTIONS = {"moebius", "inv_t_plus_2", "add"}
COEFF_COMPAT_SIGNATURE_NAMES = {"linear", "exp", "round", "pow"}
COEFF_PACKED_DISPLAY_OUTLIERS = {"pow"}


def _json(path):
    with open(path, "r", encoding="utf-8") as fh:
        return json.load(fh)


def _functions(payload):
    return list(payload.get("functions") or [])


def _function_map(payload):
    return {fn["name"]: fn for fn in _functions(payload)}


def _assert_unique(values, label):
    assert len(values) == len(set(values)), f"duplicate {label}: {values}"


def _assert_common_registry_shape(payload, *, label, allowed_arg_types, category_meta):
    assert payload.get("version") == 1
    functions = _functions(payload)
    assert functions, f"{label} registry has no functions"

    names = [fn.get("name") for fn in functions]
    fn_indices = [fn.get("fn_index") for fn in functions]
    _assert_unique(names, f"{label} function names")
    _assert_unique(fn_indices, f"{label} fn_index values")

    for fn in functions:
        name = fn.get("name")
        assert isinstance(name, str) and name and name == name.lower(), fn
        assert isinstance(fn.get("fn_index"), int) and fn["fn_index"] > 0, name
        assert isinstance(fn.get("ui"), dict), name
        assert str(fn["ui"].get("desc") or "").strip(), f"{label}:{name} missing ui.desc"

        category = fn.get("category") or fn["ui"].get("category")
        assert category in category_meta, f"{label}:{name} unknown category {category!r}"

        for idx, arg in enumerate(fn.get("args") or []):
            assert str(arg.get("name") or "").strip(), f"{label}:{name} arg {idx} missing name"
            assert str(arg.get("type") or "").strip() in allowed_arg_types, f"{label}:{name}.{arg.get('name')}"
            assert "default" in arg, f"{label}:{name}.{arg.get('name')} missing default"


def test_param_registry_common_schema():
    payload = _json(PARAM_REGISTRY)
    profile = _json(PROGRAM_PROFILES)["profiles"]["param"]
    category_meta = payload["category_meta"]
    assert "ui" not in payload or "categories" not in payload.get("ui", {})
    assert set(payload.get("variable_arg_forms") or {}) == PARAM_VARIABLE_ARITY_EXEMPTIONS

    _assert_common_registry_shape(
        payload,
        label="param",
        allowed_arg_types=PARAM_ALLOWED_ARG_TYPES,
        category_meta=category_meta,
    )

    selectors = set(profile["selectors"]["legacy_src"]) | set(profile["selectors"]["legacy_tgt"])
    for fn in _functions(payload):
        name = fn["name"]
        assert set(fn.get("allowed_src") or []) <= selectors, name
        assert set(fn.get("allowed_tgt") or []) <= selectors, name
        runtime_args = list(fn.get("args") or [])
        ui_params = list((fn.get("ui") or {}).get("params") or [])
        if name in PARAM_VARIABLE_ARITY_EXEMPTIONS:
            continue
        assert len(ui_params) == len(runtime_args), f"param:{name} ui.params/runtime arg mismatch"
        for arg, ui_arg in zip(runtime_args, ui_params):
            assert (ui_arg.get("ph") or ui_arg.get("name")) == arg["name"], f"param:{name}.{arg['name']}"
            assert str(ui_arg.get("title") or ui_arg.get("help") or "").strip(), f"param:{name}.{arg['name']}"


def test_param_registry_compat_family_is_complete_and_runtime_pinned():
    payload = _json(PARAM_REGISTRY)
    compat = payload["compat"]
    expected_keys = {
        "target_arg_indexes",
        "independent_targets",
        "variable_arg_counts",
        "target_first",
        "target_last",
        "dither_target_first",
    }
    assert set(compat) == expected_keys
    assert compat["target_arg_indexes"] == {
        name: idx for name, idx in sorted(param_chain._LEGACY_TARGET_ARG_INDEXES.items())
    }
    assert sorted(compat["independent_targets"]) == sorted(param_chain._REDUNDANT_LEGACY_TARGET_ARG_NAMES)
    assert sorted(compat["target_first"]) == sorted(param_chain._LEGACY_TARGET_FIRST_CHIPS)
    assert sorted(compat["target_last"]) == sorted(param_chain._LEGACY_TARGET_LAST_CHIPS)
    assert sorted(compat["dither_target_first"]) == sorted(param_chain._LEGACY_DITHER_TARGET_FIRST_CHIPS)
    assert {
        name: sorted(int(v) for v in counts)
        for name, counts in compat["variable_arg_counts"].items()
    } == {
        name: sorted(int(v) for v in counts)
        for name, counts in param_chain._VARIABLE_LEGACY_ARG_COUNTS.items()
    }
    assert set(compat["variable_arg_counts"]) == PARAM_VARIABLE_ARITY_EXEMPTIONS


def test_coeff_registry_common_schema_and_compat_signature_exceptions():
    payload = _json(COEFF_REGISTRY)
    profile = _json(PROGRAM_PROFILES)["profiles"]["coeff"]
    category_meta = payload["category_meta"]

    _assert_common_registry_shape(
        payload,
        label="coeff",
        allowed_arg_types=COEFF_ALLOWED_ARG_TYPES,
        category_meta=category_meta,
    )

    selectors = set(profile["selectors"]["src"]) | set(profile["selectors"]["tgt"])
    for fn in _functions(payload):
        name = fn["name"]
        assert set(fn.get("allowed_src") or []) <= selectors, name
        assert set(fn.get("allowed_tgt") or []) <= selectors, name
        runtime_args = list(fn.get("args") or [])
        ui_params = list((fn.get("ui") or {}).get("params") or [])
        if name in COEFF_PACKED_DISPLAY_OUTLIERS:
            assert name == "pow"
            assert len(runtime_args) == 4
            assert len(ui_params) == 2
            continue
        if runtime_args:
            assert len(ui_params) == len(runtime_args), f"coeff:{name} ui.params/runtime arg mismatch"
            for arg, ui_arg in zip(runtime_args, ui_params):
                assert (ui_arg.get("ph") or ui_arg.get("name")), f"coeff:{name}.{arg['name']}"
                assert str(ui_arg.get("title") or ui_arg.get("help") or "").strip(), f"coeff:{name}.{arg['name']}"

    by_name = _function_map(payload)
    signature_names = {name for name, fn in by_name.items() if fn.get("compat_signatures")}
    assert signature_names == COEFF_COMPAT_SIGNATURE_NAMES
    assert signature_names == {
        spec["name"]
        for spec in coeff_chain.legacy_registry()["by_name"].values()
        if spec.get("compat_signatures")
    }

    for name in signature_names:
        for sig in by_name[name]["compat_signatures"]:
            assert sig["wire"] in {"complex_lanes", "flat_complex_components", "real_lanes"}
            assert sig.get("arg_counts") or sig.get("andy_arg_counts"), f"coeff:{name}:{sig['name']}"
            assert set(sig.get("arg_types") or []) <= {"real", "complex"}, f"coeff:{name}:{sig['name']}"


def test_coeff_registry_optional_args_are_normal_args_not_capability_flags():
    payload = _json(COEFF_REGISTRY)
    assert "supports_andy" not in json.dumps(payload)
    shared_optional = payload.get("shared_optional_args") or []
    assert [arg["name"] for arg in shared_optional] == ["andy"]
    andy = shared_optional[0]
    assert andy["type"] == "real"
    assert andy["default"] == 0.0
    assert andy.get("optional") is True

    for name, spec in coeff_chain.legacy_registry()["by_name"].items():
        optional = list(spec.get("optional_args") or ())
        assert [arg["name"] for arg in optional] == ["andy"], name
        assert optional[0].get("optional") is True
