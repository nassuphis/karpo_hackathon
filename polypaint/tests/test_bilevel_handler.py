import json
import os
import sys
import tempfile
import unittest
from unittest.mock import MagicMock, patch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class _Body:
    def __init__(self, data):
        self._data = data

    def read(self):
        return self._data


def _encode_fragment_pairs(pairs):
    payload = bytearray()
    for pixel_idx, score in pairs:
        payload.extend(int(pixel_idx).to_bytes(4, "little", signed=False))
        payload.append(int(score) & 0xFF)
    return bytes(payload)


def _bounds_from_center_scale(width, height, center_re, center_im, scale):
    half_w_world = (float(width) / 2.0) / float(scale)
    half_h_world = (float(height) / 2.0) / float(scale)
    return {
        "min_re": center_re - half_w_world,
        "max_re": center_re + half_w_world,
        "min_im": center_im - half_h_world,
        "max_im": center_im + half_h_world,
    }


class TestBilevelHandler(unittest.TestCase):
    @patch("handler_bilevel.report_status")
    def test_handler_reports_malformed_json_when_job_id_present(self, mock_report):
        import handler_bilevel as mod

        with self.assertRaisesRegex(RuntimeError, "could not parse request body"):
            mod.handler({
                "body": '{"job_id":"job","task_id":"bilevel_bad_json","phase":"from_raw_color"',
            }, None)

        mock_report.assert_called_once()
        self.assertEqual(mock_report.call_args.args[0], "job")
        self.assertEqual(mock_report.call_args.args[1], "bilevel_bad_json")
        self.assertEqual(mock_report.call_args.args[2], "error")
        self.assertIn("could not parse request body", mock_report.call_args.args[3])
        rd = mock_report.call_args.kwargs.get("result_data") or {}
        self.assertEqual(rd.get("phase"), "handler_entry")
        self.assertEqual(rd.get("phase_label"), "BiLevel dispatch")
        self.assertEqual(rd.get("phase_raw"), "from_raw_color")

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
    def test_handler_section_raster_reports_malformed_payload(self, mock_report):
        import handler_bilevel as mod

        with self.assertRaisesRegex(RuntimeError, "requires fragment_prefix"):
            mod.handler({"body": json.dumps({
                "phase": "section_raster",
                "job_id": "job",
                "task_id": "bilevel_section_bad",
                "section_idx": 0,
                "step_start": 0,
                "step_count": 1,
                "solve_source_manifest": {"v": 2},
                "pix": 8,
                **_bounds_from_center_scale(8, 8, 0, 0, 1),
                "degree": 2,
            })}, None)

        mock_report.assert_called_once()
        self.assertEqual(mock_report.call_args.args[0], "job")
        self.assertEqual(mock_report.call_args.args[1], "bilevel_section_bad")
        self.assertEqual(mock_report.call_args.args[2], "error")
        self.assertIn("requires fragment_prefix", mock_report.call_args.args[3])
        rd = mock_report.call_args.kwargs.get("result_data") or {}
        self.assertEqual(rd.get("phase"), "bilevel_raster")
        self.assertEqual(rd.get("phase_label"), "BiLevel raster")
        self.assertEqual(rd.get("section_idx"), "0")
        self.assertEqual(rd.get("step_count"), "1")

    @patch("handler_bilevel.report_status")
    def test_handler_section_raster_requires_section_idx(self, mock_report):
        import handler_bilevel as mod

        with self.assertRaisesRegex(RuntimeError, "requires section_idx"):
            mod.handler({"body": json.dumps({
                "phase": "section_raster",
                "job_id": "job",
                "task_id": "bilevel_section_bad",
                "step_start": 0,
                "step_count": 1,
                "solve_source_manifest": {"v": 2},
                "fragment_prefix": "renders/job/bilevel_section_",
                "pix": 8,
                **_bounds_from_center_scale(8, 8, 0, 0, 1),
                "degree": 2,
            })}, None)

        mock_report.assert_called_once()
        self.assertEqual(mock_report.call_args.args[0], "job")
        self.assertEqual(mock_report.call_args.args[1], "bilevel_section_bad")
        self.assertEqual(mock_report.call_args.args[2], "error")
        self.assertIn("requires section_idx", mock_report.call_args.args[3])
        rd = mock_report.call_args.kwargs.get("result_data") or {}
        self.assertEqual(rd.get("phase"), "bilevel_raster")
        self.assertEqual(rd.get("phase_label"), "BiLevel raster")
        self.assertEqual(rd.get("step_count"), "1")

    @patch("handler_bilevel.report_status")
    def test_handler_section_raster_requires_step_start(self, mock_report):
        import handler_bilevel as mod

        with self.assertRaisesRegex(RuntimeError, "requires step_start"):
            mod.handler({"body": json.dumps({
                "phase": "section_raster",
                "job_id": "job",
                "task_id": "bilevel_section_bad",
                "section_idx": 0,
                "step_count": 1,
                "solve_source_manifest": {"v": 2},
                "fragment_prefix": "renders/job/bilevel_section_",
                "pix": 8,
                **_bounds_from_center_scale(8, 8, 0, 0, 1),
                "degree": 2,
            })}, None)

        mock_report.assert_called_once()
        self.assertEqual(mock_report.call_args.args[0], "job")
        self.assertEqual(mock_report.call_args.args[1], "bilevel_section_bad")
        self.assertEqual(mock_report.call_args.args[2], "error")
        self.assertIn("requires step_start", mock_report.call_args.args[3])
        rd = mock_report.call_args.kwargs.get("result_data") or {}
        self.assertEqual(rd.get("phase"), "bilevel_raster")
        self.assertEqual(rd.get("phase_label"), "BiLevel raster")
        self.assertEqual(rd.get("section_idx"), "0")

    @patch("handler_bilevel.report_status")
    def test_handler_finalize_reports_malformed_payload(self, mock_report):
        import handler_bilevel as mod

        with self.assertRaisesRegex(RuntimeError, "requires fragment_prefix, out_key, and preview_key"):
            mod.handler({"body": json.dumps({
                "phase": "finalize",
                "job_id": "job",
                "task_id": "bilevel_finalize_bad",
                "pix": 8,
                "source_item_count": 2,
                "fragment_prefix": "renders/job/bilevel_section_",
                "preview_key": "renders/job/bilevel/art/preview.png",
            })}, None)

        mock_report.assert_called_once()
        self.assertEqual(mock_report.call_args.args[0], "job")
        self.assertEqual(mock_report.call_args.args[1], "bilevel_finalize_bad")
        self.assertEqual(mock_report.call_args.args[2], "error")
        self.assertIn("requires fragment_prefix, out_key, and preview_key", mock_report.call_args.args[3])
        rd = mock_report.call_args.kwargs.get("result_data") or {}
        self.assertEqual(rd.get("phase"), "bilevel_finalize")
        self.assertEqual(rd.get("phase_label"), "Assemble + encode")
        self.assertEqual(rd.get("source_item_count"), "2")

    @patch("handler_bilevel.report_status")
    def test_handler_coeff_raster_reports_malformed_payload(self, mock_report):
        import handler_bilevel as mod

        with self.assertRaisesRegex(RuntimeError, "requires section_idx"):
            mod.handler({"body": json.dumps({
                "phase": "coeff_raster",
                "job_id": "job",
                "task_id": "coeff_bilevel_raster_bad",
            })}, None)

        mock_report.assert_called_once()
        self.assertEqual(mock_report.call_args.args[0], "job")
        self.assertEqual(mock_report.call_args.args[1], "coeff_bilevel_raster_bad")
        self.assertEqual(mock_report.call_args.args[2], "error")
        self.assertIn("requires section_idx", mock_report.call_args.args[3])
        rd = mock_report.call_args.kwargs.get("result_data") or {}
        self.assertEqual(rd.get("phase"), "coeff_bilevel_raster")
        self.assertEqual(rd.get("phase_label"), "Coeffs raster")

    @patch("handler_bilevel.report_status")
    def test_handler_from_raw_color_reports_malformed_payload(self, mock_report):
        import handler_bilevel as mod

        with self.assertRaisesRegex(RuntimeError, "requires artifact_id"):
            mod.handler({"body": json.dumps({
                "phase": "from_raw_color",
                "job_id": "job",
                "task_id": "color_to_bilevel_bad",
            })}, None)

        mock_report.assert_called_once()
        self.assertEqual(mock_report.call_args.args[0], "job")
        self.assertEqual(mock_report.call_args.args[1], "color_to_bilevel_bad")
        self.assertEqual(mock_report.call_args.args[2], "error")
        self.assertIn("requires artifact_id", mock_report.call_args.args[3])
        rd = mock_report.call_args.kwargs.get("result_data") or {}
        self.assertEqual(rd.get("phase"), "bilevel_from_raw_prepare")
        self.assertEqual(rd.get("phase_label"), "Color2Bilevel: source")

    @patch("handler_bilevel.report_status")
    @patch("handler_bilevel.subprocess.run")
    @patch("handler_bilevel.stitch_spans_to_file")
    @patch("handler_bilevel.build_source_spans")
    @patch("handler_bilevel.s3")
    def test_section_raster_uploads_sparse_section_fragment(
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
            with open("/tmp/section.frag", "wb") as fh:
                fh.write(_encode_fragment_pairs([(3, 1)]))
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"roots_plotted": 4, "roots_clipped": 1, "roots_deduped": 2, "file_size": 5}),
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
                "fragment_prefix": "renders/job/bilevel_section_",
                "pix": 8,
                **_bounds_from_center_scale(8, 8, 0, 0, 1),
                "degree": 5,
                "rotation": 0,
                "root_transforms": [],
            })

        body = json.loads(result["body"])
        self.assertEqual(body["fragment_key"], "renders/job/bilevel_section_0000.frag")
        self.assertIn("renders/job/bilevel_section_0000.frag", uploaded)
        self.assertEqual(uploaded["renders/job/bilevel_section_0000.frag"], _encode_fragment_pairs([(3, 1)]))

    @patch("handler_bilevel.report_status")
    @patch("handler_bilevel.subprocess.run")
    @patch("handler_bilevel.stitch_spans_to_file")
    @patch("handler_bilevel.build_source_spans")
    @patch("handler_bilevel.s3")
    def test_coeff_raster_uploads_sparse_section_fragment(
        self,
        mock_s3,
        mock_build_spans,
        mock_stitch,
        mock_run,
        mock_report,
    ):
        import handler_bilevel as mod

        uploaded = {}
        mock_build_spans.return_value = [{"key": "renders/job/coeffs.bin", "offset": 0, "length": 48}]

        def stitch_side_effect(s3_client, bucket, spans, path):
            with open(path, "wb") as fh:
                fh.write(b"coeffs")

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None):
            with open("/tmp/section.frag", "wb") as fh:
                fh.write(_encode_fragment_pairs([(4, 1)]))
            return MagicMock(
                returncode=0,
                stdout=json.dumps({
                    "coeffs_plotted": 3,
                    "coeffs_clipped": 1,
                    "coeffs_deduped": 2,
                    "pixels_set": 1,
                    "file_size": 5,
                }),
                stderr="",
            )

        def put_object(**kwargs):
            body = kwargs["Body"]
            uploaded[kwargs["Key"]] = body.read() if hasattr(body, "read") else body

        mock_stitch.side_effect = stitch_side_effect
        mock_run.side_effect = run_side_effect
        mock_s3.put_object.side_effect = put_object

        with tempfile.TemporaryDirectory():
            result = mod.handle_coeff_raster({
                "job_id": "job",
                "task_id": "coeff_bilevel_section_0",
                "section_idx": 0,
                "section_count": 2,
                "step_start": 0,
                "step_count": 10,
                "solve_source_manifest": {"v": 2, "s": {"cf": {"r": 48, "k": ["renders/job/coeffs.bin"], "g": [[0, 0, 10, 0]]}}},
                "fragment_prefix": "renders/job/coeff_bilevel_section_",
                "pix": 8,
                **_bounds_from_center_scale(8, 8, 0, 0, 1),
                "degree": 5,
                "n_coeffs": 6,
                "rotation": 0,
            })

        body = json.loads(result["body"])
        self.assertEqual(body["fragment_key"], "renders/job/coeff_bilevel_section_0000.frag")
        self.assertEqual(body["coeffs_plotted"], 3)
        self.assertIn("renders/job/coeff_bilevel_section_0000.frag", uploaded)
        self.assertEqual(uploaded["renders/job/coeff_bilevel_section_0000.frag"], _encode_fragment_pairs([(4, 1)]))

    @patch("handler_bilevel.report_status")
    @patch("handler_bilevel._upload_file")
    @patch("handler_bilevel._finalize_s3_client")
    @patch("handler_bilevel.subprocess.run")
    def test_finalize_assembles_sparse_fragments_into_final_outputs(
        self,
        mock_run,
        mock_finalize_s3_client,
        mock_upload,
        mock_report,
    ):
        import handler_bilevel as mod

        uploaded = []

        class _FakeFinalizeS3:
            def generate_presigned_url(self, op, Params=None, ExpiresIn=None):
                self.last = (op, dict(Params or {}), ExpiresIn)
                return f"http://example.invalid/{Params['Key']}"

        fake_finalize_s3 = _FakeFinalizeS3()
        mock_finalize_s3_client.return_value = fake_finalize_s3

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "assemble_greyscale":
                self.assertTrue(any(str(arg).startswith("--url-manifest=") for arg in cmd))
                with open("/tmp/final_bilevel.raw", "wb") as fh:
                    fh.write(b"\x01" * 64)
                with open("/tmp/final_bilevel.hist.json", "w", encoding="utf-8") as fh:
                    json.dump({"histogram": [0, 64] + [0] * 254, "background_pixels": 0, "nonzero_pixels": 64}, fh)
                return MagicMock(returncode=0, stdout="", stderr="")
            if exe == "raw_to_bilevel":
                with open("/tmp/final.tif", "wb") as fh:
                    fh.write(b"II*\x00final")
                with open("/tmp/final_preview.png", "wb") as fh:
                    fh.write(b"\x89PNG")
                return MagicMock(
                    returncode=0,
                    stdout=json.dumps({"file_size": 9}),
                    stderr="",
                )
            raise AssertionError(f"unexpected executable {exe}")

        def upload_side_effect(path, key, *, content_type, metadata=None):
            uploaded.append((path, key, content_type, dict(metadata or {})))

        mock_run.side_effect = run_side_effect
        mock_upload.side_effect = upload_side_effect

        with tempfile.TemporaryDirectory():
            result = mod.handle_finalize({
                "job_id": "job",
                "task_id": "bilevel_finalize",
                "pix": 8,
                "source_item_count": 2,
                "fragment_prefix": "renders/job/bilevel_section_",
                "out_key": "renders/job/bilevel/art/image.tif",
                "preview_key": "renders/job/bilevel/art/preview.png",
                "assemble_workers": 3,
                "metadata": {
                    "artifact_id": "art",
                    "created_at": "2026-04-19T00:00:00Z",
                    "degree": "7",
                    "pix": "8",
                    "rotation": "-0.25",
                    "root_transforms": '[["unit_circle"]]',
                    "render_execution": {"raster_section_mode": "logical_sections_auto"},
                    "bilevel_section_mode": "logical_sections_auto",
                    "bilevel_section_count": "2",
                },
            })

        body = json.loads(result["body"])
        self.assertGreaterEqual(body["prep_ms"], 0)
        self.assertEqual(body["workers"], mod.DEFAULT_BILEVEL_FINALIZE_WORKERS)
        self.assertEqual(body["out_key"], "renders/job/bilevel/art/image.tif")
        self.assertEqual(uploaded[0][1], "renders/job/bilevel/art/image.tif")
        self.assertEqual(uploaded[0][2], "image/tiff")
        self.assertEqual(uploaded[0][3]["bilevel_pipeline"], mod.BILEVEL_SPARSE_PIPELINE)
        self.assertEqual(uploaded[0][3]["degree"], "7")
        self.assertEqual(uploaded[0][3]["rotation"], "-0.25")
        self.assertEqual(uploaded[0][3]["root_transforms"], '[["unit_circle"]]')
        self.assertEqual(uploaded[0][3]["render_execution"], '{"raster_section_mode":"logical_sections_auto"}')
        self.assertEqual(uploaded[1][1], "renders/job/bilevel/art/preview.png")
        self.assertEqual(uploaded[1][2], "image/png")
        self.assertEqual(fake_finalize_s3.last[0], "get_object")
        self.assertEqual(fake_finalize_s3.last[1]["Key"], "renders/job/bilevel_section_0001.frag")
        self.assertEqual(fake_finalize_s3.last[2], mod.FRAGMENT_URL_EXPIRES_S)
        done_rows = [
            call.kwargs.get("result_data") or {}
            for call in mock_report.call_args_list
            if len(call.args) >= 3 and call.args[2] == "done"
        ]
        self.assertTrue(done_rows)
        self.assertEqual(done_rows[-1].get("file_size"), 9)

    @patch("handler_bilevel.report_status")
    @patch("handler_bilevel._finalize_s3_client")
    @patch("handler_bilevel._assemble_sparse_bilevel_raw")
    def test_finalize_requires_raw_output_after_assemble(
        self,
        mock_assemble,
        mock_finalize_s3_client,
        mock_report,
    ):
        import handler_bilevel as mod

        class _FakeFinalizeS3:
            def generate_presigned_url(self, op, Params=None, ExpiresIn=None):
                return f"http://example.invalid/{Params['Key']}"

        mock_finalize_s3_client.return_value = _FakeFinalizeS3()
        mock_assemble.return_value = {
            "histogram": [0] * 256,
            "background_pixels": 64,
            "nonzero_pixels": 0,
        }

        with tempfile.TemporaryDirectory():
            with self.assertRaisesRegex(RuntimeError, "did not produce final raw output"):
                mod.handle_finalize({
                    "job_id": "job",
                    "task_id": "bilevel_finalize",
                    "pix": 8,
                    "source_item_count": 2,
                    "fragment_prefix": "renders/job/bilevel_section_",
                    "out_key": "renders/job/bilevel/art/image.tif",
                    "preview_key": "renders/job/bilevel/art/preview.png",
                    "metadata": {"artifact_id": "art", "pix": "8"},
                })

    @patch("handler_bilevel.report_status")
    @patch("handler_bilevel._upload_file")
    @patch("handler_bilevel._finalize_s3_client")
    @patch("handler_bilevel.subprocess.run")
    def test_finalize_preserves_coeff_family_metadata(
        self,
        mock_run,
        mock_finalize_s3_client,
        mock_upload,
        mock_report,
    ):
        import handler_bilevel as mod

        class _FakeFinalizeS3:
            def generate_presigned_url(self, op, Params=None, ExpiresIn=None):
                return f"http://example.invalid/{Params['Key']}"

        mock_finalize_s3_client.return_value = _FakeFinalizeS3()
        uploaded = []

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "assemble_greyscale":
                with open("/tmp/final_bilevel.raw", "wb") as fh:
                    fh.write(b"\x01" * 64)
                with open("/tmp/final_bilevel.hist.json", "w", encoding="utf-8") as fh:
                    json.dump({"histogram": [0, 64] + [0] * 254, "background_pixels": 0, "nonzero_pixels": 64}, fh)
                return MagicMock(returncode=0, stdout="", stderr="")
            if exe == "raw_to_bilevel":
                with open("/tmp/final.tif", "wb") as fh:
                    fh.write(b"II*\x00final")
                with open("/tmp/final_preview.png", "wb") as fh:
                    fh.write(b"\x89PNG")
                return MagicMock(returncode=0, stdout=json.dumps({"file_size": 9}), stderr="")
            raise AssertionError(f"unexpected executable {exe}")

        def upload_side_effect(path, key, *, content_type, metadata=None):
            uploaded.append((path, key, content_type, dict(metadata or {})))

        mock_run.side_effect = run_side_effect
        mock_upload.side_effect = upload_side_effect

        with tempfile.TemporaryDirectory():
            mod.handle_finalize({
                "job_id": "job",
                "task_id": "coeff_bilevel_finalize",
                "pix": 8,
                "source_item_count": 1,
                "fragment_prefix": "renders/job/coeff_bilevel_section_",
                "out_key": "renders/job/coeffs/art/image.tif",
                "preview_key": "renders/job/coeffs/art/preview.png",
                "metadata": {
                    "artifact_id": "coeffs_1",
                    "family": "coeffs",
                    "mode": "coeffs",
                    "pix": "8",
                    "bilevel_pipeline": mod.BILEVEL_SPARSE_PIPELINE,
                    "bilevel_section_mode": "logical_sections_auto",
                    "bilevel_section_count": "1",
                },
            })

        self.assertEqual(uploaded[0][1], "renders/job/coeffs/art/image.tif")
        self.assertEqual(uploaded[0][3]["family"], "coeffs")
        self.assertEqual(uploaded[0][3]["mode"], "coeffs")
        self.assertEqual(uploaded[0][3]["bilevel_pipeline"], mod.BILEVEL_SPARSE_PIPELINE)
        done_rows = [
            call.kwargs.get("result_data") or {}
            for call in mock_report.call_args_list
            if len(call.args) >= 3 and call.args[2] == "done"
        ]
        self.assertEqual(done_rows[-1].get("phase"), "coeff_bilevel_finalize")

    @patch("handler_bilevel.report_status")
    @patch("handler_bilevel._upload_file")
    @patch("handler_bilevel._finalize_s3_client")
    @patch("handler_bilevel.subprocess.run")
    def test_finalize_rejects_oversized_upload_metadata(
        self,
        mock_run,
        mock_finalize_s3_client,
        mock_upload,
        mock_report,
    ):
        import handler_bilevel as mod

        class _FakeFinalizeS3:
            def generate_presigned_url(self, op, Params=None, ExpiresIn=None):
                return f"http://example.invalid/{Params['Key']}"

        mock_finalize_s3_client.return_value = _FakeFinalizeS3()

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "assemble_greyscale":
                with open("/tmp/final_bilevel.raw", "wb") as fh:
                    fh.write(b"\x01" * 64)
                with open("/tmp/final_bilevel.hist.json", "w", encoding="utf-8") as fh:
                    json.dump({"histogram": [0, 64] + [0] * 254, "background_pixels": 0, "nonzero_pixels": 64}, fh)
                return MagicMock(returncode=0, stdout="", stderr="")
            if exe == "raw_to_bilevel":
                with open("/tmp/final.tif", "wb") as fh:
                    fh.write(b"II*\x00final")
                with open("/tmp/final_preview.png", "wb") as fh:
                    fh.write(b"\x89PNG")
                return MagicMock(returncode=0, stdout=json.dumps({"file_size": 9}), stderr="")
            raise AssertionError(f"unexpected executable {exe}")

        mock_run.side_effect = run_side_effect

        with tempfile.TemporaryDirectory():
            with self.assertRaisesRegex(RuntimeError, "metadata root_transforms exceeds"):
                mod.handle_finalize({
                    "job_id": "job",
                    "task_id": "bilevel_finalize",
                    "pix": 8,
                    "source_item_count": 2,
                    "fragment_prefix": "renders/job/bilevel_section_",
                    "out_key": "renders/job/bilevel/art/image.tif",
                    "preview_key": "renders/job/bilevel/art/preview.png",
                    "metadata": {
                        "artifact_id": "art",
                        "pix": "8",
                        "root_transforms": "x" * 513,
                    },
                })
        mock_upload.assert_not_called()

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
                "view_mode": "explicit",
                "min_re": "-3.5",
                "max_re": "1.25",
                "min_im": "-0.75",
                "max_im": "2.0",
                "rotation": "0.125",
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
        self.assertEqual(uploads[0][2]["view_mode"], "explicit")
        self.assertEqual(uploads[0][2]["min_re"], "-3.5")
        self.assertEqual(uploads[0][2]["max_re"], "1.25")
        self.assertEqual(uploads[0][2]["min_im"], "-0.75")
        self.assertEqual(uploads[0][2]["max_im"], "2.0")
        self.assertEqual(uploads[0][2]["rotation"], "0.125")
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
