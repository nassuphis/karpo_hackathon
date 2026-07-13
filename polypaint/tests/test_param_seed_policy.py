"""CR32 F4: the param_gen dither seed policy may depend only on the REQUEST,
never on host CPU topology.

Contract (pre-existing seam, now pinned):
  - full requests with n_threads == 1 use the serial pass-seeded stream;
  - every other path (full n_threads >= 2, all range requests) seeds per row;
  - the CPU cap (sysconf, overridable via PP_TEST_ONLINE_CPUS) may reduce the
    worker count but must NEVER flip a request across the serial/threaded
    seed boundary — a multi-worker request on a 1-CPU host stays row-seeded.
"""
import hashlib
import json
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))
sys.path.insert(0, os.path.dirname(__file__))

from native_program_helpers import translate_legacy_transforms_for_native

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")

N1 = N2 = 16
DITHER = [["sdith", "1"]]


def _run_param_gen_stream(n_threads, step_start=None, step_count=None):
    """Invoke exactly like the pre-CR33 fused handler: argv "-", stdout to a
    regular file. Selects the ordered-ring scheduler."""
    payload = translate_legacy_transforms_for_native(
        {"mode": "param_gen", "n1": N1, "n2": N2, "times": 1,
         "n_threads": n_threads, "param_transforms": DITHER})
    if step_start is not None:
        payload["step_start"] = step_start
        payload["step_count"] = step_count
    with tempfile.NamedTemporaryFile(prefix="pp_seed_stream_", suffix=".bin", delete=False) as fh:
        out_path = fh.name
    try:
        with open(out_path, "wb") as out:
            proc = subprocess.run(
                [SWEEP, "-"], input=json.dumps(payload).encode(),
                stdout=out, stderr=subprocess.PIPE, timeout=60,
            )
        assert proc.returncode == 0, proc.stderr.decode()
        with open(out_path, "rb") as fh:
            return hashlib.sha256(fh.read()).hexdigest()
    finally:
        try:
            os.remove(out_path)
        except FileNotFoundError:
            pass


def _run_param_gen(n_threads, step_start=None, step_count=None, env_cpus=None):
    payload = translate_legacy_transforms_for_native(
        {"mode": "param_gen", "n1": N1, "n2": N2, "times": 1,
         "n_threads": n_threads, "param_transforms": DITHER})
    if step_start is not None:
        payload["step_start"] = step_start
        payload["step_count"] = step_count
    env = dict(os.environ)
    env.pop("PP_TEST_ONLINE_CPUS", None)
    if env_cpus is not None:
        env["PP_TEST_ONLINE_CPUS"] = str(env_cpus)
    with tempfile.NamedTemporaryFile(prefix="pp_seed_policy_", suffix=".bin", delete=False) as fh:
        out_path = fh.name
    try:
        proc = subprocess.run(
            [SWEEP, out_path], input=json.dumps(payload),
            capture_output=True, text=True, timeout=60, env=env,
        )
        assert proc.returncode == 0, proc.stderr
        with open(out_path, "rb") as fh:
            return hashlib.sha256(fh.read()).hexdigest()
    finally:
        try:
            os.remove(out_path)
        except FileNotFoundError:
            pass


def test_full_threaded_counts_agree():
    """t2 == t4 == t8 for full dithered requests (row seeding)."""
    h2 = _run_param_gen(2)
    h4 = _run_param_gen(4)
    h8 = _run_param_gen(8)
    assert h2 == h4 == h8


def test_cpu_cap_does_not_flip_full_request_to_serial():
    """The F4 regression: threads=4 on a 1-CPU host must produce the same
    bytes as threads=4 on a many-CPU host."""
    unrestricted = _run_param_gen(4)
    capped = _run_param_gen(4, env_cpus=1)
    assert capped == unrestricted


def test_cpu_cap_reduces_but_preserves_bytes_at_two_cpus():
    unrestricted = _run_param_gen(8)
    capped = _run_param_gen(8, env_cpus=2)
    assert capped == unrestricted


def test_serial_full_request_unaffected_by_cap():
    """t1 keeps the historical pass-seeded stream regardless of topology."""
    plain = _run_param_gen(1)
    capped = _run_param_gen(1, env_cpus=1)
    many = _run_param_gen(1, env_cpus=64)
    assert plain == capped == many


def test_full_serial_differs_from_threaded_documented_seam():
    """Pre-existing seam, pinned intentionally: full serial (pass-seeded)
    differs from full threaded (row-seeded). If this test ever fails because
    the hashes became EQUAL, the seed policy was unified — update the docs
    and this pin together."""
    assert _run_param_gen(1) != _run_param_gen(4)


def test_range_requests_are_row_seeded_at_every_thread_count():
    """Range requests use computeParamGenRow everywhere, including capped-to-
    serial, so bytes match across thread counts and topologies."""
    total = N1 * N2
    r1 = _run_param_gen(1, step_start=0, step_count=total)
    r4 = _run_param_gen(4, step_start=0, step_count=total)
    r4_capped = _run_param_gen(4, step_start=0, step_count=total, env_cpus=1)
    assert r1 == r4 == r4_capped


def test_partial_range_bytes_stable_across_threads():
    r1 = _run_param_gen(1, step_start=24, step_count=100)
    r4 = _run_param_gen(4, step_start=24, step_count=100)
    r8 = _run_param_gen(8, step_start=24, step_count=100)
    assert r1 == r4 == r8


def test_stream_and_file_modes_are_byte_identical():
    """CR33 F1: the fused handler switched from "-" (ordered ring) to the
    real output path (static pwrite scheduler). Both threaded modes seed per
    row, so the swap must be byte-neutral — pinned here at every worker
    count, full and non-row-aligned range requests."""
    for threads in (2, 4, 8):
        assert _run_param_gen(threads) == _run_param_gen_stream(threads), threads
    total = N1 * N2
    for threads in (2, 4):
        assert (_run_param_gen(threads, step_start=24, step_count=100)
                == _run_param_gen_stream(threads, step_start=24, step_count=100)), threads
        assert (_run_param_gen(threads, step_start=0, step_count=total)
                == _run_param_gen_stream(threads, step_start=0, step_count=total)), threads
