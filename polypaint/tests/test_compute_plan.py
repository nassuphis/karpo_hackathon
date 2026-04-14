"""
Tests for compute plan/finalize Lambda helpers.
"""
import json
import os
import sys
import unittest
from unittest.mock import patch

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
                "param_gen_threads": 6,
                "coeffgen_threads": 5,
                "lores_param_gen_threads": 2,
                "lores_coeffgen_threads": 3,
                "function": "g1",
                "param_transforms": [],
                "coeff_transforms": [],
                "cfpv": [],
            },
        })
        plan = json.loads(result["body"])
        self.assertEqual(plan["compute"]["n_chunks"], 4)
        self.assertEqual(plan["compute"]["param_gen_threads"], 6)
        self.assertEqual(plan["compute"]["coeffgen_threads"], 5)
        self.assertEqual(plan["compute"]["lores_param_gen_threads"], 2)
        self.assertEqual(plan["compute"]["lores_coeffgen_threads"], 3)
        self.assertEqual(plan["param_gen"]["threads"], 6)
        self.assertEqual(plan["coeffgen"]["threads"], 5)
        self.assertEqual(len(plan["chunk_items"]), 4)
        first = plan["chunk_items"][0]
        self.assertEqual(first["solve_task_id"], "compute_run_abc_solve_0")
        self.assertEqual(first["bin_key"], "renders/compute_j/chunk_0.bin")

    @patch("handler_compute_plan._get_ddb")
    def test_post_coeffgen_returns_compact_payload_without_sweep_items(self, mock_get_ddb):
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
            "coeffgen": {"task_prefix": "compute_run_abc_coeffgen_"},
        }
        mock_get_ddb.return_value.query.return_value = {
            "Items": [
                {
                    "task_id": {"S": f"compute_run_abc_coeffgen_{i}"},
                    "task_status": {"S": "done"},
                    "result_data": {"S": json.dumps({
                        "chunk_idx": i,
                        "coeffs_size": 1000,
                        "n_coeffs": 11,
                        "degree": 10,
                    })},
                }
                for i in range(4)
            ]
        }
        result = mod.handle_post_coeffgen({
            "job_id": "compute_j",
            "plan": plan,
            "task_prefix": "compute_run_abc_coeffgen_",
        })
        post = json.loads(result["body"])
        self.assertEqual(post["degree"], 10)
        self.assertEqual(post["n_coeffs"], 11)
        self.assertEqual(post["total_coeffs_size"], 4000)
        self.assertNotIn("sweep_items", post)
        self.assertIn("lores", post)
        kwargs = mock_get_ddb.return_value.query.call_args.kwargs
        self.assertTrue(kwargs["ConsistentRead"])
        self.assertEqual(kwargs["ExpressionAttributeValues"][":pfx"]["S"], "compute_run_abc_coeffgen_")

    @patch("handler_compute_plan._get_ddb")
    def test_post_coeffgen_payload_stays_small_for_500_chunks(self, mock_get_ddb):
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
            "coeffgen": {"task_prefix": "compute_run_big_coeffgen_"},
        }
        mock_get_ddb.return_value.query.return_value = {
            "Items": [
                {
                    "task_id": {"S": f"compute_run_big_coeffgen_{i}"},
                    "task_status": {"S": "done"},
                    "result_data": {"S": json.dumps({
                        "chunk_idx": i,
                        "coeffs_size": 4_400_000,
                        "n_coeffs": 11,
                        "degree": 10,
                    })},
                }
                for i in range(500)
            ]
        }
        result = mod.handle_post_coeffgen({
            "job_id": "compute_j",
            "plan": plan,
            "task_prefix": "compute_run_big_coeffgen_",
        })
        body = result["body"]
        self.assertLess(len(body.encode("utf-8")), 4096)

    @patch("handler_compute_plan.s3")
    def test_finalize_metadata_persists_lores_coeffs_key(self, mock_s3):
        import handler_compute_plan as mod

        plan = {
            "job_id": "compute_j",
            "run_id": "run_abc",
            "pipeline": {
                "function": "g1",
                "param_transforms": [],
                "param_transforms_display": [],
                "coeff_transforms": [],
                "cfpv": [],
            },
            "compute": {"N": 100, "times": 1, "n_chunks": 1, "n_steps": 10000, "params_key": "renders/compute_j/params.bin", "param_gen_threads": 5, "coeffgen_threads": 4, "lores_param_gen_threads": 2, "lores_coeffgen_threads": 3},
            "solve": {"mode": "aberth"},
            "chunk_items": [{"coeffs_key": "renders/compute_j/coeffs_0000.bin"}],
        }
        post = {
            "degree": 10,
            "n_coeffs": 11,
            "total_coeffs_size": 4000,
            "lores": {
                "N": 50,
                "n_steps": 2500,
                "bin_key": "renders/compute_j/lores.bin",
                "coeffs_key": "renders/compute_j/lores_coeffs.bin",
                "param_gen_threads": 2,
                "coeffgen_threads": 3,
            },
        }
        lores_solve = {"s3_key": "renders/compute_j/lores.bin", "bin_size": 1234}
        solve_results = [{"chunk_idx": 0, "s3_key": "renders/compute_j/chunk_0.bin", "bin_size": 5678, "compute_us": 111, "n_t": 10, "avg_iterations": 7.5}]

        mod.handle_finalize_metadata({
            "plan": plan,
            "post": post,
            "lores_solve": lores_solve,
            "solve_results": solve_results,
        })

        body = json.loads(mock_s3.put_object.call_args.kwargs["Body"])
        self.assertEqual(body["lores"]["bin_key"], "renders/compute_j/lores.bin")
        self.assertEqual(body["lores"]["coeffs_key"], "renders/compute_j/lores_coeffs.bin")
        self.assertEqual(body["lores"]["params_key"], "renders/compute_j/lores_params.bin")
        self.assertEqual(body["param_gen_threads"], 5)
        self.assertEqual(body["coeffgen_threads"], 4)
        self.assertEqual(body["lores_param_gen_threads"], 2)
        self.assertEqual(body["lores_coeffgen_threads"], 3)
        self.assertEqual(body["lores"]["param_gen_threads"], 2)
        self.assertEqual(body["lores"]["coeffgen_threads"], 3)


if __name__ == "__main__":
    unittest.main()
