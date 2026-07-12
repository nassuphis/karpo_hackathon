"""CR32 F12: prepared root-transform chains must be bit-identical to the
legacy per-row fallback for every transform, arity, default form, pole, and
non-finite input.

The probe (tests/root_xforms_native_probe.c) parses each chain through
parse_root_xform_json — the exact path every consumer binary uses — then
applies the prepared entries and a prep_fn-zeroed copy to identical roots and
compares the float32 results bit-for-bit.
"""
import json
import os
import pathlib
import shutil
import subprocess

import pytest

ROOT = pathlib.Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"
PROBE_SRC = ROOT / "tests" / "root_xforms_native_probe.c"


@pytest.fixture(scope="module")
def probe(tmp_path_factory):
    cc = os.environ.get("CC") or shutil.which("cc") or shutil.which("clang") or shutil.which("gcc")
    if not cc:
        pytest.skip("no C compiler available")
    out = tmp_path_factory.mktemp("root_probe") / "root_xforms_native_probe"
    result = subprocess.run(
        [cc, "-O2", "-I", str(LAMBDA_DIR), str(PROBE_SRC), "-lm", "-o", str(out)],
        capture_output=True, text=True, timeout=120,
    )
    assert result.returncode == 0, f"probe compile failed: {result.stderr}"
    return out


CHAINS = {
    # every transform with explicit args
    "rotate_full": [{"name": "rotate_roots", "args": [0.125]}],
    "pull_unit_circle_full": [{"name": "pull_unit_circle", "args": [0.75, 0.9]}],
    "roots_toline": [{"name": "roots_toline", "args": []}],
    "line_to_unit_circle": [{"name": "line_to_unit_circle", "args": []}],
    "invert_roots": [{"name": "invert_roots", "args": []}],
    "add_complex_full": [{"name": "add_complex", "args": [0.2, -0.1]}],
    "mul_complex_full": [{"name": "mul_complex", "args": [0.9, 0.1]}],
    "moebius_full": [{"name": "moebius", "args": [1.0, 0.5, 0.25, 1.0]}],
    "pull_towards_center_full": [{"name": "pull_towards_center", "args": [0.8, 0.6]}],
    # omitted / partial defaults
    "rotate_no_args": [{"name": "rotate_roots", "args": []}],
    "pull_unit_circle_defaults": [{"name": "pull_unit_circle", "args": []}],
    "pull_unit_circle_one_arg": [{"name": "pull_unit_circle", "args": [0.5]}],
    "add_complex_one_arg": [{"name": "add_complex", "args": [0.3]}],
    "mul_complex_no_args": [{"name": "mul_complex", "args": []}],
    "moebius_partial": [{"name": "moebius", "args": [2.0, 1.0]}],
    "pull_towards_center_defaults": [{"name": "pull_towards_center", "args": []}],
    # numeric edges: zero / negative sigma (prepared clamps at 1e-10; the
    # legacy body must see the identical clamped value)
    "pull_zero_sigma": [{"name": "pull_unit_circle", "args": [0.0, 1.0]}],
    "pull_negative_sigma": [{"name": "pull_unit_circle", "args": [-0.5, 1.0]}],
    "center_zero_sigma": [{"name": "pull_towards_center", "args": [1.0, 0.0]}],
    # poles: moebius denominator hits zero for some root
    "moebius_pole": [{"name": "moebius", "args": [1.0, 0.0, 1.0, 0.0]}],
    # fn_index-only entry (no name)
    "fn_index_only": [{"fn_index": 1, "args": [0.25]}],
    # tuple wire form
    "tuple_form": [["rotate_roots", "0.25"], ["mul_complex", "0.5", "0.5"]],
    # full 16-entry mixed chain
    "sixteen_chain": [
        {"name": "rotate_roots", "args": [0.05 * i]} if i % 3 == 0 else
        {"name": "mul_complex", "args": [0.95, 0.02 * i]} if i % 3 == 1 else
        {"name": "add_complex", "args": [0.01 * i, -0.01 * i]}
        for i in range(16)
    ],
    # empty chain (no-op)
    "empty": [],
}


@pytest.mark.parametrize("label", sorted(CHAINS))
def test_prepared_matches_legacy(probe, tmp_path, label):
    chain_path = tmp_path / f"{label}.json"
    chain_path.write_text(json.dumps(CHAINS[label]))
    result = subprocess.run(
        [str(probe), str(chain_path)], capture_output=True, text=True, timeout=60,
    )
    assert result.returncode == 0, (
        f"chain {label}: prepared/legacy mismatch\n{result.stderr}\n{result.stdout}"
    )
    payload = json.loads(result.stdout.strip())
    assert payload["match"] == 1
    expected_entries = len(CHAINS[label])
    assert payload["n_entries"] == expected_entries
