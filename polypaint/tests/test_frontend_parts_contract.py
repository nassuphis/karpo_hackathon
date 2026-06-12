"""Structural contract for the split frontend parts (js/*.js).

Classic scripts hoist per-file only, so top-level *executable* statements in
one part must not reference functions defined in a later part — the CR12
load-order blocker. The runtime sequential-load gate (tests/test_frontend_js.sh)
catches everything that executes under its DOM stub, but a top-level
statement behind a condition could slip past it. This test closes that gap
structurally: every part may contain only declarations at top level;
executable statements are allowed solely inside the marked boot block at the
end of js/12 (which runs after every part is parsed) plus each part's
__ppParts registration line.

The scanner is template-literal-aware (multi-line HTML strings contain
column-zero lines that are not top-level code).
"""
import pathlib
import re
import unittest

ROOT = pathlib.Path(__file__).resolve().parent.parent
INDEX = ROOT / "index.html"

BOOT_START = "/* ---- Boot sequence ----"
REGISTRATION = re.compile(r"^;\(window\.__ppParts = window\.__ppParts \|\| \[\]\)\.push\('[^']+'\);$")

# A column-zero line is a declaration (or part of one) if it starts with one
# of these. Anything else at top level is an executable statement.
DECLARATION_STARTS = (
    "function ", "async function ", "const ", "let ", "var ", "class ",
    "// ", "//", "/* ", "/*", " *", "*/",
)
# Closers / continuations that legitimately sit at column zero inside a
# multi-line top-level declaration.
CLOSER_RE = re.compile(r"^[}\])].*$")


def _strip_non_code(source: str) -> str:
    """Blank out comments, strings, and template literals (preserving every
    newline) so column-zero classification only sees real code. Template
    interpolations are tracked with a mode stack so `${ ... }` nesting and
    the code after a template are classified correctly."""
    out = []
    i, n = 0, len(source)
    stack = ["code"]          # code | line | block | s1 | s2 | tpl
    braces = []               # open-brace depth per code frame entered from ${

    def emit(ch):
        out.append("\n" if ch == "\n" else " ")

    while i < n:
        mode = stack[-1]
        ch = source[i]
        nxt = source[i + 1] if i + 1 < n else ""
        if mode == "code":
            if ch == "/" and nxt == "/":
                stack.append("line"); emit(ch); emit(nxt); i += 2; continue
            if ch == "/" and nxt == "*":
                stack.append("block"); emit(ch); emit(nxt); i += 2; continue
            if ch == "'":
                stack.append("s1"); emit(ch); i += 1; continue
            if ch == '"':
                stack.append("s2"); emit(ch); i += 1; continue
            if ch == "`":
                stack.append("tpl"); emit(ch); i += 1; continue
            if ch == "{" and braces:
                braces[-1] += 1
            if ch == "}" and braces:
                if braces[-1] == 0:
                    # closes a ${ interpolation: back to the template
                    braces.pop(); stack.pop()
                    emit(ch); i += 1; continue
                braces[-1] -= 1
            out.append(ch); i += 1; continue
        if mode == "line":
            if ch == "\n":
                stack.pop()
            emit(ch); i += 1; continue
        if mode == "block":
            if ch == "*" and nxt == "/":
                stack.pop(); emit(ch); emit(nxt); i += 2; continue
            emit(ch); i += 1; continue
        if mode in ("s1", "s2"):
            quote = "'" if mode == "s1" else '"'
            if ch == "\\":
                emit(ch); emit(nxt); i += 2; continue
            if ch == quote or ch == "\n":
                stack.pop()
            emit(ch); i += 1; continue
        if mode == "tpl":
            if ch == "\\":
                emit(ch); emit(nxt); i += 2; continue
            if ch == "$" and nxt == "{":
                stack.append("code"); braces.append(0)
                emit(ch); emit(nxt); i += 2; continue
            if ch == "`":
                stack.pop()
            emit(ch); i += 1; continue
    return "".join(out)


def _part_names():
    html = INDEX.read_text()
    return re.findall(r'<script src="js/([^"?]+\.js)"></script>', html)


class TestFrontendPartsContract(unittest.TestCase):
    def test_tags_match_disk(self):
        tags = _part_names()
        disk = sorted(p.name for p in (ROOT / "js").glob("*.js"))
        self.assertEqual(sorted(tags), disk)
        self.assertEqual(len(tags), len(set(tags)))

    def test_parts_are_declaration_only_outside_boot(self):
        violations = []
        for name in _part_names():
            raw = (ROOT / "js" / name).read_text()
            raw_lines = raw.split("\n")
            code_lines = _strip_non_code(raw).split("\n")
            # guard the guard: a stripper bug that eats newlines would let
            # zip() silently skip trailing lines
            self.assertEqual(len(raw_lines), len(code_lines), name)
            boot_line = None
            for idx, rl in enumerate(raw_lines):
                if rl.startswith(BOOT_START):
                    boot_line = idx
                    break
            for lineno, (raw_line, line) in enumerate(
                    zip(raw_lines, code_lines), start=1):
                if boot_line is not None and lineno > boot_line:
                    continue  # the js/12 boot block may execute statements
                stripped = line.rstrip()
                if not stripped or stripped[0] in " \t":
                    continue
                if REGISTRATION.match(raw_line.strip()):
                    continue
                if stripped.startswith(DECLARATION_STARTS):
                    continue
                if CLOSER_RE.match(stripped):
                    continue
                violations.append(f"{name}:{lineno}: {raw_line.strip()[:90]}")
        self.assertEqual(violations, [], (
            "top-level executable statements outside the js/12 boot block —\n"
            "classic scripts hoist per-file only, so these can forward-reference\n"
            "later parts (the CR12 load-order class). Move calls into the boot\n"
            "block or convert to a declaration:\n" + "\n".join(violations[:20])
        ))


if __name__ == "__main__":
    unittest.main()
