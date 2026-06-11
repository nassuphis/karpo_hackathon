#!/usr/bin/env python3
"""Validate deploy_manifest.json and emit the bash it describes.

The manifest is the single source of truth for the deployed Lambda fleet:
per-function specs (name, handler, zip, memory, layers, env, /tmp), API
routes, gateway invoke permissions (derived: every routed function), and
retired resources. deploy.sh keeps the engine (create/update helpers, API
Gateway mechanics, IAM, packaging) and sources the output of:

    deploy_manifest.py --emit-bash

so the data lives once, here, and cannot drift between code paths. Tests
validate the manifest and the emitted bash (tests/test_deploy_packaging.py);
api_manifest.py reads routes from the manifest instead of grepping deploy.sh.

Usage:
    deploy_manifest.py --check        validate, print a summary
    deploy_manifest.py --emit-bash    validate, print bash to stdout
"""
import json
import os
import re
import sys

MANIFEST_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "deploy_manifest.json")

# Symbolic layer names -> the deploy.sh variables that hold published ARNs.
LAYER_VARS = {
    "lapack": "$LAPACK_LAYER",
    "libvips": "$LIBVIPS_LAYER",
    "pdf_py": "$PDF_PY_LAYER",
}

# Variables an env/layers/tmp fragment may reference besides the manifest's
# own name/memory variables. Everything else is a validation error.
ALLOWED_EXTERNAL_VARS = {
    "BUCKET", "JOBS_TABLE", "BINARY_TMP", "TIMEOUT", "RASTER_MT_THREADS",
    "RENDER_SM_ARN", "COMPUTE_SM_ARN", "PALETTE_SM_ARN",
}

GROUPS = ("core", "orchestrator")


def load_manifest(path=MANIFEST_PATH):
    with open(path, "r", encoding="utf-8") as fh:
        return json.load(fh)


def validate(manifest):
    errors = []

    def err(msg):
        errors.append(msg)

    functions = manifest.get("functions")
    if not isinstance(functions, list) or not functions:
        return ["manifest must contain a non-empty functions array"]

    # zip is deliberately not in the uniqueness set: bilevel and
    # color-to-bilevel ship the same bundle under two function configs.
    seen = {"key": set(), "name": set(), "name_var": set(), "memory_var": set()}
    declared_vars = set()
    for fn in functions:
        for field in ("key", "name", "name_var", "memory_var", "handler", "zip", "group"):
            if not isinstance(fn.get(field), str) or not fn.get(field):
                err(f"{fn.get('key', fn.get('name', '?'))}: missing or non-string {field}")
        declared_vars.add(fn.get("name_var", ""))
        declared_vars.add(fn.get("memory_var", ""))

    route_owner = {}
    for fn in functions:
        key = fn.get("key", "?")
        for field, value in (("key", fn.get("key")), ("name", fn.get("name")),
                             ("name_var", fn.get("name_var")),
                             ("memory_var", fn.get("memory_var"))):
            if value in seen[field]:
                err(f"{key}: duplicate {field} {value!r}")
            seen[field].add(value)
        if not str(fn.get("name", "")).startswith("polypaint-"):
            err(f"{key}: name must start with polypaint-")
        if fn.get("group") not in GROUPS:
            err(f"{key}: group must be one of {GROUPS}")
        if not isinstance(fn.get("memory_mb"), int) or fn["memory_mb"] < 128:
            err(f"{key}: memory_mb must be an int >= 128")
        if not re.fullmatch(r"handler_[a-z0-9_]+\.handler", str(fn.get("handler", ""))):
            err(f"{key}: handler must look like handler_<module>.handler")
        if not str(fn.get("zip", "")).startswith("/tmp/polypaint-"):
            err(f"{key}: zip must live under /tmp/polypaint-")
        for layer in fn.get("layers", []):
            if layer not in LAYER_VARS:
                err(f"{key}: unknown layer {layer!r} (known: {sorted(LAYER_VARS)})")
        tmp = fn.get("tmp")
        if tmp is not None and tmp != "binary" and not isinstance(tmp, int):
            err(f"{key}: tmp must be \"binary\", an int, or absent")
        rc = fn.get("reserved_concurrency")
        if rc is not None and (not isinstance(rc, int) or rc < 1):
            err(f"{key}: reserved_concurrency must be a positive int")
        for route in fn.get("routes", []):
            if not re.fullmatch(r"/[a-z0-9\-]+", route):
                err(f"{key}: route {route!r} must match /[a-z0-9-]+")
            if route in route_owner:
                err(f"{key}: route {route!r} already owned by {route_owner[route]}")
            route_owner[route] = key
        if not isinstance(fn.get("env"), str):
            err(f"{key}: env must be a string (may be empty)")
        else:
            for var in re.findall(r"\$\{?([A-Za-z_][A-Za-z0-9_]*)\}?", fn["env"]):
                if var in ALLOWED_EXTERNAL_VARS or var in declared_vars:
                    continue
                err(f"{key}: env references undeclared variable ${var}")

    removed = manifest.get("removed", {})
    active_names = {fn.get("name") for fn in functions}
    for name in removed.get("functions", []):
        if name in active_names:
            err(f"removed function {name!r} is still in the active fleet")
    for route in removed.get("routes", []):
        if route in route_owner:
            err(f"removed route {route!r} is still owned by {route_owner[route]}")

    return errors


def _spec_args(fn):
    layers = " ".join(LAYER_VARS[layer] for layer in fn.get("layers", []))
    args = [f'"${fn["memory_var"]}"', f'"{layers}"', f'"{fn["env"]}"']
    tmp = fn.get("tmp")
    if tmp == "binary":
        args.append('"$BINARY_TMP"')
    elif isinstance(tmp, int):
        args.append(f'"{tmp}"')
    return " ".join(args)


def emit_bash(manifest):
    functions = manifest["functions"]
    core = [fn for fn in functions if fn["group"] == "core"]
    orch = [fn for fn in functions if fn["group"] == "orchestrator"]
    removed = manifest.get("removed", {})
    out = []
    w = out.append

    w("# Generated by deploy_manifest.py from deploy_manifest.json — do not edit.")
    w("# Function names and memory sizes (the why lives in the manifest too):")
    for fn in functions:
        w(f'{fn["name_var"]}="{fn["name"]}"')
        why = f'  # {fn["memory_why"]}' if fn.get("memory_why") else ""
        w(f'{fn["memory_var"]}={fn["memory_mb"]}{why}')
    w("")

    w("# The Lambda spec list: one deploy_lambda call per function — name,")
    w("# handler, zip, memory, layers, env, /tmp MB. Both action branches run")
    w("# this verbatim; orchestrators deploy separately because they need the")
    w("# state machine ARNs in their env.")
    w("deploy_all_lambdas() {")
    for i, fn in enumerate(core):
        if i:
            w("")
        if fn.get("note"):
            w(f'    # {fn["note"]}')
        w(f'    deploy_lambda "${fn["name_var"]}" "{fn["handler"]}" "{fn["zip"]}" \\')
        w(f"        {_spec_args(fn)}")
        if fn.get("reserved_concurrency"):
            w(f'    # Reserve concurrency for {fn["key"]} so it\'s never starved by render/merge Lambdas')
            w(f'    aws lambda put-function-concurrency --function-name "${fn["name_var"]}" \\')
            w(f'        --reserved-concurrent-executions {fn["reserved_concurrency"]} --region "$REGION"')
    w("}")
    w("")

    w("deploy_orchestrator_lambdas() {")
    for fn in orch:
        w(f'    deploy_lambda "${fn["name_var"]}" "{fn["handler"]}" "{fn["zip"]}" \\')
        w(f"        {_spec_args(fn)}")
    w("}")
    w("")

    w("# Retired Lambdas deleted on every deploy until gone from all accounts.")
    w("delete_removed_lambdas() {")
    for name in removed.get("functions", []):
        w(f'    delete_lambda_if_exists "{name}"')
    if not removed.get("functions"):
        w("    :")
    w("}")
    w("")

    routed = [fn for fn in functions if fn.get("routes")]
    w("# Grant API Gateway permission to invoke each routed Lambda (derived:")
    w("# exactly the functions that own routes; async-only workers are invoked")
    w("# by dispatch and stay out of this list). Runs inside setup_api_gateway,")
    w("# where $ACCT and $API_ID are in scope.")
    w("grant_api_gateway_invoke_permissions() {")
    fname_list = " ".join(f'"${fn["name_var"]}"' for fn in routed)
    w(f"    for FNAME in {fname_list}; do")
    w('        aws lambda add-permission --function-name "$FNAME" \\')
    w('            --statement-id "apigateway-invoke" \\')
    w("            --action lambda:InvokeFunction \\")
    w("            --principal apigateway.amazonaws.com \\")
    w('            --source-arn "arn:aws:execute-api:$REGION:$ACCT:$API_ID/*/*" \\')
    w('            --region "$REGION" >/dev/null 2>&1 || true')
    w("    done")
    w("}")
    w("")

    w("# One integration per routed Lambda, then its routes. Removed routes and")
    w("# integrations are cleaned up first. Runs inside setup_api_gateway.")
    w("publish_api_routes() {")
    for route in removed.get("routes", []):
        w(f'    delete_route_if_exists "POST {route}"')
    for name in removed.get("functions", []):
        w(f'    delete_integration_for_lambda_if_exists "{name}"')
    w("    local INT")
    for fn in routed:
        w(f'    INT=$(create_integration "${fn["name_var"]}")')
        for route in fn["routes"]:
            w(f'    ensure_route "POST {route}" "$INT"')
    w("}")
    return "\n".join(out) + "\n"


def main(argv):
    if len(argv) != 2 or argv[1] not in ("--check", "--emit-bash"):
        sys.stderr.write(__doc__)
        return 2
    manifest = load_manifest()
    errors = validate(manifest)
    if errors:
        for msg in errors:
            sys.stderr.write(f"deploy_manifest.json: {msg}\n")
        return 1
    if argv[1] == "--check":
        functions = manifest["functions"]
        routes = sum(len(fn.get("routes", [])) for fn in functions)
        print(f"deploy_manifest.json: OK ({len(functions)} functions, {routes} routes)")
        return 0
    sys.stdout.write(emit_bash(manifest))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
