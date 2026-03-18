"""
Standalone visual test: poly_125 with bugfix (cf[61:71] not cf[61:70]).

The original poly200.py has cf[61:70] = [... for n in range(61, 71)] which is
9 slots but 10 values → ValueError. Fixed here to cf[61:71].

Usage:
  cd polypaint && uv run python tests/test_poly_125.py
  # outputs /tmp/poly125_roots.png
"""
import math
import struct
import zlib
import numpy as np


def poly_125_fixed(t1, t2):
    """Exact copy from poly200.py line 542, with cf[61:70] fixed to cf[61:71]."""
    cf = np.zeros(71, dtype=complex)
    cf[0:15] = [(-1)**j * (j**2) * (np.abs(t1) + np.abs(t2)) for j in range(1, 16)]
    cf[15:30] = [(-1)**(k + 1) * (k**3) * np.angle(t1 + 1j * t2) for k in range(16, 31)]
    cf[30:45] = [(-1)**(r + 1) * np.cos(r * t1) + np.sin(r * t2) for r in range(31, 46)]
    for s in range(46, 61):
        cf[s] = (-1)**s * (s**2) * np.conj(t1) * np.conj(t2)
    cf[61:71] = [n**3 * np.log(np.abs(t1 * t2) + 1) for n in range(61, 71)]  # FIXED: was cf[61:70]
    cf[70] = 1
    cf = np.where(np.isfinite(cf), cf, 0)
    return cf.astype(np.complex128)


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
            coeffs = poly_125_fixed(t1, t2)
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
    out_path = "/tmp/poly125_roots.png"
    with open(out_path, "wb") as f:
        f.write(png_data)
    print(f"  saved {out_path} ({len(png_data):,} bytes)")


if __name__ == "__main__":
    print("poly_125_fixed [unit_circle] [rev] N=100, view=[-2,2]x[-2,2], 1000px bilevel")
    main()
