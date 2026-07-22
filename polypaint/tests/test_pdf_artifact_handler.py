import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(**overrides):
    payload = {
        "job_id": "job1",
        "task_id": "pdf_run_1",
        "artifact_id": "pdf_123",
        "source_artifact_id": "color_src",
        "source_image_key": "renders/job1/color/color_src/image.jpeg",
    }
    payload.update(overrides)
    return payload


class TestPdfArtifactHandler(unittest.TestCase):

    def test_rejects_cross_job_source_key(self):
        # code-review-27 F5: a source key from another job must be refused
        # BEFORE any S3 head/get
        from handler_pdf_artifact import handler
        with self.assertRaises(ValueError):
            handler(_event(source_image_key="renders/OTHERJOB/color/color_src/image.jpeg"), None)


    def test_report_model_prefers_stored_solve_score_source_text(self):
        from handler_pdf_artifact import build_pdf_report_model

        authored = "a = metric(spread, cf, q=0.1%)\nemit_norm(flip(a))"
        report = build_pdf_report_model(
            "job1",
            {"function": "poly_1", "degree": 35},
            {
                "family": "color",
                "color_mode": "solve_score",
                "solve_score_program_source_text": authored,
                "solve_score_chain": json.dumps([["spread", "cf", "0.1"], ["emit", "norm"]]),
                "score_program": "v2;m0-0;emit_norm",
            },
            "color_src",
        )

        programs = {p["label"]: p for p in report["programs"]}
        self.assertIn("Param Program", programs)
        self.assertEqual(programs["Param Program"]["source"], "none")
        self.assertEqual(programs["Solve Score Program"]["source"], authored)
        self.assertIn("Coeff Program", programs)
        self.assertIn("Root Program", programs)
        self.assertEqual(programs["Root Program"]["source"], "none")

    def test_report_model_reconstructs_editable_program_sources(self):
        from handler_pdf_artifact import build_pdf_report_model
        from coeff_program_chain import compile_coeff_program_chain
        from coeff_program_source import compile_coeff_program_source

        report = build_pdf_report_model(
            "job1",
            {
                "pipeline": {
                    "function": "poly_1",
                    "param_transforms": [["unit_circle"]],
                    "coeff_transforms": ["rev"],
                    # This is the lowered display string old artifacts may
                    # carry; the PDF must prefer source reconstruction.
                    "coeff_program_display": "_native_transform(rev, poly, poly)",
                },
                "degree": 35,
            },
            {
                "family": "color",
                "color_mode": "solve_score",
                "root_transforms": '[["rotate_roots","0.25"]]',
                "solve_metric": "clusteriness",
                "solve_score_quantile": "0.047",
            },
            "color_src",
        )

        programs = {p["label"]: p for p in report["programs"]}
        self.assertIn("unit_circle", programs["Param Program"]["source"])
        coeff_source = programs["Coeff Program"]["source"]
        self.assertNotIn("_native_transform", programs["Coeff Program"]["source"])
        self.assertEqual(
            compile_coeff_program_source(coeff_source)["fingerprint"],
            compile_coeff_program_chain([["legacy", "rev", "poly", "poly"], ["emit"]])["fingerprint"],
        )
        self.assertIn("rotate_roots", programs["Root Program"]["fallback"])
        self.assertEqual(
            programs["Solve Score Program"]["source"],
            "push(metric(clusteriness, slv, q=4.7%))\nomega_cosine(1)\nscore = pop()",
        )

    @patch("handler_pdf_artifact.report_status")
    @patch("handler_pdf_artifact.prepare_pdf_image")
    @patch("handler_pdf_artifact.build_color_spread_pdf")
    @patch("handler_pdf_artifact.s3")
    def test_handler_uploads_pdf_artifact_with_metadata(self, mock_s3, mock_build, mock_prepare, mock_report):
        from handler_pdf_artifact import handler

        mock_s3.head_object.return_value = {
            "Metadata": {
                "family": "color",
                "artifact_id": "color_src",
                "created_at": "2026-04-04T10:00:00Z",
                "color_mode": "solve_score",
                "palette": "tri_redgold",
                "solve_score_chain": json.dumps([["spread", "0.1"], ["omega_cosine", "3", "0.5"]]),
                "solve_metric": "clusteriness",
                "solve_score_quantile": "0.05",
                "solve_score_omega": "3",
                "associated_palette_mode": "generated",
                "associated_palette_id": "pal_color_src",
                "associated_palette_image_key": "renders/job1/palettes/pal_color_src/image.jpeg",
                "root_transforms": '[["rotate_roots","0.25"]]',
            }
        }

        def get_object(Bucket=None, Key=None):
            if Key == "renders/job1/color/color_src/image.jpeg":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"jpeg-bytes"])}
            if Key == "renders/job1/palettes/pal_color_src/image.jpeg":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"palette-jpeg"])}
            if Key == "renders/job1/calc.json":
                return {"Body": MagicMock(read=lambda: json.dumps({
                    "function": "poly_645",
                    "degree": 24,
                    "N": 500,
                    "times": 3,
                    "solver": "aberth",
                }).encode())}
            raise AssertionError(f"unexpected get_object key: {Key}")

        uploads = {}

        def upload_fileobj(fileobj, bucket, key, ExtraArgs=None):
            uploads[key] = {
                "body": fileobj.read(),
                "extra": ExtraArgs or {},
            }

        mock_prepare.side_effect = [
            {
                "source_width": 10000,
                "source_height": 10000,
                "prepared_width": 5000,
                "prepared_height": 5000,
                "resized": True,
                "image_max_px": 5000,
                "prepared_path": "/tmp/pdf_source_prepared.jpg",
                "prepared_format": "jpeg",
            },
            {
                "source_width": 1000,
                "source_height": 1000,
                "prepared_width": 800,
                "prepared_height": 800,
                "resized": True,
                "image_max_px": 800,
                "prepared_path": "/tmp/pdf_palette_prepared.jpg",
                "prepared_format": "jpeg",
            },
        ]

        def fake_build(image_path, output_path, title, body=None, filename=None, meta=None, palette_image_path=None, report=None):
            self.assertTrue(str(image_path).endswith("pdf_source_prepared.jpg"))
            self.assertTrue(str(palette_image_path).endswith("pdf_palette_prepared.jpg"))
            self.assertEqual(title, "job1 / color_src")
            self.assertIsNone(meta)
            self.assertIsNotNone(report)
            self.assertEqual(report["compute_id"], "job1")
            self.assertEqual(report["color_artifact_id"], "color_src")
            self.assertEqual(report["palette_label"], "pal_color_src")
            self.assertIn(("Solver", "AE"), report["summary_rows"])
            self.assertNotIn("Color mode", [label for label, _value in report["summary_rows"]])
            self.assertFalse(any(p["label"] == "Coefficient Function" for p in report["programs"]))
            self.assertTrue(any(
                p["label"] == "Coeff Program" and p["source"] == "none"
                for p in report["programs"]
            ))
            self.assertTrue(any(
                p["label"] == "Root Program" and p["fallback"] == "rotate_roots(0.25)"
                for p in report["programs"]
            ))
            score_programs = [p for p in report["programs"] if p["label"] == "Solve Score Program"]
            self.assertEqual(len(score_programs), 1)
            self.assertIn("push(metric(spread, slv, q=0.1%))", score_programs[0]["source"])
            self.assertIn("omega_cosine(3, 0.5)", score_programs[0]["source"])
            self.assertIn("score = pop()", score_programs[0]["source"])
            self.assertNotIn("m0", score_programs[0]["source"])
            with open(output_path, "wb") as fh:
                fh.write(b"%PDF-1.4 fake pdf")
            return {"path": output_path, "page_count": 3}

        mock_s3.get_object.side_effect = get_object
        mock_s3.upload_fileobj.side_effect = upload_fileobj
        mock_build.side_effect = fake_build

        result = handler(_event(), None)
        body = json.loads(result["body"])

        self.assertEqual(result["statusCode"], 200)
        self.assertEqual(body["family"], "pdf")
        self.assertEqual(body["artifact_id"], "pdf_123")
        self.assertEqual(body["image_key"], "renders/job1/pdf/pdf_123/document.pdf")

        pdf_key = "renders/job1/pdf/pdf_123/document.pdf"
        self.assertIn(pdf_key, uploads)
        extra = uploads[pdf_key]["extra"]
        self.assertEqual(extra["ContentType"], "application/pdf")
        meta = extra["Metadata"]
        self.assertEqual(meta["family"], "pdf")
        self.assertEqual(meta["artifact_id"], "pdf_123")
        self.assertEqual(meta["pdf_kind"], "color_spread")
        self.assertEqual(meta["source_family"], "color")
        self.assertEqual(meta["source_artifact_id"], "color_src")
        self.assertEqual(meta["source_image_key"], "renders/job1/color/color_src/image.jpeg")
        self.assertIn("omega-cos", meta["source_display_name"])
        self.assertNotIn("ω", meta["source_display_name"])
        self.assertEqual(meta["source_palette"], "tri_redgold")
        self.assertEqual(meta["source_solve_metric"], "spread")
        self.assertEqual(meta["source_associated_palette_mode"], "generated")
        self.assertEqual(meta["source_associated_palette_id"], "pal_color_src")
        self.assertEqual(meta["source_associated_palette_image_key"], "renders/job1/palettes/pal_color_src/image.jpeg")
        self.assertEqual(meta["page_count"], "3")
        self.assertEqual(meta["width_mm"], "586")
        self.assertEqual(meta["height_mm"], "296")
        self.assertEqual(meta["function"], "poly_645")
        self.assertEqual(meta["degree"], "24")
        self.assertEqual(meta["N"], "500")
        self.assertEqual(meta["times"], "3")
        self.assertEqual(meta["source_width"], "10000")
        self.assertEqual(meta["source_height"], "10000")
        self.assertEqual(meta["prepared_width"], "5000")
        self.assertEqual(meta["prepared_height"], "5000")
        self.assertEqual(meta["image_resized"], "true")
        self.assertEqual(meta["image_max_px"], "5000")
        self.assertEqual(meta["palette_source_width"], "1000")
        self.assertEqual(meta["palette_prepared_width"], "800")
        for key, value in meta.items():
            key.encode("ascii")
            value.encode("ascii")

        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "downloading", "processing", "processing", "processing", "uploading", "done"])
        prepare_rows = [
            call.kwargs["result_data"]
            for call in mock_report.call_args_list
            if call.kwargs.get("result_data", {}).get("phase") == "prepare_image"
        ]
        self.assertTrue(any(row.get("prepared_width") == 5000 for row in prepare_rows))
        self.assertEqual(mock_prepare.call_args_list[0].kwargs, {
            "max_px": 5000,
            "quality": 92,
            "image_format": "jpeg",
        })
        self.assertEqual(mock_prepare.call_args_list[1].kwargs, {
            "max_px": 800,
            "quality": 92,
            "image_format": "jpeg",
        })

    @patch("handler_pdf_artifact.report_status")
    @patch("handler_pdf_artifact.s3")
    def test_handler_rejects_non_color_source(self, mock_s3, mock_report):
        from handler_pdf_artifact import handler

        mock_s3.head_object.return_value = {"Metadata": {"family": "palette", "artifact_id": "pal_src"}}

        with self.assertRaisesRegex(RuntimeError, "ColorSpread requires Color source"):
            handler(_event(source_artifact_id="pal_src", source_image_key="renders/job1/palettes/pal_src/image.jpeg"), None)

    @patch("handler_pdf_artifact.report_status")
    @patch("handler_pdf_artifact.prepare_pdf_image")
    @patch("handler_pdf_artifact.build_color_spread_pdf")
    @patch("handler_pdf_artifact.s3")
    def test_handler_reads_associated_palette_from_color_sidecar_overlay(self, mock_s3, mock_build, mock_prepare, mock_report):
        from handler_pdf_artifact import handler

        mock_s3.head_object.return_value = {
            "Metadata": {
                "family": "color",
                "artifact_id": "color_src",
                "created_at": "2026-04-04T10:00:00Z",
                "color_mode": "solve_score",
                "palette": "magma",
                "solve_metric": "spread",
                "solve_score_quantile": "0.02",
                "solve_score_omega": "6",
                "root_transforms": "[]",
            }
        }

        def get_object(Bucket=None, Key=None):
            if Key == "renders/job1/color/color_src/meta.json":
                return {"Body": MagicMock(read=lambda: json.dumps({
                    "associated_palette_mode": "generated",
                    "associated_palette_id": "pal_sidecar",
                    "associated_palette_image_key": "renders/job1/palettes/pal_sidecar/image.jpeg",
                }).encode())}
            if Key == "renders/job1/color/color_src/image.jpeg":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"jpeg-bytes"])}
            if Key == "renders/job1/palettes/pal_sidecar/image.jpeg":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"palette-jpeg"])}
            if Key == "renders/job1/calc.json":
                return {"Body": MagicMock(read=lambda: json.dumps({
                    "function": "poly_645",
                    "degree": 24,
                    "N": 500,
                    "times": 3,
                    "solver": "aberth",
                }).encode())}
            raise AssertionError(f"unexpected get_object key: {Key}")

        uploads = {}

        def upload_fileobj(fileobj, bucket, key, ExtraArgs=None):
            uploads[key] = {"body": fileobj.read(), "extra": ExtraArgs or {}}

        mock_prepare.side_effect = [
            {
                "source_width": 100,
                "source_height": 100,
                "prepared_width": 100,
                "prepared_height": 100,
                "resized": False,
                "image_max_px": 5000,
                "prepared_path": "/tmp/pdf_source_prepared.jpg",
                "prepared_format": "jpeg",
            },
            {
                "source_width": 64,
                "source_height": 64,
                "prepared_width": 64,
                "prepared_height": 64,
                "resized": False,
                "image_max_px": 800,
                "prepared_path": "/tmp/pdf_palette_prepared.jpg",
                "prepared_format": "jpeg",
            },
        ]

        def fake_build(image_path, output_path, title, body=None, filename=None, meta=None, palette_image_path=None, report=None):
            self.assertTrue(str(palette_image_path).endswith("pdf_palette_prepared.jpg"))
            self.assertEqual(report["palette_label"], "pal_sidecar")
            with open(output_path, "wb") as fh:
                fh.write(b"%PDF-1.4 fake pdf")
            return {"path": output_path, "page_count": 2}

        mock_s3.get_object.side_effect = get_object
        mock_s3.upload_fileobj.side_effect = upload_fileobj
        mock_build.side_effect = fake_build

        result = handler(_event(), None)
        body = json.loads(result["body"])

        self.assertEqual(result["statusCode"], 200)
        pdf_key = "renders/job1/pdf/pdf_123/document.pdf"
        meta = uploads[pdf_key]["extra"]["Metadata"]
        self.assertEqual(body["artifact_id"], "pdf_123")
        self.assertEqual(meta["source_associated_palette_mode"], "generated")
        self.assertEqual(meta["source_associated_palette_id"], "pal_sidecar")
        self.assertEqual(meta["source_associated_palette_image_key"], "renders/job1/palettes/pal_sidecar/image.jpeg")


if __name__ == "__main__":
    unittest.main()
