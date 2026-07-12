#!/usr/bin/env bash
# CR32 F7: rebuild EVERY deploy binary from the current tree, then write the
# build manifest (lambda/binary_manifest.json). Build commands mirror
# deploy.sh exactly — this script exists so "rebuild all" can never silently
# mean "rebuild the musl subset" again (the CR31 failure mode).
#
# Requires: aarch64-linux-musl-gcc on PATH, Docker running (libcurl + LAPACK
# builds run in ARM64 amazonlinux containers), lambda/layer-build-lapack.
set -euo pipefail
cd "$(dirname "$0")/.."
SCRIPT_DIR="$(pwd)"

echo "== musl static binaries =="
aarch64-linux-musl-gcc -O3 -static -pthread -o lambda/sweep lambda/sweep_cli.c -lm
aarch64-linux-musl-gcc -O3 -static -pthread -o lambda/sweep_mt lambda/sweep_mt.c -lm
aarch64-linux-musl-gcc -O3 -static -o lambda/coeffs_bilevel_raster lambda/coeffs_bilevel_raster.c -lm
aarch64-linux-musl-gcc -O3 -static -o lambda/bilevel_section_raster lambda/bilevel_section_raster.c -lm
aarch64-linux-musl-gcc -O3 -static -pthread -o lambda/solve_proximity_stats lambda/solve_proximity_stats.c -lm
aarch64-linux-musl-gcc -O3 -static -o lambda/solve_palette_debug lambda/solve_palette_debug.c -lm
aarch64-linux-musl-gcc -O3 -static -o lambda/solve_palette_chunk lambda/solve_palette_chunk.c -lm
aarch64-linux-musl-gcc -O3 -static -o lambda/palette_bins_render lambda/palette_bins_render.c -lm
aarch64-linux-musl-gcc -O3 -static -o lambda/step_scores_to_palette_raw lambda/step_scores_to_palette_raw.c -lm
echo "musl set done"

echo "== local test binary =="
cc -O2 -pthread -o lambda/sweep_test lambda/sweep_cli.c -lm

build_libcurl_binary() {
    local BIN="$1"; shift
    local GCC_INPUTS="/src/$BIN.c"
    local EXTRA
    for EXTRA in "$@"; do
        GCC_INPUTS="$GCC_INPUTS /src/$EXTRA"
    done
    echo "  $BIN (Docker ARM64, dynamic libcurl)..."
    docker run --rm --platform linux/arm64 \
        -v "$SCRIPT_DIR/lambda:/src" \
        -e "BIN=$BIN" -e "GCC_INPUTS=$GCC_INPUTS" \
        public.ecr.aws/amazonlinux/amazonlinux:2023 \
        bash -c '
            set -euo pipefail
            dnf install -y gcc libcurl-devel 2>&1 | tail -1
            gcc -O3 -pthread -o "/src/$BIN" $GCC_INPUTS \
                -lcurl -lm -Wl,--disable-new-dtags -Wl,-rpath,\$ORIGIN/lib
            rm -rf "/src/${BIN}_lib"
            mkdir -p "/src/${BIN}_lib"
            for lib in $(ldd "/src/$BIN" | awk "/=> \// {print \$3}"); do
                base=$(basename "$lib")
                case "$base" in
                    libc.so.*|libm.so.*|libpthread.so.*|ld-linux-aarch64.so.*|libdl.so.*|librt.so.*|libresolv.so.*|libnss_*.so.*)
                        continue
                        ;;
                esac
                cp -L "$lib" "/src/${BIN}_lib/"
            done
            echo "  $BIN staged libs: $(ls /src/${BIN}_lib)"
        '
}

echo "== Docker libcurl binaries =="
build_libcurl_binary roots2pix_mt multispan_reader.c
build_libcurl_binary solve_proximity_hist_sectioned multispan_reader.c
build_libcurl_binary solve_palette_chunk_mt multispan_reader.c
build_libcurl_binary assemble_greyscale

echo "== Docker libvips binaries =="
LAYER_BUILD="$SCRIPT_DIR/lambda/layer-build"
if [ ! -d "$LAYER_BUILD/lib" ] || [ ! -d "$LAYER_BUILD/include" ]; then
    echo "ERROR: libvips layer-build directory missing. Run lambda/build-libvips-layer.sh first."
    exit 1
fi
docker run --rm --platform linux/arm64 \
    -v "$SCRIPT_DIR/lambda:/src" \
    -v "$LAYER_BUILD:/opt" \
    public.ecr.aws/amazonlinux/amazonlinux:2023 \
    bash -c '
        set -euo pipefail
        dnf install -y gcc glib2-devel libtiff-devel 2>&1 | tail -1
        VIPS_CFLAGS="-I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include"
        VIPS_LIBS="-L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm -Wl,--disable-new-dtags -Wl,-rpath,/opt/lib:/opt/vipsdeps"
        VIPS_TIFF_LIBS="-L/opt/lib -lvips -ltiff -lgobject-2.0 -lglib-2.0 -lm -Wl,--disable-new-dtags -Wl,-rpath,/opt/lib:/opt/vipsdeps"
        gcc -O3 -o /src/raw2jpeg /src/raw2jpeg.c $VIPS_CFLAGS $VIPS_LIBS
        gcc -O3 -o /src/score_raw_render /src/score_raw_render.c $VIPS_CFLAGS $VIPS_LIBS
        gcc -O3 -o /src/bilevel_merge /src/bilevel_merge.c $VIPS_CFLAGS $VIPS_TIFF_LIBS
        gcc -O3 -o /src/raw_to_bilevel /src/raw_to_bilevel.c $VIPS_CFLAGS $VIPS_TIFF_LIBS
        gcc -O3 -o /src/tiff_compat /src/tiff_compat.c $VIPS_CFLAGS $VIPS_TIFF_LIBS
        gcc -O3 -o /src/png_export /src/png_export.c $VIPS_CFLAGS $VIPS_LIBS
        gcc -O3 -o /src/dz_export /src/dz_export.c $VIPS_CFLAGS $VIPS_LIBS
        gcc -O3 -o /src/wall_dz /src/wall_dz.c $VIPS_CFLAGS $VIPS_LIBS
        gcc -O3 -o /src/autolevels_render /src/autolevels_render.c $VIPS_CFLAGS $VIPS_LIBS
        echo "  libvips set compiled"
    '

echo "== Docker LAPACK binaries =="
LAPACK_BUILD="$SCRIPT_DIR/lambda/layer-build-lapack"
if [ ! -d "$LAPACK_BUILD/lib" ]; then
    echo "ERROR: LAPACK layer-build directory missing. Run lambda/build-lapack-layer.sh first."
    exit 1
fi
docker run --rm --platform linux/arm64 \
    -v "$SCRIPT_DIR/lambda:/src" \
    -v "$LAPACK_BUILD:/opt" \
    public.ecr.aws/amazonlinux/amazonlinux:2023 \
    bash -c '
        set -euo pipefail
        dnf install -y gcc 2>&1 | tail -1
        export LD_LIBRARY_PATH=/opt/lib
        gcc -O3 -pthread -DHAVE_LAPACK_COMPANION -o /src/sweep_coeffgen /src/sweep_cli.c \
            -L/opt/lib -llapack -lopenblas -lm -Wl,-rpath,/opt/lib
        gcc -O3 -o /src/sweep_cm /src/sweep_cm.c \
            -L/opt/lib -llapack -lopenblas -lm -Wl,-rpath,/opt/lib
        echo "  sweep_coeffgen + sweep_cm compiled"
    '

echo "== manifest + freshness =="
# write FIRST: after a rebuild the old manifest legitimately mismatches the
# fresh binaries; --check then validates mtimes AND the new hashes agree.
python3 scripts/check_binary_freshness.py --write-manifest
python3 scripts/check_binary_freshness.py --check
echo "ALL DEPLOY BINARIES REBUILT"
