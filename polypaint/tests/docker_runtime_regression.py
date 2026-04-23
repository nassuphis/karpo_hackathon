#!/usr/bin/env python3
"""
Docker runtime regression test for deploy binaries.

Runs inside ARM64 Docker container with /src mounted to lambda/.
Tests: AE/AE-MT/CM solvers, CFPV coeffgen, libvips preview/resize runtime,
solve_proximity_stats, and catalog degree verification.

Invoked by scripts/test-docker-runtime.sh — not run directly.
"""
import json
import math
import os
import http.server
import socketserver
import struct
import subprocess
import sys
import threading
import types
import urllib.parse


# ── Helpers ──────────────────────────────────────────────────────────────

def write_cf(path, coeffs_list, n_coeffs):
    with open(path, "wb") as f:
        for cf in coeffs_list:
            padded = list(cf) + [0.0] * (n_coeffs - len(cf))
            for c in padded[:n_coeffs]:
                re = c.real if hasattr(c, "real") else float(c)
                im = c.imag if hasattr(c, "imag") else 0.0
                f.write(struct.pack("<ff", re, im))


def read_roots(path, degree):
    with open(path, "rb") as f:
        data = f.read()
    vals = struct.unpack("<" + "f" * (len(data) // 4), data)
    roots = [complex(vals[i], vals[i + 1]) for i in range(0, len(vals), 2)]
    polys = []
    for i in range(0, len(roots), degree):
        polys.append(roots[i:i + degree])
    return polys


def polyval(cf, z):
    y = 0j
    for a in cf:
        y = y * z + complex(a)
    return y


def run_coeffgen(func, cfpv=None):
    spec = {
        "mode": "coeffgen",
        "function": func,
        "n1": 4, "n2": 4,
        "i1_start": 0, "i1_end": 4,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [],
        "times": 1,
    }
    if cfpv is not None:
        spec["cfpv"] = cfpv
    r = subprocess.run(["/src/sweep", "/tmp/cfpv_test.bin"],
                       input=json.dumps(spec), capture_output=True, text=True, timeout=10)
    if r.returncode != 0:
        return None, r.stderr
    return json.loads(r.stdout), None


def cleanup(*paths):
    for p in paths:
        try:
            os.remove(p)
        except OSError:
            pass


class _RangeHandler(http.server.BaseHTTPRequestHandler):
    file_bytes = b""

    def do_GET(self):
        if self.path != "/input.bin":
            self.send_response(404)
            self.end_headers()
            return
        data = type(self).file_bytes
        start = 0
        end = len(data) - 1
        range_hdr = self.headers.get("Range")
        if range_hdr and range_hdr.startswith("bytes="):
            raw = range_hdr[len("bytes="):]
            if "-" not in raw:
                self.send_response(416)
                self.end_headers()
                return
            lo, hi = raw.split("-", 1)
            start = int(lo) if lo else 0
            end = int(hi) if hi else len(data) - 1
            if start < 0 or end < start or end >= len(data):
                self.send_response(416)
                self.end_headers()
                return
            self.send_response(206)
            self.send_header("Content-Range", f"bytes {start}-{end}/{len(data)}")
        else:
            self.send_response(200)
        chunk = data[start:end + 1]
        self.send_header("Content-Type", "application/octet-stream")
        self.send_header("Content-Length", str(len(chunk)))
        self.end_headers()
        self.wfile.write(chunk)

    def log_message(self, fmt, *args):
        return


def read_f32_array(path):
    with open(path, "rb") as f:
        data = f.read()
    assert len(data) % 4 == 0, "float32 array size mismatch for %s" % path
    return struct.unpack("<" + "f" * (len(data) // 4), data)


def read_png_dims(path):
    with open(path, "rb") as f:
        header = f.read(24)
    assert header[:8] == b"\x89PNG\r\n\x1a\n", "PNG signature missing for %s" % path
    assert header[12:16] == b"IHDR", "PNG IHDR missing for %s" % path
    return struct.unpack(">II", header[16:24])


def read_jpeg_dims(path):
    with open(path, "rb") as f:
        assert f.read(2) == b"\xff\xd8", "JPEG SOI missing for %s" % path
        while True:
            marker = f.read(1)
            if not marker:
                break
            while marker == b"\xff":
                marker = f.read(1)
                if not marker:
                    break
            if not marker:
                break
            code = marker[0]
            if code in (0xD8, 0xD9):
                continue
            seg_len_bytes = f.read(2)
            if len(seg_len_bytes) != 2:
                break
            seg_len = struct.unpack(">H", seg_len_bytes)[0]
            assert seg_len >= 2, "bad JPEG segment length in %s" % path
            if code in (0xC0, 0xC1, 0xC2, 0xC3, 0xC5, 0xC6, 0xC7, 0xC9, 0xCA, 0xCB, 0xCD, 0xCE, 0xCF):
                data = f.read(5)
                assert len(data) == 5, "truncated JPEG SOF in %s" % path
                height, width = struct.unpack(">HH", data[1:5])
                return width, height
            f.seek(seg_len - 2, os.SEEK_CUR)
    raise AssertionError("JPEG SOF marker not found for %s" % path)


def read_csv_grid(path):
    rows = []
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            rows.append([float(cell) for cell in line.replace("\t", ",").split(",") if cell])
    return rows


def encode_fragment_pairs(pairs):
    payload = bytearray()
    for pixel_idx, score in pairs:
        payload.extend(int(pixel_idx).to_bytes(4, "little", signed=False))
        payload.append(int(score) & 0xFF)
    return bytes(payload)


class _MemBody:
    def __init__(self, data):
        self._data = bytes(data)

    def read(self):
        return self._data

    def iter_chunks(self, chunk_size=1024 * 1024):
        for start in range(0, len(self._data), chunk_size):
            yield self._data[start:start + chunk_size]


class _MemS3:
    def __init__(self):
        self.objects = {}
        self.heads = {}

    def seed_object(self, key, body, *, content_type="application/octet-stream", metadata=None):
        self.objects[key] = {
            "Body": bytes(body),
            "ContentType": content_type,
            "Metadata": dict(metadata or {}),
        }

    def head_object(self, *, Bucket, Key):
        obj = self.objects.get(Key)
        if obj is None:
            raise RuntimeError("NoSuchKey: %s" % Key)
        return {
            "ContentType": obj.get("ContentType"),
            "Metadata": dict(obj.get("Metadata") or {}),
        }

    def get_object(self, *, Bucket, Key, Range=None):
        obj = self.objects.get(Key)
        if obj is None:
            raise RuntimeError("NoSuchKey: %s" % Key)
        body = obj.get("Body") or b""
        if Range:
            if not str(Range).startswith("bytes="):
                raise RuntimeError("Unsupported Range: %r" % (Range,))
            raw = str(Range)[len("bytes="):]
            if "-" not in raw:
                raise RuntimeError("Unsupported Range: %r" % (Range,))
            lo, hi = raw.split("-", 1)
            start = int(lo) if lo else 0
            end = int(hi) if hi else (len(body) - 1)
            body = body[start:end + 1]
        return {
            "Body": _MemBody(body),
            "ContentType": obj.get("ContentType"),
            "Metadata": dict(obj.get("Metadata") or {}),
        }

    def put_object(self, *, Bucket, Key, Body, ContentType=None, Metadata=None, **kwargs):
        if hasattr(Body, "read"):
            body = Body.read()
        else:
            body = Body
        self.objects[Key] = {
            "Body": bytes(body),
            "ContentType": ContentType or "application/octet-stream",
            "Metadata": dict(Metadata or {}),
        }
        return {"ETag": '"mem"'}


class _MemS3ObjectHandler(http.server.BaseHTTPRequestHandler):
    objects = {}

    def do_GET(self):
        key = urllib.parse.unquote(self.path.lstrip("/"))
        obj = type(self).objects.get(key)
        if obj is None:
            self.send_response(404)
            self.end_headers()
            return
        body = obj.get("Body") or b""
        self.send_response(200)
        self.send_header("Content-Type", obj.get("ContentType") or "application/octet-stream")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, fmt, *args):
        return


# ── AE/CM Solver Tests ───────────────────────────────────────────────────

def test_ae_cm_solvers():
    print("--- AE/AE-MT/CM solver regression ---")

    polys = [
        [1, -6, 11, -6],
        [1, -10, 35, -50, 24],
        [1, -2, 1],
        [1, 0, 1],
        [0, 1, -3, 2],
        [0, 0, 0, 0, 0],
    ]
    n_coeffs = 5

    write_cf("/tmp/test_cf.bin", polys, n_coeffs)

    ae_spec = json.dumps({
        "mode": "solve", "coeffs_file": "/tmp/test_cf.bin",
        "n_coeffs": n_coeffs, "n2": len(polys),
        "i1_start": 0, "i1_end": 1, "match_roots": False,
    })
    r = subprocess.run(["/src/sweep", "/tmp/ae_out.bin"],
                       input=ae_spec, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, "AE FAILED: " + r.stderr[:200]
    print("AE: %s" % json.loads(r.stdout))

    mt_spec_single = json.dumps({
        "mode": "solve_mt", "coeffs_file": "/tmp/test_cf.bin",
        "n_coeffs": n_coeffs, "n2": len(polys),
        "i1_start": 0, "i1_end": 1, "match_roots": False,
        "n_threads": 1,
    })
    r = subprocess.run(["/src/sweep_mt", "/tmp/ae_mt_single_out.bin"],
                       input=mt_spec_single, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, "AE-MT FAILED: " + r.stderr[:200]
    mt_single_meta = json.loads(r.stdout)
    print("AE-MT(1): %s" % mt_single_meta)
    assert mt_single_meta["n_threads"] == 1, "AE-MT single-thread did not report n_threads=1"

    mt_spec_multi = json.dumps({
        "mode": "solve_mt", "coeffs_file": "/tmp/test_cf.bin",
        "n_coeffs": n_coeffs, "n2": len(polys),
        "i1_start": 0, "i1_end": 1, "match_roots": False,
        "n_threads": 4,
    })
    r = subprocess.run(["/src/sweep_mt", "/tmp/ae_mt_out.bin"],
                       input=mt_spec_multi, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, "AE-MT(4) FAILED: " + r.stderr[:200]
    mt_meta = json.loads(r.stdout)
    print("AE-MT(4): %s" % mt_meta)
    assert mt_meta["n_threads"] >= 1, "AE-MT did not report n_threads"

    cm_spec = json.dumps({
        "mode": "solve_cm", "coeffs_file": "/tmp/test_cf.bin",
        "n_coeffs": n_coeffs, "n_steps": len(polys),
    })
    r = subprocess.run(["/src/sweep_cm", "/tmp/cm_out.bin"],
                       input=cm_spec, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, "CM FAILED: " + r.stderr[:200]
    print("CM: %s" % json.loads(r.stdout))

    ae_size = os.path.getsize("/tmp/ae_out.bin")
    ae_mt_single_size = os.path.getsize("/tmp/ae_mt_single_out.bin")
    ae_mt_size = os.path.getsize("/tmp/ae_mt_out.bin")
    cm_size = os.path.getsize("/tmp/cm_out.bin")
    expected = len(polys) * (n_coeffs - 1) * 2 * 4
    assert ae_size == expected, "AE size %d != %d" % (ae_size, expected)
    assert ae_mt_single_size == expected, "AE-MT(1) size %d != %d" % (ae_mt_single_size, expected)
    assert ae_mt_size == expected, "AE-MT size %d != %d" % (ae_mt_size, expected)
    assert cm_size == expected, "CM size %d != %d" % (cm_size, expected)

    with open("/tmp/ae_out.bin", "rb") as fh:
        ae_bytes = fh.read()
    with open("/tmp/ae_mt_single_out.bin", "rb") as fh:
        ae_mt_single_bytes = fh.read()
    assert ae_bytes == ae_mt_single_bytes, "AE-MT(1) output diverges from AE for match_roots=false fixture"

    degree = n_coeffs - 1
    ae_roots = read_roots("/tmp/ae_out.bin", degree)
    ae_mt_single_roots = read_roots("/tmp/ae_mt_single_out.bin", degree)
    ae_mt_roots = read_roots("/tmp/ae_mt_out.bin", degree)
    cm_roots = read_roots("/tmp/cm_out.bin", degree)
    assert len(ae_roots) == len(polys)
    assert len(ae_mt_single_roots) == len(polys)
    assert len(ae_mt_roots) == len(polys)
    assert len(cm_roots) == len(polys)

    labels = ["cubic", "quartic", "repeated", "complex", "leading-zero", "all-zero"]
    for i, (cf, ae_r, ae_mt_single_r, ae_mt_r, cm_r, label) in enumerate(zip(polys, ae_roots, ae_mt_single_roots, ae_mt_roots, cm_roots, labels)):
        ae_finite = all(math.isfinite(r.real) and math.isfinite(r.imag) for r in ae_r)
        ae_mt_single_finite = all(math.isfinite(r.real) and math.isfinite(r.imag) for r in ae_mt_single_r)
        ae_mt_finite = all(math.isfinite(r.real) and math.isfinite(r.imag) for r in ae_mt_r)
        cm_finite = all(math.isfinite(r.real) and math.isfinite(r.imag) for r in cm_r)
        assert ae_finite, "%s: AE has non-finite roots" % label
        assert ae_mt_single_finite, "%s: AE-MT(1) has non-finite roots" % label
        assert ae_mt_finite, "%s: AE-MT has non-finite roots" % label
        assert cm_finite, "%s: CM has non-finite roots" % label

        if label == "all-zero":
            continue

        active_ae = [r for r in ae_r if abs(r) > 1e-10]
        active_ae_mt_single = [r for r in ae_mt_single_r if abs(r) > 1e-10]
        active_ae_mt = [r for r in ae_mt_r if abs(r) > 1e-10]
        active_cm = [r for r in cm_r if abs(r) > 1e-10]
        if active_ae:
            ae_resid = max(abs(polyval(cf, r)) for r in active_ae)
            assert ae_resid < 1e-2, "%s: AE max residual %.2e" % (label, ae_resid)
        if active_ae_mt_single:
            ae_mt_single_resid = max(abs(polyval(cf, r)) for r in active_ae_mt_single)
            assert ae_mt_single_resid < 1e-2, "%s: AE-MT(1) max residual %.2e" % (label, ae_mt_single_resid)
        if active_ae_mt:
            ae_mt_resid = max(abs(polyval(cf, r)) for r in active_ae_mt)
            assert ae_mt_resid < 1e-2, "%s: AE-MT max residual %.2e" % (label, ae_mt_resid)
        if active_cm:
            cm_resid = max(abs(polyval(cf, r)) for r in active_cm)
            assert cm_resid < 1e-2, "%s: CM max residual %.2e" % (label, cm_resid)

        print("  %s: AE OK, AE-MT(1) OK, AE-MT(4) OK, CM OK" % label)

    repeat_polys = [[1, -6, 11, -6] for _ in range(16)]
    write_cf("/tmp/repeat_cf.bin", repeat_polys, 4)
    r = subprocess.run(["/src/sweep", "/tmp/repeat_ae_out.bin"],
                       input=json.dumps({
                           "mode": "solve", "coeffs_file": "/tmp/repeat_cf.bin",
                           "n_coeffs": 4, "n2": len(repeat_polys),
                           "i1_start": 0, "i1_end": 1, "match_roots": False,
                       }),
                       capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, "repeat AE FAILED: " + r.stderr[:200]
    repeat_ae_meta = json.loads(r.stdout)
    r = subprocess.run(["/src/sweep_mt", "/tmp/repeat_mt_out.bin"],
                       input=json.dumps({
                           "mode": "solve_mt", "coeffs_file": "/tmp/repeat_cf.bin",
                           "n_coeffs": 4, "n2": len(repeat_polys),
                           "i1_start": 0, "i1_end": 1, "match_roots": False,
                           "n_threads": 4,
                       }),
                       capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, "repeat AE-MT FAILED: " + r.stderr[:200]
    repeat_mt_meta = json.loads(r.stdout)
    assert repeat_ae_meta["avg_iterations"] < repeat_mt_meta["avg_iterations"], (
        "AE warm-start chain should reduce avg_iterations relative to multi-thread cold starts at block boundaries"
    )

    print("=== AE/AE-MT/CM solver tests PASSED ===")


# ── CFPV Coeffgen Tests ──────────────────────────────────────────────────

def test_cfpv_coeffgen():
    print("\n--- CFPV coeffgen regression ---")

    m, err = run_coeffgen("creative9")
    assert m and m["n_coeffs"] == 71, "creative9 default: %s" % (err or m)
    print("  creative9 default (n=71): OK")

    m, err = run_coeffgen("creative9", [30])
    assert m and m["n_coeffs"] == 30, "creative9 cfpv=[30]: %s" % (err or m)
    print("  creative9 cfpv=[30] (n=30): OK")

    m, err = run_coeffgen("creative8")
    assert m and m["n_coeffs"] == 71, "creative8 default: %s" % (err or m)
    print("  creative8 default (n=71): OK")

    m, err = run_coeffgen("creative8", [40])
    assert m and m["n_coeffs"] == 40, "creative8 cfpv=[40]: %s" % (err or m)
    print("  creative8 cfpv=[40] (n=40): OK")

    m1, _ = run_coeffgen("g1")
    m2, _ = run_coeffgen("g1", [999])
    assert m1 and m2 and m1["n_coeffs"] == m2["n_coeffs"], "g1 cfpv should be ignored"
    print("  g1 unaffected by cfpv: OK")

    print("=== CFPV coeffgen tests PASSED ===")


def test_param_gen_threaded_runtime():
    print("\n--- Param-gen threaded runtime ---")

    single_path = "/tmp/param_gen_single.bin"
    mt_path = "/tmp/param_gen_mt.bin"
    spec_base = {
        "mode": "param_gen",
        "n1": 12,
        "n2": 12,
        "times": 3,
        "param_transforms": [["unit_circle"], ["square"]],
    }

    r = subprocess.run(
        ["/src/sweep_coeffgen", single_path],
        input=json.dumps({**spec_base, "n_threads": 1}),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "param_gen single-thread failed: " + r.stderr[:200]
    meta_single = json.loads(r.stdout)
    assert meta_single["threads"] == 1, "param_gen single-thread metadata missing threads=1"

    r = subprocess.run(
        ["/src/sweep_coeffgen", mt_path],
        input=json.dumps({**spec_base, "n_threads": 4}),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "param_gen multi-thread failed: " + r.stderr[:200]
    meta_mt = json.loads(r.stdout)
    assert meta_mt["threads"] == 4, "param_gen multi-thread metadata missing threads=4"

    with open(single_path, "rb") as f:
        single_bytes = f.read()
    with open(mt_path, "rb") as f:
        mt_bytes = f.read()

    assert single_bytes == mt_bytes, "param_gen threaded output diverged for deterministic transforms"
    assert len(single_bytes) == meta_single["data_bytes"], "param_gen single-thread byte count mismatch"
    assert len(mt_bytes) == meta_mt["data_bytes"], "param_gen multi-thread byte count mismatch"
    print("  sweep_coeffgen param_gen n_threads=1 vs 4: OK (%d bytes)" % len(mt_bytes))

    range_single_path = "/tmp/param_gen_range_single.bin"
    range_mt_path = "/tmp/param_gen_range_mt.bin"
    start, count = 17, 41
    r = subprocess.run(
        ["/src/sweep_coeffgen", range_single_path],
        input=json.dumps({**spec_base, "n_threads": 1, "step_start": start, "step_count": count}),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "param_gen range single-thread failed: " + r.stderr[:200]
    meta_range_single = json.loads(r.stdout)
    r = subprocess.run(
        ["/src/sweep_coeffgen", range_mt_path],
        input=json.dumps({**spec_base, "n_threads": 4, "step_start": start, "step_count": count}),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "param_gen range multi-thread failed: " + r.stderr[:200]
    meta_range_mt = json.loads(r.stdout)
    with open(range_single_path, "rb") as f:
        range_single_bytes = f.read()
    with open(range_mt_path, "rb") as f:
        range_mt_bytes = f.read()
    expected_range = single_bytes[start * 16:(start + count) * 16]
    assert meta_range_single["n_steps"] == count, "param_gen range metadata n_steps mismatch"
    assert meta_range_single["total_steps"] == meta_single["n_steps"], "param_gen range total_steps mismatch"
    assert meta_range_single["step_start"] == start, "param_gen range step_start mismatch"
    assert meta_range_single["step_count"] == count, "param_gen range step_count mismatch"
    assert meta_range_mt["threads"] == 4, "param_gen range threaded metadata missing threads=4"
    assert range_single_bytes == expected_range, "param_gen range single-thread output mismatch"
    assert range_mt_bytes == expected_range, "param_gen range multi-thread output mismatch"
    print("  sweep_coeffgen param_gen ranged chunks: OK (%d bytes)" % len(range_mt_bytes))

    cleanup(single_path, mt_path, range_single_path, range_mt_path)
    print("=== Param-gen threaded runtime PASSED ===")


def test_coeffgen_chunked_threaded_runtime():
    print("\n--- Coeffgen-chunked threaded runtime ---")

    params_path = "/tmp/coeffgen_params.bin"
    single_path = "/tmp/coeffgen_chunk_single.bin"
    mt_path = "/tmp/coeffgen_chunk_mt.bin"
    plain_path = "/tmp/coeffgen_chunk_plain.bin"
    andy_path = "/tmp/coeffgen_chunk_andy.bin"

    param_spec = {
        "mode": "param_gen",
        "n1": 10,
        "n2": 10,
        "times": 2,
        "param_transforms": [["unit_circle"]],
        "n_threads": 1,
    }
    r = subprocess.run(
        ["/src/sweep_coeffgen", params_path],
        input=json.dumps(param_spec),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "coeffgen runtime param_gen failed: " + r.stderr[:200]

    spec_base = {
        "mode": "coeffgen_chunked",
        "function": "g1",
        "coeff_transforms": [["exp", "0.3", "-0.2"]],
        "params_file": params_path,
        "step_start": 0,
        "step_count": 200,
    }

    r = subprocess.run(
        ["/src/sweep_coeffgen", single_path],
        input=json.dumps({**spec_base, "n_threads": 1}),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "coeffgen_chunked single-thread failed: " + r.stderr[:200]
    meta_single = json.loads(r.stdout)
    assert meta_single["threads"] == 1, "coeffgen_chunked single-thread metadata missing threads=1"

    r = subprocess.run(
        ["/src/sweep_coeffgen", mt_path],
        input=json.dumps({**spec_base, "n_threads": 4}),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "coeffgen_chunked multi-thread failed: " + r.stderr[:200]
    meta_mt = json.loads(r.stdout)
    assert meta_mt["threads"] == 4, "coeffgen_chunked multi-thread metadata missing threads=4"

    with open(single_path, "rb") as f:
        single_bytes = f.read()
    with open(mt_path, "rb") as f:
        mt_bytes = f.read()

    assert single_bytes == mt_bytes, "coeffgen_chunked threaded output diverged for deterministic inputs"
    assert len(single_bytes) == meta_single["data_bytes"], "coeffgen_chunked single-thread byte count mismatch"
    assert len(mt_bytes) == meta_mt["data_bytes"], "coeffgen_chunked multi-thread byte count mismatch"
    print("  sweep_coeffgen coeffgen_chunked n_threads=1 vs 4: OK (%d bytes)" % len(mt_bytes))

    plain_spec = {
        "mode": "coeffgen_chunked",
        "function": "g1",
        "coeff_transforms": [],
        "params_file": params_path,
        "step_start": 0,
        "step_count": 200,
        "n_threads": 1,
    }
    r = subprocess.run(
        ["/src/sweep_coeffgen", plain_path],
        input=json.dumps(plain_spec),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "coeffgen_chunked plain failed: " + r.stderr[:200]
    r = subprocess.run(
        ["/src/sweep_coeffgen", andy_path],
        input=json.dumps({**plain_spec, "coeff_transforms": [["scale100", "100", "0", "0", "0", "1e-5"]]}),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "coeffgen_chunked andy blend failed: " + r.stderr[:200]
    plain_vals = read_f32_array(plain_path)
    andy_vals = read_f32_array(andy_path)
    assert len(plain_vals) == len(andy_vals), "andy blend output length mismatch"
    factor = 100.0 * (1.0 - 1e-5) + 1e-5
    for idx, (base, got) in enumerate(zip(plain_vals, andy_vals)):
        expected = base * factor
        tol = max(2e-4, abs(expected) * 2e-5)
        assert abs(got - expected) <= tol, "andy blend mismatch at float %d: got %.9g expected %.9g" % (idx, got, expected)
    print("  coeff transform andy blend: OK (scale100 andy=1e-5)")

    cleanup(params_path, single_path, mt_path, plain_path, andy_path)
    print("=== Coeffgen-chunked threaded runtime PASSED ===")


def test_compute_preview_runtime_combo():
    print("\n--- Compute preview runtime combo ---")

    env = {**os.environ, "LD_LIBRARY_PATH": "/opt/lib", "PATH": "/opt/bin:" + os.environ.get("PATH", "")}
    coeff_path = "/tmp/compute_preview_coeffs.bin"
    roots_path = "/tmp/compute_preview_roots.bin"

    coeff_spec = {
        "mode": "coeffgen",
        "function": "g1",
        "n1": 8,
        "n2": 8,
        "i1_start": 0,
        "i1_end": 8,
        "times": 1,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [["roots_cm", "hi"]],
    }
    r = subprocess.run(["/src/sweep_coeffgen", coeff_path],
                       input=json.dumps(coeff_spec), capture_output=True, text=True, timeout=30, env=env)
    assert r.returncode == 0, "sweep_coeffgen preview combo failed: " + r.stderr[:200]
    coeff_meta = json.loads(r.stdout)
    assert coeff_meta["n_coeffs"] >= 2, "unexpected n_coeffs %r" % coeff_meta
    assert os.path.getsize(coeff_path) == coeff_meta["data_bytes"], "coeff preview combo size mismatch"
    print("  sweep_coeffgen + roots_cm: OK (%s bytes)" % coeff_meta["data_bytes"])

    solve_spec = {
        "mode": "solve_cm",
        "coeffs_file": coeff_path,
        "n_coeffs": coeff_meta["n_coeffs"],
        "n_steps": 64,
    }
    r = subprocess.run(["/src/sweep_cm", roots_path],
                       input=json.dumps(solve_spec), capture_output=True, text=True, timeout=30, env=env)
    assert r.returncode == 0, "sweep_cm preview combo failed: " + r.stderr[:200]
    solve_meta = json.loads(r.stdout)
    degree = solve_meta["degree"]
    assert degree == coeff_meta["degree"], "degree mismatch coeffgen=%s solve=%s" % (coeff_meta["degree"], degree)
    roots = read_roots(roots_path, degree)
    assert len(roots) == 64, "expected 64 polynomials, got %d" % len(roots)
    finite = all(math.isfinite(z.real) and math.isfinite(z.imag) for poly in roots for z in poly)
    assert finite, "compute preview combo produced non-finite roots"
    print("  sweep_cm on preview coeffs: OK (degree=%d, polys=%d)" % (degree, len(roots)))

    cleanup(coeff_path, roots_path)
    print("=== Compute preview runtime combo PASSED ===")


# ── Palette Chunk MT Runtime ─────────────────────────────────────────────

def test_palette_chunk_mt_runtime():
    print("\n--- solve_palette_chunk_mt runtime ---")

    bin_path = "/src/solve_palette_chunk_mt"
    assert os.path.exists(bin_path), "%s not found" % bin_path
    assert open(bin_path, "rb").read(4) == b"\x7fELF", "solve_palette_chunk_mt is not ELF"

    r = subprocess.run(["ldd", bin_path], capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "ldd failed for solve_palette_chunk_mt: " + r.stderr[:200]
    assert "not found" not in r.stdout, "solve_palette_chunk_mt shared libs unresolved: " + r.stdout
    print("  ldd: OK")

    roots_path = "/tmp/palette_chunk_mt_roots.bin"
    scores_path = "/tmp/palette_chunk_mt_scores.bin"
    bins_path = "/tmp/palette_chunk_mt_bins.bin"
    with open(roots_path, "wb") as f:
        solves = [
            [(0.0, 0.0), (1.0, 0.0)],
            [(0.0, 0.0), (0.5, 0.0)],
            [(0.0, 0.0), (0.2, 0.0)],
            [(0.0, 0.0), (0.8, 0.0)],
        ]
        for roots in solves:
            for re, im in roots:
                f.write(struct.pack("<ff", re, im))

    r = subprocess.run([
        bin_path, roots_path,
        "--degree=2",
        "--metric=proximity",
        "--clip_lo=0.0",
        "--clip_hi=2.0",
        "--cuts=0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9",
        "--omega=1.0",
        "--step_count=4",
        "--threads=3",
        "--input_mode=tmpfile",
        "--retries=2",
        "--scores_out=" + scores_path,
        "--bins_out=" + bins_path,
    ], capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "solve_palette_chunk_mt failed: " + r.stderr[:200]
    meta = json.loads(r.stdout)
    assert meta["threads"] == 3, "solve_palette_chunk_mt did not report thread count"
    assert meta["input_mode"] == "tmpfile", "solve_palette_chunk_mt did not report input_mode"
    assert meta["retries"] == 2, "solve_palette_chunk_mt did not report retries"
    assert meta["n_samples"] == 4, "solve_palette_chunk_mt did not report n_samples"

    scores = read_f32_array(scores_path)
    with open(bins_path, "rb") as f:
        bins = f.read()
    assert len(scores) == 4, "unexpected score count for solve_palette_chunk_mt"
    assert len(bins) == 4, "unexpected bin count for solve_palette_chunk_mt"
    assert all(math.isfinite(v) for v in scores), "solve_palette_chunk_mt produced non-finite scores"
    assert all(0 <= b <= 9 for b in bins), "solve_palette_chunk_mt produced invalid bins"
    print("  solve_palette_chunk_mt: OK (threads=%d, bytes=%d)" % (meta["threads"], meta["bytes_downloaded"]))

    cleanup(roots_path, scores_path, bins_path)
    print("=== solve_palette_chunk_mt runtime PASSED ===")


def test_palette_chunk_mt_param_sectioned_runtime():
    print("\n--- solve_palette_chunk_mt param-source sectioned runtime ---")

    bin_path = "/src/solve_palette_chunk_mt"
    roots_path = "/tmp/palette_chunk_mt_param_roots.bin"
    params_path = "/tmp/palette_chunk_mt_param_rows.bin"
    scores_path = "/tmp/palette_chunk_mt_param_scores.bin"
    bins_path = "/tmp/palette_chunk_mt_param_bins.bin"

    with open(roots_path, "wb") as f:
        solves = [
            [(0.0, 0.0), (1.0, 0.0)],
            [(0.0, 0.0), (1.0, 0.0)],
            [(0.0, 0.0), (1.0, 0.0)],
            [(0.0, 0.0), (1.0, 0.0)],
        ]
        for roots in solves:
            for re_val, im_val in roots:
                f.write(struct.pack("<ff", re_val, im_val))

    with open(params_path, "wb") as f:
        for row in [
            (0.10, 0.20, 0.0, 0.0),
            (0.25, 0.15, 0.0, 0.0),
            (0.40, 0.50, 0.0, 0.0),
            (0.75, 0.80, 0.0, 0.0),
        ]:
            f.write(struct.pack("<ffff", *row))

    _RangeHandler.file_bytes = open(roots_path, "rb").read()
    with socketserver.TCPServer(("127.0.0.1", 0), _RangeHandler) as httpd:
        port = httpd.server_address[1]
        thread = threading.Thread(target=httpd.serve_forever, daemon=True)
        thread.start()
        try:
            r = subprocess.run([
                bin_path, roots_path,
                "--degree=2",
                "--metric=t1_re",
                "--cuts=0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9",
                "--step_count=4",
                "--threads=2",
                "--input_mode=sectioned",
                "--url=http://127.0.0.1:%d/input.bin" % port,
                "--input_size=%d" % os.path.getsize(roots_path),
                "--retries=2",
                "--score_metrics=t1_re,t1_im",
                "--score_sources=pm,pm",
                "--score_clip_los=0,0",
                "--score_clip_his=1,1",
                "--score_program=m0;m1;mul",
                "--score_params_file=%s" % params_path,
                "--scores_out=" + scores_path,
                "--bins_out=" + bins_path,
            ], capture_output=True, text=True, timeout=10)
            assert r.returncode == 0, "solve_palette_chunk_mt param sectioned failed: " + r.stderr[:200]
            meta = json.loads(r.stdout)
            assert meta["threads"] == 2, "solve_palette_chunk_mt param sectioned did not report thread count"
            assert meta["input_mode"] == "sectioned", "solve_palette_chunk_mt param sectioned did not report input_mode"

            scores = read_f32_array(scores_path)
            with open(bins_path, "rb") as f:
                bins = list(f.read())

            expected_scores = [0.02, 0.0375, 0.2, 0.6]
            for got, want in zip(scores, expected_scores):
                assert abs(got - want) < 1e-6, "unexpected param-sectioned score %.6f != %.6f" % (got, want)
            assert len(set(bins)) >= 3, "param-sectioned bins unexpectedly collapsed: %r" % bins
            assert bins[0] == bins[1] == 0, "expected the two smallest param products to share the first bin, got %r" % bins
            assert bins[2] > bins[1], "expected the third param product to advance to a higher bin, got %r" % bins
            assert bins[3] > bins[2], "expected the largest param product to land in the highest bin of the set, got %r" % bins
            print("  solve_palette_chunk_mt param sectioned: OK (scores=%s)" % (",".join("%.4f" % s for s in scores)))
        finally:
            httpd.shutdown()
            thread.join(timeout=5)

    cleanup(roots_path, params_path, scores_path, bins_path)
    print("=== solve_palette_chunk_mt param-source sectioned runtime PASSED ===")


def test_roots2pix_mt_multispan_runtime():
    print("\n--- roots2pix_mt multispan runtime ---")

    bin_path = "/src/roots2pix_mt"
    assert os.path.exists(bin_path), "%s not found" % bin_path
    assert open(bin_path, "rb").read(4) == b"\x7fELF", "roots2pix_mt is not ELF"

    r = subprocess.run(["ldd", bin_path], capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "ldd failed for roots2pix_mt: " + r.stderr[:200]
    assert "not found" not in r.stdout, "roots2pix_mt shared libs unresolved: " + r.stdout
    print("  ldd: OK")

    roots_path = "/tmp/roots2pix_mt_roots.bin"
    manifest_path = "/tmp/roots2pix_mt_manifest.json"
    pixbin_prefix = "/tmp/roots2pix_mt_pixbin"
    cleanup(roots_path, manifest_path, pixbin_prefix + ".frag")

    roots_bytes = bytearray()
    for re_val, im_val in [(0.0, 0.0), (1.0, 0.0), (100.0, 100.0)]:
        roots_bytes.extend(struct.pack("<ff", re_val, im_val))
    with open(roots_path, "wb") as f:
        f.write(roots_bytes)

    _RangeHandler.file_bytes = bytes(roots_bytes)
    with socketserver.TCPServer(("127.0.0.1", 0), _RangeHandler) as httpd:
        port = httpd.server_address[1]
        thread = threading.Thread(target=httpd.serve_forever, daemon=True)
        thread.start()
        try:
            with open(manifest_path, "w", encoding="utf-8") as f:
                json.dump({
                    "source_family": "slv",
                    "logical_size": len(roots_bytes),
                    "row_bytes": 8,
                    "solve_start": 0,
                    "solve_count": 3,
                    "sources": [{
                        "id": 0,
                        "url": "http://127.0.0.1:%d/input.bin" % port,
                        "key": "input.bin",
                    }],
                    "spans": [{
                        "source_id": 0,
                        "logical_byte_start": 0,
                        "byte_start": 0,
                        "byte_length": len(roots_bytes),
                    }],
                }, f)

            r = subprocess.run([
                bin_path,
                "/tmp/roots2pix_mt_pix",
                "--pix=8",
                "--tile_size=8",
                "--n_tile_cols=1",
                "--n_tile_rows=1",
                "--min_re=-2",
                "--max_re=2",
                "--min_im=-2",
                "--max_im=2",
                "--degree=1",
                "--rotation=0",
                "--threads=2",
                "--input_manifest=" + manifest_path,
                "--retries=2",
                "--score_metrics=centroid_re",
                "--score_clip_los=-1",
                "--score_clip_his=1",
                "--score_program=m0",
                "--pixel_bin_prefix=" + pixbin_prefix,
            ], capture_output=True, text=True, timeout=10)
            assert r.returncode == 0, "roots2pix_mt failed: " + r.stderr[:200]
            meta = json.loads(r.stdout)
            assert meta["threads"] == 2, "roots2pix_mt did not report thread count"
            assert meta["input_mode"] == "multispan_sectioned", "roots2pix_mt did not report input_mode"
            assert meta["roots_plotted"] == 2, "roots2pix_mt plotted unexpected solve count"
            assert meta["roots_clipped"] == 1, "roots2pix_mt clipped unexpected solve count"

            expected = encode_fragment_pairs([(36, 128), (38, 255)])
            with open(pixbin_prefix + ".frag", "rb") as f:
                assert f.read() == expected, "roots2pix_mt fragment payload mismatch"
            print("  roots2pix_mt: OK (threads=%d, roots_plotted=%d)" % (meta["threads"], meta["roots_plotted"]))
        finally:
            httpd.shutdown()
            thread.join(timeout=5)

    cleanup(roots_path, manifest_path, pixbin_prefix + ".frag")
    print("=== roots2pix_mt multispan runtime PASSED ===")


# ── Render Preview (vipsthumbnail) Tests ─────────────────────────────────

def test_render_preview():
    print("\n--- Render preview (vipsthumbnail) ---")

    vt_path = "/opt/bin/vipsthumbnail"
    assert os.path.exists(vt_path), "vipsthumbnail not found at %s" % vt_path
    r = subprocess.run([vt_path, "--vips-version"], capture_output=True, text=True, timeout=5)
    print("  vipsthumbnail: %s" % (r.stdout.strip() or r.stderr.strip()))

    test_raw = "/tmp/preview_test.raw"
    test_jpeg = "/tmp/preview_test.jpeg"
    test_png = "/tmp/preview_test_thumb.png"

    with open(test_raw, "wb") as f:
        f.write(struct.pack("<III", 8, 8, 3))
        for _ in range(8 * 8):
            f.write(bytes([100, 150, 200]))

    env = {**os.environ, "LD_LIBRARY_PATH": "/opt/lib", "PATH": "/opt/bin:" + os.environ.get("PATH", "")}

    r = subprocess.run(["/src/raw2jpeg", test_raw, test_jpeg, "--quality=90"],
                       capture_output=True, text=True, timeout=10, env=env)
    assert r.returncode == 0, "raw2jpeg failed: " + r.stderr[:200]
    jpeg_size = os.path.getsize(test_jpeg)
    assert jpeg_size > 0, "test JPEG is empty"
    print("  test JPEG: %d bytes" % jpeg_size)

    r = subprocess.run([vt_path, test_jpeg, "-s", "4x4", "-o", test_png + "[strip]"],
                       capture_output=True, text=True, timeout=10, env=env)
    assert r.returncode == 0, "vipsthumbnail failed: " + r.stderr[:200]
    png_size = os.path.getsize(test_png)
    assert png_size > 0, "thumbnail PNG is empty"
    print("  thumbnail PNG: %d bytes" % png_size)

    test_tif = "/tmp/preview_test.tif"
    r = subprocess.run(["/opt/bin/vips", "im_copy", test_jpeg, test_tif],
                       capture_output=True, text=True, timeout=10, env=env)
    if r.returncode == 0:
        test_tif_thumb = "/tmp/preview_test_tif_thumb.png"
        r = subprocess.run([vt_path, test_tif, "-s", "4x4", "-o", test_tif_thumb + "[strip]"],
                           capture_output=True, text=True, timeout=10, env=env)
        assert r.returncode == 0, "vipsthumbnail TIFF failed: " + r.stderr[:200]
        tif_png_size = os.path.getsize(test_tif_thumb)
        assert tif_png_size > 0, "TIFF thumbnail is empty"
        print("  TIFF thumbnail PNG: %d bytes" % tif_png_size)
    else:
        print("  SKIP TIFF test: vips im_copy not available")

    cleanup(test_raw, test_jpeg, test_png, test_tif, "/tmp/preview_test_tif_thumb.png")
    print("=== Render preview tests PASSED ===")


# ── Resize Artifact libvips Runtime ──────────────────────────────────────

def test_resize_runtime():
    print("\n--- Resize artifact libvips runtime ---")

    vips_path = "/opt/bin/vips"
    vt_path = "/opt/bin/vipsthumbnail"
    assert os.path.exists(vips_path), "vips not found at %s" % vips_path
    assert os.path.exists(vt_path), "vipsthumbnail not found at %s" % vt_path

    r = subprocess.run([vips_path, "--vips-version"], capture_output=True, text=True, timeout=5)
    assert r.returncode == 0, "vips --vips-version failed: " + r.stderr[:200]
    print("  vips: %s" % (r.stdout.strip() or r.stderr.strip()))

    r = subprocess.run([vt_path, "--vips-version"], capture_output=True, text=True, timeout=5)
    assert r.returncode == 0, "vipsthumbnail --vips-version failed: " + r.stderr[:200]
    print("  vipsthumbnail: %s" % (r.stdout.strip() or r.stderr.strip()))

    env = {**os.environ, "LD_LIBRARY_PATH": "/opt/lib", "PATH": "/opt/bin:" + os.environ.get("PATH", "")}
    test_raw = "/tmp/resize_rt.raw"
    test_jpeg = "/tmp/resize_rt.jpeg"
    thumb_png = "/tmp/resize_rt_thumb.png"
    resize_png = "/tmp/resize_rt_resized.png"

    with open(test_raw, "wb") as f:
        f.write(struct.pack("<III", 64, 64, 3))
        for y in range(64):
            for x in range(64):
                f.write(bytes([(x * 4) % 256, (y * 8) % 256, ((x + y) * 3) % 256]))

    r = subprocess.run(["/src/raw2jpeg", test_raw, test_jpeg, "--quality=90"],
                       capture_output=True, text=True, timeout=10, env=env)
    assert r.returncode == 0, "raw2jpeg failed: " + r.stderr[:200]
    assert os.path.getsize(test_jpeg) > 0, "runtime resize test JPEG is empty"
    jpeg_dims = read_jpeg_dims(test_jpeg)
    assert jpeg_dims == (64, 64), "unexpected source JPEG dims %r" % (jpeg_dims,)
    print("  source JPEG: %dx%d" % jpeg_dims)

    r = subprocess.run([
        vips_path, "thumbnail", test_jpeg, thumb_png, "16",
        "--height", "16", "--size", "down", "--linear",
        "--intent", "perceptual", "--fail-on", "none"
    ], capture_output=True, text=True, timeout=20, env=env)
    assert r.returncode == 0, "vips thumbnail failed: " + r.stderr[:200]
    thumb_dims = read_png_dims(thumb_png)
    assert thumb_dims == (16, 16), "unexpected thumbnail dims %r" % (thumb_dims,)
    print("  vips thumbnail: OK (%dx%d)" % thumb_dims)

    r = subprocess.run([
        vips_path, "resize", test_jpeg, resize_png,
        str(32.0 / 64.0),
        "--kernel", "mitchell",
        "--gap", "2"
    ], capture_output=True, text=True, timeout=20, env=env)
    assert r.returncode == 0, "vips resize failed: " + r.stderr[:200]
    resize_dims = read_png_dims(resize_png)
    assert resize_dims == (32, 32), "unexpected resized dims %r" % (resize_dims,)
    print("  vips resize: OK (%dx%d)" % resize_dims)

    cleanup(test_raw, test_jpeg, thumb_png, resize_png)
    print("=== Resize artifact runtime tests PASSED ===")


def test_bilevel_section_raster_runtime():
    print("\n--- bilevel_section_raster runtime ---")

    bin_path = "/src/bilevel_section_raster"
    assert os.path.exists(bin_path), "bilevel_section_raster not found at %s" % bin_path

    section_path = "/tmp/bilevel_section_rt.bin"
    out_frag = "/tmp/bilevel_section_rt.frag"

    with open(section_path, "wb") as f:
        for roots in [[(0.0, 0.0), (1.0, 0.0)], [(0.0, 0.0), (100.0, 100.0)], [(-4.25, 0.0), (0.0, 0.0)]]:
            for re, im in roots:
                f.write(struct.pack("<ff", re, im))

    r = subprocess.run(
        [
            bin_path,
            section_path,
            out_frag,
            "--pix=8",
            "--min_re=-4",
            "--max_re=4",
            "--min_im=-4",
            "--max_im=4",
            "--degree=2",
        ],
        capture_output=True,
        text=True,
        timeout=10,
    )
    assert r.returncode == 0, "bilevel_section_raster failed: " + r.stderr[:200]
    meta = json.loads(r.stdout)
    assert meta["roots_plotted"] == 2, "unexpected plotted count %r" % (meta,)
    assert meta["roots_clipped"] == 2, "unexpected clipped count %r" % (meta,)
    assert meta["roots_deduped"] == 2, "unexpected dedup count %r" % (meta,)

    with open(out_frag, "rb") as f:
        frag = f.read()
    assert frag == encode_fragment_pairs([(36, 1), (37, 1)]), "unexpected fragment bytes %r" % (frag,)
    print(
        "  bilevel_section_raster: OK (plotted=%d, clipped=%d, dedup=%d)"
        % (meta["roots_plotted"], meta["roots_clipped"], meta["roots_deduped"])
    )

    cleanup(section_path, out_frag)
    print("=== bilevel_section_raster runtime PASSED ===")


def test_bilevel_merge_assemble_runtime():
    print("\n--- bilevel_merge assemble runtime ---")

    bin_path = "/src/bilevel_merge"
    assert os.path.exists(bin_path), "bilevel_merge not found at %s" % bin_path

    env = {**os.environ, "LD_LIBRARY_PATH": "/opt/lib", "PATH": "/opt/bin:" + os.environ.get("PATH", "")}
    ldd = subprocess.run(["ldd", bin_path], capture_output=True, text=True, timeout=10, env=env)
    assert ldd.returncode == 0, "ldd bilevel_merge failed: " + ldd.stderr[:200]
    assert "not found" not in (ldd.stdout + ldd.stderr), "bilevel_merge has missing shared libs:\n%s" % (ldd.stdout + ldd.stderr)

    bits_a = "/tmp/bilevel_assemble_a.bits"
    bits_b = "/tmp/bilevel_assemble_b.bits"
    out_tif = "/tmp/bilevel_assemble.tif"
    preview_png = "/tmp/bilevel_assemble_preview.png"
    out_csv = "/tmp/bilevel_assemble.csv"

    with open(bits_a, "wb") as f:
        f.write(bytes([0x01, 0, 0, 0, 0, 0, 0, 0]))
    with open(bits_b, "wb") as f:
        f.write(bytes([0, 0, 0, 0, 0, 0, 0, 0x80]))

    r = subprocess.run(
        [
            bin_path,
            "assemble",
            "--pix=8",
            "--output=" + out_tif,
            "--preview=" + preview_png,
            "--preview_size=16",
            bits_a,
            bits_b,
        ],
        capture_output=True,
        text=True,
        timeout=20,
        env=env,
    )
    assert r.returncode == 0, "bilevel_merge assemble failed: " + r.stderr[:200]
    meta = json.loads(r.stdout)
    assert meta["sections"] == 2, "unexpected assemble section count %r" % (meta,)
    assert meta["pixels_set"] == 2, "unexpected assemble pixels_set %r" % (meta,)
    assert os.path.getsize(out_tif) > 0, "assemble TIFF is empty"
    assert os.path.getsize(preview_png) > 0, "assemble preview PNG is empty"
    assert read_png_dims(preview_png) == (8, 8), "unexpected assemble preview dims %r" % (read_png_dims(preview_png),)

    r = subprocess.run(
        ["/opt/bin/vips", "csvsave", out_tif, out_csv],
        capture_output=True,
        text=True,
        timeout=20,
        env=env,
    )
    assert r.returncode == 0, "vips csvsave assemble TIFF failed: " + r.stderr[:200]
    grid = read_csv_grid(out_csv)
    assert len(grid) == 8 and all(len(row) == 8 for row in grid), "unexpected assemble CSV shape"
    assert grid[0][0] > 0, "expected pixel (0,0) to be set"
    assert grid[7][7] > 0, "expected pixel (7,7) to be set"
    assert grid[0][7] == 0, "expected pixel (7,0) to stay clear"
    assert grid[7][0] == 0, "expected pixel (0,7) to stay clear"
    print("  bilevel_merge assemble: OK (pixels_set=%d)" % meta["pixels_set"])

    cleanup(bits_a, bits_b, out_tif, preview_png, out_csv)
    print("=== bilevel_merge assemble runtime PASSED ===")


def test_raw_to_bilevel_runtime():
    print("\n--- raw_to_bilevel runtime ---")

    bin_path = "/src/raw_to_bilevel"
    assert os.path.exists(bin_path), "raw_to_bilevel not found at %s" % bin_path

    env = {**os.environ, "LD_LIBRARY_PATH": "/opt/lib", "PATH": "/opt/bin:" + os.environ.get("PATH", "")}
    ldd = subprocess.run(["ldd", bin_path], capture_output=True, text=True, timeout=10, env=env)
    assert ldd.returncode == 0, "ldd raw_to_bilevel failed: " + ldd.stderr[:200]
    assert "not found" not in (ldd.stdout + ldd.stderr), "raw_to_bilevel has missing shared libs:\n%s" % (ldd.stdout + ldd.stderr)

    raw_path = "/tmp/raw_to_bilevel_rt.raw"
    out_tif = "/tmp/raw_to_bilevel_rt.tif"
    preview_png = "/tmp/raw_to_bilevel_rt_preview.png"
    out_csv = "/tmp/raw_to_bilevel_rt.csv"

    with open(raw_path, "wb") as f:
        f.write(bytes([0, 1, 128, 255, 10, 127, 129, 200, 0, 0, 255, 1, 128, 129, 127, 126]))

    r = subprocess.run(
        [
            bin_path,
            raw_path,
            out_tif,
            "--pix=4",
            "--threshold=127",
            "--preview=" + preview_png,
            "--preview_size=16",
        ],
        capture_output=True,
        text=True,
        timeout=20,
        env=env,
    )
    assert r.returncode == 0, "raw_to_bilevel failed: " + r.stderr[:200]
    meta = json.loads(r.stdout)
    assert meta["threshold"] == 127, "unexpected threshold metadata %r" % (meta,)
    assert meta["width"] == 4 and meta["height"] == 4, "unexpected image size metadata %r" % (meta,)
    assert os.path.getsize(out_tif) > 0, "raw_to_bilevel TIFF is empty"
    assert os.path.getsize(preview_png) > 0, "raw_to_bilevel preview PNG is empty"
    assert read_png_dims(preview_png) == (4, 4), "unexpected preview dims %r" % (read_png_dims(preview_png),)

    r = subprocess.run(
        ["/opt/bin/vips", "csvsave", out_tif, out_csv],
        capture_output=True,
        text=True,
        timeout=20,
        env=env,
    )
    assert r.returncode == 0, "vips csvsave raw_to_bilevel TIFF failed: " + r.stderr[:200]
    grid = read_csv_grid(out_csv)
    expected = [[0, 0, 1, 1], [0, 0, 1, 1], [0, 0, 1, 0], [1, 1, 0, 0]]
    assert len(grid) == 4 and all(len(row) == 4 for row in grid), "unexpected raw_to_bilevel CSV shape"
    for y, row in enumerate(grid):
        for x, value in enumerate(row):
            assert (value > 0) == bool(expected[y][x]), "unexpected thresholded pixel (%d,%d)=%.3f" % (x, y, value)
    print("  raw_to_bilevel: OK (threshold=%d)" % meta["threshold"])

    cleanup(raw_path, out_tif, preview_png, out_csv)
    print("=== raw_to_bilevel runtime PASSED ===")


def test_color_to_bilevel_handler_runtime():
    print("\n--- Color2Bilevel handler runtime ---")

    env = {**os.environ, "LD_LIBRARY_PATH": "/src/solve_palette_chunk_mt_lib:/opt/lib", "PATH": "/opt/bin:" + os.environ.get("PATH", "")}

    if "boto3" not in sys.modules:
        sys.modules["boto3"] = types.SimpleNamespace(client=lambda *_args, **_kwargs: object())
    if "botocore" not in sys.modules:
        sys.modules["botocore"] = types.ModuleType("botocore")
    if "botocore.exceptions" not in sys.modules:
        exc_mod = types.ModuleType("botocore.exceptions")
        exc_mod.ClientError = RuntimeError
        sys.modules["botocore.exceptions"] = exc_mod
    if "botocore.config" not in sys.modules:
        cfg_mod = types.ModuleType("botocore.config")
        class _Config:
            def __init__(self, *args, **kwargs):
                self.args = args
                self.kwargs = kwargs
        cfg_mod.Config = _Config
        sys.modules["botocore.config"] = cfg_mod

    if "/src" not in sys.path:
        sys.path.insert(0, "/src")

    import handler_bilevel as mod
    from raw_sidecar import build_raw_sidecar

    fake_s3 = _MemS3()
    status_rows = []

    def fake_report(job_id, task_id, status, error_msg=None, result_data=None):
        status_rows.append({
            "job_id": job_id,
            "task_id": task_id,
            "status": status,
            "error_msg": error_msg,
            "result_data": dict(result_data or {}),
        })

    raw_key = "renders/job-color2bil/color/src_color/greyscale.raw"
    raw_meta_key = "renders/job-color2bil/color/src_color/greyscale.meta.json"
    image_key = "renders/job-color2bil/color/src_color/image.jpeg"
    raw_bytes = bytes([0, 1, 128, 255, 10, 127, 129, 200, 0, 0, 255, 1, 128, 129, 127, 126])
    raw_sidecar = build_raw_sidecar(
        job_id="job-color2bil",
        run_id="run-color2bil",
        artifact_family="color",
        artifact_id="src_color",
        width=4,
        height=4,
        chain_fingerprint="fp_color2bil",
        score_chain=[],
        score_program="m0",
        clip_slots=[{"slot": 0, "metric": "proximity", "source": "slv", "clip_lo": 0.0, "clip_hi": 1.0}],
        background_color=[0, 0, 0],
        plan_params_digest="digest_color2bil",
        render_execution={},
        raw_key=raw_key,
        image_key=image_key,
        preview_key="renders/job-color2bil/color/src_color/preview.png",
        meta_key=raw_meta_key,
        created_at="2026-04-20T00:00:00Z",
        histogram=[0] * 256,
    )
    fake_s3.seed_object(
        image_key,
        b"\xff\xd8\xff\xd9",
        content_type="image/jpeg",
        metadata={
            "artifact_id": "src_color",
            "family": "color",
            "format": "jpeg",
            "pix": "4",
            "raw_key": raw_key,
            "raw_meta_key": raw_meta_key,
        },
    )
    fake_s3.seed_object(raw_key, raw_bytes, content_type="application/octet-stream")
    fake_s3.seed_object(raw_meta_key, json.dumps(raw_sidecar).encode("utf-8"), content_type="application/json")

    old_s3 = mod.s3
    old_report = mod.report_status
    old_interval = mod.RAW_TO_BILEVEL_PROGRESS_INTERVAL_S
    try:
        mod.s3 = fake_s3
        mod.report_status = fake_report
        mod.RAW_TO_BILEVEL_PROGRESS_INTERVAL_S = 0.001

        result = mod.handler({
            "body": json.dumps({
                "phase": "from_raw_color",
                "job_id": "job-color2bil",
                "task_id": "color_to_bilevel_runtime",
                "artifact_id": "bil_1",
                "source_artifact_id": "src_color",
                "threshold": 127,
            })
        }, None)
    finally:
        mod.s3 = old_s3
        mod.report_status = old_report
        mod.RAW_TO_BILEVEL_PROGRESS_INTERVAL_S = old_interval

    body = json.loads(result["body"])
    assert body["artifact_id"] == "bil_1", "unexpected artifact_id %r" % (body,)
    assert body["threshold"] == 127, "unexpected threshold %r" % (body,)

    bilevel_key = "renders/job-color2bil/bilevel/bil_1/image.tif"
    preview_key = "renders/job-color2bil/bilevel/bil_1/preview.png"
    assert bilevel_key in fake_s3.objects, "bilevel TIFF not uploaded"
    assert preview_key in fake_s3.objects, "preview PNG not uploaded"
    tif_obj = fake_s3.objects[bilevel_key]
    preview_obj = fake_s3.objects[preview_key]
    assert tif_obj["Metadata"]["postprocess_kind"] == "color_to_bilevel", "missing Color2Bilevel metadata"
    assert tif_obj["Metadata"]["threshold"] == "127", "threshold metadata not preserved"
    assert tif_obj["Metadata"]["derived_from_artifact_id"] == "src_color", "source artifact metadata missing"

    tif_path = "/tmp/color2bilevel_handler_rt.tif"
    png_path = "/tmp/color2bilevel_handler_rt_preview.png"
    csv_path = "/tmp/color2bilevel_handler_rt.csv"
    with open(tif_path, "wb") as fh:
        fh.write(tif_obj["Body"])
    with open(png_path, "wb") as fh:
        fh.write(preview_obj["Body"])

    r = subprocess.run(
        ["/opt/bin/vips", "csvsave", tif_path, csv_path],
        capture_output=True,
        text=True,
        timeout=20,
        env=env,
    )
    assert r.returncode == 0, "vips csvsave Color2Bilevel TIFF failed: " + r.stderr[:200]
    grid = read_csv_grid(csv_path)
    expected = [
        [0, 0, 1, 1],
        [0, 0, 1, 1],
        [0, 0, 1, 0],
        [1, 1, 0, 0],
    ]
    assert len(grid) == 4 and all(len(row) == 4 for row in grid), "unexpected Color2Bilevel TIFF shape"
    for y, row in enumerate(grid):
        for x, value in enumerate(row):
            assert (value > 0) == bool(expected[y][x]), "unexpected Color2Bilevel pixel (%d,%d)=%.3f" % (x, y, value)
    assert read_png_dims(png_path) == (4, 4), "unexpected Color2Bilevel preview dims %r" % (read_png_dims(png_path),)

    statuses = [row["status"] for row in status_rows]
    for expected_status in ("started", "source_ready", "raw_downloaded", "rendered", "uploading", "done"):
        assert expected_status in statuses, "missing status %s in %r" % (expected_status, statuses)
    phase_labels = [row["result_data"].get("phase_label") for row in status_rows if row["result_data"]]
    for label in (
        "Color2Bilevel: source",
        "Color2Bilevel: raw download",
        "Color2Bilevel: threshold + encode",
        "Color2Bilevel: upload",
    ):
        assert label in phase_labels, "missing phase label %r in %r" % (label, phase_labels)

    cleanup(tif_path, png_path, csv_path)
    print("  handler_bilevel from_raw_color: OK (threshold=%d)" % body["threshold"])
    print("=== Color2Bilevel handler runtime PASSED ===")


def test_bilevel_handler_sparse_finalize_runtime():
    print("\n--- BiLevel handler sparse finalize runtime ---")

    env = {
        **os.environ,
        "LD_LIBRARY_PATH": "/src/assemble_greyscale_lib:/src/solve_palette_chunk_mt_lib:/opt/lib",
        "PATH": "/opt/bin:" + os.environ.get("PATH", ""),
    }

    if "boto3" not in sys.modules:
        sys.modules["boto3"] = types.SimpleNamespace(client=lambda *_args, **_kwargs: object())
    if "botocore" not in sys.modules:
        sys.modules["botocore"] = types.ModuleType("botocore")
    if "botocore.exceptions" not in sys.modules:
        exc_mod = types.ModuleType("botocore.exceptions")
        exc_mod.ClientError = RuntimeError
        sys.modules["botocore.exceptions"] = exc_mod

    if "/src" not in sys.path:
        sys.path.insert(0, "/src")

    import handler_bilevel as mod
    from logical_sections import build_solve_source_manifest

    fake_s3 = _MemS3()
    status_rows = []

    def fake_report(job_id, task_id, status, error_msg=None, result_data=None):
        status_rows.append({
            "job_id": job_id,
            "task_id": task_id,
            "status": status,
            "error_msg": error_msg,
            "result_data": dict(result_data or {}),
        })

    roots_key = "renders/job-bilevel/roots.bin"
    roots_bytes = bytearray()
    for roots in [[(0.0, 0.0), (1.0, 0.0)], [(0.0, 0.0), (100.0, 100.0)]]:
        for re, im in roots:
            roots_bytes.extend(struct.pack("<ff", re, im))
    fake_s3.seed_object(roots_key, roots_bytes, content_type="application/octet-stream")

    chunk_items = [{
        "chunk_idx": 0,
        "step_start": 0,
        "step_count": 2,
        "bin_key": roots_key,
        "bin_size": len(roots_bytes),
    }]
    solve_source_manifest = build_solve_source_manifest(
        chunk_items,
        job_id="job-bilevel",
        degree=2,
        n_coeffs=3,
        include_coeff=False,
        include_param=False,
    )

    old_s3 = mod.s3
    old_report = mod.report_status
    old_finalize_s3_client = mod._finalize_s3_client
    old_env = os.environ.get("LD_LIBRARY_PATH")
    server = None
    server_thread = None
    try:
        mod.s3 = fake_s3
        mod.report_status = fake_report
        os.environ["LD_LIBRARY_PATH"] = env["LD_LIBRARY_PATH"]

        handler_cls = type("_LocalMemS3Handler", (_MemS3ObjectHandler,), {})
        handler_cls.objects = fake_s3.objects
        server = socketserver.TCPServer(("127.0.0.1", 0), handler_cls)
        server_thread = threading.Thread(target=server.serve_forever, daemon=True)
        server_thread.start()
        port = server.server_address[1]

        class _FinalizeS3:
            def generate_presigned_url(self, op, Params=None, ExpiresIn=None):
                key = urllib.parse.quote(str((Params or {}).get("Key") or ""), safe="/")
                return f"http://127.0.0.1:{port}/{key}"

        mod._finalize_s3_client = lambda workers: _FinalizeS3()

        for section_idx, step_start in enumerate((0, 1)):
            result = mod.handle_section_raster({
                "job_id": "job-bilevel",
                "task_id": f"bilevel_section_{section_idx}",
                "section_idx": section_idx,
                "section_count": 2,
                "step_start": step_start,
                "step_count": 1,
                "solve_source_manifest": solve_source_manifest,
                "fragment_prefix": "renders/job-bilevel/bilevel_section_",
                "pix": 8,
                "min_re": -4,
                "max_re": 4,
                "min_im": -4,
                "max_im": 4,
                "degree": 2,
                "rotation": 0,
                "root_transforms": [],
            })
            body = json.loads(result["body"])
            assert body["fragment_key"] == f"renders/job-bilevel/bilevel_section_{section_idx:04d}.frag"

        result = mod.handle_finalize({
            "job_id": "job-bilevel",
            "task_id": "bilevel_finalize",
            "pix": 8,
            "source_item_count": 2,
            "fragment_prefix": "renders/job-bilevel/bilevel_section_",
            "out_key": "renders/job-bilevel/bilevel/art/image.tif",
            "preview_key": "renders/job-bilevel/bilevel/art/preview.png",
            "metadata": {
                "artifact_id": "art",
                "created_at": "2026-04-20T00:00:00Z",
                "pix": "8",
                "render_execution": {"raster_section_mode": "logical_sections_auto"},
                "bilevel_section_mode": "logical_sections_auto",
                "bilevel_section_count": "2",
            },
        })
    finally:
        mod.s3 = old_s3
        mod.report_status = old_report
        mod._finalize_s3_client = old_finalize_s3_client
        if server is not None:
            server.shutdown()
            server.server_close()
        if server_thread is not None:
            server_thread.join(timeout=1.0)
        if old_env is None:
            os.environ.pop("LD_LIBRARY_PATH", None)
        else:
            os.environ["LD_LIBRARY_PATH"] = old_env

    body = json.loads(result["body"])
    assert body["pixels_set"] == 2, "unexpected bilevel finalize result %r" % (body,)

    bilevel_key = "renders/job-bilevel/bilevel/art/image.tif"
    preview_key = "renders/job-bilevel/bilevel/art/preview.png"
    assert bilevel_key in fake_s3.objects, "bilevel TIFF not uploaded"
    assert preview_key in fake_s3.objects, "bilevel preview not uploaded"
    tif_obj = fake_s3.objects[bilevel_key]
    assert tif_obj["Metadata"]["bilevel_pipeline"] == "logical_sections_sparse_fragments_v1"

    tif_path = "/tmp/bilevel_handler_sparse_rt.tif"
    png_path = "/tmp/bilevel_handler_sparse_rt_preview.png"
    csv_path = "/tmp/bilevel_handler_sparse_rt.csv"
    with open(tif_path, "wb") as fh:
        fh.write(tif_obj["Body"])
    with open(png_path, "wb") as fh:
        fh.write(fake_s3.objects[preview_key]["Body"])

    r = subprocess.run(
        ["/opt/bin/vips", "csvsave", tif_path, csv_path],
        capture_output=True,
        text=True,
        timeout=20,
        env=env,
    )
    assert r.returncode == 0, "vips csvsave bilevel TIFF failed: " + r.stderr[:200]
    grid = read_csv_grid(csv_path)
    assert len(grid) == 8 and all(len(row) == 8 for row in grid), "unexpected bilevel TIFF shape"
    assert grid[4][4] > 0, "expected center pixel to be set"
    assert grid[4][5] > 0, "expected adjacent pixel to be set"
    assert grid[0][0] == 0, "unexpected corner pixel set"
    assert read_png_dims(png_path) == (8, 8), "unexpected bilevel preview dims %r" % (read_png_dims(png_path),)

    cleanup(tif_path, png_path, csv_path)
    print("  handler_bilevel sparse finalize: OK (pixels_set=%d)" % body["pixels_set"])
    print("=== BiLevel handler sparse finalize runtime PASSED ===")


# ── solve_proximity_stats Tests (all metrics) ────────────────────────────

def _write_sps_bin(path):
    """Write 3-solve degree-2 test fixture for solve_proximity_stats."""
    with open(path, "wb") as f:
        for roots in [[(0, 0), (1, 0)], [(0, 0), (0.01, 0)], [(0, 0), (0.1, 0)]]:
            for re, im in roots:
                f.write(struct.pack("<ff", re, im))


def test_solve_proximity_stats():
    print("\n--- solve_proximity_stats (multi-metric) ---")

    sps_path = "/src/solve_proximity_stats"
    if not os.path.exists(sps_path):
        print("  SKIP: %s not found (not yet compiled)" % sps_path)
        return

    magic = open(sps_path, "rb").read(4)
    assert magic == b"\x7fELF", "solve_proximity_stats is not ELF"
    print("  %s: ELF OK" % sps_path)

    sps_bin = "/tmp/sps_test.bin"
    _write_sps_bin(sps_bin)

    # 1. Proximity clip
    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2", "--metric=proximity"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "proximity clip failed: " + r.stderr[:200]
    clip = json.loads(r.stdout)
    assert clip["n_solves"] == 3
    assert clip["metric"] == "proximity"
    assert clip["clip_lo"] <= clip["clip_hi"]
    print("  proximity clip: OK (n=%d, lo=%.2f, hi=%.2f)" % (clip["n_solves"], clip["clip_lo"], clip["clip_hi"]))

    # Hist mode
    r = subprocess.run([sps_path, sps_bin, "--mode=hist", "--degree=2", "--metric=proximity",
                        "--clip_lo=" + str(clip["clip_lo"]),
                        "--clip_hi=" + str(clip["clip_hi"]),
                        "--hist_bins=10"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "proximity hist failed: " + r.stderr[:200]
    hist = json.loads(r.stdout)
    assert len(hist["hist"]) == 10
    assert sum(hist["hist"]) == 3
    print("  proximity hist: OK (bins=%d, total=%d)" % (len(hist["hist"]), sum(hist["hist"])))

    # 2. Crowding clip
    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2", "--metric=crowding"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "crowding clip failed: " + r.stderr[:200]
    crowd = json.loads(r.stdout)
    assert crowd["metric"] == "crowding"
    assert crowd["n_solves"] == 3
    print("  crowding clip: OK (lo=%.2f, hi=%.2f)" % (crowd["clip_lo"], crowd["clip_hi"]))

    # 3. Spread clip
    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2", "--metric=spread"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "spread clip failed: " + r.stderr[:200]
    spread = json.loads(r.stdout)
    assert spread["metric"] == "spread"
    assert spread["n_solves"] == 3
    print("  spread clip: OK (lo=%.2f, hi=%.2f)" % (spread["clip_lo"], spread["clip_hi"]))

    # 3b. Score program ops, including m* binary ops and unary post-process ops, must parse and run.
    for program in ["m0;m1;mul", "m0;m1;max"]:
        r = subprocess.run(
            [
                sps_path,
                sps_bin,
                "--mode=summary",
                "--degree=2",
                "--score_metrics=proximity,spread",
                "--score_clip_los=" + str(clip["clip_lo"]) + "," + str(spread["clip_lo"]),
                "--score_clip_his=" + str(clip["clip_hi"]) + "," + str(spread["clip_hi"]),
                "--score_program=" + program,
            ],
            capture_output=True,
            text=True,
            timeout=10,
        )
        assert r.returncode == 0, f"{program} summary failed: " + r.stderr[:200]
        summary = json.loads(r.stdout)
        assert summary["n_solves"] == 3
        print(f"  score program {program}: OK")

    for program in ["m0;flip", "m0;sawtooth:10", "m0;omega_cosine:3:1.57079632679"]:
        r = subprocess.run(
            [
                sps_path,
                sps_bin,
                "--mode=summary",
                "--degree=2",
                "--score_metrics=proximity",
                "--score_clip_los=" + str(clip["clip_lo"]),
                "--score_clip_his=" + str(clip["clip_hi"]),
                "--score_program=" + program,
            ],
            capture_output=True,
            text=True,
            timeout=10,
        )
        assert r.returncode == 0, f"{program} summary failed: " + r.stderr[:200]
        summary = json.loads(r.stdout)
        assert summary["n_solves"] == 3
        print(f"  score program {program}: OK")

    # 4. Clusteriness clip (v2 metric)
    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2", "--metric=clusteriness"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "clusteriness clip failed: " + r.stderr[:200]
    clust = json.loads(r.stdout)
    assert clust["metric"] == "clusteriness"
    assert clust["n_solves"] == 3
    print("  clusteriness clip: OK (lo=%.2f, hi=%.2f)" % (clust["clip_lo"], clust["clip_hi"]))

    # 5. Real-axis proximity clip (v2 metric)
    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2", "--metric=real_axis_proximity"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "real_axis_proximity clip failed: " + r.stderr[:200]
    rap = json.loads(r.stdout)
    assert rap["metric"] == "real_axis_proximity"
    assert rap["n_solves"] == 3
    print("  real_axis_proximity clip: OK (lo=%.2f, hi=%.2f)" % (rap["clip_lo"], rap["clip_hi"]))

    # 5b. New v4 metrics
    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2", "--metric=min_mod"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "min_mod clip failed: " + r.stderr[:200]
    min_mod = json.loads(r.stdout)
    assert min_mod["metric"] == "min_mod"
    assert min_mod["n_solves"] == 3
    print("  min_mod clip: OK (lo=%.2f, hi=%.2f)" % (min_mod["clip_lo"], min_mod["clip_hi"]))

    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2", "--metric=min_angular_separation"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "min_angular_separation clip failed: " + r.stderr[:200]
    min_ang = json.loads(r.stdout)
    assert min_ang["metric"] == "min_angular_separation"
    assert min_ang["n_solves"] == 3
    print("  min_angular_separation clip: OK (lo=%.2f, hi=%.2f)" % (min_ang["clip_lo"], min_ang["clip_hi"]))

    # 6. Non-default quantile clip (q=0.05)
    _write_sps_bin(sps_bin)
    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2", "--metric=proximity",
                        "--quantile_lo=0.05", "--quantile_hi=0.95"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "q=0.05 clip failed: " + r.stderr[:200]
    q05 = json.loads(r.stdout)
    assert q05["n_solves"] == 3
    print("  proximity clip q=5%%: OK (lo=%.2f, hi=%.2f)" % (q05["clip_lo"], q05["clip_hi"]))

    cleanup(sps_bin)
    print("=== solve_proximity_stats tests PASSED ===")


# ── Catalog Degree Verification ──────────────────────────────────────────

def test_catalog_degrees():
    print("\n--- Catalog degree verification ---")

    catalog_path = "/src/../coeff_func_catalog_js.js"
    if not os.path.exists(catalog_path):
        print("  SKIP: coeff_func_catalog_js.js not found (run gen_catalog.py first)")
        return

    import re as re_mod
    with open(catalog_path) as f:
        js_text = f.read()
    cat_json = js_text.split("window._coeffFuncCatalog = ")[1].rstrip(";\n")
    catalog = json.loads(cat_json)

    mismatches = []
    tested = 0
    for entry in catalog:
        if entry.get("probe_failed"):
            continue
        m, err = run_coeffgen(entry["name"])
        if m is None:
            continue
        actual_degree = m["n_coeffs"] - 1
        if actual_degree != entry["degree"]:
            mismatches.append("%s: catalog=%s deploy=%s" % (entry["name"], entry["degree"], actual_degree))
        tested += 1

    if mismatches:
        print("  FAIL: %d degree mismatches:" % len(mismatches))
        for mm in mismatches[:10]:
            print("    %s" % mm)
        sys.exit(1)

    print("  %d functions: all degrees match between host catalog and deploy binary" % tested)
    print("=== Catalog degree verification PASSED ===")


# ── Main ─────────────────────────────────────────────────────────────────

if __name__ == "__main__":
    print("--- Binary validation ---")
    for bin_path in [
        "/src/sweep",
        "/src/sweep_mt",
        "/src/sweep_cm",
        "/src/sweep_coeffgen",
        "/src/roots2pix_mt",
        "/src/solve_palette_chunk_mt",
        "/src/bilevel_section_raster",
        "/src/bilevel_merge",
        "/src/raw_to_bilevel",
        "/src/assemble_greyscale",
    ]:
        magic = open(bin_path, "rb").read(4)
        assert magic == b"\x7fELF", "%s is not an ELF binary" % bin_path
        print("  %s: ELF OK" % bin_path)

    print("--- Generating test fixtures ---")
    test_ae_cm_solvers()
    test_cfpv_coeffgen()
    test_param_gen_threaded_runtime()
    test_coeffgen_chunked_threaded_runtime()
    test_compute_preview_runtime_combo()
    test_palette_chunk_mt_runtime()
    test_palette_chunk_mt_param_sectioned_runtime()
    test_roots2pix_mt_multispan_runtime()
    test_render_preview()
    test_resize_runtime()
    test_bilevel_section_raster_runtime()
    test_bilevel_merge_assemble_runtime()
    test_raw_to_bilevel_runtime()
    test_color_to_bilevel_handler_runtime()
    test_bilevel_handler_sparse_finalize_runtime()
    test_solve_proximity_stats()
    test_catalog_degrees()

    print("\n=== All Docker runtime tests PASSED ===")
