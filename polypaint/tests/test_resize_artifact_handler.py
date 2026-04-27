import json
import os
import struct
import sys
import unittest
from unittest.mock import MagicMock, patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(**overrides):
    payload = {
        "job_id": "job1",
        "task_id": "resize_run_1",
        "artifact_id": "resize_123",
        "source_artifact_id": "color_src",
        "source_image_key": "renders/job1/color/color_src/image.jpeg",
        "resize_params": {
            "engine": "thumbnail",
            "target_size": 2048,
            "size_mode": "down",
            "linear": True,
            "intent": "relative",
            "fail_on": "warning",
            "quality": 83,
            "jpeg_subsample_mode": "off",
            "jpeg_optimize_coding": True,
            "jpeg_interlace": True,
        },
    }
    payload.update(overrides)
    return payload


class TestResizeArtifactHandler(unittest.TestCase):

    @staticmethod
    def _fake_png_bytes(width, height):
        return (
            b"\x89PNG\r\n\x1a\n"
            + struct.pack(">I", 13)
            + b"IHDR"
            + struct.pack(">II", width, height)
            + b"\x08\x02\x00\x00\x00"
            + b"\x00\x00\x00\x00"
        )

    @staticmethod
    def _fake_jpeg_bytes(width, height):
        app0 = b"\xff\xe0" + struct.pack(">H", 16) + b"JFIF\x00\x01\x01\x00\x00\x01\x00\x01\x00\x00"
        sof0 = b"\xff\xc0" + struct.pack(">H", 17) + b"\x08" + struct.pack(">HH", height, width) + b"\x03\x01\x11\x00\x02\x11\x00\x03\x11\x00"
        return b"\xff\xd8" + app0 + sof0 + b"\xff\xd9"

    @patch("handler_resize_artifact.report_status")
    @patch("handler_resize_artifact.subprocess.run")
    @patch("handler_resize_artifact.s3")
    def test_handler_thumbnail_uploads_resize_artifact_and_metadata(self, mock_s3, mock_run, mock_report):
        from handler_resize_artifact import handler

        mock_s3.head_object.return_value = {
            "ContentLength": 1234,
            "Metadata": {
                "artifact_id": "color_src",
                "family": "color",
                "created_at": "2026-04-10T09:00:00Z",
                "format": "jpeg",
                "quality": "77",
                "width": "3000",
                "height": "3000",
                "pix": "3000",
                "view_mode": "explicit",
                "min_re": "-3.5",
                "max_re": "1.25",
                "min_im": "-0.75",
                "max_im": "2.0",
                "rotation": "0.125",
                "color_mode": "solve_score",
                "palette": "tri_redgold",
                "solve_metric": "spread",
                "associated_palette_mode": "generated",
                "associated_palette_id": "pal_resize_src",
                "associated_palette_display_name": "spread q=1.0% w=4 inferno",
                "associated_palette_image_key": "renders/job1/palettes/pal_resize_src/image.jpeg",
                "associated_palette_preview_key": "renders/job1/palettes/pal_resize_src/preview.png",
                "associated_palette_palette": "inferno",
                "associated_palette_color_interpretation": "rgb_lut",
                "associated_palette_metric": "spread",
                "associated_palette_quantile": "0.01",
                "associated_palette_omega": "4",
                "associated_palette_omega_enabled": "false",
                "associated_palette_raw_key": "renders/job1/palettes/pal_resize_src/greyscale.raw",
                "associated_palette_raw_meta_key": "renders/job1/palettes/pal_resize_src/greyscale.meta.json",
                "associated_palette_meta_key": "renders/job1/palettes/pal_resize_src/meta.json",
                "raw_key": "renders/job1/color/color_src/greyscale.raw",
                "raw_meta_key": "renders/job1/color/color_src/greyscale.meta.json",
                "raw_channels": "3",
                "raw_layout": "u8_packed_channels_row_major",
                "step_scores_key": "renders/job1/color/color_src/step_scores.raw",
                "step_count": "9000000",
                "step_scores_grid_n": "3000",
                "repalette_capable": "true",
            },
        }
        mock_s3.get_object.return_value = {
            "Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"src-image-bytes"])
        }

        uploads = {}
        puts = {}

        def upload_fileobj(fileobj, bucket, key, ExtraArgs=None):
            uploads[key] = {
                "body": fileobj.read(),
                "extra": ExtraArgs or {},
            }

        def put_object(Bucket=None, Key=None, Body=None, ContentType=None):
            puts[Key] = {
                "body": Body if isinstance(Body, (bytes, bytearray)) else Body.read(),
                "content_type": ContentType,
            }

        mock_s3.upload_fileobj.side_effect = upload_fileobj
        mock_s3.put_object.side_effect = put_object

        def fake_run(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "vips":
                self.assertEqual(cmd[1], "thumbnail")
                self.assertIn("--height", cmd)
                self.assertIn("--size", cmd)
                self.assertNotIn("--crop", cmd)
                self.assertIn("--linear", cmd)
                self.assertIn("--intent", cmd)
                self.assertIn("--fail-on", cmd)
                out_path = cmd[3].split("[", 1)[0]
                with open(out_path, "wb") as fh:
                    fh.write(self._fake_jpeg_bytes(2048, 2048))
                return MagicMock(returncode=0, stdout="", stderr="")
            if exe == "vipsthumbnail":
                out_path = cmd[5].split("[", 1)[0]
                with open(out_path, "wb") as fh:
                    fh.write(b"\x89PNGpreview")
                return MagicMock(returncode=0, stdout="", stderr="")
            raise AssertionError(f"unexpected subprocess call: {cmd}")

        mock_run.side_effect = fake_run

        result = handler(_event(), None)
        body = json.loads(result["body"])

        self.assertEqual(result["statusCode"], 200)
        self.assertEqual(body["artifact_id"], "resize_123")
        image_key = "renders/job1/color/resize_123/image.jpeg"
        preview_key = "renders/job1/color/resize_123/preview.png"
        self.assertIn(image_key, uploads)
        self.assertIn(preview_key, uploads)

        image_meta = uploads[image_key]["extra"]["Metadata"]
        self.assertEqual(image_meta["artifact_id"], "resize_123")
        self.assertEqual(image_meta["derived_from_artifact_id"], "color_src")
        self.assertEqual(image_meta["derived_from_image_key"], "renders/job1/color/color_src/image.jpeg")
        self.assertEqual(image_meta["postprocess_kind"], "resize")
        self.assertEqual(image_meta["postprocess_profile"], "libvips_resize_v1")
        self.assertEqual(image_meta["format"], "jpeg")
        self.assertEqual(image_meta["quality"], "83")
        self.assertNotIn("repalette_capable", image_meta)
        self.assertNotIn("associated_palette_mode", image_meta)
        self.assertNotIn("resize_params", image_meta)

        meta_key = "renders/job1/color/resize_123/meta.json"
        self.assertIn(meta_key, puts)
        sidecar = json.loads(puts[meta_key]["body"])
        self.assertEqual(sidecar["repalette_capable"], "false")
        self.assertEqual(sidecar["associated_palette_mode"], "generated")
        self.assertEqual(sidecar["associated_palette_id"], "pal_resize_src")
        self.assertEqual(sidecar["associated_palette_image_key"], "renders/job1/palettes/pal_resize_src/image.jpeg")
        self.assertEqual(sidecar["associated_palette_preview_key"], "renders/job1/palettes/pal_resize_src/preview.png")
        self.assertEqual(sidecar["associated_palette_metric"], "spread")
        self.assertEqual(sidecar["associated_palette_palette"], "inferno")
        self.assertEqual(sidecar["associated_palette_color_interpretation"], "rgb_lut")
        self.assertEqual(sidecar["associated_palette_raw_key"], "renders/job1/palettes/pal_resize_src/greyscale.raw")
        self.assertEqual(sidecar["associated_palette_raw_meta_key"], "renders/job1/palettes/pal_resize_src/greyscale.meta.json")
        self.assertEqual(sidecar["associated_palette_meta_key"], "renders/job1/palettes/pal_resize_src/meta.json")
        self.assertEqual(sidecar["associated_palette_quantile"], "0.01")
        self.assertEqual(sidecar["associated_palette_omega"], "4")
        self.assertEqual(sidecar["associated_palette_omega_enabled"], "false")
        self.assertEqual(sidecar["view_mode"], "explicit")
        self.assertEqual(sidecar["min_re"], "-3.5")
        self.assertEqual(sidecar["max_re"], "1.25")
        self.assertEqual(sidecar["min_im"], "-0.75")
        self.assertEqual(sidecar["max_im"], "2.0")
        self.assertEqual(sidecar["rotation"], "0.125")
        self.assertIn("resize_params", sidecar)
        resize_meta = json.loads(sidecar["resize_params"])
        self.assertEqual(resize_meta["engine"], "thumbnail")
        self.assertEqual(resize_meta["target_size"], 2048)
        self.assertNotIn("crop", resize_meta)
        self.assertNotIn("vscale", resize_meta)
        for raw_key in (
            "raw_key",
            "raw_meta_key",
            "raw_channels",
            "raw_layout",
            "step_scores_key",
            "step_count",
            "step_scores_grid_n",
        ):
            self.assertNotIn(raw_key, sidecar)

        preview_extra = uploads[preview_key]["extra"]
        self.assertEqual(preview_extra["ContentType"], "image/png")
        self.assertEqual(preview_extra["Metadata"]["pix"], "2048")
        self.assertEqual(preview_extra["Metadata"]["width"], "2048")
        self.assertEqual(preview_extra["Metadata"]["height"], "2048")

        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertIn("started", statuses)
        self.assertIn("done", statuses)
        done_call = mock_report.call_args_list[-1]
        dbg = done_call.kwargs["result_data"]["resize_debug"]
        self.assertEqual(dbg["engine"], "thumbnail")
        self.assertEqual(dbg["target_size"], 2048)
        self.assertEqual(dbg["size_mode"], "down")
        self.assertNotIn("crop", dbg)
        self.assertNotIn("vscale", dbg)
        self.assertEqual(dbg["out_dims"], "2048x2048")

    @patch("handler_resize_artifact.report_status")
    @patch("handler_resize_artifact.subprocess.run")
    @patch("handler_resize_artifact.s3")
    def test_handler_resize_engine_uses_resize_kernel_and_png_source_fallback(self, mock_s3, mock_run, mock_report):
        from handler_resize_artifact import handler

        def head_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/job1/color/color_src/image.jpeg":
                raise Exception("NoSuchKey")
            return {
                "ContentLength": 4567,
                "Metadata": {
                    "artifact_id": "color_src",
                    "family": "color",
                    "created_at": "2026-04-10T09:00:00Z",
                    "width": "4000",
                    "height": "4000",
                    "pix": "4000",
                },
            }

        mock_s3.head_object.side_effect = head_object
        mock_s3.get_object.return_value = {
            "Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"\x89PNGsrc-image-bytes"])
        }

        uploads = {}
        puts = {}

        def upload_fileobj(fileobj, bucket, key, ExtraArgs=None):
            uploads[key] = {
                "body": fileobj.read(),
                "extra": ExtraArgs or {},
            }

        def put_object(Bucket=None, Key=None, Body=None, ContentType=None):
            puts[Key] = {
                "body": Body if isinstance(Body, (bytes, bytearray)) else Body.read(),
                "content_type": ContentType,
            }

        mock_s3.upload_fileobj.side_effect = upload_fileobj
        mock_s3.put_object.side_effect = put_object

        def fake_run(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "vips":
                self.assertEqual(cmd[1], "resize")
                self.assertEqual(cmd[2], "/tmp/resize_in.png")
                self.assertIn("--kernel", cmd)
                self.assertIn("mitchell", cmd)
                self.assertIn("--gap", cmd)
                self.assertIn("3.5", cmd)
                self.assertNotIn("--vscale", cmd)
                out_path = cmd[3].split("[", 1)[0]
                with open(out_path, "wb") as fh:
                    fh.write(self._fake_png_bytes(1000, 1000))
                return MagicMock(returncode=0, stdout="", stderr="")
            if exe == "vipsthumbnail":
                out_path = cmd[5].split("[", 1)[0]
                with open(out_path, "wb") as fh:
                    fh.write(b"\x89PNGpreview")
                return MagicMock(returncode=0, stdout="", stderr="")
            raise AssertionError(f"unexpected subprocess call: {cmd}")

        mock_run.side_effect = fake_run

        result = handler(_event(
            source_image_key="renders/job1/color/color_src/image.png",
            resize_params={
                "engine": "resize",
                "target_size": 1000,
                "kernel": "mitchell",
                "gap": 3.5,
                "format": "png",
                "png_compression": 9,
                "png_Q": 95,
                "png_dither": 0.25,
                "png_bitdepth": 8,
                "png_effort": 8,
                "png_interlace": True,
                "png_palette": True,
            },
        ), None)

        self.assertEqual(result["statusCode"], 200)
        image_key = "renders/job1/color/resize_123/image.png"
        self.assertIn(image_key, uploads)
        image_meta = uploads[image_key]["extra"]["Metadata"]
        self.assertEqual(image_meta["format"], "png")
        self.assertNotIn("resize_params", image_meta)
        resize_meta = json.loads(json.loads(puts["renders/job1/color/resize_123/meta.json"]["body"])["resize_params"])
        self.assertEqual(resize_meta["engine"], "resize")
        self.assertEqual(resize_meta["kernel"], "mitchell")
        self.assertEqual(resize_meta["gap"], 3.5)
        self.assertNotIn("vscale", resize_meta)

    def test_sanitize_rejects_removed_crop_and_vscale_params(self):
        from handler_resize_artifact import _sanitize_resize_params

        source_meta = {"width": "1024", "height": "1024", "pix": "1024", "format": "png"}
        with self.assertRaisesRegex(RuntimeError, "no longer accepts crop"):
            _sanitize_resize_params({"crop": "none"}, source_meta)
        with self.assertRaisesRegex(RuntimeError, "no longer accepts vscale"):
            _sanitize_resize_params({"vscale": 1}, source_meta)


if __name__ == "__main__":
    unittest.main()
