import ast
import math
import pathlib
import sys

import pytest


ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "lambda"))

from view_camera import validate_view_camera  # noqa: E402
from view_snap_cost_model import (  # noqa: E402
    camera_projection_bound,
    enforce_hard_resource_limits,
    estimate_camera_sections,
)


def _camera():
    near = 0.01
    far = 50.0
    f = 1.0 / math.tan(math.radians(50.0) / 2.0)
    return validate_view_camera({
        "version": 1,
        "projection": "perspective",
        "matrix_layout": "column_major",
        "model_view_matrix": [
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, -2.0, 1.0,
        ],
        "projection_matrix": [
            f, 0.0, 0.0, 0.0,
            0.0, f, 0.0, 0.0,
            0.0, 0.0, (far + near) / (near - far), -1.0,
            0.0, 0.0, (2.0 * far * near) / (near - far), 0.0,
        ],
        "vertical": "t2",
        "slices": 0,
        "effective_tlo": 0.0,
        "effective_thi": 1.0,
        "frame": {"aspect": 1.0, "crop": "center_square"},
    })


def _sections():
    return [
        {"section_idx": 0, "step_start": 0, "step_count": 5_000},
        {"section_idx": 1, "step_start": 5_000, "step_count": 10_000},
        {"section_idx": 2, "step_start": 15_000, "step_count": 5_000},
    ]


def _estimate(**overrides):
    params = {
        "pix": 100,
        "degree": 3,
        "channels": 1,
        "times": 2,
        "source_row_bytes": 40,
        "camera": _camera(),
    }
    params.update(overrides)
    return estimate_camera_sections(_sections(), **params)


def test_cost_model_has_no_behavioral_imports():
    tree = ast.parse(
        (ROOT / "lambda" / "view_snap_cost_model.py").read_text(encoding="utf-8")
    )
    imported = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.Import):
            imported.update(alias.name for alias in node.names)
        elif isinstance(node, ast.ImportFrom) and node.module:
            imported.add(node.module)
    assert imported <= {"__future__", "math"}


def test_camera_projection_prices_exactly_one_pixel_per_root():
    bound = camera_projection_bound(_camera(), 1_000)
    assert bound["near"] == pytest.approx(0.01)
    assert bound["min_prism_depth"] == pytest.approx(1.5)
    assert bound["pricing_depth"] == pytest.approx(1.5)
    assert bound["pixels_per_root_upper"] == 1


@pytest.mark.parametrize(("axis", "translation"), (("left", 100.0), ("far", -100.0)))
def test_empty_camera_prism_is_rejected(axis, translation):
    camera = _camera()
    camera["model_view_matrix"][12 if axis == "left" else 14] = translation
    with pytest.raises(RuntimeError, match="empty camera frustum"):
        camera_projection_bound(camera, 100)


def test_every_pass_is_projected_from_stored_scores():
    estimate = _estimate()
    pass1 = estimate["sections"][2]
    assert pass1["candidate_roots"] == pass1["step_count"] * 3
    assert pass1["fragment_bytes_upper"] > 0
    assert estimate["final_tmp_bytes"] < (
        estimate["raw_bytes"]
        + estimate["step_scores_bytes"]
        + estimate["encoded_output_bytes_upper"]
    )


def test_resource_estimate_counts_fragment_and_step_score_tmp_together():
    estimate = _estimate()
    for section in estimate["sections"]:
        assert section["raster_tmp_bytes"] == (
            section["fragment_bytes_upper"] + section["step_score_bytes"]
        )
    assert estimate["step_scores_bytes"] == 100 * 100 * 2
    assert estimate["final_tmp_bytes"] > (
        estimate["raw_bytes"]
    )
    assert estimate["total_candidate_roots"] > estimate["max_candidate_roots"]


def test_section_resource_estimate_counts_only_current_root_rows():
    estimate = _estimate()
    first, second = estimate["sections"][:2]
    assert first["source_bytes"] == 5_000 * 40
    assert second["source_bytes"] == 10_000 * 40


def test_hard_resource_limits_name_the_binding_stage():
    estimate = _estimate()
    generous = 10**12
    with pytest.raises(RuntimeError, match="Raster /tmp"):
        enforce_hard_resource_limits(
            estimate,
            raster_memory_bytes=generous,
            raster_tmp_bytes=estimate["max_raster_tmp_bytes"] - 1,
            finalize_memory_bytes=generous,
            finalize_tmp_bytes=generous,
        )
    with pytest.raises(RuntimeError, match="Finalize /tmp"):
        enforce_hard_resource_limits(
            estimate,
            raster_memory_bytes=generous,
            raster_tmp_bytes=generous,
            finalize_memory_bytes=generous,
            finalize_tmp_bytes=estimate["final_tmp_bytes"] - 1,
        )


def test_work_unit_and_pixel_arithmetic_fail_closed_on_overflow():
    with pytest.raises(RuntimeError, match="overflow"):
        estimate_camera_sections(
            [{"section_idx": 0, "step_start": 0, "step_count": 1}],
            pix=4_000_000_000,
            degree=3,
            channels=1,
            times=1,
            source_row_bytes=40,
            camera=_camera(),
        )
