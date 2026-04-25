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
        self.assertEqual(body["run_id"], "run_abc")

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
