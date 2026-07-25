"""splat_bake C tool: oracle pins on the SAME drift fixture the viewer's
splat e2e uses — one ground truth for both implementations (the JS worker
renders it live; this tool bakes it server-side). Compiles the real C
locally, runs it on hand-computed fixtures, decodes the 22-byte/splat pack
with the documented dequant formulas, and pins counts, elongation, floors,
weights, colors, slices collapse, both z axes, and both roots formats."""
import json
import math
import os
import shutil
import struct
import subprocess
import tempfile
import unittest

GRID = 4
DEGREE = 2
VIEW = (-1.0, 1.0, -1.0, 1.0)   # min_re, max_re, min_im, max_im


def _js_round(x):
    return math.floor(x + 0.5)


def _u16_of(re, lo, hi):
    return int(_js_round((re - lo) / (hi - lo) * 65534.0))


def _drift_res(row):
    """The drift fixture: per solve, one root near x=-0.5 and one near
    x=+0.5, both drifting 0.02/row; im = 0."""
    return (-0.5 + 0.02 * row, 0.5 + 0.02 * row)


def _fixture_u16():
    out = bytearray()
    for step in range(GRID * GRID):
        row = step // GRID
        for re in _drift_res(row):
            out += struct.pack("<HH", _u16_of(re, VIEW[0], VIEW[1]),
                               _u16_of(0.0, VIEW[2], VIEW[3]))
    return bytes(out)


def _fixture_f32(poison_step=None):
    out = bytearray()
    for step in range(GRID * GRID):
        row = step // GRID
        roots = _drift_res(row)
        for idx, re in enumerate(roots):
            if poison_step is not None and step == poison_step and idx == 0:
                out += struct.pack("<ff", float("nan"), 0.0)
            else:
                out += struct.pack("<ff", re, 0.0)
    return bytes(out)


def _fixture_colors():
    # constant rgb(10,20,30) per solve, row-major (row, col)
    return bytes([10, 20, 30] * (GRID * GRID))


class SplatPack:
    def __init__(self, blob, meta):
        n = meta["count"]
        self.n = n
        o = 0
        self.centers = []
        for i in range(n):
            qs = struct.unpack_from("<3H", blob, o)
            o += 6
            c = []
            for k in range(3):
                span = meta["cmax"][k] - meta["cmin"][k]
                g = span if span != 0 else 1.0
                c.append(meta["cmin"][k] + qs[k] / 65535.0 * g)
            self.centers.append(c)
        self.axisA = []
        for i in range(n):
            qs = struct.unpack_from("<3h", blob, o)
            o += 6
            self.axisA.append([q / 32767.0 * meta["amax"] for q in qs])
        self.axisB = []
        for i in range(n):
            qs = struct.unpack_from("<3h", blob, o)
            o += 6
            self.axisB.append([q / 32767.0 * meta["amax"] for q in qs])
        self.colors = []
        for i in range(n):
            self.colors.append(list(struct.unpack_from("<3B", blob, o)))
            o += 3
        self.weights = list(struct.unpack_from(f"<{n}B", blob, o))
        o += n
        assert o == len(blob) == 22 * n


class TestSplatBakeTool(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cc = shutil.which("cc") or shutil.which("clang") or shutil.which("gcc")
        if not cc:
            raise unittest.SkipTest("no C compiler available")
        cls.tmp = tempfile.mkdtemp(prefix="splat_bake_test_")
        cls.binary = os.path.join(cls.tmp, "splat_bake")
        src = os.path.join(os.path.dirname(__file__), "..", "lambda", "splat_bake.c")
        result = subprocess.run([cc, "-O2", "-o", cls.binary, src, "-lm"],
                                capture_output=True, text=True)
        if result.returncode != 0:
            raise RuntimeError(f"splat_bake compile failed: {result.stderr}")

    def _run(self, roots, colors, fmt="u16", res=64, zaxis="t2", slices=0,
             yscale=1.0, scalemul=1.0, expect_rc=0):
        rp = os.path.join(self.tmp, "roots.bin")
        cp = os.path.join(self.tmp, "colors.raw")
        op = os.path.join(self.tmp, "pack.bin")
        with open(rp, "wb") as fh:
            fh.write(roots)
        with open(cp, "wb") as fh:
            fh.write(colors)
        proc = subprocess.run([
            self.binary, f"--roots={rp}", f"--colors={cp}", f"--out={op}",
            f"--roots_format={fmt}", f"--grid_n={GRID}", f"--degree={DEGREE}",
            f"--min_re={VIEW[0]}", f"--max_re={VIEW[1]}",
            f"--min_im={VIEW[2]}", f"--max_im={VIEW[3]}",
            f"--res={res}", f"--zaxis={zaxis}", f"--slices={slices}",
            f"--yscale={yscale}", f"--scalemul={scalemul}",
        ], capture_output=True, text=True)
        self.assertEqual(proc.returncode, expect_rc, proc.stderr)
        if expect_rc != 0:
            return None, None
        meta = json.loads(proc.stdout)
        with open(op, "rb") as fh:
            blob = fh.read()
        return SplatPack(blob, meta), meta

    def test_drift_fixture_u16_matches_the_viewer_oracle(self):
        # 2 clusters x 2 voxels x 4 y-levels = 16 splats of 2 points each,
        # elongated along x — the exact pins the viewer's splat e2e asserts
        pack, meta = self._run(_fixture_u16(), _fixture_colors())
        self.assertEqual(pack.n, 16)
        self.assertEqual(meta["points_used"], 32)
        self.assertEqual(meta["points_clipped"], 0)
        self.assertEqual(pack.weights, [255] * 16)      # uniform occupancy
        self.assertEqual(pack.colors, [[10, 20, 30]] * 16)
        min_s = 0.35 / 64
        ys = sorted(round(c[1], 3) for c in pack.centers)
        self.assertEqual(ys, sorted([-0.5, -0.25, 0.0, 0.25] * 4))
        for i in range(16):
            ax, ay, az = (abs(v) for v in pack.axisA[i])
            self.assertGreater(ax, ay)                   # x-dominant major axis
            self.assertGreater(ax, az)
            la = math.sqrt(sum(v * v for v in pack.axisA[i]))
            lb = math.sqrt(sum(v * v for v in pack.axisB[i]))
            self.assertAlmostEqual(la, 0.01, delta=0.001)   # 2*sigma of the pair
            self.assertAlmostEqual(lb, min_s, delta=0.0005)  # floored minor axis
            self.assertAlmostEqual(pack.centers[i][2], 0.0, delta=1e-3)  # im=0
            self.assertGreaterEqual(la, lb)

    def test_slices_collapse_onto_plates(self):
        # slices=2: 4 y-levels merge pairwise -> 8 splats of 4, on +-0.5
        pack, meta = self._run(_fixture_u16(), _fixture_colors(), slices=2)
        self.assertEqual(pack.n, 8)
        self.assertEqual(meta["points_used"], 32)
        self.assertEqual(pack.weights, [255] * 8)
        ys = sorted(set(round(c[1], 3) for c in pack.centers))
        self.assertEqual(ys, [-0.5, 0.5])

    def test_yscale_folds_into_centers(self):
        pack, _ = self._run(_fixture_u16(), _fixture_colors(), yscale=0.5)
        ys = sorted(round(c[1], 3) for c in pack.centers)
        self.assertEqual(ys, sorted([-0.25, -0.125, 0.0, 0.125] * 4))

    def test_scalemul_folds_into_axes(self):
        base, _ = self._run(_fixture_u16(), _fixture_colors())
        doubled, _ = self._run(_fixture_u16(), _fixture_colors(), scalemul=2.0)
        la0 = math.sqrt(sum(v * v for v in base.axisA[0]))
        la1 = math.sqrt(sum(v * v for v in doubled.axisA[0]))
        self.assertAlmostEqual(la1, la0 * 2, delta=0.001)

    def test_t1_axis_collapses_rows_to_points(self):
        # z = t1: x and y both derive from the row, so each (row, cluster)
        # is 4 coincident points -> 8 zero-covariance voxels, both axes at
        # the floor
        pack, _ = self._run(_fixture_u16(), _fixture_colors(), zaxis="t1")
        self.assertEqual(pack.n, 8)
        self.assertEqual(pack.weights, [255] * 8)
        min_s = 0.35 / 64
        for i in range(8):
            la = math.sqrt(sum(v * v for v in pack.axisA[i]))
            lb = math.sqrt(sum(v * v for v in pack.axisB[i]))
            self.assertAlmostEqual(la, min_s, delta=0.0005)
            self.assertAlmostEqual(lb, min_s, delta=0.0005)

    def test_f32_format_and_clipping(self):
        pack, meta = self._run(_fixture_f32(), _fixture_colors(), fmt="f32")
        self.assertEqual(pack.n, 16)
        self.assertEqual(meta["points_clipped"], 0)
        # one poisoned NaN root drops exactly one point
        pack2, meta2 = self._run(_fixture_f32(poison_step=0), _fixture_colors(), fmt="f32")
        self.assertEqual(meta2["points_used"], 31)
        self.assertEqual(meta2["points_clipped"], 1)
        self.assertEqual(pack2.n, 16)                   # its voxel keeps 1 point
        self.assertIn(_js_round(math.sqrt(1 / 2) * 255), pack2.weights)

    def test_u16_sentinel_pairs_are_clipped(self):
        blob = bytearray(_fixture_u16())
        struct.pack_into("<HH", blob, 0, 0xFFFF, 0xFFFF)   # first root of step 0
        pack, meta = self._run(bytes(blob), _fixture_colors())
        self.assertEqual(meta["points_used"], 31)
        self.assertEqual(meta["points_clipped"], 1)
        self.assertEqual(pack.n, 16)

    def test_rejects_bad_inputs(self):
        self._run(b"", _fixture_colors(), expect_rc=1)                       # short roots
        self._run(_fixture_u16(), b"\x00" * 5, expect_rc=1)                  # short colors
        rp = os.path.join(self.tmp, "roots.bin")
        with open(rp, "wb") as fh:
            fh.write(_fixture_u16())
        proc = subprocess.run([self.binary, f"--roots={rp}"], capture_output=True, text=True)
        self.assertEqual(proc.returncode, 2)                                 # usage
        self.assertIn("usage:", proc.stderr)


if __name__ == "__main__":
    unittest.main()
