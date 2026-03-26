#!/usr/bin/env python3
"""
Docker runtime regression test for deploy binaries.

Runs inside ARM64 Docker container with /src mounted to lambda/.
Tests: AE/CM solvers, CFPV coeffgen, render preview (vipsthumbnail),
solve_proximity_stats, and catalog degree verification.

Invoked by scripts/test-docker-runtime.sh — not run directly.
"""
import json
import math
import os
import struct
import subprocess
import sys


# ── Helpers ──────────────────────────────────────────────────────────────

def write_cf(path, coeffs_list, n_coeffs):
    with open(path, "wb") as f:
        for cf in coeffs_list:
            padded = list(cf) + [0.0] * (n_coeffs - len(cf))
            for c in padded[:n_coeffs]:
                re = c.real if hasattr(c, "real") else float(c)
                im = c.imag if hasattr(c, "imag") else 0.0
                f.write(struct.pack("<ff", re, im))


def read_roots(path, degree):
    with open(path, "rb") as f:
        data = f.read()
    vals = struct.unpack("<" + "f" * (len(data) // 4), data)
    roots = [complex(vals[i], vals[i + 1]) for i in range(0, len(vals), 2)]
    polys = []
    for i in range(0, len(roots), degree):
        polys.append(roots[i:i + degree])
    return polys


def polyval(cf, z):
    y = 0j
    for a in cf:
        y = y * z + complex(a)
    return y


def run_coeffgen(func, cfpv=None):
    spec = {
        "mode": "coeffgen",
        "function": func,
        "n1": 4, "n2": 4,
        "i1_start": 0, "i1_end": 4,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [],
        "times": 1,
    }
    if cfpv is not None:
        spec["cfpv"] = cfpv
    r = subprocess.run(["/src/sweep", "/tmp/cfpv_test.bin"],
                       input=json.dumps(spec), capture_output=True, text=True, timeout=10)
    if r.returncode != 0:
        return None, r.stderr
    return json.loads(r.stdout), None


def cleanup(*paths):
    for p in paths:
        try:
            os.remove(p)
        except OSError:
            pass


# ── AE/CM Solver Tests ───────────────────────────────────────────────────

def test_ae_cm_solvers():
    print("--- AE/CM solver regression ---")

    polys = [
        [1, -6, 11, -6],
        [1, -10, 35, -50, 24],
        [1, -2, 1],
        [1, 0, 1],
        [0, 1, -3, 2],
        [0, 0, 0, 0, 0],
    ]
    n_coeffs = 5

    write_cf("/tmp/test_cf.bin", polys, n_coeffs)

    ae_spec = json.dumps({
        "mode": "solve", "coeffs_file": "/tmp/test_cf.bin",
        "n_coeffs": n_coeffs, "n2": len(polys),
        "i1_start": 0, "i1_end": 1, "match_roots": False,
    })
    r = subprocess.run(["/src/sweep", "/tmp/ae_out.bin"],
                       input=ae_spec, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, "AE FAILED: " + r.stderr[:200]
    print("AE: %s" % json.loads(r.stdout))

    cm_spec = json.dumps({
        "mode": "solve_cm", "coeffs_file": "/tmp/test_cf.bin",
        "n_coeffs": n_coeffs, "n_steps": len(polys),
    })
    r = subprocess.run(["/src/sweep_cm", "/tmp/cm_out.bin"],
                       input=cm_spec, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, "CM FAILED: " + r.stderr[:200]
    print("CM: %s" % json.loads(r.stdout))

    ae_size = os.path.getsize("/tmp/ae_out.bin")
    cm_size = os.path.getsize("/tmp/cm_out.bin")
    expected = len(polys) * (n_coeffs - 1) * 2 * 4
    assert ae_size == expected, "AE size %d != %d" % (ae_size, expected)
    assert cm_size == expected, "CM size %d != %d" % (cm_size, expected)

    degree = n_coeffs - 1
    ae_roots = read_roots("/tmp/ae_out.bin", degree)
    cm_roots = read_roots("/tmp/cm_out.bin", degree)
    assert len(ae_roots) == len(polys)
    assert len(cm_roots) == len(polys)

    labels = ["cubic", "quartic", "repeated", "complex", "leading-zero", "all-zero"]
    for i, (cf, ae_r, cm_r, label) in enumerate(zip(polys, ae_roots, cm_roots, labels)):
        ae_finite = all(math.isfinite(r.real) and math.isfinite(r.imag) for r in ae_r)
        cm_finite = all(math.isfinite(r.real) and math.isfinite(r.imag) for r in cm_r)
        assert ae_finite, "%s: AE has non-finite roots" % label
        assert cm_finite, "%s: CM has non-finite roots" % label

        if label == "all-zero":
            continue

        active_ae = [r for r in ae_r if abs(r) > 1e-10]
        active_cm = [r for r in cm_r if abs(r) > 1e-10]
        if active_ae:
            ae_resid = max(abs(polyval(cf, r)) for r in active_ae)
            assert ae_resid < 1e-2, "%s: AE max residual %.2e" % (label, ae_resid)
        if active_cm:
            cm_resid = max(abs(polyval(cf, r)) for r in active_cm)
            assert cm_resid < 1e-2, "%s: CM max residual %.2e" % (label, cm_resid)

        print("  %s: AE OK, CM OK" % label)

    print("=== AE/CM solver tests PASSED ===")


# ── CFPV Coeffgen Tests ──────────────────────────────────────────────────

def test_cfpv_coeffgen():
    print("\n--- CFPV coeffgen regression ---")

    m, err = run_coeffgen("creative9")
    assert m and m["n_coeffs"] == 71, "creative9 default: %s" % (err or m)
    print("  creative9 default (n=71): OK")

    m, err = run_coeffgen("creative9", [30])
    assert m and m["n_coeffs"] == 30, "creative9 cfpv=[30]: %s" % (err or m)
    print("  creative9 cfpv=[30] (n=30): OK")

    m, err = run_coeffgen("creative8")
    assert m and m["n_coeffs"] == 71, "creative8 default: %s" % (err or m)
    print("  creative8 default (n=71): OK")

    m, err = run_coeffgen("creative8", [40])
    assert m and m["n_coeffs"] == 40, "creative8 cfpv=[40]: %s" % (err or m)
    print("  creative8 cfpv=[40] (n=40): OK")

    m1, _ = run_coeffgen("g1")
    m2, _ = run_coeffgen("g1", [999])
    assert m1 and m2 and m1["n_coeffs"] == m2["n_coeffs"], "g1 cfpv should be ignored"
    print("  g1 unaffected by cfpv: OK")

    print("=== CFPV coeffgen tests PASSED ===")


# ── Render Preview (vipsthumbnail) Tests ─────────────────────────────────

def test_render_preview():
    print("\n--- Render preview (vipsthumbnail) ---")

    vt_path = "/opt/bin/vipsthumbnail"
    assert os.path.exists(vt_path), "vipsthumbnail not found at %s" % vt_path
    r = subprocess.run([vt_path, "--vips-version"], capture_output=True, text=True, timeout=5)
    print("  vipsthumbnail: %s" % (r.stdout.strip() or r.stderr.strip()))

    test_raw = "/tmp/preview_test.raw"
    test_jpeg = "/tmp/preview_test.jpeg"
    test_png = "/tmp/preview_test_thumb.png"

    with open(test_raw, "wb") as f:
        f.write(struct.pack("<III", 8, 8, 3))
        for _ in range(8 * 8):
            f.write(bytes([100, 150, 200]))

    env = {**os.environ, "LD_LIBRARY_PATH": "/opt/lib", "PATH": "/opt/bin:" + os.environ.get("PATH", "")}

    r = subprocess.run(["/src/raw2jpeg", test_raw, test_jpeg, "--quality=90"],
                       capture_output=True, text=True, timeout=10, env=env)
    assert r.returncode == 0, "raw2jpeg failed: " + r.stderr[:200]
    jpeg_size = os.path.getsize(test_jpeg)
    assert jpeg_size > 0, "test JPEG is empty"
    print("  test JPEG: %d bytes" % jpeg_size)

    r = subprocess.run([vt_path, test_jpeg, "-s", "4x4", "-o", test_png + "[strip]"],
                       capture_output=True, text=True, timeout=10, env=env)
    assert r.returncode == 0, "vipsthumbnail failed: " + r.stderr[:200]
    png_size = os.path.getsize(test_png)
    assert png_size > 0, "thumbnail PNG is empty"
    print("  thumbnail PNG: %d bytes" % png_size)

    test_tif = "/tmp/preview_test.tif"
    r = subprocess.run(["/opt/bin/vips", "im_copy", test_jpeg, test_tif],
                       capture_output=True, text=True, timeout=10, env=env)
    if r.returncode == 0:
        test_tif_thumb = "/tmp/preview_test_tif_thumb.png"
        r = subprocess.run([vt_path, test_tif, "-s", "4x4", "-o", test_tif_thumb + "[strip]"],
                           capture_output=True, text=True, timeout=10, env=env)
        assert r.returncode == 0, "vipsthumbnail TIFF failed: " + r.stderr[:200]
        tif_png_size = os.path.getsize(test_tif_thumb)
        assert tif_png_size > 0, "TIFF thumbnail is empty"
        print("  TIFF thumbnail PNG: %d bytes" % tif_png_size)
    else:
        print("  SKIP TIFF test: vips im_copy not available")

    cleanup(test_raw, test_jpeg, test_png, test_tif, "/tmp/preview_test_tif_thumb.png")
    print("=== Render preview tests PASSED ===")


# ── solve_proximity_stats Tests (all metrics) ────────────────────────────

def _write_sps_bin(path):
    """Write 3-solve degree-2 test fixture for solve_proximity_stats."""
    with open(path, "wb") as f:
        for roots in [[(0, 0), (1, 0)], [(0, 0), (0.01, 0)], [(0, 0), (0.1, 0)]]:
            for re, im in roots:
                f.write(struct.pack("<ff", re, im))


def test_solve_proximity_stats():
    print("\n--- solve_proximity_stats (multi-metric) ---")

    sps_path = "/src/solve_proximity_stats"
    if not os.path.exists(sps_path):
        print("  SKIP: %s not found (not yet compiled)" % sps_path)
        return

    magic = open(sps_path, "rb").read(4)
    assert magic == b"\x7fELF", "solve_proximity_stats is not ELF"
    print("  %s: ELF OK" % sps_path)

    sps_bin = "/tmp/sps_test.bin"
    _write_sps_bin(sps_bin)

    # 1. Proximity clip
    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2", "--metric=proximity"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "proximity clip failed: " + r.stderr[:200]
    clip = json.loads(r.stdout)
    assert clip["n_solves"] == 3
    assert clip["metric"] == "proximity"
    assert clip["clip_lo"] <= clip["clip_hi"]
    print("  proximity clip: OK (n=%d, lo=%.2f, hi=%.2f)" % (clip["n_solves"], clip["clip_lo"], clip["clip_hi"]))

    # Hist mode
    r = subprocess.run([sps_path, sps_bin, "--mode=hist", "--degree=2", "--metric=proximity",
                        "--clip_lo=" + str(clip["clip_lo"]),
                        "--clip_hi=" + str(clip["clip_hi"]),
                        "--hist_bins=10"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "proximity hist failed: " + r.stderr[:200]
    hist = json.loads(r.stdout)
    assert len(hist["hist"]) == 10
    assert sum(hist["hist"]) == 3
    print("  proximity hist: OK (bins=%d, total=%d)" % (len(hist["hist"]), sum(hist["hist"])))

    # 2. Crowding clip
    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2", "--metric=crowding"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "crowding clip failed: " + r.stderr[:200]
    crowd = json.loads(r.stdout)
    assert crowd["metric"] == "crowding"
    assert crowd["n_solves"] == 3
    print("  crowding clip: OK (lo=%.2f, hi=%.2f)" % (crowd["clip_lo"], crowd["clip_hi"]))

    # 3. Spread clip
    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2", "--metric=spread"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "spread clip failed: " + r.stderr[:200]
    spread = json.loads(r.stdout)
    assert spread["metric"] == "spread"
    assert spread["n_solves"] == 3
    print("  spread clip: OK (lo=%.2f, hi=%.2f)" % (spread["clip_lo"], spread["clip_hi"]))

    cleanup(sps_bin)
    print("=== solve_proximity_stats tests PASSED ===")


# ── roots2pix solve_score smoke ──────────────────────────────────────────

def test_roots2pix_solve_score():
    """Smoke test: roots2pix --color=solve_score --solve_metric=proximity."""
    print("\n--- roots2pix solve_score smoke ---")

    r2p_path = "/src/roots2pix"
    if not os.path.exists(r2p_path):
        print("  SKIP: %s not found (not yet compiled)" % r2p_path)
        return

    # Write tiny 4x4 pixel test: 2 solves, degree 2
    sps_bin = "/tmp/r2p_ss_test.bin"
    with open(sps_bin, "wb") as f:
        for roots in [[(0, 0), (1, 0)], [(0, 0), (0.5, 0)]]:
            for re, im in roots:
                f.write(struct.pack("<ff", re, im))

    r = subprocess.run([
        r2p_path, sps_bin, "/tmp/r2p_ss_pix",
        "--width=4", "--height=4", "--tile_size=4",
        "--n_tile_cols=1", "--n_tile_rows=1",
        "--center_re=0.5", "--center_im=0",
        "--scale=2", "--degree=2",
        "--color=solve_score",
        "--solve_metric=proximity",
        "--solve_score_clip_lo=0.0",
        "--solve_score_clip_hi=2.0",
        "--solve_score_cuts=0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9",
    ], capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "roots2pix solve_score failed: " + r.stderr[:200]
    meta = json.loads(r.stdout)
    assert meta["roots_plotted"] >= 0
    print("  roots2pix --color=solve_score: OK (plotted=%d, clipped=%d)" %
          (meta["roots_plotted"], meta["roots_clipped"]))

    # Clean up any .pix files
    import glob
    for f in glob.glob("/tmp/r2p_ss_pix*.pix"):
        cleanup(f)
    cleanup(sps_bin)
    print("=== roots2pix solve_score smoke PASSED ===")


# ── Catalog Degree Verification ──────────────────────────────────────────

def test_catalog_degrees():
    print("\n--- Catalog degree verification ---")

    catalog_path = "/src/../coeff_func_catalog_js.js"
    if not os.path.exists(catalog_path):
        print("  SKIP: coeff_func_catalog_js.js not found (run gen_catalog.py first)")
        return

    import re as re_mod
    with open(catalog_path) as f:
        js_text = f.read()
    cat_json = js_text.split("window._coeffFuncCatalog = ")[1].rstrip(";\n")
    catalog = json.loads(cat_json)

    mismatches = []
    tested = 0
    for entry in catalog:
        if entry.get("probe_failed"):
            continue
        m, err = run_coeffgen(entry["name"])
        if m is None:
            continue
        actual_degree = m["n_coeffs"] - 1
        if actual_degree != entry["degree"]:
            mismatches.append("%s: catalog=%s deploy=%s" % (entry["name"], entry["degree"], actual_degree))
        tested += 1

    if mismatches:
        print("  FAIL: %d degree mismatches:" % len(mismatches))
        for mm in mismatches[:10]:
            print("    %s" % mm)
        sys.exit(1)

    print("  %d functions: all degrees match between host catalog and deploy binary" % tested)
    print("=== Catalog degree verification PASSED ===")


# ── Main ─────────────────────────────────────────────────────────────────

if __name__ == "__main__":
    print("--- Binary validation ---")
    for bin_path in ["/src/sweep", "/src/sweep_cm"]:
        magic = open(bin_path, "rb").read(4)
        assert magic == b"\x7fELF", "%s is not an ELF binary" % bin_path
        print("  %s: ELF OK" % bin_path)

    print("--- Generating test fixtures ---")
    test_ae_cm_solvers()
    test_cfpv_coeffgen()
    test_render_preview()
    test_solve_proximity_stats()
    test_roots2pix_solve_score()
    test_catalog_degrees()

    print("\n=== All Docker runtime tests PASSED ===")
