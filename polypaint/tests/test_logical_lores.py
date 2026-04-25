import os
import struct
import sys
import unittest


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class _Body:
    def __init__(self, data):
        self._data = bytes(data)

    def read(self):
        return self._data


class _S3:
    def __init__(self, objects):
        self.objects = dict(objects)

    def get_object(self, *, Bucket, Key, Range=None):
        data = self.objects[Key]
        if Range:
            raw = Range[len("bytes="):]
            lo, hi = raw.split("-", 1)
            data = data[int(lo):int(hi) + 1]
        return {"Body": _Body(data)}


class TestLogicalLores(unittest.TestCase):
    def test_materialize_slv_preserves_logical_serpentine_order(self):
        from logical_lores import materialize_logical_lores

        key = "renders/j/chunk_0.bin"
        roots = bytearray()
        for step in range(16):
            roots.extend(struct.pack("<ff", float(step), 0.0))
        out_path = "/tmp/test_logical_lores_roots.bin"
        try:
            result = materialize_logical_lores(
                s3_client=_S3({key: bytes(roots)}),
                bucket="bucket",
                calc={
                    "N": 4,
                    "times": 1,
                    "degree": 1,
                    "n_coeffs": 2,
                    "chunks": [{
                        "idx": 0,
                        "bin_key": key,
                        "step_count": 16,
                        "bin_size": len(roots),
                    }],
                },
                job_id="j",
                degree=1,
                n_coeffs=2,
                view_n=2,
                out_paths={"slv": out_path},
            )
            self.assertEqual(result["n_solves"], 4)
            with open(out_path, "rb") as fh:
                vals = struct.unpack("<" + "f" * (os.path.getsize(out_path) // 4), fh.read())
            self.assertEqual(list(vals[0::2]), [0.0, 2.0, 10.0, 8.0])
        finally:
            try:
                os.remove(out_path)
            except OSError:
                pass


if __name__ == "__main__":
    unittest.main()
