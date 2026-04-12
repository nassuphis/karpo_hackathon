#!/usr/bin/env python3
"""
Generate a title + description from an image and code context using the
OpenAI Responses API.

This script is intentionally dependency-light:
- no `openai` package
- no `requests`
- pure stdlib HTTP + JSON

Usage examples:

  OPENAI_API_KEY=... ./scripts/oai_generate_title_description.py \
    snaps/foo.jpeg lambda/giga.py

  OPENAI_API_KEY=... ./scripts/oai_generate_title_description.py \
    snaps/foo.jpeg lambda/giga.py \
    --notes "This is for the coffee table book."

  ./scripts/oai_generate_title_description.py \
    snaps/foo.jpeg lambda/giga.py \
    --dry-run
"""

from __future__ import annotations

import argparse
import base64
import json
import mimetypes
import os
import sys
import textwrap
import urllib.error
import urllib.request
from pathlib import Path

try:
    from . import fetch_function as fetch_function_mod
except ImportError:
    import fetch_function as fetch_function_mod


DEFAULT_MODEL = "gpt-5-mini"
DEFAULT_API_URL = "https://api.openai.com/v1/responses"
DEFAULT_TIMEOUT_SECONDS = 180
DEFAULT_CODE_MAX_CHARS = 12000
DEFAULT_EXAMPLE_COUNT = 4
DEFAULT_STYLE_CONFIG = "polypaint_book_config.json"


def _guess_mime_type(path: Path) -> str:
    mime, _ = mimetypes.guess_type(str(path))
    if mime:
        return mime
    suffix = path.suffix.lower()
    if suffix in {".jpg", ".jpeg"}:
        return "image/jpeg"
    if suffix == ".png":
        return "image/png"
    if suffix == ".webp":
        return "image/webp"
    raise ValueError(f"unsupported image type for {path}")


def _repo_root() -> Path:
    return Path(__file__).resolve().parent.parent

def _read_code_source(path_str: str, max_chars: int, symbol: str | None = None) -> tuple[str, str]:
    if path_str == "-":
        text = sys.stdin.read()
        label = "<stdin>"
    else:
        path = Path(path_str)
        text = path.read_text(encoding="utf-8")
        label = str(path)
    if len(text) > max_chars:
        text = text[:max_chars]
        text += f"\n\n# [truncated to {max_chars} chars]"
    return label, text


def _read_code_sources(
    path_list: list[str],
    max_chars: int,
    symbol: str | None = None,
    catalog_path: str | None = None,
) -> list[tuple[str, str]]:
    if not symbol:
        return [_read_code_source(path_str, max_chars, None) for path_str in path_list]
    label, text = fetch_function_mod.fetch_function_from_code_sources(
        path_list,
        symbol,
        catalog_path or fetch_function_mod.DEFAULT_COEFF_CATALOG,
    )
    if len(text) > max_chars:
        text = text[:max_chars]
        text += f"\n\n# [truncated to {max_chars} chars]"
    return [(label, text)]


def _load_style_examples(config_path: Path, count: int) -> list[dict[str, str]]:
    if count <= 0 or not config_path.exists():
        return []
    data = json.loads(config_path.read_text(encoding="utf-8"))
    pages = data.get("pages") or []
    examples = []
    for page in pages:
        title = (page.get("title") or "").strip()
        text = (page.get("text") or "").strip()
        if not title or not text:
            continue
        examples.append({"title": title, "text": text})
        if len(examples) >= count:
            break
    return examples


def _build_examples_text(examples: list[dict[str, str]]) -> str:
    if not examples:
        return ""
    chunks = ["Style examples from the existing book config. Match the tone, not the literal content:"]
    for idx, example in enumerate(examples, start=1):
        chunks.append(
            f"\nExample {idx}\n"
            f"Title: {example['title']}\n"
            f"Description:\n{example['text']}"
        )
    return "\n".join(chunks)


def _build_instruction(args: argparse.Namespace, examples_text: str) -> str:
    base = """
    You are writing polished art-book metadata for a Polypaint image.

    Produce:
    1. a short title
    2. a descriptive paragraph

    Hard requirements:
    - Use BOTH the image and the supplied code/context.
    - Be concrete and visually specific.
    - Do not mention chatbots, AI, prompts, or "the model".
    - Do not invent technical facts not supported by the provided code/context.
    - Avoid empty artspeak and generic adjectives.
    - The title should be 2 to 6 words, vivid, and non-generic.
    - The description should be one compact paragraph, usually 90 to 180 words.
    - If the code contains named transforms, metrics, palettes, or solver details,
      you may use them, but only if they genuinely help explain the image.
    - Prefer explaining what the image looks like first, then how the code likely
      produced that structure.
    - Output JSON only.
    """
    if args.notes:
        base += f"\nExtra user guidance:\n{args.notes.strip()}\n"
    if examples_text:
        base += f"\n{examples_text}\n"
    return textwrap.dedent(base).strip()


def _build_user_text(
    image_path: Path,
    code_blocks: list[tuple[str, str]],
    args: argparse.Namespace,
) -> str:
    parts = [
        f"Image filename: {image_path.name}",
        f"Requested output mode: {args.output_mode}",
    ]
    if args.symbol:
        parts.append(f"Requested Python symbol: {args.symbol}")
    if args.subject:
        parts.append(f"Known subject or hint: {args.subject}")
    for label, text in code_blocks:
        parts.append(f"\nCode source: {label}\n```python\n{text}\n```")
    return "\n".join(parts)


def build_payload(args: argparse.Namespace) -> dict:
    image_path = Path(args.image)
    code_blocks = _read_code_sources(args.code, args.code_max_chars, args.symbol, args.coeff_catalog)
    examples = _load_style_examples(Path(args.examples_config), args.examples)
    examples_text = _build_examples_text(examples)
    instruction = _build_instruction(args, examples_text)
    mime_type = _guess_mime_type(image_path)
    image_b64 = base64.b64encode(image_path.read_bytes()).decode("ascii")

    payload = {
        "model": args.model,
        "instructions": instruction,
        "input": [
            {
                "role": "user",
                "content": [
                    {
                        "type": "input_text",
                        "text": _build_user_text(image_path, code_blocks, args),
                    },
                    {
                        "type": "input_image",
                        "image_url": f"data:{mime_type};base64,{image_b64}",
                        "detail": args.detail,
                    },
                ],
            }
        ],
        "text": {
            "format": {
                "type": "json_schema",
                "name": "art_title_description",
                "strict": True,
                "schema": {
                    "type": "object",
                    "additionalProperties": False,
                    "properties": {
                        "title": {
                            "type": "string",
                            "description": "Short vivid art title, 2 to 6 words.",
                        },
                        "description": {
                            "type": "string",
                            "description": "One polished descriptive paragraph.",
                        },
                    },
                    "required": ["title", "description"],
                },
            }
        },
    }
    if args.model.startswith("gpt-5"):
        payload["reasoning"] = {"effort": args.reasoning_effort}
    return payload


def _extract_text_from_response(data: dict) -> str:
    if isinstance(data.get("output_text"), str) and data["output_text"].strip():
        return data["output_text"].strip()

    texts = []
    for item in data.get("output", []):
        if item.get("type") != "message":
            continue
        for content in item.get("content", []):
            if content.get("type") in {"output_text", "text"}:
                text = content.get("text")
                if isinstance(text, str):
                    texts.append(text)
    joined = "\n".join(t for t in texts if t.strip()).strip()
    if joined:
        return joined
    raise ValueError("no textual output found in OpenAI response")


def parse_model_json(data: dict) -> dict[str, str]:
    text = _extract_text_from_response(data)
    parsed = json.loads(text)
    title = (parsed.get("title") or "").strip()
    description = (parsed.get("description") or "").strip()
    if not title or not description:
        raise ValueError("model JSON missing title or description")
    return {"title": title, "description": description}


def format_result_text(result: dict[str, str]) -> str:
    return f"{result['title'].strip()}\n\n{result['description'].strip()}\n"


def call_responses_api(api_key: str, payload: dict, timeout_seconds: int, api_url: str) -> dict:
    body = json.dumps(payload).encode("utf-8")
    req = urllib.request.Request(
        api_url,
        data=body,
        headers={
            "Authorization": f"Bearer {api_key}",
            "Content-Type": "application/json",
        },
        method="POST",
    )
    try:
        with urllib.request.urlopen(req, timeout=timeout_seconds) as resp:
            return json.loads(resp.read().decode("utf-8"))
    except urllib.error.HTTPError as exc:
        body_text = exc.read().decode("utf-8", errors="replace")
        raise RuntimeError(f"OpenAI API HTTP {exc.code}: {body_text}") from exc
    except urllib.error.URLError as exc:
        raise RuntimeError(f"OpenAI API request failed: {exc}") from exc


def build_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image_arg", nargs="?", help="Path to the image to analyze.")
    parser.add_argument("code_arg", nargs="?", help="Path to the code file that made the image.")
    parser.add_argument("--image", help="Path to the image to analyze.")
    parser.add_argument(
        "--code",
        action="append",
        help="Path to a code file to include. Repeatable. Use '-' to read code from stdin.",
    )
    parser.add_argument(
        "--symbol",
        help="Optional Python function name to extract from the code file before sending it.",
    )
    parser.add_argument(
        "--coeff-catalog",
        default=fetch_function_mod.DEFAULT_COEFF_CATALOG,
        help=f"Coefficient catalog JSON used for symbol-to-file lookup. Default: {fetch_function_mod.DEFAULT_COEFF_CATALOG}",
    )
    parser.add_argument("--model", default=DEFAULT_MODEL, help=f"Responses API model. Default: {DEFAULT_MODEL}")
    parser.add_argument(
        "--reasoning-effort",
        default="low",
        choices=["minimal", "low", "medium", "high"],
        help="GPT-5 reasoning effort. Ignored for non-GPT-5 models.",
    )
    parser.add_argument(
        "--detail",
        default="auto",
        choices=["low", "high", "auto"],
        help="Vision detail level for the image input.",
    )
    parser.add_argument(
        "--examples-config",
        default=DEFAULT_STYLE_CONFIG,
        help=f"JSON config to mine style examples from. Default: {DEFAULT_STYLE_CONFIG}",
    )
    parser.add_argument(
        "--examples",
        type=int,
        default=DEFAULT_EXAMPLE_COUNT,
        help=f"How many style examples to include. Default: {DEFAULT_EXAMPLE_COUNT}",
    )
    parser.add_argument(
        "--code-max-chars",
        type=int,
        default=DEFAULT_CODE_MAX_CHARS,
        help=f"Max chars per code input before truncation. Default: {DEFAULT_CODE_MAX_CHARS}",
    )
    parser.add_argument(
        "--output-mode",
        default="book",
        choices=["book", "catalog", "plain"],
        help="Writing mode hint. Currently only affects prompt wording.",
    )
    parser.add_argument("--subject", help="Optional known subject hint.")
    parser.add_argument("--notes", help="Optional extra style or content instructions.")
    parser.add_argument("--out", help="Optional output path. Defaults to stdout.")
    parser.add_argument(
        "--api-key-env",
        default="OPENAI_API_KEY",
        help="Environment variable containing the API key.",
    )
    parser.add_argument(
        "--api-url",
        default=DEFAULT_API_URL,
        help=f"Responses API URL. Default: {DEFAULT_API_URL}",
    )
    parser.add_argument(
        "--timeout-seconds",
        type=int,
        default=DEFAULT_TIMEOUT_SECONDS,
        help=f"HTTP timeout for the API call. Default: {DEFAULT_TIMEOUT_SECONDS}",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Do not call the API. Print the constructed payload summary instead.",
    )
    return parser


def normalize_args(args: argparse.Namespace, parser: argparse.ArgumentParser) -> argparse.Namespace:
    image = args.image or args.image_arg
    code_items = list(args.code or [])
    if args.code_arg:
        code_items.append(args.code_arg)
    if not image:
        parser.error("missing image path. Pass IMAGE or --image.")
    if not code_items and not args.symbol:
        parser.error("missing code path. Pass CODEFILE or use --symbol for catalog lookup.")
    args.image = image
    args.code = code_items
    return args


def main(argv: list[str] | None = None) -> int:
    parser = build_arg_parser()
    args = normalize_args(parser.parse_args(argv), parser)
    try:
        payload = build_payload(args)
    except ValueError as exc:
        parser.error(str(exc))

    if args.dry_run:
        summary = {
            "model": payload["model"],
            "has_reasoning": "reasoning" in payload,
            "image_detail": payload["input"][0]["content"][1]["detail"],
            "code_sources": args.code,
            "symbol": args.symbol,
            "examples_config": args.examples_config,
            "examples_count": args.examples,
            "instructions_preview": payload["instructions"][:600],
        }
        out_text = json.dumps(summary, indent=2) + "\n"
    else:
        api_key = os.environ.get(args.api_key_env)
        if not api_key:
            parser.error(f"missing API key in environment variable {args.api_key_env}")
        response_data = call_responses_api(
            api_key=api_key,
            payload=payload,
            timeout_seconds=args.timeout_seconds,
            api_url=args.api_url,
        )
        result = parse_model_json(response_data)
        out_text = format_result_text(result)

    if args.out:
        Path(args.out).write_text(out_text, encoding="utf-8")
    else:
        sys.stdout.write(out_text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
