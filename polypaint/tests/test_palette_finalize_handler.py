"""
Tests for handler_palette_finalize.py.

Validates exact serpentine deshuffle assembly and failure on incomplete coverage.
"""
import json
import os
import sys
import tempfile
import unittest
from array import array
from unittest.mock import MagicMock, patch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(**overrides):
    payload = {
        "job_id": "j",
        "task_id": "palette_run_finalize",
        "palette_id": "pal_1",
        "N": 4,
        "times": 1,
        "degree": 5,
        "metric": "crowding",
        "palette": "reef",
        "solve_score_quantile": 0.01,
        "root_transforms": [["rotate_roots", "0.25"]],
        "image_key": "renders/j/palettes/pal_1/image.jpeg",
        "preview_key": "renders/j/palettes/pal_1/preview.png",
        "score_key": "renders/j/palettes/pal_1/score_crowding.bin",
        "palette_bins_key": "renders/j/palettes/pal_1/palette_bins.bin",
        "meta_key": "renders/j/palettes/pal_1/meta.json",
        "chunks_prefix": "renders/j/palettes/pal_1/chunks/",
        "solve_score_prefix": "renders/j/palettes/pal_1/solve_score/",
        "solve_score_clip_key": "renders/j/palettes/pal_1/solve_score/crowding_clip.json",
        "solve_score_bins_key": "renders/j/palettes/pal_1/solve_score/crowding_bins.json",
    }
    payload.update(overrides)
    return payload


class TestPaletteFinalizeHandler(unittest.TestCase):

    @patch("handler_palette_finalize.report_status")
    @patch("handler_palette_finalize._delete_keys")
    @patch("handler_palette_finalize._list_keys")
    @patch("handler_palette_finalize.s3")
    @patch("handler_palette_finalize.subprocess.run")
    def test_finalize_assembles_full_grids_and_uploads_artifacts(
        self, mock_run, mock_s3, mock_list_keys, mock_delete_keys, mock_report
    ):
        import handler_palette_finalize as mod

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_SCORES", os.path.join(td, "scores_full.bin")), \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins_full.bin")), \
             patch.object(mod, "_TMP_RAW", os.path.join(td, "palette.raw")), \
             patch.object(mod, "_TMP_JPEG", os.path.join(td, "palette.jpeg")), \
             patch.object(mod, "_TMP_PREVIEW", os.path.join(td, "palette_preview.png")):

            chunk_keys = [
                "renders/j/palettes/pal_1/chunks/meta_chunk_0.json",
                "renders/j/palettes/pal_1/chunks/score_chunk_0.bin",
                "renders/j/palettes/pal_1/chunks/palette_bins_chunk_0.bin",
                "renders/j/palettes/pal_1/chunks/meta_chunk_1.json",
                "renders/j/palettes/pal_1/chunks/score_chunk_1.bin",
                "renders/j/palettes/pal_1/chunks/palette_bins_chunk_1.bin",
            ]
            solve_keys = [
                "renders/j/palettes/pal_1/solve_score/chunk_0_hist.json",
                "renders/j/palettes/pal_1/solve_score/chunk_1_hist.json",
            ]

            def list_keys(prefix):
                if prefix == "renders/j/palettes/pal_1/chunks/":
                    return list(chunk_keys)
                if prefix == "renders/j/palettes/pal_1/solve_score/":
                    return list(solve_keys)
                return []

            mock_list_keys.side_effect = list_keys

            scores0 = array("f", [0.5, 1.5, 2.5, 3.5, 4.5, 5.5])
            bins0 = bytes([0, 1, 2, 3, 4, 5])
            scores1 = array("f", [6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5, 13.5, 14.5, 15.5])
            bins1 = bytes([6, 7, 8, 9, 0, 1, 2, 3, 4, 5])

            meta0 = {
                "chunk_idx": 0,
                "step_start": 0,
                "step_count": 6,
                "score_key": "renders/j/palettes/pal_1/chunks/score_chunk_0.bin",
                "palette_bins_key": "renders/j/palettes/pal_1/chunks/palette_bins_chunk_0.bin",
            }
            meta1 = {
                "chunk_idx": 1,
                "step_start": 6,
                "step_count": 10,
                "score_key": "renders/j/palettes/pal_1/chunks/score_chunk_1.bin",
                "palette_bins_key": "renders/j/palettes/pal_1/chunks/palette_bins_chunk_1.bin",
            }
            bins_meta = {
                "clip_lo": -1.0,
                "clip_hi": 2.0,
                "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
            }
            clip_meta = {"clip_fallback": False, "clip_fallback_reason": None}

            def get_object(**kwargs):
                key = kwargs["Key"]
                mapping = {
                    "renders/j/palettes/pal_1/chunks/meta_chunk_0.json": json.dumps(meta0).encode(),
                    "renders/j/palettes/pal_1/chunks/meta_chunk_1.json": json.dumps(meta1).encode(),
                    "renders/j/palettes/pal_1/chunks/score_chunk_0.bin": scores0.tobytes(),
                    "renders/j/palettes/pal_1/chunks/score_chunk_1.bin": scores1.tobytes(),
                    "renders/j/palettes/pal_1/chunks/palette_bins_chunk_0.bin": bins0,
                    "renders/j/palettes/pal_1/chunks/palette_bins_chunk_1.bin": bins1,
                    "renders/j/palettes/pal_1/solve_score/crowding_bins.json": json.dumps(bins_meta).encode(),
                    "renders/j/palettes/pal_1/solve_score/crowding_clip.json": json.dumps(clip_meta).encode(),
                }
                if key not in mapping:
                    raise AssertionError(f"unexpected get_object key: {key}")
                return {"Body": MagicMock(read=lambda data=mapping[key]: data)}

            mock_s3.get_object.side_effect = get_object
            uploads = {}

            def upload_fileobj(fileobj, bucket, key, ExtraArgs=None):
                uploads[key] = fileobj.read()

            mock_s3.upload_fileobj.side_effect = upload_fileobj

            def run_side_effect(cmd, capture_output, text, timeout, env=None):
                exe = os.path.basename(cmd[0])
                if exe == "palette_bins_render":
                    with open(mod._TMP_RAW, "wb") as f:
                        f.write((4).to_bytes(4, "little"))
                        f.write((4).to_bytes(4, "little"))
                        f.write((3).to_bytes(4, "little"))
                        f.write(b"\x11\x22\x33" * 16)
                    return MagicMock(returncode=0, stdout="{}", stderr="")
                if exe == "raw2jpeg":
                    with open(mod._TMP_JPEG, "wb") as f:
                        f.write(b"\xff\xd8testjpeg")
                    return MagicMock(returncode=0, stdout="", stderr="")
                if exe == "vipsthumbnail":
                    with open(mod._TMP_PREVIEW, "wb") as f:
                        f.write(b"\x89PNGpreview")
                    return MagicMock(returncode=0, stdout="", stderr="")
                raise AssertionError(f"unexpected subprocess: {cmd}")

            mock_run.side_effect = run_side_effect

            result = mod.handler(_event(), None)
            body = json.loads(result["body"])

            self.assertEqual(body["palette_bins_key"], "renders/j/palettes/pal_1/palette_bins.bin")
            self.assertEqual(body["score_key"], "renders/j/palettes/pal_1/score_crowding.bin")
            self.assertIn("renders/j/palettes/pal_1/image.jpeg", uploads)
            self.assertIn("renders/j/palettes/pal_1/preview.png", uploads)
            self.assertIn("renders/j/palettes/pal_1/score_crowding.bin", uploads)
            self.assertIn("renders/j/palettes/pal_1/palette_bins.bin", uploads)

            final_scores = array("f")
            final_scores.frombytes(uploads["renders/j/palettes/pal_1/score_crowding.bin"])
            self.assertEqual(
                list(final_scores),
                [0.5, 1.5, 2.5, 3.5, 7.5, 6.5, 5.5, 4.5, 8.5, 9.5, 10.5, 11.5, 15.5, 14.5, 13.5, 12.5],
            )
            self.assertEqual(
                list(uploads["renders/j/palettes/pal_1/palette_bins.bin"]),
                [0, 1, 2, 3, 7, 6, 5, 4, 8, 9, 0, 1, 5, 4, 3, 2],
            )

            meta_call = mock_s3.put_object.call_args.kwargs
            meta = json.loads(meta_call["Body"])
            self.assertEqual(meta["palette_id"], "pal_1")
            self.assertEqual(meta["using_pass"], 0)
            self.assertEqual(meta["clip_lo"], -1.0)
            self.assertEqual(meta["clip_hi"], 2.0)
            self.assertEqual(len(meta["cuts_norm"]), 9)
            self.assertEqual(meta["image_key"], "renders/j/palettes/pal_1/image.jpeg")

            deleted_batches = [c.args[0] for c in mock_delete_keys.call_args_list]
            self.assertTrue(any("renders/j/palettes/pal_1/chunks/meta_chunk_0.json" in batch for batch in deleted_batches))
            self.assertTrue(any("renders/j/palettes/pal_1/solve_score/crowding_bins.json" in batch for batch in deleted_batches))

            statuses = [c.args[2] for c in mock_report.call_args_list]
            self.assertEqual(statuses, ["started", "assembled", "done"])

    @patch("handler_palette_finalize.report_status")
    @patch("handler_palette_finalize._delete_keys")
    @patch("handler_palette_finalize._list_keys")
    @patch("handler_palette_finalize.s3")
    @patch("handler_palette_finalize.subprocess.run")
    def test_finalize_raises_on_incomplete_pass0_coverage(
        self, mock_run, mock_s3, mock_list_keys, mock_delete_keys, mock_report
    ):
        import handler_palette_finalize as mod

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_SCORES", os.path.join(td, "scores_full.bin")), \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins_full.bin")), \
             patch.object(mod, "_TMP_RAW", os.path.join(td, "palette.raw")), \
             patch.object(mod, "_TMP_JPEG", os.path.join(td, "palette.jpeg")), \
             patch.object(mod, "_TMP_PREVIEW", os.path.join(td, "palette_preview.png")):

            mock_list_keys.side_effect = lambda prefix: (
                ["renders/j/palettes/pal_1/chunks/meta_chunk_0.json"] if prefix.endswith("/chunks/")
                else []
            )
            meta0 = {
                "chunk_idx": 0,
                "step_start": 0,
                "step_count": 8,
                "score_key": "renders/j/palettes/pal_1/chunks/score_chunk_0.bin",
                "palette_bins_key": "renders/j/palettes/pal_1/chunks/palette_bins_chunk_0.bin",
            }
            scores0 = array("f", [float(i) for i in range(8)])
            bins0 = bytes(range(8))

            def get_object(**kwargs):
                key = kwargs["Key"]
                mapping = {
                    "renders/j/palettes/pal_1/chunks/meta_chunk_0.json": json.dumps(meta0).encode(),
                    "renders/j/palettes/pal_1/chunks/score_chunk_0.bin": scores0.tobytes(),
                    "renders/j/palettes/pal_1/chunks/palette_bins_chunk_0.bin": bins0,
                }
                if key not in mapping:
                    raise AssertionError(f"unexpected get_object key: {key}")
                return {"Body": MagicMock(read=lambda data=mapping[key]: data)}

            mock_s3.get_object.side_effect = get_object

            with self.assertRaises(RuntimeError) as ctx:
                mod.handler(_event(), None)

            self.assertIn("filled 8 samples, expected 16", str(ctx.exception))
            mock_run.assert_not_called()
            self.assertEqual(mock_report.call_args_list[-1].args[2], "error")


if __name__ == "__main__":
    unittest.main()
