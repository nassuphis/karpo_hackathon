import http.server
import json
import os
import pathlib
import socketserver
import struct
import subprocess
import tempfile
import threading
import unittest


ROOT = pathlib.Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"
SECTIONED_SRC = LAMBDA_DIR / "solve_proximity_hist_sectioned.c"
STATS_SRC = LAMBDA_DIR / "solve_proximity_stats.c"
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
    if not SECTIONED_BIN.exists() or SECTIONED_BIN.stat().st_mtime < SECTIONED_SRC.stat().st_mtime:
        _compile_binary(SECTIONED_SRC, SECTIONED_BIN, ["-lcurl", "-lm"])
    if not STATS_BIN.exists() or STATS_BIN.stat().st_mtime < STATS_SRC.stat().st_mtime:
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


class TestSolveProximityHistSectioned(unittest.TestCase):
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
            with socketserver.TCPServer(("127.0.0.1", 0), _RangeHandler) as httpd:
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


if __name__ == "__main__":
    unittest.main()
