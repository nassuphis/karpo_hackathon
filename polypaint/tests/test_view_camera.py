import math
import pathlib
import sys

import pytest


ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "lambda"))

from view_camera import (  # noqa: E402
    CAMERA_FRAGMENT_ENCODING,
    camera_execution_hash,
    fragment_contract,
    validate_fragment_contract,
    validate_view_camera,
)


def _camera():
    near = 0.01
    far = 50.0
    f = 1.0 / math.tan(math.radians(50.0) / 2.0)
    return {
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
        "point_world_size": 0.004,
        "point_scale": 0.5,
        "point_min_fraction": 1.0 / 1024.0,
        "point_max_fraction": 1.0,
        "style": "solid",
        "show": {
            "points": True,
            "ribbons": False,
            "threads": False,
            "clu": False,
            "splats": False,
        },
        "frame": {"aspect": 1.0, "crop": "center_square"},
        "debug": {
            "camera_position": [0.0, 0.0, 2.0],
            "camera_target": [0.0, 0.0, 0.0],
            "vertical_fov_degrees": 50.0,
            "near": near,
            "far": far,
            "height": 1.0,
            "control_zlo": 0.0,
            "control_zhi": 1.0,
            "point_control": 10.0,
        },
    }


def test_valid_camera_normalizes_and_preserves_execution_fields():
    camera = validate_view_camera(_camera())
    assert camera["vertical"] == "t2"
    assert camera["frame"] == {"aspect": 1.0, "crop": "center_square"}
    assert camera["debug"]["near"] == 0.01


@pytest.mark.parametrize(
    ("field", "value", "message"),
    [
        ("slices", 7, "slices"),
        ("style", "ghost", "style=solid"),
        ("point_world_size", 0.0, "point_world_size"),
        ("point_scale", 0.49, "point_scale"),
        ("effective_tlo", 1.1, "effective t bounds"),
    ],
)
def test_invalid_camera_fields_fail_closed(field, value, message):
    camera = _camera()
    camera[field] = value
    with pytest.raises(RuntimeError, match=message):
        validate_view_camera(camera)


@pytest.mark.parametrize("field", ("version", "slices"))
def test_fractional_integer_fields_fail_closed(field):
    camera = _camera()
    camera[field] = float(camera[field]) + 0.5
    with pytest.raises(RuntimeError, match="must be an integer"):
        validate_view_camera(camera)


def test_hidden_points_and_visible_unsupported_primitives_are_rejected():
    hidden = _camera()
    hidden["show"]["points"] = False
    with pytest.raises(RuntimeError, match="visible points"):
        validate_view_camera(hidden)

    ribbons = _camera()
    ribbons["show"]["ribbons"] = True
    with pytest.raises(RuntimeError, match="ribbons"):
        validate_view_camera(ribbons)


def test_camera_execution_hash_ignores_debug_and_folds_signed_zero():
    left = _camera()
    right = _camera()
    right["debug"]["height"] = 0.25
    right["model_view_matrix"][12] = -0.0
    assert camera_execution_hash(left) == camera_execution_hash(right)


def test_camera_execution_hash_distinguishes_adjacent_binary64_values():
    left = _camera()
    right = _camera()
    right["model_view_matrix"][12] = math.nextafter(0.0, 1.0)
    assert camera_execution_hash(left) != camera_execution_hash(right)


def test_fragment_contract_is_projection_and_output_aware():
    assert fragment_contract("plan", 1) == {
        "encoding": "u32le_u8_v1",
        "record_size_bytes": 5,
        "channels": 1,
    }
    assert fragment_contract("front", 1, has_explicit_outputs=True) == {
        "encoding": "u32le_pixel_idx_plus_u8_channels_v1",
        "record_size_bytes": 5,
        "channels": 1,
    }
    camera = fragment_contract("camera", 3)
    assert camera == {
        "encoding": CAMERA_FRAGMENT_ENCODING,
        "record_size_bytes": 11,
        "channels": 3,
    }
    assert validate_fragment_contract(
        camera,
        projection="camera",
        channels=3,
    ) == camera


def test_fragment_contract_mismatch_is_rejected():
    with pytest.raises(RuntimeError, match="mismatch"):
        validate_fragment_contract(
            {
                "encoding": "u32le_u8_v1",
                "record_size_bytes": 5,
                "channels": 1,
            },
            projection="camera",
            channels=1,
        )
