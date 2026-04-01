"""
Tests for handler_solve_proximity.py merge phase logic.

The clip and hist phases delegate to the C binary (tested in test_solve_proximity_stats.py).
The merge phase is pure Python — derive 9 equal-density bin cuts from summed histograms.

Tests metric awareness: preserves requested metric, rejects mismatched artifacts,
output artifact has correct family and cuts_norm length.

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


def _run_merge(n_stripes, clip_data, hist_responses, metric="proximity", solve_score_quantile=0.001, solve_score_omega=1.0):
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
            "metric": metric,
            "solve_score_quantile": solve_score_quantile,
            "solve_score_omega": solve_score_omega,
            "n_stripes": n_stripes,
            "hist_prefix": "renders/test/solve_scores/",
            "clip_key": "renders/test/solve_scores/clip.json",
            "out_key": "renders/test/solve_scores/bins.json",
        })
        # Capture what was written to S3
        put_calls = mock_s3.put_object.call_args_list
        written_artifact = None
        if put_calls:
            body_bytes = put_calls[0][1].get("Body", b"{}")
            written_artifact = json.loads(body_bytes)
        return json.loads(result["body"]), written_artifact
    finally:
        hsp.s3 = orig_s3
        hsp.report_status = orig_report


def _uniform_hist_data(prefix, n_stripes, metric="proximity", clip_quantile=0.001):
    """Generate uniform histogram responses for n_stripes."""
    clip_data = {
        "family": "solve_score", "metric": metric,
        "clip_quantile": clip_quantile,
        "omega": 1.0,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist_responses = {}
    for s in range(n_stripes):
        hist_responses[f"{prefix}stripe_{s}_hist.json"] = {
            "family": "solve_score", "metric": metric,
            "clip_quantile": clip_quantile,
            "omega": 1.0,
            "hist": [20] * 100, "n_solves": 2000,
        }
    return clip_data, hist_responses


def test_merge_uniform_histogram():
    """Uniform histogram -> evenly spaced cuts."""
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 5, metric="proximity")
    body, artifact = _run_merge(5, clip_data, hist_responses, metric="proximity")
    cuts = body["cuts_norm"]
    assert len(cuts) == 9, f"expected 9 cuts, got {len(cuts)}"
    for i in range(9):
        assert abs(cuts[i] - (i + 1) / 10) < 0.02, f"cut[{i}]={cuts[i]}, expected ~{(i+1)/10}"
    assert all(cuts[i] <= cuts[i + 1] for i in range(8)), f"not monotonic: {cuts}"
    assert body["n_solves_total"] == 10000


def test_merge_skewed_histogram():
    """Skewed histogram -> concentrated cuts."""
    clip_data = {
        "family": "solve_score", "metric": "proximity",
        "clip_quantile": 0.001,
        "omega": 1.0,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist = [900] * 10 + [11] * 90
    hist_responses = {
        "renders/test/solve_scores/stripe_0_hist.json": {
            "family": "solve_score", "metric": "proximity",
            "clip_quantile": 0.001,
            "omega": 1.0,
            "hist": hist, "n_solves": 9990,
        }
    }
    body, _ = _run_merge(1, clip_data, hist_responses, metric="proximity")
    cuts = body["cuts_norm"]
    assert len(cuts) == 9
    low_cuts = sum(1 for c in cuts if c < 0.15)
    assert low_cuts >= 7, f"expected most cuts < 0.15, got {low_cuts}: {[round(c,3) for c in cuts]}"
    assert all(cuts[i] <= cuts[i + 1] for i in range(8))


def test_merge_single_bin_histogram():
    """All data in one bin -> degenerate but valid cuts."""
    clip_data = {
        "family": "solve_score", "metric": "proximity",
        "clip_quantile": 0.001,
        "omega": 1.0,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist = [0] * 100
    hist[50] = 10000
    hist_responses = {
        "renders/test/solve_scores/stripe_0_hist.json": {
            "family": "solve_score", "metric": "proximity",
            "clip_quantile": 0.001,
            "omega": 1.0,
            "hist": hist, "n_solves": 10000,
        }
    }
    body, _ = _run_merge(1, clip_data, hist_responses, metric="proximity")
    cuts = body["cuts_norm"]
    assert len(cuts) == 9
    assert all(0 <= c <= 1 for c in cuts), f"cuts out of range: {cuts}"
    assert all(cuts[i] <= cuts[i + 1] for i in range(8))


def test_merge_error_missing_stripe():
    """Missing stripe histogram should raise."""
    clip_data = {
        "family": "solve_score", "metric": "proximity",
        "clip_quantile": 0.001,
        "omega": 1.0,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist_responses = {
        "renders/test/solve_scores/stripe_0_hist.json": {
            "family": "solve_score", "metric": "proximity",
            "clip_quantile": 0.001,
            "omega": 1.0,
            "hist": [10] * 100, "n_solves": 1000,
        }
    }
    try:
        _run_merge(2, clip_data, hist_responses, metric="proximity")
        assert False, "should have raised"
    except RuntimeError as e:
        assert "Missing histogram" in str(e), f"wrong error: {e}"


# ================================================================
# Metric-aware merge tests (spec 9.2)
# ================================================================

def test_merge_preserves_requested_metric():
    """Merge output contains the requested metric name."""
    for metric in ["proximity", "crowding", "spread", "anisotropy", "area"]:
        clip_data, hist_responses = _uniform_hist_data(
            "renders/test/solve_scores/", 1, metric=metric)
        body, artifact = _run_merge(1, clip_data, hist_responses, metric=metric)
        assert artifact["metric"] == metric, \
            f"expected metric={metric}, got {artifact.get('metric')}"


def test_merge_rejects_clip_wrong_metric():
    """Merge with clip artifact having wrong metric raises."""
    clip_data = {
        "family": "solve_score", "metric": "crowding",
        "clip_quantile": 0.001,
        "omega": 1.0,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist_responses = {
        "renders/test/solve_scores/stripe_0_hist.json": {
            "family": "solve_score", "metric": "proximity",
            "clip_quantile": 0.001,
            "omega": 1.0,
            "hist": [10] * 100, "n_solves": 1000,
        }
    }
    try:
        _run_merge(1, clip_data, hist_responses, metric="proximity")
        assert False, "should have raised on clip metric mismatch"
    except RuntimeError as e:
        assert "mismatch" in str(e).lower(), f"wrong error: {e}"


def test_merge_rejects_stripe_wrong_metric():
    """Merge with stripe histogram having wrong metric raises."""
    clip_data = {
        "family": "solve_score", "metric": "spread",
        "clip_quantile": 0.001,
        "omega": 1.0,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist_responses = {
        "renders/test/solve_scores/stripe_0_hist.json": {
            "family": "solve_score", "metric": "crowding",
            "clip_quantile": 0.001,
            "omega": 1.0,
            "hist": [10] * 100, "n_solves": 1000,
        }
    }
    try:
        _run_merge(1, clip_data, hist_responses, metric="spread")
        assert False, "should have raised on stripe metric mismatch"
    except RuntimeError as e:
        assert "mismatch" in str(e).lower(), f"wrong error: {e}"


def test_merge_preserves_requested_omega():
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 1, metric="anisotropy")
    clip_data["omega"] = 6.0
    hist_responses["renders/test/solve_scores/stripe_0_hist.json"]["omega"] = 6.0
    _, artifact = _run_merge(1, clip_data, hist_responses, metric="anisotropy", solve_score_omega=6.0)
    assert artifact["omega"] == 6.0


def test_merge_rejects_hist_wrong_omega():
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 1, metric="proximity")
    clip_data["omega"] = 4.0
    hist_responses["renders/test/solve_scores/stripe_0_hist.json"]["omega"] = 5.0
    try:
        _run_merge(1, clip_data, hist_responses, metric="proximity", solve_score_omega=4.0)
        assert False, "should have raised on omega mismatch"
    except RuntimeError as e:
        assert "omega mismatch" in str(e).lower(), f"wrong error: {e}"


def test_merge_artifact_has_solve_score_family():
    """Merge output artifact has family == 'solve_score'."""
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 2, metric="area")
    _, artifact = _run_merge(2, clip_data, hist_responses, metric="area")
    assert artifact["family"] == "solve_score", f"family={artifact.get('family')}"


def test_merge_artifact_cuts_norm_length_9():
    """Merge output artifact has exactly 9 cut values (10 bins → 9 cuts)."""
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 3, metric="anisotropy")
    _, artifact = _run_merge(3, clip_data, hist_responses, metric="anisotropy")
    assert len(artifact["cuts_norm"]) == 9, \
        f"expected 9 cuts, got {len(artifact['cuts_norm'])}"


def test_merge_new_metric_clusteriness():
    """Merge works for a v2 metric (clusteriness)."""
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 2, metric="clusteriness")
    body, artifact = _run_merge(2, clip_data, hist_responses, metric="clusteriness")
    assert artifact["metric"] == "clusteriness"
    assert artifact["family"] == "solve_score"
    assert len(artifact["cuts_norm"]) == 9


def test_merge_rejects_clip_mismatch_new_metric():
    """Merge rejects clip artifact with wrong metric (new metric name)."""
    clip_data = {
        "family": "solve_score", "metric": "shelliness",
        "clip_quantile": 0.001,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist_responses = {
        "renders/test/solve_scores/stripe_0_hist.json": {
            "family": "solve_score", "metric": "real_axis_proximity",
            "clip_quantile": 0.001,
            "hist": [10] * 100, "n_solves": 1000,
        }
    }
    try:
        _run_merge(1, clip_data, hist_responses, metric="real_axis_proximity")
        assert False, "should have raised on clip metric mismatch"
    except RuntimeError as e:
        assert "mismatch" in str(e).lower(), f"wrong error: {e}"


def test_merge_new_metric_artifact_family():
    """Merge output for nn_variation still has family == 'solve_score'."""
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 1, metric="nn_variation")
    _, artifact = _run_merge(1, clip_data, hist_responses, metric="nn_variation")
    assert artifact["family"] == "solve_score"
    assert artifact["metric"] == "nn_variation"


# ================================================================
# Quantile tests (spec 7.3)
# ================================================================

def test_merge_artifact_stores_clip_quantile():
    """Merge output stores clip_quantile from request."""
    clip_data, hist_responses = _uniform_hist_data(
        "renders/test/solve_scores/", 1, metric="proximity", clip_quantile=0.01)
    _, artifact = _run_merge(1, clip_data, hist_responses, metric="proximity",
                             solve_score_quantile=0.01)
    assert artifact["clip_quantile"] == 0.01, f"clip_quantile={artifact.get('clip_quantile')}"


def test_merge_rejects_clip_quantile_mismatch():
    """Merge rejects clip artifact with wrong clip_quantile."""
    clip_data = {
        "family": "solve_score", "metric": "proximity",
        "clip_quantile": 0.05,  # clip says 5%
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist_responses = {
        "renders/test/solve_scores/stripe_0_hist.json": {
            "family": "solve_score", "metric": "proximity",
            "clip_quantile": 0.01,  # hist says 1%
            "hist": [10] * 100, "n_solves": 1000,
        }
    }
    # merge with q=0.01 — clip artifact has 0.05 → mismatch
    try:
        _run_merge(1, clip_data, hist_responses, metric="proximity")
        assert False, "should have raised on quantile mismatch"
    except RuntimeError as e:
        assert "quantile" in str(e).lower(), f"wrong error: {e}"


def test_merge_rejects_stripe_quantile_mismatch():
    """Merge rejects stripe artifact with wrong clip_quantile."""
    clip_data = {
        "family": "solve_score", "metric": "proximity",
        "clip_quantile": 0.001,
        "clip_lo": 0.0, "clip_hi": 10.0, "root_transforms": [],
    }
    hist_responses = {
        "renders/test/solve_scores/stripe_0_hist.json": {
            "family": "solve_score", "metric": "proximity",
            "clip_quantile": 0.05,  # different from merge request
            "hist": [10] * 100, "n_solves": 1000,
        }
    }
    try:
        _run_merge(1, clip_data, hist_responses, metric="proximity")
        assert False, "should have raised on stripe quantile mismatch"
    except RuntimeError as e:
        assert "quantile" in str(e).lower(), f"wrong error: {e}"


# ================================================================
# Summary phase tests
# ================================================================

def test_summary_validates_metric():
    """Summary rejects invalid metric."""
    import handler_solve_proximity as hsp
    try:
        hsp.handle_summary({"degree": 5, "metric": "bogus",
                            "lores_bin_key": "x", "solve_score_quantile": 0.001})
        assert False, "should have raised"
    except RuntimeError as e:
        assert "Invalid metric" in str(e)


def test_summary_validates_quantile():
    """Summary rejects out-of-range quantile."""
    import handler_solve_proximity as hsp
    try:
        hsp.handle_summary({"degree": 5, "metric": "proximity",
                            "lores_bin_key": "x", "solve_score_quantile": 0.5})
        assert False, "should have raised"
    except RuntimeError as e:
        assert "0.001" in str(e) or "0.05" in str(e)


def test_summary_does_not_report_status():
    """Summary must not call report_status."""
    import handler_solve_proximity as hsp
    source = open(hsp.__file__).read()
    # Find handle_summary function body
    idx = source.index("def handle_summary")
    body = source[idx:]
    # It should not contain report_status
    assert "report_status(" not in body.split("def ")[0] if "def " in body[20:] else body, \
        "handle_summary must not call report_status"


if __name__ == "__main__":
    tests = [
        ("uniform histogram", test_merge_uniform_histogram),
        ("skewed histogram", test_merge_skewed_histogram),
        ("single bin histogram", test_merge_single_bin_histogram),
        ("missing stripe error", test_merge_error_missing_stripe),
        ("preserves requested metric", test_merge_preserves_requested_metric),
        ("rejects clip wrong metric", test_merge_rejects_clip_wrong_metric),
        ("rejects stripe wrong metric", test_merge_rejects_stripe_wrong_metric),
        ("artifact has solve_score family", test_merge_artifact_has_solve_score_family),
        ("artifact cuts_norm length 9", test_merge_artifact_cuts_norm_length_9),
        ("new metric clusteriness", test_merge_new_metric_clusteriness),
        ("rejects clip mismatch new metric", test_merge_rejects_clip_mismatch_new_metric),
        ("new metric artifact family", test_merge_new_metric_artifact_family),
        ("artifact stores clip_quantile", test_merge_artifact_stores_clip_quantile),
        ("rejects clip quantile mismatch", test_merge_rejects_clip_quantile_mismatch),
        ("rejects stripe quantile mismatch", test_merge_rejects_stripe_quantile_mismatch),
        ("summary validates metric", test_summary_validates_metric),
        ("summary validates quantile", test_summary_validates_quantile),
        ("summary does not report_status", test_summary_does_not_report_status),
    ]

    print("solve_proximity handler tests (metric-aware)")
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
