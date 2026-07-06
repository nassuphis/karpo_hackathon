#!/usr/bin/env python3
"""Port poly100.py coefficient functions to saved Coeff Programs.

Recognizes the elementwise poly100 template:

    cf = np.zeros(N, dtype=...)
    cf[<int>] = <scalar expr>                 # sparse prelude (no cf reads)
    for k in range(A, B):
        tmp = <expr>                          # loop temps, inlined
        cf[k + OFF] = <expr in k, t1, t2>     # single write target
    cf[<int>] = <scalar expr, may read cf>    # post-loop fixups
    return cf...

and emits readable RPN-style Coeff Program source: the loop becomes vector
statements over range(...) (one additive term per line, commented with the
numpy fragment it implements), combined on the stack with add(pop, pop);
prelude/fixup assignments become scalar poly[i] = ... statements.

Every port is compiled with the real compiler and parity-checked against the
verbatim Python function through the native sweep_test binary before upload.
Emission rules the compiler/VM impose (discovered empirically):
  - `pop` in an argument slot must pop a VECTOR; scalar values fold inline.
  - args lower left-to-right, so the leftmost `pop` binds top-of-stack;
    generated code only combines with commutative add(pop, pop) + neg().
  - a scalar arg that starts with `name(` but continues with operators would
    mis-parse as a call; wrapping scalars in parentheses avoids it.
  - scalar `**` requires an integer literal exponent <= 32; anything else
    goes through power(a, b).

Usage:
    uv run python lambda/port_poly100_programs.py --list
    uv run python lambda/port_poly100_programs.py --only poly_3 --dry-run
    uv run python lambda/port_poly100_programs.py            # batch + upload
"""

import argparse
import ast
import cmath
import json
import os
import struct
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import numpy as np

from coeff_program_source import compile_coeff_program_source

LAMBDA_DIR = os.path.dirname(os.path.abspath(__file__))
POLY100 = os.path.join(LAMBDA_DIR, "poly100.py")  # default --module
_MODULE_LABEL = {"label": "poly100.py"}
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")

UNARY_FUNCS = {
    "sin", "cos", "tan", "sinh", "cosh", "tanh",
    "exp", "log", "sqrt", "abs", "real", "imag", "conj", "angle",
}
NP_UNARY_ALIASES = {"conjugate": "conj", "absolute": "abs"}

# Parity points: unit circle (typical param-program output) plus generic
# inside/outside-disk values including negative-real branch-cut cases.
PARITY_POINTS = (
    [(cmath.exp(2j * cmath.pi * a), cmath.exp(2j * cmath.pi * b))
     for a, b in [(0.13, 0.71), (0.5, 0.25), (0.9, 0.05),
                  (0.33, 0.66), (0.01, 0.99), (0.618, 0.382)]]
    + [(0.4 + 0.2j, -0.3 + 0.8j), (-0.9 - 0.4j, 0.7 - 0.6j),
       (1.04 + 0.0j, 0.0 + 1.02j), (-1.0 + 0.0j, -0.5 - 0.5j)]
)
PARITY_REL_TOL = 5e-5     # f32 wire precision with headroom
PARITY_MIN_POINTS = 5     # points that must survive the f32-range filter
F32_MAX = 3.0e38


class SkipFunction(Exception):
    """Raise with a reason when a function does not fit the template."""


# ---------------------------------------------------------------------------
# AST extraction
# ---------------------------------------------------------------------------

def load_functions(module_path=None):
    src = open(module_path or POLY100, "r", encoding="utf-8").read()
    tree = ast.parse(src)
    out = {}
    for node in tree.body:
        if isinstance(node, ast.FunctionDef) and node.name.startswith("poly_"):
            out[node.name] = (node, ast.get_source_segment(src, node))
    return src, out


class _StripAstype(ast.NodeTransformer):
    """Drop .astype(...) calls — every use in poly100/poly200 is a complex
    cast, a no-op in the VM's all-complex arithmetic."""

    def visit_Call(self, node):
        self.generic_visit(node)
        if isinstance(node.func, ast.Attribute) and node.func.attr == "astype":
            return node.func.value
        return node

    def visit_Attribute(self, node):
        self.generic_visit(node)
        if (isinstance(node.value, ast.Name) and node.value.id == "np"
                and node.attr == "pi" and isinstance(node.ctx, ast.Load)):
            return ast.copy_location(ast.Name(id="pi", ctx=ast.Load()), node)
        return node


def _unwrap_try(fn):
    body = fn.body
    if len(body) == 1 and isinstance(body[0], ast.Try):
        body = body[0].body
    return [ast.fix_missing_locations(_StripAstype().visit(stmt)) for stmt in body]


def _first_arange_bounds(node):
    for sub in ast.walk(node):
        if (isinstance(sub, ast.Call) and isinstance(sub.func, ast.Attribute)
                and isinstance(sub.func.value, ast.Name) and sub.func.value.id == "np"
                and sub.func.attr == "arange"):
            cargs = [_const_int(a) for a in sub.args]
            if len(cargs) in (1, 2) and all(a is not None for a in cargs):
                return (0, cargs[0]) if len(cargs) == 1 else tuple(cargs)
    return None


class _InlineTemps(ast.NodeTransformer):
    def __init__(self, env):
        self.env = env

    def visit_Name(self, node):
        if isinstance(node.ctx, ast.Load) and node.id in self.env:
            return ast.copy_location(self.env[node.id], node)
        return node


def _const_int(node):
    if isinstance(node, ast.Constant) and isinstance(node.value, int):
        return node.value
    return None


def _linear_k(idx, kvar):
    """Fold an index expression to (a, b) meaning a*kvar + b with a in
    {-1, 0, 1}; None when it is not that linear form."""
    if isinstance(idx, ast.Constant) and isinstance(idx.value, int):
        return (0, idx.value)
    if isinstance(idx, ast.Name):
        return (1, 0) if idx.id == kvar else None
    if isinstance(idx, ast.UnaryOp) and isinstance(idx.op, ast.USub):
        inner = _linear_k(idx.operand, kvar)
        return None if inner is None else (-inner[0], -inner[1])
    if isinstance(idx, ast.BinOp) and isinstance(idx.op, (ast.Add, ast.Sub)):
        left = _linear_k(idx.left, kvar)
        right = _linear_k(idx.right, kvar)
        if left is None or right is None:
            return None
        sign = 1 if isinstance(idx.op, ast.Add) else -1
        a = left[0] + sign * right[0]
        if a not in (-1, 0, 1):
            return None
        return (a, left[1] + sign * right[1])
    return None


def _slot_of_target(node, kvar=None):
    """cf[<int>] -> ('const', i); cf[±k + c] -> ('loop', c) / ('desc', c)."""
    if not (isinstance(node, ast.Subscript) and isinstance(node.value, ast.Name)
            and node.value.id == "cf"):
        return None
    idx = node.slice
    c = _const_int(idx)
    if c is not None:
        return ("const", c)
    linear = _linear_k(idx, kvar or "\x00")
    if linear is None:
        return None
    a, b = linear
    if a == 0:
        return ("const", b)   # constant index spelled as arithmetic (36 - 1)
    if kvar is None:
        return None
    return ("loop", b) if a == 1 else ("desc", b)


def _strided_target(node):
    """cf[a::s], cf[a:b:s], cf[np.arange(a, b, s)] with const ints, s >= 2."""
    if not (isinstance(node, ast.Subscript) and isinstance(node.value, ast.Name)
            and node.value.id == "cf"):
        return None
    sl = node.slice
    if isinstance(sl, ast.Slice):
        if sl.step is None:
            return None
        step = _const_int(sl.step)
        lo = 0 if sl.lower is None else _const_int(sl.lower)
        hi = None if sl.upper is None else _const_int(sl.upper)
        if step is None or lo is None or (sl.upper is not None and hi is None):
            return None
        return (lo, hi, step) if step >= 2 else None
    if (isinstance(sl, ast.Call) and isinstance(sl.func, ast.Attribute)
            and isinstance(sl.func.value, ast.Name) and sl.func.value.id == "np"
            and sl.func.attr == "arange" and len(sl.args) == 3):
        cargs = [_const_int(a) for a in sl.args]
        if any(a is None for a in cargs):
            return None
        a, b, step = cargs
        return (a, b, step) if step >= 2 else None
    return None


def _slice_bounds(node):
    """cf[a:b] target -> (a, b) with constant bounds, else None."""
    if not (isinstance(node, ast.Subscript) and isinstance(node.value, ast.Name)
            and node.value.id == "cf" and isinstance(node.slice, ast.Slice)):
        return None
    sl = node.slice
    if sl.step is not None:
        return None
    lo = 0 if sl.lower is None else _const_int(sl.lower)
    hi = _const_int(sl.upper) if sl.upper is not None else None
    if lo is None or hi is None:
        return None
    return lo, hi


_ARANGE_KVAR = "__arange__"


class _ArangeToK(ast.NodeTransformer):
    """Replace np.arange(c, d) calls with a synthetic loop variable name.

    The first arange fixes the base bounds; later aranges of the SAME length
    become k + (c2 - c1) since both advance one per slot. Different lengths
    set self.mismatch.
    """

    def __init__(self):
        self.bounds = None
        self.mismatch = False

    def visit_Call(self, node):
        self.generic_visit(node)
        fn = node.func
        is_arange = (isinstance(fn, ast.Attribute) and isinstance(fn.value, ast.Name)
                     and fn.value.id == "np" and fn.attr == "arange")
        if not is_arange:
            return node
        cargs = [_const_int(a) for a in node.args]
        if any(a is None for a in cargs) or len(cargs) not in (1, 2):
            self.mismatch = True
            return node
        bounds = (0, cargs[0]) if len(cargs) == 1 else tuple(cargs)
        k_name = ast.Name(id=_ARANGE_KVAR, ctx=ast.Load())
        if self.bounds is None:
            self.bounds = bounds
            return ast.copy_location(k_name, node)
        if bounds == self.bounds:
            return ast.copy_location(k_name, node)
        if bounds[1] - bounds[0] != self.bounds[1] - self.bounds[0]:
            self.mismatch = True
            return node
        delta = bounds[0] - self.bounds[0]
        shifted = ast.BinOp(left=k_name, op=ast.Add(), right=ast.Constant(value=delta))
        return ast.copy_location(ast.fix_missing_locations(shifted), node)


class _SubstituteK(ast.NodeTransformer):
    """Replace loop-var reads with (c - k): rewrites a descending-slot body
    expression into slot-index terms."""

    def __init__(self, kvar, c):
        self.kvar = kvar
        self.c = c

    def visit_Name(self, node):
        if isinstance(node.ctx, ast.Load) and node.id == self.kvar:
            flipped = ast.BinOp(left=ast.Constant(value=self.c), op=ast.Sub(),
                                right=ast.Name(id=self.kvar, ctx=ast.Load()))
            return ast.copy_location(ast.fix_missing_locations(flipped), node)
        return node


class _ReplaceMatchingLoad(ast.NodeTransformer):
    """Replace Load-context nodes whose dump matches `pattern_dump` with a
    deep copy of `replacement` (used to inline cf[i] rewrite chains)."""

    def __init__(self, pattern_dump, replacement):
        self.pattern_dump = pattern_dump
        self.replacement = replacement

    def visit_Subscript(self, node):
        self.generic_visit(node)
        if isinstance(node.ctx, ast.Load) and _load_dump(node) == self.pattern_dump:
            import copy as _copy
            return _copy.deepcopy(self.replacement)
        return node


def _load_dump(node):
    """ast.dump with the ctx normalized so Store/Load targets compare equal."""
    import copy as _copy
    node = _copy.deepcopy(node)
    for sub in ast.walk(node):
        if hasattr(sub, "ctx"):
            sub.ctx = ast.Load()
    return ast.dump(node)


def _collapse_same_slot_rewrites(body_stmts):
    """cf[i] = e1; cf[i] = e2(cf[i]) ...  ->  cf[i] = e2(e1)  (chain-inline).

    AugAssign on the same slot normalizes to Assign first. Non-consecutive or
    different-target writes are left alone.
    """
    out = []
    for st in body_stmts:
        if (isinstance(st, ast.AugAssign) and isinstance(st.target, ast.Subscript)
                and out and isinstance(out[-1], ast.Assign) and len(out[-1].targets) == 1
                and _load_dump(st.target) == _load_dump(out[-1].targets[0])):
            read = ast.copy_location(ast.Subscript(
                value=st.target.value, slice=st.target.slice, ctx=ast.Load()), st)
            st = ast.copy_location(ast.Assign(
                targets=[st.target],
                value=ast.BinOp(left=read, op=st.op, right=st.value)), st)
            st = ast.fix_missing_locations(st)
        if (isinstance(st, ast.Assign) and len(st.targets) == 1
                and isinstance(st.targets[0], ast.Subscript)
                and out and isinstance(out[-1], ast.Assign) and len(out[-1].targets) == 1
                and _load_dump(st.targets[0]) == _load_dump(out[-1].targets[0])):
            inlined = _ReplaceMatchingLoad(
                _load_dump(st.targets[0]), out[-1].value).visit(st.value)
            out[-1] = ast.fix_missing_locations(ast.copy_location(
                ast.Assign(targets=st.targets, value=inlined), st))
            continue
        out.append(st)
    return out


def _reads_cf(node):
    return any(isinstance(n, ast.Name) and n.id == "cf" and isinstance(n.ctx, ast.Load)
               for n in ast.walk(node))


def _normalize_slice_assign(tgt, value):
    """cf[a:b] = <expr over np.arange(c, d)> -> segment (kvar, c, d, off, expr).

    Loop-equivalent: slot s in [a, b) takes the expr at arange value c+(s-a).
    """
    bounds = _slice_bounds(tgt)
    if bounds is None:
        return None
    if _reads_cf(value):
        raise SkipFunction("slice expression reads cf")
    a, b = bounds
    xform = _ArangeToK()
    value_k = ast.fix_missing_locations(xform.visit(value))
    if xform.mismatch:
        raise SkipFunction("slice mixes different np.arange bounds")
    if xform.bounds is None:
        c, d = 0, b - a  # scalar broadcast; synthetic k unused
    else:
        c, d = xform.bounds
    if d - c != b - a:
        raise SkipFunction(f"slice [{a}:{b}) length != arange({c}, {d})")
    return (_ARANGE_KVAR, c, d, a - c, value_k)


class _ScanRewrite(ast.NodeTransformer):
    """Rewrite cf[k+off-1] -> __prev__, cf[k+off-2] -> __prev2__, k ->
    __scan_k__. Constant-slot cf reads OUTSIDE the scan's write span stay
    as-is (scalar_src renders them poly[c]; the scan buffers its output in
    scratch, so poly is constant while it runs — matching python, where the
    loop never writes those slots)."""

    def __init__(self, kvar, write_off, span, seeds=2):
        self.kvar = kvar
        self.write_off = write_off
        self.span = span  # (slot_lo, slot_hi) the scan will cover
        self.seeds = seeds
        self.used_prev2 = False
        self.bad_read = False

    def visit_Subscript(self, node):
        # Classify against the pristine index BEFORE renaming k inside it.
        if isinstance(node.value, ast.Name) and node.value.id == "cf":
            slot = _slot_of_target(node, self.kvar)
            if slot == ("loop", self.write_off - 1):
                return ast.copy_location(ast.Name(id="__prev__", ctx=ast.Load()), node)
            if slot == ("loop", self.write_off - 2):
                self.used_prev2 = True
                return ast.copy_location(ast.Name(id="__prev2__", ctx=ast.Load()), node)
            if slot and slot[0] == "const":
                inside = self.span[0] <= slot[1] < self.span[1]
                if not inside or slot[1] < self.span[0] + self.seeds:
                    # outside the span, or a seed slot (poked before the
                    # scan and re-emitted unchanged): reads poly[c]
                    return node
                self.bad_read = True
                return node
            self.bad_read = True
            return node
        self.generic_visit(node)
        return node

    def visit_Name(self, node):
        if node.id == self.kvar and isinstance(node.ctx, ast.Load):
            return ast.copy_location(ast.Name(id="__scan_k__", ctx=node.ctx), node)
        return node


def _branch_value(body, kvar, env, incoming):
    """One-branch value for the loop slot: Assign replaces, AugAssign(+/-/*)
    combines with `incoming`. Leading temp assigns (k = j + 3) fold into a
    branch-local env. Returns (off, target_node, value) or None."""
    env = dict(env)
    while (len(body) > 1 and isinstance(body[0], ast.Assign)
           and len(body[0].targets) == 1 and isinstance(body[0].targets[0], ast.Name)):
        env[body[0].targets[0].id] = _InlineTemps(env).visit(body[0].value)
        body = body[1:]
    if len(body) != 1:
        return None
    stmt = body[0]
    if isinstance(stmt, ast.Assign) and len(stmt.targets) == 1:
        tgt = stmt.targets[0]
        slot = _slot_of_target(tgt, kvar)
        if not slot or slot[0] != "loop":
            return None
        return slot[1], tgt, _InlineTemps(env).visit(stmt.value)
    if isinstance(stmt, ast.AugAssign) and isinstance(stmt.op, (ast.Add, ast.Sub, ast.Mult)):
        slot = _slot_of_target(stmt.target, kvar)
        if not slot or slot[0] != "loop" or incoming is None:
            return None
        value = ast.BinOp(left=incoming, op=stmt.op,
                          right=_InlineTemps(env).visit(stmt.value))
        return slot[1], stmt.target, ast.fix_missing_locations(value)
    if isinstance(stmt, ast.If):
        folded = _fold_if_to_select(stmt, kvar, env, incoming)
        if folded is None:
            return None
        tgt = folded.targets[0]
        slot = _slot_of_target(tgt, kvar)
        return slot[1], tgt, folded.value
    return None


def _fold_if_to_select(node, kvar, env, incoming=None):
    """if/elif/else branches writing (or augmenting) ONE loop slot fold to
    cf[slot] = select(cond, A, B). A missing else keeps `incoming`."""
    then = _branch_value(node.body, kvar, env, incoming)
    if then is None:
        return None
    if node.orelse:
        other = _branch_value(node.orelse, kvar, env, incoming)
        if other is None or other[0] != then[0]:
            return None
        else_value = other[2]
    elif incoming is not None:
        else_value = incoming
    else:
        return None
    cond = _InlineTemps(env).visit(node.test)
    sel = ast.Call(func=ast.Name(id="__select__", ctx=ast.Load()),
                   args=[cond, then[2], else_value], keywords=[])
    out = ast.Assign(targets=[then[1]], value=sel)
    return ast.copy_location(ast.fix_missing_locations(out), node)


def _append_poke(ordered, slot, expr):
    """Record a poke, hoisting a single vector reduction to the stack."""
    extractor = _ExtractReduction()
    rewritten = ast.fix_missing_locations(extractor.visit(expr))
    if extractor.found:
        fname, arg = extractor.found
        ordered.append(("reduce_push", fname, arg))
        ordered.append(("poke", slot, rewritten))
        ordered.append(("drop",))
    else:
        ordered.append(("poke", slot, expr))


def _slot_written_before(ordered, segments, slot):
    """True when an earlier poke, segment, or scan covers `slot`."""
    for op in ordered:
        if op[0] == "poke" and op[1] == slot:
            return True
        if op[0] == "scan" and op[1] <= slot < op[2]:
            return True
        if op[0] == "segment":
            seg = segments[op[1]]
            _kvar, lo, hi, off, _expr = seg
            if lo + off <= slot < hi + off:
                return True
    return False


def _try_scan_template(kvar, lo, hi, off, value, ordered):
    """Recognize cf[k+off] = f(cf[k+off-1] [, cf[k+off-2]], k) as a scan.

    Seed slots must have been poked before the loop; those pokes become the
    scan's init expressions and are consumed. Distance-2 recurrences use the
    five-arg form with two seeds. Returns a ("scan", slot_lo, slot_hi,
    length, k0, [init_texts...], step_text) entry.
    """
    import copy
    # Pass 1 (narrow span) learns whether prev2 is used; pass 2 re-runs with
    # the true span so constant-slot reads classify against real bounds.
    probe = _ScanRewrite(kvar, off, (lo + off - 1, hi + off))
    probe.visit(copy.deepcopy(value))
    seeds = 2 if probe.used_prev2 else 1
    slot_lo = lo + off - seeds
    if slot_lo < 0:
        return None
    rewriter = _ScanRewrite(kvar, off, (slot_lo, hi + off), seeds=seeds)
    step_node = ast.fix_missing_locations(rewriter.visit(value))
    if rewriter.bad_read:
        return None
    # Seeds read poly[slot]: their pokes (or an earlier segment/scan) must
    # already have written the slot, and they STAY in the program — the scan
    # then re-emits the same value at out[0..seeds), which the slice write
    # harmlessly rewrites.
    init_texts = []
    for s in range(slot_lo, lo + off):
        if not _slot_written_before(ordered, _SCAN_SEGMENTS_REF["segments"], s):
            return None
        init_texts.append(f"poly[{s}]")
    step_text = scalar_src(step_node)
    return ("scan", slot_lo, hi + off, hi - lo + seeds, lo - seeds, init_texts, step_text)


_SCAN_SEGMENTS_REF = {"segments": []}


def analyze(fn):
    """Return the template pieces or raise SkipFunction.

    Result: {"n": int,
             "segments": [(kvar, lo, hi, off, expr)],   # disjoint slot spans
             "postops": [("poke", slot, expr) | ("aug_slice", a, b, op, expr)]}
    Post-ops run after the segment vectors, in original python order.
    """
    body = _unwrap_try(fn)
    args = [a.arg for a in fn.args.args]
    if args[:2] != ["t1", "t2"]:
        raise SkipFunction(f"signature {args} is not (t1, t2, ...)")

    n_coeffs = None
    segments = []         # (kvar, lo, hi, off, expr)
    _SCAN_SEGMENTS_REF["segments"] = segments
    ordered = []          # ("poke", slot, expr) / ("aug_slice", a, b, op, expr)
    env = {}              # inlined temps outside loops

    def _seg_slots(seg):
        _kvar, lo, hi, off, _expr = seg
        return set(range(lo + off, hi + off))

    for stmt in body:
        if isinstance(stmt, ast.Return):
            if n_coeffs is None:
                raise SkipFunction("return before cf allocation")
            break
        if isinstance(stmt, ast.AugAssign):
            value = _InlineTemps(env).visit(stmt.value)
            slot = _slot_of_target(stmt.target)
            if slot and slot[0] == "const":
                # cf[i] op= x  ->  poke reading the current slot
                read = ast.Subscript(value=ast.Name(id="cf", ctx=ast.Load()),
                                     slice=ast.Constant(value=slot[1]), ctx=ast.Load())
                combined = ast.fix_missing_locations(
                    ast.copy_location(ast.BinOp(left=read, op=stmt.op, right=value), stmt))
                if not segments and not any(o[0] == "scan" for o in ordered):
                    raise SkipFunction("AugAssign before any segment (order-sensitive)")
                _append_poke(ordered, slot[1], combined)
                continue
            bounds = _slice_bounds(stmt.target)
            if bounds is not None:
                if not isinstance(stmt.op, (ast.Add, ast.Sub, ast.Mult)):
                    raise SkipFunction("slice AugAssign op is not +=/-=/*=")
                if _reads_cf(value) or _contains_k(value, _ARANGE_KVAR):
                    raise SkipFunction("slice AugAssign value reads cf")
                if any(isinstance(n2, ast.Call) and isinstance(n2.func, ast.Attribute)
                       and n2.func.attr == "arange" for n2 in ast.walk(value)):
                    raise SkipFunction("slice AugAssign with arange value")
                opname = {ast.Add: "add", ast.Sub: "subtract", ast.Mult: "multiply"}[type(stmt.op)]
                ordered.append(("aug_slice", bounds[0], bounds[1], opname, value, len(segments)))
                continue
            strided = _strided_target(stmt.target)
            if strided is not None:
                a, b, step = strided
                if not isinstance(stmt.op, (ast.Add, ast.Sub, ast.Mult, ast.Div)):
                    raise SkipFunction("strided AugAssign op is not +=/-=/*=//=")
                if _reads_cf(value) or _contains_vector_shape(value):
                    raise SkipFunction("strided AugAssign value is not scalar")
                opname = {ast.Add: "add", ast.Sub: "subtract",
                          ast.Mult: "multiply", ast.Div: "divide"}[type(stmt.op)]
                ordered.append(("stride_aug", a, b, step, opname, value, len(segments)))
                continue
            raise SkipFunction("unsupported AugAssign target")
        if isinstance(stmt, ast.Assign) and len(stmt.targets) == 1:
            tgt = stmt.targets[0]
            if isinstance(tgt, ast.Subscript):
                tgt = _InlineTemps(env).visit(tgt)
            value = _InlineTemps(env).visit(stmt.value)
            if isinstance(tgt, ast.Name) and tgt.id == "cf":
                call = value
                if (isinstance(call, ast.Call) and isinstance(call.func, ast.Attribute)
                        and call.func.attr == "zeros" and call.args):
                    n = _const_int(call.args[0])
                    if n is None or n < 1:
                        raise SkipFunction("cf = np.zeros(<non-const>)")
                    n_coeffs = n
                    continue
                # whole-vector definition: cf = <expr over arange>  ==  cf[0:n] = expr
                if _reads_cf(value):
                    raise SkipFunction("whole-vector cf assign reads cf")
                if n_coeffs is None:
                    ab = _first_arange_bounds(value)
                    if ab is None:
                        raise SkipFunction("cf assigned from something besides np.zeros")
                    n_coeffs = ab[1] - ab[0]
                synth = ast.fix_missing_locations(ast.copy_location(ast.Subscript(
                    value=ast.Name(id="cf", ctx=ast.Load()),
                    slice=ast.Slice(lower=ast.Constant(value=0),
                                    upper=ast.Constant(value=n_coeffs)),
                    ctx=ast.Store()), stmt))
                seg = _normalize_slice_assign(synth, value)
                if seg is None:
                    raise SkipFunction("cf assigned from something besides np.zeros")
                segments.append(seg)
                ordered.append(("segment", len(segments) - 1))
                continue
            if isinstance(tgt, ast.Name):
                env[tgt.id] = value
                continue
            slot = _slot_of_target(tgt)
            if slot and slot[0] == "const":
                wrote_any = bool(segments) or any(o[0] == "scan" for o in ordered)
                if not wrote_any and _reads_cf(value):
                    raise SkipFunction("pre-segment poke reads cf (order-sensitive)")
                _append_poke(ordered, slot[1], value)
                continue
            if (isinstance(tgt, ast.Subscript) and isinstance(tgt.value, ast.Name)
                    and tgt.value.id == "cf" and isinstance(tgt.slice, (ast.List, ast.Tuple))):
                idxs = [_const_int(e) for e in tgt.slice.elts]
                if any(i is None for i in idxs):
                    raise SkipFunction("cf[list] with non-constant indices")
                wrote_any = bool(segments) or any(o[0] == "scan" for o in ordered)
                if not wrote_any and _reads_cf(value):
                    raise SkipFunction("pre-segment poke reads cf (order-sensitive)")
                if isinstance(value, (ast.List, ast.Tuple)):
                    if len(value.elts) != len(idxs):
                        raise SkipFunction("cf[list] value length mismatch")
                    for slot_i, elt in zip(idxs, value.elts):
                        _append_poke(ordered, slot_i, elt)
                else:
                    if _contains_vector_shape(value):
                        raise SkipFunction("cf[list] with vector value")
                    for slot_i in idxs:
                        _append_poke(ordered, slot_i, value)
                continue
            slice_bounds = _slice_bounds(tgt)
            if slice_bounds is not None and _reads_cf(value):
                # In-place slice rewrite (cf[a:b] = f(cf[a:b])): reads the
                # current poly, so it stays an ordered post-op.
                ordered.append(("slice_assign", slice_bounds[0], slice_bounds[1], value, len(segments)))
                continue
            seg = _normalize_slice_assign(tgt, value)
            if seg is not None:
                segments.append(seg)
                ordered.append(("segment", len(segments) - 1))
                continue
            raise SkipFunction(f"unsupported assignment target {ast.dump(tgt)[:60]}")
        if isinstance(stmt, ast.For):
            if not (isinstance(stmt.iter, ast.Call) and isinstance(stmt.iter.func, ast.Name)
                    and stmt.iter.func.id == "range"):
                raise SkipFunction("loop is not over range()")
            def _fold_range_arg(a):
                a = _InlineTemps(env).visit(a)
                if (isinstance(a, ast.Call) and isinstance(a.func, ast.Name)
                        and a.func.id == "len" and len(a.args) == 1
                        and isinstance(a.args[0], ast.Name) and a.args[0].id == "cf"
                        and n_coeffs is not None):
                    return n_coeffs
                c = _const_int(a)
                if c is not None:
                    return c
                lin = _linear_k(a, "\x00")
                return lin[1] if lin is not None and lin[0] == 0 else None

            rargs = [_fold_range_arg(a) for a in stmt.iter.args]
            if (len(rargs) == 3 and all(a is not None for a in rargs)
                    and rargs[2] >= 2 and isinstance(stmt.target, ast.Name)):
                # stepped range: unroll into ordered pokes (token cap gates size)
                u_lo, u_hi, u_step = rargs
                u_kvar = stmt.target.id
                if (u_hi - u_lo) // u_step > 24:
                    raise SkipFunction("stepped loop too long to unroll")
                wrote_any = bool(segments) or any(o[0] == "scan" for o in ordered)
                for u_val in range(u_lo, u_hi, u_step):
                    u_env = dict(env)
                    u_env[u_kvar] = ast.Constant(value=u_val)
                    for inner in stmt.body:
                        if isinstance(inner, ast.AugAssign):
                            read = ast.Subscript(value=inner.target.value,
                                                 slice=inner.target.slice, ctx=ast.Load())
                            inner = ast.Assign(
                                targets=[inner.target],
                                value=ast.BinOp(left=read, op=inner.op, right=inner.value))
                            inner = ast.fix_missing_locations(ast.copy_location(inner, stmt))
                        if not (isinstance(inner, ast.Assign) and len(inner.targets) == 1):
                            raise SkipFunction("stepped loop body is not an assignment")
                        u_tgt = _InlineTemps(u_env).visit(inner.targets[0])
                        u_val_expr = ast.fix_missing_locations(
                            _InlineTemps(u_env).visit(inner.value))
                        u_slot = _slot_of_target(u_tgt)
                        if not (u_slot and u_slot[0] == "const"):
                            raise SkipFunction("stepped loop writes a non-constant slot")
                        if not wrote_any and _reads_cf(u_val_expr):
                            raise SkipFunction("pre-segment poke reads cf (order-sensitive)")
                        _append_poke(ordered, u_slot[1], u_val_expr)
                continue
            if any(a is None for a in rargs) or len(rargs) not in (1, 2):
                raise SkipFunction("range() args are not integer literals")
            lo, hi = (0, rargs[0]) if len(rargs) == 1 else rargs
            if not isinstance(stmt.target, ast.Name):
                raise SkipFunction("destructuring loop target")
            kvar = stmt.target.id
            loop_env = dict(env)
            write = None
            const_writes = []   # cf[<int>] = expr inside the loop
            extra_writes = []   # additional asc/desc slot writes (split loop)
            aug_writes = 0      # in-loop cf[k] op= expr lowered to slice ops
            body_stmts = _collapse_same_slot_rewrites(list(stmt.body))
            if body_stmts and isinstance(body_stmts[-1], ast.If):
                incoming = None
                prefix = body_stmts[:-1]
                if prefix and isinstance(prefix[-1], ast.Assign) and len(prefix[-1].targets) == 1:
                    prior_slot = _slot_of_target(prefix[-1].targets[0], kvar)
                    if_slot = None
                    probe = _branch_value(body_stmts[-1].body, kvar, loop_env,
                                          ast.Constant(value=0))
                    if probe is not None:
                        if_slot = probe[0]
                    if prior_slot and prior_slot[0] == "loop" and prior_slot[1] == if_slot:
                        incoming = _InlineTemps(loop_env).visit(prefix[-1].value)
                        prefix = prefix[:-1]
                folded = _fold_if_to_select(body_stmts[-1], kvar, loop_env, incoming)
                if folded is not None:
                    body_stmts = prefix + [folded]
            for inner in body_stmts:
                if isinstance(inner, ast.AugAssign):
                    a_slot = _slot_of_target(
                        _InlineTemps(loop_env).visit(inner.target), kvar)
                    a_val = _InlineTemps(loop_env).visit(inner.value)
                    if (a_slot and a_slot[0] == "loop"
                            and isinstance(inner.op, (ast.Add, ast.Sub, ast.Mult, ast.Div))
                            and not _reads_cf(a_val)):
                        a, b = lo + a_slot[1], hi + a_slot[1]
                        arange_call = ast.Call(
                            func=ast.Attribute(value=ast.Name(id="np", ctx=ast.Load()),
                                               attr="arange", ctx=ast.Load()),
                            args=[ast.Constant(value=lo), ast.Constant(value=hi)],
                            keywords=[])
                        vec_val = _ReplaceMatchingLoad(
                            _load_dump(ast.Name(id=kvar, ctx=ast.Load())), arange_call)
                        # _ReplaceMatchingLoad targets Subscripts; swap names directly
                        class _KToArange(ast.NodeTransformer):
                            def visit_Name(self, node):
                                if isinstance(node.ctx, ast.Load) and node.id == kvar:
                                    import copy as _copy
                                    return ast.copy_location(_copy.deepcopy(arange_call), node)
                                return node
                        shifted = ast.fix_missing_locations(_KToArange().visit(a_val))
                        read = ast.Subscript(
                            value=ast.Name(id="cf", ctx=ast.Load()),
                            slice=ast.Slice(lower=ast.Constant(value=a),
                                            upper=ast.Constant(value=b)),
                            ctx=ast.Load())
                        combined = ast.fix_missing_locations(ast.copy_location(
                            ast.BinOp(left=read, op=inner.op, right=shifted), inner))
                        ordered.append(("slice_assign", a, b, combined, len(segments)))
                        aug_writes += 1
                        continue
                if not (isinstance(inner, ast.Assign) and len(inner.targets) == 1):
                    raise SkipFunction("loop body statement is not an assignment")
                itgt = _InlineTemps(loop_env).visit(inner.targets[0])
                ival = _InlineTemps(loop_env).visit(inner.value)
                if isinstance(itgt, ast.Name):
                    loop_env[itgt.id] = ival
                    continue
                slot = _slot_of_target(itgt, kvar)
                if slot and slot[0] == "loop":
                    if write is not None:
                        extra_writes.append(("asc", slot[1], ival))
                        continue
                    write = (slot[1], ival)
                    continue
                if slot and slot[0] == "desc":
                    extra_writes.append(("desc", slot[1], ival))
                    continue
                if slot and slot[0] == "const":
                    # cf[<int>] = expr re-assigned every iteration: only the
                    # final iteration survives, so it is a post-loop poke
                    # with k substituted by the last loop value. cf reads are
                    # fine — after the loop they see exactly the final state.
                    const_writes.append((slot[1], ival))
                    continue
                raise SkipFunction("loop writes a non-loop-indexed slot")
            if write is None and not extra_writes:
                if not aug_writes:
                    raise SkipFunction("loop does not write cf")
                last_k = ast.Constant(value=hi - 1)
                for slot_idx, ival in const_writes:
                    final = ast.fix_missing_locations(
                        _InlineTemps({kvar: last_k}).visit(ival))
                    ordered.append(("poke", slot_idx, final))
                continue
            if extra_writes:
                # split the loop: each independent non-cf-reading write is its
                # own segment (descending targets flip k -> c - k)
                all_writes = ([("asc", write[0], write[1])] if write else []) + extra_writes
                if any(_reads_cf(e) for _kind, _p, e in all_writes):
                    raise SkipFunction("multi-write loop reads cf")
                for kind, p, expr in all_writes:
                    if kind == "asc":
                        seg = (kvar, lo, hi, p, expr)
                    else:
                        flipped = ast.fix_missing_locations(
                            _SubstituteK(kvar, p).visit(expr))
                        seg = (kvar, p - hi + 1, p - lo + 1, 0, flipped)
                    slots = _seg_slots(seg)
                    if n_coeffs is not None and (min(slots) < 0 or max(slots) >= n_coeffs):
                        raise SkipFunction(
                            f"loop write slots {min(slots)}..{max(slots)} out of range")
                    segments.append(seg)
                    ordered.append(("segment", len(segments) - 1))
                last_k = ast.Constant(value=hi - 1)
                for slot_idx, ival in const_writes:
                    final = ast.fix_missing_locations(
                        _InlineTemps({kvar: last_k}).visit(ival))
                    ordered.append(("poke", slot_idx, final))
                continue
            if _reads_cf(write[1]):
                scan_entry = _try_scan_template(kvar, lo, hi, write[0], write[1], ordered)
                if scan_entry is None:
                    raise SkipFunction("recursive: loop reads cf (not prev-slot shaped)")
                ordered.append(scan_entry)
                last_k = ast.Constant(value=hi - 1)
                for slot_idx, ival in const_writes:
                    final = ast.fix_missing_locations(
                        _InlineTemps({kvar: last_k}).visit(ival))
                    ordered.append(("poke", slot_idx, final))
                continue
            seg = (kvar, lo, hi, write[0], write[1])
            slots = _seg_slots(seg)
            if n_coeffs is not None and (min(slots) < 0 or max(slots) >= n_coeffs):
                raise SkipFunction(
                    f"loop write slots {min(slots)}..{max(slots)} out of range")
            segments.append(seg)
            ordered.append(("segment", len(segments) - 1))
            last_k = ast.Constant(value=hi - 1)
            for slot_idx, ival in const_writes:
                final = ast.fix_missing_locations(
                    _InlineTemps({kvar: last_k}).visit(ival))
                ordered.append(("poke", slot_idx, final))
            continue
        raise SkipFunction(f"unsupported statement {type(stmt).__name__}")

    if n_coeffs is None:
        raise SkipFunction("no cf = np.zeros(N)")

    for seg in segments:
        extra = {s for s in _seg_slots(seg) if s < 0 or s >= n_coeffs}
        if extra:
            raise SkipFunction(f"segment writes out-of-range slots {sorted(extra)[:6]}")
    for i, a in enumerate(segments):
        for b in segments[i + 1:]:
            if _seg_slots(a) & _seg_slots(b):
                raise SkipFunction("overlapping segments (order-dependent)")

    # A poke is dead if a later segment covers its slot: python ran the poke
    # first and the segment overwrote it. We emit all segments first, so
    # keeping the poke would wrongly resurrect it.
    def _overlaps_segment(a, b, seg_indexes):
        for seg_idx in seg_indexes:
            _kv, s_lo, s_hi, s_off, _e = segments[seg_idx]
            if a < s_hi + s_off and s_lo + s_off < b:
                return True
        return False

    for op in ordered:
        if op[0] in ("aug_slice", "slice_assign", "stride_aug"):
            a, b, born_at = op[1], op[2], op[-1]
            # A later segment overwriting this span would run BEFORE it in
            # the emitted program (segments are emitted first) but AFTER it
            # in python — not linearizable under this emission scheme.
            if _overlaps_segment(a, b, range(born_at, len(segments))):
                raise SkipFunction("slice update precedes an overlapping loop (order-sensitive)")

    postops = []
    seg_seen = 0
    for pos, op in enumerate(ordered):
        if op[0] == "segment":
            seg_seen += 1
            continue
        if op[0] == "poke":
            slot = op[1]
            later = segments[seg_seen:]
            later_scans = [o for o in ordered[pos + 1:] if o[0] == "scan"]
            # A scan's seed slots (first len(init_texts) of its span) read
            # poly BEFORE the scan runs — pokes there are live, not dead.
            covered_later = any(slot in _seg_slots(s) for s in later) or \
               any(o[1] + len(o[5]) <= slot < o[2] for o in later_scans)
            if covered_later:
                # If anything between this poke and its overwrite READS the
                # slot (RMW slices, reductions), python saw the poke value;
                # dropping it OR keeping it both mis-order — not linearizable.
                for later_op in ordered[pos + 1:]:
                    if later_op[0] in ("aug_slice", "slice_assign") and \
                       later_op[1] <= slot < later_op[2]:
                        raise SkipFunction("poke read by a slice update before a loop overwrite")
                    if later_op[0] == "stride_aug" and \
                       later_op[1] <= slot < (later_op[2] if later_op[2] is not None else 1 << 30):
                        raise SkipFunction("poke read by a slice update before a loop overwrite")
                    if later_op[0] == "reduce_push":
                        raise SkipFunction("poke may be read by a reduction before a loop overwrite")
                continue
        postops.append(op)

    covered = set()
    for seg in segments:
        covered |= _seg_slots(seg)
    for op in postops:
        if op[0] == "scan":
            covered |= set(range(op[1], op[2]))
    covered |= {op[1] for op in postops if op[0] == "poke"}
    missing = set(range(n_coeffs)) - covered
    # Loop-free programs start from fill(n, 0), so unwritten slots are
    # already zero. Segment programs evaluate their formula over ALL slots
    # (range extension); if some slots should stay zero, masking each
    # segment to its true window zeroes them without pokes.
    force_mask = bool(missing) and bool(segments)

    return {"n": n_coeffs, "segments": segments, "postops": postops,
            "force_mask": force_mask}


# ---------------------------------------------------------------------------
# Expression emission
# ---------------------------------------------------------------------------

def _fmt_num(v):
    if isinstance(v, complex):
        if v.imag == 0:
            return _fmt_num(v.real)
        if v.real == 0:
            return f"{_fmt_num(v.imag)}i"
        sign = "+" if v.imag >= 0 else "-"
        return f"({_fmt_num(v.real)} {sign} {_fmt_num(abs(v.imag))}i)"
    if isinstance(v, float) and v.is_integer() and abs(v) < 1e15:
        return str(int(v))
    return repr(v)


def _call_name(node):
    """np.sin -> 'sin', bare abs -> 'abs'; None if not a supported unary."""
    fn = node.func
    if isinstance(fn, ast.Attribute) and isinstance(fn.value, ast.Name) and fn.value.id == "np":
        name = fn.attr
    elif isinstance(fn, ast.Name):
        name = fn.id
    else:
        return None
    name = NP_UNARY_ALIASES.get(name, name)
    return name if name in UNARY_FUNCS else None


def scalar_src(node, kvar=None):
    """Emit a scalar infix expression (no loop var allowed)."""
    if isinstance(node, ast.Constant):
        if isinstance(node.value, (int, float, complex)):
            return _fmt_num(complex(node.value) if isinstance(node.value, complex) else float(node.value))
        raise SkipFunction(f"unsupported constant {node.value!r}")
    if isinstance(node, ast.Name):
        if node.id == "t1":
            return "p1"
        if node.id == "t2":
            return "p2"
        if node.id == "pi":
            return "pi"
        if node.id == "__prev__":
            return "prev"
        if node.id == "__prev2__":
            return "prev2"
        if node.id == "__scan_k__":
            return "k"
        if node.id == "__tos0__":
            return "tos[0]"
        if node.id == kvar:
            raise SkipFunction("loop var escaped to scalar context")
        raise SkipFunction(f"unsupported name {node.id!r}")
    if isinstance(node, ast.Subscript):
        slot = _slot_of_target(node)
        if slot and slot[0] == "const":
            return f"poly[{slot[1]}]"
        raise SkipFunction("unsupported subscript in scalar expression")
    if isinstance(node, ast.Attribute) and node.attr in ("real", "imag"):
        return f"{node.attr}({scalar_src(node.value, kvar)})"
    if isinstance(node, ast.Call):
        red = _reduction_call_name(node)
        if red and len(node.args) == 1:
            if _contains_vector_shape(node.args[0]):
                raise SkipFunction("reduction outside a poke statement")
            return scalar_src(node.args[0], kvar)  # sum/prod of a scalar
        name = _call_name(node)
        if name and len(node.args) == 1 and not node.keywords:
            return f"{name}({scalar_src(node.args[0], kvar)})"
        raise SkipFunction(f"unsupported call {ast.dump(node.func)[:50]}")
    if isinstance(node, ast.UnaryOp):
        if isinstance(node.op, ast.USub):
            inner = scalar_src(node.operand, kvar)
            # The target grammar binds unary minus TIGHTER than ** (opposite
            # of Python), so any non-atomic operand gets parens: -(x**2),
            # never -x**2 (which the compiler now rejects as ambiguous).
            if _scalar_prec(node.operand, kvar) < 4:
                inner = f"({inner})"
            return f"-{inner}"
        if isinstance(node.op, ast.UAdd):
            return scalar_src(node.operand, kvar)
        raise SkipFunction("unsupported unary op")
    if isinstance(node, ast.BinOp):
        if isinstance(node.op, ast.Pow):
            e = node.right
            ev = e.value if isinstance(e, ast.Constant) else None
            if isinstance(ev, int) and 0 < abs(ev) <= 32:
                base = scalar_src(node.left, kvar)
                if _scalar_prec(node.left, kvar) < 3:
                    base = f"({base})"
                return f"{base}**{ev}"
            base = scalar_src(node.left, kvar)
            expo = scalar_src(e, kvar)
            if _scalar_prec(e, kvar) < 2:
                expo = f"({expo})"
            return f"exp(log({base}) * {expo})"
        ops = {ast.Add: ("+", 1), ast.Sub: ("-", 1), ast.Mult: ("*", 2), ast.Div: ("/", 2)}
        got = ops.get(type(node.op))
        if got is None:
            raise SkipFunction(f"unsupported operator {type(node.op).__name__}")
        op, prec = got
        left = scalar_src(node.left, kvar)
        if _scalar_prec(node.left, kvar) < prec:
            left = f"({left})"
        right = scalar_src(node.right, kvar)
        # right side needs parens at equal precedence for - and /
        need = prec + (1 if op in "-/" else 0)
        if _scalar_prec(node.right, kvar) < need:
            right = f"({right})"
        return f"{left} {op} {right}"
    raise SkipFunction(f"unsupported scalar node {type(node).__name__}")


def _scalar_prec(node, kvar=None):
    """Precedence of a node's rendered form: 4=atom/call, 3=**, 2=* /, 1=+ -."""
    if isinstance(node, ast.BinOp):
        if isinstance(node.op, ast.Pow):
            e = node.right
            ev = e.value if isinstance(e, ast.Constant) else None
            return 3 if (isinstance(ev, int) and 0 < abs(ev) <= 32) else 4
        return 2 if isinstance(node.op, (ast.Mult, ast.Div)) else 1
    if isinstance(node, ast.UnaryOp) and isinstance(node.op, ast.USub):
        return 2
    if isinstance(node, ast.Constant) and isinstance(node.value, complex):
        return 1 if (node.value.real and node.value.imag) else 4
    return 4


def _contains_k(node, kvar):
    return any(isinstance(n, ast.Name) and n.id == kvar for n in ast.walk(node))


def _reduction_call_name(node):
    """np.sum/np.prod call -> 'sum'/'prod', else None."""
    fn = node.func if isinstance(node, ast.Call) else None
    if (isinstance(fn, ast.Attribute) and isinstance(fn.value, ast.Name)
            and fn.value.id == "np" and fn.attr in ("sum", "prod")):
        return fn.attr
    return None


class _ExtractReduction(ast.NodeTransformer):
    """Replace the first vector-shaped np.sum/np.prod call with __tos0__."""

    def __init__(self):
        self.found = None  # (fname, arg_node)

    def visit_Call(self, node):
        self.generic_visit(node)
        red = _reduction_call_name(node)
        if red and len(node.args) == 1 and _contains_vector_shape(node.args[0]):
            if self.found is not None:
                raise SkipFunction("multiple reductions in one statement")
            self.found = (red, node.args[0])
            return ast.copy_location(ast.Name(id="__tos0__", ctx=ast.Load()), node)
        return node


def _arange_range_text(node):
    """np.arange(c[, d]) call -> range(c, d) source text, else None."""
    fn = node.func if isinstance(node, ast.Call) else None
    if not (isinstance(fn, ast.Attribute) and isinstance(fn.value, ast.Name)
            and fn.value.id == "np" and fn.attr == "arange"):
        return None
    cargs = [_const_int(a) for a in node.args]
    if any(a is None for a in cargs) or len(cargs) not in (1, 2):
        raise SkipFunction("np.arange args are not integer literals")
    c, d = (0, cargs[0]) if len(cargs) == 1 else cargs
    return f"range({c}, {d})"


def _cf_slice_text(node):
    """cf[a:b] read -> poly[a:b] source text, else None."""
    if not (isinstance(node, ast.Subscript) and isinstance(node.value, ast.Name)
            and node.value.id == "cf" and isinstance(node.slice, ast.Slice)):
        return None
    bounds = _slice_bounds(node)
    if bounds is None:
        raise SkipFunction("cf slice bounds are not integer literals")
    return f"poly[{bounds[0]}:{bounds[1]}]"


def _contains_vector_shape(node):
    """True when the subtree carries an arange or cf-slice vector."""
    for n in ast.walk(node):
        if isinstance(n, ast.Call):
            fn = n.func
            if (isinstance(fn, ast.Attribute) and isinstance(fn.value, ast.Name)
                    and fn.value.id == "np" and fn.attr == "arange"):
                return True
        if (isinstance(n, ast.Subscript) and isinstance(n.value, ast.Name)
                and n.value.id == "cf" and isinstance(n.slice, ast.Slice)):
            return True
    return False


def vec_src(node, kvar, k_range, k_text=None):
    """Emit call-form vector expression for a k-dependent subtree.

    kvar may be None for standalone vector expressions (arange / cf-slice
    based), e.g. reduction arguments."""
    text = _arange_range_text(node) if isinstance(node, ast.Call) else None
    if text is not None:
        return text
    text = _cf_slice_text(node)
    if text is not None:
        return text
    if kvar is not None and isinstance(node, ast.Name) and node.id == kvar:
        if k_text is not None:
            return k_text
        lo, hi = k_range
        return f"range({lo}, {hi})"
    if not _contains_k(node, kvar) and not _contains_vector_shape(node):
        return scalar_src(node, kvar)
    if isinstance(node, ast.Call):
        if isinstance(node.func, ast.Name) and node.func.id == "__select__":
            c, a, b = (vec_src(arg, kvar, k_range, k_text) for arg in node.args)
            return f"select({c}, {a}, {b})"
        name = _call_name(node)
        if name and len(node.args) == 1 and not node.keywords:
            return f"{name}({vec_src(node.args[0], kvar, k_range, k_text)})"
        raise SkipFunction(f"unsupported vector call {ast.dump(node.func)[:50]}")
    if isinstance(node, ast.Attribute) and node.attr in ("real", "imag"):
        return f"{node.attr}({vec_src(node.value, kvar, k_range, k_text)})"
    if isinstance(node, ast.UnaryOp):
        if isinstance(node.op, ast.USub):
            return f"neg({vec_src(node.operand, kvar, k_range, k_text)})"
        if isinstance(node.op, ast.UAdd):
            return vec_src(node.operand, kvar, k_range, k_text)
        raise SkipFunction("unsupported unary op in vector context")
    if isinstance(node, ast.BinOp):
        if isinstance(node.op, ast.FloorDiv):
            # exact for non-negative ints: a // b == (a - rem(a, b)) / b
            a_txt = vec_src(node.left, kvar, k_range, k_text)
            b_txt = vec_src(node.right, kvar, k_range, k_text)
            return f"divide(subtract({a_txt}, rem({a_txt}, {b_txt})), {b_txt})"
        ops = {ast.Add: "add", ast.Sub: "subtract", ast.Mult: "multiply",
               ast.Div: "divide", ast.Pow: "power", ast.Mod: "rem"}
        op = ops.get(type(node.op))
        if op is None:
            raise SkipFunction(f"unsupported operator {type(node.op).__name__}")
        return f"{op}({vec_src(node.left, kvar, k_range, k_text)}, {vec_src(node.right, kvar, k_range, k_text)})"
    if isinstance(node, ast.Compare):
        if len(node.ops) != 1 or len(node.comparators) != 1:
            raise SkipFunction("chained comparison")
        cmps = {ast.Eq: "eq", ast.Gt: "gt", ast.GtE: "ge", ast.Lt: "lt", ast.LtE: "le"}
        op = cmps.get(type(node.ops[0]))
        if op is None:
            raise SkipFunction(f"unsupported comparison {type(node.ops[0]).__name__}")
        return f"{op}({vec_src(node.left, kvar, k_range, k_text)}, {vec_src(node.comparators[0], kvar, k_range, k_text)})"
    raise SkipFunction(f"unsupported vector node {type(node).__name__}")


def _contains_k(node, kvar):  # noqa: F811 (kvar may be None for standalone)
    if kvar is None:
        return False
    return any(isinstance(n, ast.Name) and n.id == kvar for n in ast.walk(node))


def _split_terms(node):
    """Flatten a top-level +/- chain into (sign, node) terms."""
    if isinstance(node, ast.BinOp) and isinstance(node.op, (ast.Add, ast.Sub)):
        left = _split_terms(node.left)
        right = _split_terms(node.right)
        if isinstance(node.op, ast.Sub):
            right = [(-s, n) for s, n in right]
        return left + right
    return [(1, node)]


def _step_mask(a, n):
    """Vector that is 1 for slot >= a, 0 below — exact 0/1 arithmetic.

    x = slot - a + 0.5 is a nonzero half-integer, so (x + |x|) / (2|x|)
    is exactly 1.0 (2x/2x) or 0.0 (0/2|x|) in floating point.
    """
    r = f"range({0.5 - a}, {n + 0.5 - a})"
    return f"divide(add({r}, abs({r})), multiply(abs({r}), 2))"


def _window_mask(a, b, n):
    """Mask for slots [a, b) as a nested-call vector expression, or None."""
    lo_needed = a > 0
    hi_needed = b < n
    if not lo_needed and not hi_needed:
        return None
    if lo_needed and hi_needed:
        return f"subtract({_step_mask(a, n)}, {_step_mask(b, n)})"
    if lo_needed:
        return _step_mask(a, n)
    return f"subtract(1, {_step_mask(b, n)})"


MAX_LINE = 100

_ALIAS_SEQ = {"n": 0}
_HOIST_MEMO = {}


def _append_commented(lines, code, frag):
    """Inline short comments; long ones go on their own line above the code
    (the book PDF renders program sources with a hard line-width budget)."""
    frag = (frag or "").strip()
    if not frag:
        lines.append(code)
    elif len(code) + len(frag) + 5 <= 110:
        lines.append(f"{code}   # {frag}")
    else:
        lines.append(f"# {frag[:104]}")
        lines.append(code)


def _fresh_alias(prefix="x"):
    _ALIAS_SEQ["n"] += 1
    return f"{prefix}{_ALIAS_SEQ['n']}"


def _hoist_vec(node, kvar, k_range, k_text, lines):
    """Render a vector subtree, hoisting long children into local aliases
    so no emitted line exceeds MAX_LINE. Locals substitute at compile time,
    so the chain (and fingerprint behavior) is unchanged."""
    text = vec_src(node, kvar, k_range, k_text)
    if len(text) <= MAX_LINE:
        return text

    def piece(child):
        if _contains_k(child, kvar) or _contains_vector_shape(child):
            return _hoist_vec(child, kvar, k_range, k_text, lines)
        return scalar_src(child, kvar)

    def hoisted(child):
        part = piece(child)
        if len(part) > 40:
            if part in _HOIST_MEMO:
                return _HOIST_MEMO[part]
            name = _fresh_alias()
            lines.append(f"{name} = {part}")
            _HOIST_MEMO[part] = name
            return name
        return part

    if isinstance(node, ast.Call):
        if isinstance(node.func, ast.Name) and node.func.id == "__select__":
            c, a, b = (hoisted(arg) for arg in node.args)
            return f"select({c}, {a}, {b})"
        name = _call_name(node)
        if name and len(node.args) == 1:
            return f"{name}({_hoist_vec(node.args[0], kvar, k_range, k_text, lines)})"
    if isinstance(node, ast.BinOp):
        ops = {ast.Add: "add", ast.Sub: "subtract", ast.Mult: "multiply",
               ast.Div: "divide", ast.Pow: "power", ast.Mod: "rem"}
        op = ops.get(type(node.op))
        if op:
            return f"{op}({hoisted(node.left)}, {hoisted(node.right)})"
    if isinstance(node, ast.Compare) and len(node.ops) == 1:
        cmps = {ast.Eq: "eq", ast.Gt: "gt", ast.GtE: "ge", ast.Lt: "lt", ast.LtE: "le"}
        op = cmps.get(type(node.ops[0]))
        if op:
            return f"{op}({hoisted(node.left)}, {hoisted(node.comparators[0])})"
    return text  # give up: one long line


def _emit_step_mask(edge, n, lines):
    """Emit the exact 0/1 step mask via labeled locals; returns its name."""
    r = _fresh_alias("r")
    m = _fresh_alias("m")
    lines.append(f"{r} = range({0.5 - edge}, {n + 0.5 - edge})")
    lines.append(f"{m} = divide(add({r}, abs({r})), multiply(abs({r}), 2))   # 1 for slot >= {edge}")
    return m


def _scalar_terms_text(sca_terms):
    parts = []
    for sign, tnode in sca_terms:
        text = scalar_src(tnode)
        parts.append(f"- {text}" if sign < 0 else (f"+ {text}" if parts else text))
    return " ".join(parts)


def _segment_vector(seg, n, module_src, lines, masked):
    """Emit statements that leave the segment's (optionally masked) vector on
    the stack; returns the number of stack values pushed (always 1)."""
    kvar, lo, hi, off, expr = seg
    k_lo, k_hi = -off, n - off
    slot_lo, slot_hi = lo + off, hi + off
    terms = _split_terms(expr)
    vec_terms = [(s, t) for s, t in terms if _contains_k(t, kvar)]
    sca_terms = [(s, t) for s, t in terms if not _contains_k(t, kvar)]
    if not vec_terms:
        body = f"fill({n}, ({_scalar_terms_text(sca_terms)}))"
        sca_terms = []
        stmts = [(body, "scalar broadcast")]
    else:
        k_text = _fresh_alias("ks")
        lines.append(f"{k_text} = range({k_lo}, {k_hi})   # loop variable {kvar}")
        stmts = []
        for sign, tnode in vec_terms:
            src_text = _hoist_vec(tnode, kvar, (k_lo, k_hi), k_text, lines)
            if sign < 0:
                src_text = f"neg({src_text})"
            frag = (ast.get_source_segment(module_src, tnode) or "").replace("\n", " ")
            stmts.append((src_text, frag[:70]))
    for src_text, frag in stmts:
        _append_commented(lines, src_text, frag)
    for _ in range(len(stmts) - 1):
        lines.append("add(pop, pop)")
    if sca_terms:
        lines.append(f"add(pop, ({_scalar_terms_text(sca_terms)}))")
    if masked:
        lo_needed = slot_lo > 0
        hi_needed = slot_hi < n
        if lo_needed or hi_needed:
            m_lo = _emit_step_mask(slot_lo, n, lines) if lo_needed else None
            m_hi = _emit_step_mask(slot_hi, n, lines) if hi_needed else None
            if m_lo and m_hi:
                mask = f"subtract({m_lo}, {m_hi})"
            elif m_lo:
                mask = m_lo
            else:
                mask = f"subtract(1, {m_hi})"
            lines.append(f"multiply(pop, {mask})   # keep slots {slot_lo}..{slot_hi - 1}")
    return 1


def emit_program(name, template, module_src):
    """Assemble readable Coeff Program source text."""
    n = template["n"]
    segments = template["segments"]
    force_mask = template.get("force_mask", False)
    _ALIAS_SEQ["n"] = 0
    _HOIST_MEMO.clear()
    lines = [f"# {name} ({_MODULE_LABEL['label']}), ported by port_poly100_programs.py"]
    if not segments:
        lines.append(f"poly = fill({n}, 0)")
    elif len(segments) == 1:
        kvar, lo, hi, off, _expr = segments[0]
        kname = "arange" if kvar == _ARANGE_KVAR else kvar
        lines.append(f"# loop: cf over {kname} with range({lo}, {hi}) -> "
                     f"slots {lo + off}..{hi + off - 1}")
        if (lo + off, hi + off) != (0, n):
            lines.append("# (formula evaluated over all slots; "
                         + ("masked to the loop window)" if force_mask
                            else "pokes below overwrite the rest)"))
        before = len(lines)
        _segment_vector(segments[0], n, module_src, lines, masked=force_mask)
        if len(lines) == before + 1 and "pop" not in lines[-1]:
            # one self-contained statement: assign it directly
            body, _sep, comment = lines.pop().partition("   #")
            lines.append(f"poly = {body.strip()}" + (f"   #{comment}" if comment else ""))
        else:
            lines.append("poly = pop")
    else:
        lines.append(f"# {len(segments)} disjoint segments, each masked to its slot window")
        for i, seg in enumerate(segments):
            kvar, lo, hi, off, _expr = seg
            lines.append(f"# segment {i + 1}: slots {lo + off}..{hi + off - 1}")
            _segment_vector(seg, n, module_src, lines, masked=True)
        for _ in range(len(segments) - 1):
            lines.append("add(pop, pop)")
        lines.append("poly = pop")
    for op in template["postops"]:
        if op[0] == "poke":
            _tag, slot, expr = op
            frag = (ast.get_source_segment(module_src, expr) or "").replace("\n", " ")
            _append_commented(lines, f"poly[{slot}] = {scalar_src(expr)}", frag)
        elif op[0] == "scan":
            _tag, a, b, length, k0, init_texts, step_text = op
            init_text = ", ".join(init_texts)
            if " / abs(" in step_text and len(step_text) > MAX_LINE - 40:
                head = step_text.split(" / abs(", 1)[0]
                core = head[1:-1] if head.startswith("(") and head.endswith(")") else head
                if step_text in (f"{head} / abs({head})", f"{head} / abs({core})"):
                    v = _fresh_alias("v")
                    lines.append(f"{v} = {core}   # recurrence step value")
                    step_text = f"{v} / abs({v})"
            lines.append(f"poly[{a}:{b}] = scan({length}, {k0}, {init_text}, {step_text})"
                         f"   # recurrence over slots {a}..{b - 1}")
        elif op[0] == "slice_assign":
            _tag, a, b, value = op[:4]
            frag = (ast.get_source_segment(module_src, value) or "").replace("\n", " ")
            _append_commented(lines, f"poly[{a}:{b}] = {_hoist_vec(value, None, None, None, lines)}", frag)
        elif op[0] == "reduce_push":
            _tag, fname, arg = op
            frag = (ast.get_source_segment(module_src, arg) or "").replace("\n", " ")
            _append_commented(lines, f"{fname}({_hoist_vec(arg, None, None, None, lines)})",
                              f"np.{fname}({frag[:60]})" if frag else "")
        elif op[0] == "stride_aug":
            _tag, a, b, step, opname, expr = op[:6]
            b_eff = n if b is None else min(b, n)
            frag = (ast.get_source_segment(module_src, expr) or "").replace("\n", " ")
            offset = 72 * step - a   # keeps rem's first arg positive for all k < 72
            periodic = f"eq(rem(add(range(0, {n}), {offset}), {step}), 0)"
            lo_needed, hi_needed = a > 0, b_eff < n
            if lo_needed and hi_needed:
                win = f"subtract({_emit_step_mask(a, n, lines)}, {_emit_step_mask(b_eff, n, lines)})"
            elif lo_needed:
                win = _emit_step_mask(a, n, lines)
            elif hi_needed:
                win = f"subtract(1, {_emit_step_mask(b_eff, n, lines)})"
            else:
                win = ""
            mask = f"multiply({win}, {periodic})" if win else periodic
            sym = {"add": "+", "subtract": "-", "multiply": "*", "divide": "/"}[opname]
            comment = f"cf[{a}:{'' if b is None else b}:{step}] {sym}= {frag[:40]}"
            if opname in ("multiply", "divide"):
                factor = f"({scalar_src(expr)})" if opname == "multiply" else f"(1/({scalar_src(expr)}))"
                _append_commented(lines,
                    f"multiply(poly, add(1, multiply({mask}, ({factor} - 1))))", comment)
            else:
                _append_commented(lines,
                    f"{opname}(poly, multiply({mask}, ({scalar_src(expr)})))", comment)
            lines.append("poly = pop")
        elif op[0] == "drop":
            lines.append("drop")
        else:
            _tag, a, b, opname, expr = op[:5]
            lo_needed, hi_needed = a > 0, b < n
            if lo_needed and hi_needed:
                mask = f"subtract({_emit_step_mask(a, n, lines)}, {_emit_step_mask(b, n, lines)})"
            elif lo_needed:
                mask = _emit_step_mask(a, n, lines)
            elif hi_needed:
                mask = f"subtract(1, {_emit_step_mask(b, n, lines)})"
            else:
                mask = "1"
            frag = (ast.get_source_segment(module_src, expr) or "").replace("\n", " ")
            if opname == "multiply":
                # cf[a:b] *= X: factor is 1 outside the window, X inside
                _append_commented(lines,
                    f"multiply(poly, add(1, multiply({mask}, (({scalar_src(expr)}) - 1))))",
                    f"cf[{a}:{b}] *= {frag[:50]}")
            else:
                _append_commented(lines,
                    f"{opname}(poly, multiply({mask}, ({scalar_src(expr)})))",
                    f"cf[{a}:{b}] {'+' if opname == 'add' else '-'}= {frag[:50]}")
            lines.append("poly = pop")
    return "\n".join(lines) + "\n"


# ---------------------------------------------------------------------------
# Parity gate
# ---------------------------------------------------------------------------

def python_reference(fn_src):
    ns = {"np": np, "pi": float(np.pi)}
    exec(fn_src, ns)  # verbatim poly100 source, numpy only
    name = fn_src.split("(")[0].split()[-1]
    return ns[name]


def run_native(compiled, n_coeffs, points):
    payload = {"version": 1, "fingerprint": compiled["fingerprint"],
               "tokens": compiled["tokens"], "stack_max": compiled["stack_max"],
               "scalar_exprs": compiled["scalar_exprs"]}
    params = []
    for p1, p2 in points:
        params += [p1.real, p1.imag, p2.real, p2.imag]
    with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as fh:
        params_path = fh.name
        fh.write(struct.pack("<" + "f" * len(params), *params))
    with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as fh:
        out_path = fh.name
    try:
        spec = {"mode": "coeffgen_chunked", "function": "const",
                "cfpv": [n_coeffs, 0, 0], "params_file": params_path,
                "step_start": 0, "source_step_start": 0,
                "source_n1": len(points), "source_n2": 1, "step_count": len(points),
                "coeff_transforms": [], "coeff_program": payload}
        proc = subprocess.run([SWEEP_TEST, out_path], input=json.dumps(spec),
                              capture_output=True, text=True, timeout=60)
        if proc.returncode != 0:
            return None, proc.stderr.strip()[:160]
        data = open(out_path, "rb").read()
    finally:
        for p in (params_path, out_path):
            try:
                os.remove(p)
            except FileNotFoundError:
                pass
    vals = struct.unpack("<" + "f" * (len(data) // 4), data)
    return [complex(vals[i], vals[i + 1]) for i in range(0, len(vals), 2)], None


def parity_check(compiled, py_fn, n_coeffs):
    native, err = run_native(compiled, n_coeffs, PARITY_POINTS)
    if native is None:
        return None, f"native run failed: {err}"
    if len(native) != len(PARITY_POINTS) * n_coeffs:
        return None, f"native output length {len(native)} != {len(PARITY_POINTS) * n_coeffs}"
    worst = 0.0
    valid = 0
    chaotic_slots = 0
    total_slots = 0
    for r, (p1, p2) in enumerate(PARITY_POINTS):
        p1q = complex(np.float32(p1.real), np.float32(p1.imag))
        p2q = complex(np.float32(p2.real), np.float32(p2.imag))
        with np.errstate(all="ignore"):
            ref = np.asarray(py_fn(p1q, p2q), dtype=np.complex128)
            # Chaos filter: recompute with the inputs perturbed by one f32
            # ulp. Slots where the reference itself moves beyond wire
            # precision have no defined value to match (recurrences like
            # poly_2 amplify ulp differences to O(1) in their tail); they
            # are excluded, and the port must match everywhere stable.
            # All four components move, with MIXED directions: a same-sign
            # perturbation cancels in combinations like Re(p1) - Im(p2)
            # (poly_67's z = t1 + 1j*t2 is invariant under it), hiding
            # genuine hypersensitivity.
            up, down = np.float32(2.0), np.float32(-2.0)
            p1b = complex(float(np.nextafter(np.float32(p1q.real), up)),
                          float(np.nextafter(np.float32(p1q.imag), down)))
            p2b = complex(float(np.nextafter(np.float32(p2q.real), down)),
                          float(np.nextafter(np.float32(p2q.imag), up)))
            ref_b = np.asarray(py_fn(p1b, p2b), dtype=np.complex128)
        if len(ref) != n_coeffs:
            return None, f"python returned {len(ref)} coeffs, expected {n_coeffs}"
        finite = np.isfinite(ref.real) & np.isfinite(ref.imag)
        if not finite.all() or np.abs(ref).max() > F32_MAX:
            continue  # beyond f32 transport range at this point
        if not ref.any():
            continue  # except-branch zeros are ambiguous
        got = native[r * n_coeffs:(r + 1) * n_coeffs]
        # Per-slot chaos test: the reference must be self-consistent under a
        # 1-ulp input change, judged against the slot's own magnitude (no
        # absolute floor — small slots are compared as tightly as big ones).
        stable = [abs(rb - rf) <= PARITY_REL_TOL * max(abs(rf), abs(rb), 1e-30)
                  for rb, rf in zip(ref_b, ref)]
        chaotic_slots += stable.count(False)
        total_slots += n_coeffs
        pairs = [(g, rf) for (g, rf), ok in zip(zip(got, ref), stable) if ok]
        if not pairs:
            continue
        # Per-slot mixed tolerance: relative to the slot's own magnitude
        # (covers the f32 wire, which quantizes each value relative to
        # itself) plus a tiny absolute term for true zeros. No row-relative
        # fallback: an O(1) error on a small slot must fail even when the
        # row's dynamic range is huge.
        rel = max(abs(g - rf) / max(abs(rf), abs(g), 1e-6) for g, rf in pairs)
        # Secondary, row-relative measure: numpy computes integer powers by
        # repeated multiplication while the VM's c_powc goes through exp/log;
        # on ill-conditioned slots (huge power + catastrophic cancellation)
        # the phase jitter lands far above the slot's own value while staying
        # deep beneath the row's dynamic range. Score that case against the
        # row maximum instead of the slot.
        worst = max(worst, rel)
        valid += 1
    if valid < PARITY_MIN_POINTS:
        return None, f"only {valid} parity points in f32 range"
    chaotic_frac = (chaotic_slots / total_slots) if total_slots else 0.0
    # Chaotic slots have no canonical value on either side (numpy's own
    # answer there changes under 1-ulp input perturbation), so they cannot
    # gate a port; but a port verified on too few slots is not a port.
    if chaotic_frac > 0.7:
        return None, f"{chaotic_frac:.0%} of slots are chaotic (reference moves under 1 ulp)"
    stats = {"points": valid, "chaotic": chaotic_frac}
    if worst <= PARITY_REL_TOL:
        return {**stats, "worst": worst, "gate": "strict"}, None
    return None, f"parity failed: rel err {worst:.2e} over {valid} points"


# ---------------------------------------------------------------------------
# Upload
# ---------------------------------------------------------------------------

def build_payload(program_name, source_text):
    import handler_storage as hs
    return hs._compile_coeff_program_payload(program_name, source_text=source_text)


def upload(program, existing_keys, force=False):
    import handler_storage as hs
    key = hs._coeff_program_key(program["id"])
    if key in existing_keys and not force:
        return key, "exists (skipped; use --force to overwrite)"
    body = (json.dumps(program, indent=2) + "\n").encode("utf-8")
    hs.s3.put_object(
        Bucket=hs.BUCKET, Key=key, Body=body,
        ContentType="application/json",
        Metadata=hs._coeff_program_put_metadata(program),
    )
    return key, "uploaded"


def list_existing_keys():
    import handler_storage as hs
    keys = set()
    paginator = hs.s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=hs.BUCKET, Prefix=hs.COEFF_PROGRAMS_PREFIX):
        for obj in page.get("Contents", []):
            keys.add(obj["Key"])
    return keys


# ---------------------------------------------------------------------------
# Driver
# ---------------------------------------------------------------------------

def port_one(name, fn_node, fn_src, module_src):
    template = analyze(fn_node)
    source_text = emit_program(name, template, module_src)
    compiled = compile_coeff_program_source(source_text)
    py_fn = python_reference(fn_src)
    stats, err = parity_check(compiled, py_fn, template["n"])
    if err:
        raise SkipFunction(err)
    return source_text, compiled, template, stats


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--only", nargs="*", help="port just these function names")
    ap.add_argument("--exclude", nargs="*", default=["poly_1"],
                    help="skip these (default: poly_1, already ported by hand)")
    ap.add_argument("--dry-run", action="store_true", help="no S3 upload")
    ap.add_argument("--force", action="store_true", help="overwrite existing S3 keys")
    ap.add_argument("--list", action="store_true", help="just list candidate functions")
    ap.add_argument("--show", action="store_true", help="print generated source for ports")
    ap.add_argument("--module", default=POLY100,
                    help="source module to port (default: poly100.py)")
    args = ap.parse_args()

    if not os.path.exists(SWEEP_TEST):
        sys.exit(f"sweep_test missing; build: cc -O2 -pthread -o {SWEEP_TEST} "
                 f"{os.path.join(LAMBDA_DIR, 'sweep_cli.c')} -lm")

    _MODULE_LABEL["label"] = os.path.basename(args.module)
    module_src, functions = load_functions(args.module)
    names = args.only or sorted(functions, key=lambda s: int(s.split("_")[1]))
    names = [n for n in names if n not in set(args.exclude or [])]

    if args.list:
        for n in names:
            print(n)
        return

    existing = set() if args.dry_run else list_existing_keys()
    ported, skipped = [], []
    for name in names:
        if name not in functions:
            skipped.append((name, "not found in module"))
            continue
        fn_node, fn_src = functions[name]
        try:
            source_text, compiled, template, stats = port_one(name, fn_node, fn_src, module_src)
        except SkipFunction as exc:
            skipped.append((name, str(exc)))
            continue
        except Exception as exc:  # compiler diagnostics etc.
            msg = str(exc)
            if hasattr(exc, "diagnostics"):
                msg = "; ".join(str(d.get("message")) for d in exc.diagnostics)
            skipped.append((name, f"compile: {msg[:120]}"))
            continue
        program_name = f"{name}-v1"
        if args.show:
            print(f"----- {program_name} -----")
            print(source_text)
        if args.dry_run:
            note = "dry-run"
        else:
            program = build_payload(program_name, source_text)
            _key, note = upload(program, existing, force=args.force)
        ported.append((name, template["n"], compiled["token_count"],
                       stats["worst"], stats["points"], note))
        chaos = f" chaotic={stats['chaotic']:.0%}" if stats.get("chaotic") else ""
        print(f"OK   {name}: n={template['n']} tokens={compiled['token_count']} "
              f"parity={stats['worst']:.1e}/{stats['points']}pts{chaos} {note}")

    print(f"\n=== ported {len(ported)} / skipped {len(skipped)} ===")
    for name, reason in skipped:
        print(f"SKIP {name}: {reason}")


if __name__ == "__main__":
    main()
