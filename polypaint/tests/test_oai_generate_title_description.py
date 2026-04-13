import base64
import json
from argparse import Namespace
from pathlib import Path

import scripts.oai_generate_title_description as mod


def _write_png(path: Path) -> None:
    # 1x1 transparent PNG
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
        model="gpt-5-mini",
        reasoning_effort="low",
        detail="auto",
        examples_config=str(config),
        examples=2,
        code_max_chars=2000,
        output_mode="book",
        subject=None,
        notes=None,
        out=None,
        api_key_env="OPENAI_API_KEY",
        api_url=mod.DEFAULT_API_URL,
        timeout_seconds=60,
        dry_run=False,
    )


def test_build_payload_includes_image_code_examples_and_schema(tmp_path):
    args = _args(tmp_path)
    payload = mod.build_payload(args)
    assert payload["model"] == "gpt-5-mini"
    assert payload["reasoning"] == {"effort": "low"}
    content = payload["input"][0]["content"]
    assert content[0]["type"] == "input_text"
    assert "Code source:" in content[0]["text"]
    assert "Amber Nautilus" in payload["instructions"]
    assert content[1]["type"] == "input_image"
    assert content[1]["image_url"].startswith("data:image/png;base64,")
    fmt = payload["text"]["format"]
    assert fmt["type"] == "json_schema"
    assert fmt["schema"]["required"] == ["title", "description"]


def test_parse_model_json_from_output_text():
    data = {
        "output_text": json.dumps(
            {"title": "Corroded Annulus", "description": "A weathered ring floats in darkness."}
        )
    }
    parsed = mod.parse_model_json(data)
    assert parsed["title"] == "Corroded Annulus"
    assert parsed["description"].startswith("A weathered ring")


def test_parse_model_json_from_message_content():
    data = {
        "output": [
            {
                "type": "message",
                "content": [
                    {
                        "type": "output_text",
                        "text": json.dumps(
                            {"title": "Golden Archipelago", "description": "Islands of gold drift in black."}
                        ),
                    }
                ],
            }
        ]
    }
    parsed = mod.parse_model_json(data)
    assert parsed["title"] == "Golden Archipelago"


def test_non_gpt5_payload_omits_reasoning(tmp_path):
    args = _args(tmp_path)
    args.model = "gpt-4.1"
    payload = mod.build_payload(args)
    assert "reasoning" not in payload


def test_build_payload_uses_symbol_extraction(tmp_path):
    args = _args(tmp_path)
    code_path = Path(args.code[0])
    code_path.write_text(
        "def alpha():\n    return 1\n\n"
        "def beta():\n    return 2\n",
        encoding="utf-8",
    )
    args.symbol = "beta"
    payload = mod.build_payload(args)
    user_text = payload["input"][0]["content"][0]["text"]
    assert "Requested Python symbol: beta" in user_text
    assert "def beta():" in user_text
    assert "def alpha():" not in user_text


def test_read_code_sources_symbol_searches_across_multiple_files(tmp_path):
    a = tmp_path / "a.py"
    b = tmp_path / "b.py"
    a.write_text("def alpha():\n    return 1\n", encoding="utf-8")
    b.write_text("def beta():\n    return 2\n", encoding="utf-8")
    blocks = mod._read_code_sources([str(a), str(b)], 2000, "beta")
    assert blocks[0][0].endswith("b.py:beta")
    assert "def beta():" in blocks[0][1]


def test_format_result_text_outputs_plain_text():
    text = mod.format_result_text(
        {"title": "Corroded Annulus", "description": "A weathered ring floats in darkness."}
    )
    assert text == "Corroded Annulus\n\nA weathered ring floats in darkness.\n"


def test_normalize_args_supports_positional_image_and_code():
    parser = mod.build_arg_parser()
    args = parser.parse_args(["foo.jpeg", "bar.py"])
    args = mod.normalize_args(args, parser)
    assert args.image == "foo.jpeg"
    assert args.code == ["bar.py"]


def test_normalize_args_supports_symbol_only_lookup():
    parser = mod.build_arg_parser()
    args = parser.parse_args(["foo.jpeg", "--symbol", "giga_45"])
    args = mod.normalize_args(args, parser)
    assert args.image == "foo.jpeg"
    assert args.code == []
