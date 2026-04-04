"""
Tests for the palette render plan Lambda.

Validates exact chunk metadata reconstruction and the new durable all-pass
chunk-local output contract used for palette reuse.
"""
import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(**params):
    payload = {
        "job_id": "j",
        "run_id": "run_pal",
        "task_id": "palette_run_run_pal",
        "params": {
            "metric": "crowding",
            "palette": "reef",
            "solve_score_quantile": 0.01,
            "solve_score_omega": 3,
            "root_transforms": [["rotate_roots", "0.25"]],
        },
    }
    payload.update(params)
    return payload


class TestPaletteRenderPlan(unittest.TestCase):

    @patch("handler_palette_render_plan.s3")
    def test_plan_reconstructs_full_chunk_spans_and_output_prefixes(self, mock_s3):
        from handler_palette_render_plan import handler

        calc = {
            "degree": 5,
            "N": 4,
            "times": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
            "chunks": [
                {"idx": 2, "bin_key": "renders/j/chunk_2.bin", "n_t": 10},
                {"idx": 0, "bin_key": "renders/j/chunk_0.bin", "n_t": 5},
                {"idx": 1, "bin_key": "renders/j/chunk_1.bin", "n_t": 17},
            ],
        }
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: json.dumps(calc).encode())}

        result = handler(_event(), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["calc"]["n_chunks"], 3)
        self.assertEqual(plan["calc"]["pass0_steps"], 16)
        self.assertEqual(plan["solve_score"]["omega"], 3.0)
        self.assertEqual(plan["params"]["solve_score_omega"], 3.0)
        self.assertEqual(
            plan["chunk_items"],
            [
                {"chunk_idx": 0, "bin_key": "renders/j/chunk_0.bin", "step_start": 0, "step_count": 5},
                {"chunk_idx": 1, "bin_key": "renders/j/chunk_1.bin", "step_start": 5, "step_count": 17},
                {"chunk_idx": 2, "bin_key": "renders/j/chunk_2.bin", "step_start": 22, "step_count": 10},
            ],
        )
        self.assertNotIn("palette_items", plan)
        self.assertTrue(plan["palette_id"].startswith("pal_"))
        self.assertEqual(plan["outputs"]["chunk_scores_prefix"], f"renders/j/palettes/{plan['palette_id']}/chunks/score_chunk_")
        self.assertEqual(plan["outputs"]["chunk_bins_prefix"], f"renders/j/palettes/{plan['palette_id']}/chunks/palette_bins_chunk_")
        self.assertEqual(plan["outputs"]["chunk_meta_prefix"], f"renders/j/palettes/{plan['palette_id']}/chunks/meta_chunk_")
        self.assertNotIn("score_key", plan["outputs"])
        self.assertNotIn("palette_bins_key", plan["outputs"])

    @patch("handler_palette_render_plan.s3")
    def test_plan_derives_step_count_from_bin_size(self, mock_s3):
        from handler_palette_render_plan import handler

        degree = 7
        step_count = 9
        record_bytes = degree * 2 * 4
        calc = {
            "degree": degree,
            "N": 3,
            "times": 1,
            "lores": {"bin_key": "renders/j/lores.bin"},
            "chunks": [
                {"idx": 0, "bin_key": "renders/j/chunk_0.bin", "bin_size": record_bytes * step_count},
            ],
        }
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: json.dumps(calc).encode())}

        result = handler(_event(), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["chunk_items"][0]["step_count"], step_count)
        self.assertEqual(plan["calc"]["n_chunks"], 1)

    @patch("handler_palette_render_plan.s3")
    def test_invalid_palette_rejected(self, mock_s3):
        from handler_palette_render_plan import handler

        calc = {
            "degree": 5,
            "N": 4,
            "times": 1,
            "lores": {"bin_key": "renders/j/lores.bin"},
            "chunks": [{"idx": 0, "bin_key": "renders/j/chunk_0.bin", "n_t": 16}],
        }
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: json.dumps(calc).encode())}

        with self.assertRaises(RuntimeError) as ctx:
            handler(_event(params={"metric": "proximity", "palette": "bogus", "solve_score_quantile": 0.01, "root_transforms": []}), None)
        self.assertIn("Invalid palette", str(ctx.exception))

    @patch("handler_palette_render_plan.s3")
    def test_tri_palette_accepted(self, mock_s3):
        from handler_palette_render_plan import handler

        calc = {
            "degree": 5,
            "N": 4,
            "times": 1,
            "lores": {"bin_key": "renders/j/lores.bin"},
            "chunks": [{"idx": 0, "bin_key": "renders/j/chunk_0.bin", "n_t": 16}],
        }
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: json.dumps(calc).encode())}

        result = handler(_event(params={"metric": "proximity", "palette": "tri_redgold", "solve_score_quantile": 0.01, "root_transforms": []}), None)
        plan = json.loads(result["body"])
        self.assertEqual(plan["params"]["palette"], "tri_redgold")

    @patch("handler_palette_render_plan.s3")
    def test_long_palette_accepted(self, mock_s3):
        from handler_palette_render_plan import handler

        calc = {
            "degree": 5,
            "N": 4,
            "times": 1,
            "lores": {"bin_key": "renders/j/lores.bin"},
            "chunks": [{"idx": 0, "bin_key": "renders/j/chunk_0.bin", "n_t": 16}],
        }
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: json.dumps(calc).encode())}

        result = handler(_event(params={"metric": "proximity", "palette": "long_marvel_spiderman_long", "solve_score_quantile": 0.01, "root_transforms": []}), None)
        plan = json.loads(result["body"])
        self.assertEqual(plan["params"]["palette"], "long_marvel_spiderman_long")

    @patch("handler_palette_render_plan.s3")
    def test_new_metric_centroid_dist_accepted(self, mock_s3):
        from handler_palette_render_plan import handler

        calc = {
            "degree": 5,
            "N": 4,
            "times": 1,
            "lores": {"bin_key": "renders/j/lores.bin"},
            "chunks": [{"idx": 0, "bin_key": "renders/j/chunk_0.bin", "n_t": 16}],
        }
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: json.dumps(calc).encode())}

        result = handler(_event(params={"metric": "centroid_dist", "palette": "inferno", "solve_score_quantile": 0.01, "root_transforms": []}), None)
        plan = json.loads(result["body"])
        self.assertEqual(plan["params"]["metric"], "centroid_dist")
        self.assertEqual(plan["solve_score"]["metric"], "centroid_dist")


if __name__ == "__main__":
    unittest.main()
