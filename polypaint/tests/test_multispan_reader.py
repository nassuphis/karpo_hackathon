import http.server
import json
import pathlib
import shutil
import socketserver
import subprocess
import tempfile
import threading
import unittest


ROOT = pathlib.Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"


class _RangeRequestHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        rel = self.path.lstrip("/")
        target = pathlib.Path(self.server.root_dir) / rel
        self.server.request_counts[rel] = self.server.request_counts.get(rel, 0) + 1
        if self.server.failures_before_success.get(rel, 0) > 0:
            self.server.failures_before_success[rel] -= 1
            self.send_response(503)
            self.send_header("Content-Length", "0")
            self.end_headers()
            return
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
            spec = range_header[len("bytes="):]
            start_s, end_s = spec.split("-", 1)
            start = int(start_s)
            end = int(end_s) if end_s else len(data) - 1
            if start < 0 or end < start or end >= len(data):
                self.send_error(416)
                return
            status = 206
            data = data[start:end + 1]

        truncate_to = self.server.truncate_bytes_by_path.get(rel)
        if truncate_to is not None:
            data = data[:truncate_to]

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


class TestMultispanReader(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls._tmpdir_obj = tempfile.TemporaryDirectory()
        cls._workdir = pathlib.Path(cls._tmpdir_obj.name)
        cc = shutil.which("cc") or shutil.which("clang") or shutil.which("gcc")
        if not cc:
            raise unittest.SkipTest("no C compiler available")
        harness_c = cls._workdir / "multispan_reader_harness.c"
        cls._harness_bin = cls._workdir / "multispan_reader_harness"
        harness_c.write_text(
            r'''
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include "multispan_reader.h"

int main(int argc, char **argv) {
    MultiSpanReader reader;
    char err[512] = {0};
    int rc = 0;

    if (argc < 4 || ((argc - 2) % 2) != 0) {
        fprintf(stderr, "usage: harness manifest offset length [offset length ...]\n");
        return 2;
    }
    if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
        fprintf(stderr, "curl_global_init failed\n");
        return 3;
    }
        if (!multispan_reader_open(&reader, argv[1], 2, err, sizeof(err))) {
            fprintf(stderr, "%s\n", err);
            curl_global_cleanup();
            return 4;
        }
    for (int i = 2; i < argc; i += 2) {
        unsigned long long offset = strtoull(argv[i], NULL, 10);
        size_t length = (size_t)strtoull(argv[i + 1], NULL, 10);
        long bytes_downloaded = 0;
        unsigned char *buf = (unsigned char *)malloc(length > 0 ? length : 1);
        if (!buf) {
            fprintf(stderr, "out of memory\n");
            multispan_reader_close(&reader);
            multispan_reader_thread_cleanup();
            curl_global_cleanup();
            return 5;
        }
        if (!multispan_reader_read_exact(&reader, offset, length, buf, &bytes_downloaded, err, sizeof(err))) {
            fprintf(stderr, "%s\n", err);
            free(buf);
            multispan_reader_close(&reader);
            multispan_reader_thread_cleanup();
            curl_global_cleanup();
            return 6;
        }
        if (fwrite(buf, 1, length, stdout) != length) {
            fprintf(stderr, "short stdout write\n");
            free(buf);
            multispan_reader_close(&reader);
            multispan_reader_thread_cleanup();
            curl_global_cleanup();
            return 7;
        }
        free(buf);
    }
    multispan_reader_close(&reader);
    multispan_reader_thread_cleanup();
    curl_global_cleanup();
    return rc;
}
''',
            encoding="utf-8",
        )
        compile_cmd = [
            cc,
            "-O2",
            "-I",
            str(LAMBDA_DIR),
            str(harness_c),
            str(LAMBDA_DIR / "multispan_reader.c"),
            "-pthread",
            "-lcurl",
            "-o",
            str(cls._harness_bin),
        ]
        result = subprocess.run(compile_cmd, capture_output=True, text=True)
        if result.returncode != 0:
            raise AssertionError(f"failed to compile multispan harness: {result.stderr}")

    @classmethod
    def tearDownClass(cls):
        cls._tmpdir_obj.cleanup()

    def _serve_dir(self, root_dir, *, failures_before_success=None, truncate_bytes_by_path=None):
        server = _ThreadedTCPServer(("127.0.0.1", 0), _RangeRequestHandler)
        server.root_dir = str(root_dir)
        server.failures_before_success = dict(failures_before_success or {})
        server.truncate_bytes_by_path = dict(truncate_bytes_by_path or {})
        server.request_counts = {}
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        return server, thread

    def _write_sources(self, root, names_to_bytes):
        for name, data in names_to_bytes.items():
            (root / name).write_bytes(data)

    def _manifest(self, port, *, logical_size, spans):
        source_names = []
        for span in spans:
            source_names.append(span["name"])
        source_names = list(dict.fromkeys(source_names))
        sources = [
            {"id": idx, "url": f"http://127.0.0.1:{port}/{name}", "key": name}
            for idx, name in enumerate(source_names)
        ]
        source_ids = {name: idx for idx, name in enumerate(source_names)}
        return {
            "logical_size": logical_size,
            "row_bytes": 1,
            "solve_start": 0,
            "solve_count": logical_size,
            "sources": sources,
            "spans": [
                {
                    "source_id": source_ids[span["name"]],
                    "logical_byte_start": span["logical_byte_start"],
                    "byte_start": span["byte_start"],
                    "byte_length": span["byte_length"],
                }
                for span in spans
            ],
        }

    def _run_harness(self, manifest_path, *pairs):
        cmd = [str(self._harness_bin), str(manifest_path)]
        for offset, length in pairs:
            cmd.extend([str(offset), str(length)])
        return subprocess.run(cmd, capture_output=True)

    def test_reads_inside_one_span(self):
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            self._write_sources(root, {"a.bin": b"ABCDEFGH"})
            server, _ = self._serve_dir(root)
            try:
                manifest = self._manifest(server.server_address[1], logical_size=4, spans=[
                    {"name": "a.bin", "logical_byte_start": 0, "byte_start": 2, "byte_length": 4},
                ])
                manifest_path = root / "inside.json"
                manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
                result = self._run_harness(manifest_path, (1, 2))
                self.assertEqual(result.returncode, 0, result.stderr.decode())
                self.assertEqual(result.stdout, b"DE")
            finally:
                server.shutdown()
                server.server_close()

    def test_read_ending_on_span_boundary(self):
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            self._write_sources(root, {"a.bin": b"ABCDEFGH", "b.bin": b"ijklmnop"})
            server, _ = self._serve_dir(root)
            try:
                manifest = self._manifest(server.server_address[1], logical_size=8, spans=[
                    {"name": "a.bin", "logical_byte_start": 0, "byte_start": 2, "byte_length": 4},
                    {"name": "b.bin", "logical_byte_start": 4, "byte_start": 1, "byte_length": 4},
                ])
                manifest_path = root / "end_boundary.json"
                manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
                result = self._run_harness(manifest_path, (0, 4))
                self.assertEqual(result.returncode, 0, result.stderr.decode())
                self.assertEqual(result.stdout, b"CDEF")
            finally:
                server.shutdown()
                server.server_close()

    def test_read_starting_on_span_boundary(self):
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            self._write_sources(root, {"a.bin": b"ABCDEFGH", "b.bin": b"ijklmnop"})
            server, _ = self._serve_dir(root)
            try:
                manifest = self._manifest(server.server_address[1], logical_size=8, spans=[
                    {"name": "a.bin", "logical_byte_start": 0, "byte_start": 2, "byte_length": 4},
                    {"name": "b.bin", "logical_byte_start": 4, "byte_start": 1, "byte_length": 4},
                ])
                manifest_path = root / "start_boundary.json"
                manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
                result = self._run_harness(manifest_path, (4, 4))
                self.assertEqual(result.returncode, 0, result.stderr.decode())
                self.assertEqual(result.stdout, b"jklm")
            finally:
                server.shutdown()
                server.server_close()

    def test_reads_across_two_spans(self):
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            self._write_sources(root, {"a.bin": b"ABCDEFGH", "b.bin": b"ijklmnop"})
            server, _ = self._serve_dir(root)
            try:
                manifest = self._manifest(server.server_address[1], logical_size=8, spans=[
                    {"name": "a.bin", "logical_byte_start": 0, "byte_start": 2, "byte_length": 4},
                    {"name": "b.bin", "logical_byte_start": 4, "byte_start": 1, "byte_length": 4},
                ])
                manifest_path = root / "two_spans.json"
                manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
                result = self._run_harness(manifest_path, (0, 8))
                self.assertEqual(result.returncode, 0, result.stderr.decode())
                self.assertEqual(result.stdout, b"CDEFjklm")
            finally:
                server.shutdown()
                server.server_close()

    def test_reads_across_three_spans(self):
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            self._write_sources(root, {"a.bin": b"ABCDEFGH", "b.bin": b"ijklmnop", "c.bin": b"QRSTUVWX"})
            server, _ = self._serve_dir(root)
            try:
                manifest = self._manifest(server.server_address[1], logical_size=9, spans=[
                    {"name": "a.bin", "logical_byte_start": 0, "byte_start": 1, "byte_length": 3},
                    {"name": "b.bin", "logical_byte_start": 3, "byte_start": 2, "byte_length": 3},
                    {"name": "c.bin", "logical_byte_start": 6, "byte_start": 0, "byte_length": 3},
                ])
                manifest_path = root / "three_spans.json"
                manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
                result = self._run_harness(manifest_path, (0, 9))
                self.assertEqual(result.returncode, 0, result.stderr.decode())
                self.assertEqual(result.stdout, b"BCDklmQRS")
            finally:
                server.shutdown()
                server.server_close()

    def test_repeated_reads_are_idempotent(self):
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            self._write_sources(root, {"a.bin": b"ABCDEFGH"})
            server, _ = self._serve_dir(root)
            try:
                manifest = self._manifest(server.server_address[1], logical_size=4, spans=[
                    {"name": "a.bin", "logical_byte_start": 0, "byte_start": 2, "byte_length": 4},
                ])
                manifest_path = root / "repeat.json"
                manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
                result = self._run_harness(manifest_path, (0, 4), (0, 4))
                self.assertEqual(result.returncode, 0, result.stderr.decode())
                self.assertEqual(result.stdout, b"CDEFCDEF")
            finally:
                server.shutdown()
                server.server_close()

    def test_rejects_empty_read(self):
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            self._write_sources(root, {"a.bin": b"ABCDEFGH"})
            server, _ = self._serve_dir(root)
            try:
                manifest = self._manifest(server.server_address[1], logical_size=4, spans=[
                    {"name": "a.bin", "logical_byte_start": 0, "byte_start": 2, "byte_length": 4},
                ])
                manifest_path = root / "empty.json"
                manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
                result = self._run_harness(manifest_path, (0, 0))
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("non-zero length", result.stderr.decode())
            finally:
                server.shutdown()
                server.server_close()

    def test_rejects_gap_in_logical_coverage_at_open(self):
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            self._write_sources(root, {"a.bin": b"ABCDEFGH", "b.bin": b"ijklmnop"})
            server, _ = self._serve_dir(root)
            try:
                manifest = self._manifest(server.server_address[1], logical_size=8, spans=[
                    {"name": "a.bin", "logical_byte_start": 0, "byte_start": 0, "byte_length": 3},
                    {"name": "b.bin", "logical_byte_start": 5, "byte_start": 0, "byte_length": 3},
                ])
                manifest_path = root / "gap.json"
                manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
                result = self._run_harness(manifest_path, (0, 8))
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("gap before logical byte 3", result.stderr.decode())
            finally:
                server.shutdown()
                server.server_close()

    def test_rejects_unsorted_spans_at_open(self):
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            self._write_sources(root, {"a.bin": b"ABCDEFGH", "b.bin": b"ijklmnop"})
            server, _ = self._serve_dir(root)
            try:
                manifest = self._manifest(server.server_address[1], logical_size=8, spans=[
                    {"name": "b.bin", "logical_byte_start": 4, "byte_start": 1, "byte_length": 4},
                    {"name": "a.bin", "logical_byte_start": 0, "byte_start": 2, "byte_length": 4},
                ])
                manifest_path = root / "unsorted.json"
                manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
                result = self._run_harness(manifest_path, (0, 8))
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("spans must be sorted", result.stderr.decode())
            finally:
                server.shutdown()
                server.server_close()

    def test_rejects_invalid_source_reference_at_open(self):
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            self._write_sources(root, {"a.bin": b"ABCDEFGH"})
            server, _ = self._serve_dir(root)
            try:
                manifest = {
                    "logical_size": 4,
                    "row_bytes": 1,
                    "solve_start": 0,
                    "solve_count": 4,
                    "sources": [
                        {"id": 0, "url": f"http://127.0.0.1:{server.server_address[1]}/a.bin", "key": "a.bin"},
                    ],
                    "spans": [
                        {"source_id": 1, "logical_byte_start": 0, "byte_start": 2, "byte_length": 4},
                    ],
                }
                manifest_path = root / "bad_source.json"
                manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
                result = self._run_harness(manifest_path, (0, 4))
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("invalid source_id", result.stderr.decode())
            finally:
                server.shutdown()
                server.server_close()

    def test_rejects_short_ranged_download(self):
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            self._write_sources(root, {"a.bin": b"ABCDEFGH"})
            server, _ = self._serve_dir(root, truncate_bytes_by_path={"a.bin": 2})
            try:
                manifest = self._manifest(server.server_address[1], logical_size=4, spans=[
                    {"name": "a.bin", "logical_byte_start": 0, "byte_start": 2, "byte_length": 4},
                ])
                manifest_path = root / "short_download.json"
                manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
                result = self._run_harness(manifest_path, (0, 4))
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("short range GET", result.stderr.decode())
            finally:
                server.shutdown()
                server.server_close()

    def test_retries_retryable_failure(self):
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            self._write_sources(root, {"a.bin": b"ABCDEFGH"})
            server, _ = self._serve_dir(root, failures_before_success={"a.bin": 1})
            try:
                manifest = self._manifest(server.server_address[1], logical_size=4, spans=[
                    {"name": "a.bin", "logical_byte_start": 0, "byte_start": 2, "byte_length": 4},
                ])
                manifest_path = root / "retry.json"
                manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
                result = self._run_harness(manifest_path, (0, 4))
                self.assertEqual(result.returncode, 0, result.stderr.decode())
                self.assertEqual(result.stdout, b"CDEF")
                self.assertEqual(server.request_counts.get("a.bin"), 2)
            finally:
                server.shutdown()
                server.server_close()
