"""
Poly-Sheet handler: parameter-scan mosaic artifacts (poly-sheet.md).

The e2e test EXECUTES handle_run against the real local binary
(S3/status mocked — the fused-lores lesson: shallow pins don't catch
handler-path bugs): a 4-frame sheet over a $T-scanned coeff program,
asserting the mosaic geometry, bilevel content, per-frame variation,
manifest shape, and the cancel path. Units cover scan spacing, the
grammar-safe literal spelling, substitution validation, and the
budget guard.
"""
import json
import math
import os
import sys
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")
sys.path.insert(0, LAMBDA_DIR)

SHEET_SRC = """poly = fill(6, 0)
poly[0] = 1
poly[1] = 2*exp(6.283185307179586i*t1)
poly[3] = $T*exp(6.283185307179586i*t2)
poly[5] = 0-2
emit
"""

SHEET_SRC_2D = """poly = fill(6, 0)
poly[0] = 1
poly[1] = $S*exp(6.283185307179586i*t1)
poly[3] = $T*exp(6.283185307179586i*t2)
poly[5] = 0-2
emit
"""


class _LeaseIsoTestCase(unittest.TestCase):
    """Round-9 finding 6: assert AFTER EVERY test (not only at suite end)
    that no lease-fn stub leaked into handler_poly_sheet. The suite-end
    sentinel alone gave false confidence — a later fixture's cleanup could
    repair a leak before the sentinel ran, so an order-dependent leak (one
    fixture stubbing without restoring) passed. This per-test check fails
    the offending test itself, even run in isolation."""

    def setUp(self):
        super().setUp()
        # Register the check FIRST so (addCleanup is LIFO) it runs LAST —
        # after every fixture's restore cleanup registered during the test.
        # A tearDown() check would run BEFORE those restores and see the
        # stubs still active.
        self.addCleanup(self._assert_no_lease_leak)

    def _assert_no_lease_leak(self):
        import handler_poly_sheet as _mod
        import shared as _shared
        self.assertIs(_mod.claim_task, _shared.claim_task,
                      "claim_task stub leaked past this test")
        self.assertIs(_mod.renew_claim, _shared.renew_claim,
                      "renew_claim stub leaked past this test")
        self.assertIs(_mod.finalize_task, _shared.finalize_task,
                      "finalize_task stub leaked past this test")


class _S3Stub:
    """ETag-aware S3 fake with real conditional-write semantics — the
    round-3 review flagged that the old stub had no ETags and silently
    exercised the (unsafe) fallback. IfMatch / IfNoneMatch here behave
    like S3's conditional PUT."""

    def __init__(self):
        self.objects = {}      # key -> bytes
        self.etags = {}        # key -> etag string
        self._seq = 0

    def _next_etag(self):
        self._seq += 1
        return f'"etag{self._seq}"'

    def put_object(self, Bucket, Key, Body, ContentType=None, CacheControl=None,
                   IfMatch=None, IfNoneMatch=None):
        from botocore.exceptions import ClientError
        if IfNoneMatch is not None:
            if Key in self.objects:
                raise ClientError({"Error": {"Code": "PreconditionFailed"}}, "PutObject")
        if IfMatch is not None:
            if self.etags.get(Key) != IfMatch:
                raise ClientError({"Error": {"Code": "PreconditionFailed"}}, "PutObject")
        self.objects[Key] = Body if isinstance(Body, (bytes, bytearray)) else Body.encode()
        self.etags[Key] = self._next_etag()

    def head_object(self, Bucket, Key):
        if Key not in self.objects:
            from botocore.exceptions import ClientError
            raise ClientError({"Error": {"Code": "404"}}, "HeadObject")
        return {"ContentLength": len(self.objects[Key]),
                "ETag": self.etags.get(Key)}

    def get_object(self, Bucket, Key):
        import io
        if Key not in self.objects:
            from botocore.exceptions import ClientError
            raise ClientError({"Error": {"Code": "NoSuchKey"}}, "GetObject")
        return {"Body": io.BytesIO(bytes(self.objects[Key])),
                "ETag": self.etags.get(Key),
                "LastModified": __import__("datetime").datetime(2026, 7, 20)}

    def delete_objects(self, Bucket, Delete):
        for entry in Delete["Objects"]:
            self.objects.pop(entry["Key"], None)
            self.etags.pop(entry["Key"], None)
        return {}

    def list_objects_v2(self, Bucket, Prefix, ContinuationToken=None):
        keys = sorted(k for k in self.objects if k.startswith(Prefix))
        return {"Contents": [{"Key": k} for k in keys], "IsTruncated": False}


def _admit(mod, stub, params, gen, worker_frames):
    """Write the run.json record begin would have produced, so
    frames/stitch requests bind (admission enforcement)."""
    sheet_id = params["sheet_id"]
    run = {
        "sheet_id": sheet_id,
        "generation": gen,
        "job_id": params.get("job_id", "sheet_job"),
        "steps": sum(len(r) for r in worker_frames),
        "workers": [
            {"task_id": f"sheet_tiles_{sheet_id}_{gen}_w{i}", "frames": r}
            for i, r in enumerate(worker_frames)
        ],
        "stitch_task_id": f"sheet_stitch_{sheet_id}_{gen}",
        "degree_probe": 5,
        "params_hash": mod._params_hash(params),
        "status": "running",
    }
    stub.put_object(Bucket="b", Key=f"sheets/{sheet_id}/run.json",
                    Body=json.dumps(run).encode())
    return run


def _run_params(sheet_id, steps=4, solver="jt64", extra=None):
    p = {
        "action": "run",
        "job_id": "sheet_job",
        "task_id": f"sheet_{sheet_id}",
        "sheet_id": sheet_id,
        "function": "const",
        "cfpv": [1, 0, 0],
        "coeff_program_source_text": SHEET_SRC,
        "scan": {"token": "$T", "from": 0.5, "to": 2.0, "steps": steps,
                 "spacing": "linear"},
        "frame": {"n": 8, "tile_px": 32, "solver_mode": solver,
                  "viewport": {"mode": "quantile", "quantile": 0.0, "shim": 0.05},
                  "rotate": 0},
        "grid_cols": 2,
    }
    if extra:
        p.update(extra)
    return p


class TestPolySheetUnits(_LeaseIsoTestCase):
    def test_scan_values_spacings(self):
        import handler_poly_sheet as mod

        self.assertEqual(mod.scan_values(0, 1, 5, "linear"),
                         [0.0, 0.25, 0.5, 0.75, 1.0])
        angle = mod.scan_values(0, 1, 4, "angle")
        self.assertEqual(angle, [0.0, 0.25, 0.5, 0.75])   # endpoint excluded
        log = mod.scan_values(1, 100, 3, "log")
        self.assertAlmostEqual(log[1], 10.0)
        with self.assertRaises(RuntimeError):
            mod.scan_values(0, 1, 3, "log")               # zero endpoint
        # step spacing: arithmetic sequence, hi ignored, integers exact
        self.assertEqual(mod.scan_values(2, 0, 4, "step", step=1),
                         [2.0, 3.0, 4.0, 5.0])
        self.assertEqual(mod.scan_values(1, 0, 3, "step", step=-0.5),
                         [1.0, 0.5, 0.0])
        with self.assertRaises(RuntimeError):
            mod.scan_values(0, 0, 3, "step", step=0)      # zero step

    def test_value_literal_is_grammar_safe(self):
        import handler_poly_sheet as mod

        self.assertEqual(mod._value_literal(0.25), "0.25")
        self.assertEqual(mod._value_literal(-0.5), "(0-0.5)")
        # integral values spell as integers (count positions: fill/scan
        # lengths, degrees) — "5.0" would not compile there
        self.assertEqual(mod._value_literal(5.0), "5")
        self.assertEqual(mod._value_literal(-3.0), "(0-3)")

    def test_substitution_requires_a_hit(self):
        import handler_poly_sheet as mod

        out = mod.substitute_token(
            {"coeff_program_source_text": "poly = fill(3, $T)\nemit"}, "$T", 0.5)
        self.assertIn("fill(3, 0.5)", out["coeff_program_source_text"])
        with self.assertRaises(RuntimeError):
            mod.substitute_token(
                {"coeff_program_source_text": "poly = fill(3, 1)\nemit"}, "$T", 0.5)

    def test_canvas_pixel_cap(self):
        import handler_poly_sheet as mod

        orig = mod.report_status
        _real = (mod.claim_task, mod.renew_claim, mod.finalize_task)
        self.addCleanup(lambda: (setattr(mod, "claim_task", _real[0]),
                                 setattr(mod, "renew_claim", _real[1]),
                                 setattr(mod, "finalize_task", _real[2])))
        mod.report_status = lambda *a, **k: None
        mod.claim_task = lambda *a, **k: True
        mod.renew_claim = lambda *a, **k: True
        mod.finalize_task = lambda *a, **k: True
        try:
            with self.assertRaises(RuntimeError) as ctx:
                mod.handle_run(_run_params(
                    "toobig", steps=256, solver="ae64",
                    extra={"grid_cols": 16,
                           "frame": {"n": 8, "tile_px": 1024,
                                     "solver_mode": "ae64",
                                     "viewport": {"mode": "quantile"},
                                     "rotate": 0}}))
            self.assertIn("mosaic too large", str(ctx.exception))
        finally:
            mod.report_status = orig

    def test_budget_guard_rejects_oversized_sheets(self):
        import handler_poly_sheet as mod

        orig = mod.report_status
        _real = (mod.claim_task, mod.renew_claim, mod.finalize_task)
        self.addCleanup(lambda: (setattr(mod, "claim_task", _real[0]),
                                 setattr(mod, "renew_claim", _real[1]),
                                 setattr(mod, "finalize_task", _real[2])))
        mod.report_status = lambda *a, **k: None
        mod.claim_task = lambda *a, **k: True
        mod.renew_claim = lambda *a, **k: True
        mod.finalize_task = lambda *a, **k: True
        try:
            with self.assertRaises(RuntimeError) as ctx:
                mod.handle_run(_run_params(
                    "toolarge", steps=256, solver="companion_matrix",
                    extra={"frame": {"n": 192, "tile_px": 32,
                                     "solver_mode": "companion_matrix",
                                     "viewport": {"mode": "quantile"},
                                     "rotate": 0}}))
            self.assertIn("budget", str(ctx.exception))
        finally:
            mod.report_status = orig


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestPolySheetEndToEnd(_LeaseIsoTestCase):
    def _patched(self, mod, s3stub):
        mod.s3 = s3stub
        mod.SWEEP_COEFFGEN = SWEEP_TEST
        mod.report_status = lambda *a, **k: None
        # atomic claim always wins in these S3-focused tests; the lease
        # semantics have their own DDB-backed test. Restore the reals
        # after the test so stubs cannot leak (round-6 finding 6).
        import shared as _shared
        # round-8 finding 6 + round-9 finding 7: restore ALL THREE lease
        # primitives to the KNOWN-REAL shared implementations — capturing
        # the current value could re-save a stub a prior leak left behind.
        self.addCleanup(lambda: setattr(mod, "claim_task", _shared.claim_task))
        self.addCleanup(lambda: setattr(mod, "renew_claim", _shared.renew_claim))
        self.addCleanup(lambda: setattr(mod, "finalize_task", _shared.finalize_task))
        mod.claim_task = lambda *a, **k: True
        mod.renew_claim = lambda *a, **k: True
        mod.finalize_task = lambda *a, **k: True

    def test_run_renders_a_bilevel_mosaic(self):
        import handler_poly_sheet as mod

        stub = _S3Stub()
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub)
        try:
            resp = mod.handle_run(_run_params("e2e-sheet"))
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig
        manifest = json.loads(resp["body"])
        self.assertEqual(manifest["frames"], 4)
        self.assertEqual(manifest["grid"], {"cols": 2, "rows": 2})
        self.assertEqual(manifest["degree"], 5)
        self.assertEqual([f["value"] for f in manifest["frame_records"]],
                         [0.5, 1.0, 1.5, 2.0])
        self.assertIn("sheets/e2e-sheet/sheet.png", stub.objects)
        self.assertIn("sheets/e2e-sheet/sheet.json", stub.objects)

        png = stub.objects["sheets/e2e-sheet/sheet.png"]
        self.assertEqual(png[:8], b"\x89PNG\r\n\x1a\n")
        # dimensions from the IHDR chunk: 2x2 tiles of 32px
        import struct as _s
        w, h = _s.unpack(">II", png[16:24])
        self.assertEqual((w, h), (64, 64))

        # per-frame variation: the scan actually changed the polynomial
        # (frames have different bounds records under per-frame quantile)
        bounds = {tuple(f["bounds"]) for f in manifest["frame_records"]}
        self.assertGreater(len(bounds), 1)

    def test_cancel_marker_stops_the_loop(self):
        import handler_poly_sheet as mod

        gen = "g00000000cafe"
        stub = _S3Stub()
        stub.objects[mod._cancel_key("cancelled-sheet", gen)] = b"1"
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub)
        try:
            resp = mod.handle_run({**_run_params("cancelled-sheet"),
                                   "generation": gen})
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig
        body = json.loads(resp["body"])
        self.assertEqual(body["cancelled"], "cancelled-sheet")
        self.assertEqual(body["frames_done"], 0)
        self.assertNotIn("sheets/cancelled-sheet/sheet.png", stub.objects)

    def test_polarity_and_margin_geometry(self):
        import handler_poly_sheet as mod

        stub = _S3Stub()
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub)
        try:
            resp = mod.handle_run(_run_params("margin-sheet", extra={
                "frame": {"n": 8, "tile_px": 32, "solver_mode": "jt64",
                          "viewport": {"mode": "quantile", "quantile": 0.0,
                                       "shim": 0.05},
                          "rotate": 0, "polarity": "black_on_white",
                          "margin_px": 4},
            }))
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig
        manifest = json.loads(resp["body"])
        self.assertEqual(manifest["polarity"], "black_on_white")
        self.assertEqual(manifest["margin_px"], 4)
        png = stub.objects["sheets/margin-sheet/sheet.png"]
        import struct as _s
        w, h = _s.unpack(">II", png[16:24])
        # 2x2 tiles of 32px + 3 gutters of 4px each way
        self.assertEqual((w, h), (76, 76))

    def test_step_spacing_scans_integers(self):
        import handler_poly_sheet as mod

        stub = _S3Stub()
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub)
        try:
            resp = mod.handle_run(_run_params("int-sheet", extra={
                "scan": {"token": "$T", "from": 1, "steps": 4,
                         "spacing": "step", "step": 1},
            }))
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig
        manifest = json.loads(resp["body"])
        self.assertEqual([f["value"] for f in manifest["frame_records"]],
                         [1.0, 2.0, 3.0, 4.0])
        self.assertEqual(manifest["scan"]["spacing"], "step")
        self.assertEqual(manifest["scan"]["step"], 1.0)
        self.assertEqual(manifest["scan"]["values"], [1.0, 2.0, 3.0, 4.0])

    def test_fanout_workers_and_stitch_match_single_run(self):
        import handler_poly_sheet as mod

        gen = "gabcdef012345"
        # single-shot reference
        stub_single = _S3Stub()
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub_single)
        try:
            mod.handle_run(_run_params("single-sheet"))
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig
        ref_png = stub_single.objects["sheets/single-sheet/sheet.png"]

        # fan-out: two workers with split frame ranges, then stitch
        stub = _S3Stub()
        self._patched(mod, stub)
        try:
            p = _run_params("fan-sheet")
            p["generation"] = gen
            _admit(mod, stub, p, gen, [[0, 1], [2, 3]])
            mod.handle_frames({**p, "action": "frames",
                               "task_id": f"sheet_tiles_fan-sheet_{gen}_w0",
                               "frame_indices": [0, 1]})
            mod.handle_frames({**p, "action": "frames",
                               "task_id": f"sheet_tiles_fan-sheet_{gen}_w1",
                               "frame_indices": [2, 3]})
            self.assertIn(f"sheets/fan-sheet/tiles/{gen}/00000.bin", stub.objects)
            resp = mod.handle_stitch({**p, "action": "stitch",
                                      "task_id": f"sheet_stitch_fan-sheet_{gen}"})
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig
        manifest = json.loads(resp["body"])
        self.assertEqual(manifest["render_mode"], "fanout")
        self.assertEqual(manifest["generation"], gen)
        self.assertEqual([f["value"] for f in manifest["frame_records"]],
                         [0.5, 1.0, 1.5, 2.0])
        self.assertEqual(manifest["degree"], 5)
        # publication is a pointer: run.json names the generation's
        # immutable png, which is byte-identical to the single-shot render
        run = json.loads(stub.objects["sheets/fan-sheet/run.json"])
        self.assertEqual(run["published_generation"], gen)
        self.assertEqual(run["status"], "done")
        self.assertEqual(stub.objects[run["published_png_key"]], ref_png)
        # round-8 finding 2: the published png is ATTEMPT-scoped under the
        # generation prefix (sheets/{id}/{gen}/{attempt}/sheet.png), so two
        # same-generation stitchers never overwrite each other's objects
        png_key = run["published_png_key"]
        self.assertTrue(png_key.startswith(f"sheets/fan-sheet/{gen}/"), png_key)
        tail = png_key[len(f"sheets/fan-sheet/{gen}/"):]
        self.assertEqual(tail.count("/"), 1, png_key)
        self.assertTrue(tail.endswith("/sheet.png"), png_key)
        # no mutable fixed keys are written by the fan-out path
        self.assertNotIn("sheets/fan-sheet/sheet.png", stub.objects)
        # scaffolding tiles are removed after the stitch
        self.assertFalse([k for k in stub.objects if "/tiles/" in k])

    def test_stitch_with_missing_tiles_errors(self):
        import handler_poly_sheet as mod

        gen = "g0123456789ab"
        stub = _S3Stub()
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub)
        try:
            p = _run_params("gap-sheet")
            p["generation"] = gen
            _admit(mod, stub, p, gen, [[0, 1], [2, 3]])
            mod.handle_frames({**p, "action": "frames",
                               "task_id": f"sheet_tiles_gap-sheet_{gen}_w0",
                               "frame_indices": [0, 1]})   # frames 2,3 never rendered
            with self.assertRaises(RuntimeError) as ctx:
                mod.handle_stitch({**p, "action": "stitch",
                                   "task_id": f"sheet_stitch_gap-sheet_{gen}"})
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig
        self.assertIn("missing 2 of 4 tiles", str(ctx.exception))

    def test_worker_cancel_stops_between_frames(self):
        import handler_poly_sheet as mod

        gen = "gfedcba987654"
        stub = _S3Stub()
        stub.objects[mod._cancel_key("cx-sheet", gen)] = b"1"
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub)
        try:
            p = _run_params("cx-sheet")
            _admit(mod, stub, p, gen, [[0, 1], [2, 3]])
            resp = mod.handle_frames({**p, "action": "frames",
                                      "generation": gen,
                                      "task_id": f"sheet_tiles_cx-sheet_{gen}_w0",
                                      "frame_indices": [0, 1]})
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig
        body = json.loads(resp["body"])
        # round-10 finding 2: the worker reports the ACTUAL resolved status
        self.assertEqual(body["status"], "cancelled")
        self.assertTrue(body["cancelled"])
        self.assertEqual(body["frames_done"], 0)
        # run.json was actually transitioned
        self.assertEqual(json.loads(stub.objects["sheets/cx-sheet/run.json"])["status"],
                         "cancelled")

    def test_unbound_worker_raises_without_poisoning_status(self):
        """Round-3 finding 7: a request that fails admission (here: no
        run.json to bind to) must RAISE without writing any status row —
        it must not be able to poison a legitimate task's row."""
        import handler_poly_sheet as mod

        rows = []
        # round-8 finding 6: capture and restore the lease primitives too —
        # stubbing them without restoring leaked into later handler tests.
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status,
                mod.claim_task, mod.renew_claim, mod.finalize_task)
        mod.s3 = _S3Stub()
        mod.SWEEP_COEFFGEN = SWEEP_TEST
        mod.report_status = lambda job, task, status, *a, **k: rows.append(status)
        mod.claim_task = lambda *a, **k: True
        mod.renew_claim = lambda *a, **k: True
        mod.finalize_task = lambda *a, **k: True
        try:
            with self.assertRaises(RuntimeError):
                mod.handle_frames({**_run_params("nogen-sheet"),
                                   "action": "frames",
                                   "generation": "g000000000000",
                                   "task_id": "sheet_tiles_nogen_w0",
                                   "frame_indices": [0]})
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status,
             mod.claim_task, mod.renew_claim, mod.finalize_task) = orig
        self.assertEqual(rows, [])

    def test_begin_probes_validates_and_prewrites_rows(self):
        """CR35-F4/F5/F6: begin measures the real degree, budgets from
        it, mints the generation, and writes every status row before
        any async dispatch."""
        import handler_poly_sheet as mod

        rows = []
        stub = _S3Stub()
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        mod.s3 = stub
        mod.SWEEP_COEFFGEN = SWEEP_TEST
        mod.report_status = lambda job, task, status, *a, **k: rows.append((task, status))
        try:
            resp = mod.handle_begin(_run_params("begin-sheet"))
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig
        run = json.loads(resp["body"])
        self.assertEqual(run["sheet_id"], "begin-sheet")
        self.assertTrue(run["generation"].startswith("g"))
        self.assertEqual(run["degree_probe"], 5)
        self.assertEqual(run["steps"], 4)
        self.assertEqual(sum(len(w["frames"]) for w in run["workers"]), 4)
        # a status row per worker + the stitch row, all pre-written
        self.assertEqual(len(rows), len(run["workers"]) + 1)
        self.assertIn("sheets/begin-sheet/run.json", stub.objects)

    def test_zero_shim_and_quantile_survive(self):
        """CR35-F15: an explicit zero must stay zero end-to-end."""
        import handler_poly_sheet as mod

        cfg = mod._parse_sheet_config(_run_params("shim-sheet", extra={
            "frame": {"n": 8, "tile_px": 32, "solver_mode": "jt64",
                      "viewport": {"mode": "quantile", "quantile": 0,
                                   "shim": 0},
                      "rotate": 0},
        }))
        self.assertEqual(cfg["shim"], 0.0)
        self.assertEqual(cfg["quantile"], 0.0)
        # absent still defaults
        cfg2 = mod._parse_sheet_config(_run_params("shim2-sheet"))
        self.assertEqual(cfg2["shim"], 0.05)

    def test_omitted_step_persists_as_executed(self):
        """CR35-F16: values and stored step agree for an omitted step."""
        import handler_poly_sheet as mod

        axis = mod._parse_scan_axis({"token": "$T", "from": 2, "steps": 3,
                                     "spacing": "step"})
        self.assertEqual(axis["values"], [2.0, 3.0, 4.0])
        self.assertEqual(axis["step"], 1.0)

    def test_overlapping_tokens_substitute_lexically(self):
        """CR35-F14: $T must not corrupt $T2; comment-only tokens are
        not presence."""
        import handler_poly_sheet as mod

        p = {"coeff_program_source_text": "poly = fill($T2, $T)\nemit\n"}
        out = mod.substitute_tokens(p, {"$T": 0.5, "$T2": 6})
        self.assertEqual(out["coeff_program_source_text"],
                         "poly = fill(6, 0.5)\nemit\n")
        with self.assertRaises(RuntimeError):
            mod.substitute_tokens(
                {"coeff_program_source_text": "# $S\npoly = fill(3, 1)\nemit\n"},
                {"$S": 1})

    def test_cross_product_two_tokens(self):
        import handler_poly_sheet as mod

        stub = _S3Stub()
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub)
        try:
            p = _run_params("xy-sheet")
            p["coeff_program_source_text"] = SHEET_SRC_2D
            del p["scan"]
            p["scans"] = [
                {"token": "$T", "from": 0.5, "to": 1.0, "steps": 2,
                 "spacing": "linear"},
                {"token": "$S", "from": 1.0, "to": 3.0, "steps": 3,
                 "spacing": "linear"},
            ]
            del p["grid_cols"]
            resp = mod.handle_run(p)
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig
        manifest = json.loads(resp["body"])
        self.assertEqual(manifest["frames"], 6)
        # cross product: axis 0 = columns, axis 1 = rows
        self.assertEqual(manifest["grid"], {"cols": 2, "rows": 3})
        self.assertEqual([f["values"] for f in manifest["frame_records"]],
                         [[0.5, 1.0], [1.0, 1.0],
                          [0.5, 2.0], [1.0, 2.0],
                          [0.5, 3.0], [1.0, 3.0]])
        self.assertEqual(len(manifest["scans"]), 2)
        self.assertEqual(manifest["scans"][1]["token"], "$S")
        self.assertEqual(manifest["scan"]["token"], "$T")   # legacy = axis 0
        import struct as _s
        w, h = _s.unpack(">II", stub.objects["sheets/xy-sheet/sheet.png"][16:24])
        self.assertEqual((w, h), (64, 96))

    def test_cross_product_validation(self):
        import handler_poly_sheet as mod

        orig = mod.report_status
        _real = (mod.claim_task, mod.renew_claim, mod.finalize_task)
        self.addCleanup(lambda: (setattr(mod, "claim_task", _real[0]),
                                 setattr(mod, "renew_claim", _real[1]),
                                 setattr(mod, "finalize_task", _real[2])))
        mod.report_status = lambda *a, **k: None
        mod.claim_task = lambda *a, **k: True
        mod.renew_claim = lambda *a, **k: True
        mod.finalize_task = lambda *a, **k: True
        try:
            p = _run_params("dup-sheet")
            del p["scan"]
            p["scans"] = [
                {"token": "$T", "from": 0, "to": 1, "steps": 2, "spacing": "linear"},
                {"token": "$T", "from": 0, "to": 1, "steps": 2, "spacing": "linear"},
            ]
            with self.assertRaises(RuntimeError) as ctx:
                mod.handle_run(p)
            self.assertIn("distinct", str(ctx.exception))

            p["scans"][1]["token"] = "$S"
            p["scans"][0]["steps"] = 40   # columns cap
            with self.assertRaises(RuntimeError) as ctx:
                mod.handle_run(p)
            self.assertIn("columns", str(ctx.exception))
        finally:
            mod.report_status = orig

    def test_frame_labels_and_pipeline_manifest(self):
        import handler_poly_sheet as mod

        def run(sheet_id, label):
            stub = _S3Stub()
            orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
            self._patched(mod, stub)
            try:
                resp = mod.handle_run(_run_params(sheet_id, extra={
                    "frame": {"n": 8, "tile_px": 128, "solver_mode": "jt64",
                              "viewport": {"mode": "quantile", "quantile": 0.0,
                                           "shim": 0.05},
                              "rotate": 0, "label": label},
                }))
            finally:
                (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig
            return (json.loads(resp["body"]),
                    stub.objects[f"sheets/{sheet_id}/sheet.png"])

        labeled, png_on = run("label-on", True)
        plain, png_off = run("label-off", False)
        self.assertTrue(labeled["label"])
        self.assertFalse(plain["label"])
        # the stamped value text changes the pixels
        self.assertNotEqual(png_on, png_off)
        # the pipeline rides in the manifest UNSUBSTITUTED for populate
        self.assertEqual(labeled["pipeline"]["function"], "const")
        self.assertIn("$T", labeled["pipeline"]["coeff_program_source_text"])
        self.assertEqual(labeled["pipeline"]["cfpv"], [1, 0, 0])

    def test_draw_tile_label_geometry(self):
        import handler_poly_sheet as mod

        tile = bytearray(bytes([0]) * (128 * 128))
        out = mod.draw_tile_label(tile, 128, "1.5", fg=255, bg=0)
        # scale 1, pad 2: glyphs land inside the top-left 30x12 box
        lit = [(i % 128, i // 128) for i, v in enumerate(out) if v == 255]
        self.assertTrue(lit)
        self.assertTrue(all(x < 2 + 3 * 8 and y < 12 for x, y in lit),
                        f"label pixels escaped the corner box: {lit[:5]}")

    def test_draw_tile_label_not_mirrored(self):
        """The font packs LSB-leftmost; reading MSB-first mirrors every
        glyph (user saw 'text from the wrong side of a window'). Pin the
        exact rendered pattern of 'L' against the correct bit order."""
        import handler_poly_sheet as mod
        from cp437_font import FONT_ROWS

        tile = bytearray(bytes([0]) * (64 * 64))
        out = mod.draw_tile_label(tile, 64, "L", fg=255, bg=0)
        rows = FONT_ROWS[ord("L") + 1]
        for gy in range(8):
            for gx in range(8):
                expected = 255 if rows[gy] & (1 << gx) else 0
                self.assertEqual(out[(2 + gy) * 64 + (2 + gx)], expected,
                                 f"glyph pixel ({gx},{gy}) wrong — mirrored?")
        # sanity on shape: L's vertical stroke hugs the LEFT edge
        left_col = min(gx for gy in range(8) for gx in range(8)
                       if out[(2 + gy) * 64 + (2 + gx)])
        self.assertLessEqual(left_col, 1)

    def test_explicit_viewport_and_rotation(self):
        import handler_poly_sheet as mod

        stub = _S3Stub()
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub)
        try:
            resp = mod.handle_run(_run_params("rot-sheet", extra={
                "frame": {"n": 8, "tile_px": 32, "solver_mode": "jt64",
                          "viewport": {"mode": "explicit", "min_re": -2,
                                       "max_re": 2, "min_im": -2, "max_im": 2},
                          "rotate": 90},
            }))
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig
        manifest = json.loads(resp["body"])
        self.assertEqual(manifest["rotate"], 90)
        self.assertEqual(manifest["viewport"]["mode"], "explicit")
        # all frames share the explicit bounds
        bounds = {tuple(f["bounds"]) for f in manifest["frame_records"]}
        self.assertEqual(len(bounds), 1)
        self.assertEqual(list(bounds)[0], (-2.0, 2.0, -2.0, 2.0))


if __name__ == "__main__":
    unittest.main()


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestAdmissionEnforcement(_LeaseIsoTestCase):
    """Review round-2 finding 4/7: frames/stitch must BIND to the
    admitted run — generation, task allocation, payload hash — and an
    older stitch must not overwrite a newer generation's sheet."""

    def _patched(self, mod, s3stub):
        mod.s3 = s3stub
        mod.SWEEP_COEFFGEN = SWEEP_TEST
        mod.report_status = lambda *a, **k: None
        # atomic claim always wins in these S3-focused tests; the lease
        # semantics have their own DDB-backed test. Restore the reals
        # after the test so stubs cannot leak (round-6 finding 6).
        import shared as _shared
        # round-8 finding 6 + round-9 finding 7: restore ALL THREE lease
        # primitives to the KNOWN-REAL shared implementations — capturing
        # the current value could re-save a stub a prior leak left behind.
        self.addCleanup(lambda: setattr(mod, "claim_task", _shared.claim_task))
        self.addCleanup(lambda: setattr(mod, "renew_claim", _shared.renew_claim))
        self.addCleanup(lambda: setattr(mod, "finalize_task", _shared.finalize_task))
        mod.claim_task = lambda *a, **k: True
        mod.renew_claim = lambda *a, **k: True
        mod.finalize_task = lambda *a, **k: True

    def test_unadmitted_and_mismatched_requests_are_refused(self):
        import handler_poly_sheet as mod

        gen = "g111111111111"
        stub = _S3Stub()
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub)
        try:
            p = _run_params("bind-sheet")
            p["generation"] = gen
            # no run.json at all
            with self.assertRaises(RuntimeError) as ctx:
                mod.handle_frames({**p, "action": "frames",
                                   "task_id": f"sheet_tiles_bind-sheet_{gen}_w0",
                                   "frame_indices": [0, 1]})
            self.assertIn("begin first", str(ctx.exception))

            run = _admit(mod, stub, p, gen, [[0, 1], [2, 3]])
            # wrong generation
            with self.assertRaises(RuntimeError):
                mod.handle_frames({**p, "action": "frames",
                                   "generation": "g222222222222",
                                   "task_id": f"sheet_tiles_bind-sheet_{gen}_w0",
                                   "frame_indices": [0, 1]})
            # unallocated task
            with self.assertRaises(RuntimeError):
                mod.handle_frames({**p, "action": "frames",
                                   "task_id": f"sheet_tiles_bind-sheet_{gen}_w9",
                                   "frame_indices": [0, 1]})
            # frame range not matching the allocation
            with self.assertRaises(RuntimeError):
                mod.handle_frames({**p, "action": "frames",
                                   "task_id": f"sheet_tiles_bind-sheet_{gen}_w0",
                                   "frame_indices": [0, 1, 2]})
            # tampered payload (different program) fails the hash
            tampered = {**p, "coeff_program_source_text":
                        p["coeff_program_source_text"].replace("$T", "3*$T")}
            with self.assertRaises(RuntimeError) as ctx:
                mod.handle_frames({**tampered, "action": "frames",
                                   "task_id": f"sheet_tiles_bind-sheet_{gen}_w0",
                                   "frame_indices": [0, 1]})
            self.assertIn("does not match the admitted", str(ctx.exception))
            self.assertTrue(run["params_hash"])
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig

    def test_superseded_stitch_cannot_publish(self):
        import handler_poly_sheet as mod

        old_gen = "gaaaaaaaaaaaa"
        stub = _S3Stub()
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub)
        try:
            p = _run_params("race-sheet")
            _admit(mod, stub, p, old_gen, [[0, 1], [2, 3]])
            for w, frames in ((0, [0, 1]), (1, [2, 3])):
                mod.handle_frames({**p, "action": "frames", "generation": old_gen,
                                   "task_id": f"sheet_tiles_race-sheet_{old_gen}_w{w}",
                                   "frame_indices": frames})
            # a NEWER run takes over before the old stitch fires
            new_gen = "gbbbbbbbbbbbb"
            _admit(mod, stub, p, new_gen, [[0, 1], [2, 3]])
            with self.assertRaises(RuntimeError) as ctx:
                mod.handle_stitch({**p, "action": "stitch", "generation": old_gen,
                                   "task_id": f"sheet_stitch_race-sheet_{old_gen}"})
            self.assertIn("superseded", str(ctx.exception))
            self.assertNotIn("sheets/race-sheet/sheet.png", stub.objects)
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig

    def test_unknown_action_is_rejected(self):
        import handler_poly_sheet as mod

        with self.assertRaises(RuntimeError):
            mod.handler({"body": json.dumps({"sheet_id": "x"})}, None)
        with self.assertRaises(RuntimeError):
            mod.handler({"body": json.dumps({"action": "evil", "sheet_id": "x"})}, None)

    def test_cancel_fails_closed_on_persistent_s3_errors(self):
        import handler_poly_sheet as mod

        class _Angry:
            def head_object(self, **kw):
                from botocore.exceptions import ClientError
                raise ClientError({"Error": {"Code": "SlowDown"}}, "HeadObject")

        orig = mod.s3
        mod.s3 = _Angry()
        try:
            self.assertTrue(mod._cancel_requested("any-sheet", "g0123456789ab"))
        finally:
            mod.s3 = orig


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestRound4Lifecycle(_LeaseIsoTestCase):
    """Round-4 regressions: the lifecycle code the earlier rounds changed
    without tests — CAS interleaving, fail-closed publish, terminal run
    state, idempotent replay, the runtime deadline, and action gating."""

    def _patched(self, mod, s3stub):
        mod.s3 = s3stub
        mod.SWEEP_COEFFGEN = SWEEP_TEST
        mod.report_status = lambda *a, **k: None
        # atomic claim always wins in these S3-focused tests; the lease
        # semantics have their own DDB-backed test. Restore the reals
        # after the test so stubs cannot leak (round-6 finding 6).
        import shared as _shared
        # round-8 finding 6 + round-9 finding 7: restore ALL THREE lease
        # primitives to the KNOWN-REAL shared implementations — capturing
        # the current value could re-save a stub a prior leak left behind.
        self.addCleanup(lambda: setattr(mod, "claim_task", _shared.claim_task))
        self.addCleanup(lambda: setattr(mod, "renew_claim", _shared.renew_claim))
        self.addCleanup(lambda: setattr(mod, "finalize_task", _shared.finalize_task))
        mod.claim_task = lambda *a, **k: True
        mod.renew_claim = lambda *a, **k: True
        mod.finalize_task = lambda *a, **k: True

    def _render_generation(self, mod, stub, sheet_id, gen):
        p = _run_params(sheet_id)
        _admit(mod, stub, p, gen, [[0, 1], [2, 3]])
        for w, frames in ((0, [0, 1]), (1, [2, 3])):
            mod.handle_frames({**p, "action": "frames", "generation": gen,
                               "task_id": f"sheet_tiles_{sheet_id}_{gen}_w{w}",
                               "frame_indices": frames})
        return p

    def test_interleaved_commit_pointer_follows_the_winner(self):
        """The reviewer's exact race: A stitches, then B begins+commits,
        then A's stitch runs. A must LOSE the commit (run.json already B),
        and each generation's immutable bytes stay distinct — the pointer
        never shows A's content while marked B."""
        import handler_poly_sheet as mod

        gen_a, gen_b = "gaaaaaaaaaaaa", "gbbbbbbbbbbbb"
        stub = _S3Stub()
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub)
        try:
            # A renders and COMMITS first (pointer -> A)
            pa = self._render_generation(mod, stub, "race-sheet", gen_a)
            mod.handle_stitch({**pa, "action": "stitch", "generation": gen_a,
                               "task_id": f"sheet_stitch_race-sheet_{gen_a}"})
            run_a = json.loads(stub.objects["sheets/race-sheet/run.json"])
            self.assertEqual(run_a["published_generation"], gen_a)
            # round-8 finding 2: the pointer resolves the winning ATTEMPT
            # under gen_a; capture that key and its bytes
            a_key = run_a["published_png_key"]
            self.assertTrue(a_key.startswith(f"sheets/race-sheet/{gen_a}/"), a_key)
            a_png = stub.objects[a_key]
            # B supersedes: fresh begin overwrites run.json, renders, commits
            _admit(mod, stub, pa, gen_b, [[0, 1], [2, 3]])
            for w, frames in ((0, [0, 1]), (1, [2, 3])):
                mod.handle_frames({**pa, "action": "frames", "generation": gen_b,
                                   "task_id": f"sheet_tiles_race-sheet_{gen_b}_w{w}",
                                   "frame_indices": frames})
            mod.handle_stitch({**pa, "action": "stitch", "generation": gen_b,
                               "task_id": f"sheet_stitch_race-sheet_{gen_b}"})
            # a stale REPLAY of A's stitch now runs and MUST be refused —
            # it can never overwrite B's pointer or B's bytes
            with self.assertRaises(RuntimeError) as ctx:
                mod.handle_stitch({**pa, "action": "stitch", "generation": gen_a,
                                   "task_id": f"sheet_stitch_race-sheet_{gen_a}_replay"})
            self.assertIn("superseded", str(ctx.exception))
            run = json.loads(stub.objects["sheets/race-sheet/run.json"])
            self.assertEqual(run["published_generation"], gen_b)
            self.assertTrue(run["published_png_key"].startswith(
                f"sheets/race-sheet/{gen_b}/"), run["published_png_key"])
            # A's winning-attempt bytes survive untouched — the stale replay
            # wrote to its OWN attempt prefix, never over A's (finding 2) —
            # and the pointer is the only thing that moved
            self.assertEqual(stub.objects[a_key], a_png)
            self.assertIn(run["published_png_key"], stub.objects)
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig

    def test_publish_fails_closed_without_conditional_writes(self):
        """If the runtime lacks S3 conditional writes, the commit must
        REFUSE rather than clobber — no silent unconditional fallback."""
        import handler_poly_sheet as mod

        # reject only IfMatch so create-only tiles (IfNoneMatch) still write —
        # this isolates the COMMIT CAS's fail-closed (round-12: the write-once
        # tile fail-closed is covered by TestRound10Fencing)
        class _NoCAS(_S3Stub):
            def put_object(self, **kw):
                if "IfMatch" in kw:
                    raise TypeError("put_object() got an unexpected keyword 'IfMatch'")
                return super().put_object(**kw)

        stub = _NoCAS()
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub)
        try:
            gen = "gcccccccccccc"
            p = _run_params("noCAS-sheet")
            # begin's run.json write also uses IfNoneMatch -> emulate admit
            stub.objects["sheets/noCAS-sheet/run.json"] = json.dumps({
                "sheet_id": "noCAS-sheet", "generation": gen, "job_id": "sheet_job",
                "steps": 4, "workers": [
                    {"task_id": f"sheet_tiles_noCAS-sheet_{gen}_w0", "frames": [0, 1]},
                    {"task_id": f"sheet_tiles_noCAS-sheet_{gen}_w1", "frames": [2, 3]}],
                "stitch_task_id": f"sheet_stitch_noCAS-sheet_{gen}",
                "degree_probe": 5, "params_hash": mod._params_hash(p),
                "status": "running"}).encode()
            stub.etags["sheets/noCAS-sheet/run.json"] = '"e0"'
            for w, frames in ((0, [0, 1]), (1, [2, 3])):
                mod.handle_frames({**p, "action": "frames", "generation": gen,
                                   "task_id": f"sheet_tiles_noCAS-sheet_{gen}_w{w}",
                                   "frame_indices": frames})
            with self.assertRaises(RuntimeError) as ctx:
                mod.handle_stitch({**p, "action": "stitch", "generation": gen,
                                   "task_id": f"sheet_stitch_noCAS-sheet_{gen}"})
            self.assertIn("conditional writes", str(ctx.exception))
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig

    def test_worker_failure_marks_run_terminal(self):
        """A worker error moves run.json off 'running' (round-3 f5) so
        /list-sheets discovery stops rediscovering a dead run."""
        import handler_poly_sheet as mod

        gen = "gdddddddddddd"
        stub = _S3Stub()
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub)
        try:
            p = _run_params("fail-sheet")
            _admit(mod, stub, p, gen, [[0, 1], [2, 3]])
            # break the binary so the render raises
            mod.SWEEP_COEFFGEN = "/nonexistent/binary"
            with self.assertRaises(Exception):
                mod.handle_frames({**p, "action": "frames", "generation": gen,
                                   "task_id": f"sheet_tiles_fail-sheet_{gen}_w0",
                                   "frame_indices": [0, 1]})
            run = json.loads(stub.objects["sheets/fail-sheet/run.json"])
            self.assertEqual(run["status"], "failed")
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig

    def test_abandon_and_cancel_mark_run_terminal(self):
        import handler_poly_sheet as mod

        gen = "geeeeeeeeeeee"
        stub = _S3Stub()
        orig = mod.s3
        mod.s3 = stub
        try:
            p = _run_params("term-sheet")
            _admit(mod, stub, p, gen, [[0, 1]])
            mod.handle_abandon({"action": "abandon", "sheet_id": "term-sheet",
                                "generation": gen})
            self.assertEqual(json.loads(stub.objects["sheets/term-sheet/run.json"])["status"],
                             "abandoned")
            # re-admit and cancel
            _admit(mod, stub, p, gen, [[0, 1]])
            mod.report_status = lambda *a, **k: None
            mod.handle_cancel({"action": "cancel", "sheet_id": "term-sheet",
                               "generation": gen})
            self.assertEqual(json.loads(stub.objects["sheets/term-sheet/run.json"])["status"],
                             "cancelled")
        finally:
            mod.s3 = orig

    def test_action_run_is_not_dispatchable(self):
        import handler_poly_sheet as mod

        for action in ("run", "evil", ""):
            with self.assertRaises(RuntimeError):
                mod.handler({"body": json.dumps({"action": action, "sheet_id": "x"})}, None)

    def test_frame_bounds_keep_full_precision(self):
        """Round-3 finding 8: bounds must not be rounded to 12 decimals."""
        import handler_poly_sheet as mod

        stub = _S3Stub()
        gen = "gffffffffffff"
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub)
        try:
            p = _run_params("prec-sheet", extra={
                "frame": {"n": 8, "tile_px": 32, "solver_mode": "jt64",
                          "viewport": {"mode": "explicit", "min_re": -1.234567890123456,
                                       "max_re": 2.0, "min_im": -2.0, "max_im": 2.0},
                          "rotate": 0}})
            resp = mod.handle_run({**p, "generation": gen})
            manifest = json.loads(resp["body"])
            b = manifest["frame_records"][0]["bounds"]
            # the square-fit preserves full binary64 in at least one edge
            self.assertTrue(any(len(repr(x)) > 14 for x in b))
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig


class TestRunBinaryDeadline(_LeaseIsoTestCase):
    def test_past_deadline_refuses_before_spawning(self):
        """Round-3 finding 3: the native-invocation deadline is a real
        budget wall, checked at the wrapper (not just the loop top)."""
        import handler_poly_sheet as mod

        with self.assertRaises(RuntimeError) as ctx:
            mod._run_binary("/bin/true", "/tmp/x", {}, "probe",
                            deadline_s=__import__("time").time() - 1)
        self.assertIn("budget", str(ctx.exception))


class _LeaseDDB:
    """A fake DDB that honours the claim/renew ConditionExpressions —
    status, lease_expiry_ms, and claim_owner — so the lease semantics are
    actually exercised (round-6)."""

    def __init__(self, clock):
        self.rows = {}     # (job,task) -> {status, lease_expiry_ms, claim_owner}
        self.clock = clock   # mutable [now_ms]

    def _k(self, Key):
        return (Key["job_id"]["S"], Key["task_id"]["S"])

    def seed(self, job, task, **attrs):
        self.rows[(job, task)] = dict(attrs)

    def update_item(self, TableName, Key, UpdateExpression,
                    ConditionExpression, ExpressionAttributeValues):
        from botocore.exceptions import ClientError
        k = self._k(Key)
        row = self.rows.get(k, {})
        v = ExpressionAttributeValues
        now = int(v[":now"]["N"]) if ":now" in v else self.clock[0]
        ok = False
        if "claim_owner = :owner" in ConditionExpression and \
                "task_status IN" not in ConditionExpression:
            # renew: owner must match
            ok = row.get("claim_owner") == v[":owner"]["S"]
        else:
            allowed = {vv["S"] for nm, vv in v.items() if nm.startswith(":s")}
            status = row.get("task_status")
            lease = row.get("lease_expiry_ms")
            ok = (status in allowed
                  or (status == "running" and (lease is None or lease < now))
                  or row.get("claim_owner") == v[":owner"]["S"])
        if not ok:
            raise ClientError(
                {"Error": {"Code": "ConditionalCheckFailedException"}}, "UpdateItem")
        # apply the SET / REMOVE
        row = dict(row)
        if ":owner" in v:
            row["claim_owner"] = v[":owner"]["S"]
        if ":lease" in v:
            row["lease_expiry_ms"] = int(v[":lease"]["N"])
        if ":running" in v and "task_status = :running" in UpdateExpression:
            row["task_status"] = "running"
        if ":status" in v and "task_status = :status" in UpdateExpression:
            row["task_status"] = v[":status"]["S"]
        if "REMOVE" in UpdateExpression:
            removed = UpdateExpression.split("REMOVE", 1)[1]
            if "lease_expiry_ms" in removed:
                row.pop("lease_expiry_ms", None)
            # round-8 finding 7: finalize clears claim_owner too, so a
            # terminal row is unreclaimable even by the same owner
            if "claim_owner" in removed:
                row.pop("claim_owner", None)
        self.rows[k] = row


class TestClaimLease(_LeaseIsoTestCase):
    """Round-6 finding 1: the claim is a LEASE — one live owner, but a
    crashed owner's lease expires so a redispatch can reclaim."""

    def _patch(self, ddb):
        import shared
        orig = shared._ddb
        shared._ddb = ddb
        self.addCleanup(lambda: setattr(shared, "_ddb", orig))

    def test_one_owner_wins_duplicate_loses(self):
        import shared

        clock = [1_000_000]
        ddb = _LeaseDDB(clock)
        ddb.seed("j", "t", task_status="started")
        self._patch(ddb)
        self.assertTrue(shared.claim_task("j", "t", owner="A", lease_seconds=120))
        # a duplicate with a DIFFERENT owner loses while A's lease is live
        self.assertFalse(shared.claim_task("j", "t", owner="B", lease_seconds=120))
        # A can renew and re-enter idempotently
        self.assertTrue(shared.renew_claim("j", "t", owner="A"))
        self.assertTrue(shared.claim_task("j", "t", owner="A"))
        # B still cannot
        self.assertFalse(shared.claim_task("j", "t", owner="B"))

    def test_expired_lease_allows_takeover(self):
        import shared
        import time as _t

        ddb = _LeaseDDB([0])
        ddb.seed("j", "t", task_status="started")
        self._patch(ddb)
        self.assertTrue(shared.claim_task("j", "t", owner="A", lease_seconds=0.001))
        _t.sleep(0.01)   # A's lease expires (A 'crashed')
        # a redispatched worker B reclaims the stale lease — crash recovery
        self.assertTrue(shared.claim_task("j", "t", owner="B", lease_seconds=120))
        # now A's renew fails: it lost the lease and must abort
        self.assertFalse(shared.renew_claim("j", "t", owner="A"))
        self.assertTrue(shared.renew_claim("j", "t", owner="B"))

    def test_done_and_missing_never_claim(self):
        import shared

        ddb = _LeaseDDB([0])
        ddb.seed("j", "done-task", task_status="done")
        self._patch(ddb)
        self.assertFalse(shared.claim_task("j", "done-task", owner="A"))
        self.assertFalse(shared.claim_task("j", "absent", owner="A"))

    def test_terminal_row_not_reclaimable_by_same_owner(self):
        """Round-8 finding 7: finalize clears claim_owner, so the SAME
        owner cannot move its own terminal row back to running via the
        `OR claim_owner = :owner` re-entry clause."""
        import shared

        ddb = _LeaseDDB([1_000_000])
        ddb.seed("j", "t", task_status="started")
        self._patch(ddb)
        self.assertTrue(shared.claim_task("j", "t", owner="A", lease_seconds=120))
        self.assertTrue(shared.finalize_task("j", "t", owner="A", status="done"))
        self.assertNotIn("claim_owner", ddb.rows[("j", "t")])
        # A owned it and its lease was cleared — it CANNOT re-claim a done row
        self.assertFalse(shared.claim_task("j", "t", owner="A", lease_seconds=120))
        self.assertEqual(ddb.rows[("j", "t")]["task_status"], "done")


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestRound5(_LeaseIsoTestCase):
    def _patched(self, mod, s3stub):
        import shared as _shared
        mod.s3 = s3stub
        mod.SWEEP_COEFFGEN = SWEEP_TEST
        mod.report_status = lambda *a, **k: None
        # round-9 finding 7: this fixture used to stub all three lease
        # primitives with NO cleanup, relying on each test's finally block
        # which restored ONLY claim_task — so renew_claim and finalize_task
        # leaked stubs into every later handler test. Restore all three to
        # the KNOWN-REAL shared implementations (not a captured current
        # value, which a prior leak could already have stubbed), leak-proof
        # regardless of test order.
        self.addCleanup(lambda: setattr(mod, "claim_task", _shared.claim_task))
        self.addCleanup(lambda: setattr(mod, "renew_claim", _shared.renew_claim))
        self.addCleanup(lambda: setattr(mod, "finalize_task", _shared.finalize_task))
        mod.claim_task = lambda *a, **k: True
        mod.renew_claim = lambda *a, **k: True
        mod.finalize_task = lambda *a, **k: True

    def test_terminal_run_refuses_late_work(self):
        """Finding 1: a cancelled/abandoned run must reject a late worker
        or stitch — a straggler cannot resurrect it to 'done'."""
        import handler_poly_sheet as mod

        gen = "g1a1a1a1a1a1a"
        stub = _S3Stub()
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status,
                getattr(mod, "claim_task", None))
        self._patched(mod, stub)
        try:
            p = _run_params("term2-sheet")
            _admit(mod, stub, p, gen, [[0, 1], [2, 3]])
            # cancel marks the run terminal
            mod.handle_cancel({"action": "cancel", "sheet_id": "term2-sheet",
                               "generation": gen})
            with self.assertRaises(RuntimeError) as ctx:
                mod.handle_frames({**p, "action": "frames", "generation": gen,
                                   "task_id": f"sheet_tiles_term2-sheet_{gen}_w0",
                                   "frame_indices": [0, 1]})
            self.assertIn("terminal", str(ctx.exception))
            with self.assertRaises(RuntimeError):
                mod.handle_stitch({**p, "action": "stitch", "generation": gen,
                                   "task_id": f"sheet_stitch_term2-sheet_{gen}"})
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status, mod.claim_task) = orig

    def test_status_write_failure_after_commit_keeps_sheet_published(self):
        """Finding 2: a DDB status-write failure AFTER the pointer commit
        must not report a published sheet as failed."""
        import handler_poly_sheet as mod

        gen = "g2b2b2b2b2b2b"
        stub = _S3Stub()
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status,
                getattr(mod, "claim_task", None))
        mod.s3 = stub
        mod.SWEEP_COEFFGEN = SWEEP_TEST
        mod.claim_task = lambda *a, **k: True
        mod.renew_claim = lambda *a, **k: True
        # round-9 finding 6: the finally block below restores only claim_task
        # (via `orig`); renew_claim and finalize_task must be restored too or
        # they LEAK into later tests (reviewer ran this test alone and got
        # renew_real=False). Restore all three to the KNOWN-REAL shared impls.
        import shared as _shared
        self.addCleanup(lambda: setattr(mod, "claim_task", _shared.claim_task))
        self.addCleanup(lambda: setattr(mod, "renew_claim", _shared.renew_claim))
        self.addCleanup(lambda: setattr(mod, "finalize_task", _shared.finalize_task))
        # fail ONLY the stitch's terminal 'done' finalize (post-commit) —
        # its failure must not turn the published sheet into a failure
        def flaky_finalize(job, task, *a, **k):
            if k.get("status") == "done" and "stitch" in task:
                raise RuntimeError("DDB throttled")
            return True
        mod.finalize_task = flaky_finalize
        mod.report_status = lambda *a, **k: None
        try:
            p = _run_params("commit-sheet")
            _admit(mod, stub, p, gen, [[0, 1], [2, 3]])
            for w, frames in ((0, [0, 1]), (1, [2, 3])):
                mod.handle_frames({**p, "action": "frames", "generation": gen,
                                   "task_id": f"sheet_tiles_commit-sheet_{gen}_w{w}",
                                   "frame_indices": frames})
            # the done-report raises internally but the stitch must SUCCEED
            resp = mod.handle_stitch({**p, "action": "stitch", "generation": gen,
                                      "task_id": f"sheet_stitch_commit-sheet_{gen}"})
            self.assertEqual(resp["statusCode"], 200)
            run = json.loads(stub.objects["sheets/commit-sheet/run.json"])
            self.assertEqual(run["status"], "done")
            self.assertEqual(run["published_generation"], gen)
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status, mod.claim_task) = orig

    def test_begin_carries_forward_prior_publication(self):
        """Finding 7: a new begin keeps the previous published pointer so
        the last-good sheet stays reachable until the new one commits."""
        import handler_poly_sheet as mod

        gen_a, gen_b = "g3c3c3c3c3c3c", "g4d4d4d4d4d4d"
        stub = _S3Stub()
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status,
                getattr(mod, "claim_task", None))
        self._patched(mod, stub)
        try:
            p = _run_params("carry-sheet")
            _admit(mod, stub, p, gen_a, [[0, 1], [2, 3]])
            for w, frames in ((0, [0, 1]), (1, [2, 3])):
                mod.handle_frames({**p, "action": "frames", "generation": gen_a,
                                   "task_id": f"sheet_tiles_carry-sheet_{gen_a}_w{w}",
                                   "frame_indices": frames})
            mod.handle_stitch({**p, "action": "stitch", "generation": gen_a,
                               "task_id": f"sheet_stitch_carry-sheet_{gen_a}"})
            # a fresh begin for gen_b must retain gen_a's pointer
            begin_params = {**p, "action": "begin", "job_id": "sheet_job",
                            "generation": gen_b}
            # bypass the probe by calling handle_begin (renders the probe
            # frame via the real binary — fine)
            mod.handle_begin(begin_params)
            run = json.loads(stub.objects["sheets/carry-sheet/run.json"])
            self.assertEqual(run["status"], "running")   # new gen in flight
            self.assertEqual(run["published_generation"], gen_a)   # still A
            # round-8 finding 2: attempt-scoped published path under gen_a
            self.assertTrue(run["published_png_key"].startswith(
                f"sheets/carry-sheet/{gen_a}/"), run["published_png_key"])
            self.assertTrue(run["published_png_key"].endswith("/sheet.png"))
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status, mod.claim_task) = orig


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestConcurrentCarryForward(_LeaseIsoTestCase):
    """Round-6 finding 5: begin's carry-forward is a CAS loop. If another
    generation commits between begin's read and its write, begin must
    re-read and preserve the newer pointer instead of erasing it."""

    def test_begin_reretries_and_keeps_a_concurrently_committed_pointer(self):
        import handler_poly_sheet as mod

        stub = _S3Stub()
        # a sheet already published at gen A
        run0 = {"sheet_id": "cc-sheet", "generation": "gaaaaaaaaaaaa",
                "job_id": "sheet_job", "status": "done",
                "published_generation": "gaaaaaaaaaaaa",
                "published_png_key": "sheets/cc-sheet/gaaaaaaaaaaaa/sheet.png",
                "published_manifest_key": "sheets/cc-sheet/gaaaaaaaaaaaa/sheet.json"}
        stub.put_object(Bucket="b", Key="sheets/cc-sheet/run.json",
                        Body=json.dumps(run0).encode())

        # inject a concurrent commit (gen A2) after begin's FIRST read but
        # before its first CAS write, so the first CAS loses and retries
        real_get = stub.get_object
        state = {"injected": False}

        def get_with_injection(Bucket, Key):
            result = real_get(Bucket=Bucket, Key=Key)
            if Key == "sheets/cc-sheet/run.json" and not state["injected"]:
                state["injected"] = True
                run_a2 = dict(run0)
                run_a2["generation"] = "ga2a2a2a2a2a"
                run_a2["published_generation"] = "ga2a2a2a2a2a"
                run_a2["published_png_key"] = "sheets/cc-sheet/ga2a2a2a2a2a/sheet.png"
                run_a2["published_manifest_key"] = "sheets/cc-sheet/ga2a2a2a2a2a/sheet.json"
                stub.put_object(Bucket="b", Key="sheets/cc-sheet/run.json",
                                Body=json.dumps(run_a2).encode())
            return result

        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        mod.s3 = stub
        stub.get_object = get_with_injection
        mod.SWEEP_COEFFGEN = SWEEP_TEST
        mod.report_status = lambda *a, **k: None
        try:
            p = _run_params("cc-sheet")
            resp = mod.handle_begin({**p, "action": "begin", "job_id": "sheet_job"})
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig
        run = json.loads(resp["body"])
        # begin's first CAS lost to the injected A2 commit and retried,
        # carrying A2's pointer forward — NOT erasing it back to A
        self.assertTrue(state["injected"])
        self.assertEqual(run["published_generation"], "ga2a2a2a2a2a")
        self.assertEqual(run["published_png_key"],
                         "sheets/cc-sheet/ga2a2a2a2a2a/sheet.png")
        self.assertEqual(run["status"], "running")


class TestFinalizeTaskFencing(_LeaseIsoTestCase):
    """Round-7 findings 1/2/6: the terminal write is OWNER-CONDITIONAL —
    a stale owner that lost its lease cannot overwrite its successor."""

    def _patch(self, ddb):
        import shared
        orig = shared._ddb
        shared._ddb = ddb
        self.addCleanup(lambda: setattr(shared, "_ddb", orig))

    def test_stale_owner_cannot_finalize_over_successor(self):
        import shared

        ddb = _LeaseDDB([0])
        ddb.seed("j", "t", task_status="started")
        self._patch(ddb)
        # A claims, then A's lease expires and B takes it over
        self.assertTrue(shared.claim_task("j", "t", owner="A", lease_seconds=0.001))
        import time as _t
        _t.sleep(0.01)
        self.assertTrue(shared.claim_task("j", "t", owner="B", lease_seconds=120))
        # the stale owner A's terminal write is REFUSED (owner mismatch)
        self.assertFalse(shared.finalize_task("j", "t", owner="A", status="error",
                                              error_msg="A crashed late"))
        self.assertEqual(ddb.rows[("j", "t")]["task_status"], "running")
        self.assertEqual(ddb.rows[("j", "t")]["claim_owner"], "B")
        # the live owner B CAN finalize
        self.assertTrue(shared.finalize_task("j", "t", owner="B", status="done"))
        self.assertEqual(ddb.rows[("j", "t")]["task_status"], "done")
        # the terminal write clears the lease so the row is not reclaimable
        self.assertNotIn("lease_expiry_ms", ddb.rows[("j", "t")])


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestRound7WorkerFencing(_LeaseIsoTestCase):
    def _patched(self, mod, stub):
        mod.s3 = stub
        mod.SWEEP_COEFFGEN = SWEEP_TEST
        mod.report_status = lambda *a, **k: None
        # round-9 finding 7: restore to the KNOWN-REAL shared impls, never a
        # captured current value (a prior leak could have stubbed it)
        import shared as _shared
        self.addCleanup(lambda: (setattr(mod, "claim_task", _shared.claim_task),
                                 setattr(mod, "renew_claim", _shared.renew_claim),
                                 setattr(mod, "finalize_task", _shared.finalize_task)))

    def test_worker_lost_lease_exits_benignly(self):
        """Finding 1: a worker whose renew fails mid-run must exit without
        failing the run (a successor owns it)."""
        import handler_poly_sheet as mod

        gen = "g7a7a7a7a7a7a"
        stub = _S3Stub()
        self._patched(mod, stub)
        mod.claim_task = lambda *a, **k: True
        mod.renew_claim = lambda *a, **k: False   # the lease was stolen
        finals = []
        mod.finalize_task = lambda *a, **k: finals.append(k.get("status")) or True
        p = _run_params("lost-sheet")
        _admit(mod, stub, p, gen, [[0, 1], [2, 3]])
        resp = mod.handle_frames({**p, "action": "frames", "generation": gen,
                                  "task_id": f"sheet_tiles_lost-sheet_{gen}_w0",
                                  "frame_indices": [0, 1]})
        body = json.loads(resp["body"])
        self.assertTrue(body.get("lost_lease"))
        # the run was NOT marked failed, and no terminal error was written
        self.assertEqual(json.loads(stub.objects["sheets/lost-sheet/run.json"])["status"],
                         "running")
        self.assertNotIn("error", finals)

    def test_failure_without_ownership_does_not_fail_run(self):
        """Round-10 finding 2: the failure path's ownership check is now a
        renew (not the terminal finalize). If renew shows we lost the lease,
        _finalize_failure_or_exit exits benignly and the run is NOT failed."""
        import handler_poly_sheet as mod

        gen = "g7b7b7b7b7b7b"
        stub = _S3Stub()
        self._patched(mod, stub)
        stub.put_object(Bucket="b", Key="sheets/lostfail-sheet/run.json",
                        Body=json.dumps({"generation": gen, "status": "running"}).encode())
        mod.renew_claim = lambda *a, **k: False    # a successor owns the task
        resp = mod._finalize_failure_or_exit(
            "j", "sheet_stitch_lostfail-sheet", "A", "lostfail-sheet", gen,
            RuntimeError("render failed"), phase_label="Stitch failed")
        self.assertTrue(json.loads(resp["body"]).get("lost_lease"))
        # the successor's run was NOT marked failed
        self.assertEqual(json.loads(stub.objects["sheets/lostfail-sheet/run.json"])["status"],
                         "running")

    def test_second_stitch_of_same_generation_is_rejected(self):
        """Finding 3: a second stitcher of an already-published generation
        must not re-publish."""
        import handler_poly_sheet as mod

        gen = "g7c7c7c7c7c7c"
        stub = _S3Stub()
        self._patched(mod, stub)
        mod.claim_task = lambda *a, **k: True
        mod.renew_claim = lambda *a, **k: True
        mod.finalize_task = lambda *a, **k: True
        p = _run_params("dbl-sheet")
        _admit(mod, stub, p, gen, [[0, 1], [2, 3]])
        for w, frames in ((0, [0, 1]), (1, [2, 3])):
            mod.handle_frames({**p, "action": "frames", "generation": gen,
                               "task_id": f"sheet_tiles_dbl-sheet_{gen}_w{w}",
                               "frame_indices": frames})
        mod.handle_stitch({**p, "action": "stitch", "generation": gen,
                           "task_id": f"sheet_stitch_dbl-sheet_{gen}"})
        # a second stitch of the SAME generation is refused — the bind
        # terminal check catches the sequential case; the commit
        # already-published check (asserted directly below) is the
        # concurrent-race backstop
        with self.assertRaises(RuntimeError) as ctx:
            mod.handle_stitch({**p, "action": "stitch", "generation": gen,
                               "task_id": f"sheet_stitch_dbl-sheet_{gen}"})
        msg = str(ctx.exception)
        self.assertTrue("terminal" in msg or "already published" in msg, msg)
        # round-9: the commit is now IDEMPOTENT through the run.json CAS — a
        # second commit of an already-published same-generation run resolves
        # to 'done' WITHOUT re-publishing or corrupting the pointer (it does
        # not raise; the CAS + already-published check is the backstop)
        published = json.loads(stub.objects["sheets/dbl-sheet/run.json"])
        outcome, won = mod._commit_run_publication("dbl-sheet", gen,
                                                   f"sheets/dbl-sheet/{gen}/")
        # round-10 finding 1: idempotent 'done' but won=False (ANOTHER
        # attempt published) — so the caller must NOT family-prune
        self.assertEqual(outcome, "done")
        self.assertFalse(won)
        # the pointer is unchanged — the first publish still owns run.json
        self.assertEqual(json.loads(stub.objects["sheets/dbl-sheet/run.json"]),
                         published)


class TestRound8Heartbeat(_LeaseIsoTestCase):
    """Round-8 finding 1: a background heartbeat keeps a live worker's
    lease across long native ops, and latches `lost` when a successor
    takes the lease over so the worker stops before its next write."""

    def _patch(self, ddb):
        import shared
        orig = shared._ddb
        shared._ddb = ddb
        self.addCleanup(lambda: setattr(shared, "_ddb", orig))

    def test_heartbeat_renews_then_latches_on_takeover(self):
        import shared
        import time as _t

        ddb = _LeaseDDB([1_000_000])
        ddb.seed("j", "t", task_status="started")
        self._patch(ddb)
        self.assertTrue(shared.claim_task("j", "t", owner="A", lease_seconds=120))
        hb = shared.LeaseHeartbeat("j", "t", owner="A", interval_s=0.02).start()
        try:
            _t.sleep(0.05)                       # a few renew ticks
            self.assertFalse(hb.lost)            # A still owns it
            lease_after_renew = ddb.rows[("j", "t")]["lease_expiry_ms"]
            self.assertGreater(lease_after_renew, 0)   # the heartbeat renewed
            # a healthy A can't be reclaimed (its lease is live) — simulate a
            # successor takeover directly by flipping the owner, which is
            # exactly what B's claim would do once A stopped renewing
            ddb.rows[("j", "t")]["claim_owner"] = "B"
            for _ in range(50):
                if hb.lost:
                    break
                _t.sleep(0.02)
            self.assertTrue(hb.lost)             # A's next renew failed -> latched
        finally:
            hb.stop()
        # after stop the thread is joined and no longer renews
        self.assertIsNone(hb._thread)

    def test_transient_ddb_error_does_not_latch_lost(self):
        """A throttle on renew must NOT be treated as a lost lease
        (finding-3 discipline applied to the heartbeat)."""
        import shared

        class _Flaky:
            def update_item(self, **kw):
                raise RuntimeError("throttled")
        orig = shared._ddb
        shared._ddb = _Flaky()
        self.addCleanup(lambda: setattr(shared, "_ddb", orig))
        import time as _t
        hb = shared.LeaseHeartbeat("j", "t", owner="A", interval_s=0.02).start()
        try:
            _t.sleep(0.08)
            # the heartbeat is a KEEP-ALIVE, not the write fence (round-9
            # finding 1) — a transient renew error must not tear down the
            # keep-alive. Ownership at a shared write is proved SEPARATELY
            # by the synchronous fail-closed _owns_for_write (tested below),
            # so a stale keep-alive can never authorize a write on its own.
            self.assertFalse(hb.lost)
        finally:
            hb.stop()


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestRound8Fencing(_LeaseIsoTestCase):
    """Round-8 findings 2/3: stitch artifacts are ATTEMPT-scoped so two
    same-generation stitchers cannot overwrite each other, and DDB
    uncertainty never authorizes failing a run."""

    def _patched(self, mod, stub):
        mod.s3 = stub
        mod.SWEEP_COEFFGEN = SWEEP_TEST
        mod.report_status = lambda *a, **k: None
        # round-9 finding 7: restore to the KNOWN-REAL shared impls, never a
        # captured current value (a prior leak could have stubbed it)
        import shared as _shared
        self.addCleanup(lambda: (setattr(mod, "claim_task", _shared.claim_task),
                                 setattr(mod, "renew_claim", _shared.renew_claim),
                                 setattr(mod, "finalize_task", _shared.finalize_task)))

    def test_stitch_artifacts_are_attempt_scoped(self):
        """Finding 2: the published png/manifest live under an owner-scoped
        prefix, so a stale same-generation stitch writing its own attempt
        cannot overwrite the winner's supposedly-immutable objects."""
        import handler_poly_sheet as mod

        gen = "g8a8a8a8a8a8a"
        stub = _S3Stub()
        self._patched(mod, stub)
        mod.claim_task = lambda *a, **k: True
        mod.renew_claim = lambda *a, **k: True
        mod.finalize_task = lambda *a, **k: True
        p = _run_params("attempt-sheet")
        _admit(mod, stub, p, gen, [[0, 1], [2, 3]])
        for w, frames in ((0, [0, 1]), (1, [2, 3])):
            mod.handle_frames({**p, "action": "frames", "generation": gen,
                               "task_id": f"sheet_tiles_attempt-sheet_{gen}_w{w}",
                               "frame_indices": frames})
        resp = mod.handle_stitch({**p, "action": "stitch", "generation": gen,
                                  "task_id": f"sheet_stitch_attempt-sheet_{gen}"})
        manifest = json.loads(resp["body"])
        # the published keys are attempt-scoped: sheets/{id}/{gen}/{token}/...
        run = json.loads(stub.objects["sheets/attempt-sheet/run.json"])
        png_key = run["published_png_key"]
        self.assertTrue(png_key.startswith(f"sheets/attempt-sheet/{gen}/"), png_key)
        # one more path segment than the bare generation prefix (the attempt)
        tail = png_key[len(f"sheets/attempt-sheet/{gen}/"):]
        self.assertEqual(tail.count("/"), 1, png_key)
        self.assertTrue(tail.endswith("/sheet.png"))
        self.assertEqual(run["status"], "done")
        self.assertEqual(manifest["png_key"], png_key)

    def test_ddb_uncertain_ownership_does_not_fail_run(self):
        """Round-10 finding 2: if the ownership renew RAISES (DDB
        throttle/outage), ownership is UNKNOWN — the run must NOT be marked
        failed (a successor may own it) and the DDB task must NOT be
        finalized. Recovery is via lease expiry."""
        import handler_poly_sheet as mod

        gen = "g8b8b8b8b8b8b"
        stub = _S3Stub()
        self._patched(mod, stub)
        stub.put_object(Bucket="b", Key="sheets/uncertain-sheet/run.json",
                        Body=json.dumps({"generation": gen, "status": "running"}).encode())
        finalized = []
        mod.finalize_task = lambda *a, **k: finalized.append(k.get("status")) or True
        def raising_renew(*a, **k):
            raise RuntimeError("DDB throttled")
        mod.renew_claim = raising_renew
        # the original error propagates (fails loudly for logging)...
        with self.assertRaises(RuntimeError):
            mod._finalize_failure_or_exit(
                "j", "sheet_stitch_uncertain-sheet", "A", "uncertain-sheet", gen,
                RuntimeError("render failed"), phase_label="Stitch failed")
        # ...run.json was NOT marked failed, and the DDB task was NOT finalized
        self.assertEqual(
            json.loads(stub.objects["sheets/uncertain-sheet/run.json"])["status"],
            "running")
        self.assertEqual(finalized, [])


class TestRound9WriteFence(_LeaseIsoTestCase):
    """Round-9 finding 1: the write fence is a SYNCHRONOUS fail-closed
    owner renew, not an async heartbeat flag."""

    def test_owns_for_write_fails_closed_on_ddb_error(self):
        import handler_poly_sheet as mod
        import shared as _shared
        # round-9 finding 6: restore to the KNOWN-REAL impl, not a captured
        # current value that a prior leak could have stubbed
        self.addCleanup(lambda: setattr(mod, "renew_claim", _shared.renew_claim))
        # lost lease -> False (renew returned False)
        mod.renew_claim = lambda *a, **k: False
        self.assertFalse(mod._owns_for_write("j", "t", "A"))
        # DDB uncertainty (renew RAISED) -> False (fail closed), never True
        def boom(*a, **k):
            raise RuntimeError("DDB unreachable")
        mod.renew_claim = boom
        self.assertFalse(mod._owns_for_write("j", "t", "A"))
        # confirmed ownership -> True
        mod.renew_claim = lambda *a, **k: True
        self.assertTrue(mod._owns_for_write("j", "t", "A"))


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestRound9Fencing(_LeaseIsoTestCase):
    """Round-9 findings 1/3/8: fail-closed write fence in the worker, a
    cancel that lands before commit blocks the publish, losing attempt
    artifacts are reaped."""

    def _patched(self, mod, stub):
        import shared as _shared
        mod.s3 = stub
        mod.SWEEP_COEFFGEN = SWEEP_TEST
        mod.report_status = lambda *a, **k: None
        self.addCleanup(lambda: (setattr(mod, "claim_task", _shared.claim_task),
                                 setattr(mod, "renew_claim", _shared.renew_claim),
                                 setattr(mod, "finalize_task", _shared.finalize_task)))

    def test_worker_skips_tile_write_when_fence_fails_closed(self):
        """Finding 1: if the pre-write ownership renew RAISES (DDB
        uncertain), the worker must NOT write the tile — it fails closed."""
        import handler_poly_sheet as mod

        gen = "g9a9a9a9a9a9a"
        stub = _S3Stub()
        self._patched(mod, stub)
        mod.claim_task = lambda *a, **k: True
        mod.finalize_task = lambda *a, **k: True
        def uncertain_renew(*a, **k):
            raise RuntimeError("DDB throttled")
        mod.renew_claim = uncertain_renew   # the FENCE cannot confirm
        p = _run_params("fence-sheet")
        _admit(mod, stub, p, gen, [[0, 1], [2, 3]])
        resp = mod.handle_frames({**p, "action": "frames", "generation": gen,
                                  "task_id": f"sheet_tiles_fence-sheet_{gen}_w0",
                                  "frame_indices": [0, 1]})
        body = json.loads(resp["body"])
        self.assertTrue(body.get("lost_lease"))
        # NO tile was written — the fence failed closed before the S3 put
        self.assertFalse([k for k in stub.objects if "/tiles/" in k],
                         "a fence that could not confirm ownership still wrote a tile")

    def test_cancel_before_commit_blocks_publish(self):
        """Finding 3: a cancel marker that appears AFTER the stitch's
        startup check but before the commit must block publication."""
        import handler_poly_sheet as mod

        gen = "g9c9c9c9c9c9c"
        stub = _S3Stub()
        self._patched(mod, stub)
        mod.claim_task = lambda *a, **k: True
        mod.renew_claim = lambda *a, **k: True
        mod.finalize_task = lambda *a, **k: True
        p = _run_params("cancelrace-sheet")
        _admit(mod, stub, p, gen, [[0, 1], [2, 3]])
        for w, frames in ((0, [0, 1]), (1, [2, 3])):
            mod.handle_frames({**p, "action": "frames", "generation": gen,
                               "task_id": f"sheet_tiles_cancelrace-sheet_{gen}_w{w}",
                               "frame_indices": frames})
        # the cancel marker lands just before the stitch commits (the
        # startup check already passed at admission time)
        stub.put_object(Bucket="b", Key=mod._cancel_key("cancelrace-sheet", gen),
                        Body=b"1")
        resp = mod.handle_stitch({**p, "action": "stitch", "generation": gen,
                                  "task_id": f"sheet_stitch_cancelrace-sheet_{gen}"})
        body = json.loads(resp["body"])
        # the stitch did NOT publish; it honored the cancel
        self.assertTrue(body.get("cancelled"))
        run = json.loads(stub.objects["sheets/cancelrace-sheet/run.json"])
        self.assertEqual(run["status"], "cancelled")
        self.assertNotEqual(run.get("published_generation"), gen)

    def test_losing_attempt_artifacts_are_reaped(self):
        """Finding 8: after a successful publish, orphaned objects of any
        LOSING same-generation stitch attempt are pruned."""
        import handler_poly_sheet as mod

        gen = "g9d9d9d9d9d9d"
        stub = _S3Stub()
        self._patched(mod, stub)
        mod.claim_task = lambda *a, **k: True
        mod.renew_claim = lambda *a, **k: True
        mod.finalize_task = lambda *a, **k: True
        p = _run_params("gc-sheet")
        _admit(mod, stub, p, gen, [[0, 1], [2, 3]])
        for w, frames in ((0, [0, 1]), (1, [2, 3])):
            mod.handle_frames({**p, "action": "frames", "generation": gen,
                               "task_id": f"sheet_tiles_gc-sheet_{gen}_w{w}",
                               "frame_indices": frames})
        # plant a LOSING attempt's orphaned artifacts under the same gen
        loser = f"sheets/gc-sheet/{gen}/deadbeef0000/"
        stub.put_object(Bucket="b", Key=loser + "sheet.png", Body=b"stale")
        stub.put_object(Bucket="b", Key=loser + "sheet.json", Body=b"{}")
        resp = mod.handle_stitch({**p, "action": "stitch", "generation": gen,
                                  "task_id": f"sheet_stitch_gc-sheet_{gen}"})
        run = json.loads(stub.objects["sheets/gc-sheet/run.json"])
        winner = run["published_png_key"].rsplit("/", 1)[0] + "/"
        self.assertNotEqual(winner, loser)
        # the loser's objects are gone; the winner's survive
        self.assertNotIn(loser + "sheet.png", stub.objects)
        self.assertNotIn(loser + "sheet.json", stub.objects)
        self.assertIn(run["published_png_key"], stub.objects)


class TestRound9MarkTerminalRetry(_LeaseIsoTestCase):
    """Round-9 finding 5: _mark_run_terminal RETRIES the CAS so the
    terminal transition reliably lands (no ghost 'running' runs)."""

    def test_mark_run_terminal_retries_cas_conflict(self):
        import handler_poly_sheet as mod

        gen = "g9e9e9e9e9e9e"
        stub = _S3Stub()
        real = mod.s3
        self.addCleanup(lambda: setattr(mod, "s3", real))
        # a run.json that a CONCURRENT writer bumps once (stale ETag on the
        # first put), then the retry succeeds
        stub.put_object(Bucket="b", Key="sheets/retry-sheet/run.json",
                        Body=json.dumps({"generation": gen, "status": "running"}).encode())

        class _ConflictOnce(_S3Stub):
            def __init__(self, inner):
                self.__dict__ = inner.__dict__
                self._conflicts = 1
            def put_object(self, **kw):
                if "IfMatch" in kw and self._conflicts > 0:
                    self._conflicts -= 1
                    from botocore.exceptions import ClientError
                    raise ClientError({"Error": {"Code": "PreconditionFailed"}}, "PutObject")
                return super().put_object(**kw)
        mod.s3 = _ConflictOnce(stub)
        # round-15 finding 5: _mark_run_terminal returns (status, run) — assert
        # the STATUS, not the truthiness of the tuple (which passes for any
        # non-empty tuple, including (None, None))
        status, run = mod._mark_run_terminal("retry-sheet", gen, "failed")
        self.assertEqual(status, "failed")
        self.assertEqual(run.get("status"), "failed")
        self.assertEqual(json.loads(stub.objects["sheets/retry-sheet/run.json"])["status"],
                         "failed")


class TestRound9CancelRace(_LeaseIsoTestCase):
    """Round-10 findings 1/2: cancellation and publication resolve through
    the SAME run.json CAS; the loser reports the ACTUAL outcome, and an
    unconfirmable transition FAILS rather than reporting a false success."""

    def _patch_s3(self, stub):
        import handler_poly_sheet as mod
        real = mod.s3
        self.addCleanup(lambda: setattr(mod, "s3", real))
        mod.s3 = stub

    def test_commit_returns_cancelled_when_cancel_won_the_cas(self):
        import handler_poly_sheet as mod
        gen = "gc1c1c1c1c1c1"
        stub = _S3Stub()
        self._patch_s3(stub)
        # a cancel already CAS'd run.json to 'cancelled'
        stub.put_object(Bucket="b", Key="sheets/cr-sheet/run.json",
                        Body=json.dumps({"generation": gen,
                                         "status": "cancelled"}).encode())
        before = stub.objects["sheets/cr-sheet/run.json"]
        outcome, won = mod._commit_run_publication("cr-sheet", gen,
                                                   f"sheets/cr-sheet/{gen}/x/")
        # the publish LOST the race: reports 'cancelled', won=False, and does
        # NOT touch run.json (no 'done', no pointer)
        self.assertEqual(outcome, "cancelled")
        self.assertFalse(won)
        self.assertEqual(stub.objects["sheets/cr-sheet/run.json"], before)

    def test_cancel_reports_the_truth_when_publish_won(self):
        """The reviewer's reproduced interleaving: a publish reached 'done'
        first — cancel must NOT report a false success."""
        import handler_poly_sheet as mod
        gen = "gd2d2d2d2d2d2"
        stub = _S3Stub()
        self._patch_s3(stub)
        stub.put_object(Bucket="b", Key="sheets/pw-sheet/run.json",
                        Body=json.dumps({"generation": gen, "status": "done",
                                         "published_generation": gen}).encode())
        resp = mod.handle_cancel({"action": "cancel", "sheet_id": "pw-sheet",
                                  "generation": gen})
        body = json.loads(resp["body"])
        self.assertFalse(body["cancelled"])       # NOT a false cancel
        self.assertEqual(body["status"], "done")  # reports the real outcome

    def test_cancel_that_cannot_confirm_raises(self):
        """Round-10 finding 2: an unconfirmable terminal transition must
        FAIL (raise), never return a false HTTP-200 success."""
        import handler_poly_sheet as mod
        gen = "ge3e3e3e3e3e3"

        class _NoRun(_S3Stub):
            def get_object(self, Bucket, Key):
                from botocore.exceptions import ClientError
                raise ClientError({"Error": {"Code": "500"}}, "GetObject")
        stub = _NoRun()
        self._patch_s3(stub)
        with self.assertRaises(RuntimeError):
            mod.handle_cancel({"action": "cancel", "sheet_id": "x-sheet",
                               "generation": gen})


class TestRound10Fencing(_LeaseIsoTestCase):
    """Round-10 findings 1/4/5: an idempotent publisher never prunes the
    winner, tiles are write-once, and an ambiguous-commit prune re-reads
    run.json before deleting."""

    def _patch_s3(self, stub):
        import handler_poly_sheet as mod
        real = mod.s3
        self.addCleanup(lambda: setattr(mod, "s3", real))
        mod.s3 = stub

    def test_idempotent_publisher_does_not_touch_the_winner(self):
        """Finding 1: when ANOTHER attempt already published, our commit
        returns (done, False) and we prune ONLY our own prefix — the
        winner's pointer target survives (the reviewer's pointer_target
        _survives=False probe)."""
        import handler_poly_sheet as mod
        gen = "gf1f1f1f1f1f1"
        stub = _S3Stub()
        self._patch_s3(stub)
        a_prefix = f"sheets/win-sheet/{gen}/aaaaaaaaaaaa/"
        stub.put_object(Bucket="b", Key=a_prefix + "sheet.png", Body=b"WINNER")
        stub.put_object(Bucket="b", Key=a_prefix + "sheet.json", Body=b"{}")
        stub.put_object(Bucket="b", Key="sheets/win-sheet/run.json",
                        Body=json.dumps({"generation": gen, "status": "done",
                                         "published_generation": gen,
                                         "published_png_key": a_prefix + "sheet.png"}).encode())
        b_prefix = f"sheets/win-sheet/{gen}/bbbbbbbbbbbb/"
        stub.put_object(Bucket="b", Key=b_prefix + "sheet.png", Body=b"loser")
        stub.put_object(Bucket="b", Key=b_prefix + "sheet.json", Body=b"{}")
        outcome, won = mod._commit_run_publication("win-sheet", gen, b_prefix)
        self.assertEqual((outcome, won), ("done", False))
        # the not-won branch prunes ONLY its own prefix — never family-wide
        mod._prune_own_attempt("win-sheet", b_prefix)
        self.assertEqual(stub.objects[a_prefix + "sheet.png"], b"WINNER")
        self.assertNotIn(b_prefix + "sheet.png", stub.objects)

    def test_ambiguous_cas_that_is_ours_returns_won(self):
        """Round-12 finding 1: a network-ambiguous CAS — S3 applied the write
        but the SDK retried and observed 412 — re-reads to find OUR OWN
        publication. `won` must be decided by the POINTER (which is ours),
        not by which invocation saw the success, so the caller keeps its
        artifacts instead of deleting the winner it just wrote."""
        import handler_poly_sheet as mod
        gen = "gf2f2f2f2f2f2"
        stub = _S3Stub()
        self._patch_s3(stub)
        prefix = f"sheets/amb-win/{gen}/dddddddddddd/"
        stub.put_object(Bucket="b", Key=prefix + "sheet.png", Body=b"OURS")
        stub.put_object(Bucket="b", Key=prefix + "sheet.json", Body=b"{}")
        # run.json ALREADY points at OUR prefix (our CAS landed, response lost)
        stub.put_object(Bucket="b", Key="sheets/amb-win/run.json",
                        Body=json.dumps({"generation": gen, "status": "done",
                                         "published_generation": gen,
                                         "published_png_key": prefix + "sheet.png",
                                         "published_manifest_key": prefix + "sheet.json"}).encode())
        outcome, won = mod._commit_run_publication("amb-win", gen, prefix)
        self.assertEqual((outcome, won), ("done", True))   # OURS -> won

    def test_tiles_are_write_once(self):
        """Finding 4: create-only tile writes reject a delayed stale worker's
        overwrite (the object is immutable once written)."""
        import handler_poly_sheet as mod
        stub = _S3Stub()
        self._patch_s3(stub)
        key = "sheets/t/tiles/g/00000.bin"
        mod._put_object_once(key, b"first", "application/octet-stream")
        mod._put_object_once(key, b"STALE-OVERWRITE", "application/octet-stream")
        self.assertEqual(stub.objects[key], b"first")   # NOT overwritten

    def test_put_once_fails_closed_without_conditional_writes(self):
        """Round-12 finding 6: the write-once helper must FAIL CLOSED (not
        fall back to an unconditional overwrite) when the runtime lacks
        conditional writes."""
        import handler_poly_sheet as mod

        class _NoIfNoneMatch(_S3Stub):
            def put_object(self, **kw):
                if "IfNoneMatch" in kw:
                    raise TypeError("put_object() got an unexpected keyword 'IfNoneMatch'")
                return super().put_object(**kw)
        stub = _NoIfNoneMatch()
        self._patch_s3(stub)
        with self.assertRaises(RuntimeError) as ctx:
            mod._put_object_once("k", b"x", "application/octet-stream")
        self.assertIn("conditional writes", str(ctx.exception))
        self.assertNotIn("k", stub.objects)   # nothing written (no overwrite)

    def test_ambiguous_commit_keeps_artifacts_if_actually_published(self):
        """Finding 5: on an ambiguous commit, re-read run.json — if it shows
        OUR prefix published (the CAS actually succeeded), keep the objects."""
        import handler_poly_sheet as mod
        gen = "gf5f5f5f5f5f5"
        stub = _S3Stub()
        self._patch_s3(stub)
        prefix = f"sheets/amb-sheet/{gen}/zzzzzzzzzzzz/"
        stub.put_object(Bucket="b", Key=prefix + "sheet.png", Body=b"pixels")
        stub.put_object(Bucket="b", Key=prefix + "sheet.json", Body=b"{}")
        stub.put_object(Bucket="b", Key="sheets/amb-sheet/run.json",
                        Body=json.dumps({"generation": gen, "status": "done",
                                         "published_png_key": prefix + "sheet.png",
                                         "published_manifest_key": prefix + "sheet.json"}).encode())
        mod._prune_own_attempt_if_not_published("amb-sheet", gen, prefix)
        self.assertIn(prefix + "sheet.png", stub.objects)   # we actually won

    def test_ambiguous_commit_prunes_orphan_when_terminal_and_not_ours(self):
        """Finding 5: prune only when run.json is TERMINAL and references a
        DIFFERENT prefix — then our conditional CAS can no longer land."""
        import handler_poly_sheet as mod
        gen = "gf6f6f6f6f6f6"
        stub = _S3Stub()
        self._patch_s3(stub)
        prefix = f"sheets/amb2-sheet/{gen}/yyyyyyyyyyyy/"
        stub.put_object(Bucket="b", Key=prefix + "sheet.png", Body=b"orphan")
        stub.put_object(Bucket="b", Key=prefix + "sheet.json", Body=b"{}")
        stub.put_object(Bucket="b", Key="sheets/amb2-sheet/run.json",
                        Body=json.dumps({"generation": gen, "status": "done",
                                         "published_png_key":
                                         f"sheets/amb2-sheet/{gen}/other/sheet.png"}).encode())
        mod._prune_own_attempt_if_not_published("amb2-sheet", gen, prefix)
        self.assertNotIn(prefix + "sheet.png", stub.objects)   # pruned

    def test_ambiguous_commit_defers_while_running(self):
        """Round-12 finding 2: while run.json is still 'running' for this
        generation, a timed-out CAS could still publish these keys — so the
        cleanup must NOT delete them (defer to the winner's sweep). Deleting
        then would leave a dangling pointer when the late CAS lands."""
        import handler_poly_sheet as mod
        gen = "gf7f7f7f7f7f7"
        stub = _S3Stub()
        self._patch_s3(stub)
        prefix = f"sheets/amb3-sheet/{gen}/wwwwwwwwwwww/"
        stub.put_object(Bucket="b", Key=prefix + "sheet.png", Body=b"pending")
        stub.put_object(Bucket="b", Key=prefix + "sheet.json", Body=b"{}")
        stub.put_object(Bucket="b", Key="sheets/amb3-sheet/run.json",
                        Body=json.dumps({"generation": gen, "status": "running"}).encode())
        mod._prune_own_attempt_if_not_published("amb3-sheet", gen, prefix)
        # NOT deleted — the late CAS could still publish this prefix
        self.assertIn(prefix + "sheet.png", stub.objects)


class TestRound12TerminalStatusMapping(_LeaseIsoTestCase):
    """Round-12 finding 4: the DDB task is recorded at the ACTUAL resolved
    run status — a run that resolved to 'done' (a concurrent publish won) is
    a task SUCCESS, not a false 'error'."""

    def _patch(self, stub):
        import handler_poly_sheet as mod
        import shared as _shared
        real_s3 = mod.s3
        self.addCleanup(lambda: setattr(mod, "s3", real_s3))
        self.addCleanup(lambda: setattr(mod, "renew_claim", _shared.renew_claim))
        self.addCleanup(lambda: setattr(mod, "finalize_task", _shared.finalize_task))
        mod.s3 = stub

    def test_failure_after_concurrent_publish_finalizes_task_done(self):
        import handler_poly_sheet as mod
        gen = "gab1ab1ab1ab"
        stub = _S3Stub()
        self._patch(stub)
        # a concurrent attempt already published the run
        stub.put_object(Bucket="b", Key="sheets/pub-sheet/run.json",
                        Body=json.dumps({"generation": gen, "status": "done",
                                         "published_generation": gen}).encode())
        finals = []
        mod.renew_claim = lambda *a, **k: True   # we own the task
        mod.finalize_task = lambda *a, **k: finals.append(k.get("status")) or True
        # our processing hit an exception, but the run is already 'done'
        resp = mod._finalize_failure_or_exit(
            "j", "sheet_stitch_pub-sheet", "A", "pub-sheet", gen,
            RuntimeError("lost response"), phase_label="Stitch failed")
        # the DDB task is 'done' (NOT a false 'error'), and we do NOT raise
        self.assertEqual(finals, ["done"])
        self.assertEqual(json.loads(resp["body"])["status"], "done")
        # run.json stays 'done' (we did not overwrite it to 'failed')
        self.assertEqual(json.loads(stub.objects["sheets/pub-sheet/run.json"])["status"],
                         "done")

    def test_genuine_failure_finalizes_task_error_and_raises(self):
        import handler_poly_sheet as mod
        gen = "gcd2cd2cd2cd"
        stub = _S3Stub()
        self._patch(stub)
        stub.put_object(Bucket="b", Key="sheets/fail-sheet/run.json",
                        Body=json.dumps({"generation": gen, "status": "running"}).encode())
        finals = []
        mod.renew_claim = lambda *a, **k: True
        mod.finalize_task = lambda *a, **k: finals.append(k.get("status")) or True
        with self.assertRaises(RuntimeError):
            mod._finalize_failure_or_exit(
                "j", "sheet_stitch_fail-sheet", "A", "fail-sheet", gen,
                RuntimeError("render failed"), phase_label="Stitch failed")
        self.assertEqual(finals, ["error"])   # genuine failure -> task error
        self.assertEqual(json.loads(stub.objects["sheets/fail-sheet/run.json"])["status"],
                         "failed")


class TestRound15Cleanup(_LeaseIsoTestCase):
    """Round-15: ALL cleanup goes through the single _reap_terminal_generation
    owner — keyed off run.json, FAIL-CLOSED on superseded/unknown/running."""

    def _patch(self, stub):
        import handler_poly_sheet as mod
        import shared as _shared
        real_s3 = mod.s3
        self.addCleanup(lambda: setattr(mod, "s3", real_s3))
        self.addCleanup(lambda: setattr(mod, "renew_claim", _shared.renew_claim))
        self.addCleanup(lambda: setattr(mod, "finalize_task", _shared.finalize_task))
        mod.s3 = stub

    def _run(self, stub, sheet, run):
        stub.put_object(Bucket="b", Key=f"sheets/{sheet}/run.json",
                        Body=json.dumps(run).encode())

    def test_superseded_generation_is_never_reaped(self):
        """Finding 1 (release blocker): run.json is a NEWER generation that
        carried OUR generation's published winner forward. Reaping our (old)
        generation would delete the still-referenced winner — so
        _reap_terminal_generation for the old gen keyed off the NEW run.json
        MUST fail closed (generation mismatch -> skip)."""
        import handler_poly_sheet as mod
        old, new = "ga1a1a1a1a1a1", "gb2b2b2b2b2b2"
        stub = _S3Stub(); self._patch(stub)
        winner = f"sheets/sup/{old}/wwwwwwwwwwww/"   # winner under the OLD gen
        stub.put_object(Bucket="b", Key=winner + "sheet.png", Body=b"WIN")
        stub.put_object(Bucket="b", Key=winner + "sheet.json", Body=b"{}")
        stub.put_object(Bucket="b", Key=mod._tile_key("sup", old, 0), Body=b"tile")
        # run.json is the NEW generation, carrying the OLD winner forward
        self._run(stub, "sup", {"generation": new, "status": "running", "steps": 1,
                                "published_generation": old,
                                "published_png_key": winner + "sheet.png",
                                "published_manifest_key": winner + "sheet.json"})
        result = mod._reap_terminal_generation("sup", old)
        self.assertIsNone(result)                       # skipped (fail closed)
        self.assertEqual(stub.objects[winner + "sheet.png"], b"WIN")   # SURVIVES

    def test_failed_run_reaps_tiles_marker_and_all_attempts(self):
        """Finding 2/4: a same-generation failure reaps tiles + marker + ALL
        attempts through the owner."""
        import handler_poly_sheet as mod
        gen = "gc3c3c3c3c3c3"
        stub = _S3Stub(); self._patch(stub)
        att = f"sheets/failr/{gen}/tttttttttttt/"
        stub.put_object(Bucket="b", Key=att + "sheet.png", Body=b"orphan")
        stub.put_object(Bucket="b", Key=att + "sheet.json", Body=b"{}")
        stub.put_object(Bucket="b", Key=mod._tile_key("failr", gen, 0), Body=b"tile")
        stub.put_object(Bucket="b", Key=mod._cancel_key("failr", gen), Body=b"1")
        self._run(stub, "failr", {"generation": gen, "status": "failed", "steps": 1})
        mod._reap_terminal_generation("failr", gen)
        self.assertNotIn(att + "sheet.png", stub.objects)
        self.assertNotIn(mod._tile_key("failr", gen, 0), stub.objects)
        self.assertNotIn(mod._cancel_key("failr", gen), stub.objects)

    def test_done_reaps_losers_keeps_winner(self):
        import handler_poly_sheet as mod
        gen = "gd4d4d4d4d4d4"
        stub = _S3Stub(); self._patch(stub)
        winner = f"sheets/donr/{gen}/wwwwwwwwwwww/"
        loser = f"sheets/donr/{gen}/llllllllllll/"
        for pfx, body in ((winner, b"WIN"), (loser, b"lose")):
            stub.put_object(Bucket="b", Key=pfx + "sheet.png", Body=body)
            stub.put_object(Bucket="b", Key=pfx + "sheet.json", Body=b"{}")
        stub.put_object(Bucket="b", Key=mod._tile_key("donr", gen, 0), Body=b"tile")
        self._run(stub, "donr", {"generation": gen, "status": "done", "steps": 1,
                                 "published_generation": gen,
                                 "published_png_key": winner + "sheet.png",
                                 "published_manifest_key": winner + "sheet.json"})
        mod._reap_terminal_generation("donr", gen)
        self.assertEqual(stub.objects[winner + "sheet.png"], b"WIN")   # winner survives
        self.assertNotIn(loser + "sheet.png", stub.objects)            # loser reaped
        self.assertNotIn(mod._tile_key("donr", gen, 0), stub.objects)

    def test_done_unknown_winner_skips(self):
        import handler_poly_sheet as mod
        gen = "ge5e5e5e5e5e5"
        stub = _S3Stub(); self._patch(stub)
        winner = f"sheets/unk/{gen}/wwwwwwwwwwww/"
        stub.put_object(Bucket="b", Key=winner + "sheet.png", Body=b"WIN")
        stub.put_object(Bucket="b", Key=winner + "sheet.json", Body=b"{}")
        # only ONE published key -> winner not determinable
        self._run(stub, "unk", {"generation": gen, "status": "done", "steps": 1,
                                "published_generation": gen,
                                "published_png_key": winner + "sheet.png"})
        result = mod._reap_terminal_generation("unk", gen)
        self.assertIsNone(result)
        self.assertEqual(stub.objects[winner + "sheet.png"], b"WIN")   # SURVIVES

    def test_running_generation_is_never_reaped(self):
        import handler_poly_sheet as mod
        gen = "gf6f6f6f6f6f6"
        stub = _S3Stub(); self._patch(stub)
        stub.put_object(Bucket="b", Key=mod._tile_key("run", gen, 0), Body=b"tile")
        self._run(stub, "run", {"generation": gen, "status": "running", "steps": 1})
        self.assertIsNone(mod._reap_terminal_generation("run", gen))
        self.assertIn(mod._tile_key("run", gen, 0), stub.objects)      # kept

    def test_finalize_failure_reaps_through_the_owner(self):
        """End-to-end: _finalize_failure_or_exit marks the run failed and
        reaps through _reap_terminal_generation (no test-local closure)."""
        import handler_poly_sheet as mod
        gen = "g707070707070"
        stub = _S3Stub(); self._patch(stub)
        att = f"sheets/e2e/{gen}/tttttttttttt/"
        stub.put_object(Bucket="b", Key=att + "sheet.png", Body=b"orphan")
        stub.put_object(Bucket="b", Key=att + "sheet.json", Body=b"{}")
        stub.put_object(Bucket="b", Key=mod._tile_key("e2e", gen, 0), Body=b"tile")
        self._run(stub, "e2e", {"generation": gen, "status": "running", "steps": 1})
        mod.renew_claim = lambda *a, **k: True
        mod.finalize_task = lambda *a, **k: True
        with self.assertRaises(RuntimeError):
            mod._finalize_failure_or_exit(
                "j", "sheet_stitch_e2e", "A", "e2e", gen,
                RuntimeError("stitch failed"), phase_label="Stitch failed")
        self.assertEqual(json.loads(stub.objects["sheets/e2e/run.json"])["status"], "failed")
        self.assertNotIn(att + "sheet.png", stub.objects)              # reaped
        self.assertNotIn(mod._tile_key("e2e", gen, 0), stub.objects)

    def test_begin_supersede_reaps_prior_but_keeps_carried_winner(self):
        """Finding 2 (delayed GC): a superseding begin reaps the prior
        generation's stragglers but preserves the carried-forward winner."""
        import handler_poly_sheet as mod
        old = "g808080808080"
        stub = _S3Stub(); self._patch(stub)
        winner = f"sheets/gc/{old}/wwwwwwwwwwww/"
        loser = f"sheets/gc/{old}/llllllllllll/"
        for pfx, body in ((winner, b"WIN"), (loser, b"lose")):
            stub.put_object(Bucket="b", Key=pfx + "sheet.png", Body=body)
            stub.put_object(Bucket="b", Key=pfx + "sheet.json", Body=b"{}")
        stub.put_object(Bucket="b", Key=mod._tile_key("gc", old, 0), Body=b"tile")
        stub.put_object(Bucket="b", Key=mod._cancel_key("gc", old), Body=b"1")
        prior = {"generation": old, "status": "done", "steps": 1,
                 "published_generation": old,
                 "published_png_key": winner + "sheet.png",
                 "published_manifest_key": winner + "sheet.json"}
        mod._reap_superseded_generation("gc", prior)
        self.assertEqual(stub.objects[winner + "sheet.png"], b"WIN")   # carried winner survives
        self.assertNotIn(loser + "sheet.png", stub.objects)            # loser reaped
        self.assertNotIn(mod._tile_key("gc", old, 0), stub.objects)    # tile reaped
        self.assertNotIn(mod._cancel_key("gc", old), stub.objects)     # marker reaped

# Defined LAST so pytest (file-definition order) runs it AFTER every other
# test — round-9 finding 7: an empirical guard that NO test leaked a lease
# stub into the handler module (exactly the check the reviewer ran by hand).
class TestZZLeaseIsolationNoLeak(_LeaseIsoTestCase):
    def test_no_lease_stub_leaked_into_handler(self):
        import handler_poly_sheet as mod
        import shared
        self.assertIs(mod.claim_task, shared.claim_task,
                      "a test leaked a claim_task stub into handler_poly_sheet")
        self.assertIs(mod.renew_claim, shared.renew_claim,
                      "a test leaked a renew_claim stub into handler_poly_sheet")
        self.assertIs(mod.finalize_task, shared.finalize_task,
                      "a test leaked a finalize_task stub into handler_poly_sheet")
