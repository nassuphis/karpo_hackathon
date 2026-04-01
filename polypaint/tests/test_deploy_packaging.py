import ast
import pathlib
import re
import shlex
import unittest


ROOT = pathlib.Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"
DEPLOY_TEXT = (ROOT / "deploy.sh").read_text()
LOCAL_MODULES = {p.stem for p in LAMBDA_DIR.glob("*.py")}


def _joined_shell_lines(text):
    return re.sub(r"\\\n\s*", " ", text)


def _packaged_handlers():
    joined = _joined_shell_lines(DEPLOY_TEXT)
    files_by_dir = {}
    for line in joined.splitlines():
        stripped = line.strip()
        if not stripped.startswith("cp ") or ' "$' not in stripped:
            continue
        try:
            parts = shlex.split(stripped)
        except ValueError:
            continue
        if len(parts) < 3:
            continue
        dest = parts[-1]
        if not dest.startswith("$") or not dest.endswith("/"):
            continue
        dir_var = dest[1:-1]
        files_by_dir.setdefault(dir_var, set()).update(
            pathlib.Path(tok).name for tok in parts[1:-1] if tok.startswith("lambda/")
        )

    packaged = {}
    for _, files in files_by_dir.items():
        handler_files = [name for name in files if name.startswith("handler_") and name.endswith(".py")]
        for handler in handler_files:
            packaged[handler] = set(files)
    return packaged


def _local_dependencies(py_name, seen=None):
    if seen is None:
        seen = set()
    if py_name in seen:
        return set()
    seen.add(py_name)

    path = LAMBDA_DIR / py_name
    source = path.read_text()
    tree = ast.parse(source, filename=str(path))
    deps = set()

    for node in ast.walk(tree):
        if isinstance(node, ast.Import):
            for alias in node.names:
                name = alias.name.split(".")[0]
                if name in LOCAL_MODULES:
                    dep = f"{name}.py"
                    deps.add(dep)
                    deps.update(_local_dependencies(dep, seen))
        elif isinstance(node, ast.ImportFrom):
            if node.level:
                continue
            if not node.module:
                continue
            name = node.module.split(".")[0]
            if name in LOCAL_MODULES:
                dep = f"{name}.py"
                deps.add(dep)
                deps.update(_local_dependencies(dep, seen))

    for match in re.finditer(r'os\.path\.join\(\s*os\.path\.dirname\(__file__\)\s*,\s*[\'"]([^\'"]+)[\'"]\s*\)', source):
        deps.add(match.group(1))

    return deps


class TestDeployPackaging(unittest.TestCase):
    def test_packaged_handlers_include_all_local_dependencies(self):
        packaged = _packaged_handlers()
        self.assertGreater(len(packaged), 10, "failed to parse deploy packaging blocks")

        missing = []
        for handler, bundled_files in sorted(packaged.items()):
            required = _local_dependencies(handler)
            absent = sorted(req for req in required if req not in bundled_files)
            if absent:
                missing.append((handler, absent, sorted(bundled_files)))

        if missing:
            lines = []
            for handler, absent, bundled in missing:
                lines.append(
                    f"{handler} missing from deploy bundle: {', '.join(absent)} "
                    f"(bundled: {', '.join(bundled)})"
                )
            self.fail("\n".join(lines))

    def test_handlers_with_local_binaries_have_chmod_entries(self):
        packaged = _packaged_handlers()
        missing = []
        for handler, bundled_files in sorted(packaged.items()):
            dir_match = re.search(rf'cp .*?\b{re.escape(handler)}\b.*?"\$(\w+)/"', _joined_shell_lines(DEPLOY_TEXT))
            self.assertIsNotNone(dir_match, f"could not find package dir for {handler}")
            dir_var = dir_match.group(1)
            for dep in sorted(_local_dependencies(handler)):
                if dep.endswith((".py", ".json", ".html")):
                    continue
                chmod_pat = rf'chmod \+x .*"\${dir_var}"/{re.escape(dep)}'
                if not re.search(chmod_pat, _joined_shell_lines(DEPLOY_TEXT)):
                    missing.append(f"{handler} packages {dep} but deploy.sh has no chmod +x for ${dir_var}/{dep}")
        if missing:
            self.fail("\n".join(missing))

    def test_known_regressions_are_covered(self):
        packaged = _packaged_handlers()
        self.assertIn("handler_render_plan.py", packaged)
        self.assertIn("palette_names.py", packaged["handler_render_plan.py"])
        self.assertIn("tri_palette_names_generated.py", packaged["handler_render_plan.py"])
        self.assertIn("long_palette_names_generated.py", packaged["handler_render_plan.py"])

        self.assertIn("handler_solve_proximity.py", packaged)
        self.assertIn("solve_proximity_stats", packaged["handler_solve_proximity.py"])

        self.assertIn("handler_raster.py", packaged)
        self.assertIn("roots2pix", packaged["handler_raster.py"])


if __name__ == "__main__":
    unittest.main()
