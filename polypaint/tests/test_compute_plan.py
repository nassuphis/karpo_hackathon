"""
Tests for compute plan/finalize Lambda helpers.
"""
import json
import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class TestComputePlan(unittest.TestCase):

    def test_build_plan_chunk_items_include_solve_fields(self):
        import handler_compute_plan as mod

        result = mod.handle_build_plan({
            "job_id": "compute_j",
            "run_id": "run_abc",
            "task_id": "compute_run_aberth_run_abc",
            "params": {
                "solver_mode": "aberth",
                "N": 100,
                "times": 1,
                "n_chunks": 4,
                "function": "g1",
                "param_transforms": [],
                "coeff_transforms": [],
                "cfpv": [],
            },
        })
        plan = json.loads(result["body"])
        self.assertEqual(plan["compute"]["n_chunks"], 4)
        self.assertEqual(len(plan["chunk_items"]), 4)
        first = plan["chunk_items"][0]
        self.assertEqual(first["solve_task_id"], "compute_run_abc_solve_0")
        self.assertEqual(first["bin_key"], "renders/compute_j/chunk_0.bin")

    def test_post_coeffgen_returns_compact_payload_without_sweep_items(self):
        import handler_compute_plan as mod

        plan = {
            "job_id": "compute_j",
            "run_id": "run_abc",
            "chunk_items": [
                {
                    "chunk_idx": i,
                    "step_start": i * 25,
                    "step_count": 25,
                    "coeffs_key": f"renders/compute_j/coeffs_{i:04d}.bin",
                    "coeffgen_task_id": f"compute_run_abc_coeffgen_{i}",
                    "solve_task_id": f"compute_run_abc_solve_{i}",
                    "bin_key": f"renders/compute_j/chunk_{i}.bin",
                }
                for i in range(4)
            ],
            "compute": {"N": 100, "times": 1},
        }
        coeffgen_results = [
            {"chunk_idx": i, "coeffs_size": 1000, "n_coeffs": 11, "degree": 10}
            for i in range(4)
        ]
        result = mod.handle_post_coeffgen({
            "plan": plan,
            "coeffgen_results": coeffgen_results,
        })
        post = json.loads(result["body"])
        self.assertEqual(post["degree"], 10)
        self.assertEqual(post["n_coeffs"], 11)
        self.assertEqual(post["total_coeffs_size"], 4000)
        self.assertNotIn("sweep_items", post)
        self.assertIn("lores", post)

    def test_post_coeffgen_payload_stays_small_for_500_chunks(self):
        import handler_compute_plan as mod

        plan = {
            "job_id": "compute_j",
            "run_id": "run_big",
            "chunk_items": [
                {
                    "chunk_idx": i,
                    "step_start": i * 50000,
                    "step_count": 50000,
                    "coeffs_key": f"renders/compute_j/coeffs_{i:04d}.bin",
                    "coeffgen_task_id": f"compute_run_big_coeffgen_{i}",
                    "solve_task_id": f"compute_run_big_solve_{i}",
                    "bin_key": f"renders/compute_j/chunk_{i}.bin",
                }
                for i in range(500)
            ],
            "compute": {"N": 5000, "times": 1},
        }
        coeffgen_results = [
            {"chunk_idx": i, "coeffs_size": 4_400_000, "n_coeffs": 11, "degree": 10}
            for i in range(500)
        ]
        result = mod.handle_post_coeffgen({
            "plan": plan,
            "coeffgen_results": coeffgen_results,
        })
        body = result["body"]
        self.assertLess(len(body.encode("utf-8")), 4096)


if __name__ == "__main__":
    unittest.main()
