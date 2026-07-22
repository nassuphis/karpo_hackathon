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
import base64
import http.server
import socketserver
import struct
import subprocess
import sys
import threading
import types
import urllib.parse

if "/src" not in sys.path:
    sys.path.insert(0, "/src")

from coeff_program_chain import compile_coeff_program_chain
from param_program_chain import compile_param_program_chain


# ── Helpers ──────────────────────────────────────────────────────────────

def param_program_payload(chain):
    compiled = compile_param_program_chain(chain)
    payload = {
        "version": compiled["version"],
        "fingerprint": compiled["fingerprint"],
        "tokens": compiled["tokens"],
        "stack_max": compiled["stack_max"],
        "token_count": compiled["token_count"],
        "uses_legacy_fast_path": compiled["uses_legacy_fast_path"],
    }
    if compiled.get("scalar_exprs"):
        payload["scalar_exprs"] = compiled["scalar_exprs"]
    return payload


def coeff_program_payload(chain):
    compiled = compile_coeff_program_chain(chain)
    return {
        "version": compiled["version"],
        "fingerprint": compiled["fingerprint"],
        "tokens": compiled["tokens"],
        "stack_max": compiled["stack_max"],
        "token_count": compiled["token_count"],
        "scalar_exprs": compiled["scalar_exprs"],
        "uses_legacy_chain_equivalent": compiled["uses_legacy_chain_equivalent"],
    }


def param_legacy_program(name, src="both", tgt="both", *args):
    return param_program_payload([["legacy", name, src, tgt, *map(str, args)]])


def coeff_legacy_program(name, src="poly", tgt="poly", *args):
    return coeff_program_payload([["legacy", name, src, tgt, *map(str, args)], ["emit"]])

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
        "param_transforms": [],
        "param_program": param_legacy_program("unit_circle"),
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


def read_png_ihdr(path):
    with open(path, "rb") as f:
        header = f.read(29)
    assert header[:8] == b"\x89PNG\r\n\x1a\n", "PNG signature missing for %s" % path
    assert header[12:16] == b"IHDR", "PNG IHDR missing for %s" % path
    width, height = struct.unpack(">II", header[16:24])
    return width, height, header[24], header[25]


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


def read_rgb_pixel_with_vips(image_path, x, y):
    r = subprocess.run(
        ["/opt/bin/vips", "getpoint", image_path, str(int(x)), str(int(y))],
        capture_output=True,
        text=True,
        timeout=20,
    )
    assert r.returncode == 0, "vips getpoint RGB image failed: " + r.stderr[:200]
    values = [float(part) for part in r.stdout.replace(",", " ").split() if part]
    assert len(values) >= 3, "vips getpoint did not return RGB bands: %r" % r.stdout
    return tuple(int(round(values[idx])) for idx in range(3))


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
        self.base_url = ""
        self.puts = []

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
        self.puts.append(Key)
        self.objects[Key] = {
            "Body": bytes(body),
            "ContentType": ContentType or "application/octet-stream",
            "Metadata": dict(Metadata or {}),
        }
        return {"ETag": '"mem"'}

    def generate_presigned_url(self, ClientMethod, Params, ExpiresIn=900):
        assert ClientMethod == "get_object", "unexpected presign method %r" % (ClientMethod,)
        assert self.base_url, "base_url must be set before presigning"
        return self.base_url.rstrip("/") + "/" + urllib.parse.quote(str(Params["Key"]))


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
        start = 0
        end = len(body) - 1
        range_hdr = self.headers.get("Range")
        if range_hdr and range_hdr.startswith("bytes="):
            raw = range_hdr[len("bytes="):]
            if "-" not in raw:
                self.send_response(416)
                self.end_headers()
                return
            lo, hi = raw.split("-", 1)
            start = int(lo) if lo else 0
            end = int(hi) if hi else len(body) - 1
            if start < 0 or end < start or end >= len(body):
                self.send_response(416)
                self.end_headers()
                return
            self.send_response(206)
            self.send_header("Content-Range", "bytes %d-%d/%d" % (start, end, len(body)))
        else:
            self.send_response(200)
        chunk = body[start:end + 1]
        self.send_header("Content-Type", obj.get("ContentType") or "application/octet-stream")
        self.send_header("Content-Length", str(len(chunk)))
        self.end_headers()
        self.wfile.write(chunk)

    def log_message(self, fmt, *args):
        return


# ── AE/CM Solver Tests ───────────────────────────────────────────────────

def test_sweep_mt_stream_flush_byte_identity():
    """CR34 §12-1b A/B gate: the incremental pwrite writer (with and without
    the progress sidecar) must produce byte-identical output to the retired
    single-fwrite writer. The input is deterministic and the expected SHA was
    captured from the pre-change binary on ARM64 Linux (commit a5c7d97 tree).
    avg_iterations is pinned too — it moves if solve order or chains change.
    """
    import hashlib
    import math
    import struct

    print("--- sweep_mt streaming-flush byte identity ---")
    n_steps, n_coeffs = 50000, 8
    coeffs_path = "/tmp/stream_ab_cf.bin"
    with open(coeffs_path, "wb") as f:
        for s in range(n_steps):
            t = s / n_steps
            row = []
            for k in range(n_coeffs):
                re = math.cos(2 * math.pi * (k + 1) * t) + (0.25 if k == 0 else 0.0)
                im = math.sin(2 * math.pi * (k + 2) * t) * 0.5
                row += [re, im]
            f.write(struct.pack("<%df" % (2 * n_coeffs), *row))
    got = hashlib.sha256(open(coeffs_path, "rb").read()).hexdigest()
    assert got == "12ba957b406633b56b40c3919736a1279e4f87b28c7c643f47beabb1e51f3d72", \
        "A/B input drifted: " + got

    expected_out = "3ab87cbd0a69cebe88918fed4a9136ad51c2cf30d41f834eea9d274d77e00c6e"
    base_spec = {
        "mode": "solve_mt", "coeffs_file": coeffs_path,
        "n_coeffs": n_coeffs, "n2": n_steps,
        "i1_start": 0, "i1_end": 1, "match_roots": False, "n_threads": 4,
    }

    r = subprocess.run(["/src/sweep_mt", "/tmp/stream_ab_plain.bin"],
                       input=json.dumps(base_spec), capture_output=True,
                       text=True, timeout=120)
    assert r.returncode == 0, "plain solve FAILED: " + r.stderr[:200]
    meta_plain = json.loads(r.stdout)
    sha_plain = hashlib.sha256(open("/tmp/stream_ab_plain.bin", "rb").read()).hexdigest()
    assert sha_plain == expected_out, "no-sidecar output drifted: " + sha_plain

    prog_spec = dict(base_spec)
    prog_spec["progress_file"] = "/tmp/stream_ab.progress"
    prog_spec["flush_bytes"] = 8192   # force MANY flushes per thread
    r = subprocess.run(["/src/sweep_mt", "/tmp/stream_ab_prog.bin"],
                       input=json.dumps(prog_spec), capture_output=True,
                       text=True, timeout=120)
    assert r.returncode == 0, "sidecar solve FAILED: " + r.stderr[:200]
    meta_prog = json.loads(r.stdout)
    sha_prog = hashlib.sha256(open("/tmp/stream_ab_prog.bin", "rb").read()).hexdigest()
    assert sha_prog == expected_out, "sidecar output drifted: " + sha_prog

    for key in meta_plain:
        if key != "elapsed_us":
            assert meta_plain[key] == meta_prog[key], \
                "meta drifted at %s: %r vs %r" % (key, meta_plain[key], meta_prog[key])
    assert abs(meta_plain["avg_iterations"] - 2.01) < 1e-9, meta_plain["avg_iterations"]

    blob = open("/tmp/stream_ab.progress", "rb").read()
    assert blob[:4] == b"PPR1", blob[:4]
    n_threads, total = struct.unpack_from("<IQ", blob, 4)
    expected_total = n_steps * (n_coeffs - 1) * 2 * 4
    assert n_threads == 4 and total == expected_total, (n_threads, total)
    covered = 0
    for i in range(n_threads):
        rs, re, fe = struct.unpack_from("<QQQ", blob, 16 + i * 24)
        assert rs <= re <= total and fe == re, (i, rs, re, fe)
        covered += re - rs
    assert covered == total, covered
    print("sweep_mt stream flush: SHA %s… identical plain/sidecar; sidecar valid" % sha_prog[:12])


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


# ── Coeff Program vector-constant / translate_roots / bimodal opcodes ────

def test_coeff_vector_constant_opcodes_runtime():
    """giga_2902 review finding 4: opcodes 48/49 and the bimodal scalar were
    only exercised by the mac-clang sweep_test build; this runs them on the
    DEPLOYED musl/ARM64 sweep_coeffgen (the CR31 stdint.h seam). Payloads are
    exact compiled captures; sources are noted beside each."""
    import struct

    print("\n--- Coeff Program vector-constant opcode regression ---")

    # poly = translate_roots(vector_literal(1, -3, 2), 0.5)\nemit
    translate_payload = {
        "version": 1,
        "fingerprint": "228f40e9296c68f2d37be0066346e2b97f33cc04",
        "tokens": [
            {"op": 48, "n_args": 1, "args": [0.0]},
            {"op": 21, "n_args": 1, "args": [0.5], "args_im": [0.0], "expr_refs": [-1]},
            {"op": 49},
            {"op": 26},
            {"op": 3},
        ],
        "stack_max": 2,
        "scalar_exprs": [],
        "vector_constants": [{"length": 3, "values": [1.0, 0.0, -3.0, 0.0, 2.0, 0.0]}],
    }
    out_path = "/tmp/coeff_vector_const_out.bin"
    r = subprocess.run(
        ["/src/sweep_coeffgen", out_path],
        input=json.dumps({
            "mode": "coeffgen",
            "function": "const",
            "cfpv": [3, 0, 0],
            "n1": 1, "n2": 1,
            "coeff_transforms": [],
            "coeff_program": translate_payload,
        }),
        capture_output=True, text=True, timeout=30,
    )
    assert r.returncode == 0, "translate_roots coeffgen FAILED: " + r.stderr[:300]
    with open(out_path, "rb") as fh:
        values = struct.unpack("<6f", fh.read())
    # Q(z) = P(z-0.5) for P = z^2 - 3z + 2 (roots 1,2 -> 1.5,2.5): [1, -4, 3.75]
    assert values == (1.0, 0.0, -4.0, 0.0, 3.75, 0.0), values
    print("  translate_roots(vector_literal(1,-3,2), 0.5) -> [1,-4,3.75]: OK")

    # vector_literal(0)\nvector_literal(10)\npoly = blend(bimodal(t2, 0.7))\nemit
    bimodal_payload = {
        "version": 1,
        "fingerprint": "882032cd1ed7f4f2e926043fa2c23945b5826325",
        "tokens": [
            {"op": 48, "n_args": 1, "args": [0.0]},
            {"op": 48, "n_args": 1, "args": [1.0]},
            {"op": 21, "n_args": 1, "args": [0.0], "args_im": [0.0], "expr_refs": [0]},
            {"op": 30},
            {"op": 26},
            {"op": 3},
        ],
        "stack_max": 3,
        "scalar_exprs": [[17.0, 0.0, 0.0, 1.0, 0.7, 0.0, 36.0, 0.0, 0.0]],
        "vector_constants": [
            {"length": 1, "values": [0.0, 0.0]},
            {"length": 1, "values": [10.0, 0.0]},
        ],
    }
    bimodal_path = "/tmp/coeff_bimodal_out.bin"
    r = subprocess.run(
        ["/src/sweep_coeffgen", bimodal_path],
        input=json.dumps({
            "mode": "coeffgen",
            "function": "const",
            "cfpv": [1, 0, 0],
            "n1": 1, "n2": 3,
            "coeff_transforms": [],
            "coeff_program": bimodal_payload,
        }),
        capture_output=True, text=True, timeout=30,
    )
    assert r.returncode == 0, "bimodal coeffgen FAILED: " + r.stderr[:300]
    with open(bimodal_path, "rb") as fh:
        rows = struct.unpack("<6f", fh.read())
    exponent = 1.0 / (1.0 - 0.7)
    expected = [
        0.0,
        10.0 * (0.5 * (2.0 / 3.0) ** exponent),
        10.0 * (1.0 - 0.5 * (2.0 / 3.0) ** exponent),
    ]
    got = [rows[0], rows[2], rows[4]]
    for actual, wanted in zip(got, expected):
        assert abs(actual - wanted) <= 2e-6, (got, expected)
    print("  blend(bimodal(t2, 0.7)) over two pool vectors: OK")

    # fail-closed native validation: out-of-range pool index must reject
    bad = dict(translate_payload)
    bad["tokens"] = [{"op": 48, "n_args": 1, "args": [7.0]}]
    bad["stack_max"] = 1
    r = subprocess.run(
        ["/src/sweep_coeffgen", "/tmp/coeff_bad_pool.bin"],
        input=json.dumps({
            "mode": "coeffgen",
            "function": "const",
            "cfpv": [1, 0, 0],
            "n1": 1, "n2": 1,
            "coeff_transforms": [],
            "coeff_program": bad,
        }),
        capture_output=True, text=True, timeout=30,
    )
    assert r.returncode != 0, "out-of-range pool index was accepted"
    assert "invalid pool index" in r.stderr, r.stderr[:300]
    print("  out-of-range pool index rejected: OK")

    cleanup(out_path, bimodal_path, "/tmp/coeff_bad_pool.bin")
    print("=== Coeff Program vector-constant opcode tests PASSED ===")


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
        "param_transforms": [],
        "param_program": param_program_payload([
            ["legacy", "unit_circle", "both", "both"],
            ["legacy", "square", "both", "both"],
        ]),
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

    def run_param_target(transforms, path):
        spec = {
            "mode": "param_gen",
            "n1": 4,
            "n2": 4,
            "times": 1,
            "n_threads": 1,
            "param_transforms": [],
            "param_program": param_program_payload(transforms),
        }
        proc = subprocess.run(
            ["/src/sweep_coeffgen", path],
            input=json.dumps(spec),
            capture_output=True,
            text=True,
            timeout=30,
        )
        assert proc.returncode == 0, "param_gen target run failed: " + proc.stderr[:200]
        with open(path, "rb") as fh:
            return fh.read()

    legacy_uc = run_param_target([["legacy", "unit_circle", "both", "both"]], "/tmp/param_gen_uc_legacy.bin")
    both_uc = run_param_target([["legacy", "unit_circle", "both", "both"]], "/tmp/param_gen_uc_both.bin")
    t1_uc = run_param_target([["legacy", "unit_circle", "both", "p1"]], "/tmp/param_gen_uc_t1.bin")
    t2_uc = run_param_target([["legacy", "unit_circle", "both", "p2"]], "/tmp/param_gen_uc_t2.bin")
    assert legacy_uc == both_uc, "unit_circle target=both must preserve legacy no-arg output"
    assert t1_uc != both_uc, "unit_circle target=t1 should not match both-target output"
    assert t2_uc != both_uc, "unit_circle target=t2 should not match both-target output"
    t1_vals = struct.unpack("<" + "f" * (len(t1_uc) // 4), t1_uc)
    t2_vals = struct.unpack("<" + "f" * (len(t2_uc) // 4), t2_uc)
    assert all(abs(t1_vals[i + 3]) < 1e-7 for i in range(0, len(t1_vals), 4)), "target=t1 should leave t2 imaginary raw"
    assert all(abs(t2_vals[i + 1]) < 1e-7 for i in range(0, len(t2_vals), 4)), "target=t2 should leave t1 imaginary raw"
    old_rtheta = run_param_target([["legacy", "rtheta", "both", "both", "1"]], "/tmp/param_gen_rtheta_old.bin")
    new_rtheta = run_param_target([["legacy", "rtheta", "both", "both", "1"]], "/tmp/param_gen_rtheta_new.bin")
    assert old_rtheta == new_rtheta, "rtheta p-only form must remain equivalent to target=both"
    print("  sweep_coeffgen targetable param transforms: OK")

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
    legacy_chain_path = "/tmp/coeffgen_legacy_chain.bin"
    legacy_program_path = "/tmp/coeffgen_legacy_program.bin"
    rev_emit_chain_path = "/tmp/coeffgen_rev_emit_chain.bin"
    rev_emit_program_path = "/tmp/coeffgen_rev_emit_program.bin"
    const_program_single_path = "/tmp/coeffgen_const_program_single.bin"
    const_program_mt_path = "/tmp/coeffgen_const_program_mt.bin"
    linspace_program_path = "/tmp/coeffgen_linspace_program.bin"
    blend_program_path = "/tmp/coeffgen_blend_program.bin"
    native_typed_program_path = "/tmp/coeffgen_native_typed_program.bin"
    native_stack_arg_program_path = "/tmp/coeffgen_native_stack_arg_program.bin"
    long_program_path = "/tmp/coeffgen_long_program.bin"
    poke_program_path = "/tmp/coeffgen_poke_program.bin"
    debug_poke_path = "/tmp/compute_debug_poke.bin"

    param_spec = {
        "mode": "param_gen",
        "n1": 10,
        "n2": 10,
        "times": 2,
        "param_transforms": [],
        "param_program": param_legacy_program("unit_circle"),
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
        "coeff_transforms": [],
        "coeff_program": coeff_legacy_program("exp", "poly", "poly", "0.3", "-0.2"),
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
    plain_meta = json.loads(r.stdout)
    r = subprocess.run(
        ["/src/sweep_coeffgen", andy_path],
        input=json.dumps({
            **plain_spec,
            "coeff_program": coeff_legacy_program("linear", "poly", "poly", "100", "0", "1e-5"),
        }),
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
    print("  coeff transform andy blend: OK (linear andy=1e-5)")

    legacy_chain_spec = {
        "mode": "coeffgen_chunked",
        "function": "g1",
        "coeff_transforms": [["rev"], ["cumsum"], ["sort_abs"], ["exp", "0.5"]],
        "params_file": params_path,
        "step_start": 0,
        "step_count": 200,
        "n_threads": 1,
    }
    legacy_program_spec = {
        "mode": "coeffgen_chunked",
        "function": "g1",
        "coeff_transforms": [],
        "params_file": params_path,
        "step_start": 0,
        "step_count": 200,
        "n_threads": 1,
        "coeff_program": {
            "version": 1,
            "fingerprint": "docker-legacy-parity",
            "tokens": [
                {"op": 9, "fn_index": 1, "src": 2, "tgt": 2},
                {"op": 9, "fn_index": 11, "src": 2, "tgt": 2},
                {"op": 9, "fn_index": 10, "src": 2, "tgt": 2},
                {"op": 9, "fn_index": 16, "src": 2, "tgt": 2, "n_args": 2,
                 "args": [0.5, 0.0], "args_im": [0.0, 0.0], "expr_refs": [-1, -1]},
            ],
            "scalar_exprs": [],
        },
    }
    r = subprocess.run(
        ["/src/sweep_coeffgen", legacy_chain_path],
        input=json.dumps(legacy_chain_spec),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode != 0, "native coeff_transforms should be rejected after Phase 5"
    assert "coeff_transforms is no longer accepted" in r.stderr
    r = subprocess.run(
        ["/src/sweep_coeffgen", legacy_program_path],
        input=json.dumps(legacy_program_spec),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "coeffgen legacy-equivalent coeff_program failed: " + r.stderr[:200]
    legacy_program_meta = json.loads(r.stdout)
    assert legacy_program_meta["coeff_program_tokens"] == 4, "coeff_program metadata missing token count"
    with open(legacy_program_path, "rb") as f:
        legacy_program_bytes = f.read()
    assert len(legacy_program_bytes) == legacy_program_meta["data_bytes"], "coeff_program byte count mismatch"
    print("  coeff_program legacy-equivalent native path: OK (%d bytes)" % len(legacy_program_bytes))

    r = subprocess.run(
        ["/src/sweep_coeffgen", rev_emit_chain_path],
        input=json.dumps({**plain_spec, "coeff_transforms": [["rev"]]}),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode != 0, "native coeff_transforms rev chain should be rejected after Phase 5"
    assert "coeff_transforms is no longer accepted" in r.stderr
    r = subprocess.run(
        ["/src/sweep_coeffgen", rev_emit_program_path],
        input=json.dumps({
            **plain_spec,
            "coeff_transforms": [],
            "coeff_program": {
                "version": 1,
                "fingerprint": "docker-rev-emit-commit",
                "tokens": [
                    {"op": 9, "fn_index": 1, "src": 2, "tgt": 2},
                    {"op": 3},
                ],
                "scalar_exprs": [],
            },
        }),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "coeffgen rev;emit coeff_program failed: " + r.stderr[:200]
    plain_vals = read_f32_array(plain_path)
    rev_emit_vals = read_f32_array(rev_emit_program_path)
    n_coeffs = int(plain_meta["n_coeffs"])
    row_stride = n_coeffs * 2
    assert len(plain_vals) == len(rev_emit_vals), "coeff_program rev output length mismatch"
    for row_start in range(0, len(plain_vals), row_stride):
        for k in range(n_coeffs):
            src = row_start + (n_coeffs - 1 - k) * 2
            dst = row_start + k * 2
            assert rev_emit_vals[dst] == plain_vals[src], "coeff_program rev real output diverged"
            assert rev_emit_vals[dst + 1] == plain_vals[src + 1], "coeff_program rev imag output diverged"
    print("  coeff_program rev;emit commit parity: OK")

    const_program_spec = {
        "mode": "coeffgen_chunked",
        "function": "g1",
        "coeff_transforms": [],
        "params_file": params_path,
        "step_start": 0,
        "step_count": 200,
        "coeff_program": {
            "version": 1,
            "fingerprint": "docker-const-two-exprs",
            "tokens": [
                {"op": 1, "n_args": 2, "args": [3.0, 0.0], "args_im": [0.0, 0.0], "expr_refs": [-1, 0]},
                {"op": 3},
                {"op": 1, "n_args": 2, "args": [3.0, 0.0], "args_im": [0.0, 0.0], "expr_refs": [-1, 1]},
                {"op": 3},
            ],
            "scalar_exprs": [
                [2.0, 0.0, 0.0, 3.0, 0.0, 0.0, 4.0, 0.0, 0.0],
                [2.0, 0.0, 0.0, 3.0, 0.0, 0.0, 5.0, 0.0, 0.0],
            ],
        },
    }
    r = subprocess.run(
        ["/src/sweep_coeffgen", const_program_single_path],
        input=json.dumps({**const_program_spec, "n_threads": 1}),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "coeffgen const coeff_program single-thread failed: " + r.stderr[:200]
    const_meta = json.loads(r.stdout)
    assert const_meta["n_coeffs"] == 3, "const coeff_program should emit length 3"
    assert const_meta["coeff_program_tokens"] == 4, "const coeff_program metadata missing token count"
    r = subprocess.run(
        ["/src/sweep_coeffgen", const_program_mt_path],
        input=json.dumps({**const_program_spec, "n_threads": 4}),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "coeffgen const coeff_program multi-thread failed: " + r.stderr[:200]
    with open(const_program_single_path, "rb") as f:
        const_single_bytes = f.read()
    with open(const_program_mt_path, "rb") as f:
        const_mt_bytes = f.read()
    assert const_single_bytes == const_mt_bytes, "const coeff_program threaded output diverged"
    params_vals = read_f32_array(params_path)
    const_vals = read_f32_array(const_program_single_path)
    for step in range(const_meta["step_count"]):
        p1 = complex(params_vals[step * 4], params_vals[step * 4 + 1])
        p2 = complex(params_vals[step * 4 + 2], params_vals[step * 4 + 3])
        expected = p1 - p2
        for coeff_idx in range(const_meta["n_coeffs"]):
            base = step * const_meta["n_coeffs"] * 2 + coeff_idx * 2
            got = complex(const_vals[base], const_vals[base + 1])
            assert abs(got.real - expected.real) <= 1e-6, "const coeff_program real mismatch"
            assert abs(got.imag - expected.imag) <= 1e-6, "const coeff_program imag mismatch"
    print("  coeff_program const(length,p1±p2) threaded runtime: OK")

    linspace_program_spec = {
        "mode": "coeffgen_chunked",
        "function": "g1",
        "coeff_transforms": [],
        "params_file": params_path,
        "step_start": 0,
        "step_count": 5,
        "n_threads": 1,
        "coeff_program": {
            "version": 1,
            "fingerprint": "docker-push-linspace-poly-len",
            "tokens": [
                {"op": 17, "n_args": 1, "args": [-1.0], "expr_refs": [-1]},
                {"op": 3},
            ],
            "scalar_exprs": [],
        },
    }
    r = subprocess.run(
        ["/src/sweep_coeffgen", linspace_program_path],
        input=json.dumps(linspace_program_spec),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "coeffgen push_linspace coeff_program failed: " + r.stderr[:200]
    linspace_meta = json.loads(r.stdout)
    linspace_vals = read_f32_array(linspace_program_path)
    n_coeffs = linspace_meta["n_coeffs"]
    assert n_coeffs >= 1, "push_linspace should emit at least one coefficient"
    for step in range(linspace_meta["step_count"]):
        for coeff_idx in range(n_coeffs):
            base = step * n_coeffs * 2 + coeff_idx * 2
            expected = 0.0 if n_coeffs == 1 else float(n_coeffs) * float(coeff_idx) / float(n_coeffs - 1)
            assert abs(linspace_vals[base] - expected) <= 1e-6, "push_linspace real mismatch"
            assert abs(linspace_vals[base + 1]) <= 1e-6, "push_linspace imag mismatch"
    print("  coeff_program push_linspace(poly_len) runtime: OK")

    blend_program_spec = {
        "mode": "coeffgen_chunked",
        "function": "g1",
        "coeff_transforms": [],
        "params_file": params_path,
        "step_start": 0,
        "step_count": 5,
        "n_threads": 1,
        "coeff_program": {
            "version": 1,
            "fingerprint": "docker-blend-order",
            "tokens": [
                {"op": 1, "n_args": 2, "args": [3.0, 1.0], "args_im": [0.0, 0.0], "expr_refs": [-1, -1]},
                {"op": 1, "n_args": 2, "args": [3.0, 3.0], "args_im": [0.0, 0.0], "expr_refs": [-1, -1]},
                {"op": 8, "n_args": 1, "args": [0.25], "args_im": [0.0], "expr_refs": [-1]},
                {"op": 3},
            ],
            "scalar_exprs": [],
        },
    }
    r = subprocess.run(
        ["/src/sweep_coeffgen", blend_program_path],
        input=json.dumps(blend_program_spec),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "coeffgen blend coeff_program failed: " + r.stderr[:200]
    blend_meta = json.loads(r.stdout)
    assert blend_meta["n_coeffs"] == 3, "blend coeff_program should emit length 3"
    blend_vals = read_f32_array(blend_program_path)
    for idx in range(0, len(blend_vals), 2):
        assert abs(blend_vals[idx] - 1.5) <= 1e-6, "blend order real mismatch"
        assert abs(blend_vals[idx + 1]) <= 1e-6, "blend order imag mismatch"
    print("  coeff_program blend(t) order: OK (below*(1-t)+top*t)")

    native_typed_program_spec = {
        "mode": "coeffgen_chunked",
        "function": "g1",
        "coeff_transforms": [],
        "params_file": params_path,
        "step_start": 0,
        "step_count": 5,
        "n_threads": 1,
        "coeff_program": {
            "version": 1,
            "fingerprint": "docker-native-transform-typed-blend",
            "tokens": [
                {"op": 18, "n_args": 3, "args": [0.0, 4.0, 1.0], "expr_refs": [-1, -1, -1]},
                {"op": 29, "fn_index": 1, "src": 3, "tgt": 5},
                {"op": 21, "n_args": 1, "args": [4.0], "args_im": [0.0], "expr_refs": [-1]},
                {"op": 21, "n_args": 1, "args": [1.0], "args_im": [0.0], "expr_refs": [-1]},
                {"op": 28},
                {"op": 21, "n_args": 1, "args": [0.25], "args_im": [0.0], "expr_refs": [-1]},
                {"op": 30},
                {"op": 3},
            ],
            "scalar_exprs": [],
        },
    }
    r = subprocess.run(
        ["/src/sweep_coeffgen", native_typed_program_path],
        input=json.dumps(native_typed_program_spec),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "coeffgen native transform + typed blend failed: " + r.stderr[:200]
    native_typed_meta = json.loads(r.stdout)
    assert native_typed_meta["n_coeffs"] == 4, "native transform typed blend should emit length 4"
    native_typed_vals = read_f32_array(native_typed_program_path)
    expected_native_typed = [2.5, 1.75, 1.0, 0.25]
    for step in range(native_typed_meta["step_count"]):
        for coeff_idx, expected in enumerate(expected_native_typed):
            base = step * native_typed_meta["n_coeffs"] * 2 + coeff_idx * 2
            assert abs(native_typed_vals[base] - expected) <= 1e-6, "native transform typed blend real mismatch"
            assert abs(native_typed_vals[base + 1]) <= 1e-6, "native transform typed blend imag mismatch"
    print("  coeff_program native transform + typed blend runtime: OK")

    native_stack_arg_program_spec = {
        "mode": "coeffgen_chunked",
        "function": "g1",
        "coeff_transforms": [],
        "params_file": params_path,
        "step_start": 0,
        "step_count": 5,
        "n_threads": 1,
        "coeff_program": {
            "version": 1,
            "fingerprint": "docker-native-transform-stack-args",
            "tokens": [
                {"op": 18, "n_args": 3, "args": [0.0, 2.0, 1.0], "expr_refs": [-1, -1, -1]},
                {"op": 21, "n_args": 1, "args": [0.0], "args_im": [1.0], "expr_refs": [-1]},
                {"op": 21, "n_args": 1, "args": [0.0], "args_im": [0.0], "expr_refs": [-1]},
                {"op": 29, "fn_index": 16, "src": 3, "tgt": 5, "stack_arg_count": 2},
                {"op": 3},
            ],
            "scalar_exprs": [],
        },
    }
    r = subprocess.run(
        ["/src/sweep_coeffgen", native_stack_arg_program_path],
        input=json.dumps(native_stack_arg_program_spec),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "coeffgen native transform stack args failed: " + r.stderr[:200]
    native_stack_arg_meta = json.loads(r.stdout)
    assert native_stack_arg_meta["n_coeffs"] == 2, "native transform stack args should emit length 2"
    native_stack_arg_vals = read_f32_array(native_stack_arg_program_path)
    expected_exp = [(1.0, 0.0), (math.cos(1.0), math.sin(1.0))]
    for step in range(native_stack_arg_meta["step_count"]):
        for coeff_idx, (er, ei) in enumerate(expected_exp):
            base = step * native_stack_arg_meta["n_coeffs"] * 2 + coeff_idx * 2
            assert abs(native_stack_arg_vals[base] - er) <= 1e-6, "native transform stack args real mismatch"
            assert abs(native_stack_arg_vals[base + 1] - ei) <= 1e-6, "native transform stack args imag mismatch"
    print("  coeff_program native transform stack args runtime: OK")

    long_tokens = []
    for idx in range(40):
        long_tokens.append({"op": 1, "n_args": 2, "args": [1.0, float(idx)], "args_im": [0.0, 0.0], "expr_refs": [-1, -1]})
        long_tokens.append({"op": 3})
    long_program_spec = {
        "mode": "coeffgen_chunked",
        "function": "g1",
        "coeff_transforms": [],
        "params_file": params_path,
        "step_start": 0,
        "step_count": 5,
        "n_threads": 1,
        "coeff_program": {
            "version": 1,
            "fingerprint": "docker-over-old-64-token-cap",
            "tokens": long_tokens,
            "scalar_exprs": [],
        },
    }
    r = subprocess.run(
        ["/src/sweep_coeffgen", long_program_path],
        input=json.dumps(long_program_spec),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "coeffgen long coeff_program failed: " + r.stderr[:200]
    long_meta = json.loads(r.stdout)
    assert long_meta["coeff_program_tokens"] == 80, "long coeff_program should report 80 tokens"
    assert long_meta["n_coeffs"] == 1, "long coeff_program should emit one coefficient"
    long_vals = read_f32_array(long_program_path)
    for step in range(long_meta["step_count"]):
        base = step * 2
        assert abs(long_vals[base] - 39.0) <= 1e-6, "long coeff_program real mismatch"
        assert abs(long_vals[base + 1]) <= 1e-6, "long coeff_program imag mismatch"
    print("  coeff_program over old 64-token cap runtime: OK")

    poke_program_spec = {
        "mode": "coeffgen_chunked",
        "function": "g1",
        "coeff_transforms": [],
        "params_file": params_path,
        "step_start": 0,
        "step_count": 10,
        "n_threads": 1,
        "coeff_program": {
            "version": 1,
            "fingerprint": "docker-poke-poly-tos",
            "tokens": [
                {"op": 10, "n_args": 2, "args": [0.0, 0.0], "args_im": [0.0, 0.0], "expr_refs": [-1, 0]},
                {"op": 2, "src": 2},
                {"op": 11, "n_args": 2, "args": [1.0, 0.0], "args_im": [0.0, 0.0], "expr_refs": [-1, 1]},
                {"op": 3},
            ],
            "scalar_exprs": [
                [1.0, 0.0, 100.0, 2.0, 0.0, 0.0, 6.0, 0.0, 0.0],
                [3.0, 0.0, 0.0],
            ],
        },
    }
    r = subprocess.run(
        ["/src/sweep_coeffgen", poke_program_path],
        input=json.dumps(poke_program_spec),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "coeffgen poke coeff_program failed: " + r.stderr[:200]
    poke_meta = json.loads(r.stdout)
    assert poke_meta["n_coeffs"] >= 2, "poke coeff_program should preserve at least two coefficients"
    poke_stride = poke_meta["n_coeffs"] * 2
    poke_vals = read_f32_array(poke_program_path)
    params_vals = read_f32_array(params_path)
    for step in range(10):
        p1 = complex(params_vals[step * 4], params_vals[step * 4 + 1])
        p2 = complex(params_vals[step * 4 + 2], params_vals[step * 4 + 3])
        coeff0 = complex(poke_vals[step * poke_stride], poke_vals[step * poke_stride + 1])
        coeff1 = complex(poke_vals[step * poke_stride + 2], poke_vals[step * poke_stride + 3])
        expected0 = p1 * 100j
        assert abs(coeff0.real - expected0.real) <= 1e-5, "poke_poly real mismatch"
        assert abs(coeff0.imag - expected0.imag) <= 1e-5, "poke_poly imag mismatch"
        assert abs(coeff1.real - p2.real) <= 1e-6, "poke_tos real mismatch"
        assert abs(coeff1.imag - p2.imag) <= 1e-6, "poke_tos imag mismatch"
    print("  coeff_program poke_poly/poke_tos runtime: OK")

    debug_spec = {
        "mode": "compute_debug",
        "function": "const",
        "u": 0.25,
        "v": 0.75,
        "grid_n": 32,
        "cfpv": [3, 1, 0],
        "coeff_program": {
            "version": 1,
            "fingerprint": "docker-debug-poke",
            "tokens": [
                {"op": 10, "n_args": 2, "args": [0.0, 0.0], "args_im": [0.0, 0.0], "expr_refs": [-1, 0]},
            ],
            "scalar_exprs": [[1.0, 0.0, 100.0, 2.0, 0.0, 0.0, 6.0, 0.0, 0.0]],
        },
    }
    r = subprocess.run(
        ["/src/sweep_coeffgen", debug_poke_path],
        input=json.dumps(debug_spec),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert r.returncode == 0, "compute_debug poke failed: " + r.stderr[:200]
    debug_meta = json.loads(r.stdout)
    assert debug_meta["mode"] == "compute_debug", "compute_debug metadata mode mismatch"
    assert debug_meta["coeff"]["poly"][0] == [0, 25], "compute_debug should expose poked coefficient"
    debug_vals = read_f32_array(debug_poke_path)
    assert abs(debug_vals[0]) <= 1e-6 and abs(debug_vals[1] - 25.0) <= 1e-6, "compute_debug output bytes mismatch"
    print("  compute_debug single-point coeff output: OK")

    cleanup(
        params_path, single_path, mt_path, plain_path, andy_path,
        legacy_chain_path, legacy_program_path,
        rev_emit_chain_path, rev_emit_program_path,
        const_program_single_path, const_program_mt_path, linspace_program_path,
        blend_program_path, native_typed_program_path, native_stack_arg_program_path, long_program_path,
        poke_program_path, debug_poke_path,
    )
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
        "param_transforms": [],
        "param_program": param_legacy_program("unit_circle"),
        "coeff_transforms": [],
        "coeff_program": coeff_legacy_program("roots_cm", "poly", "poly", "hi"),
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
                "--score_program=m0-0;m1-0;mul",
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


def _write_lagged_centroid_fixture(path):
    rows = [
        [(0.0, 0.0), (0.0, 0.0)],
        [(5.0, 0.0), (5.0, 0.0)],
        [(10.0, 0.0), (10.0, 0.0)],
    ]
    with open(path, "wb") as f:
        for roots in rows:
            for re_val, im_val in roots:
                f.write(struct.pack("<ff", re_val, im_val))


def _write_single_source_manifest(path, logical_size, row_bytes, port):
    with open(path, "w", encoding="utf-8") as f:
        json.dump({
            "source_family": "slv",
            "logical_size": logical_size,
            "row_bytes": row_bytes,
            "solve_start": 0,
            "solve_count": logical_size // row_bytes,
            "sources": [{
                "id": 0,
                "url": "http://127.0.0.1:%d/input.bin" % port,
                "key": "input.bin",
            }],
            "spans": [{
                "source_id": 0,
                "logical_byte_start": 0,
                "byte_start": 0,
                "byte_length": logical_size,
            }],
        }, f)


def test_solve_proximity_hist_sectioned_lagged_runtime():
    print("\n--- solve_proximity_hist_sectioned lagged runtime ---")

    bin_path = "/src/solve_proximity_hist_sectioned"
    assert os.path.exists(bin_path), "%s not found" % bin_path
    assert open(bin_path, "rb").read(4) == b"\x7fELF", "solve_proximity_hist_sectioned is not ELF"

    r = subprocess.run(["ldd", bin_path], capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "ldd failed for solve_proximity_hist_sectioned: " + r.stderr[:200]
    assert "not found" not in r.stdout, "solve_proximity_hist_sectioned shared libs unresolved: " + r.stdout

    roots_path = "/tmp/solve_hist_lag_roots.bin"
    manifest_path = "/tmp/solve_hist_lag_manifest.json"
    cleanup(roots_path, manifest_path)
    _write_lagged_centroid_fixture(roots_path)
    roots_bytes = open(roots_path, "rb").read()
    _RangeHandler.file_bytes = roots_bytes
    with socketserver.TCPServer(("127.0.0.1", 0), _RangeHandler) as httpd:
        port = httpd.server_address[1]
        thread = threading.Thread(target=httpd.serve_forever, daemon=True)
        thread.start()
        try:
            _write_single_source_manifest(manifest_path, len(roots_bytes), 16, port)
            r = subprocess.run([
                bin_path,
                "--input_mode=multispan_sectioned",
                "--input_manifest=" + manifest_path,
                "--degree=2",
                "--hist_bins=10",
                "--threads=2",
                "--step_count=2",
                "--prelude_rows=1",
                "--score_metrics=centroid_re",
                "--score_clip_los=0",
                "--score_clip_his=10",
                "--score_program=m0-0;m0-1;abs_diff",
            ], capture_output=True, text=True, timeout=10)
            assert r.returncode == 0, "solve_proximity_hist_sectioned lagged failed: " + r.stderr[:200]
            meta = json.loads(r.stdout)
            assert meta["n_solves"] == 2, "lagged hist counted prelude as scored row"
            assert meta["hist"][5] == 2, "lagged hist did not use previous-row centroid diff: %r" % meta["hist"]
            print("  solve_proximity_hist_sectioned lagged: OK")
        finally:
            httpd.shutdown()
            thread.join(timeout=5)
    cleanup(roots_path, manifest_path)
    print("=== solve_proximity_hist_sectioned lagged runtime PASSED ===")


def test_palette_chunk_mt_lagged_multispan_runtime():
    print("\n--- solve_palette_chunk_mt lagged multispan runtime ---")

    bin_path = "/src/solve_palette_chunk_mt"
    roots_path = "/tmp/palette_chunk_mt_lag_roots.bin"
    manifest_path = "/tmp/palette_chunk_mt_lag_manifest.json"
    scores_path = "/tmp/palette_chunk_mt_lag_scores.bin"
    bins_path = "/tmp/palette_chunk_mt_lag_bins.bin"
    cleanup(roots_path, manifest_path, scores_path, bins_path)
    _write_lagged_centroid_fixture(roots_path)
    roots_bytes = open(roots_path, "rb").read()
    _RangeHandler.file_bytes = roots_bytes
    with socketserver.TCPServer(("127.0.0.1", 0), _RangeHandler) as httpd:
        port = httpd.server_address[1]
        thread = threading.Thread(target=httpd.serve_forever, daemon=True)
        thread.start()
        try:
            _write_single_source_manifest(manifest_path, len(roots_bytes), 16, port)
            r = subprocess.run([
                bin_path, "/tmp/palette_chunk_mt_lag_unused.bin",
                "--degree=2",
                "--cuts=0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9",
                "--step_count=2",
                "--threads=2",
                "--input_mode=multispan_sectioned",
                "--input_manifest=" + manifest_path,
                "--prelude_rows=1",
                "--score_metrics=centroid_re",
                "--score_clip_los=0",
                "--score_clip_his=10",
                "--score_program=m0-0;m0-1;abs_diff",
                "--scores_out=" + scores_path,
                "--bins_out=" + bins_path,
            ], capture_output=True, text=True, timeout=10)
            assert r.returncode == 0, "solve_palette_chunk_mt lagged failed: " + r.stderr[:200]
            meta = json.loads(r.stdout)
            assert meta["n_samples"] == 2, "lagged palette chunk counted prelude as scored row"
            scores = read_f32_array(scores_path)
            bins = list(open(bins_path, "rb").read())
            assert all(abs(s - 0.5) < 1e-6 for s in scores), "lagged palette scores wrong: %r" % scores
            assert bins == [4, 4], "lagged palette bins wrong: %r" % bins
            print("  solve_palette_chunk_mt lagged: OK")
        finally:
            httpd.shutdown()
            thread.join(timeout=5)
    cleanup(roots_path, manifest_path, scores_path, bins_path)
    print("=== solve_palette_chunk_mt lagged multispan runtime PASSED ===")


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
    fragment_prefix = "/tmp/roots2pix_mt_fragment"
    cleanup(roots_path, manifest_path, fragment_prefix + ".frag")

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
                "--min_re=-2",
                "--max_re=2",
                "--min_im=-2",
                "--max_im=2",
                "--degree=1",
                "--rotation=0",
                "--threads=2",
                "--step_count=3",
                "--input_manifest=" + manifest_path,
                "--retries=2",
                "--score_metrics=centroid_re",
                "--score_clip_los=-1",
                "--score_clip_his=1",
                "--score_program=m0-0",
                "--fragment_prefix=" + fragment_prefix,
            ], capture_output=True, text=True, timeout=10)
            assert r.returncode == 0, "roots2pix_mt failed: " + r.stderr[:200]
            meta = json.loads(r.stdout)
            assert meta["threads"] == 2, "roots2pix_mt did not report thread count"
            assert meta["input_mode"] == "multispan_sectioned", "roots2pix_mt did not report input_mode"
            assert meta["roots_plotted"] == 2, "roots2pix_mt plotted unexpected solve count"
            assert meta["roots_clipped"] == 1, "roots2pix_mt clipped unexpected solve count"

            expected = encode_fragment_pairs([(36, 128), (38, 255)])
            with open(fragment_prefix + ".frag", "rb") as f:
                assert f.read() == expected, "roots2pix_mt fragment payload mismatch"
            print("  roots2pix_mt: OK (threads=%d, roots_plotted=%d)" % (meta["threads"], meta["roots_plotted"]))
        finally:
            httpd.shutdown()
            thread.join(timeout=5)

    cleanup(roots_path, manifest_path, fragment_prefix + ".frag")
    print("=== roots2pix_mt multispan runtime PASSED ===")


def test_roots2pix_mt_lagged_score_runtime():
    print("\n--- roots2pix_mt lagged solve-score runtime ---")

    bin_path = "/src/roots2pix_mt"
    roots_path = "/tmp/roots2pix_mt_lag_roots.bin"
    manifest_path = "/tmp/roots2pix_mt_lag_manifest.json"
    fragment_prefix = "/tmp/roots2pix_mt_lag_fragment"
    step_scores_path = "/tmp/roots2pix_mt_lag_step_scores.bin"
    cleanup(roots_path, manifest_path, fragment_prefix + ".frag", step_scores_path)

    # First row is the section prelude. The three scored rows should compare
    # against [prelude, previous scored row, previous scored row].
    roots_bytes = bytearray()
    for re_val, im_val in [(0.0, 0.0), (1.0, 0.0), (-1.0, 0.5), (1.0, 0.5)]:
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
                    "solve_count": 4,
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
                "/tmp/roots2pix_mt_lag_pix",
                "--pix=8",
                "--min_re=-2",
                "--max_re=2",
                "--min_im=-2",
                "--max_im=2",
                "--degree=1",
                "--rotation=0",
                "--threads=2",
                "--step_count=3",
                "--prelude_rows=1",
                "--input_manifest=" + manifest_path,
                "--retries=2",
                "--score_metrics=centroid_re",
                "--score_clip_los=-1",
                "--score_clip_his=1",
                "--score_program=m0-0;m0-1;abs_diff",
                "--fragment_prefix=" + fragment_prefix,
                "--step_scores_output=" + step_scores_path,
            ], capture_output=True, text=True, timeout=10)
            assert r.returncode == 0, "roots2pix_mt lagged run failed: " + r.stderr[:200]
            meta = json.loads(r.stdout)
            assert meta["threads"] == 2, "lagged roots2pix_mt did not report thread count"
            assert meta["roots_plotted"] == 3, "lagged roots2pix_mt plotted unexpected solve count"

            with open(step_scores_path, "rb") as f:
                assert list(f.read()) == [128, 255, 255], "lagged step scores did not use prelude/previous rows"
            print("  roots2pix_mt lagged score: OK")
        finally:
            httpd.shutdown()
            thread.join(timeout=5)

    cleanup(roots_path, manifest_path, fragment_prefix + ".frag", step_scores_path)
    print("=== roots2pix_mt lagged solve-score runtime PASSED ===")


def test_roots2pix_mt_score_output_normalization_runtime():
    print("\n--- roots2pix_mt score output normalization runtime ---")

    bin_path = "/src/roots2pix_mt"
    roots_path = "/tmp/roots2pix_mt_norm_roots.bin"
    manifest_path = "/tmp/roots2pix_mt_norm_manifest.json"
    fragment_prefix = "/tmp/roots2pix_mt_norm_fragment"
    step_scores_path = "/tmp/roots2pix_mt_norm_step_scores.bin"
    cleanup(roots_path, manifest_path, fragment_prefix + ".frag", step_scores_path)

    roots_bytes = bytearray()
    for re_val, im_val in [(0.2, 0.0), (0.4, 0.0)]:
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
                    "solve_count": 2,
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
                "/tmp/roots2pix_mt_norm_pix",
                "--pix=8",
                "--min_re=-1",
                "--max_re=1",
                "--min_im=-1",
                "--max_im=1",
                "--degree=1",
                "--rotation=0",
                "--threads=1",
                "--step_count=2",
                "--input_manifest=" + manifest_path,
                "--retries=2",
                "--score_metrics=centroid_re",
                "--score_clip_los=0",
                "--score_clip_his=1",
                "--score_program=m0-0",
                "--score_output_normalize=1",
                "--score_output_clip_lo=0.2",
                "--score_output_clip_hi=0.4",
                "--fragment_prefix=" + fragment_prefix,
                "--step_scores_output=" + step_scores_path,
            ], capture_output=True, text=True, timeout=10)
            assert r.returncode == 0, "roots2pix_mt score normalization failed: " + r.stderr[:200]
            with open(step_scores_path, "rb") as f:
                assert list(f.read()) == [1, 255], "score output normalization did not expand bytes"
            print("  roots2pix_mt score output normalization: OK")
        finally:
            httpd.shutdown()
            thread.join(timeout=5)

    cleanup(roots_path, manifest_path, fragment_prefix + ".frag", step_scores_path)
    print("=== roots2pix_mt score output normalization runtime PASSED ===")


def test_roots2pix_mt_explicit_rgb_outputs_runtime():
    print("\n--- roots2pix_mt explicit RGB output runtime ---")

    bin_path = "/src/roots2pix_mt"
    roots_path = "/tmp/roots2pix_mt_rgb_roots.bin"
    manifest_path = "/tmp/roots2pix_mt_rgb_manifest.json"
    fragment_prefix = "/tmp/roots2pix_mt_rgb_fragment"
    step_scores_path = "/tmp/roots2pix_mt_rgb_step_scores.raw"
    raw_path = "/tmp/roots2pix_mt_rgb.raw"
    palette_raw_path = "/tmp/roots2pix_mt_rgb_palette.raw"
    hist_path = "/tmp/roots2pix_mt_rgb_hist.json"
    png_path = "/tmp/roots2pix_mt_rgb.png"
    preview_path = "/tmp/roots2pix_mt_rgb_preview.png"
    hsv_png_path = "/tmp/roots2pix_mt_hsv.png"
    rgb_lut_png_path = "/tmp/roots2pix_mt_rgb_lut.png"
    rgb_lut_palette_png_path = "/tmp/roots2pix_mt_rgb_lut_palette.png"
    rgb_lut_identity_png_path = "/tmp/roots2pix_mt_rgb_lut_identity.png"
    hsv_lut_png_path = "/tmp/roots2pix_mt_hsv_lut.png"
    hsv_lut_palette_png_path = "/tmp/roots2pix_mt_hsv_lut_palette.png"
    hsv_lut_identity_png_path = "/tmp/roots2pix_mt_hsv_lut_identity.png"
    cleanup(
        roots_path,
        manifest_path,
        fragment_prefix + ".frag",
        step_scores_path,
        raw_path,
        palette_raw_path,
        hist_path,
        png_path,
        preview_path,
        hsv_png_path,
        rgb_lut_png_path,
        rgb_lut_palette_png_path,
        rgb_lut_identity_png_path,
        hsv_lut_png_path,
        hsv_lut_palette_png_path,
        hsv_lut_identity_png_path,
    )

    roots_bytes = bytearray()
    for re_val, im_val in [(0.0, 0.0), (0.5, 0.0)]:
        roots_bytes.extend(struct.pack("<ff", re_val, im_val))
    with open(roots_path, "wb") as f:
        f.write(roots_bytes)
    with open(manifest_path, "w", encoding="utf-8") as f:
        json.dump({
            "source_family": "slv",
            "logical_size": len(roots_bytes),
            "row_bytes": 8,
            "solve_start": 0,
            "solve_count": 2,
            "sources": [{"id": 0, "url": "file://" + roots_path, "key": roots_path}],
            "spans": [{
                "source_id": 0,
                "logical_byte_start": 0,
                "byte_start": 0,
                "byte_length": len(roots_bytes),
            }],
        }, f)

    try:
        r = subprocess.run([
            bin_path,
            "/tmp/roots2pix_mt_rgb_pix",
            "--pix=8",
            "--min_re=-1",
            "--max_re=1",
            "--min_im=-1",
            "--max_im=1",
            "--degree=1",
            "--rotation=0",
            "--threads=1",
            "--step_count=2",
            "--input_manifest=" + manifest_path,
            "--retries=0",
            "--score_metrics=centroid_re",
            "--score_clip_los=-1",
            "--score_clip_his=1",
            "--score_program=m0-0;emit_none;flush;m0-0;emit_norm;m0-0;flip;emit_norm;m0-0;sawtooth:2;emit",
            "--score_output_clip_los=0.5,0.25,0",
            "--score_output_clip_his=0.75,0.5,1",
            "--fragment_prefix=" + fragment_prefix,
            "--step_scores_output=" + step_scores_path,
        ], capture_output=True, text=True, timeout=10)
        assert r.returncode == 0, "roots2pix_mt explicit RGB failed: " + r.stderr[:200]
        meta = json.loads(r.stdout or "{}")
        assert meta.get("fragment_channels") == 3, "roots2pix_mt did not report 3 fragment channels"
        assert meta.get("fragment_record_size_bytes") == 7, "roots2pix_mt did not report 7-byte RGB records"
        assert meta.get("step_score_channels") == 3, "roots2pix_mt did not report 3 step-score channels"
        assert meta.get("roots_plotted") == 2, "explicit RGB roots2pix plotted unexpected root count"

        expected = bytearray()
        expected.extend((36).to_bytes(4, "little"))
        expected.extend([0, 255, 0])
        expected.extend((38).to_bytes(4, "little"))
        expected.extend([255, 0, 128])
        with open(fragment_prefix + ".frag", "rb") as f:
            assert f.read() == bytes(expected), "explicit RGB fragment payload mismatch"
        with open(step_scores_path, "rb") as f:
            assert f.read() == bytes([0, 255, 0, 255, 0, 128]), "explicit RGB step_scores payload mismatch"

        with open(step_scores_path, "wb") as f:
            f.write(bytes([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]))
        r = subprocess.run([
            "/src/step_scores_to_palette_raw",
            "--input=" + step_scores_path,
            "--output=" + palette_raw_path,
            "--grid-n=2",
            "--step-count=4",
            "--channels=3",
        ], capture_output=True, text=True, timeout=10)
        assert r.returncode == 0, "step_scores_to_palette_raw RGB failed: " + r.stderr[:200]
        with open(palette_raw_path, "rb") as f:
            assert f.read() == bytes([1, 2, 3, 4, 5, 6, 10, 11, 12, 7, 8, 9]), "RGB palette raw serpentine payload mismatch"

        r = subprocess.run([
            "/src/assemble_greyscale",
            "--pix=8",
            "--channels=3",
            "--allow-zero=1",
            "--output=" + raw_path,
            "--hist-output=" + hist_path,
            "--workers=1",
            fragment_prefix + ".frag",
        ], capture_output=True, text=True, timeout=10)
        assert r.returncode == 0, "assemble_greyscale RGB failed: " + r.stderr[:200]
        with open(raw_path, "rb") as f:
            raw = f.read()
        assert len(raw) == 8 * 8 * 3, "assembled RGB raw size mismatch"
        assert raw[36 * 3:36 * 3 + 3] == b"\x00\xff\x00", "assembled RGB pixel 36 mismatch"
        assert raw[38 * 3:38 * 3 + 3] == b"\xff\x00\x80", "assembled RGB pixel 38 mismatch"
        hist = json.load(open(hist_path, "r", encoding="utf-8"))
        assert hist["channels"] == 3, "RGB histogram did not report channel count"
        assert hist["nonzero_pixels"] == 2, "RGB histogram nonzero count mismatch"

        r = subprocess.run([
            "/src/score_raw_render",
            raw_path,
            png_path,
            "--pix=8",
            "--channels=3",
            "--interpretation=rgb",
            "--background_color=123456",
            "--quality=90",
            "--preview=" + preview_path,
            "--preview_max=512",
        ], capture_output=True, text=True, timeout=20)
        assert r.returncode == 0, "score_raw_render RGB failed: " + r.stderr[:200]
        assert read_png_dims(png_path) == (8, 8), "direct RGB PNG dims mismatch"
        assert read_png_dims(preview_path) == (8, 8), "direct RGB preview dims mismatch"
        assert read_rgb_pixel_with_vips(png_path, 0, 0) == (18, 52, 86), (
            "direct RGB reserved-zero background did not render background_color"
        )
        r = subprocess.run([
            "/src/score_raw_render",
            raw_path,
            hsv_png_path,
            "--pix=8",
            "--channels=3",
            "--interpretation=hsv",
            "--background_color=123456",
            "--quality=90",
        ], capture_output=True, text=True, timeout=20)
        assert r.returncode == 0, "score_raw_render HSV failed: " + r.stderr[:200]
        assert read_png_dims(hsv_png_path) == (8, 8), "HSV PNG dims mismatch"
        assert read_rgb_pixel_with_vips(hsv_png_path, 0, 0) == (18, 52, 86), (
            "HSV reserved-zero background did not render background_color"
        )
        r = subprocess.run([
            "/src/score_raw_render",
            raw_path,
            rgb_lut_png_path,
            "--pix=8",
            "--channels=3",
            "--interpretation=rgb_lut",
            "--palette=turbo",
            "--background_color=123456",
            "--quality=90",
        ], capture_output=True, text=True, timeout=20)
        assert r.returncode == 0, "score_raw_render RGB LUT failed: " + r.stderr[:200]
        assert read_png_dims(rgb_lut_png_path) == (8, 8), "RGB LUT PNG dims mismatch"
        assert read_rgb_pixel_with_vips(rgb_lut_png_path, 0, 0) == (18, 52, 86), (
            "RGB LUT reserved-zero background did not render background_color"
        )
        r = subprocess.run([
            "/src/score_raw_render",
            raw_path,
            rgb_lut_palette_png_path,
            "--pix=8",
            "--channels=3",
            "--interpretation=rgb_lut",
            "--palette=turbo",
            "--background_color=123456",
            "--zero_background=0",
            "--quality=90",
        ], capture_output=True, text=True, timeout=20)
        assert r.returncode == 0, "score_raw_render RGB LUT palette-mode failed: " + r.stderr[:200]
        assert read_rgb_pixel_with_vips(rgb_lut_palette_png_path, 0, 0) == (48, 18, 59), (
            "RGB LUT palette-mode zero should sample the palette left edge"
        )
        r = subprocess.run([
            "/src/score_raw_render",
            raw_path,
            rgb_lut_identity_png_path,
            "--pix=8",
            "--channels=3",
            "--interpretation=rgb_lut",
            "--palette=identity",
            "--background_color=123456",
            "--zero_background=0",
            "--quality=90",
        ], capture_output=True, text=True, timeout=20)
        assert r.returncode == 0, "score_raw_render RGB LUT identity failed: " + r.stderr[:200]
        assert read_rgb_pixel_with_vips(rgb_lut_identity_png_path, 6, 4) == (255, 0, 128), (
            "identity RGB LUT should map byte channels to identical RGB component bytes"
        )
        r = subprocess.run([
            "/src/score_raw_render",
            raw_path,
            rgb_lut_identity_png_path,
            "--pix=8",
            "--channels=3",
            "--interpretation=rgb_lut",
            "--palette=identity",
            "--background_color=123456",
            "--quality=90",
        ], capture_output=True, text=True, timeout=20)
        assert r.returncode == 0, "score_raw_render RGB LUT identity with background failed: " + r.stderr[:200]
        assert read_rgb_pixel_with_vips(rgb_lut_identity_png_path, 0, 0) == (18, 52, 86), (
            "identity RGB LUT all-zero pixel should render background_color"
        )
        assert read_rgb_pixel_with_vips(rgb_lut_identity_png_path, 4, 4) == (0, 255, 0), (
            "identity RGB LUT should not treat a single zero channel as background"
        )
        assert read_rgb_pixel_with_vips(rgb_lut_identity_png_path, 6, 4) == (255, 0, 128), (
            "identity RGB LUT should remain byte-identical when zero_background is enabled"
        )
        r = subprocess.run([
            "/src/score_raw_render",
            raw_path,
            hsv_lut_png_path,
            "--pix=8",
            "--channels=3",
            "--interpretation=hsv_lut",
            "--palette=turbo",
            "--background_color=123456",
            "--quality=90",
        ], capture_output=True, text=True, timeout=20)
        assert r.returncode == 0, "score_raw_render HSV LUT failed: " + r.stderr[:200]
        assert read_png_dims(hsv_lut_png_path) == (8, 8), "HSV LUT PNG dims mismatch"
        assert read_rgb_pixel_with_vips(hsv_lut_png_path, 0, 0) == (18, 52, 86), (
            "HSV LUT reserved-zero background did not render background_color"
        )
        r = subprocess.run([
            "/src/score_raw_render",
            raw_path,
            hsv_lut_palette_png_path,
            "--pix=8",
            "--channels=3",
            "--interpretation=hsv_lut",
            "--palette=turbo",
            "--background_color=123456",
            "--zero_background=0",
            "--quality=90",
        ], capture_output=True, text=True, timeout=20)
        assert r.returncode == 0, "score_raw_render HSV LUT palette-mode failed: " + r.stderr[:200]
        assert read_rgb_pixel_with_vips(hsv_lut_palette_png_path, 0, 0) == (48, 18, 59), (
            "HSV LUT palette-mode zero should sample the palette left edge"
        )
        r = subprocess.run([
            "/src/score_raw_render",
            raw_path,
            hsv_lut_identity_png_path,
            "--pix=8",
            "--channels=3",
            "--interpretation=hsv_lut",
            "--palette=identity_hsv",
            "--background_color=123456",
            "--zero_background=0",
            "--quality=90",
        ], capture_output=True, text=True, timeout=20)
        assert r.returncode == 0, "score_raw_render HSV LUT identity_hsv failed: " + r.stderr[:200]
        assert read_rgb_pixel_with_vips(hsv_lut_identity_png_path, 6, 4) == read_rgb_pixel_with_vips(hsv_png_path, 6, 4), (
            "identity_hsv HSV LUT should match direct HSV at byte-aligned sample points"
        )
        r = subprocess.run([
            "/src/score_raw_render",
            raw_path,
            hsv_lut_identity_png_path,
            "--pix=8",
            "--channels=3",
            "--interpretation=hsv_lut",
            "--palette=identity_hsv",
            "--background_color=123456",
            "--quality=90",
        ], capture_output=True, text=True, timeout=20)
        assert r.returncode == 0, "score_raw_render HSV LUT identity_hsv with background failed: " + r.stderr[:200]
        assert read_rgb_pixel_with_vips(hsv_lut_identity_png_path, 0, 0) == (18, 52, 86), (
            "identity_hsv HSV LUT all-zero pixel should render background_color"
        )
        assert read_rgb_pixel_with_vips(hsv_lut_identity_png_path, 6, 4) == read_rgb_pixel_with_vips(hsv_png_path, 6, 4), (
            "identity_hsv HSV LUT should remain direct-HSV-compatible when zero_background is enabled"
        )
        print("  roots2pix_mt explicit RGB outputs: OK")
    finally:
        cleanup(
            roots_path,
            manifest_path,
            fragment_prefix + ".frag",
            raw_path,
            hist_path,
            png_path,
            preview_path,
            hsv_png_path,
            rgb_lut_png_path,
            rgb_lut_palette_png_path,
            rgb_lut_identity_png_path,
            hsv_lut_png_path,
            hsv_lut_palette_png_path,
            hsv_lut_identity_png_path,
        )
    print("=== roots2pix_mt explicit RGB output runtime PASSED ===")


def test_roots2pix_mt_local_file_manifest_runtime():
    print("\n--- roots2pix_mt local file manifest runtime ---")

    bin_path = "/src/roots2pix_mt"
    roots_path = "/tmp/roots2pix_mt_local_roots.bin"
    manifest_path = "/tmp/roots2pix_mt_local_manifest.json"
    fragment_prefix = "/tmp/roots2pix_mt_local_fragment"
    step_scores_path = "/tmp/roots2pix_mt_local_scores.bin"
    roots_bytes = struct.pack("<ffff", -0.5, 0.0, 0.5, 0.0)

    try:
        with open(roots_path, "wb") as f:
            f.write(roots_bytes)
        with open(manifest_path, "w") as f:
            json.dump({
                "source_family": "slv",
                "logical_size": len(roots_bytes),
                "row_bytes": 8,
                "solve_start": 0,
                "solve_count": 2,
                "sources": [{"id": 0, "url": "file://" + roots_path, "key": roots_path}],
                "spans": [{
                    "source_id": 0,
                    "logical_byte_start": 0,
                    "byte_start": 0,
                    "byte_length": len(roots_bytes),
                }],
            }, f)

        r = subprocess.run([
            bin_path,
            "/tmp/roots2pix_mt_local_pix",
            "--pix=8",
            "--min_re=-1",
            "--max_re=1",
            "--min_im=-1",
            "--max_im=1",
            "--degree=1",
            "--rotation=0",
            "--threads=1",
            "--step_count=2",
            "--input_manifest=" + manifest_path,
            "--retries=0",
            "--score_metrics=centroid_re",
            "--score_clip_los=-1",
            "--score_clip_his=1",
            "--score_program=m0-0",
            "--fragment_prefix=" + fragment_prefix,
            "--step_scores_output=" + step_scores_path,
        ], capture_output=True, text=True, timeout=10)
        assert r.returncode == 0, "roots2pix_mt local file manifest failed: " + r.stderr[:200]
        meta = json.loads(r.stdout or "{}")
        assert meta.get("input_mode") == "multispan_sectioned", "unexpected roots2pix input_mode"
        with open(step_scores_path, "rb") as f:
            assert len(f.read()) == 2, "local file manifest did not emit step scores"
        print("  roots2pix_mt file:// multispan source: OK")
    finally:
        cleanup(roots_path, manifest_path, fragment_prefix + ".frag", step_scores_path)
    print("=== roots2pix_mt local file manifest runtime PASSED ===")


def test_render_lores_preview_handler_runtime():
    print("\n--- render-lores-preview handler runtime ---")

    for bin_path in (
        "/src/solve_proximity_stats",
        "/src/roots2pix_mt",
        "/src/score_raw_render",
        "/src/sweep_coeffgen",
        "/src/sweep_mt",
        "/src/sweep_cm",
    ):
        assert os.path.exists(bin_path), "%s not found" % bin_path
        assert open(bin_path, "rb").read(4) == b"\x7fELF", "%s is not ELF" % bin_path
        r = subprocess.run(["ldd", bin_path], capture_output=True, text=True, timeout=10)
        if r.returncode == 0:
            assert "not found" not in (r.stdout + r.stderr), "%s has missing shared libs:\n%s" % (bin_path, r.stdout + r.stderr)
    print("  ldd/binary presence: OK")

    if "boto3" not in sys.modules:
        sys.modules["boto3"] = types.SimpleNamespace(client=lambda *_args, **_kwargs: object())
    if "/src" not in sys.path:
        sys.path.insert(0, "/src")

    import handler_render_lores_preview as mod

    fake_s3 = _MemS3()
    roots_key = "renders/job-preview/lores.bin"
    roots_bytes = bytearray()
    for idx in range(32):
        re_val = -0.8 + (1.6 * idx / 31.0)
        roots_bytes.extend(struct.pack("<ff", re_val, 0.0))
    fake_s3.seed_object(roots_key, bytes(roots_bytes), content_type="application/octet-stream")

    _MemS3ObjectHandler.objects = fake_s3.objects
    with socketserver.TCPServer(("127.0.0.1", 0), _MemS3ObjectHandler) as httpd:
        port = httpd.server_address[1]
        fake_s3.base_url = "http://127.0.0.1:%d" % port
        thread = threading.Thread(target=httpd.serve_forever, daemon=True)
        thread.start()
        old_s3 = mod.s3
        try:
            mod.s3 = fake_s3
            result = mod.handler({
                "body": json.dumps({
                    "job_id": "job-preview",
                    "degree": 1,
                    "preview_pix": 64,
                    "quality": 90,
                    "palette": "inferno",
                    "view_mode": "explicit",
                    "min_re": -1.0,
                    "max_re": 1.0,
                    "min_im": -1.0,
                    "max_im": 1.0,
                    "rotation": 0.0,
                    "solve_score_chain": [["centroid_re", "slv", "0.1"]],
                    "solve_score_normalize": True,
                    "lores_bin_key": roots_key,
                    "lores_N": 4,
                    "root_transforms": [],
                    "raster_mt_threads": 1,
                    "solve_score_threads": 1,
                    "raster_sectioned_retries": 1,
                })
            }, None)
        finally:
            mod.s3 = old_s3
            httpd.shutdown()
            thread.join(timeout=5)

    assert result["statusCode"] == 200, "render-lores-preview handler failed: %r" % result
    body = json.loads(result["body"])
    assert body["content_type"] == "image/png", "unexpected content type %r" % body.get("content_type")
    png = base64.b64decode(body["image_base64"])
    palette_png = base64.b64decode(body["palette_image_base64"])
    png_path = "/tmp/render_lores_preview_runtime.png"
    palette_png_path = "/tmp/render_lores_preview_palette_runtime.png"
    with open(png_path, "wb") as f:
        f.write(png)
    with open(palette_png_path, "wb") as f:
        f.write(palette_png)
    assert read_png_dims(png_path) == (64, 64), "unexpected render-lores-preview PNG dimensions"
    assert read_png_dims(palette_png_path) == (4, 4), "unexpected render-lores-preview palette PNG dimensions"
    assert body["palette_pix"] == 4, "unexpected preview palette pix %r" % body.get("palette_pix")
    assert body["palette_fragment_entries"] == 16, "unexpected preview palette entry count"
    assert body["preview_pix"] == 64, "unexpected preview_pix %r" % body.get("preview_pix")
    assert body["raster"]["input_mode"] == "multispan_sectioned", "roots2pix path was not sectioned"
    assert body["raster"]["roots_plotted"] > 0, "preview plotted no roots"
    assert body["solve_score"]["score_output_normalize"] is True, "score normalization was not active"
    assert body["solve_score"]["score_output_clip_source"] == "lores_q05_q95", "unexpected clip source"
    assert fake_s3.puts == [], "ephemeral preview wrote durable S3 objects: %r" % fake_s3.puts
    cleanup(png_path, palette_png_path)
    print("  handler_render_lores_preview: OK (%d PNG bytes)" % len(png))

    fake_s3 = _MemS3()
    fake_s3.seed_object("renders/job-preview-recompute/calc.json", json.dumps({
        "N": 5,
        "times": 1,
        "degree": 24,
        "n_coeffs": 25,
        "solver": "aberth_mt",
        "lores": {"N": 5, "n_steps": 25},
        "pipeline": {
            "function": "g1",
            "param_transforms": [],
            "coeff_transforms": [],
            "cfpv": [],
        },
    }).encode("utf-8"), content_type="application/json")
    old_s3 = mod.s3
    try:
        mod.s3 = fake_s3
        result = mod.handler({
            "body": json.dumps({
                "job_id": "job-preview-recompute",
                "degree": 24,
                "n_coeffs": 25,
                "preview_pix": 32,
                "quality": 90,
                "palette": "inferno",
                "view_mode": "explicit",
                "min_re": -2.0,
                "max_re": 2.0,
                "min_im": -2.0,
                "max_im": 2.0,
                "rotation": 0.0,
                "solve_score_chain": [["centroid_re", "slv", "0.1"]],
                "solve_score_normalize": False,
                "preview_source_mode": "recompute",
                "preview_source_size": 5,
                "lores_bin_key": "",
                "root_transforms": [],
                "raster_mt_threads": 1,
                "solve_score_threads": 1,
                "raster_sectioned_retries": 1,
            })
        }, None)
    finally:
        mod.s3 = old_s3

    assert result["statusCode"] == 200, "render-lores-preview recompute failed: %r" % result
    body = json.loads(result["body"])
    assert body["source"]["mode"] == "recompute", "unexpected recompute source mode"
    assert body["source"]["view_N"] == 5, "unexpected recompute view_N"
    assert any("Recompute preview materialize:" in line for line in body.get("logs", [])), "missing recompute timing log"
    assert fake_s3.puts == [], "recompute preview wrote durable S3 objects: %r" % fake_s3.puts
    print("  handler_render_lores_preview recompute: OK")
    print("=== render-lores-preview handler runtime PASSED ===")


# ── Render Preview (vipsthumbnail) Tests ─────────────────────────────────

def test_render_preview():
    print("\n--- Render preview (vipsthumbnail) ---")

    vt_path = "/opt/bin/vipsthumbnail"
    vh_path = "/opt/bin/vipsheader"
    assert os.path.exists(vt_path), "vipsthumbnail not found at %s" % vt_path
    assert os.path.exists(vh_path), "vipsheader not found at %s" % vh_path
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

    r = subprocess.run([vh_path, "-f", "width", test_jpeg],
                       capture_output=True, text=True, timeout=10, env=env)
    assert r.returncode == 0, "vipsheader failed: " + r.stderr[:200]
    assert r.stdout.strip() == "8", "vipsheader width mismatch: %r" % r.stdout
    print("  vipsheader: width=8")

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


def test_coeffs_bilevel_raster_runtime():
    print("\n--- coeffs_bilevel_raster runtime ---")

    bin_path = "/src/coeffs_bilevel_raster"
    assert os.path.exists(bin_path), "coeffs_bilevel_raster not found at %s" % bin_path

    coeffs_path = "/tmp/coeffs_bilevel_rt.bin"
    out_frag = "/tmp/coeffs_bilevel_rt.frag"

    with open(coeffs_path, "wb") as f:
        for coeffs in [[(0.0, 0.0), (1.0, 0.0)], [(0.0, 0.0), (100.0, 100.0)], [(-4.25, 0.0), (0.0, 0.0)]]:
            for re, im in coeffs:
                f.write(struct.pack("<ff", re, im))

    r = subprocess.run(
        [
            bin_path,
            coeffs_path,
            out_frag,
            "--pix=8",
            "--min_re=-4",
            "--max_re=4",
            "--min_im=-4",
            "--max_im=4",
            "--n_coeffs=2",
        ],
        capture_output=True,
        text=True,
        timeout=10,
    )
    assert r.returncode == 0, "coeffs_bilevel_raster failed: " + r.stderr[:200]
    meta = json.loads(r.stdout)
    assert meta["coeffs_plotted"] == 2, "unexpected plotted count %r" % (meta,)
    assert meta["coeffs_clipped"] == 2, "unexpected clipped count %r" % (meta,)
    assert meta["coeffs_deduped"] == 2, "unexpected dedup count %r" % (meta,)

    with open(out_frag, "rb") as f:
        frag = f.read()
    assert frag == encode_fragment_pairs([(36, 1), (37, 1)]), "unexpected fragment bytes %r" % (frag,)
    print(
        "  coeffs_bilevel_raster: OK (plotted=%d, clipped=%d, dedup=%d)"
        % (meta["coeffs_plotted"], meta["coeffs_clipped"], meta["coeffs_deduped"])
    )

    cleanup(coeffs_path, out_frag)
    print("=== coeffs_bilevel_raster runtime PASSED ===")


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


def test_sheet_stitch_runtime():
    print("\n--- sheet_stitch runtime ---")

    bin_path = "/src/sheet_stitch"
    assert os.path.exists(bin_path), "sheet_stitch not found at %s" % bin_path
    env = {**os.environ, "LD_LIBRARY_PATH": "/opt/lib",
           "PATH": "/opt/bin:" + os.environ.get("PATH", "")}

    tile_paths = []
    tiles = [bytearray(16), bytearray([255] * 16), bytearray(16)]
    tiles[0][0] = 255
    tiles[1][15] = 0
    tiles[2][2 * 4 + 1] = 255
    for i, tile in enumerate(tiles):
        path = "/tmp/sheet_stitch_%d.raw" % i
        with open(path, "wb") as f:
            f.write(tile)
        tile_paths.append(path)
    list_path = "/tmp/sheet_stitch_tiles.txt"
    out_png = "/tmp/sheet_stitch_out.png"
    out_csv = "/tmp/sheet_stitch_out.csv"
    with open(list_path, "w") as f:
        f.write("\n".join(tile_paths) + "\n")

    r = subprocess.run(
        [bin_path, list_path, out_png, "4", "2", "2", "1", "0"],
        capture_output=True, text=True, timeout=20, env=env)
    assert r.returncode == 0, "sheet_stitch failed: " + r.stderr[:300]
    meta = json.loads(r.stdout)
    assert meta["width"] == 11 and meta["height"] == 11, meta
    assert meta["tiles"] == 3 and meta["bitdepth"] == 1, meta
    assert read_png_ihdr(out_png) == (11, 11, 1, 0), (
        "sheet_stitch output is not 1-bit grayscale", read_png_ihdr(out_png))

    r = subprocess.run(
        ["/opt/bin/vips", "csvsave", out_png, out_csv],
        capture_output=True, text=True, timeout=20, env=env)
    assert r.returncode == 0, "vips csvsave sheet PNG failed: " + r.stderr[:200]
    grid = read_csv_grid(out_csv)
    assert len(grid) == 11 and all(len(row) == 11 for row in grid)
    assert grid[1][1] > 0, "tile 0 marker missing"
    assert grid[1][6] > 0, "tile 1 body missing"
    assert grid[4][9] == 0, "tile 1 cleared marker missing"
    assert grid[8][2] > 0, "tile 2 marker missing"
    assert grid[6][6] == 0, "unfilled final cell must use background"
    assert all(grid[0][x] == 0 for x in range(11)), "outer margin must use background"
    assert all(grid[y][5] == 0 for y in range(11)), "inner gutter must use background"

    cleanup(*(tile_paths + [list_path, out_png, out_csv]))
    print("=== sheet_stitch runtime PASSED ===")


def test_sheet_deepzoom_grayscale_runtime():
    print("\n--- sheet DeepZoom grayscale-pyramid runtime ---")

    stitch = "/src/sheet_stitch"
    dz_export = "/src/dz_export"
    env = {**os.environ, "LD_LIBRARY_PATH": "/opt/lib",
           "PATH": "/opt/bin:" + os.environ.get("PATH", "")}
    raw_path = "/tmp/sheet_dz.raw"
    list_path = "/tmp/sheet_dz_tiles.txt"
    sheet_png = "/tmp/sheet_dz.png"
    dz_base = "/tmp/sheet_dz/image"
    tile = bytearray(64 * 64)
    for y in range(64):
        for x in range(64):
            if (x // 8 + y // 8) % 2:
                tile[y * 64 + x] = 255
    with open(raw_path, "wb") as f:
        f.write(tile)
    with open(list_path, "w") as f:
        f.write(raw_path + "\n")

    r = subprocess.run(
        [stitch, list_path, sheet_png, "64", "1", "1", "0", "0"],
        capture_output=True, text=True, timeout=20, env=env)
    assert r.returncode == 0, "sheet_stitch fixture failed: " + r.stderr[:300]
    r = subprocess.run(
        [dz_export, sheet_png, dz_base],
        capture_output=True, text=True, timeout=20, env=env)
    assert r.returncode == 0, "sheet dz_export failed: " + r.stderr[:300]
    meta = json.loads(r.stdout)
    assert meta["bitdepth"] == 8, meta
    tile_paths = []
    for root, _dirs, files in os.walk(dz_base + "_files"):
        tile_paths.extend(os.path.join(root, name) for name in files
                          if name.endswith(".png"))
    assert tile_paths, "sheet dz_export produced no PNG tiles"
    for path in tile_paths:
        _w, _h, bitdepth, color_type = read_png_ihdr(path)
        assert (bitdepth, color_type) == (8, 0), (
            "DeepZoom tile is not 8-bit grayscale", path,
            (bitdepth, color_type))

    import shutil
    cleanup(raw_path, list_path, sheet_png, dz_base + ".dzi")
    shutil.rmtree(dz_base + "_files", ignore_errors=True)
    print("=== sheet DeepZoom grayscale-pyramid runtime PASSED ===")


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
        score_program="m0-0",
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


def _write_roots_bin(path, solves, degree):
    with open(path, "wb") as f:
        for roots in solves:
            padded = list(roots) + [(0.0, 0.0)] * (degree - len(roots))
            for re, im in padded[:degree]:
                f.write(struct.pack("<ff", float(re), float(im)))


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
    for program in ["m0-0;m1-0;mul", "m0-0;m1-0;max"]:
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

    for program in ["m0-0;flip", "m0-0;sawtooth:10", "m0-0;omega_cosine:3:1.57079632679", "m0-0;omega_cosine:25:0.25"]:
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

    def _score_program_summary(program):
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
        return json.loads(r.stdout)

    omega10 = _score_program_summary("m0-0;omega_cosine:10:0.25")
    omega25 = _score_program_summary("m0-0;omega_cosine:25:0.25")
    assert abs(omega10["mean_score"] - omega25["mean_score"]) > 1e-6, "omega_cosine frequency appears clamped at 10"
    print("  score program omega_cosine uncapped frequency: OK")

    r = subprocess.run(
        [
            sps_path,
            sps_bin,
            "--mode=summary",
            "--degree=2",
            "--score_metrics=proximity",
            "--score_clip_los=" + str(clip["clip_lo"]),
            "--score_clip_his=" + str(clip["clip_hi"]),
            "--score_program=m0-0;m0-1;abs_diff",
        ],
        capture_output=True,
        text=True,
        timeout=10,
    )
    assert r.returncode == 0, "lagged summary failed: " + r.stderr[:200]
    summary = json.loads(r.stdout)
    assert summary["n_solves"] == 3
    assert summary["min_score"] == 0.0
    print("  score program m0-0;m0-1;abs_diff: OK")

    r = subprocess.run(
        [
            sps_path,
            sps_bin,
            "--mode=summary",
            "--degree=2",
            "--score_metrics=proximity",
            "--score_clip_los=" + str(clip["clip_lo"]),
            "--score_clip_his=" + str(clip["clip_hi"]),
            "--score_program=m0-0",
            "--score_output_normalize=1",
        ],
        capture_output=True,
        text=True,
        timeout=10,
    )
    assert r.returncode == 0, "score-output normalized summary failed: " + r.stderr[:200]
    summary = json.loads(r.stdout)
    assert summary["raw_hist_space"] == "score_output_normalized"
    assert summary["raw_hist_score_output_normalize"] is True
    print("  score-output normalized raw histogram: OK")

    r = subprocess.run(
        [
            sps_path,
            sps_bin,
            "--mode=summary",
            "--degree=2",
            "--score_metrics=proximity",
            "--score_clip_los=" + str(clip["clip_lo"]),
            "--score_clip_his=" + str(clip["clip_hi"]),
            "--score_program=m0-0;emit_norm;m0-0;flip;emit",
        ],
        capture_output=True,
        text=True,
        timeout=10,
    )
    assert r.returncode == 0, "explicit-output summary failed: " + r.stderr[:200]
    summary = json.loads(r.stdout)
    assert summary["score_output_channel_count"] == 2, "explicit-output summary channel count mismatch"
    assert summary["score_output_has_explicit_outputs"] is True, "explicit-output summary did not mark explicit outputs"
    channels = summary["score_output_channels"]
    assert len(channels) == 2, "explicit-output summary missing channel rows"
    assert channels[0]["range_normalized"] is True, "emit_norm channel not marked normalized"
    assert channels[1]["range_normalized"] is False, "emit channel incorrectly marked normalized"
    print("  explicit-output summary channels: OK")

    r = subprocess.run(
        [
            sps_path,
            sps_bin,
            "--mode=summary",
            "--degree=2",
            "--score_metrics=proximity,spread",
            "--score_clip_los=" + str(clip["clip_lo"]) + "," + str(clip["clip_lo"]),
            "--score_clip_his=" + str(clip["clip_hi"]) + "," + str(clip["clip_hi"]),
            "--score_program=m0-0;emit_none;flush;m1-0;emit_norm",
        ],
        capture_output=True,
        text=True,
        timeout=10,
    )
    assert r.returncode == 0, "emit_none/flush summary failed: " + r.stderr[:200]
    summary = json.loads(r.stdout)
    assert summary["score_output_channel_count"] == 1, "emit_none allocated an output channel"
    assert summary["score_output_has_explicit_outputs"] is True, "emit_none did not keep explicit output mode"
    assert summary["score_output_channels"][0]["range_normalized"] is True, "post-flush emit_norm metadata wrong"
    print("  emit_none + flush score program: OK")

    r = subprocess.run(
        [
            sps_path,
            sps_bin,
            "--mode=summary",
            "--degree=2",
            "--score_metrics=proximity",
            "--score_clip_los=" + str(clip["clip_lo"]),
            "--score_clip_his=" + str(clip["clip_hi"]),
            "--score_program=m0-0;const:0.001;add;dup;ema:0.99;sin;pow:2;clamp",
        ],
        capture_output=True,
        text=True,
        timeout=10,
    )
    assert r.returncode == 0, "stack/math score program summary failed: " + r.stderr[:200]
    summary = json.loads(r.stdout)
    assert summary["n_solves"] == 3, "stack/math score program solve count mismatch"
    print("  score program stack/math chips: OK")

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

    # 5c. New v5 geometric/angular metrics
    v5_bin = "/tmp/sps_v5_metrics.bin"
    v5_solves = [
        [(1, 0), (0, 1), (-1, 0), (0, -1)],          # unit cardinal
        [(0.5, 0), (0, 0.5), (-0.5, 0), (0, -0.5)],  # inner cardinal
        [(1, 1), (-1, 1), (-1, -1), (1, -1)],        # diagonals
        [(1, 0), (1, 0), (1, 0), (1, 0)],            # single sector
        [(0, 1), (0, 2), (0, -1), (0, -2)],          # imaginary axis
        [(0.5, 0), (2, 0), (0, 0.5), (0, 2)],        # mixed radial
        [(1, 0), (math.cos(2.0 * math.pi / 3.0), math.sin(2.0 * math.pi / 3.0)), (math.cos(4.0 * math.pi / 3.0), math.sin(4.0 * math.pi / 3.0)), (0, 0)], # non-4-fold
    ]
    _write_roots_bin(v5_bin, v5_solves, 4)

    def _clip_v5(metric):
        rr = subprocess.run([sps_path, v5_bin, "--mode=clip", "--degree=4", "--metric=" + metric],
                            capture_output=True, text=True, timeout=10)
        assert rr.returncode == 0, "%s clip failed: %s" % (metric, rr.stderr[:200])
        data = json.loads(rr.stdout)
        assert data["metric"] == metric
        assert data["n_solves"] == len(v5_solves)
        assert math.isfinite(data["min_score"]) and math.isfinite(data["max_score"])
        return data

    v5_expect_spread = [
        "mean_log_mod",
        "sd_log_mod",
        "inside_unit_fraction",
        "unit_annulus_fraction_01",
        "imag_axis_proximity",
        "diagonal_proximity",
        "angular_entropy_16",
        "sector_max_share_16",
        "angular_order_2",
        "angular_order_4",
    ]
    for metric in v5_expect_spread:
        data = _clip_v5(metric)
        assert data["max_score"] > data["min_score"], "%s should vary on v5 fixture" % metric
        print("  %s clip: OK (lo=%.3f, hi=%.3f)" % (metric, data["clip_lo"], data["clip_hi"]))

    v5_log_bin = "/tmp/sps_v5_log_mod_natural.bin"
    _write_roots_bin(v5_log_bin, [
        [(math.e, 0), (1.0 / math.e, 0)],
        [(math.e * math.e, 0), (math.e * math.e, 0)],
    ], 2)
    rr = subprocess.run([sps_path, v5_log_bin, "--mode=clip", "--degree=2", "--metric=mean_log_mod"],
                        capture_output=True, text=True, timeout=10)
    assert rr.returncode == 0, "mean_log_mod natural-log clip failed: " + rr.stderr[:200]
    mean_log = json.loads(rr.stdout)
    assert abs(mean_log["min_score"] - 0.0) < 1e-5 and abs(mean_log["max_score"] - 2.0) < 1e-5
    rr = subprocess.run([sps_path, v5_log_bin, "--mode=clip", "--degree=2", "--metric=sd_log_mod"],
                        capture_output=True, text=True, timeout=10)
    assert rr.returncode == 0, "sd_log_mod natural-log clip failed: " + rr.stderr[:200]
    sd_log = json.loads(rr.stdout)
    assert abs(sd_log["min_score"] - 0.0) < 1e-5 and abs(sd_log["max_score"] - 1.0) < 1e-5
    print("  log_mod natural-log semantics: OK")

    v5_order3_bin = "/tmp/sps_v5_order3.bin"
    tri = [(1, 0), (math.cos(2.0 * math.pi / 3.0), math.sin(2.0 * math.pi / 3.0)), (math.cos(4.0 * math.pi / 3.0), math.sin(4.0 * math.pi / 3.0))]
    line = [(1, 0), (-1, 0), (0, 1)]
    _write_roots_bin(v5_order3_bin, [tri, line], 3)
    rr = subprocess.run([sps_path, v5_order3_bin, "--mode=clip", "--degree=3", "--metric=angular_order_3"],
                        capture_output=True, text=True, timeout=10)
    assert rr.returncode == 0, "angular_order_3 clip failed: " + rr.stderr[:200]
    order3 = json.loads(rr.stdout)
    assert order3["metric"] == "angular_order_3"
    assert order3["max_score"] > order3["min_score"]
    print("  angular_order_3 clip: OK (lo=%.3f, hi=%.3f)" % (order3["clip_lo"], order3["clip_hi"]))

    # 6. Non-default quantile clip (q=0.05)
    _write_sps_bin(sps_bin)
    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2", "--metric=proximity",
                        "--quantile_lo=0.05", "--quantile_hi=0.95"],
                       capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, "q=0.05 clip failed: " + r.stderr[:200]
    q05 = json.loads(r.stdout)
    assert q05["n_solves"] == 3
    print("  proximity clip q=5%%: OK (lo=%.2f, hi=%.2f)" % (q05["clip_lo"], q05["clip_hi"]))

    cleanup(sps_bin, v5_bin, v5_log_bin, v5_order3_bin)
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


def test_libcurl_binaries_use_rpath_not_runpath():
    """The four libcurl binaries must carry DT_RPATH (not DT_RUNPATH).

    DT_RPATH makes $ORIGIN/lib resolve the binary's whole dependency closure,
    so function envs need no /var/task/lib on LD_LIBRARY_PATH — which is what
    put our staged OpenSSL in front of the Lambda runtime python and broke
    `import ssl` (the 2026-06 render-lores-preview outage).
    """
    import struct

    DT_RPATH, DT_RUNPATH, PT_DYNAMIC = 15, 29, 2
    for bin_path in [
        "/src/roots2pix_mt",
        "/src/assemble_greyscale",
        "/src/solve_proximity_hist_sectioned",
        "/src/solve_palette_chunk_mt",
    ]:
        data = open(bin_path, "rb").read()
        assert data[:4] == b"\x7fELF" and data[4] == 2, "%s: not ELF64" % bin_path
        e_phoff, = struct.unpack_from("<Q", data, 0x20)
        e_phentsize, = struct.unpack_from("<H", data, 0x36)
        e_phnum, = struct.unpack_from("<H", data, 0x38)
        tags = set()
        for i in range(e_phnum):
            off = e_phoff + i * e_phentsize
            p_type, = struct.unpack_from("<I", data, off)
            if p_type != PT_DYNAMIC:
                continue
            p_offset, = struct.unpack_from("<Q", data, off + 0x08)
            p_filesz, = struct.unpack_from("<Q", data, off + 0x20)
            for j in range(p_filesz // 16):
                d_tag, = struct.unpack_from("<q", data, p_offset + j * 16)
                if d_tag == 0:
                    break
                tags.add(d_tag)
        assert DT_RPATH in tags, "%s: missing DT_RPATH (built without --disable-new-dtags?)" % bin_path
        assert DT_RUNPATH not in tags, "%s: has DT_RUNPATH; transitive deps would leak to LD_LIBRARY_PATH" % bin_path
        # glibc-family libs must come from the runtime, never the staged
        # closure: a newer staged libresolv against the pinned runtime glibc
        # broke DNS inside curl (2026-06 incident).
        lib_dir = bin_path + "_lib"
        staged = sorted(os.listdir(lib_dir)) if os.path.isdir(lib_dir) else []
        bad = [f for f in staged if f.startswith(("libresolv.", "libnss_", "libc.", "libm.", "libpthread.", "ld-linux"))]
        assert not bad, "%s stages glibc-family libs: %s" % (lib_dir, bad)
        print("  %s: DT_RPATH OK, staged closure clean (%d libs)" % (bin_path, len(staged)))

    # Layer hygiene: layers ride on LD_LIBRARY_PATH=/opt/lib, which the
    # runtime python's own extension loader also searches. Libraries that
    # python or its ssl stack dlopens must never ship in a layer built from
    # a NEWER Amazon Linux than the pinned Lambda runtime — that is the
    # 2026-06 outage class. glibc-family and the OpenSSL pair are banned
    # outright; the frozen-version shadows python relies on (zlib/bz2/ffi/
    # gcc_s) are pinned so a layer rebuild that moves them fails loudly and
    # forces a deliberate decision plus the post-deploy INIT sweep.
    BANNED = ("libresolv.", "libnss_", "libssl.", "libcrypto.", "libc.", "ld-linux")
    PINNED_SHADOWS = {
        "layer-build": {"libz.so.1.2.11", "libbz2.so.1.0.8", "libffi.so.8.1.2"},
        "layer-build-lapack": {"libgcc_s.so.1"},
    }
    for layer, expected in PINNED_SHADOWS.items():
        layer_dir = "/src/" + layer + "/lib"
        files = set(os.listdir(layer_dir))
        banned = sorted(f for f in files if f.startswith(BANNED))
        assert not banned, "%s ships python-critical libs: %s" % (layer, banned)
        missing = expected - files
        assert not missing, "%s pinned shadow set changed (missing %s); layer rebuilt against a newer userland? Verify against the pinned runtime and update this pin deliberately." % (layer, sorted(missing))
        print("  %s: no banned libs, pinned shadows intact" % layer)

    # libarchive's crypto chain is vendored OUTSIDE /opt/lib (python-invisible)
    # and reached via the vips binaries' DT_RPATH; it must exist there and
    # must never leak into lib/ (the banned check above enforces the latter).
    vipsdeps = "/src/layer-build/vipsdeps"
    assert os.path.isdir(vipsdeps), (
        "layer-build/vipsdeps missing — the vips binaries carry RPATH"
        " /opt/lib:/opt/vipsdeps, so the layer must ship it"
        " (rebuild lambda/build-libvips-layer.sh)"
    )
    deps = set(os.listdir(vipsdeps))
    for need in ("libcrypto.so.3", "liblzma.so.5", "libzstd.so.1", "liblz4.so.1"):
        assert need in deps, "vipsdeps missing %s (rebuild lambda/build-libvips-layer.sh)" % need
    print("  layer-build/vipsdeps: libarchive crypto chain vendored (%d libs)" % len(deps))
    print("=== libcurl binary linkage check PASSED ===")


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
        "/src/solve_proximity_hist_sectioned",
        "/src/score_raw_render",
        "/src/bilevel_section_raster",
        "/src/coeffs_bilevel_raster",
        "/src/bilevel_merge",
        "/src/raw_to_bilevel",
        "/src/sheet_stitch",
        "/src/assemble_greyscale",
    ]:
        magic = open(bin_path, "rb").read(4)
        assert magic == b"\x7fELF", "%s is not an ELF binary" % bin_path
        print("  %s: ELF OK" % bin_path)

    test_libcurl_binaries_use_rpath_not_runpath()

    print("--- Generating test fixtures ---")
    test_sweep_mt_stream_flush_byte_identity()
    test_ae_cm_solvers()
    test_coeff_vector_constant_opcodes_runtime()
    test_cfpv_coeffgen()
    test_param_gen_threaded_runtime()
    test_coeffgen_chunked_threaded_runtime()
    test_compute_preview_runtime_combo()
    test_palette_chunk_mt_runtime()
    test_palette_chunk_mt_param_sectioned_runtime()
    test_solve_proximity_hist_sectioned_lagged_runtime()
    test_palette_chunk_mt_lagged_multispan_runtime()
    test_roots2pix_mt_multispan_runtime()
    test_roots2pix_mt_lagged_score_runtime()
    test_roots2pix_mt_score_output_normalization_runtime()
    test_roots2pix_mt_explicit_rgb_outputs_runtime()
    test_roots2pix_mt_local_file_manifest_runtime()
    test_render_lores_preview_handler_runtime()
    test_render_preview()
    test_resize_runtime()
    test_bilevel_section_raster_runtime()
    test_coeffs_bilevel_raster_runtime()
    test_bilevel_merge_assemble_runtime()
    test_raw_to_bilevel_runtime()
    test_sheet_stitch_runtime()
    test_sheet_deepzoom_grayscale_runtime()
    test_color_to_bilevel_handler_runtime()
    test_bilevel_handler_sparse_finalize_runtime()
    test_solve_proximity_stats()
    test_catalog_degrees()

    print("\n=== All Docker runtime tests PASSED ===")
