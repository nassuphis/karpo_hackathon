"""Shared pipeline-mode and coeff-source resolution for the compute handlers.

One source of truth (CR14) for three rules that were independently
implemented in handler_compute_plan, handler_compute_preview, and
handler_coeffgen - exactly the duplication class that produced the CR6
blank-source_text drift:

- pipeline_mode_from_params: Chain vs Program mode selection (Chain mode
  zeroes programs downstream; Program mode zeroes legacy transforms).
- coeff_source_text_for_run: request-path precedence - source text wins
  over a chain when it is non-blank, or when there is no chain to fall
  back to. The saved-payload twin of this rule lives in
  coeff_program_source.coeff_source_text_from_payload (fetch/macro paths).
- parse_coeff_source_for_run: compile with structured diagnostics. The
  editor's save/compile route always preserved line/column diagnostics;
  the compute paths used to flatten them into a bare string, making
  preview/probe failures needlessly hard to debug.
"""
from coeff_program_source import parse_coeff_program_source


class CoeffSourceCompileError(ValueError):
    """Coeff source failed to compile; carries the structured diagnostics.

    str(error) is a one-line summary including line/column (safe for Step
    Functions Cause text and status rows); .diagnostics is the full list of
    {level, message, line, column} dicts for API responses.
    """

    def __init__(self, message, diagnostics):
        super().__init__(message)
        self.diagnostics = list(diagnostics)


def pipeline_mode_from_params(params):
    """Chain vs Program selection, with legacy aliases and inference."""
    raw = str(params.get("pipeline_mode") or params.get("compute_pipeline_mode") or "").strip().lower()
    if not raw:
        raw = "program" if (
            params.get("param_program_chain")
            or params.get("coeff_program_chain")
            or str(params.get("coeff_program_source_text") or "").strip()
            or params.get("param_program")
            or params.get("coeff_program")
        ) else "chain"
    raw = {"legacy": "chain", "chains": "chain", "programs": "program"}.get(raw, raw)
    if raw not in {"chain", "program"}:
        raise ValueError("pipeline_mode must be one of chain, program")
    return raw


def coeff_source_text_for_run(params, pipeline_mode):
    """Request-path source-vs-chain precedence.

    Returns the source text when it is authoritative for this run, else
    None (use the chain). pipeline_mode=None skips the mode gate for
    callers whose payloads are already mode-filtered upstream (the
    coeffgen degree probe).
    """
    if pipeline_mode is not None and pipeline_mode != "program":
        return None
    if "coeff_program_source_text" not in params:
        return None
    raw = str(params.get("coeff_program_source_text") or "")
    if raw.strip() or not params.get("coeff_program_chain"):
        return raw
    return None


def parse_coeff_source_for_run(source_text):
    """Parse coeff source, raising CoeffSourceCompileError with structure.

    Uses strict=False so every diagnostic keeps its line/column instead of
    the first error aborting with a flattened message.
    """
    parsed = parse_coeff_program_source(source_text, strict=False)
    diagnostics = parsed.get("diagnostics") or []
    if diagnostics:
        first = diagnostics[0]
        summary = (
            f"coeff program source line {first.get('line', 0)},"
            f" column {first.get('column', 0)}: {first.get('message', 'compile error')}"
        )
        if len(diagnostics) > 1:
            summary += f" (+{len(diagnostics) - 1} more)"
        raise CoeffSourceCompileError(summary, diagnostics)
    return parsed
