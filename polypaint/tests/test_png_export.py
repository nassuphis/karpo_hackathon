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


def make_bilevel_tiff(path, pix, pixels):
    bits_path = path + ".bits"
    with open(bits_path, "wb") as f:
        f.write(make_bitset(pix, pix, pixels))
    subprocess.run([BILEVEL_MERGE, "assemble", f"--pix={pix}", f"--output={path}", bits_path], capture_output=True)
    os.remove(bits_path)


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
    pixels = [(1, 1), (19, 2), (0, 16), (18, 19)]
    make_bilevel_tiff("/tmp/pngexp_src.tif", 32, pixels)

    r = subprocess.run([PNG_EXPORT, "/tmp/pngexp_src.tif", "/tmp/pngexp_out.png"],
                       capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"failed: {r.stderr}"

    pw, ph, parr = read_png("/tmp/pngexp_out.png")
    assert pw == 32 and ph == 32, f"dimensions wrong: {pw}x{ph}"

    # Verify markers
    assert parr[1, 1] > 0, "marker 0 missing"
    assert parr[2, 19] > 0, "marker 1 missing"
    assert parr[16, 0] > 0, "marker 2 missing"
    assert parr[19, 18] > 0, "marker 3 missing"

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


def test_non_power_of_two_size():
    """Convert a non-power-of-two TIFF to PNG."""
    print("test_non_power_of_two_size: 40x40...")
    pixels = [(2, 2), (33, 33)]
    make_bilevel_tiff("/tmp/pngexp_edge_src.tif", 40, pixels)

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
    make_bilevel_tiff("/tmp/pngexp_bl_src.tif", 32, [(1, 1)])

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
    test_non_power_of_two_size()
    test_is_bilevel()
    test_missing_input()
    print("\nAll png_export tests passed.")
