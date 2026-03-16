"""
Tests for the Lambda render pipeline — key consistency, dispatch logic,
storage operations, merge pairing, and S3 prefix matching.

Run: cd polypaint/lambda && python -m pytest test_pipeline.py -v
"""
import json
import math
import re
import struct
import unittest
from unittest.mock import MagicMock, patch

# ── Helpers to simulate JS pipeline logic ──────────────────────────────────


def build_render_keys(job_id, n_stripes, T):
    """Simulate JS renderJobs key construction (tiled mode)."""
    pix = 50000
    tile_rows = math.ceil(pix / T)
    keys = []
    for s in range(n_stripes):
        for t in range(T):
            y_start = t * tile_rows
            y_height = min(tile_rows, pix - y_start)
            if y_height <= 0:
                continue
            keys.append(f"renders/{job_id}/stripe_{s}_t{t}.raw")
    return keys


def build_render_keys_full(job_id, n_stripes):
    """Simulate JS renderJobs key construction (full mode)."""
    return [f"renders/{job_id}/stripe_{s}.raw" for s in range(n_stripes)]


def build_dispatch_expected_keys(jobs):
    """Simulate handler_dispatch.py expected_keys construction."""
    keys = []
    for job in jobs:
        job_id = job["job_id"]
        stripe_idx = job["stripe_idx"]
        tile_idx = job.get("tile_idx")
        if tile_idx is not None:
            keys.append(f"renders/{job_id}/stripe_{stripe_idx}_t{tile_idx}.raw")
        else:
            keys.append(f"renders/{job_id}/stripe_{stripe_idx}.raw")
    return keys


def build_render_jobs_tiled(job_id, n_stripes, T):
    """Simulate the JS renderJobs array for dispatch."""
    pix = 50000
    tile_rows = math.ceil(pix / T)
    jobs = []
    for s in range(n_stripes):
        for t in range(T):
            y_start = t * tile_rows
            y_height = min(tile_rows, pix - y_start)
            if y_height <= 0:
                continue
            jobs.append({
                "job_id": job_id, "stripe_idx": s, "tile_idx": t,
                "width": pix, "height": pix,
                "y_start": y_start, "y_height": y_height,
            })
    return jobs


def simulate_tiled_merge(job_id, n_stripes, T):
    """Simulate the JS tiled merge phase, returning all merge keys per round."""
    # Build initial keys (array of arrays: keys[stripe][tile])
    keys = []
    for s in range(n_stripes):
        stripe_tiles = [f"renders/{job_id}/stripe_{s}_t{t}.raw" for t in range(T)]
        keys.append(stripe_tiles)

    all_rounds = []
    round_num = 0
    while len(keys) > 1:
        next_keys = []
        tile_jobs = []
        expected_out_keys = []
        pair_count = len(keys) // 2

        for i in range(0, len(keys), 2):
            if i + 1 >= len(keys):
                next_keys.append(keys[i])
                continue
            pair_idx = i // 2
            out_tile_keys = []
            for t in range(len(keys[i])):
                out_key = f"renders/{job_id}/merge_{round_num}_{pair_idx}_t{t}.raw"
                out_tile_keys.append(out_key)
                expected_out_keys.append(out_key)
                tile_jobs.append({
                    "left_key": keys[i][t],
                    "right_key": keys[i + 1][t],
                    "out_key": out_key,
                })
            next_keys.append(out_tile_keys)

        all_rounds.append({
            "round": round_num,
            "pair_count": pair_count,
            "tile_jobs": tile_jobs,
            "expected_out_keys": expected_out_keys,
            "merge_prefix": f"renders/{job_id}/merge_{round_num}_",
        })
        keys = next_keys
        round_num += 1

    return all_rounds, keys[0] if keys else []


def simulate_full_merge(job_id, n_stripes):
    """Simulate the JS full merge phase."""
    keys = [f"renders/{job_id}/stripe_{s}.raw" for s in range(n_stripes)]

    all_rounds = []
    round_num = 0
    while len(keys) > 1:
        pairs = []
        next_keys = []
        expected_out_keys = []
        for i in range(0, len(keys), 2):
            if i + 1 < len(keys):
                out_key = f"renders/{job_id}/merge_{round_num}_{i // 2}.raw"
                pairs.append({
                    "left_key": keys[i], "right_key": keys[i + 1],
                    "out_key": out_key,
                })
                next_keys.append(out_key)
                expected_out_keys.append(out_key)
            else:
                next_keys.append(keys[i])

        all_rounds.append({
            "round": round_num,
            "pair_count": len(pairs),
            "expected_out_keys": expected_out_keys,
            "merge_prefix": f"renders/{job_id}/merge_{round_num}_",
        })
        keys = next_keys
        round_num += 1

    return all_rounds, keys[0] if keys else None


# ── Test: Key pattern consistency ──────────────────────────────────────────


class TestKeyPatterns(unittest.TestCase):
    """Verify key patterns are consistent across render, dispatch, poll, merge."""

    def test_tiled_render_keys_match_dispatch(self):
        """Render job keys must match what dispatch builds for expected_keys."""
        job_id = "test_job"
        jobs = build_render_jobs_tiled(job_id, 10, 4)
        expected = build_render_keys(job_id, 10, 4)
        dispatch_keys = build_dispatch_expected_keys(jobs)
        self.assertEqual(dispatch_keys, expected)

    def test_full_render_keys_match_dispatch(self):
        """Full-mode render keys match dispatch expected_keys."""
        job_id = "test_job"
        jobs = [{"job_id": job_id, "stripe_idx": s} for s in range(10)]
        expected = build_render_keys_full(job_id, 10)
        dispatch_keys = build_dispatch_expected_keys(jobs)
        self.assertEqual(dispatch_keys, expected)

    def test_render_poll_prefix_only_matches_render_keys(self):
        """Render poll prefix must NOT match merge keys."""
        job_id = "test_job"
        poll_prefix = f"renders/{job_id}/stripe_"

        render_keys = build_render_keys(job_id, 10, 4)
        for key in render_keys:
            self.assertTrue(key.startswith(poll_prefix),
                            f"Render key {key} should match poll prefix")

        rounds, _ = simulate_tiled_merge(job_id, 10, 4)
        for rd in rounds:
            for key in rd["expected_out_keys"]:
                self.assertFalse(key.startswith(poll_prefix),
                                 f"Merge key {key} should NOT match render poll prefix")

    def test_merge_prefix_only_matches_own_round(self):
        """Merge poll prefix for round N must not match round M keys (M != N)."""
        job_id = "test_job"
        rounds, _ = simulate_tiled_merge(job_id, 500, 8)

        for rd in rounds:
            prefix = rd["merge_prefix"]
            # All keys in this round should match
            for key in rd["expected_out_keys"]:
                self.assertTrue(key.startswith(prefix),
                                f"Key {key} should match prefix {prefix}")

            # No keys from other rounds should match
            for other_rd in rounds:
                if other_rd["round"] == rd["round"]:
                    continue
                for key in other_rd["expected_out_keys"]:
                    self.assertFalse(key.startswith(prefix),
                                     f"Key {key} from round {other_rd['round']} "
                                     f"should NOT match round {rd['round']} prefix {prefix}")

    def test_merge_prefix_no_cross_round_collision(self):
        """merge_1_ must not match merge_10_, merge_11_, etc."""
        self.assertFalse("renders/j/merge_10_0.raw".startswith("renders/j/merge_1_"))
        self.assertFalse("renders/j/merge_11_0.raw".startswith("renders/j/merge_1_"))
        self.assertTrue("renders/j/merge_1_0.raw".startswith("renders/j/merge_1_"))

    def test_render_key_regex_matches_tiled(self):
        """The JS regex for parsing render keys must match tiled keys."""
        pattern = re.compile(r'stripe_(\d+)(?:_t(\d+))?\.raw$')
        key = "renders/job1/stripe_42_t7.raw"
        m = pattern.search(key)
        self.assertIsNotNone(m)
        self.assertEqual(m.group(1), "42")
        self.assertEqual(m.group(2), "7")

    def test_render_key_regex_matches_full(self):
        """The JS regex for parsing render keys must match full-mode keys."""
        pattern = re.compile(r'stripe_(\d+)(?:_t(\d+))?\.raw$')
        key = "renders/job1/stripe_42.raw"
        m = pattern.search(key)
        self.assertIsNotNone(m)
        self.assertEqual(m.group(1), "42")
        self.assertIsNone(m.group(2))

    def test_render_key_regex_rejects_merge_keys(self):
        """Merge keys must NOT match the render key regex."""
        pattern = re.compile(r'stripe_(\d+)(?:_t(\d+))?\.raw$')
        merge_key = "renders/job1/merge_0_3_t2.raw"
        m = pattern.search(merge_key)
        self.assertIsNone(m)

    def test_tile_sort_regex_matches(self):
        """The _t(N) sort regex used in tiled merge must match tile keys."""
        pattern = re.compile(r'_t(\d+)\.raw$')
        self.assertEqual(pattern.search("stripe_5_t3.raw").group(1), "3")
        self.assertEqual(pattern.search("merge_0_2_t11.raw").group(1), "11")
        self.assertIsNone(pattern.search("stripe_5.raw"))


# ── Test: Merge pairing logic ──────────────────────────────────────────────


class TestMergePairing(unittest.TestCase):
    """Verify tree-reduce pairing and round counts."""

    def test_even_stripe_count(self):
        """500 stripes → 250 pairs in round 0, 9 rounds total."""
        rounds, final = simulate_full_merge("j", 500)
        self.assertEqual(rounds[0]["pair_count"], 250)
        self.assertEqual(len(rounds), 9)  # ceil(log2(500))
        self.assertIsNotNone(final)

    def test_odd_stripe_count(self):
        """501 stripes → 250 pairs + 1 carry-forward in round 0."""
        rounds, final = simulate_full_merge("j", 501)
        self.assertEqual(rounds[0]["pair_count"], 250)
        self.assertIsNotNone(final)

    def test_two_stripes(self):
        """2 stripes → 1 round, 1 pair."""
        rounds, final = simulate_full_merge("j", 2)
        self.assertEqual(len(rounds), 1)
        self.assertEqual(rounds[0]["pair_count"], 1)
        self.assertEqual(final, "renders/j/merge_0_0.raw")

    def test_one_stripe(self):
        """1 stripe → 0 rounds, final is the render key."""
        rounds, final = simulate_full_merge("j", 1)
        self.assertEqual(len(rounds), 0)
        self.assertEqual(final, "renders/j/stripe_0.raw")

    def test_tiled_merge_tile_count(self):
        """Tiled merge: each round has pairCount × T tile jobs."""
        rounds, final_tiles = simulate_tiled_merge("j", 10, 8)
        for rd in rounds:
            self.assertEqual(len(rd["tile_jobs"]), rd["pair_count"] * 8)

    def test_tiled_merge_preserves_tile_count(self):
        """Final result should have T tile keys."""
        _, final_tiles = simulate_tiled_merge("j", 10, 8)
        self.assertEqual(len(final_tiles), 8)

    def test_tiled_merge_left_right_consistent(self):
        """Merge jobs pair tiles by index: left[t] with right[t]."""
        rounds, _ = simulate_tiled_merge("j", 4, 3)
        for rd in rounds:
            for job in rd["tile_jobs"]:
                left_t = re.search(r'_t(\d+)\.raw$', job["left_key"])
                right_t = re.search(r'_t(\d+)\.raw$', job["right_key"])
                out_t = re.search(r'_t(\d+)\.raw$', job["out_key"])
                self.assertIsNotNone(left_t)
                self.assertIsNotNone(right_t)
                self.assertIsNotNone(out_t)
                self.assertEqual(left_t.group(1), right_t.group(1))
                self.assertEqual(left_t.group(1), out_t.group(1))

    def test_merge_expected_count_excludes_carryforward(self):
        """Poll expected count must equal tile_jobs count, not include carry-forward."""
        rounds, _ = simulate_tiled_merge("j", 501, 4)
        # Round 0: 250 pairs (501 // 2), 1 carry-forward
        rd0 = rounds[0]
        self.assertEqual(rd0["pair_count"], 250)
        self.assertEqual(len(rd0["expected_out_keys"]), 250 * 4)
        self.assertEqual(len(rd0["tile_jobs"]), 250 * 4)

    def test_batch_keys_align_with_jobs(self):
        """Batch slicing of expectedOutKeys must align with job slicing."""
        rounds, _ = simulate_tiled_merge("j", 20, 4)
        for rd in rounds:
            jobs = rd["tile_jobs"]
            keys = rd["expected_out_keys"]
            BATCH = 500
            for b in range(0, len(jobs), BATCH):
                batch_jobs = jobs[b:b + BATCH]
                batch_keys = keys[b:b + BATCH]
                self.assertEqual(len(batch_jobs), len(batch_keys))
                for job, key in zip(batch_jobs, batch_keys):
                    self.assertEqual(job["out_key"], key)


# ── Test: handler_dispatch.py ──────────────────────────────────────────────


class TestDispatchHandler(unittest.TestCase):

    def _make_event(self, body):
        return {"body": json.dumps(body)}

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_fires_all_jobs(self, mock_client):
        from handler_dispatch import handler
        jobs = [{"job_id": "j", "stripe_idx": i} for i in range(5)]
        event = self._make_event({"jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 5)
        self.assertEqual(body["total"], 5)
        self.assertEqual(mock_client.invoke.call_count, 5)

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_expected_keys_tiled(self, mock_client):
        from handler_dispatch import handler
        jobs = [
            {"job_id": "j", "stripe_idx": 0, "tile_idx": 0},
            {"job_id": "j", "stripe_idx": 0, "tile_idx": 1},
            {"job_id": "j", "stripe_idx": 1, "tile_idx": 0},
        ]
        event = self._make_event({"jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["expected_keys"], [
            "renders/j/stripe_0_t0.raw",
            "renders/j/stripe_0_t1.raw",
            "renders/j/stripe_1_t0.raw",
        ])

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_expected_keys_full(self, mock_client):
        from handler_dispatch import handler
        jobs = [{"job_id": "j", "stripe_idx": s} for s in range(3)]
        event = self._make_event({"jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["expected_keys"], [
            "renders/j/stripe_0.raw",
            "renders/j/stripe_1.raw",
            "renders/j/stripe_2.raw",
        ])

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_uses_caller_expected_keys(self, mock_client):
        """Merge calls provide expected_keys explicitly — dispatch must use them."""
        from handler_dispatch import handler
        caller_keys = ["renders/j/merge_0_0.raw", "renders/j/merge_0_1.raw"]
        jobs = [
            {"left_key": "a.raw", "right_key": "b.raw", "out_key": "renders/j/merge_0_0.raw"},
            {"left_key": "c.raw", "right_key": "d.raw", "out_key": "renders/j/merge_0_1.raw"},
        ]
        event = self._make_event({
            "target": "reduce", "jobs": jobs, "expected_keys": caller_keys
        })
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["expected_keys"], caller_keys)

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_unknown_target(self, mock_client):
        from handler_dispatch import handler
        event = self._make_event({"target": "bogus", "jobs": []})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertIn("error", body)

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_reports_errors(self, mock_client):
        from handler_dispatch import handler
        mock_client.invoke.side_effect = Exception("throttled")
        jobs = [{"job_id": "j", "stripe_idx": 0}]
        event = self._make_event({"jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 0)
        self.assertEqual(len(body["errors"]), 1)


# ── Test: handler_storage.py (check_keys, clean_render) ───────────────────


class TestStorageCheckKeys(unittest.TestCase):

    @patch("handler_storage.s3")
    def test_check_keys_counts_matching_suffix(self, mock_s3):
        from handler_storage import handle_check_keys
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{
            "Contents": [
                {"Key": "renders/j/stripe_0_t0.raw"},
                {"Key": "renders/j/stripe_0_t1.raw"},
                {"Key": "renders/j/calc.json"},
                {"Key": "renders/j/stripe_1_t0.raw"},
            ]
        }]
        event = {"body": json.dumps({
            "prefix": "renders/j/stripe_",
            "expected": 3,
            "suffix": ".raw",
        })}
        result = handle_check_keys(event)
        body = json.loads(result["body"])
        self.assertEqual(body["found"], 3)
        self.assertTrue(body["done"])

    @patch("handler_storage.s3")
    def test_check_keys_excludes_merge_files(self, mock_s3):
        """Poll with stripe_ prefix must not count merge_ files.
        S3 prefix filtering is server-side, so mock only returns matching keys.
        """
        from handler_storage import handle_check_keys
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        # S3 paginate(Prefix="renders/j/stripe_") only returns stripe_ keys
        mock_paginator.paginate.return_value = [{
            "Contents": [
                {"Key": "renders/j/stripe_0.raw"},
                {"Key": "renders/j/stripe_1.raw"},
            ]
        }]
        event = {"body": json.dumps({
            "prefix": "renders/j/stripe_",
            "expected": 3,
            "suffix": ".raw",
        })}
        result = handle_check_keys(event)
        body = json.loads(result["body"])
        self.assertEqual(body["found"], 2)
        self.assertFalse(body["done"])  # expected 3, found 2

    @patch("handler_storage.s3")
    def test_check_keys_merge_prefix(self, mock_s3):
        """Merge poll prefix correctly counts merge files for specific round."""
        from handler_storage import handle_check_keys
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{
            "Contents": [
                {"Key": "renders/j/merge_0_0_t0.raw"},
                {"Key": "renders/j/merge_0_0_t1.raw"},
                {"Key": "renders/j/merge_0_1_t0.raw"},
                {"Key": "renders/j/merge_0_1_t1.raw"},
            ]
        }]
        event = {"body": json.dumps({
            "prefix": "renders/j/merge_0_",
            "expected": 4,
            "suffix": ".raw",
        })}
        result = handle_check_keys(event)
        body = json.loads(result["body"])
        self.assertEqual(body["found"], 4)
        self.assertTrue(body["done"])

    @patch("handler_storage.s3")
    def test_check_keys_not_done(self, mock_s3):
        from handler_storage import handle_check_keys
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"Contents": [
            {"Key": "renders/j/stripe_0.raw"},
        ]}]
        event = {"body": json.dumps({
            "prefix": "renders/j/stripe_",
            "expected": 10,
            "suffix": ".raw",
        })}
        result = handle_check_keys(event)
        body = json.loads(result["body"])
        self.assertEqual(body["found"], 1)
        self.assertFalse(body["done"])


class TestStorageCleanRender(unittest.TestCase):

    @patch("handler_storage.s3")
    def test_clean_render_deletes_raw_and_image_files(self, mock_s3):
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{
            "Contents": [
                {"Key": "renders/j/stripe_0.raw"},
                {"Key": "renders/j/stripe_0_t0.raw"},
                {"Key": "renders/j/merge_0_0.raw"},
                {"Key": "renders/j/image.jpeg"},
                {"Key": "renders/j/calc.json"},
                {"Key": "renders/j/stripe_0.bin"},
                {"Key": "renders/j/preview.jpg"},
            ]
        }]
        mock_s3.delete_objects.return_value = {"Deleted": [
            {"Key": "renders/j/stripe_0.raw"},
            {"Key": "renders/j/stripe_0_t0.raw"},
            {"Key": "renders/j/merge_0_0.raw"},
            {"Key": "renders/j/image.jpeg"},
            {"Key": "renders/j/preview.jpg"},
        ]}
        event = {"body": json.dumps({"job_id": "j"})}
        result = handle_clean_render(event)
        body = json.loads(result["body"])
        self.assertEqual(body["deleted"], 5)

        # Verify which files were passed to delete_objects
        call_args = mock_s3.delete_objects.call_args
        deleted_keys = [o["Key"] for o in call_args[1]["Delete"]["Objects"]]
        self.assertIn("renders/j/merge_0_0.raw", deleted_keys)
        self.assertNotIn("renders/j/calc.json", deleted_keys)
        self.assertNotIn("renders/j/stripe_0.bin", deleted_keys)


# ── Test: shared.py ───────────────────────────────────────────────────────


class TestShared(unittest.TestCase):

    def test_parse_body_string(self):
        from shared import parse_body
        event = {"body": '{"foo": 42}'}
        self.assertEqual(parse_body(event), {"foo": 42})

    def test_parse_body_dict(self):
        from shared import parse_body
        event = {"body": {"foo": 42}}
        self.assertEqual(parse_body(event), {"foo": 42})

    def test_parse_body_no_body(self):
        from shared import parse_body
        event = {"foo": 42}
        self.assertEqual(parse_body(event), {"foo": 42})

    def test_ok_response_format(self):
        from shared import ok_response
        resp = ok_response({"x": 1})
        self.assertEqual(resp["statusCode"], 200)
        self.assertEqual(resp["headers"]["Content-Type"], "application/json")
        self.assertEqual(resp["headers"]["Access-Control-Allow-Origin"], "*")
        self.assertEqual(json.loads(resp["body"]), {"x": 1})

    def test_compute_viewport_empty(self):
        from shared import compute_viewport_from_bin
        result = compute_viewport_from_bin(b"")
        self.assertEqual(result["n_roots"], 0)
        self.assertEqual(result["scale"], 1.0)

    def test_compute_viewport_single_root(self):
        from shared import compute_viewport_from_bin
        data = struct.pack("<ff", 1.0, 2.0)
        result = compute_viewport_from_bin(data)
        self.assertEqual(result["n_roots"], 1)
        self.assertAlmostEqual(result["center_re"], 1.0)
        self.assertAlmostEqual(result["center_im"], 2.0)

    def test_compute_viewport_filters_nan(self):
        from shared import compute_viewport_from_bin
        data = struct.pack("<ffff", float('nan'), float('nan'), 1.0, 2.0)
        result = compute_viewport_from_bin(data)
        self.assertEqual(result["n_roots"], 1)


# ── Test: handler_render.py S3 key construction ────────────────────────────


class TestRenderKeyConstruction(unittest.TestCase):

    def test_tiled_key_format(self):
        """Render handler builds stripe_S_tT.raw for tiled jobs."""
        # Simulate the key construction logic from handler_render.py
        job_id, stripe_idx, tile_idx = "j", 5, 3
        s3_key = f"renders/{job_id}/stripe_{stripe_idx}_t{tile_idx}.raw"
        self.assertEqual(s3_key, "renders/j/stripe_5_t3.raw")

    def test_full_key_format(self):
        """Render handler builds stripe_S.raw for full-mode jobs."""
        job_id, stripe_idx, tile_idx = "j", 5, None
        if tile_idx is not None:
            s3_key = f"renders/{job_id}/stripe_{stripe_idx}_t{tile_idx}.raw"
        else:
            s3_key = f"renders/{job_id}/stripe_{stripe_idx}.raw"
        self.assertEqual(s3_key, "renders/j/stripe_5.raw")


# ── Test: End-to-end key flow ──────────────────────────────────────────────


class TestEndToEndKeyFlow(unittest.TestCase):
    """Verify the complete key flow: render → dispatch → poll → merge → encode."""

    def test_tiled_500_stripes_8_tiles(self):
        """Full pipeline simulation with 500 stripes × 8 tiles."""
        job_id = "compute_test123"
        n_stripes, T = 500, 8

        # 1. Build render jobs
        render_keys = build_render_keys(job_id, n_stripes, T)
        self.assertEqual(len(render_keys), 4000)

        # 2. Verify render poll prefix matches ALL render keys
        poll_prefix = f"renders/{job_id}/stripe_"
        for key in render_keys:
            self.assertTrue(key.startswith(poll_prefix))

        # 3. Simulate merge
        rounds, final_tiles = simulate_tiled_merge(job_id, n_stripes, T)

        # 4. Verify NO merge keys match render poll prefix
        for rd in rounds:
            for key in rd["expected_out_keys"]:
                self.assertFalse(key.startswith(poll_prefix),
                                 f"Merge key leaked into render prefix: {key}")

        # 5. Verify merge poll counts are correct
        for rd in rounds:
            prefix = rd["merge_prefix"]
            matching = [k for k in rd["expected_out_keys"] if k.startswith(prefix)]
            self.assertEqual(len(matching), len(rd["expected_out_keys"]))

        # 6. Final tiles should have T keys
        self.assertEqual(len(final_tiles), T)

        # 7. Total rounds
        self.assertEqual(len(rounds), 9)  # ceil(log2(500))

    def test_full_10_stripes(self):
        """Full-mode pipeline with 10 stripes."""
        job_id = "compute_test456"
        n_stripes = 10

        render_keys = build_render_keys_full(job_id, n_stripes)
        self.assertEqual(len(render_keys), 10)

        rounds, final_key = simulate_full_merge(job_id, n_stripes)
        self.assertEqual(len(rounds), 4)  # ceil(log2(10))
        self.assertIn("merge_3_0.raw", final_key)

        # Verify cleanup would collect all intermediate keys
        all_temp = []
        for rd in rounds:
            all_temp.extend(rd["expected_out_keys"])
        # All render keys + all merge keys
        total_cleanup = len(render_keys) + len(all_temp)
        self.assertGreater(total_cleanup, n_stripes)


# ── Test: DynamoDB status tracking ──────────────────────────────────────────


class TestReportStatus(unittest.TestCase):
    """Test shared.report_status writes correct DynamoDB items."""

    @patch("shared._ddb", None)
    @patch("shared.boto3")
    def test_report_done(self, mock_boto3):
        import shared
        shared._ddb = None  # reset lazy init
        mock_ddb = MagicMock()
        mock_boto3.client.return_value = mock_ddb

        shared.report_status("job1", "stripe_0", "done")

        mock_ddb.put_item.assert_called_once()
        item = mock_ddb.put_item.call_args[1]["Item"]
        self.assertEqual(item["job_id"]["S"], "job1")
        self.assertEqual(item["task_id"]["S"], "stripe_0")
        self.assertEqual(item["task_status"]["S"], "done")
        self.assertIn("ttl", item)
        self.assertNotIn("error_msg", item)
        shared._ddb = None  # cleanup

    @patch("shared._ddb", None)
    @patch("shared.boto3")
    def test_report_error(self, mock_boto3):
        import shared
        shared._ddb = None
        mock_ddb = MagicMock()
        mock_boto3.client.return_value = mock_ddb

        shared.report_status("job1", "stripe_5_t3", "error", "rawreduce failed: OOM")

        item = mock_ddb.put_item.call_args[1]["Item"]
        self.assertEqual(item["task_status"]["S"], "error")
        self.assertEqual(item["error_msg"]["S"], "rawreduce failed: OOM")
        shared._ddb = None

    @patch("shared._ddb", None)
    @patch("shared.boto3")
    def test_error_msg_truncated(self, mock_boto3):
        import shared
        shared._ddb = None
        mock_ddb = MagicMock()
        mock_boto3.client.return_value = mock_ddb

        shared.report_status("job1", "t", "error", "x" * 2000)

        item = mock_ddb.put_item.call_args[1]["Item"]
        self.assertEqual(len(item["error_msg"]["S"]), 1000)
        shared._ddb = None


class TestCheckStatus(unittest.TestCase):
    """Test handle_check_status DynamoDB queries."""

    @patch("handler_storage._get_ddb")
    def test_all_done(self, mock_get_ddb):
        from handler_storage import handle_check_status
        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        mock_ddb.query.return_value = {
            "Items": [
                {"task_id": {"S": "stripe_0"}, "task_status": {"S": "done"}},
                {"task_id": {"S": "stripe_1"}, "task_status": {"S": "done"}},
                {"task_id": {"S": "stripe_2"}, "task_status": {"S": "done"}},
            ],
        }
        event = {"body": json.dumps({
            "job_id": "j", "task_prefix": "stripe_", "expected": 3,
        })}
        result = handle_check_status(event)
        body = json.loads(result["body"])
        self.assertEqual(body["done"], 3)
        self.assertEqual(body["errors"], 0)
        self.assertTrue(body["complete"])

    @patch("handler_storage._get_ddb")
    def test_partial_with_errors(self, mock_get_ddb):
        from handler_storage import handle_check_status
        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        mock_ddb.query.return_value = {
            "Items": [
                {"task_id": {"S": "stripe_0"}, "task_status": {"S": "done"}},
                {"task_id": {"S": "stripe_1"}, "task_status": {"S": "error"},
                 "error_msg": {"S": "rawreduce failed"}},
            ],
        }
        event = {"body": json.dumps({
            "job_id": "j", "task_prefix": "stripe_", "expected": 3,
        })}
        result = handle_check_status(event)
        body = json.loads(result["body"])
        self.assertEqual(body["done"], 1)
        self.assertEqual(body["errors"], 1)
        self.assertFalse(body["complete"])  # 2 < 3
        self.assertEqual(body["error_details"][0]["task_id"], "stripe_1")
        self.assertEqual(body["error_details"][0]["error_msg"], "rawreduce failed")

    @patch("handler_storage._get_ddb")
    def test_empty_result(self, mock_get_ddb):
        from handler_storage import handle_check_status
        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        mock_ddb.query.return_value = {"Items": []}
        event = {"body": json.dumps({
            "job_id": "j", "task_prefix": "merge_0_", "expected": 250,
        })}
        result = handle_check_status(event)
        body = json.loads(result["body"])
        self.assertEqual(body["done"], 0)
        self.assertEqual(body["total"], 0)
        self.assertFalse(body["complete"])

    @patch("handler_storage._get_ddb")
    def test_merge_prefix_isolation(self, mock_get_ddb):
        """merge_0_ prefix must not match merge_1_ items (DynamoDB begins_with)."""
        from handler_storage import handle_check_status
        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        # DynamoDB begins_with on sort key — only round 0 items returned
        mock_ddb.query.return_value = {
            "Items": [
                {"task_id": {"S": "merge_0_0"}, "task_status": {"S": "done"}},
                {"task_id": {"S": "merge_0_1"}, "task_status": {"S": "done"}},
            ],
        }
        event = {"body": json.dumps({
            "job_id": "j", "task_prefix": "merge_0_", "expected": 2,
        })}
        result = handle_check_status(event)
        body = json.loads(result["body"])
        self.assertEqual(body["done"], 2)
        self.assertTrue(body["complete"])

    @patch("handler_storage._get_ddb")
    def test_paginated_query(self, mock_get_ddb):
        """Handle DynamoDB pagination (LastEvaluatedKey)."""
        from handler_storage import handle_check_status
        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        # First page returns 2 items + continuation token
        # Second page returns 1 item + no continuation
        mock_ddb.query.side_effect = [
            {
                "Items": [
                    {"task_id": {"S": "stripe_0"}, "task_status": {"S": "done"}},
                    {"task_id": {"S": "stripe_1"}, "task_status": {"S": "done"}},
                ],
                "LastEvaluatedKey": {"job_id": {"S": "j"}, "task_id": {"S": "stripe_1"}},
            },
            {
                "Items": [
                    {"task_id": {"S": "stripe_2"}, "task_status": {"S": "done"}},
                ],
            },
        ]
        event = {"body": json.dumps({
            "job_id": "j", "task_prefix": "stripe_", "expected": 3,
        })}
        result = handle_check_status(event)
        body = json.loads(result["body"])
        self.assertEqual(body["done"], 3)
        self.assertTrue(body["complete"])
        self.assertEqual(mock_ddb.query.call_count, 2)


class TestCleanRenderDynamoDB(unittest.TestCase):
    """Test that clean-render also clears DynamoDB status entries."""

    @patch("handler_storage._get_ddb")
    @patch("handler_storage.s3")
    def test_clean_render_clears_ddb(self, mock_s3, mock_get_ddb):
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"Contents": [
            {"Key": "renders/j/stripe_0.raw"},
        ]}]
        mock_s3.delete_objects.return_value = {"Deleted": [{"Key": "renders/j/stripe_0.raw"}]}

        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        mock_ddb.query.return_value = {
            "Items": [
                {"job_id": {"S": "j"}, "task_id": {"S": "stripe_0"}},
                {"job_id": {"S": "j"}, "task_id": {"S": "stripe_1"}},
            ],
        }

        event = {"body": json.dumps({"job_id": "j"})}
        result = handle_clean_render(event)
        body = json.loads(result["body"])

        self.assertEqual(body["deleted"], 1)  # S3 files
        self.assertEqual(body["ddb_deleted"], 2)  # DynamoDB items
        mock_ddb.batch_write_item.assert_called_once()


class TestTaskIdDerivation(unittest.TestCase):
    """Test task_id derivation from out_key in reduce handlers."""

    def test_full_merge_task_id(self):
        out_key = "renders/compute_abc/merge_0_42.raw"
        task_id = out_key.rsplit("/", 1)[-1].replace(".raw", "")
        self.assertEqual(task_id, "merge_0_42")

    def test_tiled_merge_task_id(self):
        out_key = "renders/compute_abc/merge_2_15_t7.raw"
        task_id = out_key.rsplit("/", 1)[-1].replace(".raw", "")
        self.assertEqual(task_id, "merge_2_15_t7")

    def test_render_task_id_full(self):
        stripe_idx, tile_idx = 42, None
        task_id = f"stripe_{stripe_idx}_t{tile_idx}" if tile_idx is not None else f"stripe_{stripe_idx}"
        self.assertEqual(task_id, "stripe_42")

    def test_render_task_id_tiled(self):
        stripe_idx, tile_idx = 42, 3
        task_id = f"stripe_{stripe_idx}_t{tile_idx}" if tile_idx is not None else f"stripe_{stripe_idx}"
        self.assertEqual(task_id, "stripe_42_t3")

    def test_job_id_from_out_key_fallback(self):
        """When job_id not in params, derive from out_key."""
        out_key = "renders/compute_xyz/merge_0_5.raw"
        job_id_from_key = out_key.split("/")[1]
        self.assertEqual(job_id_from_key, "compute_xyz")


if __name__ == "__main__":
    unittest.main()
