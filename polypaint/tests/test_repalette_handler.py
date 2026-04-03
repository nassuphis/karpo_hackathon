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
        "task_id": "repalette_run_1",
        "source_palette_id": "pal_src",
        "new_palette": "tri_redgold",
    }
    payload.update(overrides)
    return payload


class _Paginator:
    def __init__(self, store):
        self.store = store

    def paginate(self, Bucket=None, Prefix=None, **kwargs):
        keys = [k for k in sorted(self.store) if k.startswith(Prefix)]
        yield {"Contents": [{"Key": key} for key in keys]}


class TestRepaletteHandler(unittest.TestCase):

    @patch("handler_repalette.report_status")
    @patch("handler_repalette.subprocess.run")
    @patch("handler_repalette.s3")
    def test_reusable_palette_copies_chunk_payload_and_writes_new_meta(self, mock_s3, mock_run, mock_report):
        import handler_repalette as mod

        source_meta = {
            "job_id": "j",
            "palette_id": "pal_src",
            "created_at": "2026-04-02T11:00:00Z",
            "display_name": "src palette",
            "metric": "crowding",
            "palette": "inferno",
            "solve_score_quantile": 0.05,
            "solve_score_omega": 4.0,
            "root_transforms": [["rotate_roots", "0.125"]],
            "degree": 7,
            "N": 2,
            "times": 3,
            "pass_count": 3,
            "base_grid_solves": 4,
            "total_solves": 6,
            "data_layout": "chunk_all_pass_v1",
            "render_reusable": True,
            "clip_lo": -1.0,
            "clip_hi": 2.0,
            "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
            "chunk_scores_prefix": "renders/j/palettes/pal_src/chunks/score_chunk_",
            "chunk_bins_prefix": "renders/j/palettes/pal_src/chunks/palette_bins_chunk_",
            "chunk_meta_prefix": "renders/j/palettes/pal_src/chunks/meta_chunk_",
        }
        source_chunk_meta = {
            "job_id": "j",
            "chunk_idx": 0,
            "step_start": 0,
            "step_count": 6,
            "metric": "crowding",
            "score_key": "renders/j/palettes/pal_src/chunks/score_chunk_0.bin",
            "palette_bins_key": "renders/j/palettes/pal_src/chunks/palette_bins_chunk_0.bin",
        }
        store = {
            "renders/j/palettes/pal_src/meta.json": json.dumps(source_meta).encode(),
            "renders/j/palettes/pal_src/chunks/meta_chunk_0.json": json.dumps(source_chunk_meta).encode(),
            "renders/j/palettes/pal_src/chunks/score_chunk_0.bin": b"\x00" * 24,
            "renders/j/palettes/pal_src/chunks/palette_bins_chunk_0.bin": bytes([1, 2, 3, 4, 9, 8]),
        }
        uploads = {}
        put_objects = {}

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key not in store:
                raise AssertionError(f"unexpected get_object key: {key}")
            data = store[key]
            return {"Body": MagicMock(read=lambda data=data: data)}

        def copy_object(Bucket=None, CopySource=None, Key=None, MetadataDirective=None):
            src_key = CopySource["Key"]
            store[Key] = store[src_key]

        def upload_fileobj(fileobj, bucket, key, ExtraArgs=None):
            uploads[key] = {
                "body": fileobj.read(),
                "extra": ExtraArgs or {},
            }

        def put_object(Bucket=None, Key=None, Body=None, ContentType=None):
            payload = Body.encode() if isinstance(Body, str) else Body
            store[Key] = payload
            put_objects[Key] = {
                "body": payload,
                "content_type": ContentType,
            }

        mock_s3.get_object.side_effect = get_object
        mock_s3.copy_object.side_effect = copy_object
        mock_s3.upload_fileobj.side_effect = upload_fileobj
        mock_s3.put_object.side_effect = put_object
        mock_s3.get_paginator.return_value = _Paginator(store)

        def fake_run(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "palette_bins_render":
                self.assertIn("--palette=tri_redgold", cmd)
                with open(cmd[2], "wb") as fh:
                    fh.write(b"\x00" * 16)
                return MagicMock(returncode=0, stdout="", stderr="")
            if exe == "raw2jpeg":
                with open(cmd[2], "wb") as fh:
                    fh.write(b"\xff\xd8\xffjpeg")
                return MagicMock(returncode=0, stdout="", stderr="")
            if cmd[0] == "/opt/bin/vipsthumbnail":
                preview_path = cmd[5].split("[", 1)[0]
                with open(preview_path, "wb") as fh:
                    fh.write(b"\x89PNGpreview")
                return MagicMock(returncode=0, stdout="", stderr="")
            raise AssertionError(f"unexpected subprocess call: {cmd}")

        mock_run.side_effect = fake_run

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins.bin")), \
             patch.object(mod, "_TMP_RAW", os.path.join(td, "image.raw")), \
             patch.object(mod, "_TMP_JPEG", os.path.join(td, "image.jpeg")), \
             patch.object(mod, "_TMP_PREVIEW", os.path.join(td, "preview.png")), \
             patch.object(mod, "_variant_id", return_value="pal_new"), \
             patch.object(mod, "_utc_now_iso", return_value="2026-04-02T12:00:00Z"):
            result = mod.handler(_event(), None)

        body = json.loads(result["body"])
        self.assertEqual(body["palette_id"], "pal_new")
        self.assertTrue(body["render_reusable"])
        self.assertIn("renders/j/palettes/pal_new/image.jpeg", uploads)
        self.assertIn("renders/j/palettes/pal_new/preview.png", uploads)
        self.assertIn("renders/j/palettes/pal_new/chunks/score_chunk_0.bin", store)
        self.assertIn("renders/j/palettes/pal_new/chunks/palette_bins_chunk_0.bin", store)

        chunk_meta = json.loads(store["renders/j/palettes/pal_new/chunks/meta_chunk_0.json"])
        self.assertEqual(chunk_meta["score_key"], "renders/j/palettes/pal_new/chunks/score_chunk_0.bin")
        self.assertEqual(chunk_meta["palette_bins_key"], "renders/j/palettes/pal_new/chunks/palette_bins_chunk_0.bin")

        meta = json.loads(put_objects["renders/j/palettes/pal_new/meta.json"]["body"])
        self.assertEqual(meta["palette_id"], "pal_new")
        self.assertEqual(meta["palette"], "tri_redgold")
        self.assertEqual(meta["derived_from_palette_id"], "pal_src")
        self.assertEqual(meta["derivation_kind"], "repalette")
        self.assertTrue(meta["render_reusable"])
        self.assertEqual(meta["data_layout"], "chunk_all_pass_v1")
        self.assertEqual(meta["chunk_bins_prefix"], "renders/j/palettes/pal_new/chunks/palette_bins_chunk_")
        self.assertEqual(meta["total_solves"], 6)

        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "copying", "copying", "rendering", "done"])
        phase_labels = [call.kwargs.get("result_data", {}).get("phase_label") for call in mock_report.call_args_list]
        self.assertIn("Copy numeric data", phase_labels)
        self.assertIn("Copy numeric data 1/1", phase_labels)

    @patch("handler_repalette.report_status")
    @patch("handler_repalette.subprocess.run")
    @patch("handler_repalette.s3")
    def test_legacy_palette_copies_pass0_payload_and_stays_non_reusable(self, mock_s3, mock_run, mock_report):
        import handler_repalette as mod

        source_meta = {
            "job_id": "j",
            "palette_id": "pal_old",
            "created_at": "2026-04-02T09:00:00Z",
            "display_name": "legacy palette",
            "metric": "proximity",
            "palette": "inferno",
            "solve_score_quantile": 0.01,
            "solve_score_omega": 1.0,
            "degree": 5,
            "N": 2,
            "times": 2,
            "palette_bins_key": "renders/j/palettes/pal_old/palette_bins.bin",
            "score_key": "renders/j/palettes/pal_old/score_proximity.bin",
            "clip_lo": -0.5,
            "clip_hi": 1.5,
        }
        store = {
            "renders/j/palettes/pal_old/meta.json": json.dumps(source_meta).encode(),
            "renders/j/palettes/pal_old/palette_bins.bin": bytes([4, 3, 2, 1]),
            "renders/j/palettes/pal_old/score_proximity.bin": b"\x00" * 16,
        }
        uploads = {}
        put_objects = {}

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key not in store:
                raise AssertionError(f"unexpected get_object key: {key}")
            data = store[key]
            return {"Body": MagicMock(read=lambda data=data: data)}

        def copy_object(Bucket=None, CopySource=None, Key=None, MetadataDirective=None):
            store[Key] = store[CopySource["Key"]]

        def upload_fileobj(fileobj, bucket, key, ExtraArgs=None):
            uploads[key] = fileobj.read()

        def put_object(Bucket=None, Key=None, Body=None, ContentType=None):
            payload = Body.encode() if isinstance(Body, str) else Body
            store[Key] = payload
            put_objects[Key] = payload

        mock_s3.get_object.side_effect = get_object
        mock_s3.copy_object.side_effect = copy_object
        mock_s3.upload_fileobj.side_effect = upload_fileobj
        mock_s3.put_object.side_effect = put_object

        def fake_run(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "palette_bins_render":
                self.assertIn("--palette=reef", cmd)
                with open(cmd[2], "wb") as fh:
                    fh.write(b"\x00" * 16)
                return MagicMock(returncode=0, stdout="", stderr="")
            if exe == "raw2jpeg":
                with open(cmd[2], "wb") as fh:
                    fh.write(b"\xff\xd8\xffjpeg")
                return MagicMock(returncode=0, stdout="", stderr="")
            if cmd[0] == "/opt/bin/vipsthumbnail":
                preview_path = cmd[5].split("[", 1)[0]
                with open(preview_path, "wb") as fh:
                    fh.write(b"\x89PNGpreview")
                return MagicMock(returncode=0, stdout="", stderr="")
            raise AssertionError(f"unexpected subprocess call: {cmd}")

        mock_run.side_effect = fake_run

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins.bin")), \
             patch.object(mod, "_TMP_RAW", os.path.join(td, "image.raw")), \
             patch.object(mod, "_TMP_JPEG", os.path.join(td, "image.jpeg")), \
             patch.object(mod, "_TMP_PREVIEW", os.path.join(td, "preview.png")), \
             patch.object(mod, "_variant_id", return_value="pal_new"), \
             patch.object(mod, "_utc_now_iso", return_value="2026-04-02T12:30:00Z"):
            result = mod.handler(_event(source_palette_id="pal_old", new_palette="reef"), None)

        body = json.loads(result["body"])
        self.assertFalse(body["render_reusable"])
        self.assertIn("renders/j/palettes/pal_new/palette_bins.bin", store)
        self.assertIn("renders/j/palettes/pal_new/score_proximity.bin", store)
        self.assertIn("renders/j/palettes/pal_new/image.jpeg", uploads)
        self.assertIn("renders/j/palettes/pal_new/preview.png", uploads)

        meta = json.loads(put_objects["renders/j/palettes/pal_new/meta.json"])
        self.assertFalse(meta["render_reusable"])
        self.assertEqual(meta["data_layout"], "legacy_pass0_v1")
        self.assertEqual(meta["palette_bins_key"], "renders/j/palettes/pal_new/palette_bins.bin")
        self.assertEqual(meta["score_key"], "renders/j/palettes/pal_new/score_proximity.bin")
        self.assertEqual(meta["derived_from_palette_id"], "pal_old")
        self.assertEqual(meta["palette"], "reef")


if __name__ == "__main__":
    unittest.main()
