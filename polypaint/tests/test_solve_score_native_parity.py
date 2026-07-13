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


    def test_feature_cache_engaging_programs_match_python(self):
        """CR32 F2: programs with >= 2 same-source root-metric slots engage the
        shared feature cache (one masked pair traversal + raw-score memo);
        single-slot programs take the direct path. Both must match the Python
        reference bit-for-bit-ish (same tolerance as every other parity case).
        Root sets are larger here so pair math dominates, and one case carries
        non-finite roots through the shared finite filter."""
        from solve_score_eval import eval_solve_score

        roots = [
            (0.31 * i - 1.4, 0.27 * ((i * 7) % 11) - 1.2) for i in range(12)
        ]
        coeff_roots = [(-1.0, 0.0), (0.5, 0.2), (0.1, -0.6), (0.9, 0.9), (-0.4, 0.7)]
        param_values = [(0.2, -0.3), (0.7, 0.1)]
        dirty_roots = list(roots)
        dirty_roots[3] = (float("inf"), 0.5)
        dirty_roots[8] = (float("nan"), float("nan"))

        cases = [
            # full pair family + duplicate slot, one source (the dense bundle)
            (
                [
                    ["proximity", "slv", "1"],
                    ["crowding", "slv", "1"],
                    ["clusteriness", "slv", "1"],
                    ["nn_variation", "slv", "1"],
                    ["proximity", "slv", "1"],
                    ["weighted_sum", "0.2", "0.2"],
                    ["weighted_sum", "0.7", "0.3"],
                    ["weighted_sum", "0.7", "0.3"],
                    ["weighted_sum", "0.7", "0.3"],
                ],
                {"proximity": (-10, 10), "crowding": (-10, 10),
                 "clusteriness": (-10, 10), "nn_variation": (-10, 10)},
                roots,
            ),
            # proximity-only pair need (masked pass: no crowding, no NN)
            (
                [
                    ["proximity", "slv", "1"],
                    ["proximity", "slv", "1"],
                    ["weighted_sum", "0.5", "0.5"],
                ],
                {"proximity": (-10, 10)},
                roots,
            ),
            # pair + non-pair metric sharing one source (filter + memo reuse)
            (
                [
                    ["crowding", "slv", "1"],
                    ["max_re", "slv", "1"],
                    ["weighted_sum", "0.6", "0.4"],
                ],
                {"crowding": (-10, 10), "max_re": (-10, 10)},
                roots,
            ),
            # mixed sources: solve engages cache, coeff single slot stays direct
            (
                [
                    ["proximity", "slv", "1"],
                    ["nn_variation", "slv", "1"],
                    ["crowding", "cf", "1"],
                    ["weighted_sum", "0.5", "0.5"],
                    ["weighted_sum", "0.5", "0.5"],
                ],
                {"proximity": (-10, 10), "nn_variation": (-10, 10),
                 "crowding": (-10, 10)},
                roots,
            ),
            # non-finite roots through the shared finite filter
            (
                [
                    ["proximity", "slv", "1"],
                    ["crowding", "slv", "1"],
                    ["clusteriness", "slv", "1"],
                    ["weighted_sum", "0.4", "0.3"],
                    ["weighted_sum", "0.8", "0.2"],
                ],
                {"proximity": (-10, 10), "crowding": (-10, 10),
                 "clusteriness": (-10, 10)},
                dirty_roots,
            ),
        ]
        for chain, clips, case_roots in cases:
            compiled, payload = self._compile_case(chain, clips)
            native_metrics, native_outputs = self._native_eval(
                compiled, payload, case_roots,
                coeff_roots=coeff_roots, param_values=param_values,
            )
            py = eval_solve_score(
                compiled, case_roots,
                coeff_roots=coeff_roots, param_values=param_values,
            )
            self._assert_close_list(native_metrics, py.metrics)
            self._assert_close_list(native_outputs, py.outputs)


    def test_family_pass_programs_match_python(self):
        """CR33 F5/F6: >= 2 same-source members of the extrema or radial
        family run through ONE shared pass, and median metrics use bounded
        selection instead of qsort. All must match the Python reference,
        including non-finite roots through the shared filter."""
        from solve_score_eval import eval_solve_score

        roots = [
            (0.31 * i - 1.4, 0.27 * ((i * 7) % 11) - 1.2) for i in range(14)
        ]
        roots[5] = (0.0, 0.0)          # min_mod zero-skip
        dirty = list(roots)
        dirty[2] = (float("inf"), 0.1)
        dirty[9] = (float("nan"), 0.0)
        coeff_roots = [(-1.0, 0.0), (0.5, 0.2), (0.1, -0.6), (0.9, 0.9)]
        param_values = [(0.2, -0.3), (0.7, 0.1)]

        cases = [
            # full extrema family, one source
            ([["max_re", "slv", "1"], ["min_re", "slv", "1"],
              ["max_im", "slv", "1"], ["min_im", "slv", "1"],
              ["weighted_sum", "0.5", "0.5"], ["weighted_sum", "0.5", "0.5"],
              ["weighted_sum", "0.5", "0.5"]],
             {"max_re": (-10, 10), "min_re": (-10, 10),
              "max_im": (-10, 10), "min_im": (-10, 10)}, roots),
            # full radial family (7 members incl. two-pass sd)
            ([["dist_unit_circle", "slv", "1"], ["min_mod", "slv", "1"],
              ["max_mod", "slv", "1"], ["mean_log_mod", "slv", "1"],
              ["sd_log_mod", "slv", "1"], ["inside_unit_fraction", "slv", "1"],
              ["unit_annulus_fraction_01", "slv", "1"],
              ["weighted_sum", "0.5", "0.5"], ["weighted_sum", "0.5", "0.5"],
              ["weighted_sum", "0.5", "0.5"], ["weighted_sum", "0.5", "0.5"],
              ["weighted_sum", "0.5", "0.5"], ["weighted_sum", "0.5", "0.5"]],
             {"dist_unit_circle": (-10, 10), "min_mod": (-10, 10),
              "max_mod": (-10, 10), "mean_log_mod": (-10, 10),
              "sd_log_mod": (-10, 10), "inside_unit_fraction": (0, 1),
              "unit_annulus_fraction_01": (0, 1)}, roots),
            # two radial members only (minimum engagement)
            ([["min_mod", "slv", "1"], ["max_mod", "slv", "1"],
              ["weighted_sum", "0.5", "0.5"]],
             {"min_mod": (-10, 10), "max_mod": (-10, 10)}, roots),
            # radial + extrema together, non-finite roots through the filter
            ([["max_re", "slv", "1"], ["min_im", "slv", "1"],
              ["dist_unit_circle", "slv", "1"], ["mean_log_mod", "slv", "1"],
              ["weighted_sum", "0.5", "0.5"], ["weighted_sum", "0.5", "0.5"],
              ["weighted_sum", "0.5", "0.5"]],
             {"max_re": (-10, 10), "min_im": (-10, 10),
              "dist_unit_circle": (-10, 10), "mean_log_mod": (-10, 10)}, dirty),
            # median metrics (introselect) alongside a family
            ([["real_axis_proximity", "slv", "1"], ["imag_axis_proximity", "slv", "1"],
              ["diagonal_proximity", "slv", "1"], ["outlierness", "slv", "1"],
              ["weighted_sum", "0.5", "0.5"], ["weighted_sum", "0.5", "0.5"],
              ["weighted_sum", "0.5", "0.5"]],
             {"real_axis_proximity": (-10, 10), "imag_axis_proximity": (-10, 10),
              "diagonal_proximity": (-10, 10), "outlierness": (-10, 10)}, roots),
        ]
        for chain, clips, case_roots in cases:
            compiled, payload = self._compile_case(chain, clips)
            native_metrics, native_outputs = self._native_eval(
                compiled, payload, case_roots,
                coeff_roots=coeff_roots, param_values=param_values,
            )
            py = eval_solve_score(
                compiled, case_roots,
                coeff_roots=coeff_roots, param_values=param_values,
            )
            self._assert_close_list(native_metrics, py.metrics)
            self._assert_close_list(native_outputs, py.outputs)


    def test_plan_telemetry_line_is_env_gated(self):
        """CR33 telemetry: PP_PLAN_TELEMETRY=1 makes program parse emit ONE
        structured pp_solve_plan line on stderr (CloudWatch-visible in
        production); without the env, stderr stays clean."""
        import os as _os

        chain = [["proximity", "slv", "1"], ["crowding", "slv", "1"],
                 ["max_re", "slv", "1"],
                 ["weighted_sum", "0.6", "0.4"], ["weighted_sum", "0.7", "0.3"]]
        compiled, payload = self._compile_case(
            chain, {"proximity": (-10, 10), "crowding": (-10, 10), "max_re": (-10, 10)})
        roots = [(0.1 * i, 0.05 * i) for i in range(8)]
        argv = [
            str(self._probe), payload["score_metrics"],
            payload.get("score_sources", ""), payload["score_clip_los"],
            payload["score_clip_his"], payload["score_program"],
            _csv(roots), _csv(None), _csv(None), _csv(None),
        ]
        env = dict(_os.environ)
        env.pop("PP_PLAN_TELEMETRY", None)
        clean = subprocess.run(argv, capture_output=True, text=True, timeout=30, env=env)
        self.assertEqual(clean.returncode, 0, clean.stderr)
        self.assertNotIn("pp_solve_plan", clean.stderr)

        env["PP_PLAN_TELEMETRY"] = "1"
        told = subprocess.run(argv, capture_output=True, text=True, timeout=30, env=env)
        self.assertEqual(told.returncode, 0, told.stderr)
        lines = [l for l in told.stderr.splitlines() if "pp_solve_plan" in l]
        self.assertEqual(len(lines), 1, told.stderr)
        plan = json.loads(lines[0])["pp_solve_plan"]
        self.assertEqual(plan["metric_count"], 3)
        self.assertEqual(plan["uses_lag"], 0)
        self.assertEqual(plan["engage"], [1, 0])       # two pair slots, solve source
        self.assertEqual(plan["pair_min"], [1, 0])     # proximity present
        self.assertEqual(plan["pair_crowd"], [1, 0])
        self.assertEqual(plan["dup_slots"], 0)


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
