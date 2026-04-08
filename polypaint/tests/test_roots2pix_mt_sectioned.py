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
SRC = LAMBDA_DIR / "roots2pix_mt.c"
BIN = pathlib.Path("/tmp/roots2pix_mt_host_test")


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


def _ensure_binary():
    if not BIN.exists() or BIN.stat().st_mtime < SRC.stat().st_mtime:
        _compile_binary(SRC, BIN, ["-lcurl", "-lm"])


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


class TestRoots2PixMTSectioned(unittest.TestCase):
    def test_sectioned_matches_tmpfile_for_saved_palette_fixture(self):
        _ensure_binary()
        solves = [
            [(-2.0, -2.0), (-1.0, -2.0)],
            [(1.0, -1.0), (2.0, -1.0)],
            [(-2.0, 1.0), (-1.0, 1.0)],
            [(1.0, 2.0), (2.0, 2.0)],
        ]
        with tempfile.TemporaryDirectory(prefix="roots2pix_mt_sectioned_") as tmpdir:
            tmpdir = pathlib.Path(tmpdir)
            input_path = tmpdir / "input.bin"
            bins_path = tmpdir / "solve_bins.bin"
            tmp_prefix = tmpdir / "tmpfile" / "pix"
            tmp_pbx_prefix = tmpdir / "tmpfile" / "pixbin"
            sec_prefix = tmpdir / "sectioned" / "pix"
            sec_pbx_prefix = tmpdir / "sectioned" / "pixbin"
            tmp_prefix.parent.mkdir(parents=True, exist_ok=True)
            sec_prefix.parent.mkdir(parents=True, exist_ok=True)

            _write_bin(input_path, solves, 2)
            bins_path.write_bytes(bytes([0, 1, 2, 3]))

            common_args = [
                "--width=64",
                "--height=64",
                "--center_re=0",
                "--center_im=0",
                "--scale=8",
                "--degree=2",
                "--tile_size=64",
                "--n_tile_cols=1",
                "--n_tile_rows=1",
                "--threads=2",
                "--color=saved_palette",
                "--palette=inferno",
                f"--solve_bins_file={bins_path}",
            ]

            tmpfile_run = subprocess.run(
                [
                    str(BIN),
                    str(input_path),
                    str(tmp_prefix),
                    *common_args,
                    "--input_mode=tmpfile",
                    f"--pixel_bin_prefix={tmp_pbx_prefix}",
                ],
                capture_output=True,
                text=True,
                timeout=60,
            )
            self.assertEqual(tmpfile_run.returncode, 0, tmpfile_run.stderr)
            tmp_meta = json.loads(tmpfile_run.stdout)
            tmp_pix = (tmp_prefix.parent / "pix_t0000.pix").read_bytes()
            tmp_pbx = (tmp_prefix.parent / "pixbin_t0000.pbx").read_bytes()

            file_bytes = input_path.read_bytes()
            _RangeHandler.file_bytes = file_bytes
            with socketserver.TCPServer(("127.0.0.1", 0), _RangeHandler) as httpd:
                port = httpd.server_address[1]
                thread = threading.Thread(target=httpd.serve_forever, daemon=True)
                thread.start()
                try:
                    url = f"http://127.0.0.1:{port}/input.bin"
                    sectioned_run = subprocess.run(
                        [
                            str(BIN),
                            str(input_path),
                            str(sec_prefix),
                            *common_args,
                            "--input_mode=sectioned",
                            f"--url={url}",
                            f"--input_size={len(file_bytes)}",
                            f"--pixel_bin_prefix={sec_pbx_prefix}",
                        ],
                        capture_output=True,
                        text=True,
                        timeout=60,
                    )
                    self.assertEqual(sectioned_run.returncode, 0, sectioned_run.stderr)
                    sec_meta = json.loads(sectioned_run.stdout)
                    sec_pix = (sec_prefix.parent / "pix_t0000.pix").read_bytes()
                    sec_pbx = (sec_prefix.parent / "pixbin_t0000.pbx").read_bytes()
                finally:
                    httpd.shutdown()
                    thread.join(timeout=5)

        self.assertEqual(tmp_meta["roots_plotted"], sec_meta["roots_plotted"])
        self.assertEqual(tmp_meta["roots_clipped"], sec_meta["roots_clipped"])
        self.assertEqual(sec_meta["input_mode"], "sectioned")
        self.assertEqual(tmp_pix, sec_pix)
        self.assertEqual(tmp_pbx, sec_pbx)


if __name__ == "__main__":
    unittest.main()
