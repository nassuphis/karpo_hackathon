import json
import pathlib
import sys
import tempfile
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

from oracle_runner import FIXTURE_DIR, compile_sweep_binary, load_corpus, run_corpus


class TestWholeSweepOracle(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls._tmpdir_obj = tempfile.TemporaryDirectory()
        cls._workdir = pathlib.Path(cls._tmpdir_obj.name)
        cls._binary = compile_sweep_binary(cls._workdir / "sweep_oracle")

    @classmethod
    def tearDownClass(cls):
        cls._tmpdir_obj.cleanup()

    def _baseline(self, name):
        path = FIXTURE_DIR / "baseline" / f"{name}.sha256.json"
        with path.open("r", encoding="utf-8") as fh:
            return json.load(fh)

    def test_whole_sweep_coeffgen_and_roots_match_checked_in_sha_manifests(self):
        cases = load_corpus()
        results = run_corpus(self._binary, cases=cases, workdir=self._workdir / "runs")
        for case in cases:
            name = case["name"]
            expected = self._baseline(name)
            actual = results[name]
            self.assertEqual(actual["coeffs_sha256"], expected["coeffs_sha256"], name)
            self.assertEqual(actual["roots_sha256"], expected["roots_sha256"], name)
            self.assertEqual(actual["coeffs_bytes"], expected["coeffs_bytes"], name)
            self.assertEqual(actual["roots_bytes"], expected["roots_bytes"], name)
            self.assertEqual(actual["coeffgen_meta"]["n_coeffs"], expected["n_coeffs"], name)
            self.assertEqual(actual["solve_meta"]["degree"], expected["degree"], name)
