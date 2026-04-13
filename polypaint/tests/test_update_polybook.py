import json
from pathlib import Path

import scripts.update_polybook as mod


def _write_config(path: Path, pages: list[dict]) -> None:
    path.write_text(
        json.dumps(
            {
                "title": "Complex Portraits",
                "snaps_dir": "snaps",
                "pages": pages,
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )


def _write_meta_and_image(tmp_path: Path, stem: str, function: str = "poly_52") -> Path:
    image_path = tmp_path / f"{stem}.jpeg"
    image_path.write_bytes(b"jpeg")
    meta_path = tmp_path / f"{stem}_meta.json"
    meta_path.write_text(
        json.dumps(
            {
                "artifact_id": stem.split("_", 1)[1],
                "job_id": stem.split("_", 2)[0] + "_" + stem.split("_", 2)[1],
                "image_file": image_path.name,
                "family": "color",
                "palette": "reef",
                "solve_metric": "clusteriness",
                "color_mode": "solve_score",
                "compute": {
                    "function": function,
                    "param_transforms": "unit_circle",
                    "coeff_transforms": "rev",
                    "degree": 70,
                    "N": 5000,
                    "times": 1,
                    "solver": "",
                    "total_roots": 1750000000,
                },
            }
        ),
        encoding="utf-8",
    )
    return meta_path


def test_preview_update_adds_new_entry(tmp_path):
    config_path = tmp_path / "book.json"
    _write_config(config_path, [])
    meta_path = _write_meta_and_image(tmp_path, "compute_abc_color_run_123")
    preview = mod.preview_update(config_path, meta_path)
    assert preview["action"] == "added"
    assert preview["entry"]["image"] == "compute_abc_color_run_123.jpeg"
    assert preview["entry"]["filename"] == "abc_color_run_123"
    assert preview["would_generate_text"] is True


def test_update_config_preserves_existing_text_by_default(tmp_path):
    config_path = tmp_path / "book.json"
    _write_config(
        config_path,
        [
            {
                "image": "old.jpeg",
                "title": "Edited Title",
                "text": "Edited body",
                "filename": "abc_color_run_123",
                "job_id": "compute_abc",
            }
        ],
    )
    meta_path = _write_meta_and_image(tmp_path, "compute_abc_color_run_123")
    called = {"count": 0}

    def fake_text_generator(*args, **kwargs):
        called["count"] += 1
        return {"title": "New Title", "description": "New body"}

    action, entry = mod.update_config(config_path, meta_path, text_generator=fake_text_generator)
    assert action == "updated"
    assert entry["title"] == "Edited Title"
    assert entry["text"] == "Edited body"
    assert entry["image"] == "compute_abc_color_run_123.jpeg"
    assert called["count"] == 0


def test_update_config_refresh_text_overwrites_existing_text(tmp_path):
    config_path = tmp_path / "book.json"
    _write_config(
        config_path,
        [
            {
                "image": "old.jpeg",
                "title": "Edited Title",
                "text": "Edited body",
                "filename": "abc_color_run_123",
                "job_id": "compute_abc",
            }
        ],
    )
    meta_path = _write_meta_and_image(tmp_path, "compute_abc_color_run_123")
    action, entry = mod.update_config(
        config_path,
        meta_path,
        refresh_text=True,
        text_generator=lambda *args, **kwargs: {
            "title": "Gemini Title",
            "description": "Gemini body",
        },
    )
    assert action == "updated"
    assert entry["title"] == "Gemini Title"
    assert entry["text"] == "Gemini body"


def test_resolve_image_path_uses_stem_fallback(tmp_path):
    meta_path = tmp_path / "compute_abc_color_run_123_meta.json"
    image_path = tmp_path / "compute_abc_color_run_123.jpeg"
    image_path.write_bytes(b"jpeg")
    resolved = mod.resolve_image_path(meta_path, {})
    assert resolved == image_path


def test_update_config_adds_new_entry_with_gemini_text(tmp_path):
    config_path = tmp_path / "book.json"
    _write_config(config_path, [])
    meta_path = _write_meta_and_image(tmp_path, "compute_abc_color_run_123")
    action, entry = mod.update_config(
        config_path,
        meta_path,
        text_generator=lambda *args, **kwargs: {
            "title": "Gemini Title",
            "description": "Gemini body",
        },
    )
    assert action == "added"
    assert entry["title"] == "Gemini Title"
    assert entry["text"] == "Gemini body"
