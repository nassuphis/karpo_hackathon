"""
Tests for the dispatch resilience fixes:
  1. check-status return_ids — DynamoDB returns found task IDs on request
  2. head-keys endpoint — batch HEAD existence check for S3 keys
  3. dispatch bilevel target — fires async bilevel Lambdas

Run: cd polypaint && python -m pytest tests/test_dispatch_resilience.py -v
"""
import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch, call

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'lambda'))


# ── Test: check-status return_ids ──────────────────────────────────────────


class TestCheckStatusReturnIds(unittest.TestCase):
    """Verify check-status returns found_ids when return_ids=True."""

    def _make_event(self, body):
        return {"body": json.dumps(body)}

    @patch("handler_storage._get_ddb")
    def test_return_ids_false_by_default(self, mock_get_ddb):
        """Without return_ids, response has no found_ids key."""
        from handler_storage import handle_check_status
        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        mock_ddb.query.return_value = {
            "Items": [
                {"task_id": {"S": "raster_0"}, "task_status": {"S": "done"}},
                {"task_id": {"S": "raster_1"}, "task_status": {"S": "done"}},
            ],
        }
        event = self._make_event({
            "job_id": "j", "task_prefix": "raster_", "expected": 3,
        })
        result = handle_check_status(event)
        body = json.loads(result["body"])
        self.assertNotIn("found_ids", body)
        self.assertEqual(body["done"], 2)

    @patch("handler_storage._get_ddb")
    def test_return_ids_true_includes_all_task_ids(self, mock_get_ddb):
        """With return_ids=True, response includes all found task_id strings."""
        from handler_storage import handle_check_status
        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        mock_ddb.query.return_value = {
            "Items": [
                {"task_id": {"S": "bilevel_raster_0"}, "task_status": {"S": "done"}},
                {"task_id": {"S": "bilevel_raster_1"}, "task_status": {"S": "done"}},
                {"task_id": {"S": "bilevel_raster_4"}, "task_status": {"S": "error"},
                 "error_msg": {"S": "OOM"}},
                {"task_id": {"S": "bilevel_raster_7"}, "task_status": {"S": "started"}},
            ],
        }
        event = self._make_event({
            "job_id": "j", "task_prefix": "bilevel_raster_",
            "expected": 10, "return_ids": True,
        })
        result = handle_check_status(event)
        body = json.loads(result["body"])

        self.assertIn("found_ids", body)
        self.assertEqual(set(body["found_ids"]), {
            "bilevel_raster_0", "bilevel_raster_1",
            "bilevel_raster_4", "bilevel_raster_7",
        })
        # Counts are still correct
        self.assertEqual(body["done"], 2)
        self.assertEqual(body["errors"], 1)
        self.assertEqual(body["status_counts"]["done"], 2)
        self.assertEqual(body["status_counts"]["error"], 1)
        self.assertEqual(body["status_counts"]["started"], 1)

    @patch("handler_storage._get_ddb")
    def test_return_ids_detects_missing_tasks(self, mock_get_ddb):
        """Client can diff found_ids against expected to find missing tasks."""
        from handler_storage import handle_check_status
        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        # Simulate 449/500 done, 51 missing (the real failure mode)
        items = [{"task_id": {"S": f"bilevel_raster_{i}"},
                  "task_status": {"S": "done"}}
                 for i in range(449)]
        mock_ddb.query.return_value = {"Items": items}
        event = self._make_event({
            "job_id": "j", "task_prefix": "bilevel_raster_",
            "expected": 500, "return_ids": True,
        })
        result = handle_check_status(event)
        body = json.loads(result["body"])

        found = set(body["found_ids"])
        expected_ids = {f"bilevel_raster_{i}" for i in range(500)}
        missing = expected_ids - found
        self.assertEqual(len(missing), 51)
        self.assertEqual(body["done"], 449)
        self.assertFalse(body["complete"])

    @patch("handler_storage._get_ddb")
    def test_return_ids_with_pagination(self, mock_get_ddb):
        """return_ids works correctly across paginated DynamoDB queries."""
        from handler_storage import handle_check_status
        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        mock_ddb.query.side_effect = [
            {
                "Items": [
                    {"task_id": {"S": "raster_0"}, "task_status": {"S": "done"}},
                    {"task_id": {"S": "raster_1"}, "task_status": {"S": "done"}},
                ],
                "LastEvaluatedKey": {"job_id": {"S": "j"}, "task_id": {"S": "raster_1"}},
            },
            {
                "Items": [
                    {"task_id": {"S": "raster_2"}, "task_status": {"S": "done"}},
                ],
            },
        ]
        event = self._make_event({
            "job_id": "j", "task_prefix": "raster_",
            "expected": 3, "return_ids": True,
        })
        result = handle_check_status(event)
        body = json.loads(result["body"])

        self.assertEqual(set(body["found_ids"]), {"raster_0", "raster_1", "raster_2"})
        self.assertTrue(body["complete"])

    @patch("handler_storage._get_ddb")
    def test_return_ids_explicit_false(self, mock_get_ddb):
        """return_ids=False behaves same as omitting it."""
        from handler_storage import handle_check_status
        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        mock_ddb.query.return_value = {
            "Items": [
                {"task_id": {"S": "raster_0"}, "task_status": {"S": "done"}},
            ],
        }
        event = self._make_event({
            "job_id": "j", "task_prefix": "raster_",
            "expected": 1, "return_ids": False,
        })
        result = handle_check_status(event)
        body = json.loads(result["body"])
        self.assertNotIn("found_ids", body)


# ── Test: head-keys endpoint ──────────────────────────────────────────────


class TestHeadKeys(unittest.TestCase):
    """Verify /head-keys batch HEAD check for S3 key existence."""

    def _make_event(self, body):
        return {"body": json.dumps(body)}

    @patch("handler_storage._head_artifact_keys")
    def test_all_exist(self, mock_head):
        from handler_storage import handle_head_keys
        keys = [
            "renders/j/image_bilevel.tif",
            "renders/j/image_bilevel_preview.png",
        ]
        mock_head.return_value = {
            k: {"exists": True, "key": k, "size": 1, "type": "image/tiff",
                "width": None, "height": None, "url": None}
            for k in keys
        }
        event = self._make_event({"keys": keys})
        result = handle_head_keys(event)
        body = json.loads(result["body"])
        self.assertEqual(set(body["exists"]), set(keys))

    @patch("handler_storage._head_artifact_keys")
    def test_none_exist(self, mock_head):
        from handler_storage import handle_head_keys
        keys = ["renders/j/image_bilevel.tif", "renders/j/image.jpeg"]
        mock_head.return_value = {
            k: {"exists": False, "key": k, "size": 0, "type": "",
                "width": None, "height": None, "url": None}
            for k in keys
        }
        event = self._make_event({"keys": keys})
        result = handle_head_keys(event)
        body = json.loads(result["body"])
        self.assertEqual(body["exists"], [])

    @patch("handler_storage._head_artifact_keys")
    def test_mixed_existence(self, mock_head):
        """Only existing keys returned — matches artifact discovery use case."""
        from handler_storage import handle_head_keys
        existing = {"renders/j/image_bilevel.tif", "renders/j/image_bilevel_preview.png"}
        keys = [
            "renders/j/image_bilevel.tif",
            "renders/j/image_bilevel_preview.png",
            "renders/j/image_bilevel_compat.tif",
            "renders/j/image_bilevel.png",
            "renders/j/image_coeffs_bilevel.tif",
            "renders/j/image_coeffs_bilevel_preview.png",
            "renders/j/image.jpeg",
            "renders/j/image.png",
        ]
        mock_head.return_value = {
            k: {"exists": k in existing, "key": k, "size": 1 if k in existing else 0,
                "type": "image/tiff" if k in existing else "",
                "width": None, "height": None, "url": None}
            for k in keys
        }
        event = self._make_event({"keys": keys})
        result = handle_head_keys(event)
        body = json.loads(result["body"])
        self.assertEqual(set(body["exists"]), existing)
        # Non-existing keys are NOT in the result
        self.assertNotIn("renders/j/image.jpeg", body["exists"])

    @patch("handler_storage._head_artifact_keys")
    def test_empty_keys_list(self, mock_head):
        from handler_storage import handle_head_keys
        event = self._make_event({"keys": []})
        result = handle_head_keys(event)
        body = json.loads(result["body"])
        self.assertEqual(body["exists"], [])
        mock_head.assert_not_called()

    @patch("handler_storage._head_artifact_keys")
    def test_single_key(self, mock_head):
        from handler_storage import handle_head_keys
        key = "renders/j/calc.json"
        mock_head.return_value = {
            key: {"exists": True, "key": key, "size": 123, "type": "application/json",
                  "width": None, "height": None, "url": None}
        }
        event = self._make_event({"keys": [key]})
        result = handle_head_keys(event)
        body = json.loads(result["body"])
        self.assertEqual(body["exists"], [key])

    @patch("handler_storage._head_artifact_keys")
    def test_delegates_keys_to_head_artifact_helper(self, mock_head):
        """handle_head_keys delegates the batch lookup to _head_artifact_keys."""
        from handler_storage import handle_head_keys
        keys = [f"renders/j/key_{i}.tif" for i in range(8)]
        mock_head.return_value = {
            k: {"exists": False, "key": k, "size": 0, "type": "",
                "width": None, "height": None, "url": None}
            for k in keys
        }
        event = self._make_event({"keys": keys})
        handle_head_keys(event)
        mock_head.assert_called_once_with(keys, presign=False)


# ── Test: dispatch bilevel target ─────────────────────────────────────────


class TestDispatchBilevelTarget(unittest.TestCase):
    """Verify dispatch handler supports bilevel and coeff-specific stitch targets."""

    def _make_event(self, body):
        return {"body": json.dumps(body)}

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_bilevel_target(self, mock_client):
        from handler_dispatch import handler
        mock_client.invoke.return_value = {"StatusCode": 202}
        jobs = [{"phase": "raster", "job_id": "j", "stripe_idx": i,
                 "pix": 4096, "tile_size": 4096,
                 "n_tile_cols": 1, "n_tile_rows": 1,
                 "min_re": -20.48, "max_re": 20.48, "min_im": -20.48, "max_im": 20.48, "degree": 5}
                for i in range(3)]
        event = self._make_event({"target": "bilevel", "jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 3)
        self.assertEqual(body["total"], 3)
        # Verify the function name used for invocation
        invoke_call = mock_client.invoke.call_args
        self.assertIn("polypaint-bilevel", invoke_call[1]["FunctionName"])

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_coeff_bilevel_stitch_target(self, mock_client):
        from handler_dispatch import handler
        mock_client.invoke.return_value = {"StatusCode": 202}
        jobs = [{"job_id": "j", "n_tile_cols": 2, "n_tile_rows": 2,
                 "pix": 8192, "tile_size": 4096,
                 "out_key": "renders/j/image_bilevel.tif"}]
        event = self._make_event({"target": "coeff_bilevel_stitch", "jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 1)

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_non_202_tracked(self, mock_client):
        """Non-202 responses are counted as fired but logged in non_202."""
        from handler_dispatch import handler
        mock_client.invoke.return_value = {"StatusCode": 429}
        jobs = [{"phase": "raster", "job_id": "j", "stripe_idx": 0}]
        event = self._make_event({"target": "bilevel", "jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 1)
        self.assertEqual(len(body.get("non_202", [])), 1)
        self.assertEqual(body["non_202"][0]["status"], 429)

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_large_batch(self, mock_client):
        """Dispatch 200 jobs — exercises ThreadPoolExecutor parallelism."""
        from handler_dispatch import handler
        mock_client.invoke.return_value = {"StatusCode": 202}
        jobs = [{"phase": "raster", "job_id": "j", "stripe_idx": i} for i in range(200)]
        event = self._make_event({"target": "bilevel", "jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 200)
        self.assertEqual(body["total"], 200)
        self.assertEqual(mock_client.invoke.call_count, 200)

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_solve_proximity_target(self, mock_client):
        """solve_proximity target fires jobs to polypaint-solve-proximity."""
        from handler_dispatch import handler
        mock_client.invoke.return_value = {"StatusCode": 202}
        jobs = [{"phase": "clip", "job_id": "j", "degree": 70,
                 "lores_bin_key": "renders/j/lores.bin",
                 "out_key": "renders/j/solve_proximity_clip.json",
                 "task_id": "solve_proximity_clip"}]
        event = self._make_event({"target": "solve_proximity", "jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 1)
        invoke_call = mock_client.invoke.call_args
        self.assertIn("polypaint-solve-proximity", invoke_call[1]["FunctionName"])

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_solve_proximity_non_202(self, mock_client):
        """solve_proximity non-202 responses are tracked."""
        from handler_dispatch import handler
        mock_client.invoke.return_value = {"StatusCode": 429}
        jobs = [{"phase": "hist", "job_id": "j", "stripe_idx": 0,
                 "bin_key": "renders/j/stripe_0.bin", "degree": 70,
                 "clip_key": "renders/j/solve_proximity_clip.json",
                 "hist_bins": 100,
                 "out_key": "renders/j/solve_proximity/stripe_0_hist.json",
                 "task_id": "solve_proximity_hist_0"}]
        event = self._make_event({"target": "solve_proximity", "jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 1)
        self.assertEqual(len(body["non_202"]), 1)
        self.assertEqual(body["non_202"][0]["status"], 429)


# ── Test: handler_storage routing ─────────────────────────────────────────


class TestStorageRouting(unittest.TestCase):
    """Verify handler_storage routes /head-keys to the right handler."""

    @patch("handler_storage.handle_head_keys")
    def test_head_keys_route(self, mock_handler):
        from handler_storage import handler
        mock_handler.return_value = {"statusCode": 200, "body": "{}",
                                     "headers": {"Content-Type": "application/json"}}
        event = {"rawPath": "/head-keys", "body": json.dumps({"keys": []})}
        handler(event, None)
        mock_handler.assert_called_once()

    def test_unknown_route_returns_400(self):
        from handler_storage import handler
        event = {"rawPath": "/nonexistent", "body": "{}"}
        result = handler(event, None)
        self.assertEqual(result["statusCode"], 400)


# ── Test: missing-task detection logic ────────────────────────────────────


class TestMissingTaskDetection(unittest.TestCase):
    """
    Test the missing-task detection pattern used by _bilevelDispatchAndPoll.
    This validates the set-diff logic that identifies tasks accepted by dispatch
    but never executed (the 449/500 failure mode).
    """

    def _compute_missing(self, found_ids, task_prefix, expected):
        """Replicate the JS missing-task detection logic in Python."""
        found_set = set(found_ids)
        missing = []
        for i in range(expected):
            if f"{task_prefix}{i}" not in found_set:
                missing.append(i)
        return missing

    def test_no_missing(self):
        found = [f"bilevel_raster_{i}" for i in range(500)]
        missing = self._compute_missing(found, "bilevel_raster_", 500)
        self.assertEqual(missing, [])

    def test_all_missing(self):
        missing = self._compute_missing([], "bilevel_raster_", 5)
        self.assertEqual(missing, [0, 1, 2, 3, 4])

    def test_contiguous_gap(self):
        """51 missing at the end — matches the observed failure."""
        found = [f"bilevel_raster_{i}" for i in range(449)]
        missing = self._compute_missing(found, "bilevel_raster_", 500)
        self.assertEqual(len(missing), 51)
        self.assertEqual(missing[0], 449)
        self.assertEqual(missing[-1], 499)

    def test_scattered_gaps(self):
        """Random missing tasks — validates non-contiguous detection."""
        present = set(range(500)) - {3, 17, 42, 199, 301, 498}
        found = [f"bilevel_raster_{i}" for i in present]
        missing = self._compute_missing(found, "bilevel_raster_", 500)
        self.assertEqual(missing, [3, 17, 42, 199, 301, 498])

    def test_different_prefix(self):
        """Works with merge task prefix too."""
        found = [f"bilevel_merge_{i}" for i in [0, 2, 3]]
        missing = self._compute_missing(found, "bilevel_merge_", 4)
        self.assertEqual(missing, [1])

    def test_coeff_prefix(self):
        """Works with coeff bilevel raster prefix."""
        found = [f"coeff_bilevel_raster_{i}" for i in range(10) if i != 7]
        missing = self._compute_missing(found, "coeff_bilevel_raster_", 10)
        self.assertEqual(missing, [7])


# ── Test: wave dispatch inflight calculation ──────────────────────────────


class TestWaveDispatchLogic(unittest.TestCase):
    """
    Test the wave dispatch calculation used by _bilevelDispatchAndPoll.
    Validates that MAX_INFLIGHT is respected.
    """

    def _simulate_waves(self, total, max_inflight, completions):
        """
        Simulate the wave dispatch loop.
        completions: list of done counts returned by successive polls.
        Returns list of (cursor_before, can_send) for each dispatch step.
        """
        cursor = 0
        last_poll_done = 0
        waves = []
        poll_idx = 0
        max_steps = total + len(completions) + 10  # safety bound
        for _ in range(max_steps):
            if cursor >= total:
                break
            in_flight = cursor - last_poll_done
            can_send = min(max_inflight - in_flight, total - cursor)
            if can_send > 0:
                waves.append((cursor, can_send))
                cursor += can_send
            if cursor < total:
                if poll_idx < len(completions):
                    last_poll_done = completions[poll_idx]
                    poll_idx += 1
                else:
                    last_poll_done = cursor  # everything done
        return waves

    def test_small_batch_single_wave(self):
        """50 tasks with MAX_INFLIGHT=200 → single wave."""
        waves = self._simulate_waves(50, 200, [])
        self.assertEqual(len(waves), 1)
        self.assertEqual(waves[0], (0, 50))

    def test_large_batch_multiple_waves(self):
        """500 tasks with MAX_INFLIGHT=200 → multiple waves."""
        # Poll returns increasing done counts
        waves = self._simulate_waves(500, 200, [120, 250, 380, 450])
        # First wave dispatches 200
        self.assertEqual(waves[0], (0, 200))
        # Total dispatched across all waves = 500
        total_dispatched = sum(w[1] for w in waves)
        self.assertEqual(total_dispatched, 500)
        # No single wave exceeds MAX_INFLIGHT
        for cursor, count in waves:
            self.assertLessEqual(count, 200)

    def test_max_inflight_respected(self):
        """With slow completions, each wave is throttled to respect MAX_INFLIGHT."""
        # 400 tasks, MAX_INFLIGHT=150. Polls return gradual progress.
        waves = self._simulate_waves(400, 150, [50, 100, 150, 200, 250, 300, 350])
        self.assertEqual(waves[0], (0, 150))  # first wave = MAX_INFLIGHT
        total_dispatched = sum(w[1] for w in waves)
        self.assertEqual(total_dispatched, 400)
        # At no point is in-flight > MAX_INFLIGHT:
        # After first wave: cursor=150, done=0 → in_flight=150 ≤ 150. OK.
        # Each subsequent wave only sends min(MAX_INFLIGHT - in_flight, remaining).

    def test_exact_inflight(self):
        """200 tasks with MAX_INFLIGHT=200 → single wave."""
        waves = self._simulate_waves(200, 200, [])
        self.assertEqual(len(waves), 1)
        self.assertEqual(waves[0], (0, 200))

    def test_all_complete_between_waves(self):
        """If everything completes between waves, next wave gets full MAX_INFLIGHT."""
        waves = self._simulate_waves(600, 200, [200, 400])
        self.assertEqual(waves[0], (0, 200))
        self.assertEqual(waves[1], (200, 200))
        self.assertEqual(waves[2], (400, 200))
        self.assertEqual(sum(w[1] for w in waves), 600)


if __name__ == "__main__":
    unittest.main()
