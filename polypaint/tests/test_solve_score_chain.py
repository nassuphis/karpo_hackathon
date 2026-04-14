import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class TestSolveScoreChain(unittest.TestCase):

    def test_compile_chain_accepts_metric_and_transfer(self):
        from solve_score_chain import compile_solve_score_chain

        compiled = compile_solve_score_chain([["crowding", "1"], ["omega_cosine", "4"]])

        self.assertEqual(
            compiled["chain"],
            [
                {"name": "crowding", "params": ["slv", "1"]},
                {"name": "omega_cosine", "params": ["4"]},
            ],
        )
        self.assertEqual(compiled["metric"], "crowding")
        self.assertEqual(compiled["quantile"], 0.01)
        self.assertEqual(compiled["omega"], 4.0)
        self.assertEqual(compiled["omega_phase"], 0.0)
        self.assertTrue(compiled["omega_enabled"])
        self.assertEqual(compiled["program_spec"], "m0;omega_cosine:4")
        self.assertEqual(compiled["metrics"][0]["source"], "slv")

    def test_compile_chain_accepts_rpn_metric_combination_and_transfer(self):
        from solve_score_chain import compile_solve_score_chain, solve_score_program_cli_payload

        compiled = compile_solve_score_chain(
            [
                ["spread", "2"],
                ["shelliness", "3"],
                ["weighted_sum", "0.7", "0.3"],
                ["omega_cosine", "5"],
            ]
        )

        self.assertEqual(compiled["metric_count"], 2)
        self.assertFalse(compiled["legacy_compatible"])
        self.assertEqual(compiled["program_spec"], "m0;m1;weighted_sum:0.7:0.3;omega_cosine:5")
        self.assertEqual(compiled["display"], "spread(q=2%) shelliness(q=3%) weighted_sum(0.7,0.3) ω-cos(5)")
        hydrated = {
            "metrics": [
                {**compiled["metrics"][0], "clip_lo": -1.0, "clip_hi": 2.0},
                {**compiled["metrics"][1], "clip_lo": -0.5, "clip_hi": 1.5},
            ],
            "program_spec": compiled["program_spec"],
        }
        payload = solve_score_program_cli_payload(hydrated)
        self.assertEqual(payload["score_metrics"], "spread,shelliness")
        self.assertEqual(payload["score_clip_los"], "-1,-0.5")
        self.assertEqual(payload["score_clip_his"], "2,1.5")
        self.assertEqual(payload["score_program"], "m0;m1;weighted_sum:0.7:0.3;omega_cosine:5")
        self.assertNotIn("score_sources", payload)

    def test_compile_chain_accepts_internal_transfer_before_combine(self):
        from solve_score_chain import compile_solve_score_chain

        compiled = compile_solve_score_chain(
            [
                ["proximity", "1"],
                ["omega_cosine", "3"],
                ["clusteriness", "2"],
                ["avg"],
            ]
        )

        self.assertEqual(compiled["metric_count"], 2)
        self.assertEqual(compiled["program_spec"], "m0;omega_cosine:3;m1;avg")
        self.assertEqual(compiled["display"], "proximity(q=1%) ω-cos(3) clusteriness(q=2%) avg")
        self.assertFalse(compiled["legacy_compatible"])
        self.assertTrue(compiled["omega_enabled"])
        self.assertEqual(compiled["omega"], 3.0)
        self.assertEqual(compiled["omega_phase"], 0.0)

    def test_compile_chain_accepts_transfer_phase_parameter(self):
        from solve_score_chain import compile_solve_score_chain

        compiled = compile_solve_score_chain(
            [
                ["proximity", "1"],
                ["omega_cosine", "3", "1.57079632679"],
            ]
        )

        self.assertEqual(compiled["program_spec"], "m0;omega_cosine:3:1.5708")
        self.assertEqual(compiled["display"], "proximity(q=1%) ω-cos(3,1.57079632679)")
        self.assertEqual(compiled["omega"], 3.0)
        self.assertAlmostEqual(compiled["omega_phase"], 1.57079632679)
        self.assertFalse(compiled["legacy_compatible"])

    def test_compile_chain_accepts_sawtooth_and_flip_unary_ops(self):
        from solve_score_chain import compile_solve_score_chain

        compiled = compile_solve_score_chain(
            [
                ["proximity", "1"],
                ["sawtooth", "10"],
                ["flip"],
            ]
        )

        self.assertEqual(compiled["program_spec"], "m0;sawtooth:10;flip")
        self.assertEqual(compiled["display"], "proximity(q=1%) sawtooth(10) flip")
        self.assertFalse(compiled["legacy_compatible"])
        self.assertFalse(compiled["omega_enabled"])

    def test_multiple_transfers_are_not_marked_legacy_compatible(self):
        from solve_score_chain import compile_solve_score_chain

        compiled = compile_solve_score_chain(
            [
                ["crowding", "1"],
                ["omega_cosine", "3"],
                ["omega_cosine", "5"],
            ]
        )

        self.assertEqual(compiled["program_spec"], "m0;omega_cosine:3;omega_cosine:5")
        self.assertFalse(compiled["legacy_compatible"])

    def test_compile_chain_or_legacy_preserves_disabled_legacy_omega(self):
        from solve_score_chain import compile_solve_score_chain_or_legacy

        compiled = compile_solve_score_chain_or_legacy(
            '["crowding"]',
            "crowding",
            0.01,
            6,
            False,
            default_metric="crowding",
        )

        self.assertEqual(compiled["chain"], [{"name": "crowding", "params": ["slv", "1"]}])
        self.assertEqual(compiled["metric"], "crowding")
        self.assertEqual(compiled["quantile"], 0.01)
        self.assertEqual(compiled["omega"], 6.0)
        self.assertFalse(compiled["omega_enabled"])

    def test_emit_and_read_metadata_round_trip_for_scopes(self):
        from solve_score_chain import emit_solve_score_metadata, read_solve_score_metadata

        metadata = {}
        metadata.update(
            emit_solve_score_metadata(
                "solve",
                metric="spread",
                quantile=0.02,
                omega=5,
                omega_enabled=True,
                chain=["spread", ["omega_cosine", "5"]],
            )
        )
        metadata.update(
            emit_solve_score_metadata(
                "palette_source",
                metric="crowding",
                quantile=0.01,
                omega=3,
                omega_enabled=False,
                chain=["crowding"],
            )
        )
        metadata.update(
            emit_solve_score_metadata(
                "associated_palette",
                metric="anisotropy",
                quantile=0.03,
                omega=4,
                omega_enabled=False,
                chain=["anisotropy"],
            )
        )

        solve = read_solve_score_metadata("solve", metadata)
        source = read_solve_score_metadata("palette_source", metadata)
        assoc = read_solve_score_metadata("associated_palette", metadata)

        self.assertEqual(
            solve["chain"],
            [
                {"name": "spread", "params": ["slv", "2"]},
                {"name": "omega_cosine", "params": ["5"]},
            ],
        )
        self.assertEqual(source["chain"], [{"name": "crowding", "params": ["slv", "1"]}])
        self.assertEqual(assoc["chain"], [{"name": "anisotropy", "params": ["slv", "3"]}])
        self.assertEqual(solve["quantile"], 0.02)
        self.assertEqual(source["quantile"], 0.01)
        self.assertEqual(assoc["quantile"], 0.03)
        self.assertEqual(source["omega"], 3.0)
        self.assertFalse(source["omega_enabled"])
        self.assertEqual(assoc["omega"], 4.0)
        self.assertFalse(assoc["omega_enabled"])

    def test_compile_chain_accepts_mixed_source_metric_slots(self):
        from solve_score_chain import compile_solve_score_chain, solve_score_program_cli_payload

        compiled = compile_solve_score_chain(
            [
                ["spread", "slv", "2"],
                ["spread", "cf", "3"],
                ["avg"],
            ]
        )

        self.assertEqual(compiled["program_spec"], "m0;m1;avg")
        self.assertEqual(compiled["display"], "spread(q=2%) spread(cf,q=3%) avg")
        self.assertFalse(compiled["legacy_compatible"])
        self.assertEqual(compiled["metrics"][0]["source"], "slv")
        self.assertEqual(compiled["metrics"][1]["source"], "cf")
        self.assertEqual(
            solve_score_program_cli_payload(
                {
                    "metrics": [
                        {**compiled["metrics"][0], "clip_lo": 0.0, "clip_hi": 1.0},
                        {**compiled["metrics"][1], "clip_lo": -2.0, "clip_hi": 4.0},
                    ],
                    "program_spec": compiled["program_spec"],
                }
            )["score_sources"],
            "slv,cf",
        )

    def test_compile_chain_accepts_param_metric_slots(self):
        from solve_score_chain import compile_solve_score_chain, solve_score_program_cli_payload

        compiled = compile_solve_score_chain(
            [
                ["t1_abs", "pm", "2"],
                ["spread", "cf", "3"],
                ["max"],
            ]
        )

        self.assertEqual(compiled["program_spec"], "m0;m1;max")
        self.assertEqual(compiled["display"], "t1_abs(pm,q=2%) spread(cf,q=3%) max")
        self.assertEqual(compiled["metrics"][0]["source"], "pm")
        self.assertEqual(compiled["metrics"][0]["metric"], "t1_abs")
        self.assertEqual(compiled["metrics"][1]["source"], "cf")
        payload = solve_score_program_cli_payload(
            {
                "metrics": [
                    {**compiled["metrics"][0], "clip_lo": 0.0, "clip_hi": 2.0},
                    {**compiled["metrics"][1], "clip_lo": -3.0, "clip_hi": 7.0},
                ],
                "program_spec": compiled["program_spec"],
            }
        )
        self.assertEqual(payload["score_sources"], "pm,cf")

    def test_compile_chain_rejects_param_metric_with_wrong_source(self):
        from solve_score_chain import compile_solve_score_chain

        with self.assertRaises(RuntimeError) as ctx:
            compile_solve_score_chain([["t1_re", "cf", "1"]])
        self.assertIn("only supports source", str(ctx.exception))

    def test_invalid_chain_rejects_second_metric(self):
        from solve_score_chain import compile_solve_score_chain

        with self.assertRaises(RuntimeError) as ctx:
            compile_solve_score_chain([["crowding", "1"], ["spread", "1"]])
        self.assertIn("stack depth 1", str(ctx.exception))


if __name__ == "__main__":
    unittest.main()
