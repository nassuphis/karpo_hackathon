#!/usr/bin/env python3
"""Transpile poly100.py functions from Python/numpy to C.

Generates C coefficient functions compatible with sweep_cli.c.
Uses the complex arithmetic helpers already defined there (c_mul, c_div, c_sin, etc.).

Usage: python3 transpile_poly.py > poly_generated.c
"""

import ast
import copy
import sys
import textwrap

# We parse each poly_N function, walk the AST, and emit C code.
# The Python functions use: np.sin, np.cos, np.log, np.abs, np.angle, np.real, np.imag,
# np.conj, np.exp, np.sqrt, np.sum, np.prod, sph_harm, array ops, loops, conditionals.

class TranspileError(Exception):
    pass

class CVar:
    """Represents a C variable (real part name, imag part name)."""
    _counter = 0

    @classmethod
    def fresh(cls, prefix="v"):
        cls._counter += 1
        return CVar(f"_{prefix}{cls._counter}r", f"_{prefix}{cls._counter}i")

    @classmethod
    def reset(cls):
        cls._counter = 0

    def __init__(self, real, imag):
        self.r = real
        self.i = imag


class PolyTranspiler(ast.NodeVisitor):
    def __init__(self, func_name, n_coeffs):
        self.func_name = func_name
        self.n_coeffs = n_coeffs
        self.lines = []
        self._scalar_t1t2 = False  # set True for t1,t2 = z[0].real, z[0].imag
        self.const_arrays = {}  # name -> [int, ...] for fancy indexing
        self.declared = set()
        self.array_sizes = {}  # name -> int (number of elements)
        self.arange_vars = {}  # name -> (start, stop) for arange loop variables
        self._loop_vars = set()  # variables declared as int in for-loops
        self.indent = 1

    def emit(self, line):
        self.lines.append("    " * self.indent + line)

    def declare(self, var):
        if var.r not in self.declared:
            self.declared.add(var.r)
            self.declared.add(var.i)
            self.emit(f"double {var.r} = 0, {var.i} = 0;")

    def declare_real(self, name):
        if name not in self.declared:
            self.declared.add(name)
            self.emit(f"double {name} = 0;")

    def expr_to_c(self, node):
        """Convert a Python expression AST node to a CVar (complex pair).
        May emit temporary variable declarations and assignments."""

        if isinstance(node, ast.Constant):
            v = node.value
            if isinstance(v, complex):
                tmp = CVar.fresh("c")
                self.declare(tmp)
                self.emit(f"{tmp.r} = {v.real}; {tmp.i} = {v.imag};")
                return tmp
            else:
                tmp = CVar.fresh("c")
                self.declare(tmp)
                self.emit(f"{tmp.r} = {float(v)}; {tmp.i} = 0;")
                return tmp

        if isinstance(node, ast.Name):
            name = node.id
            if name == "t1":
                if self._scalar_t1t2:
                    return CVar("t1", "0")
                return CVar("x1r", "x1i")
            elif name == "t2":
                if self._scalar_t1t2:
                    return CVar("t2", "0")
                return CVar("x2r", "x2i")
            elif name == "pi":
                return CVar("M_PI", "0")
            elif name == "_cf_elem":
                # Synthetic marker for cf[loop_var] inside arange loop
                return CVar("_cf_r", "_cf_i")
            else:
                # Check if this is a complex local
                cl = getattr(self, '_complex_locals', {})
                if name in cl:
                    return cl[name]
                # loop variable or other local — treat as real
                return CVar(name, "0")

        if isinstance(node, ast.Attribute):
            # np.pi, t1.real, t1.imag, t2.real, t2.imag, (expr).real, (expr).imag
            attr = node.attr
            if isinstance(node.value, ast.Name) and node.value.id == "np" and attr == "pi":
                return CVar("M_PI", "0")
            # t1.real, t1.imag, t2.real, t2.imag
            if attr in ("real", "imag"):
                val = self.expr_to_c(node.value)
                tmp = CVar.fresh("attr")
                self.declare(tmp)
                if attr == "real":
                    self.emit(f"{tmp.r} = {val.r}; {tmp.i} = 0;")
                else:
                    self.emit(f"{tmp.r} = {val.i}; {tmp.i} = 0;")
                return tmp

        if isinstance(node, ast.UnaryOp):
            if isinstance(node.op, ast.USub):
                operand = self.expr_to_c(node.operand)
                tmp = CVar.fresh("neg")
                self.declare(tmp)
                self.emit(f"{tmp.r} = -({operand.r}); {tmp.i} = -({operand.i});")
                return tmp
            elif isinstance(node.op, ast.UAdd):
                return self.expr_to_c(node.operand)

        if isinstance(node, ast.BinOp):
            return self.binop_to_c(node)

        if isinstance(node, ast.Call):
            return self.call_to_c(node)

        if isinstance(node, ast.Subscript):
            return self.subscript_to_c(node)

        if isinstance(node, ast.IfExp):
            # ternary: a if cond else b
            cond_c = self.cond_to_c(node.test)
            body = self.expr_to_c(node.body)
            orelse = self.expr_to_c(node.orelse)
            tmp = CVar.fresh("tern")
            self.declare(tmp)
            self.emit(f"if ({cond_c}) {{ {tmp.r} = {body.r}; {tmp.i} = {body.i}; }}")
            self.emit(f"else {{ {tmp.r} = {orelse.r}; {tmp.i} = {orelse.i}; }}")
            return tmp

        # Fallback: treat as zero
        tmp = CVar.fresh("unk")
        self.declare(tmp)
        self.emit(f"/* WARNING: unhandled node {ast.dump(node)} */")
        return tmp

    def cond_to_c(self, node):
        """Convert a condition to a C expression string."""
        if isinstance(node, ast.Compare):
            left = self.expr_to_c(node.left)
            comparators = [self.expr_to_c(c) for c in node.comparators]
            ops = node.ops
            parts = []
            prev = left
            for op, comp in zip(ops, comparators):
                if isinstance(op, ast.Eq):
                    parts.append(f"{prev.r} == {comp.r}")
                elif isinstance(op, ast.NotEq):
                    parts.append(f"{prev.r} != {comp.r}")
                elif isinstance(op, ast.Lt):
                    parts.append(f"{prev.r} < {comp.r}")
                elif isinstance(op, ast.LtE):
                    parts.append(f"{prev.r} <= {comp.r}")
                elif isinstance(op, ast.Gt):
                    parts.append(f"{prev.r} > {comp.r}")
                elif isinstance(op, ast.GtE):
                    parts.append(f"{prev.r} >= {comp.r}")
                prev = comp
            return " && ".join(parts)
        elif isinstance(node, ast.BoolOp):
            if isinstance(node.op, ast.And):
                return " && ".join(f"({self.cond_to_c(v)})" for v in node.values)
            else:
                return " || ".join(f"({self.cond_to_c(v)})" for v in node.values)
        elif isinstance(node, ast.UnaryOp) and isinstance(node.op, ast.Not):
            return f"!({self.cond_to_c(node.operand)})"
        else:
            v = self.expr_to_c(node)
            return f"({v.r} != 0 || {v.i} != 0)"

    def binop_to_c(self, node):
        left = self.expr_to_c(node.left)
        right = self.expr_to_c(node.right)

        if isinstance(node.op, ast.Add):
            tmp = CVar.fresh("add")
            self.declare(tmp)
            self.emit(f"{tmp.r} = {left.r} + {right.r}; {tmp.i} = {left.i} + {right.i};")
            return tmp
        elif isinstance(node.op, ast.Sub):
            tmp = CVar.fresh("sub")
            self.declare(tmp)
            self.emit(f"{tmp.r} = {left.r} - {right.r}; {tmp.i} = {left.i} - {right.i};")
            return tmp
        elif isinstance(node.op, ast.Mult):
            tmp = CVar.fresh("mul")
            self.declare(tmp)
            self.emit(f"c_mul({left.r}, {left.i}, {right.r}, {right.i}, &{tmp.r}, &{tmp.i});")
            return tmp
        elif isinstance(node.op, ast.Div):
            tmp = CVar.fresh("div")
            self.declare(tmp)
            self.emit(f"c_div({left.r}, {left.i}, {right.r}, {right.i}, &{tmp.r}, &{tmp.i});")
            return tmp
        elif isinstance(node.op, ast.FloorDiv):
            tmp = CVar.fresh("fdiv")
            self.declare(tmp)
            self.emit(f"c_div({left.r}, {left.i}, {right.r}, {right.i}, &{tmp.r}, &{tmp.i});")
            self.emit(f"{tmp.r} = floor({tmp.r}); {tmp.i} = 0;")
            return tmp
        elif isinstance(node.op, ast.Pow):
            tmp = CVar.fresh("pow")
            self.declare(tmp)
            # Check if right is a simple integer constant
            if isinstance(node.right, ast.Constant) and isinstance(node.right.value, (int, float)):
                p = node.right.value
                if p == 2:
                    self.emit(f"c_mul({left.r}, {left.i}, {left.r}, {left.i}, &{tmp.r}, &{tmp.i});")
                elif p == 3:
                    self.emit(f"c_mul({left.r}, {left.i}, {left.r}, {left.i}, &{tmp.r}, &{tmp.i});")
                    self.emit(f"c_mul({tmp.r}, {tmp.i}, {left.r}, {left.i}, &{tmp.r}, &{tmp.i});")
                elif p == 4:
                    self.emit(f"c_mul({left.r}, {left.i}, {left.r}, {left.i}, &{tmp.r}, &{tmp.i});")
                    self.emit(f"c_mul({tmp.r}, {tmp.i}, {tmp.r}, {tmp.i}, &{tmp.r}, &{tmp.i});")
                elif p == 0:
                    self.emit(f"{tmp.r} = 1.0; {tmp.i} = 0;")
                elif p == 1:
                    self.emit(f"{tmp.r} = {left.r}; {tmp.i} = {left.i};")
                elif isinstance(p, int) and p > 0 and p <= 50:
                    self.emit(f"c_powr({left.r}, {left.i}, {float(p)}, &{tmp.r}, &{tmp.i});")
                else:
                    self.emit(f"c_powr({left.r}, {left.i}, {float(p)}, &{tmp.r}, &{tmp.i});")
            else:
                # Power with complex exponent — use c_pow if imaginary part is nonzero
                if right.i in ("0", "0.0"):
                    self.emit(f"c_powr({left.r}, {left.i}, {right.r}, &{tmp.r}, &{tmp.i});")
                else:
                    self.emit(f"c_powc({left.r}, {left.i}, {right.r}, {right.i}, &{tmp.r}, &{tmp.i});")
            return tmp
        elif isinstance(node.op, ast.Mod):
            tmp = CVar.fresh("mod")
            self.declare(tmp)
            self.emit(f"{tmp.r} = fmod({left.r}, {right.r}); {tmp.i} = 0;")
            return tmp

        tmp = CVar.fresh("binop")
        self.declare(tmp)
        self.emit(f"/* WARNING: unhandled binop */")
        return tmp

    def call_to_c(self, node):
        func = node.func

        # Get function name
        if isinstance(func, ast.Attribute):
            obj = func.value
            attr = func.attr

            if isinstance(obj, ast.Name) and obj.id == "np":
                return self.numpy_call(attr, node.args)
            elif isinstance(obj, ast.Name) and obj.id == "math":
                return self.math_call(attr, node.args)
            elif isinstance(obj, ast.Name) and obj.id == "cmath":
                return self.cmath_call(attr, node.args)
        elif isinstance(func, ast.Name):
            if func.id == "abs":
                arg = self.expr_to_c(node.args[0])
                tmp = CVar.fresh("abs")
                self.declare(tmp)
                self.emit(f"{tmp.r} = c_abs({arg.r}, {arg.i}); {tmp.i} = 0;")
                return tmp
            elif func.id == "complex":
                if len(node.args) == 2:
                    re = self.expr_to_c(node.args[0])
                    im = self.expr_to_c(node.args[1])
                    tmp = CVar.fresh("cplx")
                    self.declare(tmp)
                    self.emit(f"{tmp.r} = {re.r}; {tmp.i} = {im.r};")
                    return tmp
                elif node.keywords:
                    re_val = im_val = None
                    for kw in node.keywords:
                        if kw.arg == "real": re_val = self.expr_to_c(kw.value)
                        elif kw.arg == "imag": im_val = self.expr_to_c(kw.value)
                    tmp = CVar.fresh("cplx")
                    self.declare(tmp)
                    self.emit(f"{tmp.r} = {re_val.r if re_val else '0'}; {tmp.i} = {im_val.r if im_val else '0'};")
                    return tmp
            elif func.id == "max":
                if len(node.args) == 2:
                    a = self.expr_to_c(node.args[0])
                    b = self.expr_to_c(node.args[1])
                    tmp = CVar.fresh("max")
                    self.declare(tmp)
                    self.emit(f"{tmp.r} = fmax({a.r}, {b.r}); {tmp.i} = 0;")
                    return tmp
            elif func.id == "min":
                if len(node.args) == 2:
                    a = self.expr_to_c(node.args[0])
                    b = self.expr_to_c(node.args[1])
                    tmp = CVar.fresh("min")
                    self.declare(tmp)
                    self.emit(f"{tmp.r} = fmin({a.r}, {b.r}); {tmp.i} = 0;")
                    return tmp
            elif func.id == "range":
                pass  # handled in for loop
            elif func.id == "int":
                arg = self.expr_to_c(node.args[0])
                tmp = CVar.fresh("int")
                self.declare(tmp)
                self.emit(f"{tmp.r} = (int)({arg.r}); {tmp.i} = 0;")
                return tmp
            elif func.id == "float":
                arg = self.expr_to_c(node.args[0])
                tmp = CVar.fresh("flt")
                self.declare(tmp)
                self.emit(f"{tmp.r} = (double)({arg.r}); {tmp.i} = 0;")
                return tmp
            elif func.id == "len":
                if node.args and isinstance(node.args[0], ast.Name):
                    name = node.args[0].id
                    tmp = CVar.fresh("len")
                    self.declare(tmp)
                    if name == "cf":
                        self.emit(f"{tmp.r} = {self.n_coeffs}; {tmp.i} = 0;")
                    else:
                        sz = self.array_sizes.get(name)
                        if sz is not None:
                            self.emit(f"{tmp.r} = {sz}; {tmp.i} = 0;")
                        else:
                            self.emit(f"/* WARNING: len({name}) unknown */")
                    return tmp
                tmp = CVar.fresh("len")
                self.declare(tmp)
                self.emit(f"/* WARNING: len() not directly translatable */")
                return tmp
            elif func.id == "sum":
                # sum(cf[a:b]) or sum(iterable) — try np.sum path
                if node.args and isinstance(node.args[0], ast.Subscript):
                    return self.numpy_sum_slice(node.args[0])
                arg = self.expr_to_c(node.args[0])
                return arg

        tmp = CVar.fresh("call")
        self.declare(tmp)
        self.emit(f"/* WARNING: unhandled call {ast.dump(func)} */")
        return tmp

    def numpy_call(self, attr, args):
        if attr == "sin":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("sin")
            self.declare(tmp)
            self.emit(f"c_sin({arg.r}, {arg.i}, &{tmp.r}, &{tmp.i});")
            return tmp
        elif attr == "cos":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("cos")
            self.declare(tmp)
            self.emit(f"c_cos({arg.r}, {arg.i}, &{tmp.r}, &{tmp.i});")
            return tmp
        elif attr == "log":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("log")
            self.declare(tmp)
            self.emit(f"c_log({arg.r}, {arg.i}, &{tmp.r}, &{tmp.i});")
            return tmp
        elif attr == "exp":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("exp")
            self.declare(tmp)
            self.emit(f"c_exp2({arg.r}, {arg.i}, &{tmp.r}, &{tmp.i});")
            return tmp
        elif attr == "sqrt":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("sqrt")
            self.declare(tmp)
            self.emit(f"c_powr({arg.r}, {arg.i}, 0.5, &{tmp.r}, &{tmp.i});")
            return tmp
        elif attr == "abs":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("abs")
            self.declare(tmp)
            self.emit(f"{tmp.r} = c_abs({arg.r}, {arg.i}); {tmp.i} = 0;")
            return tmp
        elif attr == "angle":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("ang")
            self.declare(tmp)
            self.emit(f"{tmp.r} = c_arg({arg.r}, {arg.i}); {tmp.i} = 0;")
            return tmp
        elif attr == "real":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("re")
            self.declare(tmp)
            self.emit(f"{tmp.r} = {arg.r}; {tmp.i} = 0;")
            return tmp
        elif attr == "imag":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("im")
            self.declare(tmp)
            self.emit(f"{tmp.r} = {arg.i}; {tmp.i} = 0;")
            return tmp
        elif attr == "conj" or attr == "conjugate":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("conj")
            self.declare(tmp)
            self.emit(f"{tmp.r} = {arg.r}; {tmp.i} = -({arg.i});")
            return tmp
        elif attr == "isfinite":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("fin")
            self.declare(tmp)
            self.emit(f"{tmp.r} = (isfinite({arg.r}) && isfinite({arg.i})) ? 1.0 : 0.0; {tmp.i} = 0;")
            return tmp
        elif attr == "zeros":
            # just return a zero
            return CVar("0", "0")
        elif attr == "array":
            # Constant array — we handle at assignment site
            return CVar("0", "0")
        elif attr == "arange":
            return CVar("0", "0")
        elif attr == "sum":
            # np.sum(cf[a:b]) or np.sum(cf)
            if len(args) == 1 and isinstance(args[0], ast.Subscript):
                return self.numpy_sum_slice(args[0])
            if len(args) == 1 and isinstance(args[0], ast.Name) and args[0].id == "cf":
                tmp = CVar.fresh("sum")
                self.declare(tmp)
                self.emit(f"{tmp.r} = 0; {tmp.i} = 0;")
                self.emit(f"for (int _si = 0; _si < {self.n_coeffs}; _si++) {{ {tmp.r} += cRe[_si]; {tmp.i} += cIm[_si]; }}")
                return tmp
            # Handle np.sum(np.array([...])) or np.sum([...])
            elts = self._extract_list_or_array_elts(args[0]) if args else None
            if elts is not None and len(elts) > 0:
                result = self.expr_to_c(elts[0])
                for elt in elts[1:]:
                    val = self.expr_to_c(elt)
                    tmp = CVar.fresh("sum")
                    self.declare(tmp)
                    self.emit(f"{tmp.r} = {result.r} + {val.r}; {tmp.i} = {result.i} + {val.i};")
                    result = tmp
                return result
            arg = self.expr_to_c(args[0])
            return arg
        elif attr == "prod":
            if len(args) == 1 and isinstance(args[0], ast.Subscript):
                return self.numpy_prod_slice(args[0])
            # Handle np.prod(np.array([expr1, expr2, ...])) or np.prod([expr1, expr2, ...])
            elts = self._extract_list_or_array_elts(args[0]) if args else None
            if elts is not None and len(elts) > 0:
                result = self.expr_to_c(elts[0])
                for elt in elts[1:]:
                    val = self.expr_to_c(elt)
                    tmp = CVar.fresh("prod")
                    self.declare(tmp)
                    self.emit(f"c_mul({result.r}, {result.i}, {val.r}, {val.i}, &{tmp.r}, &{tmp.i});")
                    result = tmp
                return result
            arg = self.expr_to_c(args[0])
            return arg
        elif attr == "sign":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("sgn")
            self.declare(tmp)
            self.emit(f"{tmp.r} = ({arg.r} > 0) ? 1.0 : ({arg.r} < 0) ? -1.0 : 0.0; {tmp.i} = 0;")
            return tmp
        elif attr == "tanh":
            # tanh(z) = sinh(z)/cosh(z), approximate for complex
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("tanh")
            self.declare(tmp)
            self.emit(f"{tmp.r} = tanh({arg.r}); {tmp.i} = 0; /* approx real tanh */")
            return tmp
        elif attr == "sinh":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("sinh")
            self.declare(tmp)
            self.emit(f"{tmp.r} = sinh({arg.r}); {tmp.i} = 0; /* approx real sinh */")
            return tmp
        elif attr == "cosh":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("cosh")
            self.declare(tmp)
            self.emit(f"{tmp.r} = cosh({arg.r}); {tmp.i} = 0; /* approx real cosh */")
            return tmp
        elif attr == "tan":
            # tan(z) = sin(z)/cos(z)
            arg = self.expr_to_c(args[0])
            s = CVar.fresh("tsin")
            c = CVar.fresh("tcos")
            tmp = CVar.fresh("tan")
            self.declare(s)
            self.declare(c)
            self.declare(tmp)
            self.emit(f"c_sin({arg.r}, {arg.i}, &{s.r}, &{s.i});")
            self.emit(f"c_cos({arg.r}, {arg.i}, &{c.r}, &{c.i});")
            self.emit(f"c_div({s.r}, {s.i}, {c.r}, {c.i}, &{tmp.r}, &{tmp.i});")
            return tmp
        elif attr == "floor":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("flr")
            self.declare(tmp)
            self.emit(f"{tmp.r} = floor({arg.r}); {tmp.i} = 0;")
            return tmp
        elif attr == "ceil":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("ceil")
            self.declare(tmp)
            self.emit(f"{tmp.r} = ceil({arg.r}); {tmp.i} = 0;")
            return tmp
        elif attr == "clip":
            arg = self.expr_to_c(args[0])
            lo = self.expr_to_c(args[1])
            hi = self.expr_to_c(args[2])
            tmp = CVar.fresh("clip")
            self.declare(tmp)
            self.emit(f"{tmp.r} = fmin(fmax({arg.r}, {lo.r}), {hi.r}); {tmp.i} = 0;")
            return tmp
        elif attr == "maximum":
            a = self.expr_to_c(args[0])
            b = self.expr_to_c(args[1])
            tmp = CVar.fresh("max")
            self.declare(tmp)
            self.emit(f"{tmp.r} = fmax({a.r}, {b.r}); {tmp.i} = 0;")
            return tmp
        elif attr == "minimum":
            a = self.expr_to_c(args[0])
            b = self.expr_to_c(args[1])
            tmp = CVar.fresh("min")
            self.declare(tmp)
            self.emit(f"{tmp.r} = fmin({a.r}, {b.r}); {tmp.i} = 0;")
            return tmp
        elif attr == "arctan2":
            a = self.expr_to_c(args[0])
            b = self.expr_to_c(args[1])
            tmp = CVar.fresh("at2")
            self.declare(tmp)
            self.emit(f"{tmp.r} = atan2({a.r}, {b.r}); {tmp.i} = 0;")
            return tmp
        elif attr == "conjugate":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("conj")
            self.declare(tmp)
            self.emit(f"{tmp.r} = {arg.r}; {tmp.i} = -({arg.i});")
            return tmp

        # Fallback
        tmp = CVar.fresh("np")
        self.declare(tmp)
        self.emit(f"/* WARNING: unhandled np.{attr} */")
        return tmp

    def math_call(self, attr, args):
        if attr in ("sin", "cos", "tan", "exp", "log", "sqrt", "tanh", "sinh", "cosh",
                     "atan2", "fabs", "floor", "ceil"):
            if attr == "atan2":
                a = self.expr_to_c(args[0])
                b = self.expr_to_c(args[1])
                tmp = CVar.fresh("at2")
                self.declare(tmp)
                self.emit(f"{tmp.r} = atan2({a.r}, {b.r}); {tmp.i} = 0;")
                return tmp
            else:
                arg = self.expr_to_c(args[0])
                tmp = CVar.fresh(attr[:3])
                self.declare(tmp)
                self.emit(f"{tmp.r} = {attr}({arg.r}); {tmp.i} = 0;")
                return tmp
        tmp = CVar.fresh("math")
        self.declare(tmp)
        self.emit(f"/* WARNING: unhandled math.{attr} */")
        return tmp

    def cmath_call(self, attr, args):
        return self.numpy_call(attr, args)

    def numpy_sum_slice(self, subscript_node):
        """Handle np.sum(cf[a:b])"""
        if isinstance(subscript_node.value, ast.Name) and subscript_node.value.id == "cf":
            sl = subscript_node.slice
            if isinstance(sl, ast.Slice):
                lo = self.get_int(sl.lower) if sl.lower else 0
                hi = self.get_int(sl.upper) if sl.upper else self.n_coeffs
                if lo is not None and hi is not None:
                    tmp = CVar.fresh("sum")
                    self.declare(tmp)
                    self.emit(f"{tmp.r} = 0; {tmp.i} = 0;")
                    self.emit(f"for (int _si = {lo}; _si < {hi}; _si++) {{ {tmp.r} += cRe[_si]; {tmp.i} += cIm[_si]; }}")
                    return tmp
                else:
                    # Dynamic bounds
                    lo_c = f"({self.expr_to_c(sl.lower).r})" if sl.lower else "0"
                    hi_c = f"({self.expr_to_c(sl.upper).r})" if sl.upper else str(self.n_coeffs)
                    tmp = CVar.fresh("sum")
                    self.declare(tmp)
                    self.emit(f"{tmp.r} = 0; {tmp.i} = 0;")
                    self.emit(f"for (int _si = (int){lo_c}; _si < (int){hi_c} && _si < {self.n_coeffs}; _si++) {{ {tmp.r} += cRe[_si]; {tmp.i} += cIm[_si]; }}")
                    return tmp
        # fallback
        tmp = CVar.fresh("sum")
        self.declare(tmp)
        self.emit(f"/* WARNING: unhandled np.sum */")
        return tmp

    def numpy_prod_slice(self, subscript_node):
        """Handle np.prod(cf[a:b])"""
        if isinstance(subscript_node.value, ast.Name) and subscript_node.value.id == "cf":
            sl = subscript_node.slice
            if isinstance(sl, ast.Slice):
                lo = self.get_int(sl.lower) if sl.lower else 0
                hi = self.get_int(sl.upper) if sl.upper else self.n_coeffs
                if lo is not None and hi is not None:
                    tmp = CVar.fresh("prod")
                    self.declare(tmp)
                    self.emit(f"{tmp.r} = 1; {tmp.i} = 0;")
                    self.emit(f"for (int _pi = {lo}; _pi < {hi}; _pi++) {{ double _pr = {tmp.r}*cRe[_pi]-{tmp.i}*cIm[_pi]; double _pp = {tmp.r}*cIm[_pi]+{tmp.i}*cRe[_pi]; {tmp.r}=_pr; {tmp.i}=_pp; }}")
                    return tmp
        tmp = CVar.fresh("prod")
        self.declare(tmp)
        self.emit(f"/* WARNING: unhandled np.prod */")
        return tmp

    def is_np_computed_array(self, node):
        """Check if node is a numpy computed array like np.cumsum(np.arange(1, N))."""
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute):
            if isinstance(node.func.value, ast.Name) and node.func.value.id == "np":
                if node.func.attr in ("cumsum", "arange", "linspace", "zeros", "ones"):
                    return True
        return False

    def eval_computed_array(self, node):
        """Try to evaluate a numpy array expression at transpile time."""
        import numpy as np
        try:
            code = ast.unparse(node)
            result = eval(code, {"np": np, "math": __import__("math")})
            return [float(x) for x in result]
        except Exception:
            return None

    def _is_np_zeros(self, node):
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute):
            return node.func.attr == "zeros" and isinstance(node.func.value, ast.Name) and node.func.value.id == "np"
        return False

    def _is_np_empty(self, node):
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute):
            return node.func.attr == "empty" and isinstance(node.func.value, ast.Name) and node.func.value.id == "np"
        return False

    def _is_vector_expr(self, node):
        """Check if expression is composed of known-length arrays and scalars."""
        if isinstance(node, ast.Name):
            return node.id in self.array_sizes
        if isinstance(node, ast.BinOp):
            return self._is_vector_expr(node.left) or self._is_vector_expr(node.right)
        if isinstance(node, ast.UnaryOp):
            return self._is_vector_expr(node.operand)
        return False

    def _lower_vector_assign(self, rhs_node):
        """Lower cf = <vector expr> into an elementwise loop."""
        n = self.n_coeffs
        self.emit(f"for (int _vi = 0; _vi < {n}; _vi++) {{")
        self.indent += 1
        val = self._expr_to_c_vector(rhs_node, "_vi")
        self.emit(f"cRe[_vi] = {val.r}; cIm[_vi] = {val.i};")
        self.indent -= 1
        self.emit(f"}}")

    def _expr_to_c_vector(self, node, idx_var):
        """Evaluate a vector expression elementwise at idx_var."""
        if isinstance(node, ast.Name) and node.id in self.array_sizes:
            return CVar(f"{node.id}[{idx_var}]", "0")
        if isinstance(node, ast.BinOp):
            left = self._expr_to_c_vector(node.left, idx_var) if self._is_vector_expr(node.left) else self.expr_to_c(node.left)
            right = self._expr_to_c_vector(node.right, idx_var) if self._is_vector_expr(node.right) else self.expr_to_c(node.right)
            tmp = CVar.fresh("vec")
            self.declare(tmp)
            if isinstance(node.op, ast.Add):
                self.emit(f"{tmp.r} = {left.r} + {right.r}; {tmp.i} = {left.i} + {right.i};")
            elif isinstance(node.op, ast.Sub):
                self.emit(f"{tmp.r} = {left.r} - {right.r}; {tmp.i} = {left.i} - {right.i};")
            elif isinstance(node.op, ast.Mult):
                self.emit(f"c_mul({left.r}, {left.i}, {right.r}, {right.i}, &{tmp.r}, &{tmp.i});")
            elif isinstance(node.op, ast.Pow):
                self.emit(f"c_powr({left.r}, {left.i}, {right.r}, &{tmp.r}, &{tmp.i});")
            elif isinstance(node.op, ast.Div):
                self.emit(f"c_div({left.r}, {left.i}, {right.r}, {right.i}, &{tmp.r}, &{tmp.i});")
            else:
                self.emit(f"/* WARNING: unhandled vector binop {type(node.op).__name__} */")
                self.emit(f"{tmp.r} = 0; {tmp.i} = 0;")
            return tmp
        # Scalar fallback
        return self.expr_to_c(node)

    def _extract_sequence_elements(self, node):
        """Extract element AST nodes from np.array([...]), [...], or (...)."""
        if isinstance(node, (ast.List, ast.Tuple)):
            return node.elts
        return self.extract_np_array_elements(node)

    def is_np_array_literal(self, node):
        """Check if node is np.array([...]) with constant elements."""
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute):
            if isinstance(node.func.value, ast.Name) and node.func.value.id == "np" and node.func.attr == "array":
                if node.args and isinstance(node.args[0], ast.List):
                    return all(isinstance(e, (ast.Constant, ast.UnaryOp)) for e in node.args[0].elts)
        return False

    def extract_array_elements(self, node):
        """Extract constant elements from np.array([...])."""
        if isinstance(node, ast.Call) and node.args and isinstance(node.args[0], ast.List):
            result = []
            for e in node.args[0].elts:
                if isinstance(e, ast.Constant):
                    result.append(e.value)
                elif isinstance(e, ast.UnaryOp) and isinstance(e.op, ast.USub) and isinstance(e.operand, ast.Constant):
                    result.append(-e.operand.value)
                else:
                    return None
            return result
        return None

    def get_int(self, node):
        """Try to extract a constant integer."""
        if isinstance(node, ast.Constant):
            return int(node.value)
        if isinstance(node, ast.UnaryOp) and isinstance(node.op, ast.USub) and isinstance(node.operand, ast.Constant):
            return -int(node.operand.value)
        return None

    def subscript_to_c(self, node):
        """Handle cf[k], cf[k-1], array[idx], etc."""
        if isinstance(node.value, ast.Name):
            name = node.value.id
            if name == "cf":
                idx = self.index_expr(node.slice)
                tmp = CVar.fresh("cf")
                self.declare(tmp)
                self.emit(f"{{ int _idx = {idx}; if (_idx >= 0 && _idx < {self.n_coeffs}) {{ {tmp.r} = cRe[_idx]; {tmp.i} = cIm[_idx]; }} }}")
                return tmp
            else:
                # Some local array — bounds-checked real constant lookup
                idx = self.index_expr(node.slice)
                tmp = CVar.fresh("arr")
                self.declare(tmp)
                sz = self.array_sizes.get(name)
                if sz is not None:
                    self.emit(f"{{ int _idx = {idx}; {tmp.r} = (_idx >= 0 && _idx < {sz}) ? {name}[_idx] : 0.0; {tmp.i} = 0; }}")
                else:
                    self.emit(f"{tmp.r} = {name}[{idx}]; {tmp.i} = 0;")
                return tmp

        tmp = CVar.fresh("sub")
        self.declare(tmp)
        self.emit(f"/* WARNING: unhandled subscript {ast.dump(node)} */")
        return tmp

    def index_expr(self, node):
        """Convert an index expression to a C expression string (integer context)."""
        if isinstance(node, ast.Constant):
            return str(int(node.value))
        if isinstance(node, ast.Name):
            name = node.id
            # Check if this is a complex local — use real part for indexing
            cl = getattr(self, '_complex_locals', {})
            if name in cl:
                return f"(int)({cl[name].r})"
            # Loop variables (declared via for-loop) are int; other locals are double and need cast
            if name in self.declared and name not in self._loop_vars:
                return f"(int)({name})"
            return name
        if isinstance(node, ast.BinOp):
            left = self.index_expr(node.left)
            right = self.index_expr(node.right)
            if isinstance(node.op, ast.Add):
                return f"({left} + {right})"
            elif isinstance(node.op, ast.Sub):
                return f"({left} - {right})"
            elif isinstance(node.op, ast.Mult):
                return f"({left} * {right})"
            elif isinstance(node.op, ast.Mod):
                return f"({left} % {right})"
            elif isinstance(node.op, ast.FloorDiv):
                return f"({left} / {right})"
        if isinstance(node, ast.UnaryOp) and isinstance(node.op, ast.USub):
            operand = self.index_expr(node.operand)
            return f"(-{operand})"
        # len(cf) -> n_coeffs, len(other) -> sizeof trick
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Name) and node.func.id == "len":
            if node.args and isinstance(node.args[0], ast.Name):
                name = node.args[0].id
                if name == "cf":
                    return str(self.n_coeffs)
                sz = self.array_sizes.get(name)
                if sz is not None:
                    return str(sz)
        return f"(int)({self.expr_to_c(node).r})"

    def assign_cf(self, idx_node, value_node):
        """Emit cf[idx] = value (bounds-checked).
        Handles fancy indexing: cf[np.array([i1,i2,...])] = np.array([v1,v2,...])
        or cf[np.array([...])] = scalar."""
        # Fancy index: cf[np.array([0, 9, 19])] = np.array([1, 2, -3])
        # Also handle cf[i] where i was assigned as np.array([...]) — look up static arrays
        idx_elts = self.extract_np_array_int_elements(idx_node)
        if idx_elts is None and isinstance(idx_node, ast.Name):
            # Look up structurally stored const array
            idx_elts = self.const_arrays.get(idx_node.id)
        if idx_elts is not None:
            val_elts = self._extract_sequence_elements(value_node)
            if val_elts is not None and len(val_elts) == len(idx_elts):
                # Parallel assignment: cf[indices] = values
                for idx_val, val_node in zip(idx_elts, val_elts):
                    val = self.expr_to_c(val_node)
                    self.emit(f"{{ int _idx = {idx_val}; if (_idx >= 0 && _idx < {self.n_coeffs}) {{ cRe[_idx] = {val.r}; cIm[_idx] = {val.i}; }} }}")
                return
            elif val_elts is not None and len(val_elts) != len(idx_elts):
                # Try as a flat list of constants
                pass
            # Scalar broadcast: cf[indices] = scalar
            val = self.expr_to_c(value_node)
            for idx_val in idx_elts:
                self.emit(f"{{ int _idx = {idx_val}; if (_idx >= 0 && _idx < {self.n_coeffs}) {{ cRe[_idx] = {val.r}; cIm[_idx] = {val.i}; }} }}")
            return

        idx = self.index_expr(idx_node)
        val = self.expr_to_c(value_node)
        self.emit(f"{{ int _idx = {idx}; if (_idx >= 0 && _idx < {self.n_coeffs}) {{ cRe[_idx] = {val.r}; cIm[_idx] = {val.i}; }} }}")

    def extract_np_array_int_elements(self, node):
        """Extract integer elements from np.array([1, 2, 3]), [1,2,3], or (1,2,3) for fancy indexing."""
        elts = None
        if isinstance(node, (ast.List, ast.Tuple)):
            elts = node.elts
        elif isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute) and node.func.attr == "array":
            if node.args and isinstance(node.args[0], ast.List):
                elts = node.args[0].elts
        if elts is None:
            return None
        try:
            result = []
            for e in elts:
                if isinstance(e, ast.Constant):
                    result.append(int(e.value))
                elif isinstance(e, ast.UnaryOp) and isinstance(e.op, ast.USub) and isinstance(e.operand, ast.Constant):
                    result.append(-int(e.operand.value))
                else:
                    return None
            return result
        except (ValueError, AttributeError):
            return None

    def assign_cf_slice(self, sl, value_node):
        """Handle cf[a:b] = expr involving arrays."""
        lo = self.get_int(sl.lower) if sl.lower else 0
        hi = self.get_int(sl.upper) if sl.upper else self.n_coeffs
        if lo is None or hi is None:
            self.emit(f"/* WARNING: dynamic slice assignment */")
            return
        count = hi - lo

        # Fix 3: np.array([expr1, expr2, ...]) — unroll element-by-element
        arr_elts = self.extract_np_array_elements(value_node)
        if arr_elts is not None:
            for i, elt_node in enumerate(arr_elts):
                if i >= count:
                    break
                val = self.expr_to_c(elt_node)
                self.emit(f"cRe[{lo + i}] = {val.r}; cIm[{lo + i}] = {val.i};")
            return

        # Fix 3b: np.array([...]) * expr or expr * np.array([...]) — unroll with multiplication
        arr_elts_binop = self.extract_np_array_binop(value_node)
        if arr_elts_binop is not None:
            elts, other_node, op, arr_is_left = arr_elts_binop
            other = self.expr_to_c(other_node)
            for i, elt_node in enumerate(elts):
                if i >= count:
                    break
                elt = self.expr_to_c(elt_node)
                left_v, right_v = (elt, other) if arr_is_left else (other, elt)
                tmp = CVar.fresh("abop")
                self.declare(tmp)
                if isinstance(op, ast.Mult):
                    self.emit(f"c_mul({left_v.r}, {left_v.i}, {right_v.r}, {right_v.i}, &{tmp.r}, &{tmp.i});")
                elif isinstance(op, ast.Add):
                    self.emit(f"{tmp.r} = {left_v.r} + {right_v.r}; {tmp.i} = {left_v.i} + {right_v.i};")
                elif isinstance(op, ast.Sub):
                    self.emit(f"{tmp.r} = {left_v.r} - {right_v.r}; {tmp.i} = {left_v.i} - {right_v.i};")
                elif isinstance(op, ast.Div):
                    self.emit(f"c_div({left_v.r}, {left_v.i}, {right_v.r}, {right_v.i}, &{tmp.r}, &{tmp.i});")
                else:
                    self.emit(f"c_mul({left_v.r}, {left_v.i}, {right_v.r}, {right_v.i}, &{tmp.r}, &{tmp.i});")
                self.emit(f"cRe[{lo + i}] = {tmp.r}; cIm[{lo + i}] = {tmp.i};")
            return

        # Fix 4: List comprehension — [expr for var in range(start, end)]
        if isinstance(value_node, ast.ListComp):
            comp = self.transpile_listcomp_slice(value_node, lo, count)
            if comp:
                return

        # Detect np.arange(start, end) * expr  or  array * expr
        # and emit element-wise loop with index
        self.emit(f"for (int _si = 0; _si < {count}; _si++) {{")
        self.indent += 1
        # Inject _si_val = _si + offset for arange patterns
        self.emit(f"int _si_idx = _si + {lo};")
        val = self.expr_to_c_slice(value_node, "_si", lo, count)
        self.emit(f"cRe[_si_idx] = {val.r}; cIm[_si_idx] = {val.i};")
        self.indent -= 1
        self.emit(f"}}")

    def extract_np_array_elements(self, node):
        """Extract element AST nodes from np.array([...]) — works with computed elements too."""
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute):
            if (isinstance(node.func.value, ast.Name) and node.func.value.id == "np"
                    and node.func.attr == "array"):
                if node.args and isinstance(node.args[0], ast.List):
                    return node.args[0].elts
        return None

    def extract_np_array_binop(self, node):
        """Detect np.array([...]) * expr or expr * np.array([...]).
        Returns (elements, other_node, op, arr_is_left) or None."""
        if not isinstance(node, ast.BinOp):
            return None
        # Check left side
        left_elts = self.extract_np_array_elements(node.left)
        if left_elts is not None:
            return (left_elts, node.right, node.op, True)
        # Check right side
        right_elts = self.extract_np_array_elements(node.right)
        if right_elts is not None:
            return (right_elts, node.left, node.op, False)
        return None

    def transpile_listcomp_slice(self, node, lo, count):
        """Transpile [expr for var in range(start, end)] as cf[lo:lo+count] = ..."""
        if len(node.generators) != 1:
            return False
        gen = node.generators[0]
        if not isinstance(gen.target, ast.Name):
            return False
        var = gen.target.id
        if not (isinstance(gen.iter, ast.Call) and isinstance(gen.iter.func, ast.Name)
                and gen.iter.func.id == "range"):
            return False
        args = gen.iter.args
        if len(args) == 1:
            r_start, r_end = 0, self.get_int(args[0])
        elif len(args) >= 2:
            r_start = self.get_int(args[0]) or 0
            r_end = self.get_int(args[1])
        else:
            return False
        if r_end is None:
            return False
        r_step = self.get_int(args[2]) if len(args) > 2 else 1

        self.declared.add(var)
        self.emit(f"{{ int _lc_i = 0;")
        self.emit(f"for (int {var} = {r_start}; {var} < {r_end}; {var} += {r_step}) {{")
        self.indent += 1
        val = self.expr_to_c(node.elt)
        self.emit(f"if (_lc_i < {count}) {{ cRe[{lo} + _lc_i] = {val.r}; cIm[{lo} + _lc_i] = {val.i}; }}")
        self.emit(f"_lc_i++;")
        self.indent -= 1
        self.emit(f"}} }}")
        return True

    def expr_to_c_slice(self, node, loop_var, lo, count):
        """Like expr_to_c but handles arange and array references element-wise."""
        # np.arange(a, b) -> (_si + a) as real
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute):
            if isinstance(node.func.value, ast.Name) and node.func.value.id == "np" and node.func.attr == "arange":
                start = 0
                if node.args:
                    start = self.get_int(node.args[0]) or 0
                tmp = CVar.fresh("arange")
                self.declare(tmp)
                self.emit(f"{tmp.r} = (double)({loop_var} + {start}); {tmp.i} = 0;")
                return tmp

        # BinOp: array * expr or np.arange(...) * expr
        if isinstance(node, ast.BinOp):
            # Check if either side is arange or a local array
            left_is_indexable = self.is_indexable_in_slice(node.left)
            right_is_indexable = self.is_indexable_in_slice(node.right)

            if left_is_indexable or right_is_indexable:
                left = self.expr_to_c_slice(node.left, loop_var, lo, count) if left_is_indexable else self.expr_to_c(node.left)
                right = self.expr_to_c_slice(node.right, loop_var, lo, count) if right_is_indexable else self.expr_to_c(node.right)
                # Re-do the binop
                if isinstance(node.op, ast.Mult):
                    tmp = CVar.fresh("smul")
                    self.declare(tmp)
                    self.emit(f"c_mul({left.r}, {left.i}, {right.r}, {right.i}, &{tmp.r}, &{tmp.i});")
                    return tmp
                elif isinstance(node.op, ast.Add):
                    tmp = CVar.fresh("sadd")
                    self.declare(tmp)
                    self.emit(f"{tmp.r} = {left.r} + {right.r}; {tmp.i} = {left.i} + {right.i};")
                    return tmp
                elif isinstance(node.op, ast.Sub):
                    tmp = CVar.fresh("ssub")
                    self.declare(tmp)
                    self.emit(f"{tmp.r} = {left.r} - {right.r}; {tmp.i} = {left.i} - {right.i};")
                    return tmp
                elif isinstance(node.op, ast.Div):
                    tmp = CVar.fresh("sdiv")
                    self.declare(tmp)
                    self.emit(f"c_div({left.r}, {left.i}, {right.r}, {right.i}, &{tmp.r}, &{tmp.i});")
                    return tmp

        # Check if node itself is a local array name
        if isinstance(node, ast.Name) and node.id in self.declared and node.id not in ("t1", "t2", "x1r", "x1i", "x2r", "x2i"):
            # Subscript the array with bounds check
            tmp = CVar.fresh("elem")
            self.declare(tmp)
            sz = self.array_sizes.get(node.id)
            if sz is not None:
                self.emit(f"{{ int _idx = {loop_var}; {tmp.r} = (_idx >= 0 && _idx < {sz}) ? {node.id}[_idx] : 0.0; {tmp.i} = 0; }}")
            else:
                self.emit(f"{tmp.r} = {node.id}[{loop_var}]; {tmp.i} = 0;")
            return tmp

        # Fallback to normal expression
        return self.expr_to_c(node)

    def is_np_arange_fancy(self, node):
        """Check if node is np.arange(start, stop, step) used as fancy index."""
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute):
            if isinstance(node.func.value, ast.Name) and node.func.value.id == "np" and node.func.attr == "arange":
                return len(node.args) >= 2
        return False

    def extract_arange_params(self, node):
        """Extract (start, stop, step) from np.arange(start, stop[, step])."""
        args = node.args
        start = self.get_int(args[0]) or 0
        stop = self.get_int(args[1]) if len(args) > 1 else self.n_coeffs
        step = self.get_int(args[2]) if len(args) > 2 else 1
        return start, stop, step

    def is_indexable_in_slice(self, node):
        """Check if an expression should be indexed element-wise in a slice context."""
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute):
            if isinstance(node.func.value, ast.Name) and node.func.value.id == "np" and node.func.attr == "arange":
                return True
        if isinstance(node, ast.Name) and node.id not in ("t1", "t2", "cf", "pi"):
            return True
        if isinstance(node, ast.BinOp):
            return self.is_indexable_in_slice(node.left) or self.is_indexable_in_slice(node.right)
        return False

    def process_stmt(self, stmt):
        """Process a single statement."""
        if isinstance(stmt, ast.Assign):
            target = stmt.targets[0]
            if isinstance(target, ast.Subscript):
                if isinstance(target.value, ast.Name) and target.value.id == "cf":
                    sl = target.slice
                    if isinstance(sl, ast.Slice):
                        self.assign_cf_slice(sl, stmt.value)
                    else:
                        self.assign_cf(sl, stmt.value)
                else:
                    # Local variable subscript assignment
                    name = target.value.id if isinstance(target.value, ast.Name) else "arr"
                    idx = self.index_expr(target.slice)
                    val = self.expr_to_c(stmt.value)
                    self.emit(f"/* {name}[{idx}] assignment — skipped (local array) */")
            elif isinstance(target, ast.Name):
                name = target.id
                if name == "cf":
                    if self._is_np_zeros(stmt.value) or self._is_np_empty(stmt.value):
                        pass  # skip — already zero-initialized
                    elif self._is_vector_expr(stmt.value):
                        # cf = <vector expression> — emit elementwise loop
                        self._lower_vector_assign(stmt.value)
                    else:
                        pass  # skip other cf assignments (np.zeros variant, etc.)
                elif self.is_np_array_literal(stmt.value):
                    # np.array([1, 2, 3]) -> static const double name[] = {1, 2, 3};
                    elts = self.extract_array_elements(stmt.value)
                    if elts is not None:
                        vals = ", ".join(str(float(v)) for v in elts)
                        self.declared.add(name)
                        self.array_sizes[name] = len(elts)
                        # Store int values for fancy indexing
                        try:
                            self.const_arrays[name] = [int(v) for v in elts]
                        except (ValueError, TypeError):
                            pass
                        self.emit(f"static const double {name}[] = {{{vals}}};")
                    else:
                        self.declared.add(name)
                        self.emit(f"/* WARNING: could not extract np.array for {name} */")
                        self.emit(f"static const double {name}[] = {{0}};")
                elif self._is_np_linspace(stmt.value):
                    # np.linspace(start, end, n) -> double name[n]; for loop
                    start_expr, end_expr, n_val = self._extract_linspace_args(stmt.value)
                    if start_expr is not None and n_val is not None:
                        self.declared.add(name)
                        self.array_sizes[name] = n_val
                        start_var = self.expr_to_c(start_expr)
                        end_var = self.expr_to_c(end_expr)
                        self.emit(f"double {name}[{n_val}];")
                        if n_val > 1:
                            self.emit(f"for (int _li = 0; _li < {n_val}; _li++) {{")
                            self.emit(f"    {name}[_li] = {start_var.r} + ({end_var.r} - {start_var.r}) * _li / {n_val - 1}.0;")
                            self.emit(f"}}")
                        else:
                            self.emit(f"{name}[0] = {start_var.r};")
                elif self.is_np_computed_array(stmt.value):
                    # Check if this is a simple np.arange — track as loop variable
                    arange_info = self._is_arange_assign(stmt)
                    if arange_info:
                        _, astart, astop = arange_info
                        self.arange_vars[name] = (astart, astop)
                        self.declared.add(name)
                        # Don't emit static array — will be used as loop variable
                        self.emit(f"/* {name} = np.arange({astart}, {astop}) — loop variable */")
                    else:
                        # np.cumsum(np.arange(1, N)) etc.
                        arr = self.eval_computed_array(stmt.value)
                        if arr is not None:
                            vals = ", ".join(str(float(v)) for v in arr)
                            self.declared.add(name)
                            self.array_sizes[name] = len(arr)
                            self.emit(f"static const double {name}[] = {{{vals}}};")
                        else:
                            self.declared.add(name)
                            self.emit(f"static const double {name}[] = {{0}}; /* WARNING: could not evaluate */")
                elif name in ("t1", "t2"):
                    val = self.expr_to_c(stmt.value)
                    if name == "t1":
                        self.emit(f"x1r = {val.r}; x1i = {val.i};")
                    else:
                        self.emit(f"x2r = {val.r}; x2i = {val.i};")
                else:
                    # Local variable — check if pre-resolved as constant
                    const_val = self._const_locals.get(name) if hasattr(self, '_const_locals') else None
                    if const_val is not None and name not in self.declared:
                        self.declared.add(name)
                        self.emit(f"double {name} = {float(const_val)};")
                    else:
                        # Pre-declare if RHS references the same variable (self-referencing assignment)
                        if name not in self.declared and self._name_in_expr(name, stmt.value):
                            self.declared.add(name)
                            # Declare as complex pair for self-referencing vars
                            nr, ni = f"{name}_r", f"{name}_i"
                            self.declared.add(nr)
                            self.declared.add(ni)
                            self.emit(f"double {nr} = 0, {ni} = 0;")
                            self._complex_locals = getattr(self, '_complex_locals', {})
                            self._complex_locals[name] = CVar(nr, ni)
                        val = self.expr_to_c(stmt.value)
                        if name not in self.declared:
                            self.declared.add(name)
                            # If imaginary part is trivially zero, keep as plain real
                            is_real = (val.i == "0" or val.i == "0.0")
                            if is_real:
                                self.emit(f"double {name} = {val.r};")
                            else:
                                # Store as complex pair (re + im)
                                nr, ni = f"{name}_r", f"{name}_i"
                                self.declared.add(nr)
                                self.declared.add(ni)
                                self.emit(f"double {nr} = {val.r}, {ni} = {val.i};")
                                self._complex_locals = getattr(self, '_complex_locals', {})
                                self._complex_locals[name] = CVar(nr, ni)
                        else:
                            cl = getattr(self, '_complex_locals', {})
                            if name in cl:
                                cv = cl[name]
                                self.emit(f"{cv.r} = {val.r}; {cv.i} = {val.i};")
                            else:
                                self.emit(f"{name} = {val.r};")
            elif isinstance(target, ast.Tuple):
                # Tuple unpacking — handle simple cases
                if isinstance(stmt.value, ast.Tuple):
                    # Skip "t1, t2 = z[0], z[1]" — these are already mapped to x1r/x2r
                    # Also handle "t1, t2 = z[0].real, z[0].imag" — map to x1r, x1i
                    names = [t.id for t in target.elts if isinstance(t, ast.Name)]
                    if set(names) == {"t1", "t2"}:
                        # Check if it's z[0].real, z[0].imag pattern
                        vals = stmt.value.elts
                        is_real_imag = (
                            len(vals) == 2 and
                            all(isinstance(v, ast.Attribute) and v.attr in ("real", "imag") for v in vals)
                        )
                        if is_real_imag:
                            # t1, t2 = z[0].real, z[0].imag → map t1=x1r, t2=x1i
                            # These functions use t1/t2 as real scalars, not complex
                            self.emit("/* t1, t2 = z[0].real, z[0].imag — scalar-from-first-input */")
                            self.emit("double t1 = x1r;")
                            self.emit("double t2 = x1i;")
                            self.declared.add("t1")
                            self.declared.add("t2")
                            # Override: when we see t1/t2 in expressions, use scalar vars
                            self._scalar_t1t2 = True
                        else:
                            pass  # standard z[0], z[1] — t1/t2 already mapped to x1r/x2r
                    else:
                        for tgt, val in zip(target.elts, stmt.value.elts):
                            if isinstance(tgt, ast.Name):
                                v = self.expr_to_c(val)
                                if tgt.id not in self.declared:
                                    self.declared.add(tgt.id)
                                    self.emit(f"double {tgt.id} = {v.r};")
                                else:
                                    self.emit(f"{tgt.id} = {v.r};")

        elif isinstance(stmt, ast.AugAssign):
            target = stmt.target
            if isinstance(target, ast.Subscript):
                if isinstance(target.value, ast.Name) and target.value.id == "cf":
                    sl = target.slice
                    if isinstance(sl, ast.Slice):
                        # cf[a:b] += expr — handle as element-wise loop
                        lo = self.get_int(sl.lower) if sl.lower else 0
                        hi = self.get_int(sl.upper) if sl.upper else self.n_coeffs
                        count = hi - lo
                        self.emit(f"for (int _si = 0; _si < {count}; _si++) {{")
                        self.indent += 1
                        self.emit(f"int _si_idx = _si + {lo};")
                        val = self.expr_to_c_slice(stmt.value, "_si", lo, count)
                        if isinstance(stmt.op, ast.Add):
                            self.emit(f"cRe[_si_idx] += {val.r}; cIm[_si_idx] += {val.i};")
                        elif isinstance(stmt.op, ast.Mult):
                            self.emit(f"{{ double _tr = cRe[_si_idx]*{val.r} - cIm[_si_idx]*{val.i}; cIm[_si_idx] = cRe[_si_idx]*{val.i} + cIm[_si_idx]*{val.r}; cRe[_si_idx] = _tr; }}")
                        self.indent -= 1
                        self.emit(f"}}")
                    elif self.is_np_arange_fancy(sl):
                        # cf[np.arange(start, stop, step)] *= expr — strided loop
                        start, stop, step = self.extract_arange_params(sl)
                        val = self.expr_to_c(stmt.value)
                        self.emit(f"for (int _fi = {start}; _fi < {stop}; _fi += {step}) {{")
                        self.indent += 1
                        if isinstance(stmt.op, ast.Mult):
                            self.emit(f"{{ double _tr = cRe[_fi]*{val.r} - cIm[_fi]*{val.i}; cIm[_fi] = cRe[_fi]*{val.i} + cIm[_fi]*{val.r}; cRe[_fi] = _tr; }}")
                        elif isinstance(stmt.op, ast.Add):
                            self.emit(f"cRe[_fi] += {val.r}; cIm[_fi] += {val.i};")
                        elif isinstance(stmt.op, ast.Sub):
                            self.emit(f"cRe[_fi] -= {val.r}; cIm[_fi] -= {val.i};")
                        self.indent -= 1
                        self.emit(f"}}")
                    else:
                        idx = self.index_expr(sl)
                        val = self.expr_to_c(stmt.value)
                        if isinstance(stmt.op, ast.Add):
                            self.emit(f"cRe[{idx}] += {val.r}; cIm[{idx}] += {val.i};")
                        elif isinstance(stmt.op, ast.Sub):
                            self.emit(f"cRe[{idx}] -= {val.r}; cIm[{idx}] -= {val.i};")
                        elif isinstance(stmt.op, ast.Mult):
                            self.emit(f"{{ double _tr = cRe[{idx}]*{val.r} - cIm[{idx}]*{val.i}; cIm[{idx}] = cRe[{idx}]*{val.i} + cIm[{idx}]*{val.r}; cRe[{idx}] = _tr; }}")
            elif isinstance(target, ast.Name):
                name = target.id
                val = self.expr_to_c(stmt.value)
                if isinstance(stmt.op, ast.Add):
                    self.emit(f"{name} += {val.r};")
                elif isinstance(stmt.op, ast.Sub):
                    self.emit(f"{name} -= {val.r};")
                elif isinstance(stmt.op, ast.Mult):
                    self.emit(f"{name} *= {val.r};")

        elif isinstance(stmt, ast.For):
            self.process_for(stmt)

        elif isinstance(stmt, ast.If):
            self._predeclare_if_vars(stmt)
            self.process_if(stmt)

        elif isinstance(stmt, ast.Return):
            # Return statement — set nCoeffs and return
            pass  # handled at function level

        elif isinstance(stmt, ast.Try):
            # try/except — process the try body
            for s in stmt.body:
                self.process_stmt(s)

        elif isinstance(stmt, ast.Expr):
            # Standalone expression — skip
            pass

    def get_int_or_expr(self, node):
        """Get integer constant or generate a C expression string."""
        val = self.get_int(node)
        if val is not None:
            return str(val)
        # len(x) → sizeof(x)/sizeof(x[0]) for arrays, or nCoeffs for cf
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Name) and node.func.id == "len":
            if node.args and isinstance(node.args[0], ast.Name):
                name = node.args[0].id
                if name == "cf":
                    return str(self.n_coeffs)
                return f"(int)(sizeof({name})/sizeof({name}[0]))"
        # Fall back to evaluating the expression
        v = self.expr_to_c(node)
        return f"(int)({v.r})"

    def process_for(self, stmt):
        """Handle for loops."""
        if isinstance(stmt.target, ast.Name):
            var = stmt.target.id
            if isinstance(stmt.iter, ast.Call) and isinstance(stmt.iter.func, ast.Name) and stmt.iter.func.id == "range":
                args = stmt.iter.args
                if len(args) == 1:
                    hi = self.get_int_or_expr(args[0])
                    self.emit(f"for (int {var} = 0; {var} < {hi}; {var}++) {{")
                elif len(args) == 2:
                    lo = self.get_int_or_expr(args[0])
                    hi = self.get_int_or_expr(args[1])
                    self.emit(f"for (int {var} = {lo}; {var} < {hi}; {var}++) {{")
                elif len(args) == 3:
                    lo = self.get_int_or_expr(args[0])
                    hi = self.get_int_or_expr(args[1])
                    step = self.get_int_or_expr(args[2])
                    self.emit(f"for (int {var} = {lo}; {var} < {hi}; {var} += {step}) {{")
                else:
                    self.emit(f"/* WARNING: unhandled range args */")
                    return

                self.declared.add(var)
                self._loop_vars.add(var)
                self.indent += 1
                for s in stmt.body:
                    self.process_stmt(s)
                self.indent -= 1
                self.emit("}")
                return

        self.emit(f"/* WARNING: unhandled for loop */")

    def process_if(self, stmt):
        """Handle if statements."""
        cond = self.cond_to_c(stmt.test)
        self.emit(f"if ({cond}) {{")
        self.indent += 1
        for s in stmt.body:
            self.process_stmt(s)
        self.indent -= 1
        if stmt.orelse:
            self.emit("} else {")
            self.indent += 1
            for s in stmt.orelse:
                self.process_stmt(s)
            self.indent -= 1
        self.emit("}")

    # --- Arange loop lowering helpers ---

    def _find_arange_var_in_expr(self, node):
        """Find if any arange variable is referenced in this expression tree."""
        for child in ast.walk(node):
            if isinstance(child, ast.Name) and child.id in self.arange_vars:
                return child.id
        return None

    def _name_in_expr(self, name, node):
        """Check if a variable name appears anywhere in an expression AST."""
        for child in ast.walk(node):
            if isinstance(child, ast.Name) and child.id == name:
                return True
        return False

    def _predeclare_if_vars(self, if_node):
        """Pre-declare variables that are first assigned inside if/elif/else branches."""
        assigned = set()
        def scan_branch(stmts):
            for s in stmts:
                if isinstance(s, ast.Assign) and len(s.targets) == 1 and isinstance(s.targets[0], ast.Name):
                    name = s.targets[0].id
                    if name not in self.declared and name != "cf":
                        assigned.add(name)
                elif isinstance(s, ast.If):
                    scan_branch(s.body)
                    scan_branch(s.orelse)
        scan_branch(if_node.body)
        scan_branch(if_node.orelse)
        for name in sorted(assigned):
            self.declared.add(name)
            self.emit(f"double {name} = 0;")

    def _is_np_zeros(self, node):
        """Check if node is np.zeros(...)."""
        return (isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute) and
                isinstance(node.func.value, ast.Name) and node.func.value.id == "np" and
                node.func.attr == "zeros")

    def _is_np_linspace(self, node):
        return (isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute) and
                isinstance(node.func.value, ast.Name) and node.func.value.id == "np" and
                node.func.attr == "linspace")

    def _extract_linspace_args(self, node):
        """Returns (start_node, end_node, n_int) or (None, None, None)."""
        # Get n from positional arg[2] or keyword num=
        n_node = None
        if len(node.args) >= 3:
            n_node = node.args[2]
        else:
            for kw in (node.keywords or []):
                if kw.arg == "num":
                    n_node = kw.value
                    break
        if n_node is not None and len(node.args) >= 2:
            n = self._eval_const_expr(n_node)
            if n is not None:
                return node.args[0], node.args[1], n
        return None, None, None

    def _resolve_const_name(self, name):
        """Try to resolve a variable name to its constant integer value from the function body."""
        if hasattr(self, '_const_locals') and name in self._const_locals:
            return self._const_locals[name]
        return None

    def _eval_const_expr(self, node):
        """Try to evaluate an AST node to a constant integer using known locals."""
        if isinstance(node, ast.Constant) and isinstance(node.value, (int, float)):
            return int(node.value)
        if isinstance(node, ast.Name):
            return self._resolve_const_name(node.id)
        if isinstance(node, ast.BinOp):
            left = self._eval_const_expr(node.left)
            right = self._eval_const_expr(node.right)
            if left is not None and right is not None:
                if isinstance(node.op, ast.Add): return left + right
                if isinstance(node.op, ast.Sub): return left - right
                if isinstance(node.op, ast.Mult): return left * right
                if isinstance(node.op, ast.FloorDiv): return left // right
        if isinstance(node, ast.UnaryOp) and isinstance(node.op, ast.USub):
            val = self._eval_const_expr(node.operand)
            if val is not None: return -val
        return None

    def _extract_list_or_array_elts(self, node):
        """Extract element AST nodes from a list literal or np.array([...])."""
        if isinstance(node, ast.List):
            return node.elts
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Attribute):
            if (isinstance(node.func.value, ast.Name) and node.func.value.id == "np" and
                    node.func.attr == "array" and node.args and isinstance(node.args[0], ast.List)):
                return node.args[0].elts
        return None

    def _is_arange_assign(self, stmt):
        """Check if stmt is `var = np.arange(...)` and return (name, start, stop) or None."""
        if not isinstance(stmt, ast.Assign):
            return None
        target = stmt.targets[0]
        if not isinstance(target, ast.Name):
            return None
        if not (isinstance(stmt.value, ast.Call) and isinstance(stmt.value.func, ast.Attribute)):
            return None
        func = stmt.value.func
        if not (isinstance(func.value, ast.Name) and func.value.id == "np" and func.attr == "arange"):
            return None
        args = stmt.value.args
        if len(args) == 1:
            n = self.get_int(args[0])
            if n is not None:
                return (target.id, 0, n)
        elif len(args) == 2:
            start = self.get_int(args[0])
            stop = self.get_int(args[1])
            if start is not None and stop is not None:
                return (target.id, start, stop)
        return None

    def _stmt_references_arange(self, stmt):
        """Check if any part of a statement references an arange variable."""
        for child in ast.walk(stmt):
            if isinstance(child, ast.Name) and child.id in self.arange_vars:
                return True
        return False

    def _is_cf_uses_cf_rhs(self, stmt):
        """Check if stmt is `cf = cf * expr` or `cf = expr * cf` (uses cf on RHS)."""
        if not isinstance(stmt, ast.Assign):
            return False
        target = stmt.targets[0]
        if not (isinstance(target, ast.Name) and target.id == "cf"):
            return False
        for child in ast.walk(stmt.value):
            if isinstance(child, ast.Name) and child.id == "cf":
                return True
        return False

    def _emit_cf_assign_in_loop(self, cf_idx, stmt_value):
        """Emit cRe[cf_idx] = ...; cIm[cf_idx] = ...; for a cf = expr statement."""
        val = self.expr_to_c(stmt_value)
        self.emit(f"if ({cf_idx} < {self.n_coeffs}) {{ cRe[{cf_idx}] = {val.r}; cIm[{cf_idx}] = {val.i}; }}")

    def _emit_cf_augassign_in_loop(self, cf_idx, op, stmt_value):
        """Emit augmented assignment cRe[cf_idx] += ...; etc."""
        val = self.expr_to_c(stmt_value)
        if isinstance(op, ast.Add):
            self.emit(f"if ({cf_idx} < {self.n_coeffs}) {{ cRe[{cf_idx}] += {val.r}; cIm[{cf_idx}] += {val.i}; }}")
        elif isinstance(op, ast.Sub):
            self.emit(f"if ({cf_idx} < {self.n_coeffs}) {{ cRe[{cf_idx}] -= {val.r}; cIm[{cf_idx}] -= {val.i}; }}")
        elif isinstance(op, ast.Mult):
            self.emit(f"if ({cf_idx} < {self.n_coeffs}) {{ double _tr = cRe[{cf_idx}]*{val.r} - cIm[{cf_idx}]*{val.i}; cIm[{cf_idx}] = cRe[{cf_idx}]*{val.i} + cIm[{cf_idx}]*{val.r}; cRe[{cf_idx}] = _tr; }}")

    def _emit_cf_selfmul_in_loop(self, cf_idx, rhs_node):
        """Emit cf = cf * expr as augmented multiply in loop.
        rhs_node is the full RHS, which contains cf references."""
        # Replace cf references with cRe[cf_idx]/cIm[cf_idx] by setting up a temp
        # that holds the current cf[cf_idx] value
        self.emit(f"if ({cf_idx} < {self.n_coeffs}) {{")
        self.indent += 1
        # Save current cf value into temps
        self.emit(f"double _cf_r = cRe[{cf_idx}], _cf_i = cIm[{cf_idx}];")
        # Temporarily make 'cf' resolve to the per-element value
        old_declared = 'cf' in self.declared
        self.declared.add('cf')
        # We need expr_to_c to treat bare 'cf' as _cf_r + _cf_i*i
        # Since expr_to_c for Name returns CVar(name, "0"), we need a workaround:
        # Replace 'cf' Name nodes in the AST with a synthetic expression
        rhs_copy = copy.deepcopy(rhs_node)
        self._replace_cf_name(rhs_copy)
        val = self.expr_to_c(rhs_copy)
        self.emit(f"cRe[{cf_idx}] = {val.r}; cIm[{cf_idx}] = {val.i};")
        if not old_declared:
            self.declared.discard('cf')
        self.indent -= 1
        self.emit(f"}}")

    def _replace_cf_name(self, node):
        """Replace all ast.Name('cf') nodes in-place with ast.Name('_cf_r') hack.
        We use a special marker that expr_to_c handles."""
        for field, value in ast.iter_fields(node):
            if isinstance(value, list):
                for i, item in enumerate(value):
                    if isinstance(item, ast.AST):
                        if isinstance(item, ast.Name) and item.id == 'cf':
                            item.id = '_cf_elem'
                        else:
                            self._replace_cf_name(item)
            elif isinstance(value, ast.AST):
                if isinstance(value, ast.Name) and value.id == 'cf':
                    value.id = '_cf_elem'
                else:
                    self._replace_cf_name(value)

    def process_arange_block(self, stmts):
        """Process a block of statements that use an arange loop variable.
        Emits a single for-loop containing all the statements."""
        # Find which arange var is used
        arange_var = None
        for s in stmts:
            for child in ast.walk(s):
                if isinstance(child, ast.Name) and child.id in self.arange_vars:
                    arange_var = child.id
                    break
            if arange_var:
                break
        if not arange_var:
            for s in stmts:
                self.process_stmt(s)
            return

        start, stop = self.arange_vars[arange_var]
        # cf index: for np.arange(0, N), cf_idx = loop_var; for np.arange(start, stop), cf_idx = loop_var - start
        cf_idx = arange_var if start == 0 else f"({arange_var} - {start})"
        self.emit(f"for (int {arange_var} = {start}; {arange_var} < {stop}; {arange_var}++) {{")
        self.indent += 1
        self.declared.add(arange_var)

        for s in stmts:
            if isinstance(s, ast.Assign):
                target = s.targets[0]
                if isinstance(target, ast.Name) and target.id == "cf":
                    if self._is_np_zeros(s.value):
                        pass  # skip, already zero
                    elif self._is_cf_uses_cf_rhs(s):
                        self._emit_cf_selfmul_in_loop(cf_idx, s.value)
                    else:
                        self._emit_cf_assign_in_loop(cf_idx, s.value)
                elif isinstance(target, ast.Name):
                    # Local variable inside loop (e.g., real_part = expr)
                    name = target.id
                    val = self.expr_to_c(s.value)
                    if name not in self.declared:
                        self.declared.add(name)
                        self.emit(f"double {name} = {val.r}; double {name}_im = {val.i};")
                    else:
                        self.emit(f"{name} = {val.r}; {name}_im = {val.i};")
                else:
                    self.process_stmt(s)
            elif isinstance(s, ast.AugAssign):
                target = s.target
                if isinstance(target, ast.Name) and target.id == "cf":
                    self._emit_cf_augassign_in_loop(cf_idx, s.op, s.value)
                else:
                    self.process_stmt(s)
            else:
                self.process_stmt(s)

        self.indent -= 1
        self.emit("}")


def extract_ncoeffs(func_node):
    """Extract the size of the cf array from np.zeros(N, ...) or np.arange(N)."""
    # First resolve any local variable assignments like n = 25
    local_vars = {}
    for stmt in ast.walk(func_node):
        if isinstance(stmt, ast.Assign):
            if (isinstance(stmt.targets[0], ast.Name) and
                    isinstance(stmt.value, ast.Constant) and
                    isinstance(stmt.value.value, (int, float))):
                local_vars[stmt.targets[0].id] = int(stmt.value.value)

    def _resolve_int(node):
        """Resolve a node to an integer, following local variable references."""
        if isinstance(node, ast.Constant) and isinstance(node.value, (int, float)):
            return int(node.value)
        if isinstance(node, ast.Name) and node.id in local_vars:
            return local_vars[node.id]
        return None

    # Also track array sizes from linspace/etc for return-shape fallback
    array_sizes = {}

    # Try np.zeros/np.empty/np.ones(N, ...) assigned to cf
    for stmt in ast.walk(func_node):
        if isinstance(stmt, ast.Assign):
            tgt = stmt.targets[0]
            if isinstance(stmt.value, ast.Call) and isinstance(stmt.value.func, ast.Attribute):
                attr = stmt.value.func.attr
                if isinstance(stmt.value.func.value, ast.Name) and stmt.value.func.value.id == "np":
                    # Track any array constructor size
                    if attr in ("zeros", "empty", "ones", "full", "linspace"):
                        sz_arg = stmt.value.args[2] if attr == "linspace" and len(stmt.value.args) >= 3 else (stmt.value.args[0] if stmt.value.args else None)
                        if sz_arg:
                            sz = _resolve_int(sz_arg)
                            if sz is not None and isinstance(tgt, ast.Name):
                                array_sizes[tgt.id] = sz
                    # Direct cf assignment
                    if isinstance(tgt, ast.Name) and tgt.id == "cf":
                        if attr in ("zeros", "empty", "ones", "full"):
                            args = stmt.value.args
                            if args:
                                val = _resolve_int(args[0])
                                if val is not None:
                                    return val

    # Check if cf is assigned from a vector expression over known-sized arrays
    for stmt in ast.walk(func_node):
        if isinstance(stmt, ast.Assign):
            tgt = stmt.targets[0]
            if isinstance(tgt, ast.Name) and tgt.id == "cf":
                # cf = <expr> — try to find array size from participating names
                for name_node in ast.walk(stmt.value):
                    if isinstance(name_node, ast.Name) and name_node.id in array_sizes:
                        return array_sizes[name_node.id]

    # Check return statements for cf.astype(...) or named array
    for stmt in ast.walk(func_node):
        if isinstance(stmt, ast.Return) and stmt.value is not None:
            ret = stmt.value
            # return cf.astype(...) → cf size already found above
            if isinstance(ret, ast.Call) and isinstance(ret.func, ast.Attribute):
                if ret.func.attr == "astype" and isinstance(ret.func.value, ast.Name):
                    name = ret.func.value.id
                    if name == "cf" and "cf" in array_sizes:
                        return array_sizes["cf"]
                    if name in array_sizes:
                        return array_sizes[name]
            # return <name> where name has known size
            if isinstance(ret, ast.Name) and ret.id in array_sizes:
                return array_sizes[ret.id]
    # Fall back to np.arange size (the arange determines cf length)
    for stmt in ast.walk(func_node):
        if isinstance(stmt, ast.Assign):
            target = stmt.targets[0]
            if isinstance(target, ast.Name) and target.id != "cf":
                if isinstance(stmt.value, ast.Call) and isinstance(stmt.value.func, ast.Attribute):
                    func = stmt.value.func
                    if (isinstance(func.value, ast.Name) and func.value.id == "np" and
                            func.attr == "arange"):
                        args = stmt.value.args
                        if len(args) == 1 and isinstance(args[0], ast.Constant):
                            return int(args[0].value)
                        elif len(args) == 2:
                            # np.arange(start, stop) — size is stop - start
                            if isinstance(args[0], ast.Constant) and isinstance(args[1], ast.Constant):
                                return int(args[1].value) - int(args[0].value)
    return 36  # default


def get_func_body(func_node):
    """Get the body statements, unwrapping try/except."""
    body = func_node.body
    if len(body) == 1 and isinstance(body[0], ast.Try):
        return body[0].body
    return body


def _process_with_arange_blocks(tp, stmts, arange_var_names):
    """Process statements, grouping consecutive vectorized stmts into arange loop blocks.

    A stmt is 'vectorized' if it references an arange variable OR assigns/augassigns
    to 'cf' and follows a vectorized statement (to catch cf = real_part + 1j*imag_part
    where real_part was computed from an arange var).
    """
    # First pass: process arange definition statements normally so tp.arange_vars gets populated
    # Then group remaining stmts
    pending_block = []  # vectorized stmts to emit in a loop
    vectorized_locals = set()  # local vars computed from arange expressions

    def _references_arange_or_vec_local(node):
        """Check if node references an arange var or a vectorized local."""
        for child in ast.walk(node):
            if isinstance(child, ast.Name):
                if child.id in arange_var_names or child.id in vectorized_locals:
                    return True
        return False

    def flush_block():
        nonlocal pending_block
        if pending_block:
            tp.process_arange_block(pending_block)
            pending_block = []

    for stmt in stmts:
        # Check if this is an arange definition
        info = tp._is_arange_assign(stmt)
        if info:
            flush_block()
            tp.process_stmt(stmt)  # registers in arange_vars
            continue

        # Check if this statement references an arange var or vectorized local
        is_vec = _references_arange_or_vec_local(stmt)

        # Also check: cf = np.zeros(...) before vectorized block — just process normally
        if isinstance(stmt, ast.Assign):
            target = stmt.targets[0]
            if isinstance(target, ast.Name) and target.id == "cf" and tp._is_np_zeros(stmt.value):
                flush_block()
                tp.process_stmt(stmt)
                continue
            # Track vectorized locals (non-cf assignments from arange expressions)
            if is_vec and isinstance(target, ast.Name) and target.id != "cf":
                vectorized_locals.add(target.id)

        if is_vec:
            pending_block.append(stmt)
        else:
            flush_block()
            vectorized_locals.clear()
            tp.process_stmt(stmt)

    flush_block()


def transpile_function(func_node):
    """Transpile a single poly_N function to C."""
    name = func_node.name
    n_coeffs = extract_ncoeffs(func_node)

    CVar.reset()
    tp = PolyTranspiler(name, n_coeffs)

    body = get_func_body(func_node)

    # Pre-scan for constant local assignments (e.g. n = 35) so linspace can resolve them
    # Also handles: n = ps.poly.get("n") or 35  →  extracts the default (35)
    tp._const_locals = {}
    for stmt in body:
        if isinstance(stmt, ast.Return):
            break
        if (isinstance(stmt, ast.Assign) and len(stmt.targets) == 1 and
                isinstance(stmt.targets[0], ast.Name)):
            val = stmt.value
            if isinstance(val, ast.Constant) and isinstance(val.value, (int, float)):
                tp._const_locals[stmt.targets[0].id] = int(val.value)
            elif (isinstance(val, ast.BoolOp) and isinstance(val.op, ast.Or) and
                  len(val.values) == 2 and isinstance(val.values[1], ast.Constant) and
                  isinstance(val.values[1].value, (int, float))):
                # Pattern: X or <default> — use the default
                tp._const_locals[stmt.targets[0].id] = int(val.values[1].value)

    # Two-pass: first pass detects arange variables, second pass processes
    # We need to pre-scan for arange vars so we can group vectorized statements
    arange_var_names = set()
    for stmt in body:
        if isinstance(stmt, ast.Return):
            break
        info = tp._is_arange_assign(stmt)
        if info:
            arange_var_names.add(info[0])

    # Process statements, grouping consecutive vectorized stmts into arange loop blocks
    stmts = []
    for stmt in body:
        if isinstance(stmt, ast.Return):
            break
        stmts.append(stmt)

    if arange_var_names:
        _process_with_arange_blocks(tp, stmts, arange_var_names)
    else:
        for stmt in stmts:
            tp.process_stmt(stmt)

    # Build the C function
    lines = []
    lines.append(f"static void {name}_c(double x1r, double x1i, double x2r, double x2i,")
    lines.append(f"                     double *cRe, double *cIm, int *nCoeffs) {{")
    lines.append(f"    *nCoeffs = {n_coeffs};")
    lines.append(f"    for (int _i = 0; _i < {n_coeffs}; _i++) {{ cRe[_i] = 0; cIm[_i] = 0; }}")
    lines.extend(tp.lines)
    # Add isfinite check
    lines.append(f"    for (int _i = 0; _i < {n_coeffs}; _i++) {{")
    lines.append(f"        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) {{ cRe[_i] = 0; cIm[_i] = 0; }}")
    lines.append(f"    }}")
    lines.append(f"}}")
    c_code = "\n".join(lines)

    # Auto-detect broken transpilations: if the C code contains unhandled warnings,
    # replace with a stub to avoid silent wrong output.
    if "WARNING: unhandled" in c_code or "WARNING: could not evaluate" in c_code:
        import sys
        print(f"  {name}: auto-stubbed (contains unhandled constructs)", file=sys.stderr)
        stub_lines = []
        stub_lines.append(f"/* {name}: auto-stubbed (unhandled constructs in source) */")
        stub_lines.append(f"static void {name}_c(double x1r, double x1i, double x2r, double x2i,")
        stub_lines.append(f"                     double *cRe, double *cIm, int *nCoeffs) {{")
        stub_lines.append(f"    *nCoeffs = {n_coeffs};")
        stub_lines.append(f"    for (int _i = 0; _i < {n_coeffs}; _i++) {{ cRe[_i] = 0; cIm[_i] = 0; }}")
        stub_lines.append(f"    (void)x1r; (void)x1i; (void)x2r; (void)x2i;")
        stub_lines.append(f"}}")
        return "\n".join(stub_lines), name, n_coeffs

    return c_code, name, n_coeffs


def transpile_file(src_path, stub_funcs=None, skip_funcs=None, label=None):
    """Transpile all poly_N functions from a Python source file.
    Returns (c_code_str, header_str, lookup_str) as strings.
    stub_funcs: emit zero-output stubs. skip_funcs: completely omit."""
    if stub_funcs is None:
        stub_funcs = set()
    if skip_funcs is None:
        skip_funcs = set()
    if label is None:
        label = src_path.rsplit("/", 1)[-1]

    with open(src_path) as f:
        source = f.read()

    tree = ast.parse(source)

    funcs = []
    for node in ast.iter_child_nodes(tree):
        if isinstance(node, ast.FunctionDef) and (node.name.startswith("poly_") or node.name.startswith("g")):
            funcs.append(node)

    c_lines = []
    c_lines.append(f"/* AUTO-GENERATED from {label} — do not edit manually */")
    c_lines.append(f"/* {len(funcs)} coefficient functions */")
    c_lines.append("")

    header_lines = []
    lookup_lines = []
    names_and_ncoeffs = []

    for func_node in funcs:
        try:
            if func_node.name in skip_funcs:
                c_lines.append(f"/* {func_node.name}: skipped (defined elsewhere) */")
                c_lines.append("")
                continue
            if func_node.name in stub_funcs:
                n_coeffs = extract_ncoeffs(func_node)
                c_lines.append(f"/* {func_node.name}: too complex for auto-transpile, stubbed */")
                c_lines.append(f"static void {func_node.name}_c(double x1r, double x1i, double x2r, double x2i,")
                c_lines.append(f"                     double *cRe, double *cIm, int *nCoeffs) {{")
                c_lines.append(f"    *nCoeffs = {n_coeffs};")
                c_lines.append(f"    for (int _i = 0; _i < {n_coeffs}; _i++) {{ cRe[_i] = 0; cIm[_i] = 0; }}")
                c_lines.append(f"    (void)x1r; (void)x1i; (void)x2r; (void)x2i;")
                c_lines.append(f"}}")
                c_lines.append("")
                names_and_ncoeffs.append((func_node.name, n_coeffs))
                continue
            c_code, name, n_coeffs = transpile_function(func_node)
            c_lines.append(c_code)
            c_lines.append("")
            names_and_ncoeffs.append((name, n_coeffs))
        except Exception as e:
            c_lines.append(f"/* ERROR transpiling {func_node.name}: {e} */")
            c_lines.append("")

    for name, _ in names_and_ncoeffs:
        header_lines.append(f"static void {name}_c(double x1r, double x1i, double x2r, double x2i, double *cRe, double *cIm, int *nCoeffs);")
        lookup_lines.append(f'    if (strcmp(name, "{name}") == 0) return {name}_c;')

    return "\n".join(c_lines), "\n".join(header_lines), "\n".join(lookup_lines)


def main():
    src_path = sys.argv[1] if len(sys.argv) > 1 else "/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly100.py"

    # Stub functions per source file
    STUBS = {
        "poly100.py": {"poly_21", "poly_35", "poly_37", "poly_40", "poly_46", "poly_58", "poly_72", "poly_74", "poly_94", "poly_100"},
    }

    STUBS["ops_poly.py"] = {"g7", "g8", "g11", "g31", "g38", "g48", "g49", "g50", "g51", "g71", "g82", "g84", "g90", "g92", "g95", "g97", "g98", "g224", "g227", "g232", "g2863", "g2864", "g2864a", "gp7f", "gpt1"}
    basename = src_path.rsplit("/", 1)[-1]
    stub_funcs = STUBS.get(basename, set())

    c_code, header, lookups = transpile_file(src_path, stub_funcs, basename)

    # Write .c file to stdout
    print(c_code)
    print()
    print("/* Lookup entries for lookupCoeffFuncC: */")
    print("/*")
    print(lookups)
    print("*/")


if __name__ == "__main__":
    main()
