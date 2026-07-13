import hashlib
import os
import struct
import sys
import threading
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

from roots_stream_upload import (  # noqa: E402
    MIN_PART_BYTES,
    PROGRESS_HEADER,
    PROGRESS_RECORD,
    RootsStreamUploader,
    read_progress,
)


def _write_sidecar(path, total, records):
    blob = bytearray(b"PPR1")
    blob += struct.pack("<IQ", len(records), total)
    for rs, re, fe in records:
        blob += struct.pack("<QQQ", rs, re, fe)
    with open(path, "wb") as fh:
        fh.write(bytes(blob))


class _FakeS3:
    """Records multipart calls; optionally fails selected operations."""

    def __init__(self, *, fail_create=False, fail_part_numbers=()):
        self.fail_create = fail_create
        self.fail_part_numbers = set(fail_part_numbers)
        self.parts = {}
        self.completed = None
        self.aborted = False

    def create_multipart_upload(self, *, Bucket, Key):
        if self.fail_create:
            raise RuntimeError("create denied")
        self.bucket, self.key = Bucket, Key
        return {"UploadId": "upload-1"}

    def upload_part(self, *, Bucket, Key, UploadId, PartNumber, Body):
        assert UploadId == "upload-1"
        if PartNumber in self.fail_part_numbers:
            raise RuntimeError(f"part {PartNumber} denied")
        self.parts[PartNumber] = bytes(Body)
        # real S3 semantics: a standard part's ETag is its MD5 — the hash
        # verification path depends on this
        return {"ETag": '"%s"' % hashlib.md5(bytes(Body)).hexdigest()}

    def complete_multipart_upload(self, *, Bucket, Key, UploadId, MultipartUpload):
        assert UploadId == "upload-1"
        numbers = [p["PartNumber"] for p in MultipartUpload["Parts"]]
        assert numbers == sorted(numbers)
        assert set(numbers) == set(self.parts)
        self.completed = b"".join(self.parts[n] for n in sorted(self.parts))
        return {}

    def abort_multipart_upload(self, *, Bucket, Key, UploadId):
        self.aborted = True
        return {}


class TestReadProgress(unittest.TestCase):

    def setUp(self):
        self.path = "/tmp/pp_test_progress.bin"

    def tearDown(self):
        try:
            os.remove(self.path)
        except OSError:
            pass

    def test_parses_valid_sidecar(self):
        _write_sidecar(self.path, 100, [(0, 60, 40), (60, 100, 60)])
        self.assertEqual(read_progress(self.path, 100),
                         [(0, 60, 40), (60, 100, 60)])

    def test_rejects_missing_wrong_magic_and_total_mismatch(self):
        self.assertIsNone(read_progress(self.path, 100))
        with open(self.path, "wb") as fh:
            fh.write(b"NOPE" + b"\x00" * 40)
        self.assertIsNone(read_progress(self.path, 100))
        _write_sidecar(self.path, 999, [(0, 999, 0)])
        self.assertIsNone(read_progress(self.path, 100))

    def test_rejects_torn_out_of_range_watermarks(self):
        """Review F2: a torn 8-byte read can mix two valid watermarks into a
        value LARGER than either — clamping it to region_end would declare
        undurable bytes durable. Out-of-range rejects the whole read."""
        _write_sidecar(self.path, 100, [(0, 60, 2 ** 63), (60, 100, 60)])
        self.assertIsNone(read_progress(self.path, 100))
        # below region_start is equally invalid
        _write_sidecar(self.path, 100, [(0, 60, 40), (60, 100, 3)])
        self.assertIsNone(read_progress(self.path, 100))

    def test_rejects_regions_that_do_not_partition_the_file(self):
        """Review F2: stale/corrupt sidecars with gaps, overlaps, or offset
        starts must reject — overlapping regions could otherwise satisfy
        coverage arithmetic while hiding a hole."""
        _write_sidecar(self.path, 100, [(0, 50, 50), (60, 100, 100)])   # gap
        self.assertIsNone(read_progress(self.path, 100))
        _write_sidecar(self.path, 100, [(0, 60, 60), (50, 100, 100)])  # overlap
        self.assertIsNone(read_progress(self.path, 100))
        _write_sidecar(self.path, 100, [(10, 100, 100)])               # offset start
        self.assertIsNone(read_progress(self.path, 100))
        _write_sidecar(self.path, 100, [(0, 90, 90)])                  # short
        self.assertIsNone(read_progress(self.path, 100))

    def test_layout_constants_match_writer(self):
        _write_sidecar(self.path, 100, [(0, 100, 50)])
        blob = open(self.path, "rb").read()
        self.assertEqual(len(blob), PROGRESS_HEADER + PROGRESS_RECORD)


class TestRootsStreamUploader(unittest.TestCase):

    def setUp(self):
        self.data_path = "/tmp/pp_test_roots.bin"
        self.progress_path = "/tmp/pp_test_roots.progress"
        self.total = 2 * MIN_PART_BYTES + MIN_PART_BYTES // 2   # 2.5 parts
        self.payload = os.urandom(self.total)

    def tearDown(self):
        for p in (self.data_path, self.progress_path):
            try:
                os.remove(p)
            except OSError:
                pass

    def _uploader(self, fake, **kwargs):
        kwargs.setdefault("part_bytes", MIN_PART_BYTES)
        # poll manually via _scan_once for determinism
        kwargs.setdefault("poll_seconds", 3600)
        return RootsStreamUploader(fake, "bkt", "k/roots.bin", self.data_path,
                                   self.progress_path, self.total, **kwargs)

    def test_parts_stream_during_solve_and_bytes_reassemble_exactly(self):
        fake = _FakeS3()
        up = self._uploader(fake)
        up.start()
        try:
            # simulate the C: two thread regions filling incrementally
            half = self.total // 2
            with open(self.data_path, "wb") as fh:
                fh.truncate(self.total)
            # region 0 fully durable, region 1 partially — part 1 ready only
            with open(self.data_path, "r+b") as fh:
                fh.write(self.payload[:half])
                fh.seek(half)
                fh.write(self.payload[half:half + 1024])
            _write_sidecar(self.progress_path, self.total,
                           [(0, half, half), (half, self.total, half + 1024)])
            up._scan_once()
            self.assertEqual(up.parts_during_solve, 1)
            self.assertEqual(set(fake.parts), {1})
            # solve finishes: file complete
            with open(self.data_path, "r+b") as fh:
                fh.seek(0)
                fh.write(self.payload)
            _write_sidecar(self.progress_path, self.total,
                           [(0, half, half), (half, self.total, self.total)])
            up._scan_once()
        finally:
            ok = up.finish()
        self.assertTrue(ok)
        self.assertEqual(fake.completed, self.payload)
        self.assertGreaterEqual(up.span_us, 0)
        self.assertFalse(fake.aborted)
        # every streamed part was hash-verified against the final file and
        # none needed repair (they were uploaded from the same bytes)
        self.assertEqual(up.parts_reverified, up.parts_during_solve)
        self.assertEqual(up.parts_repaired, 0)

    def test_stale_streamed_part_is_detected_and_repaired(self):
        """Review F1/F2 end-to-end regression: a part streamed from WRONG
        bytes (stale /tmp file, torn in-range watermark, or a read racing
        the writer) must be caught by the pre-completion hash check and
        re-uploaded from the final file — no size check can see this."""
        fake = _FakeS3()
        up = self._uploader(fake)
        up.start()
        # a stale file from a previous invocation claims part 1 is durable
        stale = bytes(reversed(self.payload))
        with open(self.data_path, "wb") as fh:
            fh.write(stale)
        _write_sidecar(self.progress_path, self.total,
                       [(0, self.total, MIN_PART_BYTES)])
        up._scan_once()
        self.assertEqual(up.parts_during_solve, 1)   # stale bytes went up
        # ...then the real solve overwrites the file with the true output
        with open(self.data_path, "wb") as fh:
            fh.write(self.payload)
        _write_sidecar(self.progress_path, self.total,
                       [(0, self.total, self.total)])
        self.assertTrue(up.finish())
        self.assertEqual(up.parts_repaired, 1)
        self.assertEqual(fake.completed, self.payload)   # NOT the stale bytes

    def test_create_failure_carries_a_reason(self):
        fake = _FakeS3(fail_create=True)
        with open(self.data_path, "wb") as fh:
            fh.write(self.payload)
        up = self._uploader(fake)
        up.start()
        self.assertFalse(up.finish())
        self.assertIn("create denied", up.fail_reason)

    def test_no_sidecar_still_completes_after_solve(self):
        # companion-matrix path: no progress file ever appears
        fake = _FakeS3()
        with open(self.data_path, "wb") as fh:
            fh.write(self.payload)
        up = self._uploader(fake)
        up.start()
        up._scan_once()   # no sidecar -> nothing uploaded yet
        self.assertEqual(up.parts_during_solve, 0)
        self.assertTrue(up.finish())
        self.assertEqual(fake.completed, self.payload)

    def test_part_boundary_straddling_two_regions_waits_for_both(self):
        fake = _FakeS3()
        with open(self.data_path, "wb") as fh:
            fh.write(self.payload)
        up = self._uploader(fake)
        up.start()
        # regions split INSIDE part 2: [0, 1.5 parts) and [1.5 parts, total)
        split = MIN_PART_BYTES + MIN_PART_BYTES // 2
        _write_sidecar(self.progress_path, self.total,
                       [(0, split, split), (split, self.total, split)])
        up._scan_once()
        self.assertEqual(set(fake.parts), {1})   # part 2 straddles: not ready
        _write_sidecar(self.progress_path, self.total,
                       [(0, split, split), (split, self.total, self.total)])
        up._scan_once()
        self.assertEqual(set(fake.parts), {1, 2, 3})
        self.assertTrue(up.finish())
        self.assertEqual(fake.completed, self.payload)

    def test_create_failure_reports_not_ok_without_upload(self):
        fake = _FakeS3(fail_create=True)
        with open(self.data_path, "wb") as fh:
            fh.write(self.payload)
        up = self._uploader(fake)
        up.start()
        self.assertFalse(up.finish())
        self.assertEqual(fake.parts, {})

    def test_part_failure_aborts_and_reports_not_ok(self):
        fake = _FakeS3(fail_part_numbers={2})
        with open(self.data_path, "wb") as fh:
            fh.write(self.payload)
        up = self._uploader(fake)
        up.start()
        self.assertFalse(up.finish())
        self.assertTrue(fake.aborted)

    def test_tail_parts_upload_concurrently(self):
        """The tail is the only remaining serial critical-path cost
        (production A/B: 3.96s of a 18.9s chunk) — finish() must ship the
        remaining parts in parallel. The barrier only releases when ALL
        three tail parts are in flight at once; a serial tail deadlocks it
        (times out -> finish() False) and fails the test."""
        class _BarrierS3(_FakeS3):
            def __init__(self, parties):
                super().__init__()
                self.barrier = threading.Barrier(parties)

            def upload_part(self, *, Bucket, Key, UploadId, PartNumber, Body):
                self.barrier.wait(timeout=10)
                return super().upload_part(
                    Bucket=Bucket, Key=Key, UploadId=UploadId,
                    PartNumber=PartNumber, Body=Body)

        fake = _BarrierS3(3)   # payload spans exactly 3 parts
        with open(self.data_path, "wb") as fh:
            fh.write(self.payload)
        up = self._uploader(fake)
        up.start()
        self.assertTrue(up.finish())
        self.assertEqual(fake.completed, self.payload)

    def test_threaded_poller_smoke(self):
        # one real threaded pass: writer thread fills the file + sidecar
        fake = _FakeS3()
        up = self._uploader(fake, poll_seconds=0.01)
        with open(self.data_path, "wb") as fh:
            fh.truncate(self.total)

        def writer():
            step = self.total // 4
            for i in range(4):
                a, b = i * step, (i + 1) * step if i < 3 else self.total
                with open(self.data_path, "r+b") as fh:
                    fh.seek(a)
                    fh.write(self.payload[a:b])
                _write_sidecar(self.progress_path, self.total,
                               [(0, self.total, b)])

        up.start()
        t = threading.Thread(target=writer)
        t.start()
        t.join()
        self.assertTrue(up.finish())
        self.assertEqual(fake.completed, self.payload)


if __name__ == "__main__":
    unittest.main()
