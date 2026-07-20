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

    def test_vector_constants_use_normal_solver_contract(self):
        import handler_compute_plan as mod

        base = {
            "job_id": "compute_j",
            "run_id": "run_vector_constants",
            "task_id": "compute_run_companion_matrix_run_vector_constants",
            "params": {
                "pipeline_mode": "program",
                "N": 20,
                "times": 1,
                "n_chunks": 2,
                "function": "const",
                "param_transforms": [],
                "coeff_transforms": [],
                "coeff_program_source_text": (
                    "poly = translate_roots(vector_literal(1, -3, 2), 0.5)\n"
                    "emit"
                ),
                "cfpv": [3, 0, 0],
            },
        }
        for solver_mode in ("aberth_mt", "companion_matrix"):
            with self.subTest(solver_mode=solver_mode):
                request = json.loads(json.dumps(base))
                request["params"]["solver_mode"] = solver_mode
                plan = json.loads(mod.handle_build_plan(request)["body"])
                self.assertEqual(plan["solve"]["mode"], solver_mode)
                self.assertEqual(
                    plan["pipeline"]["coeff_program"]["vector_constant_count"],
                    1,
                )
                self.assertNotIn("direct_coeff_solve", plan["pipeline"])
                self.assertNotIn("direct_coeff_solve", plan["compute"])

    def test_solver_brush_modes_and_iters_in_plan(self):
        """Solver-brush wave: all four solver modes build plans; the solve
        dict carries bin_mode (native mode string for the lores lambda and
        fused chunks), iters (0 = default), and routes JT/Newton to the
        sweep_cm lambda. Bad modes and out-of-range iters are rejected."""
        import handler_compute_plan as mod

        base = {
            "job_id": "compute_j",
            "run_id": "run_brush",
            "task_id": "compute_run_x_run_brush",
            "params": {
                "N": 20,
                "times": 1,
                "n_chunks": 2,
                "function": "g1",
                "param_transforms": [],
                "coeff_transforms": [],
                "cfpv": [],
            },
        }
        expected = {
            "aberth_mt": ("solve_mt", mod.SWEEP_MT_FUNCTION),
            "companion_matrix": ("solve_cm", mod.SWEEP_CM_FUNCTION),
            "jenkins_traub": ("solve_jt", mod.SWEEP_CM_FUNCTION),
            "newton": ("solve_newton", mod.SWEEP_CM_FUNCTION),
            # fused JT64/CM64: no separate solve invocation — the solve
            # runs inside coeffgen on the SWEEP_MT (fused chunk) lambda
            "jt64": ("fused_jt64", mod.SWEEP_MT_FUNCTION),
            "cm64": ("fused_cm64", mod.SWEEP_MT_FUNCTION),
            "ae64": ("fused_ae64", mod.SWEEP_MT_FUNCTION),
        }
        for solver_mode, (bin_mode, fn_name) in expected.items():
            with self.subTest(solver_mode=solver_mode):
                request = json.loads(json.dumps(base))
                request["params"]["solver_mode"] = solver_mode
                if solver_mode in ("jt64", "cm64", "ae64"):
                    # CR35-F9: fused solvers are only constructible on the
                    # fused pipeline (the classic combination is rejected —
                    # pinned below); the fused planner needs the degree probe
                    request["params"]["execution_method"] = "fused_chunk_pipeline"
                    request["probe"] = {
                        "probe_stable": True,
                "probe_signature_spec_version": mod.PROBE_SIGNATURE_SPEC_VERSION,
                        "degree": 10,
                        "n_coeffs": 11,
                        "probe_signature": mod.build_probe_signature(
                            function_name="g1",
                            param_transforms=[],
                            coeff_transforms=[],
                            cfpv=[],
                        ),
                    }
                plan = json.loads(mod.handle_build_plan(request)["body"])
                self.assertEqual(plan["solve"]["mode"], solver_mode)
                self.assertEqual(plan["solve"]["bin_mode"], bin_mode)
                self.assertEqual(plan["solve"]["iters"], 0)
                self.assertEqual(plan["solve"]["function_name"], fn_name)

        # CR35-F9: 64-bit solver + classic topology is unconstructible
        for solver_mode in ("jt64", "cm64", "ae64"):
            request = json.loads(json.dumps(base))
            request["params"]["solver_mode"] = solver_mode
            with self.assertRaises(RuntimeError) as ctx:
                mod.handle_build_plan(request)
            self.assertIn("fused_chunk_pipeline", str(ctx.exception))

        # CR35-F23: Newton above its native 50 ceiling is rejected, not
        # silently rewritten by the solver
        request = json.loads(json.dumps(base))
        request["params"]["solver_mode"] = "newton"
        request["params"]["solver_iters"] = 51
        with self.assertRaises(RuntimeError) as ctx:
            mod.handle_build_plan(request)
        self.assertIn("<= 50", str(ctx.exception))

        request = json.loads(json.dumps(base))
        request["params"]["solver_mode"] = "aberth_mt"
        request["params"]["solver_iters"] = 7
        plan = json.loads(mod.handle_build_plan(request)["body"])
        self.assertEqual(plan["solve"]["iters"], 7)

        request = json.loads(json.dumps(base))
        request["params"]["solver_mode"] = "durand_kerner"
        with self.assertRaises(RuntimeError):
            mod.handle_build_plan(request)

        for bad_iters in (-1, 65, "lots"):
            request = json.loads(json.dumps(base))
            request["params"]["solver_iters"] = bad_iters
            with self.assertRaises(RuntimeError):
                mod.handle_build_plan(request)

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

    def test_build_plan_rejects_stepfunctions_state_overflow(self):
        import handler_compute_plan as mod

        with patch.object(mod, "MAX_PLAN_BYTES", 1500):
            with self.assertRaisesRegex(RuntimeError, "Compute plan too large"):
                mod.handle_build_plan({
                    "job_id": "compute_j",
                    "run_id": "run_big_plan",
                    "task_id": "compute_run_aberth_mt_run_big_plan",
                    "params": {
                        "solver_mode": "aberth_mt",
                        "N": 100,
                        "times": 1,
                        "n_chunks": 8,
                        "function": "g1",
                        "param_transforms": [],
                        "coeff_transforms": [],
                        "cfpv": [],
                    },
                })

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

    def test_build_plan_compiles_param_program_source_text(self):
        import handler_compute_plan as mod

        result = mod.handle_build_plan({
            "job_id": "compute_j",
            "run_id": "run_param_program_source",
            "task_id": "compute_run_aberth_mt_run_param_program_source",
            "params": {
                "pipeline_mode": "program",
                "solver_mode": "aberth_mt",
                "N": 20,
                "times": 1,
                "n_chunks": 2,
                "function": "g1",
                "param_transforms": [["unit_circle"]],
                "param_program_chain": [["push", "t1"]],
                "param_program_source_text": "p1 = t1 + t2\np2 = t1 - t2",
                "coeff_transforms": [],
                "cfpv": [],
            },
        })
        plan = json.loads(result["body"])
        self.assertEqual(plan["pipeline"]["param_transforms"], [])
        self.assertEqual(plan["pipeline"]["param_program_chain"][0], ["const", "t1+t2"])
        self.assertEqual(plan["pipeline"]["param_program_source_text"], "p1 = t1 + t2\np2 = t1 - t2")
        self.assertTrue(plan["pipeline"]["param_program_fingerprint"])
        self.assertNotEqual(plan["pipeline"]["param_program_chain"], [["push", "t1"]])

    def test_build_plan_blank_source_defaults_do_not_shadow_other_program(self):
        import handler_compute_plan as mod

        coeff_only = mod.handle_build_plan({
            "job_id": "compute_j",
            "run_id": "run_coeff_only",
            "task_id": "compute_run_aberth_mt_run_coeff_only",
            "params": {
                "pipeline_mode": "program",
                "solver_mode": "aberth_mt",
                "N": 20,
                "times": 1,
                "n_chunks": 2,
                "function": "const",
                "param_transforms": [],
                "param_program_chain": [],
                "param_program_source_text": "",
                "coeff_transforms": [],
                "coeff_program_chain": [["push", "cf"], ["emit"]],
                "coeff_program_source_text": "",
                "cfpv": [8, 1, 0],
            },
        })
        coeff_plan = json.loads(coeff_only["body"])
        self.assertEqual(coeff_plan["pipeline"]["param_program"], {})
        self.assertTrue(coeff_plan["pipeline"]["coeff_program_fingerprint"])

        param_only = mod.handle_build_plan({
            "job_id": "compute_j",
            "run_id": "run_param_only",
            "task_id": "compute_run_aberth_mt_run_param_only",
            "params": {
                "pipeline_mode": "program",
                "solver_mode": "aberth_mt",
                "N": 20,
                "times": 1,
                "n_chunks": 2,
                "function": "g1",
                "param_transforms": [],
                "param_program_chain": [["push", "t1"], ["emit", "p1"]],
                "param_program_source_text": "",
                "coeff_transforms": [],
                "coeff_program_chain": [],
                "coeff_program_source_text": "",
                "cfpv": [],
            },
        })
        param_plan = json.loads(param_only["body"])
        self.assertTrue(param_plan["pipeline"]["param_program_fingerprint"])
        self.assertEqual(param_plan["pipeline"]["coeff_program"], {})

    def test_build_plan_explicit_empty_program_chains_do_not_resurrect_legacy_transforms(self):
        import handler_compute_plan as mod

        result = mod.handle_build_plan({
            "job_id": "compute_j",
            "run_id": "run_empty_programs",
            "task_id": "compute_run_aberth_mt_run_empty_programs",
            "params": {
                "pipeline_mode": "program",
                "solver_mode": "aberth_mt",
                "N": 20,
                "times": 1,
                "n_chunks": 2,
                "function": "g1",
                "param_transforms": [["unit_circle"]],
                "param_program_chain": [],
                "coeff_transforms": [["rev"]],
                "coeff_program_chain": [],
                "cfpv": [],
            },
        })
        plan = json.loads(result["body"])
        self.assertEqual(plan["pipeline"]["param_transforms"], [])
        self.assertEqual(plan["pipeline"]["coeff_transforms"], [])
        self.assertEqual(plan["pipeline"]["param_program_chain"], [])
        self.assertEqual(plan["pipeline"]["coeff_program_chain"], [])
        self.assertEqual(plan["pipeline"]["param_program"], {})
        self.assertEqual(plan["pipeline"]["coeff_program"], {})

    def test_build_plan_keeps_legacy_equivalent_param_program_as_vm_payload(self):
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
        self.assertEqual(plan["pipeline"]["param_transforms"], [])
        self.assertEqual(plan["pipeline"]["param_program"]["token_count"], 1)
        self.assertTrue(plan["pipeline"]["param_program"]["uses_legacy_fast_path"])
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

    def test_build_plan_compiles_coeff_program_source_text(self):
        import handler_compute_plan as mod

        result = mod.handle_build_plan({
            "job_id": "compute_j",
            "run_id": "run_coeff_program_source",
            "task_id": "compute_run_aberth_mt_run_coeff_program_source",
            "params": {
                "pipeline_mode": "program",
                "solver_mode": "aberth_mt",
                "N": 20,
                "times": 1,
                "n_chunks": 2,
                "function": "const",
                "param_transforms": [],
                "param_program_chain": [],
                "coeff_transforms": [["rev"]],
                "coeff_program_source_text": "fill(poly_len, p1+p2)\nemit\n",
                "cfpv": [35, 1, 0],
            },
        })
        plan = json.loads(result["body"])
        self.assertEqual(plan["pipeline"]["coeff_transforms"], [])
        self.assertEqual(plan["pipeline"]["coeff_program_source_text"], "fill(poly_len, p1+p2)\nemit\n")
        self.assertEqual(plan["pipeline"]["coeff_program_chain"], [
            ["_typed_push_scalar", "poly_len"],
            ["_typed_push_scalar", "p1"],
            ["_typed_push_scalar", "p2"],
            ["_typed_binary", "add"],
            ["_typed_fill"],
            ["emit"],
        ])
        self.assertEqual(plan["pipeline"]["coeff_program"]["token_count"], 6)
        self.assertEqual(plan["pipeline"]["coeff_program"]["scalar_expr_count"], 0)

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
        self.assertEqual(plan["pipeline"]["coeff_transforms"], [])
        self.assertEqual(plan["pipeline"]["coeff_program"]["token_count"], 1)
        self.assertTrue(plan["pipeline"]["coeff_program"]["uses_legacy_chain_equivalent"])
        self.assertTrue(plan["pipeline"]["coeff_program_uses_legacy_chain_equivalent"])
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
                "probe_signature_spec_version": mod.PROBE_SIGNATURE_SPEC_VERSION,
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
                "probe_signature_spec_version": mod.PROBE_SIGNATURE_SPEC_VERSION,
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
                "probe_signature_spec_version": mod.PROBE_SIGNATURE_SPEC_VERSION,
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
                "coeff_program_source_text": "poly[0] = p1\npoly = rev(poly)\n",
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
        self.assertEqual(body["pipeline"]["coeff_program_source_text"], "poly[0] = p1\npoly = rev(poly)\n")
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


class TestClassicMemoryFloor(unittest.TestCase):
    def test_classic_cm_chunks_raised_to_memory_floor(self):
        """CR35-F8: a legal classic CM plan must not exceed the sweep_cm
        lambda's memory — the planner derives a chunk floor from the
        probed degree (review case: degree 70, N 1600, chunks 1)."""
        import handler_compute_plan as mod

        request = {
            "job_id": "compute_j",
            "run_id": "run_mem",
            "task_id": "compute_run_x_run_mem",
            "probe": {
                "probe_stable": True,
                "probe_signature_spec_version": mod.PROBE_SIGNATURE_SPEC_VERSION,
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
                "N": 1600,
                "times": 1,
                "n_chunks": 1,
                "solver_mode": "companion_matrix",
                "function": "g1",
                "param_transforms": [],
                "coeff_transforms": [],
                "cfpv": [],
            },
        }
        plan = json.loads(mod.handle_build_plan(request)["body"])
        floor = plan["compute"]["classic_min_memory_chunks"]
        # degree 70 at N=1600 is ~2.9 GB C-side (post-streaming): fits in
        # one chunk, floor 1 — the honest post-fix number for the review
        # case, since the Python copy no longer exists
        total_bytes = 1600 * 1600 * (71 + 70) * 8
        import math as _m
        self.assertEqual(floor, _m.ceil(total_bytes / 3_300_000_000))
        self.assertGreaterEqual(plan["compute"]["n_chunks"], floor)
        # threading is a planned value (CR35-F12)
        self.assertEqual(plan["solve"]["threads"], 2)

        # a case that genuinely exceeds one chunk: degree 200 CM
        big = json.loads(json.dumps(request))
        big["probe"]["degree"] = 200
        big["probe"]["n_coeffs"] = 201
        plan_big = json.loads(mod.handle_build_plan(big)["body"])
        floor_big = plan_big["compute"]["classic_min_memory_chunks"]
        self.assertEqual(
            floor_big, _m.ceil(1600 * 1600 * (201 + 200) * 8 / 3_300_000_000))
        self.assertGreater(floor_big, 1)
        self.assertGreaterEqual(plan_big["compute"]["n_chunks"], floor_big)

        # aberth uses the 10240 MB lambda's budget (round-2 finding 2)
        ae = json.loads(json.dumps(big))
        ae["params"]["solver_mode"] = "aberth_mt"
        plan_ae = json.loads(mod.handle_build_plan(ae)["body"])
        self.assertEqual(
            plan_ae["compute"]["classic_min_memory_chunks"],
            _m.ceil(1600 * 1600 * (201 + 200) * 8 / 9_000_000_000))
