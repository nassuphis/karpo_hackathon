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
            "ColorSolveScoreHistPhase", "ColorSolveScoreHistMap",
            "ColorSolveScoreMergePhase", "ColorSolveScoreMergeTask",
            "ColorRasterPhase", "ColorRasterMap",
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
            "ColorSolveScoreHistMap", "ColorRasterMap", "ColorFinalizeMap",
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
            assert mc > 1, f"{name} has MaxConcurrency={mc}, must be > 1"

    def test_stripe_maps_concurrency_10(self):
        """Stripe-based maps must have MaxConcurrency=10."""
        for name in ["ColorRasterMap", "ColorSolveScoreHistMap",
                      "BilevelRasterMap", "CoeffRasterMap"]:
            mc = self.states[name].get("MaxConcurrency", 0)
            assert mc == 10, f"{name} MaxConcurrency={mc}, expected 10"

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

    def test_chunked_workers_use_chunk_item_bin_keys(self):
        hist_selector = self.states["ColorSolveScoreHistMap"]["ItemSelector"]
        self.assertEqual(hist_selector["bin_key.$"], "$$.Map.Item.Value.bin_key")

        color_raster_selector = self.states["ColorRasterMap"]["ItemSelector"]
        self.assertEqual(color_raster_selector["bin_key.$"], "$$.Map.Item.Value.bin_key")

        bilevel_selector = self.states["BilevelRasterMap"]["ItemSelector"]
        self.assertEqual(bilevel_selector["bin_key.$"], "$$.Map.Item.Value.bin_key")

    def test_solve_score_tasks_carry_thread_count(self):
        clip_payload = self.states["ColorSolveScoreClipTask"]["Parameters"]["Payload"]
        self.assertEqual(clip_payload["solve_score_threads.$"], "$.plan.solve_score.threads")

        hist_selector = self.states["ColorSolveScoreHistMap"]["ItemSelector"]
        self.assertEqual(hist_selector["solve_score_threads.$"], "$.plan.solve_score.threads")
        self.assertEqual(hist_selector["solve_score_hist_input_mode.$"], "$.plan.solve_score.hist_input_mode")
        self.assertEqual(hist_selector["solve_score_hist_retries.$"], "$.plan.solve_score.hist_retries")

        merge_payload = self.states["ColorSolveScoreMergeTask"]["Parameters"]["Payload"]
        self.assertEqual(merge_payload["solve_score_threads.$"], "$.plan.solve_score.threads")
        self.assertEqual(merge_payload["solve_score_merge_workers.$"], "$.plan.solve_score.merge_workers")

        finalize_selector = self.states["ColorFinalizeMap"]["ItemSelector"]
        self.assertEqual(finalize_selector["finalize_workers.$"], "$.plan.finalize.workers")

        color_raster_selector = self.states["ColorRasterMap"]["ItemSelector"]
        self.assertEqual(color_raster_selector["raster_sectioned_retries.$"], "$.plan.raster.sectioned_retries")

    def test_status_tasks_forward_run_started_at_ms(self):
        phase_states = [
            "CleanRender",
            "ColorSolveScoreClipPhase",
            "ColorSolveScoreHistPhase",
            "ColorSolveScoreMergePhase",
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
