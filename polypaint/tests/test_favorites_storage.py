"""Favorites storage contract (favorites-speedup.md).

Pins the panel-ready favorites backend: a snapshot list is one DDB query and
zero S3 ops; legacy rows resolve by EXACT key (never /render-summary, never
list_objects_v2); Add validates identity server-side and stores canonical keys;
Add/Delete return single rows; transient S3 failures surface as `error`, not
`missing`. This is a gated replacement for the old ungated TestFavoritesStorage.
"""
import io
import json
import os
import sys
import unittest
from unittest.mock import patch

from botocore.exceptions import ClientError

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

import handler_storage as hs


def _ce(code, status, op="HeadObject"):
    return ClientError(
        {"Error": {"Code": code, "Message": code},
         "ResponseMetadata": {"HTTPStatusCode": status}}, op)


class _FakeDDB:
    def __init__(self):
        self.items = {}          # (job_id, task_id) -> attr dict
        self.query_calls = 0
        self.get_calls = 0

    def query(self, TableName=None, KeyConditionExpression=None,
              ExpressionAttributeValues=None, ExclusiveStartKey=None):
        self.query_calls += 1
        jid = ExpressionAttributeValues[":jid"]["S"]
        items = [v for (j, _t), v in sorted(self.items.items()) if j == jid]
        return {"Items": items}

    def get_item(self, TableName=None, Key=None, ProjectionExpression=None):
        self.get_calls += 1
        k = (Key["job_id"]["S"], Key["task_id"]["S"])
        return {"Item": self.items[k]} if k in self.items else {}

    def put_item(self, TableName=None, Item=None, ConditionExpression=None):
        k = (Item["job_id"]["S"], Item["task_id"]["S"])
        if ConditionExpression and "attribute_not_exists" in ConditionExpression and k in self.items:
            raise _ce("ConditionalCheckFailedException", 400, "PutItem")
        self.items[k] = Item
        return {}

    def delete_item(self, TableName=None, Key=None, ReturnValues=None):
        k = (Key["job_id"]["S"], Key["task_id"]["S"])
        old = self.items.pop(k, None)
        return {"Attributes": old} if old else {}

    def update_item(self, TableName=None, Key=None, UpdateExpression=None,
                    ExpressionAttributeValues=None, ConditionExpression=None):
        k = (Key["job_id"]["S"], Key["task_id"]["S"])
        if ConditionExpression and "attribute_exists" in ConditionExpression and k not in self.items:
            raise _ce("ConditionalCheckFailedException", 400, "UpdateItem")
        self.items[k]["snapshot_version"] = ExpressionAttributeValues[":v"]
        self.items[k]["snapshot"] = ExpressionAttributeValues[":s"]
        self.items[k]["updated_at_ms"] = ExpressionAttributeValues[":u"]
        return {}


class _FakeS3:
    def __init__(self):
        self.objects = {}        # key -> {ContentLength, ContentType, Metadata, body}
        self.fail = {}           # key -> ClientError to raise
        self.head_calls = 0
        self.get_calls = 0

    def head_object(self, Bucket=None, Key=None):
        self.head_calls += 1
        if Key in self.fail:
            raise self.fail[Key]
        if Key not in self.objects:
            raise _ce("NoSuchKey", 404, "HeadObject")
        o = self.objects[Key]
        return {"Metadata": o.get("Metadata", {}), "ContentLength": o.get("ContentLength", 0),
                "ContentType": o.get("ContentType", "")}

    def get_object(self, Bucket=None, Key=None):
        self.get_calls += 1
        if Key in self.fail:
            raise self.fail[Key]
        if Key not in self.objects:
            raise _ce("NoSuchKey", 404, "GetObject")
        return {"Body": io.BytesIO(self.objects[Key].get("body", b"{}"))}

    def generate_presigned_url(self, *a, **k):
        return "https://example/presigned"

    def get_paginator(self, name):
        raise AssertionError(f"favorites must not list objects (get_paginator {name!r})")


class FavoritesStorageTests(unittest.TestCase):
    def setUp(self):
        self.ddb = _FakeDDB()
        self.s3 = _FakeS3()
        # __meta__ present -> store is "initialized" (no legacy migration).
        self.ddb.items[(hs.FAVORITES_DDB_JOB_ID, hs.FAVORITES_DDB_META_TASK_ID)] = {
            "job_id": {"S": hs.FAVORITES_DDB_JOB_ID},
            "task_id": {"S": hs.FAVORITES_DDB_META_TASK_ID},
        }
        self._patches = [
            patch.object(hs, "_get_ddb", return_value=self.ddb),
            patch.object(hs, "s3", self.s3),
            patch.object(hs, "_results_list_s3_client", return_value=self.s3),
        ]
        for p in self._patches:
            p.start()

    def tearDown(self):
        for p in self._patches:
            p.stop()

    # ── seeding helpers ──────────────────────────────────────────────
    def _seed_artifact(self, job, art, *, preview=True, overlay=None):
        self.s3.objects[f"renders/{job}/color/{art}/image.jpeg"] = {
            "ContentLength": 1234, "ContentType": "image/jpeg", "Metadata": {}}
        if preview:
            self.s3.objects[f"renders/{job}/color/{art}/preview.png"] = {
                "ContentLength": 50, "ContentType": "image/png", "Metadata": {}}
        if overlay is not None:
            self.s3.objects[f"renders/{job}/color/{art}/meta.json"] = {
                "body": json.dumps(overlay).encode()}

    def _seed_fav(self, job, art, *, snapshot=None, added_at="2026-01-01T00:00:00Z",
                  display_name=None, image_key=None, preview_key=None):
        item = {
            "job_id": {"S": hs.FAVORITES_DDB_JOB_ID},
            "task_id": {"S": hs._favorite_task_id(job, art)},
            "favorite_job_id": {"S": job}, "favorite_artifact_id": {"S": art},
            "family": {"S": "color"}, "added_at": {"S": added_at},
        }
        if display_name:
            item["display_name"] = {"S": display_name}
        if image_key:
            item["image_key"] = {"S": image_key}
        if preview_key:
            item["preview_key"] = {"S": preview_key}
        if snapshot is not None:
            item["snapshot_version"] = {"N": str(hs.FAVORITE_SNAPSHOT_VERSION)}
            item["snapshot"] = {"S": json.dumps(snapshot)}
        self.ddb.items[(hs.FAVORITES_DDB_JOB_ID, hs._favorite_task_id(job, art))] = item

    def _event(self, body):
        return {"body": json.dumps(body)}

    # ── list ─────────────────────────────────────────────────────────
    def test_snapshot_list_is_one_query_zero_s3(self):
        self._seed_fav("jobA", "cA", snapshot={"image_key": "renders/jobA/color/cA/image.jpeg",
                                                "width": 512, "height": 512, "color_mode": "solve_score"})
        self._seed_fav("jobB", "cB", snapshot={"image_key": "renders/jobB/color/cB/image.jpeg",
                                                "width": 512, "height": 512}, added_at="2026-02-01T00:00:00Z")
        body = json.loads(hs.handle_list_favorites(self._event({}))["body"])
        self.assertEqual(body["count"], 2)
        self.assertEqual(self.ddb.query_calls, 1)
        self.assertEqual(self.s3.head_calls, 0)
        self.assertEqual(self.s3.get_calls, 0)
        self.assertEqual(body["snapshot_hits"], 2)
        # newest-added first
        self.assertEqual([r["favorite_job_id"] for r in body["favorites"]], ["jobB", "jobA"])
        self.assertEqual(body["favorites"][1]["color_mode"], "solve_score")
        self.assertTrue(all(r["hydration_state"] == "ready" for r in body["favorites"]))

    def test_legacy_row_resolves_by_exact_key_and_backfills(self):
        self._seed_fav("jobA", "cA")  # no snapshot -> legacy
        self._seed_artifact("jobA", "cA", overlay={"color_mode": "solve_score", "created_at": "2026-03-01"})
        body = json.loads(hs.handle_list_favorites(self._event({}))["body"])
        self.assertEqual(body["count"], 1)
        row = body["favorites"][0]
        self.assertEqual(row["hydration_state"], "ready")
        self.assertEqual(row["image_key"], "renders/jobA/color/cA/image.jpeg")
        self.assertEqual(row["color_mode"], "solve_score")
        self.assertEqual(body["snapshot_backfills"], 1)
        # exact-key resolution, never a listing
        self.assertGreater(self.s3.head_calls, 0)
        # backfilled: a second list is now zero-S3
        self.s3.head_calls = 0
        body2 = json.loads(hs.handle_list_favorites(self._event({}))["body"])
        self.assertEqual(self.s3.head_calls, 0)
        self.assertEqual(body2["snapshot_hits"], 1)

    def test_true_404_is_missing_transient_is_error(self):
        # genuine 404: no artifact objects at all
        self._seed_fav("jobM", "gone")
        # transient: image.jpeg throttles, image.png absent
        self._seed_fav("jobT", "slow", added_at="2026-02-01T00:00:00Z")
        self.s3.fail["renders/jobT/color/slow/image.jpeg"] = _ce("SlowDown", 503)
        body = json.loads(hs.handle_list_favorites(self._event({}))["body"])
        rows = {r["favorite_job_id"]: r for r in body["favorites"]}
        self.assertEqual(rows["jobM"]["hydration_state"], "missing")
        self.assertTrue(rows["jobM"]["missing"])
        self.assertEqual(rows["jobT"]["hydration_state"], "error")
        self.assertFalse(rows["jobT"]["missing"], "a transient error must NOT be marked missing")
        self.assertEqual(body["missing"], 1)
        self.assertEqual(body["errors"], 1)

    def test_refresh_true_reresolves_snapshot_rows(self):
        self._seed_fav("jobA", "cA", snapshot={"image_key": "renders/jobA/color/cA/image.jpeg", "width": 1})
        self._seed_artifact("jobA", "cA", overlay={"color_mode": "solve_score"})
        body = json.loads(hs.handle_list_favorites(self._event({"refresh": True}))["body"])
        self.assertGreater(self.s3.head_calls, 0)  # re-resolved despite snapshot
        self.assertEqual(body["favorites"][0]["color_mode"], "solve_score")

    def test_list_never_calls_render_summary(self):
        self._seed_fav("jobA", "cA")
        self._seed_artifact("jobA", "cA")
        with patch.object(hs, "handle_render_summary",
                          side_effect=AssertionError("favorites must not call render-summary")):
            body = json.loads(hs.handle_list_favorites(self._event({}))["body"])
        self.assertEqual(body["count"], 1)

    # ── add ──────────────────────────────────────────────────────────
    def test_add_resolves_stores_canonical_keys_single_row(self):
        self._seed_artifact("jobA", "cA", overlay={"color_mode": "solve_score"})
        body = json.loads(hs.handle_add_favorite(self._event({
            "job_id": "jobA", "artifact_id": "cA", "family": "color"}))["body"])
        self.assertTrue(body["added"])
        self.assertIn("favorite", body)
        self.assertNotIn("favorites", body)  # single row, not full partition
        self.assertEqual(body["favorite"]["image_key"], "renders/jobA/color/cA/image.jpeg")
        # stored row carries a snapshot
        item = self.ddb.items[(hs.FAVORITES_DDB_JOB_ID, hs._favorite_task_id("jobA", "cA"))]
        self.assertEqual(item["snapshot_version"]["N"], str(hs.FAVORITE_SNAPSHOT_VERSION))
        self.assertIn("snapshot", item)

    def test_add_ignores_mismatched_caller_keys(self):
        self._seed_artifact("jobA", "cA")
        json.loads(hs.handle_add_favorite(self._event({
            "job_id": "jobA", "artifact_id": "cA", "family": "color",
            "image_key": "renders/EVIL/color/x/image.jpeg",   # attacker hint
            "preview_key": "renders/EVIL/color/x/preview.png"}))["body"])
        item = self.ddb.items[(hs.FAVORITES_DDB_JOB_ID, hs._favorite_task_id("jobA", "cA"))]
        self.assertEqual(item["image_key"]["S"], "renders/jobA/color/cA/image.jpeg")
        self.assertNotIn("EVIL", json.dumps(item))

    def test_add_rejects_missing_artifact(self):
        with self.assertRaises(ValueError):
            hs.handle_add_favorite(self._event({
                "job_id": "jobX", "artifact_id": "nope", "family": "color"}))
        self.assertNotIn((hs.FAVORITES_DDB_JOB_ID, hs._favorite_task_id("jobX", "nope")), self.ddb.items)

    def test_add_transient_resolution_raises_not_stored(self):
        self.s3.fail["renders/jobA/color/cA/image.jpeg"] = _ce("SlowDown", 503)
        with self.assertRaises(RuntimeError):
            hs.handle_add_favorite(self._event({
                "job_id": "jobA", "artifact_id": "cA", "family": "color"}))

    def test_add_dedupe_returns_added_false_with_row(self):
        self._seed_artifact("jobA", "cA")
        hs.handle_add_favorite(self._event({"job_id": "jobA", "artifact_id": "cA", "family": "color"}))
        body = json.loads(hs.handle_add_favorite(self._event({
            "job_id": "jobA", "artifact_id": "cA", "family": "color"}))["body"])
        self.assertFalse(body["added"])
        self.assertEqual(body["favorite"]["favorite_artifact_id"], "cA")

    def test_add_rejects_bad_ids(self):
        for bad in ({"job_id": "../evil", "artifact_id": "cA"},
                    {"job_id": "jobA", "artifact_id": "a/b"}):
            with self.assertRaises(ValueError):
                hs.handle_add_favorite(self._event({**bad, "family": "color"}))

    # ── delete ───────────────────────────────────────────────────────
    def test_delete_returns_single_row_no_partition(self):
        self._seed_fav("jobA", "cA", snapshot={"image_key": "x"})
        body = json.loads(hs.handle_delete_favorite(self._event({
            "job_id": "jobA", "artifact_id": "cA"}))["body"])
        self.assertTrue(body["deleted"])
        self.assertEqual(body["job_id"], "jobA")
        self.assertEqual(body["artifact_id"], "cA")
        self.assertNotIn("favorites", body)
        self.assertNotIn((hs.FAVORITES_DDB_JOB_ID, hs._favorite_task_id("jobA", "cA")), self.ddb.items)

    # ── performance contract ─────────────────────────────────────────
    def test_100_snapshot_favorites_one_query_zero_s3(self):
        for i in range(100):
            self._seed_fav(f"job{i}", f"c{i}", snapshot={"image_key": f"renders/job{i}/color/c{i}/image.jpeg"})
        body = json.loads(hs.handle_list_favorites(self._event({}))["body"])
        self.assertEqual(body["count"], 100)
        self.assertEqual(self.ddb.query_calls, 1)
        self.assertEqual(self.s3.head_calls, 0)
        self.assertEqual(self.s3.get_calls, 0)

    def test_100_legacy_favorites_exact_ops_no_listing(self):
        for i in range(100):
            self._seed_fav(f"job{i}", f"c{i}")
            self._seed_artifact(f"job{i}", f"c{i}")
        body = json.loads(hs.handle_list_favorites(self._event({}))["body"])
        self.assertEqual(body["count"], 100)
        self.assertEqual(body["snapshot_backfills"], 100)
        # exact-key HEADs only; _FakeS3.get_paginator would have raised on any listing
        self.assertLessEqual(self.s3.head_calls, 100 * 3)  # <=3 candidates per favorite


if __name__ == "__main__":
    unittest.main()
