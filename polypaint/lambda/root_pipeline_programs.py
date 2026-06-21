"""Root-program resolution without importing the full compute pipeline helper.

Direct root-consuming handlers only need to normalize root source/program/legacy
payloads to the existing root_transforms wire.  Keep this module root-only so
their Lambda bundles do not need the Param/Coeff/Solve-Score compilers.
"""


class RootSourceCompileError(ValueError):
    """Root source/chain failed to compile; carries structured diagnostics."""

    def __init__(self, message, diagnostics):
        super().__init__(message)
        self.diagnostics = list(diagnostics)


def root_source_text_for_run(params):
    """Request-path Root source-vs-chain precedence."""
    if "root_program_source_text" in params:
        raw = str(params.get("root_program_source_text") or "")
        if raw.strip():
            return raw
    root_program = params.get("root_program")
    if isinstance(root_program, dict):
        raw = str(root_program.get("source_text") or "")
        if raw.strip():
            return raw
    return None


def root_transforms_to_program_chain(root_transforms):
    from root_program_source import compile_root_program_chain

    return compile_root_program_chain(root_transforms or [], strict=True)["chain"]


def _raise_root_if_diagnostics(compiled, *, label):
    diagnostics = list((compiled or {}).get("diagnostics") or [])
    if diagnostics:
        first = diagnostics[0]
        summary = (
            f"{label} line {first.get('line', 0)},"
            f" column {first.get('column', 0)}: {first.get('message', 'compile error')}"
        )
        if len(diagnostics) > 1:
            summary += f" (+{len(diagnostics) - 1} more)"
        raise RootSourceCompileError(summary, diagnostics)


def root_program_for_run(params, *, strict=True):
    """Resolve Root source/program/legacy rows to a compiled root program.

    Precedence:
    1. root_program_source_text
    2. root_program.source_text
    3. root_program.chain
    4. root_transforms
    """
    from root_program_source import (
        compile_root_program_chain,
        compile_root_program_source,
        root_source_text_from_chain,
    )

    source_text = root_source_text_for_run(params)
    if source_text is not None:
        compiled = compile_root_program_source(source_text, strict=False)
        if strict:
            _raise_root_if_diagnostics(compiled, label="root program source")
        compiled["source_text"] = source_text
        return compiled

    root_program = params.get("root_program")
    if isinstance(root_program, dict) and isinstance(root_program.get("chain"), list):
        compiled = compile_root_program_chain(root_program.get("chain") or [], strict=strict)
        if strict:
            _raise_root_if_diagnostics(compiled, label="root program chain")
        compiled["source_text"] = str(root_program.get("source_text") or "") or root_source_text_from_chain(compiled["chain"])
        return compiled

    compiled = compile_root_program_chain(params.get("root_transforms") or [], strict=strict)
    if strict:
        _raise_root_if_diagnostics(compiled, label="root transforms")
    compiled["source_text"] = root_source_text_from_chain(compiled["chain"])
    return compiled
