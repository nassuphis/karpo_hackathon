import os
import sys


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def test_parse_keyword_args_respects_top_level_assignment_only():
    from program_source_core import ProgramSourceError, parse_keyword_args

    positional, kwargs = parse_keyword_args(
        ["metric(proximity, slv, q=0.5%)", "q=1%", "lag=1"],
        allowed={"q", "lag"},
        error_cls=ProgramSourceError,
    )

    assert positional == ["metric(proximity, slv, q=0.5%)"]
    assert kwargs == {"q": "1%", "lag": "1"}


def test_percent_and_numeric_literal_formatting():
    from program_source_core import format_numeric_literal, format_percent_literal, is_numeric_literal

    assert is_numeric_literal("-1.5e-3", allow_percent=False)
    assert not is_numeric_literal("1%", allow_percent=False)
    assert is_numeric_literal("1%", allow_percent=True)
    assert format_numeric_literal("2.0") == "2"
    assert format_percent_literal("0.500%") == "0.5"


def test_classify_source_error_code_covers_diagnostic_families():
    from program_source_core import classify_source_error_code

    assert classify_source_error_code("unknown function frobnicate") == "unknown_operator"
    assert classify_source_error_code("quantile q must be in (0,1)") == "bad_quantile"
    assert classify_source_error_code("poke_poly expects 2 arguments") == "bad_arity"
    assert classify_source_error_code("cf is not allowed as a source selector here") == "bad_selector"
    assert classify_source_error_code("stack underflow at emit") == "stack_error"
    assert classify_source_error_code("value must be finite") == "bad_numeric_arg"
    assert classify_source_error_code("something entirely novel") == "source_error"
    assert classify_source_error_code(None) == "source_error"


def test_source_error_subclasses_auto_classify_default_code_only():
    from coeff_program_source import CoeffProgramSourceError
    from solve_score_program_source import SolveScoreProgramSourceError

    # The bare default is upgraded from the message...
    assert CoeffProgramSourceError("unknown function nope").code == "unknown_operator"
    assert SolveScoreProgramSourceError("quantile q must be in (0,1)").code == "bad_quantile"
    # ...but an explicit code always wins, and unmatched messages keep the default.
    assert CoeffProgramSourceError("unknown thing", code="custom").code == "custom"
    assert SolveScoreProgramSourceError("mystery").code == "source_error"
