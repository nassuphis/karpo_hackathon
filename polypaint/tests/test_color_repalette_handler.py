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
        "task_id": "color_repalette_run_1",
        "artifact_id": "color_new",
        "source_artifact_id": "color_src",
        "source_image_key": "renders/j/color/color_src/image.jpeg",
        "new_palette": "tri_redgold",
    }
    payload.update(overrides)
    return payload


class TestColorRepaletteHandler(unittest.TestCase):

    @patch("color_recolor_raw.report_status")
    @patch("color_recolor_raw.histogram_from_raw_path")
    @patch("raw_score_render.subprocess.run")
    @patch("color_recolor_raw.load_color_artifact_head")
    @patch("color_recolor_raw.s3")
    def test_color_repalette_can_reuse_fused_greyscale_raw_sidecar(
        self, mock_s3, mock_load_head, mock_raw_render_run, mock_histogram, mock_report
    ):
        import handler_color_repalette as mod

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
            "solve_metric": "crowding",
            "solve_score_quantile": "0.01",
            "solve_score_omega": "4",
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
                    "created_at": "2026-04-03T09:00:00Z",
                    "histogram": [1, 1, 1, 1] + [0] * 252,
                }).encode())}
            if key == "renders/j/color/color_src/greyscale.raw":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [bytes([0, 1, 128, 255])])}
            raise AssertionError(f"unexpected get_object key: {key}")

        def put_object(Bucket=None, Key=None, Body=None, ContentType=None, Metadata=None, CacheControl=None):
            puts[Key] = {
                "body": Body if isinstance(Body, (bytes, bytearray)) else Body.read(),
                "content_type": ContentType,
                "metadata": Metadata,
                "cache_control": CacheControl,
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
            exe = os.path.basename(cmd[0])
            if exe != "score_raw_render":
                raise AssertionError(f"unexpected executable {exe}")
            self.assertIn("--palette=tri_redgold", cmd)
            self.assertIn("--background_color=000000", cmd)
            out_path = cmd[2]
            preview_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--preview="))
            with open(out_path, "wb") as fh:
                fh.write(b"jpeg")
            with open(preview_path, "wb") as fh:
                fh.write(b"png")
            return MagicMock(returncode=0, stdout=json.dumps({"file_size": 4, "preview_file_size": 3}), stderr="")

        mock_raw_render_run.side_effect = fake_run

        with tempfile.TemporaryDirectory() as td, \
             patch("color_recolor_raw.S3_USER_METADATA_LIMIT_BYTES", 4096), \
             patch("color_recolor_raw._utc_now_iso", return_value="2026-04-03T10:00:00Z"):
            result = mod.handler(_event(), None)

        body = json.loads(result["body"])
        self.assertEqual(body["artifact_id"], "color_new")
        self.assertEqual(body["raw_key"], "renders/j/color/color_new/greyscale.raw")
        self.assertEqual(body["raw_meta_key"], "renders/j/color/color_new/greyscale.meta.json")
        self.assertEqual(puts["renders/j/color/color_new/image.jpeg"]["body"], b"jpeg")
        self.assertEqual(puts["renders/j/color/color_new/preview.png"]["body"], b"png")
        self.assertEqual(
            puts["renders/j/color/color_new/preview.png"]["cache_control"],
            "public, max-age=31536000, immutable",
        )
        self.assertIn("renders/j/color/color_new/meta.json", puts)
        self.assertIn("renders/j/color/color_new/greyscale.meta.json", puts)
        raw_sidecar = json.loads(puts["renders/j/color/color_new/greyscale.meta.json"]["body"].decode())
        self.assertEqual(raw_sidecar["version"], 2)
        self.assertEqual(raw_sidecar["artifact_family"], "color")
        self.assertEqual(raw_sidecar["artifact_id"], "color_new")
        self.assertEqual(raw_sidecar["chain_fingerprint"], "fp_src")
        self.assertEqual(raw_sidecar["keys"]["raw_key"], "renders/j/color/color_new/greyscale.raw")
        self.assertEqual(raw_sidecar["keys"]["image_key"], "renders/j/color/color_new/image.jpeg")
        self.assertEqual(raw_sidecar["histogram"], [1, 1, 1, 1] + [0] * 252)
        self.assertEqual(
            copies,
            [{
                "Bucket": "polypaint",
                "CopySource": {"Bucket": "polypaint", "Key": "renders/j/color/color_src/greyscale.raw"},
                "Key": "renders/j/color/color_new/greyscale.raw",
            }],
        )
        self.assertFalse(deleted)

        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "rendering", "encoding", "encoding", "done"])

    @patch("color_recolor_raw.report_status")
    @patch("color_recolor_raw.histogram_from_raw_path")
    @patch("raw_score_render.subprocess.run")
    @patch("color_recolor_raw.load_color_artifact_head")
    @patch("color_recolor_raw.s3")
    def test_color_repalette_rerenders_three_channel_lut_raw(
        self, mock_s3, mock_load_head, mock_raw_render_run, mock_histogram, mock_report
    ):
        import handler_color_repalette as mod

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
            "color_interpretation": "rgb_lut",
            "score_output_interpretation": "rgb_lut",
            "raw_channels": "3",
            "score_output_channel_count": "3",
            "raw_key": "renders/j/color/color_src/greyscale.raw",
            "raw_meta_key": "renders/j/color/color_src/greyscale.meta.json",
            "background_color": "000000",
        }
        histogram = [1, 0, 0, 1, 0, 0, 1, 0, 0, 1] + [0] * 246
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
                    "score_chain": [["proximity", "slv", "0.1"], ["emit", "norm"]],
                    "score_program": "m0;emit_norm;m1;emit_norm;m2;emit_norm",
                    "clip_slots": [{"slot": 0, "metric": "proximity", "source": "slv", "clip_lo": 0.1, "clip_hi": 0.9}],
                    "background_color": [0, 0, 0],
                    "plan_params_digest": "sha256:plan_src",
                    "render_execution": {"raster_engine": "mt"},
                    "keys": {
                        "raw_key": "renders/j/color/color_src/greyscale.raw",
                        "image_key": "renders/j/color/color_src/image.jpeg",
                        "preview_key": "renders/j/color/color_src/preview.png",
                        "meta_key": "renders/j/color/color_src/meta.json",
                    },
                    "created_at": "2026-04-03T09:00:00Z",
                    "histogram": histogram,
                    "output_channels": [
                        {"channel": 0, "name": "r_lookup"},
                        {"channel": 1, "name": "g_lookup"},
                        {"channel": 2, "name": "b_lookup"},
                    ],
                }).encode())}
            if key == "renders/j/color/color_src/greyscale.raw":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [bytes(range(12))])}
            raise AssertionError(f"unexpected get_object key: {key}")

        def put_object(Bucket=None, Key=None, Body=None, ContentType=None, Metadata=None, CacheControl=None):
            puts[Key] = {
                "body": Body if isinstance(Body, (bytes, bytearray)) else Body.read(),
                "content_type": ContentType,
                "metadata": Metadata,
                "cache_control": CacheControl,
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
            self.assertIn("--channels=3", cmd)
            self.assertIn("--interpretation=hsv_lut", cmd)
            self.assertIn("--palette=tri_redgold", cmd)
            self.assertIn("--zero_background=1", cmd)
            self.assertFalse(any(arg.startswith("--eq_lut=") for arg in cmd))
            out_path = cmd[2]
            preview_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--preview="))
            with open(out_path, "wb") as fh:
                fh.write(b"jpeg")
            with open(preview_path, "wb") as fh:
                fh.write(b"png")
            return MagicMock(returncode=0, stdout=json.dumps({"file_size": 4, "preview_file_size": 3}), stderr="")

        mock_raw_render_run.side_effect = fake_run

        with tempfile.TemporaryDirectory(), \
             patch("color_recolor_raw.S3_USER_METADATA_LIMIT_BYTES", 4096), \
             patch("color_recolor_raw._utc_now_iso", return_value="2026-04-03T10:00:00Z"):
            result = mod.handler(_event(new_interpretation="hsv_lut"), None)

        body = json.loads(result["body"])
        self.assertEqual(body["artifact_id"], "color_new")
        self.assertEqual(body["score_output_interpretation"], "hsv_lut")
        self.assertEqual(body["raw_channels"], 3)
        raw_sidecar = json.loads(puts["renders/j/color/color_new/greyscale.meta.json"]["body"].decode())
        self.assertEqual(raw_sidecar["channels"], 3)
        self.assertEqual(raw_sidecar["raw_layout"], "u8_packed_channels_row_major")
        self.assertEqual(raw_sidecar["interpretation"], "hsv_lut")
        self.assertEqual(raw_sidecar["histogram"], histogram)
        self.assertEqual(raw_sidecar["output_channels"][0]["name"], "h_lookup")
        color_overlay = json.loads(puts["renders/j/color/color_new/meta.json"]["body"].decode())
        self.assertEqual(color_overlay["raw_channels"], "3")
        self.assertEqual(color_overlay["repalette_capable"], "true")
        self.assertEqual(color_overlay["score_output_interpretation"], "hsv_lut")
        self.assertEqual(color_overlay["color_interpretation"], "hsv_lut")
        self.assertEqual(
            [row["name"] for row in json.loads(color_overlay["score_output_channels"])],
            ["h_lookup", "s_lookup", "v_lookup"],
        )
        self.assertEqual(color_overlay["rgb_source"], "hsv_lut_raw")
        self.assertEqual(
            copies,
            [{
                "Bucket": "polypaint",
                "CopySource": {"Bucket": "polypaint", "Key": "renders/j/color/color_src/greyscale.raw"},
                "Key": "renders/j/color/color_new/greyscale.raw",
            }],
        )
        self.assertFalse(deleted)

    @patch("color_recolor_raw.report_status")
    @patch("color_recolor_raw.load_color_artifact_head")
    @patch("color_recolor_raw.s3")
    def test_color_repalette_rejects_three_channel_without_target_interpretation(
        self, mock_s3, mock_load_head, mock_report
    ):
        import handler_color_repalette as mod

        mock_load_head.return_value = {
            "artifact_id": "color_src",
            "image_key": "renders/j/color/color_src/image.jpeg",
            "metadata": {
                "family": "color",
                "artifact_id": "color_src",
                "format": "jpeg",
                "quality": "91",
                "width": "2",
                "height": "2",
                "pix": "2",
                "color_mode": "solve_score",
                "raw_key": "renders/j/color/color_src/greyscale.raw",
                "raw_meta_key": "renders/j/color/color_src/greyscale.meta.json",
                "background_color": "000000",
            },
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
                    "channels": 3,
                    "raw_layout": "u8_packed_channels_row_major",
                    "interpretation": "rgb",
                    "encoding": {
                        "type": "u8_packed_channels_v1",
                        "channels": 3,
                        "background_byte": 0,
                        "row_major": True,
                    },
                    "chain_fingerprint": "fp_src",
                    "score_chain": [["proximity", "slv", "0.1"], ["emit", "norm"]],
                    "score_program": "m0;emit_norm;m1;emit_norm;m2;emit_norm",
                    "clip_slots": [{"slot": 0, "metric": "proximity", "source": "slv", "clip_lo": 0.1, "clip_hi": 0.9}],
                    "background_color": [0, 0, 0],
                    "plan_params_digest": "sha256:plan_src",
                    "render_execution": {"raster_engine": "mt"},
                    "keys": {
                        "raw_key": "renders/j/color/color_src/greyscale.raw",
                        "image_key": "renders/j/color/color_src/image.jpeg",
                        "preview_key": "renders/j/color/color_src/preview.png",
                        "meta_key": "renders/j/color/color_src/meta.json",
                    },
                    "created_at": "2026-04-03T09:00:00Z",
                    "histogram": [3] + [1] * 9 + [0] * 246,
                }).encode())}
            raise AssertionError(f"unexpected get_object key: {key}")

        mock_s3.get_object.side_effect = get_object

        with self.assertRaisesRegex(RuntimeError, "requires new_interpretation=rgb_lut"):
            mod.handler(_event(), None)


if __name__ == "__main__":
    unittest.main()
