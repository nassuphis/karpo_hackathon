"""Parallel lattice materialization: the row fan-out must produce output
BYTE-IDENTICAL to the sequential order (each logical row lands at a fixed
offset), with identical stats, under real thread concurrency. The fake S3
client here is a plain thread-safe class — unittest.mock is not safely
callable from worker threads, which is also why tiny jobs stay sequential
below the threshold (existing handler tests keep their MagicMocks)."""
import os
import struct
import sys
import tempfile
import threading
import time
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))
from logical_lores import materialize_logical_lores  # noqa: E402

FULL_N = 40
VIEW_N = 12          # 12 rows > the 8-task threshold -> threads engage
DEGREE = 3


class _Body:
    def __init__(self, data):
        self._data = data

    def iter_chunks(self, chunk_size=1024 * 1024):
        for i in range(0, len(self._data), chunk_size):
            yield self._data[i:i + chunk_size]

    def read(self):
        return self._data


class ThreadSafeFakeS3:
    """Range-GET server over one synthetic chunk artifact; counts calls and
    records max concurrency observed (proof the pool actually fanned out)."""

    def __init__(self, blob):
        self._blob = blob
        self._lock = threading.Lock()
        self.calls = 0
        self.in_flight = 0
        self.max_in_flight = 0

    def get_object(self, Bucket=None, Key=None, Range=None, **kw):
        with self._lock:
            self.calls += 1
            self.in_flight += 1
            self.max_in_flight = max(self.max_in_flight, self.in_flight)
        try:
            # hold the request briefly: an instant in-memory response never
            # overlaps another, and the whole point of this probe is to SEE
            # the fan-out (GIL releases during sleep)
            time.sleep(0.02)
            body = self._blob
            if Range:
                lo, hi = Range[len("bytes="):].split("-", 1)
                body = body[int(lo):int(hi) + 1]
            return {"Body": _Body(body)}
        finally:
            with self._lock:
                self.in_flight -= 1


def _fixture():
    roots = bytearray()
    for solve in range(FULL_N * FULL_N):
        for r in range(DEGREE):
            roots.extend(struct.pack("<ff", float(solve), float(r)))
    calc = {
        "N": FULL_N,
        "times": 1,
        "degree": DEGREE,
        "n_coeffs": DEGREE + 1,
        "lores": {"N": FULL_N, "n_steps": FULL_N * FULL_N},
        "chunks": [{
            "idx": 0,
            "bin_key": "renders/j/chunk_0.bin",
            "step_count": FULL_N * FULL_N,
            "bin_size": len(roots),
        }],
    }
    return calc, bytes(roots)


class TestParallelMaterialization(unittest.TestCase):
    def _run(self, workers):
        calc, roots = _fixture()
        client = ThreadSafeFakeS3(roots)
        out = tempfile.NamedTemporaryFile(delete=False, suffix=".bin")
        out.close()
        try:
            stats = materialize_logical_lores(
                s3_client=client,
                bucket="bkt",
                calc=calc,
                job_id="j",
                degree=DEGREE,
                n_coeffs=DEGREE + 1,
                view_n=VIEW_N,
                out_paths={"slv": out.name},
                include_coeff=False,
                include_param=False,
                fetch_workers=workers,
            )
            with open(out.name, "rb") as fh:
                data = fh.read()
        finally:
            os.remove(out.name)
        return data, stats, client

    def test_parallel_is_byte_identical_to_sequential(self):
        seq_data, seq_stats, seq_client = self._run(workers=1)
        par_data, par_stats, par_client = self._run(workers=16)
        self.assertEqual(len(seq_data), VIEW_N * VIEW_N * DEGREE * 2 * 4)
        self.assertEqual(par_data, seq_data)                     # BYTE-identical
        self.assertEqual(par_stats["bytes_read"], seq_stats["bytes_read"])
        self.assertEqual(par_stats["range_gets"], seq_stats["range_gets"])
        self.assertEqual(par_stats["n_solves"], VIEW_N * VIEW_N)
        self.assertEqual(par_client.calls, seq_client.calls)     # same fetch plan
        self.assertGreater(par_client.max_in_flight, 1)          # threads REALLY ran
        self.assertEqual(seq_client.max_in_flight, 1)
        self.assertEqual(par_stats["fetch_workers"], 16)

    def test_lattice_semantics_survive_the_fan_out(self):
        # spot-check the compact content: logical row 0 col 0 must be the
        # physical (0,0) solve, whose payload encodes its own solve index
        par_data, _, _ = self._run(workers=16)
        re0, im0 = struct.unpack_from("<ff", par_data, 0)
        self.assertEqual((re0, im0), (0.0, 0.0))
        # logical (0,1) -> physical col floor(1*40/12)=3 -> solve 3
        re1, _ = struct.unpack_from("<ff", par_data, DEGREE * 2 * 4)
        self.assertEqual(re1, 3.0)


if __name__ == "__main__":
    unittest.main()
