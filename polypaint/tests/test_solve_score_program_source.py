import json
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def test_solve_score_source_compiles_scalar_expression():
    from solve_score_program_source import compile_solve_score_program_source

    compiled = compile_solve_score_program_source(
        "score = omega_cosine(metric(crowding, slv, q=1%), 4)"
    )

    assert compiled["chain"] == [["crowding", "1"], ["omega_cosine", "4"]]
    assert compiled["program_spec"] == "m0-0;omega_cosine:4"
    assert compiled["output_channel_count"] == 1
    assert compiled["source_text"].startswith("score = ")


def test_solve_score_source_compiles_explicit_outputs_and_sources():
    from solve_score_program_source import compile_solve_score_program_source

    compiled = compile_solve_score_program_source(
        "emit_norm(metric(proximity, slv, q=0.5%))\n"
        "emit(metric(spread, cf, q=0.5%))"
    )

    assert compiled["chain"] == [
        ["proximity", "0.5"],
        ["emit", "norm"],
        ["spread", "cf", "0.5"],
        ["emit", "raw"],
    ]
    assert compiled["program_spec"] == "m0-0;emit_norm;m1-0;emit"
    assert compiled["output_channel_count"] == 2


def test_solve_score_source_rejects_infix_and_use_before_definition():
    from solve_score_program_source import parse_solve_score_program_source

    parsed = parse_solve_score_program_source(
        "score = metric(proximity, slv, q=0.5%) + const(1)\n"
        "emit_norm(missing)",
        strict=False,
    )

    messages = " ".join(item["message"] for item in parsed["diagnostics"])
    assert "infix arithmetic" in messages
    assert "unknown solve-score expression" in messages


def test_solve_score_source_accepts_negative_numeric_params_inside_calls():
    from solve_score_program_source import compile_solve_score_program_source

    cases = [
        ("score = sawtooth(metric(proximity, slv, q=0.5%), -2)", "sawtooth:-2"),
        ("score = pow(metric(proximity, slv, q=0.5%), -1)", "pow:-1"),
        ("score = omega_cosine(metric(proximity, slv, q=0.5%), 4, -0.25)", "omega_cosine:4:-0.25"),
        (
            "score = weighted_sum(metric(proximity, slv, q=0.5%), metric(spread, slv, q=0.5%), -1, 2)",
            "weighted_sum:-1:2",
        ),
    ]

    for source, spec_part in cases:
        compiled = compile_solve_score_program_source(source)
        assert spec_part in compiled["program_spec"]


def test_solve_score_source_text_from_chain_round_trips_program_spec_and_fingerprint():
    from solve_score_chain import compile_solve_score_chain, compiled_solve_score_fingerprint
    from solve_score_program_source import (
        compile_solve_score_program_source,
        solve_score_source_text_from_chain,
    )

    chain = [
        ["metric", "angular_entropy_16", "cf", "0.5"],
        ["metric", "angular_entropy_16", "cf-1", "0.5"],
        ["abs_diff"],
    ]
    source = solve_score_source_text_from_chain(chain)
    old = compile_solve_score_chain(chain)
    new = compile_solve_score_program_source(source)

    assert "metric(angular_entropy_16, cf, q=0.5%)" in source
    assert "lag=1" in source
    assert json.dumps(new["chain"], separators=(",", ":")) == json.dumps(
        json.loads(json.dumps(new["chain"])), separators=(",", ":")
    )
    assert new["program_spec"] == old["program_spec"]
    assert new["fingerprint"] == compiled_solve_score_fingerprint(old)


def test_solve_score_source_text_from_chain_preserves_emit_none_flush():
    from solve_score_program_source import (
        compile_solve_score_program_source,
        solve_score_source_text_from_chain,
    )

    chain = [
        ["proximity", "slv", "0.5"],
        ["emit", "none"],
        "flush",
        ["spread", "slv", "0.5"],
        ["emit", "norm"],
    ]
    source = solve_score_source_text_from_chain(chain)
    compiled = compile_solve_score_program_source(source)

    assert "emit_none(" in source
    assert "flush()" in source
    assert compiled["chain"] == [["proximity", "0.5"], ["emit", "none"], "flush", ["spread", "0.5"], ["emit", "norm"]]


def test_solve_score_source_text_from_chain_preserves_dup_and_flush_stack_history():
    from solve_score_chain import compile_solve_score_chain
    from solve_score_program_source import (
        compile_solve_score_program_source,
        solve_score_source_text_from_chain,
    )

    chain = [
        ["spread", "0.5"],
        ["crowding", "0.5"],
        "dup",
        "flush",
        ["proximity", "0.5"],
        ["emit", "norm"],
    ]
    source = solve_score_source_text_from_chain(chain)
    compiled = compile_solve_score_program_source(source)
    old = compile_solve_score_chain(chain)

    assert "dup()" in source
    assert "flush()" in source
    assert compiled["program_spec"] == old["program_spec"]


def test_solve_score_source_text_from_chain_preserves_materialized_param_ops():
    from solve_score_chain import compile_solve_score_chain
    from solve_score_program_source import (
        compile_solve_score_program_source,
        solve_score_source_text_from_chain,
    )

    chain = [
        ["proximity", "0.5"],
        "dup",
        ["pow", "-1"],
        ["emit", "norm"],
        ["sawtooth", "-2"],
        ["emit", "raw"],
    ]
    source = solve_score_source_text_from_chain(chain)
    compiled = compile_solve_score_program_source(source)
    old = compile_solve_score_chain(chain)

    assert "pow(-1)" in source
    assert "sawtooth(-2)" in source
    assert compiled["program_spec"] == old["program_spec"]
