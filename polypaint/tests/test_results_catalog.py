"""Results catalog contract (results-list.md Phase 2 / favorites-speedup ideas).

Pins the /list fast path: membership from ONE cheap prefix listing, table
fields from ONE DDB Query, calc.json read only for jobs the catalog has never
seen (then cached forever — calc.json is written once, at compute completion).
Self-healing: vanished jobs are pruned, calc-less prefixes are re-probed only
inside the mid-compute window, transient errors are surfaced but never cached,
and rebuild=true re-reads everything.
"""
import io
import json
import os
import sys
import time
import unittest
from unittest.mock import patch

from botocore.exceptions import ClientError

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

import handler_storage as hs


def _ce(code, status, op="GetObject"):
    return ClientError(
        {"Error": {"Code": code, "Message": code},
         "ResponseMetadata": {"HTTPStatusCode": status}}, op)


class _FakeDDB:
    def __init__(self):
        self.items = {}          # (job_id, task_id) -> attr dict
        self.query_calls = 0

    def query(self, TableName=None, KeyConditionExpression=None,
              ExpressionAttributeValues=None, ExclusiveStartKey=None):
        self.query_calls += 1
        jid = ExpressionAttributeValues[":jid"]["S"]
        items = [v for (j, _t), v in sorted(self.items.items()) if j == jid]
        return {"Items": items}

    def put_item(self, TableName=None, Item=None, ConditionExpression=None):
        self.items[(Item["job_id"]["S"], Item["task_id"]["S"])] = Item
        return {}

    def delete_item(self, TableName=None, Key=None, ReturnValues=None):
        self.items.pop((Key["job_id"]["S"], Key["task_id"]["S"]), None)
        return {}

    def get_item(self, TableName=None, Key=None, ProjectionExpression=None):
        k = (Key["job_id"]["S"], Key["task_id"]["S"])
        return {"Item": self.items[k]} if k in self.items else {}

    def batch_write_item(self, RequestItems=None):
        self.batch_calls = getattr(self, "batch_calls", 0) + 1
        if getattr(self, "unprocessed_once", False):
            self.unprocessed_once = False
            table, reqs = next(iter(RequestItems.items()))
            # process all but one, hand the last back as unprocessed
            for req in reqs[:-1]:
                self._apply(req)
            return {"UnprocessedItems": {table: reqs[-1:]}}
        for reqs in RequestItems.values():
            for req in reqs:
                self._apply(req)
        return {"UnprocessedItems": {}}

    def _apply(self, req):
        if "PutRequest" in req:
            it = req["PutRequest"]["Item"]
            self.items[(it["job_id"]["S"], it["task_id"]["S"])] = it
        else:
            k = req["DeleteRequest"]["Key"]
            self.items.pop((k["job_id"]["S"], k["task_id"]["S"]), None)

    def catalog_rows(self):
        return {t.split("#", 1)[1]: v for (j, t), v in self.items.items()
                if j == hs.RESULTS_CATALOG_DDB_JOB_ID and t.startswith("result#")}


class _FakeS3:
    """Exact-key GET + delimiter-aware prefix listing + batch delete."""

    def __init__(self):
        self.objects = {}        # key -> bytes
        self.fail = {}           # key -> ClientError
        self.get_calls = []

    def get_object(self, Bucket=None, Key=None):
        self.get_calls.append(Key)
        if Key in self.fail:
            raise self.fail[Key]
        if Key not in self.objects:
            raise _ce("NoSuchKey", 404)
        import hashlib
        return {"Body": io.BytesIO(self.objects[Key]),
                "ETag": '"' + hashlib.md5(self.objects[Key]).hexdigest() + '"'}

    def get_paginator(self, name):
        outer = self

        class _P:
            def paginate(self, Bucket=None, Prefix=None, Delimiter=None,
                         ContinuationToken=None):
                if Delimiter == "/":
                    jobs = sorted({k.split("/")[1] for k in outer.objects
                                   if k.startswith(Prefix) and k.count("/") >= 2})
                    return [{"CommonPrefixes": [{"Prefix": f"{Prefix}{j}/"} for j in jobs]}]
                return [{"Contents": [{"Key": k} for k in sorted(outer.objects)
                                      if k.startswith(Prefix)]}]

        return _P()

    def delete_objects(self, Bucket=None, Delete=None):
        for obj in (Delete or {}).get("Objects", []):
            self.objects.pop(obj["Key"], None)
        return {"Deleted": (Delete or {}).get("Objects", [])}

    def put_object(self, Bucket=None, Key=None, Body=None, ContentType=None,
                   CacheControl=None, Metadata=None):
        self.objects[Key] = Body if isinstance(Body, (bytes, bytearray)) else str(Body).encode()
        return {}


def _calc(function="poly_1", degree=8, N=2048, chunks=(500, 700), coeffs=100):
    return json.dumps({
        "function": function, "degree": degree, "N": N, "n1": N,
        "n_chunks": len(chunks), "times": 1,
        "chunks": [{"bin_size": b} for b in chunks],
        "total_coeffs_size": coeffs,
        "total_roots": sum(chunks) // 8,
    }).encode()


class ResultsCatalogTests(unittest.TestCase):
    def setUp(self):
        self.ddb = _FakeDDB()
        self.s3 = _FakeS3()
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

    def _seed_job(self, job, **kw):
        self.s3.objects[f"renders/{job}/calc.json"] = _calc(**kw)

    def _list(self, **body):
        resp = hs.handle_list({"body": json.dumps(body)})
        return json.loads(resp["body"])

    def _calc_gets(self):
        return [k for k in self.s3.get_calls if k.endswith("calc.json")]

    # ── the fast path ─────────────────────────────────────────────────
    def test_first_list_reads_calc_once_and_builds_catalog(self):
        self._seed_job("compute_a", function="poly_a", degree=8)
        self._seed_job("compute_b", function="poly_b", degree=12, chunks=(3000, 4000), coeffs=500)
        body = self._list()
        self.assertEqual(body["count"], 2)
        self.assertEqual([r["job_id"] for r in body["results"]],
                         ["compute_b", "compute_a"])            # job_id desc
        b = body["results"][0]
        self.assertEqual(b["function"], "poly_b")
        self.assertEqual(b["total_size"], 7500)                  # chunks + coeffs
        self.assertEqual(b["total_roots"], 875)
        self.assertEqual(body["catalog_misses"], 2)
        self.assertEqual(body["catalog_hits"], 0)
        self.assertEqual(set(self.ddb.catalog_rows()), {"compute_a", "compute_b"})

    def test_second_list_is_served_from_the_catalog_with_zero_calc_reads(self):
        self._seed_job("compute_a")
        self._seed_job("compute_b")
        first = self._list()
        n_gets = len(self._calc_gets())
        second = self._list()
        self.assertEqual(len(self._calc_gets()), n_gets)         # ZERO new calc reads
        self.assertEqual(second["results"], first["results"])    # identical table
        self.assertEqual(second["catalog_hits"], 2)
        self.assertEqual(second["catalog_misses"], 0)

    def test_new_job_reconciles_with_exactly_one_calc_read(self):
        self._seed_job("compute_a")
        self._list()
        self._seed_job("compute_new")
        n_gets = len(self._calc_gets())
        body = self._list()
        self.assertEqual(len(self._calc_gets()), n_gets + 1)     # only the new job
        self.assertIn("compute_new", [r["job_id"] for r in body["results"]])
        self.assertEqual(body["catalog_misses"], 1)

    def test_vanished_job_is_pruned_from_catalog_and_results(self):
        self._seed_job("compute_a")
        self._seed_job("compute_gone")
        self._list()
        del self.s3.objects["renders/compute_gone/calc.json"]
        body = self._list()
        self.assertEqual([r["job_id"] for r in body["results"]], ["compute_a"])
        self.assertEqual(body["catalog_pruned"], 1)
        self.assertEqual(set(self.ddb.catalog_rows()), {"compute_a"})

    # ── calc-less prefixes (mid-compute window) ───────────────────────
    def test_calcless_prefix_reprobed_young_trusted_old(self):
        self.s3.objects["renders/compute_midflight/chunk0.bin"] = b"x"
        self._list()
        rows = self.ddb.catalog_rows()
        self.assertIn("no_calc", rows["compute_midflight"])
        # young no_calc row -> re-probed (compute may have just finished)
        n = len(self._calc_gets())
        self._seed_job("compute_midflight")
        body = self._list()
        self.assertEqual(len(self._calc_gets()), n + 1)
        self.assertIn("compute_midflight", [r["job_id"] for r in body["results"]])
        # permanently calc-less prefix: an OLD row is trusted, never re-read
        self.s3.objects["renders/junk_prefix/leftover.bin"] = b"x"
        self._list()
        key = (hs.RESULTS_CATALOG_DDB_JOB_ID, hs._results_catalog_task_id("junk_prefix"))
        self.ddb.items[key]["cached_at_ms"]["N"] = str(
            int(time.time() * 1000) - hs.RESULTS_CATALOG_NO_CALC_RETRY_MS - 1000)
        n = len(self._calc_gets())
        body = self._list()
        self.assertEqual(len([k for k in self._calc_gets()[n:] if "junk_prefix" in k]), 0)
        self.assertNotIn("junk_prefix", [r["job_id"] for r in body["results"]])
        self.assertGreaterEqual(body["skipped_missing_calc"], 1)

    # ── error handling (favorites rule: transient is never cached) ────
    def test_transient_calc_error_is_reported_but_never_cached(self):
        self._seed_job("compute_a")
        self.s3.fail["renders/compute_flaky/calc.json"] = _ce("SlowDown", 503)
        self.s3.objects["renders/compute_flaky/other.bin"] = b"x"
        body = self._list()
        self.assertEqual(body["metadata_error_count"], 1)
        flaky = next(r for r in body["results"] if r["job_id"] == "compute_flaky")
        self.assertEqual(flaky["function"], "?")                 # surfaced, degraded
        self.assertNotIn("compute_flaky", self.ddb.catalog_rows())   # NOT cached
        # provider recovers -> next list reads it for real
        del self.s3.fail["renders/compute_flaky/calc.json"]
        self._seed_job("compute_flaky", function="poly_ok")
        body = self._list()
        flaky = next(r for r in body["results"] if r["job_id"] == "compute_flaky")
        self.assertEqual(flaky["function"], "poly_ok")

    def test_rebuild_rereads_every_job(self):
        self._seed_job("compute_a")
        self._seed_job("compute_b")
        self._list()
        n = len(self._calc_gets())
        body = self._list(rebuild=True)
        self.assertEqual(len(self._calc_gets()), n + 2)          # both re-read
        self.assertEqual(body["catalog_misses"], 2)

    def test_internal_prefixes_are_never_listed_or_probed(self):
        self._seed_job("compute_a")
        self.s3.objects["renders/_index/color_mosaic/x/all.json"] = b"{}"
        self.s3.objects["renders/_shared_mosaic/gallery/x/manifest.json"] = b"{}"
        body = self._list()
        self.assertEqual([r["job_id"] for r in body["results"]], ["compute_a"])
        self.assertFalse(any("_index" in k or "_shared" in k for k in self._calc_gets()))

    # ── lockstep hooks ────────────────────────────────────────────────
    def test_save_metadata_upserts_the_catalog_row(self):
        meta = json.loads(_calc(function="poly_saved", degree=9).decode())
        hs.handle_save_metadata({"body": json.dumps({"job_id": "compute_s", "metadata": meta})})
        row = self.ddb.catalog_rows()["compute_s"]
        self.assertEqual(row["fn"]["S"], "poly_saved")
        self.assertEqual(row["degree"]["N"], "9")

    def test_delete_removes_the_catalog_row(self):
        self._seed_job("compute_del")
        self._list()
        self.assertIn("compute_del", self.ddb.catalog_rows())
        hs.handle_delete({"body": json.dumps({"job_id": "compute_del"})})
        self.assertNotIn("compute_del", self.ddb.catalog_rows())

    def test_catalog_partition_is_reserved(self):
        with self.assertRaises(ValueError):
            hs._assert_mutable_job_partition("results#catalog")

    # ── code-review-30 F9: batched writes, observable cost ───────────────
    def test_first_build_uses_bounded_batches_not_per_row_puts(self):
        for i in range(60):
            self._seed_job(f"compute_{i:03d}")
        puts = []
        self.ddb.put_item = lambda **kw: puts.append(kw)   # any per-row put is a regression
        body = self._list()
        self.assertEqual(body["count"], 60)
        self.assertEqual(puts, [])                          # no serial PutItem round trips
        self.assertEqual(self.ddb.batch_calls, 3)           # ceil(60/25) bounded batches
        self.assertEqual(body["catalog_writes_attempted"], 60)
        self.assertEqual(body["catalog_writes_failed"], 0)
        self.assertIn("catalog_write_us", body)
        self.assertIn("catalog_prune_us", body)

    def test_unprocessed_items_are_retried(self):
        self._seed_job("compute_a")
        self._seed_job("compute_b")
        self.ddb.unprocessed_once = True
        body = self._list()
        self.assertEqual(body["catalog_writes_failed"], 0)  # retried to completion
        self.assertEqual(set(self.ddb.catalog_rows()), {"compute_a", "compute_b"})

    # ── code-review-30 F10: schema-versioned rows ────────────────────────
    def test_old_schema_rows_reconcile_automatically(self):
        self._seed_job("compute_a")
        self._list()
        key = (hs.RESULTS_CATALOG_DDB_JOB_ID, hs._results_catalog_task_id("compute_a"))
        row = self.ddb.items[key]
        self.assertEqual(row["v"]["N"], str(hs.RESULTS_CATALOG_SCHEMA_VERSION))
        self.assertIn("calc_etag", row)                     # source identity recorded
        # simulate a row written by an OLDER release
        del row["v"]
        n = len(self._calc_gets())
        body = self._list()
        self.assertEqual(len(self._calc_gets()), n + 1)     # re-read, not trusted
        self.assertEqual(body["catalog_misses"], 1)
        self.assertEqual(self.ddb.items[key]["v"]["N"], str(hs.RESULTS_CATALOG_SCHEMA_VERSION))
        # current-version row is trusted again
        n = len(self._calc_gets())
        self._list()
        self.assertEqual(len(self._calc_gets()), n)

    def test_catalog_write_failure_never_breaks_the_read(self):
        self._seed_job("compute_a")
        def boom(**kw):
            raise _ce("ProvisionedThroughputExceededException", 400, "BatchWriteItem")
        self.ddb.batch_write_item = boom
        body = self._list()                                       # still answers
        self.assertEqual([r["job_id"] for r in body["results"]], ["compute_a"])
        self.assertEqual(body["catalog_writes_failed"], body["catalog_writes_attempted"])


if __name__ == "__main__":
    unittest.main()
