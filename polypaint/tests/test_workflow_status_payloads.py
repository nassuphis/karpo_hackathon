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


class TestStopExecutionIamContract(unittest.TestCase):
    def test_stop_execution_is_granted_on_execution_arns(self):
        """CR35-F10: StartExecution authorizes against stateMachine ARNs,
        StopExecution against EXECUTION ARNs. The deploy policy must keep
        the two actions in separate statements with the right resource
        types — one merged statement ships a Kill that always gets
        AccessDenied while stubbed unit tests stay green."""
        deploy = open(os.path.join(ROOT, "deploy.sh")).read()
        start = deploy.index("grant_sfn_start_policy()")
        block = deploy[start:start + 2000]
        self.assertIn('states:StartExecution', block)
        self.assertIn('states:StopExecution', block)
        # StopExecution must NOT share a statement with the state-machine
        # resources: it must reference the :execution: rewritten ARNs
        self.assertIn(':stateMachine:/:execution:', block)
        self.assertIn('EXEC_ARN', block)
        stop_stmt = block[block.index("states:StopExecution"):]
        self.assertIn("EXEC_ARN", stop_stmt[:400])

    def test_solve_map_carries_planned_threads(self):
        """CR35-F12: the classic SolveMap payload forwards the planned
        solve thread count to handler_sweep_cm."""
        template = open(os.path.join(
            ROOT, "stepfunctions", "compute_workflow.asl.json.template")).read()
        self.assertIn('"n_threads.$": "$.plan.solve.threads"', template)
