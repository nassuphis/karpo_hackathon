"""
CFPV (Coefficient Function Parameter Vector) feature tests.

Tests the end-to-end CFPV pipeline:
- default behavior when cfpv is absent
- parametric functions (creative9 with varying n)
- non-parametric functions unaffected by cfpv
- nCoeffs invariant enforcement
- JSON parsing edge cases
- chunked and legacy coeffgen modes

Run: cd polypaint && uv run python tests/test_cfpv.py
"""
import json
import os
import struct
import subprocess
import sys

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")

passed = 0
failed = 0


def run_sweep(spec, out_path="/tmp/cfpv_test.bin"):
    """Run sweep binary with JSON spec, return (meta_dict, returncode, stderr)."""
    r = subprocess.run(
        [SWEEP, out_path],
        input=json.dumps(spec),
        capture_output=True, text=True, timeout=30,
    )
    meta = None
    if r.stdout.strip():
        try:
            meta = json.loads(r.stdout.strip())
        except json.JSONDecodeError:
            pass
    return meta, r.returncode, r.stderr


def check(name, condition, detail=""):
    global passed, failed
    if condition:
        passed += 1
        print(f"  PASS: {name}")
    else:
        failed += 1
        print(f"  FAIL: {name} — {detail}")


def test_creative9_default():
    """creative9 without cfpv should return n_coeffs=71 (default)."""
    meta, rc, _ = run_sweep({
        "mode": "coeffgen",
        "function": "creative9",
        "n1": 4, "n2": 4, "i1_start": 0, "i1_end": 4,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [], "times": 1,
    })
    check("creative9 default returns 71",
          rc == 0 and meta and meta["n_coeffs"] == 71,
          f"rc={rc}, n_coeffs={meta.get('n_coeffs') if meta else '?'}")


def test_creative9_cfpv_override():
    """creative9 with cfpv=[30] should return n_coeffs=30."""
    meta, rc, _ = run_sweep({
        "mode": "coeffgen",
        "function": "creative9",
        "n1": 4, "n2": 4, "i1_start": 0, "i1_end": 4,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [], "times": 1,
        "cfpv": [30],
    })
    check("creative9 cfpv=[30] returns 30",
          rc == 0 and meta and meta["n_coeffs"] == 30,
          f"rc={rc}, n_coeffs={meta.get('n_coeffs') if meta else '?'}")


def test_creative9_cfpv_10():
    """creative9 with cfpv=[10] should return n_coeffs=10."""
    meta, rc, _ = run_sweep({
        "mode": "coeffgen",
        "function": "creative9",
        "n1": 4, "n2": 4, "i1_start": 0, "i1_end": 4,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [], "times": 1,
        "cfpv": [10],
    })
    check("creative9 cfpv=[10] returns 10",
          rc == 0 and meta and meta["n_coeffs"] == 10,
          f"rc={rc}, n_coeffs={meta.get('n_coeffs') if meta else '?'}")


def test_creative9_output_size():
    """Verify binary output size matches n_coeffs from cfpv."""
    n1, n2, n_cfpv = 5, 5, 20
    out = "/tmp/cfpv_size_test.bin"
    meta, rc, _ = run_sweep({
        "mode": "coeffgen",
        "function": "creative9",
        "n1": n1, "n2": n2, "i1_start": 0, "i1_end": n1,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [], "times": 1,
        "cfpv": [n_cfpv],
    }, out)
    expected_bytes = n1 * n2 * n_cfpv * 2 * 4  # steps × coeffs × (re,im) × float32
    actual = os.path.getsize(out) if os.path.exists(out) else -1
    check("creative9 cfpv=[20] output size correct",
          rc == 0 and actual == expected_bytes,
          f"expected {expected_bytes}, got {actual}")
    if os.path.exists(out):
        os.remove(out)


def test_nonparametric_ignores_cfpv():
    """g1 (non-parametric) should return same n_coeffs with or without cfpv."""
    meta1, rc1, _ = run_sweep({
        "mode": "coeffgen",
        "function": "g1",
        "n1": 4, "n2": 4, "i1_start": 0, "i1_end": 4,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [], "times": 1,
    })
    meta2, rc2, _ = run_sweep({
        "mode": "coeffgen",
        "function": "g1",
        "n1": 4, "n2": 4, "i1_start": 0, "i1_end": 4,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [], "times": 1,
        "cfpv": [999],
    })
    check("g1 unaffected by cfpv",
          rc1 == 0 and rc2 == 0 and meta1 and meta2 and
          meta1["n_coeffs"] == meta2["n_coeffs"],
          f"without={meta1.get('n_coeffs') if meta1 else '?'}, with={meta2.get('n_coeffs') if meta2 else '?'}")


def test_empty_cfpv():
    """Empty cfpv array should behave like absent cfpv."""
    meta, rc, _ = run_sweep({
        "mode": "coeffgen",
        "function": "creative9",
        "n1": 4, "n2": 4, "i1_start": 0, "i1_end": 4,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [], "times": 1,
        "cfpv": [],
    })
    check("empty cfpv same as absent",
          rc == 0 and meta and meta["n_coeffs"] == 71,
          f"rc={rc}, n_coeffs={meta.get('n_coeffs') if meta else '?'}")


def test_chunked_with_cfpv():
    """coeffgen_chunked with cfpv should work end-to-end."""
    params_path = "/tmp/cfpv_test_params.bin"
    r = subprocess.run(
        [SWEEP, params_path],
        input=json.dumps({
            "mode": "param_gen",
            "n1": 5, "n2": 5, "times": 1,
            "param_transforms": [["unit_circle"]],
        }),
        capture_output=True, text=True, timeout=30,
    )
    if r.returncode != 0:
        check("chunked cfpv: param_gen", False, r.stderr[:200])
        return

    out = "/tmp/cfpv_test_chunked.bin"
    meta, rc, stderr = run_sweep({
        "mode": "coeffgen_chunked",
        "function": "creative9",
        "params_file": params_path,
        "step_start": 0, "step_count": 25,
        "coeff_transforms": [],
        "cfpv": [15],
    }, out)
    check("chunked coeffgen with cfpv=[15]",
          rc == 0 and meta and meta["n_coeffs"] == 15,
          f"rc={rc}, n_coeffs={meta.get('n_coeffs') if meta else '?'}, stderr={stderr[:200]}")
    for p in [params_path, out]:
        if os.path.exists(p):
            os.remove(p)


def test_chunked_without_cfpv():
    """coeffgen_chunked without cfpv should use defaults."""
    params_path = "/tmp/cfpv_test_params2.bin"
    subprocess.run(
        [SWEEP, params_path],
        input=json.dumps({
            "mode": "param_gen",
            "n1": 4, "n2": 4, "times": 1,
            "param_transforms": [["unit_circle"]],
        }),
        capture_output=True, text=True, timeout=30,
    )

    out = "/tmp/cfpv_test_chunked2.bin"
    meta, rc, stderr = run_sweep({
        "mode": "coeffgen_chunked",
        "function": "creative9",
        "params_file": params_path,
        "step_start": 0, "step_count": 16,
        "coeff_transforms": [],
    }, out)
    check("chunked coeffgen without cfpv defaults to 71",
          rc == 0 and meta and meta["n_coeffs"] == 71,
          f"rc={rc}, n_coeffs={meta.get('n_coeffs') if meta else '?'}, stderr={stderr[:200]}")
    for p in [params_path, out]:
        if os.path.exists(p):
            os.remove(p)


def test_transpiled_function_unaffected():
    """A transpiled function (poly_1) should work normally with new ABI."""
    meta, rc, stderr = run_sweep({
        "mode": "coeffgen",
        "function": "poly_1",
        "n1": 4, "n2": 4, "i1_start": 0, "i1_end": 4,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [], "times": 1,
    })
    check("transpiled poly_1 works with new ABI",
          rc == 0 and meta and meta["n_coeffs"] > 0,
          f"rc={rc}, n_coeffs={meta.get('n_coeffs') if meta else '?'}, stderr={stderr[:200]}")


def test_cfpv_clamp_too_small():
    """creative9 with cfpv=[1] — below minimum (2) — should use default 71."""
    meta, rc, _ = run_sweep({
        "mode": "coeffgen",
        "function": "creative9",
        "n1": 4, "n2": 4, "i1_start": 0, "i1_end": 4,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [], "times": 1,
        "cfpv": [1],
    })
    check("creative9 cfpv=[1] clamps to default 71",
          rc == 0 and meta and meta["n_coeffs"] == 71,
          f"rc={rc}, n_coeffs={meta.get('n_coeffs') if meta else '?'}")


def test_multiple_g_functions():
    """Several non-parametric transpiled/hand-written functions work with new ABI."""
    funcs = ["g1", "g2", "g12", "giga_1", "giga_5", "creative8", "p821"]
    all_ok = True
    for fn in funcs:
        meta, rc, stderr = run_sweep({
            "mode": "coeffgen",
            "function": fn,
            "n1": 3, "n2": 3, "i1_start": 0, "i1_end": 3,
            "param_transforms": [["unit_circle"]],
            "coeff_transforms": [], "times": 1,
        })
        if rc != 0 or not meta or meta.get("n_coeffs", 0) <= 0:
            check(f"{fn} with new ABI", False,
                  f"rc={rc}, stderr={stderr[:100]}")
            all_ok = False
    if all_ok:
        check(f"all {len(funcs)} non-parametric functions work", True)


def test_creative8_default():
    """creative8 without cfpv should return n_coeffs=71 (default)."""
    meta, rc, _ = run_sweep({
        "mode": "coeffgen",
        "function": "creative8",
        "n1": 4, "n2": 4, "i1_start": 0, "i1_end": 4,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [], "times": 1,
    })
    check("creative8 default returns 71",
          rc == 0 and meta and meta["n_coeffs"] == 71,
          f"rc={rc}, n_coeffs={meta.get('n_coeffs') if meta else '?'}")


def test_creative8_cfpv_override():
    """creative8 with cfpv=[40] should return n_coeffs=40."""
    meta, rc, _ = run_sweep({
        "mode": "coeffgen",
        "function": "creative8",
        "n1": 4, "n2": 4, "i1_start": 0, "i1_end": 4,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [], "times": 1,
        "cfpv": [40],
    })
    check("creative8 cfpv=[40] returns 40",
          rc == 0 and meta and meta["n_coeffs"] == 40,
          f"rc={rc}, n_coeffs={meta.get('n_coeffs') if meta else '?'}")


def test_creative8_output_size():
    """Verify creative8 binary output size matches cfpv."""
    n1, n2, n_cfpv = 5, 5, 25
    out = "/tmp/cfpv_c8_size.bin"
    meta, rc, _ = run_sweep({
        "mode": "coeffgen",
        "function": "creative8",
        "n1": n1, "n2": n2, "i1_start": 0, "i1_end": n1,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [], "times": 1,
        "cfpv": [n_cfpv],
    }, out)
    expected = n1 * n2 * n_cfpv * 2 * 4
    actual = os.path.getsize(out) if os.path.exists(out) else -1
    check("creative8 cfpv=[25] output size correct",
          rc == 0 and actual == expected,
          f"expected {expected}, got {actual}")
    if os.path.exists(out):
        os.remove(out)


def test_catalog_exists():
    """Catalog JSON exists and is valid."""
    cat_path = os.path.join(LAMBDA_DIR, "coeff_func_catalog.json")
    try:
        with open(cat_path) as f:
            cat = json.load(f)
        check("catalog JSON exists and is valid",
              isinstance(cat, list) and len(cat) > 100,
              f"type={type(cat).__name__}, len={len(cat) if isinstance(cat, list) else '?'}")
    except Exception as e:
        check("catalog JSON exists and is valid", False, str(e))


def test_catalog_has_params():
    """Catalog has params for creative8 and creative9."""
    cat_path = os.path.join(LAMBDA_DIR, "coeff_func_catalog.json")
    with open(cat_path) as f:
        cat = json.load(f)
    by_name = {e["name"]: e for e in cat}
    c8 = by_name.get("creative8", {})
    c9 = by_name.get("creative9", {})
    check("catalog creative8 has params",
          len(c8.get("params", [])) > 0,
          f"params={c8.get('params')}")
    check("catalog creative9 has params",
          len(c9.get("params", [])) > 0,
          f"params={c9.get('params')}")


def test_catalog_nonparametric_no_params():
    """Non-parametric functions in catalog have no params."""
    cat_path = os.path.join(LAMBDA_DIR, "coeff_func_catalog.json")
    with open(cat_path) as f:
        cat = json.load(f)
    by_name = {e["name"]: e for e in cat}
    for fn in ["g1", "giga_1", "poly_1", "p821"]:
        entry = by_name.get(fn, {})
        if entry.get("params"):
            check(f"{fn} has no params", False, f"params={entry['params']}")
            return
    check("non-parametric functions have no params", True)


def test_catalog_js_exists():
    """Generated JS catalog exists."""
    js_path = os.path.join(LAMBDA_DIR, "..", "coeff_func_catalog_js.js")
    check("JS catalog exists",
          os.path.exists(js_path) and os.path.getsize(js_path) > 1000,
          f"exists={os.path.exists(js_path)}")


def test_catalog_js_covers_source():
    """Every source catalog entry appears in generated JS (no silent drops)."""
    cat_path = os.path.join(LAMBDA_DIR, "coeff_func_catalog.json")
    js_path = os.path.join(LAMBDA_DIR, "..", "coeff_func_catalog_js.js")
    with open(cat_path) as f:
        source = json.load(f)
    with open(js_path) as f:
        js_text = f.read()
    js_data = json.loads(js_text.split("window._coeffFuncCatalog = ")[1].rstrip(";\n"))
    js_names = {e["name"] for e in js_data}
    source_names = {e["name"] for e in source}
    missing = source_names - js_names
    check("JS catalog covers all source entries",
          len(missing) == 0,
          f"{len(missing)} missing: {', '.join(sorted(missing)[:10])}")


def test_catalog_no_transforms():
    """Catalog contains no transform names (only coefficient functions)."""
    cat_path = os.path.join(LAMBDA_DIR, "coeff_func_catalog.json")
    with open(cat_path) as f:
        cat = json.load(f)
    transform_names = {"add_sub", "rev", "unit_circle", "conjugate", "swap",
                       "sort_abs", "cumsum", "conj", "cube", "exp", "deriv",
                       "square", "reciprocal", "negate"}
    found = [e["name"] for e in cat if e["name"] in transform_names]
    check("catalog has no transform names",
          len(found) == 0,
          f"found: {found}")


def test_metrics_exists():
    """Metrics overlay exists and has entries."""
    met_path = os.path.join(LAMBDA_DIR, "coeff_func_metrics.json")
    try:
        with open(met_path) as f:
            met = json.load(f)
        check("metrics JSON exists with entries",
              isinstance(met, dict) and len(met) > 50,
              f"type={type(met).__name__}, len={len(met) if isinstance(met, dict) else '?'}")
    except Exception as e:
        check("metrics JSON exists with entries", False, str(e))


if __name__ == "__main__":
    if not os.path.exists(SWEEP):
        print(f"ERROR: sweep_test not found at {SWEEP}")
        print("Build first: cd lambda && cc -O2 -Wall -o sweep_test sweep_cli.c -lm")
        sys.exit(1)

    print("CFPV feature tests")
    print("=" * 50)

    print("\n--- creative9 parametric behavior ---")
    test_creative9_default()
    test_creative9_cfpv_override()
    test_creative9_cfpv_10()
    test_creative9_output_size()
    test_cfpv_clamp_too_small()

    print("\n--- creative8 parametric behavior (second function) ---")
    test_creative8_default()
    test_creative8_cfpv_override()
    test_creative8_output_size()

    print("\n--- non-parametric functions ---")
    test_nonparametric_ignores_cfpv()
    test_transpiled_function_unaffected()
    test_multiple_g_functions()

    print("\n--- edge cases ---")
    test_empty_cfpv()

    print("\n--- chunked coeffgen ---")
    test_chunked_with_cfpv()
    test_chunked_without_cfpv()

    print("\n--- catalog system ---")
    test_catalog_exists()
    test_catalog_has_params()
    test_catalog_nonparametric_no_params()
    test_catalog_js_exists()
    test_catalog_js_covers_source()
    test_catalog_no_transforms()
    test_metrics_exists()

    print("\n" + "=" * 50)
    total = passed + failed
    print(f"{passed}/{total} passed")
    if failed:
        print("FAIL")
        sys.exit(1)
    else:
        print("ALL PASS")
