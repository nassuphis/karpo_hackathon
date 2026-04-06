import json
import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parent.parent
MANIFEST_PATH = ROOT / "api_manifest.json"

from api_manifest import build_manifest


class TestApiRouteContracts(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.generated = build_manifest()
        cls.checked_in = json.loads(MANIFEST_PATH.read_text())

    def test_checked_in_manifest_matches_current_tree(self):
        self.assertEqual(self.checked_in, self.generated)

    def test_manifest_parser_sanity(self):
        frontend = self.generated["frontend"]
        backend = self.generated["backend"]
        deploy = self.generated["deploy"]
        self.assertGreaterEqual(len(frontend["literal_services"]), 8)
        self.assertGreaterEqual(len(frontend["storage_paths"]), 10)
        self.assertGreaterEqual(len(frontend["dispatch_targets"]), 6)
        self.assertEqual(set(frontend["solver_lores_endpoints"]), {"aberth", "aberth_mt", "companion_matrix"})
        self.assertEqual(set(frontend["solver_dispatch_targets"]), {"aberth", "aberth_mt", "companion_matrix"})
        self.assertGreaterEqual(len(backend["storage_routes"]), 15)
        self.assertGreaterEqual(len(backend["dispatch_targets"]), 12)
        self.assertGreaterEqual(len(deploy["config_services"]), 10)
        self.assertGreaterEqual(len(deploy["api_routes"]), 20)

    def test_frontend_literal_services_exist_in_deploy_config(self):
        frontend_services = self.generated["frontend"]["literal_services"]
        deploy_services = set(self.generated["deploy"]["config_services"])
        missing = [service for service in frontend_services if service not in deploy_services]
        if missing:
            self.fail(
                "index.html uses services missing from deploy.sh config.json generation: "
                + ", ".join(missing)
            )

    def test_frontend_storage_paths_exist_in_storage_handler(self):
        frontend_paths = self.generated["frontend"]["storage_paths"]
        backend_paths = set(self.generated["backend"]["storage_routes"])
        missing = [path for path in frontend_paths if path not in backend_paths]
        if missing:
            self.fail(
                "index.html uses storage routes missing from handler_storage.py: "
                + ", ".join(missing)
            )

    def test_frontend_storage_paths_are_published_by_deploy(self):
        frontend_paths = self.generated["frontend"]["storage_paths"]
        deploy_paths = set(self.generated["deploy"]["api_routes"])
        missing = [path for path in frontend_paths if path not in deploy_paths]
        if missing:
            self.fail(
                "index.html uses storage routes not published by deploy.sh: "
                + ", ".join(missing)
            )

    def test_frontend_dispatch_targets_exist_in_dispatch_handler(self):
        frontend_targets = self.generated["frontend"]["dispatch_targets"]
        backend_targets = set(self.generated["backend"]["dispatch_targets"])
        missing = [target for target in frontend_targets if target not in backend_targets]
        if missing:
            self.fail(
                "index.html uses dispatch targets missing from handler_dispatch.py: "
                + ", ".join(missing)
            )

    def test_solver_lores_endpoints_exist_in_deploy_config(self):
        lores_services = sorted(set(self.generated["frontend"]["solver_lores_endpoints"].values()))
        deploy_services = set(self.generated["deploy"]["config_services"])
        missing = [service for service in lores_services if service not in deploy_services]
        if missing:
            self.fail(
                "_solverLoresEndpoint uses services missing from deploy.sh config.json generation: "
                + ", ".join(missing)
            )

    def test_solver_dispatch_targets_exist_in_dispatch_handler(self):
        solver_targets = sorted(set(self.generated["frontend"]["solver_dispatch_targets"].values()))
        backend_targets = set(self.generated["backend"]["dispatch_targets"])
        missing = [target for target in solver_targets if target not in backend_targets]
        if missing:
            self.fail(
                "_solverDispatchTarget uses targets missing from handler_dispatch.py: "
                + ", ".join(missing)
            )

    def test_deploy_config_service_paths_have_api_routes(self):
        config_services = self.generated["deploy"]["config_services"]
        deploy_paths = set(self.generated["deploy"]["api_routes"])
        missing = []
        for service, suffix in sorted(config_services.items()):
            if not suffix:
                continue
            if suffix not in deploy_paths:
                missing.append(f"{service}:{suffix}")
        if missing:
            self.fail(
                "deploy.sh config.json services are missing API Gateway routes: "
                + ", ".join(missing)
            )


if __name__ == "__main__":
    unittest.main()
