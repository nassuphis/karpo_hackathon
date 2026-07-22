#!/bin/bash
# Docker runtime regression test for deploy binaries.
# Uses deploy binaries (lambda/sweep, lambda/sweep_mt, lambda/sweep_cm) — NOT sweep_test.
# Must pass before deploy. Runs inside ARM64 Docker.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LAPACK_BUILD="$ROOT/lambda/layer-build-lapack"
LIBVIPS_BUILD="$ROOT/lambda/layer-build"

echo "=== Docker Runtime Regression Test ==="

# CR32 F7: a green Docker run must never certify stale binaries — fail up
# front if any deploy binary is older than its sources (transitive includes).
python3 "$ROOT/scripts/check_binary_freshness.py" --check

# Verify binaries exist
for BIN in "$ROOT/lambda/sweep" "$ROOT/lambda/sweep_mt" "$ROOT/lambda/sweep_cm" "$ROOT/lambda/sweep_coeffgen" "$ROOT/lambda/roots2pix_mt" "$ROOT/lambda/solve_palette_chunk_mt" "$ROOT/lambda/solve_proximity_stats" "$ROOT/lambda/solve_proximity_hist_sectioned" "$ROOT/lambda/score_raw_render" "$ROOT/lambda/bilevel_section_raster" "$ROOT/lambda/coeffs_bilevel_raster" "$ROOT/lambda/bilevel_merge" "$ROOT/lambda/raw_to_bilevel" "$ROOT/lambda/assemble_greyscale"; do
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

if [ ! -d "$ROOT/lambda/solve_proximity_hist_sectioned_lib" ]; then
    echo "FATAL: solve_proximity_hist_sectioned_lib not found. Run deploy.sh to compile."
    exit 1
fi

if [ ! -d "$ROOT/lambda/roots2pix_mt_lib" ]; then
    echo "FATAL: roots2pix_mt_lib not found. Run deploy.sh to compile."
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
    # Merge both layers into /opt (Lambda runtime layout). vipsdeps is the
    # vendored libarchive crypto chain (CR10/CR11) and is REQUIRED: the vips
    # binaries and CLI tools carry RPATH /opt/lib:/opt/vipsdeps.
    cp -a /opt-lapack/lib /opt/ 2>/dev/null || true
    cp -a /opt-vips/lib/* /opt/lib/ 2>/dev/null || true
    if [ ! -d /opt-vips/vipsdeps ]; then
        echo "FATAL: layer-build/vipsdeps missing; rebuild lambda/build-libvips-layer.sh"
        exit 1
    fi
    cp -a /opt-vips/vipsdeps /opt/
    mkdir -p /opt/bin
    cp -a /opt-vips/bin/* /opt/bin/ 2>/dev/null || true
    # Production rule: only layer paths on LD_LIBRARY_PATH. The staged
    # libcurl closures are exercised hermetically below via zip layouts;
    # the python test process must not see them (the 2026-06 outage class).
    export LD_LIBRARY_PATH=/opt/lib
    export PATH="/opt/bin:$PATH"

    echo "--- Hermetic loader-resolution assertions ---"
    # libcurl binaries: zip layout (binary + lib/), DT_RPATH must resolve
    # the entire closure from $ORIGIN/lib with NO env help.
    for b in roots2pix_mt assemble_greyscale solve_proximity_hist_sectioned solve_palette_chunk_mt; do
        mkdir -p "/hermetic/$b/lib"
        cp "/src/$b" "/hermetic/$b/"
        cp "/src/${b}_lib/"* "/hermetic/$b/lib/"
        NF=$(env -u LD_LIBRARY_PATH ldd "/hermetic/$b/$b" | grep -c "not found" || true)
        SYS_CURL=$(env -u LD_LIBRARY_PATH ldd "/hermetic/$b/$b" | grep "libcurl" | grep -c "/hermetic/" || true)
        if [ "$NF" != "0" ] || [ "$SYS_CURL" != "1" ]; then
            echo "FATAL: $b zip-layout closure broken (not_found=$NF curl_from_layout=$SYS_CURL)"
            env -u LD_LIBRARY_PATH ldd "/hermetic/$b/$b" | grep -E "not found|curl"
            exit 1
        fi
        echo "  $b: hermetic closure OK"
    done
    # vips binaries + layer CLI tools: libarchive crypto chain must resolve
    # from /opt/vipsdeps, never the container system.
    for b in /src/raw2jpeg /src/score_raw_render /src/bilevel_merge /src/raw_to_bilevel /src/tiff_compat /src/png_export /src/dz_export /src/sheet_stitch /src/autolevels_render /opt/bin/vips /opt/bin/vipsthumbnail /opt/bin/vipsheader; do
        [ -f "$b" ] || { echo "FATAL: missing $b"; exit 1; }
        BAD=$(ldd "$b" 2>/dev/null | grep -E "libcrypto|liblzma|libzstd|liblz4" | grep -vc "/opt/vipsdeps" || true)
        if [ "$BAD" != "0" ]; then
            echo "FATAL: $b resolves libarchive chain outside /opt/vipsdeps:"
            ldd "$b" | grep -E "libcrypto|liblzma|libzstd|liblz4"
            exit 1
        fi
        echo "  $(basename $b): crypto chain from /opt/vipsdeps OK"
    done
    echo "--- Loader assertions PASSED ---"

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
