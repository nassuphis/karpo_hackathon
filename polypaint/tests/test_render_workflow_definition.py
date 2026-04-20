"""
Tests for the rendered Step Functions ASL template.

The color pipeline is fused-only now. These tests pin the shipped workflow,
not the deleted classic color states.
"""

import json
import os
import re
import unittest


TEMPLATE_PATH = os.path.join(
    os.path.dirname(__file__), "..", "stepfunctions", "render_workflow.asl.json.template"
)


def _load_asl():
    with open(TEMPLATE_PATH, encoding="utf-8") as fh:
        raw = fh.read()
    rendered = re.sub(
        r"\$\{(\w+)\}",
        r"arn:aws:lambda:us-east-1:123456789012:function:placeholder-\1",
        raw,
    )
    return json.loads(rendered)


def _walk_state_containers(states, path=""):
    yield path, states or {}
    for state_name, state in (states or {}).items():
        if not isinstance(state, dict):
            continue
        item_processor = state.get("ItemProcessor") or {}
        inner_states = item_processor.get("States")
        if isinstance(inner_states, dict):
            yield from _walk_state_containers(inner_states, f"{path}/{state_name}/ItemProcessor")
        for idx, branch in enumerate(state.get("Branches") or []):
            branch_states = branch.get("States")
            if isinstance(branch_states, dict):
                yield from _walk_state_containers(branch_states, f"{path}/{state_name}/Branches[{idx}]")


def _collect_state_names(states):
    names = {}
    for path, container in _walk_state_containers(states):
        for name in container:
            names.setdefault(name, []).append(f"{path}/{name}" or f"/{name}")
    return names


class TestWorkflowDefinition(unittest.TestCase):
    def setUp(self):
        self.asl = _load_asl()
        self.top_states = self.asl["States"]
        wrapper = self.top_states["WorkflowWrapper"]
        branches = wrapper.get("Branches", [{}])
        self.states = branches[0].get("States", {}) if branches else {}
        self.all_states = {**self.top_states, **self.states}

    def _state_at_path(self, path):
        parts = [part for part in path.split("/") if part]
        state = None
        container = self.asl["States"]
        idx = 0
        while idx < len(parts):
            part = parts[idx]
            if part == "ItemProcessor":
                state = (state.get("ItemProcessor") or {})
                container = state["States"]
                idx += 1
                continue
            if part.startswith("Branches["):
                branch_idx = int(part[len("Branches["):-1])
                state = (state.get("Branches") or [])[branch_idx]
                container = state["States"]
                idx += 1
                continue
            state = container[part]
            idx += 1
            if idx < len(parts):
                next_part = parts[idx]
                if next_part == "ItemProcessor" or next_part.startswith("Branches["):
                    continue
                raise KeyError(f"unsupported state path: {path}")
        return state

    def test_template_parses(self):
        self.assertIn("StartAt", self.asl)
        self.assertIn("States", self.asl)
        self.assertIn("WorkflowWrapper", self.top_states)
        self.assertTrue(self.states)

    def test_required_top_level_states_exist(self):
        for name in ["WorkflowWrapper", "ReportError", "Fail", "Succeed"]:
            self.assertIn(name, self.top_states)

    def test_required_inner_states_exist(self):
        for name in [
            "CleanRender",
            "BuildPlan",
            "ParsePlan",
            "ModeChoice",
            "ColorClipPhase",
            "ColorClipTask",
            "ColorRasterPhase",
            "ColorRasterMap",
            "ColorAssembleEncodePhase",
            "ColorAssembleEncodeTask",
            "ReportDoneColor",
            "BilevelRasterPhase",
            "BilevelRasterMap",
            "BilevelFinalizePhase",
            "BilevelFinalizeTask",
            "ReportDoneBilevel",
            "CoeffRasterPhase",
            "CoeffRasterMap",
            "CoeffMergePhase",
            "CoeffMergeMap",
            "CoeffStitchPhase",
            "CoeffStitchTask",
            "CoeffPreviewTask",
            "ReportDoneCoeffBilevel",
        ]:
            self.assertIn(name, self.states, f"missing state {name}")

    def test_deleted_classic_color_states_stay_deleted(self):
        for name in [
            "ColorSolveScoreChoice",
            "ColorRenderPipelineChoice",
            "ColorSolveScoreClipPhase",
            "ColorSolveScoreClipTask",
            "ColorFusedClipPhase",
            "ColorFusedClipTask",
            "ColorSolveScoreHistPhase",
            "ColorSolveScoreHistMap",
            "ColorSolveScoreMergePhase",
            "ColorSolveScoreMergeTask",
            "ColorAssociatedPaletteChoice",
            "ColorAssociatedPalettePhase",
            "ColorAssociatedPaletteMap",
            "ColorAssociatedPaletteFinalizePhase",
            "ColorAssociatedPaletteFinalizeTask",
            "ColorFusedRasterPhase",
            "ColorFusedRasterMap",
            "ColorFinalizePhase",
            "ColorFinalizeMap",
            "ColorEncodePhase",
            "ColorEncodeTask",
            "ColorPreviewTask",
        ]:
            self.assertNotIn(name, self.states)

    def test_mode_choice_routes_color_to_fused_clip(self):
        choice = self.states["ModeChoice"]
        color_choice = next(item for item in choice["Choices"] if item["StringEquals"] == "color")
        self.assertEqual(color_choice["Next"], "ColorClipPhase")

    def test_map_states_have_expected_concurrency(self):
        self.assertEqual(self.states["ColorRasterMap"]["Type"], "Map")
        self.assertEqual(self.states["ColorRasterMap"]["MaxConcurrencyPath"], "$.plan.raster.workers")
        self.assertEqual(self.states["BilevelRasterMap"]["MaxConcurrency"], 10)
        self.assertEqual(self.states["CoeffRasterMap"]["MaxConcurrency"], 10)
        self.assertEqual(self.states["CoeffMergeMap"]["MaxConcurrency"], 32)

    def test_color_clip_payload_threads_coeffs_and_params(self):
        payload = self.states["ColorClipTask"]["Parameters"]["Payload"]
        self.assertEqual(payload["phase"], "clip")
        self.assertEqual(payload["metric.$"], "$.plan.solve_score.metric")
        self.assertEqual(payload["solve_score_chain.$"], "$.plan.solve_score.chain")
        self.assertEqual(payload["solve_score_threads.$"], "$.plan.solve_score.threads")
        self.assertEqual(payload["lores_bin_key.$"], "$.plan.calc.lores_bin_key")
        self.assertEqual(payload["lores_coeffs_key.$"], "$.plan.calc.lores_coeffs_key")
        self.assertEqual(payload["lores_params_key.$"], "$.plan.calc.lores_params_key")
        self.assertEqual(payload["n_coeffs.$"], "$.plan.calc.n_coeffs")
        self.assertEqual(payload["out_key.$"], "$.plan.solve_score.clip_key")
        self.assertEqual(self.states["ColorClipTask"]["ResultPath"], "$.solve_score_clip")
        self.assertEqual(
            self.states["ColorClipTask"]["ResultSelector"]["parsed.$"],
            "States.StringToJson($.Payload.body)",
        )

    def test_color_raster_selector_is_fused_only(self):
        color_map = self.states["ColorRasterMap"]
        self.assertEqual(color_map["ItemsPath"], "$.plan.raster.map_items")
        selector = color_map["ItemSelector"]
        self.assertEqual(selector["color_pipeline"], "fused")
        self.assertEqual(selector["logical_section.$"], "$.plan.raster.logical_section")
        self.assertEqual(selector["solve_source_manifest.$"], "$.plan.solve_source_manifest")
        self.assertEqual(selector["n_coeffs.$"], "$.plan.calc.n_coeffs")
        self.assertEqual(selector["solve_score_clip_key.$"], "$.plan.solve_score.clip_key")
        self.assertEqual(selector["solve_score_chain.$"], "$.plan.solve_score.chain")
        self.assertEqual(selector["solve_score_quantile.$"], "$.plan.solve_score.quantile")
        self.assertEqual(selector["solve_score_omega.$"], "$.plan.solve_score.omega")
        self.assertEqual(selector["solve_score_omega_enabled.$"], "$.plan.solve_score.omega_enabled")
        self.assertEqual(selector["raster_function_name.$"], "$.plan.raster.function_name")
        self.assertEqual(selector["raster_input_mode.$"], "$.plan.raster.input_mode")
        self.assertEqual(selector["raster_sectioned_retries.$"], "$.plan.raster.sectioned_retries")
        self.assertEqual(selector["associated_palette_mode.$"], "$.plan.associated_palette.mode")
        self.assertEqual(selector["associated_palette_fragment_prefix.$"], "$.plan.associated_palette.fragment_prefix")
        self.assertEqual(selector["fragment_prefix.$"], "$.plan.outputs.fragment_prefix")
        for deleted_field in [
            "bin_key.$",
            "bin_size.$",
            "coeffs_key.$",
            "coeffs_bin_size.$",
            "params_key.$",
            "params_bin_size.$",
            "params_step_start.$",
            "params_step_count.$",
            "group_idx.$",
            "sections.$",
            "section_indices.$",
            "emit_raw_score_bins.$",
            "pixel_bins_drive_rgb.$",
        ]:
            self.assertNotIn(deleted_field, selector)
        worker = color_map["ItemProcessor"]["States"]["ColorRasterWorker"]
        self.assertEqual(color_map["ItemProcessor"]["StartAt"], "ColorRasterWorker")
        self.assertEqual(worker["Parameters"]["FunctionName.$"], "$.raster_function_name")

    def test_finalize_mt_payload_is_fused_contract(self):
        payload = self.states["ColorAssembleEncodeTask"]["Parameters"]["Payload"]
        self.assertEqual(payload["phase"], "finalize_mt")
        self.assertEqual(payload["render_execution.$"], "$.plan.render_execution")
        self.assertEqual(payload["metadata.$"], "$.plan.outputs.metadata")
        self.assertEqual(payload["raw_key.$"], "$.plan.outputs.raw_key")
        self.assertEqual(payload["raw_meta_key.$"], "$.plan.outputs.raw_meta_key")
        self.assertEqual(payload["fragment_prefix.$"], "$.plan.outputs.fragment_prefix")
        self.assertEqual(payload["source_item_count.$"], "$.plan.raster.item_count")
        self.assertEqual(payload["clip_slots.$"], "$.solve_score_clip.parsed.clip_slots")
        self.assertEqual(payload["score_program.$"], "$.solve_score_clip.parsed.score_program")
        self.assertEqual(payload["chain_fingerprint.$"], "$.solve_score_clip.parsed.chain_fingerprint")
        manifest = payload["fragment_manifest"]
        self.assertEqual(manifest["version"], 1)
        self.assertEqual(manifest["pair_encoding"], "u32le_u8_v1")
        self.assertEqual(manifest["item_count.$"], "$.plan.raster.item_count")
        self.assertEqual(manifest["fragment_prefix.$"], "$.plan.outputs.fragment_prefix")

    def test_bilevel_and_coeff_pipelines_are_intact(self):
        bilevel_raster = self.states["BilevelRasterMap"]["ItemSelector"]
        self.assertEqual(bilevel_raster["solve_source_manifest.$"], "$.plan.solve_source_manifest")
        self.assertEqual(bilevel_raster["section_bits_prefix.$"], "$.plan.bilevel.section_bits_prefix")
        self.assertEqual(bilevel_raster["step_start.$"], "$$.Map.Item.Value.step_start")
        self.assertEqual(bilevel_raster["step_count.$"], "$$.Map.Item.Value.step_count")

        bilevel_finalize = self.states["BilevelFinalizeTask"]["Parameters"]["Payload"]
        self.assertEqual(bilevel_finalize["phase"], "finalize")
        self.assertEqual(bilevel_finalize["source_item_count.$"], "$.plan.bilevel.item_count")
        self.assertEqual(bilevel_finalize["section_bits_prefix.$"], "$.plan.bilevel.section_bits_prefix")

        coeff_raster = self.states["CoeffRasterMap"]["ItemSelector"]
        self.assertEqual(coeff_raster["coeffs_key.$"], "States.ArrayGetItem($.plan.calc.coeffs_keys, $$.Map.Item.Value.chunk_idx)")
        self.assertEqual(coeff_raster["n_coeffs.$"], "$.plan.calc.n_coeffs")
        coeff_stitch = self.states["CoeffStitchTask"]["Parameters"]["Payload"]
        self.assertEqual(coeff_stitch["out_key.$"], "$.plan.outputs.coeff_bilevel_key")
        self.assertEqual(coeff_stitch["preview_key.$"], "$.plan.outputs.preview_key")

    def test_report_states_use_expected_output_keys(self):
        color_payload = self.states["ReportDoneColor"]["Parameters"]["Payload"]
        self.assertEqual(color_payload["image_key.$"], "$.plan.outputs.image_key")

        bilevel_payload = self.states["ReportDoneBilevel"]["Parameters"]["Payload"]
        self.assertEqual(bilevel_payload["image_key.$"], "$.plan.outputs.bilevel_key")

        coeff_payload = self.states["ReportDoneCoeffBilevel"]["Parameters"]["Payload"]
        self.assertEqual(coeff_payload["image_key.$"], "$.plan.outputs.coeff_bilevel_key")

    def test_preview_tasks_target_preview_lambda(self):
        asl_str = json.dumps(self.asl)
        self.assertIn("placeholder-PreviewFunctionArn", asl_str)
        preview = self.states["CoeffPreviewTask"]
        self.assertEqual(preview["Type"], "Task")
        self.assertEqual(preview["Parameters"]["FunctionName"], "arn:aws:lambda:us-east-1:123456789012:function:placeholder-PreviewFunctionArn")

    def test_run_started_at_ms_is_forwarded(self):
        for state_name in [
            "CleanRender",
            "ColorClipPhase",
            "ColorRasterPhase",
            "ColorAssembleEncodePhase",
            "BilevelRasterPhase",
            "BilevelFinalizePhase",
            "CoeffRasterPhase",
            "CoeffMergePhase",
            "CoeffStitchPhase",
            "ReportDoneColor",
            "ReportDoneBilevel",
            "ReportDoneCoeffBilevel",
        ]:
            payload = self.states[state_name]["Parameters"]["Payload"]
            self.assertEqual(payload["run_started_at_ms.$"], "$.run_started_at_ms")
        self.assertEqual(
            self.top_states["ReportError"]["Parameters"]["Payload"]["run_started_at_ms.$"],
            "$.run_started_at_ms",
        )

    def test_parallel_wrapper_catches_all_errors(self):
        wrapper = self.top_states["WorkflowWrapper"]
        self.assertEqual(wrapper["Type"], "Parallel")
        self.assertEqual(wrapper["Catch"][0]["ErrorEquals"], ["States.ALL"])
        self.assertEqual(wrapper["Catch"][0]["Next"], "ReportError")

    def test_all_state_names_are_globally_unique(self):
        duplicates = {
            name: paths
            for name, paths in _collect_state_names(self.asl["States"]).items()
            if len(paths) > 1
        }
        self.assertEqual(duplicates, {})

    def test_nested_startat_targets_exist(self):
        for path, container in _walk_state_containers(self.asl["States"]):
            start_at = None
            if path == "":
                start_at = self.asl.get("StartAt")
            elif "/ItemProcessor" in path:
                parent_path = path.rsplit("/ItemProcessor", 1)[0]
                parent = self._state_at_path(parent_path)
                start_at = (parent.get("ItemProcessor") or {}).get("StartAt")
            elif "/Branches[" in path:
                branch_prefix, branch_idx = path.rsplit("/Branches[", 1)
                idx = int(branch_idx.rstrip("]"))
                parent = self._state_at_path(branch_prefix)
                start_at = (parent.get("Branches") or [{}])[idx].get("StartAt")
            if start_at is not None:
                self.assertIn(start_at, container, f"{path or '/'} StartAt {start_at!r} missing")

    def test_nested_transitions_target_existing_local_states(self):
        for path, container in _walk_state_containers(self.asl["States"]):
            for name, state in container.items():
                if not isinstance(state, dict):
                    continue
                targets = []
                if state.get("Next"):
                    targets.append(("Next", state["Next"]))
                if state.get("Default"):
                    targets.append(("Default", state["Default"]))
                for choice in state.get("Choices") or []:
                    if isinstance(choice, dict) and choice.get("Next"):
                        targets.append(("Choice.Next", choice["Next"]))
                for field_name, target in targets:
                    self.assertIn(
                        target,
                        container,
                        f"{path or '/'}{name} {field_name} target {target!r} missing",
                    )


if __name__ == "__main__":
    unittest.main()
