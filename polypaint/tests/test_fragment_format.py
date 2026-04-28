import pathlib
import shutil
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"


def _encode_pairs(pairs):
    payload = bytearray()
    for pixel_idx, score in pairs:
        payload.extend(int(pixel_idx).to_bytes(4, "little", signed=False))
        payload.append(int(score) & 0xFF)
    return bytes(payload)


class TestFragmentFormat(unittest.TestCase):
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

    def _run(self, pix, fragment_payloads):
        with tempfile.TemporaryDirectory() as td:
            root = pathlib.Path(td)
            out_path = root / "out.raw"
            frag_paths = []
            for idx, payload in enumerate(fragment_payloads):
                path = root / f"frag_{idx}.bin"
                path.write_bytes(payload)
                frag_paths.append(path)
            cmd = [
                str(self._binary),
                f"--pix={pix}",
                f"--output={out_path}",
                *[str(path) for path in frag_paths],
            ]
            result = subprocess.run(cmd, capture_output=True, text=True)
            output = out_path.read_bytes() if out_path.exists() else b""
            return result, output

    def test_valid_pair_round_trip(self):
        result, output = self._run(2, [_encode_pairs([(0, 1), (3, 9)])])
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(output, bytes([1, 0, 0, 9]))

    def test_invalid_length_is_rejected(self):
        result, _ = self._run(2, [b"\x00"])
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("not divisible by record size 5", result.stderr)

    def test_zero_score_is_rejected(self):
        result, _ = self._run(2, [_encode_pairs([(1, 0)])])
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("invalid zero score", result.stderr)

    def test_out_of_bounds_pixel_idx_is_rejected(self):
        result, _ = self._run(2, [_encode_pairs([(4, 7)])])
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("out of bounds", result.stderr)

    def test_empty_fragment_is_accepted(self):
        result, output = self._run(2, [b""])
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(output, bytes([0, 0, 0, 0]))
