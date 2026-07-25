"""png_rgb: the bake pipeline's stdlib PNG decoder. Round-trips our own
encoder, then pins every filter type against hand-filtered scanlines (vips
uses adaptive filtering, so all five appear in real palette PNGs), plus
RGBA/grey normalization and the loud-failure envelope."""
import os
import struct
import sys
import unittest
import zlib

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))
from png_rgb import decode_png_rgb, encode_png_rgb  # noqa: E402

_SIG = b"\x89PNG\r\n\x1a\n"


def _chunk(ctype, body):
    blob = ctype + body
    return struct.pack(">I", len(body)) + blob + struct.pack(">I", zlib.crc32(blob) & 0xFFFFFFFF)


def _png(width, height, bit_depth, color_type, scanlines):
    ihdr = struct.pack(">IIBBBBB", width, height, bit_depth, color_type, 0, 0, 0)
    return (_SIG + _chunk(b"IHDR", ihdr)
            + _chunk(b"IDAT", zlib.compress(bytes(scanlines)))
            + _chunk(b"IEND", b""))


class TestPngRgb(unittest.TestCase):
    def test_round_trip_rgb(self):
        rgb = bytes(range(2 * 3 * 3))            # 2x3 distinct bytes
        w, h, out = decode_png_rgb(encode_png_rgb(2, 3, rgb))
        self.assertEqual((w, h), (2, 3))
        self.assertEqual(out, rgb)

    def test_all_filter_types_reconstruct(self):
        # one 3-pixel RGB row per filter, hand-filtered from the target row
        target = bytes([10, 20, 30, 40, 60, 80, 90, 120, 150])
        rows = []
        # f0 None: raw as-is
        rows.append(bytes([0]) + target)
        # f1 Sub: line[i] -= line[i-3]
        f1 = bytearray(target)
        for i in range(len(target) - 1, 2, -1):
            f1[i] = (f1[i] - target[i - 3]) & 0xFF
        rows.append(bytes([1]) + bytes(f1))
        # f2 Up: line[i] -= prev[i] (prev = target, the row above)
        rows.append(bytes([2]) + bytes([0] * len(target)))
        # f3 Average: line[i] -= (left + up)//2 with up = target
        f3 = bytearray()
        for i, v in enumerate(target):
            left = target[i - 3] if i >= 3 else 0
            f3.append((v - ((left + target[i]) >> 1)) & 0xFF)
        rows.append(bytes([3]) + bytes(f3))
        # f4 Paeth: predictor collapses to max(left, up) here; check via decode
        f4 = bytearray()
        for i, v in enumerate(target):
            left = target[i - 3] if i >= 3 else 0
            up = target[i]
            ul = target[i - 3] if i >= 3 else 0
            p = left + up - ul
            pa, pb, pc = abs(p - left), abs(p - up), abs(p - ul)
            pred = left if (pa <= pb and pa <= pc) else (up if pb <= pc else ul)
            f4.append((v - pred) & 0xFF)
        rows.append(bytes([4]) + bytes(f4))
        blob = _png(3, 5, 8, 2, b"".join(rows))
        w, h, out = decode_png_rgb(blob)
        self.assertEqual((w, h), (3, 5))
        for row in range(5):
            self.assertEqual(out[row * 9:(row + 1) * 9], target, f"filter row {row}")

    def test_rgba_strips_alpha(self):
        line = bytes([0]) + bytes([1, 2, 3, 255, 4, 5, 6, 7])
        w, h, out = decode_png_rgb(_png(2, 1, 8, 6, line))
        self.assertEqual((w, h), (2, 1))
        self.assertEqual(out, bytes([1, 2, 3, 4, 5, 6]))

    def test_grey_expands(self):
        line = bytes([0]) + bytes([9, 200])
        w, h, out = decode_png_rgb(_png(2, 1, 8, 0, line))
        self.assertEqual(out, bytes([9, 9, 9, 200, 200, 200]))

    def test_rejects_outside_the_envelope(self):
        with self.assertRaises(RuntimeError):
            decode_png_rgb(b"GIF89a")
        with self.assertRaises(RuntimeError):
            decode_png_rgb(_png(1, 1, 16, 2, bytes([0, 0, 0, 0, 0, 0, 0])))   # 16-bit
        with self.assertRaises(RuntimeError):
            decode_png_rgb(_png(1, 1, 8, 3, bytes([0, 0])))                   # palettized
        # truncated scanline data
        with self.assertRaises(RuntimeError):
            decode_png_rgb(_png(4, 4, 8, 2, bytes([0, 1, 2, 3])))


if __name__ == "__main__":
    unittest.main()
