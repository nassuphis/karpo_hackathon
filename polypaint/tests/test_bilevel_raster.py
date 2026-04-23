"""
Test bilevel_raster C binary against Python reference.

Generates roots via sweep_test, runs bilevel_raster to produce .bits files,
then verifies the bitset content matches Python-computed pixel positions.

Run: cd polypaint/tests && uv run python test_bilevel_raster.py
"""
import json
import math
import os
import shutil
import struct
import subprocess
import tempfile
import numpy as np

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")
BILEVEL_RASTER = os.path.join(LAMBDA_DIR, "bilevel_raster_local")

# If no local build, try the regular name
if not os.path.exists(BILEVEL_RASTER):
    BILEVEL_RASTER = os.path.join(LAMBDA_DIR, "bilevel_raster")


def run_sweep(func_name, n):
    """Generate roots via sweep_test."""
    coeffs_file = "/tmp/test_bl_coeffs.bin"
    roots_file = "/tmp/test_bl_roots.bin"

    spec = json.dumps({
        "mode": "coeffgen", "function": func_name,
        "n1": n, "n2": n, "i1_start": 0, "i1_end": n,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": ["rev"], "times": 1, "dither_pass": 0,
    })
    r = subprocess.run([SWEEP, coeffs_file], input=spec, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"coeffgen failed: {r.stderr}"
    meta1 = json.loads(r.stdout)

    spec2 = json.dumps({
        "mode": "solve", "coeffs_file": coeffs_file,
        "n_coeffs": meta1["n_coeffs"], "n1": n, "n2": n,
        "i1_start": 0, "i1_end": n, "match_roots": False,
    })
    r2 = subprocess.run([SWEEP, roots_file], input=spec2, capture_output=True, text=True, timeout=60)
    assert r2.returncode == 0, f"solve failed: {r2.stderr}"
    meta2 = json.loads(r2.stdout)

    with open(roots_file, "rb") as f:
        data = f.read()
    degree = meta2["degree"]
    n_t = meta2["n_t"]
    floats = np.frombuffer(data[:n_t * degree * 2 * 4], dtype=np.float32)
    roots = floats.reshape(n_t, degree, 2)
    return roots, degree, roots_file


def python_project(roots, degree, width, height, center_re, center_im, scale,
                   tile_size, n_tile_cols, n_tile_rows, rotation=0.0,
                   min_re=None, max_re=None, min_im=None, max_im=None):
    """Python reference: project roots to tile bitsets."""
    cos_a = math.cos(rotation)
    sin_a = math.sin(rotation)
    if min_re is None or max_re is None or min_im is None or max_im is None:
        half_w = width / 2.0
        half_h = height / 2.0
        project = lambda re, im: (
            int(half_w + ((re - center_re) * cos_a - (im - center_im) * sin_a) * scale),
            int(half_h - ((re - center_re) * sin_a + (im - center_im) * cos_a) * scale),
        )
    else:
        center_re = (min_re + max_re) / 2.0
        center_im = (min_im + max_im) / 2.0
        x_scale = width / (max_re - min_re)
        y_scale = height / (max_im - min_im)
        project = lambda re, im: (
            int(math.floor(((center_re + ((re - center_re) * cos_a - (im - center_im) * sin_a)) - min_re) * x_scale)),
            int(math.floor((max_im - (center_im + ((re - center_re) * sin_a + (im - center_im) * cos_a))) * y_scale)),
        )
    n_tiles = n_tile_cols * n_tile_rows

    # Compute tile dimensions
    tile_w = []
    tile_h = []
    for t in range(n_tiles):
        tc = t % n_tile_cols
        tr = t // n_tile_cols
        tw = tile_size if tc < n_tile_cols - 1 else width - tc * tile_size
        th = tile_size if tr < n_tile_rows - 1 else height - tr * tile_size
        tile_w.append(tw)
        tile_h.append(th)

    # Bitsets
    bitsets = [bytearray((tw * th + 7) // 8) for tw, th in zip(tile_w, tile_h)]

    plotted = 0
    clipped = 0
    deduped = 0
    n_points = roots.shape[0]

    for p in range(n_points):
        for i in range(degree):
            re = float(roots[p, i, 0])
            im = float(roots[p, i, 1])
            px, py = project(re, im)

            if px < 0 or px >= width or py < 0 or py >= height:
                clipped += 1
                continue

            tc = px // tile_size
            tr = py // tile_size
            tile_id = tr * n_tile_cols + tc
            if tile_id < 0 or tile_id >= n_tiles:
                clipped += 1
                continue

            lx = px - tc * tile_size
            ly = py - tr * tile_size
            bit_idx = ly * tile_w[tile_id] + lx

            byte_idx = bit_idx >> 3
            mask = 1 << (bit_idx & 7)
            if bitsets[tile_id][byte_idx] & mask:
                deduped += 1
                continue
            bitsets[tile_id][byte_idx] |= mask
            plotted += 1

    return bitsets, plotted, clipped, deduped


def _bounds_args_from_center_scale(width, height, center_re, center_im, scale):
    half_w_world = (float(width) / 2.0) / float(scale)
    half_h_world = (float(height) / 2.0) / float(scale)
    return [
        f"--min_re={center_re - half_w_world}",
        f"--max_re={center_re + half_w_world}",
        f"--min_im={center_im - half_h_world}",
        f"--max_im={center_im + half_h_world}",
    ]


def test_basic():
    """Test bilevel_raster against Python reference for poly_1."""
    print("test_basic: poly_1, 50x50 grid, 1000x1000 image, 2x2 tiles...")
    roots, degree, roots_file = run_sweep("poly_1", 50)

    width = height = 1000
    tile_size = 500
    n_tile_cols = n_tile_rows = 2
    center_re = center_im = 0.0
    scale = 250.0

    # Run C binary
    out_prefix = "/tmp/test_bl_bits"
    cmd = [
        BILEVEL_RASTER, roots_file, out_prefix,
        f"--pix={width}",
        f"--tile_size={tile_size}",
        f"--n_tile_cols={n_tile_cols}", f"--n_tile_rows={n_tile_rows}",
        *_bounds_args_from_center_scale(width, height, center_re, center_im, scale),
        f"--degree={degree}",
    ]
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"bilevel_raster failed: {r.stderr}"
    c_meta = json.loads(r.stdout)

    # Run Python reference
    py_bitsets, py_plotted, py_clipped, py_deduped = python_project(
        roots, degree, width, height, center_re, center_im, scale,
        tile_size, n_tile_cols, n_tile_rows
    )

    # Compare counts
    print(f"  C:  plotted={c_meta['roots_plotted']} clipped={c_meta['roots_clipped']} deduped={c_meta['roots_deduped']}")
    print(f"  Py: plotted={py_plotted} clipped={py_clipped} deduped={py_deduped}")
    assert c_meta["roots_plotted"] == py_plotted, f"plotted mismatch: C={c_meta['roots_plotted']} Py={py_plotted}"
    assert c_meta["roots_clipped"] == py_clipped, f"clipped mismatch: C={c_meta['roots_clipped']} Py={py_clipped}"

    # Compare bitset contents
    n_tiles = n_tile_cols * n_tile_rows
    for t in range(n_tiles):
        bits_path = f"{out_prefix}_t{t:04d}.bits"
        if os.path.exists(bits_path):
            with open(bits_path, "rb") as f:
                c_bits = f.read()
            py_bits = bytes(py_bitsets[t])
            assert c_bits == py_bits, f"tile {t} bitset mismatch: C={len(c_bits)} bytes, Py={len(py_bits)} bytes, diff at byte {next(i for i,(a,b) in enumerate(zip(c_bits,py_bits)) if a!=b)}"
            os.remove(bits_path)
            print(f"  tile {t}: OK ({len(c_bits)} bytes, {sum(bin(b).count('1') for b in c_bits)} pixels)")
        else:
            # No C output — check Python is also empty
            assert all(b == 0 for b in py_bitsets[t]), f"tile {t}: C produced no output but Py has data"

    print("  PASS")


def test_rotation():
    """Test bilevel_raster with rotation."""
    print("test_rotation: poly_1, 30x30 grid, 500x500 image, rotation=0.25 turns...")
    roots, degree, roots_file = run_sweep("poly_1", 30)

    width = height = 500
    tile_size = 250
    n_tile_cols = n_tile_rows = 2
    center_re = center_im = 0.0
    scale = 125.0
    rotation = 0.25 * 2 * math.pi  # quarter turn

    # Run C binary
    out_prefix = "/tmp/test_bl_rot"
    cmd = [
        BILEVEL_RASTER, roots_file, out_prefix,
        f"--pix={width}",
        f"--tile_size={tile_size}",
        f"--n_tile_cols={n_tile_cols}", f"--n_tile_rows={n_tile_rows}",
        *_bounds_args_from_center_scale(width, height, center_re, center_im, scale),
        f"--degree={degree}",
        f"--rotation={rotation}",
    ]
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"bilevel_raster failed: {r.stderr}"
    c_meta = json.loads(r.stdout)

    # Run Python reference
    py_bitsets, py_plotted, py_clipped, py_deduped = python_project(
        roots, degree, width, height, center_re, center_im, scale,
        tile_size, n_tile_cols, n_tile_rows, rotation
    )

    assert c_meta["roots_plotted"] == py_plotted, f"plotted mismatch: C={c_meta['roots_plotted']} Py={py_plotted}"

    # Compare bitsets
    n_tiles = n_tile_cols * n_tile_rows
    for t in range(n_tiles):
        bits_path = f"{out_prefix}_t{t:04d}.bits"
        if os.path.exists(bits_path):
            with open(bits_path, "rb") as f:
                c_bits = f.read()
            py_bits = bytes(py_bitsets[t])
            assert c_bits == py_bits, f"tile {t} bitset mismatch"
            os.remove(bits_path)
        else:
            assert all(b == 0 for b in py_bitsets[t])

    print(f"  plotted={py_plotted}, PASS")


def test_empty_tiles():
    """Test that tiles with no roots produce no .bits file."""
    print("test_empty_tiles: poly_1, 10x10 grid, small viewport (most roots clipped)...")
    roots, degree, roots_file = run_sweep("poly_1", 10)

    width = height = 100
    tile_size = 50
    n_tile_cols = n_tile_rows = 2
    center_re = 10.0  # offset viewport so most roots miss
    center_im = 10.0
    scale = 25.0

    out_prefix = "/tmp/test_bl_empty"
    cmd = [
        BILEVEL_RASTER, roots_file, out_prefix,
        f"--pix={width}",
        f"--tile_size={tile_size}",
        f"--n_tile_cols={n_tile_cols}", f"--n_tile_rows={n_tile_rows}",
        *_bounds_args_from_center_scale(width, height, center_re, center_im, scale),
        f"--degree={degree}",
    ]
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"bilevel_raster failed: {r.stderr}"
    c_meta = json.loads(r.stdout)

    print(f"  plotted={c_meta['roots_plotted']} clipped={c_meta['roots_clipped']} tiles_written={c_meta['tiles_written']}")

    # Verify mostly clipped
    assert c_meta["roots_clipped"] > c_meta["roots_plotted"], "expected most roots to be clipped"

    # Clean up any .bits files
    for t in range(4):
        p = f"{out_prefix}_t{t:04d}.bits"
        if os.path.exists(p):
            os.remove(p)

    print("  PASS")


def test_exact_bounds_square_output_mapping():
    """Test exact bounds projection on a square output with anisotropic world scaling."""
    print("test_exact_bounds_square_output_mapping: manual roots, exact bounds, square output...")
    degree = 1
    roots = np.array([
        [[-2.0, -1.0]],
        [[1.0, 0.0]],
        [[3.0, 1.0]],
        [[0.0, 2.0]],
        [[5.0, 5.0]],
    ], dtype=np.float32)
    fd, roots_file = tempfile.mkstemp(suffix=".bin")
    os.close(fd)
    roots.tofile(roots_file)

    width = height = 100
    tile_size = 50
    n_tile_cols = n_tile_rows = 2
    min_re, max_re = -2.0, 4.0
    min_im, max_im = -1.0, 2.0

    out_prefix = "/tmp/test_bl_bounds"
    cmd = [
        BILEVEL_RASTER, roots_file, out_prefix,
        f"--pix={width}",
        f"--tile_size={tile_size}",
        f"--n_tile_cols={n_tile_cols}", f"--n_tile_rows={n_tile_rows}",
        f"--min_re={min_re}", f"--max_re={max_re}",
        f"--min_im={min_im}", f"--max_im={max_im}",
        f"--degree={degree}",
    ]
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"bilevel_raster failed: {r.stderr}"
    c_meta = json.loads(r.stdout)

    py_bitsets, py_plotted, py_clipped, py_deduped = python_project(
        roots, degree, width, height, 0.0, 0.0, 1.0,
        tile_size, n_tile_cols, n_tile_rows,
        min_re=min_re, max_re=max_re, min_im=min_im, max_im=max_im
    )

    assert c_meta["roots_plotted"] == py_plotted
    assert c_meta["roots_clipped"] == py_clipped
    assert c_meta["roots_deduped"] == py_deduped

    n_tiles = n_tile_cols * n_tile_rows
    for t in range(n_tiles):
        bits_path = f"{out_prefix}_t{t:04d}.bits"
        if os.path.exists(bits_path):
            with open(bits_path, "rb") as f:
                c_bits = f.read()
            assert c_bits == bytes(py_bitsets[t]), f"tile {t} bitset mismatch under exact bounds"
            os.remove(bits_path)
        else:
            assert all(b == 0 for b in py_bitsets[t])

    os.remove(roots_file)
    print("  PASS")


def test_multiple_functions():
    """Test with a few different poly functions."""
    for func_name in ["poly_1", "poly_4", "poly_49"]:
        print(f"test_multiple: {func_name}...")
        roots, degree, roots_file = run_sweep(func_name, 30)

        width = height = 800
        tile_size = 400
        n_tile_cols = n_tile_rows = 2
        scale = 200.0

        out_prefix = f"/tmp/test_bl_{func_name}"
        cmd = [
            BILEVEL_RASTER, roots_file, out_prefix,
            f"--pix={width}",
            f"--tile_size={tile_size}",
            f"--n_tile_cols={n_tile_cols}", f"--n_tile_rows={n_tile_rows}",
            *_bounds_args_from_center_scale(width, height, 0.0, 0.0, scale),
            f"--degree={degree}",
        ]
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
        assert r.returncode == 0, f"bilevel_raster failed for {func_name}: {r.stderr}"
        c_meta = json.loads(r.stdout)

        # Python reference
        py_bitsets, py_plotted, _, _ = python_project(
            roots, degree, width, height, 0, 0, scale,
            tile_size, n_tile_cols, n_tile_rows
        )
        assert c_meta["roots_plotted"] == py_plotted, f"{func_name}: plotted mismatch C={c_meta['roots_plotted']} Py={py_plotted}"

        # Compare bitsets
        for t in range(4):
            bits_path = f"{out_prefix}_t{t:04d}.bits"
            if os.path.exists(bits_path):
                with open(bits_path, "rb") as f:
                    c_bits = f.read()
                assert c_bits == bytes(py_bitsets[t]), f"{func_name} tile {t} mismatch"
                os.remove(bits_path)

        print(f"  {func_name}: plotted={py_plotted}, PASS")


def test_moebius_all_zero_params_plots_nothing():
    """Undefined moebius transform should not leak bogus plotted pixels."""
    roots = struct.pack(
        "<8f",
        0.0, 0.0,
        1.0, 0.0,
        -1.0, 0.5,
        0.25, -0.75,
    )
    with tempfile.TemporaryDirectory(prefix="bilevel_moebius_") as td:
        cc = shutil.which("cc") or shutil.which("clang") or shutil.which("gcc")
        assert cc, "no C compiler available"
        binary = os.path.join(td, "bilevel_raster_test")
        compile_cmd = [
            cc,
            "-O2",
            "-I",
            LAMBDA_DIR,
            os.path.join(LAMBDA_DIR, "bilevel_raster.c"),
            "-lm",
            "-o",
            binary,
        ]
        compiled = subprocess.run(compile_cmd, capture_output=True, text=True, timeout=30)
        assert compiled.returncode == 0, f"failed to compile bilevel_raster: {compiled.stderr}"

        roots_file = os.path.join(td, "roots.bin")
        out_prefix = os.path.join(td, "bits")
        xforms_path = os.path.join(td, "root_xforms.json")
        with open(roots_file, "wb") as fh:
            fh.write(roots)
        with open(xforms_path, "w") as fh:
            json.dump([["moebius", "0", "0", "0", "0"]], fh)

        cmd = [
            binary, roots_file, out_prefix,
            "--pix=16",
            "--tile_size=16",
            "--n_tile_cols=1", "--n_tile_rows=1",
            *_bounds_args_from_center_scale(16, 16, 0.0, 0.0, 4.0),
            "--degree=2",
            f"--root_xforms={xforms_path}",
        ]
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
        assert r.returncode == 0, f"bilevel_raster failed: {r.stderr}"
        meta = json.loads(r.stdout)
        assert meta["roots_plotted"] == 0
        assert meta["roots_clipped"] == 4
        assert meta["tiles_written"] == 0
        assert not os.path.exists(f"{out_prefix}_t0000.bits")


if __name__ == "__main__":
    test_basic()
    test_rotation()
    test_empty_tiles()
    test_multiple_functions()
    print("\nAll bilevel_raster tests passed.")
