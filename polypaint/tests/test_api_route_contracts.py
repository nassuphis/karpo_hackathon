import pathlib
import re
import unittest


ROOT = pathlib.Path(__file__).resolve().parent.parent
INDEX_TEXT = (ROOT / "index.html").read_text()
DEPLOY_TEXT = (ROOT / "deploy.sh").read_text()
STORAGE_TEXT = (ROOT / "lambda" / "handler_storage.py").read_text()


def _frontend_lambda_calls():
    pattern = re.compile(
        r"lambdaPost\(\s*['\"](?P<service>[^'\"]+)['\"]\s*,.*?(?:,\s*['\"](?P<path>/[^'\"]+)['\"])?\s*\)",
        re.DOTALL,
    )
    calls = []
    for match in pattern.finditer(INDEX_TEXT):
        calls.append((match.group("service"), match.group("path") or ""))
    return calls


def _storage_handler_routes():
    return sorted(set("/" + route for route in re.findall(r'path\.endswith\("/([^"]+)"\)', STORAGE_TEXT)))


class TestApiRouteContracts(unittest.TestCase):
    def test_frontend_storage_paths_exist_in_storage_handler(self):
        frontend_paths = sorted({
            path for service, path in _frontend_lambda_calls()
            if service == "storage" and path
        })
        handler_paths = set(_storage_handler_routes())
        missing = [path for path in frontend_paths if path not in handler_paths]
        if missing:
            self.fail(
                "index.html uses storage routes missing from handler_storage.py: "
                + ", ".join(missing)
            )

    def test_frontend_storage_paths_are_published_by_deploy(self):
        frontend_paths = sorted({
            path for service, path in _frontend_lambda_calls()
            if service == "storage" and path
        })
        missing = []
        for path in frontend_paths:
            needle = f'ensure_route "POST {path}" "$STORAGE_INT"'
            if needle not in DEPLOY_TEXT:
                missing.append(path)
        if missing:
            self.fail(
                "index.html uses storage routes not published by deploy.sh: "
                + ", ".join(missing)
            )

    def test_frontend_services_exist_in_config_json_template(self):
        frontend_services = sorted({service for service, _ in _frontend_lambda_calls()})
        missing = []
        for service in frontend_services:
            if service == "storage":
                needle = '"storage": "%s"'
            else:
                needle = f'"{service}": "%s'
            if needle not in DEPLOY_TEXT:
                missing.append(service)
        if missing:
            self.fail(
                "index.html uses services missing from deploy.sh config.json generation: "
                + ", ".join(missing)
            )


if __name__ == "__main__":
    unittest.main()
