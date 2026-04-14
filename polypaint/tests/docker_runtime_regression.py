#!/usr/bin/env python3
"""
Docker runtime regression test for deploy binaries.

Runs inside ARM64 Docker container with /src mounted to lambda/.
Tests: AE/AE-MT/CM solvers, CFPV coeffgen, libvips preview/resize runtime,
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


def read_f32_array(path):
    with open(path, "rb") as f:
        data = f.read()
    assert len(data) % 4 == 0, "float32 array size mismatch for %s" % path
    return struct.unpack("<" + "f" * (len(data) // 4), data)


def read_png_dims(path):
    with open(path, "rb") as f:
        header = f.read(24)
    assert header[:8] == b"\x89PNG\r\n\x1a\n", "PNG signature missing for %s" % path
    assert header[12:16] == b"IHDR", "PNG IHDR missing for %s" % path
    return struct.unpack(">II", header[16:24])


def read_jpeg_dims(path):
    with open(path, "rb") as f:
        assert f.read(2) == b"\xff\xd8", "JPEG SOI missing for %s" % path
        while True:
            marker = f.read(1)
            if not marker:
                break
            while marker == b"\xff":
                marker = f.read(1)
                if not marker:
                    break
            if not marker:
                break
            code = marker[0]
            if code in (0xD8, 0xD9):
                continue
            seg_len_bytes = f.read(2)
            if len(seg_len_bytes) != 2:
                break
            seg_len = struct.unpack(">H", seg_len_bytes)[0]
            assert seg_len >= 2, "bad JPEG segment length in %s" % path
            if code in (0xC0, 0xC1, 0xC2, 0xC3, 0xC5, 0xC6, 0xC7, 0xC9, 0xCA, 0xCB, 0xCD, 0xCE, 0xCF):
                data = f.read(5)
                assert len(data) == 5, "truncated JPEG SOF in %s" % path
                height, width = struct.unpack(">HH", data[1:5])
                return width, height
            f.seek(seg_len - 2, os.SEEK_CUR)
    raise AssertionError("JPEG SOF marker not found for %s" % path)


# ── AE/CM Solver Tests ───────────────────────────────────────────────────

def test_ae_cm_solvers():
    print("--- AE/AE-MT/CM solver regression ---")

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

    mt_spec_single = json.dumps({
        "mode": "solve_mt", "coeffs_file": "/tmp/test_cf.bin",
        "n_coeffs": n_coeffs, "n2": len(polys),
        "i1_start": 0, "i1_end": 1, "match_roots": False,
        "n_threads": 1,
    })
    r = subprocess.run(["/src/sweep_mt", "/tmp/ae_mt_single_out.bin"],
                       input=mt_spec_single, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, "AE-MT FAILED: " + r.stderr[:200]
    mt_single_meta = json.loads(r.stdout)
    print("AE-MT(1): %s" % mt_single_meta)
    assert mt_single_meta["n_threads"] == 1, "AE-MT single-thread did not report n_threads=1"

    mt_spec_multi = json.dumps({
        "mode": "solve_mt", "coeffs_file": "/tmp/test_cf.bin",
        "n_coeffs": n_coeffs, "n2": len(polys),
        "i1_start": 0, "i1_end": 1, "match_roots": False,
        "n_threads": 4,
    })
    r = subprocess.run(["/src/sweep_mt", "/tmp/ae_mt_out.bin"],
                       input=mt_spec_multi, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, "AE-MT(4) FAILED: " + r.stderr[:200]
    mt_meta = json.loads(r.stdout)
    print("AE-MT(4): %s" % mt_meta)
    assert mt_meta["n_threads"] >= 1, "AE-MT did not report n_threads"

    cm_spec = json.dumps({
        "mode": "solve_cm", "coeffs_file": "/tmp/test_cf.bin",
        "n_coeffs": n_coeffs, "n_steps": len(polys),
    })
    r = subprocess.run(["/src/sweep_cm", "/tmp/cm_out.bin"],
                       input=cm_spec, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, "CM FAILED: " + r.stderr[:200]
    print("CM: %s" % json.loads(r.stdout))

    ae_size = os.path.getsize("/tmp/ae_out.bin")
    ae_mt_single_size = os.path.getsize("/tmp/ae_mt_single_out.bin")
    ae_mt_size = os.path.getsize("/tmp/ae_mt_out.bin")
    cm_size = os.path.getsize("/tmp/cm_out.bin")
    expected = len(polys) * (n_coeffs - 1) * 2 * 4
    assert ae_size == expected, "AE size %d != %d" % (ae_size, expected)
    assert ae_mt_single_size == expected, "AE-MT(1) size %d != %d" % (ae_mt_single_size, expected)
    assert ae_mt_size == expected, "AE-MT size %d != %d" % (ae_mt_size, expected)
    assert cm_size == expected, "CM size %d != %d" % (cm_size, expected)

    with open("/tmp/ae_out.bin", "rb") as fh:
        ae_bytes = fh.read()
    with open("/tmp/ae_mt_single_out.bin", "rb") as fh:
        ae_mt_single_bytes = fh.read()
    assert ae_bytes == ae_mt_single_bytes, "AE-MT(1) output diverges from AE for match_roots=false fixture"

    degree = n_coeffs - 1
    ae_roots = read_roots("/tmp/ae_out.bin", degree)
    ae_mt_single_roots = read_roots("/tmp/ae_mt_single_out.bin", degree)
    ae_mt_roots = read_roots("/tmp/ae_mt_out.bin", degree)
    cm_roots = read_roots("/tmp/cm_out.bin", degree)
    assert len(ae_roots) == len(polys)
    assert len(ae_mt_single_roots) == len(polys)
    assert len(ae_mt_roots) == len(polys)
    assert len(cm_roots) == len(polys)

    labels = ["cubic", "quartic", "repeated", "complex", "leading-zero", "all-zero"]
    for i, (cf, ae_r, ae_mt_single_r, ae_mt_r, cm_r, label) in enumerate(zip(polys, ae_roots, ae_mt_single_roots, ae_mt_roots, cm_roots, labels)):
        ae_finite = all(math.isfinite(r.real) and math.isfinite(r.imag) for r in ae_r)
        ae_mt_single_finite = all(math.isfinite(r.real) and math.isfinite(r.imag) for r in ae_mt_single_r)
        ae_mt_finite = all(math.isfinite(r.real) and math.isfinite(r.imag) for r in ae_mt_r)
        cm_finite = all(math.isfinite(r.real) and math.isfinite(r.imag) for r in cm_r)
        assert ae_finite, "%s: AE has non-finite roots" % label
        assert ae_mt_single_finite, "%s: AE-MT(1) has non-finite roots" % label
        assert ae_mt_finite, "%s: AE-MT has non-finite roots" % label
        assert cm_finite, "%s: CM has non-finite roots" % label

        if label == "all-zero":
            continue

        active_ae = [r for r in ae_r if abs(r) > 1e-10]
        active_ae_mt_single = [r for r in ae_mt_single_r if abs(r) > 1e-10]
        active_ae_mt = [r for r in ae_mt_r if abs(r) > 1e-10]
        active_cm = [r for r in cm_r if abs(r) > 1e-10]
        if active_ae:
            ae_resid = max(abs(polyval(cf, r)) for r in active_ae)
            assert ae_resid < 1e-2, "%s: AE max residual %.2e" % (label, ae_resid)
        if active_ae_mt_single:
            ae_mt_single_resid = max(abs(polyval(cf, r)) for r in active_ae_mt_single)
            assert ae_mt_single_resid < 1e-2, "%s: AE-MT(1) max residual %.2e" % (label, ae_mt_single_resid)
        if active_ae_mt:
            ae_mt_resid = max(abs(polyval(cf, r)) for r in active_ae_mt)
            assert ae_mt_resid < 1e-2, "%s: AE-MT max residual %.2e" % (label, ae_mt_resid)
        if active_cm:
            cm_resid = max(abs(polyval(cf, r)) for r in active_cm)
            assert cm_resid < 1e-2, "%s: CM max residual %.2e" % (label, cm_resid)

        print("  %s: AE OK, AE-MT(1) OK, AE-MT(4) OK, CM OK" % label)

    repeat_polys = [[1, -6, 11, -6] for _ in range(16)]
    write_cf("/tmp/repeat_cf.bin", repeat_polys, 4)
    r = subprocess.run(["/src/sweep", "/tmp/repeat_ae_out.bin"],
                       input=json.dumps({
                           "mode": "solve", "coeffs_file": "/tmp/repeat_cf.bin",
                           "n_coeffs": 4, "n2": len(repeat_polys),
                           "i1_start": 0, "i1_end": 1, "match_roots": False,
                       }),
                       capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, "repeat AE FAILED: " + r.stderr[:200]
    repeat_ae_meta = json.loads(r.stdout)
    r = subprocess.run(["/src/sweep_mt", "/tmp/repeat_mt_out.bin"],
                       input=json.dumps({
                           "mode": "solve_mt", "coeffs_file": "/tmp/repeat_cf.bin",
                           "n_coeffs": 4, "n2": len(repeat_polys),
                           "i1_start": 0, "i1_end": 1, "match_roots": False,
                           "n_threads": 4,
                       }),
                       capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, "repeat AE-MT FAILED: " + r.stderr[:200]
    repeat_mt_meta = json.loads(r.stdout)
    assert repeat_ae_meta["avg_iterations"] < repeat_mt_meta["avg_iterations"], (
        "AE warm-start chain should reduce avg_iterations relative to multi-thread cold starts at block boundaries"
    )

    print("=== AE/AE-MT/CM solver tests PASSED ===")


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


def test_compute_preview_runtime_combo():
    print("\n--- Compute preview runtime combo ---")

    env = {**os.environ, "LD_LIBRARY_PATH": "/opt/lib", "PATH": "/opt/bin:" + os.environ.get("PATH", "")}
    coeff_path = "/tmp/compute_preview_coeffs.bin"
    roots_path = "/tmp/compute_preview_roots.bin"

    coeff_spec = {
        "mode": "coeffgen",
        "function": "g1",
        "n1": 8,
        "n2": 8,
        "i1_start": 0,
        "i1_end": 8,
        "times": 1,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [["roots_cm", "hi"]],
    }
    r = subprocess.run(["/src/sweep_coeffgen", coeff_path],
                       input=json.dumps(coeff_spec), capture_output=True, text=True, timeout=30, env=env)
    assert r.returncode == 0, "sweep_coeffgen preview combo failed: " + r.stderr[:200]
    coeff_meta = json.loads(r.stdout)
    assert coeff_meta["n_coeffs"] >= 2, "unexpected n_coeffs %r" % coeff_meta
    assert os.path.getsize(coeff_path) == coeff_meta["data_bytes"], "coeff preview combo size mismatch"
    print("  sweep_coeffgen + roots_cm: OK (%s bytes)" % coeff_meta["data_bytes"])

    solve_spec = {
        "mode": "solve_cm",
        "coeffs_file": coeff_path,
        "n_coeffs": coeff_meta["n_coeffs"],
        "n_steps": 64,
    }
    r = subprocess.run(["/src/sweep_cm", roots_path],
                       input=json.dumps(solve_spec), capture_output=True, text=True, timeout=30, env=env)
    assert r.returncode == 0, "sweep_cm preview combo failed: " + r.stderr[:200]
    solve_meta = json.loads(r.stdout)
    degree = solve_meta["degree"]
    assert degree == coeff_meta["degree"], "degree mismatch coeffgen=%s solve=%s" % (coeff_meta["degree"], degree)
    roots = read_roots(roots_path, degree)
    assert len(roots) == 64, "expected 64 polynomials, got %d" % len(roots)
    finite = all(math.isfinite(z.real) and math.isfinite(z.imag) for poly in roots for z in poly)
    assert finite, "compute preview combo produced non-finite roots"
    print("  sweep_cm on preview coeffs: OK (degree=%d, polys=%d)" % (degree, len(roots)))

    cleanup(coeff_path, roots_path)
    print("=== Compute preview runtime combo PASSED ===")


# ── Palette Chunk MT Runtime ─────────────────────────────────────────────

def test_palette_chunk_mt_runtime():
    print("\n--- solve_palette_chunk_mt runtime ---")

    bin_path = "/src/solve_palette_chunk_mt"
    assert os.path.exists(bin_path), "%s not found" % bin_path
    assert open(bin_path, "rb").read(4) == b"\x7fELF", "solve_palette_chunk_mt is not ELF"

    r = subprocess.run(["ldd", bin_path], capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "ldd failed for solve_palette_chunk_mt: " + r.stderr[:200]
    assert "not found" not in r.stdout, "solve_palette_chunk_mt shared libs unresolved: " + r.stdout
    print("  ldd: OK")

    roots_path = "/tmp/palette_chunk_mt_roots.bin"
    scores_path = "/tmp/palette_chunk_mt_scores.bin"
    bins_path = "/tmp/palette_chunk_mt_bins.bin"
    with open(roots_path, "wb") as f:
        solves = [
            [(0.0, 0.0), (1.0, 0.0)],
            [(0.0, 0.0), (0.5, 0.0)],
            [(0.0, 0.0), (0.2, 0.0)],
            [(0.0, 0.0), (0.8, 0.0)],
        ]
        for roots in solves:
            for re, im in roots:
                f.write(struct.pack("<ff", re, im))

    r = subprocess.run([
        bin_path, roots_path,
        "--degree=2",
        "--metric=proximity",
        "--clip_lo=0.0",
        "--clip_hi=2.0",
        "--cuts=0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9",
        "--omega=1.0",
        "--step_count=4",
        "--threads=3",
        "--input_mode=tmpfile",
        "--retries=2",
        "--scores_out=" + scores_path,
        "--bins_out=" + bins_path,
    ], capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "solve_palette_chunk_mt failed: " + r.stderr[:200]
    meta = json.loads(r.stdout)
    assert meta["threads"] == 3, "solve_palette_chunk_mt did not report thread count"
    assert meta["input_mode"] == "tmpfile", "solve_palette_chunk_mt did not report input_mode"
    assert meta["retries"] == 2, "solve_palette_chunk_mt did not report retries"
    assert meta["n_samples"] == 4, "solve_palette_chunk_mt did not report n_samples"

    scores = read_f32_array(scores_path)
    with open(bins_path, "rb") as f:
        bins = f.read()
    assert len(scores) == 4, "unexpected score count for solve_palette_chunk_mt"
    assert len(bins) == 4, "unexpected bin count for solve_palette_chunk_mt"
    assert all(math.isfinite(v) for v in scores), "solve_palette_chunk_mt produced non-finite scores"
    assert all(0 <= b <= 9 for b in bins), "solve_palette_chunk_mt produced invalid bins"
    print("  solve_palette_chunk_mt: OK (threads=%d, bytes=%d)" % (meta["threads"], meta["bytes_downloaded"]))

    cleanup(roots_path, scores_path, bins_path)
    print("=== solve_palette_chunk_mt runtime PASSED ===")


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


# ── Resize Artifact libvips Runtime ──────────────────────────────────────

def test_resize_runtime():
    print("\n--- Resize artifact libvips runtime ---")

    vips_path = "/opt/bin/vips"
    vt_path = "/opt/bin/vipsthumbnail"
    assert os.path.exists(vips_path), "vips not found at %s" % vips_path
    assert os.path.exists(vt_path), "vipsthumbnail not found at %s" % vt_path

    r = subprocess.run([vips_path, "--vips-version"], capture_output=True, text=True, timeout=5)
    assert r.returncode == 0, "vips --vips-version failed: " + r.stderr[:200]
    print("  vips: %s" % (r.stdout.strip() or r.stderr.strip()))

    r = subprocess.run([vt_path, "--vips-version"], capture_output=True, text=True, timeout=5)
    assert r.returncode == 0, "vipsthumbnail --vips-version failed: " + r.stderr[:200]
    print("  vipsthumbnail: %s" % (r.stdout.strip() or r.stderr.strip()))

    env = {**os.environ, "LD_LIBRARY_PATH": "/opt/lib", "PATH": "/opt/bin:" + os.environ.get("PATH", "")}
    test_raw = "/tmp/resize_rt.raw"
    test_jpeg = "/tmp/resize_rt.jpeg"
    thumb_png = "/tmp/resize_rt_thumb.png"
    resize_png = "/tmp/resize_rt_resized.png"
    resize_v_png = "/tmp/resize_rt_resized_v.png"

    with open(test_raw, "wb") as f:
        f.write(struct.pack("<III", 64, 32, 3))
        for y in range(32):
            for x in range(64):
                f.write(bytes([(x * 4) % 256, (y * 8) % 256, ((x + y) * 3) % 256]))

    r = subprocess.run(["/src/raw2jpeg", test_raw, test_jpeg, "--quality=90"],
                       capture_output=True, text=True, timeout=10, env=env)
    assert r.returncode == 0, "raw2jpeg failed: " + r.stderr[:200]
    assert os.path.getsize(test_jpeg) > 0, "runtime resize test JPEG is empty"
    jpeg_dims = read_jpeg_dims(test_jpeg)
    assert jpeg_dims == (64, 32), "unexpected source JPEG dims %r" % (jpeg_dims,)
    print("  source JPEG: %dx%d" % jpeg_dims)

    r = subprocess.run([
        vips_path, "thumbnail", test_jpeg, thumb_png, "16",
        "--height", "16", "--size", "down", "--linear", "--crop", "attention",
        "--intent", "perceptual", "--fail-on", "none"
    ], capture_output=True, text=True, timeout=20, env=env)
    assert r.returncode == 0, "vips thumbnail failed: " + r.stderr[:200]
    thumb_dims = read_png_dims(thumb_png)
    assert thumb_dims == (16, 16), "unexpected thumbnail dims %r" % (thumb_dims,)
    print("  vips thumbnail: OK (%dx%d)" % thumb_dims)

    r = subprocess.run([
        vips_path, "resize", test_jpeg, resize_png,
        str(32.0 / 64.0),
        "--kernel", "mitchell",
        "--gap", "2"
    ], capture_output=True, text=True, timeout=20, env=env)
    assert r.returncode == 0, "vips resize failed: " + r.stderr[:200]
    resize_dims = read_png_dims(resize_png)
    assert resize_dims == (32, 16), "unexpected resized dims %r" % (resize_dims,)
    print("  vips resize: OK (%dx%d)" % resize_dims)

    r = subprocess.run([
        vips_path, "resize", test_jpeg, resize_v_png,
        str(32.0 / 64.0),
        "--kernel", "mitchell",
        "--gap", "2",
        "--vscale", "1"
    ], capture_output=True, text=True, timeout=20, env=env)
    assert r.returncode == 0, "vips resize --vscale failed: " + r.stderr[:200]
    resize_v_dims = read_png_dims(resize_v_png)
    assert resize_v_dims == (32, 32), "unexpected resized --vscale dims %r" % (resize_v_dims,)
    print("  vips resize + vscale=1: OK (%dx%d)" % resize_v_dims)

    cleanup(test_raw, test_jpeg, thumb_png, resize_png, resize_v_png)
    print("=== Resize artifact runtime tests PASSED ===")


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

    # 3b. Score program ops, including m* binary ops and unary post-process ops, must parse and run.
    for program in ["m0;m1;mul", "m0;m1;max"]:
        r = subprocess.run(
            [
                sps_path,
                sps_bin,
                "--mode=summary",
                "--degree=2",
                "--score_metrics=proximity,spread",
                "--score_clip_los=" + str(clip["clip_lo"]) + "," + str(spread["clip_lo"]),
                "--score_clip_his=" + str(clip["clip_hi"]) + "," + str(spread["clip_hi"]),
                "--score_program=" + program,
            ],
            capture_output=True,
            text=True,
            timeout=10,
        )
        assert r.returncode == 0, f"{program} summary failed: " + r.stderr[:200]
        summary = json.loads(r.stdout)
        assert summary["n_solves"] == 3
        print(f"  score program {program}: OK")

    for program in ["m0;flip", "m0;sawtooth:10", "m0;omega_cosine:3:1.57079632679"]:
        r = subprocess.run(
            [
                sps_path,
                sps_bin,
                "--mode=summary",
                "--degree=2",
                "--score_metrics=proximity",
                "--score_clip_los=" + str(clip["clip_lo"]),
                "--score_clip_his=" + str(clip["clip_hi"]),
                "--score_program=" + program,
            ],
            capture_output=True,
            text=True,
            timeout=10,
        )
        assert r.returncode == 0, f"{program} summary failed: " + r.stderr[:200]
        summary = json.loads(r.stdout)
        assert summary["n_solves"] == 3
        print(f"  score program {program}: OK")

    # 4. Clusteriness clip (v2 metric)
    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2", "--metric=clusteriness"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "clusteriness clip failed: " + r.stderr[:200]
    clust = json.loads(r.stdout)
    assert clust["metric"] == "clusteriness"
    assert clust["n_solves"] == 3
    print("  clusteriness clip: OK (lo=%.2f, hi=%.2f)" % (clust["clip_lo"], clust["clip_hi"]))

    # 5. Real-axis proximity clip (v2 metric)
    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2", "--metric=real_axis_proximity"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "real_axis_proximity clip failed: " + r.stderr[:200]
    rap = json.loads(r.stdout)
    assert rap["metric"] == "real_axis_proximity"
    assert rap["n_solves"] == 3
    print("  real_axis_proximity clip: OK (lo=%.2f, hi=%.2f)" % (rap["clip_lo"], rap["clip_hi"]))

    # 5b. New v4 metrics
    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2", "--metric=min_mod"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "min_mod clip failed: " + r.stderr[:200]
    min_mod = json.loads(r.stdout)
    assert min_mod["metric"] == "min_mod"
    assert min_mod["n_solves"] == 3
    print("  min_mod clip: OK (lo=%.2f, hi=%.2f)" % (min_mod["clip_lo"], min_mod["clip_hi"]))

    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2", "--metric=min_angular_separation"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "min_angular_separation clip failed: " + r.stderr[:200]
    min_ang = json.loads(r.stdout)
    assert min_ang["metric"] == "min_angular_separation"
    assert min_ang["n_solves"] == 3
    print("  min_angular_separation clip: OK (lo=%.2f, hi=%.2f)" % (min_ang["clip_lo"], min_ang["clip_hi"]))

    # 6. Non-default quantile clip (q=0.05)
    _write_sps_bin(sps_bin)
    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2", "--metric=proximity",
                        "--quantile_lo=0.05", "--quantile_hi=0.95"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "q=0.05 clip failed: " + r.stderr[:200]
    q05 = json.loads(r.stdout)
    assert q05["n_solves"] == 3
    print("  proximity clip q=5%%: OK (lo=%.2f, hi=%.2f)" % (q05["clip_lo"], q05["clip_hi"]))

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

    # Smoke for clusteriness (v2 metric)
    r = subprocess.run([
        r2p_path, sps_bin, "/tmp/r2p_ss_pix",
        "--width=4", "--height=4", "--tile_size=4",
        "--n_tile_cols=1", "--n_tile_rows=1",
        "--center_re=0.5", "--center_im=0",
        "--scale=2", "--degree=2",
        "--color=solve_score",
        "--solve_metric=clusteriness",
        "--solve_score_clip_lo=0.0",
        "--solve_score_clip_hi=2.0",
        "--solve_score_cuts=0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9",
    ], capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "roots2pix solve_score clusteriness failed: " + r.stderr[:200]
    meta2 = json.loads(r.stdout)
    print("  roots2pix --solve_metric=clusteriness: OK (plotted=%d)" % meta2["roots_plotted"])

    # Smoke for max_mod (v4 metric)
    r = subprocess.run([
        r2p_path, sps_bin, "/tmp/r2p_ss_pix",
        "--width=4", "--height=4", "--tile_size=4",
        "--n_tile_cols=1", "--n_tile_rows=1",
        "--center_re=0.5", "--center_im=0",
        "--scale=2", "--degree=2",
        "--color=solve_score",
        "--solve_metric=max_mod",
        "--solve_score_clip_lo=0.0",
        "--solve_score_clip_hi=2.0",
        "--solve_score_cuts=0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9",
    ], capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "roots2pix solve_score max_mod failed: " + r.stderr[:200]
    meta3 = json.loads(r.stdout)
    print("  roots2pix --solve_metric=max_mod: OK (plotted=%d)" % meta3["roots_plotted"])

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
    for bin_path in ["/src/sweep", "/src/sweep_mt", "/src/sweep_cm", "/src/sweep_coeffgen", "/src/solve_palette_chunk_mt"]:
        magic = open(bin_path, "rb").read(4)
        assert magic == b"\x7fELF", "%s is not an ELF binary" % bin_path
        print("  %s: ELF OK" % bin_path)

    print("--- Generating test fixtures ---")
    test_ae_cm_solvers()
    test_cfpv_coeffgen()
    test_compute_preview_runtime_combo()
    test_palette_chunk_mt_runtime()
    test_render_preview()
    test_resize_runtime()
    test_solve_proximity_stats()
    test_roots2pix_solve_score()
    test_catalog_degrees()

    print("\n=== All Docker runtime tests PASSED ===")
