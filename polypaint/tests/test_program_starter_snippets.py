"""Every Starter-tab snippet is one click from a user's program: gate that
each one compiles through the real parser for its language, the same
discipline test_program_help_forms applies to Help `forms:` lines.

Tables are extracted from js/08-chip-editors.js by bracket-matching the
const declarations (pure literals by construction — the solve-score tables
were restructured into consts specifically so this gate could exist).
"""
import json
import os
import subprocess
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

ROOT = os.path.join(os.path.dirname(__file__), "..")

EXTRACT_JS = r"""
const fs = require('fs');
const path = require('path');
const vm = require('vm');
// Dynamic rows (registry-driven button builders) evaluate to [] via a
// stub-everything context; this gate targets the static snippet strings.
const stubCtx = vm.createContext(new Proxy({}, {
  get: (t, k) => (typeof k === 'symbol' ? undefined : (() => [])),
  has: () => true,
}));
const src = fs.readFileSync(path.join(process.argv[1], 'js', '08-chip-editors.js'), 'utf8');
function grabConst(name) {
  const at = src.indexOf(`const ${name} = [`);
  if (at < 0) { console.error('missing ' + name); process.exit(1); }
  const i = src.indexOf('[', at);
  let depth = 0;
  let quote = null;
  for (let j = i; j < src.length; j++) {
    const ch = src[j];
    if (quote) {
      if (ch === '\\') { j++; continue; }
      if (ch === quote) quote = null;
      continue;
    }
    if (ch === "'" || ch === '"' || ch === '`') { quote = ch; continue; }
    if (ch === '[') depth++;
    else if (ch === ']') {
      depth--;
      if (depth === 0) return vm.runInContext('(' + src.slice(i, j + 1) + ')', stubCtx);
    }
  }
  console.error('unterminated ' + name); process.exit(1);
}
const out = {
  param: grabConst('_paramProgramCheatSections'),
  coeff: grabConst('_coeffProgramCheatSections'),
  root: grabConst('_rootProgramCheatSections'),
  ssStarters: grabConst('_solveScoreFallbackStarters'),
  ssLanguage: grabConst('_solveScoreLanguageSnippets'),
};
console.log(JSON.stringify(out));
"""


def _snippets(sections):
    for section in sections:
        for item in section.get("buttons") or []:
            yield section.get("title", "?"), item.get("label", "?"), str(item.get("snippet") or "")


class TestStarterSnippetsCompile(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        raw = subprocess.check_output(["node", "-e", EXTRACT_JS, ROOT], text=True)
        cls.tables = json.loads(raw)

    def _assert_all_compile(self, entries, parser, kind, min_expected):
        failures = []
        checked = 0
        for section, label, snippet in entries:
            if not snippet.strip():
                continue
            checked += 1
            parsed = parser(snippet, strict=False)
            errors = [d for d in (parsed.get("diagnostics") or []) if d.get("level") == "error"]
            if errors:
                msg = "; ".join(f"{e.get('code')}: {e.get('message')}" for e in errors)
                failures.append(f"{kind} [{section}] {label}: {snippet!r}: {msg}")
        self.assertGreaterEqual(checked, min_expected, f"too few {kind} snippets checked ({checked})")
        if failures:
            self.fail("\n".join(failures[:20]))

    def test_param_starter_snippets_compile(self):
        from param_program_source import parse_param_program_source

        self._assert_all_compile(
            _snippets(self.tables["param"]), parse_param_program_source, "param", 10)

    def test_coeff_starter_snippets_compile(self):
        from coeff_program_source import parse_coeff_program_source

        self._assert_all_compile(
            _snippets(self.tables["coeff"]), parse_coeff_program_source, "coeff", 15)

    def test_root_starter_snippets_compile(self):
        from root_program_source import parse_root_program_source

        self._assert_all_compile(
            _snippets(self.tables["root"]), parse_root_program_source, "root", 3)

    def test_solve_score_snippets_compile(self):
        from solve_score_program_source import parse_solve_score_program_source

        entries = [("Starters", i.get("label", "?"), str(i.get("snippet") or ""))
                   for i in self.tables["ssStarters"]]
        entries += [("Language", i.get("label", "?"), str(i.get("snippet") or ""))
                    for i in self.tables["ssLanguage"]]

        def parser(text, strict=False):
            return parse_solve_score_program_source(text, strict=strict)

        self._assert_all_compile(entries, parser, "solve-score", 5)

    def test_root_registry_help_forms_compile(self):
        """The root Help tab renders name(defaults...) for every registry
        transform (mirroring _rootTransformHelpItem); each must compile."""
        from root_program_source import _registry_by_name, parse_root_program_source

        failures = []
        checked = 0
        for name, spec in sorted(_registry_by_name().items()):
            params = (spec.get("ui") or {}).get("params") or []
            defaults = []
            for idx, param in enumerate(params):
                default = param.get("def")
                defaults.append(str(default) if default is not None else "0.5")
            form = f"{name}({', '.join(defaults)})"
            checked += 1
            parsed = parse_root_program_source(form, strict=False)
            errors = [d for d in (parsed.get("diagnostics") or []) if d.get("level") == "error"]
            if errors:
                msg = "; ".join(f"{e.get('code')}: {e.get('message')}" for e in errors)
                failures.append(f"root help form {form!r}: {msg}")
        self.assertGreaterEqual(checked, 5)
        if failures:
            self.fail("\n".join(failures[:20]))


if __name__ == "__main__":
    unittest.main()


class TestEditorPlaceholdersCompile(unittest.TestCase):
    """The textarea placeholder="Example:..." texts are the first code a
    user ever sees; each must compile with its editor's parser."""

    def test_placeholders_compile(self):
        import re

        from coeff_program_source import parse_coeff_program_source
        from param_program_source import parse_param_program_source
        from root_program_source import parse_root_program_source
        from solve_score_program_source import parse_solve_score_program_source

        html = open(os.path.join(ROOT, "index.html"), encoding="utf-8").read()
        parsers = {
            "rt-source-text": parse_root_program_source,
            "pp-source-text": parse_param_program_source,
            "cp-source-text": parse_coeff_program_source,
            "ss-source-text": parse_solve_score_program_source,
        }
        found = 0
        failures = []
        for m in re.finditer(
            r'id="([^"]*(?:rt|pp|cp|ss)-source-text)"[^>]*placeholder="Example:&#10;([^"]+)"', html
        ):
            ident, raw = m.group(1), m.group(2).replace("&#10;", "\n")
            parser = next(p for suffix, p in parsers.items() if ident.endswith(suffix))
            found += 1
            parsed = parser(raw, strict=False)
            errors = [d for d in (parsed.get("diagnostics") or []) if d.get("level") == "error"]
            if errors:
                failures.append(f"{ident}: {raw!r}: {errors[0].get('message')}")
        self.assertGreaterEqual(found, 6, f"expected all six editor placeholders, found {found}")
        if failures:
            self.fail("\n".join(failures))
