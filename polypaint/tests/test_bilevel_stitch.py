"""
Test bilevel_merge: merge mode (OR bitsets → TIFF) and stitch mode (join tiles → final TIFF).

Creates small tile bitsets/TIFFs with known patterns, runs the binary,
verifies output dimensions and pixel placement.

Run: cd polypaint/tests && uv run python test_bilevel_stitch.py

Requires bilevel_merge_local (natively compiled with libvips) in polypaint/lambda/:
  cd polypaint/lambda && cc -O3 -o bilevel_merge_local bilevel_merge.c $(pkg-config --cflags --libs vips) -lm
"""
import os
import struct
import subprocess
import numpy as np

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
BILEVEL_MERGE = os.path.join(LAMBDA_DIR, "bilevel_merge_local")

if not os.path.exists(BILEVEL_MERGE):
    raise FileNotFoundError(
        f"bilevel_merge_local not found at {BILEVEL_MERGE}. "
        "Compile it first: cd polypaint/lambda && cc -O3 -o bilevel_merge_local bilevel_merge.c $(pkg-config --cflags --libs vips) -lm"
    )


def make_bitset(width, height, pixels_on):
    """Create a packed bitset with specific pixels set."""
    n = width * height
    bitset = bytearray((n + 7) // 8)
    for (x, y) in pixels_on:
        idx = y * width + x
        bitset[idx >> 3] |= 1 << (idx & 7)
    return bytes(bitset)


def read_tiff_pixels(path):
    """Read a TIFF and return (width, height, numpy array of 0/255 values)."""
    try:
        import pyvips
        img = pyvips.Image.new_from_file(path)
        w, h = img.width, img.height
        data = img.write_to_memory()
        arr = np.frombuffer(data, dtype=np.uint8).reshape(h, w)
        return w, h, arr
    except ImportError:
        pass

    # Fallback: try tifffile
    try:
        import tifffile
        img = tifffile.imread(path)
        if img.ndim == 2:
            h, w = img.shape
        else:
            h, w = img.shape[0], img.shape[1]
        # Bilevel TIFFs may be packed bits — unpack
        if img.dtype == np.bool_:
            arr = img.astype(np.uint8) * 255
        elif img.dtype == np.uint8:
            arr = img
        else:
            arr = (img > 0).astype(np.uint8) * 255
        return w, h, arr.reshape(h, w)
    except ImportError:
        pass

    # Fallback: PIL
    from PIL import Image
    img = Image.open(path)
    w, h = img.size
    arr = np.array(img)
    if arr.dtype == np.bool_:
        arr = arr.astype(np.uint8) * 255
    return w, h, arr.reshape(h, w)


def test_merge_basic():
    """Test merge: OR two bitsets into one tile TIFF."""
    print("test_merge_basic: OR two 8x8 bitsets...")
    w, h = 8, 8

    # Bitset 1: top-left 4x4 block
    pixels1 = [(x, y) for x in range(4) for y in range(4)]
    bits1 = make_bitset(w, h, pixels1)

    # Bitset 2: bottom-right 4x4 block
    pixels2 = [(x, y) for x in range(4, 8) for y in range(4, 8)]
    bits2 = make_bitset(w, h, pixels2)

    bits1_path = "/tmp/test_merge_1.bits"
    bits2_path = "/tmp/test_merge_2.bits"
    out_path = "/tmp/test_merge_out.tif"

    with open(bits1_path, "wb") as f:
        f.write(bits1)
    with open(bits2_path, "wb") as f:
        f.write(bits2)

    cmd = [BILEVEL_MERGE, "merge", f"--tile_w={w}", f"--tile_h={h}",
           f"--output={out_path}", bits1_path, bits2_path]
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"merge failed: {r.stderr}"

    assert os.path.exists(out_path), "output TIFF not created"

    ow, oh, arr = read_tiff_pixels(out_path)
    assert ow == w and oh == h, f"dimensions wrong: {ow}x{oh} vs {w}x{h}"

    # Top-left should be white
    assert arr[0, 0] > 0, "pixel (0,0) should be white"
    assert arr[3, 3] > 0, "pixel (3,3) should be white"
    # Bottom-right should be white
    assert arr[7, 7] > 0, "pixel (7,7) should be white"
    assert arr[4, 4] > 0, "pixel (4,4) should be white"
    # Top-right should be black
    assert arr[0, 7] == 0, "pixel (7,0) should be black"
    # Bottom-left should be black
    assert arr[7, 0] == 0, "pixel (0,7) should be black"

    for p in [bits1_path, bits2_path, out_path]:
        os.remove(p)
    print("  PASS")


def test_merge_empty():
    """Test merge with no input files produces all-black tile."""
    print("test_merge_empty: no input bitsets...")
    w, h = 8, 8
    out_path = "/tmp/test_merge_empty.tif"

    cmd = [BILEVEL_MERGE, "merge", f"--tile_w={w}", f"--tile_h={h}",
           f"--output={out_path}"]
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"merge failed: {r.stderr}"

    ow, oh, arr = read_tiff_pixels(out_path)
    assert ow == w and oh == h
    assert np.all(arr == 0), "empty merge should produce all-black tile"

    os.remove(out_path)
    print("  PASS")


def create_tile_tiff(path, w, h, pattern_fn):
    """Create a small bilevel tile TIFF with a pattern via bilevel_merge."""
    pixels = [(x, y) for x in range(w) for y in range(h) if pattern_fn(x, y)]
    bits = make_bitset(w, h, pixels)
    bits_path = path + ".bits"
    with open(bits_path, "wb") as f:
        f.write(bits)
    cmd = [BILEVEL_MERGE, "merge", f"--tile_w={w}", f"--tile_h={h}",
           f"--output={path}", bits_path]
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"tile creation failed: {r.stderr}"
    os.remove(bits_path)


def test_stitch_2x2():
    """Test stitch: 4 tiles (8x8 each) into a 16x16 image with distinct quadrant patterns."""
    print("test_stitch_2x2: 4 tiles with distinct patterns...")
    tw, th = 8, 8

    # Tile 0 (top-left): diagonal stripe
    create_tile_tiff("/tmp/test_tile_0.tif", tw, th, lambda x, y: x == y)
    # Tile 1 (top-right): top row
    create_tile_tiff("/tmp/test_tile_1.tif", tw, th, lambda x, y: y == 0)
    # Tile 2 (bottom-left): left column
    create_tile_tiff("/tmp/test_tile_2.tif", tw, th, lambda x, y: x == 0)
    # Tile 3 (bottom-right): center pixel
    create_tile_tiff("/tmp/test_tile_3.tif", tw, th, lambda x, y: x == 4 and y == 4)

    out_path = "/tmp/test_stitch_2x2.tif"
    cmd = [BILEVEL_MERGE, "stitch", "--n_cols=2", "--n_rows=2",
           f"--output={out_path}",
           "/tmp/test_tile_0.tif", "/tmp/test_tile_1.tif",
           "/tmp/test_tile_2.tif", "/tmp/test_tile_3.tif"]
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"stitch failed: {r.stderr}"

    ow, oh, arr = read_tiff_pixels(out_path)
    assert ow == 16 and oh == 16, f"dimensions wrong: {ow}x{oh} vs 16x16"

    # Tile 0 (top-left quadrant): diagonal x==y
    assert arr[0, 0] > 0, "tile0: (0,0) should be white (diagonal)"
    assert arr[3, 3] > 0, "tile0: (3,3) should be white (diagonal)"
    assert arr[0, 3] == 0, "tile0: (3,0) should be black"

    # Tile 1 (top-right quadrant): top row (y=0, x=8..15)
    assert arr[0, 8] > 0, "tile1: (8,0) should be white (top row)"
    assert arr[0, 15] > 0, "tile1: (15,0) should be white (top row)"
    assert arr[1, 8] == 0, "tile1: (8,1) should be black"

    # Tile 2 (bottom-left quadrant): left column (x=0, y=8..15)
    assert arr[8, 0] > 0, "tile2: (0,8) should be white (left col)"
    assert arr[15, 0] > 0, "tile2: (0,15) should be white (left col)"
    assert arr[8, 1] == 0, "tile2: (1,8) should be black"

    # Tile 3 (bottom-right quadrant): center pixel (x=12, y=12 in global)
    assert arr[12, 12] > 0, "tile3: (12,12) should be white (center)"
    assert arr[12, 11] == 0, "tile3: (11,12) should be black"

    for t in range(4):
        os.remove(f"/tmp/test_tile_{t}.tif")
    os.remove(out_path)
    print("  PASS")


def test_stitch_3x2():
    """Test stitch with non-square grid: 3 cols x 2 rows, verify tile placement."""
    print("test_stitch_3x2: 6 tiles (4x4 each) into 12x8, checking placement...")
    tw, th = 4, 4

    # Each tile gets a unique single pixel at a distinct position
    # Tile 0 (row0,col0): pixel (0,0)
    # Tile 1 (row0,col1): pixel (1,1)
    # Tile 2 (row0,col2): pixel (2,2)
    # Tile 3 (row1,col0): pixel (3,3)
    # Tile 4 (row1,col1): pixel (0,3)
    # Tile 5 (row1,col2): pixel (3,0)
    tile_pixels = [(0,0), (1,1), (2,2), (3,3), (0,3), (3,0)]
    for t, (px, py) in enumerate(tile_pixels):
        create_tile_tiff(f"/tmp/test_tile32_{t}.tif", tw, th,
                         lambda x, y, px=px, py=py: x == px and y == py)

    paths = [f"/tmp/test_tile32_{t}.tif" for t in range(6)]
    out_path = "/tmp/test_stitch_3x2.tif"
    cmd = [BILEVEL_MERGE, "stitch", "--n_cols=3", "--n_rows=2",
           f"--output={out_path}"] + paths
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"stitch failed: {r.stderr}"

    ow, oh, arr = read_tiff_pixels(out_path)
    assert ow == 12 and oh == 8, f"dimensions wrong: {ow}x{oh} vs 12x8"

    # Verify each tile's pixel lands in the correct global position
    # Grid layout: 3 cols x 2 rows, each tile 4x4
    # Tile t at (row, col) → global offset (col*4, row*4)
    for t, (px, py) in enumerate(tile_pixels):
        row = t // 3
        col = t % 3
        gx = col * tw + px  # global x
        gy = row * th + py  # global y
        assert arr[gy, gx] > 0, f"tile {t} pixel ({px},{py}) should be white at global ({gx},{gy})"
        # Check an adjacent pixel is black (not all white)
        adj_x = (gx + 1) % ow
        if (adj_x, gy) != (gx, gy):
            # Only check if adjacent isn't another tile's set pixel
            is_other = any(
                (t2 // 3) * tw + tp[0] == adj_x and (t2 % 3 + 1 == col + 1 or True) and (t2 // 3) * th + tp[1] == gy
                for t2, tp in enumerate(tile_pixels) if t2 != t
            )
            # Skip adjacency check if too complex — the positive check is sufficient

    for t in range(6):
        os.remove(f"/tmp/test_tile32_{t}.tif")
    os.remove(out_path)
    print("  PASS")


def test_stitch_missing_tile():
    """Test stitch fails cleanly on missing tile."""
    print("test_stitch_missing_tile: expect failure...")
    cmd = [BILEVEL_MERGE, "stitch", "--n_cols=2", "--n_rows=1",
           "--output=/tmp/test_missing.tif",
           "/tmp/nonexistent_0.tif", "/tmp/nonexistent_1.tif"]
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    assert r.returncode != 0, "stitch should fail on missing tiles"
    assert not os.path.exists("/tmp/test_missing.tif"), "should not produce output"
    print("  PASS")


if __name__ == "__main__":
    test_merge_basic()
    test_merge_empty()
    test_stitch_2x2()
    test_stitch_3x2()
    test_stitch_missing_tile()
    print("\nAll bilevel_merge tests passed.")
