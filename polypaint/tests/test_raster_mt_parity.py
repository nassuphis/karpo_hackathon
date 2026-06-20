import http.server
import json
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
