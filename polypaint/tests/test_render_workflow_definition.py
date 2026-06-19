"""
Tests for the rendered Step Functions ASL template.

The color pipeline is fused-only now. These tests pin the shipped workflow,
not the deleted classic color states.
"""

import json
import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

from workflow_contracts import (
    LEGACY_VIEWPORT_SELECTOR_FIELDS,
    RENDER_BILEVEL_FINALIZE_TASK_PAYLOAD,
    RENDER_BILEVEL_RASTER_ITEM_SELECTOR,
    RENDER_COEFF_FINALIZE_TASK_PAYLOAD,
    RENDER_COEFF_RASTER_ITEM_SELECTOR,
    RENDER_COLOR_CLIP_TASK_PAYLOAD,
    RENDER_COLOR_RASTER_ITEM_SELECTOR,
    RENDER_FINALIZE_MT_FRAGMENT_MANIFEST,
    RENDER_FINALIZE_MT_TASK_PAYLOAD,
)
from workflow_template_render import render_render_workflow_definition_for_tests


def _load_asl():
    return render_render_workflow_definition_for_tests(
        account_id="123456789012",
        region="us-east-1",
    )


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

    def _assert_exact_mapping(self, actual, expected):
        self.assertEqual(actual, expected)

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
            "CoeffFinalizePhase",
            "CoeffFinalizeTask",
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

    def test_map_workers_retry_lambda_throttles(self):
        for map_name, worker_name in [
            ("ColorRasterMap", "ColorRasterWorker"),
            ("BilevelRasterMap", "BilevelRasterWorker"),
            ("CoeffRasterMap", "CoeffRasterWorker"),
        ]:
            worker = self.states[map_name]["ItemProcessor"]["States"][worker_name]
            retry_errors = worker["Retry"][0]["ErrorEquals"]
            self.assertIn("Lambda.TooManyRequestsException", retry_errors, map_name)

    def test_color_clip_payload_threads_coeffs_and_params(self):
        payload = self.states["ColorClipTask"]["Parameters"]["Payload"]
        self._assert_exact_mapping(payload, RENDER_COLOR_CLIP_TASK_PAYLOAD)
        self.assertEqual(self.states["ColorClipTask"]["ResultPath"], "$.solve_score_clip")
        self.assertEqual(
            self.states["ColorClipTask"]["ResultSelector"]["parsed.$"],
            "States.StringToJson($.Payload.body)",
        )

    def test_color_raster_selector_is_fused_only(self):
        color_map = self.states["ColorRasterMap"]
        self.assertEqual(color_map["ItemsPath"], "$.plan.raster.map_items")
        selector = color_map["ItemSelector"]
        self._assert_exact_mapping(selector, RENDER_COLOR_RASTER_ITEM_SELECTOR)
        worker = color_map["ItemProcessor"]["States"]["ColorRasterWorker"]
        self.assertEqual(color_map["ItemProcessor"]["StartAt"], "ColorRasterWorker")
        self.assertEqual(worker["Parameters"]["FunctionName.$"], "$.raster_function_name")

    def test_finalize_mt_payload_is_fused_contract(self):
        payload = self.states["ColorAssembleEncodeTask"]["Parameters"]["Payload"]
        self._assert_exact_mapping(payload, RENDER_FINALIZE_MT_TASK_PAYLOAD)
        manifest = payload["fragment_manifest"]
        self._assert_exact_mapping(manifest, RENDER_FINALIZE_MT_FRAGMENT_MANIFEST)

    def test_bilevel_and_coeff_pipelines_are_intact(self):
        bilevel_raster = self.states["BilevelRasterMap"]["ItemSelector"]
        self._assert_exact_mapping(bilevel_raster, RENDER_BILEVEL_RASTER_ITEM_SELECTOR)

        bilevel_finalize = self.states["BilevelFinalizeTask"]["Parameters"]["Payload"]
        self._assert_exact_mapping(bilevel_finalize, RENDER_BILEVEL_FINALIZE_TASK_PAYLOAD)

        coeff_raster = self.states["CoeffRasterMap"]["ItemSelector"]
        self._assert_exact_mapping(coeff_raster, RENDER_COEFF_RASTER_ITEM_SELECTOR)
        coeff_finalize = self.states["CoeffFinalizeTask"]["Parameters"]["Payload"]
        self._assert_exact_mapping(coeff_finalize, RENDER_COEFF_FINALIZE_TASK_PAYLOAD)

    def test_render_worker_selectors_reject_legacy_viewport_fields(self):
        for selector in (
            self.states["ColorRasterMap"]["ItemSelector"],
            self.states["BilevelRasterMap"]["ItemSelector"],
            self.states["CoeffRasterMap"]["ItemSelector"],
        ):
            for field_name in LEGACY_VIEWPORT_SELECTOR_FIELDS:
                self.assertNotIn(field_name, selector)

    def test_report_states_use_expected_output_keys(self):
        color_payload = self.states["ReportDoneColor"]["Parameters"]["Payload"]
        self.assertEqual(color_payload["image_key.$"], "$.plan.outputs.image_key")

        bilevel_payload = self.states["ReportDoneBilevel"]["Parameters"]["Payload"]
        self.assertEqual(bilevel_payload["image_key.$"], "$.plan.outputs.bilevel_key")

        coeff_payload = self.states["ReportDoneCoeffBilevel"]["Parameters"]["Payload"]
        self.assertEqual(coeff_payload["image_key.$"], "$.plan.outputs.coeff_bilevel_key")

    def test_coeff_bilevel_no_longer_uses_preview_or_stitch_lambdas(self):
        asl_str = json.dumps(self.asl)
        self.assertNotIn("CoeffPreviewTask", self.states)
        self.assertNotIn("CoeffStitchTask", self.states)
        self.assertNotIn("CoeffMergeMap", self.states)
        self.assertNotIn("placeholder-CoeffBilevelStitchFunctionArn", asl_str)

    def test_run_started_at_ms_is_forwarded(self):
        for state_name in [
            "CleanRender",
            "ColorClipPhase",
            "ColorRasterPhase",
            "ColorAssembleEncodePhase",
            "BilevelRasterPhase",
            "BilevelFinalizePhase",
            "CoeffRasterPhase",
            "CoeffFinalizePhase",
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
