"""
Tests for handler_palette_finalize.py.

Validates pass-0 image assembly from all-pass chunk-local bins and confirms new
reusable palette artifacts keep chunk-local numeric data instead of uploading
monolithic sidecars.
"""
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
        "task_id": "palette_run_finalize",
        "palette_id": "pal_1",
        "N": 4,
        "times": 2,
        "degree": 5,
        "metric": "crowding",
        "palette": "reef",
        "solve_score_quantile": 0.01,
        "solve_score_omega": 3.0,
        "solve_score_omega_enabled": True,
        "root_transforms": [["rotate_roots", "0.25"]],
        "image_key": "renders/j/palettes/pal_1/image.jpeg",
        "preview_key": "renders/j/palettes/pal_1/preview.png",
        "meta_key": "renders/j/palettes/pal_1/meta.json",
        "chunks_prefix": "renders/j/palettes/pal_1/chunks/",
        "chunk_scores_prefix": "renders/j/palettes/pal_1/chunks/score_chunk_",
        "chunk_bins_prefix": "renders/j/palettes/pal_1/chunks/palette_bins_chunk_",
        "chunk_meta_prefix": "renders/j/palettes/pal_1/chunks/meta_chunk_",
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
    @patch("handler_palette_finalize.load_color_artifact_head")
    def test_finalize_assembles_pass0_image_and_preserves_chunk_local_data(
        self, mock_load_head, mock_run, mock_s3, mock_list_keys, mock_delete_keys, mock_report
    ):
        import handler_palette_finalize as mod

        mock_load_head.return_value = {
            "artifact_id": "color_src",
            "image_key": "renders/j/color/color_src/image.jpeg",
            "metadata": {
                "view_mode": "explicit",
                "min_re": "-3.5",
                "max_re": "1.25",
                "min_im": "-0.75",
                "max_im": "2.0",
                "rotation": "0.125",
            },
        }

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins_full.bin")), \
             patch.object(mod, "_TMP_RAW", os.path.join(td, "palette.raw")), \
             patch.object(mod, "_TMP_JPEG", os.path.join(td, "palette.jpeg")), \
             patch.object(mod, "_TMP_PREVIEW", os.path.join(td, "palette_preview.png")):

            chunk_meta_prefix = "renders/j/palettes/pal_1/chunks/meta_chunk_"
            solve_prefix = "renders/j/palettes/pal_1/solve_score/"
            chunk_meta_keys = [
                "renders/j/palettes/pal_1/chunks/meta_chunk_0.json",
                "renders/j/palettes/pal_1/chunks/meta_chunk_1.json",
                "renders/j/palettes/pal_1/chunks/meta_chunk_2.json",
            ]
            solve_keys = [
                "renders/j/palettes/pal_1/solve_score/chunk_0_hist.json",
                "renders/j/palettes/pal_1/solve_score/chunk_1_hist.json",
            ]

            def list_keys(prefix):
                if prefix == chunk_meta_prefix:
                    return list(chunk_meta_keys)
                if prefix == solve_prefix:
                    return list(solve_keys)
                return []

            mock_list_keys.side_effect = list_keys

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
                "step_count": 14,
                "score_key": "renders/j/palettes/pal_1/chunks/score_chunk_1.bin",
                "palette_bins_key": "renders/j/palettes/pal_1/chunks/palette_bins_chunk_1.bin",
            }
            meta2 = {
                "chunk_idx": 2,
                "step_start": 20,
                "step_count": 12,
                "score_key": "renders/j/palettes/pal_1/chunks/score_chunk_2.bin",
                "palette_bins_key": "renders/j/palettes/pal_1/chunks/palette_bins_chunk_2.bin",
            }
            bins0 = bytes([0, 1, 2, 3, 4, 5])
            # first 10 values complete pass0 (g=6..15), remaining 4 belong to pass1 and must be ignored for image assembly
            bins1 = bytes([6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 8, 8, 8, 8])
            bins_meta = {
                "clip_lo": -1.0,
                "clip_hi": 2.0,
                "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
                "omega": 3.0,
            }
            clip_meta = {"clip_fallback": False, "clip_fallback_reason": None}
            requested_bins = []

            def get_object(**kwargs):
                key = kwargs["Key"]
                mapping = {
                    "renders/j/palettes/pal_1/chunks/meta_chunk_0.json": json.dumps(meta0).encode(),
                    "renders/j/palettes/pal_1/chunks/meta_chunk_1.json": json.dumps(meta1).encode(),
                    "renders/j/palettes/pal_1/chunks/meta_chunk_2.json": json.dumps(meta2).encode(),
                    "renders/j/palettes/pal_1/chunks/palette_bins_chunk_0.bin": bins0,
                    "renders/j/palettes/pal_1/chunks/palette_bins_chunk_1.bin": bins1,
                    "renders/j/palettes/pal_1/solve_score/crowding_bins.json": json.dumps(bins_meta).encode(),
                    "renders/j/palettes/pal_1/solve_score/crowding_clip.json": json.dumps(clip_meta).encode(),
                }
                if key not in mapping:
                    raise AssertionError(f"unexpected get_object key: {key}")
                data = mapping[key]
                if "palette_bins_chunk_" in key:
                    requested_bins.append((key, kwargs.get("Range")))
                range_header = kwargs.get("Range")
                if range_header:
                    prefix = "bytes=0-"
                    if not range_header.startswith(prefix):
                        raise AssertionError(f"unexpected range header: {range_header}")
                    data = data[:int(range_header[len(prefix):]) + 1]
                return {"Body": MagicMock(read=lambda data=data: data)}

            mock_s3.get_object.side_effect = get_object
            uploads = {}

            def upload_fileobj(fileobj, bucket, key, ExtraArgs=None):
                uploads[key] = {
                    "body": fileobj.read(),
                    "extra": ExtraArgs or {},
                }

            mock_s3.upload_fileobj.side_effect = upload_fileobj

            def run_side_effect(cmd, capture_output, text, timeout, env=None):
                exe = os.path.basename(cmd[0])
                if exe == "palette_bins_render":
                    with open(mod._TMP_BINS, "rb") as bf:
                        self.assertEqual(
                            bf.read(),
                            bytes([0, 1, 2, 3, 7, 6, 5, 4, 8, 9, 0, 1, 5, 4, 3, 2]),
                        )
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

            result = mod.handler(_event(
                source_color_artifact_id="color_src",
                render_execution={
                    "raster_engine": "mt",
                    "raster_mt_threads": 6,
                    "solve_score_threads": 3,
                    "solve_score_hist_input_mode": "sectioned",
                    "solve_score_hist_retries": 4,
                    "raster_input_mode": "sectioned",
                    "raster_sectioned_retries": 5,
                    "raster_bin_group_size": 2,
                    "solve_score_merge_workers": 12,
                    "finalize_workers": 18,
                    "save_associated_palette": True,
                    "palette_chunk_threads": 7,
                    "palette_chunk_input_mode": "tmpfile",
                    "palette_chunk_retries": 6,
                    "palette_chunk_workers": 22,
                },
            ), None)
            body = json.loads(result["body"])

            self.assertEqual(body["palette_id"], "pal_1")
            self.assertEqual(body["pass0_chunks_read"], 2)
            self.assertEqual(body["pass0_chunks_skipped"], 1)
            self.assertEqual(body["pass0_chunk_count"], 3)
            self.assertEqual(body["pass0_bytes_read"], 16)
            self.assertIn(("renders/j/palettes/pal_1/chunks/palette_bins_chunk_0.bin", None), requested_bins)
            self.assertIn(("renders/j/palettes/pal_1/chunks/palette_bins_chunk_1.bin", "bytes=0-9"), requested_bins)
            self.assertFalse(
                any(key == "renders/j/palettes/pal_1/chunks/palette_bins_chunk_2.bin" for key, _ in requested_bins)
            )
            self.assertEqual(body["image_key"], "renders/j/palettes/pal_1/image.jpeg")
            self.assertIn("renders/j/palettes/pal_1/image.jpeg", uploads)
            self.assertIn("renders/j/palettes/pal_1/preview.png", uploads)
            self.assertNotIn("renders/j/palettes/pal_1/score_crowding.bin", uploads)
            self.assertNotIn("renders/j/palettes/pal_1/palette_bins.bin", uploads)
            image_meta = uploads["renders/j/palettes/pal_1/image.jpeg"]["extra"]["Metadata"]
            self.assertEqual(image_meta["palette"], "reef")
            self.assertEqual(image_meta["pix"], "4")
            self.assertEqual(image_meta["width"], "4")
            self.assertEqual(image_meta["view_mode"], "explicit")
            self.assertEqual(image_meta["min_re"], "-3.5")
            self.assertEqual(image_meta["max_re"], "1.25")
            self.assertEqual(image_meta["min_im"], "-0.75")
            self.assertEqual(image_meta["max_im"], "2.0")
            self.assertEqual(image_meta["rotation"], "0.125")
            self.assertNotIn("render_execution", image_meta)
            self.assertNotIn("solve_score_chain", image_meta)
            self.assertNotIn("solve_metric", image_meta)

            meta_call = mock_s3.put_object.call_args.kwargs
            meta = json.loads(meta_call["Body"])
            self.assertEqual(meta["palette_id"], "pal_1")
            self.assertEqual(meta["using_pass"], 0)
            self.assertEqual(meta["image_pass"], 0)
            self.assertEqual(meta["data_layout"], "chunk_all_pass_v1")
            self.assertTrue(meta["render_reusable"])
            self.assertEqual(meta["base_grid_solves"], 16)
            self.assertEqual(meta["total_solves"], 32)
            self.assertEqual(meta["section_scores_prefix"], "renders/j/palettes/pal_1/chunks/score_chunk_")
            self.assertEqual(meta["section_bins_prefix"], "renders/j/palettes/pal_1/chunks/palette_bins_chunk_")
            self.assertEqual(meta["section_meta_prefix"], "renders/j/palettes/pal_1/chunks/meta_chunk_")
            self.assertEqual(meta["chunk_scores_prefix"], meta["section_scores_prefix"])
            self.assertEqual(meta["chunk_bins_prefix"], meta["section_bins_prefix"])
            self.assertEqual(meta["chunk_meta_prefix"], meta["section_meta_prefix"])
            self.assertEqual(meta["derived_from_color_artifact_id"], "color_src")
            self.assertEqual(meta["derivation_kind"], "extract_palette")
            self.assertEqual(meta["view_mode"], "explicit")
            self.assertEqual(meta["min_re"], "-3.5")
            self.assertEqual(meta["max_re"], "1.25")
            self.assertEqual(meta["min_im"], "-0.75")
            self.assertEqual(meta["max_im"], "2.0")
            self.assertEqual(meta["rotation"], "0.125")
            self.assertEqual(meta["render_execution"]["raster_engine"], "mt")
            self.assertEqual(meta["render_execution"]["palette_chunk_workers"], 22)
            self.assertNotIn("score_key", meta)
            self.assertNotIn("palette_bins_key", meta)

            deleted_batches = [c.args[0] for c in mock_delete_keys.call_args_list]
            flat_deleted = [key for batch in deleted_batches for key in batch]
            self.assertIn("renders/j/palettes/pal_1/solve_score/crowding_bins.json", flat_deleted)
            self.assertIn("renders/j/palettes/pal_1/solve_score/chunk_0_hist.json", flat_deleted)
            self.assertNotIn("renders/j/palettes/pal_1/chunks/meta_chunk_0.json", flat_deleted)

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
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins_full.bin")), \
             patch.object(mod, "_TMP_RAW", os.path.join(td, "palette.raw")), \
             patch.object(mod, "_TMP_JPEG", os.path.join(td, "palette.jpeg")), \
             patch.object(mod, "_TMP_PREVIEW", os.path.join(td, "palette_preview.png")):

            mock_list_keys.side_effect = lambda prefix: (
                ["renders/j/palettes/pal_1/chunks/meta_chunk_0.json"]
                if prefix.endswith("meta_chunk_")
                else []
            )
            meta0 = {
                "chunk_idx": 0,
                "step_start": 0,
                "step_count": 8,
                "palette_bins_key": "renders/j/palettes/pal_1/chunks/palette_bins_chunk_0.bin",
            }

            def get_object(**kwargs):
                key = kwargs["Key"]
                mapping = {
                    "renders/j/palettes/pal_1/chunks/meta_chunk_0.json": json.dumps(meta0).encode(),
                    "renders/j/palettes/pal_1/chunks/palette_bins_chunk_0.bin": bytes(range(8)),
                }
                if key not in mapping:
                    raise AssertionError(f"unexpected get_object key: {key}")
                return {"Body": MagicMock(read=lambda data=mapping[key]: data)}

            mock_s3.get_object.side_effect = get_object

            with self.assertRaises(RuntimeError) as ctx:
                mod.handler(_event(times=1), None)

            self.assertIn("filled 8 samples, expected 16", str(ctx.exception))
            mock_run.assert_not_called()
            self.assertEqual(mock_report.call_args_list[-1].args[2], "error")

    @patch("handler_palette_finalize.report_status")
    @patch("handler_palette_finalize._delete_keys")
    @patch("handler_palette_finalize._list_keys")
    @patch("handler_palette_finalize.s3")
    @patch("handler_palette_finalize.subprocess.run")
    def test_finalize_respects_explicit_omega_enabled_false(
        self, mock_run, mock_s3, mock_list_keys, mock_delete_keys, mock_report
    ):
        import handler_palette_finalize as mod

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins_full.bin")), \
             patch.object(mod, "_TMP_RAW", os.path.join(td, "palette.raw")), \
             patch.object(mod, "_TMP_JPEG", os.path.join(td, "palette.jpeg")), \
             patch.object(mod, "_TMP_PREVIEW", os.path.join(td, "palette_preview.png")):

            mock_list_keys.side_effect = lambda prefix: (
                ["renders/j/palettes/pal_1/chunks/meta_chunk_0.json"]
                if prefix.endswith("meta_chunk_")
                else (["renders/j/palettes/pal_1/solve_score/chunk_0_hist.json"] if prefix.endswith("solve_score/") else [])
            )

            meta0 = {
                "chunk_idx": 0,
                "step_start": 0,
                "step_count": 4,
                "palette_bins_key": "renders/j/palettes/pal_1/chunks/palette_bins_chunk_0.bin",
            }
            bins_meta = {
                "clip_lo": -1.0,
                "clip_hi": 2.0,
                "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
                "omega": 3.0,
                "omega_enabled": False,
            }
            clip_meta = {"clip_fallback": False, "clip_fallback_reason": None}

            def get_object(**kwargs):
                key = kwargs["Key"]
                mapping = {
                    "renders/j/palettes/pal_1/chunks/meta_chunk_0.json": json.dumps(meta0).encode(),
                    "renders/j/palettes/pal_1/chunks/palette_bins_chunk_0.bin": bytes([0, 1, 2, 3]),
                    "renders/j/palettes/pal_1/solve_score/crowding_bins.json": json.dumps(bins_meta).encode(),
                    "renders/j/palettes/pal_1/solve_score/crowding_clip.json": json.dumps(clip_meta).encode(),
                }
                if key not in mapping:
                    raise AssertionError(f"unexpected get_object key: {key}")
                return {"Body": MagicMock(read=lambda data=mapping[key]: data)}

            mock_s3.get_object.side_effect = get_object
            mock_s3.upload_fileobj.side_effect = lambda *args, **kwargs: None

            def run_side_effect(cmd, capture_output, text, timeout, env=None):
                exe = os.path.basename(cmd[0])
                if exe == "palette_bins_render":
                    with open(mod._TMP_RAW, "wb") as f:
                        f.write((2).to_bytes(4, "little"))
                        f.write((2).to_bytes(4, "little"))
                        f.write((3).to_bytes(4, "little"))
                        f.write(b"\x11\x22\x33" * 4)
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

            result = mod.handler(_event(N=2, times=1, solve_score_omega_enabled=False), None)
            body = json.loads(result["body"])
            self.assertEqual(body["palette_id"], "pal_1")
            meta_put = json.loads(mock_s3.put_object.call_args.kwargs["Body"])
            self.assertFalse(meta_put["solve_score_omega_enabled"])

    @patch("handler_palette_finalize.report_status")
    @patch("handler_palette_finalize._delete_keys")
    @patch("handler_palette_finalize._list_keys")
    @patch("handler_palette_finalize.s3")
    @patch("handler_palette_finalize.subprocess.run")
    def test_finalize_does_not_warn_when_omega_enabled_missing_and_chain_defaults_apply(
        self, mock_run, mock_s3, mock_list_keys, mock_delete_keys, mock_report
    ):
        import handler_palette_finalize as mod

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins_full.bin")), \
             patch.object(mod, "_TMP_RAW", os.path.join(td, "palette.raw")), \
             patch.object(mod, "_TMP_JPEG", os.path.join(td, "palette.jpeg")), \
             patch.object(mod, "_TMP_PREVIEW", os.path.join(td, "palette_preview.png")):

            mock_list_keys.side_effect = lambda prefix: (
                ["renders/j/palettes/pal_1/chunks/meta_chunk_0.json"]
                if prefix.endswith("meta_chunk_")
                else (["renders/j/palettes/pal_1/solve_score/chunk_0_hist.json"] if prefix.endswith("solve_score/") else [])
            )

            meta0 = {
                "chunk_idx": 0,
                "step_start": 0,
                "step_count": 4,
                "palette_bins_key": "renders/j/palettes/pal_1/chunks/palette_bins_chunk_0.bin",
            }
            bins_meta = {
                "clip_lo": -1.0,
                "clip_hi": 2.0,
                "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
                "omega": 3.0,
                "omega_enabled": True,
            }
            clip_meta = {"clip_fallback": False, "clip_fallback_reason": None}

            def get_object(**kwargs):
                key = kwargs["Key"]
                mapping = {
                    "renders/j/palettes/pal_1/chunks/meta_chunk_0.json": json.dumps(meta0).encode(),
                    "renders/j/palettes/pal_1/chunks/palette_bins_chunk_0.bin": bytes([0, 1, 2, 3]),
                    "renders/j/palettes/pal_1/solve_score/crowding_bins.json": json.dumps(bins_meta).encode(),
                    "renders/j/palettes/pal_1/solve_score/crowding_clip.json": json.dumps(clip_meta).encode(),
                }
                if key not in mapping:
                    raise AssertionError(f"unexpected get_object key: {key}")
                return {"Body": MagicMock(read=lambda data=mapping[key]: data)}

            mock_s3.get_object.side_effect = get_object
            mock_s3.upload_fileobj.side_effect = lambda *args, **kwargs: None

            def run_side_effect(cmd, capture_output, text, timeout, env=None):
                exe = os.path.basename(cmd[0])
                if exe == "palette_bins_render":
                    with open(mod._TMP_RAW, "wb") as f:
                        f.write((2).to_bytes(4, "little"))
                        f.write((2).to_bytes(4, "little"))
                        f.write((3).to_bytes(4, "little"))
                        f.write(b"\x11\x22\x33" * 4)
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

            event = _event(N=2, times=1)
            event.pop("solve_score_omega_enabled")
            mod.handler(event, None)

            done_kwargs = mock_report.call_args_list[-1].kwargs
            warned = {w["param"] for w in done_kwargs["result_data"]["contract_warnings"]}
            self.assertNotIn("solve_score_omega_enabled", warned)

    @patch("handler_palette_finalize.report_status")
    @patch("handler_palette_finalize._delete_keys")
    @patch("handler_palette_finalize._list_keys")
    @patch("handler_palette_finalize.s3")
    @patch("handler_palette_finalize.subprocess.run")
    def test_finalize_skips_solve_score_cleanup_when_disabled(
        self, mock_run, mock_s3, mock_list_keys, mock_delete_keys, mock_report
    ):
        import handler_palette_finalize as mod

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins_full.bin")), \
             patch.object(mod, "_TMP_RAW", os.path.join(td, "palette.raw")), \
             patch.object(mod, "_TMP_JPEG", os.path.join(td, "palette.jpeg")), \
             patch.object(mod, "_TMP_PREVIEW", os.path.join(td, "palette_preview.png")):

            mock_list_keys.side_effect = lambda prefix: (
                ["renders/j/palettes/pal_1/chunks/meta_chunk_0.json"]
                if prefix.endswith("meta_chunk_")
                else ["renders/j/palettes/pal_1/solve_score/chunk_0_hist.json"]
            )
            meta0 = {
                "chunk_idx": 0,
                "step_start": 0,
                "step_count": 4,
                "palette_bins_key": "renders/j/palettes/pal_1/chunks/palette_bins_chunk_0.bin",
            }
            bins_meta = {
                "clip_lo": -1.0,
                "clip_hi": 2.0,
                "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
                "omega": 3.0,
                "omega_enabled": True,
            }
            clip_meta = {"clip_fallback": False, "clip_fallback_reason": None}

            def get_object(**kwargs):
                key = kwargs["Key"]
                mapping = {
                    "renders/j/palettes/pal_1/chunks/meta_chunk_0.json": json.dumps(meta0).encode(),
                    "renders/j/palettes/pal_1/chunks/palette_bins_chunk_0.bin": bytes([0, 1, 2, 3]),
                    "renders/j/palettes/pal_1/solve_score/crowding_bins.json": json.dumps(bins_meta).encode(),
                    "renders/j/palettes/pal_1/solve_score/crowding_clip.json": json.dumps(clip_meta).encode(),
                }
                if key not in mapping:
                    raise AssertionError(f"unexpected get_object key: {key}")
                return {"Body": MagicMock(read=lambda data=mapping[key]: data)}

            mock_s3.get_object.side_effect = get_object
            mock_s3.upload_fileobj.side_effect = lambda *args, **kwargs: None

            def run_side_effect(cmd, capture_output, text, timeout, env=None):
                exe = os.path.basename(cmd[0])
                if exe == "palette_bins_render":
                    with open(mod._TMP_RAW, "wb") as f:
                        f.write((2).to_bytes(4, "little"))
                        f.write((2).to_bytes(4, "little"))
                        f.write((3).to_bytes(4, "little"))
                        f.write(b"\x11\x22\x33" * 4)
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

            mod.handler(_event(N=2, times=1, cleanup_solve_score_scratch=False), None)
            mock_delete_keys.assert_not_called()


if __name__ == "__main__":
    unittest.main()
