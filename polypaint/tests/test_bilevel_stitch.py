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
           f"--pix={tw*2}", f"--tile_size={tw}",
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


def test_stitch_3x3_square_grid():
    """Test stitch with 3 cols x 3 rows, verify square tile placement."""
    print("test_stitch_3x3_square_grid: 9 tiles (4x4 each) into 12x12, checking placement...")
    tw, th = 4, 4

    # Each tile gets a unique single pixel at a distinct position
    tile_pixels = [(0,0), (1,1), (2,2), (3,3), (0,3), (3,0), (1,3), (3,1), (2,0)]
    for t, (px, py) in enumerate(tile_pixels):
        create_tile_tiff(f"/tmp/test_tile32_{t}.tif", tw, th,
                         lambda x, y, px=px, py=py: x == px and y == py)

    paths = [f"/tmp/test_tile32_{t}.tif" for t in range(9)]
    out_path = "/tmp/test_stitch_3x3.tif"
    cmd = [BILEVEL_MERGE, "stitch", "--n_cols=3", "--n_rows=3",
           f"--pix={tw*3}", f"--tile_size={tw}",
           f"--output={out_path}"] + paths
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"stitch failed: {r.stderr}"

    ow, oh, arr = read_tiff_pixels(out_path)
    assert ow == 12 and oh == 12, f"dimensions wrong: {ow}x{oh} vs 12x12"

    # Verify each tile's pixel lands in the correct global position
    # Grid layout: 3 cols x 3 rows, each tile 4x4
    # Tile t at (row, col) → global offset (col*4, row*4)
    expected_globals = set()
    for t, (px, py) in enumerate(tile_pixels):
        row = t // 3
        col = t % 3
        gx = col * tw + px
        gy = row * th + py
        assert arr[gy, gx] > 0, f"tile {t} pixel ({px},{py}) should be white at global ({gx},{gy})"
        expected_globals.add((gx, gy))

    # Verify ONLY the expected pixels are set (no extra content)
    total_white = int(np.sum(arr > 0))
    assert total_white == len(expected_globals), \
        f"expected {len(expected_globals)} white pixels, got {total_white} — extra content present"

    for t in range(9):
        os.remove(f"/tmp/test_tile32_{t}.tif")
    os.remove(out_path)
    print("  PASS")


def test_stitch_missing_tile():
    """Test stitch fails cleanly on missing tile."""
    print("test_stitch_missing_tile: expect failure...")
    cmd = [BILEVEL_MERGE, "stitch", "--n_cols=2", "--n_rows=2",
           "--pix=16", "--tile_size=8",
           "--output=/tmp/test_missing.tif",
           "/tmp/nonexistent_0.tif", "/tmp/nonexistent_1.tif",
           "/tmp/nonexistent_2.tif", "/tmp/nonexistent_3.tif"]
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    assert r.returncode != 0, "stitch should fail on missing tiles"
    # BigTIFF file may be created before tile load fails; clean up
    if os.path.exists("/tmp/test_missing.tif"):
        os.remove("/tmp/test_missing.tif")
    print("  PASS")


def test_stitch_edge_tiles():
    """Test stitch with non-multiple image size: exact dimensions, no padding."""
    print("test_stitch_edge_tiles: 40x40 image, tileSize=16, 3x3 grid (edge tiles 8px)...")
    tw_full, th_full = 16, 16
    tileSz = 16
    fullW, fullH = 40, 40
    nCols, nRows = 3, 3

    for t in range(9):
        tc, tr = t % nCols, t // nCols
        tw = 8 if tc == 2 else tw_full
        th = 8 if tr == 2 else th_full
        # Unique marker per tile
        mx = min(t % 5, tw - 1)
        my = min(t // 3, th - 1)
        create_tile_tiff(f"/tmp/test_edge_{t}.tif", tw, th,
                         lambda x, y, px=mx, py=my: x == px and y == py)

    paths = [f"/tmp/test_edge_{t}.tif" for t in range(9)]
    out_path = "/tmp/test_edge_stitch.tif"
    cmd = [BILEVEL_MERGE, "stitch", "--n_cols=3", "--n_rows=3",
           f"--pix={fullW}", f"--tile_size={tileSz}",
           f"--output={out_path}"] + paths
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"stitch failed: {r.stderr}"

    ow, oh, arr = read_tiff_pixels(out_path)
    assert ow == fullW and oh == fullH, f"dimensions wrong: {ow}x{oh} vs {fullW}x{fullH}"

    # Verify markers at correct global positions
    for t in range(9):
        tc, tr = t % nCols, t // nCols
        tw = 8 if tc == 2 else tw_full
        th = 8 if tr == 2 else th_full
        mx = min(t % 5, tw - 1)
        my = min(t // 3, th - 1)
        gx = tc * tileSz + mx
        gy = tr * tileSz + my
        assert arr[gy, gx] > 0, f"tile {t}: marker missing at global ({gx},{gy})"

    for t in range(9):
        os.remove(f"/tmp/test_edge_{t}.tif")
    os.remove(out_path)
    print(f"  output exactly {fullW}x{fullH}, all markers placed correctly")
    print("  PASS")


def test_stitch_square_remainder():
    """Test stitch with square remainder tiles: 40x40, tileSize=16."""
    print("test_stitch_square_remainder: 40x40, tileSize=16, 3x3 grid...")
    tileSz = 16
    fullW, fullH = 40, 40
    nCols = 3  # widths: 16, 16, 8
    nRows = 3  # heights: 16, 16, 8

    markers = [(2,2), (3,3), (1,1), (0,0), (4,4), (2,0), (1,6), (7,1), (0,0)]
    for t in range(9):
        tc, tr = t % nCols, t // nCols
        tw = 8 if tc == 2 else 16
        th = 8 if tr == 2 else 16
        mx = min(markers[t][0], tw - 1)
        my = min(markers[t][1], th - 1)
        create_tile_tiff(f"/tmp/test_nsq_{t}.tif", tw, th,
                         lambda x, y, px=mx, py=my: x == px and y == py)

    paths = [f"/tmp/test_nsq_{t}.tif" for t in range(9)]
    out_path = "/tmp/test_nsq_stitch.tif"
    cmd = [BILEVEL_MERGE, "stitch", f"--n_cols={nCols}", f"--n_rows={nRows}",
           f"--pix={fullW}", f"--tile_size={tileSz}",
           f"--output={out_path}"] + paths
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"stitch failed: {r.stderr}"

    ow, oh, arr = read_tiff_pixels(out_path)
    assert ow == fullW and oh == fullH, f"dimensions wrong: {ow}x{oh} vs {fullW}x{fullH}"

    # Verify markers and total count
    expected = set()
    for t in range(9):
        tc, tr = t % nCols, t // nCols
        tw = 8 if tc == 2 else 16
        th = 8 if tr == 2 else 16
        mx = min(markers[t][0], tw - 1)
        my = min(markers[t][1], th - 1)
        gx = tc * tileSz + mx
        gy = tr * tileSz + my
        assert arr[gy, gx] > 0, f"tile {t}: marker missing at ({gx},{gy})"
        expected.add((gx, gy))

    total_white = int(np.sum(arr > 0))
    assert total_white == len(expected), \
        f"expected {len(expected)} white pixels, got {total_white}"

    for t in range(9):
        os.remove(f"/tmp/test_nsq_{t}.tif")
    os.remove(out_path)
    print(f"  output exactly {fullW}x{fullH}, {len(expected)} markers correct, no extras")
    print("  PASS")


if __name__ == "__main__":
    test_merge_basic()
    test_merge_empty()
    test_stitch_2x2()
    test_stitch_3x3_square_grid()
    test_stitch_missing_tile()
    test_stitch_edge_tiles()
    test_stitch_square_remainder()
    print("\nAll bilevel_merge tests passed.")
