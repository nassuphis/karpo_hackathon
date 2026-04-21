import json
import os
import sys
import tempfile
import unittest
from unittest.mock import MagicMock, patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(**overrides):
    payload = {
        "job_id": "j",
        "task_id": "recolor_from_raw_run_1",
        "artifact_id": "color_new",
        "source_artifact_id": "color_src",
        "source_image_key": "renders/j/color/color_src/image.jpeg",
        "new_palette": "tri_redgold",
    }
    payload.update(overrides)
    return {"body": json.dumps(payload)}


class TestRecolorFromRaw(unittest.TestCase):
    @patch("color_recolor_raw.report_status")
    @patch("color_recolor_raw.histogram_from_raw_path")
    @patch("raw_score_render.subprocess.run")
    @patch("color_recolor_raw.load_color_artifact_head")
    @patch("color_recolor_raw.s3")
    def test_recolor_from_raw_reuses_sidecar_and_never_falls_back_to_pixel_bins(
        self, mock_s3, mock_load_head, mock_raw_render_run, mock_histogram, mock_report
    ):
        import handler_recolor_from_raw as mod

        source_meta = {
            "family": "color",
            "artifact_id": "color_src",
            "created_at": "2026-04-03T09:00:00Z",
            "format": "jpeg",
            "quality": "91",
            "width": "2",
            "height": "2",
            "pix": "2",
            "tile_size": "2",
            "view_mode": "explicit",
            "min_re": "-3.5",
            "max_re": "1.25",
            "min_im": "-0.75",
            "max_im": "2.0",
            "rotation": "0.125",
            "color_mode": "solve_score",
            "palette": "inferno",
            "repalette_capable": "false",
            "raw_key": "renders/j/color/color_src/greyscale.raw",
            "raw_meta_key": "renders/j/color/color_src/greyscale.meta.json",
            "background_color": "000000",
        }
        puts = {}
        copies = []
        deleted = []

        mock_load_head.return_value = {
            "artifact_id": "color_src",
            "image_key": "renders/j/color/color_src/image.jpeg",
            "metadata": source_meta,
        }

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/color/color_src/greyscale.meta.json":
                return {"Body": MagicMock(read=lambda: json.dumps({
                    "version": 2,
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
                    "score_chain": [["proximity", "0.1"]],
                    "score_program": "m0",
                    "clip_slots": [{"slot": 0, "metric": "proximity", "source": "slv", "clip_lo": 0.1, "clip_hi": 0.9}],
                    "background_color": [0, 0, 0],
                    "plan_params_digest": "sha256:plan_src",
                    "render_execution": {"color_pipeline": "fused", "raster_engine": "mt"},
                    "keys": {
                        "raw_key": "renders/j/color/color_src/greyscale.raw",
                        "image_key": "renders/j/color/color_src/image.jpeg",
                        "preview_key": "renders/j/color/color_src/preview.png",
                        "meta_key": "renders/j/color/color_src/meta.json",
                    },
                    "created_at": "2026-04-03T09:00:00Z",
                    "histogram": [1, 1, 1, 1] + [0] * 252,
                }).encode())}
            if key == "renders/j/color/color_src/greyscale.raw":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [bytes([0, 1, 128, 255])])}
            raise AssertionError(f"unexpected get_object key: {key}")

        def put_object(Bucket=None, Key=None, Body=None, ContentType=None, Metadata=None):
            puts[Key] = {
                "body": Body if isinstance(Body, (bytes, bytearray)) else Body.read(),
                "content_type": ContentType,
                "metadata": Metadata,
            }

        def copy_object(Bucket=None, CopySource=None, Key=None):
            copies.append({"Bucket": Bucket, "CopySource": CopySource, "Key": Key})

        def delete_objects(Bucket=None, Delete=None):
            deleted.extend(obj["Key"] for obj in Delete["Objects"])

        mock_s3.get_object.side_effect = get_object
        mock_s3.put_object.side_effect = put_object
        mock_s3.copy_object.side_effect = copy_object
        mock_s3.delete_objects.side_effect = delete_objects
        mock_histogram.side_effect = AssertionError("histogram scan should be skipped when sidecar carries histogram")

        def fake_run(cmd, capture_output=False, text=False, timeout=None, env=None):
            self.assertEqual(os.path.basename(cmd[0]), "score_raw_render")
            out_path = cmd[2]
            preview_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--preview="))
            with open(out_path, "wb") as fh:
                fh.write(b"jpeg")
            with open(preview_path, "wb") as fh:
                fh.write(b"png")
            return MagicMock(returncode=0, stdout=json.dumps({"file_size": 4, "preview_file_size": 3}), stderr="")

        mock_raw_render_run.side_effect = fake_run

        with tempfile.TemporaryDirectory(), patch("color_recolor_raw.S3_USER_METADATA_LIMIT_BYTES", 4096), patch("color_recolor_raw._utc_now_iso", return_value="2026-04-03T10:00:00Z"):
            result = mod.handler(_event(), None)

        body = json.loads(result["body"])
        self.assertEqual(body["artifact_id"], "color_new")
        self.assertEqual(body["raw_key"], "renders/j/color/color_new/greyscale.raw")
        self.assertEqual(body["raw_meta_key"], "renders/j/color/color_new/greyscale.meta.json")
        self.assertNotIn("renders/j/color/color_new/pixel_bins/tile_0000.bin", puts)
        self.assertEqual(puts["renders/j/color/color_new/image.jpeg"]["body"], b"jpeg")
        self.assertEqual(puts["renders/j/color/color_new/preview.png"]["body"], b"png")
        raw_sidecar = json.loads(puts["renders/j/color/color_new/greyscale.meta.json"]["body"].decode())
        self.assertEqual(raw_sidecar["version"], 2)
        self.assertEqual(raw_sidecar["artifact_family"], "color")
        self.assertEqual(raw_sidecar["artifact_id"], "color_new")
        self.assertEqual(raw_sidecar["chain_fingerprint"], "fp_src")
        self.assertEqual(raw_sidecar["keys"]["raw_key"], "renders/j/color/color_new/greyscale.raw")
        self.assertEqual(raw_sidecar["keys"]["image_key"], "renders/j/color/color_new/image.jpeg")
        self.assertEqual(raw_sidecar["histogram"], [1, 1, 1, 1] + [0] * 252)
        color_overlay = json.loads(puts["renders/j/color/color_new/meta.json"]["body"].decode())
        self.assertEqual(color_overlay["view_mode"], "explicit")
        self.assertEqual(color_overlay["min_re"], "-3.5")
        self.assertEqual(color_overlay["max_re"], "1.25")
        self.assertEqual(color_overlay["min_im"], "-0.75")
        self.assertEqual(color_overlay["max_im"], "2.0")
        self.assertEqual(color_overlay["rotation"], "0.125")
        self.assertEqual(
            copies,
            [{
                "Bucket": "polypaint",
                "CopySource": {"Bucket": "polypaint", "Key": "renders/j/color/color_src/greyscale.raw"},
                "Key": "renders/j/color/color_new/greyscale.raw",
            }],
        )
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "rendering", "encoding", "encoding", "done"])

    @patch("color_recolor_raw.report_status")
    @patch("color_recolor_raw.histogram_from_raw_path")
    @patch("raw_score_render.subprocess.run")
    @patch("color_recolor_raw.load_color_artifact_head")
    @patch("color_recolor_raw.s3")
    def test_recolor_from_raw_preserves_step_scores_sidecar_metadata(
        self, mock_s3, mock_load_head, mock_raw_render_run, mock_histogram, mock_report
    ):
        import handler_recolor_from_raw as mod

        source_meta = {
            "family": "color",
            "artifact_id": "color_src",
            "created_at": "2026-04-03T09:00:00Z",
            "format": "jpeg",
            "quality": "91",
            "width": "2",
            "height": "2",
            "pix": "2",
            "color_mode": "solve_score",
            "palette": "inferno",
            "raw_key": "renders/j/color/color_src/greyscale.raw",
            "raw_meta_key": "renders/j/color/color_src/greyscale.meta.json",
            "step_scores_key": "renders/j/color/color_src/step_scores.raw",
            "step_count": "4",
            "step_scores_grid_n": "2",
            "background_color": "000000",
        }
        puts = {}
        copies = []

        mock_load_head.return_value = {
            "artifact_id": "color_src",
            "image_key": "renders/j/color/color_src/image.jpeg",
            "metadata": source_meta,
        }

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/color/color_src/greyscale.meta.json":
                return {"Body": MagicMock(read=lambda: json.dumps({
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
                    "score_chain": [["proximity", "0.1"]],
                    "score_program": "m0",
                    "clip_slots": [{"slot": 0, "metric": "proximity", "source": "slv", "clip_lo": 0.1, "clip_hi": 0.9}],
                    "background_color": [0, 0, 0],
                    "plan_params_digest": "sha256:plan_src",
                    "render_execution": {"color_pipeline": "fused", "raster_engine": "mt"},
                    "keys": {
                        "raw_key": "renders/j/color/color_src/greyscale.raw",
                        "image_key": "renders/j/color/color_src/image.jpeg",
                        "preview_key": "renders/j/color/color_src/preview.png",
                        "meta_key": "renders/j/color/color_src/meta.json",
                    },
                    "created_at": "2026-04-03T09:00:00Z",
                    "histogram": [1, 1, 1, 1] + [0] * 252,
                    "step_scores_key": "renders/j/color/color_src/step_scores.raw",
                    "step_count": 4,
                    "step_scores_grid_n": 2,
                }).encode())}
            if key == "renders/j/color/color_src/greyscale.raw":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [bytes([0, 1, 128, 255])])}
            raise AssertionError(f"unexpected get_object key: {key}")

        def put_object(Bucket=None, Key=None, Body=None, ContentType=None, Metadata=None):
            puts[Key] = {
                "body": Body if isinstance(Body, (bytes, bytearray)) else Body.read(),
                "content_type": ContentType,
                "metadata": Metadata,
            }

        def copy_object(Bucket=None, CopySource=None, Key=None):
            copies.append({"Bucket": Bucket, "CopySource": CopySource, "Key": Key})

        mock_s3.get_object.side_effect = get_object
        mock_s3.put_object.side_effect = put_object
        mock_s3.copy_object.side_effect = copy_object
        mock_histogram.side_effect = AssertionError("histogram scan should be skipped when sidecar carries histogram")

        def fake_run(cmd, capture_output=False, text=False, timeout=None, env=None):
            out_path = cmd[2]
            preview_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--preview="))
            with open(out_path, "wb") as fh:
                fh.write(b"jpeg")
            with open(preview_path, "wb") as fh:
                fh.write(b"png")
            return MagicMock(returncode=0, stdout=json.dumps({"file_size": 4, "preview_file_size": 3}), stderr="")

        mock_raw_render_run.side_effect = fake_run

        with tempfile.TemporaryDirectory(), patch("color_recolor_raw.S3_USER_METADATA_LIMIT_BYTES", 4096), patch("color_recolor_raw._utc_now_iso", return_value="2026-04-03T10:00:00Z"):
            result = mod.handler(_event(), None)

        body = json.loads(result["body"])
        self.assertEqual(body["artifact_id"], "color_new")
        self.assertEqual(
            copies,
            [
                {
                    "Bucket": "polypaint",
                    "CopySource": {"Bucket": "polypaint", "Key": "renders/j/color/color_src/greyscale.raw"},
                    "Key": "renders/j/color/color_new/greyscale.raw",
                },
                {
                    "Bucket": "polypaint",
                    "CopySource": {"Bucket": "polypaint", "Key": "renders/j/color/color_src/step_scores.raw"},
                    "Key": "renders/j/color/color_new/step_scores.raw",
                },
            ],
        )
        raw_sidecar = json.loads(puts["renders/j/color/color_new/greyscale.meta.json"]["body"].decode())
        self.assertEqual(raw_sidecar["version"], 3)
        self.assertEqual(raw_sidecar["step_scores_key"], "renders/j/color/color_new/step_scores.raw")
        self.assertEqual(raw_sidecar["step_count"], 4)
        self.assertEqual(raw_sidecar["step_scores_grid_n"], 2)

    @patch("color_recolor_raw.report_status")
    @patch("color_recolor_raw.histogram_from_raw_path")
    @patch("raw_score_render.subprocess.run")
    @patch("color_recolor_raw.load_color_artifact_head")
    @patch("color_recolor_raw.s3")
    def test_recolor_from_raw_regenerates_associated_palette_when_source_has_one(
        self, mock_s3, mock_load_head, mock_raw_render_run, mock_histogram, mock_report
    ):
        import handler_recolor_from_raw as mod

        source_meta = {
            "family": "color",
            "artifact_id": "color_src",
            "created_at": "2026-04-03T09:00:00Z",
            "format": "jpeg",
            "quality": "91",
            "width": "2",
            "height": "2",
            "pix": "2",
            "tile_size": "2",
            "color_mode": "solve_score",
            "palette": "inferno",
            "repalette_capable": "false",
            "raw_key": "renders/j/color/color_src/greyscale.raw",
            "raw_meta_key": "renders/j/color/color_src/greyscale.meta.json",
            "background_color": "000000",
            "associated_palette_mode": "generated",
            "associated_palette_id": "pal_color_src",
            "associated_palette_display_name": "crowding q=1.0% inferno",
            "associated_palette_palette": "inferno",
            "associated_palette_metric": "crowding",
            "associated_palette_score_chain": '[["crowding","0.1"]]',
            "associated_palette_quantile": "0.01",
            "associated_palette_omega": "4",
            "associated_palette_omega_enabled": "false",
            "associated_palette_raw_key": "renders/j/palettes/pal_color_src/greyscale.raw",
            "associated_palette_raw_meta_key": "renders/j/palettes/pal_color_src/greyscale.meta.json",
            "associated_palette_image_key": "renders/j/palettes/pal_color_src/image.jpeg",
            "associated_palette_preview_key": "renders/j/palettes/pal_color_src/preview.png",
            "associated_palette_meta_key": "renders/j/palettes/pal_color_src/meta.json",
        }
        puts = {}
        copies = []

        mock_load_head.return_value = {
            "artifact_id": "color_src",
            "image_key": "renders/j/color/color_src/image.jpeg",
            "metadata": source_meta,
        }

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/color/color_src/greyscale.meta.json":
                return {"Body": MagicMock(read=lambda: json.dumps({
                    "version": 2,
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
                    "render_execution": {"color_pipeline": "fused", "raster_engine": "mt"},
                    "keys": {
                        "raw_key": "renders/j/color/color_src/greyscale.raw",
                        "image_key": "renders/j/color/color_src/image.jpeg",
                        "preview_key": "renders/j/color/color_src/preview.png",
                        "meta_key": "renders/j/color/color_src/meta.json",
                    },
                    "created_at": "2026-04-03T09:00:00Z",
                    "histogram": [1, 1, 1, 1] + [0] * 252,
                }).encode())}
            if key == "renders/j/palettes/pal_color_src/greyscale.meta.json":
                return {"Body": MagicMock(read=lambda: json.dumps({
                    "version": 2,
                    "job_id": "j",
                    "run_id": "run_src",
                    "artifact_family": "palette",
                    "artifact_id": "pal_color_src",
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
                    "render_execution": {"color_pipeline": "fused", "raster_engine": "mt"},
                    "keys": {
                        "raw_key": "renders/j/palettes/pal_color_src/greyscale.raw",
                        "image_key": "renders/j/palettes/pal_color_src/image.jpeg",
                        "preview_key": "renders/j/palettes/pal_color_src/preview.png",
                        "meta_key": "renders/j/palettes/pal_color_src/meta.json",
                    },
                    "created_at": "2026-04-03T09:00:00Z",
                    "histogram": [1, 2, 1] + [0] * 253,
                }).encode())}
            if key == "renders/j/palettes/pal_color_src/meta.json":
                return {"Body": MagicMock(read=lambda: json.dumps({
                    "degree": 5,
                    "times": 1,
                    "using_pass": 0,
                    "image_pass": 0,
                    "base_grid_solves": 4,
                    "total_solves": 4,
                    "pass_count": 1,
                }).encode())}
            if key == "renders/j/color/color_src/greyscale.raw":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [bytes([0, 1, 128, 255])])}
            if key == "renders/j/palettes/pal_color_src/greyscale.raw":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [bytes([0, 12, 80, 255])])}
            raise AssertionError(f"unexpected get_object key: {key}")

        def put_object(Bucket=None, Key=None, Body=None, ContentType=None, Metadata=None):
            puts[Key] = {
                "body": Body if isinstance(Body, (bytes, bytearray)) else Body.read(),
                "content_type": ContentType,
                "metadata": Metadata,
            }

        def copy_object(Bucket=None, CopySource=None, Key=None):
            copies.append({"Bucket": Bucket, "CopySource": CopySource, "Key": Key})

        mock_s3.get_object.side_effect = get_object
        mock_s3.put_object.side_effect = put_object
        mock_s3.copy_object.side_effect = copy_object
        mock_histogram.side_effect = AssertionError("histogram scan should be skipped when sidecar carries histogram")

        def fake_run(cmd, capture_output=False, text=False, timeout=None, env=None):
            self.assertEqual(os.path.basename(cmd[0]), "score_raw_render")
            out_path = cmd[2]
            preview_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--preview="))
            payload = b"palette-jpeg" if out_path.endswith("color_repalette_palette_image.jpeg") else b"main-jpeg"
            with open(out_path, "wb") as fh:
                fh.write(payload)
            with open(preview_path, "wb") as fh:
                fh.write(b"png")
            return MagicMock(returncode=0, stdout=json.dumps({"file_size": len(payload), "preview_file_size": 3}), stderr="")

        mock_raw_render_run.side_effect = fake_run

        with tempfile.TemporaryDirectory(), patch("color_recolor_raw.S3_USER_METADATA_LIMIT_BYTES", 4096), patch("color_recolor_raw._utc_now_iso", return_value="2026-04-03T10:00:00Z"):
            result = mod.handler(_event(), None)

        body = json.loads(result["body"])
        self.assertEqual(body["artifact_id"], "color_new")
        self.assertIn("associated_palette", body)
        assoc = body["associated_palette"]
        self.assertEqual(assoc["palette_id"], "pal_color_new")
        self.assertEqual(assoc["image_key"], "renders/j/palettes/pal_color_new/image.jpeg")
        self.assertEqual(assoc["raw_key"], "renders/j/palettes/pal_color_new/greyscale.raw")
        color_overlay = json.loads(puts["renders/j/color/color_new/meta.json"]["body"].decode())
        self.assertEqual(color_overlay["associated_palette_mode"], "generated")
        self.assertEqual(color_overlay["associated_palette_id"], "pal_color_new")
        self.assertEqual(color_overlay["associated_palette_image_key"], "renders/j/palettes/pal_color_new/image.jpeg")
        self.assertEqual(color_overlay["associated_palette_raw_key"], "renders/j/palettes/pal_color_new/greyscale.raw")
        palette_sidecar = json.loads(puts["renders/j/palettes/pal_color_new/greyscale.meta.json"]["body"].decode())
        self.assertEqual(palette_sidecar["version"], 2)
        self.assertEqual(palette_sidecar["artifact_family"], "palette")
        self.assertEqual(palette_sidecar["artifact_id"], "pal_color_new")
        self.assertEqual(palette_sidecar["histogram"], [1, 2, 1] + [0] * 253)
        self.assertEqual(
            copies,
            [
                {
                    "Bucket": "polypaint",
                    "CopySource": {"Bucket": "polypaint", "Key": "renders/j/palettes/pal_color_src/greyscale.raw"},
                    "Key": "renders/j/palettes/pal_color_new/greyscale.raw",
                },
                {
                    "Bucket": "polypaint",
                    "CopySource": {"Bucket": "polypaint", "Key": "renders/j/color/color_src/greyscale.raw"},
                    "Key": "renders/j/color/color_new/greyscale.raw",
                },
            ],
        )
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "rendering", "encoding", "encoding", "done"])

    @patch("color_recolor_raw.report_status")
    @patch("color_recolor_raw.histogram_from_raw_path")
    @patch("raw_score_render.subprocess.run")
    @patch("color_recolor_raw.load_color_artifact_head")
    @patch("color_recolor_raw.s3")
    def test_recolor_from_raw_v1_sidecar_falls_back_to_histogram_scan_and_rewrites_v2(
        self, mock_s3, mock_load_head, mock_raw_render_run, mock_histogram, mock_report
    ):
        import handler_recolor_from_raw as mod

        source_meta = {
            "family": "color",
            "artifact_id": "color_src",
            "created_at": "2026-04-03T09:00:00Z",
            "format": "jpeg",
            "quality": "91",
            "width": "2",
            "height": "2",
            "pix": "2",
            "tile_size": "2",
            "color_mode": "solve_score",
            "palette": "inferno",
            "repalette_capable": "false",
            "raw_key": "renders/j/color/color_src/greyscale.raw",
            "raw_meta_key": "renders/j/color/color_src/greyscale.meta.json",
            "background_color": "000000",
        }
        puts = {}
        copies = []

        mock_load_head.return_value = {
            "artifact_id": "color_src",
            "image_key": "renders/j/color/color_src/image.jpeg",
            "metadata": source_meta,
        }

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/color/color_src/greyscale.meta.json":
                return {"Body": MagicMock(read=lambda: json.dumps({
                    "version": 1,
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
                    "score_chain": [["proximity", "0.1"]],
                    "score_program": "m0",
                    "clip_slots": [{"slot": 0, "metric": "proximity", "source": "slv", "clip_lo": 0.1, "clip_hi": 0.9}],
                    "background_color": [0, 0, 0],
                    "plan_params_digest": "sha256:plan_src",
                    "render_execution": {"color_pipeline": "fused", "raster_engine": "mt"},
                    "keys": {
                        "raw_key": "renders/j/color/color_src/greyscale.raw",
                        "image_key": "renders/j/color/color_src/image.jpeg",
                        "preview_key": "renders/j/color/color_src/preview.png",
                        "meta_key": "renders/j/color/color_src/meta.json",
                    },
                    "created_at": "2026-04-03T09:00:00Z",
                }).encode())}
            if key == "renders/j/color/color_src/greyscale.raw":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [bytes([0, 1, 128, 255])])}
            raise AssertionError(f"unexpected get_object key: {key}")

        def put_object(Bucket=None, Key=None, Body=None, ContentType=None, Metadata=None):
            puts[Key] = {
                "body": Body if isinstance(Body, (bytes, bytearray)) else Body.read(),
                "content_type": ContentType,
                "metadata": Metadata,
            }

        def copy_object(Bucket=None, CopySource=None, Key=None):
            copies.append({"Bucket": Bucket, "CopySource": CopySource, "Key": Key})

        mock_s3.get_object.side_effect = get_object
        mock_s3.put_object.side_effect = put_object
        mock_s3.copy_object.side_effect = copy_object
        mock_histogram.return_value = [1, 1, 1, 1] + [0] * 252

        def fake_run(cmd, capture_output=False, text=False, timeout=None, env=None):
            self.assertEqual(os.path.basename(cmd[0]), "score_raw_render")
            out_path = cmd[2]
            preview_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--preview="))
            with open(out_path, "wb") as fh:
                fh.write(b"jpeg")
            with open(preview_path, "wb") as fh:
                fh.write(b"png")
            return MagicMock(returncode=0, stdout=json.dumps({"file_size": 4, "preview_file_size": 3}), stderr="")

        mock_raw_render_run.side_effect = fake_run

        with tempfile.TemporaryDirectory(), patch("color_recolor_raw.S3_USER_METADATA_LIMIT_BYTES", 4096), patch("color_recolor_raw._utc_now_iso", return_value="2026-04-03T10:00:00Z"):
            result = mod.handler(_event(), None)

        body = json.loads(result["body"])
        self.assertEqual(body["artifact_id"], "color_new")
        mock_histogram.assert_called_once()
        self.assertEqual(
            mock_histogram.call_args.kwargs,
            {"expected_size": 4},
        )
        self.assertEqual(
            copies,
            [{
                "Bucket": "polypaint",
                "CopySource": {"Bucket": "polypaint", "Key": "renders/j/color/color_src/greyscale.raw"},
                "Key": "renders/j/color/color_new/greyscale.raw",
            }],
        )
        raw_sidecar = json.loads(puts["renders/j/color/color_new/greyscale.meta.json"]["body"].decode())
        self.assertEqual(raw_sidecar["version"], 2)
        self.assertEqual(raw_sidecar["histogram"], [1, 1, 1, 1] + [0] * 252)

    @patch("color_recolor_raw.report_status")
    @patch("color_recolor_raw.load_color_artifact_head")
    def test_recolor_from_raw_rejects_missing_sidecar(self, mock_load_head, mock_report):
        import handler_recolor_from_raw as mod

        mock_load_head.return_value = {
            "artifact_id": "color_src",
            "image_key": "renders/j/color/color_src/image.jpeg",
            "metadata": {
                "family": "color",
                "artifact_id": "color_src",
                "color_mode": "solve_score",
                "palette": "inferno",
                "repalette_capable": "true",
                "pixel_bins_prefix": "renders/j/color/color_src/pixel_bins/tile_",
            },
        }

        with self.assertRaisesRegex(RuntimeError, "requires raw_key and raw_meta_key"):
            mod.handler(_event(), None)
