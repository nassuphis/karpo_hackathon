#!/usr/bin/env python3
"""
Add or update a single entry in polypaint_book_config.json from a snap meta file.

Examples:

  python3 scripts/update_polybook.py \
    snaps/compute_mn3m0d9d_color_run_1775888715163_99401o_meta.json

  python3 scripts/update_polybook.py \
    snaps/compute_mn3m0d9d_color_run_1775888715163_99401o_meta.json \
    --refresh-text

  python3 scripts/update_polybook.py \
    snaps/compute_mn3m0d9d_color_run_1775888715163_99401o_meta.json \
    --dry-run
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

try:
    import gemini_generate_title_description as gemini_mod
except ImportError:
    from scripts import gemini_generate_title_description as gemini_mod

DEFAULT_CONFIG = "polypaint_book_config.json"
META_SUFFIX = "_meta.json"


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def resolve_image_path(meta_path: Path, meta: dict) -> Path:
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
        tried = ", ".join(str(path) for path in candidates)
        raise ValueError(f"could not find local image for {meta_path}; tried: {tried}")
    raise ValueError(f"metadata file {meta_path} does not contain enough information to resolve an image")


def solver_tag(solver):
    if solver == "companion_matrix":
        return "Companion Matrix"
    if solver == "aberth_mt":
        return "Aberth-Ehrlich (parallel)"
    if solver in ("aberth", ""):
        return "Aberth-Ehrlich"
    return solver


def color_description(meta):
    color_mode = str(meta.get("color_mode", "") or "").strip()
    palette = str(meta.get("palette", "") or "").replace("tri_", "").replace("long_", "").replace("_", " ")
    metric = str(meta.get("solve_metric", "") or "").replace("_", " ")
    if color_mode == "solve_score":
        return f"Colored by {metric} score, {palette} palette"
    if color_mode == "proximity":
        return f"Root proximity coloring, {palette} palette"
    if color_mode == "constant":
        return "Uniform coloring"
    return "Index rainbow coloring"


def title_from_meta(meta):
    compute = meta.get("compute", {})
    fn = compute.get("function", "")
    if not fn:
        return "Title"
    fn_short = fn.replace("poly_", "").replace("giga_", "g")
    return f"Study {fn_short}"


def text_from_meta(meta):
    compute = meta.get("compute", {})
    fn = compute.get("function", "")
    pt = compute.get("param_transforms", "none")
    ct = compute.get("coeff_transforms", "none")
    cfpv = compute.get("cfpv", "")
    degree = compute.get("degree", meta.get("degree", ""))
    n_val = compute.get("N", "")
    times = compute.get("times", 1)
    solver = compute.get("solver", "")
    total_roots = compute.get("total_roots", "")

    lines = []
    fn_str = f"{fn}({cfpv})" if cfpv else fn
    if fn_str:
        lines.append(fn_str)
    if pt and pt != "none":
        lines.append(f"Parameters swept through {pt}")
    if ct and ct != "none":
        lines.append(f"Coefficients transformed by {ct}")

    scale_parts = []
    if degree:
        scale_parts.append(f"degree {degree}")
    if n_val:
        grid = f"{n_val}×{n_val}"
        if times and int(times) > 1:
            grid += f"×{times}"
        scale_parts.append(f"{grid} grid")
    if total_roots:
        try:
            scale_parts.append(f"{int(total_roots):,} roots")
        except (ValueError, TypeError):
            pass
    if scale_parts:
        lines.append(", ".join(scale_parts))

    color_desc = color_description(meta)
    if color_desc:
        lines.append(color_desc)
    stag = solver_tag(solver)
    if stag:
        lines.append(f"Solved by {stag}")

    body = "\n".join(lines) if lines else ""
    filename = meta.get("artifact_id", "")
    job_id = meta.get("job_id", "")
    title = title_from_meta(meta)
    return title, body, filename, job_id


def relative_posix(path: Path, start: Path) -> str:
    return path.relative_to(start).as_posix()


def build_entry(meta_path: Path, config_path: Path, refresh_text: bool = False) -> dict:
    meta = load_json(meta_path)
    image_path = resolve_image_path(meta_path, meta)
    title, body, filename, job_id = text_from_meta(meta)
    entry = {
        "image": relative_posix(image_path, config_path.parent),
        "title": title,
        "text": body,
        "filename": filename,
        "job_id": job_id,
    }
    if refresh_text:
        entry["_refresh_text"] = True
    return entry


def find_existing_index(pages: list[dict], entry: dict) -> int | None:
    filename = entry.get("filename") or ""
    job_id = entry.get("job_id") or ""
    image_name = Path(entry.get("image", "")).name
    for idx, page in enumerate(pages):
        page_filename = page.get("filename") or ""
        page_job_id = page.get("job_id") or ""
        page_image_name = Path(page.get("image", "")).name
        if filename and page_filename == filename:
            return idx
        if filename and job_id and page_filename == filename and page_job_id == job_id:
            return idx
        if image_name and page_image_name == image_name:
            return idx
    return None


def merge_entry(existing: dict | None, incoming: dict, refresh_text: bool) -> dict:
    merged = dict(existing or {})
    merged["image"] = incoming["image"]
    merged["filename"] = incoming["filename"]
    merged["job_id"] = incoming["job_id"]
    if refresh_text or not (merged.get("title") or "").strip():
        merged["title"] = incoming["title"]
    if refresh_text or not (merged.get("text") or "").strip():
        merged["text"] = incoming["text"]
    return merged


def should_generate_text(existing: dict | None, refresh_text: bool) -> bool:
    if existing is None:
        return True
    if refresh_text:
        return True
    if not (existing.get("title") or "").strip():
        return True
    if not (existing.get("text") or "").strip():
        return True
    return False


def build_gemini_args(
    meta_path: Path,
    model: str,
    temperature: float,
    examples_config: str,
    examples: int,
    api_key_env: str,
    api_key_file: str,
) -> argparse.Namespace:
    parser = gemini_mod.build_arg_parser()
    argv = [
        str(meta_path),
        "--model",
        model,
        "--temperature",
        str(temperature),
        "--examples-config",
        examples_config,
        "--examples",
        str(examples),
        "--api-key-env",
        api_key_env,
        "--api-key-file",
        api_key_file,
    ]
    return gemini_mod.normalize_args(parser.parse_args(argv), parser)


def generate_title_text_with_gemini(
    meta_path: Path,
    *,
    model: str,
    temperature: float,
    examples_config: str,
    examples: int,
    api_key_env: str,
    api_key_file: str,
) -> dict[str, str]:
    args = build_gemini_args(
        meta_path,
        model=model,
        temperature=temperature,
        examples_config=examples_config,
        examples=examples,
        api_key_env=api_key_env,
        api_key_file=api_key_file,
    )
    return gemini_mod.generate_result(args)


def update_config(
    config_path: Path,
    meta_path: Path,
    refresh_text: bool = False,
    use_gemini: bool = True,
    text_generator=generate_title_text_with_gemini,
    *,
    gemini_model: str = gemini_mod.DEFAULT_MODEL,
    gemini_temperature: float = 0.4,
    gemini_examples_config: str = gemini_mod.DEFAULT_STYLE_CONFIG,
    gemini_examples: int = gemini_mod.DEFAULT_EXAMPLE_COUNT,
    gemini_api_key_env: str = "GEMINI_API_KEY",
    gemini_api_key_file: str = gemini_mod.DEFAULT_KEY_FILE,
) -> tuple[str, dict]:
    config = load_json(config_path)
    pages = list(config.get("pages") or [])
    incoming = build_entry(meta_path, config_path, refresh_text=refresh_text)
    idx = find_existing_index(pages, incoming)
    existing = pages[idx] if idx is not None else None
    clean_incoming = {k: v for k, v in incoming.items() if not k.startswith("_")}
    if use_gemini and should_generate_text(existing, refresh_text):
        generated = text_generator(
            meta_path,
            model=gemini_model,
            temperature=gemini_temperature,
            examples_config=gemini_examples_config,
            examples=gemini_examples,
            api_key_env=gemini_api_key_env,
            api_key_file=gemini_api_key_file,
        )
        clean_incoming["title"] = generated["title"]
        clean_incoming["text"] = generated["description"]
    if idx is None:
        pages.append(clean_incoming)
        action = "added"
        final_entry = clean_incoming
    else:
        final_entry = merge_entry(existing, clean_incoming, refresh_text)
        pages[idx] = final_entry
        action = "updated"
    config["pages"] = pages
    config_path.write_text(json.dumps(config, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    return action, final_entry


def preview_update(
    config_path: Path,
    meta_path: Path,
    refresh_text: bool = False,
    use_gemini: bool = True,
) -> dict:
    config = load_json(config_path)
    pages = list(config.get("pages") or [])
    incoming = build_entry(meta_path, config_path, refresh_text=refresh_text)
    idx = find_existing_index(pages, incoming)
    existing = pages[idx] if idx is not None else None
    clean_incoming = {k: v for k, v in incoming.items() if not k.startswith("_")}
    if idx is None:
        action = "added"
        final_entry = clean_incoming
    else:
        action = "updated"
        final_entry = merge_entry(existing, clean_incoming, refresh_text)
    return {
        "action": action,
        "entry": final_entry,
        "would_generate_text": bool(use_gemini and should_generate_text(existing, refresh_text)),
        "text_generation_engine": "gemini" if use_gemini else "metadata-defaults",
    }


def build_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("meta_file", help="Path to a snap *_meta.json file.")
    parser.add_argument("--config", default=DEFAULT_CONFIG, help=f"Book config path. Default: {DEFAULT_CONFIG}")
    parser.add_argument(
        "--refresh-text",
        action="store_true",
        help="Overwrite existing title/text from meta-derived defaults instead of preserving edited text.",
    )
    parser.add_argument(
        "--no-gemini",
        action="store_true",
        help="Do not generate title/text with Gemini; use metadata-derived defaults only.",
    )
    parser.add_argument("--gemini-model", default=gemini_mod.DEFAULT_MODEL)
    parser.add_argument("--gemini-temperature", type=float, default=0.4)
    parser.add_argument("--gemini-examples-config", default=gemini_mod.DEFAULT_STYLE_CONFIG)
    parser.add_argument("--gemini-examples", type=int, default=gemini_mod.DEFAULT_EXAMPLE_COUNT)
    parser.add_argument("--gemini-api-key-env", default="GEMINI_API_KEY")
    parser.add_argument("--gemini-api-key-file", default=gemini_mod.DEFAULT_KEY_FILE)
    parser.add_argument("--dry-run", action="store_true", help="Print the would-be change without writing the config.")
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_arg_parser()
    args = parser.parse_args(argv)
    meta_path = Path(args.meta_file)
    config_path = Path(args.config)
    if not meta_path.exists():
        parser.error(f"meta file not found: {meta_path}")
    if not config_path.exists():
        parser.error(f"config file not found: {config_path}")

    if args.dry_run:
        preview = preview_update(
            config_path,
            meta_path,
            refresh_text=args.refresh_text,
            use_gemini=not args.no_gemini,
        )
        print(json.dumps(preview, indent=2, ensure_ascii=False))
        return 0

    action, entry = update_config(
        config_path,
        meta_path,
        refresh_text=args.refresh_text,
        use_gemini=not args.no_gemini,
        gemini_model=args.gemini_model,
        gemini_temperature=args.gemini_temperature,
        gemini_examples_config=args.gemini_examples_config,
        gemini_examples=args.gemini_examples,
        gemini_api_key_env=args.gemini_api_key_env,
        gemini_api_key_file=args.gemini_api_key_file,
    )
    print(f"{action}: {entry['image']}")
    print(f"filename={entry.get('filename','')}")
    print(f"job_id={entry.get('job_id','')}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
