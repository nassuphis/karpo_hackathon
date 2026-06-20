import os
import sys


LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
if LAMBDA_DIR not in sys.path:
    sys.path.insert(0, LAMBDA_DIR)

from coeff_program_chain import compile_coeff_program_chain
from param_program_chain import compile_param_program_chain
from pipeline_programs import coeff_transforms_to_program_chain, param_transforms_to_program_chain


def _row_name_args(row):
    if isinstance(row, str):
        return row, []
    if isinstance(row, (list, tuple)) and row:
        return str(row[0]), [str(arg) for arg in row[1:]]
    raise ValueError(f"invalid transform row: {row!r}")


def _param_program_payload_from_transforms(transforms):
    chain = param_transforms_to_program_chain(transforms)
    compiled = compile_param_program_chain(chain)
    payload = {
        "version": compiled["version"],
        "fingerprint": compiled["fingerprint"],
        "tokens": compiled["tokens"],
        "stack_max": compiled["stack_max"],
        "token_count": compiled["token_count"],
        "uses_legacy_fast_path": compiled["uses_legacy_fast_path"],
    }
    if compiled.get("scalar_exprs"):
        payload["scalar_exprs"] = compiled["scalar_exprs"]
    return payload


def _coeff_program_payload_from_transforms(transforms):
    chain = coeff_transforms_to_program_chain(transforms)
    compiled = compile_coeff_program_chain(chain)
    return {
        "version": compiled["version"],
        "fingerprint": compiled["fingerprint"],
        "tokens": compiled["tokens"],
        "stack_max": compiled["stack_max"],
        "token_count": compiled["token_count"],
        "scalar_exprs": compiled["scalar_exprs"],
        "uses_legacy_chain_equivalent": compiled["uses_legacy_chain_equivalent"],
    }


def translate_legacy_transforms_for_native(spec):
    """Mirror the Lambda boundary for direct sweep_test fixtures.

    The native JSON-chain parsers are retired; tests that still describe old
    Chain-mode rows should exercise the compiled Program payload path instead.
    """
    out = dict(spec)
    param_transforms = out.get("param_transforms") or []
    if param_transforms and not out.get("param_program"):
        out["param_program"] = _param_program_payload_from_transforms(param_transforms)
        out["param_transforms"] = []
    coeff_transforms = out.get("coeff_transforms") or []
    if coeff_transforms and not out.get("coeff_program"):
        out["coeff_program"] = _coeff_program_payload_from_transforms(coeff_transforms)
        out["coeff_transforms"] = []
    return out
