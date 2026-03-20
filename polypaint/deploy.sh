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
VIEWPORT_MEMORY=512   # pure Python
STORAGE_MEMORY=512    # pure Python
DISPATCH_MEMORY=1769  # 1 vCPU — 50 threads doing SSL need real CPU
RASTER_MEMORY=1769    # 1 vCPU, roots2pix (no canvas allocation)
FINALIZE_MEMORY=1769  # 1 vCPU, pixassemble (64 MB tile buffer for 4096²)
COEFFGEN_MEMORY=1769  # 1 vCPU, coefficient generation (no solver, striped)
PREVIEW_MEMORY=1024   # pure Python, PNG encoding via zlib (512 OOMs on large lores)
BILEVEL_MEMORY=1769   # 1 vCPU, bilevel raster + merge
PARAM_DEBUG_NAME="polypaint-param-debug"
PARAM_DEBUG_MEMORY=1769  # 1 vCPU + libvips for TIFF output
BILEVEL_STITCH_NAME="polypaint-bilevel-stitch"
BILEVEL_STITCH_MEMORY=6144  # ~4 vCPUs, libvips multithreaded stitch
BINARY_TMP=10240      # /tmp size for Lambdas that process raw images (max 10GB)
TIMEOUT=900
BUCKET="polypaint"
JOBS_TABLE="polypaint-jobs"
LIBVIPS_LAYER="arn:aws:lambda:us-east-1:710848990594:layer:polypaint-libvips:6"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

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
        dnf install -y gcc glib2-devel 2>&1 | tail -1
        gcc -O3 -o /src/raw2jpeg /src/raw2jpeg.c \
            -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
            -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include \
            -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm \
            -Wl,-rpath,/opt/lib
        echo "  raw2jpeg compiled: $(file /src/raw2jpeg)"
        gcc -O3 -o /src/bilevel_merge /src/bilevel_merge.c \
            -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
            -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include \
            -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm \
            -Wl,-rpath,/opt/lib
        echo "  bilevel_merge compiled: $(file /src/bilevel_merge)"
    '

# --- Package 6 Lambdas ---
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
cp lambda/bilevel_merge "$PARAM_DEBUG_DIR/"
chmod +x "$PARAM_DEBUG_DIR"/bilevel_merge
cd "$PARAM_DEBUG_DIR" && zip -r9 /tmp/polypaint-param-debug.zip . -q && cd "$SCRIPT_DIR"
echo "  ParamDbg: $(du -h /tmp/polypaint-param-debug.zip | cut -f1)  (param debug + libvips layer)"

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
    for FNAME in "$SWEEP_NAME" "$COEFFGEN_NAME" "$ENCODE_NAME" "$VIEWPORT_NAME" "$STORAGE_NAME" "$DISPATCH_NAME" "$RASTER_NAME" "$FINALIZE_NAME" "$PREVIEW_NAME" "$BILEVEL_NAME" "$BILEVEL_STITCH_NAME" "$PARAM_DEBUG_NAME"; do
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
  "param-debug": "%s/param-debug"
}' "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" "$API_URL" \
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
        "$COEFFGEN_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE"

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
        "$DISPATCH_MEMORY" "$ROLE_ARN" "" "BUCKET=$BUCKET,RASTER_FUNCTION=$RASTER_NAME,FINALIZE_FUNCTION=$FINALIZE_NAME,ENCODE_FUNCTION=$ENCODE_NAME,SWEEP_FUNCTION=$SWEEP_NAME,BILEVEL_FUNCTION=$BILEVEL_NAME,BILEVEL_STITCH_FUNCTION=$BILEVEL_STITCH_NAME"
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

    # Disable async retries on fire-and-forget Lambdas (prevents retry storms)
    for fn in "$RASTER_NAME" "$FINALIZE_NAME" "$BILEVEL_NAME" "$BILEVEL_STITCH_NAME"; do
        aws lambda put-function-event-invoke-config \
            --function-name "$fn" \
            --maximum-retry-attempts 0 \
            --maximum-event-age-in-seconds 300 \
            --region "$REGION" >/dev/null 2>&1
    done

    # --- Set up API Gateway routes ---
    echo ""
    echo "Setting up API Gateway..."
    setup_api_gateway

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
        "$COEFFGEN_MEMORY" "" "BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE"

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
        "$DISPATCH_MEMORY" "" "BUCKET=$BUCKET,RASTER_FUNCTION=$RASTER_NAME,FINALIZE_FUNCTION=$FINALIZE_NAME,ENCODE_FUNCTION=$ENCODE_NAME,SWEEP_FUNCTION=$SWEEP_NAME,BILEVEL_FUNCTION=$BILEVEL_NAME,BILEVEL_STITCH_FUNCTION=$BILEVEL_STITCH_NAME"
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

    # Disable async retries on fire-and-forget Lambdas (prevents retry storms)
    for fn in "$RASTER_NAME" "$FINALIZE_NAME" "$BILEVEL_NAME" "$BILEVEL_STITCH_NAME"; do
        aws lambda put-function-event-invoke-config \
            --function-name "$fn" \
            --maximum-retry-attempts 0 \
            --maximum-event-age-in-seconds 300 \
            --region "$REGION" >/dev/null 2>&1
    done

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

    # Upload index.html to S3
    echo "Uploading index.html to S3..."
    aws s3 cp "$SCRIPT_DIR/index.html" "s3://$BUCKET/index.html" \
        --content-type "text/html" --region "$REGION"

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
