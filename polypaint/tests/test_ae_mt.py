import json
import pathlib
import shutil
import struct
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parent.parent
AE_SRC = ROOT / "lambda" / "sweep_cli.c"
MT_SRC = ROOT / "lambda" / "sweep_mt.c"
AE_BIN = pathlib.Path("/tmp/sweep_ae_compare_test")
MT_BIN = pathlib.Path("/tmp/sweep_mt_compare_test")


def _compile():
    if not shutil.which("cc"):
        return False, "cc not found"

    ae = subprocess.run(
        ["cc", "-O3", "-pthread", str(AE_SRC), "-lm", "-o", str(AE_BIN)],
        capture_output=True,
        text=True,
    )
    if ae.returncode != 0:
        return False, ae.stderr.strip() or ae.stdout.strip()

    mt = subprocess.run(
        ["cc", "-O3", str(MT_SRC), "-lm", "-pthread", "-o", str(MT_BIN)],
        capture_output=True,
        text=True,
    )
    if mt.returncode != 0:
        return False, mt.stderr.strip() or mt.stdout.strip()

    return True, ""


def _write_coeffs(path, coeffs_list, n_coeffs):
    with open(path, "wb") as fh:
        for coeffs in coeffs_list:
            padded = list(coeffs) + [0.0] * (n_coeffs - len(coeffs))
            for c in padded[:n_coeffs]:
                re = c.real if hasattr(c, "real") else float(c)
                im = c.imag if hasattr(c, "imag") else 0.0
                fh.write(struct.pack("<ff", re, im))


class TestAEMTNative(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        ok, msg = _compile()
        if not ok:
            raise unittest.SkipTest(f"could not compile AE/AE-MT test binaries: {msg}")

    def test_single_thread_mt_is_byte_exact_with_single_thread_ae(self):
        coeffs = [
            [1, -6, 11, -6],
            [1, -10, 35, -50, 24],
            [1, -2, 1],
            [1, 0, 1],
            [0, 1, -3, 2],
            [1, 0, 0, 0, -1],
        ]
        n_coeffs = 5
        with tempfile.TemporaryDirectory() as td:
            td = pathlib.Path(td)
            coeffs_path = td / "coeffs.bin"
            ae_out = td / "ae.bin"
            mt_out = td / "ae_mt.bin"
            _write_coeffs(coeffs_path, coeffs, n_coeffs)

            ae_spec = {
                "mode": "solve",
                "coeffs_file": str(coeffs_path),
                "n_coeffs": n_coeffs,
                "n2": len(coeffs),
                "i1_start": 0,
                "i1_end": 1,
                "match_roots": False,
            }
            mt_spec = {
                "mode": "solve_mt",
                "coeffs_file": str(coeffs_path),
                "n_coeffs": n_coeffs,
                "n2": len(coeffs),
                "i1_start": 0,
                "i1_end": 1,
                "match_roots": False,
                "n_threads": 1,
            }

            ae = subprocess.run(
                [str(AE_BIN), str(ae_out)],
                input=json.dumps(ae_spec),
                capture_output=True,
                text=True,
                timeout=30,
            )
            mt = subprocess.run(
                [str(MT_BIN), str(mt_out)],
                input=json.dumps(mt_spec),
                capture_output=True,
                text=True,
                timeout=30,
            )
            self.assertEqual(ae.returncode, 0, ae.stderr)
            self.assertEqual(mt.returncode, 0, mt.stderr)

            ae_meta = json.loads(ae.stdout)
            mt_meta = json.loads(mt.stdout)
            self.assertEqual(ae_meta["n_t"], mt_meta["n_t"])
            self.assertEqual(ae_meta["degree"], mt_meta["degree"])
            self.assertEqual(ae_meta["avg_iterations"], mt_meta["avg_iterations"])
            self.assertEqual(mt_meta["n_threads"], 1)
            self.assertEqual(ae_out.read_bytes(), mt_out.read_bytes())

    def test_single_thread_ae_warm_start_reduces_iterations_vs_multi_thread_blocks(self):
        coeffs = [[1, -6, 11, -6] for _ in range(16)]
        n_coeffs = 4
        with tempfile.TemporaryDirectory() as td:
            td = pathlib.Path(td)
            coeffs_path = td / "coeffs.bin"
            ae_out = td / "ae.bin"
            mt_out = td / "ae_mt.bin"
            _write_coeffs(coeffs_path, coeffs, n_coeffs)

            ae_spec = {
                "mode": "solve",
                "coeffs_file": str(coeffs_path),
                "n_coeffs": n_coeffs,
                "n2": len(coeffs),
                "i1_start": 0,
                "i1_end": 1,
                "match_roots": False,
            }
            mt_spec = {
                "mode": "solve_mt",
                "coeffs_file": str(coeffs_path),
                "n_coeffs": n_coeffs,
                "n2": len(coeffs),
                "i1_start": 0,
                "i1_end": 1,
                "match_roots": False,
                "n_threads": 4,
            }

            ae = subprocess.run(
                [str(AE_BIN), str(ae_out)],
                input=json.dumps(ae_spec),
                capture_output=True,
                text=True,
                timeout=30,
            )
            mt = subprocess.run(
                [str(MT_BIN), str(mt_out)],
                input=json.dumps(mt_spec),
                capture_output=True,
                text=True,
                timeout=30,
            )
            self.assertEqual(ae.returncode, 0, ae.stderr)
            self.assertEqual(mt.returncode, 0, mt.stderr)

            ae_meta = json.loads(ae.stdout)
            mt_meta = json.loads(mt.stdout)
            self.assertEqual(ae_meta["n_t"], len(coeffs))
            self.assertEqual(mt_meta["n_t"], len(coeffs))
            self.assertEqual(ae_meta["degree"], mt_meta["degree"])
            self.assertEqual(mt_meta["n_threads"], 4)
            self.assertLess(ae_meta["avg_iterations"], mt_meta["avg_iterations"])

    def test_match_true_is_rejected(self):
        coeffs = [[1, -3, 2]]
        with tempfile.TemporaryDirectory() as td:
            td = pathlib.Path(td)
            coeffs_path = td / "coeffs.bin"
            out = td / "out.bin"
            _write_coeffs(coeffs_path, coeffs, 3)
            spec = {
                "mode": "solve_mt",
                "coeffs_file": str(coeffs_path),
                "n_coeffs": 3,
                "n2": 1,
                "i1_start": 0,
                "i1_end": 1,
                "match_roots": True,
                "n_threads": 2,
            }
            mt = subprocess.run(
                [str(MT_BIN), str(out)],
                input=json.dumps(spec),
                capture_output=True,
                text=True,
                timeout=30,
            )
            self.assertNotEqual(mt.returncode, 0)
            self.assertIn("match_roots=false", mt.stderr)


if __name__ == "__main__":
    unittest.main()
