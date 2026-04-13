import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class TestSolveScoreChain(unittest.TestCase):

    def test_compile_chain_accepts_metric_and_transfer(self):
        from solve_score_chain import compile_solve_score_chain

        compiled = compile_solve_score_chain(["crowding", ["omega_cosine", "4"]])

        self.assertEqual(
            compiled["chain"],
            [
                {"name": "crowding", "params": []},
                {"name": "omega_cosine", "params": ["4"]},
            ],
        )
        self.assertEqual(compiled["metric"], "crowding")
        self.assertEqual(compiled["omega"], 4.0)
        self.assertTrue(compiled["omega_enabled"])

    def test_compile_chain_or_legacy_preserves_disabled_legacy_omega(self):
        from solve_score_chain import compile_solve_score_chain_or_legacy

        compiled = compile_solve_score_chain_or_legacy(
            '["crowding"]',
            "crowding",
            6,
            False,
            default_metric="crowding",
        )

        self.assertEqual(compiled["chain"], [{"name": "crowding", "params": []}])
        self.assertEqual(compiled["metric"], "crowding")
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
                {"name": "spread", "params": []},
                {"name": "omega_cosine", "params": ["5"]},
            ],
        )
        self.assertEqual(source["chain"], [{"name": "crowding", "params": []}])
        self.assertEqual(assoc["chain"], [{"name": "anisotropy", "params": []}])
        self.assertEqual(source["omega"], 3.0)
        self.assertFalse(source["omega_enabled"])
        self.assertEqual(assoc["omega"], 4.0)
        self.assertFalse(assoc["omega_enabled"])

    def test_invalid_chain_rejects_second_metric(self):
        from solve_score_chain import compile_solve_score_chain

        with self.assertRaises(RuntimeError) as ctx:
            compile_solve_score_chain(["crowding", "spread"])
        self.assertIn("omega_cosine", str(ctx.exception))


if __name__ == "__main__":
    unittest.main()
