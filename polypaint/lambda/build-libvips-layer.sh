#!/bin/bash
# Build libvips 8.16.1 ARM64 Lambda layer
# Produces: libvips-layer.zip ready to publish as a Lambda layer
#
# Layer structure: lib/ and include/ at zip root
# Lambda extracts into /opt/, so files end up at /opt/lib/ and /opt/include/
set -euo pipefail

VIPS_VERSION=8.16.1
OUTDIR="$(cd "$(dirname "$0")" && pwd)/layer-build"
rm -rf "$OUTDIR"
mkdir -p "$OUTDIR"

echo "=== Building libvips $VIPS_VERSION for ARM64 Lambda ==="

docker run --rm --platform linux/arm64 \
  -v "$OUTDIR:/out" \
  public.ecr.aws/amazonlinux/amazonlinux:2023 \
  bash -c '
    set -euo pipefail

    echo "--- Installing build tools ---"
    dnf install -y gcc gcc-c++ make meson ninja-build pkg-config \
      glib2-devel expat-devel libjpeg-turbo-devel libpng-devel \
      zlib-devel tar xz wget 2>&1 | tail -3

    echo "--- Downloading libvips '"$VIPS_VERSION"' ---"
    cd /tmp
    wget -q "https://github.com/libvips/libvips/releases/download/v'"$VIPS_VERSION"'/vips-'"$VIPS_VERSION"'.tar.xz"
    tar xJf "vips-'"$VIPS_VERSION"'.tar.xz"
    cd "vips-'"$VIPS_VERSION"'"

    echo "--- Configuring (JPEG+PNG only) ---"
    meson setup builddir --prefix=/opt \
      --buildtype=release \
      -Dmodules=disabled \
      -Dintrospection=disabled \
      -Dheif=disabled \
      -Dpoppler=disabled \
      -Drsvg=disabled \
      -Dtiff=disabled \
      -Dopenexr=disabled \
      -Dfits=disabled \
      -Dimagequant=disabled \
      -Dcgif=disabled \
      -Dspng=disabled \
      -Dwebp=disabled \
      -Dpdfium=disabled \
      -Dnifti=disabled \
      -Dopenslide=disabled \
      -Dexif=disabled \
      -Dlcms=disabled \
      2>&1 | tail -5

    echo "--- Building ---"
    cd builddir
    ninja -j$(nproc) 2>&1 | tail -3
    DESTDIR=/staging ninja install

    echo "--- Packaging layer ---"
    # Layer zip root: lib/ and include/
    # Lambda extracts into /opt/, so lib/ becomes /opt/lib/
    mkdir -p /out/lib /out/include

    # Copy libvips shared libs
    cp -a /staging/opt/lib64/libvips*.so* /out/lib/ 2>/dev/null || \
    cp -a /staging/opt/lib/libvips*.so* /out/lib/ 2>/dev/null || true

    # Copy headers (for compiling imgpipe in Docker)
    cp -r /staging/opt/include/vips /out/include/ 2>/dev/null || true
    mkdir -p /out/include/glib-2.0 /out/lib/glib-2.0/include
    cp -r /usr/include/glib-2.0/* /out/include/glib-2.0/
    GLIBCONFIG=$(find /usr/lib64 /usr/lib -name glibconfig.h 2>/dev/null | head -1)
    if [ -n "$GLIBCONFIG" ]; then
      cp "$GLIBCONFIG" /out/lib/glib-2.0/include/
    fi

    # Copy pkg-config
    mkdir -p /out/lib/pkgconfig
    cp /staging/opt/lib64/pkgconfig/vips*.pc /out/lib/pkgconfig/ 2>/dev/null || \
    cp /staging/opt/lib/pkgconfig/vips*.pc /out/lib/pkgconfig/ 2>/dev/null || true

    # Bundle ALL transitive shared library dependencies
    # These may or may not be present on Lambda runtime — bundle them all to be safe
    echo "--- Copying transitive dependencies ---"
    DEPS=(
      libgobject-2.0.so.0
      libglib-2.0.so.0
      libgio-2.0.so.0
      libgmodule-2.0.so.0
      libexpat.so.1
      libffi.so.8
      libpcre2-8.so.0
      libz.so.1
      libmount.so.1
      libselinux.so.1
      libblkid.so.1
      libjpeg.so.62
      libpng16.so.16
    )
    for lib in "${DEPS[@]}"; do
      # Find the actual file (follow symlinks) and copy it + create symlinks
      FOUND=$(find /usr/lib64 /lib64 /usr/lib /lib -name "${lib}*" 2>/dev/null | head -1)
      if [ -n "$FOUND" ]; then
        # Copy all files matching this lib name (includes versioned symlinks)
        LIBDIR=$(dirname "$FOUND")
        BASENAME=$(basename "$FOUND" | sed "s/\\.so.*//" )
        for f in "${LIBDIR}/${BASENAME}".so*; do
          if [ -L "$f" ]; then
            # Resolve symlink and copy as real file
            REAL=$(readlink -f "$f")
            cp "$REAL" "/out/lib/$(basename "$f")"
          elif [ -f "$f" ]; then
            cp "$f" "/out/lib/$(basename "$f")"
          fi
        done
        echo "  bundled: $lib"
      else
        echo "  WARNING: $lib not found"
      fi
    done

    echo "--- Layer contents ---"
    ls -lh /out/lib/*.so* 2>/dev/null || true
    echo "--- Done ---"
  '

echo "=== Creating layer zip ==="
cd "$OUTDIR"
# Zip lib/ and include/ at root level
zip -r9 "$OUTDIR/libvips-layer.zip" lib/ include/
echo "Layer zip: $OUTDIR/libvips-layer.zip ($(du -h libvips-layer.zip | cut -f1))"
