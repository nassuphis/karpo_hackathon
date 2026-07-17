"""
giga_cf10p450 parity: the deterministic port of xfrm
'unit_circle,coeff10' -> poly_450 -> zfrm rev, plus the one migration
addition — a 1e-30 rescale that rescues the rows from f32 coefficient
transport (the raw cumulative ladder exceeds 3.4e38 on ~100% of rows,
which is why the equivalent UI chain renders nothing).

Program shape under test: the self-carrying [P6 | payload] accumulator
whose gated arg-position scans read the header via tos (the parked
left operand of multiply(poly, scan(...)) IS tos during the arg-scan
— the giga_2897 trap used as the tool), fitting the 256-chip and
64-token-per-expr caps simultaneously.

Measured at authoring (5 probes): coefficient relative parity
<= 5.7e-15 vs the exact oracle, the |z|<1 art cluster carries 34 of
35 roots with positional match <= 9.5e-5, raw (unscaled) row maxima
8e40..9.8e57 — every probe would overflow f32 without the rescale.
"""
import importlib.util
import json
import os
import subprocess
import sys
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")
sys.path.insert(0, LAMBDA_DIR)

PROBES = ((0.137, 0.823), (0.75, 0.31), (0.31, 0.77), (0.9, 0.9), (0.05, 0.6))


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_cf10p450_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_cf10p450_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _oracle_coeffs(u, v):
    """Exact mirror of the VM program (identity form of the base,
    repeated-product conj powers, cumprod ladder, rev, 1e-30)."""
    import numpy as np

    z1p, z2 = np.exp(25.132741228718345j * u), np.exp(6.283185307179586j * v)
    z2p, z1 = np.exp(25.132741228718345j * v), np.exp(6.283185307179586j * u)
    v1 = (z1p - z2) / (z1p + z2)
    v2 = (z2p - z1) / (z2p + z1)
    A1, A2 = np.angle(v1), np.angle(v2)
    M = abs(v1) + abs(v2)
    j = np.arange(1, 36)
    S = np.sin(j * A1 + np.cos(j * A2))
    P = A1 * np.cos(j * 0.08975979010256552) - A2 * np.sin(j * 0.08975979010256552)
    MAG = np.log(M + j) * (1 + 0.1 * j)
    base = np.exp(1j * S) * np.cos(P) * MAG + np.exp(-1j * S) * np.sin(P) * (-1j) * MAG
    c1, c2 = np.conj(v1), np.conj(v2)
    base[34] = base[34] * c1 * c1 + c2
    base[33] = base[33] * c1 * c1 * c1 + c2 * c2
    base[32] = base[32] * c1 + c2 * c2 * c2
    base[31] = base[31] * c1 * c1 + 1
    base[30] = base[30] * c1 * c1 * c1 + c2
    cf = base + np.cumprod(M + j) / (j + 1)
    return cf[::-1] * 1e-30


def _run_vm(compiled, t1, t2):
    payload = {
        "version": 1,
        "fingerprint": compiled["fingerprint"],
        "tokens": compiled["tokens"],
        "stack_max": compiled["stack_max"],
        "scalar_exprs": compiled["scalar_exprs"],
        "vector_constants": compiled["vector_constants"],
    }
    proc = subprocess.run(
        [SWEEP_TEST, "/tmp/giga_cf10p450_test_row.bin"],
        input=json.dumps({
            "mode": "compute_debug",
            "function": "const",
            "cfpv": [1, 0, 0],
            "u": t1,
            "v": t2,
            "grid_n": 1000,
            "coeff_transforms": [],
            "coeff_program": payload,
        }),
        capture_output=True, text=True, timeout=120,
    )
    if proc.returncode != 0:
        raise AssertionError(proc.stderr[:300])
    meta = json.loads(proc.stdout)
    return [complex(re, im) for re, im in meta["coeff"]["poly"]]


class TestGigaCf10p450Document(unittest.TestCase):
    def test_generator_document_is_fresh_and_compiles(self):
        from coeff_program_source import compile_coeff_program_source

        proc = subprocess.run(
            [sys.executable, os.path.join(ROOT, "scripts", "gen_giga_cf10p450_coeff_program.py"), "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

        generator = _load_generator()
        stored = json.load(open(os.path.join(ROOT, "giga_cf10p450.coeff-program.json")))
        self.assertEqual(stored["name"], "giga_cf10p450")
        self.assertEqual(stored["source_text"], generator.build_source_text())
        compiled = compile_coeff_program_source(stored["source_text"])
        errors = [d for d in (compiled.get("diagnostics") or []) if d.get("level") == "error"]
        self.assertFalse(errors)
        self.assertLessEqual(compiled["token_count"], 256)
        # the migration's one addition: the transport rescue
        self.assertIn("1e-30", stored["source_text"])
        self.assertIn("poly = rev(poly)", stored["source_text"])


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestGigaCf10p450VmParity(unittest.TestCase):
    def test_vm_rows_match_oracle_and_stay_inside_f32(self):
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for u, v in PROBES:
            with self.subTest(u=u, v=v):
                got = np.array(_run_vm(compiled, u, v))
                ref = _oracle_coeffs(u, v)
                self.assertEqual(len(got), 35)
                rel = float(np.abs(got - ref).max() / np.abs(ref).max())
                self.assertLess(rel, 1e-12)
                # the art: the |z|<1 root cluster (the +/-0.15 view)
                rg = sorted([z for z in np.roots(got) if abs(z) < 1],
                            key=lambda z: (z.real, z.imag))
                rr = sorted([z for z in np.roots(ref) if abs(z) < 1],
                            key=lambda z: (z.real, z.imag))
                self.assertEqual(len(rg), len(rr))
                self.assertEqual(len(rg), 34)
                self.assertLess(max(abs(a - b) for a, b in zip(rg, rr)), 1e-2)
                # scaled rows sit inside f32 transport; unscaled they
                # would overflow — the whole reason the UI chain drew
                # nothing and this program exists
                self.assertLess(float(np.abs(got).max()), 3.4e38)
                self.assertGreater(float(np.abs(got).max()) * 1e30, 3.4e38)


if __name__ == "__main__":
    unittest.main()
