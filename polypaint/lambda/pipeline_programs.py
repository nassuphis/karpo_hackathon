"""Shared program-source resolution for compute handlers.

One source of truth (CR14) for three rules that were independently
implemented in handler_compute_plan, handler_compute_preview, and
handler_coeffgen - exactly the duplication class that produced the CR6
blank-source_text drift:

- pipeline_mode_from_params: compatibility shim for the retired Chain/Program
  selector. All accepted legacy aliases normalize to Program; legacy transform
  rows, when present, are translated to program payloads at the Lambda boundary.
- coeff_source_text_for_run / param_source_text_for_run: request-path
  precedence - non-blank source text wins over a chain. Blank strings are
  treated as absent because orchestrators add them as ASL JSONPath defaults.
  The saved-payload twins of the stricter save-time rule live in
  coeff_program_source/param_program_source.*_source_text_from_payload.
- parse_coeff_source_for_run: compile with structured diagnostics. The
  editor's save/compile route always preserved line/column diagnostics;
  the compute paths used to flatten them into a bare string, making
preview/probe failures needlessly hard to debug.
"""
from root_pipeline_programs import (
    RootSourceCompileError,
    root_program_for_run,
    root_source_text_for_run,
    root_transforms_to_program_chain,
)
from solve_score_pipeline_programs import (
    SolveScoreSourceCompileError,
    solve_score_program_for_run,
    solve_score_source_text_for_run,
)


class CoeffSourceCompileError(ValueError):
    """Coeff source failed to compile; carries the structured diagnostics.

    str(error) is a one-line summary including line/column (safe for Step
    Functions Cause text and status rows); .diagnostics is the full list of
    {level, message, line, column} dicts for API responses.
    """

    def __init__(self, message, diagnostics):
        super().__init__(message)
        self.diagnostics = list(diagnostics)


class ParamSourceCompileError(ValueError):
    """Param source failed to compile; carries structured diagnostics."""

    def __init__(self, message, diagnostics):
        super().__init__(message)
        self.diagnostics = list(diagnostics)


def pipeline_mode_from_params(params):
    """Return the only supported compute pipeline mode.

    Chain mode was retired after the native chain parsers were deleted. Keep
    accepting old aliases so stale browser tabs and saved payloads still route
    through the boundary translators instead of reaching native arrays.
    """
    raw = str(params.get("pipeline_mode") or params.get("compute_pipeline_mode") or "").strip().lower()
    if not raw:
        return "program"
    raw = {"legacy": "program", "chains": "program", "chain": "program", "programs": "program"}.get(raw, raw)
    if raw != "program":
        raise ValueError("pipeline_mode must be program")
    return "program"


def coeff_source_text_for_run(params, pipeline_mode):
    """Request-path source-vs-chain precedence.

    Returns the source text when it is authoritative for this run, else
    None (use the chain/legacy fallback). pipeline_mode is kept only for
    old call sites; Chain mode normalizes to Program before this point.
    """
    if pipeline_mode is not None and pipeline_mode != "program":
        return None
    if "coeff_program_source_text" not in params:
        return None
    raw = str(params.get("coeff_program_source_text") or "")
    if raw.strip():
        return raw
    return None


def param_source_text_for_run(params, pipeline_mode):
    """Request-path Param source-vs-chain precedence."""
    if pipeline_mode is not None and pipeline_mode != "program":
        return None
    if "param_program_source_text" not in params:
        return None
    raw = str(params.get("param_program_source_text") or "")
    if raw.strip():
        return raw
    return None


def parse_coeff_source_for_run(source_text):
    """Parse coeff source, raising CoeffSourceCompileError with structure.

    Uses strict=False so every diagnostic keeps its line/column instead of
    the first error aborting with a flattened message.
    """
    from coeff_program_source import parse_coeff_program_source

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


def parse_param_source_for_run(source_text):
    """Parse Param source, raising ParamSourceCompileError with structure."""
    from param_program_source import parse_param_program_source

    parsed = parse_param_program_source(source_text, strict=False)
    diagnostics = parsed.get("diagnostics") or []
    if diagnostics:
        first = diagnostics[0]
        summary = (
            f"param program source line {first.get('line', 0)},"
            f" column {first.get('column', 0)}: {first.get('message', 'compile error')}"
        )
        if len(diagnostics) > 1:
            summary += f" (+{len(diagnostics) - 1} more)"
        raise ParamSourceCompileError(summary, diagnostics)
    return parsed


def _transform_row_name_args(row, *, label):
    if isinstance(row, str):
        name = row.strip()
        args = []
    elif isinstance(row, list) and row:
        name = str(row[0] or "").strip()
        args = [str(arg) for arg in row[1:]]
    else:
        raise ValueError(f"{label} transform row must be a non-empty string or array")
    if not name:
        raise ValueError(f"{label} transform name is empty")
    return name, args


_PARAM_TARGET_FIRST_TRANSFORMS = {
    "crd",
    "hrt",
    "spdl",
    "lmc",
    "rsc",
    "lss",
    "ast",
    "asp",
    "lsp",
    "dlt",
    "rply",
    "star",
    "rect",
    "rrect",
}

_PARAM_DITHER_TARGET_FIRST_TRANSFORMS = {
    "ddith",
    "adth",
    "ldth",
    "crdth",
    "scdth",
}

_PARAM_TARGET_LAST_TRANSFORMS = {"rtheta"}


def _param_legacy_target_selector(value):
    raw = str(value if value is not None else "both").strip().lower()
    if raw in {"0", "t1", "p1"}:
        return "p1"
    if raw in {"1", "t2", "p2"}:
        return "p2"
    if raw in {"2", "both", ""}:
        return "both"
    raise ValueError(f"param transform target must be t1, t2, or both, got {value!r}")


def _param_legacy_target_index(value):
    raw = str(value if value is not None else "both").strip().lower()
    if raw in {"0", "t1", "p1"}:
        return "0"
    if raw in {"1", "t2", "p2"}:
        return "1"
    if raw in {"2", "both", ""}:
        return "2"
    raise ValueError(f"param transform target must be t1, t2, or both, got {value!r}")


def param_transforms_to_program_chain(transforms):
    """Translate legacy Param Chain rows into Param Program legacy chips.

    Old chip rows encode some target selectors as ordinary arguments. The
    Param Program VM has explicit src/tgt selector slots, so normalize those
    rows here instead of forwarding obsolete target arguments to the compiler.
    Dither rows intentionally keep a numeric target argument because the RNG
    draw count is part of the historical behavior.
    """
    chain = []
    for row in list(transforms or []):
        name, args = _transform_row_name_args(row, label="param")
        legacy_name = name.strip().lower()
        if legacy_name in _PARAM_TARGET_LAST_TRANSFORMS:
            value_args = list(args)
            target = "both"
            if len(value_args) >= 2:
                target = _param_legacy_target_selector(value_args.pop())
            chain.append(["legacy", name, target, target, *value_args])
            continue
        if legacy_name in _PARAM_TARGET_FIRST_TRANSFORMS:
            target = _param_legacy_target_selector(args[0]) if args else "both"
            chain.append(["legacy", name, target, target, *args[1:]])
            continue
        if legacy_name in _PARAM_DITHER_TARGET_FIRST_TRANSFORMS:
            target = _param_legacy_target_index(args[0]) if args else "2"
            chain.append(["legacy", name, "both", "both", target, *args[1:]])
            continue
        chain.append(["legacy", name, "both", "both", *args])
    return chain


def coeff_transforms_to_program_chain(transforms):
    """Translate legacy Coeff Chain rows into a Coeff Program.

    Coeff Program starts with poly initialized from cf; each legacy transform
    mutates poly in-place; emit finalizes the result.
    """
    chain = []
    for row in list(transforms or []):
        name, args = _transform_row_name_args(row, label="coeff")
        legacy_name = name.strip().lower()
        if legacy_name == "exp" and len(args) in {2, 3}:
            # Old Chain exp(a,b[,andy]) means exp(z*(a+b*i)). The Program
            # form is exp_affine(multiplier, offset[,andy]), so repack the
            # old real/imag pair and pin offset to zero.
            multiplier = f"({args[0]})+({args[1]})*1j"
            next_args = [multiplier, "0"]
            if len(args) == 3:
                next_args.append(args[2])
            chain.append(["legacy", name, "poly", "poly", *next_args])
            continue
        if legacy_name == "round" and len(args) in {2, 3}:
            # Old Chain round(a,b[,andy]) means round(z*(a+b*i)); Program
            # round takes the packed complex multiplier as one field.
            multiplier = f"({args[0]})+({args[1]})*1j"
            next_args = [multiplier]
            if len(args) == 3:
                next_args.append(args[2])
            chain.append(["legacy", name, "poly", "poly", *next_args])
            continue
        chain.append(["legacy", name, "poly", "poly", *args])
    if chain:
        chain.append(["emit"])
    return chain
