"""Drift guards for the Phase-1 program profile registry."""
import json
import os
import sys


ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
LAMBDA_DIR = os.path.join(ROOT, "lambda")
PROGRAM_PROFILES_JSON = os.path.join(LAMBDA_DIR, "program_profiles.json")
PROGRAM_PROFILES_JS = os.path.join(ROOT, "program_profiles_js.js")

sys.path.insert(0, LAMBDA_DIR)


def _json_profiles():
    with open(PROGRAM_PROFILES_JSON, "r", encoding="utf-8") as fh:
        return json.load(fh)


def test_generated_python_profiles_match_json():
    from program_profiles import PROGRAM_PROFILES

    assert PROGRAM_PROFILES == _json_profiles()


def test_generated_js_profiles_match_json_and_generator():
    from gen_program_profiles import render_js, JS_OUT

    assert JS_OUT == PROGRAM_PROFILES_JS
    with open(PROGRAM_PROFILES_JS, "r", encoding="utf-8") as fh:
        assert fh.read() == render_js()


def test_program_source_core_uses_generated_profile_module():
    import program_source_core
    from program_profiles import PROGRAM_PROFILES

    saved = program_source_core._PROFILES
    program_source_core._PROFILES = None
    try:
        assert program_source_core.load_program_profiles() is PROGRAM_PROFILES
    finally:
        program_source_core._PROFILES = saved


def test_profile_registry_has_required_profiles_and_contracts():
    profiles = _json_profiles()["profiles"]
    assert set(profiles) == {"param", "coeff", "root", "solve_score"}
    assert profiles["param"]["output_contract"]["kind"] == "scalar_registers"
    assert profiles["coeff"]["output_contract"]["kind"] == "fixed_vector"
    assert profiles["root"]["output_contract"]["kind"] == "in_place_vector"
    assert profiles["solve_score"]["output_contract"]["kind"] == "terminal_scalar_or_emit_channels"


def test_solve_score_profile_caps_match_compiler_constants():
    import solve_score_chain

    caps = _json_profiles()["profiles"]["solve_score"]["value_caps"]
    assert caps["program_tokens"] == solve_score_chain.MAX_PROGRAM_TOKENS
    assert caps["max_metric_slots"] == solve_score_chain.MAX_METRIC_SLOTS
    assert caps["max_output_channels"] == solve_score_chain.MAX_OUTPUT_CHANNELS


def test_root_profile_registry_matches_root_xforms_dispatch():
    profiles = _json_profiles()["profiles"]
    root_profile = profiles["root"]
    assert root_profile["source"]["registry"] == "root_legacy_registry.json"
    registry_path = os.path.join(LAMBDA_DIR, root_profile["source"]["registry"])
    with open(registry_path, "r", encoding="utf-8") as fh:
        registry = json.load(fh)
    names = {str(item["name"]) for item in registry.get("functions") or []}
    with open(os.path.join(LAMBDA_DIR, "root_xforms.h"), "r", encoding="utf-8") as fh:
        root_xforms = fh.read()
    for name in names:
        assert f'"{name}"' in root_xforms
