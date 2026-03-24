#!/bin/bash
# Docker runtime regression test for AE and CM solvers.
# Uses deploy binaries (lambda/sweep, lambda/sweep_cm) — NOT sweep_test.
# Must pass before deploy. Runs inside ARM64 Docker.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LAPACK_BUILD="$ROOT/lambda/layer-build-lapack"

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

docker run --rm --platform linux/arm64 \
  -v "$ROOT/lambda:/src" \
  -v "$LAPACK_BUILD:/opt" \
  public.ecr.aws/amazonlinux/amazonlinux:2023 \
  bash -c '
    set -euo pipefail
    export LD_LIBRARY_PATH=/opt/lib

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

print("=== All Docker runtime tests PASSED ===")
PYEOF
  '

echo "=== Docker Runtime Test PASSED ==="
