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
        assert rd["run_started_at_ms"] == 1000

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
            "run_started_at_ms": 1000,
            "expected": 10,
            "subtask_prefix": "render_run_abc_raster_",
        }
        handler(event, None)
        item = mock_get_ddb.return_value.put_item.call_args[1]["Item"]
        rd = json.loads(item["result_data"]["S"])
        assert rd["phase"] == "raster"
        assert rd["phase_label"] == "Raster"
        assert rd["run_started_at_ms"] == 1000
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
            "run_started_at_ms": 1000,
            "image_key": "renders/j/image.jpeg",
        }
        handler(event, None)
        item = mock_get_ddb.return_value.put_item.call_args[1]["Item"]
        assert item["task_status"]["S"] == "done"
        rd = json.loads(item["result_data"]["S"])
        assert rd["phase"] == "done"
        assert rd["run_started_at_ms"] == 1000
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

    @patch("handler_render_status._get_ddb")
    def test_all_rows_include_updated_at_ms(self, mock_get_ddb):
        """Every row written by the status Lambda includes top-level updated_at_ms."""
        from handler_render_status import handler
        import time
        now_ms = int(time.time() * 1000)
        for action, extra in [
            ("queued", {"run_id": "r", "mode": "color"}),
            ("phase", {"run_id": "r", "mode": "color", "phase": "raster", "phase_label": "Raster"}),
            ("done", {"run_id": "r", "mode": "color"}),
            ("error", {"run_id": "r", "mode": "color", "error_msg": "boom"}),
        ]:
            mock_get_ddb.return_value.put_item.reset_mock()
            event = {"action": action, "job_id": "j", "task_id": "t", **extra}
            handler(event, None)
            item = mock_get_ddb.return_value.put_item.call_args[1]["Item"]
            self.assertIn("updated_at_ms", item, f"{action} row missing updated_at_ms")
            ms = int(item["updated_at_ms"]["N"])
            self.assertAlmostEqual(ms, now_ms, delta=5000)

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
