import base64
import json
from argparse import Namespace
from io import BytesIO
from pathlib import Path

import scripts.gemini_generate_title_description as mod


def _write_png(path: Path) -> None:
    png_b64 = (
        "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8"
        "/w8AAn8B9pLxWQAAAABJRU5ErkJggg=="
    )
    path.write_bytes(base64.b64decode(png_b64))


def _args(tmp_path: Path) -> Namespace:
    image = tmp_path / "sample.png"
    code = tmp_path / "sample.py"
    config = tmp_path / "config.json"
    _write_png(image)
    code.write_text("def poly(t1, t2):\n    return t1 + t2\n", encoding="utf-8")
    config.write_text(
        json.dumps(
            {
                "pages": [
                    {"title": "Amber Nautilus", "text": "A shell-like spiral in reef tones."},
                    {"title": "Rust Iris", "text": "A burning center ringed by cool teal."},
                ]
            }
        ),
        encoding="utf-8",
    )
    return Namespace(
        image=str(image),
        code=[str(code)],
        symbol=None,
        coeff_catalog=mod.fetch_function_mod.DEFAULT_COEFF_CATALOG,
        meta=None,
        meta_path=None,
        meta_data={},
        model="gemini-3-pro-preview",
        temperature=0.4,
        max_side=2048,
        jpeg_quality=90,
        examples_config=str(config),
        examples=2,
        code_max_chars=2000,
        subject=None,
        notes=None,
        out=None,
        api_key_env="GEMINI_API_KEY",
        api_key_file="gemini_key.txt",
        timeout_seconds=60,
        dry_run=False,
    )


def test_build_prompt_text_includes_code_and_examples(tmp_path):
    args = _args(tmp_path)
    code_blocks = mod._read_code_sources(args.code, args.code_max_chars)
    examples = mod._load_style_examples(Path(args.examples_config), args.examples)
    prompt = mod._build_prompt_text(Path(args.image), code_blocks, args, mod._build_examples_text(examples))
    assert "Code source:" in prompt
    assert "Amber Nautilus" in prompt
    assert "sample.png" in prompt


def test_build_prompt_text_includes_meta_summary_when_present(tmp_path):
    args = _args(tmp_path)
    args.meta_path = "snaps/example_meta.json"
    args.meta_data = {
        "artifact_id": "color_run_x",
        "palette": "reef",
        "solve_metric": "clusteriness",
        "compute": {"function": "poly_8", "degree": 50},
    }
    code_blocks = mod._read_code_sources(args.code, args.code_max_chars)
    prompt = mod._build_prompt_text(Path(args.image), code_blocks, args, "")
    assert "Snap metadata file: snaps/example_meta.json" in prompt
    assert "Artifact ID: color_run_x" in prompt
    assert "Function: poly_8" in prompt


def test_parse_model_json_text():
    parsed = mod.parse_model_json_text(
        json.dumps({"title": "Corroded Annulus", "description": "A weathered ring floats in darkness."})
    )
    assert parsed["title"] == "Corroded Annulus"


def test_format_result_text_outputs_plain_text():
    text = mod.format_result_text(
        {"title": "Corroded Annulus", "description": "A weathered ring floats in darkness."}
    )
    assert text == "Corroded Annulus\n\nA weathered ring floats in darkness.\n"


def test_normalize_args_supports_symbol_only_lookup():
    parser = mod.build_arg_parser()
    args = parser.parse_args(["foo.jpeg", "--symbol", "giga_45"])
    args = mod.normalize_args(args, parser)
    assert args.image == "foo.jpeg"
    assert args.code == []


def test_resolve_image_from_meta_uses_image_file(tmp_path):
    meta_path = tmp_path / "example_meta.json"
    image_path = tmp_path / "example.jpeg"
    image_path.write_bytes(b"jpeg")
    resolved = mod._resolve_image_from_meta(meta_path, {"image_file": "example.jpeg"})
    assert resolved == image_path


def test_normalize_args_accepts_meta_file_and_extracts_symbol(tmp_path):
    image_path = tmp_path / "compute_x_color_run_1.jpeg"
    image_path.write_bytes(b"jpeg")
    meta_path = tmp_path / "compute_x_color_run_1_meta.json"
    meta_path.write_text(
        json.dumps(
            {
                "image_file": image_path.name,
                "compute": {"function": "giga_45"},
            }
        ),
        encoding="utf-8",
    )
    parser = mod.build_arg_parser()
    args = parser.parse_args([str(meta_path)])
    args = mod.normalize_args(args, parser)
    assert args.image == str(image_path)
    assert args.symbol == "giga_45"
    assert args.meta_path == str(meta_path)


def test_read_code_sources_uses_fetch_function_for_symbol():
    blocks = mod._read_code_sources([], 4000, "giga_45", mod.fetch_function_mod.DEFAULT_COEFF_CATALOG)
    assert blocks[0][0] == "lambda/giga.py:poly_giga_45"
    assert "def poly_giga_45" in blocks[0][1]


def test_resolve_api_key_prefers_env(tmp_path, monkeypatch):
    args = _args(tmp_path)
    monkeypatch.setenv("GEMINI_API_KEY", "abc123")
    assert mod._resolve_api_key(args) == "abc123"


def test_resolve_api_key_uses_file_when_env_missing(tmp_path, monkeypatch):
    args = _args(tmp_path)
    key_file = tmp_path / "gemini_key.txt"
    key_file.write_text("xyz789\n", encoding="utf-8")
    args.api_key_file = str(key_file)
    monkeypatch.delenv("GEMINI_API_KEY", raising=False)
    assert mod._resolve_api_key(args) == "xyz789"


def test_prepare_image_for_model_downscales_large_png(tmp_path):
    Image = mod._import_pil_image()
    image_path = tmp_path / "big.png"
    img = Image.new("RGB", (4000, 3000), color=(10, 20, 30))
    img.save(image_path, format="PNG")
    data, mime, info = mod._prepare_image_for_model(image_path, max_side=1024, jpeg_quality=85)
    assert mime == "image/jpeg"
    assert info["transformed"] is True
    assert info["prepared_width"] <= 1024
    assert info["prepared_height"] <= 1024
    Image = mod._import_pil_image()
    with Image.open(BytesIO(data)) as out:
        assert out.width == info["prepared_width"]
        assert out.height == info["prepared_height"]
