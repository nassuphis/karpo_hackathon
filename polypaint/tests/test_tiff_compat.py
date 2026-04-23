"""
Test tiff_compat: tiled TIFF → strip-based TIFF conversion.

Run: cd polypaint/tests && uv run python test_tiff_compat.py
"""
import os
import subprocess
import numpy as np

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
TIFF_COMPAT = os.path.join(LAMBDA_DIR, "tiff_compat_local")
BILEVEL_MERGE = os.path.join(LAMBDA_DIR, "bilevel_merge_local")

if not os.path.exists(TIFF_COMPAT):
    raise FileNotFoundError(
        f"tiff_compat_local not found. Compile: cd polypaint/lambda && "
        "cc -O3 -o tiff_compat_local tiff_compat.c $(pkg-config --cflags --libs vips) "
        "$(pkg-config --cflags --libs libtiff-4) -lm"
    )
if not os.path.exists(BILEVEL_MERGE):
    raise FileNotFoundError("bilevel_merge_local not found")


def make_bitset(w, h, pixels):
    bs = bytearray((w * h + 7) // 8)
    for x, y in pixels:
        idx = y * w + x
        bs[idx >> 3] |= 1 << (idx & 7)
    return bytes(bs)


def make_tile_tiff(path, w, h, pixels):
    bits_path = path + ".bits"
    with open(bits_path, "wb") as f:
        f.write(make_bitset(w, h, pixels))
    r = subprocess.run([BILEVEL_MERGE, "merge", f"--tile_w={w}", f"--tile_h={h}",
                        f"--output={path}", bits_path], capture_output=True, text=True)
    os.remove(bits_path)
    assert r.returncode == 0


def make_tiled_tiff(path, fullW, fullH, tileSz, nCols, nRows, tile_pixels):
    """Create a tiled TIFF via bilevel_merge stitch."""
    tile_paths = []
    for t in range(nCols * nRows):
        tc, tr = t % nCols, t // nCols
        tw = tileSz if tc < nCols - 1 or fullW % tileSz == 0 else fullW - tc * tileSz
        th = tileSz if tr < nRows - 1 or fullH % tileSz == 0 else fullH - tr * tileSz
        tp = f"/tmp/compat_tile_{t}.tif"
        make_tile_tiff(tp, tw, th, tile_pixels.get(t, []))
        tile_paths.append(tp)
    cmd = [BILEVEL_MERGE, "stitch", f"--n_cols={nCols}", f"--n_rows={nRows}",
           f"--pix={fullW}", f"--tile_size={tileSz}",
           f"--output={path}"] + tile_paths
    r = subprocess.run(cmd, capture_output=True, text=True)
    for p in tile_paths:
        os.remove(p)
    assert r.returncode == 0


def read_tiff(path):
    from PIL import Image
    img = Image.open(path)
    w, h = img.size
    arr = np.array(img)
    if arr.dtype == np.bool_:
        arr = arr.astype(np.uint8) * 255
    return w, h, arr.reshape(h, w)


def test_basic_conversion():
    """Convert a tiled TIFF to strip-based and verify content is preserved."""
    print("test_basic_conversion...")
    pixels = {0: [(1, 1)], 1: [(2, 2)], 2: [(0, 0)], 3: [(3, 3)]}
    make_tiled_tiff("/tmp/compat_src.tif", 32, 32, 16, 2, 2, pixels)

    r = subprocess.run([TIFF_COMPAT, "/tmp/compat_src.tif", "/tmp/compat_out.tif"],
                       capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"tiff_compat failed: {r.stderr}"

    # Verify dimensions preserved
    sw, sh, sarr = read_tiff("/tmp/compat_src.tif")
    cw, ch, carr = read_tiff("/tmp/compat_out.tif")
    assert sw == cw and sh == ch, f"dimensions differ: {sw}x{sh} vs {cw}x{ch}"

    # Verify content identical
    assert np.array_equal(sarr > 0, carr > 0), "pixel content differs"

    # Verify it's strip-based (sips-readable)
    info = subprocess.run(["tiffinfo", "/tmp/compat_out.tif"],
                          capture_output=True, text=True)
    assert "Rows/Strip" in info.stdout, "output should be strip-based"
    assert "Tile Width" not in info.stdout, "output should not be tiled"

    os.remove("/tmp/compat_src.tif")
    os.remove("/tmp/compat_out.tif")
    print("  PASS")


def test_edge_tiles():
    """Convert a non-multiple-sized tiled TIFF."""
    print("test_edge_tiles: 40x40, tileSize=16...")
    pixels = {0: [(1, 1)], 4: [(3, 3)], 8: [(1, 1)]}
    make_tiled_tiff("/tmp/compat_edge_src.tif", 40, 40, 16, 3, 3, pixels)

    r = subprocess.run([TIFF_COMPAT, "/tmp/compat_edge_src.tif", "/tmp/compat_edge_out.tif"],
                       capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"failed: {r.stderr}"

    cw, ch, carr = read_tiff("/tmp/compat_edge_out.tif")
    assert cw == 40 and ch == 40, f"dimensions wrong: {cw}x{ch}"

    # Verify marker pixels
    assert carr[1, 1] > 0, "tile 0 marker missing"
    assert carr[16 + 3, 16 + 3] > 0, "tile 4 marker missing"
    assert carr[32 + 1, 32 + 1] > 0, "tile 8 marker missing"

    os.remove("/tmp/compat_edge_src.tif")
    os.remove("/tmp/compat_edge_out.tif")
    print("  PASS")


def test_square_remainder():
    """Convert a square tiled TIFF with remainder edge tiles."""
    print("test_square_remainder: 40x40...")
    pixels = {0: [(2, 2)], 8: [(1, 1)]}
    make_tiled_tiff("/tmp/compat_nsq_src.tif", 40, 40, 16, 3, 3, pixels)

    r = subprocess.run([TIFF_COMPAT, "/tmp/compat_nsq_src.tif", "/tmp/compat_nsq_out.tif"],
                       capture_output=True, text=True, timeout=30)
    assert r.returncode == 0

    cw, ch, _ = read_tiff("/tmp/compat_nsq_out.tif")
    assert cw == 40 and ch == 40, f"dimensions wrong: {cw}x{ch}"

    os.remove("/tmp/compat_nsq_src.tif")
    os.remove("/tmp/compat_nsq_out.tif")
    print("  PASS")


def test_error_on_missing_input():
    """Conversion should fail on missing input."""
    print("test_error_on_missing_input...")
    r = subprocess.run([TIFF_COMPAT, "/tmp/nonexistent.tif", "/tmp/compat_err.tif"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode != 0
    print("  PASS")


if __name__ == "__main__":
    test_basic_conversion()
    test_edge_tiles()
    test_square_remainder()
    test_error_on_missing_input()
    print("\nAll tiff_compat tests passed.")
