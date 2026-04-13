from pathlib import Path

import scripts.fetch_function as mod


def test_extract_symbol_source_returns_only_named_function():
    text = (
        "def alpha():\n"
        "    return 1\n\n"
        "def beta(x):\n"
        "    return x * 2\n"
    )
    resolved, snippet = mod.extract_symbol_source(text, "beta")
    assert resolved == "beta"
    assert snippet == "def beta(x):\n    return x * 2\n"


def test_extract_symbol_source_accepts_poly_prefix_alias():
    text = "def poly_giga_45(t1, t2):\n    return t1 + t2\n"
    resolved, snippet = mod.extract_symbol_source(text, "giga_45")
    assert resolved == "poly_giga_45"
    assert "def poly_giga_45" in snippet


def test_extract_symbol_source_raises_when_missing():
    text = "def alpha():\n    return 1\n"
    try:
        mod.extract_symbol_source(text, "beta")
    except ValueError as exc:
        assert "symbol 'beta' not found" in str(exc)
    else:
        raise AssertionError("expected ValueError")


def test_resolve_function_search_paths_uses_catalog_python_source():
    paths = mod.resolve_function_search_paths(None, "giga_45", mod.DEFAULT_COEFF_CATALOG)
    assert "lambda/giga.py" in paths


def test_resolve_function_search_paths_falls_back_when_catalog_source_is_native():
    paths = mod.resolve_function_search_paths(None, "p11b3", mod.DEFAULT_COEFF_CATALOG)
    assert any(path in {"lambda/giga.py", "lambda/poly.py"} for path in paths)


def test_fetch_function_from_code_sources_searches_multiple_files(tmp_path):
    a = tmp_path / "a.py"
    b = tmp_path / "b.py"
    a.write_text("def alpha():\n    return 1\n", encoding="utf-8")
    b.write_text("def beta():\n    return 2\n", encoding="utf-8")
    label, text = mod.fetch_function_from_code_sources([str(a), str(b)], "beta")
    assert label.endswith("b.py:beta")
    assert "def beta():" in text


def test_fetch_function_from_code_sources_symbol_only_uses_catalog():
    label, text = mod.fetch_function_from_code_sources([], "giga_45", mod.DEFAULT_COEFF_CATALOG)
    assert label == "lambda/giga.py:poly_giga_45"
    assert "def poly_giga_45" in text
