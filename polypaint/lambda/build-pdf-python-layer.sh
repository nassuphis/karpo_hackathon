#!/bin/bash
# Build ReportLab + Pillow ARM64 Lambda layer
# Produces: layer-build-pdf/pdf-python-layer.zip
set -euo pipefail

OUTDIR="$(cd "$(dirname "$0")" && pwd)/layer-build-pdf"
rm -rf "$OUTDIR"
mkdir -p "$OUTDIR"

echo "=== Building PDF Python layer for ARM64 Lambda ==="

docker run --rm --platform linux/arm64 \
  -v "$OUTDIR:/out" \
  public.ecr.aws/lambda/python:3.12 \
  bash -lc '
    set -euo pipefail
    mkdir -p /out/python
    python -m pip install --upgrade pip >/dev/null
    python -m pip install --no-cache-dir -t /out/python reportlab Pillow
    find /out/python -name "__pycache__" -type d -prune -exec rm -rf {} +
    find /out/python -name "*.pyc" -delete
    cd /out
    zip -r9 /out/pdf-python-layer.zip python >/dev/null
  '

echo "Layer zip: $OUTDIR/pdf-python-layer.zip ($(du -h "$OUTDIR/pdf-python-layer.zip" | cut -f1))"
