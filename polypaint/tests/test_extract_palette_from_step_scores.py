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
    class _S3NotFound(Exception):
        response = {"Error": {"Code": "404"}}

    def test_palette_raw_allows_zero_for_explicit_scalar_emit(self):
        import handler_extract_palette_from_step_scores as mod

        self.assertFalse(mod._palette_raw_allows_zero({"score_program": "m0"}, 1))
        self.assertTrue(mod._palette_raw_allows_zero({"score_program": "m0;emit"}, 1))
        self.assertTrue(mod._palette_raw_allows_zero({"score_program": "m0;emit_norm"}, 1))
        self.assertTrue(mod._palette_raw_allows_zero({"score_program": "m0"}, 3))

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

        def put_object(Bucket=None, Key=None, Body=None, ContentType=None, Metadata=None, CacheControl=None):
            puts[Key] = {
                "body": Body if isinstance(Body, (bytes, bytearray)) else Body.read(),
                "content_type": ContentType,
                "metadata": Metadata,
                "cache_control": CacheControl,
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
                self.assertIn("--channels=1", cmd)
                out_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--output="))
                with open(out_path, "wb") as fh:
                    fh.write(bytes([0, 11, 22, 33]))
                return MagicMock(returncode=0, stdout="", stderr="")
            self.assertEqual(exe, "score_raw_render")
            self.assertIn("--zero_background=1", cmd)
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
        self.assertEqual(
            puts["renders/j/palettes/pal_color_src/preview.png"]["cache_control"],
            "public, max-age=31536000, immutable",
        )
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

    @patch("raw_score_render.subprocess.run")
    @patch("handler_extract_palette_from_step_scores.subprocess.run")
    @patch("handler_extract_palette_from_step_scores.s3")
    def test_fused_extract_palette_renders_from_cached_three_channel_step_scores(
        self,
        mock_s3,
        mock_step_scores_run,
        mock_raw_render_run,
    ):
        import handler_extract_palette_from_step_scores as mod

        puts = {}
        source_meta = {
            "artifact_id": "rgb_lut_color",
            "color_mode": "solve_score",
            "palette": "inferno",
            "raw_key": "renders/j/color/rgb_lut_color/greyscale.raw",
            "raw_meta_key": "renders/j/color/rgb_lut_color/greyscale.meta.json",
            "step_scores_key": "renders/j/color/rgb_lut_color/step_scores.raw",
            "raw_channels": "3",
            "score_output_channel_count": "3",
            "score_output_interpretation": "rgb_lut",
        }
        sidecar = {
            "version": 3,
            "job_id": "j",
            "run_id": "run_src",
            "artifact_family": "color",
            "artifact_id": "rgb_lut_color",
            "width": 2,
            "height": 2,
            "channels": 3,
            "raw_layout": "u8_packed_channels_row_major",
            "interpretation": "rgb_lut",
            "encoding": {
                "type": "u8_packed_channels_v1",
                "channels": 3,
                "background_byte": 0,
                "row_major": True,
            },
            "chain_fingerprint": "fp_src",
            "score_chain": [["metric", "proximity", "slv", "0.1"], ["emit", "norm"]],
            "score_program": "m0-0;emit_norm;m0-0;emit_norm;m0-0;emit_norm",
            "clip_slots": [{"slot": 0, "metric": "proximity", "source": "slv", "clip_lo": 0.1, "clip_hi": 0.9}],
            "background_color": [0, 0, 0],
            "plan_params_digest": "sha256:plan_src",
            "render_execution": {"raster_engine": "mt"},
            "keys": {
                "raw_key": "renders/j/color/rgb_lut_color/greyscale.raw",
                "image_key": "renders/j/color/rgb_lut_color/image.jpeg",
                "preview_key": "renders/j/color/rgb_lut_color/preview.png",
                "meta_key": "renders/j/color/rgb_lut_color/meta.json",
            },
            "created_at": "2026-04-18T00:00:00Z",
            "histogram": [1, 0, 0, 0] + [0] * 252,
            "output_channels": [
                {"channel": 0, "name": "r_lookup"},
                {"channel": 1, "name": "g_lookup"},
                {"channel": 2, "name": "b_lookup"},
            ],
            "step_scores_key": "renders/j/color/rgb_lut_color/step_scores.raw",
            "step_count": 4,
            "step_scores_grid_n": 2,
        }

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/color/rgb_lut_color/step_scores.raw":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [bytes(range(12))])}
            raise AssertionError(f"unexpected get_object key: {key}")

        def put_object(Bucket=None, Key=None, Body=None, ContentType=None, Metadata=None, CacheControl=None):
            puts[Key] = {
                "body": Body if isinstance(Body, (bytes, bytearray)) else Body.read(),
                "content_type": ContentType,
                "metadata": Metadata,
                "cache_control": CacheControl,
            }

        def fake_run(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "step_scores_to_palette_raw":
                self.assertIn("--channels=3", cmd)
                out_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--output="))
                with open(out_path, "wb") as fh:
                    fh.write(bytes(range(12)))
                return MagicMock(returncode=0, stdout="", stderr="")
            self.assertEqual(exe, "score_raw_render")
            self.assertIn("--channels=3", cmd)
            self.assertIn("--interpretation=rgb_lut", cmd)
            self.assertIn("--zero_background=0", cmd)
            out_path = cmd[2]
            preview_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--preview="))
            with open(out_path, "wb") as fh:
                fh.write(b"jpeg")
            with open(preview_path, "wb") as fh:
                fh.write(b"png")
            return MagicMock(returncode=0, stdout=json.dumps({"file_size": 4, "preview_file_size": 3}), stderr="")

        mock_s3.get_object.side_effect = get_object
        mock_s3.put_object.side_effect = put_object
        mock_step_scores_run.side_effect = fake_run
        mock_raw_render_run.side_effect = fake_run

        with patch("handler_extract_palette_from_step_scores._load_json_key", return_value=sidecar):
            result = mod._render_palette_from_step_scores("j", "rgb_lut_color", source_meta, "task")

        self.assertEqual(result["color_interpretation"], "rgb_lut")
        self.assertEqual(puts["renders/j/palettes/pal_rgb_lut_color/greyscale.raw"]["body"], bytes(range(12)))
        palette_sidecar = json.loads(puts["renders/j/palettes/pal_rgb_lut_color/greyscale.meta.json"]["body"].decode())
        self.assertEqual(palette_sidecar["channels"], 3)
        self.assertEqual(palette_sidecar["interpretation"], "rgb_lut")
        palette_meta = json.loads(puts["renders/j/palettes/pal_rgb_lut_color/meta.json"]["body"].decode())
        self.assertEqual(palette_meta["raw_channels"], 3)
        self.assertEqual(palette_meta["color_interpretation"], "rgb_lut")

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

    @patch("handler_extract_palette_from_step_scores.report_status")
    @patch("handler_extract_palette_from_step_scores.load_color_artifact_head")
    @patch("handler_extract_palette_from_step_scores.s3")
    def test_rejects_stale_associated_multi_output_without_palette_object(
        self,
        mock_s3,
        mock_load_head,
        mock_report,
    ):
        import handler_extract_palette_from_step_scores as mod

        mock_load_head.return_value = {
            "artifact_id": "hsv_lut_color",
            "image_key": "renders/j/color/hsv_lut_color/image.jpeg",
            "metadata": {
                "family": "color",
                "artifact_id": "hsv_lut_color",
                "color_mode": "solve_score",
                "color_interpretation": "hsv_lut",
                "score_output_channel_count": "3",
                "raw_channels": "3",
                "associated_palette_mode": "generated",
                "associated_palette_id": "pal_hsv_lut_color",
                "associated_palette_image_key": "renders/j/palettes/pal_hsv_lut_color/image.jpeg",
            },
        }
        mock_s3.head_object.side_effect = self._S3NotFound()

        with self.assertRaisesRegex(RuntimeError, r"associated_palette_id=pal_hsv_lut_color.*missing"):
            mod.handler(_event(artifact_id="hsv_lut_color"), None)

        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "error"])

    @patch("handler_extract_palette_from_step_scores.report_status")
    @patch("handler_extract_palette_from_step_scores.load_color_artifact_head")
    @patch("handler_extract_palette_from_step_scores._render_palette_from_step_scores")
    @patch("handler_extract_palette_from_step_scores.s3")
    def test_multi_output_solve_score_with_step_scores_uses_fused_extract(
        self,
        mock_s3,
        mock_render_palette,
        mock_load_head,
        mock_report,
    ):
        import handler_extract_palette_from_step_scores as mod

        mock_load_head.return_value = {
            "artifact_id": "rgb_color",
            "image_key": "renders/j/color/rgb_color/image.jpeg",
            "metadata": {
                "family": "color",
                "artifact_id": "rgb_color",
                "color_mode": "solve_score",
                "color_interpretation": "rgb",
                "score_output_channel_count": "3",
                "raw_channels": "3",
                "raw_key": "renders/j/color/rgb_color/color.raw",
                "raw_meta_key": "renders/j/color/rgb_color/color.meta.json",
                "step_scores_key": "renders/j/color/rgb_color/step_scores.raw",
            },
        }
        mock_render_palette.return_value = {
            "palette_id": "pal_rgb_color",
            "display_name": "pal rgb",
            "palette": "inferno",
            "metric": "proximity",
            "score_chain": [],
            "quantile": "",
            "omega": "",
            "omega_enabled": "",
            "image_key": "renders/j/palettes/pal_rgb_color/image.jpeg",
            "preview_key": "renders/j/palettes/pal_rgb_color/preview.png",
            "raw_key": "renders/j/palettes/pal_rgb_color/greyscale.raw",
            "raw_meta_key": "renders/j/palettes/pal_rgb_color/greyscale.meta.json",
            "meta_key": "renders/j/palettes/pal_rgb_color/meta.json",
            "color_interpretation": "rgb",
            "file_size": 4,
        }

        result = mod.handler(_event(artifact_id="rgb_color"), None)
        body = json.loads(result["body"])

        self.assertEqual(body["resolution"], "fused")
        self.assertEqual(body["associated_palette"]["palette_id"], "pal_rgb_color")
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "extracting", "done"])

    @patch("handler_extract_palette_from_step_scores.report_status")
    @patch("handler_extract_palette_from_step_scores.load_color_artifact_head")
    def test_rejects_multi_output_solve_score_ancestor_without_associated_palette(
        self,
        mock_load_head,
        mock_report,
    ):
        import handler_extract_palette_from_step_scores as mod

        child_meta = {
            "family": "color",
            "artifact_id": "child_rgb_color",
            "color_mode": "postprocess",
            "derived_from_artifact_id": "rgb_parent",
        }
        parent_meta = {
            "family": "color",
            "artifact_id": "rgb_parent",
            "color_mode": "solve_score",
            "score_output_interpretation": "rgb_lut",
            "score_output_channel_count": "3",
        }

        def load_head(_s3, _bucket, _job_id, artifact_id):
            if artifact_id == "child_rgb_color":
                return {"artifact_id": artifact_id, "image_key": "renders/j/color/child_rgb_color/image.jpeg", "metadata": child_meta}
            if artifact_id == "rgb_parent":
                return {"artifact_id": artifact_id, "image_key": "renders/j/color/rgb_parent/image.jpeg", "metadata": parent_meta}
            raise AssertionError(f"unexpected artifact_id {artifact_id}")

        mock_load_head.side_effect = load_head

        with self.assertRaisesRegex(RuntimeError, r"resolves to rgb_parent.*rgb_lut"):
            mod.handler(_event(artifact_id="child_rgb_color"), None)

        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "error"])
