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


def _encode_pairs(pairs):
    payload = bytearray()
    for pixel_idx, score in pairs:
        payload.extend(int(pixel_idx).to_bytes(4, "little", signed=False))
        payload.append(int(score) & 0xFF)
    return bytes(payload)


def _reference_assemble(width, height, fragment_payloads):
    npix = width * height
    buf = bytearray(npix)
    for payload in fragment_payloads:
        if len(payload) % 5 != 0:
            raise AssertionError("bad test fixture")
        for off in range(0, len(payload), 5):
            pixel_idx = int.from_bytes(payload[off:off + 4], "little")
            score = payload[off + 4]
            if pixel_idx >= npix or score == 0:
                raise AssertionError("bad test fixture")
            buf[pixel_idx] = score
    return bytes(buf)


class TestAssembleGreyscale(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls._tmpdir_obj = tempfile.TemporaryDirectory()
        cls._workdir = pathlib.Path(cls._tmpdir_obj.name)
        cc = shutil.which("cc") or shutil.which("clang") or shutil.which("gcc")
        if not cc:
            raise unittest.SkipTest("no C compiler available")
        cls._binary = cls._workdir / "assemble_greyscale_test"
        result = subprocess.run(
            [
                cc,
                "-O2",
                "-pthread",
                str(LAMBDA_DIR / "assemble_greyscale.c"),
                "-lcurl",
                "-o",
                str(cls._binary),
            ],
            capture_output=True,
            text=True,
        )
        if result.returncode != 0:
            raise AssertionError(f"failed to compile assemble_greyscale: {result.stderr}")

    @classmethod
    def tearDownClass(cls):
        cls._tmpdir_obj.cleanup()

    def _run(self, width, height, fragment_payloads, workers=1, include_hist=False, missing_path=False):
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            out_path = root / "out.raw"
            hist_path = root / "hist.json"
            frag_paths = []
            for idx, payload in enumerate(fragment_payloads):
                path = root / f"frag_{idx}.bin"
                path.write_bytes(payload)
                frag_paths.append(path)
            if missing_path:
                frag_paths.append(root / "missing.frag")
            cmd = [
                str(self._binary),
                f"--width={width}",
                f"--height={height}",
                f"--output={out_path}",
                f"--workers={workers}",
            ]
            if include_hist:
                cmd.append(f"--hist-output={hist_path}")
            cmd.extend(str(path) for path in frag_paths)
            result = subprocess.run(cmd, capture_output=True, text=True)
            output = out_path.read_bytes() if out_path.exists() else b""
            hist = json.loads(hist_path.read_text()) if hist_path.exists() else None
            return result, output, hist

    def _run_with_url_manifest(self, width, height, fragment_payloads, workers=1):
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            for idx, payload in enumerate(fragment_payloads):
                (root / f"frag_{idx}.bin").write_bytes(payload)

            class QuietHandler(http.server.SimpleHTTPRequestHandler):
                def __init__(self, *args, **kwargs):
                    super().__init__(*args, directory=str(root), **kwargs)

                def log_message(self, format, *args):
                    pass

            with socketserver.TCPServer(("127.0.0.1", 0), QuietHandler) as server:
                thread = threading.Thread(target=server.serve_forever, daemon=True)
                thread.start()
                try:
                    manifest = root / "urls.txt"
                    urls = [
                        f"http://127.0.0.1:{server.server_address[1]}/frag_{idx}.bin"
                        for idx in range(len(fragment_payloads))
                    ]
                    manifest.write_text("\n".join(urls) + "\n", encoding="utf-8")
                    out_path = root / "out.raw"
                    hist_path = root / "hist.json"
                    cmd = [
                        str(self._binary),
                        f"--width={width}",
                        f"--height={height}",
                        f"--output={out_path}",
                        f"--workers={workers}",
                        f"--hist-output={hist_path}",
                        f"--url-manifest={manifest}",
                    ]
                    result = subprocess.run(cmd, capture_output=True, text=True)
                    output = out_path.read_bytes() if out_path.exists() else b""
                    hist = json.loads(hist_path.read_text()) if hist_path.exists() else None
                    return result, output, hist
                finally:
                    server.shutdown()
                    thread.join(timeout=5)

    def test_single_thread_matches_reference(self):
        frags = [
            _encode_pairs([(0, 4), (3, 9)]),
            _encode_pairs([(1, 5), (2, 7)]),
        ]
        result, output, _ = self._run(2, 2, frags, workers=1)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(output, _reference_assemble(2, 2, frags))

    def test_multi_thread_matches_reference_for_non_overlapping_fragments(self):
        frags = [
            _encode_pairs([(0, 11), (5, 12)]),
            _encode_pairs([(1, 21), (4, 22)]),
            _encode_pairs([(2, 31), (3, 32)]),
        ]
        result, output, _ = self._run(3, 2, frags, workers=3)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(output, _reference_assemble(3, 2, frags))

    def test_histogram_side_output_matches_reference(self):
        frags = [_encode_pairs([(0, 1), (1, 1), (3, 9)])]
        result, output, hist = self._run(2, 2, frags, workers=2, include_hist=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(output, bytes([1, 1, 0, 9]))
        self.assertEqual(hist["width"], 2)
        self.assertEqual(hist["height"], 2)
        self.assertEqual(hist["background_pixels"], 1)
        self.assertEqual(hist["nonzero_pixels"], 3)
        self.assertEqual(hist["histogram"][0], 1)
        self.assertEqual(hist["histogram"][1], 2)
        self.assertEqual(hist["histogram"][9], 1)

    def test_repeated_aliased_writes_are_accepted(self):
        frags = [
            _encode_pairs([(0, 3), (1, 4)]),
            _encode_pairs([(0, 7)]),
        ]
        result, output, _ = self._run(2, 1, frags, workers=2)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn(output[0], (3, 7))
        self.assertEqual(output[1], 4)

    def test_missing_fragment_path_is_clear_error(self):
        frags = [_encode_pairs([(0, 5)])]
        result, _, _ = self._run(1, 1, frags, workers=1, missing_path=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("cannot open", result.stderr)

    def test_url_manifest_fetches_fragments_natively(self):
        frags = [
            _encode_pairs([(0, 9), (3, 7)]),
            _encode_pairs([(1, 5), (2, 4)]),
        ]
        result, output, hist = self._run_with_url_manifest(2, 2, frags, workers=2)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(output, _reference_assemble(2, 2, frags))
        self.assertEqual(hist["background_pixels"], 0)
        self.assertEqual(hist["nonzero_pixels"], 4)
