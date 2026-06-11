#!/bin/bash
# Deploy the polypaint stack (Lambdas, Step Functions, API Gateway, frontend).
# Usage: ./deploy.sh [create|update|show-build]
#
# Phases, in execution order (create and update share every helper; the
# branches differ only in IAM/DynamoDB bootstrap):
#   1. Palette assets + JS syntax check + predeploy contract checks
#   2. Lambda layers: libvips, LAPACK, PDF python (built once, ARNs cached)
#   3. Catalog artifacts + parity overlay + fused-render JS execution test
#   4. Native binaries: static musl-gcc + Docker amazonlinux builds (with smoke tests)
#   5. Docker runtime regression test (tests/docker_runtime_regression.py)
#   6. Per-Lambda zip staging under /tmp/polypaint-*
#   7. deploy_all_lambdas: one create-or-update spec per function
#   8. Step Functions state machines, orchestrators, IAM policies
#   9. API Gateway routes, config.json, frontend upload + hash verification
#
# show-build deploys nothing: it diffs what is live against the local build.

set -euo pipefail
export AWS_PAGER=""

ROLE_NAME="polypaint-lambda-role"
REGION="us-east-1"
API_NAME="polypaint-api"
RUNTIME="python3.12"
ARCH="arm64"
RASTER_MT_THREADS=4   # default per-Lambda worker count for color raster MT
RENDER_STATE_MACHINE_NAME="polypaint-render-workflow"
COMPUTE_STATE_MACHINE_NAME="polypaint-compute-workflow"
PALETTE_STATE_MACHINE_NAME="polypaint-palette-workflow"
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

# --- Fleet data: names, memories, specs, routes (deploy_manifest.json) ---
# The manifest is the single source of truth for the Lambda fleet;
# deploy_manifest.py validates it and emits the bash sourced here
# (deploy_all_lambdas, deploy_orchestrator_lambdas, publish_api_routes,
# grant_api_gateway_invoke_permissions, delete_removed_lambdas).
DEPLOY_SPECS_SH=/tmp/polypaint-deploy-specs.sh
"${TEST_PYTHON[@]}" "$SCRIPT_DIR/deploy_manifest.py" --check
"${TEST_PYTHON[@]}" "$SCRIPT_DIR/deploy_manifest.py" --emit-bash > "$DEPLOY_SPECS_SH"
# shellcheck source=/dev/null
source "$DEPLOY_SPECS_SH"

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

    # SECURITY MODEL (deliberate, single-user art project): the whole bucket
    # is public-read and the HTTP API has no authorizer, including the
    # destructive storage routes. Anyone with the URL can read artifacts and
    # delete prefixes. Acceptable here; revisit before sharing the stack.
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
        done < <(frontend_asset_keys) | shasum -a 256 | cut -d' ' -f1
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
        LOCAL_HASH=$(shasum -a 256 "$SCRIPT_DIR/${asset}" | cut -d' ' -f1)
        REMOTE_HASH=$(shasum -a 256 "${TMP_DIR}/${asset}" | cut -d' ' -f1)
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
       lambda/color_render_contract.py \
       lambda/solve_score_chain.py \
       lambda/palette_names.py lambda/tri_palette_names_generated.py lambda/long_palette_names_generated.py "$PLAN_DIR/"
    cd "$PLAN_DIR" && zip -FS -r9 "$ZIP_PATH" . -q && cd "$SCRIPT_DIR"
}

render_compute_workflow_definition() {
    local OUT_PATH="$1"
    local ACCT="$2"
    local PLAN_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${COMPUTE_PLAN_NAME}"
    local STATUS_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${COMPUTE_STATUS_NAME}"
    local COEFFGEN_FN_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${COEFFGEN_NAME}"
    local FUSED_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${COMPUTE_FUSED_CHUNK_NAME}"
    sed -e "s|\${PlanFunctionArn}|${PLAN_ARN}|g" \
        -e "s|\${StatusFunctionArn}|${STATUS_ARN}|g" \
        -e "s|\${CoeffgenFunctionArn}|${COEFFGEN_FN_ARN}|g" \
        -e "s|\${FusedChunkFunctionArn}|${FUSED_ARN}|g" \
        stepfunctions/compute_workflow.asl.json.template > "$OUT_PATH"
}

render_palette_workflow_definition() {
    local OUT_PATH="$1"
    local ACCT="$2"
    local PLAN_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${PALETTE_PLAN_NAME}"
    local STATUS_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${RENDER_STATUS_NAME}"
    local PROX_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${SOLVE_PROXIMITY_NAME}"
    local CHUNK_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${PALETTE_CHUNK_NAME}"
    local FINALIZE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${PALETTE_FINALIZE_NAME}"
    local ATTACH_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${ATTACH_PALETTE_NAME}"
    sed -e "s|\${PlanFunctionArn}|${PLAN_ARN}|g" \
        -e "s|\${StatusFunctionArn}|${STATUS_ARN}|g" \
        -e "s|\${SolveProximityFunctionArn}|${PROX_ARN}|g" \
        -e "s|\${PaletteChunkFunctionArn}|${CHUNK_ARN}|g" \
        -e "s|\${PaletteFinalizeFunctionArn}|${FINALIZE_ARN}|g" \
        -e "s|\${AttachPaletteFunctionArn}|${ATTACH_ARN}|g" \
        stepfunctions/palette_workflow.asl.json.template > "$OUT_PATH"
}

# Async invoke config, shared by the create and update paths. Must-succeed:
# stdout is silenced but stderr stays visible and set -e aborts on failure
# (the old >/dev/null 2>&1 form died silently on throttles).
configure_async_invoke_policies() {
    # No retries for most Lambdas (prevents retry storms), but bilevel gets
    # 2 retries / 1hr age to handle concurrency throttle drops.
    local fn
    for fn in "$FINALIZE_MT_NAME" "$DZ_EXPORT_NAME" "$RENDER_PREVIEW_NAME" "$AUTOLEVELS_NAME" "$RESIZE_ARTIFACT_NAME" "$REPALETTE_NAME" "$PDF_ARTIFACT_NAME" "$SOLVE_PROXIMITY_NAME" "$PALETTE_CHUNK_NAME" "$PALETTE_FINALIZE_NAME" "$ATTACH_PALETTE_NAME"; do
        aws lambda put-function-event-invoke-config \
            --function-name "$fn" \
            --maximum-retry-attempts 0 \
            --maximum-event-age-in-seconds 300 \
            --region "$REGION" >/dev/null
    done
    aws lambda put-function-event-invoke-config \
        --function-name "$BILEVEL_NAME" \
        --maximum-retry-attempts 2 \
        --maximum-event-age-in-seconds 3600 \
        --region "$REGION" >/dev/null
    # Orchestrators: no retries (self-reinvoke), long event age.
    for fn in "$RENDER_ORCHESTRATOR_NAME" "$PALETTE_ORCHESTRATOR_NAME"; do
        aws lambda put-function-event-invoke-config \
            --function-name "$fn" \
            --maximum-retry-attempts 0 \
            --maximum-event-age-in-seconds 3600 \
            --region "$REGION" >/dev/null
    done
}

render_render_workflow_definition() {
    local OUT_PATH="$1"
    local ACCT="$2"
    local RENDER_PLAN_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${RENDER_PLAN_NAME}"
    local RENDER_STATUS_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${RENDER_STATUS_NAME}"
    local FINALIZE_MT_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${FINALIZE_MT_NAME}"
    local STORAGE_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${STORAGE_NAME}"
    local BILEVEL_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${BILEVEL_NAME}"
    local SOLVE_PROXIMITY_ARN="arn:aws:lambda:${REGION}:${ACCT}:function:${SOLVE_PROXIMITY_NAME}"
    "${TEST_PYTHON[@]}" "$SCRIPT_DIR/workflow_template_render.py" render-workflow \
        --out "$OUT_PATH" \
        --plan-function-arn "$RENDER_PLAN_ARN" \
        --status-function-arn "$RENDER_STATUS_ARN" \
        --finalize-mt-function-arn "$FINALIZE_MT_ARN" \
        --storage-function-arn "$STORAGE_ARN" \
        --bilevel-function-arn "$BILEVEL_ARN" \
        --solve-proximity-function-arn "$SOLVE_PROXIMITY_ARN"
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
"${TEST_PYTHON[@]}" "$SCRIPT_DIR/scripts/generate_tri_palettes.py" || { echo "FATAL: tri-palette generation failed"; exit 1; }
"${TEST_PYTHON[@]}" "$SCRIPT_DIR/scripts/generate_long_palettes.py" || { echo "FATAL: long-palette generation failed"; exit 1; }

# --- JS syntax check ---
echo "Checking index.html JS syntax..."
SCRIPT_BLOCKS=$(grep -c '<script>' index.html || true)
if [ "$SCRIPT_BLOCKS" != "1" ]; then
    echo "ERROR: expected exactly one <script> block in index.html, found $SCRIPT_BLOCKS; the JS gate extraction below would corrupt."
    exit 1
fi
sed -n '/<script>/,/<\/script>/p' index.html | sed '1d;$d' > /tmp/_jscheck.js
if ! node --check /tmp/_jscheck.js; then
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

    # Skip rebuild if zip exists and build script hasn't changed. The cache
    # key is the build-script hash only (deliberate): layers pick up new
    # upstream package versions only when the script text changes.
    if [ -f "$ZIP_PATH" ] && [ -f "$HASH_FILE" ] && [ "$(cat "$HASH_FILE")" = "$SCRIPT_HASH" ]; then
        echo "  $LAYER_NAME: zip up to date (skipping rebuild)"
    else
        echo "  $LAYER_NAME: building..."
        if ! bash "$BUILD_SCRIPT"; then
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
(cd lambda && "${TEST_PYTHON[@]}" -c "
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
(cd lambda && "${TEST_PYTHON[@]}" -c "
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

# Helper: compile one dynamically-linked ARM64 binary against system libcurl
# inside Amazon Linux 2023 (the Lambda runtime image), then stage its non-base
# shared libraries into lambda/<name>_lib/ — packaged as lib/ next to the
# binary and found via -rpath $ORIGIN/lib (LD_LIBRARY_PATH=/var/task/lib).
# Args: binary name, then any extra .c inputs (e.g. multispan_reader.c).
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
                -lcurl -lm -Wl,-rpath,\$ORIGIN/lib
            rm -rf "/src/${BIN}_lib"
            mkdir -p "/src/${BIN}_lib"
            for lib in $(ldd "/src/$BIN" | awk "/=> \// {print \$3}"); do
                base=$(basename "$lib")
                case "$base" in
                    libc.so.*|libm.so.*|libpthread.so.*|ld-linux-aarch64.so.*|libdl.so.*|librt.so.*)
                        continue
                        ;;
                esac
                cp -L "$lib" "/src/${BIN}_lib/"
            done
            echo "  $BIN compiled: $(file "/src/$BIN")"
        '
}

build_libcurl_binary roots2pix_mt multispan_reader.c

build_libcurl_binary assemble_greyscale

echo "  coeffs_bilevel_raster (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/coeffs_bilevel_raster lambda/coeffs_bilevel_raster.c -lm

echo "  bilevel_section_raster (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/bilevel_section_raster lambda/bilevel_section_raster.c -lm

echo "  solve_proximity_stats (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -pthread -o lambda/solve_proximity_stats lambda/solve_proximity_stats.c -lm
build_libcurl_binary solve_proximity_hist_sectioned multispan_reader.c
echo "  solve_palette_debug (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/solve_palette_debug lambda/solve_palette_debug.c -lm
echo "  solve_palette_chunk (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/solve_palette_chunk lambda/solve_palette_chunk.c -lm
build_libcurl_binary solve_palette_chunk_mt multispan_reader.c
echo "  palette_bins_render (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/palette_bins_render lambda/palette_bins_render.c -lm

echo "  step_scores_to_palette_raw (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/step_scores_to_palette_raw lambda/step_scores_to_palette_raw.c -lm

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
        # Shared libvips build flags; every gcc below uses the same set.
        VIPS_CFLAGS="-I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include"
        VIPS_LIBS="-L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm -Wl,-rpath,/opt/lib"
        VIPS_TIFF_LIBS="-L/opt/lib -lvips -ltiff -lgobject-2.0 -lglib-2.0 -lm -Wl,-rpath,/opt/lib"
        gcc -O3 -o /src/raw2jpeg /src/raw2jpeg.c $VIPS_CFLAGS $VIPS_LIBS
        echo "  raw2jpeg compiled: $(file /src/raw2jpeg)"
        gcc -O3 -o /src/score_raw_render /src/score_raw_render.c $VIPS_CFLAGS $VIPS_LIBS
        echo "  score_raw_render compiled: $(file /src/score_raw_render)"
        gcc -O3 -o /src/bilevel_merge /src/bilevel_merge.c $VIPS_CFLAGS $VIPS_TIFF_LIBS
        echo "  bilevel_merge compiled: $(file /src/bilevel_merge)"
        gcc -O3 -o /src/raw_to_bilevel /src/raw_to_bilevel.c $VIPS_CFLAGS $VIPS_TIFF_LIBS
        echo "  raw_to_bilevel compiled: $(file /src/raw_to_bilevel)"
        gcc -O3 -o /src/tiff_compat /src/tiff_compat.c $VIPS_CFLAGS $VIPS_TIFF_LIBS
        echo "  tiff_compat compiled: $(file /src/tiff_compat)"
        gcc -O3 -o /src/png_export /src/png_export.c $VIPS_CFLAGS $VIPS_LIBS
        echo "  png_export compiled: $(file /src/png_export)"
        gcc -O3 -o /src/dz_export /src/dz_export.c $VIPS_CFLAGS $VIPS_LIBS
        echo "  dz_export compiled: $(file /src/dz_export)"
        gcc -O3 -o /src/autolevels_render /src/autolevels_render.c $VIPS_CFLAGS $VIPS_LIBS
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
        gcc -O2 -o /tmp/probe_ops /tmp/probe_ops.c $VIPS_CFLAGS $VIPS_LIBS
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
        gcc -O2 -o /tmp/make_test_tif /tmp/make_test_tif.c $VIPS_CFLAGS $VIPS_LIBS
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

# Sweep-MT: handler_sweep_mt.py + shared.py + sweep_mt
SWEEP_MT_DIR=/tmp/polypaint-sweep-mt
rm -rf "$SWEEP_MT_DIR"
mkdir -p "$SWEEP_MT_DIR"
cp lambda/handler_sweep_mt.py lambda/shared.py "$SWEEP_MT_DIR/"
cp lambda/sweep_mt "$SWEEP_MT_DIR/"
chmod +x "$SWEEP_MT_DIR"/sweep_mt
cd "$SWEEP_MT_DIR" && zip -FS -r9 /tmp/polypaint-sweep-mt.zip . -q && cd "$SCRIPT_DIR"
echo "  SweepMT:  $(du -h /tmp/polypaint-sweep-mt.zip | cut -f1)  (sweep_mt)"

# Coeffgen: handler_coeffgen.py + shared.py + compute_fused.py + sweep_coeffgen (needs LAPACK layer)
COEFFGEN_DIR=/tmp/polypaint-coeffgen
rm -rf "$COEFFGEN_DIR"
mkdir -p "$COEFFGEN_DIR"
cp lambda/handler_coeffgen.py lambda/shared.py lambda/compute_fused.py \
   lambda/param_program_chain.py lambda/param_legacy_registry.json \
   lambda/coeff_program_chain.py lambda/coeff_program_source.py lambda/coeff_legacy_registry.json "$COEFFGEN_DIR/"
cp lambda/sweep_coeffgen "$COEFFGEN_DIR/"
chmod +x "$COEFFGEN_DIR"/sweep_coeffgen
cd "$COEFFGEN_DIR" && zip -FS -r9 /tmp/polypaint-coeffgen.zip . -q && cd "$SCRIPT_DIR"
echo "  Coeffgen: $(du -h /tmp/polypaint-coeffgen.zip | cut -f1)  (sweep_coeffgen + LAPACK layer)"

# Viewport: handler_viewport.py + shared.py (pure Python)
VIEWPORT_DIR=/tmp/polypaint-viewport
rm -rf "$VIEWPORT_DIR"
mkdir -p "$VIEWPORT_DIR"
cp lambda/handler_viewport.py lambda/shared.py "$VIEWPORT_DIR/"
cd "$VIEWPORT_DIR" && zip -FS -r9 /tmp/polypaint-viewport.zip . -q && cd "$SCRIPT_DIR"
echo "  Viewport: $(du -h /tmp/polypaint-viewport.zip | cut -f1)  (pure Python)"

# Storage: handler_storage.py + shared.py + color artifact metadata + logical section helpers (pure Python)
STORAGE_DIR=/tmp/polypaint-storage
rm -rf "$STORAGE_DIR"
mkdir -p "$STORAGE_DIR"
cp lambda/handler_storage.py lambda/shared.py lambda/color_artifact_meta.py lambda/solve_score_chain.py \
   lambda/param_program_chain.py lambda/param_legacy_registry.json \
   lambda/coeff_program_chain.py lambda/coeff_program_source.py lambda/coeff_legacy_registry.json \
   lambda/color_render_contract.py lambda/logical_sections.py "$STORAGE_DIR/"
cd "$STORAGE_DIR" && zip -FS -r9 /tmp/polypaint-storage.zip . -q && cd "$SCRIPT_DIR"
echo "  Storage:  $(du -h /tmp/polypaint-storage.zip | cut -f1)  (pure Python)"

# Dispatch: handler_dispatch.py + shared.py (pure Python, invokes render Lambdas)
DISPATCH_DIR=/tmp/polypaint-dispatch
rm -rf "$DISPATCH_DIR"
mkdir -p "$DISPATCH_DIR"
cp lambda/handler_dispatch.py lambda/shared.py "$DISPATCH_DIR/"
cd "$DISPATCH_DIR" && zip -FS -r9 /tmp/polypaint-dispatch.zip . -q && cd "$SCRIPT_DIR"
echo "  Dispatch: $(du -h /tmp/polypaint-dispatch.zip | cut -f1)  (pure Python)"

# Raster-MT: handler_raster_mt.py + shared.py + solve_score_chain.py + roots2pix_mt
RASTER_MT_DIR=/tmp/polypaint-raster-mt
rm -rf "$RASTER_MT_DIR"
mkdir -p "$RASTER_MT_DIR/lib"
cp lambda/handler_raster_mt.py lambda/shared.py lambda/solve_score_chain.py lambda/logical_sections.py "$RASTER_MT_DIR/"
cp lambda/roots2pix_mt "$RASTER_MT_DIR/"
cp lambda/roots2pix_mt_lib/* "$RASTER_MT_DIR/lib/"
chmod +x "$RASTER_MT_DIR"/roots2pix_mt
cd "$RASTER_MT_DIR" && zip -FS -r9 /tmp/polypaint-raster-mt.zip . -q && cd "$SCRIPT_DIR"
echo "  RastMT:   $(du -h /tmp/polypaint-raster-mt.zip | cut -f1)  (fused roots2pix_mt)"

# Finalize-MT: fused solve-score assemble + encode from raw score bins
FINALIZE_MT_DIR=/tmp/polypaint-finalize-mt
rm -rf "$FINALIZE_MT_DIR"
mkdir -p "$FINALIZE_MT_DIR/lib"
cp lambda/handler_finalize_mt.py lambda/shared.py lambda/color_artifact_meta.py lambda/color_render_contract.py lambda/solve_score_chain.py lambda/raw_sidecar.py lambda/raw_score_render.py "$FINALIZE_MT_DIR/"
cp lambda/assemble_greyscale lambda/score_raw_render "$FINALIZE_MT_DIR/"
cp lambda/assemble_greyscale_lib/* "$FINALIZE_MT_DIR/lib/"
chmod +x "$FINALIZE_MT_DIR"/assemble_greyscale "$FINALIZE_MT_DIR"/score_raw_render
cd "$FINALIZE_MT_DIR" && zip -FS -r9 /tmp/polypaint-finalize-mt.zip . -q && cd "$SCRIPT_DIR"
echo "  FnlzMT:   $(du -h /tmp/polypaint-finalize-mt.zip | cut -f1)  (fused assemble + encode)"

# Preview: handler_preview.py + shared.py (pure Python, PNG via zlib)
PREVIEW_DIR=/tmp/polypaint-preview
rm -rf "$PREVIEW_DIR"
mkdir -p "$PREVIEW_DIR"
cp lambda/handler_preview.py lambda/shared.py "$PREVIEW_DIR/"
cd "$PREVIEW_DIR" && zip -FS -r9 /tmp/polypaint-preview.zip . -q && cd "$SCRIPT_DIR"
echo "  Preview:  $(du -h /tmp/polypaint-preview.zip | cut -f1)  (pure Python)"

# Compute Preview: handler_compute_preview.py + shared.py + coeffgen/solve binaries
COMPUTE_PREVIEW_DIR=/tmp/polypaint-compute-preview
rm -rf "$COMPUTE_PREVIEW_DIR"
mkdir -p "$COMPUTE_PREVIEW_DIR"
cp lambda/handler_compute_preview.py lambda/shared.py \
   lambda/param_program_chain.py lambda/param_legacy_registry.json \
   lambda/coeff_program_chain.py lambda/coeff_program_source.py lambda/coeff_legacy_registry.json \
   "$COMPUTE_PREVIEW_DIR/"
cp lambda/sweep_coeffgen lambda/sweep_mt lambda/sweep_cm "$COMPUTE_PREVIEW_DIR/"
chmod +x "$COMPUTE_PREVIEW_DIR"/sweep_coeffgen "$COMPUTE_PREVIEW_DIR"/sweep_mt "$COMPUTE_PREVIEW_DIR"/sweep_cm
cd "$COMPUTE_PREVIEW_DIR" && zip -FS -r9 /tmp/polypaint-compute-preview.zip . -q && cd "$SCRIPT_DIR"
echo "  CPreview: $(du -h /tmp/polypaint-compute-preview.zip | cut -f1)  (sync coeffgen+solve preview)"

# Bilevel: handler_bilevel.py + shared.py + sparse section/finalize helpers (needs libvips layer)
BILEVEL_DIR=/tmp/polypaint-bilevel
rm -rf "$BILEVEL_DIR"
mkdir -p "$BILEVEL_DIR/lib"
cp lambda/handler_bilevel.py lambda/shared.py lambda/logical_sections.py lambda/raw_sidecar.py lambda/color_render_contract.py lambda/color_artifact_meta.py lambda/solve_score_chain.py "$BILEVEL_DIR/"
cp lambda/bilevel_section_raster lambda/coeffs_bilevel_raster lambda/raw_to_bilevel lambda/assemble_greyscale "$BILEVEL_DIR/"
cp lambda/assemble_greyscale_lib/* "$BILEVEL_DIR/lib/"
chmod +x "$BILEVEL_DIR"/bilevel_section_raster "$BILEVEL_DIR"/coeffs_bilevel_raster "$BILEVEL_DIR"/raw_to_bilevel "$BILEVEL_DIR"/assemble_greyscale
cd "$BILEVEL_DIR" && zip -FS -r9 /tmp/polypaint-bilevel.zip . -q && cd "$SCRIPT_DIR"
echo "  Bilevel:  $(du -h /tmp/polypaint-bilevel.zip | cut -f1)  (bilevel sparse fragments + finalize)"

# Param Debug: handler_param_debug.py + shared.py + bilevel_merge (needs libvips layer)
PARAM_DEBUG_DIR=/tmp/polypaint-param-debug
rm -rf "$PARAM_DEBUG_DIR"
mkdir -p "$PARAM_DEBUG_DIR"
cp lambda/handler_param_debug.py lambda/shared.py \
   lambda/param_program_chain.py lambda/param_legacy_registry.json \
   "$PARAM_DEBUG_DIR/"
cp lambda/sweep lambda/bilevel_merge "$PARAM_DEBUG_DIR/"
chmod +x "$PARAM_DEBUG_DIR"/sweep "$PARAM_DEBUG_DIR"/bilevel_merge
cd "$PARAM_DEBUG_DIR" && zip -FS -r9 /tmp/polypaint-param-debug.zip . -q && cd "$SCRIPT_DIR"
echo "  ParamDbg: $(du -h /tmp/polypaint-param-debug.zip | cut -f1)  (param debug + libvips layer)"

# TIFF Compat: handler_tiff_compat.py + shared.py + tiff_compat (needs libtiff from layer)
TIFF_COMPAT_DIR=/tmp/polypaint-tiff-compat
rm -rf "$TIFF_COMPAT_DIR"
mkdir -p "$TIFF_COMPAT_DIR"
cp lambda/handler_tiff_compat.py lambda/shared.py "$TIFF_COMPAT_DIR/"
cp lambda/tiff_compat "$TIFF_COMPAT_DIR/"
chmod +x "$TIFF_COMPAT_DIR"/tiff_compat
cd "$TIFF_COMPAT_DIR" && zip -FS -r9 /tmp/polypaint-tiff-compat.zip . -q && cd "$SCRIPT_DIR"
echo "  TiffCmp: $(du -h /tmp/polypaint-tiff-compat.zip | cut -f1)  (tiff_compat + libtiff layer)"

# PNG Export: handler_png_export.py + shared.py + png_export (needs libvips layer)
PNG_EXPORT_DIR=/tmp/polypaint-png-export
rm -rf "$PNG_EXPORT_DIR"
mkdir -p "$PNG_EXPORT_DIR"
cp lambda/handler_png_export.py lambda/shared.py "$PNG_EXPORT_DIR/"
cp lambda/png_export "$PNG_EXPORT_DIR/"
chmod +x "$PNG_EXPORT_DIR"/png_export
cd "$PNG_EXPORT_DIR" && zip -FS -r9 /tmp/polypaint-png-export.zip . -q && cd "$SCRIPT_DIR"
echo "  PngExp:  $(du -h /tmp/polypaint-png-export.zip | cut -f1)  (png_export + libvips layer)"

# Render Preview: handler_render_preview.py + shared.py (needs libvips layer for vipsthumbnail)
RENDER_PREVIEW_DIR=/tmp/polypaint-render-preview
rm -rf "$RENDER_PREVIEW_DIR"
mkdir -p "$RENDER_PREVIEW_DIR"
cp lambda/handler_render_preview.py lambda/shared.py "$RENDER_PREVIEW_DIR/"
cd "$RENDER_PREVIEW_DIR" && zip -FS -r9 /tmp/polypaint-render-preview.zip . -q && cd "$SCRIPT_DIR"
echo "  RndPrev: $(du -h /tmp/polypaint-render-preview.zip | cut -f1)  (vipsthumbnail via libvips layer)"

# Render Lores Preview: ephemeral lores fused raster + palette encode (needs libcurl + libvips layers)
RENDER_LORES_PREVIEW_DIR=/tmp/polypaint-render-lores-preview
rm -rf "$RENDER_LORES_PREVIEW_DIR"
mkdir -p "$RENDER_LORES_PREVIEW_DIR/lib"
cp lambda/handler_render_lores_preview.py lambda/shared.py \
   lambda/logical_sections.py lambda/logical_lores.py lambda/calc_chunks.py lambda/param_source.py \
   lambda/color_render_contract.py lambda/solve_score_chain.py lambda/raw_score_render.py "$RENDER_LORES_PREVIEW_DIR/"
cp lambda/roots2pix_mt lambda/solve_proximity_stats lambda/score_raw_render \
   lambda/sweep_coeffgen lambda/sweep_mt lambda/sweep_cm "$RENDER_LORES_PREVIEW_DIR/"
cp lambda/roots2pix_mt_lib/* "$RENDER_LORES_PREVIEW_DIR/lib/"
chmod +x "$RENDER_LORES_PREVIEW_DIR"/roots2pix_mt "$RENDER_LORES_PREVIEW_DIR"/solve_proximity_stats "$RENDER_LORES_PREVIEW_DIR"/score_raw_render \
    "$RENDER_LORES_PREVIEW_DIR"/sweep_coeffgen "$RENDER_LORES_PREVIEW_DIR"/sweep_mt "$RENDER_LORES_PREVIEW_DIR"/sweep_cm
cd "$RENDER_LORES_PREVIEW_DIR" && zip -FS -r9 /tmp/polypaint-render-lores-preview.zip . -q && cd "$SCRIPT_DIR"
echo "  LoresPv: $(du -h /tmp/polypaint-render-lores-preview.zip | cut -f1)  (ephemeral lores render preview)"

# Autolevels: handler_autolevels.py + shared.py + color_artifact_meta.py + autolevels_render (needs libvips layer)
AUTOLEVELS_DIR=/tmp/polypaint-autolevels
rm -rf "$AUTOLEVELS_DIR"
mkdir -p "$AUTOLEVELS_DIR"
cp lambda/handler_autolevels.py lambda/shared.py lambda/color_artifact_meta.py lambda/solve_score_chain.py "$AUTOLEVELS_DIR/"
cp lambda/autolevels_render "$AUTOLEVELS_DIR/"
chmod +x "$AUTOLEVELS_DIR"/autolevels_render
cd "$AUTOLEVELS_DIR" && zip -FS -r9 /tmp/polypaint-autolevels.zip . -q && cd "$SCRIPT_DIR"
echo "  AutoLvl: $(du -h /tmp/polypaint-autolevels.zip | cut -f1)  (autolevels_render + libvips layer)"

# Resize Artifact: handler_resize_artifact.py + shared.py + color_artifact_meta.py (needs libvips layer)
RESIZE_ARTIFACT_DIR=/tmp/polypaint-resize-artifact
rm -rf "$RESIZE_ARTIFACT_DIR"
mkdir -p "$RESIZE_ARTIFACT_DIR"
cp lambda/handler_resize_artifact.py lambda/shared.py lambda/color_artifact_meta.py lambda/solve_score_chain.py "$RESIZE_ARTIFACT_DIR/"
cd "$RESIZE_ARTIFACT_DIR" && zip -FS -r9 /tmp/polypaint-resize-artifact.zip . -q && cd "$SCRIPT_DIR"
echo "  Resize:  $(du -h /tmp/polypaint-resize-artifact.zip | cut -f1)  (resize artifact + libvips layer)"

# RePalette: handler_repalette.py + shared.py + palette helpers + palette_bins_render + raw2jpeg (needs libvips layer)
REPALETTE_DIR=/tmp/polypaint-repalette
rm -rf "$REPALETTE_DIR"
mkdir -p "$REPALETTE_DIR"
cp lambda/handler_repalette.py lambda/shared.py \
   lambda/palette_names.py lambda/tri_palette_names_generated.py lambda/long_palette_names_generated.py "$REPALETTE_DIR/"
cp lambda/palette_bins_render lambda/raw2jpeg "$REPALETTE_DIR/"
chmod +x "$REPALETTE_DIR"/palette_bins_render "$REPALETTE_DIR"/raw2jpeg
cd "$REPALETTE_DIR" && zip -FS -r9 /tmp/polypaint-repalette.zip . -q && cd "$SCRIPT_DIR"
echo "  RePal:   $(du -h /tmp/polypaint-repalette.zip | cut -f1)  (repalette + libvips layer)"

# Color RePalette: handler_color_repalette.py + shared raw-sidecar recolor helpers
COLOR_REPALETTE_DIR=/tmp/polypaint-color-repalette
rm -rf "$COLOR_REPALETTE_DIR"
mkdir -p "$COLOR_REPALETTE_DIR"
cp lambda/handler_color_repalette.py lambda/shared.py lambda/raw_sidecar.py lambda/raw_score_render.py \
   lambda/color_render_contract.py lambda/color_artifact_meta.py lambda/solve_score_chain.py lambda/color_recolor_raw.py \
   lambda/palette_names.py lambda/tri_palette_names_generated.py lambda/long_palette_names_generated.py "$COLOR_REPALETTE_DIR/"
cp lambda/score_raw_render "$COLOR_REPALETTE_DIR/"
chmod +x "$COLOR_REPALETTE_DIR"/score_raw_render
cd "$COLOR_REPALETTE_DIR" && zip -FS -r9 /tmp/polypaint-color-repalette.zip . -q && cd "$SCRIPT_DIR"
echo "  ClrRePal: $(du -h /tmp/polypaint-color-repalette.zip | cut -f1)  (raw-sidecar score_raw_render)"

# Recolor-from-raw: standalone raw-sidecar recolor path
RECOLOR_FROM_RAW_DIR=/tmp/polypaint-recolor-from-raw
rm -rf "$RECOLOR_FROM_RAW_DIR"
mkdir -p "$RECOLOR_FROM_RAW_DIR"
cp lambda/handler_recolor_from_raw.py lambda/shared.py lambda/raw_sidecar.py lambda/raw_score_render.py lambda/color_recolor_raw.py \
   lambda/color_render_contract.py lambda/color_artifact_meta.py lambda/solve_score_chain.py \
   lambda/palette_names.py lambda/tri_palette_names_generated.py lambda/long_palette_names_generated.py "$RECOLOR_FROM_RAW_DIR/"
cp lambda/score_raw_render "$RECOLOR_FROM_RAW_DIR/"
chmod +x "$RECOLOR_FROM_RAW_DIR"/score_raw_render
cd "$RECOLOR_FROM_RAW_DIR" && zip -FS -r9 /tmp/polypaint-recolor-from-raw.zip . -q && cd "$SCRIPT_DIR"
echo "  RecolorRaw: $(du -h /tmp/polypaint-recolor-from-raw.zip | cut -f1)  (standalone score_raw_render)"

# ExtractPalette-fused: standalone step_scores.raw -> associated palette path
EXTRACT_PALETTE_FUSED_DIR=/tmp/polypaint-extract-palette-fused
rm -rf "$EXTRACT_PALETTE_FUSED_DIR"
mkdir -p "$EXTRACT_PALETTE_FUSED_DIR"
cp lambda/handler_extract_palette_from_step_scores.py lambda/shared.py lambda/raw_sidecar.py lambda/raw_score_render.py \
   lambda/color_render_contract.py lambda/color_artifact_meta.py lambda/solve_score_chain.py "$EXTRACT_PALETTE_FUSED_DIR/"
cp lambda/score_raw_render lambda/step_scores_to_palette_raw "$EXTRACT_PALETTE_FUSED_DIR/"
chmod +x "$EXTRACT_PALETTE_FUSED_DIR"/score_raw_render "$EXTRACT_PALETTE_FUSED_DIR"/step_scores_to_palette_raw
cd "$EXTRACT_PALETTE_FUSED_DIR" && zip -FS -r9 /tmp/polypaint-extract-palette-fused.zip . -q && cd "$SCRIPT_DIR"
echo "  ExtPalFx: $(du -h /tmp/polypaint-extract-palette-fused.zip | cut -f1)  (step_scores_to_palette_raw + score_raw_render)"

# PDF Artifact: handler_pdf_artifact.py + shared.py + color artifact metadata + spread builder
PDF_ARTIFACT_DIR=/tmp/polypaint-pdf-artifact
rm -rf "$PDF_ARTIFACT_DIR"
mkdir -p "$PDF_ARTIFACT_DIR"
cp lambda/handler_pdf_artifact.py lambda/shared.py lambda/color_artifact_meta.py lambda/solve_score_chain.py lambda/spread_pdf.py "$PDF_ARTIFACT_DIR/"
cd "$PDF_ARTIFACT_DIR" && zip -FS -r9 /tmp/polypaint-pdf-artifact.zip . -q && cd "$SCRIPT_DIR"
echo "  PDFArt:  $(du -h /tmp/polypaint-pdf-artifact.zip | cut -f1)  (spread builder + python pdf layer)"

# DeepZoom Export: handler_deepzoom_export.py + shared.py + source metadata helpers + dz_export (needs libvips layer)
DZ_EXPORT_DIR=/tmp/polypaint-deepzoom-export
rm -rf "$DZ_EXPORT_DIR"
mkdir -p "$DZ_EXPORT_DIR"
cp lambda/handler_deepzoom_export.py lambda/shared.py lambda/raw_sidecar.py \
   lambda/color_render_contract.py lambda/color_artifact_meta.py lambda/solve_score_chain.py \
   lambda/deepzoom_viewer_template.html "$DZ_EXPORT_DIR/"
cp lambda/dz_export "$DZ_EXPORT_DIR/"
chmod +x "$DZ_EXPORT_DIR"/dz_export
cd "$DZ_EXPORT_DIR" && zip -FS -r9 /tmp/polypaint-deepzoom-export.zip . -q && cd "$SCRIPT_DIR"
echo "  DzExp:   $(du -h /tmp/polypaint-deepzoom-export.zip | cut -f1)  (dz_export + libvips layer)"

# DeepZoom-from-raw: strict wrapper over the raw-sidecar DeepZoom path
DZ_FROM_RAW_DIR=/tmp/polypaint-deepzoom-from-raw
rm -rf "$DZ_FROM_RAW_DIR"
mkdir -p "$DZ_FROM_RAW_DIR"
cp lambda/handler_deepzoom_from_raw.py lambda/handler_deepzoom_export.py lambda/shared.py lambda/raw_sidecar.py \
   lambda/color_render_contract.py lambda/color_artifact_meta.py lambda/solve_score_chain.py \
   lambda/deepzoom_viewer_template.html "$DZ_FROM_RAW_DIR/"
cp lambda/dz_export "$DZ_FROM_RAW_DIR/"
chmod +x "$DZ_FROM_RAW_DIR"/dz_export
cd "$DZ_FROM_RAW_DIR" && zip -FS -r9 /tmp/polypaint-deepzoom-from-raw.zip . -q && cd "$SCRIPT_DIR"
echo "  DzRaw:   $(du -h /tmp/polypaint-deepzoom-from-raw.zip | cut -f1)  (raw-sidecar-only deepzoom)"

# Solve Proximity: handler_solve_proximity.py + shared.py + solve_score_chain.py + logical section helpers + solve_proximity_stats binary
SOLVE_PROXIMITY_DIR=/tmp/polypaint-solve-proximity
rm -rf "$SOLVE_PROXIMITY_DIR"
mkdir -p "$SOLVE_PROXIMITY_DIR/lib"
cp lambda/handler_solve_proximity.py lambda/shared.py lambda/solve_score_chain.py \
   lambda/logical_sections.py "$SOLVE_PROXIMITY_DIR/"
cp lambda/solve_proximity_stats lambda/solve_proximity_hist_sectioned "$SOLVE_PROXIMITY_DIR/"
cp lambda/solve_proximity_hist_sectioned_lib/* "$SOLVE_PROXIMITY_DIR/lib/"
chmod +x "$SOLVE_PROXIMITY_DIR"/solve_proximity_stats "$SOLVE_PROXIMITY_DIR"/solve_proximity_hist_sectioned
cd "$SOLVE_PROXIMITY_DIR" && zip -FS -r9 /tmp/polypaint-solve-proximity.zip . -q && cd "$SCRIPT_DIR"
echo "  SolvPrx: $(du -h /tmp/polypaint-solve-proximity.zip | cut -f1)  (solve_proximity_stats + sectioned hist)"

# Solve Proximity Bench: benchmark handler + solve proximity helpers + logical section helpers + solve_proximity_stats binary
SOLVE_PROXIMITY_BENCH_DIR=/tmp/polypaint-solve-proximity-bench
rm -rf "$SOLVE_PROXIMITY_BENCH_DIR"
mkdir -p "$SOLVE_PROXIMITY_BENCH_DIR/lib"
cp lambda/handler_solve_proximity_bench.py lambda/handler_solve_proximity.py lambda/shared.py \
   lambda/solve_score_chain.py lambda/logical_sections.py "$SOLVE_PROXIMITY_BENCH_DIR/"
cp lambda/solve_proximity_stats lambda/solve_proximity_hist_sectioned "$SOLVE_PROXIMITY_BENCH_DIR/"
cp lambda/solve_proximity_hist_sectioned_lib/* "$SOLVE_PROXIMITY_BENCH_DIR/lib/"
chmod +x "$SOLVE_PROXIMITY_BENCH_DIR"/solve_proximity_stats "$SOLVE_PROXIMITY_BENCH_DIR"/solve_proximity_hist_sectioned
cd "$SOLVE_PROXIMITY_BENCH_DIR" && zip -FS -r9 /tmp/polypaint-solve-proximity-bench.zip . -q && cd "$SCRIPT_DIR"
echo "  SolvPrxB: $(du -h /tmp/polypaint-solve-proximity-bench.zip | cut -f1)  (AWS hist benchmark)"

# Palette Debug: handler_palette_debug.py + shared.py + solve_palette_debug + raw2jpeg (needs libvips layer)
PALETTE_DEBUG_DIR=/tmp/polypaint-palette-debug
rm -rf "$PALETTE_DEBUG_DIR"
mkdir -p "$PALETTE_DEBUG_DIR"
cp lambda/handler_palette_debug.py lambda/shared.py \
   lambda/palette_names.py lambda/tri_palette_names_generated.py lambda/long_palette_names_generated.py "$PALETTE_DEBUG_DIR/"
cp lambda/solve_palette_debug lambda/raw2jpeg "$PALETTE_DEBUG_DIR/"
chmod +x "$PALETTE_DEBUG_DIR"/solve_palette_debug "$PALETTE_DEBUG_DIR"/raw2jpeg
cd "$PALETTE_DEBUG_DIR" && zip -FS -r9 /tmp/polypaint-palette-debug.zip . -q && cd "$SCRIPT_DIR"
echo "  PalDbg:  $(du -h /tmp/polypaint-palette-debug.zip | cut -f1)  (palette debug)"

# Palette Orchestrator: handler_palette_orchestrator.py + shared.py
PALETTE_ORCHESTRATOR_DIR=/tmp/polypaint-palette-orchestrator
rm -rf "$PALETTE_ORCHESTRATOR_DIR"
mkdir -p "$PALETTE_ORCHESTRATOR_DIR"
cp lambda/handler_palette_orchestrator.py lambda/shared.py "$PALETTE_ORCHESTRATOR_DIR/"
cd "$PALETTE_ORCHESTRATOR_DIR" && zip -FS -r9 /tmp/polypaint-palette-orchestrator.zip . -q && cd "$SCRIPT_DIR"
echo "  PalOrch: $(du -h /tmp/polypaint-palette-orchestrator.zip | cut -f1)  (starter only)"

# Palette Render Plan: handler_palette_render_plan.py + shared.py + chunk helpers
PALETTE_PLAN_DIR=/tmp/polypaint-palette-render-plan
rm -rf "$PALETTE_PLAN_DIR"
mkdir -p "$PALETTE_PLAN_DIR"
cp lambda/handler_palette_render_plan.py lambda/shared.py \
   lambda/calc_chunks.py \
   lambda/param_source.py \
   lambda/color_artifact_meta.py \
   lambda/solve_score_chain.py \
   lambda/logical_sections.py \
   lambda/palette_names.py lambda/tri_palette_names_generated.py lambda/long_palette_names_generated.py "$PALETTE_PLAN_DIR/"
cd "$PALETTE_PLAN_DIR" && zip -FS -r9 /tmp/polypaint-palette-render-plan.zip . -q && cd "$SCRIPT_DIR"
echo "  PalPlan: $(du -h /tmp/polypaint-palette-render-plan.zip | cut -f1)  (plan builder)"

# Palette Chunk: handler_palette_chunk.py + shared.py + solve_score_chain.py + logical section helpers + solve_palette_chunk + solve_palette_chunk_mt
PALETTE_CHUNK_DIR=/tmp/polypaint-palette-chunk
rm -rf "$PALETTE_CHUNK_DIR"
mkdir -p "$PALETTE_CHUNK_DIR/lib"
cp lambda/handler_palette_chunk.py lambda/shared.py lambda/solve_score_chain.py \
   lambda/logical_sections.py "$PALETTE_CHUNK_DIR/"
cp lambda/solve_palette_chunk lambda/solve_palette_chunk_mt "$PALETTE_CHUNK_DIR/"
cp lambda/solve_palette_chunk_mt_lib/* "$PALETTE_CHUNK_DIR/lib/"
chmod +x "$PALETTE_CHUNK_DIR"/solve_palette_chunk "$PALETTE_CHUNK_DIR"/solve_palette_chunk_mt
cd "$PALETTE_CHUNK_DIR" && zip -FS -r9 /tmp/polypaint-palette-chunk.zip . -q && cd "$SCRIPT_DIR"
echo "  PalChnk: $(du -h /tmp/polypaint-palette-chunk.zip | cut -f1)  (chunk scorer)"

# Palette Finalize: handler_palette_finalize.py + shared.py + source metadata helpers + palette_bins_render + raw2jpeg (needs libvips layer)
PALETTE_FINALIZE_DIR=/tmp/polypaint-palette-finalize
rm -rf "$PALETTE_FINALIZE_DIR"
mkdir -p "$PALETTE_FINALIZE_DIR"
cp lambda/handler_palette_finalize.py lambda/shared.py lambda/color_artifact_meta.py lambda/solve_score_chain.py "$PALETTE_FINALIZE_DIR/"
cp lambda/palette_bins_render lambda/raw2jpeg "$PALETTE_FINALIZE_DIR/"
chmod +x "$PALETTE_FINALIZE_DIR"/palette_bins_render "$PALETTE_FINALIZE_DIR"/raw2jpeg
cd "$PALETTE_FINALIZE_DIR" && zip -FS -r9 /tmp/polypaint-palette-finalize.zip . -q && cd "$SCRIPT_DIR"
echo "  PalFin:  $(du -h /tmp/polypaint-palette-finalize.zip | cut -f1)  (finalize + libvips layer)"

# Attach Palette to Color: handler_attach_palette_to_color.py + shared.py + color_artifact_meta.py
ATTACH_PALETTE_DIR=/tmp/polypaint-attach-palette-to-color
rm -rf "$ATTACH_PALETTE_DIR"
mkdir -p "$ATTACH_PALETTE_DIR"
cp lambda/handler_attach_palette_to_color.py lambda/shared.py lambda/color_artifact_meta.py lambda/solve_score_chain.py "$ATTACH_PALETTE_DIR/"
cd "$ATTACH_PALETTE_DIR" && zip -FS -r9 /tmp/polypaint-attach-palette-to-color.zip . -q && cd "$SCRIPT_DIR"
echo "  PalAtt:  $(du -h /tmp/polypaint-attach-palette-to-color.zip | cut -f1)  (attach associated palette metadata)"

# Render Orchestrator (starter): handler_render_orchestrator.py + shared.py
RENDER_ORCHESTRATOR_DIR=/tmp/polypaint-render-orchestrator
rm -rf "$RENDER_ORCHESTRATOR_DIR"
mkdir -p "$RENDER_ORCHESTRATOR_DIR"
cp lambda/handler_render_orchestrator.py lambda/shared.py "$RENDER_ORCHESTRATOR_DIR/"
cd "$RENDER_ORCHESTRATOR_DIR" && zip -FS -r9 /tmp/polypaint-render-orchestrator.zip . -q && cd "$SCRIPT_DIR"
echo "  RndOrch: $(du -h /tmp/polypaint-render-orchestrator.zip | cut -f1)  (starter only)"

# Render Plan: same file list as show-build, via the shared helper
package_render_plan_zip /tmp/polypaint-render-plan.zip /tmp/polypaint-render-plan
echo "  RndPlan: $(du -h /tmp/polypaint-render-plan.zip | cut -f1)  (plan builder)"

# Render Status: handler_render_status.py + shared.py
RENDER_STATUS_DIR=/tmp/polypaint-render-status
rm -rf "$RENDER_STATUS_DIR"
mkdir -p "$RENDER_STATUS_DIR"
cp lambda/handler_render_status.py lambda/shared.py "$RENDER_STATUS_DIR/"
cd "$RENDER_STATUS_DIR" && zip -FS -r9 /tmp/polypaint-render-status.zip . -q && cd "$SCRIPT_DIR"
echo "  RndStat: $(du -h /tmp/polypaint-render-status.zip | cut -f1)  (status updater)"

# Compute Orchestrator (starter): handler_compute_orchestrator.py + shared.py + compute_fused.py
COMPUTE_ORCHESTRATOR_DIR=/tmp/polypaint-compute-orchestrator
rm -rf "$COMPUTE_ORCHESTRATOR_DIR"
mkdir -p "$COMPUTE_ORCHESTRATOR_DIR"
cp lambda/handler_compute_orchestrator.py lambda/shared.py lambda/compute_fused.py "$COMPUTE_ORCHESTRATOR_DIR/"
cd "$COMPUTE_ORCHESTRATOR_DIR" && zip -FS -r9 /tmp/polypaint-compute-orchestrator.zip . -q && cd "$SCRIPT_DIR"
echo "  CmpOrch: $(du -h /tmp/polypaint-compute-orchestrator.zip | cut -f1)  (starter only)"

# Compute Plan: handler_compute_plan.py + shared.py + compute_fused.py
COMPUTE_PLAN_DIR=/tmp/polypaint-compute-plan
rm -rf "$COMPUTE_PLAN_DIR"
mkdir -p "$COMPUTE_PLAN_DIR"
cp lambda/handler_compute_plan.py lambda/shared.py lambda/compute_fused.py \
   lambda/param_program_chain.py lambda/param_legacy_registry.json \
   lambda/coeff_program_chain.py lambda/coeff_program_source.py lambda/coeff_legacy_registry.json "$COMPUTE_PLAN_DIR/"
cd "$COMPUTE_PLAN_DIR" && zip -FS -r9 /tmp/polypaint-compute-plan.zip . -q && cd "$SCRIPT_DIR"
echo "  CmpPlan: $(du -h /tmp/polypaint-compute-plan.zip | cut -f1)  (plan + finalize)"

# Compute Fused Chunk: handler_compute_chunk_fused.py + shared.py + native compute binaries
COMPUTE_FUSED_CHUNK_DIR=/tmp/polypaint-compute-fused-chunk
rm -rf "$COMPUTE_FUSED_CHUNK_DIR"
mkdir -p "$COMPUTE_FUSED_CHUNK_DIR"
cp lambda/handler_compute_chunk_fused.py lambda/shared.py "$COMPUTE_FUSED_CHUNK_DIR/"
cp lambda/sweep_coeffgen lambda/sweep_mt lambda/sweep_cm "$COMPUTE_FUSED_CHUNK_DIR/"
chmod +x "$COMPUTE_FUSED_CHUNK_DIR"/sweep_coeffgen "$COMPUTE_FUSED_CHUNK_DIR"/sweep_mt "$COMPUTE_FUSED_CHUNK_DIR"/sweep_cm
cd "$COMPUTE_FUSED_CHUNK_DIR" && zip -FS -r9 /tmp/polypaint-compute-fused-chunk.zip . -q && cd "$SCRIPT_DIR"
echo "  CmpFuse: $(du -h /tmp/polypaint-compute-fused-chunk.zip | cut -f1)  (fused chunk worker)"

# Compute Status: handler_compute_status.py + shared.py
COMPUTE_STATUS_DIR=/tmp/polypaint-compute-status
rm -rf "$COMPUTE_STATUS_DIR"
mkdir -p "$COMPUTE_STATUS_DIR"
cp lambda/handler_compute_status.py lambda/shared.py "$COMPUTE_STATUS_DIR/"
cd "$COMPUTE_STATUS_DIR" && zip -FS -r9 /tmp/polypaint-compute-status.zip . -q && cd "$SCRIPT_DIR"
echo "  CmpStat: $(du -h /tmp/polypaint-compute-status.zip | cut -f1)  (status updater)"

# Sweep-CM: handler_sweep_cm.py + shared.py + sweep_cm (needs LAPACK layer)
SWEEP_CM_DIR=/tmp/polypaint-sweep-cm
rm -rf "$SWEEP_CM_DIR"
mkdir -p "$SWEEP_CM_DIR"
cp lambda/handler_sweep_cm.py lambda/shared.py "$SWEEP_CM_DIR/"
cp lambda/sweep_cm "$SWEEP_CM_DIR/"
chmod +x "$SWEEP_CM_DIR"/sweep_cm
cd "$SWEEP_CM_DIR" && zip -FS -r9 /tmp/polypaint-sweep-cm.zip . -q && cd "$SCRIPT_DIR"
echo "  SweepCM: $(du -h /tmp/polypaint-sweep-cm.zip | cut -f1)  (sweep_cm + LAPACK layer)"

# Helper: create a Lambda function
create_lambda() {
    local NAME="$1" HANDLER="$2" ZIP="$3" MEM="$4" ROLE="$5" LAYERS="${6:-}" ENV_VARS="$7" TMP="${8:-512}"
    if aws lambda get-function --function-name "$NAME" --region "$REGION" >/dev/null 2>&1; then
        # Re-running create against an existing stack: converge via update
        # instead of dying on ResourceConflictException.
        echo "  $NAME already exists; updating instead..."
        update_lambda "$NAME" "$HANDLER" "$ZIP" "$MEM" "$LAYERS" "$ENV_VARS" "$TMP"
        return
    fi
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
    local UPDATE_OUT
    if ! UPDATE_OUT=$(aws lambda update-function-code \
        --function-name "$NAME" \
        --zip-file "fileb://$ZIP" \
        --region "$REGION" \
        --query 'FunctionArn' --output text 2>&1); then
        # Only a genuinely missing function falls through to create; any other
        # failure (oversized zip, in-progress update, throttle) aborts with
        # the real error instead of a confusing "already exists" from create.
        if ! grep -q "ResourceNotFoundException" <<< "$UPDATE_OUT"; then
            echo "FATAL: update-function-code failed for $NAME:" >&2
            echo "$UPDATE_OUT" >&2
            return 1
        fi
        echo "  $NAME doesn't exist yet, creating..."
        ROLE_ARN=$(aws iam get-role --role-name "$ROLE_NAME" --query 'Role.Arn' --output text)
        create_lambda "$NAME" "$HANDLER" "$ZIP" "$MEM" "$ROLE_ARN" "$LAYERS" "$ENV_VARS" "$TMP"
        return
    fi
    echo "$UPDATE_OUT"

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


# Helper: create or update one Lambda depending on $ACTION. Every per-function
# spec goes through this wrapper, so the create and update paths share one
# spec list and cannot drift. (create_lambda takes ROLE_ARN as its fifth
# argument and update_lambda does not; the wrapper hides that difference.)
deploy_lambda() {
    local NAME="$1" HANDLER="$2" ZIP="$3" MEM="$4" LAYERS="${5:-}" ENV_VARS="$6" TMP="${7:-512}"
    if [ "$ACTION" = "create" ]; then
        create_lambda "$NAME" "$HANDLER" "$ZIP" "$MEM" "$ROLE_ARN" "$LAYERS" "$ENV_VARS" "$TMP"
    else
        update_lambda "$NAME" "$HANDLER" "$ZIP" "$MEM" "$LAYERS" "$ENV_VARS" "$TMP"
    fi
}

# Get-or-create the Step Functions execution role and converge its
# Lambda-invoke policy. Sets SFN_ROLE_ARN for ensure_state_machine.
ensure_sfn_execution_role() {
    local ACCT="$1"
    SFN_ROLE_NAME="polypaint-sfn-execution-role"
    local SFN_TRUST='"'"'{
        "Version": "2012-10-17",
        "Statement": [{
            "Effect": "Allow",
            "Principal": {"Service": "states.amazonaws.com"},
            "Action": "sts:AssumeRole"
        }]
    }'"'"'
    local CREATED=0
    SFN_ROLE_ARN=$(aws iam get-role --role-name "$SFN_ROLE_NAME" --query 'Role.Arn' --output text 2>/dev/null || echo "")
    if [ -z "$SFN_ROLE_ARN" ]; then
        echo "  Creating SFN execution role..."
        SFN_ROLE_ARN=$(aws iam create-role \
            --role-name "$SFN_ROLE_NAME" \
            --assume-role-policy-document "$SFN_TRUST" \
            --query 'Role.Arn' --output text)
        CREATED=1
    fi
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
    if [ "$CREATED" = "1" ]; then
        sleep 5  # let the new role propagate before Step Functions validates it
    fi
}

# Update the state machine definition, creating the machine if it doesn't
# exist yet. Command output is deliberately not captured: update returns an
# updateDate (not an ARN), so callers construct the ARN from the name.
ensure_state_machine() {
    local SM_NAME="$1" DEF_FILE="$2"
    if ! aws stepfunctions update-state-machine \
        --state-machine-arn "arn:aws:states:${REGION}:${ACCT}:stateMachine:${SM_NAME}" \
        --definition "file://$DEF_FILE" \
        --role-arn "$SFN_ROLE_ARN" \
        --region "$REGION" >/dev/null 2>&1; then
        echo "  State machine $SM_NAME doesn't exist, creating..."
        aws stepfunctions create-state-machine \
            --name "$SM_NAME" \
            --definition "file://$DEF_FILE" \
            --role-arn "$SFN_ROLE_ARN" \
            --type STANDARD \
            --region "$REGION" \
            --query 'stateMachineArn' --output text >/dev/null
    fi
    echo "  State machine: $SM_NAME"
}

# Allow the Lambda execution role to start the three workflows.
grant_sfn_start_policy() {
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
}

# Helper: delete a Lambda that has been removed from active deployment.
delete_lambda_if_exists() {
    local NAME="$1"
    if aws lambda get-function --function-name "$NAME" --region "$REGION" >/dev/null 2>&1; then
        echo "Deleting removed Lambda $NAME..."
        if ! aws lambda delete-function --function-name "$NAME" --region "$REGION" >/dev/null 2>&1; then
            echo "  Warning: failed to delete removed Lambda $NAME; continuing"
        fi
    fi
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
            --region "$REGION" >/dev/null
        echo "  Created API Gateway: $API_ID"
    else
        echo "  Using existing API Gateway: $API_ID"
    fi

    is_api_gateway_id() {
        local VALUE="${1:-}"
        [ -n "$VALUE" ] && [ "$VALUE" != "None" ] && [ "$VALUE" != "null" ]
    }

    first_api_gateway_id() {
        local ITEM
        for ITEM in $1; do
            if is_api_gateway_id "$ITEM"; then
                echo "$ITEM"
                return 0
            fi
        done
        return 1
    }

    # Find or create integration for a Lambda and return its ID
    create_integration() {
        local FNAME="$1"
        local TARGET_URI="arn:aws:lambda:$REGION:$ACCT:function:$FNAME"
        # Check if an integration already exists for this Lambda
        local EXISTING_RAW EXISTING
        EXISTING_RAW=$(aws apigatewayv2 get-integrations --api-id "$API_ID" --region "$REGION" \
            --query "Items[?IntegrationUri=='${TARGET_URI}'].IntegrationId" --output text 2>/dev/null)
        EXISTING=$(first_api_gateway_id "$EXISTING_RAW" || true)
        if is_api_gateway_id "$EXISTING"; then
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
        local EXISTING_RAW EXISTING
        EXISTING_RAW=$(aws apigatewayv2 get-routes --api-id "$API_ID" --region "$REGION" \
            --query "Items[?RouteKey=='${ROUTE_KEY}'].RouteId" --output text 2>/dev/null)
        EXISTING=$(first_api_gateway_id "$EXISTING_RAW" || true)
        if is_api_gateway_id "$EXISTING"; then
            aws apigatewayv2 update-route --api-id "$API_ID" --route-id "$EXISTING" \
                --target "integrations/$INT_ID" --region "$REGION" >/dev/null
        else
            aws apigatewayv2 create-route --api-id "$API_ID" \
                --route-key "$ROUTE_KEY" --target "integrations/$INT_ID" \
                --region "$REGION" >/dev/null
        fi
    }

    # Delete a removed route if it exists on an already-deployed API.
    delete_route_if_exists() {
        local ROUTE_KEY="$1"
        local EXISTING_IDS ROUTE_ID
        EXISTING_IDS=$(aws apigatewayv2 get-routes --api-id "$API_ID" --region "$REGION" \
            --query "Items[?RouteKey=='${ROUTE_KEY}'].RouteId" --output text 2>/dev/null)
        for ROUTE_ID in $EXISTING_IDS; do
            if is_api_gateway_id "$ROUTE_ID"; then
                if ! aws apigatewayv2 delete-route --api-id "$API_ID" --route-id "$ROUTE_ID" \
                    --region "$REGION" >/dev/null 2>&1; then
                    echo "  Warning: failed to delete removed route $ROUTE_KEY (route id $ROUTE_ID); continuing"
                fi
            fi
        done
    }

    # Delete integrations for Lambdas that are no longer routed.
    delete_integration_for_lambda_if_exists() {
        local FNAME="$1"
        local TARGET_URI="arn:aws:lambda:$REGION:$ACCT:function:$FNAME"
        local EXISTING_IDS
        EXISTING_IDS=$(aws apigatewayv2 get-integrations --api-id "$API_ID" --region "$REGION" \
            --query "Items[?IntegrationUri=='${TARGET_URI}'].IntegrationId" --output text 2>/dev/null)
        local INT_ID
        for INT_ID in $EXISTING_IDS; do
            if is_api_gateway_id "$INT_ID"; then
                if ! aws apigatewayv2 delete-integration --api-id "$API_ID" --integration-id "$INT_ID" \
                    --region "$REGION" >/dev/null 2>&1; then
                    echo "  Warning: failed to delete removed integration $INT_ID for $FNAME; continuing"
                fi
            fi
        done
    }

    grant_api_gateway_invoke_permissions

    # Integrations + routes come from deploy_manifest.json
    echo "  Creating integrations and routes..."
    publish_api_routes

    # Get API URL and write config.json. The "%s" placeholders are parsed by
    # api_manifest.py (_extract_deploy_config_services); keep the printf form.
    local API_URL
    API_URL=$(aws apigatewayv2 get-api --api-id "$API_ID" --region "$REGION" \
        --query 'ApiEndpoint' --output text)
    echo "  API Gateway URL: $API_URL"

    build_deploy_metadata

    printf '{
  "sweep-mt": "%s/sweep-mt",
  "coeffgen": "%s/coeffgen",
  "viewport": "%s/viewport",
  "preview": "%s/preview",
  "compute-preview": "%s/compute-preview",
  "render-lores-preview": "%s/render-lores-preview",
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
}' "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$BUILD_ID" "$BUILD_DEPLOYED_AT_UTC" "$BUILD_GIT_REV" "$BUILD_GIT_DIRTY" "$BUILD_FRONTEND_SHA256" \
    | aws s3 cp - "s3://$BUCKET/config.json" \
        --content-type "application/json" \
        --cache-control "no-cache" --region "$REGION"
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

    delete_removed_lambdas

    # --- Lambdas (single spec list shared with the update path) ---
    deploy_all_lambdas

    # Step Functions state machines
    echo "Deploying Step Functions state machines..."
    ACCT=$(aws sts get-caller-identity --query 'Account' --output text)
    ensure_sfn_execution_role "$ACCT"

    render_render_workflow_definition /tmp/render_workflow.asl.json "$ACCT"
    render_compute_workflow_definition /tmp/compute_workflow.asl.json "$ACCT"
    render_palette_workflow_definition /tmp/palette_workflow.asl.json "$ACCT"

    RENDER_SM_ARN="arn:aws:states:${REGION}:${ACCT}:stateMachine:${RENDER_STATE_MACHINE_NAME}"
    COMPUTE_SM_ARN="arn:aws:states:${REGION}:${ACCT}:stateMachine:${COMPUTE_STATE_MACHINE_NAME}"
    PALETTE_SM_ARN="arn:aws:states:${REGION}:${ACCT}:stateMachine:${PALETTE_STATE_MACHINE_NAME}"
    ensure_state_machine "$RENDER_STATE_MACHINE_NAME" /tmp/render_workflow.asl.json
    ensure_state_machine "$COMPUTE_STATE_MACHINE_NAME" /tmp/compute_workflow.asl.json
    ensure_state_machine "$PALETTE_STATE_MACHINE_NAME" /tmp/palette_workflow.asl.json

    deploy_orchestrator_lambdas

    grant_sfn_start_policy

    configure_async_invoke_policies

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
            --content-type "$(frontend_asset_content_type "$asset")" \
            --cache-control "no-cache" --region "$REGION"
    done < <(frontend_asset_keys)
    verify_frontend_assets

    echo ""
    echo "=== DEPLOYED ==="
    echo "  Coeffgen: $COEFFGEN_NAME ($COEFFGEN_MEMORY MB)"
    echo "  CmpFuse:  $COMPUTE_FUSED_CHUNK_NAME ($COMPUTE_FUSED_CHUNK_MEMORY MB)"
    echo "  FnlzMT:   $FINALIZE_MT_NAME ($FINALIZE_MT_MEMORY MB)"
    echo "  Viewport: $VIEWPORT_NAME ($VIEWPORT_MEMORY MB)"
    echo "  Storage:  $STORAGE_NAME ($STORAGE_MEMORY MB)"
    echo "  Dispatch: $DISPATCH_NAME ($DISPATCH_MEMORY MB)"
    echo "  Preview:  $PREVIEW_NAME ($PREVIEW_MEMORY MB)"
    echo "  SolvPrxB: $SOLVE_PROXIMITY_BENCH_NAME ($SOLVE_PROXIMITY_BENCH_MEMORY MB)"
    echo "  Bilevel:  $BILEVEL_NAME ($BILEVEL_MEMORY MB)"
    echo "  C2B:      $COLOR_TO_BILEVEL_NAME ($COLOR_TO_BILEVEL_MEMORY MB)"
elif [ "$ACTION" = "update" ]; then
    delete_removed_lambdas

    # --- Lambdas (single spec list shared with the create path) ---
    deploy_all_lambdas

    # Step Functions state machines
    echo "Updating Step Functions state machines..."
    ACCT=$(aws sts get-caller-identity --query 'Account' --output text)
    ensure_sfn_execution_role "$ACCT"

    render_render_workflow_definition /tmp/render_workflow.asl.json "$ACCT"
    render_compute_workflow_definition /tmp/compute_workflow.asl.json "$ACCT"
    render_palette_workflow_definition /tmp/palette_workflow.asl.json "$ACCT"

    RENDER_SM_ARN="arn:aws:states:${REGION}:${ACCT}:stateMachine:${RENDER_STATE_MACHINE_NAME}"
    COMPUTE_SM_ARN="arn:aws:states:${REGION}:${ACCT}:stateMachine:${COMPUTE_STATE_MACHINE_NAME}"
    PALETTE_SM_ARN="arn:aws:states:${REGION}:${ACCT}:stateMachine:${PALETTE_STATE_MACHINE_NAME}"
    ensure_state_machine "$RENDER_STATE_MACHINE_NAME" /tmp/render_workflow.asl.json
    ensure_state_machine "$COMPUTE_STATE_MACHINE_NAME" /tmp/compute_workflow.asl.json
    ensure_state_machine "$PALETTE_STATE_MACHINE_NAME" /tmp/palette_workflow.asl.json

    deploy_orchestrator_lambdas

    grant_sfn_start_policy 2>/dev/null || true

    configure_async_invoke_policies

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
            --content-type "$(frontend_asset_content_type "$asset")" \
            --cache-control "no-cache" --region "$REGION"
    done < <(frontend_asset_keys)
    verify_frontend_assets

    echo ""
    echo "=== UPDATED ==="
    echo "  Coeffgen: $COEFFGEN_NAME ($COEFFGEN_MEMORY MB)"
    echo "  CmpFuse:  $COMPUTE_FUSED_CHUNK_NAME ($COMPUTE_FUSED_CHUNK_MEMORY MB)"
    echo "  FnlzMT:   $FINALIZE_MT_NAME ($FINALIZE_MT_MEMORY MB)"
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
