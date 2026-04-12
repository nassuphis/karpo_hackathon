#!/usr/bin/env python3
"""
Generate a title and description from an image plus code context using Gemini.

Examples:

  GEMINI_API_KEY=... ./scripts/gemini_generate_title_description.py \
    snaps/foo.jpeg --symbol giga_45

  GEMINI_API_KEY=... ./scripts/gemini_generate_title_description.py \
    snaps/compute_xxx_meta.json

  ./scripts/gemini_generate_title_description.py \
    snaps/foo.jpeg lambda/giga.py --symbol giga_45 --dry-run
"""

from __future__ import annotations

import argparse
import json
import mimetypes
import os
import sys
import textwrap
from pathlib import Path

try:
    from . import fetch_function as fetch_function_mod
except ImportError:
    import fetch_function as fetch_function_mod


DEFAULT_MODEL = "gemini-3-pro-preview"
DEFAULT_TIMEOUT_SECONDS = 180
DEFAULT_CODE_MAX_CHARS = 12000
DEFAULT_EXAMPLE_COUNT = 4
DEFAULT_STYLE_CONFIG = "polypaint_book_config.json"
DEFAULT_KEY_FILE = "gemini_key.txt"
META_SUFFIX = "_meta.json"


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


def _read_code_source(path_str: str, max_chars: int) -> tuple[str, str]:
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
        return [_read_code_source(path_str, max_chars) for path_str in path_list]
    label, text = fetch_function_mod.fetch_function_from_code_sources(
        path_list,
        symbol,
        catalog_path or fetch_function_mod.DEFAULT_COEFF_CATALOG,
    )
    if len(text) > max_chars:
        text = text[:max_chars]
        text += f"\n\n# [truncated to {max_chars} chars]"
    return [(label, text)]


def _load_snap_meta(meta_path: Path) -> dict:
    return json.loads(meta_path.read_text(encoding="utf-8"))


def _resolve_image_from_meta(meta_path: Path, meta: dict) -> Path:
    candidates = []
    image_file = meta.get("image_file")
    if image_file:
        candidates.append(meta_path.parent / image_file)
    if meta_path.name.endswith(META_SUFFIX):
        base = meta_path.name[: -len(META_SUFFIX)]
        for ext in (".jpeg", ".jpg", ".png", ".webp"):
            candidates.append(meta_path.parent / f"{base}{ext}")
    for candidate in candidates:
        if candidate.exists():
            return candidate
    if candidates:
        raise ValueError(
            f"could not find local image file for {meta_path}; tried: "
            + ", ".join(str(path) for path in candidates)
        )
    raise ValueError(f"metadata file {meta_path} does not contain enough information to resolve an image")


def _extract_symbol_from_meta(meta: dict) -> str | None:
    compute = meta.get("compute") or {}
    value = compute.get("function") or meta.get("function")
    if isinstance(value, str) and value.strip():
        return value.strip()
    return None


def _build_meta_summary(meta: dict) -> str:
    compute = meta.get("compute") or {}
    lines = []
    for label, value in (
        ("Artifact ID", meta.get("artifact_id")),
        ("Family", meta.get("family")),
        ("Color mode", meta.get("color_mode")),
        ("Palette", meta.get("palette")),
        ("Metric", meta.get("solve_metric")),
        ("Associated palette", meta.get("associated_palette_display_name") or meta.get("associated_palette_id")),
        ("Function", compute.get("function")),
        ("Parameter transforms", compute.get("param_transforms")),
        ("Coefficient transforms", compute.get("coeff_transforms")),
        ("Degree", compute.get("degree")),
        ("N", compute.get("N")),
        ("Solver", compute.get("solver")),
    ):
        if value not in (None, "", []):
            lines.append(f"{label}: {value}")
    return "\n".join(lines)


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
    - Return JSON with keys title and description.
    """
    if args.notes:
        base += f"\nExtra user guidance:\n{args.notes.strip()}\n"
    if examples_text:
        base += f"\n{examples_text}\n"
    return textwrap.dedent(base).strip()


def _build_prompt_text(
    image_path: Path,
    code_blocks: list[tuple[str, str]],
    args: argparse.Namespace,
    examples_text: str,
) -> str:
    parts = [_build_instruction(args, examples_text)]
    parts.append(f"\nImage filename: {image_path.name}")
    if getattr(args, "meta_path", None):
        parts.append(f"Snap metadata file: {args.meta_path}")
    meta_summary = _build_meta_summary(getattr(args, "meta_data", {}) or {})
    if meta_summary:
        parts.append(f"Snap metadata summary:\n{meta_summary}")
    if args.symbol:
        parts.append(f"Requested Python symbol: {args.symbol}")
    if args.subject:
        parts.append(f"Known subject or hint: {args.subject}")
    for label, text in code_blocks:
        parts.append(f"\nCode source: {label}\n```python\n{text}\n```")
    return "\n".join(parts)


def parse_model_json_text(text: str) -> dict[str, str]:
    parsed = json.loads(text)
    title = (parsed.get("title") or "").strip()
    description = (parsed.get("description") or "").strip()
    if not title or not description:
        raise ValueError("model JSON missing title or description")
    return {"title": title, "description": description}


def format_result_text(result: dict[str, str]) -> str:
    return f"{result['title'].strip()}\n\n{result['description'].strip()}\n"


def _resolve_api_key(args: argparse.Namespace) -> str:
    env_value = os.environ.get(args.api_key_env)
    if env_value:
        return env_value.strip()
    if args.api_key_file and Path(args.api_key_file).exists():
        return Path(args.api_key_file).read_text(encoding="utf-8").strip()
    raise ValueError(
        f"missing Gemini API key. Set {args.api_key_env} or provide {args.api_key_file}"
    )


def _import_genai():
    try:
        from google import genai
        from google.genai import types
    except ImportError as exc:
        raise RuntimeError(
            "google-genai is not installed in this environment. "
            "Install it in the venv before running this script."
        ) from exc
    return genai, types


def call_gemini(args: argparse.Namespace, prompt_text: str, image_path: Path) -> dict[str, str]:
    genai, types = _import_genai()
    client = genai.Client(api_key=_resolve_api_key(args))
    image_bytes = image_path.read_bytes()
    response = client.models.generate_content(
        model=args.model,
        contents=[
            prompt_text,
            types.Part.from_bytes(data=image_bytes, mime_type=_guess_mime_type(image_path)),
        ],
        config={
            "temperature": args.temperature,
            "response_mime_type": "application/json",
            "response_json_schema": {
                "type": "object",
                "properties": {
                    "title": {"type": "string", "description": "Short vivid art title, 2 to 6 words."},
                    "description": {"type": "string", "description": "One polished descriptive paragraph."},
                },
                "required": ["title", "description"],
                "additionalProperties": False,
            },
        },
    )
    text = getattr(response, "text", None)
    if not text:
        raise RuntimeError("Gemini returned no text output")
    return parse_model_json_text(text)


def build_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image_arg", nargs="?", help="Path to the image to analyze, or a *_meta.json snap file.")
    parser.add_argument("code_arg", nargs="?", help="Optional path to the Python code file.")
    parser.add_argument("--image", help="Path to the image to analyze.")
    parser.add_argument("--meta", help="Path to a snap metadata JSON file.")
    parser.add_argument("--code", action="append", help="Optional code file. Repeatable.")
    parser.add_argument("--symbol", help="Python function name to fetch from the codebase.")
    parser.add_argument(
        "--coeff-catalog",
        default=fetch_function_mod.DEFAULT_COEFF_CATALOG,
        help=f"Coefficient catalog JSON used for symbol-to-file lookup. Default: {fetch_function_mod.DEFAULT_COEFF_CATALOG}",
    )
    parser.add_argument("--model", default=DEFAULT_MODEL, help=f"Gemini model. Default: {DEFAULT_MODEL}")
    parser.add_argument("--temperature", type=float, default=0.4, help="Generation temperature.")
    parser.add_argument("--examples-config", default=DEFAULT_STYLE_CONFIG)
    parser.add_argument("--examples", type=int, default=DEFAULT_EXAMPLE_COUNT)
    parser.add_argument("--code-max-chars", type=int, default=DEFAULT_CODE_MAX_CHARS)
    parser.add_argument("--subject")
    parser.add_argument("--notes")
    parser.add_argument("--out")
    parser.add_argument("--api-key-env", default="GEMINI_API_KEY")
    parser.add_argument("--api-key-file", default=DEFAULT_KEY_FILE)
    parser.add_argument("--timeout-seconds", type=int, default=DEFAULT_TIMEOUT_SECONDS)
    parser.add_argument("--dry-run", action="store_true")
    return parser


def normalize_args(args: argparse.Namespace, parser: argparse.ArgumentParser) -> argparse.Namespace:
    meta_arg = args.meta
    image_positional = args.image_arg
    if not meta_arg and image_positional and image_positional.endswith(META_SUFFIX):
        meta_arg = image_positional
        image_positional = None

    image = args.image or image_positional
    code_items = list(args.code or [])
    if args.code_arg:
        code_items.append(args.code_arg)
    meta_data = {}
    if meta_arg:
        meta_path = Path(meta_arg)
        if not meta_path.exists():
            parser.error(f"metadata file not found: {meta_path}")
        try:
            meta_data = _load_snap_meta(meta_path)
        except Exception as exc:
            parser.error(f"failed to read metadata file {meta_path}: {exc}")
        if not image:
            try:
                image = str(_resolve_image_from_meta(meta_path, meta_data))
            except ValueError as exc:
                parser.error(str(exc))
        if not args.symbol:
            args.symbol = _extract_symbol_from_meta(meta_data)
        args.meta_path = str(meta_path)
        args.meta_data = meta_data
    else:
        args.meta_path = None
        args.meta_data = {}

    if not image:
        parser.error("missing image path. Pass IMAGE, --image, or a *_meta.json file.")
    if not code_items and not args.symbol:
        parser.error("missing code path or function symbol. Pass CODEFILE, use --symbol, or provide snap metadata with compute.function.")
    args.image = image
    args.code = code_items
    return args


def main(argv: list[str] | None = None) -> int:
    parser = build_arg_parser()
    args = normalize_args(parser.parse_args(argv), parser)
    image_path = Path(args.image)
    code_blocks = _read_code_sources(args.code, args.code_max_chars, args.symbol, args.coeff_catalog)
    examples = _load_style_examples(Path(args.examples_config), args.examples)
    examples_text = _build_examples_text(examples)
    prompt_text = _build_prompt_text(image_path, code_blocks, args, examples_text)

    if args.dry_run:
        out_text = json.dumps(
            {
                "model": args.model,
                "meta_path": args.meta_path,
                "resolved_image": args.image,
                "code_sources": args.code,
                "symbol": args.symbol,
                "examples_config": args.examples_config,
                "examples_count": args.examples,
                "prompt_preview": prompt_text[:700],
            },
            indent=2,
        ) + "\n"
    else:
        try:
            result = call_gemini(args, prompt_text, image_path)
        except (ValueError, RuntimeError) as exc:
            parser.error(str(exc))
        out_text = format_result_text(result)

    if args.out:
        Path(args.out).write_text(out_text, encoding="utf-8")
    else:
        sys.stdout.write(out_text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
