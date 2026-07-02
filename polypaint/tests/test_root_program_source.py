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
    codes = [diag["code"] for diag in compiled["diagnostics"]]
    assert "no_op_assignment" in codes
    assert "unknown_transform" in codes


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


def test_root_chain_rejects_name_fn_index_mismatch():
    from root_program_source import RootProgramSourceCompileError, compile_root_program_chain

    try:
        compile_root_program_chain([{"name": "rotate_roots", "fn_index": 5}])
    except RootProgramSourceCompileError as exc:
        assert exc.diagnostics[0]["code"] == "root_chain_error"
        assert "name/fn_index mismatch" in exc.diagnostics[0]["message"]
    else:
        raise AssertionError("expected mismatched root row to fail")


def test_root_chain_cap_is_enforced_with_structured_diagnostic():
    from root_program_source import RootProgramSourceCompileError, compile_root_program_chain

    try:
        compile_root_program_chain([["rotate_roots", "0"]] * 17)
    except RootProgramSourceCompileError as exc:
        assert exc.diagnostics[0]["code"] == "root_chain_too_long"
        assert "max is 16" in exc.diagnostics[0]["message"]
    else:
        raise AssertionError("expected over-cap root chain to fail")


def test_bad_json_root_transforms_returns_diagnostic_in_non_strict_mode():
    from root_program_source import compile_root_program_chain

    compiled = compile_root_program_chain({"root_transforms": "5"}, strict=False)

    assert compiled["chain"] == []
    assert compiled["diagnostics"][0]["code"] == "root_chain_error"
    assert "must be a list" in compiled["diagnostics"][0]["message"]


def test_root_execution_spec_is_json():
    from root_program_source import compile_root_program_source

    compiled = compile_root_program_source("invert_roots()")
    payload = json.loads(compiled["execution_spec"])

    assert payload["kind"] == "root"
    assert payload["tokens"][0]["registry"] == "root"
    assert payload["tokens"][0]["fn_index"] == 5


def test_root_chain_cap_pinned_three_ways_profile_python_c():
    # The cap lived in three places that silently diverged (profile said 64,
    # C truncated at 16, Python enforced nothing). Pin all three together.
    import re

    from root_program_source import MAX_ROOT_TRANSFORMS

    base = os.path.join(os.path.dirname(__file__), "..", "lambda")
    with open(os.path.join(base, "program_profiles.json"), "r", encoding="utf-8") as fh:
        profile_cap = json.load(fh)["profiles"]["root"]["value_caps"]["program_tokens"]
    with open(os.path.join(base, "root_xforms.h"), "r", encoding="utf-8") as fh:
        match = re.search(r"#define\s+MAX_RT_CHAIN\s+(\d+)", fh.read())
    assert match, "MAX_RT_CHAIN not found in root_xforms.h"
    c_cap = int(match.group(1))

    assert MAX_ROOT_TRANSFORMS == c_cap == profile_cap, (
        f"root chain cap drift: python={MAX_ROOT_TRANSFORMS} c={c_cap} profile={profile_cap}"
    )


def test_null_or_blank_chain_values_are_absent_not_errors():
    # H5 regression: {"chain": null} raised after the key-presence rewrite;
    # a JSON null/blank value means absent (explicit [] is a real empty chain).
    from root_program_source import compile_root_program_chain

    for payload in (
        {"chain": None},
        {"root_transforms": None},
        {"root_transforms": ""},
        {"root_program": {"chain": None}},
    ):
        compiled = compile_root_program_chain(payload)
        assert compiled["chain"] == [], payload
    explicit_empty = compile_root_program_chain({"root_transforms": []})
    assert explicit_empty["chain"] == []


def test_non_round_numeric_args_round_trip_fingerprint_identically():
    # G9 regression corpus: _format_number was %g (6 sig figs), so
    # regenerated source silently changed the program. repr+zero-fold must
    # round-trip chain -> source -> chain fingerprint-identically and keep
    # nearby values distinct.
    from root_program_source import (
        compile_root_program_chain,
        compile_root_program_source,
        root_source_text_from_chain,
    )

    for raw in ("1.234567890123", "0.30000001", "0.30000009", "0.1", "1e-17", "12345678.90123456"):
        direct = compile_root_program_chain([["rotate_roots", raw]])
        source = root_source_text_from_chain([["rotate_roots", raw]])
        reparsed = compile_root_program_source(source)
        assert reparsed["fingerprint"] == direct["fingerprint"], (raw, source)
        # Idempotent: regenerating from the reparsed chain emits the same text.
        assert root_source_text_from_chain(reparsed["chain"]) == source, raw

    near_a = compile_root_program_source(root_source_text_from_chain([["rotate_roots", "0.30000001"]]))
    near_b = compile_root_program_source(root_source_text_from_chain([["rotate_roots", "0.30000009"]]))
    assert near_a["fingerprint"] != near_b["fingerprint"]
