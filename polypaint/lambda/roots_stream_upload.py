"""Streaming S3 multipart upload of the fused-chunk roots bin while the
native solver is still writing it (CR34 baseline §3.1b).

sweep_mt pre-sizes its output file, each solver thread pwrites its completed
slice, and a small sidecar publishes per-thread durable watermarks (format in
sweep_mt.c next to PP_PROGRESS_HEADER). This uploader polls the sidecar and
uploads fixed-size parts whose byte ranges are fully durable — S3 multipart
accepts parts in any order — then, after the solver exits and the caller has
verified the file, uploads whatever remains and completes the upload.

Failure semantics: any streaming error abandons the multipart upload (abort,
best effort) and finish() returns False; the caller falls back to the plain
serial upload. Correctness never depends on streaming, only the overlap does.
"""
import hashlib
import math
import os
import struct
import threading
import time
from concurrent.futures import ThreadPoolExecutor

PROGRESS_MAGIC = b"PPR1"
PROGRESS_HEADER = 16
PROGRESS_RECORD = 24
DEFAULT_PART_BYTES = 64 * 1024 * 1024
# S3 minimum for non-final multipart parts; the part size must never be
# configured below this or completes fail on multi-part objects.
MIN_PART_BYTES = 5 * 1024 * 1024


def read_progress(path, expected_total):
    """Parse the sidecar into [(region_start, region_end, flushed_end)].

    Returns None for absent/short/invalid sidecars — callers treat that as
    "no progress yet" and simply poll again.

    FAIL CLOSED (CR34-streaming review F2): a torn 8-byte read can mix bytes
    of two valid watermarks into a value LARGER than either (old 0x00FF /
    new 0x0100 -> 0x01FF), so clamping an out-of-range watermark would
    declare undurable bytes durable. Any watermark outside its region, or a
    region set that is not an exact contiguous partition of [0, total),
    rejects the whole read. (A torn value that lands within range is not
    detectable here at all — that is what the hash verification of streamed
    parts before multipart completion is for.)
    """
    try:
        with open(path, "rb") as fh:
            blob = fh.read()
    except OSError:
        return None
    if len(blob) < PROGRESS_HEADER or blob[:4] != PROGRESS_MAGIC:
        return None
    n_threads, total = struct.unpack_from("<IQ", blob, 4)
    if total != expected_total or n_threads <= 0 or n_threads > 4096:
        return None
    if len(blob) < PROGRESS_HEADER + n_threads * PROGRESS_RECORD:
        return None
    regions = []
    for i in range(n_threads):
        rs, re, fe = struct.unpack_from(
            "<QQQ", blob, PROGRESS_HEADER + i * PROGRESS_RECORD)
        if not rs <= re <= total:
            return None
        if not rs <= fe <= re:
            return None
        regions.append((rs, re, fe))
    # the regions must partition [0, total) exactly — no gaps, no overlaps
    ordered = sorted(regions)
    cursor = 0
    for rs, re, _ in ordered:
        if rs != cursor:
            return None
        cursor = re
    if cursor != total:
        return None
    return regions


class RootsStreamUploader:
    def __init__(self, s3_client, bucket, key, data_path, progress_path,
                 total_bytes, *, part_bytes=DEFAULT_PART_BYTES,
                 poll_seconds=0.15):
        self._s3 = s3_client
        self._bucket = bucket
        self._key = key
        self._data_path = data_path
        self._progress_path = progress_path
        self._total = int(total_bytes)
        self._part_bytes = max(int(part_bytes), MIN_PART_BYTES)
        self._poll_seconds = poll_seconds
        self._n_parts = max(1, math.ceil(self._total / self._part_bytes))
        self._etags = {}
        self._parts_during_solve = 0
        self._parts_repaired = 0
        self._parts_reverified = 0
        self._upload_id = None
        self._failed = False
        self._fail_reason = None
        self._stop = threading.Event()
        self._thread = None
        self._span_t0 = None
        self._span_us = 0

    @property
    def parts_during_solve(self):
        return self._parts_during_solve

    @property
    def parts_reverified(self):
        return self._parts_reverified

    @property
    def parts_repaired(self):
        return self._parts_repaired

    @property
    def fail_reason(self):
        return self._fail_reason

    @property
    def span_us(self):
        return self._span_us

    def _mark_failed(self, exc):
        self._failed = True
        if self._fail_reason is None:
            self._fail_reason = f"{type(exc).__name__}: {exc}"[:200]

    def start(self):
        """Create the multipart upload and start polling. A create failure
        just marks streaming unavailable — the solve proceeds regardless."""
        try:
            resp = self._s3.create_multipart_upload(
                Bucket=self._bucket, Key=self._key)
            self._upload_id = resp["UploadId"]
        except Exception as exc:
            self._mark_failed(exc)
            return
        self._span_t0 = time.time()
        self._thread = threading.Thread(target=self._poll_loop, daemon=True)
        self._thread.start()

    def finish(self):
        """Call after the solver exited successfully AND the caller verified
        the output file. Uploads remaining parts, completes the multipart
        upload, returns True; any failure aborts and returns False."""
        self._stop.set()
        if self._thread is not None:
            self._thread.join()
        if self._failed or self._upload_id is None:
            self.abort()
            return False
        try:
            # The file is complete and final now — every part is durable.
            # The tail is the ONLY serial critical-path cost left (production
            # A/B: each thread's last part lands here, ~4 of 9 parts), so the
            # remaining parts ship CONCURRENTLY — independent byte ranges of
            # a final file, and boto3 clients are thread-safe.
            #
            # Parts uploaded DURING the solve are HASH-VERIFIED against the
            # final local file before completion (review F1/F2): a stale
            # sidecar, a torn in-range watermark, or a read that raced the
            # writer would have shipped wrong bytes that no size check can
            # see. A standard part's ETag is the MD5 of its data; mismatches
            # are re-uploaded from the final file (so even SSE-KMS buckets,
            # where ETag != MD5, degrade to re-upload-everything — slower,
            # never wrong).
            streamed = sorted(self._etags)
            remaining = [p for p in range(self._n_parts)
                         if p not in self._etags]
            work = ([("verify", p) for p in streamed]
                    + [("upload", p) for p in remaining])
            if work:
                with ThreadPoolExecutor(
                        max_workers=min(4, len(work))) as pool:
                    futures = {
                        (kind, p): pool.submit(
                            self._verify_or_repair_part if kind == "verify"
                            else self._upload_one_part, p)
                        for kind, p in work}
                    for (kind, p), fut in futures.items():
                        result = fut.result()   # re-raises failures
                        if kind == "upload":
                            self._etags[p] = result
                        else:
                            self._parts_reverified += 1
                            if result is not None:
                                self._etags[p] = result
                                self._parts_repaired += 1
            parts = [{"ETag": etag, "PartNumber": idx + 1}
                     for idx, etag in sorted(self._etags.items())]
            if len(parts) != self._n_parts:
                raise RuntimeError(
                    f"multipart part count {len(parts)} != {self._n_parts}")
            self._s3.complete_multipart_upload(
                Bucket=self._bucket, Key=self._key, UploadId=self._upload_id,
                MultipartUpload={"Parts": parts})
            self._span_us = int((time.time() - self._span_t0) * 1e6)
            return True
        except Exception as exc:
            self._mark_failed(exc)
            self.abort()
            return False

    def abort(self):
        """Best-effort teardown; safe to call more than once."""
        self._stop.set()
        if self._thread is not None and self._thread is not threading.current_thread():
            self._thread.join()
            self._thread = None
        if self._upload_id is not None:
            try:
                self._s3.abort_multipart_upload(
                    Bucket=self._bucket, Key=self._key,
                    UploadId=self._upload_id)
            except Exception:
                pass
            self._upload_id = None

    # ── internals ────────────────────────────────────────────────────────

    def _poll_loop(self):
        while not self._stop.is_set():
            try:
                self._scan_once()
            except Exception as exc:
                self._mark_failed(exc)
                return
            self._stop.wait(self._poll_seconds)

    def _scan_once(self):
        """One sidecar poll: upload every not-yet-uploaded part whose byte
        range is fully durable. Runs only on the poller thread."""
        regions = read_progress(self._progress_path, self._total)
        if regions is None:
            return
        for p in sorted(self._durable_parts(regions)):
            if p in self._etags:
                continue
            self._etags[p] = self._upload_one_part(p)
            self._parts_during_solve += 1

    def _durable_parts(self, regions):
        """Part p covers [p*P, min((p+1)*P, total)). It is durable when every
        thread region intersecting it has flushed past the intersection end."""
        ready = set()
        for p in range(self._n_parts):
            a = p * self._part_bytes
            b = min(a + self._part_bytes, self._total)
            ok = True
            covered = 0
            for rs, re, fe in regions:
                lo, hi = max(a, rs), min(b, re)
                if lo >= hi:
                    continue
                covered += hi - lo
                if fe < hi:
                    ok = False
                    break
            if ok and covered == b - a:
                ready.add(p)
        return ready

    def _read_part(self, p):
        """Read part p's byte range via a private file handle so concurrent
        tail workers never share a seek position."""
        a = p * self._part_bytes
        b = min(a + self._part_bytes, self._total)
        with open(self._data_path, "rb") as fh:
            fh.seek(a)
            data = fh.read(b - a)
        if len(data) != b - a:
            raise RuntimeError(
                f"short read for part {p}: {len(data)} != {b - a}")
        return data

    def _upload_one_part(self, p):
        """Upload part p (0-based) and return its ETag."""
        data = self._read_part(p)
        resp = self._s3.upload_part(
            Bucket=self._bucket, Key=self._key, UploadId=self._upload_id,
            PartNumber=p + 1, Body=data)
        return resp["ETag"]

    def _verify_or_repair_part(self, p):
        """Compare a streamed part's recorded ETag with the MD5 of the same
        range in the FINAL local file; on mismatch re-upload from the final
        bytes and return the new ETag (None when the part was already
        correct). S3 replaces a part number re-uploaded before completion."""
        data = self._read_part(p)
        local_md5 = hashlib.md5(data).hexdigest()
        if self._etags[p].strip('"') == local_md5:
            return None
        resp = self._s3.upload_part(
            Bucket=self._bucket, Key=self._key, UploadId=self._upload_id,
            PartNumber=p + 1, Body=data)
        return resp["ETag"]
