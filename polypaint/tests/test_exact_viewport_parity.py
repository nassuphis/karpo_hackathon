import array
import json
import os
import pathlib
import shutil
import subprocess
import tempfile

import pytest


ROOT = pathlib.Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"


def _compiler():
    return shutil.which("cc") or shutil.which("clang") or shutil.which("gcc")


def _write_float_file(path, values):
    arr = array.array("f", values)
    with open(path, "wb") as fh:
        arr.tofile(fh)
    return path


def _read_bytes_if_exists(path):
    return path.read_bytes() if path.exists() else b""


def _square_bounds(width, height, center_re, center_im, scale):
    half_w_world = (float(width) / 2.0) / float(scale)
    half_h_world = (float(height) / 2.0) / float(scale)
    return {
        "min_re": center_re - half_w_world,
        "max_re": center_re + half_w_world,
        "min_im": center_im - half_h_world,
        "max_im": center_im + half_h_world,
    }


def _compile_binary(tmpdir, name, source, extra_sources=None, libs=None):
    cc = _compiler()
    if not cc:
        pytest.skip("no C compiler available")
    out = pathlib.Path(tmpdir) / name
    cmd = [cc, "-O2", "-I", str(LAMBDA_DIR), str(LAMBDA_DIR / source)]
    for extra in extra_sources or []:
        cmd.append(str(LAMBDA_DIR / extra))
    cmd.extend(libs or [])
    cmd.extend(["-o", str(out)])
    result = subprocess.run(cmd, capture_output=True, text=True)
    assert result.returncode == 0, f"failed to compile {source}: {result.stderr}"
    return out


def test_roots2pix_mt_square_bounds_matches_legacy_camera():
    with tempfile.TemporaryDirectory(prefix="exact_viewport_roots2pix_") as td:
        root = pathlib.Path(td)
        binary = _compile_binary(
            td,
            "roots2pix_mt_test",
            "roots2pix_mt.c",
            extra_sources=["multispan_reader.c"],
            libs=["-lcurl", "-lm", "-lpthread"],
        )
        roots_path = _write_float_file(
            root / "roots.bin",
            [
                -1.0, -1.0,
                0.0, 0.0,
                1.5, 2.0,
                3.0, -2.5,
            ],
        )
        width = height = 8
        camera = _square_bounds(width, height, 0.0, 0.0, 1.0)
        legacy_prefix = root / "legacy_pix"
        legacy_pixbin = root / "legacy_pixbin"
        bounds_prefix = root / "bounds_pix"
        bounds_pixbin = root / "bounds_pixbin"
        common = [
            "--width=8",
            "--height=8",
            "--tile_size=8",
            "--n_tile_cols=1",
            "--n_tile_rows=1",
            "--degree=1",
            "--color=solve_score",
            "--match=none",
            "--palette=inferno",
            "--rotation=0",
            "--threads=1",
            "--input_mode=tmpfile",
            "--solve_metric=centroid_re",
            "--solve_score_clip_lo=-4",
            "--solve_score_clip_hi=4",
            "--solve_score_cuts=0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9",
            "--solve_score_raw_bytes=1",
        ]

        legacy = subprocess.run(
            [
                str(binary),
                str(roots_path),
                str(legacy_prefix),
                *common,
                "--center_re=0",
                "--center_im=0",
                "--scale=1",
                f"--pixel_bin_prefix={legacy_pixbin}",
            ],
            capture_output=True,
            text=True,
            timeout=30,
        )
        assert legacy.returncode == 0, legacy.stderr

        bounds = subprocess.run(
            [
                str(binary),
                str(roots_path),
                str(bounds_prefix),
                *common,
                f"--min_re={camera['min_re']}",
                f"--max_re={camera['max_re']}",
                f"--min_im={camera['min_im']}",
                f"--max_im={camera['max_im']}",
                f"--pixel_bin_prefix={bounds_pixbin}",
            ],
            capture_output=True,
            text=True,
            timeout=30,
        )
        assert bounds.returncode == 0, bounds.stderr

        legacy_meta = json.loads(legacy.stdout)
        bounds_meta = json.loads(bounds.stdout)
        for key in ("roots_plotted", "roots_clipped"):
            assert legacy_meta[key] == bounds_meta[key]
        assert _read_bytes_if_exists(root / "legacy_pixbin.frag") == _read_bytes_if_exists(root / "bounds_pixbin.frag")


def test_bilevel_section_square_bounds_matches_legacy_camera():
    with tempfile.TemporaryDirectory(prefix="exact_viewport_bilevel_section_") as td:
        root = pathlib.Path(td)
        binary = _compile_binary(td, "bilevel_section_raster_test", "bilevel_section_raster.c", libs=["-lm"])
        section_path = _write_float_file(
            root / "section.bin",
            [
                0.0, 0.0,
                1.0, 0.0,
                0.0, 0.0,
                100.0, 100.0,
                -4.25, 0.0,
                0.0, 0.0,
            ],
        )
        width = height = 8
        camera = _square_bounds(width, height, 0.0, 0.0, 1.0)
        legacy_frag = root / "legacy.frag"
        bounds_frag = root / "bounds.frag"
        common = [
            "--width=8",
            "--height=8",
            "--degree=2",
        ]

        legacy = subprocess.run(
            [
                str(binary),
                str(section_path),
                str(legacy_frag),
                *common,
                "--center_re=0",
                "--center_im=0",
                "--scale=1",
            ],
            capture_output=True,
            text=True,
            timeout=30,
        )
        assert legacy.returncode == 0, legacy.stderr

        bounds = subprocess.run(
            [
                str(binary),
                str(section_path),
                str(bounds_frag),
                *common,
                f"--min_re={camera['min_re']}",
                f"--max_re={camera['max_re']}",
                f"--min_im={camera['min_im']}",
                f"--max_im={camera['max_im']}",
            ],
            capture_output=True,
            text=True,
            timeout=30,
        )
        assert bounds.returncode == 0, bounds.stderr

        legacy_meta = json.loads(legacy.stdout)
        bounds_meta = json.loads(bounds.stdout)
        for key in ("roots_plotted", "roots_clipped", "roots_deduped"):
            assert legacy_meta[key] == bounds_meta[key]
        assert legacy_frag.read_bytes() == bounds_frag.read_bytes()


def test_coeffs_bilevel_square_bounds_matches_legacy_camera():
    with tempfile.TemporaryDirectory(prefix="exact_viewport_coeffs_bilevel_") as td:
        root = pathlib.Path(td)
        binary = _compile_binary(td, "coeffs_bilevel_raster_test", "coeffs_bilevel_raster.c", libs=["-lm"])
        coeffs_path = _write_float_file(
            root / "coeffs.bin",
            [
                -1.0, -1.0,
                0.0, 0.0,
                1.0, 1.0,
                2.0, -2.0,
                10.0, 10.0,
                -10.0, -10.0,
            ],
        )
        width = height = 16
        camera = _square_bounds(width, height, 0.0, 0.0, 2.0)
        legacy_prefix = root / "legacy_bits"
        bounds_prefix = root / "bounds_bits"
        common = [
            "--width=16",
            "--height=16",
            "--tile_size=16",
            "--n_tile_cols=1",
            "--n_tile_rows=1",
            "--n_coeffs=1",
        ]

        legacy = subprocess.run(
            [
                str(binary),
                str(coeffs_path),
                str(legacy_prefix),
                *common,
                "--center_re=0",
                "--center_im=0",
                "--scale=2",
            ],
            capture_output=True,
            text=True,
            timeout=30,
        )
        assert legacy.returncode == 0, legacy.stderr

        bounds = subprocess.run(
            [
                str(binary),
                str(coeffs_path),
                str(bounds_prefix),
                *common,
                f"--min_re={camera['min_re']}",
                f"--max_re={camera['max_re']}",
                f"--min_im={camera['min_im']}",
                f"--max_im={camera['max_im']}",
            ],
            capture_output=True,
            text=True,
            timeout=30,
        )
        assert bounds.returncode == 0, bounds.stderr

        legacy_meta = json.loads(legacy.stdout)
        bounds_meta = json.loads(bounds.stdout)
        for key in ("roots_plotted", "roots_clipped", "roots_deduped"):
            assert legacy_meta[key] == bounds_meta[key]
        assert _read_bytes_if_exists(root / "legacy_bits_t0000.bits") == _read_bytes_if_exists(root / "bounds_bits_t0000.bits")
