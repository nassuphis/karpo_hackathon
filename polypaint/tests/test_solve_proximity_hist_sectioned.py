import http.server
import json
import os
import pathlib
import socketserver
import struct
import subprocess
import sys
import tempfile
import threading
import unittest


ROOT = pathlib.Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"
sys.path.insert(0, str(LAMBDA_DIR))
SECTIONED_SRC = LAMBDA_DIR / "solve_proximity_hist_sectioned.c"
STATS_SRC = LAMBDA_DIR / "solve_proximity_stats.c"
MULTISPAN_SRC = LAMBDA_DIR / "multispan_reader.c"
MULTISPAN_HEADER = LAMBDA_DIR / "multispan_reader.h"
SCORE_HEADER = LAMBDA_DIR / "solve_score.h"
SECTIONED_BIN = pathlib.Path("/tmp/solve_proximity_hist_sectioned_host_test")
STATS_BIN = pathlib.Path("/tmp/solve_proximity_stats_host_test")


def _compile_binary(src: pathlib.Path, out: pathlib.Path, extra_args: list[str]):
    cc = os.environ.get("CC", "cc")
    r = subprocess.run(
        [cc, "-O3", "-pthread", "-o", str(out), str(src), *extra_args],
        capture_output=True,
        text=True,
        timeout=120,
    )
    if r.returncode != 0:
        raise unittest.SkipTest(f"could not compile {src.name}: {r.stderr.strip()}")


def _ensure_binaries():
    sectioned_src_mtime = max(
        SECTIONED_SRC.stat().st_mtime,
        MULTISPAN_SRC.stat().st_mtime,
        MULTISPAN_HEADER.stat().st_mtime,
        SCORE_HEADER.stat().st_mtime,
    )
    stats_src_mtime = max(STATS_SRC.stat().st_mtime, SCORE_HEADER.stat().st_mtime)
    if not SECTIONED_BIN.exists() or SECTIONED_BIN.stat().st_mtime < sectioned_src_mtime:
        _compile_binary(SECTIONED_SRC, SECTIONED_BIN, [str(MULTISPAN_SRC), "-lcurl", "-lm"])
    if not STATS_BIN.exists() or STATS_BIN.stat().st_mtime < stats_src_mtime:
        _compile_binary(STATS_SRC, STATS_BIN, ["-lm"])


def _write_bin(path: pathlib.Path, solves, degree: int):
    with path.open("wb") as f:
        for solve in solves:
            for re_val, im_val in solve:
                f.write(struct.pack("<ff", re_val, im_val))
            for _ in range(degree - len(solve)):
                f.write(struct.pack("<ff", 0.0, 0.0))


class _RangeHandler(http.server.BaseHTTPRequestHandler):
    file_bytes = b""

    def do_GET(self):
        root_dir = getattr(self.server, "root_dir", None)
        if root_dir:
            rel = self.path.lstrip("/")
            target = pathlib.Path(root_dir) / rel
            if not target.is_file():
                self.send_response(404)
                self.end_headers()
                return
            data = target.read_bytes()
        else:
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


class _ThreadedTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    allow_reuse_address = True
    daemon_threads = True


class TestSolveProximityHistSectioned(unittest.TestCase):
    def _score_case_specs(self):
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
                "clips": {"crowding": (0.0, 1.0), "spread": (0.0, 2.0)},
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
                "clips": {"crowding": (0.0, 1.0), "spread": (0.0, 2.0), "t1_abs": (0.0, 2.0)},
                "uses_coeff": True,
                "uses_param": True,
            },
        ]
        out = []
        for case in cases:
            compiled = compile_solve_score_chain(case["chain"])
            metrics = []
            for metric in compiled["metrics"]:
                lo, hi = case["clips"][metric["metric"]]
                metrics.append({**metric, "clip_lo": lo, "clip_hi": hi})
            out.append({
                **case,
                "payload": solve_score_program_cli_payload({"metrics": metrics, "program_spec": compiled["program_spec"]}),
            })
        return out

    def test_stats_binary_rejects_malformed_lag_program_tokens(self):
        _ensure_binaries()
        with tempfile.TemporaryDirectory(prefix="solve_stats_bad_lag_") as tmpdir:
            bin_path = pathlib.Path(tmpdir) / "input.bin"
            _write_bin(bin_path, [[(0.0, 0.0), (1.0, 0.0)]], 2)
            for token in ("m0-2", "m0--1", "m0-01", "M0-1", "m0 -1", "m-1"):
                with self.subTest(token=token):
                    result = subprocess.run(
                        [
                            str(STATS_BIN),
                            str(bin_path),
                            "--mode=hist",
                            "--degree=2",
                            "--clip_lo=0",
                            "--clip_hi=1",
                            "--hist_bins=4",
                            "--score_metrics=proximity",
                            "--score_clip_los=0",
                            "--score_clip_his=1",
                            f"--score_program={token}",
                        ],
                        capture_output=True,
                        text=True,
                        timeout=30,
                    )
                    self.assertNotEqual(result.returncode, 0, token)
                    self.assertIn("Invalid score program", result.stderr)

    def test_stats_binary_rejects_lagged_score_sources_csv(self):
        _ensure_binaries()
        with tempfile.TemporaryDirectory(prefix="solve_stats_bad_source_lag_") as tmpdir:
            bin_path = pathlib.Path(tmpdir) / "input.bin"
            _write_bin(bin_path, [[(0.0, 0.0), (1.0, 0.0)]], 2)
            for source in ("slv-1", "cf-1", "pm-1"):
                with self.subTest(source=source):
                    result = subprocess.run(
                        [
                            str(STATS_BIN),
                            str(bin_path),
                            "--mode=hist",
                            "--degree=2",
                            "--clip_lo=0",
                            "--clip_hi=1",
                            "--hist_bins=4",
                            "--score_metrics=proximity",
                            f"--score_sources={source}",
                            "--score_clip_los=0",
                            "--score_clip_his=1",
                            "--score_program=m0",
                        ],
                        capture_output=True,
                        text=True,
                        timeout=30,
                    )
                    self.assertNotEqual(result.returncode, 0, source)
                    self.assertIn("Invalid score program", result.stderr)

    def test_sectioned_hist_matches_existing_hist(self):
        _ensure_binaries()
        solves = [
            [(0.0, 0.0), (1.0, 0.0)],
            [(0.0, 0.0), (0.1, 0.0)],
            [(0.0, 0.0), (0.01, 0.0)],
            [(0.0, 0.0), (0.001, 0.0)],
        ]
        with tempfile.TemporaryDirectory(prefix="solve_hist_sectioned_") as tmpdir:
            bin_path = pathlib.Path(tmpdir) / "input.bin"
            _write_bin(bin_path, solves, 2)
            file_bytes = bin_path.read_bytes()
            _RangeHandler.file_bytes = file_bytes
            with _ThreadedTCPServer(("127.0.0.1", 0), _RangeHandler) as httpd:
                port = httpd.server_address[1]
                thread = threading.Thread(target=httpd.serve_forever, daemon=True)
                thread.start()
                try:
                    clip = subprocess.run(
                        [
                            str(STATS_BIN),
                            str(bin_path),
                            "--mode=clip",
                            "--degree=2",
                            "--metric=proximity",
                            "--threads=1",
                        ],
                        capture_output=True,
                        text=True,
                        timeout=30,
                    )
                    self.assertEqual(clip.returncode, 0, clip.stderr)
                    clip_data = json.loads(clip.stdout)

                    ref_hist = subprocess.run(
                        [
                            str(STATS_BIN),
                            str(bin_path),
                            "--mode=hist",
                            "--degree=2",
                            "--metric=proximity",
                            f"--clip_lo={clip_data['clip_lo']}",
                            f"--clip_hi={clip_data['clip_hi']}",
                            "--hist_bins=8",
                            "--threads=1",
                        ],
                        capture_output=True,
                        text=True,
                        timeout=30,
                    )
                    self.assertEqual(ref_hist.returncode, 0, ref_hist.stderr)
                    ref_data = json.loads(ref_hist.stdout)

                    url = f"http://127.0.0.1:{port}/input.bin"
                    sectioned = subprocess.run(
                        [
                            str(SECTIONED_BIN),
                            f"--url={url}",
                            f"--input_size={len(file_bytes)}",
                            "--degree=2",
                            "--metric=proximity",
                            f"--clip_lo={clip_data['clip_lo']}",
                            f"--clip_hi={clip_data['clip_hi']}",
                            "--hist_bins=8",
                            "--threads=2",
                        ],
                        capture_output=True,
                        text=True,
                        timeout=30,
                    )
                    self.assertEqual(sectioned.returncode, 0, sectioned.stderr)
                    data = json.loads(sectioned.stdout)
                finally:
                    httpd.shutdown()
                    thread.join(timeout=5)

        self.assertEqual(data["n_solves"], ref_data["n_solves"])
        self.assertEqual(data["hist"], ref_data["hist"])
        self.assertEqual(data["threads"], 2)
        self.assertGreaterEqual(data["download_ms"], 0)
        self.assertGreaterEqual(data["compute_ms"], 0)
        self.assertGreaterEqual(data["wall_ms"], 0)

    def test_sectioned_hist_accepts_binary_and_unary_score_programs(self):
        _ensure_binaries()
        solves = [
            [(0.0, 0.0), (1.0, 0.0)],
            [(0.0, 0.0), (0.5, 0.2)],
            [(0.0, 0.0), (0.1, -0.3)],
            [(0.0, 0.0), (0.01, 0.4)],
        ]
        with tempfile.TemporaryDirectory(prefix="solve_hist_sectioned_prog_") as tmpdir:
            bin_path = pathlib.Path(tmpdir) / "input.bin"
            _write_bin(bin_path, solves, 2)
            file_bytes = bin_path.read_bytes()
            _RangeHandler.file_bytes = file_bytes
            with _ThreadedTCPServer(("127.0.0.1", 0), _RangeHandler) as httpd:
                port = httpd.server_address[1]
                thread = threading.Thread(target=httpd.serve_forever, daemon=True)
                thread.start()
                try:
                    clip_prox = subprocess.run(
                        [str(STATS_BIN), str(bin_path), "--mode=clip", "--degree=2", "--metric=proximity", "--threads=1"],
                        capture_output=True,
                        text=True,
                        timeout=30,
                    )
                    self.assertEqual(clip_prox.returncode, 0, clip_prox.stderr)
                    prox = json.loads(clip_prox.stdout)

                    clip_crowding = subprocess.run(
                        [str(STATS_BIN), str(bin_path), "--mode=clip", "--degree=2", "--metric=crowding", "--threads=1"],
                        capture_output=True,
                        text=True,
                        timeout=30,
                    )
                    self.assertEqual(clip_crowding.returncode, 0, clip_crowding.stderr)
                    crowding = json.loads(clip_crowding.stdout)

                    program_cases = [
                        {
                            "program": "m0;m1;mul",
                            "metrics": "proximity,crowding",
                            "clip_los": f"{prox['clip_lo']},{crowding['clip_lo']}",
                            "clip_his": f"{prox['clip_hi']},{crowding['clip_hi']}",
                        },
                        {
                            "program": "m0;m1;max",
                            "metrics": "proximity,crowding",
                            "clip_los": f"{prox['clip_lo']},{crowding['clip_lo']}",
                            "clip_his": f"{prox['clip_hi']},{crowding['clip_hi']}",
                        },
                        {
                            "program": "m0;flip",
                            "metrics": "proximity",
                            "clip_los": f"{prox['clip_lo']}",
                            "clip_his": f"{prox['clip_hi']}",
                        },
                        {
                            "program": "m0;sawtooth:10",
                            "metrics": "proximity",
                            "clip_los": f"{prox['clip_lo']}",
                            "clip_his": f"{prox['clip_hi']}",
                        },
                        {
                            "program": "m0;omega_cosine:3:1.57079632679",
                            "metrics": "proximity",
                            "clip_los": f"{prox['clip_lo']}",
                            "clip_his": f"{prox['clip_hi']}",
                        },
                    ]

                    for case in program_cases:
                        program = case["program"]
                        ref_hist = subprocess.run(
                            [
                                str(STATS_BIN),
                                str(bin_path),
                                "--mode=hist",
                                "--degree=2",
                                "--clip_lo=0",
                                "--clip_hi=1",
                                "--hist_bins=8",
                                "--threads=1",
                                f"--score_metrics={case['metrics']}",
                                f"--score_clip_los={case['clip_los']}",
                                f"--score_clip_his={case['clip_his']}",
                                f"--score_program={program}",
                            ],
                            capture_output=True,
                            text=True,
                            timeout=30,
                        )
                        self.assertEqual(ref_hist.returncode, 0, ref_hist.stderr)
                        ref_data = json.loads(ref_hist.stdout)

                        url = f"http://127.0.0.1:{port}/input.bin"
                        sectioned = subprocess.run(
                            [
                                str(SECTIONED_BIN),
                                f"--url={url}",
                                f"--input_size={len(file_bytes)}",
                                "--degree=2",
                                "--clip_lo=0",
                                "--clip_hi=1",
                                "--hist_bins=8",
                                "--threads=2",
                                f"--score_metrics={case['metrics']}",
                                f"--score_clip_los={case['clip_los']}",
                                f"--score_clip_his={case['clip_his']}",
                                f"--score_program={program}",
                            ],
                            capture_output=True,
                            text=True,
                            timeout=30,
                        )
                        self.assertEqual(sectioned.returncode, 0, sectioned.stderr)
                        data = json.loads(sectioned.stdout)
                        self.assertEqual(data["n_solves"], ref_data["n_solves"], program)
                        self.assertEqual(data["hist"], ref_data["hist"], program)
                finally:
                    httpd.shutdown()
                    thread.join(timeout=5)

    def test_multispan_sectioned_hist_matches_tmpfile_for_logical_section_mixed_sources(self):
        from logical_sections import build_native_multispan_manifest, build_solve_source_manifest

        _ensure_binaries()
        degree = 2
        n_coeffs = 2
        total_solves = 6
        section_start = 1
        section_count = 4
        chunk_steps = [2, 1, 3]
        solves = []
        coeff_solves = []
        params_rows = []
        for idx in range(total_solves):
            solves.append([
                (0.15 + 0.10 * idx, -0.05 * idx),
                (0.80 - 0.06 * idx, 0.12 * idx - 0.08),
            ])
            coeff_solves.append([
                (0.95 - 0.07 * idx, 0.05 * idx),
                (-0.25 + 0.04 * idx, 0.20 - 0.03 * idx),
            ])
            params_rows.append(
                (
                    0.20 + 0.09 * idx,
                    -0.03 * idx,
                    0.75 - 0.05 * idx,
                    0.02 * idx,
                )
            )
        with tempfile.TemporaryDirectory(prefix="solve_hist_multispan_") as tmpdir:
            root = pathlib.Path(tmpdir)
            roots_path = root / "input.bin"
            coeffs_path = root / "coeffs.bin"
            params_path = root / "params.bin"
            _write_bin(roots_path, solves, degree)
            _write_bin(coeffs_path, coeff_solves, n_coeffs)
            with params_path.open("wb") as f:
                for row in params_rows:
                    f.write(struct.pack("<ffff", *row))

            input_bytes = roots_path.read_bytes()
            coeff_bytes = coeffs_path.read_bytes()
            params_bytes = params_path.read_bytes()
            _RangeHandler.file_bytes = b""
            root_row_bytes = degree * 2 * 4
            coeff_row_bytes = n_coeffs * 2 * 4
            param_row_bytes = 4 * 4

            chunk_items = []
            cursor = 0
            for idx, count in enumerate(chunk_steps):
                start = cursor
                end = cursor + count
                root_key = f"chunk_{idx}.bin"
                coeff_key = f"coeffs_{idx}.bin"
                param_key = f"params_{idx}.bin"
                (root / root_key).write_bytes(input_bytes[start * root_row_bytes:end * root_row_bytes])
                (root / coeff_key).write_bytes(coeff_bytes[start * coeff_row_bytes:end * coeff_row_bytes])
                (root / param_key).write_bytes(params_bytes[start * param_row_bytes:end * param_row_bytes])
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

            section_roots_path = root / "section_roots.bin"
            section_coeffs_path = root / "section_coeffs.bin"
            section_params_path = root / "section_params.bin"
            section_roots_path.write_bytes(
                input_bytes[section_start * root_row_bytes:(section_start + section_count) * root_row_bytes]
            )
            section_coeffs_path.write_bytes(
                coeff_bytes[section_start * coeff_row_bytes:(section_start + section_count) * coeff_row_bytes]
            )
            section_params_path.write_bytes(
                params_bytes[section_start * param_row_bytes:(section_start + section_count) * param_row_bytes]
            )

            with _ThreadedTCPServer(("127.0.0.1", 0), _RangeHandler) as httpd:
                port = httpd.server_address[1]
                httpd.root_dir = str(root)
                thread = threading.Thread(target=httpd.serve_forever, daemon=True)
                thread.start()
                try:
                    solve_source_manifest = build_solve_source_manifest(
                        chunk_items,
                        job_id="hist_parity_job",
                        degree=degree,
                        n_coeffs=n_coeffs,
                    )
                    url_by_key = {
                        item_key: f"http://127.0.0.1:{port}/{item_key}"
                        for item_key in [
                            *(item["bin_key"] for item in chunk_items),
                            *(item["coeffs_key"] for item in chunk_items),
                            *(item["params_key"] for item in chunk_items),
                        ]
                    }
                    input_manifest = build_native_multispan_manifest(
                        solve_source_manifest,
                        source_family="slv",
                        solve_start=section_start,
                        solve_count=section_count,
                        url_by_key=url_by_key,
                    )
                    input_manifest_path = root / "input_manifest.json"
                    input_manifest_path.write_text(json.dumps(input_manifest), encoding="utf-8")
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

                    for case in self._score_case_specs():
                        with self.subTest(case=case["label"]):
                            payload = case["payload"]
                            ref_cmd = [
                                str(STATS_BIN),
                                str(section_roots_path),
                                "--mode=hist",
                                f"--degree={degree}",
                                "--clip_lo=0",
                                "--clip_hi=1",
                                "--hist_bins=8",
                                "--threads=1",
                                f"--score_metrics={payload['score_metrics']}",
                                f"--score_clip_los={payload['score_clip_los']}",
                                f"--score_clip_his={payload['score_clip_his']}",
                                f"--score_program={payload['score_program']}",
                            ]
                            sectioned_cmd = [
                                str(SECTIONED_BIN),
                                "--input_mode=multispan_sectioned",
                                f"--input_manifest={input_manifest_path}",
                                f"--degree={degree}",
                                "--clip_lo=0",
                                "--clip_hi=1",
                                "--hist_bins=8",
                                "--threads=2",
                                "--retries=1",
                                f"--score_metrics={payload['score_metrics']}",
                                f"--score_clip_los={payload['score_clip_los']}",
                                f"--score_clip_his={payload['score_clip_his']}",
                                f"--score_program={payload['score_program']}",
                            ]
                            if "score_sources" in payload:
                                ref_cmd.append(f"--score_sources={payload['score_sources']}")
                                sectioned_cmd.append(f"--score_sources={payload['score_sources']}")
                            if case["uses_coeff"]:
                                coeff_manifest_path = root / f"{case['label']}_coeff_manifest.json"
                                coeff_manifest_path.write_text(json.dumps(coeff_manifest), encoding="utf-8")
                                ref_cmd.extend([
                                    f"--score_coeffs_file={section_coeffs_path}",
                                    f"--score_coeff_degree={n_coeffs}",
                                ])
                                sectioned_cmd.extend([
                                    f"--score_coeff_manifest={coeff_manifest_path}",
                                    f"--score_coeff_degree={n_coeffs}",
                                ])
                            if case["uses_param"]:
                                param_manifest_path = root / f"{case['label']}_param_manifest.json"
                                param_manifest_path.write_text(json.dumps(param_manifest), encoding="utf-8")
                                ref_cmd.append(f"--score_params_file={section_params_path}")
                                sectioned_cmd.append(f"--score_params_manifest={param_manifest_path}")

                            ref_hist = subprocess.run(
                                ref_cmd,
                                capture_output=True,
                                text=True,
                                timeout=30,
                            )
                            self.assertEqual(ref_hist.returncode, 0, ref_hist.stderr)
                            ref_data = json.loads(ref_hist.stdout)

                            sectioned = subprocess.run(
                                sectioned_cmd,
                                capture_output=True,
                                text=True,
                                timeout=30,
                            )
                            self.assertEqual(sectioned.returncode, 0, sectioned.stderr)
                            data = json.loads(sectioned.stdout)
                            self.assertEqual(data["n_solves"], ref_data["n_solves"])
                            self.assertEqual(data["hist"], ref_data["hist"])
                finally:
                    httpd.shutdown()
                    thread.join(timeout=5)

    def test_multispan_sectioned_lagged_program_uses_prelude_row(self):
        from logical_sections import build_native_multispan_manifest, build_solve_source_manifest

        _ensure_binaries()
        degree = 2
        row_bytes = degree * 2 * 4
        rows = [
            [(0.0, 0.0), (0.0, 0.0)],
            [(5.0, 0.0), (5.0, 0.0)],
            [(10.0, 0.0), (10.0, 0.0)],
        ]

        with tempfile.TemporaryDirectory(prefix="solve_hist_lag_") as tmpdir:
            root = pathlib.Path(tmpdir)
            _write_bin(root / "roots_0.bin", [rows[0]], degree)
            _write_bin(root / "roots_1.bin", [rows[1], rows[2]], degree)
            chunk_items = [
                {"chunk_idx": 0, "bin_key": "roots_0.bin", "step_start": 0, "step_count": 1, "bin_size": row_bytes},
                {"chunk_idx": 1, "bin_key": "roots_1.bin", "step_start": 1, "step_count": 2, "bin_size": row_bytes * 2},
            ]
            solve_source_manifest = build_solve_source_manifest(
                chunk_items,
                job_id="lag_job",
                degree=degree,
                n_coeffs=degree,
            )
            with _ThreadedTCPServer(("127.0.0.1", 0), _RangeHandler) as httpd:
                port = httpd.server_address[1]
                httpd.root_dir = str(root)
                thread = threading.Thread(target=httpd.serve_forever, daemon=True)
                thread.start()
                try:
                    url_by_key = {
                        "roots_0.bin": f"http://127.0.0.1:{port}/roots_0.bin",
                        "roots_1.bin": f"http://127.0.0.1:{port}/roots_1.bin",
                    }
                    input_manifest = build_native_multispan_manifest(
                        solve_source_manifest,
                        source_family="slv",
                        solve_start=0,
                        solve_count=3,
                        url_by_key=url_by_key,
                    )
                    manifest_path = root / "input_manifest.json"
                    manifest_path.write_text(json.dumps(input_manifest), encoding="utf-8")
                    result = subprocess.run(
                        [
                            str(SECTIONED_BIN),
                            "--input_mode=multispan_sectioned",
                            f"--input_manifest={manifest_path}",
                            "--degree=2",
                            "--hist_bins=10",
                            "--threads=2",
                            "--step_count=2",
                            "--prelude_rows=1",
                            "--score_metrics=centroid_re",
                            "--score_clip_los=0",
                            "--score_clip_his=10",
                            "--score_program=m0-0;m0-1;abs_diff",
                        ],
                        capture_output=True,
                        text=True,
                        timeout=30,
                    )
                    self.assertEqual(result.returncode, 0, result.stderr)
                    data = json.loads(result.stdout)
                    self.assertEqual(data["n_solves"], 2)
                    self.assertEqual(data["hist"][5], 2)
                    self.assertEqual(sum(data["hist"]), 2)
                finally:
                    httpd.shutdown()
                    thread.join(timeout=5)


if __name__ == "__main__":
    unittest.main()
