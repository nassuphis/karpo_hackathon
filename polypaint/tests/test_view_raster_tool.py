"""view_raster C tool: hand-computed pixel + score-byte oracles for every
projection. The fixture reuses the drift construction (grid 4, degree 1,
one root per solve at (-0.875 + 0.25*row, -0.875 + 0.25*col), viewport
[-1,1]^2, pix 8) so plan-pipeline intuitions carry over; scores encode the
solve's own (row, col) so pixel OWNERSHIP is pinned, not just occupancy."""
import json
import math
import os
import shutil
import struct
import subprocess
import tempfile
import unittest

GRID = 4
DEGREE = 1
PIX = 8


def _js_round(x):
    return math.floor(x + 0.5)


def _roots_u16():
    out = bytearray()
    for step in range(GRID * GRID):
        row = step // GRID
        j = step % GRID
        col = (GRID - 1 - j) if (row & 1) else j
        re = -0.875 + 0.25 * row
        im = -0.875 + 0.25 * col
        out += struct.pack("<HH",
                           int(_js_round((re + 1.0) / 2.0 * 65534.0)),
                           int(_js_round((im + 1.0) / 2.0 * 65534.0)))
    return bytes(out)


def _scores():
    # row-major (row, col): score byte = 16*row + col + 1 (0 = background)
    out = bytearray(GRID * GRID)
    for row in range(GRID):
        for col in range(GRID):
            out[row * GRID + col] = 16 * row + col + 1
    return bytes(out)


def _isometric_pixel(re, im, t):
    x = (re + 1.0) / 2.0
    y = (im + 1.0) / 2.0
    extent = PIX - 1
    scale = extent / 2.0
    return (
        math.floor(extent / 2.0 + scale * (x - y) * (math.sqrt(3.0) / 2.0)),
        math.floor(extent / 2.0 + scale * ((x + y) / 2.0 - t)),
    )


class TestViewRasterTool(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cc = shutil.which("cc") or shutil.which("clang") or shutil.which("gcc")
        if not cc:
            raise unittest.SkipTest("no C compiler available")
        cls.tmp = tempfile.mkdtemp(prefix="view_raster_test_")
        cls.binary = os.path.join(cls.tmp, "view_raster")
        src = os.path.join(os.path.dirname(__file__), "..", "lambda", "view_raster.c")
        result = subprocess.run([cc, "-O2", "-o", cls.binary, src, "-lm"],
                                capture_output=True, text=True)
        if result.returncode != 0:
            raise RuntimeError(f"view_raster compile failed: {result.stderr}")

    def _run(self, projection, vertical, roots=None, expect_rc=0):
        rp = os.path.join(self.tmp, "roots.bin")
        sp = os.path.join(self.tmp, "scores.raw")
        op = os.path.join(self.tmp, "view.raw")
        with open(rp, "wb") as fh:
            fh.write(roots if roots is not None else _roots_u16())
        with open(sp, "wb") as fh:
            fh.write(_scores())
        proc = subprocess.run([
            self.binary, f"--roots={rp}", f"--scores={sp}", f"--out={op}",
            "--roots_format=u16", f"--projection={projection}", f"--vertical={vertical}",
            f"--grid_n={GRID}", f"--degree={DEGREE}", f"--pix={PIX}",
            "--min_re=-1", "--max_re=1", "--min_im=-1", "--max_im=1",
        ], capture_output=True, text=True)
        self.assertEqual(proc.returncode, expect_rc, proc.stderr)
        if expect_rc != 0:
            return None, None
        meta = json.loads(proc.stdout)
        with open(op, "rb") as fh:
            img = fh.read()
        self.assertEqual(len(img), PIX * PIX)
        cells = {}
        for idx, b in enumerate(img):
            if b:
                cells[(idx % PIX, idx // PIX)] = b
        return cells, meta

    def test_front_t2_pixels_and_ownership(self):
        # px = row (re -> {0..3}), py from t2=col/4 -> {7,6,4,2}; the score
        # byte at each pixel is exactly its solve's encoded (row, col)
        cells, meta = self._run("front", "t2")
        expected = {}
        py_of = {0: 7, 1: 6, 2: 4, 3: 2}
        for row in range(4):
            for col in range(4):
                expected[(row, py_of[col])] = 16 * row + col + 1
        self.assertEqual(cells, expected)
        self.assertEqual(meta["plotted"], 16)
        self.assertEqual(meta["deduped"], 0)

    def test_rear_mirrors_the_horizontal(self):
        cells, _ = self._run("rear", "t2")
        py_of = {0: 7, 1: 6, 2: 4, 3: 2}
        expected = {(7 - row, py_of[col]): 16 * row + col + 1
                    for row in range(4) for col in range(4)}
        self.assertEqual(cells, expected)

    def test_front_t1_first_claim_ownership(self):
        # row drives BOTH axes -> 4 pixels; each is claimed by the FIRST
        # solve of its row in step order (serpentine): row 0 first col 0,
        # row 1 (odd, reversed) first col 3, row 2 col 0, row 3 col 3
        cells, meta = self._run("front", "t1")
        self.assertEqual(cells, {
            (0, 7): 16 * 0 + 0 + 1,
            (1, 6): 16 * 1 + 3 + 1,
            (2, 4): 16 * 2 + 0 + 1,
            (3, 2): 16 * 3 + 3 + 1,
        })
        self.assertEqual(meta["deduped"], 12)   # 3 later roots per pixel

    def test_left_and_right_use_im(self):
        cells, _ = self._run("right", "t1")
        # px from im (= col), py from t1 (= row) -> full 4x4, exact owners
        py_of = {0: 7, 1: 6, 2: 4, 3: 2}
        expected = {(col, py_of[row]): 16 * row + col + 1
                    for row in range(4) for col in range(4)}
        self.assertEqual(cells, expected)
        cells, _ = self._run("left", "t1")
        expected = {(7 - col, py_of[row]): 16 * row + col + 1
                    for row in range(4) for col in range(4)}
        self.assertEqual(cells, expected)

    def test_radial_collapses_angles(self):
        # r = hypot(re, im), rmax = sqrt(2) (viewport corner), px =
        # floor(r/rmax * pix); first claim in step order wins. The default
        # grid lands every radius EXACTLY on a pixel boundary, where C libm
        # and Python hypot legitimately round to different cells — so this
        # test uses its own boundary-free layout (radii at x.2 cells).
        roots = bytearray()
        for step in range(GRID * GRID):
            row = step // GRID
            j = step % GRID
            col = (GRID - 1 - j) if (row & 1) else j
            re = -0.9 + 0.25 * row
            im = -0.9 + 0.25 * col
            roots += struct.pack("<HH",
                                 int(_js_round((re + 1.0) / 2.0 * 65534.0)),
                                 int(_js_round((im + 1.0) / 2.0 * 65534.0)))
        cells, meta = self._run("radial", "t2", roots=bytes(roots))
        rmax = math.sqrt(2.0)
        self.assertAlmostEqual(meta["rmax"], rmax, places=12)
        expected = {}
        py_of = {0: 7, 1: 6, 2: 4, 3: 2}
        for step in range(GRID * GRID):
            row = step // GRID
            j = step % GRID
            col = (GRID - 1 - j) if (row & 1) else j
            # dequantized coordinates (the u16 round trip), as the C sees
            re = -1.0 + _js_round((-0.9 + 0.25 * row + 1.0) / 2.0 * 65534.0) / 65534.0 * 2.0
            im = -1.0 + _js_round((-0.9 + 0.25 * col + 1.0) / 2.0 * 65534.0) / 65534.0 * 2.0
            px = math.floor(math.hypot(re, im) / rmax * PIX)
            key = (px, py_of[col])
            if key not in expected:            # first claim in step order
                expected[key] = 16 * row + col + 1
        self.assertEqual(cells, expected)
        self.assertEqual(meta["plotted"], len(expected))

    def test_isometric_t1_and_t2_pixels_and_ownership(self):
        # The u16 dump is dequantized before projection. Pin both geometry
        # and first-claim score ownership against those exact coordinates.
        for vertical in ("t1", "t2"):
            cells, meta = self._run("isometric", vertical)
            expected = {}
            for step in range(GRID * GRID):
                row = step // GRID
                j = step % GRID
                col = (GRID - 1 - j) if (row & 1) else j
                re_q = _js_round((-0.875 + 0.25 * row + 1.0) / 2.0 * 65534.0)
                im_q = _js_round((-0.875 + 0.25 * col + 1.0) / 2.0 * 65534.0)
                re = -1.0 + re_q / 65534.0 * 2.0
                im = -1.0 + im_q / 65534.0 * 2.0
                t = (row if vertical == "t1" else col) / GRID
                key = _isometric_pixel(re, im, t)
                if key not in expected:
                    expected[key] = 16 * row + col + 1
            self.assertEqual(cells, expected)
            self.assertEqual(meta["plotted"], len(expected))
            self.assertEqual(meta["deduped"], GRID * GRID - len(expected))

    def test_sentinel_and_rejects(self):
        roots = bytearray(_roots_u16())
        struct.pack_into("<HH", roots, 0, 0xFFFF, 0xFFFF)
        cells, meta = self._run("front", "t2", roots=bytes(roots))
        self.assertEqual(meta["clipped"], 1)
        self.assertEqual(len(cells), 15)
        self._run("top", "t2", expect_rc=2)      # bad projection
        proc = subprocess.run([self.binary], capture_output=True, text=True)
        self.assertEqual(proc.returncode, 2)
        self.assertIn("usage:", proc.stderr)


if __name__ == "__main__":
    unittest.main()
