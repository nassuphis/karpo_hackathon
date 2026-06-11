import argparse
import ast
import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parent
MANIFEST_PATH = ROOT / "api_manifest.json"
INDEX_PATH = ROOT / "index.html"
DEPLOY_PATH = ROOT / "deploy.sh"
DEPLOY_MANIFEST_PATH = ROOT / "deploy_manifest.json"
STORAGE_HANDLER_PATH = ROOT / "lambda" / "handler_storage.py"
DISPATCH_HANDLER_PATH = ROOT / "lambda" / "handler_dispatch.py"


def _read(path: Path) -> str:
    return path.read_text()


def _extract_function_calls(text: str, func_name: str) -> list[str]:
    needle = func_name + "("
    calls = []
    start = 0
    while True:
        idx = text.find(needle, start)
        if idx < 0:
            break
        i = idx + len(needle)
        depth = 1
        in_str = None
        escape = False
        while i < len(text) and depth > 0:
            ch = text[i]
            if in_str is not None:
                if escape:
                    escape = False
                elif ch == "\\":
                    escape = True
                elif ch == in_str:
                    in_str = None
            else:
                if ch in ("'", '"', "`"):
                    in_str = ch
                elif ch == "(":
                    depth += 1
                elif ch == ")":
                    depth -= 1
            i += 1
        calls.append(text[idx:i])
        start = i
    return calls


def _split_top_level_args(src: str) -> list[str]:
    args = []
    start = 0
    paren = brace = bracket = 0
    in_str = None
    escape = False
    for i, ch in enumerate(src):
        if in_str is not None:
            if escape:
                escape = False
            elif ch == "\\":
                escape = True
            elif ch == in_str:
                in_str = None
            continue
        if ch in ("'", '"', "`"):
            in_str = ch
        elif ch == "(":
            paren += 1
        elif ch == ")":
            paren -= 1
        elif ch == "{":
            brace += 1
        elif ch == "}":
            brace -= 1
        elif ch == "[":
            bracket += 1
        elif ch == "]":
            bracket -= 1
        elif ch == "," and paren == 0 and brace == 0 and bracket == 0:
            args.append(src[start:i].strip())
            start = i + 1
    tail = src[start:].strip()
    if tail:
        args.append(tail)
    return args


def _parse_js_string_literal(token: str | None) -> str | None:
    if not token:
        return None
    token = token.strip()
    if len(token) >= 2 and token[0] == token[-1] and token[0] in ("'", '"'):
        return token[1:-1]
    return None


def _extract_frontend_lambda_calls(index_text: str) -> list[dict]:
    calls = []
    for call_src in _extract_function_calls(index_text, "lambdaPost"):
        inner = call_src[len("lambdaPost("):-1]
        args = _split_top_level_args(inner)
        service = _parse_js_string_literal(args[0]) if args else None
        body = args[1].strip() if len(args) >= 2 else ""
        path = _parse_js_string_literal(args[2]) if len(args) >= 3 else None
        calls.append({
            "service": service,
            "path": path,
            "body": body,
            "source": call_src,
        })
    return calls


def _extract_literal_dispatch_targets(calls: list[dict]) -> list[str]:
    targets = set()
    for call in calls:
        if call["service"] != "dispatch":
            continue
        match = re.search(r"\btarget\s*:\s*['\"]([^'\"]+)['\"]", call["body"])
        if match:
            targets.add(match.group(1))
    return sorted(targets)


def _extract_solver_mapping(index_text: str, fn_name: str, default_key: str = "aberth_mt") -> dict[str, str]:
    pattern = rf"function {re.escape(fn_name)}\(solver\)\s*\{{\s*return\s+([^;]+);"
    match = re.search(pattern, index_text, re.DOTALL)
    if not match:
        raise RuntimeError(f"Could not parse {fn_name} from index.html")
    expr = " ".join(match.group(1).split())
    mapping = {}
    for solver_key, target in re.findall(r"solver === ['\"]([^'\"]+)['\"]\s*\?\s*['\"]([^'\"]+)['\"]", expr):
        mapping[solver_key] = target
    default_match = re.search(r":\s*['\"]([^'\"]+)['\"]\s*$", expr)
    if not default_match:
        raise RuntimeError(f"Could not parse default target for {fn_name}")
    mapping[default_key] = default_match.group(1)
    return {key: mapping[key] for key in sorted(mapping)}


def _extract_storage_routes(storage_text: str) -> list[str]:
    return sorted(set("/" + route for route in re.findall(r'path\.endswith\("/([^"]+)"\)', storage_text)))


def _extract_dispatch_targets(dispatch_text: str) -> list[str]:
    tree = ast.parse(dispatch_text, filename=str(DISPATCH_HANDLER_PATH))
    for node in tree.body:
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id == "FUNCTIONS" and isinstance(node.value, ast.Dict):
                    keys = []
                    for key in node.value.keys:
                        if isinstance(key, ast.Constant) and isinstance(key.value, str):
                            keys.append(key.value)
                    return sorted(keys)
    raise RuntimeError("Could not parse FUNCTIONS from handler_dispatch.py")


def _extract_deploy_config_services(deploy_text: str) -> dict[str, str]:
    ignored = {"build_id", "deployed_at_utc", "git_rev", "frontend_sha256"}
    services = {}
    for key, suffix in re.findall(r'"([^"]+)": "%s([^"]*)"', deploy_text):
        if key in ignored:
            continue
        services[key] = suffix
    if not services:
        raise RuntimeError("Could not parse config.json template from deploy.sh")
    return {key: services[key] for key in sorted(services)}


def _extract_deploy_routes() -> list[str]:
    # Routes live in deploy_manifest.json (the fleet source of truth that
    # deploy.sh sources via deploy_manifest.py --emit-bash), not in deploy.sh.
    manifest = json.loads(_read(DEPLOY_MANIFEST_PATH))
    routes = set()
    for fn in manifest.get("functions", []):
        routes.update(fn.get("routes", []))
    return sorted(routes)


def build_manifest() -> dict:
    index_text = _read(INDEX_PATH)
    deploy_text = _read(DEPLOY_PATH)
    storage_text = _read(STORAGE_HANDLER_PATH)
    dispatch_text = _read(DISPATCH_HANDLER_PATH)

    calls = _extract_frontend_lambda_calls(index_text)
    literal_services = sorted({call["service"] for call in calls if call["service"]})
    storage_paths = sorted({
        call["path"] for call in calls
        if call["service"] == "storage" and call["path"]
    })

    manifest = {
        "version": 1,
        "frontend": {
            "literal_services": literal_services,
            "storage_paths": storage_paths,
            "dispatch_targets": _extract_literal_dispatch_targets(calls),
            "solver_lores_endpoints": _extract_solver_mapping(index_text, "_solverLoresEndpoint"),
            "solver_dispatch_targets": _extract_solver_mapping(index_text, "_solverDispatchTarget"),
        },
        "backend": {
            "storage_routes": _extract_storage_routes(storage_text),
            "dispatch_targets": _extract_dispatch_targets(dispatch_text),
        },
        "deploy": {
            "config_services": _extract_deploy_config_services(deploy_text),
            "api_routes": _extract_deploy_routes(),
        },
    }
    return manifest


def write_manifest(path: Path = MANIFEST_PATH) -> None:
    path.write_text(json.dumps(build_manifest(), indent=2, sort_keys=True) + "\n")


def check_manifest(path: Path = MANIFEST_PATH) -> bool:
    current = build_manifest()
    if not path.exists():
        return False
    checked_in = json.loads(path.read_text())
    return checked_in == current


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate or validate api_manifest.json")
    parser.add_argument("--write", action="store_true", help="Write api_manifest.json from the current tree")
    parser.add_argument("--check", action="store_true", help="Validate api_manifest.json matches the current tree")
    args = parser.parse_args()

    if args.write:
        write_manifest()
        print(MANIFEST_PATH)
        return 0
    if args.check:
        if not check_manifest():
            print(f"FATAL: {MANIFEST_PATH.name} is out of date with the current tree")
            return 1
        print(f"{MANIFEST_PATH.name}: OK")
        return 0

    print(json.dumps(build_manifest(), indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
