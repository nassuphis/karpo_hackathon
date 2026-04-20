#!/bin/bash
# Deploy the polypaint Lambda functions
# Usage: ./deploy.sh [create|update|show-build]
#
# Core Lambdas:
#   polypaint-sweep        — single-thread AE root solver (sweep binary)
#   polypaint-sweep-mt     — multithreaded AE root solver (sweep_mt binary)
#   polypaint-coeffgen     — composable coefficient generation (sweep_coeffgen binary, coeffgen mode)
#   polypaint-raster-mt    — multithreaded color raster via parallel roots2pix workers
#   polypaint-encode       — raw→JPEG/PNG encoding (raw2jpeg binary, needs libvips)
#   polypaint-viewport     — compute viewport from lores.bin (pure Python)
#   polypaint-storage      — S3 list/delete/metadata (pure Python)
#   polypaint-dispatch     — async Lambda invocation dispatcher (pure Python)

set -euo pipefail
export AWS_PAGER=""

SWEEP_NAME="polypaint-sweep"
SWEEP_MT_NAME="polypaint-sweep-mt"
ENCODE_NAME="polypaint-encode"
VIEWPORT_NAME="polypaint-viewport"
STORAGE_NAME="polypaint-storage"
DISPATCH_NAME="polypaint-dispatch"
RASTER_MT_NAME="polypaint-raster-mt"
FINALIZE_MT_NAME="polypaint-finalize-mt"
COEFFGEN_NAME="polypaint-coeffgen"
PREVIEW_NAME="polypaint-preview"
COMPUTE_PREVIEW_NAME="polypaint-compute-preview"
BILEVEL_NAME="polypaint-bilevel"
COLOR_TO_BILEVEL_NAME="polypaint-color-to-bilevel"
ROLE_NAME="polypaint-lambda-role"
REGION="us-east-1"
API_NAME="polypaint-api"
RUNTIME="python3.12"
ARCH="arm64"
SWEEP_MEMORY=10240    # 6 vCPUs for single-thread AE solve with large chunks
SWEEP_MT_MEMORY=10240 # 6 vCPUs for multithreaded AE solve
ENCODE_MEMORY=10240   # max memory/CPU tier for very large JPEG/PNG encodes
ENCODE_EPHEMERAL=3072 # 3GB /tmp for preview generation from large images
VIEWPORT_MEMORY=512   # pure Python
STORAGE_MEMORY=512    # pure Python
DISPATCH_MEMORY=1769  # 1 vCPU — 50 threads doing SSL need real CPU
RASTER_MT_MEMORY=10240 # max memory/CPU tier for native pthread raster on large fused renders
RASTER_MT_THREADS=4   # default per-Lambda worker count for color raster MT
FINALIZE_MT_MEMORY=10240  # fused solve-score raw assemble + encode
COEFFGEN_MEMORY=10240 # 6 vCPUs for coeffgen + threaded param_gen
PREVIEW_MEMORY=1024   # pure Python, PNG encoding via zlib (512 OOMs on large lores)
COMPUTE_PREVIEW_MEMORY=4096  # sync coeffgen+solve+PNG preview, needs LAPACK for roots_cm/CM
BILEVEL_MEMORY=1769   # 1 vCPU, bilevel raster + merge
COLOR_TO_BILEVEL_MEMORY=10240  # raw-sidecar thresholding is single-shot and CPU-bound on large images
TIFF_COMPAT_NAME="polypaint-tiff-compat"
TIFF_COMPAT_MEMORY=4096  # needs RAM for scanline buffer on large images
PNG_EXPORT_NAME="polypaint-png-export"
PNG_EXPORT_MEMORY=4096  # libvips PNG encode
DZ_EXPORT_NAME="polypaint-deepzoom-export"
DZ_EXPORT_MEMORY=4096  # libvips dzsave + parallel S3 upload
DZ_FROM_RAW_NAME="polypaint-deepzoom-from-raw"
DZ_FROM_RAW_MEMORY=4096  # raw-sidecar-only DeepZoom export
PARAM_DEBUG_NAME="polypaint-param-debug"
PARAM_DEBUG_MEMORY=1769  # 1 vCPU + libvips for TIFF output
BILEVEL_STITCH_NAME="polypaint-bilevel-stitch"
BILEVEL_STITCH_MEMORY=6144  # ~4 vCPUs, libvips multithreaded stitch
RENDER_PREVIEW_NAME="polypaint-render-preview"
RENDER_PREVIEW_MEMORY=4096  # libvips vipsthumbnail on large images
AUTOLEVELS_NAME="polypaint-autolevels"
AUTOLEVELS_MEMORY=10240  # libvips autolevel post-process on saved color renders
RESIZE_ARTIFACT_NAME="polypaint-resize-artifact"
RESIZE_ARTIFACT_MEMORY=6144  # libvips resize/thumbnail post-process on saved color renders
REPALETTE_NAME="polypaint-repalette"
REPALETTE_MEMORY=4096  # libvips palette recolor on saved palette artifacts
COLOR_REPALETTE_NAME="polypaint-color-repalette"
COLOR_REPALETTE_MEMORY=1769  # fast tile recolor from persisted pixel bins
RECOLOR_FROM_RAW_NAME="polypaint-recolor-from-raw"
RECOLOR_FROM_RAW_MEMORY=1769  # raw-sidecar-only recolor
EXTRACT_PALETTE_FUSED_NAME="polypaint-extract-palette-fused"
EXTRACT_PALETTE_FUSED_MEMORY=1769  # fused step_scores.raw -> palette raw -> score_raw_render
PDF_ARTIFACT_NAME="polypaint-pdf-artifact"
PDF_ARTIFACT_MEMORY=2048  # single-shot PDF composition from saved Color image
SOLVE_PROXIMITY_NAME="polypaint-solve-proximity"
SOLVE_PROXIMITY_MEMORY=4096  # higher CPU tier for pthread solve-score phases
SOLVE_PROXIMITY_BENCH_NAME="polypaint-solve-proximity-bench"
SOLVE_PROXIMITY_BENCH_MEMORY=10240  # benchmark Lambda for high-CPU/high-network hist concurrency sweeps
RENDER_ORCHESTRATOR_NAME="polypaint-render-orchestrator"
RENDER_ORCHESTRATOR_MEMORY=512  # starter only — validates + starts Step Functions
RENDER_PLAN_NAME="polypaint-render-plan"
RENDER_PLAN_MEMORY=512
RENDER_STATUS_NAME="polypaint-render-status"
RENDER_STATUS_MEMORY=256
RENDER_STATE_MACHINE_NAME="polypaint-render-workflow"
COMPUTE_ORCHESTRATOR_NAME="polypaint-compute-orchestrator"
COMPUTE_ORCHESTRATOR_MEMORY=512
COMPUTE_PLAN_NAME="polypaint-compute-plan"
COMPUTE_PLAN_MEMORY=512
COMPUTE_FUSED_CHUNK_NAME="polypaint-compute-fused-chunk"
COMPUTE_FUSED_CHUNK_MEMORY=10240
COMPUTE_STATUS_NAME="polypaint-compute-status"
COMPUTE_STATUS_MEMORY=256
COMPUTE_STATE_MACHINE_NAME="polypaint-compute-workflow"
PALETTE_ORCHESTRATOR_NAME="polypaint-palette-orchestrator"
PALETTE_ORCHESTRATOR_MEMORY=512
PALETTE_PLAN_NAME="polypaint-palette-render-plan"
PALETTE_PLAN_MEMORY=512
PALETTE_CHUNK_NAME="polypaint-palette-chunk"
PALETTE_CHUNK_MEMORY=1769
PALETTE_FINALIZE_NAME="polypaint-palette-finalize"
PALETTE_FINALIZE_MEMORY=4096
ATTACH_PALETTE_NAME="polypaint-attach-palette-to-color"
ATTACH_PALETTE_MEMORY=512
PALETTE_STATE_MACHINE_NAME="polypaint-palette-workflow"
PALETTE_DEBUG_NAME="polypaint-palette-debug"
PALETTE_DEBUG_MEMORY=1769
BINARY_TMP=10240      # /tmp size for Lambdas that process raw images (max 10GB)
TIMEOUT=900
BUCKET="polypaint"
JOBS_TABLE="polypaint-jobs"
LAYER_PUBLISH_PREFIX="deploy/layers"
LIBVIPS_LAYER_NAME="polypaint-libvips"
LAPACK_LAYER_NAME="polypaint-lapack"
PDF_PY_LAYER_NAME="polypaint-pdf-python"
# These get set dynamically by build_and_publish_layers
LIBVIPS_LAYER=""
LAPACK_LAYER=""
PDF_PY_LAYER=""
SWEEP_CM_NAME="polypaint-sweep-cm"
SWEEP_CM_MEMORY=4096  # companion matrix eigensolve needs more memory
BUILD_ID=""
BUILD_DEPLOYED_AT_UTC=""
BUILD_GIT_REV="nogit"
BUILD_GIT_DIRTY=false
BUILD_FRONTEND_SHA256=""

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"
if command -v uv >/dev/null 2>&1; then
    TEST_PYTHON=(uv run python)
elif [ -x "$SCRIPT_DIR/.venv/bin/python" ]; then
    TEST_PYTHON=("$SCRIPT_DIR/.venv/bin/python")
elif [ -x "$SCRIPT_DIR/../.venv/bin/python" ]; then
    TEST_PYTHON=("$SCRIPT_DIR/../.venv/bin/python")
else
    TEST_PYTHON=(python3)
fi
ACTION="${1:-create}"

frontend_asset_keys() {
    printf '%s\n' \
        "index.html" \
        "coeff_func_catalog_js.js" \
        "tri_palette_catalog_js.js" \
        "long_palette_catalog_js.js"
    if [ -d "$SCRIPT_DIR/solve-score-programs" ]; then
        (
            cd "$SCRIPT_DIR"
            find "solve-score-programs" -type f | sort
        )
    fi
}

frontend_asset_content_type() {
    case "$1" in
        *.html) echo "text/html" ;;
        *.js) echo "application/javascript" ;;
        *.json) echo "application/json" ;;
        *) echo "application/octet-stream" ;;
    esac
}

print_usage() {
    echo "Usage: $0 [create|update|show-build]"
}

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

build_deploy_metadata() {
    BUILD_DEPLOYED_AT_UTC=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
    local BUILD_STAMP
    BUILD_STAMP=$(date -u +"%Y%m%d-%H%M%S")

    BUILD_GIT_REV="nogit"
    BUILD_GIT_DIRTY=false
    if git -C "$SCRIPT_DIR" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
        BUILD_GIT_REV=$(git -C "$SCRIPT_DIR" rev-parse --short=7 HEAD 2>/dev/null || echo "nogit")
        if ! git -C "$SCRIPT_DIR" diff --quiet --ignore-submodules HEAD -- >/dev/null 2>&1; then
            BUILD_GIT_DIRTY=true
        fi
    fi

    BUILD_FRONTEND_SHA256=$(
        while IFS= read -r asset; do
            cat "$SCRIPT_DIR/$asset"
        done < <(frontend_asset_keys) | shasum | cut -d' ' -f1
    )

    local BUILD_HASH_SHORT
    BUILD_HASH_SHORT=${BUILD_FRONTEND_SHA256:0:12}
    BUILD_ID="${BUILD_STAMP}-${BUILD_GIT_REV}-${BUILD_HASH_SHORT}"
    if [ "$BUILD_GIT_DIRTY" = true ]; then
        BUILD_ID="${BUILD_ID}-dirty"
    fi
}

# --- Deployed frontend smoke test ---
verify_frontend_assets() {
    local SITE_URL="http://${BUCKET}.s3-website-${REGION}.amazonaws.com"
    local TMP_DIR
    TMP_DIR=$(mktemp -d /tmp/polypaint-frontend-verify.XXXXXX)
    trap 'rm -rf "$TMP_DIR"' RETURN
    echo "Verifying deployed frontend assets..."
    while IFS= read -r asset; do
        local STATUS
        STATUS=$(curl -s -o /dev/null -w "%{http_code}" "${SITE_URL}/${asset}")
        if [ "$STATUS" != "200" ]; then
            echo "FATAL: ${SITE_URL}/${asset} returned HTTP ${STATUS} (expected 200)"
            exit 1
        fi
        local LOCAL_HASH REMOTE_HASH
        mkdir -p "$(dirname "${TMP_DIR}/${asset}")"
        curl -fsS "${SITE_URL}/${asset}" -o "${TMP_DIR}/${asset}"
        LOCAL_HASH=$(shasum "$SCRIPT_DIR/${asset}" | cut -d' ' -f1)
        REMOTE_HASH=$(shasum "${TMP_DIR}/${asset}" | cut -d' ' -f1)
        if [ "$LOCAL_HASH" != "$REMOTE_HASH" ]; then
            echo "FATAL: deployed ${asset} does not match local file"
            echo "  local:  ${LOCAL_HASH}"
            echo "  remote: ${REMOTE_HASH}"
            exit 1
        fi
        echo "  ${asset}: HTTP ${STATUS} OK"
    done < <(frontend_asset_keys)
}

zip_content_hash() {
    local ZIP_PATH="$1"
    "${TEST_PYTHON[@]}" - "$ZIP_PATH" <<'PY'
import hashlib
import sys
import zipfile

zip_path = sys.argv[1]
with zipfile.ZipFile(zip_path) as zf:
    names = sorted(name for name in zf.namelist() if not name.endswith("/"))
    h = hashlib.sha256()
    for name in names:
        data = zf.read(name)
        h.update(name.encode("utf-8"))
        h.update(b"\0")
        h.update(hashlib.sha256(data).digest())
        h.update(b"\n")
print(h.hexdigest())
PY
}

json_content_hash() {
    local JSON_PATH="$1"
    "${TEST_PYTHON[@]}" - "$JSON_PATH" <<'PY'
import hashlib
import json
import sys

path = sys.argv[1]
with open(path, "r", encoding="utf-8") as fh:
    data = json.load(fh)
payload = json.dumps(data, sort_keys=True, separators=(",", ":")).encode("utf-8")
print(hashlib.sha256(payload).hexdigest())
PY
}

package_render_plan_zip() {
    local ZIP_PATH="$1"
    local PLAN_DIR="$2"
    rm -rf "$PLAN_DIR"
    mkdir -p "$PLAN_DIR"
    cp lambda/handler_render_plan.py lambda/shared.py \
       lambda/logical_sections.py \
       lambda/calc_chunks.py \
       lambda/param_source.py \
       lambda/solve_score_chain.py \
       lambda/palette_names.py lambda/tri_palette_names_generated.py lambda/long_palette_names_generated.py "$PLAN_DIR/"
    cd "$PLAN_DIR" && zip -r9 "$ZIP_PATH" . -q && cd "$SCRIPT_DIR"
}

render_render_workflow_definition() {
    local OUT_PATH="$1"
    local ACCT="$2"
    local RENDER_PLAN_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${RENDER_PLAN_NAME}"
    local RENDER_STATUS_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${RENDER_STATUS_NAME}"
    local FINALIZE_MT_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${FINALIZE_MT_NAME}"
    local ENCODE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${ENCODE_NAME}"
    local STORAGE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${STORAGE_NAME}"
    local BILEVEL_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${BILEVEL_NAME}"
    local BILEVEL_STITCH_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${BILEVEL_STITCH_NAME}"
    local SOLVE_PROXIMITY_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${SOLVE_PROXIMITY_NAME}"
    local PREVIEW_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${RENDER_PREVIEW_NAME}"

    sed -e "s|\${PlanFunctionArn}|${RENDER_PLAN_ARN}|g" \
        -e "s|\${StatusFunctionArn}|${RENDER_STATUS_ARN}|g" \
        -e "s|\${FinalizeMTFunctionArn}|${FINALIZE_MT_ARN}|g" \
        -e "s|\${EncodeFunctionArn}|${ENCODE_ARN}|g" \
        -e "s|\${StorageFunctionArn}|${STORAGE_ARN}|g" \
        -e "s|\${BilevelFunctionArn}|${BILEVEL_ARN}|g" \
        -e "s|\${BilevelStitchFunctionArn}|${BILEVEL_STITCH_ARN}|g" \
        -e "s|\${SolveProximityFunctionArn}|${SOLVE_PROXIMITY_ARN}|g" \
        -e "s|\${PreviewFunctionArn}|${PREVIEW_ARN}|g" \
        stepfunctions/render_workflow.asl.json.template > "$OUT_PATH"
}

show_build() {
    build_deploy_metadata
    local TMP_DIR
    TMP_DIR=$(mktemp -d /tmp/polypaint-show-build.XXXXXX)
    trap 'rm -rf "$TMP_DIR"' RETURN

    local CONFIG_URL="https://${BUCKET}.s3.${REGION}.amazonaws.com/config.json"
    local CONFIG_PATH="$TMP_DIR/config.json"
    curl -fsS "$CONFIG_URL" -o "$CONFIG_PATH"

    local LOCAL_RENDER_PLAN_ZIP="$TMP_DIR/render-plan-local.zip"
    local LOCAL_RENDER_PLAN_DIR="$TMP_DIR/render-plan-local"
    package_render_plan_zip "$LOCAL_RENDER_PLAN_ZIP" "$LOCAL_RENDER_PLAN_DIR"
    local LOCAL_RENDER_PLAN_HASH
    LOCAL_RENDER_PLAN_HASH=$(zip_content_hash "$LOCAL_RENDER_PLAN_ZIP")

    local DEPLOYED_RENDER_PLAN_URL
    DEPLOYED_RENDER_PLAN_URL=$(aws lambda get-function \
        --function-name "$RENDER_PLAN_NAME" \
        --region "$REGION" \
        --query 'Code.Location' --output text)
    local DEPLOYED_RENDER_PLAN_ZIP="$TMP_DIR/render-plan-deployed.zip"
    curl -fsS "$DEPLOYED_RENDER_PLAN_URL" -o "$DEPLOYED_RENDER_PLAN_ZIP"
    local DEPLOYED_RENDER_PLAN_HASH
    DEPLOYED_RENDER_PLAN_HASH=$(zip_content_hash "$DEPLOYED_RENDER_PLAN_ZIP")

    local ACCT
    ACCT=$(aws sts get-caller-identity --region "$REGION" --query 'Account' --output text)
    local LOCAL_RENDER_WORKFLOW="$TMP_DIR/render_workflow_local.json"
    render_render_workflow_definition "$LOCAL_RENDER_WORKFLOW" "$ACCT"
    local DEPLOYED_RENDER_WORKFLOW="$TMP_DIR/render_workflow_deployed.json"
    aws stepfunctions describe-state-machine \
        --state-machine-arn "arn:aws:states:${REGION}:${ACCT}:stateMachine:${RENDER_STATE_MACHINE_NAME}" \
        --region "$REGION" \
        --query 'definition' --output text > "$DEPLOYED_RENDER_WORKFLOW"
    local LOCAL_RENDER_WORKFLOW_HASH
    local DEPLOYED_RENDER_WORKFLOW_HASH
    LOCAL_RENDER_WORKFLOW_HASH=$(json_content_hash "$LOCAL_RENDER_WORKFLOW")
    DEPLOYED_RENDER_WORKFLOW_HASH=$(json_content_hash "$DEPLOYED_RENDER_WORKFLOW")

    "${TEST_PYTHON[@]}" - \
        "$CONFIG_PATH" \
        "$BUILD_ID" \
        "$BUILD_GIT_REV" \
        "$BUILD_GIT_DIRTY" \
        "$BUILD_FRONTEND_SHA256" \
        "$LOCAL_RENDER_PLAN_HASH" \
        "$DEPLOYED_RENDER_PLAN_HASH" \
        "$LOCAL_RENDER_WORKFLOW_HASH" \
        "$DEPLOYED_RENDER_WORKFLOW_HASH" <<'PY'
import json
import sys

(
    config_path,
    local_build_id,
    local_git_rev,
    local_git_dirty_raw,
    local_frontend_sha,
    local_render_plan_hash,
    deployed_render_plan_hash,
    local_render_workflow_hash,
    deployed_render_workflow_hash,
) = sys.argv[1:]

with open(config_path, "r", encoding="utf-8") as fh:
    config = json.load(fh)
build = dict(config.get("build") or {})
deployed_build_id = str(build.get("build_id") or "")
deployed_git_rev = str(build.get("git_rev") or "")
deployed_frontend_sha = str(build.get("frontend_sha256") or "")
deployed_git_dirty = bool(build.get("git_dirty"))
local_git_dirty = local_git_dirty_raw.lower() == "true"

frontend_match = (
    deployed_git_rev == local_git_rev
    and deployed_git_dirty == local_git_dirty
    and deployed_frontend_sha == local_frontend_sha
)
render_plan_match = local_render_plan_hash == deployed_render_plan_hash
render_workflow_match = local_render_workflow_hash == deployed_render_workflow_hash
overall_match = frontend_match and render_plan_match and render_workflow_match

print("Local source")
print(f"  build_id(now):        {local_build_id}")
print(f"  git_rev:              {local_git_rev}")
print(f"  git_dirty:            {str(local_git_dirty).lower()}")
print(f"  frontend_sha256:      {local_frontend_sha}")
print("Deployed frontend")
print(f"  build_id:             {deployed_build_id}")
print(f"  git_rev:              {deployed_git_rev}")
print(f"  git_dirty:            {str(deployed_git_dirty).lower()}")
print(f"  frontend_sha256:      {deployed_frontend_sha}")
print(f"  source_match:         {'yes' if frontend_match else 'no'}")
print("Render plan bundle")
print(f"  local_content_hash:   {local_render_plan_hash}")
print(f"  deployed_content_hash:{deployed_render_plan_hash}")
print(f"  match:                {'yes' if render_plan_match else 'no'}")
print("Render workflow")
print(f"  local_json_hash:      {local_render_workflow_hash}")
print(f"  deployed_json_hash:   {deployed_render_workflow_hash}")
print(f"  match:                {'yes' if render_workflow_match else 'no'}")
print(f"Overall: {'MATCH' if overall_match else 'MISMATCH'}")
sys.exit(0 if overall_match else 2)
PY
}

if [ "$ACTION" = "show-build" ]; then
    show_build
    exit $?
fi

if [ "$ACTION" != "create" ] && [ "$ACTION" != "update" ]; then
    print_usage
    exit 1
fi

# --- Generate palette assets ---
echo "Generating palette assets..."
python3 "$SCRIPT_DIR/scripts/generate_tri_palettes.py" || { echo "FATAL: tri-palette generation failed"; exit 1; }
python3 "$SCRIPT_DIR/scripts/generate_long_palettes.py" || { echo "FATAL: long-palette generation failed"; exit 1; }

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

echo ""
echo "Running predeploy contract gate..."
bash "$SCRIPT_DIR/scripts/predeploy_check.sh" || {
    echo "FATAL: predeploy contract gate failed"
    exit 1
}

# --- Build and publish Lambda layers ---
echo ""
echo "Building and publishing Lambda layers..."

aws_retry() {
    local attempt rc delay
    delay=2
    for attempt in 1 2 3; do
        if "$@"; then
            return 0
        fi
        rc=$?
        if [ "$attempt" -lt 3 ]; then
            echo "  aws call failed (attempt ${attempt}/3), retrying in ${delay}s..."
            sleep "$delay"
            delay=$((delay * 2))
        fi
    done
    return "$rc"
}

build_and_publish_layer() {
    local LAYER_NAME="$1" BUILD_SCRIPT="$2" ZIP_NAME="$3" BUILD_DIR="$4" ARN_FILE="$5"
    local ZIP_PATH="$BUILD_DIR/$ZIP_NAME"
    local HASH_FILE="$BUILD_DIR/.build_hash"
    local PUBLISHED_HASH_FILE="$BUILD_DIR/.published_zip_hash"
    local PUBLISHED_ARN_FILE="$BUILD_DIR/.published_layer_arn"
    local SCRIPT_HASH
    local ZIP_HASH
    local S3_KEY
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

    ZIP_HASH=$(shasum "$ZIP_PATH" | cut -d' ' -f1)
    if [ -f "$PUBLISHED_HASH_FILE" ] && [ -f "$PUBLISHED_ARN_FILE" ] && \
       [ "$(cat "$PUBLISHED_HASH_FILE")" = "$ZIP_HASH" ] && [ -s "$PUBLISHED_ARN_FILE" ]; then
        echo "  $LAYER_NAME: zip unchanged (reusing published layer version)"
        cp "$PUBLISHED_ARN_FILE" "$ARN_FILE"
    else
        S3_KEY="${LAYER_PUBLISH_PREFIX}/${LAYER_NAME}/${ZIP_HASH}/${ZIP_NAME}"
        echo "  $LAYER_NAME: uploading layer zip to s3://$BUCKET/$S3_KEY"
        if ! aws_retry aws s3 cp "$ZIP_PATH" "s3://$BUCKET/$S3_KEY" --region "$REGION" >/dev/null; then
            echo "FATAL: failed to upload $LAYER_NAME layer zip to S3"
            exit 1
        fi

        echo "  $LAYER_NAME: publishing layer version from S3..."
        if ! aws_retry aws lambda publish-layer-version \
            --layer-name "$LAYER_NAME" \
            --content "S3Bucket=$BUCKET,S3Key=$S3_KEY" \
            --compatible-runtimes python3.12 python3.13 \
            --compatible-architectures arm64 \
            --region "$REGION" \
            --query 'LayerVersionArn' --output text > "$ARN_FILE"; then
            echo "FATAL: failed to publish $LAYER_NAME layer"
            exit 1
        fi
        echo "$ZIP_HASH" > "$PUBLISHED_HASH_FILE"
        cp "$ARN_FILE" "$PUBLISHED_ARN_FILE"
    fi

    if [ ! -s "$ARN_FILE" ]; then
        echo "FATAL: missing ARN for $LAYER_NAME layer"
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

build_and_publish_layer \
    "$PDF_PY_LAYER_NAME" \
    "$SCRIPT_DIR/lambda/build-pdf-python-layer.sh" \
    "pdf-python-layer.zip" \
    "$SCRIPT_DIR/lambda/layer-build-pdf" \
    /tmp/_pdf_py_layer_arn
PDF_PY_LAYER=$(cat /tmp/_pdf_py_layer_arn)

echo "  LIBVIPS_LAYER=$LIBVIPS_LAYER"
echo "  LAPACK_LAYER=$LAPACK_LAYER"
echo "  PDF_PY_LAYER=$PDF_PY_LAYER"

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
cc -O2 -pthread -o lambda/sweep_test lambda/sweep_cli.c -lm

# Step 3: Regenerate parity overlay from pytest-backed hand parity suites
echo "Generating parity overlay from pytest..."
"${TEST_PYTHON[@]}" lambda/gen_parity_results.py || { echo "FATAL: parity overlay generation failed"; exit 1; }

# Step 4: Probe degrees and generate JS catalog
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
aarch64-linux-musl-gcc -O3 -static -pthread -o lambda/sweep lambda/sweep_cli.c -lm

echo "  sweep_mt (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -pthread -o lambda/sweep_mt lambda/sweep_mt.c -lm

echo "  roots2pix_mt (Docker ARM64, dynamic libcurl)..."
docker run --rm --platform linux/arm64 \
    -v "$SCRIPT_DIR/lambda:/src" \
    public.ecr.aws/amazonlinux/amazonlinux:2023 \
    bash -c '
        set -euo pipefail
        dnf install -y gcc libcurl-devel 2>&1 | tail -1
        gcc -O3 -pthread -o /src/roots2pix_mt /src/roots2pix_mt.c /src/multispan_reader.c \
            -lcurl -lm -Wl,-rpath,\$ORIGIN/lib
        rm -rf /src/roots2pix_mt_lib
        mkdir -p /src/roots2pix_mt_lib
        for lib in $(ldd /src/roots2pix_mt | awk "/=> \// {print \$3}"); do
            base=$(basename "$lib")
            case "$base" in
                libc.so.*|libm.so.*|libpthread.so.*|ld-linux-aarch64.so.*|libdl.so.*|librt.so.*)
                    continue
                    ;;
            esac
            cp -L "$lib" /src/roots2pix_mt_lib/
        done
        echo "  roots2pix_mt compiled: $(file /src/roots2pix_mt)"
    '

echo "  assemble_greyscale (Docker ARM64, dynamic libcurl)..."
docker run --rm --platform linux/arm64 \
    -v "$SCRIPT_DIR/lambda:/src" \
    public.ecr.aws/amazonlinux/amazonlinux:2023 \
    bash -c '
        set -euo pipefail
        dnf install -y gcc libcurl-devel 2>&1 | tail -1
        gcc -O3 -pthread -o /src/assemble_greyscale /src/assemble_greyscale.c \
            -lcurl -lm -Wl,-rpath,\$ORIGIN/lib
        rm -rf /src/assemble_greyscale_lib
        mkdir -p /src/assemble_greyscale_lib
        for lib in $(ldd /src/assemble_greyscale | awk "/=> \// {print \$3}"); do
            base=$(basename "$lib")
            case "$base" in
                libc.so.*|libm.so.*|libpthread.so.*|ld-linux-aarch64.so.*|libdl.so.*|librt.so.*)
                    continue
                    ;;
            esac
            cp -L "$lib" /src/assemble_greyscale_lib/
        done
        echo "  assemble_greyscale compiled: $(file /src/assemble_greyscale)"
    '

echo "  bilevel_raster (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/bilevel_raster lambda/bilevel_raster.c -lm

echo "  coeffs_bilevel_raster (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/coeffs_bilevel_raster lambda/coeffs_bilevel_raster.c -lm

echo "  bilevel_section_raster (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/bilevel_section_raster lambda/bilevel_section_raster.c -lm

echo "  solve_proximity_stats (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -pthread -o lambda/solve_proximity_stats lambda/solve_proximity_stats.c -lm
echo "  solve_proximity_hist_sectioned (Docker ARM64, dynamic libcurl)..."
docker run --rm --platform linux/arm64 \
    -v "$SCRIPT_DIR/lambda:/src" \
    public.ecr.aws/amazonlinux/amazonlinux:2023 \
    bash -c '
        set -euo pipefail
        dnf install -y gcc libcurl-devel 2>&1 | tail -1
        gcc -O3 -pthread -o /src/solve_proximity_hist_sectioned /src/solve_proximity_hist_sectioned.c /src/multispan_reader.c \
            -lcurl -lm -Wl,-rpath,\$ORIGIN/lib
        rm -rf /src/solve_proximity_hist_sectioned_lib
        mkdir -p /src/solve_proximity_hist_sectioned_lib
        for lib in $(ldd /src/solve_proximity_hist_sectioned | awk "/=> \// {print \$3}"); do
            base=$(basename "$lib")
            case "$base" in
                libc.so.*|libm.so.*|libpthread.so.*|ld-linux-aarch64.so.*|libdl.so.*|librt.so.*)
                    continue
                    ;;
            esac
            cp -L "$lib" /src/solve_proximity_hist_sectioned_lib/
        done
        echo "  solve_proximity_hist_sectioned compiled: $(file /src/solve_proximity_hist_sectioned)"
    '
echo "  solve_palette_debug (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/solve_palette_debug lambda/solve_palette_debug.c -lm
echo "  solve_palette_chunk (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/solve_palette_chunk lambda/solve_palette_chunk.c -lm
echo "  solve_palette_chunk_mt (Docker ARM64, dynamic libcurl)..."
docker run --rm --platform linux/arm64 \
    -v "$SCRIPT_DIR/lambda:/src" \
    public.ecr.aws/amazonlinux/amazonlinux:2023 \
    bash -c '
        set -euo pipefail
        dnf install -y gcc libcurl-devel 2>&1 | tail -1
        gcc -O3 -pthread -o /src/solve_palette_chunk_mt /src/solve_palette_chunk_mt.c /src/multispan_reader.c \
            -lcurl -lm -Wl,-rpath,\$ORIGIN/lib
        rm -rf /src/solve_palette_chunk_mt_lib
        mkdir -p /src/solve_palette_chunk_mt_lib
        for lib in $(ldd /src/solve_palette_chunk_mt | awk "/=> \// {print \$3}"); do
            base=$(basename "$lib")
            case "$base" in
                libc.so.*|libm.so.*|libpthread.so.*|ld-linux-aarch64.so.*|libdl.so.*|librt.so.*)
                    continue
                    ;;
            esac
            cp -L "$lib" /src/solve_palette_chunk_mt_lib/
        done
        echo "  solve_palette_chunk_mt compiled: $(file /src/solve_palette_chunk_mt)"
    '
echo "  palette_bins_render (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/palette_bins_render lambda/palette_bins_render.c -lm

echo "  pixel_bins_render (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/pixel_bins_render lambda/pixel_bins_render.c -lm

echo "  step_scores_to_palette_raw (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/step_scores_to_palette_raw lambda/step_scores_to_palette_raw.c

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
        gcc -O3 -o /src/score_raw_render /src/score_raw_render.c \
            -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
            -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include \
            -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm \
            -Wl,-rpath,/opt/lib
        echo "  score_raw_render compiled: $(file /src/score_raw_render)"
        gcc -O3 -o /src/bilevel_merge /src/bilevel_merge.c \
            -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
            -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include \
            -L/opt/lib -lvips -ltiff -lgobject-2.0 -lglib-2.0 -lm \
            -Wl,-rpath,/opt/lib
        echo "  bilevel_merge compiled: $(file /src/bilevel_merge)"
        gcc -O3 -o /src/raw_to_bilevel /src/raw_to_bilevel.c \
            -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
            -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include \
            -L/opt/lib -lvips -ltiff -lgobject-2.0 -lglib-2.0 -lm \
            -Wl,-rpath,/opt/lib
        echo "  raw_to_bilevel compiled: $(file /src/raw_to_bilevel)"
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
        gcc -O3 -o /src/autolevels_render /src/autolevels_render.c \
            -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
            -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include \
            -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm \
            -Wl,-rpath,/opt/lib
        echo "  autolevels_render compiled: $(file /src/autolevels_render)"

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

# Compile LAPACK-backed coeffgen/sweep_cm helpers
echo "  sweep_coeffgen + sweep_cm (Docker ARM64, dynamically linked against LAPACK)..."
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
        echo "  sweep_coeffgen compiled: $(file /src/sweep_coeffgen)"
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

# Sweep-MT: handler_sweep_mt.py + shared.py + sweep_mt
SWEEP_MT_DIR=/tmp/polypaint-sweep-mt
rm -rf "$SWEEP_MT_DIR"
mkdir -p "$SWEEP_MT_DIR"
cp lambda/handler_sweep_mt.py lambda/shared.py "$SWEEP_MT_DIR/"
cp lambda/sweep_mt "$SWEEP_MT_DIR/"
chmod +x "$SWEEP_MT_DIR"/sweep_mt
cd "$SWEEP_MT_DIR" && zip -r9 /tmp/polypaint-sweep-mt.zip . -q && cd "$SCRIPT_DIR"
echo "  SweepMT:  $(du -h /tmp/polypaint-sweep-mt.zip | cut -f1)  (sweep_mt)"

# Coeffgen: handler_coeffgen.py + shared.py + compute_fused.py + sweep_coeffgen (needs LAPACK layer)
COEFFGEN_DIR=/tmp/polypaint-coeffgen
rm -rf "$COEFFGEN_DIR"
mkdir -p "$COEFFGEN_DIR"
cp lambda/handler_coeffgen.py lambda/shared.py lambda/compute_fused.py "$COEFFGEN_DIR/"
cp lambda/sweep_coeffgen "$COEFFGEN_DIR/"
chmod +x "$COEFFGEN_DIR"/sweep_coeffgen
cd "$COEFFGEN_DIR" && zip -r9 /tmp/polypaint-coeffgen.zip . -q && cd "$SCRIPT_DIR"
echo "  Coeffgen: $(du -h /tmp/polypaint-coeffgen.zip | cut -f1)  (sweep_coeffgen + LAPACK layer)"

# Encode: handler_encode.py + shared.py + color_artifact_meta.py + raw2jpeg
ENCODE_DIR=/tmp/polypaint-encode
rm -rf "$ENCODE_DIR"
mkdir -p "$ENCODE_DIR"
cp lambda/handler_encode.py lambda/shared.py lambda/color_artifact_meta.py lambda/solve_score_chain.py "$ENCODE_DIR/"
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

# Storage: handler_storage.py + shared.py + color artifact metadata + logical section helpers (pure Python)
STORAGE_DIR=/tmp/polypaint-storage
rm -rf "$STORAGE_DIR"
mkdir -p "$STORAGE_DIR"
cp lambda/handler_storage.py lambda/shared.py lambda/color_artifact_meta.py lambda/solve_score_chain.py \
   lambda/logical_sections.py "$STORAGE_DIR/"
cd "$STORAGE_DIR" && zip -r9 /tmp/polypaint-storage.zip . -q && cd "$SCRIPT_DIR"
echo "  Storage:  $(du -h /tmp/polypaint-storage.zip | cut -f1)  (pure Python)"

# Dispatch: handler_dispatch.py + shared.py (pure Python, invokes render Lambdas)
DISPATCH_DIR=/tmp/polypaint-dispatch
rm -rf "$DISPATCH_DIR"
mkdir -p "$DISPATCH_DIR"
cp lambda/handler_dispatch.py lambda/shared.py "$DISPATCH_DIR/"
cd "$DISPATCH_DIR" && zip -r9 /tmp/polypaint-dispatch.zip . -q && cd "$SCRIPT_DIR"
echo "  Dispatch: $(du -h /tmp/polypaint-dispatch.zip | cut -f1)  (pure Python)"

# Raster-MT: handler_raster_mt.py + shared.py + solve_score_chain.py + roots2pix_mt
RASTER_MT_DIR=/tmp/polypaint-raster-mt
rm -rf "$RASTER_MT_DIR"
mkdir -p "$RASTER_MT_DIR/lib"
cp lambda/handler_raster_mt.py lambda/shared.py lambda/solve_score_chain.py lambda/logical_sections.py "$RASTER_MT_DIR/"
cp lambda/roots2pix_mt "$RASTER_MT_DIR/"
cp lambda/roots2pix_mt_lib/* "$RASTER_MT_DIR/lib/" 2>/dev/null || true
chmod +x "$RASTER_MT_DIR"/roots2pix_mt
cd "$RASTER_MT_DIR" && zip -r9 /tmp/polypaint-raster-mt.zip . -q && cd "$SCRIPT_DIR"
echo "  RastMT:   $(du -h /tmp/polypaint-raster-mt.zip | cut -f1)  (fused roots2pix_mt)"

# Finalize-MT: fused solve-score assemble + encode from raw score bins
FINALIZE_MT_DIR=/tmp/polypaint-finalize-mt
rm -rf "$FINALIZE_MT_DIR"
mkdir -p "$FINALIZE_MT_DIR/lib"
cp lambda/handler_finalize_mt.py lambda/shared.py lambda/color_artifact_meta.py lambda/solve_score_chain.py lambda/raw_sidecar.py lambda/raw_score_render.py "$FINALIZE_MT_DIR/"
cp lambda/assemble_greyscale lambda/score_raw_render "$FINALIZE_MT_DIR/"
cp lambda/assemble_greyscale_lib/* "$FINALIZE_MT_DIR/lib/" 2>/dev/null || true
chmod +x "$FINALIZE_MT_DIR"/assemble_greyscale "$FINALIZE_MT_DIR"/score_raw_render
cd "$FINALIZE_MT_DIR" && zip -r9 /tmp/polypaint-finalize-mt.zip . -q && cd "$SCRIPT_DIR"
echo "  FnlzMT:   $(du -h /tmp/polypaint-finalize-mt.zip | cut -f1)  (fused assemble + encode)"

# Preview: handler_preview.py + shared.py (pure Python, PNG via zlib)
PREVIEW_DIR=/tmp/polypaint-preview
rm -rf "$PREVIEW_DIR"
mkdir -p "$PREVIEW_DIR"
cp lambda/handler_preview.py lambda/shared.py "$PREVIEW_DIR/"
cd "$PREVIEW_DIR" && zip -r9 /tmp/polypaint-preview.zip . -q && cd "$SCRIPT_DIR"
echo "  Preview:  $(du -h /tmp/polypaint-preview.zip | cut -f1)  (pure Python)"

# Compute Preview: handler_compute_preview.py + shared.py + coeffgen/solve binaries
COMPUTE_PREVIEW_DIR=/tmp/polypaint-compute-preview
rm -rf "$COMPUTE_PREVIEW_DIR"
mkdir -p "$COMPUTE_PREVIEW_DIR"
cp lambda/handler_compute_preview.py lambda/shared.py "$COMPUTE_PREVIEW_DIR/"
cp lambda/sweep_coeffgen lambda/sweep lambda/sweep_mt lambda/sweep_cm "$COMPUTE_PREVIEW_DIR/"
chmod +x "$COMPUTE_PREVIEW_DIR"/sweep_coeffgen "$COMPUTE_PREVIEW_DIR"/sweep "$COMPUTE_PREVIEW_DIR"/sweep_mt "$COMPUTE_PREVIEW_DIR"/sweep_cm
cd "$COMPUTE_PREVIEW_DIR" && zip -r9 /tmp/polypaint-compute-preview.zip . -q && cd "$SCRIPT_DIR"
echo "  CPreview: $(du -h /tmp/polypaint-compute-preview.zip | cut -f1)  (sync coeffgen+solve preview)"

# Bilevel: handler_bilevel.py + shared.py + bilevel (needs libvips layer)
BILEVEL_DIR=/tmp/polypaint-bilevel
rm -rf "$BILEVEL_DIR"
mkdir -p "$BILEVEL_DIR"
cp lambda/handler_bilevel.py lambda/shared.py lambda/logical_sections.py lambda/raw_sidecar.py lambda/color_artifact_meta.py lambda/solve_score_chain.py "$BILEVEL_DIR/"
cp lambda/bilevel_raster lambda/bilevel_section_raster lambda/coeffs_bilevel_raster lambda/bilevel_merge lambda/raw_to_bilevel "$BILEVEL_DIR/"
chmod +x "$BILEVEL_DIR"/bilevel_raster "$BILEVEL_DIR"/bilevel_section_raster "$BILEVEL_DIR"/coeffs_bilevel_raster "$BILEVEL_DIR"/bilevel_merge "$BILEVEL_DIR"/raw_to_bilevel
cd "$BILEVEL_DIR" && zip -r9 /tmp/polypaint-bilevel.zip . -q && cd "$SCRIPT_DIR"
echo "  Bilevel:  $(du -h /tmp/polypaint-bilevel.zip | cut -f1)  (bilevel raster+section+merge)"

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

# Autolevels: handler_autolevels.py + shared.py + color_artifact_meta.py + autolevels_render (needs libvips layer)
AUTOLEVELS_DIR=/tmp/polypaint-autolevels
rm -rf "$AUTOLEVELS_DIR"
mkdir -p "$AUTOLEVELS_DIR"
cp lambda/handler_autolevels.py lambda/shared.py lambda/color_artifact_meta.py lambda/solve_score_chain.py "$AUTOLEVELS_DIR/"
cp lambda/autolevels_render "$AUTOLEVELS_DIR/"
chmod +x "$AUTOLEVELS_DIR"/autolevels_render
cd "$AUTOLEVELS_DIR" && zip -r9 /tmp/polypaint-autolevels.zip . -q && cd "$SCRIPT_DIR"
echo "  AutoLvl: $(du -h /tmp/polypaint-autolevels.zip | cut -f1)  (autolevels_render + libvips layer)"

# Resize Artifact: handler_resize_artifact.py + shared.py + color_artifact_meta.py (needs libvips layer)
RESIZE_ARTIFACT_DIR=/tmp/polypaint-resize-artifact
rm -rf "$RESIZE_ARTIFACT_DIR"
mkdir -p "$RESIZE_ARTIFACT_DIR"
cp lambda/handler_resize_artifact.py lambda/shared.py lambda/color_artifact_meta.py lambda/solve_score_chain.py "$RESIZE_ARTIFACT_DIR/"
cd "$RESIZE_ARTIFACT_DIR" && zip -r9 /tmp/polypaint-resize-artifact.zip . -q && cd "$SCRIPT_DIR"
echo "  Resize:  $(du -h /tmp/polypaint-resize-artifact.zip | cut -f1)  (resize artifact + libvips layer)"

# RePalette: handler_repalette.py + shared.py + palette helpers + palette_bins_render + raw2jpeg (needs libvips layer)
REPALETTE_DIR=/tmp/polypaint-repalette
rm -rf "$REPALETTE_DIR"
mkdir -p "$REPALETTE_DIR"
cp lambda/handler_repalette.py lambda/shared.py \
   lambda/palette_names.py lambda/tri_palette_names_generated.py lambda/long_palette_names_generated.py "$REPALETTE_DIR/"
cp lambda/palette_bins_render lambda/raw2jpeg "$REPALETTE_DIR/"
chmod +x "$REPALETTE_DIR"/palette_bins_render "$REPALETTE_DIR"/raw2jpeg
cd "$REPALETTE_DIR" && zip -r9 /tmp/polypaint-repalette.zip . -q && cd "$SCRIPT_DIR"
echo "  RePal:   $(du -h /tmp/polypaint-repalette.zip | cut -f1)  (repalette + libvips layer)"

# Color RePalette: handler_color_repalette.py + shared helpers + legacy pixel_bins_render + direct raw score_raw_render
COLOR_REPALETTE_DIR=/tmp/polypaint-color-repalette
rm -rf "$COLOR_REPALETTE_DIR"
mkdir -p "$COLOR_REPALETTE_DIR"
cp lambda/handler_color_repalette.py lambda/shared.py lambda/raw_sidecar.py lambda/raw_score_render.py \
   lambda/color_artifact_meta.py lambda/solve_score_chain.py lambda/color_recolor_raw.py \
   lambda/palette_names.py lambda/tri_palette_names_generated.py lambda/long_palette_names_generated.py "$COLOR_REPALETTE_DIR/"
cp lambda/pixel_bins_render lambda/score_raw_render "$COLOR_REPALETTE_DIR/"
chmod +x "$COLOR_REPALETTE_DIR"/pixel_bins_render "$COLOR_REPALETTE_DIR"/score_raw_render
cd "$COLOR_REPALETTE_DIR" && zip -r9 /tmp/polypaint-color-repalette.zip . -q && cd "$SCRIPT_DIR"
echo "  ClrRePal: $(du -h /tmp/polypaint-color-repalette.zip | cut -f1)  (pixel_bins_render + score_raw_render)"

# Recolor-from-raw: standalone raw-sidecar recolor path
RECOLOR_FROM_RAW_DIR=/tmp/polypaint-recolor-from-raw
rm -rf "$RECOLOR_FROM_RAW_DIR"
mkdir -p "$RECOLOR_FROM_RAW_DIR"
cp lambda/handler_recolor_from_raw.py lambda/shared.py lambda/raw_sidecar.py lambda/raw_score_render.py lambda/color_recolor_raw.py \
   lambda/color_artifact_meta.py lambda/solve_score_chain.py \
   lambda/palette_names.py lambda/tri_palette_names_generated.py lambda/long_palette_names_generated.py "$RECOLOR_FROM_RAW_DIR/"
cp lambda/score_raw_render "$RECOLOR_FROM_RAW_DIR/"
chmod +x "$RECOLOR_FROM_RAW_DIR"/score_raw_render
cd "$RECOLOR_FROM_RAW_DIR" && zip -r9 /tmp/polypaint-recolor-from-raw.zip . -q && cd "$SCRIPT_DIR"
echo "  RecolorRaw: $(du -h /tmp/polypaint-recolor-from-raw.zip | cut -f1)  (standalone score_raw_render)"

# ExtractPalette-fused: standalone step_scores.raw -> associated palette path
EXTRACT_PALETTE_FUSED_DIR=/tmp/polypaint-extract-palette-fused
rm -rf "$EXTRACT_PALETTE_FUSED_DIR"
mkdir -p "$EXTRACT_PALETTE_FUSED_DIR"
cp lambda/handler_extract_palette_from_step_scores.py lambda/shared.py lambda/raw_sidecar.py lambda/raw_score_render.py \
   lambda/color_artifact_meta.py lambda/solve_score_chain.py "$EXTRACT_PALETTE_FUSED_DIR/"
cp lambda/score_raw_render lambda/step_scores_to_palette_raw "$EXTRACT_PALETTE_FUSED_DIR/"
chmod +x "$EXTRACT_PALETTE_FUSED_DIR"/score_raw_render "$EXTRACT_PALETTE_FUSED_DIR"/step_scores_to_palette_raw
cd "$EXTRACT_PALETTE_FUSED_DIR" && zip -r9 /tmp/polypaint-extract-palette-fused.zip . -q && cd "$SCRIPT_DIR"
echo "  ExtPalFx: $(du -h /tmp/polypaint-extract-palette-fused.zip | cut -f1)  (step_scores_to_palette_raw + score_raw_render)"

# PDF Artifact: handler_pdf_artifact.py + shared.py + color artifact metadata + spread builder
PDF_ARTIFACT_DIR=/tmp/polypaint-pdf-artifact
rm -rf "$PDF_ARTIFACT_DIR"
mkdir -p "$PDF_ARTIFACT_DIR"
cp lambda/handler_pdf_artifact.py lambda/shared.py lambda/color_artifact_meta.py lambda/solve_score_chain.py lambda/spread_pdf.py "$PDF_ARTIFACT_DIR/"
cd "$PDF_ARTIFACT_DIR" && zip -r9 /tmp/polypaint-pdf-artifact.zip . -q && cd "$SCRIPT_DIR"
echo "  PDFArt:  $(du -h /tmp/polypaint-pdf-artifact.zip | cut -f1)  (spread builder + python pdf layer)"

# DeepZoom Export: handler_deepzoom_export.py + shared.py + dz_export + raw-sidecar colorization helpers (needs libvips layer)
DZ_EXPORT_DIR=/tmp/polypaint-deepzoom-export
rm -rf "$DZ_EXPORT_DIR"
mkdir -p "$DZ_EXPORT_DIR"
cp lambda/handler_deepzoom_export.py lambda/shared.py lambda/raw_sidecar.py lambda/deepzoom_viewer_template.html "$DZ_EXPORT_DIR/"
cp lambda/dz_export lambda/pixel_bins_render lambda/raw2jpeg "$DZ_EXPORT_DIR/"
chmod +x "$DZ_EXPORT_DIR"/dz_export "$DZ_EXPORT_DIR"/pixel_bins_render "$DZ_EXPORT_DIR"/raw2jpeg
cd "$DZ_EXPORT_DIR" && zip -r9 /tmp/polypaint-deepzoom-export.zip . -q && cd "$SCRIPT_DIR"
echo "  DzExp:   $(du -h /tmp/polypaint-deepzoom-export.zip | cut -f1)  (dz_export + raw-sidecar colorization + libvips layer)"

# DeepZoom-from-raw: strict wrapper over the raw-sidecar DeepZoom path
DZ_FROM_RAW_DIR=/tmp/polypaint-deepzoom-from-raw
rm -rf "$DZ_FROM_RAW_DIR"
mkdir -p "$DZ_FROM_RAW_DIR"
cp lambda/handler_deepzoom_from_raw.py lambda/handler_deepzoom_export.py lambda/shared.py lambda/raw_sidecar.py lambda/deepzoom_viewer_template.html "$DZ_FROM_RAW_DIR/"
cp lambda/dz_export lambda/pixel_bins_render lambda/raw2jpeg "$DZ_FROM_RAW_DIR/"
chmod +x "$DZ_FROM_RAW_DIR"/dz_export "$DZ_FROM_RAW_DIR"/pixel_bins_render "$DZ_FROM_RAW_DIR"/raw2jpeg
cd "$DZ_FROM_RAW_DIR" && zip -r9 /tmp/polypaint-deepzoom-from-raw.zip . -q && cd "$SCRIPT_DIR"
echo "  DzRaw:   $(du -h /tmp/polypaint-deepzoom-from-raw.zip | cut -f1)  (raw-sidecar-only deepzoom)"

# Solve Proximity: handler_solve_proximity.py + shared.py + solve_score_chain.py + logical section helpers + solve_proximity_stats binary
SP_DIR=/tmp/polypaint-solve-proximity
rm -rf "$SP_DIR"
mkdir -p "$SP_DIR/lib"
cp lambda/handler_solve_proximity.py lambda/shared.py lambda/solve_score_chain.py \
   lambda/logical_sections.py "$SP_DIR/"
cp lambda/solve_proximity_stats lambda/solve_proximity_hist_sectioned "$SP_DIR/"
cp lambda/solve_proximity_hist_sectioned_lib/* "$SP_DIR/lib/" 2>/dev/null || true
chmod +x "$SP_DIR"/solve_proximity_stats "$SP_DIR"/solve_proximity_hist_sectioned
cd "$SP_DIR" && zip -r9 /tmp/polypaint-solve-proximity.zip . -q && cd "$SCRIPT_DIR"
echo "  SolvPrx: $(du -h /tmp/polypaint-solve-proximity.zip | cut -f1)  (solve_proximity_stats + sectioned hist)"

# Solve Proximity Bench: benchmark handler + solve proximity helpers + logical section helpers + solve_proximity_stats binary
SP_BENCH_DIR=/tmp/polypaint-solve-proximity-bench
rm -rf "$SP_BENCH_DIR"
mkdir -p "$SP_BENCH_DIR/lib"
cp lambda/handler_solve_proximity_bench.py lambda/handler_solve_proximity.py lambda/shared.py \
   lambda/solve_score_chain.py lambda/logical_sections.py "$SP_BENCH_DIR/"
cp lambda/solve_proximity_stats lambda/solve_proximity_hist_sectioned "$SP_BENCH_DIR/"
cp lambda/solve_proximity_hist_sectioned_lib/* "$SP_BENCH_DIR/lib/" 2>/dev/null || true
chmod +x "$SP_BENCH_DIR"/solve_proximity_stats "$SP_BENCH_DIR"/solve_proximity_hist_sectioned
cd "$SP_BENCH_DIR" && zip -r9 /tmp/polypaint-solve-proximity-bench.zip . -q && cd "$SCRIPT_DIR"
echo "  SolvPrxB: $(du -h /tmp/polypaint-solve-proximity-bench.zip | cut -f1)  (AWS hist benchmark)"

# Palette Debug: handler_palette_debug.py + shared.py + solve_palette_debug + raw2jpeg (needs libvips layer)
PD_DIR=/tmp/polypaint-palette-debug
rm -rf "$PD_DIR"
mkdir -p "$PD_DIR"
cp lambda/handler_palette_debug.py lambda/shared.py \
   lambda/palette_names.py lambda/tri_palette_names_generated.py lambda/long_palette_names_generated.py "$PD_DIR/"
cp lambda/solve_palette_debug lambda/raw2jpeg "$PD_DIR/"
chmod +x "$PD_DIR"/solve_palette_debug "$PD_DIR"/raw2jpeg
cd "$PD_DIR" && zip -r9 /tmp/polypaint-palette-debug.zip . -q && cd "$SCRIPT_DIR"
echo "  PalDbg:  $(du -h /tmp/polypaint-palette-debug.zip | cut -f1)  (palette debug)"

# Palette Orchestrator: handler_palette_orchestrator.py + shared.py
PAL_ORCH_DIR=/tmp/polypaint-palette-orchestrator
rm -rf "$PAL_ORCH_DIR"
mkdir -p "$PAL_ORCH_DIR"
cp lambda/handler_palette_orchestrator.py lambda/shared.py "$PAL_ORCH_DIR/"
cd "$PAL_ORCH_DIR" && zip -r9 /tmp/polypaint-palette-orchestrator.zip . -q && cd "$SCRIPT_DIR"
echo "  PalOrch: $(du -h /tmp/polypaint-palette-orchestrator.zip | cut -f1)  (starter only)"

# Palette Render Plan: handler_palette_render_plan.py + shared.py + chunk helpers
PAL_PLAN_DIR=/tmp/polypaint-palette-render-plan
rm -rf "$PAL_PLAN_DIR"
mkdir -p "$PAL_PLAN_DIR"
cp lambda/handler_palette_render_plan.py lambda/shared.py \
   lambda/calc_chunks.py \
   lambda/param_source.py \
   lambda/color_artifact_meta.py \
   lambda/solve_score_chain.py \
   lambda/logical_sections.py \
   lambda/palette_names.py lambda/tri_palette_names_generated.py lambda/long_palette_names_generated.py "$PAL_PLAN_DIR/"
cd "$PAL_PLAN_DIR" && zip -r9 /tmp/polypaint-palette-render-plan.zip . -q && cd "$SCRIPT_DIR"
echo "  PalPlan: $(du -h /tmp/polypaint-palette-render-plan.zip | cut -f1)  (plan builder)"

# Palette Chunk: handler_palette_chunk.py + shared.py + solve_score_chain.py + logical section helpers + solve_palette_chunk + solve_palette_chunk_mt
PAL_CHUNK_DIR=/tmp/polypaint-palette-chunk
rm -rf "$PAL_CHUNK_DIR"
mkdir -p "$PAL_CHUNK_DIR/lib"
cp lambda/handler_palette_chunk.py lambda/shared.py lambda/solve_score_chain.py \
   lambda/logical_sections.py "$PAL_CHUNK_DIR/"
cp lambda/solve_palette_chunk lambda/solve_palette_chunk_mt "$PAL_CHUNK_DIR/"
cp lambda/solve_palette_chunk_mt_lib/* "$PAL_CHUNK_DIR/lib/" 2>/dev/null || true
chmod +x "$PAL_CHUNK_DIR"/solve_palette_chunk "$PAL_CHUNK_DIR"/solve_palette_chunk_mt
cd "$PAL_CHUNK_DIR" && zip -r9 /tmp/polypaint-palette-chunk.zip . -q && cd "$SCRIPT_DIR"
echo "  PalChnk: $(du -h /tmp/polypaint-palette-chunk.zip | cut -f1)  (chunk scorer)"

# Palette Finalize: handler_palette_finalize.py + shared.py + palette_bins_render + raw2jpeg (needs libvips layer)
PAL_FINAL_DIR=/tmp/polypaint-palette-finalize
rm -rf "$PAL_FINAL_DIR"
mkdir -p "$PAL_FINAL_DIR"
cp lambda/handler_palette_finalize.py lambda/shared.py lambda/solve_score_chain.py "$PAL_FINAL_DIR/"
cp lambda/palette_bins_render lambda/raw2jpeg "$PAL_FINAL_DIR/"
chmod +x "$PAL_FINAL_DIR"/palette_bins_render "$PAL_FINAL_DIR"/raw2jpeg
cd "$PAL_FINAL_DIR" && zip -r9 /tmp/polypaint-palette-finalize.zip . -q && cd "$SCRIPT_DIR"
echo "  PalFin:  $(du -h /tmp/polypaint-palette-finalize.zip | cut -f1)  (finalize + libvips layer)"

# Attach Palette to Color: handler_attach_palette_to_color.py + shared.py + color_artifact_meta.py
ATTACH_PAL_DIR=/tmp/polypaint-attach-palette-to-color
rm -rf "$ATTACH_PAL_DIR"
mkdir -p "$ATTACH_PAL_DIR"
cp lambda/handler_attach_palette_to_color.py lambda/shared.py lambda/color_artifact_meta.py lambda/solve_score_chain.py "$ATTACH_PAL_DIR/"
cd "$ATTACH_PAL_DIR" && zip -r9 /tmp/polypaint-attach-palette-to-color.zip . -q && cd "$SCRIPT_DIR"
echo "  PalAtt:  $(du -h /tmp/polypaint-attach-palette-to-color.zip | cut -f1)  (attach associated palette metadata)"

# Render Orchestrator (starter): handler_render_orchestrator.py + shared.py
ORCH_DIR=/tmp/polypaint-render-orchestrator
rm -rf "$ORCH_DIR"
mkdir -p "$ORCH_DIR"
cp lambda/handler_render_orchestrator.py lambda/shared.py "$ORCH_DIR/"
cd "$ORCH_DIR" && zip -r9 /tmp/polypaint-render-orchestrator.zip . -q && cd "$SCRIPT_DIR"
echo "  RndOrch: $(du -h /tmp/polypaint-render-orchestrator.zip | cut -f1)  (starter only)"

# Render Plan: handler_render_plan.py + shared.py + logical section helpers + chunk helpers
PLAN_DIR=/tmp/polypaint-render-plan
rm -rf "$PLAN_DIR"
mkdir -p "$PLAN_DIR"
cp lambda/handler_render_plan.py lambda/shared.py \
   lambda/logical_sections.py \
   lambda/calc_chunks.py \
   lambda/param_source.py \
   lambda/solve_score_chain.py \
   lambda/palette_names.py lambda/tri_palette_names_generated.py lambda/long_palette_names_generated.py "$PLAN_DIR/"
cd "$PLAN_DIR" && zip -r9 /tmp/polypaint-render-plan.zip . -q && cd "$SCRIPT_DIR"
echo "  RndPlan: $(du -h /tmp/polypaint-render-plan.zip | cut -f1)  (plan builder)"

# Render Status: handler_render_status.py + shared.py
STATUS_DIR=/tmp/polypaint-render-status
rm -rf "$STATUS_DIR"
mkdir -p "$STATUS_DIR"
cp lambda/handler_render_status.py lambda/shared.py "$STATUS_DIR/"
cd "$STATUS_DIR" && zip -r9 /tmp/polypaint-render-status.zip . -q && cd "$SCRIPT_DIR"
echo "  RndStat: $(du -h /tmp/polypaint-render-status.zip | cut -f1)  (status updater)"

# Compute Orchestrator (starter): handler_compute_orchestrator.py + shared.py + compute_fused.py
COMP_ORCH_DIR=/tmp/polypaint-compute-orchestrator
rm -rf "$COMP_ORCH_DIR"
mkdir -p "$COMP_ORCH_DIR"
cp lambda/handler_compute_orchestrator.py lambda/shared.py lambda/compute_fused.py "$COMP_ORCH_DIR/"
cd "$COMP_ORCH_DIR" && zip -r9 /tmp/polypaint-compute-orchestrator.zip . -q && cd "$SCRIPT_DIR"
echo "  CmpOrch: $(du -h /tmp/polypaint-compute-orchestrator.zip | cut -f1)  (starter only)"

# Compute Plan: handler_compute_plan.py + shared.py + compute_fused.py
COMP_PLAN_DIR=/tmp/polypaint-compute-plan
rm -rf "$COMP_PLAN_DIR"
mkdir -p "$COMP_PLAN_DIR"
cp lambda/handler_compute_plan.py lambda/shared.py lambda/compute_fused.py "$COMP_PLAN_DIR/"
cd "$COMP_PLAN_DIR" && zip -r9 /tmp/polypaint-compute-plan.zip . -q && cd "$SCRIPT_DIR"
echo "  CmpPlan: $(du -h /tmp/polypaint-compute-plan.zip | cut -f1)  (plan + finalize)"

# Compute Fused Chunk: handler_compute_chunk_fused.py + shared.py + native compute binaries
COMP_FUSED_DIR=/tmp/polypaint-compute-fused-chunk
rm -rf "$COMP_FUSED_DIR"
mkdir -p "$COMP_FUSED_DIR"
cp lambda/handler_compute_chunk_fused.py lambda/shared.py "$COMP_FUSED_DIR/"
cp lambda/sweep_coeffgen lambda/sweep lambda/sweep_mt lambda/sweep_cm "$COMP_FUSED_DIR/"
chmod +x "$COMP_FUSED_DIR"/sweep_coeffgen "$COMP_FUSED_DIR"/sweep "$COMP_FUSED_DIR"/sweep_mt "$COMP_FUSED_DIR"/sweep_cm
cd "$COMP_FUSED_DIR" && zip -r9 /tmp/polypaint-compute-fused-chunk.zip . -q && cd "$SCRIPT_DIR"
echo "  CmpFuse: $(du -h /tmp/polypaint-compute-fused-chunk.zip | cut -f1)  (fused chunk worker)"

# Compute Status: handler_compute_status.py + shared.py
COMP_STATUS_DIR=/tmp/polypaint-compute-status
rm -rf "$COMP_STATUS_DIR"
mkdir -p "$COMP_STATUS_DIR"
cp lambda/handler_compute_status.py lambda/shared.py "$COMP_STATUS_DIR/"
cd "$COMP_STATUS_DIR" && zip -r9 /tmp/polypaint-compute-status.zip . -q && cd "$SCRIPT_DIR"
echo "  CmpStat: $(du -h /tmp/polypaint-compute-status.zip | cut -f1)  (status updater)"

# Sweep-CM: handler_sweep_cm.py + shared.py + sweep_cm (needs LAPACK layer)
CM_DIR=/tmp/polypaint-sweep-cm
rm -rf "$CM_DIR"
mkdir -p "$CM_DIR"
cp lambda/handler_sweep_cm.py lambda/shared.py "$CM_DIR/"
cp lambda/sweep_cm "$CM_DIR/"
chmod +x "$CM_DIR"/sweep_cm
cd "$CM_DIR" && zip -r9 /tmp/polypaint-sweep-cm.zip . -q && cd "$SCRIPT_DIR"
echo "  SweepCM: $(du -h /tmp/polypaint-sweep-cm.zip | cut -f1)  (sweep_cm + LAPACK layer)"

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
        create_lambda "$NAME" "$HANDLER" "$ZIP" "$MEM" "$ROLE_ARN" "$LAYERS" "$ENV_VARS" "$TMP"
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
    for FNAME in "$SWEEP_NAME" "$SWEEP_MT_NAME" "$COEFFGEN_NAME" "$ENCODE_NAME" "$VIEWPORT_NAME" "$STORAGE_NAME" "$DISPATCH_NAME" "$PREVIEW_NAME" "$COMPUTE_PREVIEW_NAME" "$BILEVEL_NAME" "$BILEVEL_STITCH_NAME" "$PARAM_DEBUG_NAME" "$TIFF_COMPAT_NAME" "$PNG_EXPORT_NAME" "$DZ_EXPORT_NAME" "$SWEEP_CM_NAME" "$SOLVE_PROXIMITY_NAME" "$PALETTE_DEBUG_NAME" "$REPALETTE_NAME"; do
        aws lambda add-permission --function-name "$FNAME" \
            --statement-id "apigateway-invoke" \
            --action lambda:InvokeFunction \
            --principal apigateway.amazonaws.com \
            --source-arn "arn:aws:execute-api:$REGION:$ACCT:$API_ID/*/*" \
            --region "$REGION" >/dev/null 2>&1 || true
    done

    # Create integrations
    echo "  Creating integrations..."
    local SWEEP_INT SWEEP_MT_INT COEFFGEN_INT ENCODE_INT VIEWPORT_INT STORAGE_INT DISPATCH_INT
    SWEEP_INT=$(create_integration "$SWEEP_NAME")
    SWEEP_MT_INT=$(create_integration "$SWEEP_MT_NAME")
    COEFFGEN_INT=$(create_integration "$COEFFGEN_NAME")
    ENCODE_INT=$(create_integration "$ENCODE_NAME")
    VIEWPORT_INT=$(create_integration "$VIEWPORT_NAME")
    STORAGE_INT=$(create_integration "$STORAGE_NAME")
    DISPATCH_INT=$(create_integration "$DISPATCH_NAME")

    # Create routes
    echo "  Setting up routes..."
    ensure_route "POST /sweep" "$SWEEP_INT"
    ensure_route "POST /sweep-mt" "$SWEEP_MT_INT"
    ensure_route "POST /coeffgen" "$COEFFGEN_INT"

    local PREVIEW_INT COMPUTE_PREVIEW_INT
    PREVIEW_INT=$(create_integration "$PREVIEW_NAME")
    COMPUTE_PREVIEW_INT=$(create_integration "$COMPUTE_PREVIEW_NAME")
    local PARAM_DEBUG_INT
    PARAM_DEBUG_INT=$(create_integration "$PARAM_DEBUG_NAME")
    ensure_route "POST /preview" "$PREVIEW_INT"
    ensure_route "POST /compute-preview" "$COMPUTE_PREVIEW_INT"
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
    local SOLVE_PROXIMITY_INT
    SOLVE_PROXIMITY_INT=$(create_integration "$SOLVE_PROXIMITY_NAME")
    ensure_route "POST /solve-proximity" "$SOLVE_PROXIMITY_INT"
    local PALETTE_DEBUG_INT
    PALETTE_DEBUG_INT=$(create_integration "$PALETTE_DEBUG_NAME")
    ensure_route "POST /palette-debug" "$PALETTE_DEBUG_INT"

    ensure_route "POST /encode-upload" "$ENCODE_INT"
    ensure_route "POST /viewport" "$VIEWPORT_INT"
    ensure_route "POST /list" "$STORAGE_INT"
    ensure_route "POST /list-favorites" "$STORAGE_INT"
    ensure_route "POST /add-favorite" "$STORAGE_INT"
    ensure_route "POST /delete-favorite" "$STORAGE_INT"
    ensure_route "POST /list-palettes" "$STORAGE_INT"
    ensure_route "POST /delete-palette" "$STORAGE_INT"
    ensure_route "POST /delete-render-artifact" "$STORAGE_INT"
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
    ensure_route "POST /delete-prefix" "$STORAGE_INT"
    ensure_route "POST /list-deepzoom" "$STORAGE_INT"
    ensure_route "POST /dispatch-render" "$DISPATCH_INT"

    # Get API URL and write config.json
    local API_URL
    API_URL=$(aws apigatewayv2 get-api --api-id "$API_ID" --region "$REGION" \
        --query 'ApiEndpoint' --output text)
    echo "  API Gateway URL: $API_URL"

    build_deploy_metadata

    printf '{
  "sweep": "%s/sweep",
  "sweep-mt": "%s/sweep-mt",
  "coeffgen": "%s/coeffgen",
  "encode": "%s/encode-upload",
  "viewport": "%s/viewport",
  "preview": "%s/preview",
  "compute-preview": "%s/compute-preview",
  "storage": "%s",
  "dispatch": "%s/dispatch-render",
  "param-debug": "%s/param-debug",
  "tiff-compat": "%s/tiff-compat",
  "png-export": "%s/png-export",
  "deepzoom-export": "%s/deepzoom-export",
  "sweep-cm": "%s/sweep-cm",
  "solve_proximity": "%s/solve-proximity",
  "palette-debug": "%s/palette-debug",
  "build": {
    "build_id": "%s",
    "deployed_at_utc": "%s",
    "git_rev": "%s",
    "git_dirty": %s,
    "frontend_sha256": "%s"
  }
}' "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$BUILD_ID" "$BUILD_DEPLOYED_AT_UTC" "$BUILD_GIT_REV" "$BUILD_GIT_DIRTY" "$BUILD_FRONTEND_SHA256" \
    | aws s3 cp - "s3://$BUCKET/config.json" \
        --content-type "application/json" --region "$REGION"
    echo "  config.json uploaded"
    echo "  Build ID: $BUILD_ID"
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
            \"Action\": [\"dynamodb:PutItem\", \"dynamodb:GetItem\", \"dynamodb:DeleteItem\", \"dynamodb:Query\", \"dynamodb:BatchWriteItem\"],
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
        "$SWEEP_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE" "$BINARY_TMP"

    create_lambda "$SWEEP_MT_NAME" "handler_sweep_mt.handler" "/tmp/polypaint-sweep-mt.zip" \
        "$SWEEP_MT_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE" "$BINARY_TMP"

    create_lambda "$COEFFGEN_NAME" "handler_coeffgen.handler" "/tmp/polypaint-coeffgen.zip" \
        "$COEFFGEN_MEMORY" "$ROLE_ARN" "$LAPACK_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib,FUSED_WORKER_MEMORY_MB=$COMPUTE_FUSED_CHUNK_MEMORY,FUSED_WORKER_TMP_MB=$BINARY_TMP,FUSED_WORKER_TIMEOUT_SEC=$TIMEOUT" "$BINARY_TMP"

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
        "$DISPATCH_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,ENCODE_FUNCTION=$ENCODE_NAME,SWEEP_FUNCTION=$SWEEP_NAME,SWEEP_MT_FUNCTION=$SWEEP_MT_NAME,BILEVEL_FUNCTION=$BILEVEL_NAME,COLOR_TO_BILEVEL_FUNCTION=$COLOR_TO_BILEVEL_NAME,BILEVEL_STITCH_FUNCTION=$BILEVEL_STITCH_NAME,DZ_EXPORT_FUNCTION=$DZ_EXPORT_NAME,DZ_FROM_RAW_FUNCTION=$DZ_FROM_RAW_NAME,COEFFGEN_FUNCTION=$COEFFGEN_NAME,SWEEP_CM_FUNCTION=$SWEEP_CM_NAME,RENDER_PREVIEW_FUNCTION=$RENDER_PREVIEW_NAME,AUTOLEVELS_FUNCTION=$AUTOLEVELS_NAME,RESIZE_ARTIFACT_FUNCTION=$RESIZE_ARTIFACT_NAME,REPALETTE_FUNCTION=$REPALETTE_NAME,COLOR_REPALETTE_FUNCTION=$COLOR_REPALETTE_NAME,RECOLOR_FROM_RAW_FUNCTION=$RECOLOR_FROM_RAW_NAME,EXTRACT_PALETTE_FUSED_FUNCTION=$EXTRACT_PALETTE_FUSED_NAME,PDF_ARTIFACT_FUNCTION=$PDF_ARTIFACT_NAME,SOLVE_PROXIMITY_FUNCTION=$SOLVE_PROXIMITY_NAME,RENDER_ORCHESTRATOR_FUNCTION=$RENDER_ORCHESTRATOR_NAME,COMPUTE_ORCHESTRATOR_FUNCTION=$COMPUTE_ORCHESTRATOR_NAME,PALETTE_ORCHESTRATOR_FUNCTION=$PALETTE_ORCHESTRATOR_NAME"
    # Reserve concurrency for dispatch so it's never starved by render/merge Lambdas
    aws lambda put-function-concurrency --function-name "$DISPATCH_NAME" \
        --reserved-concurrent-executions 5 --region "$REGION"

    create_lambda "$RASTER_MT_NAME" "handler_raster_mt.handler" "/tmp/polypaint-raster-mt.zip" \
        "$RASTER_MT_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,RASTER_MT_THREADS=$RASTER_MT_THREADS,LD_LIBRARY_PATH=/var/task/lib" "$BINARY_TMP"
    create_lambda "$FINALIZE_MT_NAME" "handler_finalize_mt.handler" "/tmp/polypaint-finalize-mt.zip" \
        "$FINALIZE_MT_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/var/task/lib:/opt/lib" "$BINARY_TMP"

    create_lambda "$PREVIEW_NAME" "handler_preview.handler" "/tmp/polypaint-preview.zip" \
        "$PREVIEW_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET"

    create_lambda "$COMPUTE_PREVIEW_NAME" "handler_compute_preview.handler" "/tmp/polypaint-compute-preview.zip" \
        "$COMPUTE_PREVIEW_MEMORY" "$ROLE_ARN" "$LAPACK_LAYER" "BUCKET=$BUCKET,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    create_lambda "$BILEVEL_NAME" "handler_bilevel.handler" "/tmp/polypaint-bilevel.zip" \
        "$BILEVEL_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    create_lambda "$COLOR_TO_BILEVEL_NAME" "handler_bilevel.handler" "/tmp/polypaint-bilevel.zip" \
        "$COLOR_TO_BILEVEL_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

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
    create_lambda "$DZ_FROM_RAW_NAME" "handler_deepzoom_from_raw.handler" "/tmp/polypaint-deepzoom-from-raw.zip" \
        "$DZ_FROM_RAW_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    create_lambda "$RENDER_PREVIEW_NAME" "handler_render_preview.handler" "/tmp/polypaint-render-preview.zip" \
        "$RENDER_PREVIEW_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    create_lambda "$AUTOLEVELS_NAME" "handler_autolevels.handler" "/tmp/polypaint-autolevels.zip" \
        "$AUTOLEVELS_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    create_lambda "$RESIZE_ARTIFACT_NAME" "handler_resize_artifact.handler" "/tmp/polypaint-resize-artifact.zip" \
        "$RESIZE_ARTIFACT_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    create_lambda "$REPALETTE_NAME" "handler_repalette.handler" "/tmp/polypaint-repalette.zip" \
        "$REPALETTE_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    create_lambda "$COLOR_REPALETTE_NAME" "handler_color_repalette.handler" "/tmp/polypaint-color-repalette.zip" \
        "$COLOR_REPALETTE_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,ENCODE_FUNCTION=$ENCODE_NAME,RENDER_PREVIEW_FUNCTION=$RENDER_PREVIEW_NAME,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"
    create_lambda "$RECOLOR_FROM_RAW_NAME" "handler_recolor_from_raw.handler" "/tmp/polypaint-recolor-from-raw.zip" \
        "$RECOLOR_FROM_RAW_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"
    create_lambda "$EXTRACT_PALETTE_FUSED_NAME" "handler_extract_palette_from_step_scores.handler" "/tmp/polypaint-extract-palette-fused.zip" \
        "$EXTRACT_PALETTE_FUSED_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    create_lambda "$PDF_ARTIFACT_NAME" "handler_pdf_artifact.handler" "/tmp/polypaint-pdf-artifact.zip" \
        "$PDF_ARTIFACT_MEMORY" "$ROLE_ARN" "$PDF_PY_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE"

    create_lambda "$SOLVE_PROXIMITY_NAME" "handler_solve_proximity.handler" "/tmp/polypaint-solve-proximity.zip" \
        "$SOLVE_PROXIMITY_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/var/task/lib" "$BINARY_TMP"

    create_lambda "$SOLVE_PROXIMITY_BENCH_NAME" "handler_solve_proximity_bench.handler" "/tmp/polypaint-solve-proximity-bench.zip" \
        "$SOLVE_PROXIMITY_BENCH_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,LD_LIBRARY_PATH=/var/task/lib" "$BINARY_TMP"

    create_lambda "$PALETTE_DEBUG_NAME" "handler_palette_debug.handler" "/tmp/polypaint-palette-debug.zip" \
        "$PALETTE_DEBUG_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    create_lambda "$PALETTE_PLAN_NAME" "handler_palette_render_plan.handler" "/tmp/polypaint-palette-render-plan.zip" \
        "$PALETTE_PLAN_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET"

    create_lambda "$PALETTE_CHUNK_NAME" "handler_palette_chunk.handler" "/tmp/polypaint-palette-chunk.zip" \
        "$PALETTE_CHUNK_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/var/task/lib" "$BINARY_TMP"

    create_lambda "$PALETTE_FINALIZE_NAME" "handler_palette_finalize.handler" "/tmp/polypaint-palette-finalize.zip" \
        "$PALETTE_FINALIZE_MEMORY" "$ROLE_ARN" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    create_lambda "$ATTACH_PALETTE_NAME" "handler_attach_palette_to_color.handler" "/tmp/polypaint-attach-palette-to-color.zip" \
        "$ATTACH_PALETTE_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE"

    # Render plan + status Lambdas
    create_lambda "$RENDER_PLAN_NAME" "handler_render_plan.handler" "/tmp/polypaint-render-plan.zip" \
        "$RENDER_PLAN_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,VIEWPORT_FUNCTION=$VIEWPORT_NAME,STORAGE_FUNCTION=$STORAGE_NAME,RASTER_MT_FUNCTION=$RASTER_MT_NAME"

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
    COMPUTE_PLAN_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${COMPUTE_PLAN_NAME}"
    COMPUTE_FUSED_CHUNK_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${COMPUTE_FUSED_CHUNK_NAME}"
    COMPUTE_STATUS_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${COMPUTE_STATUS_NAME}"
    COEFFGEN_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${COEFFGEN_NAME}"
    FINALIZE_MT_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${FINALIZE_MT_NAME}"
    ENCODE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${ENCODE_NAME}"
    STORAGE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${STORAGE_NAME}"
    BILEVEL_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${BILEVEL_NAME}"
    BILEVEL_STITCH_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${BILEVEL_STITCH_NAME}"
    SOLVE_PROXIMITY_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${SOLVE_PROXIMITY_NAME}"
    PREVIEW_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${RENDER_PREVIEW_NAME}"

    sed -e "s|\${PlanFunctionArn}|${RENDER_PLAN_ARN}|g" \
        -e "s|\${StatusFunctionArn}|${RENDER_STATUS_ARN}|g" \
        -e "s|\${FinalizeMTFunctionArn}|${FINALIZE_MT_ARN}|g" \
        -e "s|\${EncodeFunctionArn}|${ENCODE_ARN}|g" \
        -e "s|\${StorageFunctionArn}|${STORAGE_ARN}|g" \
        -e "s|\${BilevelFunctionArn}|${BILEVEL_ARN}|g" \
        -e "s|\${BilevelStitchFunctionArn}|${BILEVEL_STITCH_ARN}|g" \
        -e "s|\${SolveProximityFunctionArn}|${SOLVE_PROXIMITY_ARN}|g" \
        -e "s|\${PreviewFunctionArn}|${PREVIEW_ARN}|g" \
        stepfunctions/render_workflow.asl.json.template > /tmp/render_workflow.asl.json

    sed -e "s|\${PlanFunctionArn}|${COMPUTE_PLAN_ARN}|g" \
        -e "s|\${StatusFunctionArn}|${COMPUTE_STATUS_ARN}|g" \
        -e "s|\${CoeffgenFunctionArn}|${COEFFGEN_ARN}|g" \
        -e "s|\${FusedChunkFunctionArn}|${COMPUTE_FUSED_CHUNK_ARN}|g" \
        stepfunctions/compute_workflow.asl.json.template > /tmp/compute_workflow.asl.json

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

    COMPUTE_SM_ARN=$(aws stepfunctions create-state-machine \
        --name "$COMPUTE_STATE_MACHINE_NAME" \
        --definition "file:///tmp/compute_workflow.asl.json" \
        --role-arn "$SFN_ROLE_ARN" \
        --type STANDARD \
        --region "$REGION" \
        --query 'stateMachineArn' --output text 2>/dev/null || \
        aws stepfunctions update-state-machine \
            --state-machine-arn "arn:aws:states:${REGION}:${ACCT}:stateMachine:${COMPUTE_STATE_MACHINE_NAME}" \
            --definition "file:///tmp/compute_workflow.asl.json" \
            --role-arn "$SFN_ROLE_ARN" \
            --region "$REGION" \
            --query 'updateDate' --output text)
    echo "  State machine: $COMPUTE_STATE_MACHINE_NAME ($COMPUTE_SM_ARN)"

    COMPUTE_SM_ARN="arn:aws:states:${REGION}:${ACCT}:stateMachine:${COMPUTE_STATE_MACHINE_NAME}"

    PALETTE_PLAN_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${PALETTE_PLAN_NAME}"
    PALETTE_CHUNK_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${PALETTE_CHUNK_NAME}"
    PALETTE_FINALIZE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${PALETTE_FINALIZE_NAME}"
    ATTACH_PALETTE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${ATTACH_PALETTE_NAME}"
    sed -e "s|\${PlanFunctionArn}|${PALETTE_PLAN_ARN}|g" \
        -e "s|\${StatusFunctionArn}|${RENDER_STATUS_ARN}|g" \
        -e "s|\${SolveProximityFunctionArn}|${SOLVE_PROXIMITY_ARN}|g" \
        -e "s|\${PaletteChunkFunctionArn}|${PALETTE_CHUNK_ARN}|g" \
        -e "s|\${PaletteFinalizeFunctionArn}|${PALETTE_FINALIZE_ARN}|g" \
        -e "s|\${AttachPaletteFunctionArn}|${ATTACH_PALETTE_ARN}|g" \
        stepfunctions/palette_workflow.asl.json.template > /tmp/palette_workflow.asl.json

    PALETTE_SM_ARN=$(aws stepfunctions create-state-machine \
        --name "$PALETTE_STATE_MACHINE_NAME" \
        --definition "file:///tmp/palette_workflow.asl.json" \
        --role-arn "$SFN_ROLE_ARN" \
        --type STANDARD \
        --region "$REGION" \
        --query 'stateMachineArn' --output text 2>/dev/null || \
        aws stepfunctions update-state-machine \
            --state-machine-arn "arn:aws:states:${REGION}:${ACCT}:stateMachine:${PALETTE_STATE_MACHINE_NAME}" \
            --definition "file:///tmp/palette_workflow.asl.json" \
            --role-arn "$SFN_ROLE_ARN" \
            --region "$REGION" \
            --query 'updateDate' --output text)
    echo "  State machine: $PALETTE_STATE_MACHINE_NAME ($PALETTE_SM_ARN)"

    PALETTE_SM_ARN="arn:aws:states:${REGION}:${ACCT}:stateMachine:${PALETTE_STATE_MACHINE_NAME}"

    # Starter Lambda — now only needs state machine ARN, not all worker names
    create_lambda "$RENDER_ORCHESTRATOR_NAME" "handler_render_orchestrator.handler" "/tmp/polypaint-render-orchestrator.zip" \
        "$RENDER_ORCHESTRATOR_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,RENDER_STATE_MACHINE_ARN=$RENDER_SM_ARN"
    create_lambda "$COMPUTE_ORCHESTRATOR_NAME" "handler_compute_orchestrator.handler" "/tmp/polypaint-compute-orchestrator.zip" \
        "$COMPUTE_ORCHESTRATOR_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,COMPUTE_STATE_MACHINE_ARN=$COMPUTE_SM_ARN"
    create_lambda "$COMPUTE_PLAN_NAME" "handler_compute_plan.handler" "/tmp/polypaint-compute-plan.zip" \
        "$COMPUTE_PLAN_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,SWEEP_FUNCTION=$SWEEP_NAME,SWEEP_MT_FUNCTION=$SWEEP_MT_NAME,SWEEP_CM_FUNCTION=$SWEEP_CM_NAME,FUSED_WORKER_MEMORY_MB=$COMPUTE_FUSED_CHUNK_MEMORY,FUSED_WORKER_TMP_MB=$BINARY_TMP,FUSED_WORKER_TIMEOUT_SEC=$TIMEOUT"
    create_lambda "$COMPUTE_FUSED_CHUNK_NAME" "handler_compute_chunk_fused.handler" "/tmp/polypaint-compute-fused-chunk.zip" \
        "$COMPUTE_FUSED_CHUNK_MEMORY" "$ROLE_ARN" "$LAPACK_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"
    create_lambda "$COMPUTE_STATUS_NAME" "handler_compute_status.handler" "/tmp/polypaint-compute-status.zip" \
        "$COMPUTE_STATUS_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE"

    create_lambda "$PALETTE_ORCHESTRATOR_NAME" "handler_palette_orchestrator.handler" "/tmp/polypaint-palette-orchestrator.zip" \
        "$PALETTE_ORCHESTRATOR_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,PALETTE_STATE_MACHINE_ARN=$PALETTE_SM_ARN"

    # Add states:StartExecution permission to Lambda role
    aws iam put-role-policy --role-name "$ROLE_NAME" \
        --policy-name polypaint-sfn-start \
        --policy-document "{
            \"Version\": \"2012-10-17\",
            \"Statement\": [{
                \"Effect\": \"Allow\",
                \"Action\": \"states:StartExecution\",
                \"Resource\": [\"${RENDER_SM_ARN}\", \"${COMPUTE_SM_ARN}\", \"${PALETTE_SM_ARN}\"]
            }]
        }"

    create_lambda "$SWEEP_CM_NAME" "handler_sweep_cm.handler" "/tmp/polypaint-sweep-cm.zip" \
        "$SWEEP_CM_MEMORY" "$ROLE_ARN" "$LAPACK_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    # Async invoke config: no retries for most Lambdas (prevents retry storms),
    # but bilevel gets 2 retries / 1hr age to handle concurrency throttle drops.
    for fn in "$FINALIZE_MT_NAME" "$BILEVEL_STITCH_NAME" "$DZ_EXPORT_NAME" "$RENDER_PREVIEW_NAME" "$AUTOLEVELS_NAME" "$RESIZE_ARTIFACT_NAME" "$REPALETTE_NAME" "$PDF_ARTIFACT_NAME" "$SOLVE_PROXIMITY_NAME" "$PALETTE_CHUNK_NAME" "$PALETTE_FINALIZE_NAME" "$ATTACH_PALETTE_NAME"; do
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
    aws lambda put-function-event-invoke-config \
        --function-name "$PALETTE_ORCHESTRATOR_NAME" \
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
    while IFS= read -r asset; do
        aws s3 cp "$SCRIPT_DIR/$asset" "s3://$BUCKET/$asset" \
            --content-type "$(frontend_asset_content_type "$asset")" --region "$REGION"
    done < <(frontend_asset_keys)
    verify_frontend_assets

    echo ""
    echo "=== DEPLOYED ==="
    echo "  Sweep:    $SWEEP_NAME ($SWEEP_MEMORY MB)"
    echo "  Coeffgen: $COEFFGEN_NAME ($COEFFGEN_MEMORY MB)"
    echo "  CmpFuse:  $COMPUTE_FUSED_CHUNK_NAME ($COMPUTE_FUSED_CHUNK_MEMORY MB)"
    echo "  FnlzMT:   $FINALIZE_MT_NAME ($FINALIZE_MT_MEMORY MB)"
    echo "  Encode:   $ENCODE_NAME ($ENCODE_MEMORY MB)"
    echo "  Viewport: $VIEWPORT_NAME ($VIEWPORT_MEMORY MB)"
    echo "  Storage:  $STORAGE_NAME ($STORAGE_MEMORY MB)"
    echo "  Dispatch: $DISPATCH_NAME ($DISPATCH_MEMORY MB)"
    echo "  Preview:  $PREVIEW_NAME ($PREVIEW_MEMORY MB)"
    echo "  SolvPrxB: $SOLVE_PROXIMITY_BENCH_NAME ($SOLVE_PROXIMITY_BENCH_MEMORY MB)"
    echo "  Bilevel:  $BILEVEL_NAME ($BILEVEL_MEMORY MB)"
    echo "  C2B:      $COLOR_TO_BILEVEL_NAME ($COLOR_TO_BILEVEL_MEMORY MB)"
    echo "  BiStitch: $BILEVEL_STITCH_NAME ($BILEVEL_STITCH_MEMORY MB)"

elif [ "$ACTION" = "update" ]; then
    update_lambda "$SWEEP_NAME" "handler_sweep.handler" "/tmp/polypaint-sweep.zip" \
        "$SWEEP_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE" "$BINARY_TMP"

    update_lambda "$SWEEP_MT_NAME" "handler_sweep_mt.handler" "/tmp/polypaint-sweep-mt.zip" \
        "$SWEEP_MT_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE" "$BINARY_TMP"

    update_lambda "$COEFFGEN_NAME" "handler_coeffgen.handler" "/tmp/polypaint-coeffgen.zip" \
        "$COEFFGEN_MEMORY" "$LAPACK_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib,FUSED_WORKER_MEMORY_MB=$COMPUTE_FUSED_CHUNK_MEMORY,FUSED_WORKER_TMP_MB=$BINARY_TMP,FUSED_WORKER_TIMEOUT_SEC=$TIMEOUT" "$BINARY_TMP"

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
        "$DISPATCH_MEMORY" "" "BUCKET=$BUCKET,ENCODE_FUNCTION=$ENCODE_NAME,SWEEP_FUNCTION=$SWEEP_NAME,SWEEP_MT_FUNCTION=$SWEEP_MT_NAME,BILEVEL_FUNCTION=$BILEVEL_NAME,COLOR_TO_BILEVEL_FUNCTION=$COLOR_TO_BILEVEL_NAME,BILEVEL_STITCH_FUNCTION=$BILEVEL_STITCH_NAME,DZ_EXPORT_FUNCTION=$DZ_EXPORT_NAME,DZ_FROM_RAW_FUNCTION=$DZ_FROM_RAW_NAME,COEFFGEN_FUNCTION=$COEFFGEN_NAME,SWEEP_CM_FUNCTION=$SWEEP_CM_NAME,RENDER_PREVIEW_FUNCTION=$RENDER_PREVIEW_NAME,AUTOLEVELS_FUNCTION=$AUTOLEVELS_NAME,RESIZE_ARTIFACT_FUNCTION=$RESIZE_ARTIFACT_NAME,REPALETTE_FUNCTION=$REPALETTE_NAME,COLOR_REPALETTE_FUNCTION=$COLOR_REPALETTE_NAME,RECOLOR_FROM_RAW_FUNCTION=$RECOLOR_FROM_RAW_NAME,EXTRACT_PALETTE_FUSED_FUNCTION=$EXTRACT_PALETTE_FUSED_NAME,PDF_ARTIFACT_FUNCTION=$PDF_ARTIFACT_NAME,SOLVE_PROXIMITY_FUNCTION=$SOLVE_PROXIMITY_NAME,RENDER_ORCHESTRATOR_FUNCTION=$RENDER_ORCHESTRATOR_NAME,COMPUTE_ORCHESTRATOR_FUNCTION=$COMPUTE_ORCHESTRATOR_NAME,PALETTE_ORCHESTRATOR_FUNCTION=$PALETTE_ORCHESTRATOR_NAME"
    # Reserve concurrency for dispatch so it's never starved by render/merge Lambdas
    aws lambda put-function-concurrency --function-name "$DISPATCH_NAME" \
        --reserved-concurrent-executions 5 --region "$REGION"

    update_lambda "$RASTER_MT_NAME" "handler_raster_mt.handler" "/tmp/polypaint-raster-mt.zip" \
        "$RASTER_MT_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,RASTER_MT_THREADS=$RASTER_MT_THREADS,LD_LIBRARY_PATH=/var/task/lib" "$BINARY_TMP"
    update_lambda "$FINALIZE_MT_NAME" "handler_finalize_mt.handler" "/tmp/polypaint-finalize-mt.zip" \
        "$FINALIZE_MT_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/var/task/lib:/opt/lib" "$BINARY_TMP"

    update_lambda "$PREVIEW_NAME" "handler_preview.handler" "/tmp/polypaint-preview.zip" \
        "$PREVIEW_MEMORY" "" "BUCKET=$BUCKET"

    update_lambda "$COMPUTE_PREVIEW_NAME" "handler_compute_preview.handler" "/tmp/polypaint-compute-preview.zip" \
        "$COMPUTE_PREVIEW_MEMORY" "$LAPACK_LAYER" "BUCKET=$BUCKET,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    update_lambda "$BILEVEL_NAME" "handler_bilevel.handler" "/tmp/polypaint-bilevel.zip" \
        "$BILEVEL_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    update_lambda "$COLOR_TO_BILEVEL_NAME" "handler_bilevel.handler" "/tmp/polypaint-bilevel.zip" \
        "$COLOR_TO_BILEVEL_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

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
    update_lambda "$DZ_FROM_RAW_NAME" "handler_deepzoom_from_raw.handler" "/tmp/polypaint-deepzoom-from-raw.zip" \
        "$DZ_FROM_RAW_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    update_lambda "$RENDER_PREVIEW_NAME" "handler_render_preview.handler" "/tmp/polypaint-render-preview.zip" \
        "$RENDER_PREVIEW_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    update_lambda "$AUTOLEVELS_NAME" "handler_autolevels.handler" "/tmp/polypaint-autolevels.zip" \
        "$AUTOLEVELS_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    update_lambda "$RESIZE_ARTIFACT_NAME" "handler_resize_artifact.handler" "/tmp/polypaint-resize-artifact.zip" \
        "$RESIZE_ARTIFACT_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    update_lambda "$REPALETTE_NAME" "handler_repalette.handler" "/tmp/polypaint-repalette.zip" \
        "$REPALETTE_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    update_lambda "$COLOR_REPALETTE_NAME" "handler_color_repalette.handler" "/tmp/polypaint-color-repalette.zip" \
        "$COLOR_REPALETTE_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,ENCODE_FUNCTION=$ENCODE_NAME,RENDER_PREVIEW_FUNCTION=$RENDER_PREVIEW_NAME,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"
    update_lambda "$RECOLOR_FROM_RAW_NAME" "handler_recolor_from_raw.handler" "/tmp/polypaint-recolor-from-raw.zip" \
        "$RECOLOR_FROM_RAW_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"
    update_lambda "$EXTRACT_PALETTE_FUSED_NAME" "handler_extract_palette_from_step_scores.handler" "/tmp/polypaint-extract-palette-fused.zip" \
        "$EXTRACT_PALETTE_FUSED_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    update_lambda "$PDF_ARTIFACT_NAME" "handler_pdf_artifact.handler" "/tmp/polypaint-pdf-artifact.zip" \
        "$PDF_ARTIFACT_MEMORY" "$PDF_PY_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE"

    update_lambda "$SOLVE_PROXIMITY_NAME" "handler_solve_proximity.handler" "/tmp/polypaint-solve-proximity.zip" \
        "$SOLVE_PROXIMITY_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/var/task/lib" "$BINARY_TMP"

    update_lambda "$SOLVE_PROXIMITY_BENCH_NAME" "handler_solve_proximity_bench.handler" "/tmp/polypaint-solve-proximity-bench.zip" \
        "$SOLVE_PROXIMITY_BENCH_MEMORY" "" "BUCKET=$BUCKET,LD_LIBRARY_PATH=/var/task/lib" "$BINARY_TMP"

    update_lambda "$PALETTE_DEBUG_NAME" "handler_palette_debug.handler" "/tmp/polypaint-palette-debug.zip" \
        "$PALETTE_DEBUG_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    update_lambda "$PALETTE_PLAN_NAME" "handler_palette_render_plan.handler" "/tmp/polypaint-palette-render-plan.zip" \
        "$PALETTE_PLAN_MEMORY" "" "BUCKET=$BUCKET"

    update_lambda "$PALETTE_CHUNK_NAME" "handler_palette_chunk.handler" "/tmp/polypaint-palette-chunk.zip" \
        "$PALETTE_CHUNK_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/var/task/lib" "$BINARY_TMP"

    update_lambda "$PALETTE_FINALIZE_NAME" "handler_palette_finalize.handler" "/tmp/polypaint-palette-finalize.zip" \
        "$PALETTE_FINALIZE_MEMORY" "$LIBVIPS_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    update_lambda "$ATTACH_PALETTE_NAME" "handler_attach_palette_to_color.handler" "/tmp/polypaint-attach-palette-to-color.zip" \
        "$ATTACH_PALETTE_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE"

    update_lambda "$RENDER_PLAN_NAME" "handler_render_plan.handler" "/tmp/polypaint-render-plan.zip" \
        "$RENDER_PLAN_MEMORY" "" "BUCKET=$BUCKET,VIEWPORT_FUNCTION=$VIEWPORT_NAME,STORAGE_FUNCTION=$STORAGE_NAME,RASTER_MT_FUNCTION=$RASTER_MT_NAME"

    update_lambda "$RENDER_STATUS_NAME" "handler_render_status.handler" "/tmp/polypaint-render-status.zip" \
        "$RENDER_STATUS_MEMORY" "" "JOBS_TABLE=$JOBS_TABLE"
    update_lambda "$COMPUTE_PLAN_NAME" "handler_compute_plan.handler" "/tmp/polypaint-compute-plan.zip" \
        "$COMPUTE_PLAN_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,SWEEP_FUNCTION=$SWEEP_NAME,SWEEP_MT_FUNCTION=$SWEEP_MT_NAME,SWEEP_CM_FUNCTION=$SWEEP_CM_NAME,FUSED_WORKER_MEMORY_MB=$COMPUTE_FUSED_CHUNK_MEMORY,FUSED_WORKER_TMP_MB=$BINARY_TMP,FUSED_WORKER_TIMEOUT_SEC=$TIMEOUT"
    update_lambda "$COMPUTE_FUSED_CHUNK_NAME" "handler_compute_chunk_fused.handler" "/tmp/polypaint-compute-fused-chunk.zip" \
        "$COMPUTE_FUSED_CHUNK_MEMORY" "$LAPACK_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"
    update_lambda "$COMPUTE_STATUS_NAME" "handler_compute_status.handler" "/tmp/polypaint-compute-status.zip" \
        "$COMPUTE_STATUS_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE"

    # Update Step Functions state machine
    echo "Updating Step Functions state machine..."
    ACCT=$(aws sts get-caller-identity --query 'Account' --output text)
    RENDER_PLAN_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${RENDER_PLAN_NAME}"
    RENDER_STATUS_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${RENDER_STATUS_NAME}"
    COMPUTE_PLAN_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${COMPUTE_PLAN_NAME}"
    COMPUTE_FUSED_CHUNK_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${COMPUTE_FUSED_CHUNK_NAME}"
    COMPUTE_STATUS_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${COMPUTE_STATUS_NAME}"
    COEFFGEN_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${COEFFGEN_NAME}"
    FINALIZE_MT_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${FINALIZE_MT_NAME}"
    ENCODE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${ENCODE_NAME}"
    STORAGE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${STORAGE_NAME}"
    BILEVEL_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${BILEVEL_NAME}"
    BILEVEL_STITCH_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${BILEVEL_STITCH_NAME}"
    SOLVE_PROXIMITY_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${SOLVE_PROXIMITY_NAME}"
    RENDER_SM_ARN="arn:aws:states:${REGION}:${ACCT}:stateMachine:${RENDER_STATE_MACHINE_NAME}"
    COMPUTE_SM_ARN="arn:aws:states:${REGION}:${ACCT}:stateMachine:${COMPUTE_STATE_MACHINE_NAME}"
    PALETTE_PLAN_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${PALETTE_PLAN_NAME}"
    PALETTE_CHUNK_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${PALETTE_CHUNK_NAME}"
    PALETTE_FINALIZE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${PALETTE_FINALIZE_NAME}"
    ATTACH_PALETTE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${ATTACH_PALETTE_NAME}"
    PALETTE_SM_ARN="arn:aws:states:${REGION}:${ACCT}:stateMachine:${PALETTE_STATE_MACHINE_NAME}"

    SFN_ROLE_NAME="polypaint-sfn-execution-role"
    SFN_ROLE_ARN=$(aws iam get-role --role-name "$SFN_ROLE_NAME" --query 'Role.Arn' --output text 2>/dev/null || echo "")

    PREVIEW_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${RENDER_PREVIEW_NAME}"

    sed -e "s|\${PlanFunctionArn}|${RENDER_PLAN_ARN}|g" \
        -e "s|\${StatusFunctionArn}|${RENDER_STATUS_ARN}|g" \
        -e "s|\${FinalizeMTFunctionArn}|${FINALIZE_MT_ARN}|g" \
        -e "s|\${EncodeFunctionArn}|${ENCODE_ARN}|g" \
        -e "s|\${StorageFunctionArn}|${STORAGE_ARN}|g" \
        -e "s|\${BilevelFunctionArn}|${BILEVEL_ARN}|g" \
        -e "s|\${BilevelStitchFunctionArn}|${BILEVEL_STITCH_ARN}|g" \
        -e "s|\${SolveProximityFunctionArn}|${SOLVE_PROXIMITY_ARN}|g" \
        -e "s|\${PreviewFunctionArn}|${PREVIEW_ARN}|g" \
        stepfunctions/render_workflow.asl.json.template > /tmp/render_workflow.asl.json

    sed -e "s|\${PlanFunctionArn}|${COMPUTE_PLAN_ARN}|g" \
        -e "s|\${StatusFunctionArn}|${COMPUTE_STATUS_ARN}|g" \
        -e "s|\${CoeffgenFunctionArn}|${COEFFGEN_ARN}|g" \
        -e "s|\${FusedChunkFunctionArn}|${COMPUTE_FUSED_CHUNK_ARN}|g" \
        stepfunctions/compute_workflow.asl.json.template > /tmp/compute_workflow.asl.json

    sed -e "s|\${PlanFunctionArn}|${PALETTE_PLAN_ARN}|g" \
        -e "s|\${StatusFunctionArn}|${RENDER_STATUS_ARN}|g" \
        -e "s|\${SolveProximityFunctionArn}|${SOLVE_PROXIMITY_ARN}|g" \
        -e "s|\${PaletteChunkFunctionArn}|${PALETTE_CHUNK_ARN}|g" \
        -e "s|\${PaletteFinalizeFunctionArn}|${PALETTE_FINALIZE_ARN}|g" \
        -e "s|\${AttachPaletteFunctionArn}|${ATTACH_PALETTE_ARN}|g" \
        stepfunctions/palette_workflow.asl.json.template > /tmp/palette_workflow.asl.json

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

    aws stepfunctions update-state-machine \
        --state-machine-arn "$COMPUTE_SM_ARN" \
        --definition "$(cat /tmp/compute_workflow.asl.json)" \
        --role-arn "$SFN_ROLE_ARN" \
        --region "$REGION" >/dev/null 2>&1 || {
        echo "  Compute state machine doesn't exist, creating..."
        aws stepfunctions create-state-machine \
            --name "$COMPUTE_STATE_MACHINE_NAME" \
            --definition "$(cat /tmp/compute_workflow.asl.json)" \
            --role-arn "$SFN_ROLE_ARN" \
            --type STANDARD \
            --region "$REGION" \
            --query 'stateMachineArn' --output text
    }
    echo "  State machine: $COMPUTE_STATE_MACHINE_NAME"

    aws stepfunctions update-state-machine \
        --state-machine-arn "$PALETTE_SM_ARN" \
        --definition "$(cat /tmp/palette_workflow.asl.json)" \
        --role-arn "$SFN_ROLE_ARN" \
        --region "$REGION" >/dev/null 2>&1 || {
        echo "  Palette state machine doesn't exist, creating..."
        aws stepfunctions create-state-machine \
            --name "$PALETTE_STATE_MACHINE_NAME" \
            --definition "$(cat /tmp/palette_workflow.asl.json)" \
            --role-arn "$SFN_ROLE_ARN" \
            --type STANDARD \
            --region "$REGION" \
            --query 'stateMachineArn' --output text
    }
    echo "  State machine: $PALETTE_STATE_MACHINE_NAME"

    # Starter Lambda — uses state machine ARN, not worker names
    update_lambda "$RENDER_ORCHESTRATOR_NAME" "handler_render_orchestrator.handler" "/tmp/polypaint-render-orchestrator.zip" \
        "$RENDER_ORCHESTRATOR_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,RENDER_STATE_MACHINE_ARN=$RENDER_SM_ARN"
    update_lambda "$COMPUTE_ORCHESTRATOR_NAME" "handler_compute_orchestrator.handler" "/tmp/polypaint-compute-orchestrator.zip" \
        "$COMPUTE_ORCHESTRATOR_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,COMPUTE_STATE_MACHINE_ARN=$COMPUTE_SM_ARN"
    update_lambda "$PALETTE_ORCHESTRATOR_NAME" "handler_palette_orchestrator.handler" "/tmp/polypaint-palette-orchestrator.zip" \
        "$PALETTE_ORCHESTRATOR_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,PALETTE_STATE_MACHINE_ARN=$PALETTE_SM_ARN"

    # Ensure states:StartExecution permission
    aws iam put-role-policy --role-name "$ROLE_NAME" \
        --policy-name polypaint-sfn-start \
        --policy-document "{
            \"Version\": \"2012-10-17\",
            \"Statement\": [{
                \"Effect\": \"Allow\",
                \"Action\": \"states:StartExecution\",
                \"Resource\": [\"${RENDER_SM_ARN}\", \"${COMPUTE_SM_ARN}\", \"${PALETTE_SM_ARN}\"]
            }]
        }" 2>/dev/null || true

    update_lambda "$SWEEP_CM_NAME" "handler_sweep_cm.handler" "/tmp/polypaint-sweep-cm.zip" \
        "$SWEEP_CM_MEMORY" "$LAPACK_LAYER" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,LD_LIBRARY_PATH=/opt/lib" "$BINARY_TMP"

    # Async invoke config: no retries for most Lambdas (prevents retry storms),
    # but bilevel gets 2 retries / 1hr age to handle concurrency throttle drops.
    for fn in "$FINALIZE_MT_NAME" "$BILEVEL_STITCH_NAME" "$DZ_EXPORT_NAME" "$RENDER_PREVIEW_NAME" "$AUTOLEVELS_NAME" "$RESIZE_ARTIFACT_NAME" "$REPALETTE_NAME" "$PDF_ARTIFACT_NAME" "$SOLVE_PROXIMITY_NAME" "$PALETTE_CHUNK_NAME" "$PALETTE_FINALIZE_NAME" "$ATTACH_PALETTE_NAME"; do
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
    aws lambda put-function-event-invoke-config \
        --function-name "$PALETTE_ORCHESTRATOR_NAME" \
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
                \"Action\": [\"dynamodb:PutItem\", \"dynamodb:GetItem\", \"dynamodb:DeleteItem\", \"dynamodb:Query\", \"dynamodb:BatchWriteItem\"],
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

    # Upload frontend assets to S3
    while IFS= read -r asset; do
        echo "Uploading ${asset} to S3..."
        aws s3 cp "$SCRIPT_DIR/$asset" "s3://$BUCKET/$asset" \
            --content-type "$(frontend_asset_content_type "$asset")" --region "$REGION"
    done < <(frontend_asset_keys)
    verify_frontend_assets

    echo ""
    echo "=== UPDATED ==="
    echo "  Sweep:    $SWEEP_NAME ($SWEEP_MEMORY MB)"
    echo "  Coeffgen: $COEFFGEN_NAME ($COEFFGEN_MEMORY MB)"
    echo "  CmpFuse:  $COMPUTE_FUSED_CHUNK_NAME ($COMPUTE_FUSED_CHUNK_MEMORY MB)"
    echo "  FnlzMT:   $FINALIZE_MT_NAME ($FINALIZE_MT_MEMORY MB)"
    echo "  Encode:   $ENCODE_NAME ($ENCODE_MEMORY MB)"
    echo "  Viewport: $VIEWPORT_NAME ($VIEWPORT_MEMORY MB)"
    echo "  Storage:  $STORAGE_NAME ($STORAGE_MEMORY MB)"
    echo "  Dispatch: $DISPATCH_NAME ($DISPATCH_MEMORY MB)"
    echo "  Preview:  $PREVIEW_NAME ($PREVIEW_MEMORY MB)"
    echo "  SolvPrxB: $SOLVE_PROXIMITY_BENCH_NAME ($SOLVE_PROXIMITY_BENCH_MEMORY MB)"
    echo "  Build:    $BUILD_ID"
    echo "  Site:"
    echo "    HTTP:   http://$BUCKET.s3-website-$REGION.amazonaws.com"
    echo "    HTTPS:  https://$BUCKET.s3.$REGION.amazonaws.com/index.html"
else
    print_usage
    exit 1
fi
