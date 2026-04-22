import array
import http.server
import json
import math
import pathlib
import shutil
import socketserver
import subprocess
import tempfile
import threading

import pytest


ROOT = pathlib.Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"


class _RangeRequestHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        rel = self.path.lstrip("/")
        target = pathlib.Path(self.server.root_dir) / rel
        if not target.is_file():
            self.send_error(404)
            return
        data = target.read_bytes()
        range_header = self.headers.get("Range")
        start = 0
        end = len(data) - 1
        status = 200
        if range_header:
            if not range_header.startswith("bytes="):
                self.send_error(400)
                return
            start_s, end_s = range_header[len("bytes="):].split("-", 1)
            start = int(start_s)
            end = int(end_s) if end_s else len(data) - 1
            if start < 0 or end < start or end >= len(data):
                self.send_error(416)
                return
            data = data[start:end + 1]
            status = 206
        self.send_response(status)
        self.send_header("Content-Type", "application/octet-stream")
        self.send_header("Content-Length", str(len(data)))
        if status == 206:
            self.send_header("Content-Range", f"bytes {start}-{end}/{target.stat().st_size}")
        self.end_headers()
        self.wfile.write(data)

    def log_message(self, fmt, *args):
        return


class _ThreadedTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    allow_reuse_address = True
    daemon_threads = True


def _compiler():
    return shutil.which("cc") or shutil.which("clang") or shutil.which("gcc")


def _compile_binary(tmpdir, name, source, extra_sources=None, libs=None):
    cc = _compiler()
    assert cc, "no C compiler available"
    out = pathlib.Path(tmpdir) / name
    cmd = [cc, "-O2", "-I", str(LAMBDA_DIR), str(LAMBDA_DIR / source)]
    for extra in extra_sources or []:
        cmd.append(str(LAMBDA_DIR / extra))
    cmd.extend(libs or [])
    cmd.extend(["-o", str(out)])
    result = subprocess.run(cmd, capture_output=True, text=True)
    assert result.returncode == 0, f"failed to compile {source}: {result.stderr}"
    return out


def _write_float_file(path, values):
    arr = array.array("f", values)
    with open(path, "wb") as fh:
        arr.tofile(fh)
    return path


def _serve_dir(root_dir):
    server = _ThreadedTCPServer(("127.0.0.1", 0), _RangeRequestHandler)
    server.root_dir = str(root_dir)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    return server, thread


def _write_single_span_manifest(path, *, file_name, port, row_bytes, solve_count, source_family="slv"):
    logical_size = int(row_bytes) * int(solve_count)
    manifest = {
        "source_family": source_family,
        "logical_size": logical_size,
        "row_bytes": int(row_bytes),
        "solve_start": 0,
        "solve_count": int(solve_count),
        "sources": [{
            "id": 0,
            "url": f"http://127.0.0.1:{port}/{file_name}",
            "key": file_name,
        }],
        "spans": [{
            "source_id": 0,
            "logical_byte_start": 0,
            "byte_start": 0,
            "byte_length": logical_size,
        }],
    }
    path.write_text(json.dumps(manifest), encoding="utf-8")
    return path


def _square_bounds(ext):
    return {
        "min_re": -float(ext),
        "max_re": float(ext),
        "min_im": -float(ext),
        "max_im": float(ext),
    }


def _square_scale(width, ext):
    return float(width) / (2.0 * float(ext))


def _legacy_square_project(re, im, *, width, height, center_re, center_im, scale):
    half_w = width / 2.0
    half_h = height / 2.0
    px = int(half_w + (re - center_re) * scale)
    py = int(half_h - (im - center_im) * scale)
    return px, py


def _bounds_project(re, im, *, width, height, min_re, max_re, min_im, max_im, rotation=0.0):
    center_re = (float(min_re) + float(max_re)) / 2.0
    center_im = (float(min_im) + float(max_im)) / 2.0
    dx = float(re) - center_re
    dy = float(im) - center_im
    cos_a = math.cos(float(rotation))
    sin_a = math.sin(float(rotation))
    rot_re = center_re + (dx * cos_a - dy * sin_a)
    rot_im = center_im + (dx * sin_a + dy * cos_a)
    x_scale = float(width) / (float(max_re) - float(min_re))
    y_scale = float(height) / (float(max_im) - float(min_im))
    px = int(math.floor((rot_re - float(min_re)) * x_scale))
    py = int(math.floor((float(max_im) - rot_im) * y_scale))
    return px, py


def _read_u32le_u8_pairs(path):
    raw = path.read_bytes()
    assert len(raw) % 5 == 0
    pairs = []
    for off in range(0, len(raw), 5):
        pairs.append((int.from_bytes(raw[off:off + 4], "little"), raw[off + 4]))
    return pairs


def _encode_u32le_u8_pairs(pairs):
    payload = bytearray()
    for pixel_idx, score in pairs:
        payload.extend(int(pixel_idx).to_bytes(4, "little", signed=False))
        payload.append(int(score) & 0xFF)
    return bytes(payload)


def _centroid_re_score_byte(re, clip_lo, clip_hi):
    u = (float(re) - float(clip_lo)) / (float(clip_hi) - float(clip_lo))
    u = max(0.0, min(1.0, u))
    raw = 1 + int(math.floor((u * 254.0) + 0.5))
    return max(1, min(255, raw))


def _expected_roots2pix_fragment(roots, *, width, height, ext, clip_lo, clip_hi):
    scale = _square_scale(width, ext)
    claimed = set()
    pairs = []
    for re, im in roots:
        px, py = _legacy_square_project(re, im, width=width, height=height, center_re=0.0, center_im=0.0, scale=scale)
        if px < 0 or px >= width or py < 0 or py >= height:
            continue
        global_idx = py * width + px
        if global_idx in claimed:
            continue
        claimed.add(global_idx)
        pairs.append((global_idx, _centroid_re_score_byte(re, clip_lo, clip_hi)))
    return pairs


def _expected_section_fragment(roots, *, width, height, ext):
    scale = _square_scale(width, ext)
    claimed = set()
    pairs = []
    for re, im in roots:
        px, py = _legacy_square_project(re, im, width=width, height=height, center_re=0.0, center_im=0.0, scale=scale)
        if px < 0 or px >= width or py < 0 or py >= height:
            continue
        global_idx = py * width + px
        if global_idx in claimed:
            continue
        claimed.add(global_idx)
        pairs.append((global_idx, 1))
    return pairs


def _expected_roots2pix_fragment_bounds(roots, *, width, height, bounds, clip_lo, clip_hi, rotation=0.0):
    claimed = set()
    pairs = []
    for re, im in roots:
        px, py = _bounds_project(
            re,
            im,
            width=width,
            height=height,
            min_re=bounds["min_re"],
            max_re=bounds["max_re"],
            min_im=bounds["min_im"],
            max_im=bounds["max_im"],
            rotation=rotation,
        )
        if px < 0 or px >= width or py < 0 or py >= height:
            continue
        global_idx = py * width + px
        if global_idx in claimed:
            continue
        claimed.add(global_idx)
        pairs.append((global_idx, _centroid_re_score_byte(re, clip_lo, clip_hi)))
    return pairs


def _expected_section_fragment_bounds(roots, *, width, height, bounds, rotation=0.0):
    claimed = set()
    pairs = []
    for re, im in roots:
        px, py = _bounds_project(
            re,
            im,
            width=width,
            height=height,
            min_re=bounds["min_re"],
            max_re=bounds["max_re"],
            min_im=bounds["min_im"],
            max_im=bounds["max_im"],
            rotation=rotation,
        )
        if px < 0 or px >= width or py < 0 or py >= height:
            continue
        global_idx = py * width + px
        if global_idx in claimed:
            continue
        claimed.add(global_idx)
        pairs.append((global_idx, 1))
    return pairs


def _expected_coeff_bits(coeff_rows, *, width, height, ext):
    scale = _square_scale(width, ext)
    bits = bytearray((width * height + 7) // 8)
    plotted = 0
    clipped = 0
    deduped = 0
    for re, im in coeff_rows:
        px, py = _legacy_square_project(re, im, width=width, height=height, center_re=0.0, center_im=0.0, scale=scale)
        if px < 0 or px >= width or py < 0 or py >= height:
            clipped += 1
            continue
        bit_idx = py * width + px
        byte_idx = bit_idx >> 3
        mask = 1 << (bit_idx & 7)
        if bits[byte_idx] & mask:
            deduped += 1
            continue
        bits[byte_idx] |= mask
        plotted += 1
    return bytes(bits), plotted, clipped, deduped


def _expected_coeff_bits_bounds(coeff_rows, *, width, height, bounds, rotation=0.0):
    bits = bytearray((width * height + 7) // 8)
    plotted = 0
    clipped = 0
    deduped = 0
    for re, im in coeff_rows:
        px, py = _bounds_project(
            re,
            im,
            width=width,
            height=height,
            min_re=bounds["min_re"],
            max_re=bounds["max_re"],
            min_im=bounds["min_im"],
            max_im=bounds["max_im"],
            rotation=rotation,
        )
        if px < 0 or px >= width or py < 0 or py >= height:
            clipped += 1
            continue
        bit_idx = py * width + px
        byte_idx = bit_idx >> 3
        mask = 1 << (bit_idx & 7)
        if bits[byte_idx] & mask:
            deduped += 1
            continue
        bits[byte_idx] |= mask
        plotted += 1
    return bytes(bits), plotted, clipped, deduped


def test_roots2pix_mt_square_ext_2_5_matches_legacy_square_camera_oracle():
    with tempfile.TemporaryDirectory(prefix="exact_viewport_roots2pix_") as td:
        root = pathlib.Path(td)
        binary = _compile_binary(
            td,
            "roots2pix_mt_test",
            "roots2pix_mt.c",
            extra_sources=["multispan_reader.c"],
            libs=["-lcurl", "-lm", "-lpthread"],
        )
        roots = [
            (-2.25, -2.25),
            (-1.00, 1.00),
            (0.00, 0.00),
            (0.00, 0.00),
            (2.25, 2.25),
            (3.00, 0.00),
        ]
        roots_path = _write_float_file(root / "roots.bin", [coord for pair in roots for coord in pair])
        width = height = 20
        ext = 2.5
        bounds = _square_bounds(ext)
        clip_lo = -2.5
        clip_hi = 2.5
        pixbin_prefix = root / "pixbin"
        server, thread = _serve_dir(root)
        try:
            manifest_path = _write_single_span_manifest(
                root / "roots_manifest.json",
                file_name=roots_path.name,
                port=server.server_address[1],
                row_bytes=8,
                solve_count=len(roots),
            )
            cmd = [
                str(binary),
                str(root / "pix"),
                f"--width={width}",
                f"--height={height}",
                "--tile_size=20",
                "--n_tile_cols=1",
                "--n_tile_rows=1",
                f"--min_re={bounds['min_re']}",
                f"--max_re={bounds['max_re']}",
                f"--min_im={bounds['min_im']}",
                f"--max_im={bounds['max_im']}",
                "--degree=1",
                "--color=solve_score",
                "--match=none",
                "--palette=inferno",
                "--rotation=0",
                "--threads=1",
                "--input_mode=multispan_sectioned",
                f"--input_manifest={manifest_path}",
                "--solve_metric=centroid_re",
                f"--solve_score_clip_lo={clip_lo}",
                f"--solve_score_clip_hi={clip_hi}",
                "--solve_score_omega_enabled=0",
                "--solve_score_raw_bytes=1",
                "--skip_pix_output=1",
                f"--pixel_bin_prefix={pixbin_prefix}",
            ]
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
            assert result.returncode == 0, result.stderr
            meta = json.loads(result.stdout)
            expected_pairs = _expected_roots2pix_fragment(
                roots,
                width=width,
                height=height,
                ext=ext,
                clip_lo=clip_lo,
                clip_hi=clip_hi,
            )
            assert meta["roots_plotted"] == len(expected_pairs)
            assert meta["roots_clipped"] == 1
            assert meta["input_mode"] == "multispan_sectioned"
            assert (root / "pixbin.frag").read_bytes() == _encode_u32le_u8_pairs(expected_pairs)
        finally:
            server.shutdown()
            server.server_close()
            thread.join(timeout=5)


def test_bilevel_section_square_ext_2_5_matches_legacy_square_camera_oracle():
    with tempfile.TemporaryDirectory(prefix="exact_viewport_bilevel_section_") as td:
        root = pathlib.Path(td)
        binary = _compile_binary(td, "bilevel_section_raster_test", "bilevel_section_raster.c", libs=["-lm"])
        roots = [
            (-2.25, -2.25),
            (-1.00, 1.00),
            (0.00, 0.00),
            (0.00, 0.00),
            (2.25, 2.25),
            (3.00, 0.00),
        ]
        section_path = _write_float_file(root / "section.bin", [coord for pair in roots for coord in pair])
        width = height = 20
        ext = 2.5
        bounds = _square_bounds(ext)
        out_frag = root / "section.frag"
        cmd = [
            str(binary),
            str(section_path),
            str(out_frag),
            f"--width={width}",
            f"--height={height}",
            f"--min_re={bounds['min_re']}",
            f"--max_re={bounds['max_re']}",
            f"--min_im={bounds['min_im']}",
            f"--max_im={bounds['max_im']}",
            "--degree=1",
        ]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
        assert result.returncode == 0, result.stderr
        meta = json.loads(result.stdout)
        expected_pairs = _expected_section_fragment(roots, width=width, height=height, ext=ext)
        assert meta["roots_plotted"] == len(expected_pairs)
        assert meta["roots_clipped"] == 1
        assert meta["roots_deduped"] == 1
        assert out_frag.read_bytes() == _encode_u32le_u8_pairs(expected_pairs)


def test_coeffs_bilevel_square_ext_2_5_matches_legacy_square_camera_oracle():
    with tempfile.TemporaryDirectory(prefix="exact_viewport_coeffs_bilevel_") as td:
        root = pathlib.Path(td)
        binary = _compile_binary(td, "coeffs_bilevel_raster_test", "coeffs_bilevel_raster.c", libs=["-lm"])
        coeff_rows = [
            (-2.25, -2.25),
            (-1.00, 1.00),
            (0.00, 0.00),
            (0.00, 0.00),
            (2.25, 2.25),
            (3.00, 0.00),
        ]
        coeffs_path = _write_float_file(root / "coeffs.bin", [coord for pair in coeff_rows for coord in pair])
        width = height = 20
        ext = 2.5
        bounds = _square_bounds(ext)
        out_prefix = root / "coeff_bits"
        cmd = [
            str(binary),
            str(coeffs_path),
            str(out_prefix),
            f"--width={width}",
            f"--height={height}",
            "--tile_size=20",
            "--n_tile_cols=1",
            "--n_tile_rows=1",
            f"--min_re={bounds['min_re']}",
            f"--max_re={bounds['max_re']}",
            f"--min_im={bounds['min_im']}",
            f"--max_im={bounds['max_im']}",
            "--n_coeffs=1",
        ]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
        assert result.returncode == 0, result.stderr
        meta = json.loads(result.stdout)
        expected_bits, plotted, clipped, deduped = _expected_coeff_bits(coeff_rows, width=width, height=height, ext=ext)
        assert meta["roots_plotted"] == plotted
        assert meta["roots_clipped"] == clipped
        assert meta["roots_deduped"] == deduped
        assert (root / "coeff_bits_t0000.bits").read_bytes() == expected_bits


def test_roots2pix_mt_asymmetric_bounds_match_independent_bounds_oracle():
    with tempfile.TemporaryDirectory(prefix="exact_viewport_roots2pix_asym_") as td:
        root = pathlib.Path(td)
        binary = _compile_binary(
            td,
            "roots2pix_mt_test",
            "roots2pix_mt.c",
            extra_sources=["multispan_reader.c"],
            libs=["-lcurl", "-lm", "-lpthread"],
        )
        roots = [
            (-3.50, -0.50),
            (-1.00, 1.50),
            (0.00, 0.00),
            (0.00, 0.00),
            (7.50, 1.75),
            (9.00, 0.00),
        ]
        roots_path = _write_float_file(root / "roots.bin", [coord for pair in roots for coord in pair])
        width = height = 24
        bounds = {
            "min_re": -4.0,
            "max_re": 8.0,
            "min_im": -1.0,
            "max_im": 2.0,
        }
        clip_lo = -4.0
        clip_hi = 8.0
        pixbin_prefix = root / "pixbin"
        server, thread = _serve_dir(root)
        try:
            manifest_path = _write_single_span_manifest(
                root / "roots_manifest.json",
                file_name=roots_path.name,
                port=server.server_address[1],
                row_bytes=8,
                solve_count=len(roots),
            )
            cmd = [
                str(binary),
                str(root / "pix"),
                f"--width={width}",
                f"--height={height}",
                "--tile_size=24",
                "--n_tile_cols=1",
                "--n_tile_rows=1",
                f"--min_re={bounds['min_re']}",
                f"--max_re={bounds['max_re']}",
                f"--min_im={bounds['min_im']}",
                f"--max_im={bounds['max_im']}",
                "--degree=1",
                "--color=solve_score",
                "--match=none",
                "--palette=inferno",
                "--rotation=0",
                "--threads=1",
                "--input_mode=multispan_sectioned",
                f"--input_manifest={manifest_path}",
                "--solve_metric=centroid_re",
                f"--solve_score_clip_lo={clip_lo}",
                f"--solve_score_clip_hi={clip_hi}",
                "--solve_score_omega_enabled=0",
                "--solve_score_raw_bytes=1",
                "--skip_pix_output=1",
                f"--pixel_bin_prefix={pixbin_prefix}",
            ]
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
            assert result.returncode == 0, result.stderr
            meta = json.loads(result.stdout)
            expected_pairs = _expected_roots2pix_fragment_bounds(
                roots,
                width=width,
                height=height,
                bounds=bounds,
                clip_lo=clip_lo,
                clip_hi=clip_hi,
            )
            assert meta["roots_plotted"] == len(expected_pairs)
            assert meta["roots_clipped"] == 1
            assert meta["input_mode"] == "multispan_sectioned"
            assert (root / "pixbin.frag").read_bytes() == _encode_u32le_u8_pairs(expected_pairs)
        finally:
            server.shutdown()
            server.server_close()
            thread.join(timeout=5)


def test_bilevel_section_asymmetric_bounds_match_independent_bounds_oracle():
    with tempfile.TemporaryDirectory(prefix="exact_viewport_bilevel_section_asym_") as td:
        root = pathlib.Path(td)
        binary = _compile_binary(td, "bilevel_section_raster_test", "bilevel_section_raster.c", libs=["-lm"])
        roots = [
            (-3.50, -0.50),
            (-1.00, 1.50),
            (0.00, 0.00),
            (0.00, 0.00),
            (7.50, 1.75),
            (9.00, 0.00),
        ]
        section_path = _write_float_file(root / "section.bin", [coord for pair in roots for coord in pair])
        width = height = 24
        bounds = {
            "min_re": -4.0,
            "max_re": 8.0,
            "min_im": -1.0,
            "max_im": 2.0,
        }
        out_frag = root / "section.frag"
        cmd = [
            str(binary),
            str(section_path),
            str(out_frag),
            f"--width={width}",
            f"--height={height}",
            f"--min_re={bounds['min_re']}",
            f"--max_re={bounds['max_re']}",
            f"--min_im={bounds['min_im']}",
            f"--max_im={bounds['max_im']}",
            "--degree=1",
        ]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
        assert result.returncode == 0, result.stderr
        meta = json.loads(result.stdout)
        expected_pairs = _expected_section_fragment_bounds(
            roots,
            width=width,
            height=height,
            bounds=bounds,
        )
        assert meta["roots_plotted"] == len(expected_pairs)
        assert meta["roots_clipped"] == 1
        assert meta["roots_deduped"] == 1
        assert out_frag.read_bytes() == _encode_u32le_u8_pairs(expected_pairs)


def test_coeffs_bilevel_asymmetric_bounds_match_independent_bounds_oracle():
    with tempfile.TemporaryDirectory(prefix="exact_viewport_coeffs_bilevel_asym_") as td:
        root = pathlib.Path(td)
        binary = _compile_binary(td, "coeffs_bilevel_raster_test", "coeffs_bilevel_raster.c", libs=["-lm"])
        coeff_rows = [
            (-3.50, -0.50),
            (-1.00, 1.50),
            (0.00, 0.00),
            (0.00, 0.00),
            (7.50, 1.75),
            (9.00, 0.00),
        ]
        coeffs_path = _write_float_file(root / "coeffs.bin", [coord for pair in coeff_rows for coord in pair])
        width = height = 24
        bounds = {
            "min_re": -4.0,
            "max_re": 8.0,
            "min_im": -1.0,
            "max_im": 2.0,
        }
        out_prefix = root / "coeff_bits"
        cmd = [
            str(binary),
            str(coeffs_path),
            str(out_prefix),
            f"--width={width}",
            f"--height={height}",
            "--tile_size=24",
            "--n_tile_cols=1",
            "--n_tile_rows=1",
            f"--min_re={bounds['min_re']}",
            f"--max_re={bounds['max_re']}",
            f"--min_im={bounds['min_im']}",
            f"--max_im={bounds['max_im']}",
            "--n_coeffs=1",
        ]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
        assert result.returncode == 0, result.stderr
        meta = json.loads(result.stdout)
        expected_bits, plotted, clipped, deduped = _expected_coeff_bits_bounds(
            coeff_rows,
            width=width,
            height=height,
            bounds=bounds,
        )
        assert meta["roots_plotted"] == plotted
        assert meta["roots_clipped"] == clipped
        assert meta["roots_deduped"] == deduped
        assert (root / "coeff_bits_t0000.bits").read_bytes() == expected_bits


def test_active_binaries_reject_legacy_square_camera_args():
    with tempfile.TemporaryDirectory(prefix="exact_viewport_no_legacy_") as td:
        root = pathlib.Path(td)
        roots_binary = _compile_binary(
            td,
            "roots2pix_mt_test",
            "roots2pix_mt.c",
            extra_sources=["multispan_reader.c"],
            libs=["-lcurl", "-lm", "-lpthread"],
        )
        section_binary = _compile_binary(td, "bilevel_section_raster_test", "bilevel_section_raster.c", libs=["-lm"])
        coeff_binary = _compile_binary(td, "coeffs_bilevel_raster_test", "coeffs_bilevel_raster.c", libs=["-lm"])

        roots_path = _write_float_file(root / "roots.bin", [0.0, 0.0])
        coeffs_path = _write_float_file(root / "coeffs.bin", [0.0, 0.0])
        server, thread = _serve_dir(root)
        try:
            manifest_path = _write_single_span_manifest(
                root / "roots_manifest.json",
                file_name=roots_path.name,
                port=server.server_address[1],
                row_bytes=8,
                solve_count=1,
            )
            roots_result = subprocess.run(
                [
                    str(roots_binary),
                    str(root / "pix"),
                    "--width=8",
                    "--height=8",
                    "--tile_size=8",
                    "--n_tile_cols=1",
                    "--n_tile_rows=1",
                    "--center_re=0",
                    "--center_im=0",
                    "--scale=1",
                    "--degree=1",
                    "--color=solve_score",
                    "--match=none",
                    "--palette=inferno",
                    "--threads=1",
                    "--input_mode=multispan_sectioned",
                    f"--input_manifest={manifest_path}",
                    "--solve_metric=centroid_re",
                    "--solve_score_clip_lo=-1",
                    "--solve_score_clip_hi=1",
                    "--solve_score_omega_enabled=0",
                    "--solve_score_raw_bytes=1",
                    "--skip_pix_output=1",
                    f"--pixel_bin_prefix={root / 'pixbin'}",
                ],
                capture_output=True,
                text=True,
                timeout=30,
            )
            assert roots_result.returncode != 0
            assert "no longer supported" in roots_result.stderr
        finally:
            server.shutdown()
            server.server_close()
            thread.join(timeout=5)

        section_result = subprocess.run(
            [
                str(section_binary),
                str(roots_path),
                str(root / "section.frag"),
                "--width=8",
                "--height=8",
                "--center_re=0",
                "--center_im=0",
                "--scale=1",
                "--degree=1",
            ],
            capture_output=True,
            text=True,
            timeout=30,
        )
        assert section_result.returncode != 0
        assert "no longer supported" in section_result.stderr

        coeff_result = subprocess.run(
            [
                str(coeff_binary),
                str(coeffs_path),
                str(root / "coeff_bits"),
                "--width=8",
                "--height=8",
                "--tile_size=8",
                "--n_tile_cols=1",
                "--n_tile_rows=1",
                "--center_re=0",
                "--center_im=0",
                "--scale=1",
                "--n_coeffs=1",
            ],
            capture_output=True,
            text=True,
            timeout=30,
        )
        assert coeff_result.returncode != 0
        assert "no longer supported" in coeff_result.stderr


def test_roots2pix_mt_rejects_legacy_input_modes():
    with tempfile.TemporaryDirectory(prefix="exact_viewport_roots2pix_input_modes_") as td:
        root = pathlib.Path(td)
        binary = _compile_binary(
            td,
            "roots2pix_mt_test",
            "roots2pix_mt.c",
            extra_sources=["multispan_reader.c"],
            libs=["-lcurl", "-lm", "-lpthread"],
        )
        roots_path = _write_float_file(root / "roots.bin", [0.0, 0.0])
        common_args = [
            str(binary),
            str(root / "pix"),
            "--width=8",
            "--height=8",
            "--tile_size=8",
            "--n_tile_cols=1",
            "--n_tile_rows=1",
            "--min_re=-1",
            "--max_re=1",
            "--min_im=-1",
            "--max_im=1",
            "--degree=1",
            "--color=solve_score",
            "--match=none",
            "--palette=inferno",
            "--threads=1",
            "--solve_metric=centroid_re",
            "--solve_score_clip_lo=-1",
            "--solve_score_clip_hi=1",
            "--solve_score_omega_enabled=0",
            "--solve_score_raw_bytes=1",
            "--skip_pix_output=1",
            f"--pixel_bin_prefix={root / 'pixbin'}",
        ]
        for legacy_mode in ("tmpfile", "sectioned"):
            result = subprocess.run(
                [*common_args, f"--input_mode={legacy_mode}"],
                capture_output=True,
                text=True,
                timeout=30,
            )
            assert result.returncode != 0
            assert "only supports --input_mode=multispan_sectioned" in result.stderr
