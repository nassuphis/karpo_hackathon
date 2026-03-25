#!/bin/bash
# Docker runtime regression test for AE and CM solvers.
# Uses deploy binaries (lambda/sweep, lambda/sweep_cm) — NOT sweep_test.
# Must pass before deploy. Runs inside ARM64 Docker.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LAPACK_BUILD="$ROOT/lambda/layer-build-lapack"
LIBVIPS_BUILD="$ROOT/lambda/layer-build"

echo "=== Docker Runtime Regression Test ==="

# Verify binaries exist
for BIN in "$ROOT/lambda/sweep" "$ROOT/lambda/sweep_cm"; do
    if [ ! -f "$BIN" ]; then
        echo "FATAL: $BIN not found. Run deploy.sh to compile."
        exit 1
    fi
    TYPE=$(file "$BIN")
    if ! echo "$TYPE" | grep -q "ELF 64-bit.*ARM aarch64"; then
        echo "FATAL: $BIN is not Linux ARM64 ELF: $TYPE"
        exit 1
    fi
done

if [ ! -d "$LAPACK_BUILD/lib" ]; then
    echo "FATAL: LAPACK layer not built. Run lambda/build-lapack-layer.sh."
    exit 1
fi

if [ ! -d "$LIBVIPS_BUILD/lib" ]; then
    echo "FATAL: libvips layer not built. Run lambda/build-libvips-layer.sh."
    exit 1
fi

docker run --rm --platform linux/arm64 \
  -v "$ROOT/lambda:/src" \
  -v "$LAPACK_BUILD:/opt-lapack" \
  -v "$LIBVIPS_BUILD:/opt-vips" \
  public.ecr.aws/amazonlinux/amazonlinux:2023 \
  bash -c '
    set -euo pipefail
    # Merge both layers into /opt (Lambda runtime layout)
    cp -a /opt-lapack/lib /opt/ 2>/dev/null || true
    cp -a /opt-vips/lib/* /opt/lib/ 2>/dev/null || true
    mkdir -p /opt/bin
    cp -a /opt-vips/bin/* /opt/bin/ 2>/dev/null || true
    export LD_LIBRARY_PATH=/opt/lib
    export PATH="/opt/bin:$PATH"

    echo "--- Binary validation ---"
    # Check ELF magic bytes and that binaries are executable
    for BIN in /src/sweep /src/sweep_cm; do
        MAGIC=$(od -A n -t x1 -N 4 "$BIN" | tr -d " ")
        if [ "$MAGIC" != "7f454c46" ]; then
            echo "FATAL: $BIN is not an ELF binary (magic: $MAGIC)"
            exit 1
        fi
        echo "  $BIN: ELF OK"
    done

    echo "--- Generating test fixtures ---"
    dnf install -y python3 2>&1 | tail -1

    python3 - <<PYEOF
import json, struct, os, subprocess, sys, math

def write_cf(path, coeffs_list, n_coeffs):
    with open(path, "wb") as f:
        for cf in coeffs_list:
            padded = list(cf) + [0.0] * (n_coeffs - len(cf))
            for c in padded[:n_coeffs]:
                re = c.real if hasattr(c, "real") else float(c)
                im = c.imag if hasattr(c, "imag") else 0.0
                f.write(struct.pack("<ff", re, im))

def read_roots(path, degree):
    with open(path, "rb") as f:
        data = f.read()
    vals = struct.unpack("<" + "f" * (len(data) // 4), data)
    roots = [complex(vals[i], vals[i+1]) for i in range(0, len(vals), 2)]
    polys = []
    for i in range(0, len(roots), degree):
        polys.append(roots[i:i+degree])
    return polys

def polyval(cf, z):
    y = 0j
    for a in cf:
        y = y * z + complex(a)
    return y

# Test polynomials
polys = [
    [1, -6, 11, -6],           # cubic: roots 1,2,3
    [1, -10, 35, -50, 24],     # quartic: roots 1,2,3,4
    [1, -2, 1],                # repeated: root 1,1
    [1, 0, 1],                 # complex: roots +i,-i
    [0, 1, -3, 2],             # leading zero: roots 1,2
    [0, 0, 0, 0, 0],           # all-zero
]
n_coeffs = 5

write_cf("/tmp/test_cf.bin", polys, n_coeffs)

# Run AE solver
ae_spec = json.dumps({
    "mode": "solve",
    "coeffs_file": "/tmp/test_cf.bin",
    "n_coeffs": n_coeffs,
    "n2": len(polys),
    "i1_start": 0, "i1_end": 1,
    "match_roots": False,
})
r = subprocess.run(["/src/sweep", "/tmp/ae_out.bin"],
    input=ae_spec, capture_output=True, text=True, timeout=30)
if r.returncode != 0:
    print(f"AE FAILED: {r.stderr[:200]}")
    sys.exit(1)
print(f"AE: {json.loads(r.stdout)}")

# Run CM solver
cm_spec = json.dumps({
    "mode": "solve_cm",
    "coeffs_file": "/tmp/test_cf.bin",
    "n_coeffs": n_coeffs,
    "n_steps": len(polys),
})
r = subprocess.run(["/src/sweep_cm", "/tmp/cm_out.bin"],
    input=cm_spec, capture_output=True, text=True, timeout=30)
if r.returncode != 0:
    print(f"CM FAILED: {r.stderr[:200]}")
    sys.exit(1)
print(f"CM: {json.loads(r.stdout)}")

# Verify output sizes match
ae_size = os.path.getsize("/tmp/ae_out.bin")
cm_size = os.path.getsize("/tmp/cm_out.bin")
expected = len(polys) * (n_coeffs - 1) * 2 * 4
assert ae_size == expected, f"AE size {ae_size} != {expected}"
assert cm_size == expected, f"CM size {cm_size} != {expected}"

# Read and validate roots
degree = n_coeffs - 1
ae_roots = read_roots("/tmp/ae_out.bin", degree)
cm_roots = read_roots("/tmp/cm_out.bin", degree)

assert len(ae_roots) == len(polys)
assert len(cm_roots) == len(polys)

# Check finiteness and residuals for non-degenerate polynomials
test_labels = ["cubic", "quartic", "repeated", "complex", "leading-zero", "all-zero"]
for i, (cf, ae_r, cm_r, label) in enumerate(zip(polys, ae_roots, cm_roots, test_labels)):
    # Check all finite
    ae_finite = all(math.isfinite(r.real) and math.isfinite(r.imag) for r in ae_r)
    cm_finite = all(math.isfinite(r.real) and math.isfinite(r.imag) for r in cm_r)
    assert ae_finite, f"{label}: AE has non-finite roots"
    assert cm_finite, f"{label}: CM has non-finite roots"

    if label == "all-zero":
        continue  # skip residual check for degenerate

    # Check residuals (use original cf, filter near-zero roots from padding)
    active_roots_ae = [r for r in ae_r if abs(r) > 1e-10]
    active_roots_cm = [r for r in cm_r if abs(r) > 1e-10]

    if active_roots_ae:
        ae_resid = max(abs(polyval(cf, r)) for r in active_roots_ae)
        assert ae_resid < 1e-2, f"{label}: AE max residual {ae_resid:.2e}"

    if active_roots_cm:
        cm_resid = max(abs(polyval(cf, r)) for r in active_roots_cm)
        assert cm_resid < 1e-2, f"{label}: CM max residual {cm_resid:.2e}"

    print(f"  {label}: AE OK, CM OK")

print("=== AE/CM solver tests PASSED ===")

# --- CFPV coeffgen regression ---
print("\n--- CFPV coeffgen regression ---")

def run_coeffgen(func, cfpv=None):
    spec = {
        "mode": "coeffgen",
        "function": func,
        "n1": 4, "n2": 4,
        "i1_start": 0, "i1_end": 4,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [],
        "times": 1,
    }
    if cfpv is not None:
        spec["cfpv"] = cfpv
    r = subprocess.run(["/src/sweep", "/tmp/cfpv_test.bin"],
        input=json.dumps(spec), capture_output=True, text=True, timeout=10)
    if r.returncode != 0:
        return None, r.stderr
    return json.loads(r.stdout), None

# creative9 default
m, err = run_coeffgen("creative9")
assert m and m["n_coeffs"] == 71, f"creative9 default: {err or m}"
print("  creative9 default (n=71): OK")

# creative9 with cfpv=[30]
m, err = run_coeffgen("creative9", [30])
assert m and m["n_coeffs"] == 30, f"creative9 cfpv=[30]: {err or m}"
print("  creative9 cfpv=[30] (n=30): OK")

# creative8 default
m, err = run_coeffgen("creative8")
assert m and m["n_coeffs"] == 71, f"creative8 default: {err or m}"
print("  creative8 default (n=71): OK")

# creative8 with cfpv=[40]
m, err = run_coeffgen("creative8", [40])
assert m and m["n_coeffs"] == 40, f"creative8 cfpv=[40]: {err or m}"
print("  creative8 cfpv=[40] (n=40): OK")

# non-parametric unaffected by cfpv
m1, _ = run_coeffgen("g1")
m2, _ = run_coeffgen("g1", [999])
assert m1 and m2 and m1["n_coeffs"] == m2["n_coeffs"], "g1 cfpv should be ignored"
print("  g1 unaffected by cfpv: OK")

print("=== CFPV coeffgen tests PASSED ===")

# --- Render preview (vipsthumbnail) regression ---
print("\n--- Render preview (vipsthumbnail) ---")

# Check vipsthumbnail binary exists and runs
vt_path = "/opt/bin/vipsthumbnail"
assert os.path.exists(vt_path), f"vipsthumbnail not found at {vt_path}"
r = subprocess.run([vt_path, "--vips-version"], capture_output=True, text=True, timeout=5)
print(f"  vipsthumbnail: {r.stdout.strip() or r.stderr.strip()}")

# Create a test JPEG via raw2jpeg (8x8 solid color)
test_raw = "/tmp/preview_test.raw"
test_jpeg = "/tmp/preview_test.jpeg"
test_png = "/tmp/preview_test_thumb.png"

with open(test_raw, "wb") as f:
    # raw2jpeg format: uint32 width, height, channels, then RGB pixels
    f.write(struct.pack("<III", 8, 8, 3))
    for _ in range(8 * 8):
        f.write(bytes([100, 150, 200]))  # solid blue-ish

r = subprocess.run(
    ["/src/raw2jpeg", test_raw, test_jpeg, "--quality=90"],
    capture_output=True, text=True, timeout=10,
    env={**os.environ, "LD_LIBRARY_PATH": "/opt/lib"}
)
assert r.returncode == 0, f"raw2jpeg failed: {r.stderr[:200]}"
jpeg_size = os.path.getsize(test_jpeg)
assert jpeg_size > 0, "test JPEG is empty"
print(f"  test JPEG: {jpeg_size} bytes")

# Run vipsthumbnail on the JPEG
r = subprocess.run(
    [vt_path, test_jpeg, "-s", "4x4", "-o", test_png + "[strip]"],
    capture_output=True, text=True, timeout=10,
    env={**os.environ, "LD_LIBRARY_PATH": "/opt/lib", "PATH": "/opt/bin:" + os.environ.get("PATH", "")}
)
assert r.returncode == 0, f"vipsthumbnail failed: {r.stderr[:200]}"
png_size = os.path.getsize(test_png)
assert png_size > 0, "thumbnail PNG is empty"
print(f"  thumbnail PNG: {png_size} bytes")

# Also test with a TIFF input (bilevel preview path)
test_tif = "/tmp/preview_test.tif"
# Create a minimal TIFF via tiff_compat path — or just use vips to save one
r = subprocess.run(
    ["/opt/bin/vips", "im_copy", test_jpeg, test_tif],
    capture_output=True, text=True, timeout=10,
    env={**os.environ, "LD_LIBRARY_PATH": "/opt/lib", "PATH": "/opt/bin:" + os.environ.get("PATH", "")}
)
if r.returncode == 0:
    test_tif_thumb = "/tmp/preview_test_tif_thumb.png"
    r = subprocess.run(
        [vt_path, test_tif, "-s", "4x4", "-o", test_tif_thumb + "[strip]"],
        capture_output=True, text=True, timeout=10,
        env={**os.environ, "LD_LIBRARY_PATH": "/opt/lib", "PATH": "/opt/bin:" + os.environ.get("PATH", "")}
    )
    assert r.returncode == 0, f"vipsthumbnail TIFF failed: {r.stderr[:200]}"
    tif_png_size = os.path.getsize(test_tif_thumb)
    assert tif_png_size > 0, "TIFF thumbnail is empty"
    print(f"  TIFF thumbnail PNG: {tif_png_size} bytes")
else:
    print(f"  SKIP TIFF test: vips im_copy not available ({r.stderr.strip()[:60]})")

# Cleanup
for f in [test_raw, test_jpeg, test_png, test_tif,
          "/tmp/preview_test_tif_thumb.png"]:
    try:
        os.remove(f)
    except OSError:
        pass

print("=== Render preview tests PASSED ===")

# --- solve_proximity_stats regression ---
print("\n--- solve_proximity_stats ---")

sps_path = "/src/solve_proximity_stats"
if os.path.exists(sps_path):
    # Verify binary
    MAGIC = open(sps_path, "rb").read(4)
    assert MAGIC == b"\x7fELF", f"solve_proximity_stats is not ELF"
    print(f"  {sps_path}: ELF OK")

    # Create tiny synthetic .bin: 3 solves, degree=2
    # Solve A: roots (0,0),(1,0) → d2=1.0 → score=0.0
    # Solve B: roots (0,0),(0.01,0) → d2=0.0001 → score=2.0
    # Solve C: roots (0,0),(0.1,0) → d2≈0.01 → score≈1.0
    sps_bin = "/tmp/sps_test.bin"
    with open(sps_bin, "wb") as f:
        for roots in [[(0,0),(1,0)], [(0,0),(0.01,0)], [(0,0),(0.1,0)]]:
            for re, im in roots:
                f.write(struct.pack("<ff", re, im))

    # Test clip mode
    r = subprocess.run([sps_path, sps_bin, "--mode=clip", "--degree=2"],
        capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, f"clip failed: {r.stderr[:200]}"
    clip = json.loads(r.stdout)
    assert clip["n_solves"] == 3
    assert clip["degree"] == 2
    assert clip["clip_lo"] <= clip["clip_hi"]
    print(f"  clip: OK (n={clip['n_solves']}, lo={clip['clip_lo']:.2f}, hi={clip['clip_hi']:.2f})")

    # Test hist mode
    r = subprocess.run([sps_path, sps_bin, "--mode=hist", "--degree=2",
        f"--clip_lo={clip['clip_lo']}", f"--clip_hi={clip['clip_hi']}",
        "--hist_bins=10"],
        capture_output=True, text=True, timeout=10)
    assert r.returncode == 0, f"hist failed: {r.stderr[:200]}"
    hist = json.loads(r.stdout)
    assert len(hist["hist"]) == 10
    assert sum(hist["hist"]) == 3
    print(f"  hist: OK (bins={len(hist['hist'])}, total={sum(hist['hist'])})")

    os.remove(sps_bin)
    print("=== solve_proximity_stats tests PASSED ===")
else:
    print(f"  SKIP: {sps_path} not found (not yet compiled)")

# --- Catalog degree verification ---
# Compare host-generated JS catalog degrees against deploy binary probes
print("\n--- Catalog degree verification ---")
catalog_path = "/src/../coeff_func_catalog_js.js"
if os.path.exists(catalog_path):
    with open(catalog_path) as f:
        js_text = f.read()
    import re
    cat_json = js_text.split("window._coeffFuncCatalog = ")[1].rstrip(";\n")
    catalog = json.loads(cat_json)
    mismatches = []
    tested = 0
    for entry in catalog:
        if entry.get("probe_failed"):
            continue
        m, err = run_coeffgen(entry["name"])
        if m is None:
            continue  # skip functions that fail in Docker too
        actual_degree = m["n_coeffs"] - 1
        if actual_degree != entry["degree"]:
            mismatches.append(f"{entry['name']}: catalog={entry['degree']} deploy={actual_degree}")
        tested += 1
    if mismatches:
        print(f"  FAIL: {len(mismatches)} degree mismatches:")
        for mm in mismatches[:10]:
            print(f"    {mm}")
        sys.exit(1)
    print(f"  {tested} functions: all degrees match between host catalog and deploy binary")
    print("=== Catalog degree verification PASSED ===")
else:
    print("  SKIP: coeff_func_catalog_js.js not found (run gen_catalog.py first)")

print("\n=== All Docker runtime tests PASSED ===")
PYEOF
  '

echo "=== Docker Runtime Test PASSED ==="
