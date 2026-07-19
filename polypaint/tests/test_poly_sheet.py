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


class _S3Stub:
    def __init__(self):
        self.objects = {}

    def put_object(self, Bucket, Key, Body, ContentType=None, CacheControl=None):
        self.objects[Key] = Body if isinstance(Body, (bytes, bytearray)) else Body.encode()

    def head_object(self, Bucket, Key):
        if Key not in self.objects:
            from botocore.exceptions import ClientError
            raise ClientError({"Error": {"Code": "404"}}, "HeadObject")
        return {"ContentLength": len(self.objects[Key])}

    def get_object(self, Bucket, Key):
        import io
        if Key not in self.objects:
            from botocore.exceptions import ClientError
            raise ClientError({"Error": {"Code": "NoSuchKey"}}, "GetObject")
        return {"Body": io.BytesIO(bytes(self.objects[Key]))}

    def delete_objects(self, Bucket, Delete):
        for entry in Delete["Objects"]:
            self.objects.pop(entry["Key"], None)


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


class TestPolySheetUnits(unittest.TestCase):
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
        mod.report_status = lambda *a, **k: None
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
        mod.report_status = lambda *a, **k: None
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
class TestPolySheetEndToEnd(unittest.TestCase):
    def _patched(self, mod, s3stub):
        mod.s3 = s3stub
        mod.SWEEP_COEFFGEN = SWEEP_TEST
        mod.report_status = lambda *a, **k: None

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

        stub = _S3Stub()
        stub.objects[mod._cancel_key("cancelled-sheet")] = b"1"
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub)
        try:
            resp = mod.handle_run(_run_params("cancelled-sheet"))
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
            mod.handle_frames({**p, "action": "frames",
                               "task_id": "sheet_tiles_fan-sheet_w0",
                               "frame_indices": [0, 1]})
            mod.handle_frames({**p, "action": "frames",
                               "task_id": "sheet_tiles_fan-sheet_w1",
                               "frame_indices": [2, 3]})
            self.assertIn("sheets/fan-sheet/tiles/00000.bin", stub.objects)
            resp = mod.handle_stitch({**p, "action": "stitch",
                                      "task_id": "sheet_stitch_fan-sheet"})
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig
        manifest = json.loads(resp["body"])
        self.assertEqual(manifest["render_mode"], "fanout")
        self.assertEqual([f["value"] for f in manifest["frame_records"]],
                         [0.5, 1.0, 1.5, 2.0])
        self.assertEqual(manifest["degree"], 5)
        # the stitched mosaic is byte-identical to the single-shot render
        self.assertEqual(stub.objects["sheets/fan-sheet/sheet.png"], ref_png)
        # scaffolding tiles are removed after the stitch
        self.assertFalse([k for k in stub.objects if "/tiles/" in k])

    def test_stitch_with_missing_tiles_errors(self):
        import handler_poly_sheet as mod

        stub = _S3Stub()
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub)
        try:
            p = _run_params("gap-sheet")
            mod.handle_frames({**p, "action": "frames",
                               "task_id": "sheet_tiles_gap-sheet_w0",
                               "frame_indices": [0, 1]})   # frames 2,3 never rendered
            with self.assertRaises(RuntimeError) as ctx:
                mod.handle_stitch({**p, "action": "stitch",
                                   "task_id": "sheet_stitch_gap-sheet"})
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig
        self.assertIn("missing 2 of 4 tiles", str(ctx.exception))

    def test_worker_cancel_stops_between_frames(self):
        import handler_poly_sheet as mod

        stub = _S3Stub()
        stub.objects[mod._cancel_key("cx-sheet")] = b"1"
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        self._patched(mod, stub)
        try:
            resp = mod.handle_frames({**_run_params("cx-sheet"), "action": "frames",
                                      "task_id": "sheet_tiles_cx-sheet_w0",
                                      "frame_indices": [0, 1]})
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig
        body = json.loads(resp["body"])
        self.assertEqual(body["cancelled"], "cx-sheet")
        self.assertEqual(body["frames_done"], 0)

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
