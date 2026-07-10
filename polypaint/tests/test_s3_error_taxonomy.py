"""S3 error taxonomy (code-review-28 F13).

One policy — shared.is_missing_s3_error — decides whether an S3 exception means
the object is genuinely absent (404/NoSuchKey) versus a transient/config error
that must be retried or propagated, never silently relabeled as "missing". This
suite pins that policy and the four canonical read/existence helpers that used
to turn every exception into absence.
"""
import io
import os
import sys
import unittest
import unittest.mock

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

from botocore.exceptions import ClientError, EndpointConnectionError

import shared
from shared import is_missing_s3_error, s3_error_reason, s3_error_code


def _ce(code, status=None, op="HeadObject"):
    resp = {"Error": {"Code": code, "Message": code}}
    if status is not None:
        resp["ResponseMetadata"] = {"HTTPStatusCode": status}
    return ClientError(resp, op)


class TestMissingPolicy(unittest.TestCase):
    def test_only_404_family_is_missing(self):
        for code in ("NoSuchKey", "404", "NotFound"):
            self.assertTrue(is_missing_s3_error(_ce(code)), code)
        # bare HTTP 404 with no error code is still absence
        self.assertTrue(is_missing_s3_error(ClientError(
            {"ResponseMetadata": {"HTTPStatusCode": 404}}, "HeadObject")))

    def test_transient_and_config_errors_are_not_missing(self):
        for code in ("SlowDown", "503", "500", "InternalError", "ServiceUnavailable",
                     "AccessDenied", "403", "NoSuchBucket", "RequestTimeout"):
            self.assertFalse(is_missing_s3_error(_ce(code)), code)
        self.assertFalse(is_missing_s3_error(EndpointConnectionError(endpoint_url="https://x")))
        self.assertFalse(is_missing_s3_error(TimeoutError("read timed out")))

    def test_nosuchbucket_is_config_error_not_absence(self):
        # A missing bucket is a deployment/config fault; treating it as "object
        # absent" would mask a total outage as an empty result.
        self.assertFalse(is_missing_s3_error(_ce("NoSuchBucket")))
        self.assertEqual(s3_error_reason(_ce("NoSuchBucket")), "error")

    def test_string_fallback_only_matches_absence_markers(self):
        # Non-ClientError wrappers: only explicit absence markers count.
        self.assertTrue(is_missing_s3_error(Exception("NoSuchKey")))
        self.assertTrue(is_missing_s3_error(Exception("boto3 NotFound")))
        self.assertFalse(is_missing_s3_error(Exception("SlowDown please retry")))
        self.assertFalse(is_missing_s3_error(Exception("AccessDenied")))

    def test_reason_buckets(self):
        self.assertEqual(s3_error_reason(_ce("NoSuchKey")), "missing")
        self.assertEqual(s3_error_reason(_ce("AccessDenied")), "access_denied")
        self.assertEqual(s3_error_reason(_ce("403")), "access_denied")
        self.assertEqual(s3_error_reason(_ce("SlowDown")), "throttled")
        self.assertEqual(s3_error_reason(_ce("503")), "throttled")
        self.assertEqual(s3_error_reason(_ce("InternalError", 500)), "server_error")
        self.assertEqual(s3_error_reason(_ce("InternalError")), "server_error")
        self.assertEqual(s3_error_reason(EndpointConnectionError(endpoint_url="https://x")), "transport")
        self.assertEqual(s3_error_reason(_ce("BadDigest", 400)), "error")

    def test_error_code_prefers_code_then_status(self):
        self.assertEqual(s3_error_code(_ce("SlowDown", 503)), "SlowDown")
        self.assertEqual(s3_error_code(ClientError(
            {"ResponseMetadata": {"HTTPStatusCode": 502}}, "HeadObject")), "502")
        self.assertEqual(s3_error_code(Exception("no response attr")), "")


class _Store:
    """Minimal S3 stand-in whose missing-object errors look like real boto3."""
    def __init__(self):
        self.objects = {}
        self.metadata = {}
        self.fail = {}  # key -> ClientError to raise on head/get

    def head_object(self, Bucket=None, Key=None):
        if Key in self.fail:
            raise self.fail[Key]
        if Key not in self.objects:
            raise _ce("NoSuchKey", 404, "HeadObject")
        return {"Metadata": dict(self.metadata.get(Key) or {}), "ContentLength": len(self.objects[Key])}

    def get_object(self, Bucket=None, Key=None):
        if Key in self.fail:
            raise self.fail[Key]
        if Key not in self.objects:
            raise _ce("NoSuchKey", 404, "GetObject")
        return {"Body": io.BytesIO(self.objects[Key])}

    def generate_presigned_url(self, *a, **k):
        return "https://example/presigned"


class TestKeyExistsPropagates(unittest.TestCase):
    def setUp(self):
        import handler_storage
        self.hs = handler_storage

    def test_missing_returns_false(self):
        store = _Store()
        with unittest.mock.patch.object(self.hs, "s3", store):
            self.assertFalse(self.hs._key_exists("renders/j/color/a/image.jpeg"))

    def test_present_returns_true(self):
        store = _Store()
        store.objects["k"] = b"x"
        with unittest.mock.patch.object(self.hs, "s3", store):
            self.assertTrue(self.hs._key_exists("k"))

    def test_throttle_propagates_not_absent(self):
        store = _Store()
        store.fail["k"] = _ce("SlowDown", 503)
        with unittest.mock.patch.object(self.hs, "s3", store):
            with self.assertRaises(ClientError):
                self.hs._key_exists("k")


class TestHeadArtifactKeysReason(unittest.TestCase):
    def test_missing_is_clean_absence_transient_surfaces_reason(self):
        import handler_storage
        store = _Store()
        store.objects["renders/j/color/a/image.jpeg"] = b"img"
        store.fail["renders/j/color/b/image.jpeg"] = _ce("SlowDown", 503)
        res = handler_storage._head_artifact_keys(
            ["renders/j/color/a/image.jpeg", "renders/j/color/b/image.jpeg",
             "renders/j/color/c/image.jpeg"],
            presign=False, s3_client=store)
        self.assertTrue(res["renders/j/color/a/image.jpeg"]["exists"])
        # genuine-missing: clean exists=False, no error flag
        missing = res["renders/j/color/c/image.jpeg"]
        self.assertFalse(missing["exists"])
        self.assertNotIn("error", missing)
        # transient: exists=False BUT reason surfaced, not silently "missing"
        throttled = res["renders/j/color/b/image.jpeg"]
        self.assertFalse(throttled["exists"])
        self.assertTrue(throttled.get("error"))
        self.assertEqual(throttled.get("error_reason"), "throttled")


class TestColorOverlayPropagates(unittest.TestCase):
    def test_missing_overlay_is_none_transient_propagates(self):
        import handler_storage
        store = _Store()
        with unittest.mock.patch.object(handler_storage, "s3", store):
            # genuine missing -> no overlay
            self.assertIsNone(handler_storage._load_color_artifact_overlay("j", "a"))
            # transient -> propagate, never a silent None
            from color_artifact_meta import color_artifact_meta_key
            store.fail[color_artifact_meta_key("j", "a")] = _ce("SlowDown", 503)
            with self.assertRaises(ClientError):
                handler_storage._load_color_artifact_overlay("j", "a")


class TestLoadColorArtifactHead(unittest.TestCase):
    def test_transient_candidate_propagates_not_next_key(self):
        from color_artifact_meta import (load_color_artifact_head,
                                         color_artifact_image_candidates)
        store = _Store()
        candidates = color_artifact_image_candidates("j", "a")
        # First candidate throttles: must NOT walk to the next and claim
        # not-found; it must propagate.
        store.fail[candidates[0]] = _ce("SlowDown", 503)
        with self.assertRaises(RuntimeError) as ctx:
            load_color_artifact_head(store, "bucket", "j", "a")
        self.assertIn("throttled", str(ctx.exception))

    def test_all_candidates_missing_raises_not_found(self):
        from color_artifact_meta import load_color_artifact_head
        store = _Store()  # nothing present -> every candidate 404s
        with self.assertRaises(RuntimeError) as ctx:
            load_color_artifact_head(store, "bucket", "j", "a")
        self.assertIn("not found", str(ctx.exception).lower())


if __name__ == "__main__":
    unittest.main()
