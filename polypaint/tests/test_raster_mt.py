import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _encode_fragment_pairs(pairs):
    payload = bytearray()
    for pixel_idx, score in pairs:
        payload.extend(int(pixel_idx).to_bytes(4, "little", signed=False))
        payload.append(int(score) & 0xFF)
    return bytes(payload)


def _bounds_from_center_scale(width, height, center_re, center_im, scale):
    half_w_world = (float(width) / 2.0) / float(scale)
    half_h_world = (float(height) / 2.0) / float(scale)
    return {
        "min_re": center_re - half_w_world,
        "max_re": center_re + half_w_world,
        "min_im": center_im - half_h_world,
        "max_im": center_im + half_h_world,
    }


def _clip_artifact(
    *,
    chain,
    metric,
    quantile,
    omega,
    omega_enabled,
    clip_lo,
    clip_hi,
    score_output_normalize=False,
    score_output_clip_lo=0.0,
    score_output_clip_hi=1.0,
):
    from solve_score_chain import compile_solve_score_chain_or_legacy, compiled_solve_score_fingerprint

    compiled = compile_solve_score_chain_or_legacy(
        chain,
        metric,
        quantile,
        omega,
        omega_enabled,
        default_metric=metric,
    )
    return {
        "family": "solve_score",
        "version": 2,
        "metric": str(compiled["metric"]),
        "clip_quantile": float(compiled["quantile"]),
        "omega": float(compiled["omega"]),
        "omega_enabled": bool(compiled["omega_enabled"]),
        "clip_lo": float(clip_lo),
        "clip_hi": float(clip_hi),
        "program": str(compiled["program_spec"]),
        "metrics": [
            {
                "slot": int(item["slot"]),
                "metric": str(item["metric"]),
                "source": str(item.get("source", "slv")),
                "quantile": float(item["quantile"]),
                "clip_lo": float(clip_lo),
                "clip_hi": float(clip_hi),
            }
            for item in (compiled.get("metrics") or [])
        ],
        "chain_fingerprint": compiled_solve_score_fingerprint(compiled),
        "score_output_normalize": bool(score_output_normalize),
        "score_output_clip_lo": float(score_output_clip_lo),
        "score_output_clip_hi": float(score_output_clip_hi),
    }


def _fused_event(**overrides):
    from logical_sections import build_solve_source_manifest

    solve_source_manifest = build_solve_source_manifest(
        [
            {
                "chunk_idx": 0,
                "bin_key": "renders/j/chunk_0.bin",
                "step_start": 0,
                "step_count": 2,
            },
            {
                "chunk_idx": 1,
                "bin_key": "renders/j/chunk_1.bin",
                "step_start": 2,
                "step_count": 2,
            },
        ],
        job_id="j",
        degree=5,
        n_coeffs=6,
        include_coeff=False,
        include_param=False,
    )
    payload = {
        "job_id": "j",
        "task_id": "raster_0",
        "section_idx": 0,
        "section_count": 1,
        "pix": 512,
        "degree": 5,
        "n_coeffs": 6,
        "rotation": 0.0,
        "solve_score_chain": [["crowding", "1"], ["omega_cosine", "4"]],
        "solve_score_clip_key": "renders/j/solve_scores/crowding_clip.json",
        "solve_source_manifest": solve_source_manifest,
        "logical_section": True,
        "step_start": 0,
        "step_count": 4,
        "raster_mt_threads": 2,
        "raster_input_mode": "sectioned",
        "raster_sectioned_retries": 2,
        "fragment_prefix": "renders/j/color/color_1/fragments/section_",
        "associated_palette_mode": "none",
        "associated_palette_fragment_prefix": "",
        "associated_palette_grid_n": 0,
    }
    payload.update(_bounds_from_center_scale(payload["pix"], payload["pix"], 0.0, 0.0, 1.0))
    payload.update(overrides)
    return payload


class TestRasterMT(unittest.TestCase):
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    def test_handler_reports_malformed_json_at_entry(self, mock_run, mock_report):
        import handler_raster_mt as mod

        with self.assertRaisesRegex(RuntimeError, "could not parse request body"):
            mod.handler({"body": '{"job_id":"j","task_id":"raster_7",'}, None)

        mock_run.assert_not_called()
        mock_report.assert_called_once()
        self.assertEqual(mock_report.call_args.args[:3], ("j", "raster_7", "error"))
        self.assertIn("could not parse request body", mock_report.call_args.args[3])
        self.assertEqual(mock_report.call_args.kwargs["result_data"]["phase"], "handler_entry")

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_fused_fragment_prefix_validation_happens_before_subprocess(self, mock_s3, mock_run, mock_report):
        import handler_raster_mt as mod

        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: b"{}")}

        with self.assertRaisesRegex(RuntimeError, "fused raster requires fragment_prefix"):
            mod.handler(_fused_event(fragment_prefix=""), None)

        mock_run.assert_not_called()
        mock_report.assert_called_once()
        self.assertEqual(mock_report.call_args.args[:3], ("j", "raster_0", "error"))
        self.assertIn("fragment_prefix", mock_report.call_args.args[3])

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_fused_associated_palette_rejects_missing_fragment_prefix_before_subprocess(self, mock_s3, mock_run, mock_report):
        import handler_raster_mt as mod

        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: b"{}")}

        with self.assertRaisesRegex(RuntimeError, "associated_palette_fragment_prefix"):
            mod.handler(
                _fused_event(
                    associated_palette_mode="generated",
                    associated_palette_fragment_prefix="",
                    associated_palette_grid_n=100,
                ),
                None,
            )

        mock_run.assert_not_called()
        mock_report.assert_called_once()
        self.assertEqual(mock_report.call_args.args[:3], ("j", "raster_0", "error"))
        self.assertIn("associated_palette_fragment_prefix", mock_report.call_args.args[3])

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_fused_removed_scalar_fields_fail_before_subprocess(self, mock_s3, mock_run, mock_report):
        import handler_raster_mt as mod

        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: b"{}")}

        with self.assertRaisesRegex(RuntimeError, "no longer accepts removed contract field"):
            mod.handler(
                _fused_event(
                    solve_metric="crowding",
                ),
                None,
            )

        mock_run.assert_not_called()
        self.assertEqual([call.args[2] for call in mock_report.call_args_list], ["error"])

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_fused_rejects_width_height_contract_aliases(self, mock_s3, mock_run, mock_report):
        import handler_raster_mt as mod

        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: b"{}")}

        with self.assertRaisesRegex(RuntimeError, "width, height"):
            mod.handler(_fused_event(width=512, height=512), None)

        mock_run.assert_not_called()
        self.assertEqual([call.args[2] for call in mock_report.call_args_list], ["error"])

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_fused_slv_only_raster_uses_multispan_manifest_and_emits_raw_fragment(self, mock_s3, mock_run, mock_report):
        import handler_raster_mt as mod

        uploads = {}

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/solve_scores/crowding_clip.json":
                payload = _clip_artifact(
                    chain=[["crowding", "1"], ["omega_cosine", "4"]],
                    metric="crowding",
                    quantile=0.01,
                    omega=4.0,
                    omega_enabled=True,
                    clip_lo=-1.0,
                    clip_hi=2.0,
                )
                return {"Body": MagicMock(read=lambda: json.dumps(payload).encode())}
            raise AssertionError(f"unexpected get_object key: {key}")

        def upload_fileobj(fileobj, bucket, key):
            uploads[key] = fileobj.read()

        def put_object(**kwargs):
            body = kwargs["Body"]
            uploads[kwargs["Key"]] = body.read() if hasattr(body, "read") else body

        mock_s3.get_object.side_effect = get_object
        mock_s3.generate_presigned_url.side_effect = lambda _op, Params=None, ExpiresIn=None: f"https://example.com/{Params['Key']}?sig=1"
        mock_s3.upload_fileobj.side_effect = upload_fileobj
        mock_s3.put_object.side_effect = put_object

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            self.assertTrue(any(arg.startswith("--input_manifest=") for arg in cmd))
            self.assertIn("--fragment_prefix=/tmp/fused_fragment", cmd)
            self.assertIn("--step_scores_output=/tmp/step_scores.bin", cmd)
            self.assertNotIn("--associated_palette_fragment_prefix=/tmp/palette_fragment", cmd)
            self.assertNotIn("--score_coeff_manifest=", " ".join(cmd))
            self.assertNotIn("--score_params_manifest=", " ".join(cmd))
            with open("/tmp/fused_fragment.frag", "wb") as fh:
                fh.write(_encode_fragment_pairs([(2, 55)]))
            with open("/tmp/step_scores.bin", "wb") as fh:
                fh.write(bytes([5, 7, 11, 13]))
            return MagicMock(
                returncode=0,
                stdout=json.dumps({
                    "threads": 2,
                    "roots_plotted": 24,
                    "roots_clipped": 3,
                    "roots_deduped": 6,
                    "input_mode": "multispan_sectioned",
                    "download_us": 1200,
                    "native_us": 3400,
                    "download_worker_us": 1200,
                    "native_worker_us": 3400,
                    "download_wall_us": 700,
                    "native_wall_us": 2100,
                    "input_bytes": 4096,
                    "plan_metric_count": 1,
                    "plan_dup_slots": 0,
                    "plan_uses_lag": 0,
                }),
                stderr="",
            )

        mock_run.side_effect = fake_run

        result = mod.handler(_fused_event(), None)
        body = json.loads(result["body"])

        self.assertEqual(body["engine"], "mt")
        self.assertEqual(body["threads"], 2)
        self.assertEqual(body["input_mode"], "multispan_sectioned")
        self.assertEqual(body["fragment_files_uploaded"], 1)
        self.assertEqual(body["fragment_bytes_uploaded"], 5)
        self.assertEqual(body["step_scores_bytes_uploaded"], 4)
        # post-mortem F5/F9/F13 contract: the Step Functions result carries
        # the full timing set with truthful names — the C-side native wall
        # (not the subprocess span), the subprocess span under its own name,
        # worker sums, download wall, bytes, dedup, and the solve plan.
        self.assertEqual(body["native_wall_us"], 2100)
        self.assertEqual(body["download_wall_us"], 700)
        self.assertEqual(body["download_worker_us"], 1200)
        self.assertEqual(body["native_worker_us"], 3400)
        self.assertGreater(body["subprocess_wall_us"], 0)
        self.assertGreaterEqual(body["handler_wall_us"], body["subprocess_wall_us"])
        self.assertEqual(body["input_bytes"], 4096)
        self.assertEqual(body["roots_deduped"], 6)
        self.assertEqual(body["plan_metric_count"], 1)
        self.assertEqual(body["plan_uses_lag"], 0)
        self.assertGreaterEqual(body["prep_wall_us"], 0)
        self.assertGreaterEqual(body["upload_wall_us"], 0)
        self.assertEqual(uploads["renders/j/color/color_1/fragments/section_0000.frag"], _encode_fragment_pairs([(2, 55)]))
        self.assertEqual(uploads["renders/j/color/color_1/fragments/section_0000_step_scores.raw"], bytes([5, 7, 11, 13]))
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "bin_downloaded_1/1", "rasterized_1/1", "rasterized", "done"])

    def test_build_cmd_uses_exact_bounds_when_present(self):
        import handler_raster_mt as mod

        params = _fused_event(
            min_re=-3.5,
            max_re=1.25,
            min_im=-0.75,
            max_im=2.0,
        )
        params.update({
            "effective_input_mode": "multispan_sectioned",
            "input_manifest_path": "/tmp/input_manifest.json",
            "solve_score_bins_data": _clip_artifact(
                chain=[["crowding", "1"], ["omega_cosine", "4"]],
                metric="crowding",
                quantile=0.01,
                omega=4.0,
                omega_enabled=True,
                clip_lo=-1.0,
                clip_hi=2.0,
            ),
        })

        cmd = mod._build_cmd(params)
        joined = " ".join(cmd)
        self.assertIn("--min_re=-3.5", joined)
        self.assertIn("--max_re=1.25", joined)
        self.assertIn("--min_im=-0.75", joined)
        self.assertIn("--max_im=2.0", joined)
        self.assertIn("--pix=512", joined)

    def test_build_cmd_forwards_score_output_normalization_bounds(self):
        import handler_raster_mt as mod

        params = _fused_event()
        params.update({
            "effective_input_mode": "multispan_sectioned",
            "input_manifest_path": "/tmp/input_manifest.json",
            "solve_score_bins_data": _clip_artifact(
                chain=[["crowding", "1"], ["omega_cosine", "4"]],
                metric="crowding",
                quantile=0.01,
                omega=4.0,
                omega_enabled=True,
                clip_lo=-1.0,
                clip_hi=2.0,
                score_output_normalize=True,
                score_output_clip_lo=0.02,
                score_output_clip_hi=0.08,
            ),
        })

        cmd = mod._build_cmd(params)

        self.assertIn("--score_output_normalize=1", cmd)
        self.assertIn("--score_output_clip_lo=0.02", cmd)
        self.assertIn("--score_output_clip_hi=0.08", cmd)

    def test_build_cmd_forwards_explicit_output_channel_clips(self):
        import handler_raster_mt as mod

        params = _fused_event()
        params.update({
            "effective_input_mode": "multispan_sectioned",
            "input_manifest_path": "/tmp/input_manifest.json",
            "solve_score_bins_data": {
                "family": "solve_score",
                "version": 2,
                "program": "m0-0;emit_norm;m1-0;emit_norm;m2-0;emit",
                "metrics": [
                    {"slot": 0, "metric": "proximity", "source": "slv", "quantile": 0.001, "clip_lo": 0.0, "clip_hi": 1.0},
                    {"slot": 1, "metric": "spread", "source": "slv", "quantile": 0.001, "clip_lo": 0.0, "clip_hi": 1.0},
                    {"slot": 2, "metric": "angular_entropy_16", "source": "slv", "quantile": 0.001, "clip_lo": 0.0, "clip_hi": 1.0},
                ],
                "score_output_channel_count": 3,
                "score_output_channels": [
                    {"channel": 0, "clip_lo": 0.1, "clip_hi": 0.9},
                    {"channel": 1, "clip_lo": 0.2, "clip_hi": 0.8},
                    {"channel": 2, "clip_lo": 0.0, "clip_hi": 1.0},
                ],
            },
        })

        cmd = mod._build_cmd(params)

        self.assertIn("--score_output_clip_los=0.1,0.2,0.0", cmd)
        self.assertIn("--score_output_clip_his=0.9,0.8,1.0", cmd)
        self.assertIn("--step_scores_output=/tmp/step_scores.bin", cmd)

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_fused_associated_palette_emits_inline_palette_fragment(self, mock_s3, mock_run, mock_report):
        import handler_raster_mt as mod

        uploads = {}

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/solve_scores/crowding_clip.json":
                payload = _clip_artifact(
                    chain=[["crowding", "1"], ["omega_cosine", "4"]],
                    metric="crowding",
                    quantile=0.01,
                    omega=4.0,
                    omega_enabled=True,
                    clip_lo=-1.0,
                    clip_hi=2.0,
                )
                return {"Body": MagicMock(read=lambda: json.dumps(payload).encode())}
            raise AssertionError(f"unexpected get_object key: {key}")

        def upload_fileobj(fileobj, bucket, key):
            uploads[key] = fileobj.read()

        def put_object(**kwargs):
            body = kwargs["Body"]
            uploads[kwargs["Key"]] = body.read() if hasattr(body, "read") else body

        mock_s3.get_object.side_effect = get_object
        mock_s3.generate_presigned_url.side_effect = lambda _op, Params=None, ExpiresIn=None: f"https://example.com/{Params['Key']}?sig=1"
        mock_s3.upload_fileobj.side_effect = upload_fileobj
        mock_s3.put_object.side_effect = put_object

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            self.assertIn("--associated_palette_fragment_prefix=/tmp/palette_fragment", cmd)
            self.assertIn("--palette_grid_n=100", cmd)
            self.assertIn("--palette_step_start=0", cmd)
            with open("/tmp/fused_fragment.frag", "wb") as fh:
                fh.write(_encode_fragment_pairs([(0, 33)]))
            with open("/tmp/palette_fragment.frag", "wb") as fh:
                fh.write(_encode_fragment_pairs([(1, 44)]))
            with open("/tmp/step_scores.bin", "wb") as fh:
                fh.write(bytes([5, 7, 11, 13]))
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"threads": 2, "roots_plotted": 24, "roots_clipped": 3}),
                stderr="",
            )

        mock_run.side_effect = fake_run

        result = mod.handler(
            _fused_event(
                associated_palette_mode="generated",
                associated_palette_fragment_prefix="renders/j/palettes/pal_1/fragments/section_",
                associated_palette_grid_n=100,
            ),
            None,
        )
        body = json.loads(result["body"])

        self.assertEqual(body["associated_palette_fragment_files_uploaded"], 1)
        self.assertEqual(body["associated_palette_fragment_bytes_uploaded"], 5)
        self.assertEqual(uploads["renders/j/palettes/pal_1/fragments/section_0000.frag"], _encode_fragment_pairs([(1, 44)]))

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_fused_mixed_source_multispan_passes_coeff_and_param_manifests(self, mock_s3, mock_run, mock_report):
        import handler_raster_mt as mod
        from logical_sections import build_solve_source_manifest

        uploads = {}
        solve_source_manifest = build_solve_source_manifest(
            [
                {
                    "chunk_idx": 0,
                    "bin_key": "renders/j/chunk_0.bin",
                    "coeffs_key": "renders/j/coeffs_0000.bin",
                    "params_key": "renders/j/params.bin",
                    "step_start": 0,
                    "step_count": 2,
                    "params_step_start": 0,
                    "params_step_count": 2,
                },
                {
                    "chunk_idx": 1,
                    "bin_key": "renders/j/chunk_1.bin",
                    "coeffs_key": "renders/j/coeffs_0001.bin",
                    "params_key": "renders/j/params.bin",
                    "step_start": 2,
                    "step_count": 2,
                    "params_step_start": 2,
                    "params_step_count": 2,
                },
            ],
            job_id="j",
            degree=5,
            n_coeffs=6,
            include_coeff=True,
            include_param=True,
        )

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/solve_scores/crowding_clip.json":
                payload = _clip_artifact(
                    chain=[["spread", "slv", "2"], ["spread", "cf", "2"], ["avg"], ["t1_abs", "pm", "2"], ["avg"]],
                    metric="spread",
                    quantile=0.02,
                    omega=1.0,
                    omega_enabled=False,
                    clip_lo=0.0,
                    clip_hi=1.0,
                )
                return {"Body": MagicMock(read=lambda: json.dumps(payload).encode())}
            raise AssertionError(f"unexpected get_object key: {key}")

        def upload_fileobj(fileobj, bucket, key):
            uploads[key] = fileobj.read()

        mock_s3.get_object.side_effect = get_object
        mock_s3.generate_presigned_url.side_effect = lambda _op, Params=None, ExpiresIn=None: f"https://example.com/{Params['Key']}?sig=1"
        mock_s3.upload_fileobj.side_effect = upload_fileobj
        mock_s3.put_object.side_effect = lambda **kwargs: None

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            self.assertIn("--score_sources=slv,cf,pm", cmd)
            self.assertIn("--score_program=v2;m0-0;m1-0;avg;m2-0;avg", cmd)
            self.assertTrue(any(arg.startswith("--score_coeff_manifest=") for arg in cmd))
            self.assertIn("--score_coeff_degree=6", cmd)
            self.assertTrue(any(arg.startswith("--score_params_manifest=") for arg in cmd))
            with open("/tmp/fused_fragment.frag", "wb") as fh:
                fh.write(_encode_fragment_pairs([(3, 88)]))
            with open("/tmp/step_scores.bin", "wb") as fh:
                fh.write(bytes([9, 7, 5, 3]))
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"threads": 2, "roots_plotted": 9, "roots_clipped": 0}),
                stderr="",
            )

        mock_run.side_effect = fake_run

        result = mod.handler(
            _fused_event(
                solve_score_chain=[["spread", "slv", "2"], ["spread", "cf", "2"], ["avg"], ["t1_abs", "pm", "2"], ["avg"]],
                solve_source_manifest=solve_source_manifest,
                n_coeffs=6,
                solve_score_clip_key="renders/j/solve_scores/crowding_clip.json",
            ),
            None,
        )
        body = json.loads(result["body"])

        self.assertEqual(body["input_mode"], "multispan_sectioned")
        self.assertEqual(body["fragment_files_uploaded"], 1)
        self.assertEqual(body["step_scores_bytes_uploaded"], 4)
        self.assertEqual(uploads["renders/j/color/color_1/fragments/section_0000.frag"], _encode_fragment_pairs([(3, 88)]))


    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_view_projection_flags_ride_the_section_cmd(self, mock_s3, mock_run, mock_report):
        # Views: elevations pass projection + vertical + the calc grid +
        # the SECTION's step offset — the same contract as the palette path
        import handler_raster_mt as mod
        uploads = {}

        def get_object(**kwargs):
            key = kwargs.get("Key")
            if key.endswith("_clip.json"):
                payload = _clip_artifact(
                    chain=[["crowding", "1"], ["omega_cosine", "4"]],
                    metric="crowding",
                    quantile=0.01,
                    omega=4.0,
                    omega_enabled=True,
                    clip_lo=-1.0,
                    clip_hi=2.0,
                )
                return {"Body": MagicMock(read=lambda: json.dumps(payload).encode())}
            raise AssertionError(f"unexpected get_object key: {key}")

        mock_s3.get_object.side_effect = get_object
        mock_s3.generate_presigned_url.side_effect = (
            lambda _op, Params=None, ExpiresIn=None: f"https://example.com/{Params['Key']}?sig=1")
        mock_s3.upload_fileobj.side_effect = lambda fileobj, bucket, key: uploads.__setitem__(key, fileobj.read())
        mock_s3.put_object.side_effect = lambda **kwargs: uploads.__setitem__(kwargs["Key"], kwargs["Body"])

        seen = {}

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            seen["cmd"] = list(cmd)
            with open("/tmp/fused_fragment.frag", "wb") as fh:
                fh.write(_encode_fragment_pairs([(0, 33)]))
            with open("/tmp/step_scores.bin", "wb") as fh:
                fh.write(bytes([5, 7, 11, 13]))
            return MagicMock(returncode=0,
                             stdout=json.dumps({"threads": 2, "roots_plotted": 4, "roots_clipped": 0}),
                             stderr="")
        mock_run.side_effect = fake_run

        result = mod.handler(_fused_event(
            view_projection="front",
            view_vertical="t1",
            view_grid_n=100,
            step_start=2,
        ), None)
        self.assertEqual(result["statusCode"], 200, result)
        self.assertIn("--view_projection=front", seen["cmd"])
        self.assertIn("--view_vertical=t1", seen["cmd"])
        self.assertIn("--view_grid_n=100", seen["cmd"])
        self.assertIn("--view_step_start=2", seen["cmd"])

        # plan (the default) must add NO view flags — byte-identical cmds
        seen.clear()
        result = mod.handler(_fused_event(step_start=0), None)
        self.assertEqual(result["statusCode"], 200, result)
        self.assertFalse([a for a in seen["cmd"] if a.startswith("--view_")])

        # a bad projection dies loudly before any raster runs
        with self.assertRaises(RuntimeError):
            mod.handler(_fused_event(view_projection="top", view_grid_n=100), None)

        # an elevation without the grid dies loudly too
        with self.assertRaises(RuntimeError):
            mod.handler(_fused_event(view_projection="front"), None)


if __name__ == "__main__":
    unittest.main()
