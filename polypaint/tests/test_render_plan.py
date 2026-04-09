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
        assert plan["chunk_items"][0] == {"chunk_idx": 0, "bin_key": "renders/j/chunk_0.bin"}

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
            {"chunk_idx": 0, "bin_key": "renders/j/custom/chunk_zero.bin"},
            {"chunk_idx": 1, "bin_key": "renders/j/custom/chunk_one.bin"},
            {"chunk_idx": 2, "bin_key": "renders/j/custom/chunk_two.bin"},
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
        assert len(plan["grid"]["tile_keys"]) == 4
        assert plan["grid"]["tile_keys"][0] == "renders/j/tile_0000.raw"
        assert len(plan["tile_items"]) == 4
        assert plan["tile_items"][0]["tile_w"] == 512
        assert plan["tile_items"][0]["tile_h"] == 512

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
        assert plan["solve_score"]["quantile"] == 0.01
        assert plan["solve_score"]["omega"] == 4.0
        assert plan["solve_score"]["omega_enabled"] is True
        assert plan["solve_score"]["threads"] == 1
        assert plan["solve_score"]["hist_input_mode"] == "tmpfile"
        assert plan["solve_score"]["hist_retries"] == 2
        assert "crowding" in plan["solve_score"]["clip_key"]
        assert "crowding" in plan["solve_score"]["bins_key"]
        assert plan["outputs"]["repalette_capable"] is True
        assert plan["outputs"]["metadata"]["view_mode"] == "square"
        assert plan["outputs"]["metadata"]["square_extent"] == "2.0"
        assert plan["outputs"]["metadata"]["rotation"] == "0"
        assert plan["outputs"]["metadata"]["solve_score_omega"] == "4.0"
        assert plan["outputs"]["metadata"]["solve_score_omega_enabled"] == "true"
        assert plan["outputs"]["metadata"]["background_color"] == "000000"
        assert plan["outputs"]["metadata"]["background_threshold"] == "4"
        assert plan["outputs"]["metadata"]["repalette_capable"] == "true"
        assert plan["outputs"]["metadata"]["pixel_bins_prefix"] == "renders/j/color/color_run_t/pixel_bins/tile_"
        assert plan["outputs"]["metadata"]["pixel_bins_layout"] == "tile_u8_v1"
        assert plan["grid"]["pixel_bin_tile_keys"][0] == "renders/j/color/color_run_t/pixel_bins/tile_0000.bin"
        assert plan["raster"]["requested_engine"] == "single"
        assert plan["raster"]["engine"] == "single"
        assert plan["raster"]["requested_threads"] == 4
        assert plan["raster"]["requested_input_mode"] == "tmpfile"
        assert plan["raster"]["requested_sectioned_retries"] == 2
        assert plan["raster"]["input_mode"] == "tmpfile"
        assert plan["raster"]["sectioned_retries"] == 0
        assert plan["raster"]["threads"] == 1
        assert plan["raster"]["function_name"] == "polypaint-raster"
        assert plan["raster"]["eligible"] is True
        assert plan["raster"]["reason"] == "solve_score"

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
        assert plan["outputs"]["repalette_capable"] is True
        assert plan["outputs"]["metadata"]["color_mode"] == "saved_palette"
        assert plan["outputs"]["metadata"]["palette"] == "inferno"
        assert plan["outputs"]["metadata"]["palette_source_id"] == "pal_src"
        assert plan["outputs"]["metadata"]["palette_source_palette"] == "reef"
        assert plan["outputs"]["metadata"]["solve_metric"] == "crowding"
        assert plan["outputs"]["metadata"]["solve_score_quantile"] == "0.01"
        assert plan["outputs"]["metadata"]["solve_score_omega"] == "3.0"
        assert plan["outputs"]["metadata"]["solve_score_omega_enabled"] == "false"
        assert plan["outputs"]["metadata"]["palette_source_omega_enabled"] == "false"
        assert plan["outputs"]["metadata"]["repalette_capable"] == "true"
        assert plan["outputs"]["metadata"]["pixel_bins_prefix"] == "renders/j/color/color_run_t/pixel_bins/tile_"
        assert plan["params"]["palette"] == "inferno"
        assert plan["params"]["root_transforms"] == [["rotate_roots", "0.25"]]
        assert plan["raster"]["requested_engine"] == "single"
        assert plan["raster"]["engine"] == "single"
        assert plan["raster"]["reason"] == "saved_palette"

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
