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
    stub.objects[f"sheets/{sheet_id}/run.json"] = json.dumps(run).encode()
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
        # the stitched mosaic is byte-identical to the single-shot render
        self.assertEqual(stub.objects["sheets/fan-sheet/sheet.png"], ref_png)
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
        self.assertEqual(body["cancelled"], "cx-sheet")
        self.assertEqual(body["frames_done"], 0)

    def test_worker_without_generation_writes_terminal_error(self):
        """CR35-F5: a worker that fails validation must still leave a
        terminal error row (never a silent disappearance)."""
        import handler_poly_sheet as mod

        rows = []
        orig = (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status)
        mod.s3 = _S3Stub()
        mod.SWEEP_COEFFGEN = SWEEP_TEST
        mod.report_status = lambda job, task, status, *a, **k: rows.append(status)
        try:
            with self.assertRaises(RuntimeError):
                mod.handle_frames({**_run_params("nogen-sheet"),
                                   "action": "frames",
                                   "task_id": "sheet_tiles_nogen_w0",
                                   "frame_indices": [0]})
        finally:
            (mod.s3, mod.SWEEP_COEFFGEN, mod.report_status) = orig
        self.assertIn("error", rows)

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
        mod.report_status = lambda *a, **k: None
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
class TestAdmissionEnforcement(unittest.TestCase):
    """Review round-2 finding 4/7: frames/stitch must BIND to the
    admitted run — generation, task allocation, payload hash — and an
    older stitch must not overwrite a newer generation's sheet."""

    def _patched(self, mod, s3stub):
        mod.s3 = s3stub
        mod.SWEEP_COEFFGEN = SWEEP_TEST
        mod.report_status = lambda *a, **k: None

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
