import json
import os
import sys


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


SOLVE_SCORE_CORPUS = [
    {
        "name": "legacy_metric_transfer",
        "chain": [["crowding", "1"], ["omega_cosine", "4"]],
    },
    {
        "name": "weighted_sum_transfer",
        "chain": [["spread", "2"], ["shelliness", "3"], ["weighted_sum", "0.7", "0.3"], ["omega_cosine", "5"]],
    },
    {
        "name": "internal_transfer_combine",
        "chain": [["proximity", "1"], ["omega_cosine", "3"], ["clusteriness", "2"], ["avg"]],
    },
    {
        "name": "negative_params",
        "chain": [
            ["proximity", "1"],
            ["sawtooth", "-2"],
            ["spread", "1"],
            ["weighted_sum", "-1", "2"],
            ["pow", "-1"],
        ],
    },
    {
        "name": "explicit_rgb_outputs",
        "chain": [
            ["proximity", "slv", "0.5"],
            ["emit", "norm"],
            ["spread", "slv", "0.5"],
            ["emit", "norm"],
            ["angular_entropy_16", "slv", "0.5"],
            ["emit", "norm"],
        ],
    },
    {
        "name": "emit_none_flush",
        "chain": [["proximity", "0.5"], ["emit", "none"], "flush", ["spread", "0.5"], ["emit", "norm"]],
    },
    {
        "name": "materialized_dup_param_ops",
        "chain": [["proximity", "0.5"], "dup", ["pow", "-1"], ["emit", "norm"], ["sawtooth", "-2"], ["emit", "raw"]],
    },
    {
        "name": "lagged_abs_diff",
        "chain": [["crowding", "slv", "1"], ["crowding", "slv-1", "1"], ["abs_diff"]],
    },
    {
        "name": "mixed_sources",
        "chain": [["spread", "slv", "2"], ["spread", "cf", "2"], ["avg"], ["t1_abs", "pm", "2"], ["avg"]],
    },
    {
        "name": "generic_metric_form",
        "chain": [["metric", "angular_entropy_16", "cf", "0.5"], ["metric", "angular_entropy_16", "cf-1", "0.5"], ["abs_diff"]],
    },
    {
        "name": "stack_unaries",
        "chain": [["proximity", "0.5"], ["const", "0.001"], ["add"], "dup", ["ema", "0.99"], "sin", ["pow", "2"], "clamp"],
    },
]


def _canonical_public_chain(compiled):
    from solve_score_chain import public_solve_score_chain

    return public_solve_score_chain(compiled["chain"])


def test_solve_score_corpus_round_trips_chain_spec_fingerprint_and_outputs():
    from solve_score_chain import (
        compile_solve_score_chain,
        compiled_solve_score_fingerprint,
        serialize_solve_score_chain,
    )
    from solve_score_program_source import (
        compile_solve_score_program_source,
        solve_score_source_text_from_chain,
    )

    for case in SOLVE_SCORE_CORPUS:
        old = compile_solve_score_chain(case["chain"])
        source = solve_score_source_text_from_chain(case["chain"])
        new = compile_solve_score_program_source(source)

        assert new["program_spec"] == old["program_spec"], case["name"]
        assert new["fingerprint"] == compiled_solve_score_fingerprint(old), case["name"]
        assert new["output_channel_count"] == old["output_channel_count"], case["name"]
        assert new["output_channels"] == old["output_channels"], case["name"]
        reparsed = compile_solve_score_chain(json.loads(serialize_solve_score_chain(new["chain"])))
        assert reparsed["program_spec"] == old["program_spec"], case["name"]
        assert compiled_solve_score_fingerprint(reparsed) == compiled_solve_score_fingerprint(old), case["name"]


def test_authored_source_matches_equivalent_chain_program_spec_byte_for_byte():
    from solve_score_chain import compile_solve_score_chain, compiled_solve_score_fingerprint
    from solve_score_program_source import compile_solve_score_program_source

    source = (
        "a = metric(spread, slv, q=2%)\n"
        "b = metric(shelliness, slv, q=3%)\n"
        "score = omega_cosine(weighted_sum(a, b, 0.7, 0.3), 5)"
    )
    chain = [["spread", "2"], ["shelliness", "3"], ["weighted_sum", "0.7", "0.3"], ["omega_cosine", "5"]]
    old = compile_solve_score_chain(chain)
    new = compile_solve_score_program_source(source)

    assert new["program_spec"] == old["program_spec"]
    assert new["fingerprint"] == compiled_solve_score_fingerprint(old)
