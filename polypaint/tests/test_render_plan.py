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
        assert plan["chunk_items"][0] == {"chunk_idx": 0}

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
        assert "crowding" in plan["solve_score"]["clip_key"]
        assert "crowding" in plan["solve_score"]["bins_key"]
        assert plan["outputs"]["repalette_capable"] is True
        assert plan["outputs"]["metadata"]["view_mode"] == "square"
        assert plan["outputs"]["metadata"]["square_extent"] == "2.0"
        assert plan["outputs"]["metadata"]["rotation"] == "0"
        assert plan["outputs"]["metadata"]["solve_score_omega"] == "4.0"
        assert plan["outputs"]["metadata"]["background_color"] == "000000"
        assert plan["outputs"]["metadata"]["background_threshold"] == "4"
        assert plan["outputs"]["metadata"]["repalette_capable"] == "true"
        assert plan["outputs"]["metadata"]["pixel_bins_prefix"] == "renders/j/color/color_run_t/pixel_bins/tile_"
        assert plan["outputs"]["metadata"]["pixel_bins_layout"] == "tile_u8_v1"
        assert plan["grid"]["pixel_bin_tile_keys"][0] == "renders/j/color/color_run_t/pixel_bins/tile_0000.bin"

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
        assert plan["outputs"]["metadata"]["repalette_capable"] == "true"
        assert plan["outputs"]["metadata"]["pixel_bins_prefix"] == "renders/j/color/color_run_t/pixel_bins/tile_"
        assert plan["params"]["palette"] == "inferno"
        assert plan["params"]["root_transforms"] == [["rotate_roots", "0.25"]]

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
