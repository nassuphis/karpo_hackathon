#!/bin/bash
# Deploy the polypaint Lambda functions
# Usage: ./deploy.sh [create|update]
#
# 8 Lambdas:
#   polypaint-sweep        — multi-process root solver (sweep binary)
#   polypaint-coeffgen     — composable coefficient generation (sweep binary, coeffgen mode)
#   polypaint-raster       — bin→tile-bucketed .pix sparse pixel output (roots2pix binary)
#   polypaint-finalize     — assemble .pix files into .raw tile (pixassemble binary)
#   polypaint-encode       — raw→JPEG/PNG encoding (raw2jpeg binary, needs libvips)
#   polypaint-viewport     — compute viewport from lores.bin (pure Python)
#   polypaint-storage      — S3 list/delete/metadata (pure Python)
#   polypaint-dispatch     — async Lambda invocation dispatcher (pure Python)

set -euo pipefail
export AWS_PAGER=""

SWEEP_NAME="polypaint-sweep"
ENCODE_NAME="polypaint-encode"
VIEWPORT_NAME="polypaint-viewport"
STORAGE_NAME="polypaint-storage"
DISPATCH_NAME="polypaint-dispatch"
RASTER_NAME="polypaint-raster"
FINALIZE_NAME="polypaint-finalize"
COEFFGEN_NAME="polypaint-coeffgen"
PREVIEW_NAME="polypaint-preview"
BILEVEL_NAME="polypaint-bilevel"
ROLE_NAME="polypaint-lambda-role"
REGION="us-east-1"
API_NAME="polypaint-api"
RUNTIME="python3.12"
ARCH="arm64"
SWEEP_MEMORY=10240    # 6 vCPUs for multi-process sweep
ENCODE_MEMORY=1769    # 1 vCPU + libvips for JPEG/PNG encoding
ENCODE_EPHEMERAL=3072 # 3GB /tmp for preview generation from large images
VIEWPORT_MEMORY=512   # pure Python
STORAGE_MEMORY=512    # pure Python
DISPATCH_MEMORY=1769  # 1 vCPU — 50 threads doing SSL need real CPU
RASTER_MEMORY=1769    # 1 vCPU, roots2pix (no canvas allocation)
FINALIZE_MEMORY=1769  # 1 vCPU, pixassemble (64 MB tile buffer for 4096²)
COEFFGEN_MEMORY=1769  # 1 vCPU, coefficient generation (no solver, striped)
PREVIEW_MEMORY=1024   # pure Python, PNG encoding via zlib (512 OOMs on large lores)
BILEVEL_MEMORY=1769   # 1 vCPU, bilevel raster + merge
TIFF_COMPAT_NAME="polypaint-tiff-compat"
TIFF_COMPAT_MEMORY=4096  # needs RAM for scanline buffer on large images
PNG_EXPORT_NAME="polypaint-png-export"
PNG_EXPORT_MEMORY=4096  # libvips PNG encode
DZ_EXPORT_NAME="polypaint-deepzoom-export"
DZ_EXPORT_MEMORY=4096  # libvips dzsave + parallel S3 upload
PARAM_DEBUG_NAME="polypaint-param-debug"
PARAM_DEBUG_MEMORY=1769  # 1 vCPU + libvips for TIFF output
BILEVEL_STITCH_NAME="polypaint-bilevel-stitch"
BILEVEL_STITCH_MEMORY=6144  # ~4 vCPUs, libvips multithreaded stitch
RENDER_PREVIEW_NAME="polypaint-render-preview"
RENDER_PREVIEW_MEMORY=4096  # libvips vipsthumbnail on large images
SOLVE_PROXIMITY_NAME="polypaint-solve-proximity"
SOLVE_PROXIMITY_MEMORY=1769  # 1 vCPU, solve_proximity_stats binary
RENDER_ORCHESTRATOR_NAME="polypaint-render-orchestrator"
RENDER_ORCHESTRATOR_MEMORY=512  # starter only — validates + starts Step Functions
RENDER_PLAN_NAME="polypaint-render-plan"
RENDER_PLAN_MEMORY=512
RENDER_STATUS_NAME="polypaint-render-status"
RENDER_STATUS_MEMORY=256
RENDER_STATE_MACHINE_NAME="polypaint-render-workflow"
BINARY_TMP=10240      # /tmp size for Lambdas that process raw images (max 10GB)
TIMEOUT=900
BUCKET="polypaint"
JOBS_TABLE="polypaint-jobs"
LIBVIPS_LAYER_NAME="polypaint-libvips"
LAPACK_LAYER_NAME="polypaint-lapack"
# These get set dynamically by build_and_publish_layers
LIBVIPS_LAYER=""
LAPACK_LAYER=""
SWEEP_CM_NAME="polypaint-sweep-cm"
SWEEP_CM_MEMORY=4096  # companion matrix eigensolve needs more memory

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

# --- S3 website bucket setup (idempotent) ---
ensure_bucket_website() {
    echo "Ensuring S3 website config..."

    # Website hosting
    aws s3api put-bucket-website --bucket "$BUCKET" \
        --website-configuration '{"IndexDocument":{"Suffix":"index.html"}}' \
        --region "$REGION"
    echo "  website hosting: OK"

    # Public access block off (required for anonymous GetObject)
    aws s3api put-public-access-block --bucket "$BUCKET" \
        --public-access-block-configuration \
        '{"BlockPublicAcls":false,"IgnorePublicAcls":false,"BlockPublicPolicy":false,"RestrictPublicBuckets":false}' \
        --region "$REGION"
    echo "  public access block: disabled"

    # Bucket policy: anonymous read on all objects
    aws s3api put-bucket-policy --bucket "$BUCKET" --region "$REGION" \
        --policy "{
  \"Version\": \"2012-10-17\",
  \"Statement\": [
    {
      \"Sid\": \"PublicReadSiteAssets\",
      \"Effect\": \"Allow\",
      \"Principal\": \"*\",
      \"Action\": \"s3:GetObject\",
      \"Resource\": \"arn:aws:s3:::${BUCKET}/*\"
    }
  ]
}"
    echo "  bucket policy: public read on ${BUCKET}/*"
}

# --- Deployed frontend smoke test ---
verify_frontend_assets() {
    local SITE_URL="http://${BUCKET}.s3-website-${REGION}.amazonaws.com"
    echo "Verifying deployed frontend assets..."
    for asset in index.html coeff_func_catalog_js.js; do
        local STATUS
        STATUS=$(curl -s -o /dev/null -w "%{http_code}" "${SITE_URL}/${asset}")
        if [ "$STATUS" != "200" ]; then
            echo "FATAL: ${SITE_URL}/${asset} returned HTTP ${STATUS} (expected 200)"
            exit 1
        fi
        echo "  ${asset}: HTTP ${STATUS} OK"
    done
}

# --- JS syntax check ---
echo "Checking index.html JS syntax..."
sed -n '/<script>/,/<\/script>/p' index.html | sed '1d;$d' > /tmp/_jscheck.js
node --check /tmp/_jscheck.js 2>&1
if [ $? -ne 0 ]; then
    rm -f /tmp/_jscheck.js
    echo "ERROR: index.html has JavaScript syntax errors. Aborting deploy."
    exit 1
fi
rm -f /tmp/_jscheck.js
echo "  JS syntax OK"

# --- Build and publish Lambda layers ---
echo ""
echo "Building and publishing Lambda layers..."

build_and_publish_layer() {
    local LAYER_NAME="$1" BUILD_SCRIPT="$2" ZIP_NAME="$3" BUILD_DIR="$4" ARN_FILE="$5"
    local ZIP_PATH="$BUILD_DIR/$ZIP_NAME"
    local HASH_FILE="$BUILD_DIR/.build_hash"
    local SCRIPT_HASH
    SCRIPT_HASH=$(shasum "$BUILD_SCRIPT" | cut -d' ' -f1)

    # Skip rebuild if zip exists and build script hasn't changed
    if [ -f "$ZIP_PATH" ] && [ -f "$HASH_FILE" ] && [ "$(cat "$HASH_FILE")" = "$SCRIPT_HASH" ]; then
        echo "  $LAYER_NAME: zip up to date (skipping rebuild)"
    else
        echo "  $LAYER_NAME: building..."
        bash "$BUILD_SCRIPT"
        if [ $? -ne 0 ]; then
            echo "FATAL: $BUILD_SCRIPT failed"
            exit 1
        fi
        if [ ! -f "$ZIP_PATH" ]; then
            echo "FATAL: $ZIP_PATH not found after build"
            exit 1
        fi
        echo "$SCRIPT_HASH" > "$HASH_FILE"
    fi

    echo "  $LAYER_NAME: publishing layer version..."
    aws lambda publish-layer-version \
        --layer-name "$LAYER_NAME" \
        --zip-file "fileb://$ZIP_PATH" \
        --compatible-runtimes python3.12 python3.13 \
        --compatible-architectures arm64 \
        --region "$REGION" \
        --query 'LayerVersionArn' --output text > "$ARN_FILE"
    if [ $? -ne 0 ] || [ ! -s "$ARN_FILE" ]; then
        echo "FATAL: failed to publish $LAYER_NAME layer"
        exit 1
    fi
    echo "  $LAYER_NAME: $(cat "$ARN_FILE")"
}

build_and_publish_layer \
    "$LIBVIPS_LAYER_NAME" \
    "$SCRIPT_DIR/lambda/build-libvips-layer.sh" \
    "libvips-layer.zip" \
    "$SCRIPT_DIR/lambda/layer-build" \
    /tmp/_libvips_layer_arn
LIBVIPS_LAYER=$(cat /tmp/_libvips_layer_arn)

build_and_publish_layer \
    "$LAPACK_LAYER_NAME" \
    "$SCRIPT_DIR/lambda/build-lapack-layer.sh" \
    "lapack-layer.zip" \
    "$SCRIPT_DIR/lambda/layer-build-lapack" \
    /tmp/_lapack_layer_arn
LAPACK_LAYER=$(cat /tmp/_lapack_layer_arn)

echo "  LIBVIPS_LAYER=$LIBVIPS_LAYER"
echo "  LAPACK_LAYER=$LAPACK_LAYER"

# --- Regenerate catalog artifacts ---
# Step 1: Generate C lookup header from catalog JSON (no binary needed)
echo "Generating C lookup header from catalog..."
(cd lambda && python3 -c "
from gen_catalog import load_catalog, generate_c_header, H_OUT
catalog = load_catalog()
h = generate_c_header(catalog)
with open(H_OUT, 'w') as f:
    f.write(h)
print(f'  coeff_func_lookup.h: {len(catalog)} entries')
") || { echo "FATAL: C header generation failed"; exit 1; }

# Step 2: Build host binary (now uses fresh lookup header)
echo "Building sweep_test (host, for probing)..."
cc -O2 -o lambda/sweep_test lambda/sweep_cli.c -lm

# Step 3: Probe degrees and generate JS catalog
echo "Generating JS catalog (probing degrees)..."
(cd lambda && python3 -c "
from gen_catalog import load_catalog, load_metrics, generate_js, JS_OUT
catalog = load_catalog()
metrics = load_metrics()
js = generate_js(catalog, metrics)
with open(JS_OUT, 'w') as f:
    f.write(js)
import re
count = len(re.findall(r'\"name\":', js))
print(f'  coeff_func_catalog_js.js: {count} entries')
" 2>&1) || { echo "FATAL: JS catalog generation failed"; exit 1; }

# --- Frontend JS execution test ---
echo "Running frontend JS execution test..."
bash "$SCRIPT_DIR/tests/test_frontend_js.sh" || { echo "FATAL: Frontend JS test failed"; exit 1; }

# --- Compile binaries ---
echo "Compiling binaries..."

echo "  sweep (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/sweep lambda/sweep_cli.c -lm

echo "  roots2pix (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/roots2pix lambda/roots2pix.c -lm

echo "  pixassemble (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/pixassemble lambda/pixassemble.c -lm

echo "  bilevel_raster (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/bilevel_raster lambda/bilevel_raster.c -lm

echo "  coeffs_bilevel_raster (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/coeffs_bilevel_raster lambda/coeffs_bilevel_raster.c -lm

echo "  solve_proximity_stats (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/solve_proximity_stats lambda/solve_proximity_stats.c -lm

# param_gen removed — param debug now uses sweep in param_dump mode

echo "  raw2jpeg (Docker ARM64, dynamically linked against libvips)..."
LAYER_BUILD="$SCRIPT_DIR/lambda/layer-build"
if [ ! -d "$LAYER_BUILD/lib" ] || [ ! -d "$LAYER_BUILD/include" ]; then
    echo "ERROR: layer-build directory missing. Run lambda/build-libvips-layer.sh first."
    exit 1
fi
docker run --rm --platform linux/arm64 \
    -v "$SCRIPT_DIR/lambda:/src" \
    -v "$LAYER_BUILD:/opt" \
    public.ecr.aws/amazonlinux/amazonlinux:2023 \
    bash -c '
        set -euo pipefail
        dnf install -y gcc glib2-devel libtiff-devel 2>&1 | tail -1
        gcc -O3 -o /src/raw2jpeg /src/raw2jpeg.c \
            -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
            -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include \
            -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm \
            -Wl,-rpath,/opt/lib
        echo "  raw2jpeg compiled: $(file /src/raw2jpeg)"
        gcc -O3 -o /src/bilevel_merge /src/bilevel_merge.c \
            -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
            -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include \
            -L/opt/lib -lvips -ltiff -lgobject-2.0 -lglib-2.0 -lm \
            -Wl,-rpath,/opt/lib
        echo "  bilevel_merge compiled: $(file /src/bilevel_merge)"
        gcc -O3 -o /src/tiff_compat /src/tiff_compat.c \
            -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
            -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include \
            -L/opt/lib -lvips -ltiff -lgobject-2.0 -lglib-2.0 -lm \
            -Wl,-rpath,/opt/lib
        echo "  tiff_compat compiled: $(file /src/tiff_compat)"
        gcc -O3 -o /src/png_export /src/png_export.c \
            -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
            -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include \
            -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm \
            -Wl,-rpath,/opt/lib
        echo "  png_export compiled: $(file /src/png_export)"
        gcc -O3 -o /src/dz_export /src/dz_export.c \
            -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
            -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include \
            -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm \
            -Wl,-rpath,/opt/lib
        echo "  dz_export compiled: $(file /src/dz_export)"

        # ── Runtime smoke tests ──────────────────────────────────────
        # Verify libvips ops actually work, not just that binaries compile.
        echo ""
        echo "--- Runtime smoke tests ---"
        export LD_LIBRARY_PATH=/opt/lib

        # 1. Probe required libvips operations
        cat > /tmp/probe_ops.c <<PROBE
#include <stdio.h>
#include <vips/vips.h>
int main(int argc, char **argv) {
    if (VIPS_INIT(argv[0])) { fprintf(stderr, "VIPS_INIT failed\\n"); return 1; }
    const char *ops[] = {"tiffsave", "pngsave", "dzsave", NULL};
    int fail = 0;
    for (int i = 0; ops[i]; i++) {
        GType t = vips_type_find("VipsForeignSave", ops[i]);
        if (t == 0) { fprintf(stderr, "MISSING: %s\\n", ops[i]); fail = 1; }
        else { printf("  OK: %s\\n", ops[i]); }
    }
    vips_shutdown();
    return fail;
}
PROBE
        gcc -O2 -o /tmp/probe_ops /tmp/probe_ops.c \
            -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
            -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include \
            -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm -Wl,-rpath,/opt/lib
        /tmp/probe_ops || { echo "FATAL: libvips missing required operations"; exit 1; }

        # 2. Create a tiny test TIFF (8x8 white)
        cat > /tmp/make_test_tif.c <<TIFTEST
#include <stdio.h>
#include <vips/vips.h>
int main(int argc, char **argv) {
    if (VIPS_INIT(argv[0])) return 1;
    VipsImage *img = vips_image_new_matrix(8, 8);
    if (!img) return 1;
    vips_image_set_int(img, "bands", 1);
    /* black+white 8x8 via vips_black then cast */
    VipsImage *bw = NULL;
    if (vips_black(&bw, 8, 8, "bands", 1, NULL)) return 1;
    if (vips_tiffsave(bw, "/tmp/test_8x8.tif", NULL)) {
        fprintf(stderr, "tiffsave failed: %s\\n", vips_error_buffer());
        return 1;
    }
    g_object_unref(bw);
    g_object_unref(img);
    printf("  test TIFF written\\n");
    vips_shutdown();
    return 0;
}
TIFTEST
        gcc -O2 -o /tmp/make_test_tif /tmp/make_test_tif.c \
            -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
            -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include \
            -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm -Wl,-rpath,/opt/lib
        /tmp/make_test_tif || { echo "FATAL: cannot create test TIFF"; exit 1; }

        # 3. Smoke test dz_export (actual DeepZoom generation)
        /src/dz_export /tmp/test_8x8.tif /tmp/dz_test/image || \
            { echo "FATAL: dz_export failed on test TIFF"; exit 1; }
        if [ ! -f /tmp/dz_test/image.dzi ]; then
            echo "FATAL: dz_export did not produce .dzi file"
            exit 1
        fi
        TILE_COUNT=$(find /tmp/dz_test/image_files -name "*.png" 2>/dev/null | wc -l)
        if [ "$TILE_COUNT" -eq 0 ]; then
            echo "FATAL: dz_export produced no tile PNGs"
            exit 1
        fi
        echo "  dz_export: .dzi + $TILE_COUNT tiles OK"

        # 4. Smoke test png_export
        /src/png_export /tmp/test_8x8.tif /tmp/test_out.png || \
            { echo "FATAL: png_export failed on test TIFF"; exit 1; }
        if [ ! -f /tmp/test_out.png ]; then
            echo "FATAL: png_export did not produce output PNG"
            exit 1
        fi
        echo "  png_export OK"

        # 5. Smoke test tiff_compat
        /src/tiff_compat /tmp/test_8x8.tif /tmp/test_compat.tif || \
            { echo "FATAL: tiff_compat failed on test TIFF"; exit 1; }
        if [ ! -f /tmp/test_compat.tif ]; then
            echo "FATAL: tiff_compat did not produce output TIFF"
            exit 1
        fi
        echo "  tiff_compat OK"

        echo "--- All smoke tests passed ---"
    '

# Compile sweep_cm (companion matrix solver, needs LAPACK/OpenBLAS)
echo "  sweep_cm (Docker ARM64, dynamically linked against LAPACK)..."
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
        gcc -O3 -o /src/sweep_cm /src/sweep_cm.c \
            -L/opt/lib -llapack -lopenblas -lm -Wl,-rpath,/opt/lib
        echo "  sweep_cm compiled: $(file /src/sweep_cm)"
        # Smoke test: known cubic x^3 - 6x^2 + 11x - 6
        python3 -c "
import struct
data = b\"\"
for c in [1, -6, 11, -6]:
    data += struct.pack(\"<ff\", float(c), 0.0)
with open(\"/tmp/test_cf.bin\", \"wb\") as f:
    f.write(data)
"
        echo "{\"mode\":\"solve_cm\",\"coeffs_file\":\"/tmp/test_cf.bin\",\"n_coeffs\":4,\"n_steps\":1}" | /src/sweep_cm /tmp/test_out.bin > /dev/null
        python3 -c "
import struct
with open(\"/tmp/test_out.bin\", \"rb\") as f: data = f.read()
roots = sorted([abs(struct.unpack_from(\"<ff\", data, i*8)[0]) for i in range(3)])
assert abs(roots[0]-1)<0.01 and abs(roots[1]-2)<0.01 and abs(roots[2]-3)<0.01, f\"Bad roots: {roots}\"
print(\"  sweep_cm smoke test: PASSED (roots 1,2,3)\")
"
    '

# --- Docker Runtime Regression Test (AE + CM) ---
echo ""
echo "Running Docker runtime regression test..."
bash "$SCRIPT_DIR/scripts/test-docker-runtime.sh" || { echo "FATAL: Docker runtime test failed"; exit 1; }

# --- Package Lambdas ---
echo "Packaging Lambdas..."

# Sweep: handler_sweep.py + shared.py + sweep
SWEEP_DIR=/tmp/polypaint-sweep
rm -rf "$SWEEP_DIR"
mkdir -p "$SWEEP_DIR"
cp lambda/handler_sweep.py lambda/shared.py "$SWEEP_DIR/"
cp lambda/sweep "$SWEEP_DIR/"
chmod +x "$SWEEP_DIR"/sweep
cd "$SWEEP_DIR" && zip -r9 /tmp/polypaint-sweep.zip . -q && cd "$SCRIPT_DIR"
echo "  Sweep:    $(du -h /tmp/polypaint-sweep.zip | cut -f1)  (sweep)"

# Coeffgen: handler_coeffgen.py + shared.py + sweep
COEFFGEN_DIR=/tmp/polypaint-coeffgen
rm -rf "$COEFFGEN_DIR"
mkdir -p "$COEFFGEN_DIR"
cp lambda/handler_coeffgen.py lambda/shared.py "$COEFFGEN_DIR/"
cp lambda/sweep "$COEFFGEN_DIR/"
chmod +x "$COEFFGEN_DIR"/sweep
cd "$COEFFGEN_DIR" && zip -r9 /tmp/polypaint-coeffgen.zip . -q && cd "$SCRIPT_DIR"
echo "  Coeffgen: $(du -h /tmp/polypaint-coeffgen.zip | cut -f1)  (sweep)"

# Encode: handler_encode.py + shared.py + raw2jpeg
ENCODE_DIR=/tmp/polypaint-encode
rm -rf "$ENCODE_DIR"
mkdir -p "$ENCODE_DIR"
cp lambda/handler_encode.py lambda/shared.py "$ENCODE_DIR/"
cp lambda/raw2jpeg "$ENCODE_DIR/"
chmod +x "$ENCODE_DIR"/raw2jpeg
cd "$ENCODE_DIR" && zip -r9 /tmp/polypaint-encode.zip . -q && cd "$SCRIPT_DIR"
echo "  Encode:   $(du -h /tmp/polypaint-encode.zip | cut -f1)  (raw2jpeg + libvips layer)"

# Viewport: handler_viewport.py + shared.py (pure Python)
VIEWPORT_DIR=/tmp/polypaint-viewport
rm -rf "$VIEWPORT_DIR"
mkdir -p "$VIEWPORT_DIR"
cp lambda/handler_viewport.py lambda/shared.py "$VIEWPORT_DIR/"
cd "$VIEWPORT_DIR" && zip -r9 /tmp/polypaint-viewport.zip . -q && cd "$SCRIPT_DIR"
echo "  Viewport: $(du -h /tmp/polypaint-viewport.zip | cut -f1)  (pure Python)"

# Storage: handler_storage.py + shared.py (pure Python)
STORAGE_DIR=/tmp/polypaint-storage
rm -rf "$STORAGE_DIR"
mkdir -p "$STORAGE_DIR"
cp lambda/handler_storage.py lambda/shared.py "$STORAGE_DIR/"
cd "$STORAGE_DIR" && zip -r9 /tmp/polypaint-storage.zip . -q && cd "$SCRIPT_DIR"
echo "  Storage:  $(du -h /tmp/polypaint-storage.zip | cut -f1)  (pure Python)"

# Dispatch: handler_dispatch.py + shared.py (pure Python, invokes render Lambdas)
DISPATCH_DIR=/tmp/polypaint-dispatch
rm -rf "$DISPATCH_DIR"
mkdir -p "$DISPATCH_DIR"
cp lambda/handler_dispatch.py lambda/shared.py "$DISPATCH_DIR/"
cd "$DISPATCH_DIR" && zip -r9 /tmp/polypaint-dispatch.zip . -q && cd "$SCRIPT_DIR"
echo "  Dispatch: $(du -h /tmp/polypaint-dispatch.zip | cut -f1)  (pure Python)"

# Raster: handler_raster.py + shared.py + roots2pix
RASTER_DIR=/tmp/polypaint-raster
rm -rf "$RASTER_DIR"
mkdir -p "$RASTER_DIR"
cp lambda/handler_raster.py lambda/shared.py "$RASTER_DIR/"
cp lambda/roots2pix "$RASTER_DIR/"
chmod +x "$RASTER_DIR"/roots2pix
cd "$RASTER_DIR" && zip -r9 /tmp/polypaint-raster.zip . -q && cd "$SCRIPT_DIR"
echo "  Raster:   $(du -h /tmp/polypaint-raster.zip | cut -f1)  (roots2pix)"

# Finalize: handler_finalize.py + shared.py + pixassemble
FINALIZE_DIR=/tmp/polypaint-finalize
rm -rf "$FINALIZE_DIR"
mkdir -p "$FINALIZE_DIR"
cp lambda/handler_finalize.py lambda/shared.py "$FINALIZE_DIR/"
cp lambda/pixassemble "$FINALIZE_DIR/"
chmod +x "$FINALIZE_DIR"/pixassemble
cd "$FINALIZE_DIR" && zip -r9 /tmp/polypaint-finalize.zip . -q && cd "$SCRIPT_DIR"
echo "  Finalize: $(du -h /tmp/polypaint-finalize.zip | cut -f1)  (pixassemble)"

# Preview: handler_preview.py + shared.py (pure Python, PNG via zlib)
PREVIEW_DIR=/tmp/polypaint-preview
rm -rf "$PREVIEW_DIR"
mkdir -p "$PREVIEW_DIR"
cp lambda/handler_preview.py lambda/shared.py "$PREVIEW_DIR/"
cd "$PREVIEW_DIR" && zip -r9 /tmp/polypaint-preview.zip . -q && cd "$SCRIPT_DIR"
echo "  Preview:  $(du -h /tmp/polypaint-preview.zip | cut -f1)  (pure Python)"

# Bilevel: handler_bilevel.py + shared.py + bilevel (needs libvips layer)
BILEVEL_DIR=/tmp/polypaint-bilevel
rm -rf "$BILEVEL_DIR"
mkdir -p "$BILEVEL_DIR"
cp lambda/handler_bilevel.py lambda/shared.py "$BILEVEL_DIR/"
cp lambda/bilevel_raster lambda/coeffs_bilevel_raster lambda/bilevel_merge "$BILEVEL_DIR/"
chmod +x "$BILEVEL_DIR"/bilevel_raster "$BILEVEL_DIR"/coeffs_bilevel_raster "$BILEVEL_DIR"/bilevel_merge
cd "$BILEVEL_DIR" && zip -r9 /tmp/polypaint-bilevel.zip . -q && cd "$SCRIPT_DIR"
echo "  Bilevel:  $(du -h /tmp/polypaint-bilevel.zip | cut -f1)  (bilevel raster+merge)"

# Bilevel Stitch: handler_bilevel_stitch.py + shared.py + bilevel_merge (needs libvips layer)
BILEVEL_STITCH_DIR=/tmp/polypaint-bilevel-stitch
rm -rf "$BILEVEL_STITCH_DIR"
mkdir -p "$BILEVEL_STITCH_DIR"
cp lambda/handler_bilevel_stitch.py lambda/shared.py "$BILEVEL_STITCH_DIR/"
cp lambda/bilevel_merge "$BILEVEL_STITCH_DIR/"
chmod +x "$BILEVEL_STITCH_DIR"/bilevel_merge
cd "$BILEVEL_STITCH_DIR" && zip -r9 /tmp/polypaint-bilevel-stitch.zip . -q && cd "$SCRIPT_DIR"
echo "  BiStitch: $(du -h /tmp/polypaint-bilevel-stitch.zip | cut -f1)  (bilevel stitch + libvips layer)"

# Param Debug: handler_param_debug.py + shared.py + bilevel_merge (needs libvips layer)
PARAM_DEBUG_DIR=/tmp/polypaint-param-debug
rm -rf "$PARAM_DEBUG_DIR"
mkdir -p "$PARAM_DEBUG_DIR"
cp lambda/handler_param_debug.py lambda/shared.py "$PARAM_DEBUG_DIR/"
cp lambda/sweep lambda/bilevel_merge "$PARAM_DEBUG_DIR/"
chmod +x "$PARAM_DEBUG_DIR"/sweep "$PARAM_DEBUG_DIR"/bilevel_merge
cd "$PARAM_DEBUG_DIR" && zip -r9 /tmp/polypaint-param-debug.zip . -q && cd "$SCRIPT_DIR"
echo "  ParamDbg: $(du -h /tmp/polypaint-param-debug.zip | cut -f1)  (param debug + libvips layer)"

# TIFF Compat: handler_tiff_compat.py + shared.py + tiff_compat (needs libtiff from layer)
TIFF_COMPAT_DIR=/tmp/polypaint-tiff-compat
rm -rf "$TIFF_COMPAT_DIR"
mkdir -p "$TIFF_COMPAT_DIR"
cp lambda/handler_tiff_compat.py lambda/shared.py "$TIFF_COMPAT_DIR/"
cp lambda/tiff_compat "$TIFF_COMPAT_DIR/"
chmod +x "$TIFF_COMPAT_DIR"/tiff_compat
cd "$TIFF_COMPAT_DIR" && zip -r9 /tmp/polypaint-tiff-compat.zip . -q && cd "$SCRIPT_DIR"
echo "  TiffCmp: $(du -h /tmp/polypaint-tiff-compat.zip | cut -f1)  (tiff_compat + libtiff layer)"

# PNG Export: handler_png_export.py + shared.py + png_export (needs libvips layer)
PNG_EXPORT_DIR=/tmp/polypaint-png-export
rm -rf "$PNG_EXPORT_DIR"
mkdir -p "$PNG_EXPORT_DIR"
cp lambda/handler_png_export.py lambda/shared.py "$PNG_EXPORT_DIR/"
cp lambda/png_export "$PNG_EXPORT_DIR/"
chmod +x "$PNG_EXPORT_DIR"/png_export
cd "$PNG_EXPORT_DIR" && zip -r9 /tmp/polypaint-png-export.zip . -q && cd "$SCRIPT_DIR"
echo "  PngExp:  $(du -h /tmp/polypaint-png-export.zip | cut -f1)  (png_export + libvips layer)"

# Render Preview: handler_render_preview.py + shared.py (needs libvips layer for vipsthumbnail)
RENDER_PREVIEW_DIR=/tmp/polypaint-render-preview
rm -rf "$RENDER_PREVIEW_DIR"
mkdir -p "$RENDER_PREVIEW_DIR"
cp lambda/handler_render_preview.py lambda/shared.py "$RENDER_PREVIEW_DIR/"
cd "$RENDER_PREVIEW_DIR" && zip -r9 /tmp/polypaint-render-preview.zip . -q && cd "$SCRIPT_DIR"
echo "  RndPrev: $(du -h /tmp/polypaint-render-preview.zip | cut -f1)  (vipsthumbnail via libvips layer)"

# DeepZoom Export: handler_deepzoom_export.py + shared.py + dz_export (needs libvips layer)
DZ_EXPORT_DIR=/tmp/polypaint-deepzoom-export
rm -rf "$DZ_EXPORT_DIR"
mkdir -p "$DZ_EXPORT_DIR"
cp lambda/handler_deepzoom_export.py lambda/shared.py "$DZ_EXPORT_DIR/"
cp lambda/dz_export "$DZ_EXPORT_DIR/"
chmod +x "$DZ_EXPORT_DIR"/dz_export
cd "$DZ_EXPORT_DIR" && zip -r9 /tmp/polypaint-deepzoom-export.zip . -q && cd "$SCRIPT_DIR"
echo "  DzExp:   $(du -h /tmp/polypaint-deepzoom-export.zip | cut -f1)  (dz_export + libvips layer)"

# Solve Proximity: handler_solve_proximity.py + shared.py + solve_proximity_stats binary
SP_DIR=/tmp/polypaint-solve-proximity
rm -rf "$SP_DIR"
mkdir -p "$SP_DIR"
cp lambda/handler_solve_proximity.py lambda/shared.py "$SP_DIR/"
cp lambda/solve_proximity_stats "$SP_DIR/"
chmod +x "$SP_DIR"/solve_proximity_stats
cd "$SP_DIR" && zip -r9 /tmp/polypaint-solve-proximity.zip . -q && cd "$SCRIPT_DIR"
echo "  SolvPrx: $(du -h /tmp/polypaint-solve-proximity.zip | cut -f1)  (solve_proximity_stats binary)"

# Render Orchestrator (starter): handler_render_orchestrator.py + shared.py
ORCH_DIR=/tmp/polypaint-render-orchestrator
rm -rf "$ORCH_DIR"
mkdir -p "$ORCH_DIR"
cp lambda/handler_render_orchestrator.py lambda/shared.py "$ORCH_DIR/"
cd "$ORCH_DIR" && zip -r9 /tmp/polypaint-render-orchestrator.zip . -q && cd "$SCRIPT_DIR"
echo "  RndOrch: $(du -h /tmp/polypaint-render-orchestrator.zip | cut -f1)  (starter only)"

# Render Plan: handler_render_plan.py + shared.py
PLAN_DIR=/tmp/polypaint-render-plan
rm -rf "$PLAN_DIR"
mkdir -p "$PLAN_DIR"
cp lambda/handler_render_plan.py lambda/shared.py "$PLAN_DIR/"
cd "$PLAN_DIR" && zip -r9 /tmp/polypaint-render-plan.zip . -q && cd "$SCRIPT_DIR"
echo "  RndPlan: $(du -h /tmp/polypaint-render-plan.zip | cut -f1)  (plan builder)"

# Render Status: handler_render_status.py + shared.py
STATUS_DIR=/tmp/polypaint-render-status
rm -rf "$STATUS_DIR"
mkdir -p "$STATUS_DIR"
cp lambda/handler_render_status.py lambda/shared.py "$STATUS_DIR/"
cd "$STATUS_DIR" && zip -r9 /tmp/polypaint-render-status.zip . -q && cd "$SCRIPT_DIR"
echo "  RndStat: $(du -h /tmp/polypaint-render-status.zip | cut -f1)  (status updater)"

# Sweep-CM: handler_sweep_cm.py + shared.py + sweep_cm (needs LAPACK layer)
CM_DIR=/tmp/polypaint-sweep-cm
rm -rf "$CM_DIR"
mkdir -p "$CM_DIR"
cp lambda/handler_sweep_cm.py lambda/shared.py "$CM_DIR/"
cp lambda/sweep_cm "$CM_DIR/"
chmod +x "$CM_DIR"/sweep_cm
cd "$CM_DIR" && zip -r9 /tmp/polypaint-sweep-cm.zip . -q && cd "$SCRIPT_DIR"
echo "  SweepCM: $(du -h /tmp/polypaint-sweep-cm.zip | cut -f1)  (sweep_cm + LAPACK layer)"

ACTION="${1:-create}"

# Helper: create a Lambda function
create_lambda() {
    local NAME="$1" HANDLER="$2" ZIP="$3" MEM="$4" ROLE="$5" LAYERS="${6:-}" ENV_VARS="$7" TMP="${8:-512}"
    echo "Creating $NAME ($MEM MB, /tmp=${TMP}MB)..."
    local LAYER_ARGS=""
    if [ -n "$LAYERS" ]; then
        LAYER_ARGS="--layers $LAYERS"
    fi
    aws lambda create-function \
        --function-name "$NAME" \
        --runtime "$RUNTIME" \
        --architectures "$ARCH" \
        --handler "$HANDLER" \
        --zip-file "fileb://$ZIP" \
        --role "$ROLE" \
        --memory-size "$MEM" \
        --timeout "$TIMEOUT" \
        --ephemeral-storage "{\"Size\": $TMP}" \
        --region "$REGION" \
        $LAYER_ARGS \
        --environment "Variables={$ENV_VARS}" \
        --query 'FunctionArn' --output text
}

# Helper: update a Lambda function
update_lambda() {
    local NAME="$1" HANDLER="$2" ZIP="$3" MEM="$4" LAYERS="${5:-}" ENV_VARS="$6" TMP="${7:-512}"
    echo "Updating $NAME code..."
    aws lambda update-function-code \
        --function-name "$NAME" \
        --zip-file "fileb://$ZIP" \
        --region "$REGION" \
        --query 'FunctionArn' --output text 2>/dev/null || {
        echo "  $NAME doesn't exist yet, creating..."
        ROLE_ARN=$(aws iam get-role --role-name "$ROLE_NAME" --query 'Role.Arn' --output text)
        create_lambda "$NAME" "$HANDLER" "$ZIP" "$MEM" "$ROLE_ARN" "$LAYERS" "$ENV_VARS"
        return
    }

    echo "Updating $NAME config ($MEM MB)..."
    aws lambda wait function-updated --function-name "$NAME" --region "$REGION" 2>/dev/null || true
    local LAYER_ARGS="--layers []"
    if [ -n "$LAYERS" ]; then
        LAYER_ARGS="--layers $LAYERS"
    fi
    aws lambda update-function-configuration \
        --function-name "$NAME" \
        --handler "$HANDLER" \
        --memory-size "$MEM" \
        --ephemeral-storage "{\"Size\": $TMP}" \
        $LAYER_ARGS \
        --environment "Variables={$ENV_VARS}" \
        --region "$REGION" \
        --query 'MemorySize' --output text
}

# Helper: ensure API Gateway HTTP API exists with routes to each Lambda
setup_api_gateway() {
    local ACCT
    ACCT=$(aws sts get-caller-identity --query 'Account' --output text)

    # Create or get API
    local API_ID
    API_ID=$(aws apigatewayv2 get-apis --region "$REGION" \
        --query "Items[?Name=='$API_NAME'].ApiId" --output text 2>/dev/null)
    if [ -z "$API_ID" ] || [ "$API_ID" = "None" ]; then
        API_ID=$(aws apigatewayv2 create-api --name "$API_NAME" \
            --protocol-type HTTP \
            --cors-configuration '{"AllowOrigins":["*"],"AllowMethods":["POST","OPTIONS"],"AllowHeaders":["content-type"]}' \
            --region "$REGION" --query 'ApiId' --output text)
        # Create default stage with auto-deploy
        aws apigatewayv2 create-stage --api-id "$API_ID" \
            --stage-name '$default' --auto-deploy \
            --region "$REGION" >/dev/null 2>&1
        echo "  Created API Gateway: $API_ID"
    else
        echo "  Using existing API Gateway: $API_ID"
    fi

    # Find or create integration for a Lambda and return its ID
    create_integration() {
        local FNAME="$1"
        local TARGET_URI="arn:aws:lambda:$REGION:$ACCT:function:$FNAME"
        # Check if an integration already exists for this Lambda
        local EXISTING
        EXISTING=$(aws apigatewayv2 get-integrations --api-id "$API_ID" --region "$REGION" \
            --query "Items[?IntegrationUri=='${TARGET_URI}'].IntegrationId | [0]" --output text 2>/dev/null)
        if [ -n "$EXISTING" ] && [ "$EXISTING" != "None" ]; then
            echo "$EXISTING"
            return
        fi
        aws apigatewayv2 create-integration --api-id "$API_ID" \
            --integration-type AWS_PROXY \
            --integration-uri "$TARGET_URI" \
            --payload-format-version "2.0" \
            --region "$REGION" --query 'IntegrationId' --output text
    }

    # Create or update a route
    ensure_route() {
        local ROUTE_KEY="$1" INT_ID="$2"
        local EXISTING
        EXISTING=$(aws apigatewayv2 get-routes --api-id "$API_ID" --region "$REGION" \
            --query "Items[?RouteKey=='${ROUTE_KEY}'].RouteId" --output text 2>/dev/null)
        if [ -n "$EXISTING" ] && [ "$EXISTING" != "None" ]; then
            aws apigatewayv2 update-route --api-id "$API_ID" --route-id "$EXISTING" \
                --target "integrations/$INT_ID" --region "$REGION" >/dev/null 2>&1
        else
            aws apigatewayv2 create-route --api-id "$API_ID" \
                --route-key "$ROUTE_KEY" --target "integrations/$INT_ID" \
                --region "$REGION" >/dev/null 2>&1
        fi
    }

    # Grant API Gateway permission to invoke each Lambda
    for FNAME in "$SWEEP_NAME" "$COEFFGEN_NAME" "$ENCODE_NAME" "$VIEWPORT_NAME" "$STORAGE_NAME" "$DISPATCH_NAME" "$RASTER_NAME" "$FINALIZE_NAME" "$PREVIEW_NAME" "$BILEVEL_NAME" "$BILEVEL_STITCH_NAME" "$PARAM_DEBUG_NAME" "$TIFF_COMPAT_NAME" "$PNG_EXPORT_NAME" "$DZ_EXPORT_NAME" "$SWEEP_CM_NAME"; do
        aws lambda add-permission --function-name "$FNAME" \
            --statement-id "apigateway-invoke" \
            --action lambda:InvokeFunction \
            --principal apigateway.amazonaws.com \
            --source-arn "arn:aws:execute-api:$REGION:$ACCT:$API_ID/*/*" \
            --region "$REGION" >/dev/null 2>&1 || true
    done

    # Create integrations
    echo "  Creating integrations..."
    local SWEEP_INT COEFFGEN_INT ENCODE_INT VIEWPORT_INT STORAGE_INT DISPATCH_INT
    SWEEP_INT=$(create_integration "$SWEEP_NAME")
    COEFFGEN_INT=$(create_integration "$COEFFGEN_NAME")
    ENCODE_INT=$(create_integration "$ENCODE_NAME")
    VIEWPORT_INT=$(create_integration "$VIEWPORT_NAME")
    STORAGE_INT=$(create_integration "$STORAGE_NAME")
    DISPATCH_INT=$(create_integration "$DISPATCH_NAME")

    # Create routes
    echo "  Setting up routes..."
    ensure_route "POST /sweep" "$SWEEP_INT"
    ensure_route "POST /coeffgen" "$COEFFGEN_INT"

    local RASTER_INT FINALIZE_INT PREVIEW_INT
    RASTER_INT=$(create_integration "$RASTER_NAME")
    FINALIZE_INT=$(create_integration "$FINALIZE_NAME")
    PREVIEW_INT=$(create_integration "$PREVIEW_NAME")
    local PARAM_DEBUG_INT
    PARAM_DEBUG_INT=$(create_integration "$PARAM_DEBUG_NAME")
    ensure_route "POST /raster" "$RASTER_INT"
    ensure_route "POST /finalize" "$FINALIZE_INT"
    ensure_route "POST /preview" "$PREVIEW_INT"
    ensure_route "POST /param-debug" "$PARAM_DEBUG_INT"
    local TIFF_COMPAT_INT
    TIFF_COMPAT_INT=$(create_integration "$TIFF_COMPAT_NAME")
    ensure_route "POST /tiff-compat" "$TIFF_COMPAT_INT"
    local PNG_EXPORT_INT
    PNG_EXPORT_INT=$(create_integration "$PNG_EXPORT_NAME")
    ensure_route "POST /png-export" "$PNG_EXPORT_INT"
    local DZ_EXPORT_INT
    DZ_EXPORT_INT=$(create_integration "$DZ_EXPORT_NAME")
    ensure_route "POST /deepzoom-export" "$DZ_EXPORT_INT"
    local SWEEP_CM_INT
    SWEEP_CM_INT=$(create_integration "$SWEEP_CM_NAME")
    ensure_route "POST /sweep-cm" "$SWEEP_CM_INT"

    ensure_route "POST /encode-upload" "$ENCODE_INT"
    ensure_route "POST /viewport" "$VIEWPORT_INT"
    ensure_route "POST /list" "$STORAGE_INT"
    ensure_route "POST /delete" "$STORAGE_INT"
    ensure_route "POST /save-metadata" "$STORAGE_INT"
    ensure_route "POST /cleanup" "$STORAGE_INT"
    ensure_route "POST /clean-render" "$STORAGE_INT"
    ensure_route "POST /check-keys" "$STORAGE_INT"
    ensure_route "POST /check-status" "$STORAGE_INT"
    ensure_route "POST /presign" "$STORAGE_INT"
    ensure_route "POST /detail" "$STORAGE_INT"
    ensure_route "POST /list-prefix" "$STORAGE_INT"
    ensure_route "POST /head-keys" "$STORAGE_INT"
    ensure_route "POST /delete-task" "$STORAGE_INT"
    ensure_route "POST /render-summary" "$STORAGE_INT"
    ensure_route "POST /dispatch-render" "$DISPATCH_INT"

    # Get API URL and write config.json
    local API_URL
    API_URL=$(aws apigatewayv2 get-api --api-id "$API_ID" --region "$REGION" \
        --query 'ApiEndpoint' --output text)
    echo "  API Gateway URL: $API_URL"

    printf '{
  "sweep": "%s/sweep",
  "coeffgen": "%s/coeffgen",
  "raster": "%s/raster",
  "finalize": "%s/finalize",
  "encode": "%s/encode-upload",
  "viewport": "%s/viewport",
  "preview": "%s/preview",
  "storage": "%s",
  "dispatch": "%s/dispatch-render",
  "param-debug": "%s/param-debug",
  "tiff-compat": "%s/tiff-compat",
  "png-export": "%s/png-export",
  "deepzoom-export": "%s/deepzoom-export",
  "sweep-cm": "%s/sweep-cm"
}' "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" \
    | aws s3 cp - "s3://$BUCKET/config.json" \
        --content-type "application/json" --region "$REGION"
    echo "  config.json uploaded"
}

if [ "$ACTION" = "create" ]; then
    # --- Create IAM role ---
    echo "Creating IAM role..."
    TRUST_POLICY='{
        "Version": "2012-10-17",
        "Statement": [{
            "Effect": "Allow",
            "Principal": {"Service": "lambda.amazonaws.com"},
            "Action": "sts:AssumeRole"
        }]
    }'

    ROLE_ARN=$(aws iam create-role \
        --role-name "$ROLE_NAME" \
        --assume-role-policy-document "$TRUST_POLICY" \
        --query 'Role.Arn' --output text 2>/dev/null || \
        aws iam get-role --role-name "$ROLE_NAME" --query 'Role.Arn' --output text)

    echo "Role ARN: $ROLE_ARN"

    # Attach policies
    aws iam attach-role-policy --role-name "$ROLE_NAME" \
        --policy-arn arn:aws:iam::aws:policy/service-role/AWSLambdaBasicExecutionRole 2>/dev/null || true

    # S3 access
    S3_POLICY="{
        \"Version\": \"2012-10-17\",
        \"Statement\": [{
            \"Effect\": \"Allow\",
            \"Action\": [\"s3:PutObject\", \"s3:GetObject\", \"s3:ListBucket\", \"s3:DeleteObject\"],
            \"Resource\": [
                \"arn:aws:s3:::${BUCKET}\",
                \"arn:aws:s3:::${BUCKET}/*\"
            ]
        }]
    }"
    aws iam put-role-policy --role-name "$ROLE_NAME" \
        --policy-name polypaint-s3-access \
        --policy-document "$S3_POLICY"

    # Lambda invoke access (for dispatch Lambda to invoke render Lambdas)
    ACCT=$(aws sts get-caller-identity --query 'Account' --output text)
    LAMBDA_POLICY="{
        \"Version\": \"2012-10-17\",
        \"Statement\": [{
            \"Effect\": \"Allow\",
            \"Action\": \"lambda:InvokeFunction\",
            \"Resource\": \"arn:aws:lambda:${REGION}:${ACCT}:function:polypaint-*\"
        }]
    }"
    aws iam put-role-policy --role-name "$ROLE_NAME" \
        --policy-name polypaint-lambda-invoke \
        --policy-document "$LAMBDA_POLICY"

    # DynamoDB access (for status tracking)
    DDB_POLICY="{
        \"Version\": \"2012-10-17\",
        \"Statement\": [{
            \"Effect\": \"Allow\",
            \"Action\": [\"dynamodb:PutItem\", \"dynamodb:Query\", \"dynamodb:BatchWriteItem\"],
            \"Resource\": \"arn:aws:dynamodb:${REGION}:${ACCT}:table/${JOBS_TABLE}\"
        }]
    }"
    aws iam put-role-policy --role-name "$ROLE_NAME" \
        --policy-name polypaint-dynamodb-access \
        --policy-document "$DDB_POLICY"

    # Create DynamoDB status tracking table (on-demand, auto-scales)
    echo "Creating DynamoDB table ${JOBS_TABLE}..."
    aws dynamodb create-table \
        --table-name "$JOBS_TABLE" \
        --attribute-definitions \
            AttributeName=job_id,AttributeType=S \
            AttributeName=task_id,AttributeType=S \
        --key-schema \
            AttributeName=job_id,KeyType=HASH \
            AttributeName=task_id,KeyType=RANGE \
        --billing-mode PAY_PER_REQUEST \
        --region "$REGION" 2>/dev/null || echo "  Table already exists"
    aws dynamodb update-time-to-live \
        --table-name "$JOBS_TABLE" \
        --time-to-live-specification "Enabled=true,AttributeName=ttl" \
        --region "$REGION" 2>/dev/null || true

    echo "Waiting for role to propagate..."
    sleep 10

    # --- Create all Lambdas ---
    create_lambda "$SWEEP_NAME" "handler_sweep.handler" "/tmp/polypaint-sweep.zip" \
        "$SWEEP_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE"

    create_lambda "$COEFFGEN_NAME" "handler_coeffgen.handler" "/tmp/polypaint-coeffgen.zip" \
        "$COEFFGEN_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE" "$BINARY_TMP"

    create_lambda "$ENCODE_NAME" "handler_encode.handler" "/tmp/polypaint-encode.zip" \
        "$ENCODE_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    create_lambda "$VIEWPORT_NAME" "handler_viewport.handler" "/tmp/polypaint-viewport.zip" \
        "$VIEWPORT_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET"

    create_lambda "$STORAGE_NAME" "handler_storage.handler" "/tmp/polypaint-storage.zip" \
        "$STORAGE_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE"
    # Reserve concurrency for storage so it's never starved by render/merge Lambdas
    aws lambda put-function-concurrency --function-name "$STORAGE_NAME" \
        --reserved-concurrent-executions 5 --region "$REGION"

    create_lambda "$DISPATCH_NAME" "handler_dispatch.handler" "/tmp/polypaint-dispatch.zip" \
        "$DISPATCH_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,RASTER_FUNCTION=$RASTER_NAME,FINALIZE_FUNCTION=$FINALIZE_NAME,ENCODE_FUNCTION=$ENCODE_NAME,SWEEP_FUNCTION=$SWEEP_NAME,BILEVEL_FUNCTION=$BILEVEL_NAME,BILEVEL_STITCH_FUNCTION=$BILEVEL_STITCH_NAME,DZ_EXPORT_FUNCTION=$DZ_EXPORT_NAME,COEFFGEN_FUNCTION=$COEFFGEN_NAME,SWEEP_CM_FUNCTION=$SWEEP_CM_NAME,RENDER_PREVIEW_FUNCTION=$RENDER_PREVIEW_NAME,SOLVE_PROXIMITY_FUNCTION=$SOLVE_PROXIMITY_NAME,RENDER_ORCHESTRATOR_FUNCTION=$RENDER_ORCHESTRATOR_NAME"
    # Reserve concurrency for dispatch so it's never starved by render/merge Lambdas
    aws lambda put-function-concurrency --function-name "$DISPATCH_NAME" \
        --reserved-concurrent-executions 5 --region "$REGION"

    create_lambda "$RASTER_NAME" "handler_raster.handler" "/tmp/polypaint-raster.zip" \
        "$RASTER_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE" "$BINARY_TMP"

    create_lambda "$FINALIZE_NAME" "handler_finalize.handler" "/tmp/polypaint-finalize.zip" \
        "$FINALIZE_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE" "$BINARY_TMP"

    create_lambda "$PREVIEW_NAME" "handler_preview.handler" "/tmp/polypaint-preview.zip" \
        "$PREVIEW_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET"

    create_lambda "$BILEVEL_NAME" "handler_bilevel.handler" "/tmp/polypaint-bilevel.zip" \
        "$BILEVEL_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    create_lambda "$BILEVEL_STITCH_NAME" "handler_bilevel_stitch.handler" "/tmp/polypaint-bilevel-stitch.zip" \
        "$BILEVEL_STITCH_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    create_lambda "$PARAM_DEBUG_NAME" "handler_param_debug.handler" "/tmp/polypaint-param-debug.zip" \
        "$PARAM_DEBUG_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,LD_LIBRARY_PATH=/opt/lib"

    create_lambda "$TIFF_COMPAT_NAME" "handler_tiff_compat.handler" "/tmp/polypaint-tiff-compat.zip" \
        "$TIFF_COMPAT_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    create_lambda "$PNG_EXPORT_NAME" "handler_png_export.handler" "/tmp/polypaint-png-export.zip" \
        "$PNG_EXPORT_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    create_lambda "$DZ_EXPORT_NAME" "handler_deepzoom_export.handler" "/tmp/polypaint-deepzoom-export.zip" \
        "$DZ_EXPORT_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    create_lambda "$RENDER_PREVIEW_NAME" "handler_render_preview.handler" "/tmp/polypaint-render-preview.zip" \
        "$RENDER_PREVIEW_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    create_lambda "$SOLVE_PROXIMITY_NAME" "handler_solve_proximity.handler" "/tmp/polypaint-solve-proximity.zip" \
        "$SOLVE_PROXIMITY_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE" "$BINARY_TMP"

    # Render plan + status Lambdas
    create_lambda "$RENDER_PLAN_NAME" "handler_render_plan.handler" "/tmp/polypaint-render-plan.zip" \
        "$RENDER_PLAN_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,VIEWPORT_FUNCTION=$VIEWPORT_NAME,STORAGE_FUNCTION=$STORAGE_NAME"

    create_lambda "$RENDER_STATUS_NAME" "handler_render_status.handler" "/tmp/polypaint-render-status.zip" \
        "$RENDER_STATUS_MEMORY" "$ROLE_ARN" "" "JOBS_TABLE=$JOBS_TABLE"

    # Step Functions state machine
    echo "Deploying Step Functions state machine..."
    ACCT=$(aws sts get-caller-identity --query 'Account' --output text)

    # Create Step Functions execution role
    SFN_ROLE_NAME="polypaint-sfn-execution-role"
    SFN_TRUST='{
        "Version": "2012-10-17",
        "Statement": [{
            "Effect": "Allow",
            "Principal": {"Service": "states.amazonaws.com"},
            "Action": "sts:AssumeRole"
        }]
    }'
    SFN_ROLE_ARN=$(aws iam create-role \
        --role-name "$SFN_ROLE_NAME" \
        --assume-role-policy-document "$SFN_TRUST" \
        --query 'Role.Arn' --output text 2>/dev/null || \
        aws iam get-role --role-name "$SFN_ROLE_NAME" --query 'Role.Arn' --output text)

    aws iam put-role-policy --role-name "$SFN_ROLE_NAME" \
        --policy-name polypaint-sfn-lambda-invoke \
        --policy-document "{
            \"Version\": \"2012-10-17\",
            \"Statement\": [{
                \"Effect\": \"Allow\",
                \"Action\": \"lambda:InvokeFunction\",
                \"Resource\": \"arn:aws:lambda:${REGION}:${ACCT}:function:polypaint-*\"
            }]
        }"

    # Render ASL template with actual Lambda ARNs
    RENDER_PLAN_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${RENDER_PLAN_NAME}"
    RENDER_STATUS_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${RENDER_STATUS_NAME}"
    RASTER_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${RASTER_NAME}"
    FINALIZE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${FINALIZE_NAME}"
    ENCODE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${ENCODE_NAME}"
    STORAGE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${STORAGE_NAME}"
    BILEVEL_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${BILEVEL_NAME}"
    BILEVEL_STITCH_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${BILEVEL_STITCH_NAME}"
    SOLVE_PROXIMITY_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${SOLVE_PROXIMITY_NAME}"

    sed -e "s|\${PlanFunctionArn}|${RENDER_PLAN_ARN}|g" \
        -e "s|\${StatusFunctionArn}|${RENDER_STATUS_ARN}|g" \
        -e "s|\${RasterFunctionArn}|${RASTER_ARN}|g" \
        -e "s|\${FinalizeFunctionArn}|${FINALIZE_ARN}|g" \
        -e "s|\${EncodeFunctionArn}|${ENCODE_ARN}|g" \
        -e "s|\${StorageFunctionArn}|${STORAGE_ARN}|g" \
        -e "s|\${BilevelFunctionArn}|${BILEVEL_ARN}|g" \
        -e "s|\${BilevelStitchFunctionArn}|${BILEVEL_STITCH_ARN}|g" \
        -e "s|\${SolveProximityFunctionArn}|${SOLVE_PROXIMITY_ARN}|g" \
        stepfunctions/render_workflow.asl.json.template > /tmp/render_workflow.asl.json

    # Create or update state machine
    RENDER_SM_ARN=$(aws stepfunctions create-state-machine \
        --name "$RENDER_STATE_MACHINE_NAME" \
        --definition "file:///tmp/render_workflow.asl.json" \
        --role-arn "$SFN_ROLE_ARN" \
        --type STANDARD \
        --region "$REGION" \
        --query 'stateMachineArn' --output text 2>/dev/null || \
        aws stepfunctions update-state-machine \
            --state-machine-arn "arn:aws:states:${REGION}:${ACCT}:stateMachine:${RENDER_STATE_MACHINE_NAME}" \
            --definition "file:///tmp/render_workflow.asl.json" \
            --role-arn "$SFN_ROLE_ARN" \
            --region "$REGION" \
            --query 'updateDate' --output text)
    echo "  State machine: $RENDER_STATE_MACHINE_NAME ($RENDER_SM_ARN)"

    RENDER_SM_ARN="arn:aws:states:${REGION}:${ACCT}:stateMachine:${RENDER_STATE_MACHINE_NAME}"

    # Starter Lambda — now only needs state machine ARN, not all worker names
    create_lambda "$RENDER_ORCHESTRATOR_NAME" "handler_render_orchestrator.handler" "/tmp/polypaint-render-orchestrator.zip" \
        "$RENDER_ORCHESTRATOR_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,RENDER_STATE_MACHINE_ARN=$RENDER_SM_ARN"

    # Add states:StartExecution permission to Lambda role
    aws iam put-role-policy --role-name "$ROLE_NAME" \
        --policy-name polypaint-sfn-start \
        --policy-document "{
            \"Version\": \"2012-10-17\",
            \"Statement\": [{
                \"Effect\": \"Allow\",
                \"Action\": \"states:StartExecution\",
                \"Resource\": \"${RENDER_SM_ARN}\"
            }]
        }"

    create_lambda "$SWEEP_CM_NAME" "handler_sweep_cm.handler" "/tmp/polypaint-sweep-cm.zip" \
        "$SWEEP_CM_MEMORY" "$ROLE_ARN" "$LAPACK_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    # Async invoke config: no retries for most Lambdas (prevents retry storms),
    # but bilevel gets 2 retries / 1hr age to handle concurrency throttle drops.
    for fn in "$RASTER_NAME" "$FINALIZE_NAME" "$BILEVEL_STITCH_NAME" "$DZ_EXPORT_NAME" "$RENDER_PREVIEW_NAME" "$SOLVE_PROXIMITY_NAME"; do
        aws lambda put-function-event-invoke-config \
            --function-name "$fn" \
            --maximum-retry-attempts 0 \
            --maximum-event-age-in-seconds 300 \
            --region "$REGION" >/dev/null 2>&1
    done
    aws lambda put-function-event-invoke-config \
        --function-name "$BILEVEL_NAME" \
        --maximum-retry-attempts 2 \
        --maximum-event-age-in-seconds 3600 \
        --region "$REGION" >/dev/null 2>&1
    # Orchestrator: no retries (self-reinvokes), long event age
    aws lambda put-function-event-invoke-config \
        --function-name "$RENDER_ORCHESTRATOR_NAME" \
        --maximum-retry-attempts 0 \
        --maximum-event-age-in-seconds 3600 \
        --region "$REGION" >/dev/null 2>&1

    # --- Set up API Gateway routes ---
    echo ""
    echo "Setting up API Gateway..."
    setup_api_gateway

    # Ensure bucket is configured for website hosting
    ensure_bucket_website

    # Upload frontend assets to S3
    echo "Uploading frontend assets to S3..."
    aws s3 cp "$SCRIPT_DIR/index.html" "s3://$BUCKET/index.html" \
        --content-type "text/html" --region "$REGION"
    aws s3 cp "$SCRIPT_DIR/coeff_func_catalog_js.js" "s3://$BUCKET/coeff_func_catalog_js.js" \
        --content-type "application/javascript" --region "$REGION"
    verify_frontend_assets

    echo ""
    echo "=== DEPLOYED ==="
    echo "  Sweep:    $SWEEP_NAME ($SWEEP_MEMORY MB)"
    echo "  Coeffgen: $COEFFGEN_NAME ($COEFFGEN_MEMORY MB)"
    echo "  Raster:   $RASTER_NAME ($RASTER_MEMORY MB)"
    echo "  Finalize: $FINALIZE_NAME ($FINALIZE_MEMORY MB)"
    echo "  Encode:   $ENCODE_NAME ($ENCODE_MEMORY MB)"
    echo "  Viewport: $VIEWPORT_NAME ($VIEWPORT_MEMORY MB)"
    echo "  Storage:  $STORAGE_NAME ($STORAGE_MEMORY MB)"
    echo "  Dispatch: $DISPATCH_NAME ($DISPATCH_MEMORY MB)"
    echo "  Preview:  $PREVIEW_NAME ($PREVIEW_MEMORY MB)"
    echo "  Bilevel:  $BILEVEL_NAME ($BILEVEL_MEMORY MB)"
    echo "  BiStitch: $BILEVEL_STITCH_NAME ($BILEVEL_STITCH_MEMORY MB)"

elif [ "$ACTION" = "update" ]; then
    update_lambda "$SWEEP_NAME" "handler_sweep.handler" "/tmp/polypaint-sweep.zip" \
        "$SWEEP_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE"

    update_lambda "$COEFFGEN_NAME" "handler_coeffgen.handler" "/tmp/polypaint-coeffgen.zip" \
        "$COEFFGEN_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE" "$BINARY_TMP"

    update_lambda "$ENCODE_NAME" "handler_encode.handler" "/tmp/polypaint-encode.zip" \
        "$ENCODE_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    update_lambda "$VIEWPORT_NAME" "handler_viewport.handler" "/tmp/polypaint-viewport.zip" \
        "$VIEWPORT_MEMORY" "" "BUCKET=$BUCKET"

    update_lambda "$STORAGE_NAME" "handler_storage.handler" "/tmp/polypaint-storage.zip" \
        "$STORAGE_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE"
    # Reserve concurrency for storage so it's never starved by render/merge Lambdas
    aws lambda put-function-concurrency --function-name "$STORAGE_NAME" \
        --reserved-concurrent-executions 5 --region "$REGION"

    update_lambda "$DISPATCH_NAME" "handler_dispatch.handler" "/tmp/polypaint-dispatch.zip" \
        "$DISPATCH_MEMORY" "" "BUCKET=$BUCKET,RASTER_FUNCTION=$RASTER_NAME,FINALIZE_FUNCTION=$FINALIZE_NAME,ENCODE_FUNCTION=$ENCODE_NAME,SWEEP_FUNCTION=$SWEEP_NAME,BILEVEL_FUNCTION=$BILEVEL_NAME,BILEVEL_STITCH_FUNCTION=$BILEVEL_STITCH_NAME,DZ_EXPORT_FUNCTION=$DZ_EXPORT_NAME,COEFFGEN_FUNCTION=$COEFFGEN_NAME,SWEEP_CM_FUNCTION=$SWEEP_CM_NAME,RENDER_PREVIEW_FUNCTION=$RENDER_PREVIEW_NAME,SOLVE_PROXIMITY_FUNCTION=$SOLVE_PROXIMITY_NAME,RENDER_ORCHESTRATOR_FUNCTION=$RENDER_ORCHESTRATOR_NAME"
    # Reserve concurrency for dispatch so it's never starved by render/merge Lambdas
    aws lambda put-function-concurrency --function-name "$DISPATCH_NAME" \
        --reserved-concurrent-executions 5 --region "$REGION"

    update_lambda "$RASTER_NAME" "handler_raster.handler" "/tmp/polypaint-raster.zip" \
        "$RASTER_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE" "$BINARY_TMP"

    update_lambda "$FINALIZE_NAME" "handler_finalize.handler" "/tmp/polypaint-finalize.zip" \
        "$FINALIZE_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE" "$BINARY_TMP"

    update_lambda "$PREVIEW_NAME" "handler_preview.handler" "/tmp/polypaint-preview.zip" \
        "$PREVIEW_MEMORY" "" "BUCKET=$BUCKET"

    update_lambda "$BILEVEL_NAME" "handler_bilevel.handler" "/tmp/polypaint-bilevel.zip" \
        "$BILEVEL_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    update_lambda "$BILEVEL_STITCH_NAME" "handler_bilevel_stitch.handler" "/tmp/polypaint-bilevel-stitch.zip" \
        "$BILEVEL_STITCH_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    update_lambda "$PARAM_DEBUG_NAME" "handler_param_debug.handler" "/tmp/polypaint-param-debug.zip" \
        "$PARAM_DEBUG_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,LD_LIBRARY_PATH=/opt/lib"

    update_lambda "$TIFF_COMPAT_NAME" "handler_tiff_compat.handler" "/tmp/polypaint-tiff-compat.zip" \
        "$TIFF_COMPAT_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    update_lambda "$PNG_EXPORT_NAME" "handler_png_export.handler" "/tmp/polypaint-png-export.zip" \
        "$PNG_EXPORT_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    update_lambda "$DZ_EXPORT_NAME" "handler_deepzoom_export.handler" "/tmp/polypaint-deepzoom-export.zip" \
        "$DZ_EXPORT_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    update_lambda "$RENDER_PREVIEW_NAME" "handler_render_preview.handler" "/tmp/polypaint-render-preview.zip" \
        "$RENDER_PREVIEW_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    update_lambda "$SOLVE_PROXIMITY_NAME" "handler_solve_proximity.handler" "/tmp/polypaint-solve-proximity.zip" \
        "$SOLVE_PROXIMITY_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE" "$BINARY_TMP"

    update_lambda "$RENDER_PLAN_NAME" "handler_render_plan.handler" "/tmp/polypaint-render-plan.zip" \
        "$RENDER_PLAN_MEMORY" "" "BUCKET=$BUCKET,VIEWPORT_FUNCTION=$VIEWPORT_NAME,STORAGE_FUNCTION=$STORAGE_NAME"

    update_lambda "$RENDER_STATUS_NAME" "handler_render_status.handler" "/tmp/polypaint-render-status.zip" \
        "$RENDER_STATUS_MEMORY" "" "JOBS_TABLE=$JOBS_TABLE"

    # Update Step Functions state machine
    echo "Updating Step Functions state machine..."
    ACCT=$(aws sts get-caller-identity --query 'Account' --output text)
    RENDER_PLAN_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${RENDER_PLAN_NAME}"
    RENDER_STATUS_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${RENDER_STATUS_NAME}"
    RASTER_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${RASTER_NAME}"
    FINALIZE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${FINALIZE_NAME}"
    ENCODE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${ENCODE_NAME}"
    STORAGE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${STORAGE_NAME}"
    BILEVEL_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${BILEVEL_NAME}"
    BILEVEL_STITCH_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${BILEVEL_STITCH_NAME}"
    SOLVE_PROXIMITY_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${SOLVE_PROXIMITY_NAME}"
    RENDER_SM_ARN="arn:aws:states:${REGION}:${ACCT}:stateMachine:${RENDER_STATE_MACHINE_NAME}"

    SFN_ROLE_NAME="polypaint-sfn-execution-role"
    SFN_ROLE_ARN=$(aws iam get-role --role-name "$SFN_ROLE_NAME" --query 'Role.Arn' --output text 2>/dev/null || echo "")

    sed -e "s|\${PlanFunctionArn}|${RENDER_PLAN_ARN}|g" \
        -e "s|\${StatusFunctionArn}|${RENDER_STATUS_ARN}|g" \
        -e "s|\${RasterFunctionArn}|${RASTER_ARN}|g" \
        -e "s|\${FinalizeFunctionArn}|${FINALIZE_ARN}|g" \
        -e "s|\${EncodeFunctionArn}|${ENCODE_ARN}|g" \
        -e "s|\${StorageFunctionArn}|${STORAGE_ARN}|g" \
        -e "s|\${BilevelFunctionArn}|${BILEVEL_ARN}|g" \
        -e "s|\${BilevelStitchFunctionArn}|${BILEVEL_STITCH_ARN}|g" \
        -e "s|\${SolveProximityFunctionArn}|${SOLVE_PROXIMITY_ARN}|g" \
        stepfunctions/render_workflow.asl.json.template > /tmp/render_workflow.asl.json

    SFN_TRUST='{
        "Version": "2012-10-17",
        "Statement": [{
            "Effect": "Allow",
            "Principal": {"Service": "states.amazonaws.com"},
            "Action": "sts:AssumeRole"
        }]
    }'
    if [ -z "$SFN_ROLE_ARN" ]; then
        echo "  Creating SFN execution role..."
        SFN_ROLE_ARN=$(aws iam create-role \
            --role-name "$SFN_ROLE_NAME" \
            --assume-role-policy-document "$SFN_TRUST" \
            --query 'Role.Arn' --output text 2>/dev/null || \
            aws iam get-role --role-name "$SFN_ROLE_NAME" --query 'Role.Arn' --output text)
        aws iam put-role-policy --role-name "$SFN_ROLE_NAME" \
            --policy-name polypaint-sfn-lambda-invoke \
            --policy-document "{
                \"Version\": \"2012-10-17\",
                \"Statement\": [{
                    \"Effect\": \"Allow\",
                    \"Action\": \"lambda:InvokeFunction\",
                    \"Resource\": \"arn:aws:lambda:${REGION}:${ACCT}:function:polypaint-*\"
                }]
            }"
        sleep 5  # IAM propagation
    fi

    # Create or update state machine
    aws stepfunctions update-state-machine \
        --state-machine-arn "$RENDER_SM_ARN" \
        --definition "$(cat /tmp/render_workflow.asl.json)" \
        --role-arn "$SFN_ROLE_ARN" \
        --region "$REGION" >/dev/null 2>&1 || {
        echo "  State machine doesn't exist, creating..."
        aws stepfunctions create-state-machine \
            --name "$RENDER_STATE_MACHINE_NAME" \
            --definition "$(cat /tmp/render_workflow.asl.json)" \
            --role-arn "$SFN_ROLE_ARN" \
            --type STANDARD \
            --region "$REGION" \
            --query 'stateMachineArn' --output text
    }
    echo "  State machine: $RENDER_STATE_MACHINE_NAME"

    # Starter Lambda — uses state machine ARN, not worker names
    update_lambda "$RENDER_ORCHESTRATOR_NAME" "handler_render_orchestrator.handler" "/tmp/polypaint-render-orchestrator.zip" \
        "$RENDER_ORCHESTRATOR_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,RENDER_STATE_MACHINE_ARN=$RENDER_SM_ARN"

    # Ensure states:StartExecution permission
    aws iam put-role-policy --role-name "$ROLE_NAME" \
        --policy-name polypaint-sfn-start \
        --policy-document "{
            \"Version\": \"2012-10-17\",
            \"Statement\": [{
                \"Effect\": \"Allow\",
                \"Action\": \"states:StartExecution\",
                \"Resource\": \"${RENDER_SM_ARN}\"
            }]
        }" 2>/dev/null || true

    update_lambda "$SWEEP_CM_NAME" "handler_sweep_cm.handler" "/tmp/polypaint-sweep-cm.zip" \
        "$SWEEP_CM_MEMORY" "$LAPACK_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    # Async invoke config: no retries for most Lambdas (prevents retry storms),
    # but bilevel gets 2 retries / 1hr age to handle concurrency throttle drops.
    for fn in "$RASTER_NAME" "$FINALIZE_NAME" "$BILEVEL_STITCH_NAME" "$DZ_EXPORT_NAME" "$RENDER_PREVIEW_NAME" "$SOLVE_PROXIMITY_NAME"; do
        aws lambda put-function-event-invoke-config \
            --function-name "$fn" \
            --maximum-retry-attempts 0 \
            --maximum-event-age-in-seconds 300 \
            --region "$REGION" >/dev/null 2>&1
    done
    aws lambda put-function-event-invoke-config \
        --function-name "$BILEVEL_NAME" \
        --maximum-retry-attempts 2 \
        --maximum-event-age-in-seconds 3600 \
        --region "$REGION" >/dev/null 2>&1
    aws lambda put-function-event-invoke-config \
        --function-name "$RENDER_ORCHESTRATOR_NAME" \
        --maximum-retry-attempts 0 \
        --maximum-event-age-in-seconds 3600 \
        --region "$REGION" >/dev/null 2>&1

    # Add Lambda invoke + DynamoDB permissions if missing
    ACCT=$(aws sts get-caller-identity --query 'Account' --output text)
    aws iam put-role-policy --role-name "$ROLE_NAME" \
        --policy-name polypaint-lambda-invoke \
        --policy-document "{
            \"Version\": \"2012-10-17\",
            \"Statement\": [{
                \"Effect\": \"Allow\",
                \"Action\": \"lambda:InvokeFunction\",
                \"Resource\": \"arn:aws:lambda:${REGION}:${ACCT}:function:polypaint-*\"
            }]
        }" 2>/dev/null || true
    aws iam put-role-policy --role-name "$ROLE_NAME" \
        --policy-name polypaint-dynamodb-access \
        --policy-document "{
            \"Version\": \"2012-10-17\",
            \"Statement\": [{
                \"Effect\": \"Allow\",
                \"Action\": [\"dynamodb:PutItem\", \"dynamodb:Query\", \"dynamodb:BatchWriteItem\"],
                \"Resource\": \"arn:aws:dynamodb:${REGION}:${ACCT}:table/${JOBS_TABLE}\"
            }]
        }" 2>/dev/null || true

    # Ensure DynamoDB table exists
    aws dynamodb create-table \
        --table-name "$JOBS_TABLE" \
        --attribute-definitions \
            AttributeName=job_id,AttributeType=S \
            AttributeName=task_id,AttributeType=S \
        --key-schema \
            AttributeName=job_id,KeyType=HASH \
            AttributeName=task_id,KeyType=RANGE \
        --billing-mode PAY_PER_REQUEST \
        --region "$REGION" 2>/dev/null || true
    aws dynamodb update-time-to-live \
        --table-name "$JOBS_TABLE" \
        --time-to-live-specification "Enabled=true,AttributeName=ttl" \
        --region "$REGION" 2>/dev/null || true

    # Ensure API Gateway routes and config.json
    echo ""
    echo "Setting up API Gateway..."
    setup_api_gateway

    # Ensure bucket is configured for website hosting
    ensure_bucket_website

    # Upload index.html and JS catalog to S3
    echo "Uploading index.html to S3..."
    aws s3 cp "$SCRIPT_DIR/index.html" "s3://$BUCKET/index.html" \
        --content-type "text/html" --region "$REGION"
    echo "Uploading coeff_func_catalog_js.js to S3..."
    aws s3 cp "$SCRIPT_DIR/coeff_func_catalog_js.js" "s3://$BUCKET/coeff_func_catalog_js.js" \
        --content-type "application/javascript" --region "$REGION"
    verify_frontend_assets

    echo ""
    echo "=== UPDATED ==="
    echo "  Sweep:    $SWEEP_NAME ($SWEEP_MEMORY MB)"
    echo "  Coeffgen: $COEFFGEN_NAME ($COEFFGEN_MEMORY MB)"
    echo "  Raster:   $RASTER_NAME ($RASTER_MEMORY MB)"
    echo "  Finalize: $FINALIZE_NAME ($FINALIZE_MEMORY MB)"
    echo "  Encode:   $ENCODE_NAME ($ENCODE_MEMORY MB)"
    echo "  Viewport: $VIEWPORT_NAME ($VIEWPORT_MEMORY MB)"
    echo "  Storage:  $STORAGE_NAME ($STORAGE_MEMORY MB)"
    echo "  Dispatch: $DISPATCH_NAME ($DISPATCH_MEMORY MB)"
    echo "  Preview:  $PREVIEW_NAME ($PREVIEW_MEMORY MB)"
    echo "  Site:     http://$BUCKET.s3-website-$REGION.amazonaws.com"
else
    echo "Usage: $0 [create|update]"
    exit 1
fi
