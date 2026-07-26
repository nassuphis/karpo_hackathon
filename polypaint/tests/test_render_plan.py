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

from workflow_contracts import (
    FINALIZE_MT_ASSOCIATED_PALETTE_REQUIRED_FIELDS,
    FINALIZE_MT_METADATA_REQUIRED_FIELDS,
    RENDER_BILEVEL_FINALIZE_TASK_PAYLOAD,
    RENDER_BILEVEL_RASTER_ITEM_SELECTOR,
    RENDER_COLOR_CLIP_TASK_PAYLOAD,
    RENDER_COLOR_RASTER_ITEM_SELECTOR,
    RENDER_COEFF_FINALIZE_TASK_PAYLOAD,
    RENDER_COEFF_RASTER_ITEM_SELECTOR,
    RENDER_FINALIZE_MT_TASK_PAYLOAD,
    iter_jsonpath_values,
)


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


def _make_event(mode="color", pix=1024, **extra_params):
    params = {
        "pix": pix,
        "view_mode": "square",
        "square_extent": 2.0,
        "fmt": "jpeg",
        "quality": 90,
        "rotation": 0,
        "color_mode": "solve_score",
        "match_mode": "none",
        "palette": "inferno",
        "solve_score_chain": [["proximity", "0.1"]],
    }
    params.update(extra_params)
    return {
        "job_id": "j",
        "run_id": "run_t",
        "task_id": "render_run_color_run_t",
        "mode": mode,
        "params": params,
    }


def _assert_plan_path_exists(testcase, plan, jsonpath):
    if not jsonpath.startswith("$.plan."):
        return
    cur = plan
    for part in jsonpath[len("$.plan."):].split("."):
        testcase.assertIsInstance(cur, dict, f"{jsonpath} hit non-dict before {part!r}")
        testcase.assertIn(part, cur, f"missing planner field for contract path {jsonpath}")
        cur = cur[part]


class TestRenderPlanHelpers(unittest.TestCase):
    def test_solve_score_scratch_key_uses_root_program_fingerprint_when_available(self):
        from handler_render_plan import _solve_score_scratch_key

        compiled = {
            "metric": "proximity",
            "legacy_compatible": True,
            "chain": [["proximity", "0.1"]],
        }
        key = _solve_score_scratch_key(
            "job1",
            compiled,
            [["rotate_roots", "0.25"]],
            root_program_fingerprint="sha256:abcdef123456",
        )
        fallback = _solve_score_scratch_key(
            "job1",
            compiled,
            [["rotate_roots", "0.25"]],
        )

        self.assertIn("_rtabcdef12/", key)
        self.assertNotEqual(key, fallback)


class TestRenderPlan(unittest.TestCase):
    def setUp(self):
        import handler_render_plan as mod

        self._s3_patcher = patch.object(mod, "s3")
        self.mock_s3 = self._s3_patcher.start()
        self.addCleanup(self._s3_patcher.stop)

    @patch("handler_render_plan._storage_call")
    def test_color_plan_square_viewport_and_grid(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        result = handler(_make_event(), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["mode"], "color")
        self.assertEqual(plan["viewport"]["min_re"], -2.0)
        self.assertEqual(plan["viewport"]["max_re"], 2.0)
        self.assertEqual(plan["viewport"]["min_im"], -2.0)
        self.assertEqual(plan["viewport"]["max_im"], 2.0)
        self.assertEqual(plan["grid"]["pix"], 1024)
        self.assertNotIn("width", plan["grid"])
        self.assertNotIn("height", plan["grid"])
        self.assertEqual(plan["physical_source_items"], [])
        self.assertEqual(plan["calc"]["degree"], 5)
        self.assertEqual(plan["calc"]["n_coeffs"], 6)
        self.assertEqual(plan["calc"]["n_chunks"], 4)

    @patch("handler_render_plan._storage_call")
    def test_view_plan_inherits_full_grid_and_uses_views_namespace(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc(N=2000))
        from handler_render_plan import handler

        result = handler(_make_event(
            pix=8192,
            view_mode="explicit",
            min_re=-3.0,
            max_re=2.0,
            min_im=-1.5,
            max_im=4.0,
            view_projection="radial",
            view_vertical="t1",
            source_color_artifact_id="color_source_7",
        ), None)
        plan = json.loads(result["body"])

        # View dimensions come from the parameter square, not the unrelated
        # pixel size of the source Color artifact/request.
        self.assertEqual(plan["grid"]["pix"], 2000)
        self.assertEqual(plan["calc"]["N"], 2000)
        self.assertEqual(plan["params"]["pix"], 2000)
        self.assertEqual(plan["params"]["view_projection"], "radial")
        self.assertEqual(plan["params"]["view_vertical"], "t1")
        self.assertEqual(plan["params"]["source_color_artifact_id"], "color_source_7")
        self.assertEqual(plan["outputs"]["family"], "views")
        self.assertEqual(plan["outputs"]["artifact_id"], "view_run_t")
        self.assertEqual(plan["outputs"]["artifact_prefix"], "renders/j/views/view_run_t/")
        self.assertEqual(plan["outputs"]["image_key"], "renders/j/views/view_run_t/image.jpeg")
        meta = plan["outputs"]["metadata"]
        self.assertEqual(meta["family"], "views")
        self.assertEqual(meta["view_id"], "view_run_t")
        self.assertEqual(meta["source_artifact_id"], "color_source_7")
        self.assertEqual(meta["projection"], "radial")
        self.assertEqual(meta["vertical"], "t1")
        self.assertEqual(meta["lattice_n"], "2000")
        self.assertEqual(meta["pix"], "2000")

    @patch("handler_render_plan._storage_call")
    def test_view_plan_requires_source_color_artifact(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        with self.assertRaisesRegex(RuntimeError, "source_color_artifact_id"):
            handler(_make_event(view_projection="front"), None)

    @patch("handler_render_plan._storage_call")
    def test_view_plan_accepts_established_128_char_artifact_ids(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        source_id = "color_" + ("a" * 122)
        result = handler(_make_event(
            view_projection="front",
            source_color_artifact_id=source_id,
        ), None)
        plan = json.loads(result["body"])
        self.assertEqual(plan["params"]["source_color_artifact_id"], source_id)

    @patch("handler_render_plan._storage_call")
    def test_render_plan_rejects_oversized_pix(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        import handler_render_plan as mod

        with self.assertRaisesRegex(RuntimeError, "pix <= 32768"):
            mod.handler(_make_event(pix=32769), None)
        mock_storage.assert_not_called()

    @patch("handler_render_plan._storage_call")
    def test_color_plan_accepts_identity_palette(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        for palette in ("identity", "identity_hsv"):
            with self.subTest(palette=palette):
                result = handler(_make_event(palette=palette), None)
                plan = json.loads(result["body"])

                self.assertEqual(plan["params"]["palette"], palette)
                self.assertEqual(plan["outputs"]["metadata"]["palette"], palette)

    @patch("handler_render_plan._storage_call")
    def test_custom_palette_display_name_is_artifact_provenance(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        custom = "custom:879caa-aaa4a4-0e3057"
        result = handler(_make_event(
            palette=custom,
            palette_display_name="Night reef",
            save_associated_palette=True,
        ), None)
        plan = json.loads(result["body"])

        self.assertEqual(plan["params"]["palette"], custom)
        self.assertEqual(plan["params"]["palette_display_name"], "Night reef")
        self.assertEqual(
            plan["outputs"]["metadata"]["palette_display_name"],
            "Night reef",
        )
        self.assertEqual(
            plan["associated_palette"]["palette_display_name"],
            "Night reef",
        )
        self.assertIn("Night reef", plan["associated_palette"]["display_name"])

    @patch("handler_render_plan._storage_call")
    def test_builtin_palette_drops_spurious_display_name(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        plan = json.loads(handler(_make_event(
            palette="inferno",
            palette_display_name="misleading",
        ), None)["body"])

        self.assertEqual(plan["params"]["palette_display_name"], "")
        self.assertEqual(plan["outputs"]["metadata"]["palette_display_name"], "")

    @patch("handler_render_plan._storage_call")
    def test_color_plan_accepts_custom_background_color(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        result = handler(_make_event(background_color="#123456"), None)
        plan = json.loads(result["body"])
        default_plan = json.loads(handler(_make_event(), None)["body"])

        self.assertEqual(plan["params"]["background_color"], "123456")
        self.assertEqual(plan["outputs"]["metadata"]["background_color"], "123456")
        self.assertIn('"background_color":"123456"', plan["outputs"]["metadata"]["render_execution"])
        self.assertIn("sha256:", plan["outputs"]["plan_params_digest"])
        self.assertNotEqual(plan["outputs"]["plan_params_digest"], default_plan["outputs"]["plan_params_digest"])

    @patch("handler_render_plan._storage_call")
    def test_color_plan_rejects_invalid_background_color(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        with self.assertRaisesRegex(RuntimeError, "background_color must be 6-digit hex"):
            handler(_make_event(background_color="not-a-color"), None)

    @patch("handler_render_plan._invoke_sync")
    @patch("handler_render_plan._storage_call")
    def test_color_plan_auto_viewport(self, mock_storage, mock_invoke):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        mock_invoke.return_value = {"q_re": [0.0, 2.0], "q_im": [-1.0, 1.0]}
        from handler_render_plan import handler

        result = handler(_make_event(view_mode="auto"), None)
        plan = json.loads(result["body"])

        self.assertAlmostEqual(plan["viewport"]["min_re"], -0.05)
        self.assertAlmostEqual(plan["viewport"]["max_re"], 2.05)
        self.assertAlmostEqual(plan["viewport"]["min_im"], -1.05)
        self.assertAlmostEqual(plan["viewport"]["max_im"], 1.05)

    @patch("handler_render_plan._storage_call")
    @patch("handler_render_plan._invoke_sync")
    def test_color_plan_auto_viewport_degenerate_axis_uses_other_span(self, mock_invoke, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        mock_invoke.return_value = {"q_re": [1.0, 1.0], "q_im": [-1.0, 1.0], "scale_ref": 1950.4761904761904}
        from handler_render_plan import handler

        result = handler(_make_event(view_mode="auto"), None)
        plan = json.loads(result["body"])

        self.assertAlmostEqual(plan["viewport"]["min_re"], -0.05)
        self.assertAlmostEqual(plan["viewport"]["max_re"], 2.05)
        self.assertAlmostEqual(plan["viewport"]["min_im"], -1.05)
        self.assertAlmostEqual(plan["viewport"]["max_im"], 1.05)

    @patch("handler_render_plan._storage_call")
    def test_color_plan_explicit_viewport(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        result = handler(
            _make_event(
                view_mode="explicit",
                min_re=-3.5,
                max_re=1.25,
                min_im=-0.75,
                max_im=2.0,
            ),
            None,
        )
        plan = json.loads(result["body"])

        self.assertEqual(
            plan["viewport"],
            {"min_re": -3.5, "max_re": 1.25, "min_im": -0.75, "max_im": 2.0},
        )
        self.assertNotIn("min_re", plan["params"])
        self.assertNotIn("max_re", plan["params"])
        self.assertNotIn("min_im", plan["params"])
        self.assertNotIn("max_im", plan["params"])

    @patch("handler_render_plan._storage_call")
    def test_color_plan_rejects_unknown_view_mode(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        with self.assertRaisesRegex(RuntimeError, "unsupported view_mode"):
            handler(_make_event(view_mode="bogus_mode"), None)

    @patch("handler_render_plan._storage_call")
    def test_color_plan_rejects_width_height_contract_aliases(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        with self.assertRaisesRegex(RuntimeError, "no longer accepts width/height"):
            handler(_make_event(width=1024, height=1024), None)
        mock_storage.assert_not_called()

    @patch("handler_render_plan._storage_call")
    def test_fused_color_plan_emits_raw_output_contract(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        result = handler(_make_event(raster_workers=24, save_associated_palette=False), None)
        plan = json.loads(result["body"])

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
        self.assertTrue(plan["outputs"]["repalette_capable"])
        self.assertEqual(plan["render_execution"]["raster_workers"], 24)
        self.assertEqual(plan["solve_score"]["threads"], 4)
        self.assertEqual(
            set(plan["solve_score"].keys()),
            {
                "enabled",
                "threads",
                "chain",
                "source_text",
                "clip_key",
                "uses_lag",
                "max_lag",
                "prelude_by_source",
                "normalize",
            },
        )
        self.assertFalse(plan["solve_score"]["normalize"])
        self.assertEqual(
            set(plan["render_execution"].keys()),
            {
                "raster_engine",
                "save_associated_palette",
                "background_color",
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
        metadata = plan["outputs"]["metadata"]
        self.assertEqual(metadata["solve_score_normalize"], "false")
        for field_name in FINALIZE_MT_METADATA_REQUIRED_FIELDS:
            self.assertIn(field_name, metadata)
        self.assertEqual(metadata["color_mode"], "solve_score")
        self.assertTrue(str(metadata["background_color"]).strip())
        self.assertTrue(str(metadata["solve_score_chain_fingerprint"]).strip())
        self.assertTrue(str(metadata["score_program"]).strip())

    @patch("handler_render_plan._storage_call")
    def test_color_plan_emits_fields_required_by_shared_render_contracts(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        result = handler(_make_event(), None)
        plan = json.loads(result["body"])

        for contract in (
            RENDER_COLOR_CLIP_TASK_PAYLOAD,
            RENDER_COLOR_RASTER_ITEM_SELECTOR,
            RENDER_FINALIZE_MT_TASK_PAYLOAD,
        ):
            for jsonpath in iter_jsonpath_values(contract):
                _assert_plan_path_exists(self, plan, jsonpath)

    @patch("handler_render_plan._storage_call")
    def test_bilevel_plan_emits_fields_required_by_shared_render_contracts(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        result = handler(_make_event(mode="bilevel"), None)
        plan = json.loads(result["body"])

        for contract in (
            RENDER_BILEVEL_RASTER_ITEM_SELECTOR,
            RENDER_BILEVEL_FINALIZE_TASK_PAYLOAD,
        ):
            for jsonpath in iter_jsonpath_values(contract):
                _assert_plan_path_exists(self, plan, jsonpath)

    @patch("handler_render_plan._storage_call")
    def test_coeff_bilevel_plan_emits_fields_required_by_shared_render_contracts(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail({
            "degree": 22,
            "n_coeffs": 23,
            "coeffs_keys": ["renders/j/coeffs_0000.bin", "renders/j/coeffs_0001.bin"],
            "chunks": [
                {"idx": 0, "bin_key": "renders/j/chunk_0.bin", "step_start": 0, "step_count": 10},
                {"idx": 1, "bin_key": "renders/j/chunk_1.bin", "step_start": 10, "step_count": 10},
            ],
        })
        from handler_render_plan import handler

        result = handler(_make_event(mode="coeff_bilevel"), None)
        plan = json.loads(result["body"])

        for contract in (
            RENDER_COEFF_RASTER_ITEM_SELECTOR,
            RENDER_COEFF_FINALIZE_TASK_PAYLOAD,
        ):
            for jsonpath in iter_jsonpath_values(contract):
                _assert_plan_path_exists(self, plan, jsonpath)

    def test_shared_contract_jsonpath_extractor_finds_paths_inside_states_expressions(self):
        self.assertIn(
            "$.plan.coeff_bilevel.fragment_prefix",
            set(iter_jsonpath_values(RENDER_COEFF_RASTER_ITEM_SELECTOR)),
        )

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

        self.assertEqual(plan["solve_source_manifest"], {})
        self.assertEqual(
            plan["solve_source_manifest_key"],
            "renders/j/manifests/run_t/color_solve_source_manifest.json",
        )
        manifest_put = self.mock_s3.put_object.call_args.kwargs
        self.assertEqual(manifest_put["Bucket"], "polypaint")
        self.assertEqual(manifest_put["Key"], plan["solve_source_manifest_key"])
        manifest = json.loads(manifest_put["Body"].decode("utf-8"))["s"]
        self.assertGreater(manifest["slv"]["r"], 0)
        self.assertGreater(manifest["cf"]["r"], 0)
        self.assertGreater(manifest["pm"]["r"], 0)
        self.assertEqual(plan["calc"]["lores_coeffs_key"], "renders/j/lores_coeffs.bin")
        self.assertEqual(plan["calc"]["lores_params_key"], "renders/j/lores_params.bin")
        self.assertTrue(plan["associated_palette"]["enabled"])
        self.assertEqual(plan["associated_palette"]["mode"], "generated")
        self.assertEqual(plan["associated_palette"]["fragment_prefix"], "renders/j/palettes/pal_color_run_t/fragments/section_")
        assoc = plan["associated_palette"]
        for field_name in FINALIZE_MT_ASSOCIATED_PALETTE_REQUIRED_FIELDS:
            self.assertIn(field_name, assoc)
        for field_name in (
            "palette_id",
            "display_name",
            "image_key",
            "preview_key",
            "meta_key",
            "raw_key",
            "raw_meta_key",
            "fragment_prefix",
            "source_color_artifact_id",
            "metric",
            "palette",
            "score_chain",
        ):
            self.assertTrue(str(assoc[field_name]).strip(), field_name)

    @patch("handler_render_plan._storage_call")
    def test_fused_color_plan_publicizes_generic_metric_chain(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc(
            lores={
                "bin_key": "renders/j/lores.bin",
                "coeffs_key": "renders/j/lores_coeffs.bin",
            },
        ))
        from handler_render_plan import handler

        result = handler(_make_event(
            solve_score_chain=[["metric", "angular_entropy_16", "cf", "0.5"]],
            save_associated_palette=True,
        ), None)
        plan = json.loads(result["body"])

        expected_chain = [["metric", "angular_entropy_16", "cf", "0.5"]]
        self.assertEqual(plan["params"]["solve_score_chain"], expected_chain)
        self.assertEqual(plan["solve_score"]["chain"], expected_chain)
        self.assertEqual(plan["associated_palette"]["score_chain"], expected_chain)
        self.assertNotIn("__metric", result["body"])

    @patch("handler_render_plan._storage_call")
    def test_fused_color_plan_accepts_explicit_rgb_outputs(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        result = handler(_make_event(
            save_associated_palette=True,
            color_interpretation="rgb",
            solve_score_chain=[
                ["proximity", "0.1"],
                ["emit_norm"],
                ["spread", "0.1"],
                ["emit_norm"],
                ["angular_entropy_16", "0.1"],
                ["emit_norm"],
            ],
        ), None)
        plan = json.loads(result["body"])

        metadata = plan["outputs"]["metadata"]
        self.assertEqual(metadata["raw_channels"], "3")
        self.assertEqual(metadata["score_output_channel_count"], "3")
        self.assertEqual(metadata["score_output_interpretation"], "rgb")
        self.assertTrue(plan["associated_palette"]["enabled"])
        self.assertEqual(plan["associated_palette"]["mode"], "generated")
        self.assertEqual(plan["associated_palette"]["palette"], "")
        self.assertEqual(plan["associated_palette"]["fragment_prefix"], "renders/j/palettes/pal_color_run_t/fragments/section_")
        self.assertEqual(plan["associated_palette"]["raw_key"], "renders/j/palettes/pal_color_run_t/greyscale.raw")
        self.assertEqual(plan["associated_palette"]["color_interpretation"], "rgb")
        self.assertEqual(metadata["associated_palette_mode"], "generated")
        self.assertEqual(metadata["associated_palette_color_interpretation"], "rgb")

    @patch("handler_render_plan._storage_call")
    def test_fused_color_plan_accepts_palette_component_lut_outputs(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        result = handler(_make_event(
            save_associated_palette=True,
            color_interpretation="hsv_lut",
            solve_score_chain=[
                ["proximity", "0.1"],
                ["emit", "norm"],
                ["spread", "0.1"],
                ["emit", "norm"],
                ["angular_entropy_16", "0.1"],
                ["emit", "norm"],
            ],
        ), None)
        plan = json.loads(result["body"])

        metadata = plan["outputs"]["metadata"]
        self.assertEqual(metadata["raw_channels"], "3")
        self.assertEqual(metadata["score_output_interpretation"], "hsv_lut")
        self.assertEqual(plan["params"]["color_interpretation"], "hsv_lut")
        self.assertTrue(plan["associated_palette"]["enabled"])
        self.assertEqual(plan["associated_palette"]["mode"], "generated")
        self.assertEqual(plan["associated_palette"]["palette"], "inferno")
        self.assertEqual(plan["associated_palette"]["fragment_prefix"], "renders/j/palettes/pal_color_run_t/fragments/section_")
        self.assertEqual(plan["associated_palette"]["raw_key"], "renders/j/palettes/pal_color_run_t/greyscale.raw")
        self.assertEqual(plan["associated_palette"]["color_interpretation"], "hsv_lut")
        channels = json.loads(metadata["score_output_channels"])
        self.assertEqual([row["name"] for row in channels], ["h_lookup", "s_lookup", "v_lookup"])

    @patch("handler_render_plan._storage_call")
    def test_fused_color_plan_rejects_two_channel_outputs_for_v1(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        with self.assertRaisesRegex(RuntimeError, "Scalar LUT requires 1"):
            handler(_make_event(
                solve_score_chain=[
                    ["proximity", "0.1"],
                    ["emit_norm"],
                    ["spread", "0.1"],
                    ["emit_norm"],
                ],
            ), None)

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
        self.assertEqual(plan["physical_source_items"], [])
        self.assertTrue(plan["coeff_bilevel"]["enabled"])
        self.assertEqual(plan["coeff_bilevel"]["fragment_prefix"], "renders/j/coeff_bilevel_section_")
        self.assertEqual(plan["render_execution"], {})
        self.assertEqual(
            set(plan["params"].keys()),
            {
                "pix",
                "view_mode",
                "quantile",
                "shim",
                "square_extent",
                "root_transforms",
                "root_program_source_text",
                "root_program",
                "root_program_fingerprint",
                "root_spec_version",
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
                "view_mode",
                "quantile",
                "shim",
                "square_extent",
                "root_transforms",
                "root_program_source_text",
                "root_program",
                "root_program_fingerprint",
                "root_spec_version",
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
    def test_fused_color_plan_rejects_removed_scalar_solve_score_params(self, mock_storage):
        mock_storage.side_effect = _mock_storage_detail(_base_color_calc())
        from handler_render_plan import handler

        with self.assertRaisesRegex(RuntimeError, "solve_metric is not supported for fused color"):
            handler(_make_event(solve_metric="proximity"), None)

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
                    save_associated_palette=True,
                    solve_score_chain=[["max_re", "cf", "0.1"], ["t1_abs", "pm", "0.1"], ["avg"]],
                ), None)

        msg = str(ctx.exception)
        self.assertIn("Counts: chunks=", msg)
        self.assertIn("raster_items=", msg)
        self.assertIn("bilevel_sections=", msg)
        self.assertIn("coeff_sections=", msg)
        self.assertNotIn("solve_score_items=", msg)
        self.assertNotIn("palette_items=", msg)
        self.assertNotIn("stripe_count", msg)


if __name__ == "__main__":
    unittest.main()
