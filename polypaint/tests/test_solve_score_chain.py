import json
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
        self.assertEqual(compiled["program_spec"], "m0-0;omega_cosine:4")
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
        self.assertEqual(compiled["program_spec"], "m0-0;m1-0;weighted_sum:0.7:0.3;omega_cosine:5")
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
        self.assertEqual(payload["score_program"], "m0-0;m1-0;weighted_sum:0.7:0.3;omega_cosine:5")
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
        self.assertEqual(compiled["program_spec"], "m0-0;omega_cosine:3;m1-0;avg")
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

        self.assertEqual(compiled["program_spec"], "m0-0;omega_cosine:3:1.5708")
        self.assertEqual(compiled["display"], "proximity(q=1%) ω-cos(3,1.57079632679)")
        self.assertEqual(compiled["omega"], 3.0)
        self.assertAlmostEqual(compiled["omega_phase"], 1.57079632679)
        self.assertFalse(compiled["legacy_compatible"])

    def test_compile_chain_accepts_uncapped_transfer_frequency(self):
        from solve_score_chain import compile_solve_score_chain

        compiled = compile_solve_score_chain(
            [
                ["proximity", "1"],
                ["omega_cosine", "25", "0.25"],
            ]
        )

        self.assertEqual(compiled["program_spec"], "m0-0;omega_cosine:25:0.25")
        self.assertEqual(compiled["display"], "proximity(q=1%) ω-cos(25,0.25)")
        self.assertEqual(compiled["omega"], 25.0)
        self.assertAlmostEqual(compiled["omega_phase"], 0.25)

    def test_compile_chain_accepts_sawtooth_and_flip_unary_ops(self):
        from solve_score_chain import compile_solve_score_chain

        compiled = compile_solve_score_chain(
            [
                ["proximity", "1"],
                ["sawtooth", "10"],
                ["flip"],
            ]
        )

        self.assertEqual(compiled["program_spec"], "m0-0;sawtooth:10;flip")
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

        self.assertEqual(compiled["program_spec"], "m0-0;omega_cosine:3;omega_cosine:5")
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

        self.assertEqual(compiled["program_spec"], "m0-0;m1-0;avg")
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

        self.assertEqual(compiled["program_spec"], "m0-0;m1-0;max")
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

    def test_compile_chain_accepts_multi_source_extrema_metrics(self):
        from solve_score_chain import compile_solve_score_chain, solve_score_program_cli_payload

        compiled = compile_solve_score_chain(
            [
                ["max_re", "pm", "2"],
                ["min_im", "cf", "3"],
                ["max"],
            ]
        )

        self.assertEqual(compiled["program_spec"], "m0-0;m1-0;max")
        self.assertEqual(compiled["display"], "max_re(pm,q=2%) min_im(cf,q=3%) max")
        self.assertEqual(compiled["metrics"][0]["source"], "pm")
        self.assertEqual(compiled["metrics"][1]["source"], "cf")
        payload = solve_score_program_cli_payload(
            {
                "metrics": [
                    {**compiled["metrics"][0], "clip_lo": -2.0, "clip_hi": 4.0},
                    {**compiled["metrics"][1], "clip_lo": -3.0, "clip_hi": 7.0},
                ],
                "program_spec": compiled["program_spec"],
            }
        )
        self.assertEqual(payload["score_sources"], "pm,cf")

    def test_compile_chain_accepts_max_mod_param_source(self):
        from solve_score_chain import compile_solve_score_chain

        compiled = compile_solve_score_chain(
            [
                ["max_mod", "pm", "1"],
                ["spread", "slv", "1"],
                ["avg"],
            ]
        )

        self.assertEqual(compiled["display"], "max_mod(pm,q=1%) spread(q=1%) avg")
        self.assertEqual(compiled["metrics"][0]["metric"], "max_mod")
        self.assertEqual(compiled["metrics"][0]["source"], "pm")

    def test_compile_chain_lowers_lagged_ref_to_base_slot(self):
        from solve_score_chain import compile_solve_score_chain

        compiled = compile_solve_score_chain(
            [["proximity", "cf", "0.4"], ["proximity", "cf-1", "0.5"], ["abs_diff"]]
        )

        self.assertEqual(compiled["metric_count"], 1)
        self.assertEqual(compiled["metrics"][0]["source"], "cf")
        self.assertEqual(compiled["metrics"][0]["quantile"], 0.004)
        self.assertEqual(compiled["program_spec"], "m0-0;m0-1;abs_diff")
        self.assertTrue(compiled["uses_lag"])
        self.assertEqual(compiled["prelude_by_source"], {"slv": 0, "cf": 1, "pm": 0})

    def test_compile_chain_lowers_lagged_ref_to_later_base_slot(self):
        from solve_score_chain import compile_solve_score_chain

        compiled = compile_solve_score_chain(
            [["proximity", "cf-1", "0.9"], ["proximity", "cf", "0.4"], ["abs_diff"]]
        )

        self.assertEqual(compiled["metric_count"], 1)
        self.assertEqual(compiled["metrics"][0]["source"], "cf")
        self.assertEqual(compiled["metrics"][0]["quantile"], 0.004)
        self.assertEqual(compiled["program_spec"], "m0-1;m0-0;abs_diff")

    def test_generic_metric_chip_is_internal_macro_with_public_serialization(self):
        from solve_score_chain import (
            compile_solve_score_chain,
            public_solve_score_chain,
            serialize_solve_score_chain,
            solve_score_chain_id,
        )

        generic_chain = [
            ["metric", "angular_entropy_16", "cf", "0.5"],
            ["metric", "angular_entropy_16", "cf-1", "0.5"],
            ["abs_diff"],
        ]
        generic_serialized_chain = [
            ["metric", "angular_entropy_16", "cf", "0.5"],
            ["metric", "angular_entropy_16", "cf-1", "0.5"],
            "abs_diff",
        ]
        concrete_chain = [
            ["angular_entropy_16", "cf", "0.5"],
            ["angular_entropy_16", "cf-1", "0.5"],
            ["abs_diff"],
        ]

        compiled = compile_solve_score_chain(generic_chain)

        self.assertEqual(
            compiled["chain"],
            [
                {"name": "__metric", "params": ["angular_entropy_16", "cf", "0.5"]},
                {"name": "__metric", "params": ["angular_entropy_16", "cf-1", "0.5"]},
                {"name": "abs_diff", "params": []},
            ],
        )
        self.assertEqual(
            compiled["expanded_chain"],
            [
                {"name": "angular_entropy_16", "params": ["cf", "0.5"]},
                {"name": "angular_entropy_16", "params": ["cf-1", "0.5"]},
                {"name": "abs_diff", "params": []},
            ],
        )
        self.assertEqual(compiled["program_spec"], "m0-0;m0-1;abs_diff")
        self.assertEqual(compiled["display"], "metric(angular_entropy_16,cf,q=0.5%) metric(angular_entropy_16,cf-1,q=0.5%) abs_diff")
        self.assertEqual(compiled["metrics"][0]["metric"], "angular_entropy_16")
        self.assertEqual(compiled["metrics"][0]["source"], "cf")
        self.assertEqual(public_solve_score_chain(compiled["chain"]), generic_serialized_chain)
        self.assertEqual(json.loads(serialize_solve_score_chain(compiled["chain"])), generic_serialized_chain)
        self.assertEqual(solve_score_chain_id(generic_chain), solve_score_chain_id(concrete_chain))

        internal_wire_chain = [
            ["__metric", "angular_entropy_16", "cf", "0.5"],
        ]
        self.assertEqual(
            json.loads(serialize_solve_score_chain(internal_wire_chain)),
            [["metric", "angular_entropy_16", "cf", "0.5"]],
        )

    def test_read_metadata_keeps_internal_chain_and_exposes_public_chain(self):
        from solve_score_chain import emit_solve_score_metadata, read_solve_score_metadata

        metadata = emit_solve_score_metadata(
            "solve",
            metric="angular_entropy_16",
            quantile=0.005,
            omega=1,
            omega_enabled=False,
            chain=[["metric", "angular_entropy_16", "cf", "0.5"]],
        )
        score = read_solve_score_metadata("solve", metadata, default_metric="proximity")

        self.assertEqual(score["chain"], [{"name": "__metric", "params": ["angular_entropy_16", "cf", "0.5"]}])
        self.assertEqual(score["chain_public"], [["metric", "angular_entropy_16", "cf", "0.5"]])
        self.assertEqual(json.loads(score["chain_json"]), [["metric", "angular_entropy_16", "cf", "0.5"]])

    def test_generic_metric_chip_rejects_non_generic_sources_and_metrics(self):
        from solve_score_chain import compile_solve_score_chain

        cases = [
            ([["metric", "angular_entropy_16", "pm", "0.5"]], "source must be one of slv, cf"),
            ([["__metric", "t1_abs", "slv", "0.5"]], "supports both slv and cf"),
        ]
        for chain, expected in cases:
            with self.subTest(chain=chain):
                with self.assertRaises(RuntimeError) as ctx:
                    compile_solve_score_chain(chain)
                self.assertIn(expected, str(ctx.exception))

    def test_compile_explicit_emit_outputs_raw_vector_contract(self):
        from solve_score_chain import compile_solve_score_chain

        compiled = compile_solve_score_chain([
            ["proximity", "slv", "0.5"],
            ["emit", "norm"],
            ["spread", "cf", "0.5"],
            ["emit", "raw"],
        ])

        self.assertEqual(compiled["program_spec"], "m0-0;emit_norm;m1-0;emit")
        self.assertTrue(compiled["has_explicit_outputs"])
        self.assertFalse(compiled["legacy_compatible"])
        self.assertEqual(compiled["output_channel_count"], 2)
        self.assertEqual(
            compiled["output_channels"],
            [
                {"name": "channel_0", "emit": "emit_norm", "mode": "norm", "channel": 0, "range_normalized": True},
                {"name": "channel_1", "emit": "emit", "mode": "raw", "channel": 1, "range_normalized": False},
            ],
        )

    def test_compile_legacy_emit_alias_serializes_unified(self):
        from solve_score_chain import compile_solve_score_chain, serialize_solve_score_chain

        compiled = compile_solve_score_chain([
            ["proximity", "slv", "0.5"],
            ["emit_norm"],
        ])

        self.assertEqual(compiled["program_spec"], "m0-0;emit_norm")
        self.assertEqual(serialize_solve_score_chain(compiled["chain"]), '[["proximity","0.5"],["emit","norm"]]')

    def test_compile_emit_none_and_flush_for_debug_branches(self):
        from solve_score_chain import compile_solve_score_chain, serialize_solve_score_chain

        compiled = compile_solve_score_chain([
            ["proximity", "slv", "0.5"],
            ["emit", "none"],
            ["flush"],
            ["spread", "slv", "0.5"],
            ["emit", "norm"],
        ])

        self.assertEqual(compiled["program_spec"], "m0-0;emit_none;flush;m1-0;emit_norm")
        self.assertTrue(compiled["has_explicit_outputs"])
        self.assertEqual(compiled["output_channel_count"], 1)
        self.assertEqual(compiled["output_channels"][0]["emit"], "emit_norm")
        self.assertEqual(serialize_solve_score_chain(compiled["chain"]), '[["proximity","0.5"],["emit","none"],"flush",["spread","0.5"],["emit","norm"]]')

        with self.assertRaisesRegex(RuntimeError, "must emit at least one channel"):
            compile_solve_score_chain([
                ["proximity", "slv", "0.5"],
                ["emit", "none"],
            ])

    def test_compile_stack_math_chips(self):
        from solve_score_chain import compile_solve_score_chain

        compiled = compile_solve_score_chain([
            ["proximity", "slv", "0.5"],
            ["const", "1e-3"],
            ["add"],
            ["dup"],
            ["ema", "0.99"],
            ["sin"],
            ["pow", "2"],
            ["clamp"],
        ])

        self.assertEqual(compiled["program_spec"], "m0-0;const:0.001;add;dup;ema:0.99;sin;pow:2;clamp")

    def test_compile_explicit_emit_requires_empty_final_stack(self):
        from solve_score_chain import compile_solve_score_chain

        with self.assertRaisesRegex(RuntimeError, "stack depth 0"):
            compile_solve_score_chain([
                ["proximity", "slv", "0.5"],
                ["spread", "slv", "0.5"],
                ["emit"],
            ])

    def test_compile_chain_lagged_only_seeds_base_slot(self):
        from solve_score_chain import compile_solve_score_chain

        compiled = compile_solve_score_chain([["proximity", "cf-1", "0.9"]])

        self.assertEqual(compiled["metric_count"], 1)
        self.assertEqual(compiled["metrics"][0]["source"], "cf")
        self.assertAlmostEqual(compiled["metrics"][0]["quantile"], 0.009)
        self.assertEqual(compiled["program_spec"], "m0-1")

    def test_canonical_program_rejects_malformed_lag_tokens(self):
        from solve_score_chain import canonicalize_solve_score_program_spec

        for token in ("m0-2", "m0--1", "m0-01", "M0-1", "m0 -1", "m-1"):
            with self.subTest(token=token):
                with self.assertRaises(RuntimeError):
                    canonicalize_solve_score_program_spec(token)

    def test_canonical_program_detects_lag_refs(self):
        from solve_score_chain import solve_score_program_spec_uses_lag

        self.assertTrue(solve_score_program_spec_uses_lag("m0;m0-1;abs_diff"))
        self.assertFalse(solve_score_program_spec_uses_lag("m0;m1-0;avg"))

    def test_compile_chain_rejects_ambiguous_free_form_lagged_ref(self):
        from solve_score_chain import compile_solve_score_chain

        with self.assertRaises(RuntimeError) as ctx:
            compile_solve_score_chain(
                [["proximity", "cf", "0.4"], ["proximity", "cf", "0.5"], ["proximity", "cf-1", "0.7"], ["avg"], ["avg"]]
            )

        self.assertIn("ambiguous", str(ctx.exception))

    def test_compile_chain_rejects_param_metric_with_wrong_source(self):
        from solve_score_chain import compile_solve_score_chain

        with self.assertRaises(RuntimeError) as ctx:
            compile_solve_score_chain([["t1_re", "cf", "1"]])
        self.assertIn("only supports source", str(ctx.exception))

    def test_compile_chain_rejects_multi_source_extrema_metric_with_bad_source(self):
        from solve_score_chain import compile_solve_score_chain

        with self.assertRaises(RuntimeError) as ctx:
            compile_solve_score_chain([["max_re", "bogus", "1"]])
        self.assertIn("must be one of", str(ctx.exception))

    def test_invalid_chain_rejects_second_metric(self):
        from solve_score_chain import compile_solve_score_chain

        with self.assertRaises(RuntimeError) as ctx:
            compile_solve_score_chain([["crowding", "1"], ["spread", "1"]])
        self.assertIn("stack depth 1", str(ctx.exception))


if __name__ == "__main__":
    unittest.main()
