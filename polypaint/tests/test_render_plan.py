"""
Tests for the render plan Lambda.

Color render is fused-only now. These tests pin the shipped fused plan
shape and the remaining bilevel / coeff_bilevel planner behavior.
"""

import json
import os
import sys
import unittest
from unittest.mock import patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _mock_storage_detail(calc):
    def side_effect(path, body):
        if "/detail" in path:
            return {"calc": calc}
        return {}
    return side_effect


def _base_color_calc(**overrides):
    calc = {
        "degree": 5,
        "n_coeffs": 6,
        "N": 100,
        "times": 1,
        "lores": {"bin_key": "renders/j/lores.bin"},
        "chunks": [
            {"idx": 0, "bin_key": "renders/j/chunk_0.bin", "step_start": 0, "step_count": 25},
            {"idx": 1, "bin_key": "renders/j/chunk_1.bin", "step_start": 25, "step_count": 25},
            {"idx": 2, "bin_key": "renders/j/chunk_2.bin", "step_start": 50, "step_count": 25},
            {"idx": 3, "bin_key": "renders/j/chunk_3.bin", "step_start": 75, "step_count": 25},
        ],
    }
    calc.update(overrides)
    return calc


def _make_event(mode="color", pix=1024, tile_size=512, **extra_params):
    params = {
        "pix": pix,
        "tile_size": tile_size,
        "view_mode": "square",
        "square_extent": 2.0,
        "fmt": "jpeg",
        "quality": 90,
        "rotation": 0,
        "color_mode": "solve_score",
        "match_mode": "none",
        "palette": "inferno",
        "constant_color": "ffffff",
    }
    params.update(extra_params)
    return {
        "job_id": "j",
        "run_id": "run_t",
        "task_id": "render_run_color_run_t",
        "mode": mode,
        "params": params,
    }


class TestRenderPlan(unittest.TestCase):
    @patch("handler_render_plan._storage_call")
    def test_color_plan_square_viewport_and_grid(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        result = handler(_make_event(), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["mode"], "color")
        self.assertEqual(plan["render_execution"]["color_pipeline"], "fused")
        self.assertEqual(plan["viewport"]["center_re"], 0)
        self.assertEqual(plan["viewport"]["center_im"], 0)
        self.assertEqual(plan["viewport"]["scale"], 1024 / 4.0)
        self.assertEqual(plan["grid"]["pix"], 1024)
        self.assertEqual(plan["grid"]["tile_size"], 512)
        self.assertEqual(plan["grid"]["n_tiles"], 4)
        self.assertEqual(plan["physical_source_items"], [])
        self.assertEqual(plan["tile_items"], [])
        self.assertEqual(plan["calc"]["degree"], 5)
        self.assertEqual(plan["calc"]["n_coeffs"], 6)
        self.assertEqual(plan["calc"]["n_chunks"], 4)

    @patch("handler_render_plan._invoke_sync")
    @patch("handler_render_plan._storage_call")
    def test_color_plan_auto_viewport(self, mock_storage, mock_invoke):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        mock_invoke.return_value = {"center_re": 0.5, "center_im": -0.25, "scale_ref": 128}
        from handler_render_plan import handler

        result = handler(_make_event(view_mode="auto"), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["viewport"]["center_re"], 0.5)
        self.assertEqual(plan["viewport"]["center_im"], -0.25)
        self.assertAlmostEqual(plan["viewport"]["scale"], 128 * 1024 / 4096, places=6)

    @patch("handler_render_plan._storage_call")
    def test_fused_color_plan_emits_raw_output_contract(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        result = handler(_make_event(raster_workers=24, save_associated_palette=False), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["params"]["color_pipeline"], "fused")
        self.assertEqual(plan["params"]["raster_input_mode"], "sectioned")
        self.assertEqual(plan["params"]["raster_section_mode"], "logical_sections_auto")
        self.assertEqual(plan["raster"]["engine"], "mt")
        self.assertEqual(plan["raster"]["input_mode"], "sectioned")
        self.assertEqual(plan["raster"]["workers"], 24)
        self.assertTrue(plan["raster"]["emit_raw_score_bins"])
        self.assertTrue(plan["raster"]["logical_section"])
        self.assertEqual(plan["outputs"]["raw_key"], "renders/j/color/color_run_t/greyscale.raw")
        self.assertEqual(plan["outputs"]["raw_meta_key"], "renders/j/color/color_run_t/greyscale.meta.json")
        self.assertEqual(plan["outputs"]["fragment_prefix"], "renders/j/color/color_run_t/fragments/section_")
        self.assertEqual(plan["outputs"]["metadata"]["rgb_source"], "raw_score_bins")
        self.assertFalse(plan["outputs"]["repalette_capable"])
        self.assertFalse(plan["outputs"]["pixel_bins_drive_rgb"])
        self.assertEqual(plan["render_execution"]["raster_workers"], 24)
        self.assertEqual(plan["solve_score"]["metric"], "proximity")
        self.assertEqual(plan["solve_score"]["threads"], 4)
        self.assertEqual(set(plan["solve_score"].keys()), {"enabled", "threads", "metric", "quantile", "omega", "omega_enabled", "chain", "clip_key"})
        self.assertEqual(
            set(plan["render_execution"].keys()),
            {
                "color_pipeline",
                "raster_engine",
                "save_associated_palette",
                "raster_mt_threads",
                "raster_workers",
                "solve_score_threads",
                "raster_input_mode",
                "raster_sectioned_retries",
                "raster_section_mode",
                "raster_section_count",
                "raster_section_count_auto",
                "finalize_workers",
            },
        )

    @patch("handler_render_plan._storage_call")
    def test_color_plan_forces_fused_even_if_caller_passes_classic(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        result = handler(_make_event(color_pipeline="classic"), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["params"]["color_pipeline"], "fused")
        self.assertEqual(plan["render_execution"]["color_pipeline"], "fused")

    @patch("handler_render_plan._storage_call")
    def test_fused_color_plan_supports_mixed_source_solve_score_chains(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc(
            lores={
                "bin_key": "renders/j/lores.bin",
                "coeffs_key": "renders/j/lores_coeffs.bin",
                "params_key": "renders/j/lores_params.bin",
            },
            params_key="renders/j/params.bin",
        ))
        from handler_render_plan import handler

        result = handler(_make_event(
            solve_score_chain=[["max_re", "cf", "0.1"], ["t1_abs", "pm", "0.1"], ["avg"]],
            save_associated_palette=True,
        ), None)
        plan = json.loads(result["body"])

        manifest = plan["solve_source_manifest"]["s"]
        self.assertGreater(manifest["slv"]["r"], 0)
        self.assertGreater(manifest["cf"]["r"], 0)
        self.assertGreater(manifest["pm"]["r"], 0)
        self.assertEqual(plan["calc"]["lores_coeffs_key"], "renders/j/lores_coeffs.bin")
        self.assertEqual(plan["calc"]["lores_params_key"], "renders/j/lores_params.bin")
        self.assertTrue(plan["associated_palette"]["enabled"])
        self.assertEqual(plan["associated_palette"]["mode"], "generated")
        self.assertEqual(plan["associated_palette"]["fragment_prefix"], "renders/j/palettes/pal_color_run_t/fragments/section_")

    @patch("handler_render_plan._storage_call")
    def test_fused_color_plan_drops_disabled_associated_palette_payload(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        result = handler(_make_event(save_associated_palette=False), None)
        plan = json.loads(result["body"])

        self.assertFalse(plan["associated_palette"]["enabled"])
        self.assertEqual(plan["associated_palette"]["mode"], "none")
        self.assertEqual(plan["associated_palette"]["fragment_prefix"], "")
        self.assertNotIn("section_items", plan["raster"])

    @patch("handler_render_plan._storage_call")
    def test_fused_plan_coerces_null_n_coeffs_to_degree_plus_one(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc(n_coeffs=None, degree=22))
        from handler_render_plan import handler

        result = handler(_make_event(), None)
        plan = json.loads(result["body"])
        self.assertEqual(plan["calc"]["n_coeffs"], 23)

    @patch("handler_render_plan._storage_call")
    def test_fused_plan_coerces_missing_n_coeffs_to_degree_plus_one(self, mock_storage):
        calc = _base_color_calc(degree=22)
        del calc["n_coeffs"]
        mock_storage.side_effect = _mock_storage_detail(calc)
        from handler_render_plan import handler

        result = handler(_make_event(), None)
        plan = json.loads(result["body"])
        self.assertEqual(plan["calc"]["n_coeffs"], 23)

    @patch("handler_render_plan._storage_call")
    def test_fused_plan_slv_only_lores_keys_are_strings(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        result = handler(_make_event(), None)
        plan = json.loads(result["body"])

        self.assertIsInstance(plan["calc"]["lores_coeffs_key"], str)
        self.assertIsInstance(plan["calc"]["lores_params_key"], str)
        self.assertEqual(plan["calc"]["lores_coeffs_key"], "renders/j/lores_coeffs.bin")
        self.assertEqual(plan["calc"]["lores_params_key"], "renders/j/lores_params.bin")

    @patch("handler_render_plan._storage_call")
    def test_coeff_bilevel_plan_coerces_null_n_coeffs_to_degree_plus_one(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 22,
            "n_coeffs": None,
            "coeffs_keys": ["renders/j/coeffs_0000.bin", "renders/j/coeffs_0001.bin"],
            "chunks": [
                {"idx": 0, "bin_key": "renders/j/chunk_0.bin", "step_start": 0, "step_count": 10},
                {"idx": 1, "bin_key": "renders/j/chunk_1.bin", "step_start": 10, "step_count": 10},
            ],
        })
        from handler_render_plan import handler

        result = handler(_make_event(mode="coeff_bilevel"), None)
        plan = json.loads(result["body"])
        self.assertEqual(plan["calc"]["n_coeffs"], 23)
        self.assertEqual(plan["physical_source_items"][0]["chunk_idx"], 0)
        self.assertEqual(plan["grid"]["raw_tile_prefix"], "renders/j/tile_")
        self.assertEqual(plan["render_execution"], {})
        self.assertEqual(
            set(plan["params"].keys()),
            {"pix", "tile_size", "view_mode", "quantile", "shim", "square_extent", "root_transforms", "rotation"},
        )
        self.assertNotIn("solve_score", plan)
        self.assertNotIn("raster", plan)
        self.assertNotIn("associated_palette", plan)
        self.assertNotIn("saved_palette", plan)

    @patch("handler_render_plan._storage_call")
    def test_bilevel_plan_coerces_null_n_coeffs_to_degree_plus_one(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc(n_coeffs=None, degree=22))
        from handler_render_plan import handler

        result = handler(_make_event(mode="bilevel"), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["calc"]["n_coeffs"], 23)

    @patch("handler_render_plan._storage_call")
    def test_bilevel_plan_uses_logical_sections(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        result = handler(_make_event(mode="bilevel", raster_section_mode="logical_sections", raster_section_count=3), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["mode"], "bilevel")
        self.assertTrue(plan["bilevel"]["enabled"])
        self.assertEqual(plan["bilevel"]["section_mode"], "logical_sections")
        self.assertEqual(plan["bilevel"]["section_count"], 3)
        self.assertTrue(plan["bilevel"]["logical_section"])
        self.assertEqual(plan["bilevel"]["item_count"], 3)
        self.assertEqual(plan["outputs"]["metadata"]["bilevel_pipeline"], "logical_sections_sparse_fragments_v1")
        self.assertEqual(plan["bilevel"]["fragment_prefix"], "renders/j/bilevel_section_")
        self.assertEqual(
            set(plan["bilevel"]["section_items"][0].keys()),
            {"section_idx", "section_count", "step_start", "step_count"},
        )
        self.assertEqual(plan["render_execution"], {})
        self.assertEqual(
            set(plan["params"].keys()),
            {
                "pix",
                "tile_size",
                "view_mode",
                "quantile",
                "shim",
                "square_extent",
                "root_transforms",
                "rotation",
                "raster_section_mode",
                "raster_section_count",
                "raster_section_count_auto",
            },
        )
        self.assertNotIn("solve_score", plan)
        self.assertNotIn("raster", plan)
        self.assertNotIn("associated_palette", plan)
        self.assertNotIn("saved_palette", plan)

    @patch("handler_render_plan._storage_call")
    def test_bilevel_plan_ignores_color_only_params(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        result = handler(_make_event(
            mode="bilevel",
            solve_score_hist_input_mode="sectioned",
            palette_chunk_workers=32,
            save_associated_palette=True,
            color_pipeline="classic",
        ), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["mode"], "bilevel")
        self.assertEqual(plan["render_execution"], {})

    @patch("handler_render_plan._storage_call")
    def test_color_plan_rejects_zero_degree(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc(degree=0))
        from handler_render_plan import handler

        with self.assertRaisesRegex(RuntimeError, "calc.degree must be >= 1"):
            handler(_make_event(), None)

    @patch("handler_render_plan._storage_call")
    def test_color_plan_defaults_null_degree_to_one(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc(degree=None, n_coeffs=None))
        from handler_render_plan import handler

        result = handler(_make_event(), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["calc"]["degree"], 1)
        self.assertEqual(plan["calc"]["n_coeffs"], 2)

    @patch("handler_render_plan._storage_call")
    def test_fused_color_plan_requires_calc_n_for_step_scores(self, mock_storage):
        calc = _base_color_calc()
        del calc["N"]
        mock_storage.side_effect = _mock_storage_detail(calc)
        from handler_render_plan import handler

        with self.assertRaisesRegex(RuntimeError, "requires calc N/n1"):
            handler(_make_event(), None)

    @patch("handler_render_plan._storage_call")
    def test_fused_color_plan_requires_chunk_step_metadata(self, mock_storage):
        calc = _base_color_calc()
        del calc["chunks"][0]["step_count"]
        mock_storage.side_effect = _mock_storage_detail(calc)
        from handler_render_plan import handler

        with self.assertRaisesRegex(RuntimeError, "requires chunk step metadata"):
            handler(_make_event(), None)

    @patch("handler_render_plan._storage_call")
    def test_fused_color_plan_rejects_non_solve_score_modes(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        with self.assertRaisesRegex(RuntimeError, "supports only color_mode=solve_score"):
            handler(_make_event(color_mode="rainbow"), None)

    @patch("handler_render_plan._storage_call")
    def test_fused_color_plan_rejects_tmpfile_raster_input(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        with self.assertRaisesRegex(RuntimeError, "requires raster_input_mode=sectioned"):
            handler(_make_event(raster_input_mode="tmpfile"), None)

    @patch("handler_render_plan._storage_call")
    def test_fused_color_plan_rejects_physical_chunk_sections(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        with self.assertRaisesRegex(RuntimeError, "does not support physical chunk raster sections"):
            handler(_make_event(raster_section_mode="physical_chunks"), None)

    @patch("handler_render_plan._storage_call")
    def test_fused_color_plan_rejects_classic_hist_controls(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        with self.assertRaisesRegex(RuntimeError, "solve_score_hist_input_mode is not supported for fused color"):
            handler(_make_event(solve_score_hist_input_mode="sectioned"), None)

    @patch("handler_render_plan._storage_call")
    def test_large_fused_mixed_source_plan_stays_under_limit(self, mock_storage):
        chunk_count = 317
        step_count = 31_500
        chunks = [
            {
                "idx": idx,
                "bin_key": f"renders/j/chunk_{idx}.bin",
                "step_start": idx * step_count,
                "step_count": step_count,
            }
            for idx in range(chunk_count)
        ]
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 22,
            "n_coeffs": 23,
            "N": 11_560,
            "times": 1,
            "params_key": "renders/j/params.bin",
            "chunks": chunks,
            "lores": {
                "bin_key": "renders/j/lores.bin",
                "coeffs_key": "renders/j/lores_coeffs.bin",
                "params_key": "renders/j/lores_params.bin",
            },
        })
        import handler_render_plan as mod

        result = mod.handler(_make_event(
            pix=4096,
            tile_size=512,
            raster_mt_threads=4,
            raster_workers=10,
            solve_score_chain=[["proximity", "slv", "0.1"], ["min_angular_separation", "cf", "0.1"], ["t2_abs", "pm", "0.1"], ["avg"], ["avg"]],
            save_associated_palette=True,
        ), None)
        self.assertLess(len(result["body"].encode("utf-8")), mod.MAX_PLAN_BYTES)

    @patch("handler_render_plan._storage_call")
    def test_plan_too_large_error_message_includes_current_controls(self, mock_storage):
        chunk_count = 4
        step_count = 1_000_000
        chunks = [
            {
                "idx": idx,
                "bin_key": f"renders/j/chunk_{idx}.bin",
                "step_start": idx * step_count,
                "step_count": step_count,
            }
            for idx in range(chunk_count)
        ]
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 70,
            "n_coeffs": 71,
            "N": 4_000_000,
            "times": 1,
            "params_key": "renders/j/params.bin",
            "chunks": chunks,
            "lores": {
                "bin_key": "renders/j/lores.bin",
                "coeffs_key": "renders/j/lores_coeffs.bin",
                "params_key": "renders/j/lores_params.bin",
            },
        })
        import handler_render_plan as mod

        with patch.object(mod, "MAX_PLAN_BYTES", 1500):
            with self.assertRaises(RuntimeError) as ctx:
                mod.handler(_make_event(
                    pix=4096,
                    tile_size=512,
                    save_associated_palette=True,
                    solve_score_chain=[["max_re", "cf", "0.1"], ["t1_abs", "pm", "0.1"], ["avg"]],
                ), None)

        msg = str(ctx.exception)
        self.assertIn("Counts: chunks=", msg)
        self.assertIn("raster_items=", msg)
        self.assertIn("tiles=", msg)
        self.assertNotIn("solve_score_items=", msg)
        self.assertNotIn("palette_items=", msg)
        self.assertIn("tile_size", msg)
        self.assertNotIn("stripe_count", msg)


if __name__ == "__main__":
    unittest.main()
