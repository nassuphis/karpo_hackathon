import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class TestComputeStatusHandler(unittest.TestCase):

    @patch("handler_compute_status._get_ddb")
    def test_error_preserves_last_reported_phase_context(self, mock_get_ddb):
        import handler_compute_status as mod

        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        mock_ddb.get_item.return_value = {
            "Item": {
                "result_data": {
                    "S": json.dumps({
                        "phase": "degree_probe",
                        "phase_label": "Degree probe",
                        "subtask_prefix": "compute_x/degree_probe",
                        "expected": 1,
                    })
                }
            }
        }

        result = mod.handler({
            "action": "error",
            "job_id": "job1",
            "task_id": "task1",
            "run_id": "run1",
            "mode": "compute",
            "solver_mode": "aberth_mt",
            "Error": "RuntimeError",
            "Cause": json.dumps({"errorMessage": "degree probe failed"}),
        }, None)

        self.assertEqual(result["statusCode"], 200)
        mock_ddb.get_item.assert_called_once()
        item = mock_ddb.put_item.call_args[1]["Item"]
        self.assertEqual(item["task_status"]["S"], "error")
        self.assertEqual(item["error_msg"]["S"], "degree probe failed")

        data = json.loads(item["result_data"]["S"])
        self.assertEqual(data["phase"], "error")
        self.assertEqual(data["failed_phase"], "degree_probe")
        self.assertEqual(data["failed_phase_label"], "Degree probe")
        self.assertEqual(data["failed_subtask_prefix"], "compute_x/degree_probe")
        self.assertEqual(data["failed_expected"], 1)
        self.assertEqual(data["failed_error"], "RuntimeError")

    @patch("handler_compute_status._get_ddb")
    def test_error_write_survives_missing_previous_phase_row(self, mock_get_ddb):
        import handler_compute_status as mod

        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        mock_ddb.get_item.return_value = {}

        result = mod.handler({
            "action": "error",
            "job_id": "job1",
            "task_id": "task1",
            "error_msg": "explicit failure",
        }, None)

        self.assertEqual(result["statusCode"], 200)
        item = mock_ddb.put_item.call_args[1]["Item"]
        self.assertEqual(item["error_msg"]["S"], "explicit failure")
        data = json.loads(item["result_data"]["S"])
        self.assertEqual(data["phase"], "error")
        self.assertNotIn("failed_phase", data)


if __name__ == "__main__":
    unittest.main()
