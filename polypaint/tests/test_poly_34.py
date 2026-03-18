"""
Standalone visual test: run poly_34 pipeline in pure Python and save bilevel PNG.

Pipeline: x1,x2 ∈ [0,1) → unit_circle → poly_34 → rev → np.roots → plot on [-2,2]x[-2,2]

Usage:
  cd polypaint && uv run python tests/test_poly_34.py
  # outputs /tmp/poly34_roots.png
"""
import math
import struct
import zlib

import numpy as np


def poly_34(t1, t2):
    try:
        cf = np.zeros(71, dtype=complex)
        for i in range(1, 72):
            cf[i-1] = (t1 * t2) * np.sin(i * t1 + t2) + np.cos(i * t1 - t2) + i * np.log(np.abs(t1 + i * t2) + 1) / (1 + np.abs(t1 + i * t2))
        cf[12] = 3 * (t1**2 - t2**2)
        cf[13] = cf[12] + t1 * t2 * np.sin(np.angle(t1 + t2))
        cf[14] = 2 * cf[13] - t1 * t2 * np.cos(np.angle(t1 - t2))
        cf[15] = 3 * cf[12] - cf[13] + t1 * t2 * np.sin(2 * np.angle(t1 - t2))
        cf[16] = 2 * cf[12] - 3 * cf[13] + cf[14] - t1 * t2 * np.cos(2 * np.angle(t1 + t2))
        cf[69] = 2 * cf[13] - 3 * cf[12] + t1 * t2 * np.sin(np.angle(2 * t1 - t2))
        cf[70] = cf[16] - cf[13] * t1 * t2 * np.cos(np.angle(2 * t1 + t2))
        # NaN guard
        cf = np.where(np.isfinite(cf), cf, 0)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(71, dtype=np.complex128)


def encode_png_gray(width, height, buf):
    def chunk(ctype, data):
        c = ctype + data
        crc = zlib.crc32(c) & 0xFFFFFFFF
        return struct.pack(">I", len(data)) + c + struct.pack(">I", crc)
    raw = bytearray()
    for y in range(height):
        raw.append(0)
        raw.extend(buf[y * width:(y + 1) * width])
    ihdr = struct.pack(">IIBBBBB", width, height, 8, 0, 0, 0, 0)
    idat = zlib.compress(bytes(raw), 6)
    out = b'\x89PNG\r\n\x1a\n'
    out += chunk(b'IHDR', ihdr)
    out += chunk(b'IDAT', idat)
    out += chunk(b'IEND', b'')
    return out


def main():
    N = 100
    extent = 2.0
    pix = 1000

    half = pix / 2.0
    scale = pix / (2 * extent)

    gray = bytearray(pix * pix)
    total_roots = 0
    plotted = 0

    for i1 in range(N):
        x1 = i1 / N
        if i1 % 20 == 0:
            print(f"  row {i1}/{N}...")
        for i2 in range(N):
            x2 = i2 / N

            t1 = np.exp(2j * np.pi * x1)
            t2 = np.exp(2j * np.pi * x2)

            coeffs = poly_34(t1, t2)
            coeffs = coeffs[::-1]

            first_nz = 0
            while first_nz < len(coeffs) - 1 and abs(coeffs[first_nz]) < 1e-30:
                first_nz += 1
            effective = coeffs[first_nz:]

            if len(effective) < 2:
                continue
            if not np.all(np.isfinite(effective)):
                continue

            roots = np.roots(effective)
            total_roots += len(roots)

            for r in roots:
                if not (np.isfinite(r.real) and np.isfinite(r.imag)):
                    continue
                px = int(half + r.real * scale)
                py = int(half - r.imag * scale)
                if 0 <= px < pix and 0 <= py < pix:
                    gray[py * pix + px] = 255
                    plotted += 1

    print(f"  total roots: {total_roots:,}, plotted: {plotted:,}")

    png_data = encode_png_gray(pix, pix, gray)
    out_path = "/tmp/poly34_roots.png"
    with open(out_path, "wb") as f:
        f.write(png_data)
    print(f"  saved {out_path} ({len(png_data):,} bytes)")


if __name__ == "__main__":
    print("poly_34 [unit_circle] [rev] N=100, view=[-2,2]x[-2,2], 1000px bilevel")
    main()
