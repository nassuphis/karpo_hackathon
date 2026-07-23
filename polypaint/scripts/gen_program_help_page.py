#!/usr/bin/env python3
"""Generate program-help.html — the standalone program-language reference.

Reading long-form documentation inside the app's narrow sidepanels is
painful (user, 2026-07-23); this emits a static, searchable HTML document
covering all four program languages (Param, Coeff, Root, Solve-Score),
built from the SAME data the in-app Starter/Help panels render — the
generated vocab files plus the hand-authored catalogs in js/07 + js/08 —
so it cannot drift from the app. Extraction happens in a node vm exactly
the way tests/test_frontend_js.sh's makeContext does.

Modes:
  (default)  regenerate program-help.html
  --check    regenerate to memory and fail if the committed page differs
             (predeploy-gated; requires node, like the frontend harness)
"""
from __future__ import annotations

import argparse
import html
import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PAGE_PATH = ROOT / "program-help.html"

LANGS = [
    ("param", "Param Program", "pp"),
    ("coeff", "Coeff Program", "cp"),
    ("root", "Root Program", "rt"),
    ("solve-score", "Solve-Score Program", "render-ss"),
]

_NODE_EXTRACT = r"""
const fs = require('fs'), path = require('path'), vm = require('vm');
const root = process.argv[2];
function makeEl(id) {
  return { id, innerHTML: '', textContent: '', value: '', hidden: false,
           style: {}, classList: { toggle() {}, add() {}, remove() {} },
           setAttribute() {}, addEventListener() {} };
}
const els = {};
const ctx = {
  console, Math, JSON, Number, String, Boolean, Array, Object, Map, Set, RegExp,
  document: { getElementById(id) { els[id] = els[id] || makeEl(id); return els[id]; }, addEventListener() {} },
  _escapeHtml(value) {
    return String(value == null ? '' : value)
      .replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;').replace(/'/g, '&#39;');
  },
};
ctx.window = ctx; ctx.globalThis = ctx;
vm.createContext(ctx);
const load = f => vm.runInContext(fs.readFileSync(path.join(root, f), 'utf8'), ctx, { filename: f });
load('param_vocab_js.js');
load('root_vocab_js.js');
load('solve_score_vocab_js.js');
// js/07's _ssCatalog IIFE calls these two js/02 helpers at load time; this
// context skips js/02, so mirror them (same vocab-driven logic — identical
// to tests/test_frontend_js.sh makeContext).
ctx._solveScoreMetricAllowedSources = (name) => {
  const fromVocab = ctx._solveScoreVocab.allowedSourcesByMetric[name];
  if (Array.isArray(fromVocab) && fromVocab.length) return fromVocab.slice();
  if ((ctx._solveScoreVocab.paramMetricNames || []).includes(name)) return ['pm'];
  if ((ctx._solveScoreVocab.paramCapableMetricNames || []).includes(name)) return ['slv', 'cf', 'pm'];
  return ['slv', 'cf'];
};
ctx._solveScoreMetricSourceChoices = (name) => {
  const base = ctx._solveScoreMetricAllowedSources(name);
  return base.concat(base.map(source => source + '-1'));
};
load('coeff_vocab_js.js');
load('coeff_func_catalog_js.js');
load('js/06-popup-init.js');
load('js/07-transform-catalogs.js');
load('js/08-chip-editors.js');

const inner = fs.readFileSync(process.argv[3], 'utf8');
process.stdout.write(String(vm.runInContext(inner, ctx, { filename: 'assemble.js' })));
"""

# The assembly below runs INSIDE the vm context (top-level `const`s from the
# js parts live in the context's lexical scope, not as ctx properties — they
# are only reachable by evaluating code in the context).
_NODE_ASSEMBLE = r"""
(function () {
const out = {};
const cheatsByLang = {
  param: _paramProgramCheatSections,
  coeff: _coeffProgramCheatSections,
  root: _rootProgramCheatSections,
};
// solve-score cheats are assembled dynamically — mirror
// _renderSolveScoreCheatsheet's data pass without the DOM.
const ssStarters = (_solveScoreStarterSnippets.length
  ? _solveScoreStarterSnippets : _solveScoreFallbackStarters)
  .map(item => ({ label: item.label, snippet: item.snippet, title: item.title || '' }));
const ssMetrics = _solveScoreMetricNames.map(name => ({
  label: name,
  snippet: _solveScoreMetricSnippet(name),
  title: (() => { const c = _solveScoreMetricAllowedSources(name).join('/'); return c ? c + '; q=0.1%' : ''; })(),
}));
const ssOutputs = Object.keys(_solveScoreOutputSpecs).map(name => {
  const expr = 'metric(proximity, slv, q=0.1%)';
  const spec = _solveScoreOutputSpecs[name] || {};
  const snippet = spec.snippet || (name === 'emit'
    ? `emit(${expr})`
    : (name === 'emit_none' ? `emit_none(${expr})\nemit_norm(${expr})` : `${name}(${expr})`));
  return { label: name, snippet, title: spec.tooltip || '' };
});
const ssUnary = Object.keys(_solveScoreUnarySpecs).map(name => ({
  label: name,
  snippet: _solveScoreUnarySnippet(name, _solveScoreUnarySpecs[name]),
  title: (_solveScoreUnarySpecs[name] || {}).tooltip || '',
}));
const ssCombine = Object.keys(_solveScoreCombineSpecs).map(name => ({
  label: name,
  snippet: _solveScoreCombineSnippet(name, _solveScoreCombineSpecs[name]),
  title: (_solveScoreCombineSpecs[name] || {}).tooltip || '',
}));
cheatsByLang['solve-score'] = [
  { title: 'Language', buttons: _solveScoreLanguageSnippets.map(i => ({ label: i.label, snippet: i.snippet, title: i.title || '' })) },
  { title: 'Starters', buttons: ssStarters },
  { title: 'Metrics', buttons: ssMetrics },
  { title: 'Outputs', buttons: ssOutputs },
  { title: 'Unary / Stack', buttons: ssUnary },
  { title: 'Combine', buttons: ssCombine },
];
const registryKey = { param: 'pp', coeff: 'cp', root: 'rt', 'solve-score': 'render-ss' };
for (const lang of ['param', 'coeff', 'root', 'solve-score']) {
  out[lang] = {
    cheats: (cheatsByLang[lang] || []).map(section => ({
      title: section.title,
      buttons: (section.buttons || []).map(b => ({
        label: String(b.label || ''), snippet: String(b.snippet || ''), title: String(b.title || ''),
      })),
    })),
    help: _programHelpRegistry(registryKey[lang]).sections.map(section => ({
      title: section.title,
      items: section.items.map(item => ({
        name: item.name, signature: item.signature, help: item.help,
        category: item.category, aliases: item.aliases || [],
        params: (item.params || []).map((p, i) => ({
          text: _programHelpParamText(p, i),
          detail: String((p && (p.help || p.title)) || ''),
        })),
        forms: item.forms || [], effect: item.effect || '',
        examples: item.examples || [], notes: item.notes || [],
      })),
    })),
  };
}
return JSON.stringify(out);
})()
"""


_PAGE_CSS = """
:root { color-scheme: dark; }
* { box-sizing: border-box; }
body { margin: 0; background: #10141c; color: #d7dce6;
       font: 14px/1.55 system-ui, -apple-system, 'Segoe UI', sans-serif; }
header { position: sticky; top: 0; background: #141a26; z-index: 5;
         border-bottom: 1px solid #2b3a5e; padding: 12px 20px;
         display: flex; gap: 14px; align-items: center; flex-wrap: wrap; }
header h1 { font-size: 16px; margin: 0 12px 0 0; color: #fff; }
header input { flex: 1 1 260px; max-width: 480px; background: #0d1118;
               color: #d7dce6; border: 1px solid #2b3a5e; border-radius: 6px;
               padding: 7px 10px; font-size: 14px; }
header nav a { color: #9ec1e8; margin-right: 10px; text-decoration: none; font-size: 13px; }
header nav a:hover { text-decoration: underline; }
#match-count { color: #8b93a7; font-size: 12px; }
main { max-width: 960px; margin: 0 auto; padding: 18px 20px 80px; }
h2 { color: #fff; border-bottom: 1px solid #2b3a5e; padding-bottom: 6px;
     margin: 40px 0 6px; font-size: 20px; }
h3 { color: #c8cdd6; margin: 26px 0 8px; font-size: 15px; }
.entry { border: 1px solid #222c42; border-radius: 8px; padding: 10px 14px;
         margin: 8px 0; background: #131926; }
.entry .sig { font-family: ui-monospace, 'SF Mono', Menlo, monospace;
              color: #8fd3a7; font-size: 13.5px; }
.entry .cat { color: #7f8aa5; font-size: 11.5px; text-transform: uppercase;
              letter-spacing: 0.4px; margin-top: 2px; }
.entry .desc { margin-top: 5px; color: #c3c9d6; }
.entry .meta { margin-top: 5px; color: #9aa3b8; font-size: 13px; }
.entry .meta strong { color: #c3c9d6; }
.entry code, .entry pre { font-family: ui-monospace, 'SF Mono', Menlo, monospace;
    background: #0d1118; border: 1px solid #222c42; border-radius: 5px; }
.entry code { padding: 1px 6px; font-size: 12.5px; }
.entry pre { padding: 8px 10px; margin: 6px 0 0; overflow-x: auto;
             font-size: 12.5px; line-height: 1.5; white-space: pre; }
.section-empty, .lang-empty { display: none !important; }
.hidden { display: none !important; }
footer { color: #58607a; font-size: 12px; text-align: center; padding: 24px; }
"""

_PAGE_JS = """
const input = document.getElementById('help-search');
const count = document.getElementById('match-count');
function applyFilter() {
  const q = input.value.trim().toLowerCase();
  let shown = 0, total = 0;
  document.querySelectorAll('.entry').forEach(el => {
    total += 1;
    const hit = !q || el.dataset.search.includes(q);
    el.classList.toggle('hidden', !hit);
    if (hit) shown += 1;
  });
  document.querySelectorAll('.help-section').forEach(sec => {
    sec.classList.toggle('section-empty',
      !sec.querySelector('.entry:not(.hidden)'));
  });
  document.querySelectorAll('.lang-block').forEach(block => {
    block.classList.toggle('lang-empty',
      !block.querySelector('.entry:not(.hidden)'));
  });
  count.textContent = q ? `${shown} of ${total} entries` : `${total} entries`;
}
input.addEventListener('input', applyFilter);
applyFilter();
"""


def _esc(text) -> str:
    return html.escape(str(text or ""), quote=True)


def _search_blob(*parts) -> str:
    return _esc(" ".join(str(p or "") for p in parts).lower())


def _cheat_entry_html(button) -> str:
    blob = _search_blob(button["label"], button["snippet"], button["title"])
    title = f'<div class="desc">{_esc(button["title"])}</div>' if button["title"] else ""
    return (f'<div class="entry" data-search="{blob}">'
            f'<div class="sig">{_esc(button["label"])}</div>{title}'
            f'<pre>{_esc(button["snippet"])}</pre></div>')


def _help_entry_html(item) -> str:
    blob = _search_blob(item["name"], item["signature"], item["help"],
                        item["category"], " ".join(item["aliases"]),
                        " ".join(item["forms"]), " ".join(item["examples"]))
    rows = [f'<div class="sig">{_esc(item["signature"])}</div>']
    if item["category"]:
        rows.append(f'<div class="cat">{_esc(item["category"])}</div>')
    if item["help"]:
        rows.append(f'<div class="desc">{_esc(item["help"])}</div>')
    if item["aliases"]:
        rows.append('<div class="meta"><strong>Aliases:</strong> '
                    + " ".join(f"<code>{_esc(a)}</code>" for a in item["aliases"]) + "</div>")
    if item["params"]:
        rows.append('<div class="meta"><strong>Params:</strong> '
                    + " ".join(f"<code>{_esc(p['text'])}</code>" for p in item["params"]) + "</div>")
        for p in item["params"]:
            if p["detail"]:
                rows.append(f'<div class="meta"><strong>{_esc(p["text"])}:</strong> {_esc(p["detail"])}</div>')
    if item["forms"]:
        rows.append('<div class="meta"><strong>Forms:</strong> '
                    + " ".join(f"<code>{_esc(f)}</code>" for f in item["forms"]) + "</div>")
    if item["effect"]:
        rows.append(f'<div class="meta"><strong>Effect:</strong> {_esc(item["effect"])}</div>')
    for ex in item["examples"]:
        rows.append(f"<pre>{_esc(ex)}</pre>")
    for note in item["notes"]:
        rows.append(f'<div class="meta">{_esc(note)}</div>')
    return f'<div class="entry" data-search="{blob}">' + "".join(rows) + "</div>"


def render_page(data) -> str:
    nav = " ".join(f'<a href="#{anchor}">{_esc(title)}</a>' for anchor, title, _ in LANGS)
    blocks = []
    for anchor, title, _key in LANGS:
        lang = data[anchor]
        parts = [f'<section class="lang-block" id="{anchor}"><h2>{_esc(title)}</h2>']
        for section in lang["cheats"]:
            entries = "".join(_cheat_entry_html(b) for b in section["buttons"])
            if entries:
                parts.append(f'<div class="help-section"><h3>Starter · {_esc(section["title"])}</h3>{entries}</div>')
        for section in lang["help"]:
            entries = "".join(_help_entry_html(i) for i in section["items"])
            if entries:
                parts.append(f'<div class="help-section"><h3>{_esc(section["title"])}</h3>{entries}</div>')
        parts.append("</section>")
        blocks.append("".join(parts))
    return f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>PolyPaint Program Reference</title>
<style>{_PAGE_CSS}</style>
</head>
<body>
<header>
  <h1>Program Reference</h1>
  <input id="help-search" type="search" placeholder="Search functions, snippets, metrics… (or use the browser's find)" autofocus>
  <span id="match-count"></span>
  <nav>{nav}</nav>
</header>
<main>
{"".join(blocks)}
</main>
<footer>Generated by scripts/gen_program_help_page.py from the same catalogs the in-app Starter/Help panels use — regenerate, never hand-edit.</footer>
<script>{_PAGE_JS}</script>
</body>
</html>
"""


def extract() -> dict:
    with tempfile.NamedTemporaryFile("w", suffix=".js", delete=False,
                                     encoding="utf-8") as fh:
        fh.write(_NODE_EXTRACT)
        script = fh.name
    with tempfile.NamedTemporaryFile("w", suffix=".js", delete=False,
                                     encoding="utf-8") as fh:
        fh.write(_NODE_ASSEMBLE)
        assemble = fh.name
    proc = subprocess.run(["node", script, str(ROOT), assemble],
                          capture_output=True, text=True, timeout=120)
    if proc.returncode != 0:
        raise SystemExit(f"FATAL: node extraction failed: {proc.stderr[:800]}")
    return json.loads(proc.stdout)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    page = render_page(extract())
    if args.check:
        if not PAGE_PATH.exists() or PAGE_PATH.read_text(encoding="utf-8") != page:
            print(f"STALE {PAGE_PATH} — run scripts/gen_program_help_page.py")
            return 1
        print(f"OK {PAGE_PATH}")
        return 0
    PAGE_PATH.write_text(page, encoding="utf-8")
    print(f"wrote {PAGE_PATH} ({PAGE_PATH.stat().st_size:,} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
