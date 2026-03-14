#!/bin/bash
# Deploy the polypaint Lambda function
# Usage: ./deploy.sh [create|update]

set -euo pipefail

FUNCTION_NAME="polypaint-solver"
ROLE_NAME="polypaint-lambda-role"
REGION="us-east-1"
RUNTIME="python3.12"
ARCH="arm64"
MEMORY=1024
TIMEOUT=900
API_ID="smojhi4gqe"
BUCKET="polypaint"
LIBVIPS_LAYER="arn:aws:lambda:us-east-1:710848990594:layer:polypaint-libvips:5"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

# --- Package Lambda ---
echo "Packaging Lambda..."
rm -rf /tmp/polypaint-deploy
mkdir -p /tmp/polypaint-deploy

# Cross-compile binaries for ARM64 Linux (Graviton)
echo "Compiling sweep..."
aarch64-linux-musl-gcc -O3 -static -o lambda/sweep lambda/sweep_cli.c -lm
echo "Compiling lores_viewport..."
aarch64-linux-musl-gcc -O3 -static -o lambda/lores_viewport lambda/lores_viewport.c -lm

echo "Compiling imgpipe (Docker ARM64, dynamically linked against libvips)..."
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
        gcc -O3 -o /src/imgpipe /src/imgpipe.c \
            -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
            -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include \
            -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm \
            -Wl,-rpath,/opt/lib
        echo "imgpipe compiled: $(file /src/imgpipe)"
    '

cp lambda/handler.py /tmp/polypaint-deploy/
cp lambda/sweep /tmp/polypaint-deploy/
cp lambda/lores_viewport /tmp/polypaint-deploy/
cp lambda/imgpipe /tmp/polypaint-deploy/
chmod +x /tmp/polypaint-deploy/sweep /tmp/polypaint-deploy/lores_viewport /tmp/polypaint-deploy/imgpipe
cd /tmp/polypaint-deploy

zip -r9 /tmp/polypaint-deploy.zip . -q
cd "$SCRIPT_DIR"
echo "Package size: $(du -h /tmp/polypaint-deploy.zip | cut -f1)"

ACTION="${1:-create}"

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

    # Inline policy for S3 access to polypaint bucket
    S3_POLICY="{
        \"Version\": \"2012-10-17\",
        \"Statement\": [{
            \"Effect\": \"Allow\",
            \"Action\": [\"s3:PutObject\", \"s3:GetObject\", \"s3:ListBucket\"],
            \"Resource\": [
                \"arn:aws:s3:::${BUCKET}\",
                \"arn:aws:s3:::${BUCKET}/*\"
            ]
        }]
    }"
    aws iam put-role-policy --role-name "$ROLE_NAME" \
        --policy-name polypaint-s3-access \
        --policy-document "$S3_POLICY"

    echo "Waiting for role to propagate..."
    sleep 10

    # --- Create Lambda function ---
    echo "Creating Lambda function..."
    aws lambda create-function \
        --function-name "$FUNCTION_NAME" \
        --runtime "$RUNTIME" \
        --architectures "$ARCH" \
        --handler handler.handler \
        --zip-file fileb:///tmp/polypaint-deploy.zip \
        --role "$ROLE_ARN" \
        --memory-size "$MEMORY" \
        --timeout "$TIMEOUT" \
        --region "$REGION" \
        --layers "$LIBVIPS_LAYER" \
        --environment "Variables={BUCKET=$BUCKET,LD_LIBRARY_PATH=/opt/lib}" \
        --query 'FunctionArn' --output text

    # --- Create Function URL (public, for experimentation) ---
    echo "Creating function URL..."
    aws lambda add-permission \
        --function-name "$FUNCTION_NAME" \
        --statement-id public-access \
        --action lambda:InvokeFunctionUrl \
        --principal "*" \
        --function-url-auth-type NONE \
        --region "$REGION" 2>/dev/null || true

    FUNC_URL=$(aws lambda create-function-url-config \
        --function-name "$FUNCTION_NAME" \
        --auth-type NONE \
        --cors '{"AllowOrigins":["*"],"AllowMethods":["POST","OPTIONS"],"AllowHeaders":["content-type"]}' \
        --region "$REGION" \
        --query 'FunctionUrl' --output text 2>/dev/null || \
        aws lambda get-function-url-config \
            --function-name "$FUNCTION_NAME" \
            --region "$REGION" \
            --query 'FunctionUrl' --output text)

    echo ""
    echo "=== DEPLOYED ==="
    echo "Function: $FUNCTION_NAME"
    echo "URL:      $FUNC_URL"
    echo ""
    echo "Test with:"
    echo "  curl -X POST '$FUNC_URL' -H 'Content-Type: application/json' -d '{\"coefficients\":[[1,0],[0,0],[-1,0]]}'"

elif [ "$ACTION" = "update" ]; then
    echo "Updating Lambda code..."
    aws lambda update-function-code \
        --function-name "$FUNCTION_NAME" \
        --zip-file fileb:///tmp/polypaint-deploy.zip \
        --region "$REGION" \
        --query 'FunctionArn' --output text

    FUNC_URL=$(aws lambda get-function-url-config \
        --function-name "$FUNCTION_NAME" \
        --region "$REGION" \
        --query 'FunctionUrl' --output text 2>/dev/null || echo "(no function URL)")

    # Ensure libvips layer is attached
    echo "Ensuring libvips layer..."
    aws lambda wait function-updated --function-name "$FUNCTION_NAME" --region "$REGION" 2>/dev/null || true
    aws lambda update-function-configuration \
        --function-name "$FUNCTION_NAME" \
        --layers "$LIBVIPS_LAYER" \
        --environment "Variables={BUCKET=$BUCKET,LD_LIBRARY_PATH=/opt/lib}" \
        --region "$REGION" \
        --query 'Layers[*].Arn' --output json

    # Upload index.html to S3
    echo "Uploading index.html to S3..."
    aws s3 cp "$SCRIPT_DIR/index.html" "s3://$BUCKET/index.html" \
        --content-type "text/html" --region "$REGION"

    echo ""
    echo "=== UPDATED ==="
    echo "URL: $FUNC_URL"
    echo "Site: http://$BUCKET.s3-website-$REGION.amazonaws.com"
else
    echo "Usage: $0 [create|update]"
    exit 1
fi
