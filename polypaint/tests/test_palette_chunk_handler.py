"""
Tests for handler_palette_chunk.py.

Validates chunk scoring/bin upload and failure handling without invoking the real binary.
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
        "task_id": "palette_run_chunk_3",
        "chunk_idx": 3,
        "bin_key": "renders/j/chunk_3.bin",
        "degree": 5,
        "metric": "crowding",
        "solve_score_quantile": 0.01,
        "solve_score_bins_key": "renders/j/palettes/p1/solve_score/crowding_bins.json",
        "step_start": 10,
        "step_count": 4,
        "root_transforms": [["rotate_roots", "0.25"]],
        "score_key": "renders/j/palettes/p1/chunks/score_chunk_3.bin",
        "palette_bins_key": "renders/j/palettes/p1/chunks/palette_bins_chunk_3.bin",
        "meta_key": "renders/j/palettes/p1/chunks/meta_chunk_3.json",
    }
    payload.update(overrides)
    return payload


class TestPaletteChunkHandler(unittest.TestCase):

    @patch("handler_palette_chunk.report_status")
    @patch("handler_palette_chunk.s3")
    @patch("handler_palette_chunk.subprocess.run")
    def test_success_uploads_score_bins_and_meta(self, mock_run, mock_s3, mock_report):
        import handler_palette_chunk as mod

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_INPUT", os.path.join(td, "input.bin")), \
             patch.object(mod, "_TMP_SCORES", os.path.join(td, "scores.bin")), \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins.bin")), \
             patch.object(mod, "_TMP_XFORMS", os.path.join(td, "xforms.json")):

            chunk_bytes = b"\x00" * (5 * 2 * 4 * 4)
            bins_meta = {
                "family": "solve_score",
                "metric": "crowding",
                "clip_quantile": 0.01,
                "clip_lo": -1.5,
                "clip_hi": 2.5,
                "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
            }

            def get_object(**kwargs):
                key = kwargs["Key"]
                if key == "renders/j/chunk_3.bin":
                    body = MagicMock()
                    body.iter_chunks.return_value = [chunk_bytes]
                    return {"Body": body}
                if key == "renders/j/palettes/p1/solve_score/crowding_bins.json":
                    return {"Body": MagicMock(read=lambda: json.dumps(bins_meta).encode())}
                raise AssertionError(f"unexpected get_object key: {key}")

            mock_s3.get_object.side_effect = get_object
            uploads = {}

            def upload_fileobj(fileobj, bucket, key, ExtraArgs=None):
                uploads[key] = fileobj.read()

            mock_s3.upload_fileobj.side_effect = upload_fileobj

            def run_side_effect(cmd, capture_output, text, timeout):
                self.assertIn("--degree=5", cmd)
                self.assertIn("--metric=crowding", cmd)
                self.assertIn("--step_count=4", cmd)
                self.assertTrue(any(a.startswith("--cuts=") for a in cmd))
                self.assertTrue(any(a.startswith("--root_xforms=") for a in cmd))

                scores = array("f", [1.25, 2.25, 3.25, 4.25])
                with open(mod._TMP_SCORES, "wb") as f:
                    scores.tofile(f)
                with open(mod._TMP_BINS, "wb") as f:
                    f.write(bytes([1, 3, 5, 7]))
                return MagicMock(
                    returncode=0,
                    stdout=json.dumps({"min_score": 1.25, "max_score": 4.25}),
                    stderr="",
                )

            mock_run.side_effect = run_side_effect

            result = mod.handler(_event(), None)
            body = json.loads(result["body"])

            self.assertEqual(body["chunk_idx"], 3)
            self.assertEqual(body["step_start"], 10)
            self.assertEqual(body["step_count"], 4)
            self.assertIn("renders/j/palettes/p1/chunks/score_chunk_3.bin", uploads)
            self.assertIn("renders/j/palettes/p1/chunks/palette_bins_chunk_3.bin", uploads)
            self.assertEqual(len(uploads["renders/j/palettes/p1/chunks/score_chunk_3.bin"]), 16)
            self.assertEqual(uploads["renders/j/palettes/p1/chunks/palette_bins_chunk_3.bin"], bytes([1, 3, 5, 7]))

            meta_call = mock_s3.put_object.call_args.kwargs
            meta = json.loads(meta_call["Body"])
            self.assertEqual(meta["chunk_idx"], 3)
            self.assertEqual(meta["step_start"], 10)
            self.assertEqual(meta["step_count"], 4)
            self.assertEqual(meta["metric"], "crowding")
            self.assertEqual(meta["min_score"], 1.25)
            self.assertEqual(meta["max_score"], 4.25)

            statuses = [c.args[2] for c in mock_report.call_args_list]
            self.assertEqual(statuses, ["started", "bin_downloaded", "computed", "done"])

    @patch("handler_palette_chunk.report_status")
    @patch("handler_palette_chunk.s3")
    @patch("handler_palette_chunk.subprocess.run")
    def test_bins_quantile_mismatch_raises_and_reports_error(self, mock_run, mock_s3, mock_report):
        import handler_palette_chunk as mod

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_INPUT", os.path.join(td, "input.bin")), \
             patch.object(mod, "_TMP_SCORES", os.path.join(td, "scores.bin")), \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins.bin")), \
             patch.object(mod, "_TMP_XFORMS", os.path.join(td, "xforms.json")):

            chunk_bytes = b"\x00" * 32
            bins_meta = {
                "family": "solve_score",
                "metric": "crowding",
                "clip_quantile": 0.02,
                "clip_lo": -1.5,
                "clip_hi": 2.5,
                "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
            }

            def get_object(**kwargs):
                key = kwargs["Key"]
                if key == "renders/j/chunk_3.bin":
                    body = MagicMock()
                    body.iter_chunks.return_value = [chunk_bytes]
                    return {"Body": body}
                if key == "renders/j/palettes/p1/solve_score/crowding_bins.json":
                    return {"Body": MagicMock(read=lambda: json.dumps(bins_meta).encode())}
                raise AssertionError(f"unexpected get_object key: {key}")

            mock_s3.get_object.side_effect = get_object

            with self.assertRaises(RuntimeError) as ctx:
                mod.handler(_event(), None)

            self.assertIn("quantile mismatch", str(ctx.exception))
            mock_run.assert_not_called()
            self.assertEqual(mock_report.call_args_list[-1].args[2], "error")


if __name__ == "__main__":
    unittest.main()
