"""
Tests for handler_solve_proximity.py merge phase logic.

The clip and hist phases delegate to the C binary (tested in test_solve_proximity_stats.py).
The merge phase is pure Python — derive 9 equal-density bin cuts from summed histograms.

Run: cd polypaint && uv run python tests/test_solve_proximity_handler.py
"""
import json
import os
import sys

# Add lambda dir to path for imports
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

passed = 0
failed = 0


def check(name, condition, detail=""):
    global passed, failed
    if condition:
        passed += 1
        print(f"  PASS: {name}")
    else:
        failed += 1
        print(f"  FAIL: {name} — {detail}")


def test_merge_uniform_histogram():
    """Uniform histogram → evenly spaced cuts."""
    from handler_solve_proximity import handle_merge

    # Mock S3 and report_status
    import unittest.mock as mock

    hist = [100] * 100  # 10000 total, uniform
    clip_data = {"clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": []}

    mock_s3 = mock.MagicMock()
    # clip_key returns clip data
    mock_s3.get_object.return_value = {"Body": mock.MagicMock(read=lambda: json.dumps(clip_data).encode())}

    hist_responses = {}
    for s in range(5):
        key = f"renders/test/solve_proximity/stripe_{s}_hist.json"
        hist_responses[key] = {"hist": [20] * 100, "n_solves": 2000}  # 5 stripes × 20 = 100 per bin

    orig_get = mock_s3.get_object

    def mock_get(**kwargs):
        key = kwargs.get("Key", "")
        if "clip" in key:
            return {"Body": mock.MagicMock(read=lambda: json.dumps(clip_data).encode())}
        if key in hist_responses:
            data = hist_responses[key]
            return {"Body": mock.MagicMock(read=lambda d=data: json.dumps(d).encode())}
        raise Exception(f"NoSuchKey: {key}")

    mock_s3.get_object = mock_get
    mock_s3.put_object = mock.MagicMock()

    import handler_solve_proximity as hsp
    orig_s3 = hsp.s3
    orig_report = hsp.report_status
    hsp.s3 = mock_s3
    hsp.report_status = mock.MagicMock()

    try:
        result = hsp.handle_merge({
            "job_id": "test",
            "task_id": "merge_test",
            "n_stripes": 5,
            "hist_prefix": "renders/test/solve_proximity/",
            "clip_key": "renders/test/solve_proximity_clip.json",
            "out_key": "renders/test/solve_proximity_bins.json",
        })
        body = json.loads(result["body"])
        cuts = body["cuts_norm"]
        check("uniform: 9 cuts", len(cuts) == 9, f"got {len(cuts)}")
        check("uniform: cuts roughly evenly spaced",
              all(abs(cuts[i] - (i + 1) / 10) < 0.02 for i in range(9)),
              f"cuts={[round(c, 3) for c in cuts]}")
        check("uniform: monotonic", all(cuts[i] <= cuts[i + 1] for i in range(8)))
        check("uniform: n_solves_total=10000", body["n_solves_total"] == 10000)
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report


def test_merge_skewed_histogram():
    """Skewed histogram → non-uniform cuts."""
    import unittest.mock as mock
    import handler_solve_proximity as hsp

    clip_data = {"clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": []}

    # 90% of data in first 10 bins, 10% in rest
    hist = [900] * 10 + [11] * 90  # total = 9000 + 990 = 9990
    hist_responses = {}
    for s in range(1):
        key = f"renders/test/solve_proximity/stripe_{s}_hist.json"
        hist_responses[key] = {"hist": hist, "n_solves": 9990}

    mock_s3 = mock.MagicMock()
    def mock_get(**kwargs):
        key = kwargs.get("Key", "")
        if "clip" in key:
            return {"Body": mock.MagicMock(read=lambda: json.dumps(clip_data).encode())}
        if key in hist_responses:
            data = hist_responses[key]
            return {"Body": mock.MagicMock(read=lambda d=data: json.dumps(d).encode())}
        raise Exception(f"NoSuchKey: {key}")
    mock_s3.get_object = mock_get
    mock_s3.put_object = mock.MagicMock()

    orig_s3 = hsp.s3
    orig_report = hsp.report_status
    hsp.s3 = mock_s3
    hsp.report_status = mock.MagicMock()

    try:
        result = hsp.handle_merge({
            "job_id": "test",
            "task_id": "merge_test",
            "n_stripes": 1,
            "hist_prefix": "renders/test/solve_proximity/",
            "clip_key": "renders/test/solve_proximity_clip.json",
            "out_key": "renders/test/solve_proximity_bins.json",
        })
        body = json.loads(result["body"])
        cuts = body["cuts_norm"]
        check("skewed: 9 cuts", len(cuts) == 9)
        # Most cuts should be in [0, 0.1] range since 90% of data is there
        low_cuts = sum(1 for c in cuts if c < 0.15)
        check("skewed: most cuts < 0.15 (data concentrated in low bins)",
              low_cuts >= 7, f"got {low_cuts} cuts < 0.15, cuts={[round(c, 3) for c in cuts]}")
        check("skewed: monotonic", all(cuts[i] <= cuts[i + 1] for i in range(8)))
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report


def test_merge_single_bin_histogram():
    """All data in one bin → degenerate but valid cuts."""
    import unittest.mock as mock
    import handler_solve_proximity as hsp

    clip_data = {"clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": []}
    hist = [0] * 100
    hist[50] = 10000  # all data in bin 50

    mock_s3 = mock.MagicMock()
    def mock_get(**kwargs):
        key = kwargs.get("Key", "")
        if "clip" in key:
            return {"Body": mock.MagicMock(read=lambda: json.dumps(clip_data).encode())}
        return {"Body": mock.MagicMock(read=lambda: json.dumps({"hist": hist, "n_solves": 10000}).encode())}
    mock_s3.get_object = mock_get
    mock_s3.put_object = mock.MagicMock()

    orig_s3 = hsp.s3
    orig_report = hsp.report_status
    hsp.s3 = mock_s3
    hsp.report_status = mock.MagicMock()

    try:
        result = hsp.handle_merge({
            "job_id": "test",
            "task_id": "merge_test",
            "n_stripes": 1,
            "hist_prefix": "renders/test/solve_proximity/",
            "clip_key": "renders/test/solve_proximity_clip.json",
            "out_key": "renders/test/solve_proximity_bins.json",
        })
        body = json.loads(result["body"])
        cuts = body["cuts_norm"]
        check("single bin: 9 cuts", len(cuts) == 9)
        check("single bin: all cuts in [0,1]", all(0 <= c <= 1 for c in cuts))
        check("single bin: monotonic", all(cuts[i] <= cuts[i + 1] for i in range(8)))
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report


def test_merge_error_missing_stripe():
    """Missing stripe histogram should raise."""
    import unittest.mock as mock
    import handler_solve_proximity as hsp

    clip_data = {"clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": []}

    mock_s3 = mock.MagicMock()
    call_count = [0]
    def mock_get(**kwargs):
        key = kwargs.get("Key", "")
        if "clip" in key:
            return {"Body": mock.MagicMock(read=lambda: json.dumps(clip_data).encode())}
        # First stripe exists, second doesn't
        call_count[0] += 1
        if "stripe_0" in key:
            return {"Body": mock.MagicMock(read=lambda: json.dumps({"hist": [10]*100, "n_solves": 1000}).encode())}
        raise hsp.s3.exceptions.NoSuchKey({"Error": {"Code": "NoSuchKey"}}, "GetObject")
    mock_s3.get_object = mock_get
    mock_s3.put_object = mock.MagicMock()
    mock_s3.exceptions = type('Exc', (), {'NoSuchKey': type('NoSuchKey', (Exception,), {})})()

    orig_s3 = hsp.s3
    orig_report = hsp.report_status
    hsp.s3 = mock_s3
    hsp.report_status = mock.MagicMock()

    try:
        raised = False
        try:
            hsp.handle_merge({
                "job_id": "test",
                "task_id": "merge_test",
                "n_stripes": 2,
                "hist_prefix": "renders/test/solve_proximity/",
                "clip_key": "renders/test/solve_proximity_clip.json",
                "out_key": "renders/test/solve_proximity_bins.json",
            })
        except RuntimeError as e:
            raised = True
            check("missing stripe: raises RuntimeError", "Missing histogram" in str(e), str(e))
        if not raised:
            check("missing stripe: raises RuntimeError", False, "no exception raised")
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report


if __name__ == "__main__":
    print("solve_proximity handler tests")
    print("=" * 50)

    print("\n--- merge phase ---")
    test_merge_uniform_histogram()
    test_merge_skewed_histogram()
    test_merge_single_bin_histogram()
    test_merge_error_missing_stripe()

    print("\n" + "=" * 50)
    total = passed + failed
    print(f"{passed}/{total} passed")
    if failed:
        print("FAIL")
        sys.exit(1)
    else:
        print("ALL PASS")
