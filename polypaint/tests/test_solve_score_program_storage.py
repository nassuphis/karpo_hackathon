import io
import json
import os
import sys
import unittest
from unittest.mock import patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class _FakeS3:
    def __init__(self):
        self.objects = {}
        self.metadata = {}

    def get_paginator(self, name):
        if name != "list_objects_v2":
            raise AssertionError(f"unexpected paginator {name}")
        return self

    def paginate(self, Bucket=None, Prefix=None, Delimiter=None):
        keys = sorted(key for key in self.objects if key.startswith(Prefix or ""))
        return [{"Contents": [{"Key": key} for key in keys]}]

    def get_object(self, Bucket=None, Key=None):
        if Key not in self.objects:
            raise Exception("NoSuchKey")
        return {"Body": io.BytesIO(self.objects[Key])}

    def put_object(self, Bucket=None, Key=None, Body=None, ContentType=None, Metadata=None):
        if isinstance(Body, bytes):
            data = Body
        else:
            data = str(Body or "").encode("utf-8")
        self.objects[Key] = data
        self.metadata[Key] = dict(Metadata or {})
        return {}

    def head_object(self, Bucket=None, Key=None):
        if Key not in self.objects:
            raise Exception("NoSuchKey")
        return {"Metadata": dict(self.metadata.get(Key) or {})}

    def delete_object(self, Bucket=None, Key=None):
        self.objects.pop(Key, None)
        self.metadata.pop(Key, None)
        return {}


class TestSolveScoreProgramStorage(unittest.TestCase):
    def _event(self, path, body):
        return {"path": path, "body": json.dumps(body)}

    @patch("handler_storage.s3")
    def test_storage_routes_round_trip_saved_program(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.paginate.side_effect = getattr(fake_s3, "paginate", None)
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object
        mock_s3.delete_object.side_effect = fake_s3.delete_object

        save_resp = handler_storage.handler(
            self._event(
                "/save-solve-score-program",
                {
                    "name": "Proximity q=0.1%",
                    "chain": [["proximity", "0.1"]],
                    "recommended_interpretation": "direct_rgb",
                },
            ),
            None,
        )
        self.assertEqual(save_resp["statusCode"], 200)
        save_body = json.loads(save_resp["body"])
        program_id = save_body["program"]["id"]
        self.assertFalse(save_body["overwritten"])
        self.assertEqual(save_body["program"]["chain"], [["proximity", "0.1"]])
        self.assertIn("source_text", save_body["program"])
        self.assertIn("metric(proximity", save_body["program"]["source_text"])
        self.assertTrue(save_body["program"]["fingerprint"].startswith("sha256:"))
        self.assertEqual(save_body["program"]["recommended_interpretation"], "rgb")

        list_resp = handler_storage.handler(self._event("/list-solve-score-programs", {}), None)
        list_body = json.loads(list_resp["body"])
        self.assertEqual(list_body["count"], 1)
        self.assertEqual(list_body["programs"][0]["id"], program_id)
        self.assertEqual(list_body["programs"][0]["statement_count"], 1)
        self.assertEqual(list_body["error_count"], 0)

        fetch_resp = handler_storage.handler(
            self._event("/fetch-solve-score-program", {"id": program_id}),
            None,
        )
        fetch_body = json.loads(fetch_resp["body"])
        self.assertEqual(fetch_body["program"]["id"], program_id)
        self.assertEqual(fetch_body["program"]["metric"], "proximity")
        self.assertIn("metric(proximity", fetch_body["program"]["source_text"])
        self.assertEqual(fetch_body["program"]["recommended_interpretation"], "rgb")

        delete_resp = handler_storage.handler(
            self._event("/delete-solve-score-program", {"id": program_id}),
            None,
        )
        delete_body = json.loads(delete_resp["body"])
        self.assertEqual(delete_body, {"id": program_id, "deleted": 1})

        refetch_resp = handler_storage.handler(
            self._event("/fetch-solve-score-program", {"id": program_id}),
            None,
        )
        self.assertEqual(refetch_resp["statusCode"], 404)

    @patch("handler_storage.s3")
    def test_list_excludes_v2_subdirectory_copies(self, mock_s3):
        # Regression: a Migrate-v2 copy under solve-score-programs/v2/<id>.json
        # must NOT surface as a phantom "v2/<id>" program (which then double-
        # prefixed into v2/v2/<id> on migrate/load).
        import handler_storage

        fake_s3 = _FakeS3()
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.paginate.side_effect = getattr(fake_s3, "paginate", None)
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object
        mock_s3.delete_object.side_effect = fake_s3.delete_object

        save_resp = handler_storage.handler(
            self._event("/save-solve-score-program", {"name": "hsv 3", "chain": [["proximity", "0.1"]]}),
            None,
        )
        program_id = json.loads(save_resp["body"])["program"]["id"]

        # Simulate the Migrate-v2 copy living under the v2/ subdirectory.
        v1_key = f"{handler_storage.SOLVE_SCORE_PROGRAMS_PREFIX}{program_id}.json"
        v2_key = f"{handler_storage.SOLVE_SCORE_PROGRAMS_PREFIX}v2/{program_id}.json"
        fake_s3.objects[v2_key] = fake_s3.objects[v1_key]
        fake_s3.metadata[v2_key] = dict(fake_s3.metadata.get(v1_key) or {})

        list_resp = handler_storage.handler(self._event("/list-solve-score-programs", {}), None)
        list_body = json.loads(list_resp["body"])
        ids = [p["id"] for p in list_body["programs"]]
        self.assertEqual(ids, [program_id])
        self.assertNotIn(f"v2/{program_id}", ids)
        self.assertTrue(all("/" not in pid for pid in ids))
        self.assertEqual(list_body["count"], 1)

    def test_program_key_builders_normalize_stray_v2_prefix(self):
        # A phantom id carrying a stray v2/ prefix must never double-prefix into
        # .../v2/v2/<id>.json on any program kind.
        import handler_storage as hs

        self.assertEqual(hs._solve_score_program_v2_key("v2/hsv-3"), hs._solve_score_program_v2_key("hsv-3"))
        self.assertEqual(hs._solve_score_program_key("v2/v2/hsv-3"), hs._solve_score_program_key("hsv-3"))
        self.assertNotIn("v2/v2", hs._solve_score_program_v2_key("v2/v2/hsv-3"))
        self.assertEqual(hs._param_program_v2_key("v2/foo"), hs._param_program_v2_key("foo"))
        self.assertEqual(hs._coeff_program_v2_key("v2/v2/bar"), hs._coeff_program_v2_key("bar"))
        # Flat ids are untouched.
        self.assertTrue(hs._solve_score_program_key("hsv-3").endswith("solve-score-programs/hsv-3.json"))
        self.assertTrue(hs._solve_score_program_v2_key("hsv-3").endswith("solve-score-programs/v2/hsv-3.json"))

    @patch("handler_storage.s3")
    def test_fetch_prefers_v2_copy_when_present(self, mock_s3):
        # Load must prefer the migrated v2/ copy (with its source_text) over v1,
        # and fall back to v1 when no v2 copy exists.
        import handler_storage

        fake_s3 = _FakeS3()
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.paginate.side_effect = getattr(fake_s3, "paginate", None)
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object
        mock_s3.delete_object.side_effect = fake_s3.delete_object

        save_resp = handler_storage.handler(
            self._event("/save-solve-score-program", {"name": "hsv 3", "chain": [["proximity", "0.1"]]}),
            None,
        )
        program_id = json.loads(save_resp["body"])["program"]["id"]

        # A migrated v2 copy with distinct, source-backed content (no
        # source_text_authoritative flag, mirroring translate_solve_score_from_old).
        v2_obj = {
            "program_kind": "solve_score_program",
            "version": 2,
            "spec_version": 2,
            "id": program_id,
            "name": "hsv 3",
            "source_text": "score = metric(area, slv, q=0.5%)\n",
            "chain": [["area", "0.5"]],
        }
        v2_key = f"{handler_storage.SOLVE_SCORE_PROGRAMS_PREFIX}v2/{program_id}.json"
        fake_s3.objects[v2_key] = (json.dumps(v2_obj) + "\n").encode("utf-8")
        fake_s3.metadata[v2_key] = {}

        prog = json.loads(
            handler_storage.handler(self._event("/fetch-solve-score-program", {"id": program_id}), None)["body"]
        )["program"]
        self.assertIn("area", prog["source_text"])  # v2 source surfaced (forced authoritative)
        self.assertNotIn("proximity", prog["source_text"])

        # Without the v2 copy, fetch falls back to v1.
        del fake_s3.objects[v2_key]
        prog2 = json.loads(
            handler_storage.handler(self._event("/fetch-solve-score-program", {"id": program_id}), None)["body"]
        )["program"]
        self.assertIn("proximity", prog2["source_text"])
        self.assertNotIn("area", prog2["source_text"])

    @patch("handler_storage.s3")
    def test_compile_solve_score_program_source_route_and_source_wins_save(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object

        source_text = "score = omega_cosine(metric(crowding, slv, q=1%), 4)"
        compile_resp = handler_storage.handler(
            self._event("/compile-solve-score-program-source", {"source_text": source_text}),
            None,
        )
        self.assertEqual(compile_resp["statusCode"], 200)
        compile_body = json.loads(compile_resp["body"])
        self.assertTrue(compile_body["ok"])
        self.assertEqual(compile_body["chain"], [["crowding", "1"], ["omega_cosine", "4"]])
        self.assertEqual(compile_body["program_spec"], "m0-0;omega_cosine:4")

        save_resp = handler_storage.handler(
            self._event(
                "/save-solve-score-program",
                {
                    "name": "Source Wins",
                    "source_text": source_text,
                    "chain": [["proximity", "0.1"]],
                },
            ),
            None,
        )
        self.assertEqual(save_resp["statusCode"], 200)
        program = json.loads(save_resp["body"])["program"]
        self.assertEqual(program["chain"], [["crowding", "1"], ["omega_cosine", "4"]])
        self.assertEqual(program["source_text"], source_text)

    def test_solve_score_chain_to_source_route_returns_compiled_contract(self):
        import handler_storage

        resp = handler_storage.handler(
            self._event(
                "/solve-score-chain-to-source",
                {"chain": [["proximity", "slv", "0.1"], ["emit", "norm"]]},
            ),
            None,
        )
        self.assertEqual(resp["statusCode"], 200)
        body = json.loads(resp["body"])
        self.assertTrue(body["ok"])
        self.assertIn("metric(proximity", body["source_text"])
        self.assertEqual(body["chain"], [["proximity", "0.1"], ["emit", "norm"]])
        self.assertEqual(body["program_spec"], "m0-0;emit_norm")
        self.assertEqual(body["output_channel_count"], 1)
        self.assertEqual(body["output_channels"][0]["mode"], "norm")
        self.assertEqual(body["output_channels"][0]["emit"], "emit_norm")
        self.assertTrue(body["has_explicit_outputs"])
        self.assertTrue(body["fingerprint"].startswith("sha256:"))

    def test_solve_score_chain_to_source_route_rejects_empty_or_bad_chain(self):
        import handler_storage

        for payload in ({}, {"chain": []}, {"chain": ""}, {"chain": [["not_a_metric", "0.1"]]}):
            resp = handler_storage.handler(self._event("/solve-score-chain-to-source", payload), None)
            self.assertEqual(resp["statusCode"], 400, payload)
            body = json.loads(resp["body"])
            self.assertFalse(body["ok"])
            self.assertIn("error", body)

    def test_solve_score_chain_to_source_route_propagates_roundtrip_code(self):
        import handler_storage
        from solve_score_program_source import SolveScoreProgramSourceError

        with patch(
            "handler_storage.solve_score_source_text_from_chain",
            side_effect=SolveScoreProgramSourceError("forced roundtrip failure", code="source_roundtrip_failed"),
        ):
            resp = handler_storage.handler(
                self._event("/solve-score-chain-to-source", {"chain": [["proximity", "slv", "0.1"]]}),
                None,
            )
        self.assertEqual(resp["statusCode"], 400)
        body = json.loads(resp["body"])
        self.assertEqual(body["code"], "source_roundtrip_failed")

    def test_render_artifact_entry_reconstructs_source_for_chain_only_metadata(self):
        import handler_storage

        entry = handler_storage._render_artifact_entry(
            "color",
            "artifact-1",
            {
                "key": "renders/job/artifact-1.png",
                "url": "https://example.invalid/artifact-1.png",
                "modified_at": "2026-06-22T00:00:00Z",
                "user_meta": {
                    "solve_score_chain": json.dumps([["spread", "cf", "0.5"], ["emit", "norm"]]),
                },
            },
        )
        self.assertIn("metric(spread", entry["solve_score_program_source_text"])
        self.assertIn("cf", entry["solve_score_program_source_text"])
        self.assertEqual(entry["score_source_text"], entry["solve_score_program_source_text"])

    def test_render_artifact_entry_reconstructs_palette_and_associated_sources(self):
        import handler_storage

        entry = handler_storage._render_artifact_entry(
            "color",
            "artifact-2",
            {
                "key": "renders/job/artifact-2.png",
                "url": "https://example.invalid/artifact-2.png",
                "modified_at": "2026-06-22T00:00:00Z",
                "user_meta": {
                    "palette_source_score_chain": json.dumps([["spread", "cf", "0.5"], ["emit", "norm"]]),
                    "associated_palette_score_chain": json.dumps([["crowding", "slv", "0.5"], ["emit", "norm"]]),
                },
            },
        )
        self.assertIn("metric(spread", entry["palette_source_solve_score_program_source_text"])
        self.assertEqual(
            entry["palette_source_score_source_text"],
            entry["palette_source_solve_score_program_source_text"],
        )
        self.assertIn("metric(crowding", entry["associated_palette_solve_score_program_source_text"])
        self.assertEqual(
            entry["associated_palette_score_source_text"],
            entry["associated_palette_solve_score_program_source_text"],
        )

    @patch("handler_storage.s3")
    def test_generic_metric_chip_round_trips_as_saved_program(self, mock_s3):
        import handler_storage
        from solve_score_chain import compile_solve_score_chain, solve_score_chain_id

        fake_s3 = _FakeS3()
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object

        generic_chain = [["metric", "angular_entropy_16", "cf", "0.5"], ["metric", "angular_entropy_16", "cf-1", "0.5"], ["abs_diff"]]
        canonical_generic_chain = [["metric", "angular_entropy_16", "cf", "0.5"], ["metric", "angular_entropy_16", "cf-1", "0.5"], "abs_diff"]
        concrete_chain = [["angular_entropy_16", "cf", "0.5"], ["angular_entropy_16", "cf-1", "0.5"], ["abs_diff"]]
        compiled = compile_solve_score_chain(generic_chain)
        self.assertEqual(compiled["program_spec"], "m0-0;m0-1;abs_diff")
        self.assertEqual(compiled["metrics"][0]["metric"], "angular_entropy_16")
        self.assertEqual(compiled["metrics"][0]["source"], "cf")
        self.assertEqual(solve_score_chain_id(generic_chain), solve_score_chain_id(concrete_chain))

        save_resp = handler_storage.handler(
            self._event(
                "/save-solve-score-program",
                {"name": "Generic Metric Program", "chain": generic_chain},
            ),
            None,
        )
        self.assertEqual(save_resp["statusCode"], 200)
        save_body = json.loads(save_resp["body"])
        self.assertEqual(save_body["program"]["chain"], canonical_generic_chain)
        self.assertEqual(save_body["program"]["metric"], "angular_entropy_16")
        self.assertEqual(save_body["program"]["program_spec"], "m0-0;m0-1;abs_diff")

        stored = json.loads(fake_s3.objects["polypaint/solve-score-programs/generic-metric-program.json"].decode("utf-8"))
        self.assertEqual(stored["chain"], canonical_generic_chain)

        fetch_resp = handler_storage.handler(
            self._event("/fetch-solve-score-program", {"id": "generic-metric-program"}),
            None,
        )
        fetch_body = json.loads(fetch_resp["body"])
        self.assertEqual(fetch_body["program"]["chain"], canonical_generic_chain)

    @patch("handler_storage.s3")
    def test_generic_metric_chip_rejects_pm_source_and_pm_only_metric(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object

        for chain, expected in [
            ([["metric", "angular_entropy_16", "pm", "0.5"]], "source must be one of slv, cf"),
            ([["metric", "t1_abs", "slv", "0.5"]], "supports both slv and cf"),
        ]:
            resp = handler_storage.handler(
                self._event(
                    "/save-solve-score-program",
                    {"name": "Bad Generic Metric", "chain": chain},
                ),
                None,
            )
            self.assertEqual(resp["statusCode"], 400)
            body = json.loads(resp["body"])
            self.assertIn(expected, body["error"])
        self.assertEqual(fake_s3.objects, {})

    @patch("handler_storage.s3")
    def test_client_supplied_id_and_derived_fields_are_ignored(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object

        resp = handler_storage.handler(
            self._event(
                "/save-solve-score-program",
                {
                    "id": "attacker-slug",
                    "version": 999,
                    "name": "My Safe Name",
                    "chain": [["proximity", "0.1"]],
                    "metric": "clusteriness",
                    "display": "bad",
                    "program_spec": "bad",
                    "statement_count": 999,
                    "saved_at": "1999-01-01T00:00:00Z",
                },
            ),
            None,
        )
        body = json.loads(resp["body"])
        self.assertEqual(resp["statusCode"], 200)
        self.assertEqual(body["program"]["id"], "my-safe-name")
        self.assertEqual(body["program"]["version"], 1)
        self.assertEqual(body["program"]["metric"], "proximity")
        stored = json.loads(fake_s3.objects["polypaint/solve-score-programs/my-safe-name.json"].decode("utf-8"))
        self.assertEqual(stored["id"], "my-safe-name")
        self.assertEqual(stored["version"], 1)
        self.assertEqual(stored["metric"], "proximity")

    @patch("handler_storage.s3")
    def test_save_compile_error_returns_400_and_writes_nothing(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object

        resp = handler_storage.handler(
            self._event(
                "/save-solve-score-program",
                {"name": "Broken Program", "chain": [["weighted_sum", "1", "2"]]},
            ),
            None,
        )
        self.assertEqual(resp["statusCode"], 400)
        body = json.loads(resp["body"])
        self.assertIn("error", body)
        self.assertEqual(fake_s3.objects, {})

    @patch("handler_storage.s3")
    def test_slug_collision_overwrites_same_saved_program_key(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object

        first = handler_storage.handler(
            self._event(
                "/save-solve-score-program",
                {"name": "My Prog!", "chain": [["proximity", "0.1"]]},
            ),
            None,
        )
        first_body = json.loads(first["body"])
        program_id = first_body["program"]["id"]
        self.assertFalse(first_body["overwritten"])

        second = handler_storage.handler(
            self._event(
                "/save-solve-score-program",
                {"name": "my prog?", "chain": [["proximity", "0.2"]]},
            ),
            None,
        )
        second_body = json.loads(second["body"])
        self.assertTrue(second_body["overwritten"])
        self.assertEqual(second_body["program"]["id"], program_id)

        fetch = handler_storage.handler(
            self._event("/fetch-solve-score-program", {"id": program_id}),
            None,
        )
        fetch_body = json.loads(fetch["body"])
        self.assertEqual(fetch_body["program"]["name"], "my prog?")
        self.assertEqual(fetch_body["program"]["chain"], [["proximity", "0.2"]])

    @patch("handler_storage.s3")
    def test_list_uses_object_metadata_and_surfaces_corrupt_entries(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        good_key = "polypaint/solve-score-programs/good.json"
        fake_s3.put_object(
            Key=good_key,
            Body=json.dumps({
                "version": 1,
                "id": "good",
                "name": "Good",
                "chain": [["proximity", "0.1"]],
                "metric": "proximity",
                "display": "proximity(slv,0.1)",
                "program_spec": "m0",
                "statement_count": 1,
                "saved_at": "2026-04-20T12:00:00Z",
            }).encode("utf-8"),
            Metadata={
                handler_storage.SOLVE_SCORE_PROGRAM_META_NAME: "Good",
                handler_storage.SOLVE_SCORE_PROGRAM_META_STATEMENT_COUNT: "1",
                handler_storage.SOLVE_SCORE_PROGRAM_META_SAVED_AT: "2026-04-20T12:00:00Z",
            },
        )
        bad_key = "polypaint/solve-score-programs/bad.json"
        fake_s3.put_object(Key=bad_key, Body=b"{not-json}", Metadata={})

        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object

        resp = handler_storage.handler(self._event("/list-solve-score-programs", {}), None)
        body = json.loads(resp["body"])
        self.assertEqual(body["count"], 1)
        self.assertEqual(body["programs"][0]["id"], "good")
        self.assertEqual(body["error_count"], 1)
        self.assertEqual(body["errors"][0]["id"], "bad")

    @patch("handler_storage.s3")
    def test_validation_limits_and_missing_ids(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object
        mock_s3.delete_object.side_effect = fake_s3.delete_object

        too_long_name = handler_storage.handler(
            self._event("/save-solve-score-program", {
                "name": "x" * 121,
                "chain": [["proximity", "0.1"]],
            }),
            None,
        )
        self.assertEqual(too_long_name["statusCode"], 400)

        too_many_statements = handler_storage.handler(
            self._event("/save-solve-score-program", {
                "name": "Many",
                "chain": [["proximity", "0.1"]] * 257,
            }),
            None,
        )
        self.assertEqual(too_many_statements["statusCode"], 400)

        too_long_token = handler_storage.handler(
            self._event("/save-solve-score-program", {
                "name": "Token",
                "chain": [["proximity", "0.1"], ["sawtooth", "x" * 129]],
            }),
            None,
        )
        self.assertEqual(too_long_token["statusCode"], 400)

        oversized_chain = handler_storage.handler(
            self._event("/save-solve-score-program", {
                "name": "Large",
                "chain": [["proximity", "0.1"]] + [["sawtooth", "1" * 120]] * 150,
            }),
            None,
        )
        self.assertEqual(oversized_chain["statusCode"], 400)

        fetch_missing = handler_storage.handler(
            self._event("/fetch-solve-score-program", {"id": "missing"}),
            None,
        )
        self.assertEqual(fetch_missing["statusCode"], 404)

        delete_missing = handler_storage.handler(
            self._event("/delete-solve-score-program", {"id": "missing"}),
            None,
        )
        self.assertEqual(delete_missing["statusCode"], 404)

        empty_name = handler_storage.handler(
            self._event("/save-solve-score-program", {
                "name": "   ",
                "chain": [["proximity", "0.1"]],
            }),
            None,
        )
        self.assertEqual(empty_name["statusCode"], 400)

    @patch("handler_storage.s3")
    def test_slug_edges_use_fallback_and_truncate(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object

        punctuation_only = handler_storage.handler(
            self._event("/save-solve-score-program", {
                "name": "!!!",
                "chain": [["proximity", "0.1"]],
            }),
            None,
        )
        punctuation_body = json.loads(punctuation_only["body"])
        self.assertEqual(punctuation_body["program"]["id"], "solve-score-program")

        long_name = "a" * 120
        long_resp = handler_storage.handler(
            self._event("/save-solve-score-program", {
                "name": long_name,
                "chain": [["proximity", "0.1"]],
            }),
            None,
        )
        long_body = json.loads(long_resp["body"])
        self.assertEqual(len(long_body["program"]["id"]), 64)


if __name__ == "__main__":
    unittest.main()
