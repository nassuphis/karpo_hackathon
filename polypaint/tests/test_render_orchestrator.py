"""
Tests for the render orchestrator starter Lambda.

Validates that the starter:
- validates mode
- calls StartExecution exactly once
- writes queued DDB row
- returns execution_arn
- does NOT self-invoke, poll, or dispatch workers

Run: cd polypaint && uv run python -m pytest tests/test_render_orchestrator.py -v
"""
import json
import os
import sys
import unittest
from unittest.mock import patch, MagicMock

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _make_event(payload):
    return payload


class TestStarterLambda(unittest.TestCase):
    def setUp(self):
        import handler_render_orchestrator as mod

        self._ddb_patcher = patch.object(mod, "ddb_client")
        self.mock_ddb = self._ddb_patcher.start()
        self.mock_ddb.query.return_value = {"Items": []}
        self.addCleanup(self._ddb_patcher.stop)

    @patch("handler_render_orchestrator.report_status")
    @patch("handler_render_orchestrator.sfn_client")
    def test_starter_calls_start_execution_once(self, mock_sfn, mock_report):
        mock_sfn.start_execution.return_value = {
            "executionArn": "arn:aws:states:us-east-1:123:execution:polypaint-render-workflow:render_color_run_abc"
        }
        from handler_render_orchestrator import handler
        event = _make_event({
            "job_id": "j", "run_id": "run_abc", "mode": "color",
            "params": {"pix": 512}
        })
        result = handler(event, None)
        body = json.loads(result["body"])

        assert mock_sfn.start_execution.call_count == 1
        call_kwargs = mock_sfn.start_execution.call_args[1]
        assert "polypaint" in call_kwargs["stateMachineArn"] or call_kwargs["stateMachineArn"] == ""
        assert call_kwargs["name"] == "render_color_run_abc"
        sfn_input = json.loads(call_kwargs["input"])
        assert sfn_input["job_id"] == "j"
        assert sfn_input["mode"] == "color"

    @patch("handler_render_orchestrator.report_status")
    @patch("handler_render_orchestrator.sfn_client")
    def test_starter_writes_queued_ddb_row(self, mock_sfn, mock_report):
        mock_sfn.start_execution.return_value = {
            "executionArn": "arn:aws:states:us-east-1:123:execution:test:test"
        }
        from handler_render_orchestrator import handler
        event = _make_event({
            "job_id": "j", "run_id": "run_q", "mode": "bilevel",
            "params": {"pix": 512}
        })
        handler(event, None)

        mock_report.assert_called_once()
        args = mock_report.call_args
        assert args[0][0] == "j"  # job_id
        assert args[0][1] == "render_run_bilevel_run_q"  # task_id
        assert args[0][2] == "queued"  # status
        rd = args[1]["result_data"]
        assert rd["phase"] == "queued"
        assert rd["phase_label"] == "Queued"
        assert "execution_arn" in rd

    @patch("handler_render_orchestrator.report_status")
    @patch("handler_render_orchestrator.sfn_client")
    def test_starter_returns_execution_arn(self, mock_sfn, mock_report):
        mock_sfn.start_execution.return_value = {
            "executionArn": "arn:aws:states:us-east-1:123:execution:wf:exec1"
        }
        from handler_render_orchestrator import handler
        event = _make_event({
            "job_id": "j", "run_id": "run_r", "mode": "color",
            "params": {"pix": 512}
        })
        result = handler(event, None)
        body = json.loads(result["body"])
        assert body["execution_arn"] == "arn:aws:states:us-east-1:123:execution:wf:exec1"
        assert body["task_id"] == "render_run_color_run_r"
        assert body["run_id"] == "run_r"

    @patch("handler_render_orchestrator.time.time", return_value=2.0)
    @patch("handler_render_orchestrator.report_status")
    @patch("handler_render_orchestrator.sfn_client")
    def test_starter_rejects_duplicate_active_render_run(self, mock_sfn, mock_report, mock_time):
        from handler_render_orchestrator import handler
        self.mock_ddb.query.return_value = {
            "Items": [{
                "task_id": {"S": "render_run_color_run_existing"},
                "task_status": {"S": "raster"},
                "updated_at_ms": {"N": "1000"},
                "result_data": {"S": json.dumps({
                    "phase": "raster",
                    "phase_label": "Raster",
                    "execution_arn": "arn:existing",
                })},
            }]
        }
        result = handler(_make_event({
            "job_id": "j", "run_id": "run_new", "mode": "color",
            "params": {"pix": 512},
        }), None)
        body = json.loads(result["body"])
        assert result["statusCode"] == 409
        assert body["active_task_id"] == "render_run_color_run_existing"
        assert body["active_phase"] == "raster"
        mock_sfn.start_execution.assert_not_called()
        mock_report.assert_not_called()

    @patch("handler_render_orchestrator.time.time", return_value=200000.0)
    @patch("handler_render_orchestrator.report_status")
    @patch("handler_render_orchestrator.sfn_client")
    def test_starter_ignores_stale_active_render_row(self, mock_sfn, mock_report, mock_time):
        from handler_render_orchestrator import handler
        mock_sfn.start_execution.return_value = {
            "executionArn": "arn:aws:states:us-east-1:123:execution:wf:new"
        }
        self.mock_ddb.query.return_value = {
            "Items": [{
                "task_id": {"S": "render_run_color_run_stale"},
                "task_status": {"S": "raster"},
                "updated_at_ms": {"N": str(100000 * 1000)},
                "result_data": {"S": json.dumps({"phase": "raster"})},
            }]
        }
        result = handler(_make_event({
            "job_id": "j", "run_id": "run_new", "mode": "color",
            "params": {"pix": 512},
        }), None)
        body = json.loads(result["body"])
        assert result["statusCode"] == 200
        assert body["run_id"] == "run_new"
        mock_sfn.start_execution.assert_called_once()

    @patch("handler_render_orchestrator.report_status")
    @patch("handler_render_orchestrator.sfn_client")
    def test_starter_rejects_invalid_mode(self, mock_sfn, mock_report):
        from handler_render_orchestrator import handler
        event = _make_event({
            "job_id": "j", "run_id": "run_bad", "mode": "invalid_mode",
            "params": {"pix": 512}
        })
        with self.assertRaises(RuntimeError) as ctx:
            handler(event, None)
        assert "invalid_mode" in str(ctx.exception).lower()
        mock_sfn.start_execution.assert_not_called()

    def test_starter_has_no_self_invoke(self):
        """The starter module must not contain self-invocation patterns."""
        import handler_render_orchestrator as mod
        source = open(mod.__file__).read()
        assert "_SelfReinvoke" not in source, "starter still contains _SelfReinvoke"
        assert "SELF_FUNCTION" not in source, "starter still contains SELF_FUNCTION"
        assert "_check_timeout" not in source, "starter still contains _check_timeout"
        assert "_poll_completion" not in source, "starter still contains _poll_completion"
        assert "run_color" not in source, "starter still contains run_color"

    def test_starter_does_not_block_on_execution(self):
        """The starter must not call DescribeExecution or poll for completion."""
        import handler_render_orchestrator as mod
        source = open(mod.__file__).read()
        assert "describe_execution" not in source.lower(), "starter must not call DescribeExecution"
        assert "poll_completion" not in source, "starter must not have poll_completion"
        assert "_poll" not in source, "starter must not have _poll"

    def test_dispatch_handler_uses_starter_not_sfn(self):
        """handler_dispatch.py must invoke the starter Lambda, not Step Functions directly."""
        dispatch_path = os.path.join(os.path.dirname(__file__), "..", "lambda", "handler_dispatch.py")
        source = open(dispatch_path).read()
        assert "start_execution" not in source.lower(), \
            "dispatch handler must not call StartExecution directly"
        assert "stepfunctions" not in source.lower(), \
            "dispatch handler must not import stepfunctions client"
        assert "render_orchestrator" in source, \
            "dispatch handler must still have render_orchestrator target"


if __name__ == "__main__":
    unittest.main()
