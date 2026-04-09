"""
Tests for the compute Step Functions ASL template.
"""
import json
import os
import re
import unittest

TEMPLATE_PATH = os.path.join(
    os.path.dirname(__file__), "..", "stepfunctions", "compute_workflow.asl.json.template"
)


def _load_asl():
    with open(TEMPLATE_PATH) as f:
        raw = f.read()
    rendered = re.sub(r'\$\{(\w+)\}', r'arn:aws:lambda:us-east-1:123456789012:function:placeholder-\1', raw)
    return json.loads(rendered)


class TestComputeWorkflowDefinition(unittest.TestCase):

    def setUp(self):
        self.asl = _load_asl()
        self.top_states = self.asl["States"]
        wrapper = self.top_states.get("WorkflowWrapper", {})
        branches = wrapper.get("Branches", [{}])
        self.states = branches[0].get("States", {}) if branches else {}
        self.all_states = {**self.top_states, **self.states}

    def test_template_parses(self):
        self.assertIn("StartAt", self.asl)
        self.assertIn("States", self.asl)
        self.assertGreater(len(self.states), 0)

    def test_required_states_exist(self):
        for name in [
            "PlanPhase", "BuildPlan", "ParsePlan",
            "ParamGenPhase", "ParamGenTask",
            "CoeffgenPhase", "CoeffgenMap",
            "PostCoeffgenPhase", "PostCoeffgen", "ParsePostCoeffgen",
            "LoresParamGenPhase", "LoresParamGenTask",
            "LoresCoeffgenPhase", "LoresCoeffgenTask",
            "LoresSolvePhase", "LoresSolveTask",
            "SolvePhase", "SolveMap",
            "SaveMetadataPhase", "SaveMetadataTask",
            "ReportDone",
        ]:
            self.assertIn(name, self.states, f"missing compute state: {name}")
        for name in ["WorkflowWrapper", "ReportError", "Fail", "Succeed"]:
            self.assertIn(name, self.top_states, f"missing top-level state: {name}")

    def test_required_map_states(self):
        self.assertEqual(self.states["CoeffgenMap"]["Type"], "Map")
        self.assertEqual(self.states["SolveMap"]["Type"], "Map")

    def test_map_concurrency_matches_current_compute_shape(self):
        self.assertEqual(self.states["CoeffgenMap"]["MaxConcurrency"], 50)
        self.assertEqual(self.states["SolveMap"]["MaxConcurrency"], 500)

    def test_worker_states_have_retry(self):
        for name, state in self.states.items():
            if state.get("Type") != "Map":
                continue
            inner_states = state["ItemProcessor"]["States"]
            for inner_name, inner in inner_states.items():
                if inner.get("Type") == "Task":
                    self.assertTrue(inner.get("Retry"), f"{name}/{inner_name} missing Retry")

    def test_no_state_targets_compute_orchestrator(self):
        asl_str = json.dumps(self.asl)
        self.assertNotIn("polypaint-compute-orchestrator", asl_str)

    def test_wrapper_catches_all(self):
        wrapper = self.top_states["WorkflowWrapper"]
        self.assertEqual(wrapper["Type"], "Parallel")
        self.assertIn("States.ALL", wrapper["Catch"][0]["ErrorEquals"])
        self.assertEqual(wrapper["Catch"][0]["Next"], "ReportError")

    def test_status_tasks_use_null_result_path(self):
        for name, state in self.all_states.items():
            if state.get("Type") != "Task":
                continue
            if "Phase" in name or "Report" in name:
                self.assertIsNone(state.get("ResultPath"), f"{name} must use ResultPath: null")

    def test_coeffgen_and_solve_worker_outputs_are_parsed_json(self):
        coeffgen_worker = self.states["CoeffgenMap"]["ItemProcessor"]["States"]["CoeffgenWorker"]
        self.assertEqual(coeffgen_worker["OutputPath"], "$.body")
        self.assertIn("States.StringToJson($.Payload.body)", json.dumps(coeffgen_worker["ResultSelector"]))

        solve_worker = self.states["SolveMap"]["ItemProcessor"]["States"]["SolveWorker"]
        self.assertEqual(solve_worker["OutputPath"], "$.body")
        self.assertIn("States.StringToJson($.Payload.body)", json.dumps(solve_worker["ResultSelector"]))

    def test_solve_map_derives_items_from_plan_not_post(self):
        solve_map = self.states["SolveMap"]
        self.assertEqual(solve_map["ItemsPath"], "$.plan.chunk_items")
        selector = solve_map["ItemSelector"]
        self.assertEqual(selector["n_coeffs.$"], "$.post.n_coeffs")
        self.assertEqual(selector["task_id.$"], "$$.Map.Item.Value.solve_task_id")
        self.assertEqual(selector["s3_key.$"], "$$.Map.Item.Value.bin_key")

    def test_parse_post_coeffgen_drops_large_coeffgen_results(self):
        parse_post = self.states["ParsePostCoeffgen"]
        params = json.dumps(parse_post["Parameters"])
        self.assertNotIn('"coeffgen_results.$"', params)

    def test_post_coeffgen_phase_is_explicit(self):
        post_phase = self.states["PostCoeffgenPhase"]
        self.assertEqual(post_phase["Type"], "Task")
        self.assertEqual(post_phase["Parameters"]["Payload"]["phase"], "post_coeffgen")
        self.assertEqual(post_phase["Parameters"]["Payload"]["phase_label"], "Post coeffgen")


if __name__ == "__main__":
    unittest.main()
