import json
import os
import sys


ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
LAMBDA_DIR = os.path.join(ROOT, "lambda")
sys.path.insert(0, LAMBDA_DIR)


def test_root_source_compiles_and_default_expands_args():
    from root_program_source import compile_root_program_source

    compiled = compile_root_program_source("rotate_roots(0.25)\npull_unit_circle()")

    assert compiled["chain"] == [
        {"name": "rotate_roots", "fn_index": 1, "args": [0.25]},
        {"name": "pull_unit_circle", "fn_index": 2, "args": [0.75, 1.0]},
    ]
    assert compiled["root_transforms"] == [
        ["rotate_roots", "0.25"],
        ["pull_unit_circle", "0.75", "1"],
    ]
    assert compiled["fingerprint"].startswith("sha256:")
    assert '"kind":"root"' in compiled["execution_spec"]


def test_root_source_and_legacy_rows_have_same_fingerprint_for_default_args():
    from root_program_source import compile_root_program_chain, compile_root_program_source

    source = compile_root_program_source("pull_unit_circle()")
    explicit = compile_root_program_chain([["pull_unit_circle", "0.75", "1.0"]])
    omitted = compile_root_program_chain([["pull_unit_circle"]])

    assert source["chain"] == explicit["chain"] == omitted["chain"]
    assert source["fingerprint"] == explicit["fingerprint"] == omitted["fingerprint"]


def test_root_source_rejects_unknown_and_noop_assignment_non_strict():
    from root_program_source import compile_root_program_source

    compiled = compile_root_program_source("roots = roots\nunknown_op(1)", strict=False)

    assert compiled["chain"] == []
    messages = [diag["message"] for diag in compiled["diagnostics"]]
    assert any("no-op" in message for message in messages)
    assert any("unknown root transform" in message for message in messages)


def test_root_source_text_from_object_and_array_rows():
    from root_program_source import root_source_text_from_chain

    source = root_source_text_from_chain([
        {"name": "rotate_roots", "fn_index": 1, "args": [0.5]},
        ["moebius", "1", "0", "0", "1"],
    ])

    assert source.splitlines() == [
        "rotate_roots(0.5)",
        "moebius(1, 0, 0, 1)",
    ]


def test_root_execution_spec_is_json():
    from root_program_source import compile_root_program_source

    compiled = compile_root_program_source("invert_roots()")
    payload = json.loads(compiled["execution_spec"])

    assert payload["kind"] == "root"
    assert payload["tokens"][0]["registry"] == "root"
    assert payload["tokens"][0]["fn_index"] == 5
