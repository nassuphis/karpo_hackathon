#!/usr/bin/env python3
"""VM performance benchmark tool (code-review-31 F0).

Measures the four deployed program runtimes the way production runs them:

  - Param VM        (param_gen, buffered + thread sweep)
  - Coeff VM        (coeffgen buffered AND the production coeffgen_chunked path)
  - Root Transform  (direct C microbench, tests/native/vm_microbench.c)
  - Solve Score     (direct C microbench: stack VM vs metric extraction)

plus the harvested real program `compute_mqlacwaq` — the representative case
for prepared-plan work.

Rules (from the review):
  - compiles with the DEPLOYMENT optimization level (-O3), not the -O2 oracle
    runner default;
  - every macro case hashes its complete output file: reps must agree, and an
    A/B comparison must be byte-identical or it FAILS loudly;
  - A/B runs are interleaved (base, cand, base, cand, ...) to defeat drift;
  - reports carry git SHA + dirty state, compiler, flags, arch, CPU count;
  - stats are median / min / max / MAD — never a single best run;
  - timing is ADVISORY: this tool is not part of the predeploy pass/fail gate
    (byte/parity/oracle pytest suites stay mandatory there).

Usage:
  python3 scripts/bench_program_vms.py                       # full matrix
  python3 scripts/bench_program_vms.py --cases 'coeff*'      # subset
  python3 scripts/bench_program_vms.py --reps 5 --no-micro   # quick pass
  python3 scripts/bench_program_vms.py --compare /tmp/base /tmp/cand --cases mqlacwaq_coeff
"""
import argparse
import fnmatch
import hashlib
import json
import os
import pathlib
import platform
import statistics
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "lambda"))
sys.path.insert(0, str(ROOT / "tests"))

from bench_vm import _compiled_coeff_program_payload, _compiled_param_program_payload, _run_json  # noqa: E402

CC_FLAGS = ["-O3", "-pthread", "-I", "lambda"]
MICRO_SRC = ROOT / "tests" / "native" / "vm_microbench.c"
HARVESTED = ROOT / "tests" / "fixtures" / "program-m3-oracle" / "harvested" / "compute_mqlacwaq.calc.json"


def sh(cmd, **kw):
    return subprocess.run(cmd, check=True, capture_output=True, text=True, **kw)


def build_binary(out_path):
    sh(["cc", *CC_FLAGS, str(ROOT / "lambda" / "sweep_cli.c"), "-lm", "-o", str(out_path)], cwd=ROOT)
    return out_path


def build_micro(out_path):
    sh(["cc", "-O3", "-I", "lambda", str(MICRO_SRC), "-lm", "-o", str(out_path)], cwd=ROOT)
    return out_path


def host_metadata():
    def _git(*args):
        try:
            return sh(["git", *args], cwd=ROOT).stdout.strip()
        except Exception:
            return ""
    cc_version = ""
    try:
        cc_version = sh(["cc", "--version"]).stdout.splitlines()[0]
    except Exception:
        pass
    return {
        "git_sha": _git("rev-parse", "HEAD"),
        "git_dirty": bool(_git("status", "--porcelain")),
        "compiler": cc_version,
        "cc_flags": " ".join(CC_FLAGS),
        "platform": platform.platform(),
        "machine": platform.machine(),
        "cpu_count": os.cpu_count(),
        "python": platform.python_version(),
        # CR32 F9: pin the measurement inputs, not just the host. git fields
        # above describe the CHECKOUT RUNNING THE HARNESS; the binary hashes
        # below are the ground truth for what was measured.
        "bench_script_sha256": sha256_file(pathlib.Path(__file__)),
        "micro_source_sha256": sha256_file(MICRO_SRC) if MICRO_SRC.exists() else None,
        "harvested_fixture_sha256": sha256_file(HARVESTED) if HARVESTED.exists() else None,
    }


def mad(values):
    med = statistics.median(values)
    return statistics.median([abs(v - med) for v in values])


def sha256_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


# ── case payloads ───────────────────────────────────────────────────────────

def param_payload(program=None, threads=1):
    payload = {"mode": "param_gen", "n1": 256, "n2": 256, "times": 4, "n_threads": threads}
    if program is not None:
        payload["param_program"] = _compiled_param_program_payload(program)
    return payload


PARAM_EXPR = [
    ["const", "exp(t1*6.283185307179586j)"], ["emit", "p1"],
    ["const", "exp(t2*6.283185307179586j)"], ["emit", "p2"],
]


def coeff_payload(source=None, function="poly_1", n=128):
    payload = {"mode": "coeffgen", "function": function,
               "cfpv": [5, 1, 0] if function == "const" else [],
               "n1": n, "n2": n, "times": 1}
    if source is not None:
        payload["coeff_program"] = _compiled_coeff_program_payload(source)
    return payload


def chunked_payload(params_file, threads, source=None, n=256):
    payload = {"mode": "coeffgen_chunked", "function": "poly_1", "cfpv": [],
               "params_file": str(params_file), "step_start": 0, "step_count": n * n,
               "source_step_start": 0, "source_n1": n, "source_n2": n,
               "n_threads": threads}
    if source is not None:
        payload["coeff_program"] = _compiled_coeff_program_payload(source)
    return payload


def mqlacwaq_payloads():
    """The harvested real program: coeff-only and param+coeff variants."""
    import coeff_program_chain
    import param_program_chain
    calc = json.loads(HARVESTED.read_text())
    params = calc.get("params") or {}
    pipeline = calc.get("pipeline") or {}
    base = {"mode": "coeffgen", "function": pipeline.get("function") or calc.get("function"),
            "cfpv": pipeline.get("cfpv") or [], "n1": 128, "n2": 128, "times": 1}
    out = {"mqlacwaq_baseline": dict(base)}
    coeff_chain = params.get("coeff_program_chain")
    if coeff_chain:
        compiled = coeff_program_chain.compile_coeff_program_chain(coeff_chain)
        with_coeff = dict(base)
        with_coeff["coeff_program"] = compiled
        out["mqlacwaq_coeff"] = with_coeff
    param_chain = params.get("param_program_chain")
    if param_chain and coeff_chain:
        full = dict(out["mqlacwaq_coeff"])
        full["param_program"] = param_program_chain.compile_param_program_chain(param_chain)
        out["mqlacwaq_param_coeff"] = full
    return out


def build_cases(workdir, binary):
    """All macro cases. The chunked cases need a params file generated first."""
    params_file = workdir / "params.bin"
    _run_json(binary, {"mode": "param_gen", "n1": 256, "n2": 256, "times": 1,
                       "n_threads": 4}, params_file)
    cases = {
        "param_baseline": param_payload(),
        "param_identity_4tok": param_payload([["push", "t1"], ["emit", "p1"],
                                              ["push", "t2"], ["emit", "p2"]]),
        "param_arith_8tok": param_payload([["push", "t1"], ["push", "t2"], ["add"], ["emit", "p1"],
                                           ["push", "t1"], ["push", "t2"], ["subtract"], ["emit", "p2"]]),
        "param_unit_circle_native": param_payload([["unit_circle", "both"]]),
        "param_unit_circle_expr": param_payload(PARAM_EXPR),
        "param_expr_t2": param_payload(PARAM_EXPR, threads=2),
        "param_expr_t4": param_payload(PARAM_EXPR, threads=4),
        "param_expr_t8": param_payload(PARAM_EXPR, threads=8),
        "coeff35_baseline": coeff_payload(),
        "coeff35_rev": coeff_payload("poly = rev(poly)"),
        "coeff35_neg": coeff_payload("poly = neg(poly)"),
        "coeff35_add": coeff_payload("poly = add(poly, poly)"),
        "coeff35_sin": coeff_payload("poly = sin(poly)"),
        "coeff35_rev16": coeff_payload("\n".join(["poly = rev(poly)"] * 16)),
        "coeff35_neg16": coeff_payload("\n".join(["poly = neg(poly)"] * 16)),
        "coeff4_scalar21": coeff_payload("\n".join([
            "poly[0] = 1", "poly[1] = p1 + p2", "poly[2] = p1 * p2",
            "poly[3] = sin(p1) + cos(p2)", "poly = rev(poly)"]), function="const", n=128),
        "coeff4_baseline": coeff_payload(function="const", n=128),
        "chunked35_t1": chunked_payload(params_file, 1),
        "chunked35_t2": chunked_payload(params_file, 2),
        "chunked35_t4": chunked_payload(params_file, 4),
        "chunked35_t8": chunked_payload(params_file, 8),
        "chunked35_sin_t1": chunked_payload(params_file, 1, "poly = sin(poly)"),
        "chunked35_sin_t4": chunked_payload(params_file, 4, "poly = sin(poly)"),
    }
    cases.update(mqlacwaq_payloads())
    return cases


def run_case(binary, payload, workdir, name, reps):
    values, hashes = [], set()
    for i in range(reps):
        out = workdir / f"{name}_{i}.bin"
        meta = _run_json(binary, payload, out)
        values.append(int(meta["elapsed_us"]))
        hashes.add(sha256_file(out))
        out.unlink(missing_ok=True)
    if len(hashes) != 1:
        raise SystemExit(f"FATAL: case {name} produced varying output hashes: {hashes}")
    return {"median_us": statistics.median(values), "min_us": min(values),
            "max_us": max(values), "mad_us": mad(values), "reps": reps,
            "output_sha256": hashes.pop()}


def run_compare(bin_a, bin_b, payload, workdir, name, reps):
    va, vb, ha, hb = [], [], set(), set()
    for i in range(reps):  # interleaved to defeat thermal/scheduler drift
        out_a = workdir / f"{name}_a{i}.bin"
        out_b = workdir / f"{name}_b{i}.bin"
        va.append(int(_run_json(bin_a, payload, out_a)["elapsed_us"]))
        vb.append(int(_run_json(bin_b, payload, out_b)["elapsed_us"]))
        ha.add(sha256_file(out_a)); hb.add(sha256_file(out_b))
        out_a.unlink(missing_ok=True); out_b.unlink(missing_ok=True)
    if ha != hb or len(ha) != 1:
        raise SystemExit(f"FATAL: A/B output mismatch for {name}: A={ha} B={hb}")
    med_a, med_b = statistics.median(va), statistics.median(vb)
    return {"base_median_us": med_a, "cand_median_us": med_b,
            "base_mad_us": mad(va), "cand_mad_us": mad(vb),
            "delta_pct": round(100.0 * (med_b - med_a) / med_a, 2) if med_a else None,
            "reps_each": reps, "output_sha256": ha.pop()}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--cases", default="*", help="fnmatch filter on case names")
    ap.add_argument("--reps", type=int, default=11)
    ap.add_argument("--binary", help="use an existing sweep binary instead of building")
    ap.add_argument("--compare", nargs=2, metavar=("BASE", "CAND"),
                    help="interleaved A/B between two binaries (bytes must match)")
    ap.add_argument("--no-micro", action="store_true", help="skip the Root/Solve C microbench")
    ap.add_argument("--out", help="report path (default reports/vm_bench_<machine>_<git>.json)")
    ap.add_argument("--meta", action="append", default=[], metavar="KEY=VALUE",
                    help="extra provenance recorded verbatim in metadata (e.g. "
                         "base_source_commit=32e01ff) — CR32 F9/F4 follow-up")
    args = ap.parse_args()

    meta = host_metadata()
    for kv in args.meta:
        key, _, value = kv.partition("=")
        meta[f"note_{key}"] = value
    report = {"metadata": meta, "cases": {}}
    with tempfile.TemporaryDirectory(prefix="bench_program_vms_") as td:
        workdir = pathlib.Path(td)
        if args.compare:
            bin_a, bin_b = map(pathlib.Path, args.compare)
            meta["base_binary_sha256"] = sha256_file(bin_a)
            meta["cand_binary_sha256"] = sha256_file(bin_b)
        else:
            bin_a = pathlib.Path(args.binary) if args.binary else build_binary(workdir / "sweep_o3")
            bin_b = None
            meta["binary_sha256"] = sha256_file(bin_a)
            meta["binary_built_here"] = not bool(args.binary)
        # CR32 F14: "micro" is a selectable case, not a hidden side effect.
        # Micro compiles tests/native/vm_microbench.c from THIS checkout, so
        # it is skipped in --compare mode (prebuilt binaries would not be what
        # it measures) — pass each binary's checkout through --binary instead.
        micro_requested = (not args.no_micro and not args.compare
                           and fnmatch.fnmatch("micro", args.cases))
        cases = build_cases(workdir, bin_a)
        selected = {k: v for k, v in cases.items() if fnmatch.fnmatch(k, args.cases)}
        if not selected and not micro_requested:
            raise SystemExit(f"no cases match {args.cases!r}; available: micro, {', '.join(sorted(cases))}")
        for name in sorted(selected):
            payload = selected[name]
            if bin_b is not None:
                report["cases"][name] = run_compare(bin_a, bin_b, payload, workdir, name, args.reps)
                r = report["cases"][name]
                print(f"{name:28s} base {r['base_median_us']:>10.0f}us  cand {r['cand_median_us']:>10.0f}us  "
                      f"delta {r['delta_pct']:+.2f}%  bytes ok")
            else:
                report["cases"][name] = run_case(bin_a, payload, workdir, name, args.reps)
                r = report["cases"][name]
                print(f"{name:28s} median {r['median_us']:>10.0f}us  mad {r['mad_us']:>7.0f}us")
        if micro_requested:
            micro_bin = build_micro(workdir / "vm_micro")
            report["micro"] = json.loads(sh([str(micro_bin)]).stdout)
            report["metadata"]["micro_binary_sha256"] = sha256_file(micro_bin)
            for k, v in report["micro"].items():
                if k != "sink":
                    print(f"micro:{k:38s} {v}")

    git7 = (meta["git_sha"][:7] or "nogit") + ("-dirty" if meta["git_dirty"] else "")
    out_path = pathlib.Path(args.out) if args.out else ROOT / "reports" / f"vm_bench_{meta['machine']}_{git7}.json"
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(json.dumps(report, indent=2, sort_keys=True))
    print(f"\nreport: {out_path}")


if __name__ == "__main__":
    main()
