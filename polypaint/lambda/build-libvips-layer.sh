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
      libtiff-devel libwebp-devel jbigkit-devel libxml2-devel \
      libarchive-devel \
      zlib-devel tar xz wget bzip2-devel 2>&1 | tail -3
    echo "  libarchive: $(pkg-config --modversion libarchive 2>/dev/null || echo MISSING)"

    echo "--- Downloading libvips '"$VIPS_VERSION"' ---"
    cd /tmp
    wget -q "https://github.com/libvips/libvips/releases/download/v'"$VIPS_VERSION"'/vips-'"$VIPS_VERSION"'.tar.xz"
    tar xJf "vips-'"$VIPS_VERSION"'.tar.xz"
    cd "vips-'"$VIPS_VERSION"'"

    echo "--- Configuring (JPEG+PNG+TIFF+dzsave) ---"
    meson setup builddir --prefix=/opt \
      --buildtype=release \
      -Dmodules=disabled \
      -Dintrospection=disabled \
      -Dheif=disabled \
      -Dpoppler=disabled \
      -Drsvg=disabled \
      -Dtiff=enabled \
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
      2>&1 | grep -iE "pyramid|archive|tiff|png|jpeg|found|enabled|disabled|error" || true
    # dzsave support is validated post-build by the deploy smoke test, not here.
    # meson setup output above already confirms "image pyramid save with libarchive: YES".

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

    # Copy vipsthumbnail CLI binary (used for preview generation)
    mkdir -p /out/bin
    for vbin in vipsthumbnail vips; do
      VBIN_PATH=$(find /staging/opt -name "$vbin" -type f 2>/dev/null | head -1)
      if [ -n "$VBIN_PATH" ]; then
        cp "$VBIN_PATH" /out/bin/
        echo "  bundled binary: $vbin"
      else
        echo "  WARNING: $vbin not found in staging"
      fi
    done

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
      libtiff.so.5
      libwebp.so.7
      libjbig.so.2.1
      libxml2.so.2
      libbz2.so.1
      libarchive.so.13
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

    echo "--- Patching CLI tool RPATH (vips, vipsthumbnail) ---"
    # The layer CLI tools ship with empty RPATH; handlers exec them with only
    # LD_LIBRARY_PATH=/opt/lib, which would resolve libarchive'"'"'s crypto
    # chain from the pinned runtime system — the CR10/CR11 coupling class.
    dnf install -y patchelf 2>&1 | tail -1
    for tool in /out/bin/vips /out/bin/vipsthumbnail; do
      [ -f "$tool" ] || continue
      patchelf --remove-rpath "$tool" 2>/dev/null || true
      patchelf --force-rpath --set-rpath /opt/lib:/opt/vipsdeps "$tool"
      echo "  patched: $tool -> RPATH=/opt/lib:/opt/vipsdeps"
    done

    echo "--- Vendoring libarchive crypto chain into vipsdeps/ ---"
    # libarchive needs libcrypto/lzma/zstd/lz4. They must NOT live in
    # /opt/lib (python searches it via LD_LIBRARY_PATH and a layer libcrypto
    # newer than the pinned runtime breaks `import ssl` — the 2026-06
    # outage class). They go in /opt/vipsdeps, reachable only through the
    # vips binaries DT_RPATH, so the binary stack is version-consistent
    # with the layer and python never loads any of it.
    mkdir -p /out/vipsdeps
    for lib in libcrypto.so liblzma.so libzstd.so liblz4.so; do
      FOUND=$(find /usr/lib64 /lib64 -name "${lib}*" 2>/dev/null | head -1)
      if [ -z "$FOUND" ]; then echo "  WARNING: $lib not found"; continue; fi
      LIBDIR=$(dirname "$FOUND")
      BASENAME=$(basename "$FOUND" | sed "s/\.so.*//")
      for f in "${LIBDIR}/${BASENAME}".so*; do
        if [ -L "$f" ]; then
          cp "$(readlink -f "$f")" "/out/vipsdeps/$(basename "$f")"
        elif [ -f "$f" ]; then
          cp "$f" "/out/vipsdeps/$(basename "$f")"
        fi
      done
      echo "  vendored: $lib"
    done

    echo "--- Layer contents ---"
    ls -lh /out/lib/*.so* 2>/dev/null || true
    ls -lh /out/vipsdeps/*.so* 2>/dev/null || true
    echo "--- Done ---"
  '

echo "=== Creating layer zip ==="
cd "$OUTDIR"
# Zip lib/, bin/ (if exists), and include/ at root level
ZIPDIRS="lib/ vipsdeps/ include/"
if [ -d "$OUTDIR/bin" ] && [ "$(ls -A "$OUTDIR/bin" 2>/dev/null)" ]; then
    ZIPDIRS="lib/ vipsdeps/ bin/ include/"
fi
zip -r9 "$OUTDIR/libvips-layer.zip" $ZIPDIRS
echo "Layer zip: $OUTDIR/libvips-layer.zip ($(du -h libvips-layer.zip | cut -f1))"
