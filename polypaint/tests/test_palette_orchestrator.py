"""
Tests for the palette orchestrator starter Lambda.

Validates both normal palette runs and ExtractPalette launches.
"""
import json
import os
import sys
import unittest
from unittest.mock import patch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class TestPaletteOrchestrator(unittest.TestCase):

    @patch("handler_palette_orchestrator.report_status")
    @patch("handler_palette_orchestrator.sfn_client")
    def test_extract_palette_forwards_artifact_id_and_mode(self, mock_sfn, mock_report):
        mock_sfn.start_execution.return_value = {
            "executionArn": "arn:aws:states:us-east-1:123:execution:test:palette_extract_run_abc"
        }
        import handler_palette_orchestrator as mod
        mod.STATE_MACHINE_ARN = "arn:aws:states:us-east-1:123:stateMachine:test"

        result = mod.handler({
            "job_id": "j",
            "run_id": "run_abc",
            "task_id": "extract_palette_run_run_abc",
            "artifact_id": "color_src",
        }, None)
        body = json.loads(result["body"])

        self.assertEqual(body["run_id"], "run_abc")
        self.assertEqual(body["task_id"], "extract_palette_run_run_abc")
        call_kwargs = mock_sfn.start_execution.call_args[1]
        sfn_input = json.loads(call_kwargs["input"])
        self.assertEqual(sfn_input["mode"], "extract_palette")
        self.assertEqual(sfn_input["artifact_id"], "color_src")

        rd = mock_report.call_args.kwargs["result_data"]
        self.assertEqual(rd["mode"], "extract_palette")
        self.assertEqual(rd["artifact_id"], "color_src")


if __name__ == "__main__":
    unittest.main()
