import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(**overrides):
    payload = {
        "job_id": "job1",
        "task_id": "autolevels_run_1",
        "artifact_id": "autolevels_123",
        "source_artifact_id": "color_src",
        "source_image_key": "renders/job1/color/color_src/image.jpeg",
        "autolevels_params": {
            "clip_low": 0.25,
            "clip_high": 1.5,
            "peak_factor": 0.1,
            "gamma": 1.1,
            "auto_gamma": "median",
            "target": 0.55,
            "sigmoid_strength": 0.2,
            "sigmoid_mid": 0.45,
            "vibrance": 0.05,
            "pooled_rgb": 0.02,
            "quality": 83,
            "jpeg_subsample_mode": "off",
            "jpeg_optimize_coding": True,
            "jpeg_interlace": True,
            "exclude_background": True,
        },
    }
    payload.update(overrides)
    return payload


class TestAutolevelsHandler(unittest.TestCase):

    def test_sanitize_params_clamps_background_threshold(self):
        from handler_autolevels import _sanitize_params

        self.assertEqual(_sanitize_params({"background_threshold": -5})["background_threshold"], 0)
        self.assertEqual(_sanitize_params({"background_threshold": 999})["background_threshold"], 255)
        self.assertIsNone(_sanitize_params({"background_threshold": ""})["background_threshold"])
        defaults = _sanitize_params({})
        self.assertEqual(defaults["pooled_rgb"], 0.1)
        self.assertTrue(defaults["enable_pooled_rgb"])

    @patch("handler_autolevels.report_status")
    @patch("handler_autolevels.s3")
    @patch("handler_autolevels.subprocess.run")
    def test_handler_uploads_derived_artifact_and_metadata(self, mock_run, mock_s3, mock_report):
        from handler_autolevels import handler

        mock_s3.get_object.return_value = {
            "Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"src-image-bytes"])
        }
        mock_s3.head_object.return_value = {
            "Metadata": {
                "artifact_id": "color_src",
                "family": "color",
                "created_at": "2026-04-01T10:00:00Z",
                "format": "jpeg",
                "pix": "3000",
                "quality": "77",
                "view_mode": "auto",
                "quantile": "0.02",
                "shim": "0.05",
                "rotation": "0.125",
                "root_transforms": '[["rotate_roots","0.25"]]',
                "color_mode": "solve_score",
                "solve_metric": "anisotropy",
                "solve_score_quantile": "0.02",
                "solve_score_omega": "6",
                "palette": "tri_redgold",
                "associated_palette_mode": "generated",
                "associated_palette_id": "pal_src",
                "associated_palette_display_name": "anisotropy q=2.0% w=6 tri_redgold",
                "associated_palette_image_key": "renders/job1/palettes/pal_src/image.jpeg",
                "associated_palette_preview_key": "renders/job1/palettes/pal_src/preview.png",
                "associated_palette_palette": "tri_redgold",
                "associated_palette_metric": "anisotropy",
                "associated_palette_quantile": "0.02",
                "associated_palette_omega": "6",
                "associated_palette_omega_enabled": "false",
                "match_mode": "greedy",
                "background_color": "101214",
                "background_threshold": "7",
            }
        }

        uploads = {}
        puts = {}
        puts = {}
        puts = {}
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
            if exe == "autolevels_render":
                with open(cmd[2], "wb") as fh:
                    fh.write(b"\xff\xd8\xffderived-jpeg")
                return MagicMock(
                    returncode=0,
                    stdout=json.dumps({
                        "width": 1200, "height": 1200, "bands": 3, "file_size": 15,
                        "black_bin": 12, "white_bin": 220, "black": 0.047, "white": 0.863,
                        "gamma": 1.1, "final_stretch": True, "final_lo_bin": 4, "final_hi_bin": 250,
                        "r_min_bin": 16, "r_max_bin": 210, "g_min_bin": 18, "g_max_bin": 220,
                        "b_min_bin": 20, "b_max_bin": 240, "included_pixels": 1000, "excluded_pixels": 2000
                    }),
                    stderr="",
                )
            if cmd[0] == "/opt/bin/vipsthumbnail":
                preview_path = cmd[5].split("[", 1)[0]
                with open(preview_path, "wb") as fh:
                    fh.write(b"\x89PNGpreview")
                return MagicMock(returncode=0, stdout="", stderr="")
            raise AssertionError(f"unexpected subprocess call: {cmd}")

        mock_run.side_effect = fake_run

        result = handler(_event(autolevels_params={**_event()["autolevels_params"], "background_threshold": 11}), None)
        body = json.loads(result["body"])

        self.assertEqual(result["statusCode"], 200)
        self.assertEqual(body["artifact_id"], "autolevels_123")
        self.assertEqual(body["family"], "color")

        image_key = "renders/job1/color/autolevels_123/image.jpeg"
        preview_key = "renders/job1/color/autolevels_123/preview.png"
        self.assertIn(image_key, uploads)
        self.assertIn(preview_key, uploads)

        image_meta = uploads[image_key]["extra"]["Metadata"]
        render_cmd = mock_run.call_args_list[0].args[0]
        self.assertEqual(image_meta["artifact_id"], "autolevels_123")
        self.assertEqual(image_meta["derived_from_artifact_id"], "color_src")
        self.assertEqual(image_meta["derived_from_image_key"], "renders/job1/color/color_src/image.jpeg")
        self.assertEqual(image_meta["postprocess_kind"], "autolevels")
        self.assertEqual(image_meta["postprocess_profile"], "preview_default_v1")
        self.assertEqual(image_meta["family"], "color")
        self.assertEqual(image_meta["format"], "jpeg")
        self.assertEqual(image_meta["quality"], "83")
        self.assertNotIn("color_mode", image_meta)
        self.assertNotIn("associated_palette_mode", image_meta)
        self.assertNotIn("background_color", image_meta)
        self.assertNotIn("autolevels_params", image_meta)

        meta_key = "renders/job1/color/autolevels_123/meta.json"
        self.assertIn(meta_key, puts)
        sidecar = json.loads(puts[meta_key]["body"])
        self.assertEqual(sidecar["color_mode"], "solve_score")
        self.assertEqual(sidecar["solve_metric"], "anisotropy")
        self.assertEqual(sidecar["palette"], "tri_redgold")
        self.assertEqual(sidecar["associated_palette_mode"], "generated")
        self.assertEqual(sidecar["associated_palette_id"], "pal_src")
        self.assertEqual(sidecar["associated_palette_image_key"], "renders/job1/palettes/pal_src/image.jpeg")
        self.assertEqual(sidecar["associated_palette_preview_key"], "renders/job1/palettes/pal_src/preview.png")
        self.assertEqual(sidecar["associated_palette_metric"], "anisotropy")
        self.assertEqual(sidecar["associated_palette_palette"], "tri_redgold")
        self.assertEqual(sidecar["associated_palette_quantile"], "0.02")
        self.assertEqual(sidecar["associated_palette_omega"], "6")
        self.assertEqual(sidecar["associated_palette_omega_enabled"], "false")
        self.assertEqual(sidecar["background_color"], "101214")
        self.assertEqual(sidecar["background_threshold"], "11")
        self.assertIn("autolevels_params", sidecar)
        self.assertIn("--background-color=101214", render_cmd)
        self.assertIn("--background-threshold=11", render_cmd)
        self.assertIn("--exclude-background=1", render_cmd)
        self.assertIn("--enable-levels=1", render_cmd)

        preview_extra = uploads[preview_key]["extra"]
        self.assertEqual(preview_extra["ContentType"], "image/png")
        self.assertEqual(preview_extra["Metadata"]["pix"], "1200")
        self.assertEqual(preview_extra["Metadata"]["width"], "1200")
        self.assertEqual(preview_extra["Metadata"]["height"], "1200")

        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertIn("started", statuses)
        self.assertIn("done", statuses)
        done_call = mock_report.call_args_list[-1]
        self.assertEqual(done_call.kwargs["result_data"]["autolevel_debug"]["r_min_bin"], 16)
        self.assertEqual(done_call.kwargs["result_data"]["autolevel_debug"]["background_color"], "101214")
        self.assertEqual(done_call.kwargs["result_data"]["autolevel_debug"]["background_threshold"], 11)

    @patch("handler_autolevels.report_status")
    @patch("handler_autolevels.s3")
    @patch("handler_autolevels.subprocess.run")
    def test_handler_defaults_background_metadata_to_black(self, mock_run, mock_s3, mock_report):
        from handler_autolevels import handler

        mock_s3.get_object.return_value = {
            "Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"src-image-bytes"])
        }
        mock_s3.head_object.return_value = {"Metadata": {"pix": "1200"}}

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
            if exe == "autolevels_render":
                with open(cmd[2], "wb") as fh:
                    fh.write(b"\xff\xd8\xffderived-jpeg")
                return MagicMock(
                    returncode=0,
                    stdout=json.dumps({"width": 1200, "height": 1200, "bands": 3, "file_size": 15}),
                    stderr="",
                )
            if cmd[0] == "/opt/bin/vipsthumbnail":
                preview_path = cmd[5].split("[", 1)[0]
                with open(preview_path, "wb") as fh:
                    fh.write(b"\x89PNGpreview")
                return MagicMock(returncode=0, stdout="", stderr="")
            raise AssertionError(f"unexpected subprocess call: {cmd}")

        mock_run.side_effect = fake_run

        result = handler(_event(), None)
        self.assertEqual(result["statusCode"], 200)
        render_cmd = mock_run.call_args_list[0].args[0]
        self.assertIn("--background-color=000000", render_cmd)
        self.assertIn("--background-threshold=4", render_cmd)
        self.assertIn("--exclude-background=1", render_cmd)
        image_meta = uploads["renders/job1/color/autolevels_123/image.jpeg"]["extra"]["Metadata"]
        self.assertNotIn("background_color", image_meta)
        self.assertNotIn("background_threshold", image_meta)
        sidecar = json.loads(puts["renders/job1/color/autolevels_123/meta.json"]["body"])
        self.assertEqual(sidecar["background_color"], "000000")
        self.assertEqual(sidecar["background_threshold"], "4")

    @patch("handler_autolevels.report_status")
    @patch("handler_autolevels.s3")
    @patch("handler_autolevels.subprocess.run")
    def test_handler_can_disable_background_exclusion(self, mock_run, mock_s3, mock_report):
        from handler_autolevels import handler

        mock_s3.get_object.return_value = {
            "Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"src-image-bytes"])
        }
        mock_s3.head_object.return_value = {
            "Metadata": {"pix": "1200", "background_color": "000000", "background_threshold": "4"}
        }
        mock_s3.upload_fileobj.side_effect = lambda *args, **kwargs: None

        def fake_run(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "autolevels_render":
                with open(cmd[2], "wb") as fh:
                    fh.write(b"\xff\xd8\xffderived-jpeg")
                return MagicMock(
                    returncode=0,
                    stdout=json.dumps({"width": 1200, "height": 1200, "bands": 3, "file_size": 15}),
                    stderr="",
                )
            if cmd[0] == "/opt/bin/vipsthumbnail":
                preview_path = cmd[5].split("[", 1)[0]
                with open(preview_path, "wb") as fh:
                    fh.write(b"\x89PNGpreview")
                return MagicMock(returncode=0, stdout="", stderr="")
            raise AssertionError(f"unexpected subprocess call: {cmd}")

        mock_run.side_effect = fake_run

        result = handler(_event(autolevels_params={**_event()["autolevels_params"], "exclude_background": False, "background_threshold": 9}), None)
        self.assertEqual(result["statusCode"], 200)
        render_cmd = mock_run.call_args_list[0].args[0]
        self.assertIn("--exclude-background=0", render_cmd)
        self.assertIn("--background-threshold=9", render_cmd)

    @patch("handler_autolevels.report_status")
    @patch("handler_autolevels.s3")
    @patch("handler_autolevels.subprocess.run")
    def test_handler_reports_error_when_binary_fails(self, mock_run, mock_s3, mock_report):
        from handler_autolevels import handler

        mock_s3.get_object.return_value = {
            "Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"src-image-bytes"])
        }
        mock_s3.head_object.return_value = {"Metadata": {"pix": "1200"}}
        mock_run.return_value = MagicMock(returncode=1, stdout="", stderr="bad levels")

        with self.assertRaises(RuntimeError) as ctx:
            handler(_event(), None)

        self.assertIn("autolevels_render failed", str(ctx.exception))
        last = mock_report.call_args_list[-1]
        self.assertEqual(last.args[2], "error")
        self.assertIn("bad levels", last.args[3])


if __name__ == "__main__":
    unittest.main()
