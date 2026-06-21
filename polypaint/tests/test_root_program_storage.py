import json
import os
import sys
import unittest


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(path, body):
    return {"path": path, "body": json.dumps(body)}


class TestRootProgramStorage(unittest.TestCase):
    def test_compile_root_program_source_route(self):
        import handler_storage

        resp = handler_storage.handler(
            _event("/compile-root-program-source", {"source_text": "rotate_roots(0.25)\npull_unit_circle()"}),
            None,
        )
        self.assertEqual(resp["statusCode"], 200)
        body = json.loads(resp["body"])
        self.assertTrue(body["ok"])
        self.assertEqual(body["chain"][1], {"name": "pull_unit_circle", "fn_index": 2, "args": [0.75, 1.0]})
        self.assertEqual(body["root_transforms"][1], ["pull_unit_circle", "0.75", "1"])
        self.assertTrue(body["fingerprint"].startswith("sha256:"))
        self.assertEqual(body["program"]["program_kind"], "root_program")
        self.assertIn("execution_spec", body["program"])

    def test_compile_root_program_source_route_reports_diagnostics(self):
        import handler_storage

        resp = handler_storage.handler(
            _event("/compile-root-program-source", {"source_text": "roots = roots\nunknown_op(1)"}),
            None,
        )
        self.assertEqual(resp["statusCode"], 200)
        body = json.loads(resp["body"])
        self.assertFalse(body["ok"])
        self.assertEqual(body["chain"], [])
        self.assertEqual(body["program"]["chain"], [])
        self.assertGreaterEqual(len(body["diagnostics"]), 2)


if __name__ == "__main__":
    unittest.main()
