import io
import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

IMMUTABLE = "public, max-age=31536000, immutable"


def _tile(created, job, aid, key, palette=False):
    t = {"created_at": created, "job_id": job, "key": key,
         "artifact_id": aid, "function": "f", "degree": 5, "N": 100}
    if palette:
        t["palette_id"] = aid
    return t


class WallOrderTests(unittest.TestCase):
    def test_default_order_matches_js_created_branch(self):
        import handler_wall_pyramid as mod

        tiles = [
            _tile("2026-01-01T00:00:00Z", "jB", "a1", "k1"),
            _tile("2026-03-01T00:00:00Z", "jZ", "a9", "k2"),
            _tile("2026-03-01T00:00:00Z", "jA", "a5", "k3"),
            _tile("2026-03-01T00:00:00Z", "jA", "a2", "k4"),
        ]
        ordered = mod.default_wall_order(tiles)
        # created_at desc first; ties: job_id asc, then artifact id asc
        self.assertEqual([t["key"] for t in ordered], ["k4", "k3", "k2", "k1"])

    def test_palette_id_wins_tiebreak(self):
        import handler_wall_pyramid as mod

        tiles = [
            _tile("2026-01-01T00:00:00Z", "j", "zz", "k1", palette=True),
            _tile("2026-01-01T00:00:00Z", "j", "aa", "k2", palette=True),
        ]
        ordered = mod.default_wall_order(tiles)
        self.assertEqual([t["key"] for t in ordered], ["k2", "k1"])

    def test_placeholder_png_is_valid_512(self):
        import handler_wall_pyramid as mod

        png = mod._placeholder_png()
        self.assertEqual(png[:8], b"\x89PNG\r\n\x1a\n")
        self.assertEqual(int.from_bytes(png[16:20], "big"), 512)
        self.assertEqual(int.from_bytes(png[20:24], "big"), 512)


class BuildWallPyramidTests(unittest.TestCase):
    def _run(self, *, fail_download=False, fail_subprocess=False):
        import handler_wall_pyramid as mod

        manifest = {"tiles": [
            _tile("2026-02-01T00:00:00Z", "j1", "a1", "renders/j1/color/a1/preview.jpg"),
            _tile("2026-01-01T00:00:00Z", "j2", "a2", "renders/j2/color/a2/preview.jpg"),
        ]}
        puts = []
        ddb_updates = []

        fake_s3 = MagicMock()
        fake_s3.get_object.return_value = {
            "Body": io.BytesIO(json.dumps(manifest).encode())}

        def download_file(bucket, key, path):
            if fail_download and key.endswith("a2/preview.jpg"):
                raise RuntimeError("boom")
            with open(path, "wb") as fh:
                fh.write(b"jpgbytes")

        fake_s3.download_file.side_effect = download_file
        fake_s3.put_object.side_effect = lambda **kw: puts.append(kw)

        fake_ddb = MagicMock()
        fake_ddb.update_item.side_effect = lambda **kw: ddb_updates.append(kw)
        fake_ddb.exceptions.ConditionalCheckFailedException = type("CCF", (Exception,), {})

        def fake_run(cmd, capture_output=False, text=False, timeout=None, env=None):
            if fail_subprocess:
                return MagicMock(returncode=1, stdout="", stderr="vips exploded")
            listfile, cols, outbase = cmd[1], cmd[2], cmd[3]
            lines = [l for l in open(listfile).read().splitlines() if l]
            assert cols == "2"  # ceil(sqrt(2))
            os.makedirs(outbase + "_files/0", exist_ok=True)
            with open(outbase + ".dzi", "w") as fh:
                fh.write("<dzi/>")
            with open(outbase + "_files/0/0_0.jpg", "wb") as fh:
                fh.write(b"tile")
            return MagicMock(returncode=0, stdout=json.dumps(
                {"width": 1024, "height": 512, "count": len(lines), "across": 2}), stderr="")

        with patch.object(mod, "s3", fake_s3), \
             patch.object(mod, "boto3") as fake_boto3, \
             patch.object(mod.subprocess, "run", side_effect=fake_run):
            fake_boto3.client.return_value = fake_ddb
            resp = mod.handle_build_wall_pyramid({
                "kind": "color", "refresh_id": "mosaic_x",
                "manifest_key": "renders/_index/color_mosaic/mosaic_x/all.json"})
        return json.loads(resp["body"]), puts, ddb_updates

    def test_success_uploads_pyramid_and_wall_json(self):
        body, puts, ddb_updates = self._run()

        dzi = next(p for p in puts if p["Key"].endswith("wall.dzi"))
        self.assertEqual(dzi["ContentType"], "application/xml")
        self.assertEqual(dzi["CacheControl"], IMMUTABLE)
        tile = next(p for p in puts if "wall_files/" in p["Key"])
        self.assertEqual(tile["ContentType"], "image/jpeg")
        self.assertEqual(tile["CacheControl"], IMMUTABLE)

        wall_put = next(p for p in puts if p["Key"].endswith("wall.json"))
        self.assertEqual(wall_put["CacheControl"], "no-cache, max-age=0")
        wall = json.loads(wall_put["Body"])
        self.assertEqual(wall["cols"], 2)
        self.assertEqual(wall["rows"], 1)
        self.assertEqual(wall["cell_px"], 512)
        self.assertEqual(wall["sort"], "created")
        self.assertEqual(wall["width"], 1024)
        # baked order: newest first
        self.assertEqual([t["artifact_id"] for t in wall["tiles"]], ["a1", "a2"])

        update = ddb_updates[-1]
        self.assertEqual(update["ConditionExpression"], "refresh_id = :rid")
        self.assertEqual(update["ExpressionAttributeValues"][":ws"], {"S": "ready"})
        self.assertEqual(update["ExpressionAttributeValues"][":rid"], {"S": "mosaic_x"})
        self.assertEqual(body["placeholders"], 0)

    def test_download_failure_becomes_placeholder(self):
        body, puts, _ = self._run(fail_download=True)
        self.assertEqual(body["placeholders"], 1)
        wall = json.loads(next(p for p in puts if p["Key"].endswith("wall.json"))["Body"])
        self.assertEqual(wall["placeholders"], 1)

    def test_subprocess_failure_marks_wall_error(self):
        body, puts, ddb_updates = self._run(fail_subprocess=True)
        self.assertIn("wall_dz failed", body["error"])
        self.assertEqual(ddb_updates[-1]["ExpressionAttributeValues"][":ws"], {"S": "error"})
        self.assertFalse(any(p["Key"].endswith("wall.json") for p in puts))


if __name__ == "__main__":
    unittest.main()
