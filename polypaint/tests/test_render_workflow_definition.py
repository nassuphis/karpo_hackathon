"""
Tests for the Step Functions ASL template.

Validates structural correctness: required states exist, Map states
have correct concurrency, no state targets the starter Lambda,
ResultPath conventions are followed, retry policies exist.

Run: cd polypaint && uv run python -m pytest tests/test_render_workflow_definition.py -v
"""
import json
import os
import re
import sys
import unittest

TEMPLATE_PATH = os.path.join(
    os.path.dirname(__file__), "..", "stepfunctions", "render_workflow.asl.json.template"
)


def _load_asl():
    """Load and render the ASL template with placeholder ARNs."""
    with open(TEMPLATE_PATH) as f:
        raw = f.read()
    # Replace all ${...Arn} placeholders with dummy ARNs
    rendered = re.sub(r'\$\{(\w+)\}', r'arn:aws:lambda:us-east-1:123456789012:function:placeholder-\1', raw)
    return json.loads(rendered)


class TestWorkflowDefinition(unittest.TestCase):

    def setUp(self):
        self.asl = _load_asl()
        self.top_states = self.asl["States"]
        # Inner states live inside the Parallel branch
        wrapper = self.top_states.get("WorkflowWrapper", {})
        branches = wrapper.get("Branches", [{}])
        self.states = branches[0].get("States", {}) if branches else {}
        # Merge top-level states for tests that check ReportError/Succeed/Fail
        self.all_states = {**self.top_states, **self.states}

    def test_template_parses_as_valid_json(self):
        """ASL template renders to valid JSON."""
        assert "StartAt" in self.asl
        assert "States" in self.asl
        assert len(self.states) > 0

    def test_required_top_level_states(self):
        """Required states exist (top-level or inside Parallel branch)."""
        for name in ["CleanRender", "BuildPlan", "ModeChoice",
                      "ReportDoneColor", "ReportDoneBilevel", "ReportDoneCoeffBilevel"]:
            assert name in self.states, f"missing inner state: {name}"
        for name in ["Succeed", "ReportError", "Fail", "WorkflowWrapper"]:
            assert name in self.top_states, f"missing top-level state: {name}"

    def test_required_color_states(self):
        """Required color pipeline states exist."""
        for name in [
            "ColorSolveScoreChoice",
            "ColorSolveScoreClipPhase", "ColorSolveScoreClipTask",
            "ColorSolveScoreHistNeededChoice",
            "ColorSolveScoreHistPhase", "ColorSolveScoreHistMap",
            "ColorSolveScoreMergePhase", "ColorSolveScoreMergeTask",
            "ColorAssociatedPaletteChoice",
            "ColorAssociatedPalettePhase", "ColorAssociatedPaletteMap",
            "ColorAssociatedPaletteFinalizePhase", "ColorAssociatedPaletteFinalizeTask",
            "ColorRasterPhase", "ColorRasterMap",
            "ColorOutputPipelineChoice", "ColorAssembleEncodePhase", "ColorAssembleEncodeTask",
            "ColorFinalizePhase", "ColorFinalizeMap",
            "ColorEncodePhase", "ColorEncodeTask",
            "ColorPreviewTask",
        ]:
            assert name in self.states, f"missing color state: {name}"

    def test_required_bilevel_states(self):
        """Required bilevel pipeline states exist."""
        for name in [
            "BilevelRasterPhase", "BilevelRasterMap",
            "BilevelMergePhase", "BilevelMergeMap",
            "BilevelStitchPhase", "BilevelStitchTask",
            "BilevelPreviewTask",
        ]:
            assert name in self.states, f"missing bilevel state: {name}"

    def test_required_coeff_states(self):
        """Required coeff bilevel pipeline states exist."""
        for name in [
            "CoeffRasterPhase", "CoeffRasterMap",
            "CoeffMergePhase", "CoeffMergeMap",
            "CoeffStitchPhase", "CoeffStitchTask",
            "CoeffPreviewTask",
        ]:
            assert name in self.states, f"missing coeff state: {name}"

    def test_required_map_states(self):
        """All expected Map states are actually Map type."""
        map_names = [
            "ColorSolveScoreHistMap", "ColorAssociatedPaletteMap", "ColorRasterMap", "ColorFinalizeMap",
            "BilevelRasterMap", "BilevelMergeMap",
            "CoeffRasterMap", "CoeffMergeMap",
        ]
        for name in map_names:
            assert self.states[name]["Type"] == "Map", f"{name} should be Map"

    def test_no_state_targets_orchestrator(self):
        """No state should invoke polypaint-render-orchestrator."""
        asl_str = json.dumps(self.asl)
        assert "polypaint-render-orchestrator" not in asl_str, \
            "state machine must not invoke the starter Lambda"

    def test_no_generic_phase_lambda(self):
        """No state targets a generic 'sfn-phase' style Lambda."""
        asl_str = json.dumps(self.asl)
        assert "sfn-phase" not in asl_str
        assert "sfn_phase" not in asl_str

    def test_build_plan_writes_to_plan(self):
        """BuildPlan must write results (plan data flows through $.plan)."""
        bp = self.states["BuildPlan"]
        assert bp.get("ResultPath") is not None, "BuildPlan must have ResultPath"

    def test_parse_plan_preserves_run_started_at_ms(self):
        parse_plan = self.states["ParsePlan"]
        self.assertEqual(parse_plan["Parameters"]["run_started_at_ms.$"], "$.run_started_at_ms")

    def test_report_states_use_null_result_path(self):
        """Status/report states must use ResultPath: null."""
        report_states = [n for n in self.all_states
                         if isinstance(self.all_states[n], dict) and ("Phase" in n or "Report" in n)]
        for name in report_states:
            s = self.all_states[name]
            if s.get("Type") == "Task":
                assert s.get("ResultPath") is None, \
                    f"{name} must use ResultPath: null, got {s.get('ResultPath')}"

    def test_worker_states_do_not_overwrite_plan(self):
        """Worker Task and Map states must not overwrite top-level plan data."""
        worker_states = [n for n in self.states
                         if isinstance(self.states[n], dict)
                         and ("Map" in n or "Task" in n)
                         and n not in ("BuildPlan", "ParsePlan")]
        for name in worker_states:
            s = self.states[name]
            rp = s.get("ResultPath")
            assert rp != "$", f"{name} overwrites entire state with ResultPath: $"

    def test_map_concurrency_set(self):
        """Map states must have MaxConcurrency > 1."""
        map_states = [n for n in self.states
                      if isinstance(self.states[n], dict) and self.states[n].get("Type") == "Map"]
        assert len(map_states) > 0, "no Map states found"
        for name in map_states:
            mc = self.states[name].get("MaxConcurrency", 0)
            mc_path = self.states[name].get("MaxConcurrencyPath", "")
            assert mc > 1 or mc_path, f"{name} missing MaxConcurrency/MaxConcurrencyPath"

    def test_stripe_maps_concurrency_10(self):
        """Stripe-based maps must have MaxConcurrency=10."""
        for name in ["ColorRasterMap", "ColorSolveScoreHistMap",
                      "BilevelRasterMap", "CoeffRasterMap"]:
            mc = self.states[name].get("MaxConcurrency", 0)
            assert mc == 10, f"{name} MaxConcurrency={mc}, expected 10"
        self.assertEqual(
            self.states["ColorAssociatedPaletteMap"].get("MaxConcurrencyPath"),
            "$.plan.associated_palette.chunk_workers",
        )

    def test_logical_section_workers_receive_source_manifest_not_inline_spans(self):
        hist_selector = self.states["ColorSolveScoreHistMap"]["ItemSelector"]
        palette_selector = self.states["ColorAssociatedPaletteMap"]["ItemSelector"]
        self.assertEqual(hist_selector["solve_source_manifest.$"], "$.plan.solve_source_manifest")
        self.assertEqual(palette_selector["solve_source_manifest.$"], "$.plan.solve_source_manifest")
        self.assertNotIn("root_spans.$", hist_selector)
        self.assertNotIn("coeff_spans.$", hist_selector)
        self.assertNotIn("param_spans.$", hist_selector)
        self.assertNotIn("root_spans.$", palette_selector)
        self.assertNotIn("coeff_spans.$", palette_selector)
        self.assertNotIn("param_spans.$", palette_selector)

    def test_tile_maps_concurrency_32(self):
        """Tile-based maps must have MaxConcurrency=32."""
        for name in ["ColorFinalizeMap", "BilevelMergeMap", "CoeffMergeMap"]:
            mc = self.states[name].get("MaxConcurrency", 0)
            assert mc == 32, f"{name} MaxConcurrency={mc}, expected 32"

    def test_retry_policy_on_top_level_invoke_states(self):
        """Lambda invoke Task states must have retry policy."""
        for name, s in self.all_states.items():
            if not isinstance(s, dict):
                continue
            if s.get("Type") == "Task" and "lambda:invoke" in str(s.get("Resource", "")):
                retry = s.get("Retry", [])
                assert len(retry) > 0, f"{name} missing Retry policy"
                errors = retry[0].get("ErrorEquals", [])
                assert "Lambda.ServiceException" in errors, \
                    f"{name} Retry missing Lambda.ServiceException"

    def test_retry_policy_on_map_worker_states(self):
        """Worker states inside Map processors must have retry policy."""
        for name, s in self.states.items():
            if not isinstance(s, dict):
                continue
            if s.get("Type") == "Map":
                processor = s.get("ItemProcessor", {})
                inner_states = processor.get("States", {})
                for iname, ist in inner_states.items():
                    if ist.get("Type") == "Task" and "lambda:invoke" in str(ist.get("Resource", "")):
                        retry = ist.get("Retry", [])
                        assert len(retry) > 0, f"{name}/{iname} missing Retry policy"

    def test_mode_specific_done_states(self):
        """Each mode has its own ReportDone with the correct output key."""
        color = self.states["ReportDoneColor"]
        assert "image_key" in json.dumps(color["Parameters"]["Payload"])
        assert "outputs.image_key" in json.dumps(color)

        bilevel = self.states["ReportDoneBilevel"]
        assert "outputs.bilevel_key" in json.dumps(bilevel)

        coeff = self.states["ReportDoneCoeffBilevel"]
        assert "outputs.coeff_bilevel_key" in json.dumps(coeff)

    def test_parallel_wrapper_catches_all(self):
        """WorkflowWrapper Parallel must catch States.ALL including States.Runtime."""
        wrapper = self.top_states["WorkflowWrapper"]
        assert wrapper["Type"] == "Parallel"
        catch = wrapper.get("Catch", [])
        assert len(catch) > 0, "WorkflowWrapper missing Catch"
        assert "States.ALL" in catch[0]["ErrorEquals"]
        assert catch[0]["Next"] == "ReportError"

    def test_preview_tasks_target_preview_lambda(self):
        """Each pipeline's preview task invokes the preview Lambda."""
        asl_str = json.dumps(self.asl)
        assert "placeholder-PreviewFunctionArn" in asl_str, "missing PreviewFunctionArn placeholder"
        for name in ["ColorPreviewTask", "BilevelPreviewTask", "CoeffPreviewTask"]:
            assert name in self.states, f"missing preview state: {name}"
            s = self.states[name]
            assert s["Type"] == "Task"
            params = json.dumps(s.get("Parameters", {}))
            assert "PreviewFunctionArn" in params, f"{name} should target preview Lambda"

    def test_worker_states_target_real_workers(self):
        """Worker states inside Map processors target real worker Lambdas, not intermediaries."""
        # Check that Map item processors invoke actual worker functions
        asl_str = json.dumps(self.asl)
        # The rendered template should contain placeholder ARNs for real workers
        assert "placeholder-FinalizeFunctionArn" in asl_str
        assert "placeholder-EncodeFunctionArn" in asl_str
        assert "placeholder-BilevelFunctionArn" in asl_str
        assert "placeholder-BilevelStitchFunctionArn" in asl_str
        assert "placeholder-SolveProximityFunctionArn" in asl_str

    def test_color_raster_worker_uses_dynamic_function_name(self):
        color_map = self.states["ColorRasterMap"]
        selector = color_map["ItemSelector"]
        self.assertEqual(selector["raster_function_name.$"], "$.plan.raster.function_name")
        self.assertEqual(selector["raster_input_mode.$"], "$.plan.raster.input_mode")
        worker = color_map["ItemProcessor"]["States"]["RasterWorker"]
        self.assertEqual(worker["Parameters"]["FunctionName.$"], "$.raster_function_name")

    def test_color_raster_maps_pass_emit_raw_score_bins_flag(self):
        selector = self.states["ColorRasterMap"]["ItemSelector"]
        self.assertEqual(selector["emit_raw_score_bins.$"], "$.plan.raster.emit_raw_score_bins")
        self.assertEqual(selector["solve_score_clip_key.$"], "$.plan.solve_score.clip_key")

    def test_fused_skips_hist_merge_after_clip(self):
        choice = self.states["ColorSolveScoreHistNeededChoice"]
        self.assertEqual(choice["Type"], "Choice")
        self.assertEqual(choice["Choices"][0]["Variable"], "$.plan.render_execution.color_pipeline")
        self.assertEqual(choice["Choices"][0]["StringEquals"], "fused")
        self.assertEqual(choice["Choices"][0]["Next"], "ColorRasterPhase")
        self.assertEqual(choice["Default"], "ColorSolveScoreHistPhase")
        self.assertEqual(self.states["ColorSolveScoreClipTask"]["Next"], "ColorSolveScoreHistNeededChoice")
        clip_task = self.states["ColorSolveScoreClipTask"]
        self.assertEqual(clip_task["ResultPath"], "$.solve_score_clip")
        self.assertEqual(
            clip_task["ResultSelector"]["parsed.$"],
            "States.StringToJson($.Payload.body)",
        )

    def test_color_output_pipeline_choice_routes_fused_to_finalize_mt(self):
        choice = self.states["ColorOutputPipelineChoice"]
        self.assertEqual(choice["Type"], "Choice")
        self.assertEqual(choice["Choices"][0]["Variable"], "$.plan.render_execution.color_pipeline")
        self.assertEqual(choice["Choices"][0]["StringEquals"], "fused")
        self.assertEqual(choice["Choices"][0]["Next"], "ColorAssembleEncodePhase")
        self.assertEqual(choice["Default"], "ColorFinalizePhase")

    def test_color_raster_map_flows_into_output_pipeline_choice(self):
        self.assertEqual(self.states["ColorRasterMap"]["Next"], "ColorOutputPipelineChoice")

    def test_color_assemble_encode_task_targets_finalize_mt(self):
        phase = self.states["ColorAssembleEncodePhase"]
        self.assertEqual(phase["Parameters"]["Payload"]["phase"], "finalize_mt")
        self.assertEqual(phase["Next"], "ColorAssembleEncodeTask")

        task = self.states["ColorAssembleEncodeTask"]
        payload = task["Parameters"]["Payload"]
        params_json = json.dumps(task["Parameters"])
        self.assertIn("FinalizeMTFunctionArn", params_json)
        self.assertEqual(payload["phase"], "finalize_mt")
        self.assertEqual(payload["run_id.$"], "$.run_id")
        self.assertEqual(payload["mode.$"], "$.mode")
        self.assertEqual(payload["source_item_count.$"], "$.plan.raster.item_count")
        self.assertEqual(payload["render_execution.$"], "$.plan.render_execution")
        self.assertEqual(payload["metadata.$"], "$.plan.outputs.metadata")
        self.assertEqual(payload["image_key.$"], "$.plan.outputs.image_key")
        self.assertEqual(payload["preview_key.$"], "$.plan.outputs.preview_key")
        self.assertEqual(payload["meta_key.$"], "$.plan.outputs.meta_key")
        self.assertEqual(payload["raw_key.$"], "$.plan.outputs.raw_key")
        self.assertEqual(payload["raw_meta_key.$"], "$.plan.outputs.raw_meta_key")
        self.assertEqual(payload["plan_params_digest.$"], "$.plan.outputs.plan_params_digest")
        self.assertEqual(payload["clip_slots.$"], "$.solve_score_clip.parsed.clip_slots")
        self.assertEqual(payload["score_program.$"], "$.solve_score_clip.parsed.score_program")
        self.assertEqual(payload["chain_fingerprint.$"], "$.solve_score_clip.parsed.chain_fingerprint")
        self.assertEqual(payload["fragment_prefix.$"], "$.plan.outputs.fragment_prefix")
        self.assertEqual(payload["fragment_manifest"]["version"], 1)
        self.assertEqual(payload["fragment_manifest"]["pair_encoding"], "u32le_u8_v1")
        self.assertEqual(payload["fragment_manifest"]["item_count.$"], "$.plan.raster.item_count")
        self.assertEqual(payload["fragment_manifest"]["fragment_prefix.$"], "$.plan.outputs.fragment_prefix")
        self.assertEqual(payload["fragment_manifest"]["chain_fingerprint.$"], "$.plan.outputs.metadata.solve_score_chain_fingerprint")
        self.assertEqual(payload["associated_palette.$"], "$.plan.associated_palette")
        self.assertEqual(payload["associated_palette_grid_n.$"], "$.plan.calc.N")
        self.assertEqual(payload["associated_palette_times.$"], "$.plan.calc.times")
        self.assertEqual(payload["associated_palette_degree.$"], "$.plan.calc.degree")
        self.assertEqual(task["Next"], "ReportDoneColor")

    def test_chunked_workers_use_chunk_item_bin_keys(self):
        hist_selector = self.states["ColorSolveScoreHistMap"]["ItemSelector"]
        self.assertEqual(hist_selector["bin_key.$"], "$$.Map.Item.Value.bin_key")

        color_raster_selector = self.states["ColorRasterMap"]["ItemSelector"]
        self.assertEqual(color_raster_selector["bin_key.$"], "$$.Map.Item.Value.bin_key")

        bilevel_selector = self.states["BilevelRasterMap"]["ItemSelector"]
        self.assertEqual(bilevel_selector["bin_key.$"], "$$.Map.Item.Value.bin_key")

        assoc_selector = self.states["ColorAssociatedPaletteMap"]["ItemSelector"]
        self.assertEqual(assoc_selector["bin_key.$"], "$$.Map.Item.Value.bin_key")
        self.assertEqual(assoc_selector["bin_size.$"], "$$.Map.Item.Value.bin_size")
        self.assertEqual(assoc_selector["coeffs_key.$"], "$$.Map.Item.Value.coeffs_key")
        self.assertEqual(assoc_selector["coeffs_bin_size.$"], "$$.Map.Item.Value.coeffs_bin_size")
        self.assertEqual(assoc_selector["n_coeffs.$"], "$.plan.calc.n_coeffs")

    def test_associated_palette_finalize_forwards_solve_score_chain(self):
        finalize = self.states["ColorAssociatedPaletteFinalizeTask"]["Parameters"]["Payload"]
        self.assertEqual(finalize["solve_score_chain.$"], "$.plan.associated_palette.score_chain")
        self.assertNotIn("solve_score_quantile.$", finalize)
        self.assertNotIn("solve_score_omega.$", finalize)
        self.assertNotIn("solve_score_omega_enabled.$", finalize)
        self.assertEqual(finalize["render_execution.$"], "$.plan.render_execution")

    def test_solve_score_tasks_carry_thread_count(self):
        clip_payload = self.states["ColorSolveScoreClipTask"]["Parameters"]["Payload"]
        self.assertEqual(clip_payload["solve_score_threads.$"], "$.plan.solve_score.threads")

        hist_selector = self.states["ColorSolveScoreHistMap"]["ItemSelector"]
        self.assertEqual(hist_selector["solve_score_threads.$"], "$.plan.solve_score.threads")
        self.assertEqual(hist_selector["solve_score_hist_input_mode.$"], "$.plan.solve_score.hist_input_mode")
        self.assertEqual(hist_selector["solve_score_hist_retries.$"], "$.plan.solve_score.hist_retries")

    def test_associated_palette_chunk_forwards_mt_fields(self):
        selector = self.states["ColorAssociatedPaletteMap"]["ItemSelector"]
        self.assertEqual(selector["palette_chunk_threads.$"], "$.plan.associated_palette.chunk_threads")
        self.assertEqual(selector["palette_chunk_input_mode.$"], "$.plan.associated_palette.chunk_input_mode")
        self.assertEqual(selector["palette_chunk_retries.$"], "$.plan.associated_palette.chunk_retries")
        self.assertEqual(selector["palette_chunk_workers.$"], "$.plan.associated_palette.chunk_workers")

        merge_payload = self.states["ColorSolveScoreMergeTask"]["Parameters"]["Payload"]
        self.assertEqual(merge_payload["solve_score_threads.$"], "$.plan.solve_score.threads")
        self.assertEqual(merge_payload["solve_score_merge_workers.$"], "$.plan.solve_score.merge_workers")

        finalize_selector = self.states["ColorFinalizeMap"]["ItemSelector"]
        self.assertEqual(finalize_selector["finalize_workers.$"], "$.plan.finalize.workers")

        color_raster_selector = self.states["ColorRasterMap"]["ItemSelector"]
        self.assertEqual(color_raster_selector["raster_sectioned_retries.$"], "$.plan.raster.sectioned_retries")
        self.assertEqual(self.states["ColorRasterPhase"]["Parameters"]["Payload"]["expected.$"], "$.plan.raster.item_count")
        self.assertEqual(self.states["ColorRasterMap"]["ItemsPath"], "$.plan.raster.map_items")

    def test_color_solve_score_tasks_forward_critical_fields(self):
        clip = self.states["ColorSolveScoreClipTask"]["Parameters"]["Payload"]
        self.assertEqual(clip["solve_score_chain.$"], "$.plan.solve_score.chain")
        self.assertNotIn("solve_score_quantile.$", clip)
        self.assertNotIn("solve_score_omega.$", clip)
        self.assertNotIn("solve_score_omega_enabled.$", clip)
        self.assertEqual(clip["root_transforms.$"], "$.plan.params.root_transforms")
        self.assertEqual(clip["lores_params_key.$"], "$.plan.calc.lores_params_key")

        hist = self.states["ColorSolveScoreHistMap"]["ItemSelector"]
        self.assertEqual(hist["solve_score_chain.$"], "$.plan.solve_score.chain")
        self.assertNotIn("solve_score_quantile.$", hist)
        self.assertNotIn("solve_score_omega.$", hist)
        self.assertNotIn("solve_score_omega_enabled.$", hist)
        self.assertEqual(hist["root_transforms.$"], "$.plan.params.root_transforms")
        self.assertEqual(hist["params_key.$"], "$$.Map.Item.Value.params_key")
        self.assertEqual(hist["params_step_start.$"], "$$.Map.Item.Value.params_step_start")
        self.assertEqual(hist["params_step_count.$"], "$$.Map.Item.Value.params_step_count")
        self.assertEqual(hist["step_start.$"], "$$.Map.Item.Value.step_start")
        self.assertEqual(hist["step_count.$"], "$$.Map.Item.Value.step_count")

        merge = self.states["ColorSolveScoreMergeTask"]["Parameters"]["Payload"]
        self.assertEqual(merge["solve_score_chain.$"], "$.plan.solve_score.chain")
        self.assertNotIn("solve_score_quantile.$", merge)
        self.assertNotIn("solve_score_omega.$", merge)
        self.assertNotIn("solve_score_omega_enabled.$", merge)

        raster_selector = self.states["ColorRasterMap"]["ItemSelector"]
        self.assertEqual(raster_selector["root_transforms.$"], "$.plan.params.root_transforms")
        self.assertEqual(raster_selector["pixel_bins_drive_rgb.$"], "$.plan.outputs.pixel_bins_drive_rgb")
        self.assertEqual(raster_selector["solve_score_chain.$"], "$.plan.solve_score.chain")
        self.assertNotIn("solve_score_quantile.$", raster_selector)
        self.assertNotIn("solve_score_omega.$", raster_selector)
        self.assertNotIn("solve_score_omega_enabled.$", raster_selector)
        self.assertEqual(raster_selector["pixel_bin_fragment_mode.$"], "$.plan.raster.pixel_bin_fragment_mode")
        self.assertEqual(raster_selector["raster_bin_group_size.$"], "$.plan.raster.raster_bin_group_size")
        self.assertEqual(raster_selector["saved_palette_bins_prefix.$"], "$.plan.saved_palette.section_bins_prefix")
        self.assertEqual(raster_selector["logical_section.$"], "$.plan.raster.logical_section")
        self.assertEqual(raster_selector["solve_source_manifest.$"], "$.plan.solve_source_manifest")
        self.assertEqual(raster_selector["params_key.$"], "$$.Map.Item.Value.params_key")
        self.assertEqual(raster_selector["params_step_start.$"], "$$.Map.Item.Value.params_step_start")
        self.assertEqual(raster_selector["params_step_count.$"], "$$.Map.Item.Value.params_step_count")
        self.assertEqual(raster_selector["step_start.$"], "$$.Map.Item.Value.step_start")
        self.assertEqual(raster_selector["step_count.$"], "$$.Map.Item.Value.step_count")
        self.assertEqual(raster_selector["section_idx.$"], "$$.Map.Item.Value.section_idx")
        self.assertEqual(raster_selector["section_count.$"], "$$.Map.Item.Value.section_count")
        self.assertEqual(raster_selector["group_idx.$"], "$$.Map.Item.Value.group_idx")
        self.assertEqual(raster_selector["sections.$"], "$$.Map.Item.Value.sections")
        self.assertEqual(raster_selector["section_indices.$"], "$$.Map.Item.Value.section_indices")
        self.assertEqual(raster_selector["color_pipeline.$"], "$.plan.render_execution.color_pipeline")
        self.assertEqual(raster_selector["associated_palette_mode.$"], "$.plan.associated_palette.mode")
        self.assertEqual(raster_selector["associated_palette_fragment_prefix.$"], "$.plan.associated_palette.fragment_prefix")
        self.assertEqual(raster_selector["associated_palette_grid_n.$"], "$.plan.calc.N")
        self.assertEqual(raster_selector["fragment_prefix.$"], "$.plan.outputs.fragment_prefix")

        assoc_choice = self.states["ColorAssociatedPaletteChoice"]
        self.assertEqual(
            assoc_choice["Choices"][0]["Variable"],
            "$.plan.associated_palette.mode",
        )
        self.assertEqual(
            assoc_choice["Choices"][0]["StringEquals"],
            "generated",
        )

        assoc_map = self.states["ColorAssociatedPaletteMap"]["ItemSelector"]
        self.assertEqual(assoc_map["step_start.$"], "$$.Map.Item.Value.step_start")
        self.assertEqual(assoc_map["step_count.$"], "$$.Map.Item.Value.step_count")
        self.assertEqual(assoc_map["coeffs_key.$"], "$$.Map.Item.Value.coeffs_key")
        self.assertEqual(assoc_map["coeffs_bin_size.$"], "$$.Map.Item.Value.coeffs_bin_size")
        self.assertEqual(assoc_map["params_key.$"], "$$.Map.Item.Value.params_key")
        self.assertEqual(assoc_map["params_step_start.$"], "$$.Map.Item.Value.params_step_start")
        self.assertEqual(assoc_map["params_step_count.$"], "$$.Map.Item.Value.params_step_count")
        self.assertEqual(assoc_map["n_coeffs.$"], "$.plan.calc.n_coeffs")
        self.assertEqual(assoc_map["metric.$"], "$.plan.associated_palette.metric")
        self.assertEqual(assoc_map["solve_score_chain.$"], "$.plan.associated_palette.score_chain")
        self.assertNotIn("solve_score_quantile.$", assoc_map)
        self.assertNotIn("solve_score_omega.$", assoc_map)
        self.assertNotIn("solve_score_omega_enabled.$", assoc_map)
        self.assertEqual(assoc_map["solve_score_bins_key.$"], "$.plan.solve_score.bins_key")

        assoc_finalize = self.states["ColorAssociatedPaletteFinalizeTask"]["Parameters"]["Payload"]
        self.assertEqual(assoc_finalize["N.$"], "$.plan.calc.N")
        self.assertEqual(assoc_finalize["times.$"], "$.plan.calc.times")
        self.assertEqual(assoc_finalize["metric.$"], "$.plan.associated_palette.metric")
        self.assertEqual(assoc_finalize["image_key.$"], "$.plan.associated_palette.image_key")
        self.assertEqual(assoc_finalize["section_bins_prefix.$"], "$.plan.associated_palette.section_bins_prefix")
        self.assertEqual(assoc_finalize["cleanup_solve_score_scratch"], False)

    def test_color_finalize_and_encode_forward_critical_fields(self):
        finalize = self.states["ColorFinalizeMap"]["ItemSelector"]
        self.assertEqual(finalize["emit_pixel_bins.$"], "$.plan.outputs.repalette_capable")
        self.assertEqual(finalize["pixel_bins_drive_rgb.$"], "$.plan.outputs.pixel_bins_drive_rgb")
        self.assertEqual(finalize["pixel_bin_fragment_mode.$"], "$.plan.raster.pixel_bin_fragment_mode")
        self.assertEqual(finalize["raster_item_count.$"], "$.plan.raster.item_count")
        self.assertEqual(finalize["pixel_bins_out_prefix.$"], "$.plan.grid.pixel_bin_tile_prefix")
        self.assertEqual(finalize["palette.$"], "$.plan.params.palette")
        self.assertEqual(finalize["background_color.$"], "$.plan.outputs.metadata.background_color")
        self.assertEqual(finalize["pixel_bins_empty.$"], "$.plan.outputs.metadata.pixel_bins_empty")
        self.assertEqual(finalize["finalize_workers.$"], "$.plan.finalize.workers")
        self.assertEqual(finalize["width.$"], "$.plan.grid.pix")
        self.assertEqual(finalize["height.$"], "$.plan.grid.pix")
        self.assertEqual(finalize["tile_size.$"], "$.plan.grid.tile_size")

        encode = self.states["ColorEncodeTask"]["Parameters"]["Payload"]
        self.assertEqual(encode["metadata.$"], "$.plan.outputs.metadata")
        self.assertEqual(encode["out_key.$"], "$.plan.outputs.image_key")
        self.assertEqual(encode["tile_grid"]["tile_prefix.$"], "$.plan.grid.raw_tile_prefix")

    def test_bilevel_and_coeff_pipeline_forward_critical_fields(self):
        bilevel_raster = self.states["BilevelRasterMap"]["ItemSelector"]
        self.assertEqual(bilevel_raster["root_transforms.$"], "$.plan.params.root_transforms")

        bilevel_stitch = self.states["BilevelStitchTask"]["Parameters"]["Payload"]
        self.assertEqual(bilevel_stitch["metadata.$"], "$.plan.outputs.metadata")
        self.assertEqual(bilevel_stitch["preview_key.$"], "$.plan.outputs.preview_key")

        coeff_stitch = self.states["CoeffStitchTask"]["Parameters"]["Payload"]
        self.assertEqual(coeff_stitch["metadata.$"], "$.plan.outputs.metadata")
        self.assertEqual(coeff_stitch["preview_key.$"], "$.plan.outputs.preview_key")

    def test_status_tasks_forward_run_started_at_ms(self):
        phase_states = [
            "CleanRender",
            "ColorSolveScoreClipPhase",
            "ColorSolveScoreHistPhase",
            "ColorSolveScoreMergePhase",
            "ColorAssociatedPalettePhase",
            "ColorAssociatedPaletteFinalizePhase",
            "ColorRasterPhase",
            "ColorFinalizePhase",
            "ColorEncodePhase",
            "BilevelRasterPhase",
            "BilevelMergePhase",
            "BilevelStitchPhase",
            "CoeffRasterPhase",
            "CoeffMergePhase",
            "CoeffStitchPhase",
        ]
        for name in phase_states:
            payload = self.states[name]["Parameters"]["Payload"]
            self.assertEqual(payload["run_started_at_ms.$"], "$.run_started_at_ms")

        for name in ["ReportDoneColor", "ReportDoneBilevel", "ReportDoneCoeffBilevel"]:
            payload = self.states[name]["Parameters"]["Payload"]
            self.assertEqual(payload["run_started_at_ms.$"], "$.run_started_at_ms")

        error_payload = self.top_states["ReportError"]["Parameters"]["Payload"]
        self.assertEqual(error_payload["run_started_at_ms.$"], "$.run_started_at_ms")


if __name__ == "__main__":
    unittest.main()
    def test_color_raster_map_uses_unified_map_items(self):
        color_map = self.states["ColorRasterMap"]
        self.assertEqual(color_map["ItemsPath"], "$.plan.raster.map_items")
        selector = color_map["ItemSelector"]
        self.assertEqual(selector["group_idx.$"], "$$.Map.Item.Value.group_idx")
        self.assertEqual(selector["sections.$"], "$$.Map.Item.Value.sections")
        self.assertEqual(selector["section_indices.$"], "$$.Map.Item.Value.section_indices")
        self.assertNotIn("ColorRasterItemsChoice", self.states)
        self.assertNotIn("ColorRasterMapSparse", self.states)
        self.assertNotIn("ColorRasterMapDense", self.states)
