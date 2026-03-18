"""
Standalone visual test: run poly_33 pipeline in pure Python and save bilevel PNG.

Pipeline: x1,x2 ∈ [0,1) → unit_circle → poly_33 → rev → np.roots → plot on [-2,2]x[-2,2]

Usage:
  cd polypaint && uv run python tests/test_poly_33.py
  # outputs /tmp/poly33_roots.png
"""
import math
import struct
import zlib

import numpy as np


def poly_33(t1, t2):
    try:
        cf = np.zeros(71, dtype=complex)
        f = lambda z, n: z**n - 1
        cf[0:35] = [np.real(f(t1, n)) - np.imag(f(t2, n)) for n in range(1, 36)]
        cf[35:70] = [np.log(np.abs(f(t2, n))) + np.angle(f(t1, n)) + np.sin(np.abs(f(t1, n))) + np.cos(np.angle(f(t2, n))) for n in range(1, 36)]
        cf[70] = np.prod(cf[0:70])
        # Replace any NaN/inf with 0
        cf = np.where(np.isfinite(cf), cf, 0)
        return cf.astype(np.complex128)
    except Exception:
        return np.zeros(71, dtype=np.complex128)


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
        if i1 % 50 == 0:
            print(f"  row {i1}/{N}...")
        for i2 in range(N):
            x2 = i2 / N

            # unit_circle
            t1 = np.exp(2j * np.pi * x1)
            t2 = np.exp(2j * np.pi * x2)

            # poly_33
            coeffs = poly_33(t1, t2)

            # rev
            coeffs = coeffs[::-1]

            # strip leading zeros
            first_nz = 0
            while first_nz < len(coeffs) - 1 and abs(coeffs[first_nz]) < 1e-30:
                first_nz += 1
            effective = coeffs[first_nz:]

            if len(effective) < 2:
                continue

            # skip if any NaN/inf in coefficients
            if not np.all(np.isfinite(effective)):
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
    out_path = "/tmp/poly33_roots.png"
    with open(out_path, "wb") as f:
        f.write(png_data)
    print(f"  saved {out_path} ({len(png_data):,} bytes)")


if __name__ == "__main__":
    print("poly_33 [unit_circle] [rev] N=100, view=[-2,2]x[-2,2], 1000px bilevel")
    main()
