import json
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"
PROBE_SRC = ROOT / "tests" / "solve_score_native_probe.c"
sys.path.insert(0, str(LAMBDA_DIR))


def _csv(values):
    if values is None:
        return "-"
    flat = []
    for value in values:
        if isinstance(value, complex):
            flat.extend([value.real, value.imag])
        elif isinstance(value, (tuple, list)):
            flat.extend(value)
        else:
            flat.append(value)
    return ",".join(f"{float(v):.17g}" for v in flat)


class TestSolveScoreNativeParity(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls._tmpdir_obj = tempfile.TemporaryDirectory()
        cls._workdir = pathlib.Path(cls._tmpdir_obj.name)
        cc = shutil.which("cc") or shutil.which("clang") or shutil.which("gcc")
        if not cc:
            raise unittest.SkipTest("no C compiler available")
        cls._probe = cls._workdir / "solve_score_native_probe"
        cmd = [
            cc,
            "-O2",
            "-I",
            str(LAMBDA_DIR),
            str(PROBE_SRC),
            "-lm",
            "-o",
            str(cls._probe),
        ]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
        if result.returncode != 0:
            raise AssertionError(f"failed to compile solve_score native probe: {result.stderr}")

    @classmethod
    def tearDownClass(cls):
        cls._tmpdir_obj.cleanup()

    def _compile_case(self, chain, clips):
        from solve_score_chain import compile_solve_score_chain, solve_score_program_cli_payload

        compiled = compile_solve_score_chain(chain)
        metrics = []
        for metric in compiled["metrics"]:
            lo, hi = clips.get(metric["metric"], (-20.0, 20.0))
            metrics.append({**metric, "clip_lo": lo, "clip_hi": hi})
        compiled = {**compiled, "metrics": metrics}
        return compiled, solve_score_program_cli_payload(compiled)

    def _native_eval(self, compiled, payload, roots, coeff_roots=None, param_values=None, recent_metrics=None):
        result = subprocess.run(
            [
                str(self._probe),
                payload["score_metrics"],
                payload.get("score_sources", ""),
                payload["score_clip_los"],
                payload["score_clip_his"],
                payload["score_program"],
                _csv(roots),
                _csv(coeff_roots),
                _csv(param_values),
                _csv(recent_metrics),
            ],
            capture_output=True,
            text=True,
            timeout=30,
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        data = json.loads(result.stdout)
        metrics = [float(v) for v in data["metrics"].split(",") if v != ""]
        outputs = [float(v) for v in data["outputs"].split(",") if v != ""]
        self.assertEqual(int(data["output_count"]), len(outputs))
        self.assertEqual(len(metrics), len(compiled["metrics"]))
        return metrics, outputs

    def _assert_close_list(self, actual, expected, *, tol=2e-6):
        self.assertEqual(len(actual), len(expected))
        for a, e in zip(actual, expected):
            self.assertAlmostEqual(a, e, delta=tol)

    def test_every_metric_matches_native_normalized_slot_eval(self):
        from solve_score_chain import PARAM_SOLVE_SCORE_METRICS, VALID_SOLVE_SCORE_METRICS
        from solve_score_eval import eval_solve_score

        roots = [(0.0, 0.0), (1.0, 0.0), (0.25, 0.75), (-0.5, 0.4)]
        coeff_roots = [(-1.0, 0.0), (0.5, 0.2), (0.1, -0.6)]
        param_values = [(0.2, -0.3), (0.7, 0.1)]
        for metric in sorted(VALID_SOLVE_SCORE_METRICS):
            source = "pm" if metric in PARAM_SOLVE_SCORE_METRICS else "slv"
            chain = [[metric, source, "1"]]
            compiled, payload = self._compile_case(chain, {metric: (-20.0, 20.0)})
            native_metrics, native_outputs = self._native_eval(
                compiled,
                payload,
                roots,
                coeff_roots=coeff_roots,
                param_values=param_values,
            )
            py = eval_solve_score(
                compiled,
                roots,
                coeff_roots=coeff_roots,
                param_values=param_values,
            )
            self._assert_close_list(native_metrics, py.metrics)
            self._assert_close_list(native_outputs, py.outputs)

    def test_program_vm_outputs_match_native_for_sources_emit_and_lag(self):
        from solve_score_eval import eval_solve_score

        roots = [(0.0, 0.0), (1.0, 0.0), (0.25, 0.75), (-0.5, 0.4)]
        coeff_roots = [(-1.0, 0.0), (0.5, 0.2), (0.1, -0.6)]
        param_values = [(0.2, -0.3), (0.7, 0.1)]
        cases = [
            (
                [
                    ["crowding", "slv", "1"],
                    ["spread", "cf", "2"],
                    ["weighted_sum", "0.7", "0.3"],
                    ["omega_cosine", "3", "0.25"],
                ],
                {"crowding": (-10, 10), "spread": (-10, 10)},
                None,
            ),
            (
                [
                    ["crowding", "slv", "1"],
                    ["dup"],
                    ["sin"],
                    ["emit", "raw"],
                    ["cos"],
                    ["emit", "raw"],
                ],
                {"crowding": (-10, 10)},
                None,
            ),
            (
                [
                    ["crowding", "slv", "1"],
                    ["crowding", "slv-1", "1"],
                    ["subtract"],
                ],
                {"crowding": (-10, 10)},
                [0.25],
            ),
            (
                [
                    ["max_re", "pm", "1"],
                    ["t1_abs", "pm", "1"],
                    ["ratio"],
                    ["clamp"],
                ],
                {"max_re": (-2, 2), "t1_abs": (0, 2)},
                None,
            ),
        ]
        for chain, clips, recent in cases:
            compiled, payload = self._compile_case(chain, clips)
            native_metrics, native_outputs = self._native_eval(
                compiled,
                payload,
                roots,
                coeff_roots=coeff_roots,
                param_values=param_values,
                recent_metrics=recent,
            )
            py = eval_solve_score(
                compiled,
                roots,
                coeff_roots=coeff_roots,
                param_values=param_values,
                recent_metrics=recent,
            )
            self._assert_close_list(native_metrics, py.metrics)
            self._assert_close_list(native_outputs, py.outputs)


class TestSolveScoreCPartitionDrift(unittest.TestCase):
    def test_c_param_metric_partition_matches_python(self):
        """solve_metric_is_param_metric (C) vs PARAM_SOLVE_SCORE_METRICS.

        The C executor re-decides param-vs-root dispatch at runtime from its
        own hardcoded case list; the value-parity loop derives its source
        choice from the Python set, so a disagreement on the partition itself
        was previously undetectable.
        """
        import re

        import solve_score_chain as chain

        header = (LAMBDA_DIR / "solve_score.h").read_text()
        match = re.search(
            r"static int solve_metric_is_param_metric\([^)]*\)\s*\{(.*?)\n\}",
            header,
            re.S,
        )
        self.assertIsNotNone(match, "solve_metric_is_param_metric not found in solve_score.h")
        cases = re.findall(r"case\s+SOLVE_METRIC_([A-Z0-9_]+)\s*:", match.group(1))
        c_partition = {name.lower() for name in cases}
        self.assertEqual(
            c_partition,
            set(chain.PARAM_SOLVE_SCORE_METRICS),
            "C param-metric partition drifted from Python PARAM_SOLVE_SCORE_METRICS",
        )

    def test_c_min_roots_table_matches_pinned_expectations(self):
        """solve_metric_min_roots (C) vs a test-side pinned table.

        The switch decides how many finite roots a metric needs before the
        C executor scores it; a metric added to the enum but not the switch
        silently inherits default:2. This pin forces every change to the
        switch (or a new metric's intended minimum) to be declared here.
        """
        import re

        import solve_score_chain as chain

        header = (LAMBDA_DIR / "solve_score.h").read_text()
        match = re.search(
            r"static int solve_metric_min_roots\([^)]*\)\s*\{(.*?)\n\}",
            header,
            re.S,
        )
        self.assertIsNotNone(match, "solve_metric_min_roots not found in solve_score.h")
        body = match.group(1)

        parsed = {}
        pending = []
        for kind, value in re.findall(
            r"case\s+SOLVE_METRIC_([A-Z0-9_]+)\s*:|return\s+(\d+)\s*;", body
        ):
            if kind:
                pending.append(kind.lower())
            else:
                for name in pending:
                    parsed[name] = int(value)
                pending = []
        default_match = re.search(r"default\s*:\s*return\s+(\d+)\s*;", body)
        self.assertIsNotNone(default_match, "min_roots default return not found")
        default_value = int(default_match.group(1))

        expected_explicit = {
            "centroid_re": 1,
            "centroid_im": 1,
            "centroid_dist": 1,
            "dist_unit_circle": 1,
            "asymmetry_re": 1,
            "max_re": 1,
            "min_re": 1,
            "max_im": 1,
            "min_im": 1,
            "min_mod": 1,
            "max_mod": 1,
            "mean_log_mod": 1,
            "inside_unit_fraction": 1,
            "unit_annulus_fraction_01": 1,
            "imag_axis_proximity": 1,
            "diagonal_proximity": 1,
            "sector_max_share_16": 1,
            "area": 3,
            "min_angular_separation": 2,
            "sd_log_mod": 2,
            "angular_entropy_16": 2,
            "angular_order_2": 2,
            "angular_order_3": 2,
            "angular_order_4": 2,
        }
        self.assertEqual(parsed, expected_explicit, "C min_roots switch drifted from pinned table")
        self.assertEqual(default_value, 2, "C min_roots default drifted")
        # No stale cases for metrics Python no longer knows about.
        self.assertLessEqual(
            set(parsed),
            set(chain.VALID_SOLVE_SCORE_METRICS),
            "C min_roots switch names metrics missing from VALID_SOLVE_SCORE_METRICS",
        )
