import json
import os
import re
import subprocess
import sys
import unittest


ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
LAMBDA_DIR = os.path.join(ROOT, "lambda")
sys.path.insert(0, LAMBDA_DIR)


HELP_FORMS_JS = r"""
const fs = require('fs'), vm = require('vm'), path = require('path');
const root = process.argv[1];
function makeEl(id) {
  return {
    id, textContent: '', value: '', selectionStart: 0, selectionEnd: 0,
    hidden: false, style: {}, _innerHTML: '',
    classList: { values: new Set(), toggle(n,on){ if(on)this.values.add(n); else this.values.delete(n); }, contains(n){ return this.values.has(n); } },
    setAttribute(n,v){ this[n]=v; },
    getBoundingClientRect(){ return {width: 280, height: 220}; },
    contains(node){ return node === this; },
    set innerHTML(v){ this._innerHTML = String(v || ''); },
    get innerHTML(){ return this._innerHTML; },
  };
}
const els = {};
const ctx = {
  console, window: {},
  document: { getElementById(id){ return els[id] || (els[id] = makeEl(id)); }, addEventListener(){} },
  Math, JSON, Number, String, Boolean, Array, Object, Map, Set, RegExp,
  _escapeHtml(value) {
    return String(value == null ? '' : value)
      .replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;')
      .replace(/\"/g,'&quot;').replace(/'/g,'&#39;');
  },
};
ctx.window = ctx;
ctx.globalThis = ctx;
vm.createContext(ctx);
for (const f of [
  'program_profiles_js.js',
  'param_vocab_js.js',
  'coeff_vocab_js.js',
  'coeff_func_catalog_js.js',
  'js/06-popup-init.js',
  'js/07-transform-catalogs.js',
  'js/08-chip-editors.js',
]) {
  vm.runInContext(fs.readFileSync(path.join(root, f), 'utf8'), ctx, {filename: f});
}
const result = vm.runInContext(`(() => {
  function forms(which) {
    return _programHelpRegistry(which).sections.flatMap(section =>
      (section.items || []).flatMap(item =>
        (item.forms || []).map(form => ({
          which, section: section.title, name: item.name, signature: item.signature, form
        }))
      )
    );
  }
  return {pp: forms('pp'), cp: forms('cp')};
})()`, ctx);
console.log(JSON.stringify(result));
"""


PLACEHOLDER_RE = re.compile(
    r"\b(expr|name|left|right|source|value|index|transform|andy|i)\b|"
    r"\.\.\.|\[|\]"
)


class TestProgramHelpForms(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        raw = subprocess.check_output(["node", "-e", HELP_FORMS_JS, ROOT], text=True)
        cls.forms = json.loads(raw)

    def test_concrete_param_help_forms_compile(self):
        from param_program_source import parse_param_program_source

        self._assert_forms_compile("pp", parse_param_program_source)

    def test_concrete_coeff_help_forms_compile(self):
        from coeff_program_source import parse_coeff_program_source

        self._assert_forms_compile("cp", parse_coeff_program_source)

    def _assert_forms_compile(self, which, parser):
        failures = []
        checked = 0
        for entry in self.forms[which]:
            form = str(entry.get("form") or "")
            if PLACEHOLDER_RE.search(form):
                continue
            checked += 1
            parsed = parser(form, strict=False)
            diagnostics = parsed.get("diagnostics") or []
            errors = [diag for diag in diagnostics if diag.get("level") == "error"]
            if errors:
                failures.append((entry, errors))
        self.assertGreater(checked, 20, f"too few concrete {which} help forms checked")
        if failures:
            lines = []
            for entry, errors in failures[:20]:
                code_msg = "; ".join(f"{err.get('code')}: {err.get('message')}" for err in errors)
                lines.append(f"{entry['which']} {entry['section']} {entry['name']} form={entry['form']!r}: {code_msg}")
            self.fail("\n".join(lines))


if __name__ == "__main__":
    unittest.main()
