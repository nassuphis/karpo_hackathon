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
POLY100 = os.path.join(LAMBDA_DIR, "poly100.py")
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
ROW_REL_TOL = 1e-6        # secondary gate: error vs row max (wire floor ~6e-8)
PARITY_MIN_POINTS = 5     # points that must survive the f32-range filter
F32_MAX = 3.0e38


class SkipFunction(Exception):
    """Raise with a reason when a function does not fit the template."""


# ---------------------------------------------------------------------------
# AST extraction
# ---------------------------------------------------------------------------

def load_functions():
    src = open(POLY100, "r", encoding="utf-8").read()
    tree = ast.parse(src)
    out = {}
    for node in tree.body:
        if isinstance(node, ast.FunctionDef) and node.name.startswith("poly_"):
            out[node.name] = (node, ast.get_source_segment(src, node))
    return src, out


def _unwrap_try(fn):
    body = fn.body
    if len(body) == 1 and isinstance(body[0], ast.Try):
        return body[0].body
    return body


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


def _slot_of_target(node, kvar=None):
    """cf[<int>] -> ('const', i); cf[k+c]/cf[k-c]/cf[k] -> ('loop', off)."""
    if not (isinstance(node, ast.Subscript) and isinstance(node.value, ast.Name)
            and node.value.id == "cf"):
        return None
    idx = node.slice
    c = _const_int(idx)
    if c is not None:
        return ("const", c)
    if kvar is None:
        return None
    if isinstance(idx, ast.Name) and idx.id == kvar:
        return ("loop", 0)
    if isinstance(idx, ast.BinOp) and isinstance(idx.left, ast.Name) and idx.left.id == kvar:
        c = _const_int(idx.right)
        if c is None:
            return None
        if isinstance(idx.op, ast.Add):
            return ("loop", c)
        if isinstance(idx.op, ast.Sub):
            return ("loop", -c)
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
    """Rewrite cf[k+prev_off] -> __prev__ and k -> __scan_k__."""

    def __init__(self, kvar, prev_off):
        self.kvar = kvar
        self.prev_off = prev_off
        self.bad_read = False

    def visit_Subscript(self, node):
        # Classify against the pristine index BEFORE renaming k inside it.
        if isinstance(node.value, ast.Name) and node.value.id == "cf":
            slot = _slot_of_target(node, self.kvar)
            if slot == ("loop", self.prev_off):
                return ast.copy_location(ast.Name(id="__prev__", ctx=ast.Load()), node)
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
    combines with `incoming`. Returns (off, target_node, value) or None."""
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


def _try_scan_template(kvar, lo, hi, off, value, ordered):
    """Recognize cf[k+off] = f(cf[k+off-1], k) and build a scan entry.

    The recurrence's first read slot (lo+off-1) must have been poked before
    the loop; that poke becomes the scan's init expression and is consumed.
    Returns ("scan", slot_lo, slot_hi, length, k0, init_text, step_text).
    """
    rewriter = _ScanRewrite(kvar, off - 1)
    step_node = ast.fix_missing_locations(rewriter.visit(value))
    if rewriter.bad_read or _reads_cf(step_node):
        return None
    init_slot = lo + off - 1
    init_expr = None
    for pos, op in enumerate(ordered):
        if op[0] == "poke" and op[1] == init_slot:
            init_expr = op[2]
    if init_expr is None or _reads_cf(init_expr):
        return None
    ordered[:] = [op for op in ordered if not (op[0] == "poke" and op[1] == init_slot)]
    init_text = scalar_src(init_expr)
    step_text = scalar_src(step_node)
    return ("scan", init_slot, hi + off, hi - lo + 1, lo - 1, init_text, step_text)


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
                ordered.append(("aug_slice", bounds[0], bounds[1], opname, value))
                continue
            raise SkipFunction("unsupported AugAssign target")
        if isinstance(stmt, ast.Assign) and len(stmt.targets) == 1:
            tgt = stmt.targets[0]
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
                raise SkipFunction("cf assigned from something besides np.zeros")
            if isinstance(tgt, ast.Name):
                env[tgt.id] = value
                continue
            slot = _slot_of_target(tgt)
            if slot and slot[0] == "const":
                wrote_any = bool(segments) or any(o[0] == "scan" for o in ordered)
                if not wrote_any and _reads_cf(value) and not _contains_vector_shape(value):
                    raise SkipFunction("pre-segment poke reads cf (order-sensitive)")
                _append_poke(ordered, slot[1], value)
                continue
            slice_bounds = _slice_bounds(tgt)
            if slice_bounds is not None and _reads_cf(value):
                # In-place slice rewrite (cf[a:b] = f(cf[a:b])): reads the
                # current poly, so it stays an ordered post-op.
                ordered.append(("slice_assign", slice_bounds[0], slice_bounds[1], value))
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
            rargs = [_const_int(a) for a in stmt.iter.args]
            if any(a is None for a in rargs) or len(rargs) not in (1, 2):
                raise SkipFunction("range() args are not integer literals")
            lo, hi = (0, rargs[0]) if len(rargs) == 1 else rargs
            if not isinstance(stmt.target, ast.Name):
                raise SkipFunction("destructuring loop target")
            kvar = stmt.target.id
            loop_env = dict(env)
            write = None
            const_writes = []   # cf[<int>] = expr inside the loop
            body_stmts = list(stmt.body)
            if body_stmts and isinstance(body_stmts[-1], ast.If):
                incoming = None
                prefix = body_stmts[:-1]
                if prefix and isinstance(prefix[-1], ast.Assign) and len(prefix[-1].targets) == 1:
                    prior_slot = _slot_of_target(prefix[-1].targets[0], kvar)
                    if prior_slot and prior_slot[0] == "loop":
                        incoming = _InlineTemps(loop_env).visit(prefix[-1].value)
                        prefix = prefix[:-1]
                folded = _fold_if_to_select(body_stmts[-1], kvar, loop_env, incoming)
                if folded is not None:
                    body_stmts = prefix + [folded]
            for inner in body_stmts:
                if not (isinstance(inner, ast.Assign) and len(inner.targets) == 1):
                    raise SkipFunction("loop body statement is not an assignment")
                itgt = inner.targets[0]
                ival = _InlineTemps(loop_env).visit(inner.value)
                if isinstance(itgt, ast.Name):
                    loop_env[itgt.id] = ival
                    continue
                slot = _slot_of_target(itgt, kvar)
                if slot and slot[0] == "loop":
                    if write is not None:
                        raise SkipFunction("loop writes more than one cf slot")
                    write = (slot[1], ival)
                    continue
                if slot and slot[0] == "const":
                    # cf[<int>] = expr re-assigned every iteration: only the
                    # final iteration survives, so it is a post-loop poke
                    # with k substituted by the last loop value. cf reads are
                    # fine — after the loop they see exactly the final state.
                    const_writes.append((slot[1], ival))
                    continue
                raise SkipFunction("loop writes a non-loop-indexed slot")
            if write is None:
                raise SkipFunction("loop does not write cf")
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
            segments.append((kvar, lo, hi, write[0], write[1]))
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
            if any(slot in _seg_slots(s) for s in later) or \
               any(o[1] <= slot < o[2] for o in later_scans):
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
            return f"(-{scalar_src(node.operand, kvar)})"
        if isinstance(node.op, ast.UAdd):
            return scalar_src(node.operand, kvar)
        raise SkipFunction("unsupported unary op")
    if isinstance(node, ast.BinOp):
        if isinstance(node.op, ast.Pow):
            e = node.right
            ev = e.value if isinstance(e, ast.Constant) else None
            if isinstance(ev, int) and 0 < abs(ev) <= 32:
                return f"({scalar_src(node.left, kvar)} ** {ev})"
            return f"power({scalar_src(node.left, kvar)}, {scalar_src(e, kvar)})"
        ops = {ast.Add: "+", ast.Sub: "-", ast.Mult: "*", ast.Div: "/"}
        op = ops.get(type(node.op))
        if op is None:
            raise SkipFunction(f"unsupported operator {type(node.op).__name__}")
        return f"({scalar_src(node.left, kvar)} {op} {scalar_src(node.right, kvar)})"
    raise SkipFunction(f"unsupported scalar node {type(node).__name__}")


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


def vec_src(node, kvar, k_range):
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
        lo, hi = k_range
        return f"range({lo}, {hi})"
    if not _contains_k(node, kvar) and not _contains_vector_shape(node):
        return scalar_src(node, kvar)
    if isinstance(node, ast.Call):
        if isinstance(node.func, ast.Name) and node.func.id == "__select__":
            c, a, b = (vec_src(arg, kvar, k_range) for arg in node.args)
            return f"select({c}, {a}, {b})"
        name = _call_name(node)
        if name and len(node.args) == 1 and not node.keywords:
            return f"{name}({vec_src(node.args[0], kvar, k_range)})"
        raise SkipFunction(f"unsupported vector call {ast.dump(node.func)[:50]}")
    if isinstance(node, ast.Attribute) and node.attr in ("real", "imag"):
        return f"{node.attr}({vec_src(node.value, kvar, k_range)})"
    if isinstance(node, ast.UnaryOp):
        if isinstance(node.op, ast.USub):
            return f"neg({vec_src(node.operand, kvar, k_range)})"
        if isinstance(node.op, ast.UAdd):
            return vec_src(node.operand, kvar, k_range)
        raise SkipFunction("unsupported unary op in vector context")
    if isinstance(node, ast.BinOp):
        ops = {ast.Add: "add", ast.Sub: "subtract", ast.Mult: "multiply",
               ast.Div: "divide", ast.Pow: "power", ast.Mod: "rem"}
        op = ops.get(type(node.op))
        if op is None:
            raise SkipFunction(f"unsupported operator {type(node.op).__name__}")
        return f"{op}({vec_src(node.left, kvar, k_range)}, {vec_src(node.right, kvar, k_range)})"
    if isinstance(node, ast.Compare):
        if len(node.ops) != 1 or len(node.comparators) != 1:
            raise SkipFunction("chained comparison")
        cmps = {ast.Eq: "eq", ast.Gt: "gt", ast.GtE: "ge", ast.Lt: "lt", ast.LtE: "le"}
        op = cmps.get(type(node.ops[0]))
        if op is None:
            raise SkipFunction(f"unsupported comparison {type(node.ops[0]).__name__}")
        return f"{op}({vec_src(node.left, kvar, k_range)}, {vec_src(node.comparators[0], kvar, k_range)})"
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
        stmts = []
        for sign, tnode in vec_terms:
            src_text = vec_src(tnode, kvar, (k_lo, k_hi))
            if sign < 0:
                src_text = f"neg({src_text})"
            frag = (ast.get_source_segment(module_src, tnode) or "").replace("\n", " ")
            stmts.append((src_text, frag[:70]))
    for src_text, frag in stmts:
        lines.append(src_text + (f"   # {frag}" if frag else ""))
    for _ in range(len(stmts) - 1):
        lines.append("add(pop, pop)")
    if sca_terms:
        lines.append(f"add(pop, ({_scalar_terms_text(sca_terms)}))")
    if masked:
        mask = _window_mask(slot_lo, slot_hi, n)
        if mask is not None:
            lines.append(f"multiply(pop, {mask})   # keep slots {slot_lo}..{slot_hi - 1}")
    return 1


def emit_program(name, template, module_src):
    """Assemble readable Coeff Program source text."""
    n = template["n"]
    segments = template["segments"]
    force_mask = template.get("force_mask", False)
    lines = [f"# {name} (poly100.py), ported by port_poly100_programs.py"]
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
            lines.append(f"poly[{slot}] = {scalar_src(expr)}" + (f"   # {frag[:70]}" if frag else ""))
        elif op[0] == "scan":
            _tag, a, b, length, k0, init_text, step_text = op
            lines.append(f"poly[{a}:{b}] = scan({length}, {k0}, {init_text}, {step_text})"
                         f"   # recurrence over slots {a}..{b - 1}")
        elif op[0] == "slice_assign":
            _tag, a, b, value = op
            frag = (ast.get_source_segment(module_src, value) or "").replace("\n", " ")
            lines.append(f"poly[{a}:{b}] = {vec_src(value, None, None)}"
                         + (f"   # {frag[:70]}" if frag else ""))
        elif op[0] == "reduce_push":
            _tag, fname, arg = op
            frag = (ast.get_source_segment(module_src, arg) or "").replace("\n", " ")
            lines.append(f"{fname}({vec_src(arg, None, None)})"
                         + (f"   # np.{fname}({frag[:60]})" if frag else ""))
        elif op[0] == "drop":
            lines.append("drop")
        else:
            _tag, a, b, opname, expr = op
            mask = _window_mask(a, b, n) or "1"
            frag = (ast.get_source_segment(module_src, expr) or "").replace("\n", " ")
            if opname == "multiply":
                # cf[a:b] *= X: factor is 1 outside the window, X inside
                lines.append(f"multiply(poly, add(1, multiply({mask}, (({scalar_src(expr)}) - 1))))"
                             + f"   # cf[{a}:{b}] *= {frag[:50]}")
            else:
                lines.append(f"{opname}(poly, multiply({mask}, ({scalar_src(expr)})))"
                             + f"   # cf[{a}:{b}] {'+' if opname == 'add' else '-'}= {frag[:50]}")
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
    worst_row = 0.0
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
            p1b = complex(float(np.nextafter(np.float32(p1q.real), np.float32(2.0))), p1q.imag)
            p2b = complex(p2q.real, float(np.nextafter(np.float32(p2q.imag), np.float32(2.0))))
            ref_b = np.asarray(py_fn(p1b, p2b), dtype=np.complex128)
        if len(ref) != n_coeffs:
            return None, f"python returned {len(ref)} coeffs, expected {n_coeffs}"
        finite = np.isfinite(ref.real) & np.isfinite(ref.imag)
        if not finite.all() or np.abs(ref).max() > F32_MAX:
            continue  # beyond f32 transport range at this point
        if not ref.any():
            continue  # except-branch zeros are ambiguous
        got = native[r * n_coeffs:(r + 1) * n_coeffs]
        stable = [abs(rb - rf) / max(abs(rf), 1.0) <= PARITY_REL_TOL
                  for rb, rf in zip(ref_b, ref)]
        chaotic_slots += stable.count(False)
        total_slots += n_coeffs
        pairs = [(g, rf) for (g, rf), ok in zip(zip(got, ref), stable) if ok]
        if not pairs:
            continue
        rel = max(abs(g - rf) / max(abs(rf), 1.0) for g, rf in pairs)
        # Secondary, row-relative measure: numpy computes integer powers by
        # repeated multiplication while the VM's c_powc goes through exp/log;
        # on ill-conditioned slots (huge power + catastrophic cancellation)
        # the phase jitter lands far above the slot's own value while staying
        # deep beneath the row's dynamic range. Score that case against the
        # row maximum instead of the slot.
        row_scale = max(float(np.abs(ref).max()), 1.0)
        row_rel = max(abs(g - rf) / row_scale for g, rf in pairs)
        worst = max(worst, rel)
        worst_row = max(worst_row, row_rel)
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
    if worst_row <= ROW_REL_TOL:
        return {**stats, "worst": worst_row, "gate": "row-rel"}, None
    return None, (f"parity failed: rel err {worst:.2e} "
                  f"(row-rel {worst_row:.2e}) over {valid} points")


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
    args = ap.parse_args()

    if not os.path.exists(SWEEP_TEST):
        sys.exit(f"sweep_test missing; build: cc -O2 -pthread -o {SWEEP_TEST} "
                 f"{os.path.join(LAMBDA_DIR, 'sweep_cli.c')} -lm")

    module_src, functions = load_functions()
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
            skipped.append((name, "not found in poly100.py"))
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
        gate = "" if stats.get("gate") == "strict" else f" [{stats.get('gate')}]"
        print(f"OK   {name}: n={template['n']} tokens={compiled['token_count']} "
              f"parity={stats['worst']:.1e}/{stats['points']}pts{gate} {note}")

    print(f"\n=== ported {len(ported)} / skipped {len(skipped)} ===")
    for name, reason in skipped:
        print(f"SKIP {name}: {reason}")


if __name__ == "__main__":
    main()
