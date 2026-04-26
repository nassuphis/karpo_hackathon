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
        self.assertEqual(plan["calc"]["n_sections"], 3)
        self.assertEqual(plan["calc"]["pass0_steps"], 16)
        self.assertEqual(
            plan["solve_score"]["chain"],
            [
                ["crowding", "1"],
                ["omega_cosine", "3"],
            ],
        )
        self.assertEqual(plan["solve_score"]["omega"], 3.0)
        self.assertTrue(plan["solve_score"]["omega_enabled"])
        self.assertEqual(
            plan["params"]["solve_score_chain"],
            [
                ["crowding", "1"],
                ["omega_cosine", "3"],
            ],
        )
        self.assertEqual(plan["params"]["solve_score_omega"], 3.0)
        self.assertTrue(plan["params"]["solve_score_omega_enabled"])
        self.assertEqual(plan["params"]["solve_score_threads"], 1)
        self.assertEqual(plan["params"]["solve_score_hist_input_mode"], "tmpfile")
        self.assertEqual(plan["params"]["solve_score_hist_retries"], 2)
        self.assertEqual(plan["params"]["solve_score_merge_workers"], 16)
        self.assertEqual(plan["params"]["palette_chunk_threads"], 4)
        self.assertEqual(plan["params"]["palette_chunk_input_mode"], "sectioned")
        self.assertEqual(plan["params"]["palette_chunk_retries"], 2)
        self.assertEqual(plan["params"]["palette_chunk_workers"], 16)
        self.assertEqual(
            plan["section_items"],
            [
                {
                    "section_idx": 0,
                    "section_count": 3,
                    "bin_key": "renders/j/chunk_0.bin",
                    "coeffs_key": "renders/j/coeffs_0000.bin",
                    "step_start": 0,
                    "step_count": 5,
                    "params_key": "renders/j/params.bin",
                    "params_step_start": 0,
                    "params_step_count": 5,
                    "params_bin_size": 5 * 16,
                    "bin_size": 5 * 5 * 2 * 4,
                    "coeffs_bin_size": 5 * 6 * 2 * 4,
                },
                {
                    "section_idx": 1,
                    "section_count": 3,
                    "bin_key": "renders/j/chunk_1.bin",
                    "coeffs_key": "renders/j/coeffs_0001.bin",
                    "step_start": 5,
                    "step_count": 17,
                    "params_key": "renders/j/params.bin",
                    "params_step_start": 5,
                    "params_step_count": 17,
                    "params_bin_size": 17 * 16,
                    "bin_size": 17 * 5 * 2 * 4,
                    "coeffs_bin_size": 17 * 6 * 2 * 4,
                },
                {
                    "section_idx": 2,
                    "section_count": 3,
                    "bin_key": "renders/j/chunk_2.bin",
                    "coeffs_key": "renders/j/coeffs_0002.bin",
                    "step_start": 22,
                    "step_count": 10,
                    "params_key": "renders/j/params.bin",
                    "params_step_start": 22,
                    "params_step_count": 10,
                    "params_bin_size": 10 * 16,
                    "bin_size": 10 * 5 * 2 * 4,
                    "coeffs_bin_size": 10 * 6 * 2 * 4,
                },
            ],
        )
        self.assertNotIn("palette_items", plan)
        self.assertTrue(plan["palette_id"].startswith("pal_"))
        self.assertEqual(plan["outputs"]["section_scores_prefix"], f"renders/j/palettes/{plan['palette_id']}/chunks/score_section_")
        self.assertEqual(plan["outputs"]["section_bins_prefix"], f"renders/j/palettes/{plan['palette_id']}/chunks/palette_bins_section_")
        self.assertEqual(plan["outputs"]["section_meta_prefix"], f"renders/j/palettes/{plan['palette_id']}/chunks/meta_section_")
        self.assertEqual(plan["outputs"]["chunk_scores_prefix"], plan["outputs"]["section_scores_prefix"])
        self.assertEqual(plan["outputs"]["chunk_bins_prefix"], plan["outputs"]["section_bins_prefix"])
        self.assertEqual(plan["outputs"]["chunk_meta_prefix"], plan["outputs"]["section_meta_prefix"])
        self.assertNotIn("score_key", plan["outputs"])
        self.assertNotIn("palette_bins_key", plan["outputs"])

    @patch("handler_palette_render_plan.s3")
    def test_plan_supports_lagged_solve_score_programs(self, mock_s3):
        from handler_palette_render_plan import handler

        calc = {
            "degree": 5,
            "N": 4,
            "times": 2,
            "lores": {"bin_key": "renders/j/lores.bin"},
            "chunks": [{"idx": 0, "bin_key": "renders/j/chunk_0.bin", "n_t": 16}],
        }
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: json.dumps(calc).encode())}

        result = handler(_event(params={
                "metric": "proximity",
                "palette": "reef",
                "solve_score_chain": [["proximity", "slv", "1"], ["proximity", "slv-1", "1"], ["abs_diff"]],
            }), None)
        plan = json.loads(result["body"])
        self.assertTrue(plan["logical_section"])
        self.assertEqual(plan["params"]["solve_score_hist_input_mode"], "sectioned")
        self.assertEqual(plan["params"]["palette_chunk_input_mode"], "sectioned")
        self.assertEqual(plan["solve_score"]["program"], "m0-0;m0-1;abs_diff")
        self.assertTrue(plan["solve_score"]["uses_lag"])
        self.assertEqual(plan["solve_score"]["prelude_rows"], 1)
        self.assertEqual(plan["solve_score"]["score_coeff_prelude_rows"], 0)
        self.assertEqual(plan["solve_score"]["score_param_prelude_rows"], 0)
        self.assertIn("s", plan["solve_source_manifest"])

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

        self.assertEqual(plan["section_items"][0]["step_count"], step_count)
        self.assertEqual(plan["section_items"][0]["bin_size"], record_bytes * step_count)
        self.assertEqual(plan["calc"]["n_chunks"], 1)
        self.assertEqual(plan["calc"]["n_sections"], 1)

    @patch("handler_palette_render_plan.s3")
    def test_extract_plan_accepts_palette_chunk_execution_knobs(self, mock_s3):
        from handler_palette_render_plan import handler

        calc = {
            "degree": 5,
            "N": 4,
            "times": 1,
            "lores": {"bin_key": "renders/j/lores.bin"},
            "chunks": [{"idx": 0, "bin_key": "renders/j/chunk_0.bin", "n_t": 16}],
        }
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: json.dumps(calc).encode())}

        result = handler(_event(params={
            "metric": "crowding",
            "palette": "reef",
            "solve_score_quantile": 0.01,
            "solve_score_omega": 3,
            "root_transforms": [],
            "palette_chunk_threads": 6,
            "palette_chunk_input_mode": "tmpfile",
            "palette_chunk_retries": 5,
            "palette_chunk_workers": 32,
        }), None)
        plan = json.loads(result["body"])
        self.assertEqual(plan["params"]["palette_chunk_threads"], 6)
        self.assertEqual(plan["params"]["palette_chunk_input_mode"], "tmpfile")
        self.assertEqual(plan["params"]["palette_chunk_retries"], 5)
        self.assertEqual(plan["params"]["palette_chunk_workers"], 32)

    @patch("handler_palette_render_plan.s3")
    def test_plan_accepts_solve_score_chain_input(self, mock_s3):
        from handler_palette_render_plan import handler

        calc = {
            "degree": 5,
            "N": 4,
            "times": 1,
            "lores": {"bin_key": "renders/j/lores.bin"},
            "chunks": [{"idx": 0, "bin_key": "renders/j/chunk_0.bin", "n_t": 16}],
        }
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: json.dumps(calc).encode())}

        result = handler(_event(params={
            "metric": "crowding",
            "palette": "reef",
            "solve_score_chain": ["spread", ["omega_cosine", "5"]],
            "solve_score_quantile": 0.01,
            "solve_score_omega": 2,
            "root_transforms": [],
        }), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["solve_score"]["metric"], "spread")
        self.assertEqual(plan["solve_score"]["omega"], 5.0)
        self.assertTrue(plan["solve_score"]["omega_enabled"])
        self.assertEqual(
            plan["solve_score"]["chain"],
            [
                ["spread", "1"],
                ["omega_cosine", "5"],
            ],
        )
        self.assertEqual(
            plan["params"]["solve_score_chain"],
            [
                ["spread", "1"],
                ["omega_cosine", "5"],
            ],
        )

    @patch("handler_palette_render_plan.s3")
    def test_palette_plan_accepts_mixed_source_chain(self, mock_s3):
        from handler_palette_render_plan import handler

        calc = {
            "degree": 5,
            "N": 4,
            "times": 1,
            "n_coeffs": 7,
            "lores": {"bin_key": "renders/j/lores.bin", "coeffs_key": "renders/j/lores_coeffs.bin"},
            "chunks": [{"idx": 0, "bin_key": "renders/j/chunk_0.bin", "n_t": 16}],
        }
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: json.dumps(calc).encode())}

        result = handler(_event(params={
            "metric": "spread",
            "palette": "reef",
            "solve_score_chain": [["spread", "slv", "1"], ["spread", "cf", "1"], ["avg"]],
            "solve_score_quantile": 0.01,
            "root_transforms": [],
        }), None)
        plan = json.loads(result["body"])
        self.assertEqual(plan["calc"]["lores_coeffs_key"], "renders/j/lores_coeffs.bin")
        self.assertEqual(plan["calc"]["n_coeffs"], 7)
        self.assertEqual(plan["solve_score"]["metrics"][0]["source"], "slv")
        self.assertEqual(plan["solve_score"]["metrics"][1]["source"], "cf")

    @patch("handler_palette_render_plan.s3")
    def test_palette_generation_rejects_explicit_output_chain(self, mock_s3):
        from handler_palette_render_plan import handler

        with self.assertRaisesRegex(RuntimeError, "explicit emit/emit_norm outputs are color-render only"):
            handler(_event(params={
                "metric": "proximity",
                "palette": "reef",
                "solve_score_chain": [
                    ["proximity", "0.1"],
                    ["emit_norm"],
                    ["spread", "0.1"],
                    ["emit_norm"],
                    ["angular_entropy_16", "0.1"],
                    ["emit_norm"],
                ],
            }), None)

        mock_s3.get_object.assert_not_called()

    @patch("handler_palette_render_plan.s3")
    def test_extract_plan_rejects_direct_rgb_color_artifact(self, mock_s3):
        from handler_palette_render_plan import handler

        def head_object(**kwargs):
            if kwargs["Key"] != "renders/j/color/color_rgb/image.jpeg":
                raise AssertionError(f"unexpected head_object key: {kwargs['Key']}")
            return {
                "Metadata": {
                    "artifact_id": "color_rgb",
                    "family": "color",
                    "color_mode": "solve_score",
                    "raw_channels": "3",
                    "score_output_channel_count": "3",
                    "score_output_interpretation": "direct_rgb",
                    "solve_metric": "proximity",
                    "solve_score_chain": '[["proximity","0.1"],["emit_norm"],["spread","0.1"],["emit_norm"],["angular_entropy_16","0.1"],["emit_norm"]]',
                    "solve_score_quantile": "0.01",
                    "solve_score_omega": "1",
                    "solve_score_omega_enabled": "true",
                    "palette": "reef",
                    "root_transforms": "[]",
                }
            }

        mock_s3.head_object.side_effect = head_object
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: b"{}")}

        with self.assertRaisesRegex(RuntimeError, "multi-output/direct-RGB artifacts are not palette-extractable"):
            handler(_event(artifact_id="color_rgb"), None)

    @patch("handler_palette_render_plan.s3")
    def test_palette_plan_publicizes_generic_metric_chain(self, mock_s3):
        from handler_palette_render_plan import handler

        calc = {
            "degree": 5,
            "N": 4,
            "times": 1,
            "n_coeffs": 7,
            "lores": {"bin_key": "renders/j/lores.bin", "coeffs_key": "renders/j/lores_coeffs.bin"},
            "chunks": [{"idx": 0, "bin_key": "renders/j/chunk_0.bin", "n_t": 16}],
        }
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: json.dumps(calc).encode())}

        result = handler(_event(params={
            "metric": "angular_entropy_16",
            "palette": "reef",
            "solve_score_chain": [["metric", "angular_entropy_16", "cf", "0.5"]],
            "solve_score_quantile": 0.005,
            "root_transforms": [],
        }), None)
        plan = json.loads(result["body"])

        expected_chain = [["metric", "angular_entropy_16", "cf", "0.5"]]
        self.assertEqual(plan["params"]["solve_score_chain"], expected_chain)
        self.assertEqual(plan["solve_score"]["chain"], expected_chain)
        self.assertNotIn("__metric", result["body"])

    @patch("handler_palette_render_plan.s3")
    def test_palette_plan_accepts_param_source_chain(self, mock_s3):
        from handler_palette_render_plan import handler

        calc = {
            "degree": 5,
            "N": 4,
            "times": 1,
            "params_key": "renders/j/params.bin",
            "lores": {"bin_key": "renders/j/lores.bin", "params_key": "renders/j/lores_params.bin"},
            "chunks": [{"idx": 0, "bin_key": "renders/j/chunk_0.bin", "n_t": 16}],
        }
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: json.dumps(calc).encode())}

        result = handler(_event(params={
            "metric": "t1_abs",
            "palette": "reef",
            "solve_score_chain": [["t1_abs", "pm", "1"], ["spread", "1"], ["avg"]],
            "solve_score_quantile": 0.01,
            "root_transforms": [],
        }), None)
        plan = json.loads(result["body"])
        self.assertEqual(plan["calc"]["lores_params_key"], "renders/j/lores_params.bin")
        self.assertEqual(plan["calc"]["params_key"], "renders/j/params.bin")
        self.assertEqual(plan["section_items"][0]["params_key"], "renders/j/params.bin")
        self.assertEqual(plan["section_items"][0]["params_step_start"], 0)
        self.assertEqual(plan["section_items"][0]["params_step_count"], 16)
        self.assertEqual(plan["solve_score"]["metrics"][0]["source"], "pm")
        self.assertEqual(plan["solve_score"]["metrics"][0]["metric"], "t1_abs")

    @patch("handler_palette_render_plan.s3")
    def test_palette_plan_accepts_chunked_param_source_chain(self, mock_s3):
        from handler_palette_render_plan import handler

        calc = {
            "degree": 5,
            "N": 4,
            "times": 1,
            "param_storage_mode": "chunked",
            "params_key": "",
            "lores": {"bin_key": "renders/j/lores.bin", "params_key": "renders/j/lores_params.bin"},
            "chunks": [{
                "idx": 0,
                "bin_key": "renders/j/chunk_0.bin",
                "n_t": 16,
                "params_key": "renders/j/params_0000.bin",
                "params_step_start": 0,
                "params_step_count": 16,
            }],
        }
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: json.dumps(calc).encode())}

        result = handler(_event(params={
            "metric": "t1_abs",
            "palette": "reef",
            "solve_score_chain": [["t1_abs", "pm", "1"], ["spread", "1"], ["avg"]],
            "solve_score_quantile": 0.01,
            "root_transforms": [],
        }), None)
        plan = json.loads(result["body"])
        self.assertEqual(plan["calc"]["params_key"], "")
        self.assertEqual(plan["calc"]["param_storage_mode"], "chunked")
        self.assertEqual(plan["section_items"][0]["params_key"], "renders/j/params_0000.bin")
        self.assertEqual(plan["section_items"][0]["params_step_start"], 0)

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

    @patch("handler_palette_render_plan.s3")
    def test_plan_accepts_disabled_omega(self, mock_s3):
        from handler_palette_render_plan import handler

        calc = {
            "degree": 5,
            "N": 4,
            "times": 1,
            "lores": {"bin_key": "renders/j/lores.bin"},
            "chunks": [{"idx": 0, "bin_key": "renders/j/chunk_0.bin", "n_t": 16}],
        }
        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: json.dumps(calc).encode())}

        result = handler(_event(params={"metric": "proximity", "palette": "inferno", "solve_score_quantile": 0.01, "solve_score_omega": 7, "solve_score_omega_enabled": False, "root_transforms": []}), None)
        plan = json.loads(result["body"])
        self.assertFalse(plan["solve_score"]["omega_enabled"])
        self.assertFalse(plan["params"]["solve_score_omega_enabled"])

    @patch("handler_palette_render_plan.s3")
    def test_extract_plan_short_circuits_when_associated_palette_already_exists(self, mock_s3):
        from handler_palette_render_plan import handler

        mock_s3.head_object.return_value = {
            "Metadata": {
                "artifact_id": "color_assoc",
                "family": "color",
                "associated_palette_mode": "generated",
                "associated_palette_id": "pal_assoc",
                "associated_palette_display_name": "spread q=1.0% w=4 inferno",
                "associated_palette_image_key": "renders/j/palettes/pal_assoc/image.jpeg",
                "associated_palette_preview_key": "renders/j/palettes/pal_assoc/preview.png",
                "associated_palette_palette": "inferno",
                "associated_palette_metric": "spread",
                "associated_palette_quantile": "0.01",
                "associated_palette_omega": "4",
                "associated_palette_omega_enabled": "false",
            }
        }

        result = handler(_event(artifact_id="color_assoc"), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["mode"], "extract_palette")
        self.assertEqual(plan["extract"]["action"], "done")
        self.assertFalse(plan["attach"]["enabled"])
        self.assertEqual(plan["palette_id"], "pal_assoc")
        self.assertEqual(plan["outputs"]["image_key"], "renders/j/palettes/pal_assoc/image.jpeg")

    @patch("handler_palette_render_plan.s3")
    def test_extract_plan_attaches_saved_palette_dependency(self, mock_s3):
        from handler_palette_render_plan import handler

        def head_object(**kwargs):
            if kwargs["Key"] != "renders/j/color/color_saved/image.jpeg":
                raise AssertionError(f"unexpected head_object key: {kwargs['Key']}")
            return {
                "Metadata": {
                    "artifact_id": "color_saved",
                    "family": "color",
                    "color_mode": "saved_palette",
                    "palette_source_id": "pal_src",
                    "palette_source_display_name": "crowding q=1.0% w=off magma",
                    "palette_source_palette": "magma",
                    "palette_source_metric": "crowding",
                    "palette_source_score_chain": '["crowding"]',
                    "palette_source_quantile": "0.01",
                    "palette_source_omega": "6",
                    "palette_source_omega_enabled": "false",
                }
            }

        def get_object(**kwargs):
            if kwargs["Key"] != "renders/j/palettes/pal_src/meta.json":
                raise AssertionError(f"unexpected get_object key: {kwargs['Key']}")
            meta = {
                "job_id": "j",
                "palette_id": "pal_src",
                "display_name": "crowding q=1.0% w=off magma",
                "image_key": "renders/j/palettes/pal_src/image.jpeg",
                "preview_key": "renders/j/palettes/pal_src/preview.png",
                "metric": "crowding",
                "palette": "magma",
                "solve_score_omega_enabled": False,
            }
            return {"Body": MagicMock(read=lambda: json.dumps(meta).encode())}

        mock_s3.head_object.side_effect = head_object
        mock_s3.get_object.side_effect = get_object

        result = handler(_event(artifact_id="color_saved"), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["extract"]["action"], "attach")
        self.assertEqual(plan["attach"]["mode"], "dependency")
        self.assertEqual(plan["palette_id"], "pal_src")
        self.assertEqual(plan["attach"]["palette"], "magma")
        self.assertEqual(plan["attach"]["score_chain"], [["crowding", "1"]])
        self.assertFalse(plan["attach"]["omega_enabled"])

    @patch("handler_palette_render_plan.s3")
    def test_extract_plan_reuses_matching_solve_score_scratch_without_cleanup(self, mock_s3):
        from handler_palette_render_plan import handler

        def head_object(**kwargs):
            mapping = {
                "renders/j/color/color_child/image.jpeg": {
                    "Metadata": {
                        "artifact_id": "color_child",
                        "family": "color",
                        "postprocess_kind": "autolevels",
                        "derived_from_artifact_id": "color_src",
                    }
                },
                "renders/j/color/color_src/image.jpeg": {
                    "Metadata": {
                        "artifact_id": "color_src",
                        "family": "color",
                        "color_mode": "solve_score",
                        "solve_metric": "spread",
                        "solve_score_quantile": "0.02",
                        "solve_score_omega": "6",
                        "solve_score_omega_enabled": "true",
                        "palette": "magma",
                        "root_transforms": "[]",
                    }
                },
            }
            if kwargs["Key"] not in mapping:
                raise AssertionError(f"unexpected head_object key: {kwargs['Key']}")
            return mapping[kwargs["Key"]]

        def get_object(**kwargs):
            mapping = {
                "renders/j/calc.json": {
                    "degree": 5,
                    "N": 4,
                    "times": 1,
                    "lores": {"bin_key": "renders/j/lores.bin"},
                    "chunks": [{"idx": 0, "bin_key": "renders/j/chunk_0.bin", "n_t": 16}],
                },
                "renders/j/solve_scores/spread_rt97d170e1/clip.json": {
                    "family": "solve_score",
                    "metric": "spread",
                    "clip_quantile": 0.02,
                    "omega": 6.0,
                    "omega_enabled": True,
                    "root_transforms": [],
                },
                "renders/j/solve_scores/spread_rt97d170e1/bins.json": {
                    "family": "solve_score",
                    "metric": "spread",
                    "clip_quantile": 0.02,
                    "omega": 6.0,
                    "omega_enabled": True,
                    "root_transforms": [],
                },
            }
            if kwargs["Key"] not in mapping:
                raise AssertionError(f"unexpected get_object key: {kwargs['Key']}")
            return {"Body": MagicMock(read=lambda data=mapping[kwargs["Key"]]: json.dumps(data).encode())}

        mock_s3.head_object.side_effect = head_object
        mock_s3.get_object.side_effect = get_object

        result = handler(_event(artifact_id="color_child"), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["extract"]["action"], "generate_reuse")
        self.assertEqual(plan["extract"]["source_artifact_id"], "color_src")
        self.assertEqual(plan["attach"]["artifact_id"], "color_child")
        self.assertFalse(plan["solve_score"]["cleanup_scratch"])
        self.assertEqual(plan["solve_score"]["clip_key"], "renders/j/solve_scores/spread_rt97d170e1/clip.json")
        self.assertEqual(plan["solve_score"]["bins_key"], "renders/j/solve_scores/spread_rt97d170e1/bins.json")

    @patch("handler_palette_render_plan.s3")
    def test_extract_plan_attaches_inherited_associated_palette_from_parent(self, mock_s3):
        from handler_palette_render_plan import handler

        def head_object(**kwargs):
            mapping = {
                "renders/j/color/color_child/image.jpeg": {
                    "Metadata": {
                        "artifact_id": "color_child",
                        "family": "color",
                        "postprocess_kind": "resize",
                        "derived_from_artifact_id": "color_src",
                    }
                },
                "renders/j/color/color_src/image.jpeg": {
                    "Metadata": {
                        "artifact_id": "color_src",
                        "family": "color",
                        "color_mode": "solve_score",
                        "associated_palette_mode": "generated",
                        "associated_palette_id": "pal_src",
                        "associated_palette_display_name": "spread q=1.0% w=4 inferno",
                        "associated_palette_image_key": "renders/j/palettes/pal_src/image.jpeg",
                        "associated_palette_preview_key": "renders/j/palettes/pal_src/preview.png",
                        "associated_palette_palette": "inferno",
                        "associated_palette_metric": "spread",
                        "associated_palette_score_chain": '["spread"]',
                        "associated_palette_quantile": "0.01",
                        "associated_palette_omega": "4",
                        "associated_palette_omega_enabled": "false",
                    }
                },
            }
            if kwargs["Key"] not in mapping:
                raise AssertionError(f"unexpected head_object key: {kwargs['Key']}")
            return mapping[kwargs["Key"]]

        mock_s3.head_object.side_effect = head_object

        result = handler(_event(artifact_id="color_child"), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["extract"]["action"], "attach")
        self.assertEqual(plan["extract"]["reason"], "inherit_existing_association")
        self.assertEqual(plan["attach"]["artifact_id"], "color_child")
        self.assertEqual(plan["attach"]["mode"], "generated")
        self.assertEqual(plan["attach"]["palette_id"], "pal_src")
        self.assertEqual(plan["attach"]["image_key"], "renders/j/palettes/pal_src/image.jpeg")
        self.assertEqual(plan["attach"]["preview_key"], "renders/j/palettes/pal_src/preview.png")
        self.assertEqual(plan["attach"]["score_chain"], [["spread", "1"]])
        self.assertFalse(plan["attach"]["omega_enabled"])

    @patch("handler_palette_render_plan.s3")
    def test_extract_plan_reruns_prepass_when_scratch_missing(self, mock_s3):
        from handler_palette_render_plan import handler

        def head_object(**kwargs):
            if kwargs["Key"] != "renders/j/color/color_src/image.jpeg":
                raise AssertionError(f"unexpected head_object key: {kwargs['Key']}")
            return {
                "Metadata": {
                    "artifact_id": "color_src",
                    "family": "color",
                    "color_mode": "solve_score",
                    "solve_metric": "clusteriness",
                    "solve_score_chain": '["clusteriness",["omega_cosine","3"]]',
                    "solve_score_quantile": "0.01",
                    "solve_score_omega": "3",
                    "solve_score_omega_enabled": "true",
                    "palette": "reef",
                    "root_transforms": '[["rotate_roots","0.25"]]',
                }
            }

        def get_object(**kwargs):
            if kwargs["Key"] == "renders/j/calc.json":
                calc = {
                    "degree": 5,
                    "N": 4,
                    "times": 1,
                    "lores": {"bin_key": "renders/j/lores.bin"},
                    "chunks": [{"idx": 0, "bin_key": "renders/j/chunk_0.bin", "n_t": 16}],
                }
                return {"Body": MagicMock(read=lambda: json.dumps(calc).encode())}
            raise RuntimeError("NoSuchKey")

        mock_s3.head_object.side_effect = head_object
        mock_s3.get_object.side_effect = get_object

        result = handler(_event(artifact_id="color_src"), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["extract"]["action"], "generate_full")
        self.assertTrue(plan["solve_score"]["cleanup_scratch"])
        self.assertEqual(
            plan["solve_score"]["chain"],
            [
                ["clusteriness", "1"],
                ["omega_cosine", "3"],
            ],
        )
        self.assertIn("/palettes/", plan["solve_score"]["clip_key"])
        self.assertIn("/palettes/", plan["solve_score"]["bins_key"])
        self.assertEqual(plan["params"]["root_transforms"], [["rotate_roots", "0.25"]])

    @patch("handler_palette_render_plan.s3")
    def test_extract_plan_preserves_requested_execution_knobs(self, mock_s3):
        from handler_palette_render_plan import handler

        def head_object(**kwargs):
            if kwargs["Key"] != "renders/j/color/color_src/image.jpeg":
                raise AssertionError(f"unexpected head_object key: {kwargs['Key']}")
            return {
                "Metadata": {
                    "artifact_id": "color_src",
                    "family": "color",
                    "color_mode": "solve_score",
                    "solve_metric": "spread",
                    "solve_score_quantile": "0.02",
                    "solve_score_omega": "6",
                    "solve_score_omega_enabled": "true",
                    "palette": "magma",
                    "root_transforms": "[]",
                }
            }

        def get_object(**kwargs):
            if kwargs["Key"] == "renders/j/calc.json":
                calc = {
                    "degree": 5,
                    "N": 4,
                    "times": 1,
                    "lores": {"bin_key": "renders/j/lores.bin"},
                    "chunks": [{"idx": 0, "bin_key": "renders/j/chunk_0.bin", "n_t": 16}],
                }
                return {"Body": MagicMock(read=lambda: json.dumps(calc).encode())}
            raise RuntimeError("NoSuchKey")

        mock_s3.head_object.side_effect = head_object
        mock_s3.get_object.side_effect = get_object

        result = handler(_event(
            artifact_id="color_src",
            params={
                "solve_score_threads": 8,
                "solve_score_hist_input_mode": "sectioned",
                "solve_score_hist_retries": 5,
                "solve_score_merge_workers": 24,
            },
        ), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["params"]["solve_score_threads"], 8)
        self.assertEqual(plan["params"]["solve_score_hist_input_mode"], "sectioned")
        self.assertEqual(plan["params"]["solve_score_hist_retries"], 5)
        self.assertEqual(plan["params"]["solve_score_merge_workers"], 24)


if __name__ == "__main__":
    unittest.main()
