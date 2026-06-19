"""Shared CR18 whole-sweep oracle runner.

Used by both tests/test_whole_sweep_oracle.py and scripts/oracle_baseline.sh
so baseline generation and baseline verification execute the same pipeline.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile


ROOT = pathlib.Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"
FIXTURE_DIR = ROOT / "tests" / "fixtures" / "oracle"
sys.path.insert(0, str(LAMBDA_DIR))


def compile_sweep_binary(out_path: pathlib.Path):
    cc = os.environ.get("CC") or shutil.which("cc") or shutil.which("clang") or shutil.which("gcc")
    if not cc:
        raise RuntimeError("no C compiler available")
    cmd = [
        cc,
        "-O2",
        "-pthread",
        "-I",
        str(LAMBDA_DIR),
        str(LAMBDA_DIR / "sweep_cli.c"),
        "-lm",
        "-o",
        str(out_path),
    ]
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
    if result.returncode != 0:
        raise RuntimeError(f"failed to compile sweep oracle binary: {result.stderr}")
    return out_path


def _sha256_file(path: pathlib.Path):
    digest = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _run_json(binary: pathlib.Path, payload: dict, out_path: pathlib.Path):
    result = subprocess.run(
        [str(binary), str(out_path)],
        input=json.dumps(payload, sort_keys=True, separators=(",", ":")),
        capture_output=True,
        text=True,
        timeout=120,
    )
    if result.returncode != 0:
        raise RuntimeError(
            f"sweep oracle command failed ({payload.get('mode')}):\nSTDOUT: {result.stdout}\nSTDERR: {result.stderr}"
        )
    try:
        meta = json.loads(result.stdout.strip().splitlines()[-1])
    except Exception as exc:
        raise RuntimeError(f"sweep oracle command did not emit JSON metadata: {result.stdout!r}") from exc
    return meta


def _compile_coeff_program_source(payload: dict):
    source_text = str(payload.pop("coeff_program_source_text", "") or "").strip()
    if not source_text:
        return payload
    from coeff_program_source import compile_coeff_program_source

    compiled = compile_coeff_program_source(source_text)
    payload["coeff_program"] = {
        "version": compiled["version"],
        "fingerprint": compiled["fingerprint"],
        "display": compiled["display"],
        "stack_max": compiled["stack_max"],
        "token_count": compiled["token_count"],
        "scalar_expr_count": compiled["scalar_expr_count"],
        "uses_legacy_chain_equivalent": compiled["uses_legacy_chain_equivalent"],
        "tokens": compiled["tokens"],
        "scalar_exprs": compiled["scalar_exprs"],
    }
    return payload


def load_corpus(corpus_path: pathlib.Path = FIXTURE_DIR / "corpus.json"):
    with corpus_path.open("r", encoding="utf-8") as fh:
        corpus = json.load(fh)
    cases = []
    for entry in corpus["cases"]:
        with (corpus_path.parent / entry["program"]).open("r", encoding="utf-8") as fh:
            case = json.load(fh)
        case.setdefault("name", entry["name"])
        cases.append(case)
    return cases


def run_case(binary: pathlib.Path, case: dict, workdir: pathlib.Path):
    name = case["name"]
    coeffs_path = workdir / f"{name}.coeffs.bin"
    roots_path = workdir / f"{name}.roots.bin"
    coeffgen_payload = _compile_coeff_program_source(dict(case["coeffgen"]))
    coeff_meta = _run_json(binary, coeffgen_payload, coeffs_path)
    solve_payload = dict(case.get("solve") or {})
    solve_payload.update(
        {
            "mode": "solve",
            "coeffs_file": str(coeffs_path),
            "n_coeffs": int(coeff_meta["n_coeffs"]),
            "n1": int(coeffgen_payload.get("n1", 100)),
            "n2": int(coeffgen_payload.get("n2", 100)),
        }
    )
    solve_meta = _run_json(binary, solve_payload, roots_path)
    return {
        "name": name,
        "coeffs_sha256": _sha256_file(coeffs_path),
        "roots_sha256": _sha256_file(roots_path),
        "coeffs_bytes": coeffs_path.stat().st_size,
        "roots_bytes": roots_path.stat().st_size,
        "coeffgen_meta": coeff_meta,
        "solve_meta": solve_meta,
    }


def run_corpus(binary: pathlib.Path, cases=None, workdir: pathlib.Path | None = None):
    cases = cases or load_corpus()
    if workdir is None:
        with tempfile.TemporaryDirectory() as td:
            return run_corpus(binary, cases=cases, workdir=pathlib.Path(td))
    workdir.mkdir(parents=True, exist_ok=True)
    return {case["name"]: run_case(binary, case, workdir) for case in cases}


def write_baselines(results: dict, baseline_dir: pathlib.Path):
    baseline_dir.mkdir(parents=True, exist_ok=True)
    for name, result in results.items():
        manifest = {
            "version": 1,
            "name": name,
            "coeffs_sha256": result["coeffs_sha256"],
            "roots_sha256": result["roots_sha256"],
            "coeffs_bytes": result["coeffs_bytes"],
            "roots_bytes": result["roots_bytes"],
            "n_coeffs": result["coeffgen_meta"]["n_coeffs"],
            "degree": result["solve_meta"]["degree"],
        }
        with (baseline_dir / f"{name}.sha256.json").open("w", encoding="utf-8") as fh:
            json.dump(manifest, fh, indent=2, sort_keys=True)
            fh.write("\n")


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--binary", type=pathlib.Path)
    parser.add_argument("--compile-binary", type=pathlib.Path)
    parser.add_argument("--write-baselines", action="store_true")
    parser.add_argument("--baseline-dir", type=pathlib.Path, default=FIXTURE_DIR / "baseline")
    parser.add_argument("--workdir", type=pathlib.Path)
    args = parser.parse_args(argv)

    binary = args.binary
    if args.compile_binary:
        binary = compile_sweep_binary(args.compile_binary)
    if binary is None:
        raise SystemExit("--binary or --compile-binary is required")
    results = run_corpus(binary, workdir=args.workdir)
    if args.write_baselines:
        write_baselines(results, args.baseline_dir)
    print(json.dumps(results, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
