import io
import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _s3_missing(*args, **kwargs):
    from botocore.exceptions import ClientError
    raise ClientError({"Error": {"Code": "404", "Message": "Not Found"},
                       "ResponseMetadata": {"HTTPStatusCode": 404}}, "HeadObject")


def _event(**overrides):
    payload = {
        "job_id": "test_dz",
        "source_key": "renders/test_dz/color/color_src/image.jpeg",
        "raw_key": "renders/test_dz/color/color_src/greyscale.raw",
        "raw_meta_key": "renders/test_dz/color/color_src/greyscale.meta.json",
        "export_id": "dz_raw_test",
    }
    payload.update(overrides)
    return {"body": json.dumps(payload)}


class TestInternalActionBoundary(unittest.TestCase):
    """code-review-25 F4: the wall-pyramid build is storage's internal async
    fan-out; it must not be reachable through the public deepzoom-export API."""

    def test_api_shaped_event_cannot_trigger_wall_build(self):
        import handler_deepzoom_export as mod
        import handler_wall_pyramid as wall
        # an API Gateway event carries these fields; a direct invoke does not
        for marker in ("requestContext", "rawPath", "httpMethod",
                       "headers", "queryStringParameters"):
            event = {"body": json.dumps({"internal_action": "build_wall_pyramid",
                                         "kind": "color",
                                         "refresh_id": "mosaic_x",
                                         "manifest_key": "renders/evil.json"}),
                     marker: {} if marker != "httpMethod" else "POST"}
            with patch.object(wall, "handle_build_wall_pyramid") as builder:
                resp = mod.handler(event, None)
            self.assertEqual(resp["statusCode"], 403, marker)
            builder.assert_not_called()

    def test_direct_invoke_reaches_the_builder(self):
        import handler_deepzoom_export as mod
        import handler_wall_pyramid as wall
        event = {"internal_action": "build_wall_pyramid", "kind": "color",
                 "refresh_id": "mosaic_20260709T120000Z_abcdef01",
                 "manifest_key": "renders/_index/color_mosaic/"
                                 "mosaic_20260709T120000Z_abcdef01/all.json"}
        with patch.object(wall, "handle_build_wall_pyramid",
                          return_value={"statusCode": 200, "body": "{}"}) as builder:
            mod.handler(event, None)
        builder.assert_called_once()

    @patch("handler_deepzoom_export.report_status")
    @patch("handler_deepzoom_export.s3")
    def test_rejects_cross_job_source_key(self, mock_s3, _report):
        # code-review-27 F5: a source key from another job is refused before GET
        import handler_deepzoom_export as mod
        with self.assertRaises(ValueError):
            mod.handle_deepzoom_export_request(
                {"job_id": "test_dz",
                 "source_key": "renders/OTHERJOB/color/color_src/image.jpeg"})
        mock_s3.get_object.assert_not_called()

    @patch("handler_deepzoom_export.report_status")
    @patch("handler_deepzoom_export.s3")
    def test_raw_sidecars_must_match_source_artifact_not_just_job(self, mock_s3, _report):
        # code-review-28 F12: raw sidecars were tied only to the job prefix. A
        # raw_key under a DIFFERENT artifact of the same job must now be refused
        # before any GET — the sidecars belong to the source artifact.
        import handler_deepzoom_export as mod
        with self.assertRaises(ValueError):
            mod.handle_deepzoom_export_request(
                {"job_id": "test_dz",
                 "source_key": "renders/test_dz/color/color_src/image.jpeg",
                 "raw_key": "renders/test_dz/color/OTHER_ART/greyscale.raw",
                 "raw_meta_key": "renders/test_dz/color/color_src/greyscale.meta.json"},
                require_raw_sidecar=True)
        mock_s3.get_object.assert_not_called()

    @patch("handler_deepzoom_export.report_status")
    @patch("handler_deepzoom_export.s3")
    def test_source_ref_parser_extracts_exact_artifact(self, mock_s3, _report):
        # The unified render-key parser (code-review-28 F12) must return the
        # exact artifact segment, not a substring coincidence.
        import handler_deepzoom_export as mod
        ref = mod._source_ref_from_key("renders/j/palettes/pal_7/image.jpeg")
        self.assertEqual(ref["job_id"], "j")
        self.assertEqual(ref["family"], "palette")
        self.assertEqual(ref["artifact_id"], "pal_7")
        # a non-canonical (legacy root) key yields an empty ref, not a guess
        self.assertEqual(mod._source_ref_from_key("renders/j/image.jpeg")["artifact_id"], "")


class TestDeepZoomExportRaw(unittest.TestCase):
    @patch("handler_deepzoom_export.report_status")
    @patch("handler_deepzoom_export.s3")
    @patch("handler_deepzoom_export.subprocess")
    @patch("handler_deepzoom_export.load_color_artifact_head")
    def test_export_uses_exact_source_image_even_when_raw_sidecar_is_present(self, mock_load_head, mock_subprocess, mock_s3, mock_report):
        import handler_deepzoom_export as mod

        put_calls = []

        mock_load_head.return_value = {
            "artifact_id": "color_src",
            "image_key": "renders/test_dz/color/color_src/image.jpeg",
            "metadata": {
                "artifact_id": "color_src",
                "family": "color",
                "min_re": "-3.5",
                "max_re": "1.25",
                "min_im": "-0.75",
                "max_im": "2.0",
                "rotation": "0.125",
                "palette": "custom:879caa-0e3057",
                "palette_display_name": "Night reef",
            },
        }

        def get_object(Bucket=None, Key=None):
            if Key == "renders/test_dz/color/color_src/image.jpeg":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"exact image bytes"])}
            raise AssertionError(f"unexpected get_object key: {Key}")

        def put_object(**kwargs):
            body = kwargs.get("Body")
            if hasattr(body, "read"):
                body = body.read()
            put_calls.append({**kwargs, "Body": body})

        mock_s3.get_object.side_effect = get_object
        mock_s3.put_object.side_effect = put_object
        mock_s3.head_object.side_effect = _s3_missing
        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "dz_export":
                self.assertEqual(cmd[1], "/tmp/deepzoom_source.jpeg")
                return MagicMock(returncode=0, stdout=json.dumps({"width": 2, "height": 2}), stderr="")
            raise AssertionError(f"unexpected executable {exe}")

        mock_subprocess.run.side_effect = run_side_effect

        import builtins
        real_open = builtins.open
        fake_file = MagicMock(read=lambda: b"fake", write=lambda x: None)
        fake_cm = MagicMock(__enter__=MagicMock(return_value=fake_file), __exit__=MagicMock(return_value=False))

        def selective_open(*args, **kwargs):
            path = args[0] if args else kwargs.get("file", "")
            if isinstance(path, str) and (
                path.endswith("_template.html")
                or path.startswith("/tmp/deepzoom_source")
            ):
                return real_open(*args, **kwargs)
            return fake_cm

        with patch.object(os, "makedirs", side_effect=lambda *a, **kw: None), \
             patch.object(os, "remove", side_effect=lambda *a: None), \
             patch.object(os.path, "exists", return_value=True), \
             patch.object(os.path, "isdir", return_value=True), \
             patch.object(os, "walk", return_value=[("/tmp/dz/image_files/0", [], ["0_0.png"])]), \
             patch("shutil.rmtree"), \
             patch("builtins.open", side_effect=selective_open):
            result = mod.handler(_event(), None)

        body = json.loads(result["body"])
        self.assertEqual(body["source_kind"], "image")

        meta_puts = [c for c in put_calls if c.get("Key") == "deepzoom/test_dz/dz_raw_test/meta.json"]
        self.assertEqual(len(meta_puts), 1)
        manifest = json.loads(meta_puts[0]["Body"])
        self.assertEqual(manifest["source_kind"], "image")
        self.assertEqual(manifest["source_artifact_id"], "color_src")
        self.assertEqual(manifest["source_family"], "color")
        self.assertEqual(manifest["viewport_min_re"], -3.5)
        self.assertEqual(manifest["viewport_max_re"], 1.25)
        self.assertEqual(manifest["viewport_min_im"], -0.75)
        self.assertEqual(manifest["viewport_max_im"], 2.0)
        self.assertEqual(manifest["source_rotation"], 0.125)
        self.assertEqual(manifest["source_palette"], "custom:879caa-0e3057")
        self.assertEqual(manifest["source_palette_display_name"], "Night reef")
        self.assertNotIn("raw_key", manifest)
        self.assertNotIn("raw_meta_key", manifest)
        self.assertNotIn("palette", manifest)

        # tiles/dzi/viewer are immutable (export-scoped keys) and cacheable
        # forever; the meta.json pointer must stay uncached
        immutable = "public, max-age=31536000, immutable"
        tile_puts = [c for c in put_calls if "/image_files/" in c.get("Key", "")]
        self.assertTrue(tile_puts)
        for call in tile_puts:
            self.assertEqual(call["CacheControl"], immutable)
        viewer_puts = [c for c in put_calls if c.get("Key", "").endswith("/viewer.html")]
        self.assertEqual(len(viewer_puts), 1)
        self.assertEqual(viewer_puts[0]["CacheControl"], immutable)
        self.assertNotIn("CacheControl", meta_puts[0])

        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "generating", "uploading", "done"])

    @patch("handler_deepzoom_export.report_status")
    @patch("handler_deepzoom_export.s3")
    @patch("handler_deepzoom_export.subprocess")
    @patch("handler_deepzoom_export.load_color_artifact_head")
    def test_manifest_omits_viewport_fields_when_source_lacks_canonical_bounds(
        self, mock_load_head, mock_subprocess, mock_s3, mock_report
    ):
        import handler_deepzoom_export as mod

        put_calls = []
        mock_load_head.return_value = {
            "artifact_id": "color_src",
            "image_key": "renders/test_dz/color/color_src/image.jpeg",
            "metadata": {
                "artifact_id": "color_src",
                "family": "color",
            },
        }
        mock_s3.get_object.side_effect = lambda Bucket=None, Key=None: {
            "Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"exact image bytes"])
        }
        mock_s3.put_object.side_effect = lambda **kwargs: put_calls.append(kwargs)
        mock_s3.head_object.side_effect = _s3_missing
        mock_subprocess.run.return_value = MagicMock(returncode=0, stdout=json.dumps({"width": 2, "height": 2}), stderr="")

        import builtins
        real_open = builtins.open
        fake_file = MagicMock(read=lambda: b"fake", write=lambda x: None)
        fake_cm = MagicMock(__enter__=MagicMock(return_value=fake_file), __exit__=MagicMock(return_value=False))

        def selective_open(*args, **kwargs):
            path = args[0] if args else kwargs.get("file", "")
            if isinstance(path, str) and (
                path.endswith("_template.html")
                or path.startswith("/tmp/deepzoom_source")
            ):
                return real_open(*args, **kwargs)
            return fake_cm

        with patch.object(os, "makedirs", side_effect=lambda *a, **kw: None), \
             patch.object(os, "remove", side_effect=lambda *a: None), \
             patch.object(os.path, "exists", return_value=True), \
             patch.object(os.path, "isdir", return_value=True), \
             patch.object(os, "walk", return_value=[("/tmp/dz/image_files/0", [], ["0_0.png"])]), \
             patch("shutil.rmtree"), \
             patch("builtins.open", side_effect=selective_open):
            mod.handler(_event(export_id="dz_missing_viewport"), None)

        meta_puts = [c for c in put_calls if c.get("Key") == "deepzoom/test_dz/dz_missing_viewport/meta.json"]
        self.assertEqual(len(meta_puts), 1)
        manifest = json.loads(meta_puts[0]["Body"])
        self.assertEqual(manifest["source_artifact_id"], "color_src")
        self.assertEqual(manifest["source_family"], "color")
        self.assertEqual(manifest["source_rotation"], 0.0)
        self.assertNotIn("viewport_min_re", manifest)
        self.assertNotIn("viewport_max_re", manifest)
        self.assertNotIn("viewport_min_im", manifest)
        self.assertNotIn("viewport_max_im", manifest)


if __name__ == "__main__":
    unittest.main()


class TestOperationIdentity(unittest.TestCase):
    """code-review-29 F1: overlapping exports must never share a status row or
    an export prefix — the caller's task_id threads through both entry points,
    and a supposedly NEW export refuses an already-occupied prefix."""

    @patch("handler_deepzoom_export.report_status")
    @patch("handler_deepzoom_export.s3")
    def test_handler_threads_caller_task_id_into_status_rows(self, mock_s3, report):
        import handler_deepzoom_export as mod
        mock_s3.head_object.side_effect = _s3_missing
        with self.assertRaises(ValueError):
            mod.handler(_event(source_key="renders/OTHERJOB/color/color_src/image.jpeg",
                               task_id="deepzoom_export_op123"), None)
        rows = {c.args[1] for c in report.call_args_list}
        self.assertEqual(rows, {"deepzoom_export_op123"})

    def test_handler_rejects_unsafe_task_id(self):
        import handler_deepzoom_export as mod
        with self.assertRaises(ValueError):
            mod.handler(_event(task_id="../evil"), None)

    @patch("handler_deepzoom_export.report_status")
    @patch("handler_deepzoom_export.s3")
    def test_new_export_refuses_existing_prefix(self, mock_s3, report):
        import handler_deepzoom_export as mod
        mock_s3.head_object.return_value = {}   # meta.json already at the prefix
        with self.assertRaisesRegex(RuntimeError, "already exists"):
            mod.handle_deepzoom_export_request(
                {"job_id": "test_dz",
                 "source_key": "renders/test_dz/color/color_src/image.jpeg",
                 "export_id": "dz_dupe"})
        mock_s3.get_object.assert_not_called()   # refused before any download
        self.assertIn("error", {c.args[2] for c in report.call_args_list})

    @patch("handler_deepzoom_export.report_status")
    @patch("handler_deepzoom_export.s3")
    def test_from_raw_wrapper_threads_task_id(self, mock_s3, report):
        import handler_deepzoom_from_raw as raw
        mock_s3.head_object.side_effect = _s3_missing
        with self.assertRaises(RuntimeError):   # missing raw sidecars
            raw.handler({"body": json.dumps({"job_id": "test_dz",
                                             "task_id": "deepzoom_from_raw_op9"})}, None)
        rows = {c.args[1] for c in report.call_args_list}
        self.assertEqual(rows, {"deepzoom_from_raw_op9"})
