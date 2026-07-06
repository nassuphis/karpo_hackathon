"""Drift guards for the CR18 merged opcode registry."""
import json
import os
import re
import sys


ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
sys.path.insert(0, LAMBDA_DIR)


def _read(path):
    with open(path, "r", encoding="utf-8") as fh:
        return fh.read()


def _json_payload(name):
    with open(os.path.join(LAMBDA_DIR, name), "r", encoding="utf-8") as fh:
        return json.load(fh)


def _c_enum_values(source, prefix):
    return {
        name: int(value)
        for name, value in re.findall(rf"\b{prefix}(\w+)\s*=\s*(\d+)", source)
    }


def test_generated_merged_opcode_artifacts_are_fresh():
    import gen_merged_opcodes

    assert _read(gen_merged_opcodes.PY_OUT) == gen_merged_opcodes.render_py()
    assert _read(gen_merged_opcodes.H_OUT) == gen_merged_opcodes.render_h()
    assert _read(gen_merged_opcodes.JS_OUT) == gen_merged_opcodes.render_js()


def test_merged_opcode_registry_ranges_and_symbols_are_unique():
    payload = _json_payload("merged_opcodes.json")
    ids = [int(op["id"]) for op in payload["opcodes"]]
    symbols = [op["symbol"] for op in payload["opcodes"]]
    assert len(ids) == len(set(ids))
    assert len(symbols) == len(set(symbols))
    assert payload["ranges"]["coeff_reference"] == [1, 30]
    assert payload["ranges"]["param_specific"] == [31, 47]
    assert payload["ranges"]["reserved"] == [48, 63]
    assert payload["ranges"]["solve_score"] == [64, 95]
    assert set(range(45, 64)).isdisjoint(ids)  # 43/44 = param registers
    assert set(range(93, 96)).isdisjoint(ids)


def test_merged_opcode_python_and_header_constants_match_json():
    import merged_opcodes

    payload = _json_payload("merged_opcodes.json")
    header = _read(os.path.join(LAMBDA_DIR, "merged_opcodes.h"))
    h_ops = _c_enum_values(header, "MERGED_OP_")
    for op in payload["opcodes"]:
        assert getattr(merged_opcodes, op["symbol"]) == int(op["id"])
        short = op["symbol"][len("MERGED_OP_"):]
        assert h_ops[short] == int(op["id"])
    for name, value in payload["registries"].items():
        assert getattr(merged_opcodes, f"MERGED_REGISTRY_{name.upper()}") == int(value)


def test_merged_coeff_reference_ids_match_existing_coeff_constants():
    import coeff_program_chain as coeff

    payload = _json_payload("merged_opcodes.json")
    for op in payload["opcodes"]:
        if not op.get("source_symbol", "").startswith("COEFF_OP_"):
            continue
        assert int(op["id"]) == getattr(coeff, op["source_symbol"])


def test_merged_param_specific_ids_map_to_existing_param_constants():
    import param_program_chain as param

    payload = _json_payload("merged_opcodes.json")
    expected = {
        "PARAM_OP_PUSH_T1": 31,
        "PARAM_OP_PUSH_T2": 32,
        "PARAM_OP_PUSH_P1": 33,
        "PARAM_OP_PUSH_P2": 34,
        "PARAM_OP_EMIT_P1": 35,
        "PARAM_OP_EMIT_P2": 36,
        "PARAM_OP_RATIO": 37,
        "PARAM_OP_DIVIDE": 38,
        "PARAM_OP_RECIPROCAL": 39,
        "PARAM_OP_UNIT_CIRCLE": 40,
        "PARAM_OP_SQUARE": 41,
        "PARAM_OP_CUBE": 42,
        "PARAM_OP_PUSH_REG": 43,
        "PARAM_OP_STORE_REG": 44,
    }
    by_source = {op["source_symbol"]: int(op["id"]) for op in payload["opcodes"] if op.get("profile") == "param"}
    assert by_source == expected
    for source_symbol in expected:
        assert getattr(param, source_symbol) > 0


def test_merged_solve_score_ids_map_to_existing_solve_score_constants():
    payload = _json_payload("merged_opcodes.json")
    source = _read(os.path.join(LAMBDA_DIR, "solve_score.h"))
    score_ops = _c_enum_values(source, "SOLVE_SCORE_OP_")
    for op in payload["opcodes"]:
        if op.get("profile") != "solve_score" or "source_symbol" not in op:
            continue
        source_symbol = op["source_symbol"]
        expected = int(op["id"])
        assert expected == score_ops[source_symbol[len("SOLVE_SCORE_OP_"):]]
