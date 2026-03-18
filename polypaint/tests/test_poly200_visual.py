"""
Visual test: render poly_101..poly_109 via Python numpy AND C sweep, compare.

Pipeline: x1,x2 ∈ [0,1) → unit_circle → poly_N → rev → np.roots → plot on [-2,2]x[-2,2]

Usage:
  cd polypaint && uv run python tests/test_poly200_visual.py
  # outputs /tmp/poly_NNN_py.png and /tmp/poly_NNN_c.png
"""
import json
import math
import os
import struct
import subprocess
import zlib
import numpy as np

pi = math.pi

def poly_101(t1, t2):
    try:
        cf = np.zeros(71, dtype=complex)
        cf[0:10] = 1000 * np.arange(1, 11)
        cf[10:71] = 1
        cf[14] = -1 * np.abs(t1)**3
        cf[29] = -1 * np.abs(t2)**4
        cf[44] = np.abs(t1)**5
        cf[59] = np.abs(t2)**6
        cf[19] = np.abs(t1)**2 * np.sin(np.angle(t1))
        cf[39] = np.abs(t2)**3 * np.cos(np.angle(t2))
        cf[24] = np.log(np.abs(t1) + 1) * np.abs(t1)
        cf[49] = np.log(np.abs(t2) + 1) * np.abs(t2)
        for j in range(1, 36):
            cf[2*j] = cf[2*j] * (np.sin(j * t1) + np.cos(j * t2)) + cf[2*j + 1] * (np.cos(j * t1) + np.sin(j * t2))
        cf = np.where(np.isfinite(cf), cf, 0)
        return cf.astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_102(t1, t2):
    try:
        cf = np.zeros(71, dtype=complex)
        cf[0] = 1000 * (t1 + t2)**2
        for k in range(1, 20):
            cf[k] = (k + 1) * cf[k - 1] + np.sin((k + 1) * t1) + np.cos((k + 1) * t2)
        for k in range(20, 40):
            cf[k] = (k + 1) * cf[k - 1] - np.sin((k + 1) * t1) - np.cos((k + 1) * t2)
        for k in range(40, 60):
            cf[k] = (k + 1) * cf[k - 1] + np.sin((k + 1) * t1 * t2) + np.cos((k + 1) * t1 * t2)
        for k in range(60, 70):
            cf[k] = (k + 1) * cf[k - 1] - np.sin((k + 1) * t1 * t2) - np.cos((k + 1) * t1 * t2)
        cf[70] = np.abs(cf[69]) + np.angle(t1) - np.angle(t2) + np.real(t1 * t2) - np.imag(np.conj(t1) * t2)
        cf = np.where(np.isfinite(cf), cf, 0)
        return cf.astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_103(t1, t2):
    try:
        cf = np.zeros(71, dtype=complex)
        cf[0] = t2 * np.log(np.abs(t1) + 1)
        cf[1] = t1 * np.log(np.abs(t2) + 1)
        for k in range(2, 32, 2):
            cf[k] = np.sin(k * t1) * np.log(np.abs(t2) + 1)
            cf[k + 1] = np.cos(k * t2) * np.log(np.abs(t1) + 1)
        for k in range(32, 52, 2):
            cf[k] = np.cos(k * t1) * np.log(np.abs(t2) + 1)
            cf[k + 1] = np.sin(k * t2) * np.log(np.abs(t1) + 1)
        for k in range(52, 72, 2):
            cf[k] = t1 * np.log(np.abs(t2 * (k + 1)) + 1)
            cf[k + 1] = t2 * np.log(np.abs(t1 * (k + 1)) + 1)
        mod_cf = (71 - np.arange(1, 72)) * np.abs(cf)
        arg_cf = np.arange(1, 72) / 71 * np.angle(cf)
        cf = mod_cf * np.exp(1j * arg_cf)
        cf = np.where(np.isfinite(cf), cf, 0)
        return cf.astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_104(t1, t2):
    try:
        cf = np.zeros(71, dtype=complex)
        for k in range(1, 72):
            cf[k - 1] = ((np.abs(t1)**(1/k)) * (np.cos(np.angle(t1)) + 1j * np.sin(np.angle(t1))) +
                          (np.abs(t2)**(1/k)) * (np.cos(np.angle(t2)) + 1j * np.sin(np.angle(t2)))) / k
        cf[np.arange(0, 71, 3)] *= -1
        cf[np.arange(1, 71, 4)] *= 2
        cf[np.arange(2, 71, 5)] *= 3
        cf[np.arange(3, 71, 6)] *= 4
        cf[np.arange(4, 71, 7)] *= 5
        cf = np.where(np.isfinite(cf), cf, 0)
        return cf.astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_105(t1, t2):
    try:
        cf = np.zeros(71, dtype=complex)
        for k in range(1, 72):
            cf[k - 1] = np.sin(k * (np.real(t1) * np.imag(t2))**3) + np.cos(k * np.log(np.abs(t1 * t2 + 1)) * np.angle(t1 + np.conj(t2)))
        cf = np.where(np.isfinite(cf), cf, 0)
        return cf.astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_106(t1, t2):
    try:
        cf = np.zeros(71, dtype=complex)
        for k in range(1, 72):
            cf[k - 1] = ((t1**3 + t2**2)**2 + np.abs(t1 - t2) + np.sin(t1 * t2)) * np.abs(t1 + t2)**(1/k)
        cf[0] *= 100; cf[1] *= 90; cf[2] *= 80; cf[3] *= 70; cf[4] *= 60
        cf[5] *= 50; cf[6] *= 40; cf[7] *= 30; cf[8] *= 20; cf[9] *= 10
        cf[11] *= 5; cf[23] *= 4; cf[35] *= 3; cf[47] *= 2; cf[59] *= 1
        for k in range(15, 72):
            cf[k] = -cf[k] * np.log(np.abs(k))
        cf = np.where(np.isfinite(cf), cf, 0)
        return cf.astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_107(t1, t2):
    try:
        cf = np.zeros(71, dtype=complex)
        for r in range(1, 72):
            cf[r - 1] = (100 * (t1 ** (71 - r))) * np.sin(0.5 * t1 * r) + \
                         (100 * (t2 ** r)) * np.cos(0.5 * t2 * r)
        cf[14] = 100 * t2**3 - 100 * t2**2 + (100 * t2 - 100)
        cf[29] = 100 * np.log(np.abs(t1 * t2) + 1)
        cf[44] = np.abs(10 * t1 + 0.5 * t2)
        cf[59] = np.angle(0.2 * t1 - 3j * t2)
        cf[70] = np.real(10 * t1 + 0.5 * t2)
        cf = np.where(np.isfinite(cf), cf, 0)
        return cf.astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_108(t1, t2):
    try:
        cf = np.zeros(71, dtype=complex)
        cf[0] = t1 + t2
        cf[1] = -(t1**2 + t2)
        cf[2] = t1**2 - t2**2 - 1j
        cf[3:10] = [1 - t1, -1 + t2, 2 - t1, -2 + t2, 3 - t1, -3 + t2, 4 - t1]
        cf[10] = 15 * (np.real(t1) + np.imag(t2))
        cf[11] = -17 * np.angle(t1) * np.angle(t2)
        cf[14] = 30 * np.abs(t1) * np.abs(t2)
        cf[16] = -(t1**3 + t2**3)
        cf[18] = (t1**2 - t2**2) * 1j
        cf[19] = 5 + 1j * t1
        cf[24] = 50 * np.abs(t1 - t2)
        cf[29] = -40 * np.real(t1) + 35 * np.imag(t2)
        cf[34] = np.sum([3, 3, 9, 15, -12]) * (np.real(t1) - np.imag(t2))
        cf[39] = -t1**4 + t2**4 - 3
        cf[44] = 3 * np.angle(t1) + 4 * np.angle(t2)
        cf[49] = -55 * np.abs(np.abs(t1) - np.abs(t2))
        cf[54] = 33 * np.abs(t1)**3 + np.abs(t2)**2
        cf[59] = t1**5 + t2**5 - 29
        cf[64] = -22 * np.real(t1**2) + 22 * np.imag(t2**2)
        cf[69] = (np.sum(range(1, 6)) * np.imag(t1)) + (np.prod(range(1, 6)) * np.real(t2))
        cf = np.where(np.isfinite(cf), cf, 0)
        return cf.astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)

def poly_109(t1, t2):
    try:
        cf = np.zeros(71, dtype=complex)
        cf[0] = t1 + t2
        cf[1] = t1**2 - 2*t2 + 5
        cf[2] = np.conj(t1) * t2 + 7
        cf[3] = t2**2 - t1 + 11
        cf[4] = np.abs(t1 + t2) + 13
        cf[5] = np.angle(t1) * np.angle(t2) + 17
        cf[6] = t1 * t2 - 19
        cf[7] = t1**3 + t2**3 + 23
        cf[8] = np.sin(t1) + np.cos(t2) + 29
        cf[9] = np.log(np.abs(t1 + t2) + 1) + 31
        cf[10] = t1**2 - t2**2 + 37
        cf[11] = np.conj(t2) * t1 + 41
        cf[12] = np.imag(t1) * np.real(t2) - 43
        cf[13] = t1 * np.conj(t2) + 47
        cf[14] = np.abs(t1 - t2) + 53
        cf[15] = t1**4 - t2**4 + 59
        cf[16] = 61 - 5 * t1 * t2
        cf[17] = 67 + np.abs(t1**2 + t2**2)
        cf[18] = 71 + t1**5 + t2**5
        cf[19] = 73 - np.angle(t1) * np.angle(t2)
        cf[20] = 79 + np.abs(t1**3 + t2**3)
        cf[21] = 83 - t1**6 + t2**6
        cf[22] = 89 + np.sin(t1 + t2)
        cf[23] = np.abs(np.real(t1) * np.imag(t2)) + 97
        cf[24] = 101 + t1 * t2**2
        cf[25] = 103 - np.conj(t1) * np.real(t2)
        cf[26] = 107 + t1**7 - t2**7
        cf[27] = 109 + np.abs(np.conj(t1 - t2))
        cf[28] = 113 - np.abs(t1**2 - t2**2)
        cf[29] = 127 + (t1**8 * t2**8)
        cf[30] = t1 - t2 + np.abs(t1 * t2) + 131
        cf[31] = 137 + np.angle(t1**2) - np.angle(t2**2)
        cf[32] = 139 - t1**9 + t2**9
        cf[33] = np.log(np.abs(t1 * t2) + 1) + 149
        cf[34] = 151 + (np.abs(t1) + np.abs(t2))**2
        cf[35] = np.sin(2 * t1) - np.cos(2 * t2) + 157
        cf[36] = np.log(np.abs(t1 - t2) + 1) + 163
        cf[37] = 167 + np.real(t1**3) - np.imag(t2**3)
        cf[38] = 173 - (t1**2 * t2**2)**1.5
        cf[39] = 179 + np.angle(t1 * t2) + 1j
        cf[40] = 181 - np.conj(t1**3 - t2**3)
        cf[41] = 191 + np.abs(t1) * np.abs(t2)
        cf[42] = 193 - np.abs(np.real(t1) + np.imag(t2))
        cf[43] = 197 + np.sin(t1**2 + t2**2)
        cf[44] = 199 - t1 * t2**3
        cf[45] = t1 * np.imag(t2) + 211
        cf[46] = np.abs(t1**4 + t2**4) + 223
        cf[47] = 227 - np.conj(t1**2) * np.conj(t2**2)
        cf[48] = 229 + np.sin(t1 * t2) - np.cos(t1 - t2)
        cf[49] = 233 + t1**9 - t2**9
        cf[50] = 239 - np.abs(np.conj(t1**2 + t2**2))
        cf[51] = 241 + t1**3 + t2**3
        cf[52] = t1**10 + t2**10 + 251
        cf[53] = t1 * t2 * np.real(t1 + t2) - 257
        cf[54] = np.abs(t1 - t2) - 263
        cf[55] = t1**11 - t2**11 + 269
        cf[56] = 271 + np.abs(t1 * t2**2 - t2**3)
        cf[57] = 277 + np.sin(t1**3 - t2**3)
        cf[58] = 281 - np.conj(t1**2 * t2)
        cf[59] = np.conj(t1**5 + t2**5) + 283
        cf[60] = np.angle(t1**3 * t2**3) + 293
        cf[61] = 307 - np.sin(t1 * t2 + 1j)
        cf[62] = np.abs(t1**6 + t2**6) + 311
        cf[63] = 313 - np.cos(t1**3 - t2**3)
        cf[64] = np.angle(t1 * t2) + 317
        cf[65] = np.real(t1**2 - t2**2) - 331
        cf[66] = 337 + np.abs(t1**6 * t2**6)
        cf[67] = 347 - np.abs(t1**4 - t2**4)
        cf[68] = 349 + np.sin(np.conj(t1 - t2))
        cf[69] = 353 - np.cos(t1 + t2**2)
        cf[70] = np.abs((t1 + t2)**3 - 359)
        cf = np.where(np.isfinite(cf), cf, 0)
        return cf.astype(np.complex128)
    except:
        return np.zeros(71, dtype=np.complex128)


POLY_FUNCS = {
    101: poly_101, 102: poly_102, 103: poly_103, 104: poly_104,
    105: poly_105, 106: poly_106, 107: poly_107, 108: poly_108,
    109: poly_109,
}


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


def render_python(poly_func, N, pix, extent):
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
            coeffs = poly_func(t1, t2)
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

    return gray, total_roots, plotted


SWEEP = os.path.join(os.path.dirname(__file__), "..", "lambda", "sweep_test")


def render_sweep(func_name, N, pix, extent):
    """Run C sweep binary: coeffgen → solve → collect pixels."""
    import subprocess, json

    # Coeffgen
    coeffs_path = f"/tmp/{func_name}_coeffs.bin"
    cg_json = json.dumps({
        "mode": "coeffgen",
        "function": func_name,
        "n1": N, "n2": N,
        "i1_start": 0, "i1_end": N,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": ["rev"],
        "times": 1, "dither_pass": 0
    })
    r = subprocess.run([SWEEP, coeffs_path], input=cg_json.encode(),
                       capture_output=True, timeout=60)
    if r.returncode != 0:
        print(f"  coeffgen FAILED: {r.stderr.decode()}")
        return None, 0, 0
    meta = json.loads(r.stdout)
    n_coeffs = meta["n_coeffs"]
    n_t = meta["n_t"]

    # Solve
    roots_path = f"/tmp/{func_name}_roots_c.bin"
    solve_json = json.dumps({
        "mode": "solve",
        "coeffs_file": coeffs_path,
        "n_coeffs": n_coeffs,
        "n1": N, "n2": N,
        "n_t": n_t
    })
    r = subprocess.run([SWEEP, roots_path], input=solve_json.encode(),
                       capture_output=True, timeout=120)
    if r.returncode != 0:
        print(f"  solve FAILED: {r.stderr.decode()}")
        return None, 0, 0

    # Read roots and render
    with open(roots_path, "rb") as f:
        data = f.read()
    n_roots = len(data) // 8
    half = pix / 2.0
    scale = pix / (2 * extent)
    gray = bytearray(pix * pix)
    plotted = 0
    for i in range(n_roots):
        re = struct.unpack_from('<f', data, i * 8)[0]
        im = struct.unpack_from('<f', data, i * 8 + 4)[0]
        if not (math.isfinite(re) and math.isfinite(im)):
            continue
        px = int(half + re * scale)
        py = int(half - im * scale)
        if 0 <= px < pix and 0 <= py < pix:
            gray[py * pix + px] = 255
            plotted += 1

    return gray, n_roots, plotted


def pixel_overlap(gray_a, gray_b, pix):
    set_a = {i for i in range(pix * pix) if gray_a[i]}
    set_b = {i for i in range(pix * pix) if gray_b[i]}
    if not set_a and not set_b:
        return 100.0, 0, 0
    if not set_a or not set_b:
        return 0.0, len(set_a), len(set_b)
    overlap = len(set_a & set_b)
    return 100.0 * overlap / max(len(set_a), len(set_b)), len(set_a), len(set_b)


def main():
    N = 100
    extent = 2.0
    pix = 1000
    funcs_to_test = [101, 102, 103, 104, 105, 106, 107, 108, 109]

    for fnum in funcs_to_test:
        func_name = f"poly_{fnum}"
        print(f"\n{func_name} [unit_circle] [rev] N={N}, view=[-{extent},{extent}]x[-{extent},{extent}], {pix}px bilevel")

        # Python reference
        print(f"  Python...")
        py_gray, py_total, py_plotted = render_python(POLY_FUNCS[fnum], N, pix, extent)
        py_path = f"/tmp/{func_name}_py.png"
        with open(py_path, "wb") as f:
            f.write(encode_png_gray(pix, pix, py_gray))
        print(f"    total roots: {py_total:,}, plotted: {py_plotted:,}")

        # C sweep
        print(f"  C sweep...")
        c_gray, c_total, c_plotted = render_sweep(func_name, N, pix, extent)
        if c_gray is None:
            print(f"    FAILED — no C image")
            continue
        c_path = f"/tmp/{func_name}_c.png"
        with open(c_path, "wb") as f:
            f.write(encode_png_gray(pix, pix, c_gray))
        print(f"    total roots: {c_total:,}, plotted: {c_plotted:,}")

        # Compare
        overlap, py_px, c_px = pixel_overlap(py_gray, c_gray, pix)
        status = "OK" if overlap >= 60 else "BROKEN"
        print(f"  Overlap: {overlap:.1f}% (py={py_px}, c={c_px}) → {status}")


if __name__ == "__main__":
    main()
