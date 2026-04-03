import os
import pathlib
import shlex
import shutil
import struct
import subprocess
import tempfile
import unittest
import zlib


ROOT = pathlib.Path(__file__).resolve().parent.parent
SRC = ROOT / "lambda" / "autolevels_render.c"
BIN = pathlib.Path("/tmp/autolevels_render_native_test")


def _try_compile():
    if not shutil.which("cc"):
        return False, "cc not found"

    cmd = ["cc", "-O3"]
    cflags = []
    libs = []
    pkg_cflags = subprocess.run(["pkg-config", "--cflags", "vips"], capture_output=True, text=True)
    pkg_libs = subprocess.run(["pkg-config", "--libs", "vips"], capture_output=True, text=True)
    if pkg_cflags.returncode == 0 and pkg_libs.returncode == 0:
        cflags = shlex.split(pkg_cflags.stdout.strip())
        libs = shlex.split(pkg_libs.stdout.strip())
    else:
        cflags = [
            "-I/opt/homebrew/include",
            "-I/opt/homebrew/include/glib-2.0",
            "-I/opt/homebrew/lib/glib-2.0/include",
        ]
        libs = [
            "-L/opt/homebrew/lib",
            "-lvips",
            "-lgobject-2.0",
            "-lglib-2.0",
        ]
    cmd.extend(cflags)
    cmd.extend([str(SRC), "-lm"])
    cmd.extend(libs)
    cmd.extend(["-o", str(BIN)])
    r = subprocess.run(cmd, capture_output=True, text=True)
    return r.returncode == 0, r.stderr.strip() or r.stdout.strip()


def _write_ppm(path, width, height, pixels):
    with open(path, "wb") as fh:
        fh.write(f"P6\n{width} {height}\n255\n".encode("ascii"))
        for r, g, b in pixels:
            fh.write(bytes((r, g, b)))


def _paeth(a, b, c):
    p = a + b - c
    pa = abs(p - a)
    pb = abs(p - b)
    pc = abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    if pb <= pc:
        return b
    return c


def _decode_png_rgb(path):
    data = pathlib.Path(path).read_bytes()
    assert data[:8] == b"\x89PNG\r\n\x1a\n"
    pos = 8
    width = height = None
    color_type = None
    bit_depth = None
    idat = bytearray()
    while pos < len(data):
        length = struct.unpack(">I", data[pos:pos + 4])[0]
        ctype = data[pos + 4:pos + 8]
        chunk = data[pos + 8:pos + 8 + length]
        pos += 12 + length
        if ctype == b"IHDR":
            width, height, bit_depth, color_type = struct.unpack(">IIBB", chunk[:10])
        elif ctype == b"IDAT":
            idat.extend(chunk)
        elif ctype == b"IEND":
            break
    if bit_depth != 8 or color_type not in (2, 6):
        raise AssertionError(f"unsupported PNG format: bit_depth={bit_depth}, color_type={color_type}")
    bands = 3 if color_type == 2 else 4
    stride = width * bands
    raw = zlib.decompress(bytes(idat))
    rows = []
    i = 0
    prev = bytearray(stride)
    for _ in range(height):
        filter_type = raw[i]
        i += 1
        cur = bytearray(raw[i:i + stride])
        i += stride
        if filter_type == 1:
            for x in range(stride):
                left = cur[x - bands] if x >= bands else 0
                cur[x] = (cur[x] + left) & 0xFF
        elif filter_type == 2:
            for x in range(stride):
                cur[x] = (cur[x] + prev[x]) & 0xFF
        elif filter_type == 3:
            for x in range(stride):
                left = cur[x - bands] if x >= bands else 0
                up = prev[x]
                cur[x] = (cur[x] + ((left + up) // 2)) & 0xFF
        elif filter_type == 4:
            for x in range(stride):
                left = cur[x - bands] if x >= bands else 0
                up = prev[x]
                up_left = prev[x - bands] if x >= bands else 0
                cur[x] = (cur[x] + _paeth(left, up, up_left)) & 0xFF
        elif filter_type != 0:
            raise AssertionError(f"unsupported PNG filter {filter_type}")
        rows.append(bytes(cur))
        prev = cur
    pixels = []
    for row in rows:
        for x in range(width):
            off = x * bands
            pixels.append(tuple(row[off:off + 3]))
    return width, height, pixels


@unittest.skipUnless(SRC.exists(), "autolevels_render.c not found")
class TestAutolevelsRenderNative(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        ok, msg = _try_compile()
        if not ok:
            raise unittest.SkipTest(f"could not compile autolevels_render native test binary: {msg}")

    def _run(self, in_path, out_path, **opts):
        cmd = [str(BIN), str(in_path), str(out_path)]
        for key, value in opts.items():
            cmd.append(f"--{key.replace('_', '-')}={value}")
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
        self.assertEqual(r.returncode, 0, r.stderr or r.stdout)
        return _decode_png_rgb(out_path)

    def test_excluding_background_changes_output(self):
        width = height = 8
        bg = (2, 2, 2)
        pixels = [bg] * (width * height)
        pixels[2 * width + 2] = (80, 140, 220)
        pixels[5 * width + 5] = (160, 200, 240)
        with tempfile.TemporaryDirectory() as td:
            src = os.path.join(td, "src.ppm")
            out_a = os.path.join(td, "a.png")
            out_b = os.path.join(td, "b.png")
            _write_ppm(src, width, height, pixels)
            _, _, px_no = self._run(src, out_a, exclude_background=0, background_color="000000", background_threshold=4)
            _, _, px_yes = self._run(src, out_b, exclude_background=1, background_color="000000", background_threshold=4)
        idx_a = 2 * width + 2
        idx_b = 5 * width + 5
        idx_bg = 0
        self.assertNotEqual(px_no, px_yes)
        self.assertNotEqual(px_no[idx_a], px_yes[idx_a], "background exclusion should affect signal pixels")
        self.assertNotEqual(px_no[idx_b], px_yes[idx_b], "background exclusion should affect brighter signal pixels")
        self.assertEqual(px_no[idx_bg], px_yes[idx_bg], "background pixels should remain unchanged")

    def test_threshold_controls_near_background_exclusion_per_channel(self):
        width = height = 6
        bg = (2, 2, 2)
        pixels = [bg] * (width * height)
        pixels[1 * width + 1] = (70, 90, 110)
        pixels[4 * width + 4] = (150, 180, 210)
        with tempfile.TemporaryDirectory() as td:
            src = os.path.join(td, "src.ppm")
            out_lo = os.path.join(td, "lo.png")
            out_hi = os.path.join(td, "hi.png")
            _write_ppm(src, width, height, pixels)
            _, _, px_lo = self._run(src, out_lo, exclude_background=1, background_color="000000", background_threshold=1)
            _, _, px_hi = self._run(src, out_hi, exclude_background=1, background_color="000000", background_threshold=2)
        idx = 1 * width + 1
        self.assertNotEqual(px_lo, px_hi)
        self.assertLess(px_hi[idx][0], px_lo[idx][0], "threshold=2 should exclude (2,2,2) halo pixels per channel")

    def test_all_background_image_falls_back_and_succeeds(self):
        width = height = 4
        pixels = [(0, 0, 0)] * (width * height)
        with tempfile.TemporaryDirectory() as td:
            src = os.path.join(td, "src.ppm")
            outp = os.path.join(td, "out.png")
            _write_ppm(src, width, height, pixels)
            out_w, out_h, px = self._run(src, outp, exclude_background=1, background_color="000000", background_threshold=4)
        self.assertEqual((out_w, out_h), (width, height))
        self.assertTrue(all(p == (0, 0, 0) for p in px))


if __name__ == "__main__":
    unittest.main()
