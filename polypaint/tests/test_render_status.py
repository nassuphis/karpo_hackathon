"""
Tests for the render status Lambda.

Validates DDB row writes for queued/phase/done/error actions
and error message extraction from Step Functions error envelopes.

Run: cd polypaint && uv run python -m pytest tests/test_render_status.py -v
"""
import json
import os
import sys
import unittest
from unittest.mock import patch, MagicMock

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class TestRenderStatus(unittest.TestCase):

    @patch("handler_render_status._get_ddb")
    def test_queued_row_write(self, mock_get_ddb):
        from handler_render_status import handler
        event = {
            "action": "queued",
            "job_id": "j",
            "task_id": "render_run_color_run_abc",
            "run_id": "run_abc",
            "mode": "color",
            "execution_arn": "arn:test",
            "started_at_ms": 1000,
        }
        result = handler(event, None)
        body = json.loads(result["body"])
        assert body["action"] == "queued"

        item = mock_get_ddb.return_value.put_item.call_args[1]["Item"]
        assert item["job_id"]["S"] == "j"
        assert item["task_id"]["S"] == "render_run_color_run_abc"
        assert item["task_status"]["S"] == "queued"
        rd = json.loads(item["result_data"]["S"])
        assert rd["phase"] == "queued"
        assert rd["phase_label"] == "Queued"
        assert rd["execution_arn"] == "arn:test"

    @patch("handler_render_status._get_ddb")
    def test_phase_update_with_subtask(self, mock_get_ddb):
        from handler_render_status import handler
        event = {
            "action": "phase",
            "job_id": "j",
            "task_id": "render_run_color_run_abc",
            "run_id": "run_abc",
            "mode": "color",
            "phase": "raster",
            "phase_label": "Raster",
            "expected": 10,
            "subtask_prefix": "render_run_abc_raster_",
        }
        handler(event, None)
        item = mock_get_ddb.return_value.put_item.call_args[1]["Item"]
        rd = json.loads(item["result_data"]["S"])
        assert rd["phase"] == "raster"
        assert rd["phase_label"] == "Raster"
        assert rd["expected"] == 10
        assert rd["subtask_prefix"] == "render_run_abc_raster_"

    @patch("handler_render_status._get_ddb")
    def test_done_update_with_image_key(self, mock_get_ddb):
        from handler_render_status import handler
        event = {
            "action": "done",
            "job_id": "j",
            "task_id": "render_run_color_run_abc",
            "run_id": "run_abc",
            "mode": "color",
            "image_key": "renders/j/image.jpeg",
        }
        handler(event, None)
        item = mock_get_ddb.return_value.put_item.call_args[1]["Item"]
        assert item["task_status"]["S"] == "done"
        rd = json.loads(item["result_data"]["S"])
        assert rd["phase"] == "done"
        assert rd["image_key"] == "renders/j/image.jpeg"

    @patch("handler_render_status._get_ddb")
    def test_error_with_direct_message(self, mock_get_ddb):
        from handler_render_status import handler
        event = {
            "action": "error",
            "job_id": "j",
            "task_id": "render_run_color_run_abc",
            "run_id": "run_abc",
            "mode": "color",
            "error_msg": "raster exploded: out of memory",
        }
        handler(event, None)
        item = mock_get_ddb.return_value.put_item.call_args[1]["Item"]
        assert item["task_status"]["S"] == "error"
        assert "raster exploded" in item["error_msg"]["S"]

    @patch("handler_render_status._get_ddb")
    def test_error_extracts_from_sfn_cause(self, mock_get_ddb):
        """Error action extracts readable message from Step Functions Error/Cause."""
        from handler_render_status import handler
        cause_json = json.dumps({
            "errorMessage": "solve_proximity_stats clip failed: invalid metric",
            "errorType": "RuntimeError",
        })
        event = {
            "action": "error",
            "job_id": "j",
            "task_id": "render_run_color_run_abc",
            "run_id": "run_abc",
            "mode": "color",
            "Error": "Lambda.Unknown",
            "Cause": cause_json,
        }
        handler(event, None)
        item = mock_get_ddb.return_value.put_item.call_args[1]["Item"]
        assert "solve_proximity_stats clip failed" in item["error_msg"]["S"]
        # Should NOT just say "Lambda.Unknown"
        assert item["error_msg"]["S"] != "Lambda.Unknown"

    def test_status_does_not_dispatch_or_poll(self):
        """Status Lambda must not contain dispatch/poll logic."""
        import handler_render_status as mod
        source = open(mod.__file__).read()
        assert "lambda_client" not in source
        assert "_dispatch" not in source
        assert "_poll" not in source
        assert "InvokeFunction" not in source


if __name__ == "__main__":
    unittest.main()
