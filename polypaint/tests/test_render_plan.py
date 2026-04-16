"""
Tests for the render plan Lambda.

Validates plan output shape, compactness, viewport computation,
solve-score normalization, and tile/stripe plan correctness.

Does NOT test worker dispatch or DDB polling (plan Lambda must not do those).

Run: cd polypaint && uv run python -m pytest tests/test_render_plan.py -v
"""
import json
import math
import os
import sys
import unittest
from unittest.mock import patch, MagicMock

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _mock_storage_detail(calc):
    """Build a mock _storage_call that returns calc metadata."""
    def side_effect(path, body):
        if "/detail" in path:
            return {"calc": calc}
        return {}
    return side_effect


def _make_event(mode="color", pix=1024, tile_size=512, **extra_params):
    params = {"pix": pix, "tile_size": tile_size, "view_mode": "square",
              "square_extent": 2.0, "fmt": "jpeg", "quality": 90,
              "rotation": 0, "color_mode": "rainbow", "match_mode": "none",
              "palette": "inferno", "constant_color": "ffffff"}
    params.update(extra_params)
    return {
        "job_id": "j", "run_id": "run_t", "task_id": "render_run_color_run_t",
        "mode": mode, "params": params,
    }


class TestRenderPlan(unittest.TestCase):

    @patch("handler_render_plan._storage_call")
    def test_color_plan_square_viewport(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 3,
        })
        from handler_render_plan import handler
        result = handler(_make_event(), None)
        plan = json.loads(result["body"])

        assert plan["mode"] == "color"
        assert plan["viewport"]["center_re"] == 0
        assert plan["viewport"]["center_im"] == 0
        assert plan["viewport"]["scale"] == 1024 / (2 * 2.0)  # pix/(2*extent)
        assert plan["calc"]["degree"] == 5
        assert plan["calc"]["n_chunks"] == 3
        assert len(plan["chunk_items"]) == 3
        assert plan["chunk_items"][0] == {
            "chunk_idx": 0,
            "bin_key": "renders/j/chunk_0.bin",
            "coeffs_key": "renders/j/coeffs_0000.bin",
        }

    @patch("handler_render_plan._storage_call")
    def test_color_plan_uses_calc_chunk_bin_keys_when_present(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5,
            "n_chunks": 99,
            "chunks": [
                {"idx": 2, "bin_key": "renders/j/custom/chunk_two.bin"},
                {"idx": 0, "s3_key": "renders/j/custom/chunk_zero.bin"},
                {"idx": 1, "bin_key": "renders/j/custom/chunk_one.bin"},
            ],
        })
        from handler_render_plan import handler
        result = handler(_make_event(), None)
        plan = json.loads(result["body"])

        assert plan["calc"]["n_chunks"] == 3
        assert plan["chunk_items"] == [
            {"chunk_idx": 0, "bin_key": "renders/j/custom/chunk_zero.bin", "coeffs_key": "renders/j/coeffs_0000.bin"},
            {"chunk_idx": 1, "bin_key": "renders/j/custom/chunk_one.bin", "coeffs_key": "renders/j/coeffs_0001.bin"},
            {"chunk_idx": 2, "bin_key": "renders/j/custom/chunk_two.bin", "coeffs_key": "renders/j/coeffs_0002.bin"},
        ]

    @patch("handler_render_plan._storage_call")
    def test_color_plan_derives_chunk_step_ranges_when_available(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5,
            "chunks": [
                {"idx": 0, "bin_key": "renders/j/chunk_0.bin", "step_count": 10},
                {"idx": 1, "bin_key": "renders/j/chunk_1.bin", "bin_size": 20 * 5 * 2 * 4},
            ],
        })
        from handler_render_plan import handler
        result = handler(_make_event(), None)
        plan = json.loads(result["body"])
        assert plan["chunk_items"] == [
            {
                "chunk_idx": 0,
                "bin_key": "renders/j/chunk_0.bin",
                "coeffs_key": "renders/j/coeffs_0000.bin",
                "step_start": 0,
                "step_count": 10,
                "params_key": "renders/j/params.bin",
                "params_step_start": 0,
                "params_step_count": 10,
                "params_bin_size": 10 * 16,
                "bin_size": 10 * 5 * 2 * 4,
                "coeffs_bin_size": 10 * 6 * 2 * 4,
            },
            {
                "chunk_idx": 1,
                "bin_key": "renders/j/chunk_1.bin",
                "coeffs_key": "renders/j/coeffs_0001.bin",
                "step_start": 10,
                "step_count": 20,
                "params_key": "renders/j/params.bin",
                "params_step_start": 10,
                "params_step_count": 20,
                "params_bin_size": 20 * 16,
                "bin_size": 20 * 5 * 2 * 4,
                "coeffs_bin_size": 20 * 6 * 2 * 4,
            },
        ]

    @patch("handler_render_plan._invoke_sync")
    @patch("handler_render_plan._storage_call")
    def test_color_plan_auto_viewport(self, mock_storage, mock_invoke):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
        })
        mock_invoke.return_value = {
            "center_re": 0.5, "center_im": -0.3, "scale_ref": 128,
        }
        from handler_render_plan import handler
        result = handler(_make_event(view_mode="auto"), None)
        plan = json.loads(result["body"])
        assert plan["viewport"]["center_re"] == 0.5
        assert plan["viewport"]["center_im"] == -0.3
        expected_scale = 128 * 1024 / 4096
        assert abs(plan["viewport"]["scale"] - expected_scale) < 0.01
        mock_invoke.assert_called_once()
        fn_name, payload = mock_invoke.call_args.args
        assert fn_name == "polypaint-viewport"
        assert payload["job_id"] == "j"
        assert payload["quantile"] == 0.0
        assert payload["shim"] == 0.05

    @patch("handler_render_plan._storage_call")
    def test_tile_plan_shape(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
        })
        from handler_render_plan import handler
        result = handler(_make_event(pix=1024, tile_size=512), None)
        plan = json.loads(result["body"])

        assert plan["grid"]["pix"] == 1024
        assert plan["grid"]["tile_size"] == 512
        assert plan["grid"]["n_tile_cols"] == 2
        assert plan["grid"]["n_tile_rows"] == 2
        assert plan["grid"]["n_tiles"] == 4
        assert plan["grid"]["raw_tile_prefix"] == "renders/j/tile_"
        assert plan["grid"]["pixel_bin_tile_prefix"] == ""
        assert len(plan["tile_items"]) == 4
        assert plan["tile_items"][0] == {"tile_idx": 0}

    @patch("handler_render_plan._storage_call")
    def test_solve_score_plan(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score", solve_metric="crowding",
            solve_score_quantile=0.01, solve_score_omega=4
        ), None)
        plan = json.loads(result["body"])

        assert plan["solve_score"]["enabled"] is True
        assert plan["solve_score"]["metric"] == "crowding"
        assert plan["solve_score"]["chain"] == [
            {"name": "crowding", "params": ["slv", "1"]},
            {"name": "omega_cosine", "params": ["4"]},
        ]
        assert plan["solve_score"]["quantile"] == 0.01
        assert plan["solve_score"]["omega"] == 4.0
        assert plan["solve_score"]["omega_enabled"] is True
        assert plan["solve_score"]["threads"] == 1
        assert plan["solve_score"]["hist_input_mode"] == "tmpfile"
        assert plan["solve_score"]["hist_retries"] == 2
        assert "crowding" in plan["solve_score"]["clip_key"]
        assert "crowding" in plan["solve_score"]["bins_key"]
        assert plan["outputs"]["repalette_capable"] is True
        assert plan["outputs"]["pixel_bins_drive_rgb"] is True
        assert plan["outputs"]["metadata"]["view_mode"] == "square"
        assert plan["outputs"]["metadata"]["square_extent"] == "2.0"
        assert plan["outputs"]["metadata"]["rotation"] == "0"
        assert json.loads(plan["outputs"]["metadata"]["solve_score_chain"]) == [
            ["crowding", "1"],
            ["omega_cosine", "4"],
        ]
        assert plan["outputs"]["metadata"]["solve_score_omega"] == "4.0"
        assert plan["outputs"]["metadata"]["solve_score_omega_enabled"] == "true"
        assert plan["outputs"]["metadata"]["background_color"] == "000000"
        assert plan["outputs"]["metadata"]["background_threshold"] == "4"
        assert plan["outputs"]["metadata"]["repalette_capable"] == "true"
        assert plan["outputs"]["metadata"]["pixel_bins_drive_rgb"] == "true"
        assert plan["outputs"]["metadata"]["rgb_source"] == "pixel_bins"
        assert plan["outputs"]["metadata"]["pixel_bins_prefix"] == "renders/j/color/color_run_t/pixel_bins/tile_"
        assert plan["outputs"]["metadata"]["pixel_bins_layout"] == "tile_u8_v1"
        assert plan["grid"]["pixel_bin_tile_prefix"] == "renders/j/color/color_run_t/pixel_bins/tile_"
        assert plan["raster"]["requested_engine"] == "single"
        assert plan["raster"]["pixel_bin_fragment_mode"] == "sparse_chunks"
        assert plan["raster"]["raster_bin_group_size"] == ""

    @patch("handler_render_plan._storage_call")
    def test_render_plan_carries_selected_pixel_bin_fragment_options(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 50,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="spread",
            raster_engine="mt",
            pixel_bin_fragment_mode="dense_grouped",
            raster_bin_group_size=7,
        ), None)
        plan = json.loads(result["body"])

        assert plan["params"]["pixel_bin_fragment_mode"] == "dense_grouped"
        assert plan["params"]["raster_bin_group_size"] == 7
        assert plan["raster"]["pixel_bin_fragment_mode"] == "dense_grouped"
        assert plan["raster"]["raster_bin_group_size"] == 7
        assert plan["raster"]["item_count"] == 8
        assert len(plan["raster_items"]) == 8
        assert plan["raster_items"][0]["group_idx"] == 0
        assert plan["raster_items"][0]["chunk_indices"] == [0, 1, 2, 3, 4, 5, 6]
        assert [item["chunk_idx"] for item in plan["raster_items"][0]["chunks"]] == [0, 1, 2, 3, 4, 5, 6]
        assert plan["raster_items"][-1]["chunk_indices"] == [49]
        assert plan["raster"]["engine"] == "mt"
        assert plan["raster"]["requested_threads"] == 4
        assert plan["raster"]["requested_input_mode"] == "tmpfile"
        assert plan["raster"]["requested_sectioned_retries"] == 2
        assert plan["raster"]["input_mode"] == "tmpfile"
        assert plan["raster"]["sectioned_retries"] == 2
        assert plan["raster"]["threads"] == 4
        assert plan["raster"]["function_name"] == "polypaint-raster-mt"
        assert plan["raster"]["eligible"] is True
        assert plan["raster"]["reason"] == "solve_score"

    @patch("handler_render_plan._storage_call")
    def test_raster_bin_group_size_one_keeps_sparse_chunk_runtime(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 3,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="spread",
            raster_engine="mt",
            pixel_bin_fragment_mode="dense_grouped",
            raster_bin_group_size=1,
        ), None)
        plan = json.loads(result["body"])

        assert plan["params"]["pixel_bin_fragment_mode"] == "dense_grouped"
        assert plan["params"]["raster_bin_group_size"] == 1
        assert plan["raster"]["pixel_bin_fragment_mode"] == "sparse_chunks"
        assert plan["raster"]["raster_bin_group_size"] == ""
        assert plan["raster"]["item_count"] == 3
        assert plan["raster_items"] == []

    @patch("handler_render_plan._storage_call")
    def test_render_plan_persists_full_render_execution_config(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5,
            "n_chunks": 4,
            "N": 100,
            "times": 1,
            "lores": {"bin_key": "renders/j/lores.bin"},
            "chunks": [
                {"idx": 0, "bin_key": "renders/j/chunk_0.bin", "step_count": 25},
                {"idx": 1, "bin_key": "renders/j/chunk_1.bin", "step_count": 25},
                {"idx": 2, "bin_key": "renders/j/chunk_2.bin", "step_count": 25},
                {"idx": 3, "bin_key": "renders/j/chunk_3.bin", "step_count": 25},
            ],
        })
        from handler_render_plan import handler

        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="spread",
            raster_engine="mt",
            raster_mt_threads=6,
            solve_score_threads=3,
            solve_score_hist_input_mode="sectioned",
            solve_score_hist_retries=4,
            raster_input_mode="sectioned",
            raster_sectioned_retries=5,
            pixel_bin_fragment_mode="dense_grouped",
            raster_bin_group_size=2,
            solve_score_merge_workers=12,
            finalize_workers=18,
            save_associated_palette=True,
            palette_chunk_threads=7,
            palette_chunk_input_mode="tmpfile",
            palette_chunk_retries=6,
            palette_chunk_workers=22,
            solve_score_section_mode="logical_sections",
            solve_score_section_count=4,
            palette_section_mode="logical_sections",
            palette_section_count=4,
        ), None)
        plan = json.loads(result["body"])

        render_execution = plan["render_execution"]
        assert render_execution["raster_engine"] == "mt"
        assert render_execution["raster_mt_threads"] == 6
        assert render_execution["solve_score_threads"] == 3
        assert render_execution["solve_score_hist_input_mode"] == "sectioned"
        assert render_execution["solve_score_hist_retries"] == 4
        assert render_execution["raster_input_mode"] == "sectioned"
        assert render_execution["raster_sectioned_retries"] == 5
        assert render_execution["pixel_bin_fragment_mode"] == "dense_grouped"
        assert render_execution["raster_bin_group_size"] == 2
        assert render_execution["solve_score_merge_workers"] == 12
        assert render_execution["finalize_workers"] == 18
        assert render_execution["save_associated_palette"] is True
        assert render_execution["palette_chunk_threads"] == 7
        assert render_execution["palette_chunk_input_mode"] == "tmpfile"
        assert render_execution["palette_chunk_retries"] == 6
        assert render_execution["palette_chunk_workers"] == 22
        assert render_execution["solve_score_section_mode"] == "logical_sections"
        assert render_execution["solve_score_section_count"] == 4
        assert render_execution["palette_section_mode"] == "logical_sections"
        assert render_execution["palette_section_count"] == 4

        metadata_exec = json.loads(plan["outputs"]["metadata"]["render_execution"])
        assert metadata_exec == render_execution
        assert plan["associated_palette"]["chunk_threads"] == 7
        assert plan["associated_palette"]["chunk_input_mode"] == "tmpfile"
        assert plan["associated_palette"]["chunk_retries"] == 6
        assert plan["associated_palette"]["chunk_workers"] == 22
        assert plan["solve_score"]["section_mode"] == "logical_sections"
        assert plan["solve_score"]["section_count"] == 4
        assert plan["associated_palette"]["section_mode"] == "logical_sections"
        assert plan["associated_palette"]["section_count"] == 4

    @patch("handler_render_plan._storage_call")
    def test_manual_solve_score_logical_sections_build_cross_chunk_spans(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5,
            "n_coeffs": 7,
            "N": 100,
            "times": 1,
            "params_key": "renders/j/params.bin",
            "chunks": [
                {"idx": 0, "bin_key": "renders/j/chunk_0.bin", "step_count": 25},
                {"idx": 1, "bin_key": "renders/j/chunk_1.bin", "step_count": 25},
                {"idx": 2, "bin_key": "renders/j/chunk_2.bin", "step_count": 25},
                {"idx": 3, "bin_key": "renders/j/chunk_3.bin", "step_count": 25},
            ],
            "lores": {
                "bin_key": "renders/j/lores.bin",
                "coeffs_key": "renders/j/lores_coeffs.bin",
                "params_key": "renders/j/lores_params.bin",
            },
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_score_chain=[["spread", "cf", "1"], ["t1_abs", "pm", "1"], ["avg"]],
            solve_score_section_mode="logical_sections",
            solve_score_section_count=3,
        ), None)
        plan = json.loads(result["body"])

        solve_score = plan["solve_score"]
        self.assertEqual(solve_score["section_mode"], "logical_sections")
        self.assertEqual(solve_score["section_count"], 3)
        self.assertEqual(solve_score["item_count"], 3)
        self.assertTrue(solve_score["logical_section"])
        first = solve_score["section_items"][0]
        from logical_sections import build_logical_section_spans

        self.assertEqual(first["step_start"], 0)
        self.assertEqual(first["step_count"], 34)
        self.assertEqual(first["bin_size"], 34 * 5 * 2 * 4)
        self.assertNotIn("root_spans", first)
        self.assertNotIn("coeff_spans", first)
        self.assertNotIn("param_spans", first)
        self.assertEqual(first["bin_key"], "")
        self.assertEqual(first["coeffs_key"], "")
        self.assertEqual(first["params_key"], "")
        spans = build_logical_section_spans(
            plan["chunk_items"],
            solve_start=first["step_start"],
            solve_count=first["step_count"],
            degree=5,
            n_coeffs=7,
            include_coeff=True,
            include_param=True,
        )
        self.assertEqual(len(spans["root_spans"]), 2)
        self.assertEqual(spans["root_spans"][0]["key"], "renders/j/chunk_0.bin")
        self.assertEqual(spans["root_spans"][0]["byte_start"], 0)
        self.assertEqual(spans["root_spans"][0]["byte_length"], 25 * 5 * 2 * 4)
        self.assertEqual(spans["root_spans"][1]["key"], "renders/j/chunk_1.bin")
        self.assertEqual(spans["root_spans"][1]["byte_start"], 0)
        self.assertEqual(spans["root_spans"][1]["byte_length"], 9 * 5 * 2 * 4)
        self.assertEqual(len(spans["coeff_spans"]), 2)
        self.assertEqual(spans["coeff_spans"][0]["byte_length"], 25 * 7 * 2 * 4)
        self.assertEqual(spans["coeff_spans"][1]["byte_length"], 9 * 7 * 2 * 4)
        self.assertEqual(len(spans["param_spans"]), 2)
        self.assertEqual(spans["param_spans"][0]["byte_start"], 0)
        self.assertEqual(spans["param_spans"][0]["byte_length"], 25 * 16)
        self.assertEqual(spans["param_spans"][1]["byte_start"], 25 * 16)
        self.assertEqual(spans["param_spans"][1]["byte_length"], 9 * 16)

    @patch("handler_render_plan._storage_call")
    def test_auto_solve_score_logical_sections_uses_computed_safe_count(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 70,
            "n_coeffs": 71,
            "N": 4_000_000,
            "times": 1,
            "params_key": "renders/j/params.bin",
            "chunks": [
                {"idx": 0, "bin_key": "renders/j/chunk_0.bin", "step_count": 1_000_000},
                {"idx": 1, "bin_key": "renders/j/chunk_1.bin", "step_count": 1_000_000},
                {"idx": 2, "bin_key": "renders/j/chunk_2.bin", "step_count": 1_000_000},
                {"idx": 3, "bin_key": "renders/j/chunk_3.bin", "step_count": 1_000_000},
            ],
            "lores": {
                "bin_key": "renders/j/lores.bin",
                "coeffs_key": "renders/j/lores_coeffs.bin",
                "params_key": "renders/j/lores_params.bin",
            },
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_score_threads=4,
            solve_score_chain=[["spread", "cf", "1"], ["t1_abs", "pm", "1"], ["avg"]],
            solve_score_section_mode="logical_sections_auto",
        ), None)
        plan = json.loads(result["body"])

        solve_score = plan["solve_score"]
        self.assertEqual(solve_score["section_mode"], "logical_sections_auto")
        self.assertGreater(solve_score["section_count_auto"], 1)
        self.assertEqual(solve_score["section_count"], solve_score["section_count_auto"])
        self.assertEqual(solve_score["item_count"], solve_score["section_count"])
        self.assertEqual(plan["render_execution"]["solve_score_section_count_auto"], solve_score["section_count_auto"])

    @patch("handler_render_plan._storage_call")
    def test_manual_solve_score_logical_sections_reject_below_safe_minimum(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 70,
            "n_coeffs": 71,
            "N": 4_000_000,
            "times": 1,
            "params_key": "renders/j/params.bin",
            "chunks": [
                {"idx": 0, "bin_key": "renders/j/chunk_0.bin", "step_count": 1_000_000},
                {"idx": 1, "bin_key": "renders/j/chunk_1.bin", "step_count": 1_000_000},
                {"idx": 2, "bin_key": "renders/j/chunk_2.bin", "step_count": 1_000_000},
                {"idx": 3, "bin_key": "renders/j/chunk_3.bin", "step_count": 1_000_000},
            ],
            "lores": {
                "bin_key": "renders/j/lores.bin",
                "coeffs_key": "renders/j/lores_coeffs.bin",
                "params_key": "renders/j/lores_params.bin",
            },
        })
        from handler_render_plan import handler
        with self.assertRaises(RuntimeError) as ctx:
            handler(_make_event(
                color_mode="solve_score",
                solve_score_threads=4,
                solve_score_chain=[["spread", "cf", "1"], ["t1_abs", "pm", "1"], ["avg"]],
                solve_score_section_mode="logical_sections",
                solve_score_section_count=1,
            ), None)
        self.assertIn("safe minimum", str(ctx.exception))

    @patch("handler_render_plan._storage_call")
    def test_plan_too_large_message_uses_current_controls(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 70,
            "n_coeffs": 71,
            "N": 4_000_000,
            "times": 1,
            "params_key": "renders/j/params.bin",
            "chunks": [
                {"idx": 0, "bin_key": "renders/j/chunk_0.bin", "step_count": 1_000_000},
                {"idx": 1, "bin_key": "renders/j/chunk_1.bin", "step_count": 1_000_000},
                {"idx": 2, "bin_key": "renders/j/chunk_2.bin", "step_count": 1_000_000},
                {"idx": 3, "bin_key": "renders/j/chunk_3.bin", "step_count": 1_000_000},
            ],
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
                    color_mode="solve_score",
                    solve_score_threads=4,
                    solve_score_chain=[["spread", "cf", "1"], ["t1_abs", "pm", "1"], ["avg"]],
                    solve_score_section_mode="logical_sections_auto",
                    save_associated_palette=True,
                    palette_section_mode="logical_sections_auto",
                    pix=4096,
                    tile_size=512,
                ), None)
        msg = str(ctx.exception)
        self.assertIn("Counts: chunks=", msg)
        self.assertIn("solve_score_items=", msg)
        self.assertIn("palette_items=", msg)
        self.assertIn("tile_size", msg)
        self.assertNotIn("stripe count", msg.lower())

    @patch("handler_render_plan._storage_call")
    def test_solve_score_chain_input_compiles_to_scalar_contract(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="spread",
            solve_score_omega=9,
            solve_score_chain=["clusteriness", ["omega_cosine", "5"]],
        ), None)
        plan = json.loads(result["body"])

        assert plan["solve_score"]["metric"] == "clusteriness"
        assert plan["solve_score"]["omega"] == 5.0
        assert plan["solve_score"]["omega_enabled"] is True
        assert plan["solve_score"]["chain"] == [
            {"name": "clusteriness", "params": ["slv", "0.1"]},
            {"name": "omega_cosine", "params": ["5"]},
        ]
        assert json.loads(plan["outputs"]["metadata"]["solve_score_chain"]) == [
            ["clusteriness", "0.1"],
            ["omega_cosine", "5"],
        ]
        assert plan["outputs"]["metadata"]["solve_metric"] == "clusteriness"
        assert plan["outputs"]["metadata"]["solve_score_omega"] == "5.0"

    @patch("handler_render_plan._storage_call")
    def test_solve_score_render_accepts_mixed_source_chain(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin", "coeffs_key": "renders/j/lores_coeffs.bin"},
            "n_coeffs": 7,
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="spread",
            solve_score_chain=[["spread", "slv", "1"], ["spread", "cf", "1"], ["avg"]],
        ), None)
        plan = json.loads(result["body"])
        self.assertEqual(plan["calc"]["lores_coeffs_key"], "renders/j/lores_coeffs.bin")
        self.assertEqual(plan["calc"]["n_coeffs"], 7)
        self.assertEqual(plan["solve_score"]["metrics"][0]["source"], "slv")
        self.assertEqual(plan["solve_score"]["metrics"][1]["source"], "cf")

    @patch("handler_render_plan._storage_call")
    def test_mt_request_carries_requested_threads(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="crowding",
            raster_engine="mt",
            raster_mt_threads=6,
        ), None)
        plan = json.loads(result["body"])
        assert plan["raster"]["requested_engine"] == "mt"
        assert plan["raster"]["engine"] == "mt"
        assert plan["raster"]["requested_threads"] == 6
        assert plan["raster"]["threads"] == 6
        assert plan["raster"]["input_mode"] == "tmpfile"
        assert plan["raster"]["function_name"] == "polypaint-raster-mt"
        assert plan["solve_score"]["threads"] == 6

    @patch("handler_render_plan._storage_call")
    def test_raster_input_mode_sectioned_override(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="crowding",
            raster_engine="mt",
            raster_input_mode="sectioned",
        ), None)
        plan = json.loads(result["body"])
        assert plan["raster"]["requested_input_mode"] == "sectioned"
        assert plan["raster"]["input_mode"] == "sectioned"
        assert plan["raster"]["sectioned_retries"] == 2

    @patch("handler_render_plan._storage_call")
    def test_sectioned_retry_overrides(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="crowding",
            raster_engine="mt",
            raster_input_mode="sectioned",
            raster_sectioned_retries=5,
            solve_score_hist_retries=4,
        ), None)
        plan = json.loads(result["body"])
        assert plan["solve_score"]["hist_retries"] == 4
        assert plan["raster"]["requested_sectioned_retries"] == 5
        assert plan["raster"]["sectioned_retries"] == 5

    @patch("handler_render_plan._storage_call")
    def test_explicit_solve_score_threads_override_raster_threads(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="crowding",
            raster_engine="mt",
            raster_mt_threads=6,
            solve_score_threads=3,
        ), None)
        plan = json.loads(result["body"])
        assert plan["raster"]["threads"] == 6
        assert plan["solve_score"]["threads"] == 3

    @patch("handler_render_plan._storage_call")
    def test_solve_score_merge_workers_override(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="crowding",
            solve_score_merge_workers=24,
        ), None)
        plan = json.loads(result["body"])
        assert plan["solve_score"]["merge_workers"] == 24

    @patch("handler_render_plan._storage_call")
    def test_finalize_workers_override(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="crowding",
            finalize_workers=20,
        ), None)
        plan = json.loads(result["body"])
        assert plan["finalize"]["workers"] == 20

    @patch("handler_render_plan._storage_call")
    def test_solve_score_hist_input_mode_override(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="crowding",
            solve_score_hist_input_mode="stdin",
        ), None)
        plan = json.loads(result["body"])
        assert plan["solve_score"]["hist_input_mode"] == "stdin"

    @patch("handler_render_plan._storage_call")
    def test_solve_score_hist_input_mode_sectioned_override(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="crowding",
            solve_score_hist_input_mode="sectioned",
        ), None)
        plan = json.loads(result["body"])
        assert plan["solve_score"]["hist_input_mode"] == "sectioned"

    @patch("handler_render_plan._storage_call")
    def test_invalid_solve_score_hist_input_mode_rejected(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        with self.assertRaises(RuntimeError) as ctx:
            handler(_make_event(color_mode="solve_score", solve_score_hist_input_mode="socket"), None)
        self.assertIn("solve_score_hist_input_mode", str(ctx.exception))

    @patch("handler_render_plan._storage_call")
    def test_invalid_mt_threads_rejected(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        with self.assertRaises(RuntimeError) as ctx:
            handler(_make_event(color_mode="solve_score", raster_engine="mt", raster_mt_threads=0), None)
        self.assertIn("raster_mt_threads", str(ctx.exception))

    @patch("handler_render_plan._storage_call")
    def test_invalid_raster_input_mode_rejected(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        with self.assertRaises(RuntimeError) as ctx:
            handler(_make_event(color_mode="solve_score", raster_engine="mt", raster_input_mode="socket"), None)
        self.assertIn("raster_input_mode", str(ctx.exception))

    @patch("handler_render_plan._storage_call")
    def test_invalid_solve_score_threads_rejected(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        with self.assertRaises(RuntimeError) as ctx:
            handler(_make_event(color_mode="solve_score", solve_score_threads=0), None)
        self.assertIn("solve_score_threads", str(ctx.exception))

    @patch("handler_render_plan._storage_call")
    def test_invalid_solve_score_merge_workers_rejected(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        with self.assertRaises(RuntimeError) as ctx:
            handler(_make_event(color_mode="solve_score", solve_score_merge_workers=0), None)
        self.assertIn("solve_score_merge_workers", str(ctx.exception))

    @patch("handler_render_plan._storage_call")
    def test_invalid_finalize_workers_rejected(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        with self.assertRaises(RuntimeError) as ctx:
            handler(_make_event(color_mode="solve_score", finalize_workers=0), None)
        self.assertIn("finalize_workers", str(ctx.exception))

    @patch("handler_render_plan._storage_call")
    def test_invalid_sectioned_retries_rejected(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        with self.assertRaises(RuntimeError) as ctx:
            handler(_make_event(color_mode="solve_score", solve_score_hist_retries=-1), None)
        self.assertIn("solve_score_hist_retries", str(ctx.exception))
        with self.assertRaises(RuntimeError) as ctx:
            handler(_make_event(color_mode="solve_score", raster_sectioned_retries=11), None)
        self.assertIn("raster_sectioned_retries", str(ctx.exception))

    @patch("handler_render_plan._storage_call")
    def test_tri_palette_id_accepted_and_preserved(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(color_mode="solve_score", palette="tri_redgold"), None)
        plan = json.loads(result["body"])
        assert plan["outputs"]["metadata"]["palette"] == "tri_redgold"

    @patch("handler_render_plan._storage_call")
    def test_long_palette_id_accepted_and_preserved(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(color_mode="solve_score", palette="long_marvel_spiderman_long"), None)
        plan = json.loads(result["body"])
        assert plan["outputs"]["metadata"]["palette"] == "long_marvel_spiderman_long"

    @patch("handler_render_plan._storage_call")
    def test_invalid_palette_rejected(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        with self.assertRaises(RuntimeError) as ctx:
            handler(_make_event(color_mode="solve_score", palette="tri_not_real"), None)
        self.assertIn("Invalid palette", str(ctx.exception))

    @patch("handler_render_plan.s3")
    @patch("handler_render_plan._storage_call")
    def test_saved_palette_mode_uses_source_palette_contract(self, mock_storage, mock_s3):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2, "N": 1024, "times": 3,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        source_meta = {
            "job_id": "j",
            "palette_id": "pal_src",
            "display_name": "crowding q=1.0% w=3 reef",
            "metric": "crowding",
            "palette": "reef",
            "solve_score_chain": ["crowding"],
            "solve_score_quantile": 0.01,
            "solve_score_omega": 3.0,
            "solve_score_omega_enabled": False,
            "root_transforms": [["rotate_roots", "0.25"]],
            "degree": 5,
            "N": 1024,
            "times": 3,
            "render_reusable": True,
            "data_layout": "chunk_all_pass_v1",
            "chunk_bins_prefix": "renders/j/palettes/pal_src/chunks/palette_bins_chunk_",
        }
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: json.dumps(source_meta).encode())}
        from handler_render_plan import handler
        result = handler(_make_event(color_mode="saved_palette", saved_palette_id="pal_src", palette="inferno"), None)
        plan = json.loads(result["body"])
        assert plan["solve_score"]["enabled"] is False
        assert plan["saved_palette"]["enabled"] is True
        assert plan["saved_palette"]["palette_id"] == "pal_src"
        assert plan["saved_palette"]["chunk_bins_prefix"] == "renders/j/palettes/pal_src/chunks/palette_bins_chunk_"
        assert plan["solve_score"]["chain"] == [{"name": "crowding", "params": ["slv", "1"]}]
        assert plan["outputs"]["repalette_capable"] is True
        assert plan["outputs"]["pixel_bins_drive_rgb"] is True
        assert plan["outputs"]["metadata"]["color_mode"] == "saved_palette"
        assert plan["outputs"]["metadata"]["palette"] == "inferno"
        assert plan["outputs"]["metadata"]["palette_source_id"] == "pal_src"
        assert plan["outputs"]["metadata"]["palette_source_palette"] == "reef"
        assert json.loads(plan["outputs"]["metadata"]["solve_score_chain"]) == [["crowding", "1"]]
        assert json.loads(plan["outputs"]["metadata"]["palette_source_score_chain"]) == [["crowding", "1"]]
        assert plan["outputs"]["metadata"]["solve_metric"] == "crowding"
        assert plan["outputs"]["metadata"]["solve_score_quantile"] == "0.01"
        assert plan["outputs"]["metadata"]["solve_score_omega"] == "3.0"
        assert plan["outputs"]["metadata"]["solve_score_omega_enabled"] == "false"
        assert plan["outputs"]["metadata"]["palette_source_omega_enabled"] == "false"
        assert plan["outputs"]["metadata"]["repalette_capable"] == "true"
        assert plan["outputs"]["metadata"]["pixel_bins_drive_rgb"] == "true"
        assert plan["outputs"]["metadata"]["rgb_source"] == "pixel_bins"
        assert plan["outputs"]["metadata"]["pixel_bins_prefix"] == "renders/j/color/color_run_t/pixel_bins/tile_"
        assert plan["params"]["palette"] == "inferno"
        assert plan["params"]["root_transforms"] == [["rotate_roots", "0.25"]]
        assert plan["raster"]["requested_engine"] == "single"
        assert plan["raster"]["engine"] == "single"
        assert plan["raster"]["reason"] == "saved_palette"

    @patch("handler_render_plan.s3")
    @patch("handler_render_plan._storage_call")
    def test_saved_palette_associated_dependency_metadata(self, mock_storage, mock_s3):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2, "N": 1024, "times": 3,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        source_meta = {
            "job_id": "j",
            "palette_id": "pal_src",
            "display_name": "crowding q=1.0% w=3 reef",
            "metric": "crowding",
            "palette": "reef",
            "solve_score_chain": ["crowding"],
            "solve_score_quantile": 0.01,
            "solve_score_omega": 3.0,
            "solve_score_omega_enabled": False,
            "root_transforms": [["rotate_roots", "0.25"]],
            "degree": 5,
            "N": 1024,
            "times": 3,
            "render_reusable": True,
            "data_layout": "chunk_all_pass_v1",
            "image_key": "renders/j/palettes/pal_src/image.jpeg",
            "preview_key": "renders/j/palettes/pal_src/preview.png",
            "chunk_bins_prefix": "renders/j/palettes/pal_src/chunks/palette_bins_chunk_",
        }
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: json.dumps(source_meta).encode())}
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="saved_palette",
            saved_palette_id="pal_src",
            palette="inferno",
            save_associated_palette=True,
        ), None)
        plan = json.loads(result["body"])
        assert plan["associated_palette"]["enabled"] is True
        assert plan["associated_palette"]["mode"] == "dependency"
        assert plan["associated_palette"]["palette_id"] == "pal_src"
        assert plan["associated_palette"]["image_key"] == "renders/j/palettes/pal_src/image.jpeg"
        assert plan["associated_palette"]["score_chain"] == [{"name": "crowding", "params": ["slv", "1"]}]
        assert plan["outputs"]["metadata"]["associated_palette_mode"] == "dependency"
        assert plan["outputs"]["metadata"]["associated_palette_id"] == "pal_src"
        assert plan["outputs"]["metadata"]["associated_palette_image_key"] == "renders/j/palettes/pal_src/image.jpeg"
        assert json.loads(plan["outputs"]["metadata"]["associated_palette_score_chain"]) == [["crowding", "1"]]
        assert plan["outputs"]["metadata"]["associated_palette_omega_enabled"] == "false"

    @patch("handler_render_plan._storage_call")
    def test_solve_score_plan_accepts_disabled_omega(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score", solve_metric="crowding",
            solve_score_quantile=0.01, solve_score_omega=4, solve_score_omega_enabled=False
        ), None)
        plan = json.loads(result["body"])
        assert plan["solve_score"]["omega_enabled"] is False
        assert plan["outputs"]["metadata"]["solve_score_omega_enabled"] == "false"

    @patch("handler_render_plan._storage_call")
    def test_solve_score_plan_can_generate_associated_palette(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5,
            "N": 100,
            "times": 2,
            "chunks": [
                {"idx": 0, "bin_key": "renders/j/chunk_0.bin", "step_count": 6000},
                {"idx": 1, "bin_key": "renders/j/chunk_1.bin", "step_count": 14000},
            ],
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="crowding",
            solve_score_quantile=0.01,
            solve_score_omega=4,
            save_associated_palette=True,
        ), None)
        plan = json.loads(result["body"])
        assoc = plan["associated_palette"]
        assert assoc["enabled"] is True
        assert assoc["mode"] == "generated"
        assert assoc["palette_id"] == "pal_color_run_t"
        assert assoc["image_key"] == "renders/j/palettes/pal_color_run_t/image.jpeg"
        assert assoc["chunk_bins_prefix"] == "renders/j/palettes/pal_color_run_t/chunks/palette_bins_chunk_"
        assert assoc["score_chain"] == [
            {"name": "crowding", "params": ["slv", "1"]},
            {"name": "omega_cosine", "params": ["4"]},
        ]
        assert assoc["chunk_threads"] == 4
        assert assoc["chunk_input_mode"] == "sectioned"
        assert assoc["chunk_retries"] == 2
        assert assoc["chunk_workers"] == 16
        assert plan["calc"]["N"] == 100
        assert plan["calc"]["times"] == 2
        assert plan["outputs"]["metadata"]["associated_palette_mode"] == "generated"
        assert plan["outputs"]["metadata"]["associated_palette_id"] == "pal_color_run_t"
        assert plan["outputs"]["metadata"]["associated_palette_image_key"] == "renders/j/palettes/pal_color_run_t/image.jpeg"
        assert json.loads(plan["outputs"]["metadata"]["associated_palette_score_chain"]) == [
            ["crowding", "1"],
            ["omega_cosine", "4"],
        ]
        assert plan["outputs"]["metadata"]["associated_palette_omega_enabled"] == "true"

    @patch("handler_render_plan._storage_call")
    def test_associated_palette_manual_logical_sections_build_cross_chunk_spans(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5,
            "n_coeffs": 7,
            "N": 100,
            "times": 1,
            "chunks": [
                {"idx": 0, "bin_key": "renders/j/chunk_0.bin", "step_count": 25},
                {"idx": 1, "bin_key": "renders/j/chunk_1.bin", "step_count": 25},
                {"idx": 2, "bin_key": "renders/j/chunk_2.bin", "step_count": 25},
                {"idx": 3, "bin_key": "renders/j/chunk_3.bin", "step_count": 25},
            ],
            "lores": {"bin_key": "renders/j/lores.bin", "coeffs_key": "renders/j/lores_coeffs.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_score_chain=[["spread", "cf", "1"]],
            save_associated_palette=True,
            palette_section_mode="logical_sections",
            palette_section_count=3,
        ), None)
        plan = json.loads(result["body"])
        assoc = plan["associated_palette"]
        self.assertEqual(assoc["enabled"], True)
        self.assertEqual(assoc["section_mode"], "logical_sections")
        self.assertEqual(assoc["section_count"], 3)
        self.assertEqual(assoc["item_count"], 3)
        self.assertTrue(assoc["logical_section"])
        first = assoc["section_items"][0]
        from logical_sections import build_logical_section_spans

        self.assertEqual(first["step_count"], 34)
        self.assertNotIn("root_spans", first)
        self.assertNotIn("coeff_spans", first)
        self.assertEqual(first["bin_key"], "")
        self.assertEqual(first["coeffs_key"], "")
        spans = build_logical_section_spans(
            plan["chunk_items"],
            solve_start=first["step_start"],
            solve_count=first["step_count"],
            degree=5,
            n_coeffs=7,
            include_coeff=True,
            include_param=False,
        )
        self.assertEqual(len(spans["root_spans"]), 2)
        self.assertEqual(len(spans["coeff_spans"]), 2)
        self.assertEqual(spans["root_spans"][1]["key"], "renders/j/chunk_1.bin")
        self.assertEqual(spans["coeff_spans"][1]["byte_length"], 9 * 7 * 2 * 4)

    @patch("handler_render_plan._storage_call")
    def test_mixed_source_solve_score_associated_palette_preserves_calc_coeff_metadata(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5,
            "n_coeffs": 7,
            "N": 100,
            "times": 1,
            "chunks": [
                {"idx": 0, "bin_key": "renders/j/chunk_0.bin", "step_count": 6000},
                {"idx": 1, "bin_key": "renders/j/chunk_1.bin", "step_count": 4000},
            ],
            "lores": {"bin_key": "renders/j/lores.bin", "coeffs_key": "renders/j/lores_coeffs.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="spread",
            solve_score_quantile=0.01,
            solve_score_chain=[["spread", "cf", "1"], ["shelliness", "cf", "1"], ["max"]],
            save_associated_palette=True,
        ), None)
        plan = json.loads(result["body"])
        assert plan["calc"]["n_coeffs"] == 7
        assert plan["calc"]["lores_coeffs_key"] == "renders/j/lores_coeffs.bin"
        assert plan["chunk_items"][0]["coeffs_key"] == "renders/j/coeffs_0000.bin"
        assert plan["chunk_items"][1]["coeffs_key"] == "renders/j/coeffs_0001.bin"
        assert plan["chunk_items"][0]["coeffs_bin_size"] == 6000 * 7 * 2 * 4
        assert plan["chunk_items"][1]["coeffs_bin_size"] == 4000 * 7 * 2 * 4
        assoc = plan["associated_palette"]
        assert assoc["enabled"] is True
        assert assoc["mode"] == "generated"
        assert json.loads(plan["outputs"]["metadata"]["associated_palette_score_chain"]) == [
            ["spread", "cf", "1"],
            ["shelliness", "cf", "1"],
            "max",
        ]

    @patch("handler_render_plan._storage_call")
    def test_param_source_solve_score_preserves_calc_param_metadata(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5,
            "n_coeffs": 7,
            "N": 100,
            "times": 1,
            "params_key": "renders/j/params.bin",
            "chunks": [
                {"idx": 0, "bin_key": "renders/j/chunk_0.bin", "step_count": 6000},
                {"idx": 1, "bin_key": "renders/j/chunk_1.bin", "step_count": 4000},
            ],
            "lores": {
                "bin_key": "renders/j/lores.bin",
                "params_key": "renders/j/lores_params.bin",
            },
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="t1_abs",
            solve_score_quantile=0.01,
            solve_score_chain=[["t1_abs", "pm", "1"], ["spread", "slv", "1"], ["avg"]],
        ), None)
        plan = json.loads(result["body"])
        assert plan["calc"]["lores_params_key"] == "renders/j/lores_params.bin"
        assert plan["calc"]["params_key"] == "renders/j/params.bin"
        assert plan["chunk_items"][0]["step_start"] == 0
        assert plan["chunk_items"][0]["step_count"] == 6000
        assert plan["chunk_items"][0]["params_key"] == "renders/j/params.bin"
        assert plan["chunk_items"][0]["params_step_start"] == 0
        assert plan["chunk_items"][0]["params_step_count"] == 6000
        assert plan["chunk_items"][1]["step_start"] == 6000
        assert plan["chunk_items"][1]["step_count"] == 4000
        assert plan["chunk_items"][1]["params_key"] == "renders/j/params.bin"
        assert plan["chunk_items"][1]["params_step_start"] == 6000
        assert plan["chunk_items"][1]["params_step_count"] == 4000
        assert json.loads(plan["outputs"]["metadata"]["solve_score_chain"]) == [
            ["t1_abs", "pm", "1"],
            ["spread", "1"],
            "avg",
        ]

    @patch("handler_render_plan._storage_call")
    def test_param_source_solve_score_accepts_chunked_param_metadata(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5,
            "n_coeffs": 7,
            "N": 100,
            "times": 1,
            "param_storage_mode": "chunked",
            "params_key": "",
            "chunks": [
                {
                    "idx": 0,
                    "bin_key": "renders/j/chunk_0.bin",
                    "step_count": 6000,
                    "params_key": "renders/j/params_0000.bin",
                    "params_step_start": 0,
                    "params_step_count": 6000,
                    "params_bin_size": 6000 * 16,
                },
                {
                    "idx": 1,
                    "bin_key": "renders/j/chunk_1.bin",
                    "step_count": 4000,
                    "params_key": "renders/j/params_0001.bin",
                    "params_step_start": 0,
                    "params_step_count": 4000,
                    "params_bin_size": 4000 * 16,
                },
            ],
            "lores": {
                "bin_key": "renders/j/lores.bin",
                "params_key": "renders/j/lores_params.bin",
            },
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="t1_abs",
            solve_score_quantile=0.01,
            solve_score_chain=[["t1_abs", "pm", "1"], ["spread", "slv", "1"], ["avg"]],
        ), None)
        plan = json.loads(result["body"])
        assert plan["calc"]["params_key"] == ""
        assert plan["calc"]["param_storage_mode"] == "chunked"
        assert plan["chunk_items"][0]["params_key"] == "renders/j/params_0000.bin"
        assert plan["chunk_items"][0]["params_step_start"] == 0
        assert plan["chunk_items"][1]["params_key"] == "renders/j/params_0001.bin"
        assert plan["chunk_items"][1]["params_step_start"] == 0

    @patch("handler_render_plan._storage_call")
    def test_associated_palette_chunk_defaults_follow_mt_raster_settings_when_present(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5,
            "N": 100,
            "times": 1,
            "chunks": [{"idx": 0, "bin_key": "renders/j/chunk_0.bin", "step_count": 10000}],
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="crowding",
            save_associated_palette=True,
            raster_engine="mt",
            raster_mt_threads=7,
            raster_input_mode="tmpfile",
            raster_sectioned_retries=5,
        ), None)
        plan = json.loads(result["body"])
        assoc = plan["associated_palette"]
        assert assoc["chunk_threads"] == 7
        assert assoc["chunk_input_mode"] == "tmpfile"
        assert assoc["chunk_retries"] == 0
        assert assoc["chunk_workers"] == 16

    @patch("handler_render_plan._storage_call")
    def test_solve_score_associated_palette_requires_chunk_step_metadata(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5,
            "N": 100,
            "times": 1,
            "chunks": [
                {"idx": 0, "bin_key": "renders/j/chunk_0.bin"},
            ],
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        with self.assertRaises(RuntimeError) as ctx:
            handler(_make_event(
                color_mode="solve_score",
                solve_metric="crowding",
                save_associated_palette=True,
            ), None)
        self.assertIn("requires chunk step metadata", str(ctx.exception))

    @patch("handler_render_plan.s3")
    @patch("handler_render_plan._storage_call")
    def test_saved_palette_mode_rejects_non_reusable_palette(self, mock_storage, mock_s3):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2, "N": 1024, "times": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        source_meta = {
            "job_id": "j",
            "palette_id": "pal_legacy",
            "metric": "crowding",
            "palette": "reef",
            "degree": 5,
            "N": 1024,
            "times": 2,
            "render_reusable": False,
            "data_layout": "legacy_pass0_v1",
        }
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: json.dumps(source_meta).encode())}
        from handler_render_plan import handler
        with self.assertRaises(RuntimeError) as ctx:
            handler(_make_event(color_mode="saved_palette", saved_palette_id="pal_legacy"), None)
        self.assertIn("not render-reusable", str(ctx.exception))

    @patch("handler_render_plan._storage_call")
    def test_coeff_bilevel_uses_coeffs_keys(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "coeffs_keys": ["renders/j/c0.bin", "renders/j/c1.bin"],
            "n_coeffs": 99,
        })
        from handler_render_plan import handler
        result = handler(_make_event(mode="coeff_bilevel"), None)
        plan = json.loads(result["body"])
        assert plan["calc"]["coeffs_keys"] == ["renders/j/c0.bin", "renders/j/c1.bin"]
        assert plan["calc"]["n_coeffs"] == 99

    @patch("handler_render_plan._storage_call")
    def test_plan_compactness_check(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
        })
        from handler_render_plan import handler
        result = handler(_make_event(), None)
        plan_json = json.loads(result["body"])
        assert len(plan_json) < 200 * 1024, "plan exceeds 200KB"

    @patch("handler_render_plan._storage_call")
    def test_constant_color_plan_routes_to_mt_raster(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({"degree": 5, "n_chunks": 2})
        from handler_render_plan import handler
        result = handler(_make_event(color_mode="constant"), None)
        plan = json.loads(result["body"])
        assert plan["raster"]["engine"] == "single"
        assert plan["raster"]["eligible"] is True
        assert plan["raster"]["reason"] == "constant"

    @patch("handler_render_plan._storage_call")
    def test_rainbow_match_none_routes_to_mt_raster(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({"degree": 5, "n_chunks": 2})
        from handler_render_plan import handler
        result = handler(_make_event(color_mode="rainbow", match_mode="none"), None)
        plan = json.loads(result["body"])
        assert plan["raster"]["engine"] == "single"
        assert plan["raster"]["eligible"] is True
        assert plan["raster"]["reason"] == "rainbow_match_none"

    @patch("handler_render_plan._storage_call")
    def test_explicit_mt_request_routes_solve_score_to_mt(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 5, "n_chunks": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
        })
        from handler_render_plan import handler
        result = handler(_make_event(
            color_mode="solve_score",
            solve_metric="crowding",
            solve_score_quantile=0.01,
            solve_score_omega=4,
            raster_engine="mt",
        ), None)
        plan = json.loads(result["body"])
        assert plan["raster"]["requested_engine"] == "mt"
        assert plan["raster"]["engine"] == "mt"
        assert plan["raster"]["function_name"] == "polypaint-raster-mt"
        assert plan["raster"]["eligible"] is True
        assert plan["raster"]["reason"] == "solve_score"

    @patch("handler_render_plan._storage_call")
    def test_explicit_mt_request_routes_constant_to_mt(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({"degree": 5, "n_chunks": 2})
        from handler_render_plan import handler
        result = handler(_make_event(color_mode="constant", raster_engine="mt"), None)
        plan = json.loads(result["body"])
        assert plan["raster"]["requested_engine"] == "mt"
        assert plan["raster"]["engine"] == "mt"
        assert plan["raster"]["reason"] == "constant"

    @patch("handler_render_plan._storage_call")
    def test_explicit_mt_request_routes_rainbow_match_none_to_mt(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({"degree": 5, "n_chunks": 2})
        from handler_render_plan import handler
        result = handler(_make_event(color_mode="rainbow", match_mode="none", raster_engine="mt"), None)
        plan = json.loads(result["body"])
        assert plan["raster"]["requested_engine"] == "mt"
        assert plan["raster"]["engine"] == "mt"
        assert plan["raster"]["reason"] == "rainbow_match_none"

    @patch("handler_render_plan._storage_call")
    def test_rainbow_greedy_stays_single_threaded(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({"degree": 5, "n_chunks": 2})
        from handler_render_plan import handler
        result = handler(_make_event(color_mode="rainbow", match_mode="greedy"), None)
        plan = json.loads(result["body"])
        assert plan["raster"]["engine"] == "single"
        assert plan["raster"]["function_name"] == "polypaint-raster"
        assert plan["raster"]["eligible"] is False
        assert plan["raster"]["reason"] == "rainbow_match_greedy"

    @patch("handler_render_plan._storage_call")
    def test_explicit_mt_request_for_unsupported_mode_falls_back_to_single(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({"degree": 5, "n_chunks": 2})
        from handler_render_plan import handler
        result = handler(_make_event(color_mode="proximity", raster_engine="mt"), None)
        plan = json.loads(result["body"])
        assert plan["raster"]["requested_engine"] == "mt"
        assert plan["raster"]["engine"] == "single"
        assert plan["raster"]["eligible"] is False
        assert plan["raster"]["reason"] == "mt_requested_but_proximity_single_thread_only"

    @patch("handler_render_plan._storage_call")
    def test_proximity_stays_single_threaded(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({"degree": 5, "n_chunks": 2})
        from handler_render_plan import handler
        result = handler(_make_event(color_mode="proximity"), None)
        plan = json.loads(result["body"])
        assert plan["raster"]["engine"] == "single"
        assert plan["raster"]["eligible"] is False
        assert plan["raster"]["reason"] == "proximity_single_thread_only"

    def test_plan_does_not_dispatch_or_poll(self):
        """Plan Lambda must not contain dispatch/poll/phase-transition code."""
        import handler_render_plan as mod
        source = open(mod.__file__).read()
        assert "_dispatch_async" not in source
        assert "_poll_completion" not in source
        assert "_check_timeout" not in source
        assert "InvocationType" not in source or "RequestResponse" in source  # only sync invoke allowed


if __name__ == "__main__":
    unittest.main()
