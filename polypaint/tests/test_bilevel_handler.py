import json
import os
import sys
import tempfile
import unittest
from unittest.mock import MagicMock, patch

from botocore.exceptions import ClientError


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class _Body:
    def __init__(self, data):
        self._data = data

    def read(self):
        return self._data


class TestBilevelHandler(unittest.TestCase):
    @patch("handler_bilevel.report_status")
    def test_handler_requires_phase(self, mock_report):
        import handler_bilevel as mod

        with self.assertRaisesRegex(ValueError, "requires non-empty phase"):
            mod.handler({"body": json.dumps({"job_id": "job", "task_id": "color_to_bilevel_missing_phase"})}, None)

        mock_report.assert_called_once()
        self.assertEqual(mock_report.call_args.args[0], "job")
        self.assertEqual(mock_report.call_args.args[1], "color_to_bilevel_missing_phase")
        self.assertEqual(mock_report.call_args.args[2], "error")
        self.assertIn("requires non-empty phase", mock_report.call_args.args[3])
        rd = mock_report.call_args.kwargs.get("result_data") or {}
        self.assertEqual(rd.get("phase"), "handler_entry")
        self.assertEqual(rd.get("phase_label"), "BiLevel dispatch")

    @patch("handler_bilevel.report_status")
    def test_handler_reports_unknown_phase(self, mock_report):
        import handler_bilevel as mod

        with self.assertRaisesRegex(ValueError, "Unknown bilevel phase: bogus_phase"):
            mod.handler({"body": json.dumps({
                "job_id": "job",
                "task_id": "color_to_bilevel_bad_phase",
                "phase": "bogus_phase",
                "artifact_id": "bil_1",
                "source_artifact_id": "color_src",
                "threshold": 9,
            })}, None)

        mock_report.assert_called_once()
        self.assertEqual(mock_report.call_args.args[0], "job")
        self.assertEqual(mock_report.call_args.args[1], "color_to_bilevel_bad_phase")
        self.assertEqual(mock_report.call_args.args[2], "error")
        self.assertIn("Unknown bilevel phase: bogus_phase", mock_report.call_args.args[3])
        rd = mock_report.call_args.kwargs.get("result_data") or {}
        self.assertEqual(rd.get("phase"), "handler_entry")
        self.assertEqual(rd.get("phase_raw"), "bogus_phase")
        self.assertEqual(rd.get("artifact_id"), "bil_1")
        self.assertEqual(rd.get("source_artifact_id"), "color_src")
        self.assertEqual(rd.get("threshold"), "9")

    @patch("handler_bilevel.report_status")
    @patch("handler_bilevel.subprocess.run")
    @patch("handler_bilevel.s3")
    def test_merge_does_not_fallback_to_legacy_bits_keys(self, mock_s3, mock_run, mock_report):
        import handler_bilevel as mod

        requested_keys = []
        uploaded = {}

        def get_object(**kwargs):
            key = kwargs["Key"]
            requested_keys.append(key)
            if key == "renders/job-bits/bits_chunk_0000_t0000.bits":
                raise ClientError({"Error": {"Code": "NoSuchKey", "Message": "missing"}}, "GetObject")
            if key == "renders/job-bits/bits_s0000_t0000.bits":
                return {"Body": _Body(b"\x01\x02")}
            raise ClientError({"Error": {"Code": "NoSuchKey", "Message": "missing"}}, "GetObject")

        def upload_fileobj(fileobj, bucket, key):
            uploaded[key] = fileobj.read()

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            with open("/tmp/tile.tif", "wb") as fh:
                fh.write(b"II*\x00tile")
            return MagicMock(returncode=0, stdout=json.dumps({"pixels_set": 0, "file_size": 8}), stderr="")

        mock_s3.get_object.side_effect = get_object
        mock_s3.upload_fileobj.side_effect = upload_fileobj
        mock_run.side_effect = run_side_effect

        with tempfile.TemporaryDirectory():
            result = mod.handle_merge({
                "job_id": "job-bits",
                "task_id": "merge_0",
                "tile_idx": 0,
                "tile_w": 2,
                "tile_h": 2,
                "n_chunks": 1,
                "width": 2,
                "height": 2,
                "tile_size": 2,
                "n_tile_cols": 1,
            })

        body = json.loads(result["body"])
        self.assertEqual(body["pixels_set"], 0)
        self.assertIn("renders/job-bits/bilevel_t0000.tif", uploaded)
        self.assertIn("renders/job-bits/bits_chunk_0000_t0000.bits", requested_keys)
        self.assertNotIn("renders/job-bits/bits_s0000_t0000.bits", requested_keys)

    @patch("handler_bilevel.report_status")
    @patch("handler_bilevel.subprocess.run")
    @patch("handler_bilevel.stitch_spans_to_file")
    @patch("handler_bilevel.build_source_spans")
    @patch("handler_bilevel.s3")
    def test_section_raster_uploads_full_frame_section_bits(
        self,
        mock_s3,
        mock_build_spans,
        mock_stitch,
        mock_run,
        mock_report,
    ):
        import handler_bilevel as mod

        uploaded = {}
        mock_build_spans.return_value = [{"key": "renders/job/roots.bin", "offset": 0, "length": 32}]

        def stitch_side_effect(s3_client, bucket, spans, path):
            with open(path, "wb") as fh:
                fh.write(b"roots")

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None):
            with open("/tmp/section.bits", "wb") as fh:
                fh.write(b"\x0f")
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"roots_plotted": 4, "roots_clipped": 1, "roots_deduped": 2, "file_size": 1}),
                stderr="",
            )

        def put_object(**kwargs):
            body = kwargs["Body"]
            uploaded[kwargs["Key"]] = body.read() if hasattr(body, "read") else body

        mock_stitch.side_effect = stitch_side_effect
        mock_run.side_effect = run_side_effect
        mock_s3.put_object.side_effect = put_object

        with tempfile.TemporaryDirectory():
            result = mod.handle_section_raster({
                "job_id": "job",
                "task_id": "bilevel_section_0",
                "section_idx": 0,
                "section_count": 2,
                "step_start": 0,
                "step_count": 10,
                "solve_source_manifest": {"version": 1, "chunks": []},
                "section_bits_prefix": "renders/job/bilevel_section_",
                "width": 8,
                "height": 8,
                "center_re": 0,
                "center_im": 0,
                "scale": 1,
                "degree": 5,
                "rotation": 0,
                "root_transforms": [],
            })

        body = json.loads(result["body"])
        self.assertEqual(body["section_bits_key"], "renders/job/bilevel_section_0000.bits")
        self.assertIn("renders/job/bilevel_section_0000.bits", uploaded)
        self.assertEqual(uploaded["renders/job/bilevel_section_0000.bits"], b"\x0f")

    @patch("handler_bilevel.report_status")
    @patch("handler_bilevel._upload_file")
    @patch("handler_bilevel._download_to_path")
    @patch("handler_bilevel.subprocess.run")
    def test_finalize_assembles_section_bits_into_final_outputs(
        self,
        mock_run,
        mock_download,
        mock_upload,
        mock_report,
    ):
        import handler_bilevel as mod

        downloaded = []
        uploaded = []

        def download_side_effect(key, path):
            downloaded.append(key)
            with open(path, "wb") as fh:
                fh.write(b"\x00")

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            with open("/tmp/final.tif", "wb") as fh:
                fh.write(b"II*\x00final")
            with open("/tmp/final_preview.png", "wb") as fh:
                fh.write(b"\x89PNG")
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"pixels_set": 9, "file_size": 9}),
                stderr="",
            )

        def upload_side_effect(path, key, *, content_type, metadata=None):
            uploaded.append((path, key, content_type, dict(metadata or {})))

        mock_download.side_effect = download_side_effect
        mock_run.side_effect = run_side_effect
        mock_upload.side_effect = upload_side_effect

        with tempfile.TemporaryDirectory():
            result = mod.handle_finalize({
                "job_id": "job",
                "task_id": "bilevel_finalize",
                "width": 8,
                "height": 8,
                "source_item_count": 2,
                "section_bits_prefix": "renders/job/bilevel_section_",
                "out_key": "renders/job/bilevel/art/image.tif",
                "preview_key": "renders/job/bilevel/art/preview.png",
                "metadata": {
                    "artifact_id": "art",
                    "created_at": "2026-04-19T00:00:00Z",
                    "pix": "8",
                    "render_execution": {"raster_section_mode": "logical_sections_auto"},
                    "bilevel_section_mode": "logical_sections_auto",
                    "bilevel_section_count": "2",
                },
            })

        body = json.loads(result["body"])
        self.assertEqual(
            downloaded,
            [
                "renders/job/bilevel_section_0000.bits",
                "renders/job/bilevel_section_0001.bits",
            ],
        )
        self.assertEqual(body["out_key"], "renders/job/bilevel/art/image.tif")
        self.assertEqual(uploaded[0][1], "renders/job/bilevel/art/image.tif")
        self.assertEqual(uploaded[0][2], "image/tiff")
        self.assertEqual(uploaded[0][3]["bilevel_pipeline"], "logical_sections_v1")
        self.assertEqual(uploaded[1][1], "renders/job/bilevel/art/preview.png")
        self.assertEqual(uploaded[1][2], "image/png")

    @patch("handler_bilevel.report_status")
    @patch("handler_bilevel._upload_file")
    @patch("handler_bilevel._download_to_path")
    @patch("handler_bilevel._load_json_key")
    @patch("handler_bilevel.validate_raw_sidecar")
    @patch("handler_bilevel.load_color_artifact_head")
    @patch("handler_bilevel.subprocess.run")
    def test_from_raw_color_derives_bilevel_artifact(
        self,
        mock_run,
        mock_load_head,
        mock_validate,
        mock_load_json,
        mock_download,
        mock_upload,
        mock_report,
    ):
        import handler_bilevel as mod

        uploads = []
        mock_load_head.return_value = {
            "image_key": "renders/job/color/src/image.jpeg",
            "metadata": {
                "raw_key": "renders/job/color/src/greyscale.raw",
                "raw_meta_key": "renders/job/color/src/greyscale.meta.json",
                "pix": "64",
            },
        }
        mock_load_json.return_value = {"version": 2}
        mock_validate.return_value = {
            "width": 64,
            "height": 64,
            "raw_key": "renders/job/color/src/greyscale.raw",
            "artifact_family": "color",
        }

        def download_side_effect(key, path):
            with open(path, "wb") as fh:
                fh.write(b"\x00" * (64 * 64))

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            with open("/tmp/final.tif", "wb") as fh:
                fh.write(b"II*\x00final")
            with open("/tmp/final_preview.png", "wb") as fh:
                fh.write(b"\x89PNG")
            import time
            time.sleep(0.02)
            return MagicMock(returncode=0, stdout=json.dumps({"file_size": 9}), stderr="")

        def upload_side_effect(path, key, *, content_type, metadata=None):
            uploads.append((key, content_type, dict(metadata or {})))

        mock_download.side_effect = download_side_effect
        mock_run.side_effect = run_side_effect
        mock_upload.side_effect = upload_side_effect

        with tempfile.TemporaryDirectory():
            with patch.object(mod, "RAW_TO_BILEVEL_PROGRESS_INTERVAL_S", 0.001):
                result = mod.handle_from_raw_color({
                    "job_id": "job",
                    "task_id": "color_to_bilevel_run",
                    "artifact_id": "bil_1",
                    "source_artifact_id": "color_src",
                    "threshold": 17,
                })

        body = json.loads(result["body"])
        self.assertEqual(body["artifact_id"], "bil_1")
        self.assertEqual(body["threshold"], 17)
        self.assertEqual(uploads[0][0], "renders/job/bilevel/bil_1/image.tif")
        self.assertEqual(uploads[0][2]["postprocess_kind"], "color_to_bilevel")
        self.assertEqual(uploads[0][2]["threshold"], "17")
        self.assertEqual(uploads[1][0], "renders/job/bilevel/bil_1/preview.png")
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertIn("started", statuses)
        self.assertIn("source_ready", statuses)
        self.assertIn("raw_downloaded", statuses)
        self.assertIn("rendering", statuses)
        self.assertIn("rendered", statuses)
        self.assertIn("uploading", statuses)
        self.assertIn("done", statuses)
        phase_labels = [
            (call.kwargs.get("result_data") or {}).get("phase_label")
            for call in mock_report.call_args_list
            if call.kwargs.get("result_data")
        ]
        self.assertIn("Color2Bilevel: source", phase_labels)
        self.assertIn("Color2Bilevel: raw download", phase_labels)
        self.assertIn("Color2Bilevel: threshold + encode", phase_labels)
        self.assertIn("Color2Bilevel: upload", phase_labels)

    @patch("handler_bilevel.report_status")
    def test_from_raw_color_invalid_threshold_reports_error_context(self, mock_report):
        import handler_bilevel as mod

        with self.assertRaisesRegex(RuntimeError, "threshold must be an integer"):
            mod.handle_from_raw_color({
                "job_id": "job",
                "task_id": "color_to_bilevel_run",
                "artifact_id": "bil_1",
                "source_artifact_id": "color_src",
                "threshold": "abc",
            })

        error_call = mock_report.call_args
        self.assertIsNotNone(error_call)
        self.assertEqual(error_call.args[0], "job")
        self.assertEqual(error_call.args[1], "color_to_bilevel_run")
        self.assertEqual(error_call.args[2], "error")
        self.assertIn("threshold must be an integer", error_call.args[3])
        rd = error_call.kwargs.get("result_data") or {}
        self.assertEqual(rd.get("phase"), "bilevel_from_raw_prepare")
        self.assertEqual(rd.get("phase_label"), "Color2Bilevel: source")
        self.assertEqual(rd.get("artifact_id"), "bil_1")
        self.assertEqual(rd.get("source_artifact_id"), "color_src")
        self.assertEqual(rd.get("threshold_raw"), "abc")


if __name__ == "__main__":
    unittest.main()
