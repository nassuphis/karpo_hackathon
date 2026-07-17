"""
Tests for the compute orchestrator starter Lambda.

Validates that the starter:
- validates solver mode
- calls StartExecution exactly once
- writes queued DDB row
- returns execution_arn
- does NOT dispatch workers itself
"""
import json
import os
import sys
import unittest
from unittest.mock import patch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _make_event(payload):
    return payload


class TestComputeStarterLambda(unittest.TestCase):
    def setUp(self):
        import handler_compute_orchestrator as mod

        self._ddb_patcher = patch.object(mod, "ddb_client")
        self.mock_ddb = self._ddb_patcher.start()
        self.mock_ddb.query.return_value = {"Items": []}
        self.addCleanup(self._ddb_patcher.stop)

    @patch("handler_compute_orchestrator.report_status")
    @patch("handler_compute_orchestrator.sfn_client")
    def test_rejects_unsafe_run_id_before_start_execution(self, mock_sfn, mock_report):
        # code-review-27 F9: run_id feeds the SFN execution name + DDB keys.
        # Validation raises before start_execution, so don't touch the ARN
        # global (it would leak into sibling tests).
        import handler_compute_orchestrator as mod
        for bad in ("run/../evil", "run abc", "run:1", "run\n2", "x" * 65):
            with self.assertRaises(ValueError):
                mod.handler(_make_event({
                    "job_id": "j", "run_id": bad,
                    "params": {"solver_mode": "aberth_mt", "N": 100, "n_chunks": 10,
                               "function": "g1"}}), None)
        mock_sfn.start_execution.assert_not_called()

    @patch("handler_compute_orchestrator.report_status")
    @patch("handler_compute_orchestrator.sfn_client")
    def test_stop_action_kills_execution_and_marks_status(self, mock_sfn, mock_report):
        """The jobs-rail kill: action=stop calls StopExecution with the run's
        ARN and marks the status row as a user stop (the client poll then
        terminates through its normal error path)."""
        import handler_compute_orchestrator as mod

        class _NoSuch(Exception):
            pass
        mock_sfn.exceptions.ExecutionDoesNotExist = _NoSuch

        arn = "arn:aws:states:us-east-1:123:execution:wf:compute_aberth_mt_run_x"
        result = mod.handler(_make_event({
            "action": "stop",
            "job_id": "j",
            "task_id": "compute_run_aberth_mt_run_x",
            "execution_arn": arn,
        }), None)
        body = json.loads(result["body"])
        self.assertTrue(body["stopped"])
        mock_sfn.stop_execution.assert_called_once()
        kwargs = mock_sfn.stop_execution.call_args.kwargs
        self.assertEqual(kwargs["executionArn"], arn)
        self.assertEqual(kwargs["error"], "UserStopped")
        mock_sfn.start_execution.assert_not_called()
        args = mock_report.call_args.args
        self.assertEqual(args[0], "j")
        self.assertEqual(args[1], "compute_run_aberth_mt_run_x")
        self.assertEqual(args[2], "error")
        self.assertEqual(args[3], "Stopped by user")

        # already-finished executions still mark the row stopped
        mock_report.reset_mock()
        mock_sfn.stop_execution.side_effect = _NoSuch()
        result = mod.handler(_make_event({
            "action": "stop", "job_id": "j",
            "task_id": "compute_run_aberth_mt_run_x", "execution_arn": arn,
        }), None)
        self.assertTrue(json.loads(result["body"])["stopped"])
        self.assertEqual(mock_report.call_args.args[2], "error")

        # malformed ARNs are rejected before any AWS call
        mock_sfn.stop_execution.reset_mock()
        mock_sfn.stop_execution.side_effect = None
        with self.assertRaises(RuntimeError):
            mod.handler(_make_event({
                "action": "stop", "job_id": "j",
                "task_id": "t", "execution_arn": "not-an-arn",
            }), None)
        mock_sfn.stop_execution.assert_not_called()

    @patch("handler_compute_orchestrator.report_status")
    @patch("handler_compute_orchestrator.sfn_client")
    def test_starter_calls_start_execution_once(self, mock_sfn, mock_report):
        mock_sfn.start_execution.return_value = {
            "executionArn": "arn:aws:states:us-east-1:123:execution:polypaint-compute-workflow:compute_aberth_mt_run_abc"
        }
        import handler_compute_orchestrator as mod
        mod.STATE_MACHINE_ARN = "arn:aws:states:us-east-1:123:stateMachine:test"
        handler = mod.handler
        event = _make_event({
            "job_id": "j",
            "run_id": "run_abc",
            "params": {"solver_mode": "aberth_mt", "N": 100, "n_chunks": 10, "function": "g1", "param_gen_threads": 6, "coeffgen_threads": 5, "lores_param_gen_threads": 2, "lores_coeffgen_threads": 3},
        })
        result = handler(event, None)
        body = json.loads(result["body"])

        self.assertEqual(mock_sfn.start_execution.call_count, 1)
        call_kwargs = mock_sfn.start_execution.call_args[1]
        self.assertEqual(call_kwargs["name"], "compute_aberth_mt_run_abc")
        sfn_input = json.loads(call_kwargs["input"])
        self.assertEqual(sfn_input["job_id"], "j")
        self.assertEqual(sfn_input["params"]["solver_mode"], "aberth_mt")
        self.assertEqual(sfn_input["params"]["param_gen_threads"], 6)
        self.assertEqual(sfn_input["params"]["coeffgen_threads"], 5)
        self.assertEqual(sfn_input["params"]["lores_param_gen_threads"], 2)
        self.assertEqual(sfn_input["params"]["lores_coeffgen_threads"], 3)
        self.assertNotIn("pipeline_mode", sfn_input["params"])
        self.assertNotIn("param_transforms", sfn_input["params"])
        self.assertNotIn("coeff_transforms", sfn_input["params"])
        self.assertEqual(sfn_input["params"]["param_program_chain"], [])
        self.assertEqual(sfn_input["params"]["coeff_program_chain"], [])
        self.assertEqual(body["run_id"], "run_abc")

    @patch("handler_compute_orchestrator.report_status")
    @patch("handler_compute_orchestrator.sfn_client")
    def test_starter_defaults_program_mode_when_program_chains_present(self, mock_sfn, mock_report):
        mock_sfn.start_execution.return_value = {
            "executionArn": "arn:aws:states:us-east-1:123:execution:wf:exec_program"
        }
        import handler_compute_orchestrator as mod
        mod.STATE_MACHINE_ARN = "arn:aws:states:us-east-1:123:stateMachine:test"
        mod.handler(_make_event({
            "job_id": "j",
            "run_id": "run_program",
            "params": {
                "solver_mode": "aberth_mt",
                "N": 100,
                "n_chunks": 8,
                "function": "g1",
                "coeff_program_chain": [["const", "35", "p1+p2"], ["emit"]],
            },
        }), None)
        sfn_input = json.loads(mock_sfn.start_execution.call_args.kwargs["input"])
        self.assertNotIn("pipeline_mode", sfn_input["params"])
        self.assertNotIn("param_transforms", sfn_input["params"])
        self.assertEqual(sfn_input["params"]["param_program_chain"], [])
        self.assertNotIn("coeff_transforms", sfn_input["params"])
        self.assertEqual(sfn_input["params"]["coeff_program_chain"], [["const", "35", "p1+p2"], ["emit"]])

    @patch("handler_compute_orchestrator.report_status")
    @patch("handler_compute_orchestrator.sfn_client")
    def test_starter_writes_queued_ddb_row(self, mock_sfn, mock_report):
        mock_sfn.start_execution.return_value = {
            "executionArn": "arn:aws:states:us-east-1:123:execution:test:test"
        }
        import handler_compute_orchestrator as mod
        mod.STATE_MACHINE_ARN = "arn:aws:states:us-east-1:123:stateMachine:test"
        handler = mod.handler
        handler(_make_event({
            "job_id": "j",
            "run_id": "run_q",
            "params": {"solver_mode": "companion_matrix", "N": 100, "n_chunks": 4, "function": "g1"},
        }), None)

        mock_report.assert_called_once()
        args = mock_report.call_args
        self.assertEqual(args[0][0], "j")
        self.assertEqual(args[0][1], "compute_run_companion_matrix_run_q")
        self.assertEqual(args[0][2], "queued")
        rd = args[1]["result_data"]
        self.assertEqual(rd["phase"], "queued")
        self.assertEqual(rd["solver_mode"], "companion_matrix")
        self.assertIn("execution_arn", rd)

    @patch("handler_compute_orchestrator.report_status")
    @patch("handler_compute_orchestrator.sfn_client")
    def test_starter_returns_execution_arn(self, mock_sfn, mock_report):
        mock_sfn.start_execution.return_value = {
            "executionArn": "arn:aws:states:us-east-1:123:execution:wf:exec1"
        }
        import handler_compute_orchestrator as mod
        mod.STATE_MACHINE_ARN = "arn:aws:states:us-east-1:123:stateMachine:test"
        handler = mod.handler
        result = handler(_make_event({
            "job_id": "j",
            "run_id": "run_r",
            "params": {"solver_mode": "aberth_mt", "N": 100, "n_chunks": 8, "function": "g1"},
        }), None)
        body = json.loads(result["body"])
        self.assertEqual(body["execution_arn"], "arn:aws:states:us-east-1:123:execution:wf:exec1")
        self.assertEqual(body["task_id"], "compute_run_aberth_mt_run_r")
        self.assertEqual(body["run_id"], "run_r")

    @patch("handler_compute_orchestrator.time.time", return_value=2.0)
    @patch("handler_compute_orchestrator.report_status")
    @patch("handler_compute_orchestrator.sfn_client")
    def test_starter_rejects_duplicate_active_compute_run(self, mock_sfn, mock_report, mock_time):
        import handler_compute_orchestrator as mod
        mod.STATE_MACHINE_ARN = "arn:aws:states:us-east-1:123:stateMachine:test"
        self.mock_ddb.query.return_value = {
            "Items": [{
                "task_id": {"S": "compute_run_aberth_mt_run_existing"},
                "task_status": {"S": "coeffgen"},
                "updated_at_ms": {"N": "1000"},
                "result_data": {"S": json.dumps({
                    "phase": "coeffgen",
                    "phase_label": "Coeffgen",
                    "execution_arn": "arn:existing",
                })},
            }]
        }
        result = mod.handler(_make_event({
            "job_id": "j",
            "run_id": "run_new",
            "params": {"solver_mode": "aberth_mt", "N": 100, "n_chunks": 8, "function": "g1"},
        }), None)
        body = json.loads(result["body"])
        self.assertEqual(result["statusCode"], 409)
        self.assertEqual(body["active_task_id"], "compute_run_aberth_mt_run_existing")
        self.assertEqual(body["active_phase"], "coeffgen")
        mock_sfn.start_execution.assert_not_called()
        mock_report.assert_not_called()

    @patch("handler_compute_orchestrator.time.time", return_value=200000.0)
    @patch("handler_compute_orchestrator.report_status")
    @patch("handler_compute_orchestrator.sfn_client")
    def test_starter_ignores_stale_active_compute_row(self, mock_sfn, mock_report, mock_time):
        import handler_compute_orchestrator as mod
        mod.STATE_MACHINE_ARN = "arn:aws:states:us-east-1:123:stateMachine:test"
        mock_sfn.start_execution.return_value = {
            "executionArn": "arn:aws:states:us-east-1:123:execution:wf:new"
        }
        self.mock_ddb.query.return_value = {
            "Items": [{
                "task_id": {"S": "compute_run_aberth_mt_run_stale"},
                "task_status": {"S": "coeffgen"},
                "updated_at_ms": {"N": str(100000 * 1000)},
                "result_data": {"S": json.dumps({"phase": "coeffgen"})},
            }]
        }
        result = mod.handler(_make_event({
            "job_id": "j",
            "run_id": "run_new",
            "params": {"solver_mode": "aberth_mt", "N": 100, "n_chunks": 8, "function": "g1"},
        }), None)
        body = json.loads(result["body"])
        self.assertEqual(result["statusCode"], 200)
        self.assertEqual(body["run_id"], "run_new")
        mock_sfn.start_execution.assert_called_once()

    @patch("handler_compute_orchestrator.report_status")
    @patch("handler_compute_orchestrator.sfn_client")
    def test_starter_forwards_explicit_fused_execution_method(self, mock_sfn, mock_report):
        mock_sfn.start_execution.return_value = {
            "executionArn": "arn:aws:states:us-east-1:123:execution:wf:exec2"
        }
        import handler_compute_orchestrator as mod
        mod.STATE_MACHINE_ARN = "arn:aws:states:us-east-1:123:stateMachine:test"
        mod.handler(_make_event({
            "job_id": "j",
            "run_id": "run_f",
            "params": {"solver_mode": "aberth_mt", "N": 100, "n_chunks": 8, "function": "g1", "execution_method": "fused_chunk_pipeline"},
        }), None)
        sfn_input = json.loads(mock_sfn.start_execution.call_args.kwargs["input"])
        self.assertEqual(sfn_input["params"]["execution_method"], "fused_chunk_pipeline")
        self.assertNotIn("fused", sfn_input["params"])

    @patch("handler_compute_orchestrator.report_status")
    @patch("handler_compute_orchestrator.sfn_client")
    def test_starter_forwards_fused_execution_method_for_companion_matrix(self, mock_sfn, mock_report):
        mock_sfn.start_execution.return_value = {
            "executionArn": "arn:aws:states:us-east-1:123:execution:wf:exec_cm"
        }
        import handler_compute_orchestrator as mod
        mod.STATE_MACHINE_ARN = "arn:aws:states:us-east-1:123:stateMachine:test"
        mod.handler(_make_event({
            "job_id": "j",
            "run_id": "run_cm_f",
            "params": {
                "solver_mode": "companion_matrix",
                "N": 100,
                "n_chunks": 8,
                "function": "g1",
                "execution_method": "fused_chunk_pipeline",
            },
        }), None)
        sfn_input = json.loads(mock_sfn.start_execution.call_args.kwargs["input"])
        self.assertEqual(sfn_input["params"]["solver_mode"], "companion_matrix")
        self.assertEqual(sfn_input["params"]["execution_method"], "fused_chunk_pipeline")

    @patch("handler_compute_orchestrator.report_status")
    @patch("handler_compute_orchestrator.sfn_client")
    def test_starter_resolves_conflicting_fused_inputs_to_explicit_method(self, mock_sfn, mock_report):
        mock_sfn.start_execution.return_value = {
            "executionArn": "arn:aws:states:us-east-1:123:execution:wf:exec3"
        }
        import handler_compute_orchestrator as mod
        mod.STATE_MACHINE_ARN = "arn:aws:states:us-east-1:123:stateMachine:test"
        mod.handler(_make_event({
            "job_id": "j",
            "run_id": "run_c",
            "params": {
                "solver_mode": "aberth_mt",
                "N": 100,
                "n_chunks": 8,
                "function": "g1",
                "fused": True,
                "execution_method": "classic_chunk_pipeline",
            },
        }), None)
        sfn_input = json.loads(mock_sfn.start_execution.call_args.kwargs["input"])
        self.assertEqual(sfn_input["params"]["execution_method"], "classic_chunk_pipeline")
        self.assertNotIn("fused", sfn_input["params"])

    @patch("handler_compute_orchestrator.report_status")
    @patch("handler_compute_orchestrator.sfn_client")
    def test_starter_ignores_legacy_fused_flag_without_execution_method(self, mock_sfn, mock_report):
        mock_sfn.start_execution.return_value = {
            "executionArn": "arn:aws:states:us-east-1:123:execution:wf:exec4"
        }
        import handler_compute_orchestrator as mod
        mod.STATE_MACHINE_ARN = "arn:aws:states:us-east-1:123:stateMachine:test"
        mod.handler(_make_event({
            "job_id": "j",
            "run_id": "run_legacy_f",
            "params": {"solver_mode": "aberth_mt", "N": 100, "n_chunks": 8, "function": "g1", "fused": True},
        }), None)
        sfn_input = json.loads(mock_sfn.start_execution.call_args.kwargs["input"])
        self.assertEqual(sfn_input["params"]["execution_method"], "classic_chunk_pipeline")
        self.assertNotIn("fused", sfn_input["params"])

    @patch("handler_compute_orchestrator.report_status")
    @patch("handler_compute_orchestrator.sfn_client")
    def test_starter_rejects_invalid_solver_mode(self, mock_sfn, mock_report):
        from handler_compute_orchestrator import handler
        with self.assertRaises(RuntimeError) as ctx:
            handler(_make_event({
                "job_id": "j",
                "run_id": "run_bad",
                "params": {"solver_mode": "bogus", "N": 100, "n_chunks": 8, "function": "g1"},
            }), None)
        self.assertIn("solver_mode", str(ctx.exception))
        mock_sfn.start_execution.assert_not_called()

    @patch("handler_compute_orchestrator.report_status")
    @patch("handler_compute_orchestrator.sfn_client")
    def test_starter_rejects_removed_ae_solver_mode(self, mock_sfn, mock_report):
        from handler_compute_orchestrator import handler
        with self.assertRaises(RuntimeError) as ctx:
            handler(_make_event({
                "job_id": "j",
                "run_id": "run_ae",
                "params": {"solver_mode": "aberth", "N": 100, "n_chunks": 8, "function": "g1"},
            }), None)
        self.assertIn("aberth", str(ctx.exception))
        mock_sfn.start_execution.assert_not_called()

    def test_dispatch_handler_uses_compute_starter(self):
        dispatch_path = os.path.join(os.path.dirname(__file__), "..", "lambda", "handler_dispatch.py")
        source = open(dispatch_path).read()
        self.assertIn("compute_orchestrator", source)


if __name__ == "__main__":
    unittest.main()
