"""
Test png_export: bilevel TIFF → 1-bit PNG conversion.

Run: cd polypaint/tests && uv run python test_png_export.py
"""
import os
import subprocess
import numpy as np

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
PNG_EXPORT = os.path.join(LAMBDA_DIR, "png_export_local")
BILEVEL_MERGE = os.path.join(LAMBDA_DIR, "bilevel_merge_local")

if not os.path.exists(PNG_EXPORT):
    raise FileNotFoundError(
        f"png_export_local not found. Compile: cd polypaint/lambda && "
        "cc -O3 -o png_export_local png_export.c $(pkg-config --cflags --libs vips) -lm"
    )


def make_bitset(w, h, pixels):
    bs = bytearray((w * h + 7) // 8)
    for x, y in pixels:
        idx = y * w + x
        bs[idx >> 3] |= 1 << (idx & 7)
    return bytes(bs)


def make_tiled_tiff(path, fullW, fullH, tileSz, nCols, nRows, tile_pixels):
    tile_paths = []
    for t in range(nCols * nRows):
        tc, tr = t % nCols, t // nCols
        tw = tileSz if tc < nCols - 1 or fullW % tileSz == 0 else fullW - tc * tileSz
        th = tileSz if tr < nRows - 1 or fullH % tileSz == 0 else fullH - tr * tileSz
        tp = f"/tmp/pngexp_tile_{t}.tif"
        bits_path = tp + ".bits"
        with open(bits_path, "wb") as f:
            f.write(make_bitset(tw, th, tile_pixels.get(t, [])))
        subprocess.run([BILEVEL_MERGE, "merge", f"--tile_w={tw}", f"--tile_h={th}",
                        f"--output={tp}", bits_path], capture_output=True)
        os.remove(bits_path)
        tile_paths.append(tp)
    cmd = [BILEVEL_MERGE, "stitch", f"--n_cols={nCols}", f"--n_rows={nRows}",
           f"--width={fullW}", f"--height={fullH}", f"--tile_size={tileSz}",
           f"--output={path}"] + tile_paths
    subprocess.run(cmd, capture_output=True)
    for p in tile_paths:
        os.remove(p)


def read_png(path):
    from PIL import Image
    img = Image.open(path)
    w, h = img.size
    arr = np.array(img)
    if arr.dtype == np.bool_:
        arr = arr.astype(np.uint8) * 255
    return w, h, arr.reshape(h, w)


def test_basic():
    """Convert a bilevel TIFF to PNG, verify dimensions and content."""
    print("test_basic...")
    pixels = {0: [(1, 1)], 1: [(3, 2)], 2: [(0, 0)], 3: [(2, 3)]}
    make_tiled_tiff("/tmp/pngexp_src.tif", 32, 32, 16, 2, 2, pixels)

    r = subprocess.run([PNG_EXPORT, "/tmp/pngexp_src.tif", "/tmp/pngexp_out.png"],
                       capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"failed: {r.stderr}"

    pw, ph, parr = read_png("/tmp/pngexp_out.png")
    assert pw == 32 and ph == 32, f"dimensions wrong: {pw}x{ph}"

    # Verify markers
    assert parr[1, 1] > 0, "tile 0 marker missing"
    assert parr[2, 16 + 3] > 0, "tile 1 marker missing"
    assert parr[16, 0] > 0, "tile 2 marker missing"
    assert parr[16 + 3, 16 + 2] > 0, "tile 3 marker missing"

    # Verify total pixel count
    total = int(np.sum(parr > 0))
    assert total == 4, f"expected 4 white pixels, got {total}"

    # Verify it's actually a PNG
    with open("/tmp/pngexp_out.png", "rb") as f:
        magic = f.read(4)
    assert magic == b'\x89PNG', "not a valid PNG"

    os.remove("/tmp/pngexp_src.tif")
    os.remove("/tmp/pngexp_out.png")
    print("  PASS")


def test_edge_tiles():
    """Convert a non-multiple-sized TIFF to PNG."""
    print("test_edge_tiles: 40x40, tileSize=16...")
    pixels = {0: [(2, 2)], 8: [(1, 1)]}
    make_tiled_tiff("/tmp/pngexp_edge_src.tif", 40, 40, 16, 3, 3, pixels)

    r = subprocess.run([PNG_EXPORT, "/tmp/pngexp_edge_src.tif", "/tmp/pngexp_edge_out.png"],
                       capture_output=True, text=True, timeout=30)
    assert r.returncode == 0

    pw, ph, _ = read_png("/tmp/pngexp_edge_out.png")
    assert pw == 40 and ph == 40, f"dimensions wrong: {pw}x{ph}"

    os.remove("/tmp/pngexp_edge_src.tif")
    os.remove("/tmp/pngexp_edge_out.png")
    print("  PASS")


def test_is_bilevel():
    """Verify output PNG is 1-bit."""
    print("test_is_bilevel...")
    pixels = {0: [(1, 1)]}
    make_tiled_tiff("/tmp/pngexp_bl_src.tif", 32, 32, 16, 2, 2, pixels)

    subprocess.run([PNG_EXPORT, "/tmp/pngexp_bl_src.tif", "/tmp/pngexp_bl_out.png"],
                   capture_output=True, text=True, timeout=30)

    info = subprocess.run(["file", "/tmp/pngexp_bl_out.png"],
                          capture_output=True, text=True)
    assert "1-bit" in info.stdout, f"not 1-bit: {info.stdout}"

    os.remove("/tmp/pngexp_bl_src.tif")
    os.remove("/tmp/pngexp_bl_out.png")
    print("  PASS")


def test_missing_input():
    """Conversion fails cleanly on missing input."""
    print("test_missing_input...")
    r = subprocess.run([PNG_EXPORT, "/tmp/nonexistent.tif", "/tmp/pngexp_err.png"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode != 0
    print("  PASS")


if __name__ == "__main__":
    test_basic()
    test_edge_tiles()
    test_is_bilevel()
    test_missing_input()
    print("\nAll png_export tests passed.")
