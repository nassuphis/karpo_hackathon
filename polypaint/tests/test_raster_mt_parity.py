import http.server
import json
import math
import pathlib
import shutil
import socketserver
import subprocess
import sys
import tempfile
import threading
import unittest
from array import array


ROOT = pathlib.Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"
sys.path.insert(0, str(LAMBDA_DIR))


def _isometric_pixel(re, im, t, *, pix=8, min_re=-1.0, max_re=1.0,
                     min_im=-1.0, max_im=1.0):
    x = (re - min_re) / (max_re - min_re)
    y = (im - min_im) / (max_im - min_im)
    extent = pix - 1
    scale = extent / 2.0
    px = extent / 2.0 + scale * (x - y) * (math.sqrt(3.0) / 2.0)
    py = extent / 2.0 + scale * ((x + y) / 2.0 - t)
    return math.floor(px), math.floor(py)


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


class TestRasterMtParity(unittest.TestCase):
    def _bounds_args(self, width, height, center_re, center_im, scale):
        half_w_world = (float(width) / 2.0) / float(scale)
        half_h_world = (float(height) / 2.0) / float(scale)
        return [
            f"--min_re={center_re - half_w_world}",
            f"--max_re={center_re + half_w_world}",
            f"--min_im={center_im - half_h_world}",
            f"--max_im={center_im + half_h_world}",
        ]

    def _single_metric_program_args(self, metric, clip_lo, clip_hi):
        return [
            f"--score_metrics={metric}",
            f"--score_clip_los={clip_lo}",
            f"--score_clip_his={clip_hi}",
            "--score_program=v2;m0",
        ]

    def _score_case_payloads(self):
        from solve_score_chain import compile_solve_score_chain, solve_score_program_cli_payload

        cases = [
            {
                "label": "slv",
                "chain": [["crowding", "slv", "1"]],
                "clips": {"crowding": (0.0, 1.0)},
                "uses_coeff": False,
                "uses_param": False,
            },
            {
                "label": "slv_cf",
                "chain": [["crowding", "slv", "1"], ["spread", "cf", "3"], ["max"]],
                "clips": {"crowding": (0.0, 1.0), "spread": (0.0, 3.0)},
                "uses_coeff": True,
                "uses_param": False,
            },
            {
                "label": "slv_pm",
                "chain": [["crowding", "slv", "1"], ["t1_abs", "pm", "2"], ["max"]],
                "clips": {"crowding": (0.0, 1.0), "t1_abs": (0.0, 2.0)},
                "uses_coeff": False,
                "uses_param": True,
            },
            {
                "label": "slv_cf_pm",
                "chain": [["crowding", "slv", "1"], ["spread", "cf", "3"], ["max"], ["t1_abs", "pm", "2"], ["max"]],
                "clips": {"crowding": (0.0, 1.0), "spread": (0.0, 3.0), "t1_abs": (0.0, 2.0)},
                "uses_coeff": True,
                "uses_param": True,
            },
        ]
        payloads = []
        for case in cases:
            compiled = compile_solve_score_chain(case["chain"])
            metrics = []
            for metric in compiled["metrics"]:
                lo, hi = case["clips"][metric["metric"]]
                metrics.append({**metric, "clip_lo": lo, "clip_hi": hi})
            payloads.append({
                **case,
                "payload": solve_score_program_cli_payload({"metrics": metrics, "program_spec": compiled["program_spec"]}),
            })
        return payloads

    @classmethod
    def setUpClass(cls):
        cls._tmpdir_obj = tempfile.TemporaryDirectory()
        cls._workdir = pathlib.Path(cls._tmpdir_obj.name)
        cc = shutil.which("cc") or shutil.which("clang") or shutil.which("gcc")
        if not cc:
            raise unittest.SkipTest("no C compiler available")
        cls._binary = cls._workdir / "roots2pix_mt_test"
        compile_cmd = [
            cc,
            "-O2",
            "-I",
            str(LAMBDA_DIR),
            str(LAMBDA_DIR / "roots2pix_mt.c"),
            str(LAMBDA_DIR / "multispan_reader.c"),
            "-lcurl",
            "-lm",
            "-lpthread",
            "-o",
            str(cls._binary),
        ]
        result = subprocess.run(compile_cmd, capture_output=True, text=True)
        if result.returncode != 0:
            raise AssertionError(f"failed to compile roots2pix_mt parity binary: {result.stderr}")

    @classmethod
    def tearDownClass(cls):
        cls._tmpdir_obj.cleanup()

    def _serve_dir(self, root_dir):
        server = _ThreadedTCPServer(("127.0.0.1", 0), _RangeRequestHandler)
        server.root_dir = str(root_dir)
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        return server, thread

    def _write_float_file(self, path, values):
        arr = array("f", values)
        with open(path, "wb") as fh:
            arr.tofile(fh)
        return path

    def _write_single_span_manifest(self, path, *, file_name, port, row_bytes, solve_count, source_family="slv"):
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

    def _read_u32_pairs(self, path):
        raw = path.read_bytes()
        self.assertEqual(len(raw) % 8, 0)
        ints = array("I")
        ints.frombytes(raw)
        return list(zip(ints[0::2], ints[1::2]))

    def _read_u32le_u8_pairs(self, path):
        raw = path.read_bytes()
        self.assertEqual(len(raw) % 5, 0)
        pairs = []
        for off in range(0, len(raw), 5):
            pairs.append((int.from_bytes(raw[off:off + 4], "little"), raw[off + 4]))
        return pairs

    def _read_u32le_channel_records(self, path, channels):
        raw = path.read_bytes()
        record_size = 4 + int(channels)
        self.assertEqual(len(raw) % record_size, 0)
        records = []
        for off in range(0, len(raw), record_size):
            records.append((
                int.from_bytes(raw[off:off + 4], "little"),
                tuple(raw[off + 4:off + record_size]),
            ))
        return records

    def _run_binary(self, args):
        return subprocess.run(args, capture_output=True, text=True)

    def _run_centroid_re_score_bytes(self, root, roots, *, label, extra_args=None):
        degree = 1
        step_count = len(roots) // 2
        roots_path = self._write_float_file(root / f"{label}_roots.bin", roots)
        step_scores_path = root / f"{label}_step_scores.raw"
        fragment_prefix = root / f"{label}_fragment"
        server, thread = self._serve_dir(root)
        try:
            manifest_path = self._write_single_span_manifest(
                root / f"{label}_roots_manifest.json",
                file_name=roots_path.name,
                port=server.server_address[1],
                row_bytes=degree * 2 * 4,
                solve_count=step_count,
            )
            cmd = [
                str(self._binary),
                str(root / f"{label}_pix"),
                "--pix=8",
                *self._bounds_args(8, 8, 0.0, 0.0, 1.0),
                f"--degree={degree}",
                "--rotation=0",
                "--threads=1",
                f"--input_manifest={manifest_path}",
                f"--step_count={step_count}",
                *self._single_metric_program_args("centroid_re", 0, 1),
                f"--fragment_prefix={fragment_prefix}",
                f"--step_scores_output={step_scores_path}",
                "--retries=1",
                *(extra_args or []),
            ]
            result = self._run_binary(cmd)
            self.assertEqual(result.returncode, 0, result.stderr)
            return result, step_scores_path.read_bytes()
        finally:
            server.shutdown()
            server.server_close()
            thread.join(timeout=5)

    def _projection_fixture_roots(self, grid_n=4):
        """One degree-1 root per solve at (-0.875 + 0.25*row, -0.875 +
        0.25*col): in the [-1,1]^2 viewport at pix=8 the plan pixels are
        (px=row, py=7-col)-shaped and every projection has an exact
        hand-computed pixel set."""
        roots = []
        for step in range(grid_n * grid_n):
            row = step // grid_n
            j = step % grid_n
            col = (grid_n - 1 - j) if (row & 1) else j
            roots.extend([-0.875 + 0.25 * row, -0.875 + 0.25 * col])
        return roots

    def _run_projection(self, root, roots, *, label, view_args, step_count=16, degree=1):
        roots_path = self._write_float_file(root / f"{label}_roots.bin", roots)
        server, thread = self._serve_dir(root)
        try:
            manifest_path = self._write_single_span_manifest(
                root / f"{label}_manifest.json",
                file_name=roots_path.name,
                port=server.server_address[1],
                row_bytes=degree * 2 * 4,
                solve_count=step_count,
            )
            cmd = [
                str(self._binary),
                str(root / f"{label}_pix"),
                "--pix=8",
                *self._bounds_args(8, 8, 0.0, 0.0, 4.0),
                f"--degree={degree}",
                "--rotation=0",
                "--threads=2",
                f"--input_manifest={manifest_path}",
                f"--step_count={step_count}",
                *self._single_metric_program_args("centroid_re", -2, 2),
                f"--fragment_prefix={root / label}_fragment",
                "--retries=1",
                *view_args,
            ]
            result = self._run_binary(cmd)
            self.assertEqual(result.returncode, 0, result.stderr)
        finally:
            server.shutdown()
            server.server_close()
            thread.join(timeout=5)
        pairs = self._read_u32le_u8_pairs(pathlib.Path(f"{root / label}_fragment.frag"))
        return {(idx % 8, idx // 8) for idx, _ in pairs}

    def test_view_projections_land_on_the_hand_computed_pixels(self):
        """Views are the plan pipeline with one changed point mapping.
        Every projection is pinned against exact pixel sets, including the
        degenerate elevations where dedup collapses coincident columns and
        both isometric choices that consume x, y, and the selected t."""
        grid_n = 4
        roots = self._projection_fixture_roots(grid_n)
        view = lambda proj, vert: [f"--view_projection={proj}",
                                   f"--view_vertical={vert}",
                                   f"--view_grid_n={grid_n}"]
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            # plan (default flags): px = row, py = 7 - col — all 16 pixels
            got = self._run_projection(root, roots, label="plan", view_args=[])
            self.assertEqual(got, {(r, 7 - c) for r in range(4) for c in range(4)})
            # CR36-F9 integer mapping: py = H-1-k*H/N -> {7,5,3,1} at
            # grid 4 / pix 8 (the old float floor((1-t)*H) gave {7,6,4,2})
            # front/t2: px = row (from re), py from t2 index in {7,5,3,1}
            got = self._run_projection(root, roots, label="ft2", view_args=view("front", "t2"))
            self.assertEqual(got, {(r, 7 - 2 * c) for r in range(4) for c in range(4)})
            # front/t1: row drives BOTH axes -> 4 deduped pixels on a diagonal
            got = self._run_projection(root, roots, label="ft1", view_args=view("front", "t1"))
            self.assertEqual(got, {(0, 7), (1, 5), (2, 3), (3, 1)})
            # rear/t2: horizontal mirrored -> px = 7 - row
            got = self._run_projection(root, roots, label="rt2", view_args=view("rear", "t2"))
            self.assertEqual(got, {(7 - r, 7 - 2 * c) for r in range(4) for c in range(4)})
            # right/t2: px from im (= col), py from t2 (= col) -> diagonal
            got = self._run_projection(root, roots, label="rit2", view_args=view("right", "t2"))
            self.assertEqual(got, {(0, 7), (1, 5), (2, 3), (3, 1)})
            # left/t2: px = 7 - col, py from col
            got = self._run_projection(root, roots, label="lt2", view_args=view("left", "t2"))
            self.assertEqual(got, {(7, 7), (6, 5), (5, 3), (4, 1)})
            # left/t1: px = 7 - col, py from t1 = row -> full 4x4 grid again
            got = self._run_projection(root, roots, label="lt1", view_args=view("left", "t1"))
            self.assertEqual(got, {(7 - c, 7 - 2 * r) for c in range(4) for r in range(4)})
            # radial/t2: radius from the origin fills [0, farthest viewport
            # corner], while t2 remains vertical.
            got = self._run_projection(root, roots, label="rad2", view_args=view("radial", "t2"))
            expected = set()
            for row in range(4):
                for col in range(4):
                    re = -0.875 + 0.25 * row
                    im = -0.875 + 0.25 * col
                    px = math.floor(math.hypot(re, im) * 8 / math.sqrt(2))
                    py = 7 - 2 * col                     # H-1 - col*H/N
                    expected.add((px, py))
            self.assertEqual(got, expected)
            # isometric uses all three coordinates. The selected t axis is
            # the only difference between the two unit-cube projections.
            for vertical in ("t1", "t2"):
                got = self._run_projection(
                    root, roots, label=f"iso_{vertical}",
                    view_args=view("isometric", vertical),
                )
                expected = {
                    _isometric_pixel(
                        -0.875 + 0.25 * row,
                        -0.875 + 0.25 * col,
                        (row if vertical == "t1" else col) / 4.0,
                    )
                    for row in range(4)
                    for col in range(4)
                }
                self.assertEqual(got, expected)

    def test_view_step_start_offsets_the_section_lattice(self):
        """MT sections pass their global step offset — the SAME contract as
        palette_step_start. A section holding steps 8..15 with
        --view_step_start=8 must produce exactly the t rows of grid rows
        2 and 3, not rows 0 and 1."""
        grid_n = 4
        all_roots = self._projection_fixture_roots(grid_n)
        section = all_roots[8 * 2:]              # steps 8..15, degree 1
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            got = self._run_projection(
                root, section, label="sec", step_count=8,
                view_args=[
                    "--view_projection=front", "--view_vertical=t1",
                    f"--view_grid_n={grid_n}", "--view_step_start=8",
                ])
            # rows 2,3: py = 7 - 2*k -> {3, 1}; px = row in {2, 3}
            self.assertEqual(got, {(2, 3), (3, 1)})

    def test_view_rows_are_bijective_at_pix_equals_grid(self):
        """CR36-F9 product shape: the deployed ViewRender runs at pix == N.
        py = H-1-k*H/N degrades to H-1-k there — every t index owns its own
        pixel row, the TOP row is reachable, and nothing merges into the
        bottom row (the old float mapping collapsed t rows 0 and 1 and
        could never produce row 0; fixtures at pix=2N could not see it)."""
        grid_n = 8
        roots = self._projection_fixture_roots(grid_n)
        view = lambda vert: ["--view_projection=front",
                             f"--view_vertical={vert}",
                             f"--view_grid_n={grid_n}"]
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            # front/t2: px = row, py = 7 - col -> ALL 64 pixels, bijective
            got = self._run_projection(root, roots, label="nft2",
                                       view_args=view("t2"), step_count=64)
            self.assertEqual(got, {(r, 7 - c) for r in range(8) for c in range(8)})
            # front/t1: row drives both axes -> the full anti-diagonal,
            # including the top row (7, 0)
            got = self._run_projection(root, roots, label="nft1",
                                       view_args=view("t1"), step_count=64)
            self.assertEqual(got, {(k, 7 - k) for k in range(8)})

    def test_xformed_roots_output_matches_plot_transforms(self):
        """--xformed_roots_output must capture EXACTLY what the raster plots:
        the root-xform chain (prepare_step) followed by the viewport-center
        rotation — including roots the plot then clips out of view. This is
        the sculpture data path: the 3D viewer must see what the 2D saw."""
        import math
        degree = 2
        step_count = 3
        center_re, center_im = 0.3, -0.2
        rotation = 0.5          # radians, about the viewport center
        turns = 0.25            # rotate_roots: multiply by exp(2*pi*i*turns)
        roots = []
        for s in range(step_count):
            for r in range(degree):
                roots.extend([0.1 * (s + 1) + 0.01 * r, 0.05 * (s + 1)])
        roots[-2:] = [50.0, 50.0]   # plot clips it; the dump must keep it
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            roots_path = self._write_float_file(root / "xf_roots.bin", roots)
            xforms_path = root / "xf_chain.json"
            xforms_path.write_text(json.dumps([["rotate_roots", str(turns)]]), encoding="utf-8")
            dump_path = root / "xf_dump.bin"
            server, thread = self._serve_dir(root)
            try:
                manifest_path = self._write_single_span_manifest(
                    root / "xf_manifest.json",
                    file_name=roots_path.name,
                    port=server.server_address[1],
                    row_bytes=degree * 2 * 4,
                    solve_count=step_count,
                )
                cmd = [
                    str(self._binary),
                    str(root / "xf_pix"),
                    "--pix=8",
                    *self._bounds_args(8, 8, center_re, center_im, 1.0),
                    f"--degree={degree}",
                    f"--rotation={rotation}",
                    "--threads=2",
                    f"--input_manifest={manifest_path}",
                    f"--step_count={step_count}",
                    *self._single_metric_program_args("centroid_re", 0, 1),
                    f"--fragment_prefix={root / 'xf_fragment'}",
                    f"--root_xforms={xforms_path}",
                    f"--xformed_roots_output={dump_path}",
                    "--retries=1",
                ]
                result = self._run_binary(cmd)
                self.assertEqual(result.returncode, 0, result.stderr)
                meta = json.loads(result.stdout)
                self.assertGreaterEqual(meta["roots_clipped"], 1)
            finally:
                server.shutdown()
                server.server_close()
                thread.join(timeout=5)
            dump = array("f")
            dump.frombytes(dump_path.read_bytes())
            self.assertEqual(len(dump), step_count * degree * 2)
            theta = 2.0 * math.pi * turns
            ct, st_ = math.cos(theta), math.sin(theta)
            cr, sr = math.cos(rotation), math.sin(rotation)
            for s in range(step_count):
                for r in range(degree):
                    idx = (s * degree + r) * 2
                    re0, im0 = roots[idx], roots[idx + 1]
                    xre = re0 * ct - im0 * st_
                    xim = re0 * st_ + im0 * ct
                    dx, dy = xre - center_re, xim - center_im
                    want_re = center_re + dx * cr - dy * sr
                    want_im = center_im + dx * sr + dy * cr
                    self.assertAlmostEqual(dump[idx], want_re, delta=1e-3,
                                           msg=f"step {s} root {r} re")
                    self.assertAlmostEqual(dump[idx + 1], want_im, delta=1e-3,
                                           msg=f"step {s} root {r} im")

    def test_xformed_roots_u16_quantizes_over_the_viewport_with_sentinel(self):
        """--xformed_roots_format=u16: 0..65534 spans the viewport per axis;
        (65535,65535) marks non-finite/out-of-viewport roots. Hi-res
        sculptures ride this to halve the bytes."""
        degree = 2
        step_count = 2
        center_re, center_im = 0.0, 0.0
        roots = [0.5, -1.0,      # inside
                 -2.0, 3.0,      # inside (corner-ish)
                 50.0, 50.0,     # outside -> sentinel
                 0.0, 0.0]       # dead center
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            roots_path = self._write_float_file(root / "q_roots.bin", roots)
            dump_path = root / "q_dump.bin"
            server, thread = self._serve_dir(root)
            try:
                manifest_path = self._write_single_span_manifest(
                    root / "q_manifest.json",
                    file_name=roots_path.name,
                    port=server.server_address[1],
                    row_bytes=degree * 2 * 4,
                    solve_count=step_count,
                )
                cmd = [
                    str(self._binary),
                    str(root / "q_pix"),
                    "--pix=8",
                    *self._bounds_args(8, 8, center_re, center_im, 1.0),
                    f"--degree={degree}",
                    "--rotation=0",
                    "--threads=1",
                    f"--input_manifest={manifest_path}",
                    f"--step_count={step_count}",
                    *self._single_metric_program_args("centroid_re", 0, 1),
                    f"--fragment_prefix={root / 'q_fragment'}",
                    f"--xformed_roots_output={dump_path}",
                    "--xformed_roots_format=u16",
                    "--retries=1",
                ]
                result = self._run_binary(cmd)
                self.assertEqual(result.returncode, 0, result.stderr)
            finally:
                server.shutdown()
                server.server_close()
                thread.join(timeout=5)
            dump = array("H")
            dump.frombytes(dump_path.read_bytes())
            self.assertEqual(len(dump), step_count * degree * 2)
            # viewport is [-4,4]x[-4,4] (pix 8, scale 1): q = (v+4)/8*65534
            def q(v):
                return round((v + 4.0) / 8.0 * 65534.0)
            self.assertEqual(dump[0], q(0.5))
            self.assertEqual(dump[1], q(-1.0))
            self.assertEqual(dump[2], q(-2.0))
            self.assertEqual(dump[3], q(3.0))
            self.assertEqual(dump[4], 0xFFFF)     # sentinel pair
            self.assertEqual(dump[5], 0xFFFF)
            self.assertEqual(dump[6], q(0.0))
            self.assertEqual(dump[7], q(0.0))

    def test_multispan_sectioned_raster_matches_single_span_manifest_for_logical_section_mixed_sources(self):
        from logical_sections import build_native_multispan_manifest, build_solve_source_manifest

        degree = 3
        n_coeffs = 2
        step_count = 6
        section_start = 1
        section_count = 4
        chunk_steps = [2, 1, 3]
        root_stride = degree * 2
        coeff_stride = n_coeffs * 2
        param_stride = 4

        roots = []
        coeffs = []
        params = []
        for idx in range(step_count):
            roots.extend(
                [
                    -0.40 + 0.10 * idx,
                    0.25 - 0.03 * idx,
                    0.05 + 0.08 * idx,
                    -0.30 + 0.04 * idx,
                    0.45 - 0.07 * idx,
                    0.12 + 0.02 * idx,
                ]
            )
            coeffs.extend(
                [
                    0.80 - 0.06 * idx,
                    0.10 + 0.02 * idx,
                    -0.35 + 0.05 * idx,
                    0.22 - 0.03 * idx,
                ]
            )
            params.extend(
                [
                    0.18 + 0.07 * idx,
                    -0.02 * idx,
                    0.72 - 0.05 * idx,
                    0.03 * idx,
                ]
            )

        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            roots_path = self._write_float_file(root / "roots.bin", roots)
            coeffs_path = self._write_float_file(root / "coeffs.bin", coeffs)
            params_path = self._write_float_file(root / "params.bin", params)

            root_bytes = roots_path.read_bytes()
            coeff_bytes = coeffs_path.read_bytes()
            param_bytes = params_path.read_bytes()

            root_row_bytes = root_stride * 4
            coeff_row_bytes = coeff_stride * 4
            param_row_bytes = param_stride * 4

            chunk_items = []
            cursor = 0
            for idx, count in enumerate(chunk_steps):
                start = cursor
                end = cursor + count
                root_key = f"roots_{idx}.bin"
                coeff_key = f"coeffs_{idx}.bin"
                param_key = f"params_{idx}.bin"
                (root / root_key).write_bytes(root_bytes[start * root_row_bytes:end * root_row_bytes])
                (root / coeff_key).write_bytes(coeff_bytes[start * coeff_row_bytes:end * coeff_row_bytes])
                (root / param_key).write_bytes(param_bytes[start * param_row_bytes:end * param_row_bytes])
                chunk_items.append(
                    {
                        "chunk_idx": idx,
                        "step_start": start,
                        "step_count": count,
                        "bin_key": root_key,
                        "coeffs_key": coeff_key,
                        "params_key": param_key,
                        "bin_size": count * root_row_bytes,
                        "coeffs_bin_size": count * coeff_row_bytes,
                        "params_bin_size": count * param_row_bytes,
                        "params_step_start": 0,
                        "params_step_count": count,
                    }
                )
                cursor = end

            solve_source_manifest = build_solve_source_manifest(
                chunk_items,
                job_id="raster_parity_job",
                degree=degree,
                n_coeffs=n_coeffs,
            )

            server, _ = self._serve_dir(root)
            try:
                url_by_key = {
                    key: f"http://127.0.0.1:{server.server_address[1]}/{key}"
                    for key in [
                        "roots_0.bin",
                        "roots_1.bin",
                        "roots_2.bin",
                        "coeffs_0.bin",
                        "coeffs_1.bin",
                        "coeffs_2.bin",
                        "params_0.bin",
                        "params_1.bin",
                        "params_2.bin",
                    ]
                }
                input_manifest = build_native_multispan_manifest(
                    solve_source_manifest,
                    source_family="slv",
                    solve_start=section_start,
                    solve_count=section_count,
                    url_by_key=url_by_key,
                )
                coeff_manifest = build_native_multispan_manifest(
                    solve_source_manifest,
                    source_family="cf",
                    solve_start=section_start,
                    solve_count=section_count,
                    url_by_key=url_by_key,
                )
                param_manifest = build_native_multispan_manifest(
                    solve_source_manifest,
                    source_family="pm",
                    solve_start=section_start,
                    solve_count=section_count,
                    url_by_key=url_by_key,
                )
                input_manifest_path = root / "input_manifest.json"
                input_manifest_path.write_text(json.dumps(input_manifest), encoding="utf-8")

                section_root_bytes = root_bytes[
                    section_start * root_row_bytes : (section_start + section_count) * root_row_bytes
                ]
                section_coeff_bytes = coeff_bytes[
                    section_start * coeff_row_bytes : (section_start + section_count) * coeff_row_bytes
                ]
                section_param_bytes = param_bytes[
                    section_start * param_row_bytes : (section_start + section_count) * param_row_bytes
                ]
                section_roots_path = root / "section_roots.bin"
                section_coeffs_path = root / "section_coeffs.bin"
                section_params_path = root / "section_params.bin"
                section_roots_path.write_bytes(section_root_bytes)
                section_coeffs_path.write_bytes(section_coeff_bytes)
                section_params_path.write_bytes(section_param_bytes)

                common_args = [
                    "--pix=64",
                    *self._bounds_args(64, 64, 0.0, 0.0, 3.0),
                    f"--degree={degree}",
                    "--rotation=0",
                    "--threads=1",
                    f"--step_count={section_count}",
                ]
                single_input_manifest_path = self._write_single_span_manifest(
                    root / "single_input_manifest.json",
                    file_name=section_roots_path.name,
                    port=server.server_address[1],
                    row_bytes=root_row_bytes,
                    solve_count=section_count,
                    source_family="slv",
                )
                single_coeff_manifest_path = self._write_single_span_manifest(
                    root / "single_coeff_manifest.json",
                    file_name=section_coeffs_path.name,
                    port=server.server_address[1],
                    row_bytes=coeff_row_bytes,
                    solve_count=section_count,
                    source_family="cf",
                )
                single_param_manifest_path = self._write_single_span_manifest(
                    root / "single_param_manifest.json",
                    file_name=section_params_path.name,
                    port=server.server_address[1],
                    row_bytes=param_row_bytes,
                    solve_count=section_count,
                    source_family="pm",
                )
                for case in self._score_case_payloads():
                    with self.subTest(case=case["label"]):
                        payload = case["payload"]
                        single_prefix = root / f"{case['label']}_single_pix"
                        single_fragment_prefix = root / f"{case['label']}_single_fragment"
                        ms_prefix = root / f"{case['label']}_ms_pix"
                        ms_fragment_prefix = root / f"{case['label']}_ms_fragment"

                        single_cmd = [
                            str(self._binary),
                            str(single_prefix),
                            *common_args,
                            f"--input_manifest={single_input_manifest_path}",
                            f"--score_metrics={payload['score_metrics']}",
                            f"--score_clip_los={payload['score_clip_los']}",
                            f"--score_clip_his={payload['score_clip_his']}",
                            f"--score_program={payload['score_program']}",
                            f"--fragment_prefix={single_fragment_prefix}",
                            "--retries=1",
                        ]
                        ms_cmd = [
                            str(self._binary),
                            str(ms_prefix),
                            *common_args,
                            f"--input_manifest={input_manifest_path}",
                            f"--score_metrics={payload['score_metrics']}",
                            f"--score_clip_los={payload['score_clip_los']}",
                            f"--score_clip_his={payload['score_clip_his']}",
                            f"--score_program={payload['score_program']}",
                            f"--fragment_prefix={ms_fragment_prefix}",
                            "--retries=1",
                        ]
                        if "score_sources" in payload:
                            single_cmd.append(f"--score_sources={payload['score_sources']}")
                            ms_cmd.append(f"--score_sources={payload['score_sources']}")

                        if case["uses_coeff"]:
                            chunked_coeff_manifest_path = root / f"{case['label']}_coeff_manifest.json"
                            chunked_coeff_manifest_path.write_text(json.dumps(coeff_manifest), encoding="utf-8")
                            single_cmd.extend([
                                f"--score_coeff_manifest={single_coeff_manifest_path}",
                                f"--score_coeff_degree={n_coeffs}",
                            ])
                            ms_cmd.extend([
                                f"--score_coeff_manifest={chunked_coeff_manifest_path}",
                                f"--score_coeff_degree={n_coeffs}",
                            ])
                        if case["uses_param"]:
                            chunked_param_manifest_path = root / f"{case['label']}_param_manifest.json"
                            chunked_param_manifest_path.write_text(json.dumps(param_manifest), encoding="utf-8")
                            single_cmd.append(f"--score_params_manifest={single_param_manifest_path}")
                            ms_cmd.append(f"--score_params_manifest={chunked_param_manifest_path}")

                        single_result = self._run_binary(single_cmd)
                        self.assertEqual(single_result.returncode, 0, single_result.stderr)
                        ms_result = self._run_binary(ms_cmd)
                        self.assertEqual(ms_result.returncode, 0, ms_result.stderr)

                        single_frag = (root / f"{case['label']}_single_fragment.frag").read_bytes()
                        ms_frag = (root / f"{case['label']}_ms_fragment.frag").read_bytes()

                        self.assertEqual(single_frag, ms_frag)
                        self.assertGreater(len(single_frag), 0)
            finally:
                server.shutdown()
                server.server_close()

    def test_associated_palette_emits_every_pass0_step_when_all_roots_clip_out(self):
        step_count = 4
        degree = 1
        roots = []
        for idx in range(step_count):
            roots.extend([1000.0 + idx, 1000.0 + idx])

        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            roots_path = self._write_float_file(root / "roots.bin", roots)
            out_prefix = root / "pix"
            fragment_prefix = root / "fragment"
            palette_prefix = root / "palette_fragment"
            server, thread = self._serve_dir(root)
            try:
                manifest_path = self._write_single_span_manifest(
                    root / "roots_manifest.json",
                    file_name=roots_path.name,
                    port=server.server_address[1],
                    row_bytes=degree * 2 * 4,
                    solve_count=step_count,
                )
                cmd = [
                    str(self._binary),
                    str(out_prefix),
                    "--pix=8",
                    *self._bounds_args(8, 8, 0.0, 0.0, 1.0),
                    f"--degree={degree}",
                    "--rotation=0",
                    "--threads=1",
                    f"--input_manifest={manifest_path}",
                    f"--step_count={step_count}",
                    *self._single_metric_program_args("centroid_re", 0, 2000),
                    f"--fragment_prefix={fragment_prefix}",
                    f"--associated_palette_fragment_prefix={palette_prefix}",
                    "--palette_grid_n=2",
                    "--palette_step_start=0",
                    "--retries=1",
                ]

                result = self._run_binary(cmd)
                self.assertEqual(result.returncode, 0, result.stderr)
                palette_pairs = self._read_u32le_u8_pairs(root / "palette_fragment.frag")
                self.assertEqual([pix for pix, _ in palette_pairs], [0, 1, 3, 2])
                self.assertEqual(len(palette_pairs), step_count)
                self.assertTrue(all(value > 0 for _, value in palette_pairs))
            finally:
                server.shutdown()
                server.server_close()
                thread.join(timeout=5)

    def test_associated_palette_emits_every_pass0_step_when_main_image_dedups_later_steps(self):
        step_count = 4
        degree = 1
        roots = [0.0, 0.0] * step_count

        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            roots_path = self._write_float_file(root / "roots.bin", roots)
            out_prefix = root / "pix"
            palette_prefix = root / "palette_fragment"
            fragment_prefix = root / "fragment"
            server, thread = self._serve_dir(root)
            try:
                manifest_path = self._write_single_span_manifest(
                    root / "roots_manifest.json",
                    file_name=roots_path.name,
                    port=server.server_address[1],
                    row_bytes=degree * 2 * 4,
                    solve_count=step_count,
                )
                cmd = [
                    str(self._binary),
                    str(out_prefix),
                    "--pix=8",
                    *self._bounds_args(8, 8, 0.0, 0.0, 1.0),
                    f"--degree={degree}",
                    "--rotation=0",
                    "--threads=1",
                    f"--input_manifest={manifest_path}",
                    f"--step_count={step_count}",
                    *self._single_metric_program_args("centroid_re", -1, 1),
                    f"--fragment_prefix={fragment_prefix}",
                    f"--associated_palette_fragment_prefix={palette_prefix}",
                    "--palette_grid_n=2",
                    "--palette_step_start=0",
                    "--retries=1",
                ]

                result = self._run_binary(cmd)
                self.assertEqual(result.returncode, 0, result.stderr)
                main_pairs = self._read_u32le_u8_pairs(root / "fragment.frag")
                palette_pairs = self._read_u32le_u8_pairs(root / "palette_fragment.frag")
                self.assertEqual(len(main_pairs), 1)
                self.assertEqual([pix for pix, _ in palette_pairs], [0, 1, 3, 2])
                self.assertEqual(len(palette_pairs), step_count)
                self.assertTrue(all(value > 0 for _, value in palette_pairs))
            finally:
                server.shutdown()
                server.server_close()
                thread.join(timeout=5)

    def test_associated_palette_emits_packed_three_channel_pass0_records(self):
        step_count = 4
        degree = 1
        roots = [0.0, 0.0, 0.33, 0.0, 0.66, 0.0, 1.0, 0.0]

        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            roots_path = self._write_float_file(root / "roots.bin", roots)
            out_prefix = root / "pix"
            fragment_prefix = root / "fragment"
            palette_prefix = root / "palette_fragment"
            server, thread = self._serve_dir(root)
            try:
                manifest_path = self._write_single_span_manifest(
                    root / "roots_manifest.json",
                    file_name=roots_path.name,
                    port=server.server_address[1],
                    row_bytes=degree * 2 * 4,
                    solve_count=step_count,
                )
                cmd = [
                    str(self._binary),
                    str(out_prefix),
                    "--pix=8",
                    *self._bounds_args(8, 8, 0.0, 0.0, 1.0),
                    f"--degree={degree}",
                    "--rotation=0",
                    "--threads=1",
                    f"--input_manifest={manifest_path}",
                    f"--step_count={step_count}",
                    "--score_metrics=centroid_re",
                    "--score_clip_los=0",
                    "--score_clip_his=1",
                    "--score_program=v2;m0-0;emit;m0-0;emit;m0-0;emit",
                    f"--fragment_prefix={fragment_prefix}",
                    f"--associated_palette_fragment_prefix={palette_prefix}",
                    "--palette_grid_n=2",
                    "--palette_step_start=0",
                    "--retries=1",
                ]

                result = self._run_binary(cmd)
                self.assertEqual(result.returncode, 0, result.stderr)
                palette_records = self._read_u32le_channel_records(root / "palette_fragment.frag", 3)
                self.assertEqual([pix for pix, _ in palette_records], [0, 1, 3, 2])
                self.assertEqual(len(palette_records), step_count)
                self.assertTrue(all(len(channels) == 3 for _, channels in palette_records))
                self.assertEqual(palette_records[0][1], (0, 0, 0))
                self.assertTrue(all(channels[0] == channels[1] == channels[2] for _, channels in palette_records))
            finally:
                server.shutdown()
                server.server_close()
                thread.join(timeout=5)

    def test_step_scores_capture_is_not_suppressed_by_root_clipping(self):
        step_count = 4
        degree = 1
        roots = []
        for idx in range(step_count):
            roots.extend([1000.0 + idx, 1000.0 + idx])

        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            roots_path = self._write_float_file(root / "roots.bin", roots)
            out_prefix = root / "pix"
            step_scores_path = root / "step_scores.raw"
            fragment_prefix = root / "fragment"
            server, thread = self._serve_dir(root)
            try:
                manifest_path = self._write_single_span_manifest(
                    root / "roots_manifest.json",
                    file_name=roots_path.name,
                    port=server.server_address[1],
                    row_bytes=degree * 2 * 4,
                    solve_count=step_count,
                )
                cmd = [
                    str(self._binary),
                    str(out_prefix),
                    "--pix=8",
                    *self._bounds_args(8, 8, 0.0, 0.0, 1.0),
                    f"--degree={degree}",
                    "--rotation=0",
                    "--threads=1",
                    f"--input_manifest={manifest_path}",
                    f"--step_count={step_count}",
                    *self._single_metric_program_args("centroid_re", 0, 2000),
                    f"--fragment_prefix={fragment_prefix}",
                    f"--step_scores_output={step_scores_path}",
                    "--retries=1",
                ]

                result = self._run_binary(cmd)
                self.assertEqual(result.returncode, 0, result.stderr)
                payload = step_scores_path.read_bytes()
                self.assertEqual(len(payload), step_count)
                self.assertTrue(all(value > 0 for value in payload))
            finally:
                server.shutdown()
                server.server_close()
                thread.join(timeout=5)

    def test_step_scores_capture_is_not_suppressed_by_main_image_dedup(self):
        step_count = 4
        degree = 1
        roots = [0.0, 0.0] * step_count

        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            roots_path = self._write_float_file(root / "roots.bin", roots)
            out_prefix = root / "pix"
            step_scores_path = root / "step_scores.raw"
            fragment_prefix = root / "fragment"
            server, thread = self._serve_dir(root)
            try:
                manifest_path = self._write_single_span_manifest(
                    root / "roots_manifest.json",
                    file_name=roots_path.name,
                    port=server.server_address[1],
                    row_bytes=degree * 2 * 4,
                    solve_count=step_count,
                )
                cmd = [
                    str(self._binary),
                    str(out_prefix),
                    "--pix=8",
                    *self._bounds_args(8, 8, 0.0, 0.0, 1.0),
                    f"--degree={degree}",
                    "--rotation=0",
                    "--threads=1",
                    f"--input_manifest={manifest_path}",
                    f"--step_count={step_count}",
                    *self._single_metric_program_args("centroid_re", -1, 1),
                    f"--fragment_prefix={fragment_prefix}",
                    f"--step_scores_output={step_scores_path}",
                    "--retries=1",
                ]

                result = self._run_binary(cmd)
                self.assertEqual(result.returncode, 0, result.stderr)
                main_pairs = self._read_u32le_u8_pairs(root / "fragment.frag")
                self.assertEqual(len(main_pairs), 1)
                payload = step_scores_path.read_bytes()
                self.assertEqual(len(payload), step_count)
                self.assertTrue(all(value > 0 for value in payload))
            finally:
                server.shutdown()
                server.server_close()
                thread.join(timeout=5)

    def test_score_output_normalization_expands_step_score_bytes(self):
        step_count = 2
        degree = 1
        roots = [0.2, 0.0, 0.4, 0.0]

        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            roots_path = self._write_float_file(root / "roots.bin", roots)
            out_prefix = root / "pix"
            step_scores_path = root / "step_scores.raw"
            fragment_prefix = root / "fragment"
            server, thread = self._serve_dir(root)
            try:
                manifest_path = self._write_single_span_manifest(
                    root / "roots_manifest.json",
                    file_name=roots_path.name,
                    port=server.server_address[1],
                    row_bytes=degree * 2 * 4,
                    solve_count=step_count,
                )
                cmd = [
                    str(self._binary),
                    str(out_prefix),
                    "--pix=8",
                    *self._bounds_args(8, 8, 0.0, 0.0, 1.0),
                    f"--degree={degree}",
                    "--rotation=0",
                    "--threads=1",
                    f"--input_manifest={manifest_path}",
                    f"--step_count={step_count}",
                    *self._single_metric_program_args("centroid_re", 0, 1),
                    "--score_output_normalize=1",
                    "--score_output_clip_lo=0.2",
                    "--score_output_clip_hi=0.4",
                    f"--fragment_prefix={fragment_prefix}",
                    f"--step_scores_output={step_scores_path}",
                    "--retries=1",
                ]

                result = self._run_binary(cmd)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(step_scores_path.read_bytes(), bytes([1, 255]))
            finally:
                server.shutdown()
                server.server_close()
                thread.join(timeout=5)

    def test_three_channel_step_scores_capture_solve_order_bytes(self):
        step_count = 2
        degree = 1
        roots = [0.0, 0.0, 0.5, 0.0]

        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            roots_path = self._write_float_file(root / "roots.bin", roots)
            out_prefix = root / "pix"
            step_scores_path = root / "step_scores.raw"
            fragment_prefix = root / "fragment"
            server, thread = self._serve_dir(root)
            try:
                manifest_path = self._write_single_span_manifest(
                    root / "roots_manifest.json",
                    file_name=roots_path.name,
                    port=server.server_address[1],
                    row_bytes=degree * 2 * 4,
                    solve_count=step_count,
                )
                cmd = [
                    str(self._binary),
                    str(out_prefix),
                    "--pix=8",
                    *self._bounds_args(8, 8, 0.0, 0.0, 1.0),
                    f"--degree={degree}",
                    "--rotation=0",
                    "--threads=1",
                    f"--input_manifest={manifest_path}",
                    f"--step_count={step_count}",
                    "--score_metrics=centroid_re",
                    "--score_clip_los=-1",
                    "--score_clip_his=1",
                    "--score_program=v2;m0-0;emit_none;flush;m0-0;emit_norm;m0-0;flip;emit_norm;m0-0;sawtooth:2;emit",
                    "--score_output_clip_los=0.5,0.25,0",
                    "--score_output_clip_his=0.75,0.5,1",
                    f"--fragment_prefix={fragment_prefix}",
                    f"--step_scores_output={step_scores_path}",
                    "--retries=1",
                ]

                result = self._run_binary(cmd)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(step_scores_path.read_bytes(), bytes([0, 255, 0, 255, 0, 128]))
            finally:
                server.shutdown()
                server.server_close()
                thread.join(timeout=5)

    def test_score_output_normalization_disabled_matches_default_bytes(self):
        roots = [0.2, 0.0, 0.4, 0.0]

        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            _, default_bytes = self._run_centroid_re_score_bytes(
                root,
                roots,
                label="default",
            )
            _, disabled_bytes = self._run_centroid_re_score_bytes(
                root,
                roots,
                label="disabled",
                extra_args=[
                    "--score_output_normalize=0",
                    "--score_output_clip_lo=0.2",
                    "--score_output_clip_hi=0.4",
                ],
            )

        self.assertEqual(disabled_bytes, default_bytes)

    def test_score_output_normalization_degenerate_range_warns_and_uses_identity(self):
        roots = [0.2, 0.0, 0.4, 0.0]

        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            _, default_bytes = self._run_centroid_re_score_bytes(
                root,
                roots,
                label="default_degenerate",
            )
            result, degenerate_bytes = self._run_centroid_re_score_bytes(
                root,
                roots,
                label="degenerate",
                extra_args=[
                    "--score_output_normalize=1",
                    "--score_output_clip_lo=0.4",
                    "--score_output_clip_hi=0.4",
                ],
            )

        self.assertEqual(degenerate_bytes, default_bytes)
        self.assertIn("solve_score_output_normalize: degenerate range [0.4,0.4], using identity", result.stderr)

    def test_moebius_all_zero_params_plots_nothing(self):
        step_count = 3
        degree = 2
        roots = [
            0.0, 0.0,
            1.0, 0.0,
            -1.0, 0.5,
            0.25, -0.75,
            2.0, 1.0,
            -2.0, -1.0,
        ]

        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            roots_path = self._write_float_file(root / "roots.bin", roots)
            out_prefix = root / "pix"
            fragment_prefix = root / "fragment"
            xforms_path = root / "root_xforms.json"
            xforms_path.write_text(json.dumps([["moebius", "0", "0", "0", "0"]]))
            server, thread = self._serve_dir(root)
            try:
                manifest_path = self._write_single_span_manifest(
                    root / "roots_manifest.json",
                    file_name=roots_path.name,
                    port=server.server_address[1],
                    row_bytes=degree * 2 * 4,
                    solve_count=step_count,
                )
                cmd = [
                    str(self._binary),
                    str(out_prefix),
                    "--pix=16",
                    *self._bounds_args(16, 16, 0.0, 0.0, 4.0),
                    f"--degree={degree}",
                    "--rotation=0",
                    "--threads=1",
                    f"--input_manifest={manifest_path}",
                    f"--step_count={step_count}",
                    *self._single_metric_program_args("proximity", 0, 1),
                    f"--fragment_prefix={fragment_prefix}",
                    "--retries=1",
                    f"--root_xforms={xforms_path}",
                ]

                result = self._run_binary(cmd)
                self.assertEqual(result.returncode, 0, result.stderr)
                meta = json.loads(result.stdout)
                self.assertEqual(meta["roots_plotted"], 0)
                self.assertEqual(meta["roots_clipped"], step_count * degree)
            finally:
                server.shutdown()
                server.server_close()
                thread.join(timeout=5)
