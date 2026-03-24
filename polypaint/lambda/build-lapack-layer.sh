#!/bin/bash
# Build OpenBLAS+LAPACK ARM64 Lambda layer
# Produces: layer-build-lapack/lapack-layer.zip
set -euo pipefail

OUTDIR="$(cd "$(dirname "$0")" && pwd)/layer-build-lapack"
rm -rf "$OUTDIR"
mkdir -p "$OUTDIR"

echo "=== Building OpenBLAS+LAPACK for ARM64 Lambda ==="

docker run --rm --platform linux/arm64 \
  -v "$OUTDIR:/out" \
  public.ecr.aws/amazonlinux/amazonlinux:2023 \
  bash -c '
    set -euo pipefail

    echo "--- Installing OpenBLAS + LAPACK ---"
    dnf install -y openblas-devel lapack-devel gcc-gfortran 2>&1 | tail -3

    echo "--- Copying libraries ---"
    mkdir -p /out/lib

    # Copy OpenBLAS (includes LAPACK routines)
    for lib in libopenblas liblapack libblas libgfortran libquadmath libgcc_s; do
      for f in /usr/lib64/${lib}.so* /lib64/${lib}.so*; do
        [ -f "$f" ] || [ -L "$f" ] || continue
        if [ -L "$f" ]; then
          REAL=$(readlink -f "$f")
          cp "$REAL" "/out/lib/$(basename "$f")"
        else
          cp "$f" "/out/lib/$(basename "$f")"
        fi
        echo "  bundled: $(basename "$f")"
      done
    done

    echo "--- Smoke test: check zgeev symbol ---"
    nm -D /out/lib/liblapack.so.* 2>/dev/null | grep -i zgeev | head -3 || \
    nm -D /out/lib/libopenblas.so.* 2>/dev/null | grep -i zgeev | head -3 || \
    echo "WARNING: zgeev not found in bundled libs"

    echo "--- Layer contents ---"
    ls -lh /out/lib/*.so* 2>/dev/null || true
    echo "--- Done ---"
  '

echo "=== Creating layer zip ==="
cd "$OUTDIR"
zip -r9 "$OUTDIR/lapack-layer.zip" lib/
echo "Layer zip: $OUTDIR/lapack-layer.zip ($(du -h lapack-layer.zip | cut -f1))"
