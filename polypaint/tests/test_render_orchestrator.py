"""
Tests for handler_render_orchestrator.py.

Validates phase ordering, checkpoint/self-reinvoke, subtask ID format,
and error handling for all three render modes.

Run: cd polypaint && uv run python -m pytest tests/test_render_orchestrator.py -v
"""
import json
import os
import sys
import unittest
from unittest.mock import patch, MagicMock, call

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _make_event(payload):
    return payload  # handler_render_orchestrator uses parse_body which handles raw dicts


def _mock_poll_done(n=1):
    """Return a DDB query response where n tasks are done."""
    return {
        "Items": [
            {"task_id": {"S": f"t_{i}"}, "task_status": {"S": "done"}}
            for i in range(n)
        ]
    }


class TestColorOrchestrator(unittest.TestCase):

    @patch("handler_render_orchestrator.ddb")
    @patch("handler_render_orchestrator.lambda_client")
    @patch("handler_render_orchestrator.report_status")
    @patch("handler_render_orchestrator._storage_call")
    def test_color_dispatches_raster_then_finalize_then_encode(
        self, mock_storage, mock_report, mock_lambda, mock_ddb
    ):
        """Color mode runs: clean→viewport→calc_meta→raster→finalize→encode."""
        mock_storage.side_effect = lambda path, body: (
            {"deleted": 0} if "clean" in path else
            {"job_id": "j", "calc": {"degree": 5, "n_stripes": 2, "n_chunks": 2}} if "detail" in path else
            {}
        )
        # All polls return done immediately
        mock_ddb.query.return_value = _mock_poll_done(100)

        from handler_render_orchestrator import handler
        event = _make_event({
            "job_id": "j", "run_id": "run_test", "mode": "color",
            "params": {
                "pix": 512, "fmt": "jpeg", "quality": 90,
                "view_mode": "square", "square_extent": 2.0,
                "tile_size": 512, "rotation": 0, "color_mode": "rainbow",
                "match_mode": "none", "palette": "inferno", "constant_color": "ffffff",
            }
        })
        result = handler(event, None)
        body = json.loads(result["body"])
        assert body["phase"] == "done"

        # Check phase order from report_status calls
        phases = [c.args[2] for c in mock_report.call_args_list if len(c.args) >= 3]
        assert "clean" in phases
        assert "viewport" in phases
        assert "raster_dispatch" in phases
        assert "finalize_dispatch" in phases
        assert "encode_dispatch" in phases
        # raster before finalize before encode
        ri = phases.index("raster_dispatch")
        fi = phases.index("finalize_dispatch")
        ei = phases.index("encode_dispatch")
        assert ri < fi < ei

    @patch("handler_render_orchestrator.ddb")
    @patch("handler_render_orchestrator.lambda_client")
    @patch("handler_render_orchestrator.report_status")
    @patch("handler_render_orchestrator._storage_call")
    def test_color_solve_proximity_prepass_before_raster(
        self, mock_storage, mock_report, mock_lambda, mock_ddb
    ):
        """Color + solve_proximity runs clip→hist→merge before raster."""
        mock_storage.side_effect = lambda path, body: (
            {"deleted": 0} if "clean" in path else
            {"job_id": "j", "calc": {
                "degree": 5, "n_stripes": 2, "n_chunks": 2,
                "lores": {"bin_key": "renders/j/lores.bin"}
            }} if "detail" in path else {}
        )
        mock_ddb.query.return_value = _mock_poll_done(100)

        from handler_render_orchestrator import handler
        event = _make_event({
            "job_id": "j", "run_id": "run_sp", "mode": "color",
            "params": {
                "pix": 512, "fmt": "jpeg", "quality": 90,
                "view_mode": "square", "square_extent": 2.0,
                "tile_size": 512, "rotation": 0, "color_mode": "solve_proximity",
                "match_mode": "none", "palette": "inferno", "constant_color": "ffffff",
            }
        })
        result = handler(event, None)
        body = json.loads(result["body"])
        assert body["phase"] == "done"

        phases = [c.args[2] for c in mock_report.call_args_list if len(c.args) >= 3]
        assert "solve_proximity_clip" in phases
        assert "solve_proximity_hist" in phases
        assert "solve_proximity_merge" in phases
        assert "raster_dispatch" in phases
        # solve proximity before raster
        ci = phases.index("solve_proximity_clip")
        ri = phases.index("raster_dispatch")
        assert ci < ri


class TestBilevelOrchestrator(unittest.TestCase):

    @patch("handler_render_orchestrator.ddb")
    @patch("handler_render_orchestrator.lambda_client")
    @patch("handler_render_orchestrator.report_status")
    @patch("handler_render_orchestrator._storage_call")
    def test_bilevel_dispatches_raster_merge_stitch(
        self, mock_storage, mock_report, mock_lambda, mock_ddb
    ):
        mock_storage.side_effect = lambda path, body: (
            {"deleted": 0} if "clean" in path else
            {"job_id": "j", "calc": {"degree": 5, "n_stripes": 2, "n_chunks": 2}} if "detail" in path else {}
        )
        mock_ddb.query.return_value = _mock_poll_done(100)

        from handler_render_orchestrator import handler
        event = _make_event({
            "job_id": "j", "run_id": "run_bi", "mode": "bilevel",
            "params": {"pix": 512, "tile_size": 512, "view_mode": "square", "square_extent": 2.0, "rotation": 0}
        })
        result = handler(event, None)
        body = json.loads(result["body"])
        assert body["phase"] == "done"

        phases = [c.args[2] for c in mock_report.call_args_list if len(c.args) >= 3]
        assert "bilevel_raster_dispatch" in phases
        assert "bilevel_merge_dispatch" in phases
        assert "bilevel_stitch_dispatch" in phases
        ri = phases.index("bilevel_raster_dispatch")
        mi = phases.index("bilevel_merge_dispatch")
        si = phases.index("bilevel_stitch_dispatch")
        assert ri < mi < si


class TestCoeffOrchestrator(unittest.TestCase):

    @patch("handler_render_orchestrator.ddb")
    @patch("handler_render_orchestrator.lambda_client")
    @patch("handler_render_orchestrator.report_status")
    @patch("handler_render_orchestrator._storage_call")
    def test_coeff_dispatches_raster_merge_stitch(
        self, mock_storage, mock_report, mock_lambda, mock_ddb
    ):
        mock_storage.side_effect = lambda path, body: (
            {"deleted": 0} if "clean" in path else
            {"job_id": "j", "calc": {"degree": 5, "n_stripes": 2, "n_chunks": 2}} if "detail" in path else {}
        )
        mock_ddb.query.return_value = _mock_poll_done(100)

        from handler_render_orchestrator import handler
        event = _make_event({
            "job_id": "j", "run_id": "run_cb", "mode": "coeff_bilevel",
            "params": {"pix": 512, "tile_size": 512, "view_mode": "square", "square_extent": 2.0, "rotation": 0}
        })
        result = handler(event, None)
        body = json.loads(result["body"])
        assert body["phase"] == "done"

        phases = [c.args[2] for c in mock_report.call_args_list if len(c.args) >= 3]
        assert "coeff_raster_dispatch" in phases
        assert "coeff_merge_dispatch" in phases
        assert "coeff_stitch_dispatch" in phases
        ri = phases.index("coeff_raster_dispatch")
        mi = phases.index("coeff_merge_dispatch")
        si = phases.index("coeff_stitch_dispatch")
        assert ri < mi < si


    @patch("handler_render_orchestrator.ddb")
    @patch("handler_render_orchestrator.lambda_client")
    @patch("handler_render_orchestrator.report_status")
    @patch("handler_render_orchestrator._storage_call")
    def test_coeff_uses_coeffs_keys_from_metadata(
        self, mock_storage, mock_report, mock_lambda, mock_ddb
    ):
        """Coeff bilevel uses coeffs_keys from calc metadata, not synthesized keys."""
        # Use keys and n_coeffs that DIFFER from the fallback to prove metadata is used
        real_keys = ["renders/j/custom_chunk_A.bin", "renders/j/custom_chunk_B.bin"]
        mock_storage.side_effect = lambda path, body: (
            {"deleted": 0} if "clean" in path else
            {"job_id": "j", "calc": {
                "degree": 5, "n_stripes": 2, "n_chunks": 2,
                "n_coeffs": 99,  # != degree+1 (6), proves metadata is used
                "coeffs_keys": real_keys,
            }} if "detail" in path else {}
        )
        mock_ddb.query.return_value = _mock_poll_done(100)

        from handler_render_orchestrator import handler
        event = _make_event({
            "job_id": "j", "run_id": "run_ck", "mode": "coeff_bilevel",
            "params": {"pix": 512, "tile_size": 512, "view_mode": "square", "square_extent": 2.0, "rotation": 0}
        })
        handler(event, None)

        # Check dispatched coeff raster jobs used the real keys
        dispatched_payloads = []
        for c in mock_lambda.invoke.call_args_list:
            payload = json.loads(c[1].get("Payload", b"{}"))
            if payload.get("phase") == "coeff_raster":
                dispatched_payloads.append(payload)

        assert len(dispatched_payloads) == 2, f"expected 2 coeff raster jobs, got {len(dispatched_payloads)}"
        assert dispatched_payloads[0]["coeffs_key"] == "renders/j/custom_chunk_A.bin", \
            f"stripe 0 key wrong: {dispatched_payloads[0].get('coeffs_key')}"
        assert dispatched_payloads[1]["coeffs_key"] == "renders/j/custom_chunk_B.bin", \
            f"stripe 1 key wrong: {dispatched_payloads[1].get('coeffs_key')}"
        assert dispatched_payloads[0]["n_coeffs"] == 99, \
            f"n_coeffs should be 99 from metadata, got {dispatched_payloads[0].get('n_coeffs')}"


class TestOrchestratorCheckpoint(unittest.TestCase):

    @patch("handler_render_orchestrator.ddb")
    @patch("handler_render_orchestrator.lambda_client")
    @patch("handler_render_orchestrator.report_status")
    @patch("handler_render_orchestrator._storage_call")
    def test_resumed_encode_poll_uses_stored_single_job(
        self, mock_storage, mock_report, mock_lambda, mock_ddb
    ):
        """Resume from encode_poll checkpoint uses stored single-job dispatch info."""
        mock_storage.side_effect = lambda path, body: {}
        # First poll: not done → triggers stall check. Second: done.
        poll_count = [0]
        def mock_query(**kwargs):
            poll_count[0] += 1
            if poll_count[0] <= 1:
                return {"Items": []}  # not done yet
            return {"Items": [{"task_id": {"S": "render_run_enc_encode"}, "task_status": {"S": "done"}}]}
        mock_ddb.query.side_effect = mock_query

        # Build checkpoint as if encode_dispatch just completed
        encode_job = {
            "job_id": "j", "task_id": "render_run_enc_encode",
            "out_key": "renders/j/image.jpeg", "format": "jpeg",
            "quality": 90, "width": 512, "height": 512,
            "tile_grid": {"n_cols": 1, "n_rows": 1, "tile_keys": ["renders/j/tile_0000.raw"]},
        }
        checkpoint = {
            "phase": "encode_poll",
            "started_at_ms": 1000,
            "job_id": "j", "run_id": "run_enc", "mode": "color",
            "_params": {
                "pix": 512, "fmt": "jpeg", "quality": 90,
                "view_mode": "square", "square_extent": 2.0,
                "tile_size": 512, "rotation": 0, "color_mode": "rainbow",
                "match_mode": "none", "palette": "inferno", "constant_color": "ffffff",
            },
            "_viewport": {"center_re": 0, "center_im": 0, "scale": 128},
            "_calc": {"degree": 5, "n_stripes": 2, "n_chunks": 2},
            "n_stripes": 2, "degree": 5, "n_tiles": 1,
            "image_key": "renders/j/image.jpeg",
            # This is what _dispatch_single stores:
            "_last_dispatched_jobs": [encode_job],
            "_last_dispatch_function": "polypaint-encode",
        }

        from handler_render_orchestrator import handler
        event = _make_event({
            "job_id": "j", "run_id": "run_enc", "mode": "color",
            "params": checkpoint["_params"],
            "_checkpoint": checkpoint,
        })
        result = handler(event, None)
        body = json.loads(result["body"])
        assert body["phase"] == "done", f"expected done, got {body['phase']}"

    @patch("handler_render_orchestrator.ddb")
    @patch("handler_render_orchestrator.lambda_client")
    @patch("handler_render_orchestrator.report_status")
    @patch("handler_render_orchestrator._storage_call")
    def test_checkpoints_and_self_reinvokes_when_time_low(
        self, mock_storage, mock_report, mock_lambda, mock_ddb
    ):
        """If remaining time < 60s, orchestrator checkpoints and self-invokes."""
        mock_storage.side_effect = lambda path, body: (
            {"deleted": 0} if "clean" in path else
            {"job_id": "j", "calc": {"degree": 5, "n_stripes": 2, "n_chunks": 2}} if "detail" in path else {}
        )
        # Make raster poll return not-done so we stay in poll loop
        mock_ddb.query.return_value = {"Items": []}

        # Fake context that returns low remaining time
        mock_context = MagicMock()
        mock_context.get_remaining_time_in_millis.return_value = 30000  # 30s < 60s threshold

        from handler_render_orchestrator import handler
        event = _make_event({
            "job_id": "j", "run_id": "run_ck", "mode": "color",
            "params": {
                "pix": 512, "fmt": "jpeg", "quality": 90,
                "view_mode": "square", "square_extent": 2.0,
                "tile_size": 512, "rotation": 0, "color_mode": "rainbow",
                "match_mode": "none", "palette": "inferno", "constant_color": "ffffff",
            }
        })
        result = handler(event, mock_context)
        body = json.loads(result["body"])
        assert body["status"] == "reinvoked"

        # Verify self-invoke was called
        self_invoke_calls = [
            c for c in mock_lambda.invoke.call_args_list
            if c[1].get("InvocationType") == "Event"
            and "render-orchestrator" in c[1].get("FunctionName", "")
        ]
        assert len(self_invoke_calls) >= 1, "should have self-invoked"


class TestOrchestratorStatusAndTaskIds(unittest.TestCase):

    @patch("handler_render_orchestrator.ddb")
    @patch("handler_render_orchestrator.lambda_client")
    @patch("handler_render_orchestrator.report_status")
    @patch("handler_render_orchestrator._storage_call")
    def test_writes_status_each_phase(
        self, mock_storage, mock_report, mock_lambda, mock_ddb
    ):
        mock_storage.side_effect = lambda path, body: (
            {"deleted": 0} if "clean" in path else
            {"job_id": "j", "calc": {"degree": 5, "n_stripes": 2, "n_chunks": 2}} if "detail" in path else {}
        )
        mock_ddb.query.return_value = _mock_poll_done(100)

        from handler_render_orchestrator import handler
        event = _make_event({
            "job_id": "j", "run_id": "run_st", "mode": "color",
            "params": {
                "pix": 512, "fmt": "jpeg", "quality": 90,
                "view_mode": "square", "square_extent": 2.0,
                "tile_size": 512, "rotation": 0, "color_mode": "rainbow",
                "match_mode": "none", "palette": "inferno", "constant_color": "ffffff",
            }
        })
        handler(event, None)

        # Every report_status call should include phase in result_data
        for c in mock_report.call_args_list:
            if len(c.args) >= 3 and c.kwargs.get("result_data"):
                rd = c.kwargs["result_data"]
                assert "phase" in rd, f"result_data missing phase: {rd}"

    @patch("handler_render_orchestrator.ddb")
    @patch("handler_render_orchestrator.lambda_client")
    @patch("handler_render_orchestrator.report_status")
    @patch("handler_render_orchestrator._storage_call")
    def test_worker_task_ids_include_run_id(
        self, mock_storage, mock_report, mock_lambda, mock_ddb
    ):
        """All dispatched worker jobs must have task_id containing run_id."""
        mock_storage.side_effect = lambda path, body: (
            {"deleted": 0} if "clean" in path else
            {"job_id": "j", "calc": {"degree": 5, "n_stripes": 2, "n_chunks": 2}} if "detail" in path else {}
        )
        mock_ddb.query.return_value = _mock_poll_done(100)

        from handler_render_orchestrator import handler
        run_id = "run_tid_test"
        event = _make_event({
            "job_id": "j", "run_id": run_id, "mode": "color",
            "params": {
                "pix": 512, "fmt": "jpeg", "quality": 90,
                "view_mode": "square", "square_extent": 2.0,
                "tile_size": 512, "rotation": 0, "color_mode": "rainbow",
                "match_mode": "none", "palette": "inferno", "constant_color": "ffffff",
            }
        })
        handler(event, None)

        # Check all async invocations for task_id
        for c in mock_lambda.invoke.call_args_list:
            payload_bytes = c[1].get("Payload", b"{}")
            if isinstance(payload_bytes, bytes):
                payload = json.loads(payload_bytes)
            else:
                payload = json.loads(payload_bytes)
            if "task_id" in payload:
                assert run_id in payload["task_id"], \
                    f"task_id {payload['task_id']} missing run_id {run_id}"


class TestOrchestratorError(unittest.TestCase):

    @patch("handler_render_orchestrator.ddb")
    @patch("handler_render_orchestrator.lambda_client")
    @patch("handler_render_orchestrator.report_status")
    @patch("handler_render_orchestrator._storage_call")
    def test_error_sets_terminal_error_status(
        self, mock_storage, mock_report, mock_lambda, mock_ddb
    ):
        """Downstream error → orchestrator row marked error."""
        mock_storage.side_effect = lambda path, body: (
            {"deleted": 0} if "clean" in path else
            {"job_id": "j", "calc": {"degree": 5, "n_stripes": 2, "n_chunks": 2}} if "detail" in path else {}
        )
        # Raster poll returns an error
        mock_ddb.query.return_value = {
            "Items": [
                {"task_id": {"S": "t_0"}, "task_status": {"S": "error"},
                 "error_msg": {"S": "raster exploded"}}
            ]
        }

        from handler_render_orchestrator import handler
        event = _make_event({
            "job_id": "j", "run_id": "run_err", "mode": "color",
            "params": {
                "pix": 512, "fmt": "jpeg", "quality": 90,
                "view_mode": "square", "square_extent": 2.0,
                "tile_size": 512, "rotation": 0, "color_mode": "rainbow",
                "match_mode": "none", "palette": "inferno", "constant_color": "ffffff",
            }
        })
        try:
            handler(event, None)
            assert False, "should have raised"
        except RuntimeError as e:
            assert "raster exploded" in str(e)

        # Last report_status should be error
        last_call = mock_report.call_args_list[-1]
        assert last_call.args[2] == "error"


if __name__ == "__main__":
    unittest.main()
