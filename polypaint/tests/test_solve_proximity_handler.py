"""
Tests for handler_solve_proximity.py merge phase logic.

The clip and hist phases delegate to the C binary (tested in test_solve_proximity_stats.py).
The merge phase is pure Python — derive 9 equal-density bin cuts from summed histograms.

Run: cd polypaint && uv run python tests/test_solve_proximity_handler.py
"""
import json
import os
import sys
import unittest.mock as mock

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _make_mock_s3(clip_data, hist_responses):
    """Build a mock S3 client for merge tests."""
    mock_s3 = mock.MagicMock()

    def mock_get(**kwargs):
        key = kwargs.get("Key", "")
        if "clip" in key:
            return {"Body": mock.MagicMock(read=lambda: json.dumps(clip_data).encode())}
        if key in hist_responses:
            data = hist_responses[key]
            return {"Body": mock.MagicMock(read=lambda d=data: json.dumps(d).encode())}
        raise mock_s3.exceptions.NoSuchKey({"Error": {"Code": "NoSuchKey"}}, "GetObject")

    mock_s3.get_object = mock_get
    mock_s3.put_object = mock.MagicMock()
    mock_s3.exceptions = type('Exc', (), {'NoSuchKey': type('NoSuchKey', (Exception,), {})})()
    return mock_s3


def _run_merge(n_stripes, clip_data, hist_responses):
    """Run merge phase with mocked S3."""
    import handler_solve_proximity as hsp
    mock_s3 = _make_mock_s3(clip_data, hist_responses)
    orig_s3, orig_report = hsp.s3, hsp.report_status
    hsp.s3 = mock_s3
    hsp.report_status = mock.MagicMock()
    try:
        result = hsp.handle_merge({
            "job_id": "test",
            "task_id": "merge_test",
            "n_stripes": n_stripes,
            "hist_prefix": "renders/test/solve_proximity/",
            "clip_key": "renders/test/solve_proximity_clip.json",
            "out_key": "renders/test/solve_proximity_bins.json",
        })
        return json.loads(result["body"])
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report


def test_merge_uniform_histogram():
    """Uniform histogram -> evenly spaced cuts."""
    clip_data = {"clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": []}
    hist_responses = {}
    for s in range(5):
        hist_responses[f"renders/test/solve_proximity/stripe_{s}_hist.json"] = {
            "hist": [20] * 100, "n_solves": 2000
        }
    body = _run_merge(5, clip_data, hist_responses)
    cuts = body["cuts_norm"]
    assert len(cuts) == 9, f"expected 9 cuts, got {len(cuts)}"
    for i in range(9):
        assert abs(cuts[i] - (i + 1) / 10) < 0.02, f"cut[{i}]={cuts[i]}, expected ~{(i+1)/10}"
    assert all(cuts[i] <= cuts[i + 1] for i in range(8)), f"not monotonic: {cuts}"
    assert body["n_solves_total"] == 10000


def test_merge_skewed_histogram():
    """Skewed histogram -> concentrated cuts."""
    clip_data = {"clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": []}
    hist = [900] * 10 + [11] * 90
    hist_responses = {
        "renders/test/solve_proximity/stripe_0_hist.json": {"hist": hist, "n_solves": 9990}
    }
    body = _run_merge(1, clip_data, hist_responses)
    cuts = body["cuts_norm"]
    assert len(cuts) == 9
    low_cuts = sum(1 for c in cuts if c < 0.15)
    assert low_cuts >= 7, f"expected most cuts < 0.15, got {low_cuts}: {[round(c,3) for c in cuts]}"
    assert all(cuts[i] <= cuts[i + 1] for i in range(8))


def test_merge_single_bin_histogram():
    """All data in one bin -> degenerate but valid cuts."""
    clip_data = {"clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": []}
    hist = [0] * 100
    hist[50] = 10000
    hist_responses = {
        "renders/test/solve_proximity/stripe_0_hist.json": {"hist": hist, "n_solves": 10000}
    }
    body = _run_merge(1, clip_data, hist_responses)
    cuts = body["cuts_norm"]
    assert len(cuts) == 9
    assert all(0 <= c <= 1 for c in cuts), f"cuts out of range: {cuts}"
    assert all(cuts[i] <= cuts[i + 1] for i in range(8))


def test_merge_error_missing_stripe():
    """Missing stripe histogram should raise."""
    clip_data = {"clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": []}
    hist_responses = {
        "renders/test/solve_proximity/stripe_0_hist.json": {"hist": [10] * 100, "n_solves": 1000}
    }
    try:
        _run_merge(2, clip_data, hist_responses)
        assert False, "should have raised"
    except RuntimeError as e:
        assert "Missing histogram" in str(e), f"wrong error: {e}"


if __name__ == "__main__":
    tests = [
        ("uniform histogram", test_merge_uniform_histogram),
        ("skewed histogram", test_merge_skewed_histogram),
        ("single bin histogram", test_merge_single_bin_histogram),
        ("missing stripe error", test_merge_error_missing_stripe),
    ]

    print("solve_proximity handler tests")
    print("=" * 50)

    passed = 0
    failed = 0
    for name, fn in tests:
        try:
            fn()
            passed += 1
            print(f"  PASS: {name}")
        except AssertionError as e:
            failed += 1
            print(f"  FAIL: {name} — {e}")
        except Exception as e:
            failed += 1
            print(f"  ERROR: {name} — {type(e).__name__}: {e}")

    print(f"\n{'=' * 50}")
    print(f"{passed}/{passed + failed} passed")
    if failed:
        sys.exit(1)
    else:
        print("ALL PASS")
