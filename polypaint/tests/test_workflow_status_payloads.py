"""
Every Step Functions status-lambda Payload must carry the execution's
own ARN via the context object ("execution_arn.$": "$$.Execution.Id").
The kill button arms from rd.execution_arn on
ANY poll tick; before this pin, only the orchestrator's initial queued
row carried the ARN and the first phase write wiped it to "" — the
button existed for a ~2s window that a 3s poll almost never hit
(user-reported: no kill button on running jobs).
"""
import json
import os
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
TEMPLATES = (
    "stepfunctions/compute_workflow.asl.json.template",
    "stepfunctions/palette_workflow.asl.json.template",
    "stepfunctions/render_workflow.asl.json.template",
)


def _status_payloads(node, out):
    if isinstance(node, dict):
        params = node.get("Parameters")
        if (isinstance(params, dict)
                and params.get("FunctionName") == "${StatusFunctionArn}"
                and isinstance(params.get("Payload"), dict)):
            out.append(params["Payload"])
        for value in node.values():
            _status_payloads(value, out)
    elif isinstance(node, list):
        for value in node:
            _status_payloads(value, out)


class TestWorkflowStatusPayloads(unittest.TestCase):
    def test_every_status_payload_carries_the_execution_arn(self):
        for template in TEMPLATES:
            with self.subTest(template=template):
                doc = json.load(open(os.path.join(ROOT, template)))
                payloads = []
                _status_payloads(doc, payloads)
                self.assertGreater(len(payloads), 5)
                for payload in payloads:
                    self.assertEqual(
                        payload.get("execution_arn.$"), "$$.Execution.Id",
                        f"status payload missing execution_arn in {template}: "
                        f"{sorted(payload.keys())}")


if __name__ == "__main__":
    unittest.main()
