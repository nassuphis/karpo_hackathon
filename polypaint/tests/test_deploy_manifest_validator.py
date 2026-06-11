"""Unit tests for deploy_manifest.py's validator.

The validator is deploy's first gate (deploy.sh runs --check before touching
AWS), so each rule gets a direct flip test here: take the real manifest,
break one fact, and assert the precise error fires. The LD_LIBRARY_PATH rule
encodes the 2026-06 outage: bundle paths on LD_LIBRARY_PATH put staged
OpenSSL in front of the Lambda runtime python and broke `import ssl` at
INIT for every function carrying /var/task/lib.
"""
import copy
import json
import os
import sys
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
sys.path.insert(0, ROOT)

from deploy_manifest import load_manifest, validate


class TestDeployManifestValidator(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest = load_manifest()

    def _mutated(self, fn_key=None):
        manifest = copy.deepcopy(self.manifest)
        if fn_key is None:
            return manifest, None
        fn = next(f for f in manifest["functions"] if f["key"] == fn_key)
        return manifest, fn

    def test_real_manifest_is_valid(self):
        self.assertEqual(validate(self.manifest), [])

    def test_ld_library_path_must_only_reference_layer_paths(self):
        manifest, fn = self._mutated("raster_mt")
        fn["env"] += ",LD_LIBRARY_PATH=/var/task/lib"
        errors = validate(manifest)
        self.assertTrue(any("/var/task/lib" in e and "DT_RPATH" in e for e in errors), errors)

    def test_ld_library_path_layer_paths_are_allowed(self):
        manifest, fn = self._mutated("raster_mt")
        fn["env"] += ",LD_LIBRARY_PATH=/opt/lib"
        self.assertEqual(validate(manifest), [])

    def test_duplicate_route_ownership_rejected(self):
        manifest, fn = self._mutated("viewport")
        fn.setdefault("routes", []).append("/sweep-mt")
        errors = validate(manifest)
        self.assertTrue(any("already owned" in e for e in errors), errors)

    def test_undeclared_env_variable_rejected(self):
        manifest, fn = self._mutated("preview")
        fn["env"] += ",MYSTERY=$NOT_A_REAL_VAR"
        errors = validate(manifest)
        self.assertTrue(any("undeclared variable $NOT_A_REAL_VAR" in e for e in errors), errors)

    def test_unknown_layer_rejected(self):
        manifest, fn = self._mutated("preview")
        fn["layers"] = ["imaginary"]
        errors = validate(manifest)
        self.assertTrue(any("unknown layer" in e for e in errors), errors)

    def test_duplicate_name_rejected(self):
        manifest, fn = self._mutated("preview")
        fn["name"] = "polypaint-sweep-mt"
        errors = validate(manifest)
        self.assertTrue(any("duplicate name" in e for e in errors), errors)

    def test_removed_function_must_not_be_active(self):
        manifest, _ = self._mutated()
        manifest["removed"]["functions"].append("polypaint-sweep-mt")
        errors = validate(manifest)
        self.assertTrue(any("still in the active fleet" in e for e in errors), errors)


if __name__ == "__main__":
    unittest.main()
