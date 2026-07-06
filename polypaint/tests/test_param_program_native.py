import json
import os
import subprocess
import sys
import tempfile


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

from param_program_chain import (
    EXPR_ADD,
    EXPR_T1,
    EXPR_T2,
    PARAM_OP_CONST,
    PARAM_OP_EMIT_P1,
    compile_param_program_chain,
)
from merged_opcodes import (
    MERGED_OP_NATIVE_TRANSFORM,
    MERGED_OP_PARAM_EMIT_P1,
    MERGED_OP_PARAM_EMIT_P2,
    MERGED_OP_PARAM_PUSH_T1,
    MERGED_OP_PARAM_PUSH_T2,
    MERGED_OP_PARAM_UNIT_CIRCLE,
    MERGED_OP_TYPED_BINARY,
    MERGED_OP_TYPED_PUSH_SCALAR,
)


LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")


def _run_sweep(spec, out_path):
    proc = subprocess.run(
        [SWEEP, out_path],
        input=json.dumps(spec),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert proc.returncode == 0, proc.stderr
    with open(out_path, "rb") as fh:
        data = fh.read()
    os.remove(out_path)
    return json.loads(proc.stdout), data


def _run_sweep_process(spec):
    with tempfile.NamedTemporaryFile(prefix="pp_param_program_native_", suffix=".bin", delete=False) as fh:
        out_path = fh.name
    try:
        return subprocess.run(
            [SWEEP, out_path],
            input=json.dumps(spec),
            capture_output=True,
            text=True,
            timeout=30,
        )
    finally:
        try:
            os.remove(out_path)
        except FileNotFoundError:
            pass


def _native_payload(chain):
    compiled = compile_param_program_chain(chain)
    payload = {
        "version": 1,
        "tokens": compiled["tokens"],
        "stack_max": compiled["stack_max"],
        "uses_legacy_fast_path": compiled["uses_legacy_fast_path"],
    }
    scalar_exprs = compiled.get("scalar_exprs") or []
    if scalar_exprs:
        payload["scalar_exprs"] = scalar_exprs
    return payload


def test_param_dump_rejects_native_param_transforms():
    proc = _run_sweep_process({
        "mode": "param_dump",
        "n1": 16,
        "n2": 16,
        "param_transforms": [["unit_circle"], ["square"]],
    })
    assert proc.returncode != 0
    assert "param_transforms is no longer accepted by the native runtime" in proc.stderr


def test_param_dump_compiled_legacy_tokens_run_without_native_chain():
    vm_spec = {
        "mode": "param_dump",
        "n1": 16,
        "n2": 16,
        "param_program": _native_payload([
            ["legacy", "unit_circle", "both", "both"],
            ["legacy", "square", "both", "both"],
        ]),
    }
    meta, vm = _run_sweep(vm_spec, "/tmp/pp_param_dump_vm.bin")
    assert meta["param_program_tokens"] == 2
    assert len(vm) == 16 * 16 * 16


def test_param_gen_compiled_tokens_threaded_output_runs_without_native_chain():
    vm_spec = {
        "mode": "param_gen",
        "n1": 12,
        "n2": 12,
        "times": 2,
        "n_threads": 4,
        "param_program": _native_payload([
            ["legacy", "unit_circle", "both", "both"],
            ["legacy", "square", "both", "both"],
        ]),
    }
    meta, vm = _run_sweep(vm_spec, "/tmp/pp_param_gen_vm.bin")
    assert meta["threads"] == 4
    assert meta["param_program_tokens"] == 2
    assert len(vm) == 12 * 12 * 2 * 16


def test_param_dump_compiled_coeff_legacy_tokens_run_without_native_chain():
    chain = [["coeff2"], ["coeff3"], ["coeff5"], ["coeff12"]]
    vm_spec = {
        "mode": "param_dump",
        "n1": 12,
        "n2": 12,
        "param_program": _native_payload(chain),
    }
    meta, vm = _run_sweep(vm_spec, "/tmp/pp_param_dump_coeff_vm.bin")
    assert meta["param_program_tokens"] == len(chain)
    assert len(vm) == 12 * 12 * 16


def test_param_dump_compiled_moebius_complex_coefficients_run_without_native_chain():
    vm_chain = [["legacy", "moebius", "both", "both", "1-2j", "2+1j", "-2j+4", "10j-3"]]
    vm_spec = {
        "mode": "param_dump",
        "n1": 12,
        "n2": 12,
        "param_program": _native_payload(vm_chain),
    }
    meta, vm = _run_sweep(vm_spec, "/tmp/pp_param_dump_moebius_vm.bin")
    assert meta["param_program_tokens"] == 1
    assert len(vm) == 12 * 12 * 16


def test_param_dump_stack_program_expresses_sum_difference():
    spec = {
        "mode": "param_dump",
        "n1": 8,
        "n2": 8,
        "param_program": _native_payload([
            ["push", "t1"],
            ["push", "t2"],
            ["add"],
            ["emit", "p1"],
            ["push", "t1"],
            ["push", "t2"],
            ["subtract"],
            ["emit", "p2"],
        ]),
    }
    meta, data = _run_sweep(spec, "/tmp/pp_param_dump_sumdiff.bin")
    assert meta["param_program_tokens"] == 8
    assert len(data) == 8 * 8 * 16


def test_param_program_native_accepts_missing_v1_version_and_v2_equivalent_payloads():
    payload = _native_payload([
        ["const", "1"],
        ["emit", "p1"],
    ])
    payload_without_version = dict(payload)
    payload_without_version.pop("version", None)
    base = {
        "mode": "param_dump",
        "n1": 4,
        "n2": 4,
    }

    meta, data = _run_sweep(
        {**base, "param_program": payload_without_version},
        "/tmp/pp_param_dump_missing_version.bin",
    )
    assert meta["param_program_tokens"] == 2
    assert len(data) == 4 * 4 * 16

    v1_payload = _native_payload([
        ["push", "t1"],
        ["const", "1"],
        ["add"],
        ["emit", "p1"],
        ["push", "t2"],
        ["unit_circle"],
        ["emit", "p2"],
    ])
    v2_payload = {
        "version": 2,
        "tokens": [
            {"op": MERGED_OP_PARAM_PUSH_T1},
            {"op": MERGED_OP_TYPED_PUSH_SCALAR, "n_args": 1, "args": [1.0], "args_im": [0.0]},
            {"op": MERGED_OP_TYPED_BINARY, "fn_index": 1},
            {"op": MERGED_OP_PARAM_EMIT_P1},
            {"op": MERGED_OP_PARAM_PUSH_T2},
            {"op": MERGED_OP_PARAM_UNIT_CIRCLE},
            {"op": MERGED_OP_PARAM_EMIT_P2},
        ],
        "stack_max": 2,
        "uses_legacy_fast_path": False,
    }
    _, v1_data = _run_sweep({**base, "param_program": v1_payload}, "/tmp/pp_param_dump_v1_program.bin")
    meta, v2_data = _run_sweep({**base, "param_program": v2_payload}, "/tmp/pp_param_dump_v2_program.bin")
    assert meta["param_program_tokens"] == len(v2_payload["tokens"])
    assert v2_data == v1_data

    native_transform_payload = {
        "version": 2,
        "tokens": [
            {"op": MERGED_OP_NATIVE_TRANSFORM, "registry": "param", "fn_index": 2, "src": 3, "tgt": 3}
        ],
        "stack_max": 0,
        "uses_legacy_fast_path": False,
    }
    meta, data = _run_sweep({**base, "param_program": native_transform_payload}, "/tmp/pp_param_dump_v2_native_transform.bin")
    assert meta["param_program_tokens"] == 1
    assert len(data) == 4 * 4 * 16

    bad_payload = {
        "version": 2,
        "tokens": [{"op": MERGED_OP_NATIVE_TRANSFORM, "registry": "coeff", "fn_index": 2, "src": 3, "tgt": 3}],
        "stack_max": 0,
    }
    proc = _run_sweep_process({**base, "param_program": bad_payload})
    assert proc.returncode != 0
    assert "registry must be param" in proc.stderr


def test_param_dump_dynamic_const_expression_matches_stack_program():
    expr_spec = {
        "mode": "param_dump",
        "n1": 8,
        "n2": 8,
        "param_program": _native_payload([
            ["const", "t1+t2"],
            ["emit", "p1"],
            ["const", "p1*2"],
            ["emit", "p2"],
        ]),
    }
    expr_meta, expr_data = _run_sweep(expr_spec, "/tmp/pp_param_dump_expr.bin")
    ref_spec = {
        "mode": "param_dump",
        "n1": 8,
        "n2": 8,
        "param_program": _native_payload([
            ["const", "t1+t2"],
            ["emit", "p1"],
            ["const", "(t1+t2)*2"],
            ["emit", "p2"],
        ]),
    }
    _ref_meta, ref_data = _run_sweep(ref_spec, "/tmp/pp_param_dump_expr_ref.bin")
    assert expr_meta["param_program_tokens"] == 8
    assert expr_data == ref_data


def test_param_dump_accepts_legacy_nested_scalar_expr_payload():
    old_payload = {
        "version": 1,
        "tokens": [
            {
                "op": PARAM_OP_CONST,
                "n_args": 1,
                "args": [0.0],
                "args_im": [0.0],
                "expr_refs": [0],
            },
            {"op": PARAM_OP_EMIT_P1},
        ],
        "scalar_exprs": [[
            float(EXPR_T1), 0.0, 0.0,
            float(EXPR_T2), 0.0, 0.0,
            float(EXPR_ADD), 0.0, 0.0,
        ]],
        "stack_max": 1,
        "uses_legacy_fast_path": False,
    }
    old_spec = {
        "mode": "param_dump",
        "n1": 8,
        "n2": 8,
        "param_program": old_payload,
    }
    ref_spec = {
        "mode": "param_dump",
        "n1": 8,
        "n2": 8,
        "param_program": _native_payload([["const", "t1+t2"], ["emit", "p1"]]),
    }
    old_meta, old_data = _run_sweep(old_spec, "/tmp/pp_param_dump_old_expr.bin")
    _ref_meta, ref_data = _run_sweep(ref_spec, "/tmp/pp_param_dump_old_expr_ref.bin")
    assert old_meta["param_program_tokens"] == 2
    assert old_data == ref_data


def test_param_dump_dynamic_legacy_arg_runs_native_vm():
    spec = {
        "mode": "param_dump",
        "n1": 8,
        "n2": 8,
        "param_program": _native_payload([
            ["legacy", "rtheta", "both", "both", "abs(p1)+0.5"],
        ]),
    }
    meta, data = _run_sweep(spec, "/tmp/pp_param_dump_dyn_legacy.bin")
    assert meta["param_program_tokens"] == 5
    assert len(data) == 8 * 8 * 16


def test_param_registers_rebind_and_zero_init_natively():
    """r1..r8: mutable scratch registers. r1 = f(r1) rebinding is real
    (not substitution), unwritten registers read as 0, and state resets
    per evaluation (no leakage across grid rows)."""
    from param_program_source import parse_param_program_source

    parsed = parse_param_program_source(
        "r1 = t1 + 1\n"
        "r1 = r1 * r1\n"
        "p1 = r1\n"
        "p2 = r3 + t2\n"   # r3 never written: must read 0
    )
    vm_spec = {
        "mode": "param_dump",
        "n1": 4,
        "n2": 4,
        "param_program": _native_payload(parsed["chain"]),
    }
    meta, vm = _run_sweep(vm_spec, "/tmp/pp_param_registers.bin")
    # param_dump layout: row-major (i1, i2) rows of (p1r, p1i, p2r, p2i);
    # x1 = i1/n1, x2 = i2/n2
    import struct

    values = struct.unpack("<" + "f" * (len(vm) // 4), vm)
    for i1 in range(4):
        for i2 in range(4):
            base = (i1 * 4 + i2) * 4
            x1, x2 = i1 / 4.0, i2 / 4.0
            p1r, p1i, p2r, p2i = values[base:base + 4]
            expect_p1 = (x1 + 1.0) ** 2
            assert abs(p1r - expect_p1) < 1e-5, (i1, i2, p1r, expect_p1)
            assert abs(p1i) < 1e-6
            # r3 unwritten -> 0, so p2 == t2; also proves r1's value from a
            # previous row did not leak into this row's r3/r1 reads
            assert abs(p2r - x2) < 1e-6, (i1, i2, p2r)
            assert abs(p2i) < 1e-6
