"""Solve-score program resolution without importing the full pipeline helper."""


class SolveScoreSourceCompileError(ValueError):
    """Solve-score source/chain failed to compile; carries diagnostics."""

    def __init__(self, message, diagnostics):
        super().__init__(message)
        self.diagnostics = list(diagnostics)


def solve_score_source_text_for_run(params, *, scope="solve"):
    """Request-path Solve-Score source-vs-chain precedence."""
    source_key = "solve_score_program_source_text"
    program_key = "solve_score_program"
    if scope != "solve":
        source_key = f"{scope}_score_program_source_text"
        program_key = f"{scope}_score_program"
    if source_key in params:
        raw = str(params.get(source_key) or "")
        if raw.strip():
            return raw
    program = params.get(program_key)
    if isinstance(program, dict):
        raw = str(program.get("source_text") or "")
        if raw.strip():
            return raw
    return None


def _raise_solve_score_if_diagnostics(compiled, *, label):
    diagnostics = list((compiled or {}).get("diagnostics") or [])
    if diagnostics:
        first = diagnostics[0]
        summary = (
            f"{label} line {first.get('line', 0)},"
            f" column {first.get('column', 0)}: {first.get('message', 'compile error')}"
        )
        if len(diagnostics) > 1:
            summary += f" (+{len(diagnostics) - 1} more)"
        raise SolveScoreSourceCompileError(summary, diagnostics)


def solve_score_program_for_run(params, *, scope="solve", strict=True):
    """Resolve Solve-Score source/program/chain/scalars to one compiled object."""
    from solve_score_chain import compile_solve_score_chain_or_legacy, public_solve_score_chain
    from solve_score_program_source import (
        compile_solve_score_program_source,
        solve_score_source_text_from_chain,
    )

    source_key = "solve_score_program_source_text" if scope == "solve" else f"{scope}_score_program_source_text"
    source_key_present = source_key in params
    source_text = solve_score_source_text_for_run(params, scope=scope)
    if source_text is not None:
        source_compiled = compile_solve_score_program_source(source_text, strict=False)
        if strict:
            _raise_solve_score_if_diagnostics(source_compiled, label="solve-score program source")
        compiled = compile_solve_score_chain_or_legacy(
            source_compiled.get("chain") or [],
            "",
            default_metric="proximity",
        )
        return {
            **compiled,
            "chain_public": public_solve_score_chain(compiled["chain"]),
            "source_text": source_text,
            "source_display": source_text.strip(),
        }

    program_key = "solve_score_program" if scope == "solve" else f"{scope}_score_program"
    program = params.get(program_key)
    if isinstance(program, dict) and program.get("chain") not in ("", None, []):
        compiled = compile_solve_score_chain_or_legacy(
            program.get("chain") or [],
            "",
            default_metric="proximity",
        )
        return {
            **compiled,
            "chain_public": public_solve_score_chain(compiled["chain"]),
            "source_text": str(program.get("source_text") or "")
            or solve_score_source_text_from_chain(public_solve_score_chain(compiled["chain"])),
        }

    has_chain = params.get("solve_score_chain") not in ("", None, [])
    has_legacy_metric = bool(str(params.get("solve_metric") or params.get("metric") or "").strip())
    if source_key_present and not has_chain and not has_legacy_metric:
        raise RuntimeError("solve-score program source is empty and no solve_score_chain was provided")

    compiled = compile_solve_score_chain_or_legacy(
        params.get("solve_score_chain", ""),
        params.get("solve_metric") or params.get("metric") or "",
        params.get("solve_score_quantile", ""),
        params.get("solve_score_omega", 1),
        params.get("solve_score_omega_enabled", True),
        default_metric="proximity",
    )
    return {
        **compiled,
        "chain_public": public_solve_score_chain(compiled["chain"]),
        "source_text": solve_score_source_text_from_chain(public_solve_score_chain(compiled["chain"])),
    }
