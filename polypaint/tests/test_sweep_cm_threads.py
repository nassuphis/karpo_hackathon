import json
import os
import subprocess
import sys
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_CM_LOCAL = "/tmp/polypaint_sweep_cm_test"


def _ensure_cm_binary():
    """Local macOS build of sweep_cm against Accelerate's modern LAPACK
    (the deployed netlib lineage is covered by the ARM64 docker gate)."""
    if sys.platform != "darwin":
        return False
    src = os.path.join(LAMBDA_DIR, "sweep_cm.c")
    hdr = os.path.join(LAMBDA_DIR, "companion_solver.h")
    if os.path.exists(SWEEP_CM_LOCAL):
        built = os.path.getmtime(SWEEP_CM_LOCAL)
        if built >= os.path.getmtime(src) and built >= os.path.getmtime(hdr):
            return True
    proc = subprocess.run(
        ["cc", "-O3", "-pthread", "-DPOLYPAINT_ACCELERATE_NEWLAPACK",
         "-o", SWEEP_CM_LOCAL, src, "-framework", "Accelerate", "-lm"],
        capture_output=True, text=True,
    )
    return proc.returncode == 0


HAVE_CM_BINARY = _ensure_cm_binary()


def _write_fixture(path, rows=600, n_coeffs=31):
    import numpy as np

    rng = np.random.default_rng(31)
    cf = (rng.standard_normal((rows, n_coeffs))
          + 1j * rng.standard_normal((rows, n_coeffs))).astype(np.complex64)
    cf[7] = 0                    # all-zero guard row
    cf[13, 0] = np.inf           # overflow-skip row
    cf[21, :20] = 0              # leading-strip row
    inter = np.empty((rows, n_coeffs * 2), dtype=np.float32)
    inter[:, 0::2] = cf.real
    inter[:, 1::2] = cf.imag
    inter.tofile(path)
    return rows, n_coeffs


def _run(threads, coeffs_path, n_coeffs, out_path):
    spec = {
        "mode": "solve_cm",
        "coeffs_file": coeffs_path,
        "n_coeffs": n_coeffs,
        "n_steps": 0,   # informational; sweep_cm sizes from the file
    }
    if threads is not None:
        spec["n_threads"] = threads
    proc = subprocess.run(
        [SWEEP_CM_LOCAL, out_path],
        input=json.dumps(spec),
        capture_output=True, text=True, timeout=300,
    )
    if proc.returncode != 0:
        raise AssertionError(proc.stderr[:400])
    return json.loads(proc.stdout)


@unittest.skipUnless(HAVE_CM_BINARY, "local LAPACK sweep_cm build unavailable")
class TestSweepCmThreads(unittest.TestCase):
    def test_threaded_output_is_byte_identical_and_meta_reports_threads(self):
        """The CM threading contract: static row partitioning with per-thread
        workspaces produces BYTE-IDENTICAL output at every thread count
        (same per-row arithmetic, same order, disjoint output slices), and
        the meta reports the thread count like sweep_mt does. Also pins the
        default (no n_threads field -> 1) and the guard rows (all-zero,
        overflow-skip, leading-strip) surviving the partition boundaries."""
        coeffs = "/tmp/sweep_cm_threads_fixture.bin"
        rows, n_coeffs = _write_fixture(coeffs)

        meta_default = _run(None, coeffs, n_coeffs, "/tmp/cm_t_default.bin")
        meta1 = _run(1, coeffs, n_coeffs, "/tmp/cm_t1.bin")
        meta4 = _run(4, coeffs, n_coeffs, "/tmp/cm_t4.bin")

        with open("/tmp/cm_t_default.bin", "rb") as fh:
            out_default = fh.read()
        with open("/tmp/cm_t1.bin", "rb") as fh:
            out1 = fh.read()
        with open("/tmp/cm_t4.bin", "rb") as fh:
            out4 = fh.read()

        self.assertEqual(len(out1), rows * (n_coeffs - 1) * 2 * 4)
        self.assertEqual(out_default, out1)
        self.assertEqual(out1, out4)

        self.assertEqual(meta_default["n_threads"], 1)
        self.assertEqual(meta1["n_threads"], 1)
        self.assertEqual(meta4["n_threads"], 4)
        for meta in (meta_default, meta1, meta4):
            self.assertEqual(meta["mode"], "solve_cm")
            self.assertEqual(meta["n_t"], rows)
            self.assertEqual(meta["degree"], n_coeffs - 1)
            self.assertEqual(meta["skipped_overflow"], 1)

    def test_thread_count_clamps_to_rows(self):
        """More threads than rows must not spawn empty workers or break the
        partition: 3 rows at n_threads=16 -> meta reports 3."""
        import numpy as np

        coeffs = "/tmp/sweep_cm_threads_tiny.bin"
        rng = np.random.default_rng(3)
        cf = (rng.standard_normal((3, 5))
              + 1j * rng.standard_normal((3, 5))).astype(np.complex64)
        inter = np.empty((3, 10), dtype=np.float32)
        inter[:, 0::2] = cf.real
        inter[:, 1::2] = cf.imag
        inter.tofile(coeffs)
        meta = _run(16, coeffs, 5, "/tmp/cm_t_tiny.bin")
        self.assertEqual(meta["n_threads"], 3)
        self.assertEqual(meta["n_t"], 3)


if __name__ == "__main__":
    unittest.main()
