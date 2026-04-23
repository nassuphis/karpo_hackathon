#!/usr/bin/env python3
"""
Shared Step Functions template renderer.

Deploy and tests should both call through this module so placeholder
substitution and render-workflow contract injection stay on one path.
"""

from __future__ import annotations

import argparse
import json
import sys
from copy import deepcopy
from pathlib import Path


ROOT = Path(__file__).resolve().parent
LAMBDA_DIR = ROOT / "lambda"
if str(LAMBDA_DIR) not in sys.path:
    sys.path.insert(0, str(LAMBDA_DIR))

from workflow_contracts import (
    RENDER_BILEVEL_FINALIZE_TASK_PAYLOAD,
    RENDER_BILEVEL_RASTER_ITEM_SELECTOR,
    RENDER_COEFF_FINALIZE_TASK_PAYLOAD,
    RENDER_COEFF_RASTER_ITEM_SELECTOR,
    RENDER_COLOR_CLIP_TASK_PAYLOAD,
    RENDER_COLOR_RASTER_ITEM_SELECTOR,
    RENDER_FINALIZE_MT_TASK_PAYLOAD,
)


RENDER_TEMPLATE_PATH = ROOT / "stepfunctions" / "render_workflow.asl.json.template"


def _read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def _replace_placeholders(text: str, replacements: dict[str, str]) -> str:
    rendered = text
    for key, value in replacements.items():
        rendered = rendered.replace("${" + key + "}", value)
    unresolved = sorted({frag.split("}", 1)[0] for frag in rendered.split("${")[1:] if "}" in frag})
    if unresolved:
        raise RuntimeError(f"unresolved template placeholders: {', '.join(unresolved)}")
    return rendered


def _render_json_template(path: Path, replacements: dict[str, str]) -> dict:
    return json.loads(_replace_placeholders(_read(path), replacements))


def _render_test_arn(*, account_id: str, region: str, placeholder: str) -> str:
    return f"arn:aws:lambda:{region}:{account_id}:function:placeholder-{placeholder}"


def _render_workflow_states(asl: dict) -> dict:
    wrapper = asl["States"]["WorkflowWrapper"]
    branches = wrapper.get("Branches") or []
    if not branches:
        raise RuntimeError("render workflow missing WorkflowWrapper.Branches")
    states = branches[0].get("States") or {}
    if not states:
        raise RuntimeError("render workflow missing inner state container")
    return states


def _apply_render_workflow_contracts(asl: dict) -> dict:
    states = _render_workflow_states(asl)
    states["ColorClipTask"]["Parameters"]["Payload"] = deepcopy(RENDER_COLOR_CLIP_TASK_PAYLOAD)
    states["ColorRasterMap"]["ItemSelector"] = deepcopy(RENDER_COLOR_RASTER_ITEM_SELECTOR)
    states["ColorAssembleEncodeTask"]["Parameters"]["Payload"] = deepcopy(RENDER_FINALIZE_MT_TASK_PAYLOAD)
    states["BilevelRasterMap"]["ItemSelector"] = deepcopy(RENDER_BILEVEL_RASTER_ITEM_SELECTOR)
    states["BilevelFinalizeTask"]["Parameters"]["Payload"] = deepcopy(RENDER_BILEVEL_FINALIZE_TASK_PAYLOAD)
    states["CoeffRasterMap"]["ItemSelector"] = deepcopy(RENDER_COEFF_RASTER_ITEM_SELECTOR)
    states["CoeffFinalizeTask"]["Parameters"]["Payload"] = deepcopy(RENDER_COEFF_FINALIZE_TASK_PAYLOAD)
    return asl


def render_render_workflow_definition(
    *,
    plan_function_arn: str,
    status_function_arn: str,
    finalize_mt_function_arn: str,
    storage_function_arn: str,
    bilevel_function_arn: str,
    solve_proximity_function_arn: str,
) -> dict:
    asl = _render_json_template(
        RENDER_TEMPLATE_PATH,
        {
            "PlanFunctionArn": plan_function_arn,
            "StatusFunctionArn": status_function_arn,
            "FinalizeMTFunctionArn": finalize_mt_function_arn,
            "StorageFunctionArn": storage_function_arn,
            "BilevelFunctionArn": bilevel_function_arn,
            "SolveProximityFunctionArn": solve_proximity_function_arn,
        },
    )
    return _apply_render_workflow_contracts(asl)


def render_render_workflow_definition_for_tests(
    *,
    account_id: str = "123456789012",
    region: str = "us-east-1",
) -> dict:
    return render_render_workflow_definition(
        plan_function_arn=_render_test_arn(account_id=account_id, region=region, placeholder="PlanFunctionArn"),
        status_function_arn=_render_test_arn(account_id=account_id, region=region, placeholder="StatusFunctionArn"),
        finalize_mt_function_arn=_render_test_arn(account_id=account_id, region=region, placeholder="FinalizeMTFunctionArn"),
        storage_function_arn=_render_test_arn(account_id=account_id, region=region, placeholder="StorageFunctionArn"),
        bilevel_function_arn=_render_test_arn(account_id=account_id, region=region, placeholder="BilevelFunctionArn"),
        solve_proximity_function_arn=_render_test_arn(account_id=account_id, region=region, placeholder="SolveProximityFunctionArn"),
    )


def _write_json(path: Path, payload: dict) -> None:
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="command", required=True)

    render = sub.add_parser("render-workflow")
    render.add_argument("--out", required=True)
    render.add_argument("--plan-function-arn", required=True)
    render.add_argument("--status-function-arn", required=True)
    render.add_argument("--finalize-mt-function-arn", required=True)
    render.add_argument("--storage-function-arn", required=True)
    render.add_argument("--bilevel-function-arn", required=True)
    render.add_argument("--solve-proximity-function-arn", required=True)

    return parser.parse_args()


def main() -> int:
    args = _parse_args()
    if args.command == "render-workflow":
        payload = render_render_workflow_definition(
            plan_function_arn=args.plan_function_arn,
            status_function_arn=args.status_function_arn,
            finalize_mt_function_arn=args.finalize_mt_function_arn,
            storage_function_arn=args.storage_function_arn,
            bilevel_function_arn=args.bilevel_function_arn,
            solve_proximity_function_arn=args.solve_proximity_function_arn,
        )
        _write_json(Path(args.out), payload)
        return 0
    raise RuntimeError(f"unsupported command: {args.command}")


if __name__ == "__main__":
    raise SystemExit(main())
