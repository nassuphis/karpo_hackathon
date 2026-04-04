#!/bin/bash
# Build ReportLab + Pillow ARM64 Lambda layer
# Produces: layer-build-pdf/pdf-python-layer.zip
set -euo pipefail

OUTDIR="$(cd "$(dirname "$0")" && pwd)/layer-build-pdf"
rm -rf "$OUTDIR"
mkdir -p "$OUTDIR"

echo "=== Building PDF Python layer for ARM64 Lambda ==="

docker run --rm --platform linux/arm64 \
  --entrypoint /bin/bash \
  -v "$OUTDIR:/out" \
  public.ecr.aws/lambda/python:3.12 \
  -lc '
    set -euo pipefail
    mkdir -p /out/python
    python -m pip install --upgrade pip >/dev/null
    python -m pip install --no-cache-dir -t /out/python reportlab Pillow
    python -c "import pathlib, shutil; root = pathlib.Path(\"/out/python\"); [shutil.rmtree(path) for path in root.rglob(\"__pycache__\") if path.is_dir()]; [path.unlink() for path in root.rglob(\"*.pyc\")]"
    python -c "import pathlib, zipfile; out = pathlib.Path(\"/out/pdf-python-layer.zip\"); base = pathlib.Path(\"/out\"); root = pathlib.Path(\"/out/python\"); zf = zipfile.ZipFile(out, \"w\", zipfile.ZIP_DEFLATED); [zf.write(path, path.relative_to(base)) for path in root.rglob(\"*\") if path.is_file()]; zf.close()"
  '

echo "Layer zip: $OUTDIR/pdf-python-layer.zip ($(du -h "$OUTDIR/pdf-python-layer.zip" | cut -f1))"
