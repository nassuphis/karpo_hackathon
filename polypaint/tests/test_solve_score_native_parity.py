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
