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
