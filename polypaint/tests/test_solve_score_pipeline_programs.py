import os
import sys

import pytest


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def test_blank_source_without_chain_or_metric_fails_closed():
    from solve_score_pipeline_programs import solve_score_program_for_run

    with pytest.raises(RuntimeError, match="source is empty"):
        solve_score_program_for_run({"solve_score_program_source_text": ""})


def test_blank_source_preserves_legacy_metric_fallback():
    from solve_score_pipeline_programs import solve_score_program_for_run

    compiled = solve_score_program_for_run({
        "solve_score_program_source_text": "",
        "metric": "spread",
        "solve_score_quantile": 0.005,
    })

    assert compiled["metric"] == "spread"
    assert "spread" in compiled["source_text"]


def test_non_strict_source_fallback_preserves_diagnostics():
    from solve_score_pipeline_programs import solve_score_program_for_run

    compiled = solve_score_program_for_run(
        {"solve_score_program_source_text": "score = metric(proximity, slv, q=999%)"},
        strict=False,
    )

    assert compiled["degraded"] is True
    assert compiled["diagnostics"]
    assert compiled["diagnostics"][0]["code"] == "bad_quantile"
    assert compiled["program_spec"] == "m0-0"
