import io
import importlib
import json
import os
import struct
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


def _lambda_ok(body):
    return {"Payload": io.BytesIO(json.dumps({"statusCode": 200, "body": json.dumps(body)}).encode())}


class TestColorRepaletteHandler(unittest.TestCase):

    def test_color_repalette_lambda_client_uses_extended_invoke_timeouts(self):
        with patch.dict(
            os.environ,
            {
                "LAMBDA_INVOKE_READ_TIMEOUT": "930",
                "LAMBDA_INVOKE_CONNECT_TIMEOUT": "11",
            },
            clear=False,
        ):
            import handler_color_repalette as mod

            mod = importlib.reload(mod)
            self.assertEqual(mod.lambda_client.meta.config.read_timeout, 930)
            self.assertEqual(mod.lambda_client.meta.config.connect_timeout, 11)

    @patch("handler_color_repalette.report_status")
    @patch("handler_color_repalette.subprocess.run")
    @patch("handler_color_repalette.lambda_client")
    @patch("handler_color_repalette.load_color_artifact_head")
    @patch("handler_color_repalette.s3")
    def test_color_repalette_reuses_pixel_bins_and_invokes_encode_preview(
        self, mock_s3, mock_load_head, mock_lambda, mock_run, mock_report
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
            "tile_size": "2",
            "view_mode": "explicit",
            "min_re": "-3.5",
            "max_re": "1.25",
            "min_im": "-0.75",
            "max_im": "2.0",
            "rotation": "0.125",
            "color_mode": "solve_score",
            "palette": "inferno",
            "solve_metric": "crowding",
            "solve_score_quantile": "0.01",
            "solve_score_omega": "4",
            "repalette_capable": "true",
            "pixel_bins_prefix": "renders/j/color/color_src/pixel_bins/tile_",
            "pixel_bins_empty": "255",
            "pixel_bins_layout": "tile_u8_v1",
            "background_color": "000000",
        }
        uploads = {}
        puts = {}
        deleted = []
        invocations = []

        mock_load_head.return_value = {
            "artifact_id": "color_src",
            "image_key": "renders/j/color/color_src/image.jpeg",
            "metadata": source_meta,
        }

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/color/color_src/pixel_bins/tile_0000.bin":
                return {"Body": MagicMock(read=lambda: bytes([255, 1, 2, 3]))}
            raise AssertionError(f"unexpected get_object key: {key}")

        def put_object(Bucket=None, Key=None, Body=None, ContentType=None, Metadata=None):
            puts[Key] = {
                "body": Body if isinstance(Body, (bytes, bytearray)) else Body.read(),
                "content_type": ContentType,
                "metadata": Metadata,
            }

        def upload_fileobj(fileobj, bucket, key, ExtraArgs=None):
            uploads[key] = fileobj.read()

        def delete_objects(Bucket=None, Delete=None):
            deleted.extend(obj["Key"] for obj in Delete["Objects"])

        mock_s3.get_object.side_effect = get_object
        mock_s3.put_object.side_effect = put_object
        mock_s3.upload_fileobj.side_effect = upload_fileobj
        mock_s3.delete_objects.side_effect = delete_objects

        def fake_run(cmd, capture_output=False, text=False, timeout=None, env=None):
            self.assertIn("--palette=tri_redgold", cmd)
            self.assertIn("--background_color=000000", cmd)
            out_path = cmd[2]
            with open(out_path, "wb") as fh:
                fh.write(struct.pack("<III", 2, 2, 3))
                fh.write(b"\x00" * 12)
            return MagicMock(returncode=0, stdout="{}", stderr="")

        mock_run.side_effect = fake_run

        def invoke(FunctionName=None, InvocationType=None, Payload=None):
            payload = json.loads(json.loads(Payload)["body"])
            invocations.append((FunctionName, payload))
            if FunctionName == "polypaint-encode":
                for key in ("out_key", "format", "quality", "metadata", "width", "height", "tile_grid"):
                    self.assertIn(key, payload)
                return _lambda_ok({"out_key": "renders/j/color/color_new/image.jpeg", "file_size": 1234})
            if FunctionName == "polypaint-render-preview":
                for key in ("job_id", "source_key", "preview_key", "task_id"):
                    self.assertIn(key, payload)
                return _lambda_ok({"preview_key": "renders/j/color/color_new/preview.png"})
            raise AssertionError(f"unexpected invoke function {FunctionName}")

        mock_lambda.invoke.side_effect = invoke

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "PIXEL_BINS_RENDER", "pixel_bins_render"), \
             patch.object(mod, "_utc_now_iso", return_value="2026-04-03T10:00:00Z"):
            result = mod.handler(_event(), None)

        body = json.loads(result["body"])
        self.assertEqual(body["artifact_id"], "color_new")
        self.assertEqual(body["derivation_kind"], "color_repalette")
        self.assertIn("renders/j/color/color_new/pixel_bins/tile_0000.bin", puts)
        self.assertEqual(puts["renders/j/color/color_new/pixel_bins/tile_0000.bin"]["body"], bytes([255, 1, 2, 3]))
        self.assertIn("renders/j/color/color_new/_tmp/tile_0000.raw", uploads)
        self.assertIn("renders/j/color/color_new/_tmp/tile_0000.raw", deleted)

        encode_fn, encode_payload = invocations[0]
        self.assertEqual(encode_fn, "polypaint-encode")
        self.assertEqual(encode_payload["metadata"]["palette"], "tri_redgold")
        self.assertEqual(encode_payload["metadata"]["repalette_capable"], "true")
        self.assertEqual(encode_payload["metadata"]["pixel_bins_prefix"], "renders/j/color/color_new/pixel_bins/tile_")
        self.assertEqual(encode_payload["metadata"]["derived_from_artifact_id"], "color_src")
        self.assertEqual(encode_payload["metadata"]["view_mode"], "explicit")
        self.assertEqual(encode_payload["metadata"]["min_re"], "-3.5")
        self.assertEqual(encode_payload["metadata"]["max_re"], "1.25")
        self.assertEqual(encode_payload["metadata"]["min_im"], "-0.75")
        self.assertEqual(encode_payload["metadata"]["max_im"], "2.0")
        self.assertEqual(encode_payload["metadata"]["rotation"], "0.125")

        preview_fn, preview_payload = invocations[1]
        self.assertEqual(preview_fn, "polypaint-render-preview")
        self.assertEqual(preview_payload["job_id"], "j")
        self.assertEqual(preview_payload["source_key"], "renders/j/color/color_new/image.jpeg")
        self.assertEqual(preview_payload["preview_key"], "renders/j/color/color_new/preview.png")
        self.assertEqual(preview_payload["task_id"], "color_repalette_run_1_preview")

        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "rendering", "rendering", "encoding", "preview", "done"])

    @patch("handler_color_repalette.report_status")
    @patch("color_recolor_raw.report_status")
    @patch("color_recolor_raw.histogram_from_raw_path")
    @patch("raw_score_render.subprocess.run")
    @patch("handler_color_repalette.load_color_artifact_head")
    @patch("color_recolor_raw.s3")
    def test_color_repalette_can_reuse_fused_greyscale_raw_sidecar(
        self, mock_s3, mock_load_head, mock_raw_render_run, mock_histogram, mock_report, mock_outer_report
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
            "tile_size": "2",
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
        self.assertNotIn("renders/j/color/color_new/pixel_bins/tile_0000.bin", puts)
        self.assertEqual(puts["renders/j/color/color_new/image.jpeg"]["body"], b"jpeg")
        self.assertEqual(puts["renders/j/color/color_new/preview.png"]["body"], b"png")
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

        self.assertEqual([call.args[2] for call in mock_outer_report.call_args_list], ["started"])
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["rendering", "encoding", "encoding", "done"])


if __name__ == "__main__":
    unittest.main()
