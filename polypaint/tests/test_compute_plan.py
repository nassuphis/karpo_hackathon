"""
Tests for compute plan/finalize Lambda helpers.
"""
import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class TestComputePlan(unittest.TestCase):

    def test_build_plan_chunk_items_include_solve_fields(self):
        import handler_compute_plan as mod

        result = mod.handle_build_plan({
            "job_id": "compute_j",
            "run_id": "run_abc",
            "task_id": "compute_run_aberth_mt_run_abc",
            "params": {
                "solver_mode": "aberth_mt",
                "N": 100,
                "times": 1,
                "n_chunks": 4,
                "param_gen_threads": 6,
                "coeffgen_threads": 5,
                "lores_param_gen_threads": 2,
                "lores_coeffgen_threads": 3,
                "function": "g1",
                "param_transforms": [["unit_circle"]],
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
        self.assertEqual(plan["compute"]["param_storage_mode"], "chunked")
        self.assertEqual(plan["compute"]["params_key"], "")
        self.assertEqual(plan["param_gen"]["threads"], 6)
        self.assertEqual(plan["param_gen"]["storage_mode"], "chunked")
        self.assertEqual(plan["param_gen"]["task_prefix"], "compute_run_abc_param_gen_")
        self.assertEqual(plan["coeffgen"]["threads"], 5)
        self.assertEqual(len(plan["chunk_items"]), 4)
        first = plan["chunk_items"][0]
        self.assertEqual(first["paramgen_task_id"], "compute_run_abc_param_gen_0")
        self.assertEqual(first["params_key"], "renders/compute_j/params_0000.bin")
        self.assertEqual(first["params_step_start"], 0)
        self.assertEqual(first["params_step_count"], first["step_count"])
        self.assertEqual(first["params_bin_size"], first["step_count"] * 16)
        self.assertEqual(first["solve_task_id"], "compute_run_abc_solve_0")
        self.assertEqual(first["bin_key"], "renders/compute_j/chunk_0.bin")

    def test_build_plan_compiles_param_program_chain(self):
        import handler_compute_plan as mod

        result = mod.handle_build_plan({
            "job_id": "compute_j",
            "run_id": "run_param_program",
            "task_id": "compute_run_aberth_mt_run_param_program",
            "params": {
                "solver_mode": "aberth_mt",
                "N": 20,
                "times": 1,
                "n_chunks": 2,
                "function": "g1",
                "param_transforms": [["unit_circle"]],
                "param_program_chain": [
                    ["push", "t1"],
                    ["push", "t2"],
                    ["add"],
                    ["emit", "p1"],
                    ["push", "t1"],
                    ["push", "t2"],
                    ["subtract"],
                    ["emit", "p2"],
                ],
                "coeff_transforms": [],
                "cfpv": [],
            },
        })
        plan = json.loads(result["body"])
        self.assertEqual(plan["pipeline"]["param_transforms"], [])
        self.assertEqual(plan["pipeline"]["param_program_chain"][0], ["push", "t1"])
        self.assertEqual(plan["pipeline"]["param_program"]["token_count"], 8)
        self.assertEqual(plan["pipeline"]["param_program"]["stack_max"], 2)
        self.assertNotIn("scalar_exprs", plan["pipeline"]["param_program"])
        self.assertTrue(plan["pipeline"]["param_program_fingerprint"])
        self.assertEqual(plan["pipeline"]["param_program_display"], plan["pipeline"]["param_program"]["display"])

    def test_build_plan_keeps_legacy_equivalent_param_program_on_fast_path(self):
        import handler_compute_plan as mod

        result = mod.handle_build_plan({
            "job_id": "compute_j",
            "run_id": "run_param_program_legacy",
            "task_id": "compute_run_aberth_mt_run_param_program_legacy",
            "params": {
                "solver_mode": "aberth_mt",
                "N": 20,
                "times": 1,
                "n_chunks": 2,
                "function": "g1",
                "param_transforms": [],
                "param_program_chain": [["legacy", "rtheta", "both", "both", "1"]],
                "coeff_transforms": [],
                "cfpv": [],
            },
        })
        plan = json.loads(result["body"])
        self.assertEqual(plan["pipeline"]["param_transforms"], [["rtheta", "1"]])
        self.assertEqual(plan["pipeline"]["param_program"], {})
        self.assertTrue(plan["pipeline"]["param_program_fingerprint"])
        self.assertTrue(plan["pipeline"]["param_program_uses_legacy_fast_path"])

    def test_build_plan_compiles_coeff_program_chain(self):
        import handler_compute_plan as mod

        result = mod.handle_build_plan({
            "job_id": "compute_j",
            "run_id": "run_coeff_program",
            "task_id": "compute_run_aberth_mt_run_coeff_program",
            "params": {
                "pipeline_mode": "program",
                "solver_mode": "aberth_mt",
                "N": 20,
                "times": 1,
                "n_chunks": 2,
                "function": "const",
                "param_transforms": [["unit_circle"]],
                "param_program_chain": [],
                "coeff_transforms": [["rev"]],
                "coeff_program_chain": [["const", "35", "p1+p2"], ["emit"]],
                "cfpv": [35, 1, 0],
            },
        })
        plan = json.loads(result["body"])
        self.assertEqual(plan["pipeline"]["pipeline_mode"], "program")
        self.assertEqual(plan["pipeline"]["param_transforms"], [])
        self.assertEqual(plan["pipeline"]["coeff_transforms"], [])
        self.assertEqual(plan["pipeline"]["coeff_program_chain"][0], ["const", "35", "p1+p2"])
        self.assertEqual(plan["pipeline"]["coeff_program"]["token_count"], 2)
        self.assertEqual(plan["pipeline"]["coeff_program"]["scalar_expr_count"], 1)
        self.assertTrue(plan["pipeline"]["coeff_program_fingerprint"])

    def test_build_plan_resolves_coeff_program_macro(self):
        import handler_compute_plan as mod

        saved = json.dumps({"chain": [["legacy", "rev", "poly", "poly"]]}).encode()
        fake_s3 = MagicMock()
        fake_s3.get_object.return_value = {
            "Body": MagicMock(read=lambda: saved)
        }
        with patch.object(mod, "s3", fake_s3):
            result = mod.handle_build_plan({
                "job_id": "compute_j",
                "run_id": "run_coeff_program_macro",
                "task_id": "compute_run_aberth_mt_run_coeff_program_macro",
                "params": {
                    "pipeline_mode": "program",
                    "solver_mode": "aberth_mt",
                    "N": 20,
                    "times": 1,
                    "n_chunks": 2,
                    "function": "const",
                    "param_transforms": [],
                    "param_program_chain": [],
                    "coeff_transforms": [],
                    "coeff_program_chain": [["macro", "poly-test1"]],
                    "cfpv": [3, 1, 0],
                },
            })
        plan = json.loads(result["body"])
        self.assertEqual(plan["pipeline"]["coeff_transforms"], [["rev"]])
        self.assertEqual(plan["pipeline"]["coeff_program"], {})
        fake_s3.get_object.assert_called_once_with(
            Bucket=mod.BUCKET,
            Key="polypaint/coeff-programs/poly-test1.json",
        )

    def test_probe_signature_includes_program_fingerprints(self):
        import compute_fused as mod

        base = mod.build_probe_signature(
            function_name="g1",
            param_transforms=[],
            coeff_transforms=[],
            cfpv=[],
        )
        with_param_program = mod.build_probe_signature(
            function_name="g1",
            param_transforms=[],
            coeff_transforms=[],
            cfpv=[],
            param_program={"fingerprint": "param-a"},
        )
        with_coeff_program = mod.build_probe_signature(
            function_name="g1",
            param_transforms=[],
            coeff_transforms=[],
            cfpv=[],
            coeff_program={"fingerprint": "coeff-a"},
        )
        self.assertNotEqual(base, with_param_program)
        self.assertNotEqual(base, with_coeff_program)

    def test_build_plan_fused_uses_probe_and_safe_chunk_floor(self):
        import handler_compute_plan as mod

        result = mod.handle_build_plan({
            "job_id": "compute_j",
            "run_id": "run_fused",
            "task_id": "compute_run_aberth_mt_run_fused",
            "probe": {
                "probe_stable": True,
                "degree": 10,
                "n_coeffs": 11,
                "probe_signature": mod.build_probe_signature(
                    function_name="g1",
                    param_transforms=[],
                    coeff_transforms=[],
                    cfpv=[],
                ),
            },
            "params": {
                "execution_method": "fused_chunk_pipeline",
                "solver_mode": "aberth_mt",
                "N": 100,
                "times": 1,
                "n_chunks": 4,
                "auto_hires_chunks": True,
                "fused_threads": 4,
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
        self.assertEqual(plan["compute"]["execution_method"], "fused_chunk_pipeline")
        self.assertEqual(plan["compute"]["probe_degree"], 10)
        self.assertEqual(plan["compute"]["probe_n_coeffs"], 11)
        self.assertEqual(plan["compute"]["param_gen_threads"], 4)
        self.assertEqual(plan["compute"]["coeffgen_threads"], 4)
        self.assertEqual(plan["fused"]["threads"], 4)
        self.assertGreaterEqual(plan["compute"]["n_chunks"], plan["compute"]["min_safe_chunks"])
        self.assertEqual(plan["post_seed"]["degree"], 10)
        self.assertEqual(plan["post_seed"]["n_coeffs"], 11)
        self.assertIn("fused_task_id", plan["chunk_items"][0])

    def test_build_plan_fused_rejects_unsafe_manual_chunk_count(self):
        import handler_compute_plan as mod

        with self.assertRaises(RuntimeError):
            mod.handle_build_plan({
                "job_id": "compute_j",
                "run_id": "run_fused",
                "task_id": "compute_run_aberth_mt_run_fused",
                "probe": {
                    "probe_stable": True,
                    "degree": 70,
                    "n_coeffs": 71,
                    "probe_signature": mod.build_probe_signature(
                        function_name="g1",
                        param_transforms=[],
                        coeff_transforms=[],
                        cfpv=[],
                    ),
                },
                "params": {
                    "execution_method": "fused_chunk_pipeline",
                    "solver_mode": "aberth_mt",
                    "N": 5000,
                    "times": 2,
                    "n_chunks": 1,
                    "auto_hires_chunks": False,
                    "fused_threads": 8,
                    "function": "g1",
                    "param_transforms": [],
                    "coeff_transforms": [],
                    "cfpv": [],
                },
            })

    def test_build_plan_allows_fused_for_companion_matrix(self):
        import handler_compute_plan as mod

        result = mod.handle_build_plan({
            "job_id": "compute_j",
            "run_id": "run_fused_cm",
            "task_id": "compute_run_companion_matrix_run_fused_cm",
            "probe": {
                "probe_stable": True,
                "degree": 10,
                "n_coeffs": 11,
                "probe_signature": mod.build_probe_signature(
                    function_name="g1",
                    param_transforms=[],
                    coeff_transforms=[],
                    cfpv=[],
                ),
            },
            "params": {
                "execution_method": "fused_chunk_pipeline",
                "solver_mode": "companion_matrix",
                "N": 100,
                "times": 1,
                "n_chunks": 4,
                "fused_threads": 4,
                "function": "g1",
                "param_transforms": [],
                "coeff_transforms": [],
                "cfpv": [],
            },
        })
        plan = json.loads(result["body"])
        self.assertEqual(plan["compute"]["execution_method"], "fused_chunk_pipeline")
        self.assertEqual(plan["solve"]["mode"], "companion_matrix")
        self.assertEqual(plan["compute"]["param_gen_threads"], 4)
        self.assertEqual(plan["compute"]["coeffgen_threads"], 4)
        self.assertEqual(plan["fused"]["threads"], 4)

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
            "compute": {"N": 100, "times": 1, "n_chunks": 1, "n_steps": 10000, "param_storage_mode": "chunked", "params_key": "", "param_gen_threads": 5, "coeffgen_threads": 4, "lores_param_gen_threads": 2, "lores_coeffgen_threads": 3},
            "solve": {"mode": "aberth_mt"},
            "chunk_items": [{
                "chunk_idx": 0,
                "coeffs_key": "renders/compute_j/coeffs_0000.bin",
                "params_key": "renders/compute_j/params_0000.bin",
                "params_bin_size": 160,
                "params_step_start": 0,
                "params_step_count": 10,
            }],
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
        self.assertEqual(body["param_storage_mode"], "chunked")
        self.assertEqual(body["params_key"], "")
        self.assertEqual(body["chunks"][0]["params_key"], "renders/compute_j/params_0000.bin")
        self.assertEqual(body["chunks"][0]["params_step_start"], 0)
        self.assertEqual(body["chunks"][0]["params_step_count"], 10)

    @patch("handler_compute_plan.s3")
    def test_finalize_metadata_preserves_chunk_local_params_for_multi_chunk_fused(self, mock_s3):
        import handler_compute_plan as mod

        plan = {
            "job_id": "compute_j",
            "run_id": "run_fused",
            "pipeline": {
                "function": "g1",
                "param_transforms": [],
                "param_transforms_display": [],
                "coeff_transforms": [],
                "cfpv": [],
            },
            "compute": {
                "N": 100,
                "times": 1,
                "n_chunks": 2,
                "n_steps": 20,
                "execution_method": "fused_chunk_pipeline",
                "param_storage_mode": "chunked",
                "params_key": "",
                "param_gen_threads": 5,
                "coeffgen_threads": 4,
                "lores_param_gen_threads": 2,
                "lores_coeffgen_threads": 3,
                "fused_threads": 6,
                "auto_hires_chunks": True,
                "probe_degree": 10,
                "probe_n_coeffs": 11,
                "probe_signature": "abc123",
                "min_safe_chunks": 2,
                "safe_chunk_limit_reason": "memory",
            },
            "solve": {"mode": "aberth_mt"},
            "chunk_items": [
                {
                    "chunk_idx": 0,
                    "step_start": 0,
                    "step_count": 10,
                    "coeffs_key": "renders/compute_j/coeffs_0000.bin",
                    "params_key": "renders/compute_j/params_0000.bin",
                    "params_bin_size": 160,
                    "params_step_start": 0,
                    "params_step_count": 10,
                },
                {
                    "chunk_idx": 1,
                    "step_start": 10,
                    "step_count": 10,
                    "coeffs_key": "renders/compute_j/coeffs_0001.bin",
                    "params_key": "renders/compute_j/params_0001.bin",
                    "params_bin_size": 160,
                    "params_step_start": 0,
                    "params_step_count": 10,
                },
            ],
        }
        post = {
            "degree": 10,
            "n_coeffs": 11,
            "total_coeffs_size": 1760,
            "lores": {
                "N": 50,
                "n_steps": 2500,
                "bin_key": "renders/compute_j/lores.bin",
                "coeffs_key": "renders/compute_j/lores_coeffs.bin",
                "params_key": "renders/compute_j/lores_params.bin",
                "param_gen_threads": 2,
                "coeffgen_threads": 3,
            },
        }
        lores_solve = {"s3_key": "renders/compute_j/lores.bin", "bin_size": 1234}
        solve_results = [
            {
                "chunk_idx": 1,
                "s3_key": "renders/compute_j/chunk_1.bin",
                "bin_size": 800,
                "compute_us": 222,
                "n_t": 10,
                "avg_iterations": 7.5,
                "params_step_start": 999,
                "params_step_count": 999,
                "coeffs_size": 880,
                "params_size": 160,
                "fused_threads": 6,
            },
            {
                "chunk_idx": 0,
                "s3_key": "renders/compute_j/chunk_0.bin",
                "bin_size": 800,
                "compute_us": 111,
                "n_t": 10,
                "avg_iterations": 6.5,
                "params_step_start": 999,
                "params_step_count": 999,
                "coeffs_size": 880,
                "params_size": 160,
                "fused_threads": 6,
            },
        ]

        mod.handle_finalize_metadata({
            "plan": plan,
            "post": post,
            "lores_solve": lores_solve,
            "solve_results": solve_results,
        })

        body = json.loads(mock_s3.put_object.call_args.kwargs["Body"])
        self.assertEqual(body["execution_method"], "fused_chunk_pipeline")
        self.assertEqual(body["fused_threads"], 6)
        self.assertEqual([chunk["idx"] for chunk in body["chunks"]], [0, 1])
        self.assertEqual(body["chunks"][0]["step_start"], 0)
        self.assertEqual(body["chunks"][1]["step_start"], 10)
        self.assertEqual(body["chunks"][0]["params_key"], "renders/compute_j/params_0000.bin")
        self.assertEqual(body["chunks"][1]["params_key"], "renders/compute_j/params_0001.bin")
        self.assertEqual(body["chunks"][0]["params_step_start"], 0)
        self.assertEqual(body["chunks"][1]["params_step_start"], 0)
        self.assertEqual(body["chunks"][0]["params_step_count"], 10)
        self.assertEqual(body["chunks"][1]["params_step_count"], 10)

    @patch("handler_compute_plan._get_ddb")
    @patch("handler_compute_plan.s3")
    def test_finalize_metadata_can_load_solve_results_from_ddb(self, mock_s3, mock_get_ddb):
        import handler_compute_plan as mod

        plan = {
            "job_id": "compute_j",
            "run_id": "run_fused",
            "pipeline": {
                "function": "g1",
                "param_transforms": [],
                "param_transforms_display": [],
                "coeff_transforms": [],
                "cfpv": [],
            },
            "compute": {
                "N": 100,
                "times": 1,
                "n_chunks": 2,
                "n_steps": 20,
                "execution_method": "fused_chunk_pipeline",
                "param_storage_mode": "chunked",
                "params_key": "",
                "param_gen_threads": 5,
                "coeffgen_threads": 4,
                "lores_param_gen_threads": 2,
                "lores_coeffgen_threads": 3,
                "fused_threads": 6,
            },
            "solve": {"mode": "aberth_mt", "task_prefix": "compute_run_fused_solve_"},
            "fused": {"task_prefix": "compute_run_fused_fused_"},
            "chunk_items": [
                {
                    "chunk_idx": 0,
                    "step_start": 0,
                    "step_count": 10,
                    "coeffs_key": "renders/compute_j/coeffs_0000.bin",
                    "params_key": "renders/compute_j/params_0000.bin",
                    "params_bin_size": 160,
                    "params_step_start": 0,
                    "params_step_count": 10,
                },
                {
                    "chunk_idx": 1,
                    "step_start": 10,
                    "step_count": 10,
                    "coeffs_key": "renders/compute_j/coeffs_0001.bin",
                    "params_key": "renders/compute_j/params_0001.bin",
                    "params_bin_size": 160,
                    "params_step_start": 0,
                    "params_step_count": 10,
                },
            ],
        }
        post = {
            "degree": 10,
            "n_coeffs": 11,
            "total_coeffs_size": 1760,
            "lores": {
                "N": 50,
                "n_steps": 2500,
                "bin_key": "renders/compute_j/lores.bin",
                "coeffs_key": "renders/compute_j/lores_coeffs.bin",
                "params_key": "renders/compute_j/lores_params.bin",
                "param_gen_threads": 2,
                "coeffgen_threads": 3,
            },
        }
        lores_solve = {"s3_key": "renders/compute_j/lores.bin", "bin_size": 1234}
        mock_get_ddb.return_value.query.return_value = {
            "Items": [
                {
                    "task_id": {"S": "compute_run_fused_fused_1"},
                    "task_status": {"S": "done"},
                    "result_data": {"S": json.dumps({
                        "chunk_idx": 1,
                        "s3_key": "renders/compute_j/chunk_1.bin",
                        "bin_size": 800,
                        "compute_us": 222,
                        "n_t": 10,
                        "avg_iterations": 7.5,
                        "coeffs_size": 880,
                        "params_size": 160,
                        "fused_threads": 6,
                    })},
                },
                {
                    "task_id": {"S": "compute_run_fused_fused_0"},
                    "task_status": {"S": "done"},
                    "result_data": {"S": json.dumps({
                        "chunk_idx": 0,
                        "s3_key": "renders/compute_j/chunk_0.bin",
                        "bin_size": 800,
                        "compute_us": 111,
                        "n_t": 10,
                        "avg_iterations": 6.5,
                        "coeffs_size": 880,
                        "params_size": 160,
                        "fused_threads": 6,
                    })},
                },
            ]
        }

        mod.handle_finalize_metadata({
            "job_id": "compute_j",
            "plan": plan,
            "post": post,
            "lores_solve": lores_solve,
            "expected": 2,
        })

        body = json.loads(mock_s3.put_object.call_args.kwargs["Body"])
        self.assertEqual([chunk["idx"] for chunk in body["chunks"]], [0, 1])
        self.assertEqual(body["chunks"][0]["bin_key"], "renders/compute_j/chunk_0.bin")
        self.assertEqual(body["chunks"][1]["bin_key"], "renders/compute_j/chunk_1.bin")
        self.assertEqual(body["total_bin_size"], 1600)
        query_kwargs = mock_get_ddb.return_value.query.call_args.kwargs
        self.assertEqual(query_kwargs["ExpressionAttributeValues"][":pfx"]["S"], "compute_run_fused_fused_")

    @patch("handler_compute_plan.s3")
    def test_finalize_metadata_classic_and_fused_share_render_facing_contract(self, mock_s3):
        import handler_compute_plan as mod

        def _base_plan(execution_method):
            compute = {
                "N": 100,
                "times": 1,
                "n_chunks": 2,
                "n_steps": 20,
                "execution_method": execution_method,
                "param_storage_mode": "chunked",
                "params_key": "",
                "param_gen_threads": 5,
                "coeffgen_threads": 4,
                "lores_param_gen_threads": 2,
                "lores_coeffgen_threads": 3,
            }
            if execution_method == "fused_chunk_pipeline":
                compute.update({
                    "fused_threads": 6,
                    "auto_hires_chunks": True,
                    "probe_degree": 10,
                    "probe_n_coeffs": 11,
                    "probe_signature": "abc123",
                    "min_safe_chunks": 2,
                    "safe_chunk_limit_reason": "memory",
                })
            return {
                "job_id": "compute_j",
                "run_id": f"run_{execution_method}",
                "pipeline": {
                    "function": "g1",
                    "param_transforms": [],
                    "param_transforms_display": [],
                    "coeff_transforms": [],
                    "cfpv": [],
                },
                "compute": compute,
                "solve": {"mode": "aberth_mt"},
                "chunk_items": [
                    {
                        "chunk_idx": 0,
                        "step_start": 0,
                        "step_count": 10,
                        "coeffs_key": "renders/compute_j/coeffs_0000.bin",
                        "params_key": "renders/compute_j/params_0000.bin",
                        "params_bin_size": 160,
                        "params_step_start": 0,
                        "params_step_count": 10,
                    },
                    {
                        "chunk_idx": 1,
                        "step_start": 10,
                        "step_count": 10,
                        "coeffs_key": "renders/compute_j/coeffs_0001.bin",
                        "params_key": "renders/compute_j/params_0001.bin",
                        "params_bin_size": 160,
                        "params_step_start": 0,
                        "params_step_count": 10,
                    },
                ],
            }

        post = {
            "degree": 10,
            "n_coeffs": 11,
            "total_coeffs_size": 1760,
            "lores": {
                "N": 50,
                "n_steps": 2500,
                "bin_key": "renders/compute_j/lores.bin",
                "coeffs_key": "renders/compute_j/lores_coeffs.bin",
                "params_key": "renders/compute_j/lores_params.bin",
                "param_gen_threads": 2,
                "coeffgen_threads": 3,
            },
        }
        lores_solve = {"s3_key": "renders/compute_j/lores.bin", "bin_size": 1234}
        solve_results = [
            {"chunk_idx": 0, "s3_key": "renders/compute_j/chunk_0.bin", "bin_size": 800, "compute_us": 111, "n_t": 10, "avg_iterations": 6.5},
            {"chunk_idx": 1, "s3_key": "renders/compute_j/chunk_1.bin", "bin_size": 800, "compute_us": 222, "n_t": 10, "avg_iterations": 7.5},
        ]

        rendered = {}
        for execution_method in ("classic_chunk_pipeline", "fused_chunk_pipeline"):
            mod.handle_finalize_metadata({
                "plan": _base_plan(execution_method),
                "post": post,
                "lores_solve": lores_solve,
                "solve_results": solve_results,
            })
            rendered[execution_method] = json.loads(mock_s3.put_object.call_args.kwargs["Body"])

        def _render_subset(calc):
            return {
                "function": calc["function"],
                "N": calc["N"],
                "times": calc["times"],
                "degree": calc["degree"],
                "n_coeffs": calc["n_coeffs"],
                "solver": calc["solver"],
                "param_storage_mode": calc["param_storage_mode"],
                "params_key": calc["params_key"],
                "coeffs_keys": calc["coeffs_keys"],
                "lores": calc["lores"],
                "chunks": [
                    {
                        "idx": chunk["idx"],
                        "bin_key": chunk["bin_key"],
                        "step_start": chunk["step_start"],
                        "step_count": chunk["step_count"],
                        "params_key": chunk["params_key"],
                        "params_bin_size": chunk["params_bin_size"],
                        "params_step_start": chunk["params_step_start"],
                        "params_step_count": chunk["params_step_count"],
                    }
                    for chunk in calc["chunks"]
                ],
            }

        self.assertEqual(
            _render_subset(rendered["classic_chunk_pipeline"]),
            _render_subset(rendered["fused_chunk_pipeline"]),
        )


if __name__ == "__main__":
    unittest.main()
