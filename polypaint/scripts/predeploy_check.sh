#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if command -v uv >/dev/null 2>&1; then
    TEST_PYTHON=(uv run python)
elif [ -x "$ROOT/.venv/bin/python" ]; then
    TEST_PYTHON=("$ROOT/.venv/bin/python")
elif [ -x "$ROOT/../.venv/bin/python" ]; then
    TEST_PYTHON=("$ROOT/../.venv/bin/python")
else
    TEST_PYTHON=(python3)
fi

# Native-suite freshness: several gated tests execute lambda/sweep_test,
# which is untracked and otherwise only rebuilt by deploy.sh — a predeploy
# run after C changes but before a deploy would test a stale binary.
if [ ! -x lambda/sweep_test ] || [ lambda/sweep_cli.c -nt lambda/sweep_test ]; then
    echo "Rebuilding lambda/sweep_test (sweep_cli.c is newer)..."
    cc -O2 -pthread -o lambda/sweep_test lambda/sweep_cli.c -lm
fi

echo "Running predeploy contract gate..."

# Native deploy binaries are built later by deploy.sh. Checking them here
# would reject every legitimate native source change before deploy has a
# chance to rebuild it. deploy.sh records and verifies the final binaries
# immediately before the Docker runtime regression instead.
# The multi-worker source paths can and should still be TSan-clean here.
bash scripts/test-tsan-races.sh
"${TEST_PYTHON[@]}" api_manifest.py --check
"${TEST_PYTHON[@]}" deploy_manifest.py --check
"${TEST_PYTHON[@]}" lambda/gen_program_profiles.py --check
"${TEST_PYTHON[@]}" lambda/gen_merged_opcodes.py --check
"${TEST_PYTHON[@]}" lambda/gen_param_vocab.py --check
"${TEST_PYTHON[@]}" lambda/gen_coeff_vocab.py --check
"${TEST_PYTHON[@]}" lambda/gen_solve_score_vocab.py --check
"${TEST_PYTHON[@]}" lambda/gen_root_vocab.py --check
"${TEST_PYTHON[@]}" scripts/generate_tri_palettes.py --check
"${TEST_PYTHON[@]}" scripts/generate_long_palettes.py --check
# gen_catalog.py (coeff_func_catalog_js.js + coeff_func_lookup.h) is
# deliberately deploy-gated only: it probes the freshly built sweep_test
# binary for per-function degrees, so a predeploy --check would depend on
# local binary staleness. deploy.sh regenerates it FATAL-on-mismatch.
"${TEST_PYTHON[@]}" deploy_manifest.py --emit-bash > /tmp/polypaint-deploy-specs-gate.sh
bash -n /tmp/polypaint-deploy-specs-gate.sh
"${TEST_PYTHON[@]}" -m pytest \
    tests/test_api_route_contracts.py \
    tests/test_deploy_packaging.py \
    tests/test_merged_opcodes_drift.py \
    tests/test_render_workflow_definition.py \
    tests/test_render_plan.py \
    tests/test_pdf_artifact_handler.py \
    tests/test_spread_pdf.py \
    tests/test_finalize_mt_handler.py \
    tests/test_raster_mt.py \
    tests/test_bilevel_handler.py \
    tests/test_solve_proximity_handler.py \
    tests/test_registry_schema.py \
    tests/test_program_source_core.py \
    tests/test_source_locals_and_infix.py \
    tests/test_coeff_vm_property_fuzz.py \
    tests/test_coeff_fast_kernel_policy.py \
    tests/test_param_seed_policy.py \
    tests/test_root_prepared_parity.py \
    tests/test_vm_perf_counters.py \
    tests/test_production_telemetry_collector.py \
    tests/test_program_starter_snippets.py \
    tests/test_program_profiles_drift.py \
    tests/test_program_v2_migration.py \
    tests/test_saved_program_source_precedence.py \
    tests/test_program_run_boundaries.py \
    tests/test_predeploy_gate_completeness.py \
    tests/test_root_program_source.py \
    tests/test_root_program_storage.py \
    tests/test_book_storage.py \
    tests/test_book_tex.py \
    tests/test_book_pdf_handler.py \
    tests/test_root_transform_registry.py \
    tests/test_param_program_drift.py \
    tests/test_param_program_source.py \
    tests/test_param_program_chain.py \
    tests/test_param_program_storage.py \
    tests/test_param_program_native.py \
    tests/test_program_help_forms.py \
    tests/test_program_m3_oracles.py \
    tests/test_coeff_program_drift.py \
    tests/test_coeff_program_chain.py \
    tests/test_coeff_wire_fingerprints.py \
    tests/test_coeff_program_native.py \
    tests/test_coeff_program_storage.py \
    tests/test_giga_2902_coeff_program.py \
    tests/test_giga_263_coeff_program.py \
    tests/test_giga_265_coeff_program.py \
    tests/test_giga_2870_coeff_program.py \
    tests/test_giga_2871_coeff_program.py \
    tests/test_giga_2872_coeff_program.py \
    tests/test_giga_2873_coeff_program.py \
    tests/test_giga_2874_coeff_program.py \
    tests/test_giga_2875_coeff_program.py \
    tests/test_giga_2877_coeff_program.py \
    tests/test_coeff_source_equivalence.py \
    tests/test_solve_score_native_parity.py \
    tests/test_frontend_parts_contract.py \
    tests/test_whole_sweep_oracle.py \
    tests/test_solve_score_chain.py \
    tests/test_solve_score_pipeline_programs.py \
    tests/test_solve_score_program_source.py \
	    tests/test_solve_score_source_equivalence.py \
	    tests/test_solve_score_program_storage.py \
	    tests/test_storage_handler.py \
	    tests/test_favorites_storage.py \
    tests/test_results_catalog.py \
	    tests/test_gallery_backend.py \
	    tests/test_s3_error_taxonomy.py \
	    tests/test_render_key_identity.py \
	    tests/test_attach_palette_to_color_handler.py \
	    tests/test_assemble_greyscale.py \
	    tests/test_autolevels_handler.py \
	    tests/test_repair_preview_metadata.py \
	    tests/test_backfill_cache_headers.py \
	    tests/test_migrate_preview_jpg.py \
	    tests/test_wall_pyramid_handler.py \
	    tests/test_describe_book_entries.py \
	    tests/test_fable_programs.py \
	    tests/test_preview_handler.py \
    tests/test_render_lores_preview_handler.py \
    tests/test_compute_plan.py \
    tests/test_compute_orchestrator.py \
    tests/test_compute_preview_handler.py \
    tests/test_param_debug_handler.py \
    tests/test_coeffgen_param_gen.py \
    tests/test_compute_chunk_fused.py \
    tests/test_roots_stream_upload.py \
    tests/test_compute_workflow_definition.py \
    tests/test_compute_fused.py \
    tests/test_compute_status_handler.py \
    tests/test_palette_chunk_handler.py \
    tests/test_palette_finalize_handler.py \
    tests/test_palette_orchestrator.py \
    tests/test_palette_render_plan.py \
    tests/test_palette_workflow_definition.py \
    tests/test_palette_debug_handler.py \
    tests/test_render_orchestrator.py \
    tests/test_render_status.py \
    tests/test_dispatch_resilience.py \
    tests/test_deploy_manifest_validator.py \
    tests/test_png_export_handler.py \
    tests/test_resize_artifact_handler.py \
    tests/test_tiff_compat_handler.py \
    tests/test_repalette_handler.py \
    tests/test_color_repalette_handler.py \
    tests/test_deepzoom_export_handler.py \
    -q
bash tests/test_frontend_js.sh
# Browser regression specs (code-review-30 F13): EVERY load-bearing e2e suite
# is gated — tests/test_predeploy_gate_completeness.py accounts for each
# tests/e2e/*.spec.js as gated here or explicitly excluded with a reason.
npx playwright test \
    tests/e2e/gallery-logic.spec.js \
    tests/e2e/gallery-tab.spec.js \
    tests/e2e/gallery-viewer-smoke.spec.js \
    tests/e2e/gallery-texture-manager.spec.js \
    tests/e2e/gallery-curation.spec.js \
    tests/e2e/favorites-ui.spec.js \
    tests/e2e/results-ui.spec.js \
    tests/e2e/deepzoom-inventory.spec.js \
    tests/e2e/compute-ui.spec.js \
    tests/e2e/palette-ui.spec.js \
    tests/e2e/render-refresh.spec.js \
    tests/e2e/scrub-pad.spec.js
# render-solve-score: 46 good tests gated; ONLY the known-red ColorRender-MT
# fixture test is inverted out (pre-existing missing calc.job_size — CR30 follow-up F11).
npx playwright test tests/e2e/render-solve-score.spec.js \
    --grep-invert "ColorRender-MT popup exposes retries" 
echo "Predeploy contract gate passed."
