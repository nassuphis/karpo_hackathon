"""CR32 F10: the -DPP_VM_PERF counters must reflect every direct pread/pwrite
site — chunked coeff at EVERY worker count (t1 included, via the unified block
engine) and the param static scheduler. Serial/streamed param_gen uses stdio
buffering, which is intentionally uncounted; the report fields are named
direct_* for that reason and a zero there is pinned.
"""
import json
import math
import os
import pathlib
import shutil
import subprocess
import sys

import pytest

ROOT = pathlib.Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"

BLOCK_ROWS = 128
N = 64  # 64x64 grid -> 4096 steps


@pytest.fixture(scope="module")
def perf_binary(tmp_path_factory):
    cc = os.environ.get("CC") or shutil.which("cc") or shutil.which("clang") or shutil.which("gcc")
    if not cc:
        pytest.skip("no C compiler available")
    out = tmp_path_factory.mktemp("perf_bin") / "sweep_perf"
    result = subprocess.run(
        [cc, "-O2", "-pthread", "-DPP_VM_PERF", "-I", str(LAMBDA_DIR),
         str(LAMBDA_DIR / "sweep_cli.c"), "-lm", "-o", str(out)],
        capture_output=True, text=True, timeout=240,
    )
    assert result.returncode == 0, f"PP_VM_PERF compile failed: {result.stderr}"
    return out


def _run(binary, payload, out_path):
    proc = subprocess.run(
        [str(binary), str(out_path)], input=json.dumps(payload),
        capture_output=True, text=True, timeout=120,
    )
    assert proc.returncode == 0, proc.stderr
    for line in proc.stderr.splitlines():
        if "pp_vm_perf" in line:
            return json.loads(line)["pp_vm_perf"]
    raise AssertionError(f"no pp_vm_perf report in stderr: {proc.stderr}")


@pytest.fixture(scope="module")
def params_file(perf_binary, tmp_path_factory):
    wd = tmp_path_factory.mktemp("perf_data")
    path = wd / "params.bin"
    _run(perf_binary, {"mode": "param_gen", "n1": N, "n2": N, "times": 1,
                       "n_threads": 4}, path)
    return path


def _chunked(params_file, threads):
    return {"mode": "coeffgen_chunked", "function": "poly_1", "cfpv": [],
            "params_file": str(params_file), "step_start": 0,
            "step_count": N * N, "source_step_start": 0,
            "source_n1": N, "source_n2": N, "n_threads": threads}


def test_chunked_t1_uses_blocked_io(perf_binary, params_file, tmp_path):
    """4096 rows / 128-row blocks = 32 block preads + 1 probe pread and
    32 pwrites at ONE worker — the CR32 F3 pin that t1 actually enters the
    block engine (every direct pread is counted, probe included)."""
    perf = _run(perf_binary, _chunked(params_file, 1), tmp_path / "t1.bin")
    expected = math.ceil((N * N) / BLOCK_ROWS)
    assert perf["direct_pread_calls"] == expected + 1
    assert perf["direct_pwrite_calls"] == expected


def test_chunked_t4_block_totals(perf_binary, params_file, tmp_path):
    """Four workers, contiguous ranges of 1024 rows -> 8 blocks each,
    plus the single probe pread."""
    perf = _run(perf_binary, _chunked(params_file, 4), tmp_path / "t4.bin")
    expected = 4 * math.ceil((N * N) / 4 / BLOCK_ROWS)
    assert perf["direct_pread_calls"] == expected + 1
    assert perf["direct_pwrite_calls"] == expected


def test_param_static_writes_are_batched_and_counted(perf_binary, tmp_path):
    """64 rows over 4 workers = 16 rows each -> one block flush per worker."""
    perf = _run(perf_binary, {"mode": "param_gen", "n1": N, "n2": N,
                              "times": 1, "n_threads": 4}, tmp_path / "p4.bin")
    assert perf["mode"] == "param_gen"
    assert perf["direct_pread_calls"] == 0
    assert perf["direct_pwrite_calls"] == 4


def test_param_serial_reports_zero_direct_io(perf_binary, tmp_path):
    """Serial param_gen writes through stdio: direct counters stay zero by
    design (that's why the fields are named direct_*)."""
    perf = _run(perf_binary, {"mode": "param_gen", "n1": N, "n2": N,
                              "times": 1, "n_threads": 1}, tmp_path / "p1.bin")
    assert perf["direct_pread_calls"] == 0
    assert perf["direct_pwrite_calls"] == 0
