import json
import os
import sys
import pathlib

import pytest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))
ROOT = pathlib.Path(__file__).resolve().parent.parent


def _load_solve_score_vocab():
    raw = (ROOT / "solve_score_vocab_js.js").read_text()
    return json.loads(raw[raw.index("{"):raw.rindex("}") + 1])


def test_solve_score_source_compiles_scalar_expression():
    from solve_score_program_source import compile_solve_score_program_source

    compiled = compile_solve_score_program_source(
        "score = omega_cosine(metric(crowding, slv, q=1%), 4)"
    )

    assert compiled["chain"] == [["crowding", "1"], ["omega_cosine", "4"]]
    assert compiled["program_spec"] == "m0-0;omega_cosine:4"
    assert compiled["output_channel_count"] == 1
    assert compiled["source_text"].startswith("score = ")


def test_generated_solve_score_vocab_snippets_compile():
    from solve_score_program_source import compile_solve_score_program_source

    vocab = _load_solve_score_vocab()
    snippets = []
    snippets.extend((f"starter:{item['label']}", item["snippet"]) for item in vocab["starterSnippets"])
    snippets.extend((f"metric:{name}", snippet) for name, snippet in vocab["metricSnippets"].items())
    for group in ("stackSpecs", "unarySpecs", "combineSpecs", "outputSpecs"):
        snippets.extend(
            (f"{group}:{name}", spec["snippet"])
            for name, spec in vocab[group].items()
        )

    assert snippets
    for label, source in snippets:
        compiled = compile_solve_score_program_source(source)
        assert not compiled.get("diagnostics"), label
        assert compiled["program_spec"], label


def test_generated_solve_score_vocab_exposes_quantile_range_and_source_order():
    vocab = _load_solve_score_vocab()

    assert vocab["quantilePercentRange"] == [0.1, 5.0]
    assert vocab["sourceNames"][:3] == ["slv", "cf", "pm"]
    for metric, sources in vocab["allowedSourcesByMetric"].items():
        if "slv" in sources:
            assert sources[0] == "slv", metric


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
    assert source.splitlines() == [
        "push(metric(angular_entropy_16, cf, q=0.5%))",
        "push(metric(angular_entropy_16, cf, q=0.5%, lag=1))",
        "abs_diff()",
        "score = pop()",
    ]
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


def test_solve_score_source_text_from_chain_preserves_deferred_multi_emit_order():
    from solve_score_chain import compile_solve_score_chain
    from solve_score_program_source import (
        compile_solve_score_program_source,
        solve_score_source_text_from_chain,
    )

    chain = [
        ["metric", "angular_order_2", "slv", "0.1"],
        ["metric", "angular_order_3", "slv", "0.1"],
        ["metric", "angular_order_4", "slv", "0.1"],
        ["emit", "norm"],
        ["emit", "norm"],
        ["emit", "norm"],
    ]
    source = solve_score_source_text_from_chain(chain)
    compiled = compile_solve_score_program_source(source)
    old = compile_solve_score_chain(chain)

    assert "push(metric(angular_order_2" in source
    assert source.endswith("emit_norm()\nemit_norm()\nemit_norm()")
    assert compiled["program_spec"] == old["program_spec"]


def test_solve_score_source_text_from_chain_uses_stack_form_for_explicit_combine_outputs():
    from solve_score_chain import compile_solve_score_chain
    from solve_score_program_source import (
        compile_solve_score_program_source,
        solve_score_source_text_from_chain,
    )

    chain = [
        ["metric", "diagonal_proximity", "slv", "0.1"],
        ["metric", "diagonal_proximity", "cf", "0.1"],
        "abs_diff",
        ["emit", "norm"],
    ]
    source = solve_score_source_text_from_chain(chain)
    compiled = compile_solve_score_program_source(source)
    old = compile_solve_score_chain(chain)

    assert source.splitlines() == [
        "push(metric(diagonal_proximity, slv, q=0.1%))",
        "push(metric(diagonal_proximity, cf, q=0.1%))",
        "abs_diff()",
        "emit_norm()",
    ]
    assert compiled["program_spec"] == old["program_spec"]


def test_solve_score_source_text_from_chain_reports_roundtrip_failures(monkeypatch):
    import solve_score_program_source as mod

    original_compile = mod.compile_solve_score_program_source

    def compile_with_bad_fingerprint(*args, **kwargs):
        compiled = original_compile(*args, **kwargs)
        compiled["fingerprint"] = "sha256:forced-mismatch"
        return compiled

    monkeypatch.setattr(mod, "compile_solve_score_program_source", compile_with_bad_fingerprint)

    with pytest.raises(mod.SolveScoreProgramSourceError) as exc:
        mod.solve_score_source_text_from_chain([["proximity", "slv", "0.5"]])

    assert exc.value.code == "source_roundtrip_failed"


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
