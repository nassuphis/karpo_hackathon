#!/usr/bin/env python3
"""CR32 F7: deploy-binary freshness and provenance.

Modes:
  --check           fail (exit 1) if any deploy binary is missing or OLDER
                    than any of its sources (direct .c inputs plus local
                    #include "..." headers, discovered transitively)
  --write-manifest  record lambda/binary_manifest.json: per-binary sha256,
                    size, build mtime, source git commit + dirty flag
  --verify-manifest fail if on-disk binaries do not match the manifest hashes

The Docker runtime regression runs --check first, so a green Docker run can
no longer certify stale binaries (the CR31 failure mode: five binaries
predated the source commits the run claimed to verify).
"""
import argparse
import hashlib
import json
import pathlib
import re
import subprocess
import sys
import time

ROOT = pathlib.Path(__file__).resolve().parents[1]
LAMBDA = ROOT / "lambda"
MANIFEST = LAMBDA / "binary_manifest.json"

# binary name -> direct .c inputs (mirrors deploy.sh build lines; the include
# closure is discovered automatically)
DEPLOY_BINARIES = {
    "sweep": ["sweep_cli.c"],
    "sweep_mt": ["sweep_mt.c"],
    "coeffs_bilevel_raster": ["coeffs_bilevel_raster.c"],
    "bilevel_section_raster": ["bilevel_section_raster.c"],
    "solve_proximity_stats": ["solve_proximity_stats.c"],
    "solve_palette_debug": ["solve_palette_debug.c"],
    "solve_palette_chunk": ["solve_palette_chunk.c"],
    "palette_bins_render": ["palette_bins_render.c"],
    "step_scores_to_palette_raw": ["step_scores_to_palette_raw.c"],
    "splat_bake": ["splat_bake.c"],
    "view_raster": ["view_raster.c"],
    "roots2pix_mt": ["roots2pix_mt.c", "multispan_reader.c"],
    "solve_proximity_hist_sectioned": ["solve_proximity_hist_sectioned.c", "multispan_reader.c"],
    "solve_palette_chunk_mt": ["solve_palette_chunk_mt.c", "multispan_reader.c"],
    "assemble_greyscale": ["assemble_greyscale.c"],
    "sweep_coeffgen": ["sweep_cli.c"],
    "sweep_cm": ["sweep_cm.c"],
    # libvips docker set (deploy.sh VIPS block)
    "raw2jpeg": ["raw2jpeg.c"],
    "score_raw_render": ["score_raw_render.c"],
    "bilevel_merge": ["bilevel_merge.c"],
    "raw_to_bilevel": ["raw_to_bilevel.c"],
    "tiff_compat": ["tiff_compat.c"],
    "png_export": ["png_export.c"],
    "dz_export": ["dz_export.c"],
    "wall_dz": ["wall_dz.c"],
    "sheet_stitch": ["sheet_stitch.c"],
    "autolevels_render": ["autolevels_render.c"],
}

INCLUDE_RE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.M)


def include_closure(c_files):
    seen, queue = set(), [LAMBDA / f for f in c_files]
    while queue:
        path = queue.pop()
        if path in seen or not path.exists():
            continue
        seen.add(path)
        for inc in INCLUDE_RE.findall(path.read_text(errors="replace")):
            queue.append((path.parent / inc).resolve())
    return sorted(seen)


def sha256_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def git(*args):
    try:
        return subprocess.run(["git", *args], cwd=ROOT, capture_output=True,
                              text=True, check=True).stdout.strip()
    except Exception:
        return ""


def check():
    """mtime freshness PLUS, when a manifest exists, source-hash provenance:
    touching a stale binary defeats mtimes but not the recorded source
    hashes — any source whose hash differs from the manifest marks every
    binary depending on it stale until rebuilt (which rewrites the
    manifest)."""
    manifest = json.loads(MANIFEST.read_text()) if MANIFEST.exists() else None
    stale, missing = [], []
    for name, c_files in DEPLOY_BINARIES.items():
        binary = LAMBDA / name
        if not binary.exists():
            missing.append(name)
            continue
        bin_mtime = binary.stat().st_mtime
        entry = (manifest or {}).get("binaries", {}).get(name)
        recorded = entry.get("source_sha256", {}) if entry else {}
        for src in include_closure(c_files):
            rel = str(src.relative_to(ROOT))
            if rel in recorded:
                # Hash is truth: deterministic regeneration (palette LUT
                # headers on every deploy) refreshes mtimes without changing
                # content — a matching hash means NOT stale. A differing
                # hash means stale regardless of mtimes (touch-proof).
                if sha256_file(src) != recorded[rel]:
                    stale.append((name, rel, "source hash differs from manifest"))
                continue
            if src.stat().st_mtime > bin_mtime:
                stale.append((name, rel,
                              f"{src.stat().st_mtime - bin_mtime:.0f}s newer mtime "
                              "(no manifest hash to confirm content)"))
        if entry and sha256_file(binary) != entry["sha256"]:
            stale.append((name, "(binary)", "binary hash differs from manifest"))
    for name in missing:
        print(f"MISSING: lambda/{name}")
    for name, src, why in stale:
        print(f"STALE: lambda/{name} vs {src}: {why}")
    if missing or stale:
        print("\nFAIL: rebuild with scripts/build-deploy-binaries.sh")
        return 1
    print(f"ok: all {len(DEPLOY_BINARIES)} deploy binaries newer than their sources"
          + (" and consistent with the manifest" if manifest else ""))
    return 0


def write_manifest():
    entries = {}
    for name, c_files in DEPLOY_BINARIES.items():
        binary = LAMBDA / name
        if not binary.exists():
            print(f"FAIL: cannot write manifest, lambda/{name} missing")
            return 1
        st = binary.stat()
        entries[name] = {
            "sha256": sha256_file(binary),
            "size": st.st_size,
            "built_mtime": int(st.st_mtime),
            "source_sha256": {str(p.relative_to(ROOT)): sha256_file(p)
                              for p in include_closure(c_files)},
        }
    manifest = {
        "generated_at": int(time.time()),
        "git_sha": git("rev-parse", "HEAD"),
        "git_dirty": bool(git("status", "--porcelain")),
        "binaries": entries,
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    print(f"wrote {MANIFEST.relative_to(ROOT)} for commit "
          f"{manifest['git_sha'][:9]}{'-dirty' if manifest['git_dirty'] else ''}")
    return 0


def verify_manifest():
    if not MANIFEST.exists():
        print("FAIL: lambda/binary_manifest.json missing (run --write-manifest after building)")
        return 1
    manifest = json.loads(MANIFEST.read_text())
    bad = []
    for name, entry in manifest["binaries"].items():
        binary = LAMBDA / name
        if not binary.exists():
            bad.append(f"{name}: missing")
        elif sha256_file(binary) != entry["sha256"]:
            bad.append(f"{name}: sha256 mismatch vs manifest")
    for line in bad:
        print(f"FAIL: {line}")
    if bad:
        return 1
    print(f"ok: {len(manifest['binaries'])} binaries match manifest "
          f"(commit {manifest['git_sha'][:9]})")
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    mode = ap.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--write-manifest", action="store_true")
    mode.add_argument("--verify-manifest", action="store_true")
    args = ap.parse_args()
    if args.check:
        sys.exit(check())
    if args.write_manifest:
        sys.exit(write_manifest())
    sys.exit(verify_manifest())


if __name__ == "__main__":
    main()
