import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(**overrides):
    payload = {
        "job_id": "j",
        "task_id": "extract_palette_run_1",
        "artifact_id": "color_src",
    }
    payload.update(overrides)
    return {"body": json.dumps(payload)}


class TestExtractPaletteFromStepScores(unittest.TestCase):
    @patch("handler_extract_palette_from_step_scores.report_status")
    @patch("raw_score_render.subprocess.run")
    @patch("handler_extract_palette_from_step_scores.subprocess.run")
    @patch("handler_extract_palette_from_step_scores.load_color_artifact_head")
    @patch("handler_extract_palette_from_step_scores.s3")
    def test_fused_extract_palette_renders_from_cached_step_scores_and_updates_overlay(
        self,
        mock_s3,
        mock_load_head,
        mock_step_scores_run,
        mock_raw_render_run,
        mock_report,
    ):
        import handler_extract_palette_from_step_scores as mod

        selected_meta = {
            "family": "color",
            "artifact_id": "color_src",
            "created_at": "2026-04-18T00:00:00Z",
            "format": "jpeg",
            "quality": "90",
            "width": "2",
            "height": "2",
            "color_mode": "solve_score",
            "palette": "magma",
            "solve_metric": "crowding",
            "solve_score_chain": '[["crowding","0.1"]]',
            "solve_score_quantile": "0.01",
            "solve_score_omega": "4",
            "solve_score_omega_enabled": "false",
            "raw_key": "renders/j/color/color_src/greyscale.raw",
            "raw_meta_key": "renders/j/color/color_src/greyscale.meta.json",
            "step_scores_key": "renders/j/color/color_src/step_scores.raw",
            "step_count": "4",
            "step_scores_grid_n": "2",
            "background_color": "000000",
            "view_mode": "explicit",
            "min_re": "-3.5",
            "max_re": "1.25",
            "min_im": "-0.75",
            "max_im": "2.0",
            "rotation": "0.125",
        }
        puts = {}

        mock_load_head.return_value = {
            "artifact_id": "color_src",
            "image_key": "renders/j/color/color_src/image.jpeg",
            "metadata": selected_meta,
        }

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/color/color_src/step_scores.raw":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [bytes([9, 7, 5, 3])])}
            raise AssertionError(f"unexpected get_object key: {key}")

        def put_object(Bucket=None, Key=None, Body=None, ContentType=None, Metadata=None):
            puts[Key] = {
                "body": Body if isinstance(Body, (bytes, bytearray)) else Body.read(),
                "content_type": ContentType,
                "metadata": Metadata,
            }

        mock_s3.get_object.side_effect = get_object
        mock_s3.put_object.side_effect = put_object

        sidecar = {
            "version": 3,
            "job_id": "j",
            "run_id": "run_src",
            "artifact_family": "color",
            "artifact_id": "color_src",
            "width": 2,
            "height": 2,
            "encoding": {
                "type": "u8_clipped_score_v1",
                "background_byte": 0,
                "foreground_min": 1,
                "foreground_max": 255,
                "row_major": True,
            },
            "chain_fingerprint": "fp_src",
            "score_chain": [["crowding", "0.1"]],
            "score_program": "m0",
            "clip_slots": [{"slot": 0, "metric": "crowding", "source": "slv", "clip_lo": 0.1, "clip_hi": 0.9}],
            "background_color": [0, 0, 0],
            "plan_params_digest": "sha256:plan_src",
            "render_execution": {"raster_engine": "mt"},
            "keys": {
                "raw_key": "renders/j/color/color_src/greyscale.raw",
                "image_key": "renders/j/color/color_src/image.jpeg",
                "preview_key": "renders/j/color/color_src/preview.png",
                "meta_key": "renders/j/color/color_src/meta.json",
            },
            "created_at": "2026-04-18T00:00:00Z",
            "histogram": [1, 1, 1, 1] + [0] * 252,
            "step_scores_key": "renders/j/color/color_src/step_scores.raw",
            "step_count": 4,
            "step_scores_grid_n": 2,
        }

        def fake_run(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "step_scores_to_palette_raw":
                out_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--output="))
                with open(out_path, "wb") as fh:
                    fh.write(bytes([0, 11, 22, 33]))
                return MagicMock(returncode=0, stdout="", stderr="")
            self.assertEqual(exe, "score_raw_render")
            out_path = cmd[2]
            preview_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--preview="))
            with open(out_path, "wb") as fh:
                fh.write(b"jpeg")
            with open(preview_path, "wb") as fh:
                fh.write(b"png")
            return MagicMock(returncode=0, stdout=json.dumps({"file_size": 4, "preview_file_size": 3}), stderr="")

        mock_step_scores_run.side_effect = fake_run
        mock_raw_render_run.side_effect = fake_run

        with patch("handler_extract_palette_from_step_scores._load_json_key", return_value=sidecar):
            result = mod.handler(_event(), None)

        body = json.loads(result["body"])
        assoc = body["associated_palette"]
        self.assertEqual(body["resolution"], "fused")
        self.assertEqual(assoc["palette_id"], "pal_color_src")
        self.assertEqual(assoc["image_key"], "renders/j/palettes/pal_color_src/image.jpeg")
        self.assertEqual(puts["renders/j/palettes/pal_color_src/greyscale.raw"]["body"], bytes([0, 11, 22, 33]))
        palette_sidecar = json.loads(puts["renders/j/palettes/pal_color_src/greyscale.meta.json"]["body"].decode())
        self.assertEqual(palette_sidecar["version"], 2)
        self.assertEqual(palette_sidecar["artifact_family"], "palette")
        palette_meta = json.loads(puts["renders/j/palettes/pal_color_src/meta.json"]["body"].decode())
        self.assertEqual(palette_meta["view_mode"], "explicit")
        self.assertEqual(palette_meta["min_re"], "-3.5")
        self.assertEqual(palette_meta["max_re"], "1.25")
        self.assertEqual(palette_meta["min_im"], "-0.75")
        self.assertEqual(palette_meta["max_im"], "2.0")
        self.assertEqual(palette_meta["rotation"], "0.125")
        color_overlay = json.loads(puts["renders/j/color/color_src/meta.json"]["body"].decode())
        self.assertEqual(color_overlay["associated_palette_id"], "pal_color_src")
        self.assertEqual(color_overlay["associated_palette_mode"], "generated")
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "extracting", "done"])

    @patch("handler_extract_palette_from_step_scores.report_status")
    @patch("handler_extract_palette_from_step_scores.load_color_artifact_head")
    @patch("handler_extract_palette_from_step_scores.s3")
    def test_attach_generated_palette_to_derived_color_artifact(
        self,
        mock_s3,
        mock_load_head,
        mock_report,
    ):
        import handler_extract_palette_from_step_scores as mod

        selected_meta = {
            "family": "color",
            "artifact_id": "child_color",
            "color_mode": "postprocess",
            "derived_from_artifact_id": "base_color",
        }
        parent_meta = {
            "family": "color",
            "artifact_id": "base_color",
            "color_mode": "solve_score",
            "associated_palette_mode": "generated",
            "associated_palette_id": "pal_base_color",
            "associated_palette_display_name": "crowding magma",
            "associated_palette_image_key": "renders/j/palettes/pal_base_color/image.jpeg",
            "associated_palette_preview_key": "renders/j/palettes/pal_base_color/preview.png",
            "associated_palette_palette": "magma",
            "associated_palette_metric": "crowding",
            "associated_palette_score_chain": '[["crowding","0.1"]]',
            "associated_palette_raw_key": "renders/j/palettes/pal_base_color/greyscale.raw",
            "associated_palette_raw_meta_key": "renders/j/palettes/pal_base_color/greyscale.meta.json",
            "associated_palette_meta_key": "renders/j/palettes/pal_base_color/meta.json",
        }
        puts = {}

        def load_head(_s3, _bucket, _job_id, artifact_id):
            if artifact_id == "child_color":
                return {"artifact_id": artifact_id, "image_key": "renders/j/color/child_color/image.jpeg", "metadata": selected_meta}
            if artifact_id == "base_color":
                return {"artifact_id": artifact_id, "image_key": "renders/j/color/base_color/image.jpeg", "metadata": parent_meta}
            raise AssertionError(f"unexpected artifact_id {artifact_id}")

        def put_object(Bucket=None, Key=None, Body=None, ContentType=None, Metadata=None):
            puts[Key] = Body if isinstance(Body, (bytes, bytearray)) else Body.read()

        mock_load_head.side_effect = load_head
        mock_s3.put_object.side_effect = put_object

        result = mod.handler(_event(artifact_id="child_color"), None)
        body = json.loads(result["body"])

        self.assertEqual(body["resolution"], "attach_generated")
        overlay = json.loads(puts["renders/j/color/child_color/meta.json"].decode())
        self.assertEqual(overlay["associated_palette_id"], "pal_base_color")
        self.assertEqual(overlay["associated_palette_image_key"], "renders/j/palettes/pal_base_color/image.jpeg")
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "done"])
