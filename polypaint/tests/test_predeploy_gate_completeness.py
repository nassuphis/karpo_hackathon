"""Gate meta-test: the predeploy pytest list must not rot.

Every prior audit cycle found "green by omission": bug-pinning test files that
existed on disk but were never wired into scripts/predeploy_check.sh, so the
gate stayed green while the contract they guarded regressed (code-review-23.md
H8). This test pins three properties:

1. Every tests/ file referenced by the gate exists (no phantom entries).
2. A named set of load-bearing contract files is present in the gate.
3. Every tests/test_*.py on disk is either gated or in the explicit
   ALLOWED_UNGATED snapshot below. Adding a new test file therefore forces a
   decision: gate it, or append it here with a reason comment.

ALLOWED_UNGATED is a frozen snapshot of the historically ungated files
(2026-07-02): slow visual/hand/bench/parity suites, Docker-only suites, and
handler suites that predate the gate. Removing entries from it (by gating
them) is always safe; adding entries should be rare and deliberate.
"""

import pathlib
import re
import unittest

ROOT = pathlib.Path(__file__).resolve().parent.parent
GATE = ROOT / "scripts" / "predeploy_check.sh"
TESTS = ROOT / "tests"

# Files that MUST be in the gate: each pins a verified bug fix or a
# wire/registry contract (see code-review-23.md).
REQUIRED_GATED = {
    "test_registry_schema.py",
    "test_program_v2_migration.py",
    "test_saved_program_source_precedence.py",
    "test_root_program_source.py",
    "test_root_program_storage.py",
    "test_root_transform_registry.py",
    "test_coeff_source_equivalence.py",
    "test_whole_sweep_oracle.py",
    "test_solve_score_native_parity.py",
    "test_program_source_core.py",
    "test_program_profiles_drift.py",
    "test_frontend_parts_contract.py",
    "test_program_m3_oracles.py",
    "test_coeff_wire_fingerprints.py",
    "test_param_program_drift.py",
    "test_coeff_program_drift.py",
    "test_predeploy_gate_completeness.py",
}

# Frozen snapshot of deliberately ungated files. Slow (visual/poly/hand/bench),
# environment-bound (Docker/native-binary/giga), or legacy handler suites that
# predate the gate. Gating any of these later just means deleting its line.
ALLOWED_UNGATED = {
    "test_ae_mt.py",
    # test_assemble_greyscale.py and test_attach_palette_to_color_handler.py were
    # promoted into the predeploy gate (code-review-28 F20): each is now the sole
    # coverage of load-bearing hardening (native retry semantics; palette
    # identity), so they must not remain optional bookkeeping.
    "test_autolevels_render_native.py",
    "test_bench_solve_hist_batch.py",
    "test_cfpv.py",
    "test_chunking.py",
    "test_coeff_catalog_consistency.py",
    "test_coeff_parity_results.py",
    "test_companion_matrix.py",
    "test_deepzoom_viewport_math.py",
    "test_dither.py",
    "test_exact_viewport_parity.py",
    "test_extract_palette_from_step_scores.py",
    "test_fetch_function.py",
    "test_fragment_format.py",
    "test_gemini_generate_title_description.py",
    "test_giga11_variants.py",
    "test_giga138_hand.py",
    "test_giga139_params.py",
    "test_giga62_hand.py",
    "test_logical_lores.py",
    "test_logical_sections.py",
    "test_long_palette_generation.py",
    "test_low_agreement_hand.py",
    "test_multispan_reader.py",
    "test_oai_generate_title_description.py",
    "test_p11a_hand.py",
    "test_p11b2_v2_hand.py",
    "test_palette_chunk_mt_parity.py",
    "test_palette_names.py",
    "test_param_dump.py",
    "test_pipeline.py",
    "test_png_export.py",
    "test_poly164_hand.py",
    "test_poly200_visual.py",
    "test_poly645_hand.py",
    "test_poly795_hand.py",
    "test_poly_1.py",
    "test_poly_125.py",
    "test_poly_29.py",
    "test_poly_33.py",
    "test_poly_34.py",
    "test_poly_45.py",
    "test_poly_accuracy.py",
    "test_poly_creative9.py",
    "test_poly_solver.py",
    "test_program_source_cheatsheets.py",
    "test_raster_mt_parity.py",
    "test_raw_sidecar.py",
    "test_recolor_from_raw.py",
    "test_solve_palette_debug.py",
    "test_solve_proximity_bench.py",
    "test_solve_proximity_hist_sectioned.py",
    "test_solve_proximity_stats.py",
    "test_sweep_smoke.py",
    "test_tiff_compat.py",
    "test_tri_palette_generation.py",
    "test_update_polybook.py",
    "test_visual_101_200.py",
    "test_visual_1_100.py",
    "test_visual_201_300.py",
    "test_visual_301_500.py",
    "test_visual_501_600.py",
    "test_visual_601_700.py",
    "test_visual_701_800.py",
    "test_visual_801_821.py",
    "test_visual_g1_g99.py",
    "test_visual_giga.py",
}


def _gated_files():
    text = GATE.read_text()
    return set(re.findall(r"tests/(test_\w+\.py)", text))


class TestPredeployGateCompleteness(unittest.TestCase):
    def test_gate_references_only_existing_files(self):
        missing = sorted(f for f in _gated_files() if not (TESTS / f).exists())
        self.assertEqual(missing, [], f"gate references missing test files: {missing}")

    def test_required_contract_files_are_gated(self):
        gated = _gated_files()
        absent = sorted(REQUIRED_GATED - gated)
        self.assertEqual(absent, [], f"required contract tests missing from gate: {absent}")

    def test_every_test_file_is_gated_or_explicitly_excluded(self):
        gated = _gated_files()
        disk = {p.name for p in TESTS.glob("test_*.py")}
        unaccounted = sorted(disk - gated - ALLOWED_UNGATED)
        self.assertEqual(
            unaccounted,
            [],
            "new test files must be added to scripts/predeploy_check.sh or, "
            f"deliberately, to ALLOWED_UNGATED: {unaccounted}",
        )

    def test_no_stale_exclusions(self):
        gated = _gated_files()
        disk = {p.name for p in TESTS.glob("test_*.py")}
        stale = sorted((ALLOWED_UNGATED & gated) | (ALLOWED_UNGATED - disk))
        self.assertEqual(
            stale,
            [],
            f"ALLOWED_UNGATED entries that are gated or deleted (remove them): {stale}",
        )




# code-review-30 F13: the browser gate gets the same completeness accounting as
# pytest — a new tests/e2e/*.spec.js must be gated or EXPLICITLY excluded here.
# (render-solve-score is GATED with --grep-invert on its one known-red
# ColorRender-MT fixture test — CR30 follow-up F11 — so nothing is excluded today.)
EXCLUDED_E2E = set()


class TestE2EGateCompleteness(unittest.TestCase):
    def _gate_source(self):
        root = pathlib.Path(__file__).resolve().parent.parent
        return (root / "scripts" / "predeploy_check.sh").read_text()

    def _gated_specs(self):
        return set(re.findall(r"tests/e2e/([\w.-]+\.spec\.js)", self._gate_source()))

    def _disk_specs(self):
        root = pathlib.Path(__file__).resolve().parent
        return {p.name for p in (root / "e2e").glob("*.spec.js")}

    def test_every_spec_is_gated_or_explicitly_excluded(self):
        unaccounted = sorted(self._disk_specs() - self._gated_specs() - EXCLUDED_E2E)
        self.assertEqual(unaccounted, [],
            f"e2e specs neither gated nor excluded with a reason: {unaccounted}")

    def test_gated_specs_exist_and_exclusions_are_live(self):
        missing = sorted(self._gated_specs() - self._disk_specs())
        self.assertEqual(missing, [], f"gate references deleted specs: {missing}")
        stale = sorted((EXCLUDED_E2E & self._gated_specs()) | (EXCLUDED_E2E - self._disk_specs()))
        self.assertEqual(stale, [], f"stale EXCLUDED_E2E entries: {stale}")


if __name__ == "__main__":   # LAST statement — direct runs must see every class (CR30 follow-up F11)
    unittest.main()
