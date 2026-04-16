import http.server
import json
import os
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


class TestPaletteChunkMtParity(unittest.TestCase):
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
        cls._binary = cls._workdir / "solve_palette_chunk_mt_test"
        compile_cmd = [
            cc,
            "-O2",
            "-I",
            str(LAMBDA_DIR),
            str(LAMBDA_DIR / "solve_palette_chunk_mt.c"),
            str(LAMBDA_DIR / "multispan_reader.c"),
            "-lcurl",
            "-lm",
            "-lpthread",
            "-o",
            str(cls._binary),
        ]
        result = subprocess.run(compile_cmd, capture_output=True, text=True)
        if result.returncode != 0:
            raise AssertionError(f"failed to compile solve_palette_chunk_mt parity binary: {result.stderr}")

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
        with open(path, "wb") as f:
            arr.tofile(f)
        return path

    def _run_binary(self, args):
        return subprocess.run(args, capture_output=True, text=True)

    def test_tmpfile_and_multispan_sectioned_outputs_match_for_mixed_sources(self):
        from logical_sections import build_native_multispan_manifest, build_solve_source_manifest

        degree = 3
        n_coeffs = 2
        step_count = 6
        section_start = 1
        section_count = 4
        root_stride = degree * 2
        coeff_stride = n_coeffs * 2
        param_stride = 4
        chunk_steps = [2, 1, 3]

        roots = []
        coeffs = []
        params = []
        for idx in range(step_count):
            roots.extend(
                [
                    0.45 + 0.10 * idx,
                    -0.05 * idx,
                    -0.25 + 0.03 * idx,
                    0.35 - 0.02 * idx,
                    1.10 + 0.04 * idx,
                    -0.10 + 0.01 * idx,
                ]
            )
            coeffs.extend(
                [
                    -0.40 + 0.08 * idx,
                    0.15 - 0.03 * idx,
                    0.90 - 0.05 * idx,
                    -0.20 + 0.04 * idx,
                ]
            )
            params.extend(
                [
                    0.30 + 0.07 * idx,
                    -0.05 * idx,
                    0.80 + 0.02 * idx,
                    0.01 * idx,
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
                job_id="parity_job",
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
                self.assertEqual(len(input_manifest["spans"]), 3)
                self.assertEqual(len(coeff_manifest["spans"]), 3)
                self.assertEqual(len(param_manifest["spans"]), 3)

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

                tmp_scores = root / "tmp_scores.bin"
                tmp_bins = root / "tmp_bins.bin"
                ms_scores = root / "ms_scores.bin"
                ms_bins = root / "ms_bins.bin"

                common_args = [
                    "--degree=3",
                    "--metric=crowding",
                    "--clip_lo=0",
                    "--clip_hi=1",
                    "--cuts=0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9",
                    f"--step_count={section_count}",
                    "--threads=2",
                ]
                for case in self._score_case_payloads():
                    with self.subTest(case=case["label"]):
                        payload = case["payload"]
                        tmp_scores = root / f"{case['label']}_tmp_scores.bin"
                        tmp_bins = root / f"{case['label']}_tmp_bins.bin"
                        ms_scores = root / f"{case['label']}_ms_scores.bin"
                        ms_bins = root / f"{case['label']}_ms_bins.bin"

                        tmp_cmd = [
                            str(self._binary),
                            str(section_roots_path),
                            *common_args,
                            f"--scores_out={tmp_scores}",
                            f"--bins_out={tmp_bins}",
                            "--input_mode=tmpfile",
                            f"--score_metrics={payload['score_metrics']}",
                            f"--score_clip_los={payload['score_clip_los']}",
                            f"--score_clip_his={payload['score_clip_his']}",
                            f"--score_program={payload['score_program']}",
                        ]
                        ms_cmd = [
                            str(self._binary),
                            str(section_roots_path),
                            *common_args,
                            f"--scores_out={ms_scores}",
                            f"--bins_out={ms_bins}",
                            "--input_mode=multispan_sectioned",
                            f"--input_manifest={input_manifest_path}",
                            f"--score_metrics={payload['score_metrics']}",
                            f"--score_clip_los={payload['score_clip_los']}",
                            f"--score_clip_his={payload['score_clip_his']}",
                            f"--score_program={payload['score_program']}",
                        ]
                        if "score_sources" in payload:
                            tmp_cmd.append(f"--score_sources={payload['score_sources']}")
                            ms_cmd.append(f"--score_sources={payload['score_sources']}")

                        if case["uses_coeff"]:
                            coeff_manifest_path = root / f"{case['label']}_coeff_manifest.json"
                            coeff_manifest_path.write_text(json.dumps(coeff_manifest), encoding="utf-8")
                            tmp_cmd.extend([
                                f"--score_coeffs_file={section_coeffs_path}",
                                f"--score_coeff_degree={n_coeffs}",
                            ])
                            ms_cmd.extend([
                                f"--score_coeff_manifest={coeff_manifest_path}",
                                f"--score_coeff_degree={n_coeffs}",
                            ])
                        if case["uses_param"]:
                            param_manifest_path = root / f"{case['label']}_param_manifest.json"
                            param_manifest_path.write_text(json.dumps(param_manifest), encoding="utf-8")
                            tmp_cmd.append(f"--score_params_file={section_params_path}")
                            ms_cmd.append(f"--score_params_manifest={param_manifest_path}")

                        tmp_result = self._run_binary(tmp_cmd)
                        self.assertEqual(tmp_result.returncode, 0, tmp_result.stderr)
                        ms_result = self._run_binary(ms_cmd)
                        self.assertEqual(ms_result.returncode, 0, ms_result.stderr)

                        self.assertEqual(tmp_scores.read_bytes(), ms_scores.read_bytes())
                        self.assertEqual(tmp_bins.read_bytes(), ms_bins.read_bytes())
            finally:
                server.shutdown()
                server.server_close()
