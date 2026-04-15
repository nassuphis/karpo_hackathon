#!/bin/bash
# Docker runtime regression test for deploy binaries.
# Uses deploy binaries (lambda/sweep, lambda/sweep_mt, lambda/sweep_cm) — NOT sweep_test.
# Must pass before deploy. Runs inside ARM64 Docker.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LAPACK_BUILD="$ROOT/lambda/layer-build-lapack"
LIBVIPS_BUILD="$ROOT/lambda/layer-build"

echo "=== Docker Runtime Regression Test ==="

# Verify binaries exist
for BIN in "$ROOT/lambda/sweep" "$ROOT/lambda/sweep_mt" "$ROOT/lambda/sweep_cm" "$ROOT/lambda/sweep_coeffgen" "$ROOT/lambda/solve_palette_chunk_mt" "$ROOT/lambda/pixbinassemble"; do
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

if [ ! -d "$ROOT/lambda/solve_palette_chunk_mt_lib" ]; then
    echo "FATAL: solve_palette_chunk_mt_lib not found. Run deploy.sh to compile."
    exit 1
fi

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
  -v "$ROOT/tests:/tests" \
  -v "$ROOT/coeff_func_catalog_js.js:/catalog.js:ro" \
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
    export LD_LIBRARY_PATH=/src/solve_palette_chunk_mt_lib:/opt/lib
    export PATH="/opt/bin:$PATH"

    # Install Python
    dnf install -y python3 2>&1 | tail -1

    # Copy catalog to expected location (if it exists)
    if [ -f /catalog.js ]; then
        cp /catalog.js /src/../coeff_func_catalog_js.js 2>/dev/null || true
    fi

    # Run the regression test
    python3 /tests/docker_runtime_regression.py
  '

echo "=== Docker Runtime Test PASSED ==="
