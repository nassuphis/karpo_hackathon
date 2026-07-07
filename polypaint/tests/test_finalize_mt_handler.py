import json
import os
import pathlib
import sys
import tempfile
import time
import unittest
from unittest.mock import MagicMock, patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


TEST_JOB_ID = "test_finalize_mt_job"
TEST_ARTIFACT_ID = "artifact_fused"
TEST_IMAGE_KEY = f"renders/{TEST_JOB_ID}/color/{TEST_ARTIFACT_ID}/image.jpeg"
TEST_PREVIEW_KEY = f"renders/{TEST_JOB_ID}/color/{TEST_ARTIFACT_ID}/preview.png"
TEST_META_KEY = f"renders/{TEST_JOB_ID}/color/{TEST_ARTIFACT_ID}/meta.json"
TEST_RAW_KEY = f"renders/{TEST_JOB_ID}/color/{TEST_ARTIFACT_ID}/greyscale.raw"
TEST_RAW_META_KEY = f"renders/{TEST_JOB_ID}/color/{TEST_ARTIFACT_ID}/greyscale.meta.json"
TEST_FRAGMENT_PREFIX = f"renders/{TEST_JOB_ID}/color/{TEST_ARTIFACT_ID}/fragments/section_"
TEST_FRAGMENT_KEY = f"{TEST_FRAGMENT_PREFIX}0000.frag"


def _encode_fragment_pairs(pairs):
    payload = bytearray()
    for pixel_idx, score in pairs:
        payload.extend(int(pixel_idx).to_bytes(4, "little", signed=False))
        payload.append(int(score) & 0xFF)
    return bytes(payload)


def _event(**overrides):
    payload = {
        "phase": "finalize_mt",
        "job_id": TEST_JOB_ID,
        "run_id": "run_123",
        "task_id": "render_finalize_mt_0",
        "mode": "color",
        "source_item_count": 1,
        "pix": 2,
        "format": "jpeg",
        "quality": 90,
        "palette": "inferno",
        "background_color": "000000",
        "finalize_workers": 1,
        "image_key": TEST_IMAGE_KEY,
        "preview_key": TEST_PREVIEW_KEY,
        "meta_key": TEST_META_KEY,
        "raw_key": TEST_RAW_KEY,
        "raw_meta_key": TEST_RAW_META_KEY,
        "plan_params_digest": "sha256:plan123",
        "clip_slots": [{"slot": 0, "metric": "proximity", "source": "slv", "clip_lo": 0.1, "clip_hi": 0.9}],
        "score_program": "m0",
        "score_output_normalize": False,
        "score_output_clip_lo": 0.0,
        "score_output_clip_hi": 1.0,
        "chain_fingerprint": "fp_test",
        "fragment_prefix": TEST_FRAGMENT_PREFIX,
        "render_execution": {"raster_engine": "mt"},
        "fragment_manifest": {
            "version": 1,
            "pair_encoding": "u32le_u8_v1",
            "item_count": 1,
            "fragment_prefix": TEST_FRAGMENT_PREFIX,
            "chain_fingerprint": "fp_test",
        },
        "metadata": {
            "artifact_id": TEST_ARTIFACT_ID,
            "family": "color",
            "created_at": "2026-04-17T00:00:00Z",
            "format": "jpeg",
            "quality": "90",
            "color_mode": "solve_score",
            "palette": "inferno",
            "background_color": "000000",
            "solve_metric": "proximity",
            "solve_score_chain": '[["proximity","0.1"]]',
            "solve_score_chain_fingerprint": "fp_test",
            "score_program": "m0",
        },
    }
    payload.update(overrides)
    if "fragment_manifest" not in overrides:
        payload["fragment_manifest"] = dict(payload["fragment_manifest"])
        payload["fragment_manifest"]["item_count"] = int(payload["source_item_count"])
        payload["fragment_manifest"]["fragment_prefix"] = str(payload.get("fragment_prefix") or "")
        payload["fragment_manifest"]["chain_fingerprint"] = str(
            payload.get("metadata", {}).get("solve_score_chain_fingerprint") or ""
        )
    return payload


class _Body:
    def __init__(self, data):
        self._data = data

    def read(self):
        return self._data

    def iter_chunks(self, chunk_size):
        yield self._data


class TestFinalizeMTHandler(unittest.TestCase):
    def test_finalize_mt_rejects_invalid_contract_fields(self):
        import handler_finalize_mt as mod

        with self.assertRaisesRegex(RuntimeError, "phase='finalize_mt'"):
            mod.handler(_event(phase="finalize"), None)

        with self.assertRaisesRegex(RuntimeError, "raster_engine='mt'"):
            mod.handler(_event(render_execution={"raster_engine": "single"}), None)

        bad_manifest = dict(_event()["fragment_manifest"], version=2)
        with self.assertRaisesRegex(RuntimeError, "fragment_manifest.version"):
            mod.handler(_event(fragment_manifest=bad_manifest), None)

        bad_manifest = dict(_event()["fragment_manifest"], pair_encoding="broken")
        with self.assertRaisesRegex(RuntimeError, "fragment_manifest.pair_encoding"):
            mod.handler(_event(fragment_manifest=bad_manifest), None)

    @patch("handler_finalize_mt.write_color_artifact_meta_overlay")
    @patch("handler_finalize_mt.report_status")
    @patch("raw_score_render.subprocess.run")
    @patch("handler_finalize_mt.subprocess.run")
    @patch("handler_finalize_mt._finalize_s3_client")
    def test_finalize_mt_assembles_raw_equalizes_and_uploads_outputs(
        self,
        mock_client_factory,
        mock_run,
        mock_raw_render_run,
        mock_report,
        mock_overlay,
    ):
        import handler_finalize_mt as mod

        uploads = {}
        fake_s3 = MagicMock()

        def put_object(**kwargs):
            body = kwargs["Body"]
            data = body.read() if hasattr(body, "read") else body
            uploads[kwargs["Key"]] = {
                "data": data,
                "content_type": kwargs.get("ContentType"),
                "metadata": kwargs.get("Metadata"),
                "cache_control": kwargs.get("CacheControl"),
            }

        def get_object(Bucket=None, Key=None):
            if Key == f"{TEST_FRAGMENT_PREFIX}0000_step_scores.raw":
                return {"Body": _Body(bytes([9, 7, 5, 3]))}
            raise AssertionError(f"unexpected get_object key: {Key}")

        fake_s3.get_object.side_effect = get_object
        fake_s3.generate_presigned_url.side_effect = (
            lambda _op, Params, ExpiresIn=900: f"https://example.invalid/{Params['Key']}"
        )
        fake_s3.put_object.side_effect = put_object
        mock_client_factory.return_value = fake_s3

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "assemble_greyscale":
                out_path = next(arg for arg in cmd if arg.startswith("--output=")).split("=", 1)[1]
                hist_path = next(arg for arg in cmd if arg.startswith("--hist-output=")).split("=", 1)[1]
                manifest_path = next(arg for arg in cmd if arg.startswith("--url-manifest=")).split("=", 1)[1]
                self.assertEqual(pathlib.Path(manifest_path).read_text(encoding="utf-8").strip(), f"https://example.invalid/{TEST_FRAGMENT_KEY}")
                with open(out_path, "wb") as fh:
                    fh.write(bytes([0, 1, 2, 3]))
                with open(hist_path, "w", encoding="utf-8") as fh:
                    json.dump({"version": 1, "background_pixels": 1, "nonzero_pixels": 3, "histogram": [1, 1, 1, 1] + [0] * 252}, fh)
                return MagicMock(returncode=0, stdout="", stderr="")
            if exe == "score_raw_render":
                out_path = cmd[2]
                preview_arg = next(arg for arg in cmd if arg.startswith("--preview="))
                preview_path = preview_arg.split("=", 1)[1]
                with open(out_path, "wb") as fh:
                    fh.write(b"JPEGDATA")
                with open(preview_path, "wb") as fh:
                    fh.write(b"PREVIEWPNG")
                return MagicMock(returncode=0, stdout=json.dumps({"file_size": 8, "preview_file_size": 10}), stderr="")
            raise AssertionError(f"unexpected executable {exe}")

        mock_run.side_effect = run_side_effect
        mock_raw_render_run.side_effect = run_side_effect

        result = mod.handler(_event(), None)
        body = json.loads(result["body"])

        self.assertEqual(body["image_key"], TEST_IMAGE_KEY)
        self.assertEqual(body["raw_key"], TEST_RAW_KEY)
        self.assertEqual(body["raw_meta_key"], TEST_RAW_META_KEY)
        self.assertEqual(body["file_size"], 8)
        self.assertEqual(uploads[TEST_RAW_KEY]["data"], bytes([0, 1, 2, 3]))
        raw_meta = json.loads(uploads[TEST_RAW_META_KEY]["data"].decode("utf-8"))
        self.assertEqual(raw_meta["version"], 2)
        self.assertEqual(raw_meta["artifact_family"], "color")
        self.assertEqual(raw_meta["artifact_id"], TEST_ARTIFACT_ID)
        self.assertEqual(raw_meta["chain_fingerprint"], "fp_test")
        self.assertEqual(raw_meta["score_program"], "m0")
        self.assertEqual(raw_meta["clip_slots"], [{"slot": 0, "metric": "proximity", "source": "slv", "clip_lo": 0.1, "clip_hi": 0.9}])
        self.assertEqual(raw_meta["score_output_normalize"], False)
        self.assertEqual(raw_meta["score_output_clip_lo"], 0.0)
        self.assertEqual(raw_meta["score_output_clip_hi"], 1.0)
        self.assertEqual(raw_meta["background_color"], [0, 0, 0])
        self.assertEqual(raw_meta["plan_params_digest"], "sha256:plan123")
        self.assertEqual(raw_meta["histogram"], [1, 1, 1, 1] + [0] * 252)
        self.assertEqual(raw_meta["keys"]["raw_key"], TEST_RAW_KEY)
        self.assertEqual(raw_meta["keys"]["image_key"], TEST_IMAGE_KEY)
        self.assertEqual(raw_meta["keys"]["preview_key"], TEST_PREVIEW_KEY)
        self.assertEqual(
            uploads[TEST_PREVIEW_KEY]["cache_control"],
            "public, max-age=31536000, immutable",
        )
        self.assertIsNone(uploads[TEST_IMAGE_KEY]["cache_control"])
        self.assertEqual(raw_meta["keys"]["meta_key"], TEST_META_KEY)
        self.assertEqual(uploads[TEST_IMAGE_KEY]["data"], b"JPEGDATA")
        self.assertEqual(uploads[TEST_IMAGE_KEY]["content_type"], "image/jpeg")
        self.assertEqual(uploads[TEST_IMAGE_KEY]["metadata"]["artifact_id"], TEST_ARTIFACT_ID)
        self.assertEqual(uploads[TEST_PREVIEW_KEY]["data"], b"PREVIEWPNG")
        mock_overlay.assert_called_once()
        overlay_meta = mock_overlay.call_args.args[4]
        self.assertEqual(overlay_meta["raw_key"], TEST_RAW_KEY)
        self.assertEqual(overlay_meta["raw_meta_key"], TEST_RAW_META_KEY)
        self.assertEqual(overlay_meta["score_output_normalize"], "false")
        self.assertEqual(overlay_meta["repalette_capable"], "true")
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(
            statuses,
            ["started", "assembled_score_tiles", "wrote_greyscale_raw", "rendered_rgb_tiles", "encoded", "done"],
        )

    @patch("handler_finalize_mt.write_color_artifact_meta_overlay")
    @patch("handler_finalize_mt.report_status")
    @patch("raw_score_render.subprocess.run")
    @patch("handler_finalize_mt.subprocess.run")
    @patch("handler_finalize_mt._finalize_s3_client")
    def test_finalize_mt_accepts_direct_rgb_raw_outputs(
        self,
        mock_client_factory,
        mock_run,
        mock_raw_render_run,
        mock_report,
        mock_overlay,
    ):
        import handler_finalize_mt as mod

        uploads = {}
        fake_s3 = MagicMock()

        def put_object(**kwargs):
            body = kwargs["Body"]
            data = body.read() if hasattr(body, "read") else body
            uploads[kwargs["Key"]] = {
                "data": data,
                "content_type": kwargs.get("ContentType"),
                "metadata": kwargs.get("Metadata"),
                "cache_control": kwargs.get("CacheControl"),
            }

        def get_object(Bucket=None, Key=None):
            if Key == f"{TEST_FRAGMENT_PREFIX}0000_step_scores.raw":
                return {"Body": _Body(bytes([0, 255, 0, 255, 0, 128, 1, 2, 3, 4, 5, 6]))}
            raise AssertionError(f"unexpected get_object key: {Key}")

        fake_s3.get_object.side_effect = get_object
        fake_s3.generate_presigned_url.side_effect = (
            lambda _op, Params, ExpiresIn=900: f"https://example.invalid/{Params['Key']}"
        )
        fake_s3.put_object.side_effect = put_object
        mock_client_factory.return_value = fake_s3

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "assemble_greyscale":
                self.assertIn("--channels=3", cmd)
                self.assertIn("--allow-zero=1", cmd)
                out_path = next(arg for arg in cmd if arg.startswith("--output=")).split("=", 1)[1]
                hist_path = next(arg for arg in cmd if arg.startswith("--hist-output=")).split("=", 1)[1]
                with open(out_path, "wb") as fh:
                    fh.write(bytes([0, 255, 0, 255, 0, 128, 0, 0, 0, 0, 0, 0]))
                with open(hist_path, "w", encoding="utf-8") as fh:
                    json.dump({"version": 1, "channels": 3, "background_pixels": 2, "nonzero_pixels": 2, "histogram": [3, 0, 0, 0] + [0] * 251 + [1]}, fh)
                return MagicMock(returncode=0, stdout="", stderr="")
            if exe == "score_raw_render":
                self.assertIn("--channels=3", cmd)
                self.assertIn("--interpretation=rgb", cmd)
                out_path = cmd[2]
                preview_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--preview="))
                with open(out_path, "wb") as fh:
                    fh.write(b"RGBJPEG")
                with open(preview_path, "wb") as fh:
                    fh.write(b"RGBPREVIEW")
                return MagicMock(returncode=0, stdout=json.dumps({"file_size": 7, "preview_file_size": 10}), stderr="")
            raise AssertionError(f"unexpected executable {exe}")

        mock_run.side_effect = run_side_effect
        mock_raw_render_run.side_effect = run_side_effect

        event = _event(
            score_program="m0-0;emit_norm;m1-0;emit_norm;m2-0;emit_norm",
            metadata=dict(_event()["metadata"], score_program="m0-0;emit_norm;m1-0;emit_norm;m2-0;emit_norm"),
            score_output_channel_count=3,
            score_output_has_explicit_outputs=True,
            score_output_interpretation="direct_rgb",
            score_output_channels=[
                {"channel": 0, "name": "r", "emit": "emit_norm", "clip_lo": 0.1, "clip_hi": 0.9, "range_normalized": True},
                {"channel": 1, "name": "g", "emit": "emit_norm", "clip_lo": 0.2, "clip_hi": 0.8, "range_normalized": True},
                {"channel": 2, "name": "b", "emit": "emit_norm", "clip_lo": 0.3, "clip_hi": 0.7, "range_normalized": True},
            ],
            fragment_manifest={
                "version": 1,
                "pair_encoding": "u32le_pixel_idx_plus_u8_channels_v1",
                "channels": 3,
                "record_size_bytes": 7,
                "item_count": 1,
                "fragment_prefix": TEST_FRAGMENT_PREFIX,
                "chain_fingerprint": "fp_test",
            },
            associated_palette_grid_n=2,
            associated_palette_times=1,
        )
        result = mod.handler(event, None)
        body = json.loads(result["body"])

        self.assertEqual(body["channels"], 3)
        self.assertEqual(body["step_scores_key"], f"renders/{TEST_JOB_ID}/color/{TEST_ARTIFACT_ID}/step_scores.raw")
        self.assertEqual(body["step_count"], 4)
        self.assertEqual(body["step_scores_grid_n"], 2)
        self.assertEqual(
            uploads[f"renders/{TEST_JOB_ID}/color/{TEST_ARTIFACT_ID}/step_scores.raw"]["data"],
            bytes([0, 255, 0, 255, 0, 128, 1, 2, 3, 4, 5, 6]),
        )
        raw_meta = json.loads(uploads[TEST_RAW_META_KEY]["data"].decode("utf-8"))
        self.assertEqual(raw_meta["channels"], 3)
        self.assertEqual(raw_meta["raw_layout"], "u8_packed_channels_row_major")
        self.assertEqual(raw_meta["interpretation"], "rgb")
        self.assertEqual(raw_meta["encoding"]["type"], "u8_packed_channels_v1")
        self.assertEqual(raw_meta["step_count"], 4)
        self.assertEqual(raw_meta["step_scores_key"], f"renders/{TEST_JOB_ID}/color/{TEST_ARTIFACT_ID}/step_scores.raw")
        self.assertEqual(len(raw_meta["output_channels"]), 3)
        overlay_meta = mock_overlay.call_args.args[4]
        self.assertEqual(overlay_meta["raw_channels"], "3")
        self.assertEqual(overlay_meta["step_scores_key"], f"renders/{TEST_JOB_ID}/color/{TEST_ARTIFACT_ID}/step_scores.raw")
        self.assertEqual(overlay_meta["rgb_source"], "rgb_raw")

    @patch("handler_finalize_mt.write_color_artifact_meta_overlay")
    @patch("handler_finalize_mt.report_status")
    @patch("raw_score_render.subprocess.run")
    @patch("handler_finalize_mt.subprocess.run")
    @patch("handler_finalize_mt._finalize_s3_client")
    def test_finalize_mt_accepts_hsv_lut_raw_outputs(
        self,
        mock_client_factory,
        mock_run,
        mock_raw_render_run,
        mock_report,
        mock_overlay,
    ):
        import handler_finalize_mt as mod

        uploads = {}
        fake_s3 = MagicMock()

        def put_object(**kwargs):
            body = kwargs["Body"]
            data = body.read() if hasattr(body, "read") else body
            uploads[kwargs["Key"]] = {
                "data": data,
                "content_type": kwargs.get("ContentType"),
                "metadata": kwargs.get("Metadata"),
                "cache_control": kwargs.get("CacheControl"),
            }

        def get_object(Bucket=None, Key=None):
            if Key == f"{TEST_FRAGMENT_PREFIX}0000_step_scores.raw":
                return {"Body": _Body(bytes([12, 34, 56, 78, 90, 123, 0, 0, 0, 0, 0, 0]))}
            raise AssertionError(f"unexpected get_object key: {Key}")

        fake_s3.get_object.side_effect = get_object
        fake_s3.generate_presigned_url.side_effect = (
            lambda _op, Params, ExpiresIn=900: f"https://example.invalid/{Params['Key']}"
        )
        fake_s3.put_object.side_effect = put_object
        mock_client_factory.return_value = fake_s3

        assoc_raw_key = f"renders/{TEST_JOB_ID}/palettes/pal_{TEST_ARTIFACT_ID}/greyscale.raw"
        assoc_raw_meta_key = f"renders/{TEST_JOB_ID}/palettes/pal_{TEST_ARTIFACT_ID}/greyscale.meta.json"
        assoc_image_key = f"renders/{TEST_JOB_ID}/palettes/pal_{TEST_ARTIFACT_ID}/image.jpeg"
        assoc_preview_key = f"renders/{TEST_JOB_ID}/palettes/pal_{TEST_ARTIFACT_ID}/preview.png"
        assoc_meta_key = f"renders/{TEST_JOB_ID}/palettes/pal_{TEST_ARTIFACT_ID}/meta.json"
        assoc_fragment_prefix = f"renders/{TEST_JOB_ID}/palettes/pal_{TEST_ARTIFACT_ID}/fragments/section_"

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "assemble_greyscale":
                self.assertIn("--channels=3", cmd)
                self.assertIn("--allow-zero=1", cmd)
                out_path = next(arg for arg in cmd if arg.startswith("--output=")).split("=", 1)[1]
                hist_path = next(arg for arg in cmd if arg.startswith("--hist-output=")).split("=", 1)[1]
                manifest_path = next(arg for arg in cmd if arg.startswith("--url-manifest=")).split("=", 1)[1]
                manifest_value = pathlib.Path(manifest_path).read_text(encoding="utf-8").strip()
                if "assoc_palette" in out_path:
                    self.assertEqual(manifest_value, f"https://example.invalid/{assoc_fragment_prefix}0000.frag")
                else:
                    self.assertEqual(manifest_value, f"https://example.invalid/{TEST_FRAGMENT_PREFIX}0000.frag")
                with open(out_path, "wb") as fh:
                    fh.write(bytes([12, 34, 56, 78, 90, 123, 0, 0, 0, 0, 0, 0]))
                with open(hist_path, "w", encoding="utf-8") as fh:
                    json.dump({"version": 1, "channels": 3, "background_pixels": 2, "nonzero_pixels": 2, "histogram": [2] + [0] * 255}, fh)
                return MagicMock(returncode=0, stdout="", stderr="")
            if exe == "score_raw_render":
                self.assertIn("--channels=3", cmd)
                self.assertIn("--interpretation=hsv_lut", cmd)
                self.assertIn("--palette=inferno", cmd)
                out_path = cmd[2]
                if "assoc_palette" in out_path:
                    self.assertIn("--zero_background=0", cmd)
                else:
                    self.assertIn("--zero_background=1", cmd)
                preview_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--preview="))
                with open(out_path, "wb") as fh:
                    fh.write(b"HSV_LUT_JPEG")
                with open(preview_path, "wb") as fh:
                    fh.write(b"HSV_LUT_PREVIEW")
                return MagicMock(returncode=0, stdout=json.dumps({"file_size": 12, "preview_file_size": 15}), stderr="")
            raise AssertionError(f"unexpected executable {exe}")

        mock_run.side_effect = run_side_effect
        mock_raw_render_run.side_effect = run_side_effect
        event = _event(
            score_program="m0-0;emit_norm;m1-0;emit_norm;m2-0;emit_norm",
            metadata=dict(_event()["metadata"], score_program="m0-0;emit_norm;m1-0;emit_norm;m2-0;emit_norm"),
            score_output_channel_count=3,
            score_output_has_explicit_outputs=True,
            score_output_interpretation="hsv_lut",
            score_output_channels=[
                {"channel": 0, "name": "channel_0", "emit": "emit_norm", "range_normalized": True},
                {"channel": 1, "name": "channel_1", "emit": "emit_norm", "range_normalized": True},
                {"channel": 2, "name": "channel_2", "emit": "emit_norm", "range_normalized": True},
            ],
            fragment_manifest={
                "version": 1,
                "pair_encoding": "u32le_pixel_idx_plus_u8_channels_v1",
                "channels": 3,
                "record_size_bytes": 7,
                "item_count": 1,
                "fragment_prefix": TEST_FRAGMENT_PREFIX,
                "chain_fingerprint": "fp_test",
            },
            associated_palette={
                "mode": "generated",
                "palette_id": f"pal_{TEST_ARTIFACT_ID}",
                "display_name": "assoc hsv lut",
                "palette": "inferno",
                "metric": "proximity",
                "score_chain": '[["proximity","0.1"]]',
                "raw_key": assoc_raw_key,
                "raw_meta_key": assoc_raw_meta_key,
                "image_key": assoc_image_key,
                "preview_key": assoc_preview_key,
                "meta_key": assoc_meta_key,
                "fragment_prefix": assoc_fragment_prefix,
                "source_color_artifact_id": TEST_ARTIFACT_ID,
                "color_interpretation": "hsv_lut",
            },
            associated_palette_grid_n=2,
            associated_palette_times=1,
            associated_palette_degree=5,
        )
        result = mod.handler(event, None)
        body = json.loads(result["body"])

        self.assertEqual(body["channels"], 3)
        self.assertEqual(body["step_scores_key"], f"renders/{TEST_JOB_ID}/color/{TEST_ARTIFACT_ID}/step_scores.raw")
        self.assertEqual(body["associated_palette"]["mode"], "generated")
        self.assertEqual(body["associated_palette"]["image_key"], assoc_image_key)
        self.assertEqual(body["associated_palette"]["raw_key"], assoc_raw_key)
        self.assertEqual(uploads[assoc_image_key]["data"], b"HSV_LUT_JPEG")
        self.assertEqual(uploads[assoc_preview_key]["data"], b"HSV_LUT_PREVIEW")
        self.assertEqual(uploads[assoc_raw_key]["data"], bytes([12, 34, 56, 78, 90, 123, 0, 0, 0, 0, 0, 0]))
        assoc_meta = json.loads(uploads[assoc_meta_key]["data"].decode("utf-8"))
        self.assertEqual(assoc_meta["data_layout"], "fused_pass0_raw_v1")
        self.assertEqual(assoc_meta["raw_channels"], 3)
        self.assertEqual(assoc_meta["palette"], "inferno")
        self.assertEqual(assoc_meta["color_interpretation"], "hsv_lut")
        self.assertEqual(assoc_meta["derived_from_color_artifact_id"], TEST_ARTIFACT_ID)
        assoc_raw_meta = json.loads(uploads[assoc_raw_meta_key]["data"].decode("utf-8"))
        self.assertEqual(assoc_raw_meta["channels"], 3)
        self.assertEqual(assoc_raw_meta["raw_layout"], "u8_packed_channels_row_major")
        self.assertEqual(assoc_raw_meta["interpretation"], "hsv_lut")
        self.assertEqual([row["name"] for row in assoc_raw_meta["output_channels"]], ["h_lookup", "s_lookup", "v_lookup"])
        raw_meta = json.loads(uploads[TEST_RAW_META_KEY]["data"].decode("utf-8"))
        self.assertEqual(raw_meta["interpretation"], "hsv_lut")
        self.assertEqual([row["name"] for row in raw_meta["output_channels"]], ["h_lookup", "s_lookup", "v_lookup"])
        overlay_meta = mock_overlay.call_args.args[4]
        self.assertEqual(overlay_meta["score_output_interpretation"], "hsv_lut")
        self.assertEqual(overlay_meta["rgb_source"], "hsv_lut_raw")
        self.assertEqual(overlay_meta["associated_palette_mode"], "generated")
        self.assertEqual(overlay_meta["associated_palette_color_interpretation"], "hsv_lut")
        self.assertEqual(overlay_meta["associated_palette_raw_key"], assoc_raw_key)

    @patch("handler_finalize_mt.write_color_artifact_meta_overlay")
    @patch("handler_finalize_mt.report_status")
    @patch("raw_score_render.subprocess.run")
    @patch("handler_finalize_mt.subprocess.run")
    @patch("handler_finalize_mt._finalize_s3_client")
    def test_finalize_mt_writes_v3_step_score_sidecar_when_grid_metadata_is_present(
        self,
        mock_client_factory,
        mock_run,
        mock_raw_render_run,
        mock_report,
        mock_overlay,
    ):
        import handler_finalize_mt as mod

        uploads = {}
        fake_s3 = MagicMock()
        step_scores_key = f"{TEST_FRAGMENT_PREFIX}0000_step_scores.raw"

        def get_object(Bucket=None, Key=None):
            if Key == step_scores_key:
                return {"Body": _Body(bytes([9, 7, 5, 3]))}
            raise AssertionError(f"unexpected get_object key: {Key}")

        def put_object(**kwargs):
            body = kwargs["Body"]
            data = body.read() if hasattr(body, "read") else body
            uploads[kwargs["Key"]] = {
                "data": data,
                "content_type": kwargs.get("ContentType"),
                "metadata": kwargs.get("Metadata"),
                "cache_control": kwargs.get("CacheControl"),
            }

        fake_s3.get_object.side_effect = get_object
        fake_s3.generate_presigned_url.side_effect = (
            lambda _op, Params, ExpiresIn=900: f"https://example.invalid/{Params['Key']}"
        )
        fake_s3.put_object.side_effect = put_object
        mock_client_factory.return_value = fake_s3

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "assemble_greyscale":
                out_path = next(arg for arg in cmd if arg.startswith("--output=")).split("=", 1)[1]
                hist_path = next(arg for arg in cmd if arg.startswith("--hist-output=")).split("=", 1)[1]
                with open(out_path, "wb") as fh:
                    fh.write(bytes([0, 1, 2, 3]))
                with open(hist_path, "w", encoding="utf-8") as fh:
                    json.dump({"version": 1, "background_pixels": 1, "nonzero_pixels": 3, "histogram": [1, 1, 1, 1] + [0] * 252}, fh)
                return MagicMock(returncode=0, stdout="", stderr="")
            if exe == "score_raw_render":
                out_path = cmd[2]
                preview_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--preview="))
                with open(out_path, "wb") as fh:
                    fh.write(b"JPEGDATA")
                with open(preview_path, "wb") as fh:
                    fh.write(b"PREVIEWPNG")
                return MagicMock(returncode=0, stdout=json.dumps({"file_size": 8, "preview_file_size": 10}), stderr="")
            raise AssertionError(f"unexpected executable {exe}")

        mock_run.side_effect = run_side_effect
        mock_raw_render_run.side_effect = run_side_effect

        result = mod.handler(_event(associated_palette_grid_n=2, associated_palette_times=1), None)
        body = json.loads(result["body"])

        self.assertEqual(body["step_scores_key"], f"renders/{TEST_JOB_ID}/color/{TEST_ARTIFACT_ID}/step_scores.raw")
        self.assertEqual(body["step_count"], 4)
        self.assertEqual(body["step_scores_grid_n"], 2)
        self.assertEqual(
            uploads[f"renders/{TEST_JOB_ID}/color/{TEST_ARTIFACT_ID}/step_scores.raw"]["data"],
            bytes([9, 7, 5, 3]),
        )
        raw_meta = json.loads(uploads[TEST_RAW_META_KEY]["data"].decode("utf-8"))
        self.assertEqual(raw_meta["version"], 3)
        self.assertEqual(raw_meta["step_scores_key"], f"renders/{TEST_JOB_ID}/color/{TEST_ARTIFACT_ID}/step_scores.raw")
        self.assertEqual(raw_meta["step_count"], 4)
        self.assertEqual(raw_meta["step_scores_grid_n"], 2)
        overlay_meta = mock_overlay.call_args.args[4]
        self.assertEqual(overlay_meta["step_scores_key"], f"renders/{TEST_JOB_ID}/color/{TEST_ARTIFACT_ID}/step_scores.raw")
        self.assertEqual(overlay_meta["step_count"], "4")
        self.assertEqual(overlay_meta["step_scores_grid_n"], "2")
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(
            statuses,
            ["started", "assembled_score_tiles", "wrote_greyscale_raw", "wrote_step_scores", "rendered_rgb_tiles", "encoded", "done"],
        )

    @patch("handler_finalize_mt.write_color_artifact_meta_overlay")
    @patch("handler_finalize_mt.report_status")
    @patch("raw_score_render.subprocess.run")
    @patch("handler_finalize_mt.subprocess.run")
    @patch("handler_finalize_mt._finalize_s3_client")
    def test_finalize_mt_writes_inline_associated_palette_outputs(
        self,
        mock_client_factory,
        mock_run,
        mock_raw_render_run,
        mock_report,
        mock_overlay,
    ):
        import handler_finalize_mt as mod

        uploads = {}
        fake_s3 = MagicMock()
        assoc_fragment_prefix = f"renders/{TEST_JOB_ID}/palettes/pal_{TEST_ARTIFACT_ID}/fragments/section_"
        assoc_raw_key = f"renders/{TEST_JOB_ID}/palettes/pal_{TEST_ARTIFACT_ID}/greyscale.raw"
        assoc_raw_meta_key = f"renders/{TEST_JOB_ID}/palettes/pal_{TEST_ARTIFACT_ID}/greyscale.meta.json"
        assoc_image_key = f"renders/{TEST_JOB_ID}/palettes/pal_{TEST_ARTIFACT_ID}/image.jpeg"
        assoc_preview_key = f"renders/{TEST_JOB_ID}/palettes/pal_{TEST_ARTIFACT_ID}/preview.png"
        assoc_meta_key = f"renders/{TEST_JOB_ID}/palettes/pal_{TEST_ARTIFACT_ID}/meta.json"

        def put_object(**kwargs):
            body = kwargs["Body"]
            data = body.read() if hasattr(body, "read") else body
            uploads[kwargs["Key"]] = {
                "data": data,
                "content_type": kwargs.get("ContentType"),
                "metadata": kwargs.get("Metadata"),
                "cache_control": kwargs.get("CacheControl"),
            }

        def get_object(Bucket=None, Key=None):
            if Key == f"{TEST_FRAGMENT_PREFIX}0000_step_scores.raw":
                return {"Body": _Body(bytes([9, 7, 5, 3]))}
            raise AssertionError(f"unexpected get_object key: {Key}")

        fake_s3.get_object.side_effect = get_object
        fake_s3.generate_presigned_url.side_effect = (
            lambda _op, Params, ExpiresIn=900: f"https://example.invalid/{Params['Key']}"
        )
        fake_s3.put_object.side_effect = put_object
        mock_client_factory.return_value = fake_s3

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "assemble_greyscale":
                out_path = next(arg for arg in cmd if arg.startswith("--output=")).split("=", 1)[1]
                hist_path = next(arg for arg in cmd if arg.startswith("--hist-output=")).split("=", 1)[1]
                manifest_path = next(arg for arg in cmd if arg.startswith("--url-manifest=")).split("=", 1)[1]
                manifest_value = pathlib.Path(manifest_path).read_text(encoding="utf-8").strip()
                if "assoc_palette" in out_path:
                    self.assertEqual(
                        manifest_value,
                        f"https://example.invalid/{assoc_fragment_prefix}0000.frag",
                    )
                else:
                    self.assertEqual(
                        manifest_value,
                        f"https://example.invalid/{TEST_FRAGMENT_PREFIX}0000.frag",
                    )
                data = bytes([0, 4, 5, 6]) if "assoc_palette" in out_path else bytes([0, 1, 2, 3])
                with open(out_path, "wb") as fh:
                    fh.write(data)
                histogram = [0] * 256
                for byte in data:
                    histogram[byte] += 1
                with open(hist_path, "w", encoding="utf-8") as fh:
                    json.dump({"version": 1, "background_pixels": histogram[0], "nonzero_pixels": len(data) - histogram[0], "histogram": histogram}, fh)
                return MagicMock(returncode=0, stdout="", stderr="")
            if exe == "score_raw_render":
                out_path = cmd[2]
                preview_arg = next(arg for arg in cmd if arg.startswith("--preview="))
                preview_path = preview_arg.split("=", 1)[1]
                payload = b"PALETTEJPEG" if "assoc_palette" in out_path else b"MAINJPEG"
                if "assoc_palette" in out_path:
                    self.assertIn("--zero_background=0", cmd)
                else:
                    self.assertIn("--zero_background=1", cmd)
                with open(out_path, "wb") as fh:
                    fh.write(payload)
                with open(preview_path, "wb") as fh:
                    fh.write(b"PREVIEWPNG")
                return MagicMock(
                    returncode=0,
                    stdout=json.dumps({"file_size": len(payload), "preview_file_size": 10}),
                    stderr="",
                )
            raise AssertionError(f"unexpected executable {exe}")

        mock_run.side_effect = run_side_effect
        mock_raw_render_run.side_effect = run_side_effect
        metadata = dict(_event()["metadata"])
        metadata["score_program"] = "m0;emit"

        result = mod.handler(_event(
            metadata=metadata,
            fragment_prefix=TEST_FRAGMENT_PREFIX,
            associated_palette={
                "mode": "generated",
                "palette_id": f"pal_{TEST_ARTIFACT_ID}",
                "display_name": "assoc palette",
                "palette": "inferno",
                "metric": "proximity",
                "score_chain": '[["proximity","0.1"]]',
                "raw_key": assoc_raw_key,
                "raw_meta_key": assoc_raw_meta_key,
                "image_key": assoc_image_key,
                "preview_key": assoc_preview_key,
                "meta_key": assoc_meta_key,
                "fragment_prefix": assoc_fragment_prefix,
                "source_color_artifact_id": TEST_ARTIFACT_ID,
            },
            score_program="m0;emit",
            score_output_has_explicit_outputs=True,
            score_output_channels=[
                {"channel": 0, "name": "score", "emit": "emit", "range_normalized": False},
            ],
            associated_palette_grid_n=2,
            associated_palette_times=1,
            associated_palette_degree=5,
        ), None)
        body = json.loads(result["body"])

        self.assertEqual(body["associated_palette"]["image_key"], assoc_image_key)
        self.assertEqual(uploads[TEST_RAW_KEY]["data"], bytes([0, 1, 2, 3]))
        self.assertEqual(uploads[assoc_raw_key]["data"], bytes([0, 4, 5, 6]))
        self.assertEqual(uploads[TEST_IMAGE_KEY]["data"], b"MAINJPEG")
        self.assertEqual(uploads[assoc_image_key]["data"], b"PALETTEJPEG")
        self.assertEqual(uploads[assoc_preview_key]["data"], b"PREVIEWPNG")
        assoc_meta = json.loads(uploads[assoc_meta_key]["data"].decode("utf-8"))
        self.assertEqual(assoc_meta["derived_from_color_artifact_id"], TEST_ARTIFACT_ID)
        self.assertEqual(assoc_meta["data_layout"], "fused_pass0_raw_v1")
        self.assertEqual(assoc_meta["image_key"], assoc_image_key)
        assoc_sidecar = json.loads(uploads[assoc_raw_meta_key]["data"].decode("utf-8"))
        self.assertEqual(assoc_sidecar["version"], 2)
        self.assertEqual(assoc_sidecar["artifact_family"], "palette")
        self.assertEqual(assoc_sidecar["artifact_id"], f"pal_{TEST_ARTIFACT_ID}")
        self.assertEqual(assoc_sidecar["keys"]["image_key"], assoc_image_key)
        self.assertEqual(assoc_sidecar["keys"]["preview_key"], assoc_preview_key)
        self.assertEqual(assoc_sidecar["keys"]["meta_key"], assoc_meta_key)
        self.assertEqual(assoc_sidecar["chain_fingerprint"], "fp_test")
        self.assertEqual(assoc_sidecar["score_output_normalize"], False)
        self.assertEqual(assoc_sidecar["score_output_clip_lo"], 0.0)
        self.assertEqual(assoc_sidecar["score_output_clip_hi"], 1.0)
        self.assertEqual(assoc_sidecar["histogram"][4:7], [1, 1, 1])
        mock_overlay.assert_called_once()
        overlay_meta = mock_overlay.call_args.args[4]
        self.assertEqual(overlay_meta["associated_palette_mode"], "generated")
        self.assertEqual(overlay_meta["associated_palette_id"], f"pal_{TEST_ARTIFACT_ID}")
        self.assertEqual(overlay_meta["associated_palette_image_key"], assoc_image_key)
        self.assertEqual(overlay_meta["associated_palette_raw_key"], assoc_raw_key)
        self.assertEqual(overlay_meta["associated_palette_meta_key"], assoc_meta_key)

    @patch("handler_finalize_mt.subprocess.run")
    def test_assemble_greyscale_raw_emits_periodic_progress_callbacks(self, mock_run):
        import handler_finalize_mt as mod

        progress_calls = []

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe != "assemble_greyscale":
                raise AssertionError(f"unexpected executable {exe}")
            out_path = next(arg for arg in cmd if arg.startswith("--output=")).split("=", 1)[1]
            hist_path = next(arg for arg in cmd if arg.startswith("--hist-output=")).split("=", 1)[1]
            with open(out_path, "wb") as fh:
                fh.write(bytes([0, 1, 2, 3]))
            with open(hist_path, "w", encoding="utf-8") as fh:
                json.dump({"version": 1, "background_pixels": 1, "nonzero_pixels": 3, "histogram": [1, 1, 1, 1] + [0] * 252}, fh)
            time.sleep(0.2)
            return MagicMock(returncode=0, stdout="", stderr="")

        mock_run.side_effect = run_side_effect

        with tempfile.TemporaryDirectory() as tmpdir:
            raw_path = os.path.join(tmpdir, "greyscale.raw")
            hist_path = os.path.join(tmpdir, "greyscale.hist.json")
            manifest_path = os.path.join(tmpdir, "fragments.urls")
            with patch.object(mod, "ASSEMBLE_PROGRESS_INTERVAL_S", 0.01):
                hist_meta = mod._assemble_greyscale_raw(
                    pix=2,
                    raw_path=raw_path,
                    hist_path=hist_path,
                    workers=1,
                    fragment_urls=["https://example.invalid/test.frag"],
                    manifest_path=manifest_path,
                    progress_cb=progress_calls.append,
                )

        self.assertEqual(hist_meta["nonzero_pixels"], 3)
        self.assertTrue(progress_calls)
        self.assertTrue(any(int(v) > 0 for v in progress_calls))
