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
            "PlanPhase", "RouteExecutionMethod", "BuildPlan", "ParsePlan",
            "DegreeProbePhase", "DegreeProbeTask", "BuildFusedPlan", "ParseFusedPlan", "AttachFusedPost",
            "ParamGenPhase", "ParamGenMap",
            "CoeffgenPhase", "CoeffgenMap",
            "PostCoeffgenPhase", "PostCoeffgen", "ParsePostCoeffgen",
            "LoresParamGenPhase", "LoresParamGenTask",
            "LoresCoeffgenPhase", "LoresCoeffgenTask",
            "LoresSolvePhase", "LoresSolveTask",
            "RouteHiresExecution", "SolvePhase", "SolveMap", "FusedChunkPhase", "FusedChunkMap",
            "SaveMetadataPhase", "SaveMetadataTask",
            "ReportDone",
        ]:
            self.assertIn(name, self.states, f"missing compute state: {name}")
        for name in ["WorkflowWrapper", "ReportError", "Fail", "Succeed"]:
            self.assertIn(name, self.top_states, f"missing top-level state: {name}")

    def test_required_map_states(self):
        self.assertEqual(self.states["ParamGenMap"]["Type"], "Map")
        self.assertEqual(self.states["CoeffgenMap"]["Type"], "Map")
        self.assertEqual(self.states["SolveMap"]["Type"], "Map")
        self.assertEqual(self.states["FusedChunkMap"]["Type"], "Map")

    def test_map_concurrency_matches_current_compute_shape(self):
        self.assertEqual(self.states["ParamGenMap"]["MaxConcurrency"], 50)
        self.assertEqual(self.states["CoeffgenMap"]["MaxConcurrency"], 50)
        self.assertEqual(self.states["SolveMap"]["MaxConcurrency"], 50)
        self.assertEqual(self.states["FusedChunkMap"]["MaxConcurrency"], 50)

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

    def test_report_error_forwards_stepfunctions_error_payload(self):
        report_error = self.top_states["ReportError"]
        payload = report_error["Parameters"]["Payload"]
        self.assertEqual(payload["action"], "error")
        self.assertEqual(payload["Error.$"], "$.error_info.Error")
        self.assertEqual(payload["Cause.$"], "$.error_info.Cause")

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
        retry_errors = solve_worker["Retry"][0]["ErrorEquals"]
        self.assertIn("Lambda.TooManyRequestsException", retry_errors)

        fused_worker = self.states["FusedChunkMap"]["ItemProcessor"]["States"]["FusedChunkWorker"]
        self.assertEqual(fused_worker["OutputPath"], "$.body")
        self.assertIn("States.StringToJson($.Payload.body)", json.dumps(fused_worker["ResultSelector"]))

    def test_solve_map_derives_items_from_plan_not_post(self):
        solve_map = self.states["SolveMap"]
        self.assertEqual(solve_map["ItemsPath"], "$.plan.chunk_items")
        selector = solve_map["ItemSelector"]
        self.assertEqual(selector["n_coeffs.$"], "$.post.n_coeffs")
        self.assertEqual(selector["task_id.$"], "$$.Map.Item.Value.solve_task_id")
        self.assertEqual(selector["s3_key.$"], "$$.Map.Item.Value.bin_key")

    def test_fused_route_and_worker_contract_exist(self):
        route = self.states["RouteExecutionMethod"]
        route_json = json.dumps(route)
        self.assertIn("fused_chunk_pipeline", route_json)
        self.assertIn("DegreeProbePhase", route_json)
        self.assertNotIn('"BooleanEquals": true', route_json)

        hires_route = self.states["RouteHiresExecution"]
        self.assertIn("fused_chunk_pipeline", json.dumps(hires_route))
        self.assertIn("FusedChunkPhase", json.dumps(hires_route))

        fused_map = self.states["FusedChunkMap"]
        selector = fused_map["ItemSelector"]
        self.assertEqual(selector["task_id.$"], "$$.Map.Item.Value.fused_task_id")
        self.assertEqual(selector["n_coeffs.$"], "$.post.n_coeffs")
        self.assertEqual(selector["degree.$"], "$.post.degree")
        self.assertEqual(selector["fused_threads.$"], "$.plan.fused.threads")
        self.assertEqual(selector["param_program.$"], "$.plan.pipeline.param_program")
        worker = fused_map["ItemProcessor"]["States"]["FusedChunkWorker"]
        self.assertIn("placeholder-FusedChunkFunctionArn", json.dumps(worker))

    def test_degree_probe_task_does_not_depend_on_removed_auto_chunk_flag(self):
        degree_probe = self.states["DegreeProbeTask"]["Parameters"]["Payload"]
        payload_json = json.dumps(degree_probe)
        self.assertNotIn("auto_hires_chunks", payload_json)
        self.assertEqual(degree_probe["execution_method"], "fused_chunk_pipeline")
        self.assertEqual(degree_probe["param_program_chain.$"], "$.params.param_program_chain")
        self.assertEqual(degree_probe["param_program_source_text.$"], "$.params.param_program_source_text")
        self.assertEqual(degree_probe["coeff_program_source_text.$"], "$.params.coeff_program_source_text")

    def test_parse_post_coeffgen_drops_large_coeffgen_results(self):
        parse_post = self.states["ParsePostCoeffgen"]
        params = json.dumps(parse_post["Parameters"])
        self.assertNotIn('"coeffgen_results.$"', params)

    def test_coeffgen_map_discards_worker_outputs(self):
        coeffgen_map = self.states["CoeffgenMap"]
        self.assertIsNone(coeffgen_map.get("ResultPath"))

    def test_hires_maps_discard_worker_outputs(self):
        self.assertIsNone(self.states["SolveMap"].get("ResultPath"))
        self.assertIsNone(self.states["FusedChunkMap"].get("ResultPath"))

    def test_post_coeffgen_uses_task_prefix_not_coeffgen_results(self):
        post = self.states["PostCoeffgen"]
        params = json.dumps(post["Parameters"])
        self.assertIn('"task_prefix.$": "$.plan.coeffgen.task_prefix"', params)
        self.assertNotIn('"coeffgen_results.$"', params)

    def test_save_metadata_uses_task_prefix_not_solve_results(self):
        save = self.states["SaveMetadataTask"]
        params = json.dumps(save["Parameters"])
        self.assertIn('"job_id.$": "$.job_id"', params)
        self.assertIn('"task_prefix.$": "$.plan.solve.task_prefix"', params)
        self.assertIn('"expected.$": "$.plan.compute.n_chunks"', params)
        self.assertNotIn('"solve_results.$"', params)

    def test_post_coeffgen_phase_is_explicit(self):
        post_phase = self.states["PostCoeffgenPhase"]
        self.assertEqual(post_phase["Type"], "Task")
        self.assertEqual(post_phase["Parameters"]["Payload"]["phase"], "post_coeffgen")
        self.assertEqual(post_phase["Parameters"]["Payload"]["phase_label"], "Post coeffgen")

    def test_param_gen_tasks_forward_times_and_param_transforms(self):
        self.assertEqual(self.states["ParamGenPhase"]["Parameters"]["Payload"]["expected.$"], "$.plan.compute.n_chunks")
        param_gen = self.states["ParamGenMap"]["ItemSelector"]
        self.assertEqual(param_gen["times.$"], "$.plan.compute.times")
        self.assertEqual(param_gen["param_transforms.$"], "$.plan.pipeline.param_transforms")
        self.assertEqual(param_gen["param_program.$"], "$.plan.pipeline.param_program")
        self.assertEqual(param_gen["n_threads.$"], "$.plan.compute.param_gen_threads")
        self.assertEqual(param_gen["task_id.$"], "$$.Map.Item.Value.paramgen_task_id")
        self.assertEqual(param_gen["params_key.$"], "$$.Map.Item.Value.params_key")
        self.assertEqual(param_gen["step_start.$"], "$$.Map.Item.Value.step_start")
        self.assertEqual(param_gen["step_count.$"], "$$.Map.Item.Value.step_count")

        lores_param_gen = self.states["LoresParamGenTask"]["Parameters"]["Payload"]
        self.assertEqual(lores_param_gen["times.$"], "$.plan.compute.times")
        self.assertEqual(lores_param_gen["param_transforms.$"], "$.plan.pipeline.param_transforms")
        self.assertEqual(lores_param_gen["param_program.$"], "$.plan.pipeline.param_program")
        self.assertEqual(lores_param_gen["gridN.$"], "$.plan.compute.N")
        self.assertEqual(lores_param_gen["n_threads.$"], "$.plan.compute.lores_param_gen_threads")

    def test_coeffgen_tasks_forward_pipeline_fields(self):
        coeffgen = self.states["CoeffgenMap"]["ItemSelector"]
        self.assertEqual(coeffgen["function.$"], "$.plan.pipeline.function")
        self.assertEqual(coeffgen["coeff_transforms.$"], "$.plan.pipeline.coeff_transforms")
        self.assertEqual(coeffgen["cfpv.$"], "$.plan.pipeline.cfpv")
        self.assertEqual(coeffgen["params_key.$"], "$$.Map.Item.Value.params_key")
        self.assertEqual(coeffgen["params_step_start.$"], "$$.Map.Item.Value.params_step_start")
        self.assertEqual(coeffgen["params_step_count.$"], "$$.Map.Item.Value.params_step_count")
        self.assertEqual(coeffgen["N.$"], "$.plan.compute.N")
        self.assertEqual(coeffgen["n_threads.$"], "$.plan.compute.coeffgen_threads")

        lores_coeffgen = self.states["LoresCoeffgenTask"]["Parameters"]["Payload"]
        self.assertEqual(lores_coeffgen["function.$"], "$.plan.pipeline.function")
        self.assertEqual(lores_coeffgen["coeff_transforms.$"], "$.plan.pipeline.coeff_transforms")
        self.assertEqual(lores_coeffgen["cfpv.$"], "$.plan.pipeline.cfpv")
        self.assertEqual(lores_coeffgen["N.$"], "$.post.lores.N")
        self.assertEqual(lores_coeffgen["n_threads.$"], "$.post.lores.coeffgen_threads")

    def test_solve_tasks_forward_chunk_and_solver_fields(self):
        lores_solve = self.states["LoresSolveTask"]["Parameters"]
        self.assertEqual(lores_solve["FunctionName.$"], "$.plan.solve.function_name")
        lores_payload = lores_solve["Payload"]
        self.assertEqual(lores_payload["coeffs_key.$"], "$.post.lores.coeffs_key")
        self.assertEqual(lores_payload["n_coeffs.$"], "$.post.n_coeffs")
        self.assertEqual(lores_payload["n_steps.$"], "$.post.lores.n_steps")
        self.assertEqual(lores_payload["s3_key.$"], "$.post.lores.bin_key")

        solve_map = self.states["SolveMap"]["ItemSelector"]
        self.assertEqual(solve_map["coeffs_key.$"], "$$.Map.Item.Value.coeffs_key")
        self.assertEqual(solve_map["n_coeffs.$"], "$.post.n_coeffs")
        self.assertEqual(solve_map["n_steps.$"], "$$.Map.Item.Value.step_count")
        self.assertEqual(solve_map["s3_key.$"], "$$.Map.Item.Value.bin_key")
        self.assertEqual(solve_map["solver_function_name.$"], "$.plan.solve.function_name")


if __name__ == "__main__":
    unittest.main()
