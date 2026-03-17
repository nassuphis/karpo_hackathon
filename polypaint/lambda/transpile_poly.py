#!/usr/bin/env python3
"""Transpile poly100.py functions from Python/numpy to C.

Generates C coefficient functions compatible with sweep_cli.c.
Uses the complex arithmetic helpers already defined there (c_mul, c_div, c_sin, etc.).

Usage: python3 transpile_poly.py > poly_generated.c
"""

import ast
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
        self.declared = set()
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
                return CVar("x1r", "x1i")
            elif name == "t2":
                return CVar("x2r", "x2i")
            elif name == "pi":
                return CVar("M_PI", "0")
            else:
                # loop variable or local — treat as real
                return CVar(name, "0")

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
                    parts.append(f"({prev.r} == {comp.r})")
                elif isinstance(op, ast.NotEq):
                    parts.append(f"({prev.r} != {comp.r})")
                elif isinstance(op, ast.Lt):
                    parts.append(f"({prev.r} < {comp.r})")
                elif isinstance(op, ast.LtE):
                    parts.append(f"({prev.r} <= {comp.r})")
                elif isinstance(op, ast.Gt):
                    parts.append(f"({prev.r} > {comp.r})")
                elif isinstance(op, ast.GtE):
                    parts.append(f"({prev.r} >= {comp.r})")
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
                # Power is a complex expression — use c_powr with real part
                self.emit(f"c_powr({left.r}, {left.i}, {right.r}, &{tmp.r}, &{tmp.i});")
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
                # array length — depends on context
                tmp = CVar.fresh("len")
                self.declare(tmp)
                self.emit(f"/* WARNING: len() not directly translatable */")
                return tmp

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
        elif attr == "conj":
            arg = self.expr_to_c(args[0])
            tmp = CVar.fresh("conj")
            self.declare(tmp)
            self.emit(f"{tmp.r} = {arg.r}; {tmp.i} = -({arg.i});")
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
            # np.sum(cf[a:b]) — handle specially
            if len(args) == 1 and isinstance(args[0], ast.Subscript):
                return self.numpy_sum_slice(args[0])
            arg = self.expr_to_c(args[0])
            return arg
        elif attr == "prod":
            if len(args) == 1 and isinstance(args[0], ast.Subscript):
                return self.numpy_prod_slice(args[0])
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
                self.emit(f"{tmp.r} = cRe[{idx}]; {tmp.i} = cIm[{idx}];")
                return tmp
            else:
                # Some local array — treat as real constant lookup
                idx = self.index_expr(node.slice)
                tmp = CVar.fresh("arr")
                self.declare(tmp)
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
            return node.id
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
        return f"(int)({self.expr_to_c(node).r})"

    def assign_cf(self, idx_node, value_node):
        """Emit cf[idx] = value."""
        idx = self.index_expr(idx_node)
        val = self.expr_to_c(value_node)
        self.emit(f"cRe[{idx}] = {val.r}; cIm[{idx}] = {val.i};")

    def assign_cf_slice(self, sl, value_node):
        """Handle cf[a:b] = expr involving arrays."""
        lo = self.get_int(sl.lower) if sl.lower else 0
        hi = self.get_int(sl.upper) if sl.upper else self.n_coeffs
        if lo is None or hi is None:
            self.emit(f"/* WARNING: dynamic slice assignment */")
            return
        count = hi - lo

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
            # Subscript the array
            tmp = CVar.fresh("elem")
            self.declare(tmp)
            self.emit(f"{tmp.r} = {node.id}[{loop_var}]; {tmp.i} = 0;")
            return tmp

        # Fallback to normal expression
        return self.expr_to_c(node)

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
                    # cf = np.zeros(...) — skip, already zero
                    pass
                elif self.is_np_array_literal(stmt.value):
                    # np.array([1, 2, 3]) -> static const double name[] = {1, 2, 3};
                    elts = self.extract_array_elements(stmt.value)
                    if elts is not None:
                        vals = ", ".join(str(float(v)) for v in elts)
                        self.declared.add(name)
                        self.emit(f"static const double {name}[] = {{{vals}}};")
                    else:
                        self.declared.add(name)
                        self.emit(f"/* WARNING: could not extract np.array for {name} */")
                        self.emit(f"static const double {name}[] = {{0}};")
                elif self.is_np_computed_array(stmt.value):
                    # np.cumsum(np.arange(1, N)) etc.
                    arr = self.eval_computed_array(stmt.value)
                    if arr is not None:
                        vals = ", ".join(str(float(v)) for v in arr)
                        self.declared.add(name)
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
                    # Local variable
                    val = self.expr_to_c(stmt.value)
                    if name not in self.declared:
                        self.declared.add(name)
                        self.emit(f"double {name} = {val.r}; /* +{val.i}i */")
                    else:
                        self.emit(f"{name} = {val.r};")
            elif isinstance(target, ast.Tuple):
                # Tuple unpacking — handle simple cases
                if isinstance(stmt.value, ast.Tuple):
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


def extract_ncoeffs(func_node):
    """Extract the size of the cf array from np.zeros(N, ...)."""
    for stmt in ast.walk(func_node):
        if isinstance(stmt, ast.Assign):
            if isinstance(stmt.targets[0], ast.Name) and stmt.targets[0].id == "cf":
                if isinstance(stmt.value, ast.Call):
                    func = stmt.value.func
                    if isinstance(func, ast.Attribute) and func.attr == "zeros":
                        args = stmt.value.args
                        if args and isinstance(args[0], ast.Constant):
                            return int(args[0].value)
    return 36  # default


def get_func_body(func_node):
    """Get the body statements, unwrapping try/except."""
    body = func_node.body
    if len(body) == 1 and isinstance(body[0], ast.Try):
        return body[0].body
    return body


def transpile_function(func_node):
    """Transpile a single poly_N function to C."""
    name = func_node.name
    n_coeffs = extract_ncoeffs(func_node)

    CVar.reset()
    tp = PolyTranspiler(name, n_coeffs)

    body = get_func_body(func_node)
    for stmt in body:
        if isinstance(stmt, ast.Return):
            break
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
    return "\n".join(lines), name, n_coeffs


def main():
    # Read the source file
    src_path = "/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/poly100.py"
    with open(src_path) as f:
        source = f.read()

    tree = ast.parse(source)

    funcs = []
    for node in ast.iter_child_nodes(tree):
        if isinstance(node, ast.FunctionDef) and node.name.startswith("poly_"):
            funcs.append(node)

    print("/* AUTO-GENERATED from poly100.py — do not edit manually */")
    print(f"/* {len(funcs)} coefficient functions */")
    print()

    # Functions that fail to transpile cleanly — stub them
    STUB_FUNCS = {"poly_16", "poly_21", "poly_35", "poly_37", "poly_40", "poly_46", "poly_58", "poly_72", "poly_74", "poly_94", "poly_100"}

    names_and_ncoeffs = []
    for func_node in funcs:
        try:
            if func_node.name in STUB_FUNCS:
                n_coeffs = extract_ncoeffs(func_node)
                print(f"/* {func_node.name}: too complex for auto-transpile, stubbed */")
                print(f"static void {func_node.name}_c(double x1r, double x1i, double x2r, double x2i,")
                print(f"                     double *cRe, double *cIm, int *nCoeffs) {{")
                print(f"    *nCoeffs = {n_coeffs};")
                print(f"    for (int _i = 0; _i < {n_coeffs}; _i++) {{ cRe[_i] = 0; cIm[_i] = 0; }}")
                print(f"    (void)x1r; (void)x1i; (void)x2r; (void)x2i;")
                print(f"}}")
                print()
                names_and_ncoeffs.append((func_node.name, n_coeffs))
                continue
            c_code, name, n_coeffs = transpile_function(func_node)
            print(c_code)
            print()
            names_and_ncoeffs.append((name, n_coeffs))
        except Exception as e:
            print(f"/* ERROR transpiling {func_node.name}: {e} */")
            print()

    # Generate lookup entries
    print("/* Lookup entries for lookupCoeffFuncC: */")
    print("/*")
    for name, _ in names_and_ncoeffs:
        print(f'    if (strcmp(name, "{name}") == 0) return {name}_c;')
    print("*/")


if __name__ == "__main__":
    main()
