import os
import sys
import unittest


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


from raw_sidecar import build_raw_sidecar, validate_raw_sidecar


class TestRawSidecar(unittest.TestCase):
    def test_build_and_validate_v3_color_sidecar_with_step_scores(self):
        sidecar = build_raw_sidecar(
            job_id="j",
            run_id="run_1",
            artifact_family="color",
            artifact_id="color_1",
            width=4,
            height=4,
            chain_fingerprint="fp_123",
            score_chain=[["crowding", "0.1"]],
            score_program="m0",
            score_source_text="score = metric(crowding, slv, q=0.1%)\nemit(score)",
            clip_slots=[{"slot": 0, "metric": "crowding", "source": "slv", "clip_lo": 0.1, "clip_hi": 0.9}],
            score_output_normalize=True,
            score_output_clip_lo=0.02,
            score_output_clip_hi=0.08,
            background_color="000000",
            plan_params_digest="sha256:plan",
            render_execution={"raster_engine": "mt"},
            raw_key="renders/j/color/color_1/greyscale.raw",
            image_key="renders/j/color/color_1/image.jpeg",
            preview_key="renders/j/color/color_1/preview.png",
            meta_key="renders/j/color/color_1/meta.json",
            created_at="2026-04-18T00:00:00Z",
            histogram=[0, 1, 2, 3] + [0] * 252,
            step_scores_key="renders/j/color/color_1/step_scores.raw",
            step_count=9,
            step_scores_grid_n=3,
        )
        self.assertEqual(sidecar["version"], 3)
        self.assertEqual(sidecar["solve_score_spec_version"], 1)
        self.assertEqual(sidecar["pix"], 4)
        self.assertEqual(sidecar["score_output_normalize"], True)
        self.assertEqual(sidecar["solve_score_program_source_text"], "score = metric(crowding, slv, q=0.1%)\nemit(score)")
        self.assertEqual(sidecar["score_output_clip_lo"], 0.02)
        self.assertEqual(sidecar["score_output_clip_hi"], 0.08)
        validated = validate_raw_sidecar(
            sidecar,
            expected_raw_key="renders/j/color/color_1/greyscale.raw",
            expected_artifact_family="color",
        )
        self.assertEqual(validated["version"], 3)
        self.assertEqual(validated["solve_score_spec_version"], 1)
        self.assertEqual(validated["pix"], 4)
        self.assertEqual(validated["step_scores_key"], "renders/j/color/color_1/step_scores.raw")
        self.assertEqual(validated["step_count"], 9)
        self.assertEqual(validated["step_scores_grid_n"], 3)
        self.assertEqual(validated["score_source_text"], "score = metric(crowding, slv, q=0.1%)\nemit(score)")
        self.assertEqual(validated["solve_score_program_source_text"], "score = metric(crowding, slv, q=0.1%)\nemit(score)")
        self.assertEqual(validated["score_output_normalize"], True)
        self.assertEqual(validated["score_output_clip_lo"], 0.02)
        self.assertEqual(validated["score_output_clip_hi"], 0.08)

    def test_validate_v3_requires_step_score_fields(self):
        sidecar = build_raw_sidecar(
            job_id="j",
            run_id="run_1",
            artifact_family="color",
            artifact_id="color_1",
            width=4,
            height=4,
            chain_fingerprint="fp_123",
            score_chain=[["crowding", "0.1"]],
            score_program="m0",
            clip_slots=[{"slot": 0, "metric": "crowding", "source": "slv", "clip_lo": 0.1, "clip_hi": 0.9}],
            background_color="000000",
            plan_params_digest="sha256:plan",
            render_execution={"raster_engine": "mt"},
            raw_key="renders/j/color/color_1/greyscale.raw",
            image_key="renders/j/color/color_1/image.jpeg",
            preview_key="renders/j/color/color_1/preview.png",
            meta_key="renders/j/color/color_1/meta.json",
            created_at="2026-04-18T00:00:00Z",
            histogram=[0, 1, 2, 3] + [0] * 252,
            step_scores_key="renders/j/color/color_1/step_scores.raw",
            step_count=9,
            step_scores_grid_n=3,
        )
        broken = dict(sidecar)
        broken.pop("step_scores_key")
        with self.assertRaisesRegex(RuntimeError, "step_scores_key is required"):
            validate_raw_sidecar(broken)
