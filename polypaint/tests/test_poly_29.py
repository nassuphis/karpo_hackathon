"""
Standalone visual test: run poly_29 pipeline in pure Python and save bilevel PNG.

Pipeline: x1,x2 ∈ [0,1) → unit_circle → poly_29 → rev → np.roots → plot on [-2,2]x[-2,2]

Usage:
  cd polypaint && uv run python tests/test_poly_29.py
  # outputs /tmp/poly29_roots.png
"""
import math
import struct
import zlib

import numpy as np


def poly_29(t1, t2):
    cf = np.zeros(71, dtype=complex)
    cf[0:5] = np.array([1, t1, t1**2, t1**3, t1**4])
    cf[5:10] = np.array([1, t2, t2**2, t2**3, t2**4])
    cf[10:15] = np.array([1, np.exp(1j * t1), np.exp(2j * t1), np.exp(3j * t1), np.exp(4j * t1)])
    cf[15:20] = np.array([1, np.exp(1j * t2), np.exp(2j * t2), np.exp(3j * t2), np.exp(4j * t2)])
    cf[20:30] = np.array([1, np.real(t1 + t2), np.imag(t1 + t2), np.real(t1 * t2), np.imag(t1 * t2),
                           np.real(t1 + t2)**2, np.imag(t1 + t2)**2, np.real(t1 * t2)**2, np.imag(t1 * t2)**2,
                           np.abs(t1 + t2)])
    cf[30:40] = np.arange(1, 11) * np.abs(t1) * np.abs(t2)
    cf[40:50] = np.array([1, np.log(np.abs(t1) + 1), np.log(np.abs(t2) + 1), np.log(np.abs(t1 + t2) + 1),
                           np.log(np.abs(t1 * t2) + 1), np.angle(t1), np.angle(t2), np.abs(t1), np.abs(t2),
                           np.angle(t1 + t2)])
    cf[50] = np.abs(t1 + t2) * np.angle(t1 * t2)
    return cf.astype(np.complex128)


def encode_png_gray(width, height, buf):
    """Encode grayscale buffer to PNG."""
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
    N = 500
    extent = 2.0
    pix = 4096

    half = pix / 2.0
    scale = pix / (2 * extent)

    gray = bytearray(pix * pix)
    total_roots = 0
    plotted = 0

    for i1 in range(N):
        x1 = i1 / N
        if i1 % 50 == 0:
            print(f"  row {i1}/{N}...")
        for i2 in range(N):
            x2 = i2 / N

            # unit_circle
            t1 = np.exp(2j * np.pi * x1)
            t2 = np.exp(2j * np.pi * x2)

            # poly_29
            coeffs = poly_29(t1, t2)

            # rev
            coeffs = coeffs[::-1]

            # strip leading zeros
            first_nz = 0
            while first_nz < len(coeffs) - 1 and abs(coeffs[first_nz]) < 1e-30:
                first_nz += 1
            effective = coeffs[first_nz:]

            if len(effective) < 2:
                continue

            # solve
            roots = np.roots(effective)
            total_roots += len(roots)

            # plot
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
    out_path = "/tmp/poly29_roots.png"
    with open(out_path, "wb") as f:
        f.write(png_data)
    print(f"  saved {out_path} ({len(png_data):,} bytes)")


if __name__ == "__main__":
    print("poly_29 [unit_circle] [rev] N=500, view=[-2,2]x[-2,2], 4096px bilevel")
    main()
