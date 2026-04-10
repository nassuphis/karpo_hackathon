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
            "SolveScoreClipPhase", "SolveScoreClipTask",
            "SolveScoreHistPhase", "SolveScoreHistMap",
            "SolveScoreMergePhase", "SolveScoreMergeTask",
            "PaletteChunkPhase", "PaletteChunkMap",
            "PaletteFinalizePhase", "PaletteFinalizeTask",
            "ReportDone",
        ]:
            self.assertIn(name, self.states, f"missing state: {name}")
        for name in ["WorkflowWrapper", "ReportError", "Fail", "Succeed"]:
            self.assertIn(name, self.top_states, f"missing top-level state: {name}")

    def test_map_states_have_expected_concurrency(self):
        self.assertEqual(self.states["SolveScoreHistMap"]["Type"], "Map")
        self.assertEqual(self.states["PaletteChunkMap"]["Type"], "Map")
        self.assertEqual(self.states["SolveScoreHistMap"]["MaxConcurrency"], 10)
        self.assertEqual(self.states["PaletteChunkMap"]["MaxConcurrency"], 10)

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

    def test_finalize_task_preserves_omega_enabled(self):
        payload = self.states["PaletteFinalizeTask"]["Parameters"]["Payload"]
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
        self.assertEqual(clip["root_transforms.$"], "$.plan.params.root_transforms")

        hist = self.states["SolveScoreHistMap"]["ItemSelector"]
        self.assertEqual(hist["solve_score_quantile.$"], "$.plan.solve_score.quantile")
        self.assertEqual(hist["solve_score_omega.$"], "$.plan.solve_score.omega")
        self.assertEqual(hist["solve_score_omega_enabled.$"], "$.plan.solve_score.omega_enabled")
        self.assertEqual(hist["root_transforms.$"], "$.plan.params.root_transforms")

        merge = self.states["SolveScoreMergeTask"]["Parameters"]["Payload"]
        self.assertEqual(merge["solve_score_quantile.$"], "$.plan.solve_score.quantile")
        self.assertEqual(merge["solve_score_omega.$"], "$.plan.solve_score.omega")
        self.assertEqual(merge["solve_score_omega_enabled.$"], "$.plan.solve_score.omega_enabled")

    def test_palette_chunk_and_finalize_forward_critical_fields(self):
        chunk = self.states["PaletteChunkMap"]["ItemSelector"]
        self.assertEqual(chunk["solve_score_quantile.$"], "$.plan.solve_score.quantile")
        self.assertEqual(chunk["solve_score_omega.$"], "$.plan.solve_score.omega")
        self.assertEqual(chunk["solve_score_omega_enabled.$"], "$.plan.solve_score.omega_enabled")
        self.assertEqual(chunk["root_transforms.$"], "$.plan.params.root_transforms")

        finalize = self.states["PaletteFinalizeTask"]["Parameters"]["Payload"]
        self.assertEqual(finalize["times.$"], "$.plan.calc.times")
        self.assertEqual(finalize["solve_score_quantile.$"], "$.plan.solve_score.quantile")
        self.assertEqual(finalize["solve_score_omega.$"], "$.plan.solve_score.omega")
        self.assertEqual(finalize["solve_score_omega_enabled.$"], "$.plan.solve_score.omega_enabled")
        self.assertEqual(finalize["root_transforms.$"], "$.plan.params.root_transforms")


if __name__ == "__main__":
    unittest.main()
