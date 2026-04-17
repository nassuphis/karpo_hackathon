import io
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
        "task_id": "recolor_from_raw_run_1",
        "artifact_id": "color_new",
        "source_artifact_id": "color_src",
        "source_image_key": "renders/j/color/color_src/image.jpeg",
        "new_palette": "tri_redgold",
    }
    payload.update(overrides)
    return {"body": json.dumps(payload)}


def _lambda_ok(body):
    return {"Payload": io.BytesIO(json.dumps({"statusCode": 200, "body": json.dumps(body)}).encode())}


class TestRecolorFromRaw(unittest.TestCase):
    @patch("handler_color_repalette.report_status")
    @patch("handler_color_repalette.subprocess.run")
    @patch("handler_color_repalette.lambda_client")
    @patch("handler_color_repalette.load_color_artifact_head")
    @patch("handler_color_repalette.s3")
    def test_recolor_from_raw_reuses_sidecar_and_never_falls_back_to_pixel_bins(
        self, mock_s3, mock_load_head, mock_lambda, mock_run, mock_report
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

        def upload_fileobj(fileobj, bucket, key, ExtraArgs=None):
            uploads[key] = fileobj.read()

        def delete_objects(Bucket=None, Delete=None):
            deleted.extend(obj["Key"] for obj in Delete["Objects"])

        mock_s3.get_object.side_effect = get_object
        mock_s3.put_object.side_effect = put_object
        mock_s3.upload_fileobj.side_effect = upload_fileobj
        mock_s3.delete_objects.side_effect = delete_objects

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            self.assertEqual(os.path.basename(cmd[0]), "pixel_bins_render")
            with open(cmd[1], "rb") as fh:
                self.assertEqual(fh.read(), bytes([255, 0, 4, 9]))
            with open(cmd[2], "wb") as fh:
                fh.write(struct.pack("<III", 2, 2, 3))
                fh.write(b"\x00" * 12)
            return MagicMock(returncode=0, stdout="{}", stderr="")

        mock_run.side_effect = fake_run

        def invoke(FunctionName=None, InvocationType=None, Payload=None):
            payload = json.loads(json.loads(Payload)["body"])
            invocations.append((FunctionName, payload))
            if FunctionName == "polypaint-encode":
                return _lambda_ok({"out_key": "renders/j/color/color_new/image.jpeg", "file_size": 4321})
            if FunctionName == "polypaint-render-preview":
                return _lambda_ok({"preview_key": "renders/j/color/color_new/preview.png"})
            raise AssertionError(f"unexpected invoke function {FunctionName}")

        mock_lambda.invoke.side_effect = invoke

        with tempfile.TemporaryDirectory(), patch("handler_color_repalette.PIXEL_BINS_RENDER", "pixel_bins_render"), patch("handler_color_repalette._utc_now_iso", return_value="2026-04-03T10:00:00Z"):
            result = mod.handler(_event(), None)

        body = json.loads(result["body"])
        self.assertEqual(body["artifact_id"], "color_new")
        self.assertEqual(body["raw_key"], "renders/j/color/color_new/greyscale.raw")
        self.assertEqual(body["raw_meta_key"], "renders/j/color/color_new/greyscale.meta.json")
        self.assertNotIn("renders/j/color/color_new/pixel_bins/tile_0000.bin", puts)
        self.assertEqual(invocations[0][0], "polypaint-encode")
        self.assertEqual(invocations[1][0], "polypaint-render-preview")
        raw_sidecar = json.loads(puts["renders/j/color/color_new/greyscale.meta.json"]["body"].decode())
        self.assertEqual(raw_sidecar["artifact_family"], "color")
        self.assertEqual(raw_sidecar["artifact_id"], "color_new")
        self.assertEqual(raw_sidecar["chain_fingerprint"], "fp_src")
        self.assertEqual(raw_sidecar["keys"]["raw_key"], "renders/j/color/color_new/greyscale.raw")
        self.assertEqual(raw_sidecar["keys"]["image_key"], "renders/j/color/color_new/image.jpeg")
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "rendering", "rendering", "encoding", "encoding", "preview", "done"])

    @patch("handler_color_repalette.report_status")
    @patch("handler_color_repalette.load_color_artifact_head")
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
