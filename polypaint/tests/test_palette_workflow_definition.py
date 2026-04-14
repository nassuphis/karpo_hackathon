"""
Tests for the palette Step Functions ASL template.
"""
import json
import os
import re
import unittest

TEMPLATE_PATH = os.path.join(
    os.path.dirname(__file__), "..", "stepfunctions", "palette_workflow.asl.json.template"
)


def _load_asl():
    with open(TEMPLATE_PATH) as f:
        raw = f.read()
    rendered = re.sub(r'\$\{(\w+)\}', r'arn:aws:lambda:us-east-1:123456789012:function:placeholder-\1', raw)
    return json.loads(rendered)


class TestPaletteWorkflowDefinition(unittest.TestCase):

    def setUp(self):
        self.asl = _load_asl()
        self.top_states = self.asl["States"]
        wrapper = self.top_states.get("WorkflowWrapper", {})
        branches = wrapper.get("Branches", [{}])
        self.states = branches[0].get("States", {}) if branches else {}

    def test_template_parses(self):
        self.assertIn("WorkflowWrapper", self.top_states)
        self.assertIn("BuildPlan", self.states)

    def test_required_states_exist(self):
        for name in [
            "BuildPlanPhase", "BuildPlan", "ParsePlan",
            "ExtractActionChoice",
            "SolveScoreClipPhase", "SolveScoreClipTask",
            "SolveScoreHistPhase", "SolveScoreHistMap",
            "SolveScoreMergePhase", "SolveScoreMergeTask",
            "PaletteChunkPhase", "PaletteChunkMap",
            "PaletteFinalizePhase", "PaletteFinalizeTask",
            "PostFinalizeAttachChoice",
            "AttachAssociatedPalettePhase", "AttachAssociatedPaletteTask",
            "ReportDone",
        ]:
            self.assertIn(name, self.states, f"missing state: {name}")
        for name in ["WorkflowWrapper", "ReportError", "Fail", "Succeed"]:
            self.assertIn(name, self.top_states, f"missing top-level state: {name}")

    def test_map_states_have_expected_concurrency(self):
        self.assertEqual(self.states["SolveScoreHistMap"]["Type"], "Map")
        self.assertEqual(self.states["PaletteChunkMap"]["Type"], "Map")
        self.assertEqual(self.states["SolveScoreHistMap"]["MaxConcurrency"], 10)
        self.assertEqual(
            self.states["PaletteChunkMap"]["MaxConcurrencyPath"],
            "$.plan.params.palette_chunk_workers",
        )

    def test_parallel_wrapper_catches_all(self):
        wrapper = self.top_states["WorkflowWrapper"]
        self.assertEqual(wrapper["Type"], "Parallel")
        self.assertIn("States.ALL", wrapper["Catch"][0]["ErrorEquals"])
        self.assertEqual(wrapper["Catch"][0]["Next"], "ReportError")

    def test_worker_placeholders_present(self):
        asl_str = json.dumps(self.asl)
        self.assertIn("placeholder-PlanFunctionArn", asl_str)
        self.assertIn("placeholder-StatusFunctionArn", asl_str)
        self.assertIn("placeholder-SolveProximityFunctionArn", asl_str)
        self.assertIn("placeholder-PaletteChunkFunctionArn", asl_str)
        self.assertIn("placeholder-PaletteFinalizeFunctionArn", asl_str)
        self.assertIn("placeholder-AttachPaletteFunctionArn", asl_str)

    def test_finalize_task_preserves_omega_enabled(self):
        payload = self.states["PaletteFinalizeTask"]["Parameters"]["Payload"]
        self.assertEqual(payload["solve_score_chain.$"], "$.plan.solve_score.chain")
        self.assertEqual(payload["solve_score_omega.$"], "$.plan.solve_score.omega")
        self.assertEqual(
            payload["solve_score_omega_enabled.$"],
            "$.plan.solve_score.omega_enabled",
        )

    def test_solve_score_tasks_forward_critical_fields(self):
        clip = self.states["SolveScoreClipTask"]["Parameters"]["Payload"]
        self.assertEqual(clip["solve_score_quantile.$"], "$.plan.solve_score.quantile")
        self.assertEqual(clip["solve_score_omega.$"], "$.plan.solve_score.omega")
        self.assertEqual(clip["solve_score_omega_enabled.$"], "$.plan.solve_score.omega_enabled")
        self.assertEqual(clip["solve_score_threads.$"], "$.plan.params.solve_score_threads")
        self.assertEqual(clip["root_transforms.$"], "$.plan.params.root_transforms")
        self.assertEqual(clip["lores_params_key.$"], "$.plan.calc.lores_params_key")

        hist = self.states["SolveScoreHistMap"]["ItemSelector"]
        self.assertEqual(hist["solve_score_quantile.$"], "$.plan.solve_score.quantile")
        self.assertEqual(hist["solve_score_omega.$"], "$.plan.solve_score.omega")
        self.assertEqual(hist["solve_score_omega_enabled.$"], "$.plan.solve_score.omega_enabled")
        self.assertEqual(hist["solve_score_threads.$"], "$.plan.params.solve_score_threads")
        self.assertEqual(hist["solve_score_hist_input_mode.$"], "$.plan.params.solve_score_hist_input_mode")
        self.assertEqual(hist["solve_score_hist_retries.$"], "$.plan.params.solve_score_hist_retries")
        self.assertEqual(hist["root_transforms.$"], "$.plan.params.root_transforms")
        self.assertEqual(hist["params_key.$"], "$.plan.calc.params_key")
        self.assertEqual(hist["step_start.$"], "$$.Map.Item.Value.step_start")
        self.assertEqual(hist["step_count.$"], "$$.Map.Item.Value.step_count")

        merge = self.states["SolveScoreMergeTask"]["Parameters"]["Payload"]
        self.assertEqual(merge["solve_score_quantile.$"], "$.plan.solve_score.quantile")
        self.assertEqual(merge["solve_score_omega.$"], "$.plan.solve_score.omega")
        self.assertEqual(merge["solve_score_omega_enabled.$"], "$.plan.solve_score.omega_enabled")
        self.assertEqual(merge["solve_score_threads.$"], "$.plan.params.solve_score_threads")
        self.assertEqual(merge["solve_score_merge_workers.$"], "$.plan.params.solve_score_merge_workers")

    def test_palette_chunk_and_finalize_forward_critical_fields(self):
        chunk = self.states["PaletteChunkMap"]["ItemSelector"]
        self.assertEqual(chunk["solve_score_quantile.$"], "$.plan.solve_score.quantile")
        self.assertEqual(chunk["solve_score_omega.$"], "$.plan.solve_score.omega")
        self.assertEqual(chunk["solve_score_omega_enabled.$"], "$.plan.solve_score.omega_enabled")
        self.assertEqual(chunk["root_transforms.$"], "$.plan.params.root_transforms")
        self.assertEqual(chunk["palette_chunk_threads.$"], "$.plan.params.palette_chunk_threads")
        self.assertEqual(chunk["palette_chunk_input_mode.$"], "$.plan.params.palette_chunk_input_mode")
        self.assertEqual(chunk["palette_chunk_retries.$"], "$.plan.params.palette_chunk_retries")
        self.assertEqual(chunk["palette_chunk_workers.$"], "$.plan.params.palette_chunk_workers")
        self.assertEqual(chunk["bin_size.$"], "$$.Map.Item.Value.bin_size")
        self.assertEqual(chunk["params_key.$"], "$.plan.calc.params_key")
        self.assertEqual(chunk["step_start.$"], "$$.Map.Item.Value.step_start")
        self.assertEqual(chunk["step_count.$"], "$$.Map.Item.Value.step_count")

        finalize = self.states["PaletteFinalizeTask"]["Parameters"]["Payload"]
        self.assertEqual(finalize["times.$"], "$.plan.calc.times")
        self.assertEqual(finalize["solve_score_quantile.$"], "$.plan.solve_score.quantile")
        self.assertEqual(finalize["solve_score_omega.$"], "$.plan.solve_score.omega")
        self.assertEqual(finalize["solve_score_omega_enabled.$"], "$.plan.solve_score.omega_enabled")
        self.assertEqual(finalize["root_transforms.$"], "$.plan.params.root_transforms")
        self.assertEqual(finalize["cleanup_solve_score_scratch.$"], "$.plan.solve_score.cleanup_scratch")
        self.assertEqual(finalize["source_color_artifact_id.$"], "$.plan.extract.source_artifact_id")

    def test_extract_attach_branch_is_wired(self):
        choice = self.states["ExtractActionChoice"]
        choices = {(item["StringEquals"], item["Next"]) for item in choice["Choices"]}
        self.assertIn(("done", "ReportDone"), choices)
        self.assertIn(("attach", "AttachAssociatedPalettePhase"), choices)
        self.assertIn(("generate_reuse", "PaletteChunkPhase"), choices)
        self.assertEqual(choice["Default"], "SolveScoreClipPhase")

        post_choice = self.states["PostFinalizeAttachChoice"]
        self.assertEqual(post_choice["Choices"][0]["Variable"], "$.plan.attach.enabled")
        self.assertEqual(post_choice["Choices"][0]["Next"], "AttachAssociatedPalettePhase")
        self.assertEqual(post_choice["Default"], "ReportDone")

        attach = self.states["AttachAssociatedPaletteTask"]["Parameters"]["Payload"]
        self.assertEqual(attach["artifact_id.$"], "$.plan.attach.artifact_id")
        self.assertEqual(attach["associated_palette_mode.$"], "$.plan.attach.mode")
        self.assertEqual(attach["associated_palette_id.$"], "$.plan.attach.palette_id")
        self.assertEqual(attach["associated_palette_image_key.$"], "$.plan.attach.image_key")
        self.assertEqual(attach["associated_palette_preview_key.$"], "$.plan.attach.preview_key")
        self.assertEqual(attach["associated_palette_palette.$"], "$.plan.attach.palette")
        self.assertEqual(attach["associated_palette_metric.$"], "$.plan.attach.metric")
        self.assertEqual(attach["associated_palette_score_chain.$"], "$.plan.attach.score_chain")
        self.assertEqual(attach["associated_palette_quantile.$"], "$.plan.attach.quantile")
        self.assertEqual(attach["associated_palette_omega.$"], "$.plan.attach.omega")
        self.assertEqual(attach["associated_palette_omega_enabled.$"], "$.plan.attach.omega_enabled")


if __name__ == "__main__":
    unittest.main()
