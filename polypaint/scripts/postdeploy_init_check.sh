#!/bin/bash
# Post-deploy INIT sweep: invoke every deployed Lambda once and flag any
# function that cannot even start (Runtime.ImportModuleError and friends).
#
# Why: import-time failures (env-var library contamination, packaging gaps,
# layer drift, runtime updates) return HTTP 500 on every call but are
# invisible until someone hits the feature — the 2026-06 outage shipped
# import-dead render/palette/bilevel Lambdas that nothing exercised until a
# preview click. A handler-level error (bad payload) is a PASS here: the
# probe payload is deliberately meaningless; we only care that the module
# imported and the handler ran.
#
# Usage: bash scripts/postdeploy_init_check.sh   (after ./deploy.sh update)
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REGION="us-east-1"
PAYLOAD='{"postdeploy_init_check": true}'

if command -v uv >/dev/null 2>&1; then
    TEST_PYTHON=(uv run python)
elif [ -x "$ROOT/.venv/bin/python" ]; then
    TEST_PYTHON=("$ROOT/.venv/bin/python")
elif [ -x "$ROOT/../.venv/bin/python" ]; then
    TEST_PYTHON=("$ROOT/../.venv/bin/python")
else
    TEST_PYTHON=(python3)
fi

FUNCTIONS=$("${TEST_PYTHON[@]}" - "$ROOT/deploy_manifest.json" <<'PY'
import json, sys
manifest = json.load(open(sys.argv[1]))
for fn in manifest["functions"]:
    print(fn["name"])
PY
)

echo "Post-deploy INIT sweep over $(echo "$FUNCTIONS" | wc -l | tr -d ' ') functions..."
FAILED=0
OUT=/tmp/polypaint-init-check-payload.json
for NAME in $FUNCTIONS; do
    ERR=$(aws lambda invoke --function-name "$NAME" \
        --payload "$PAYLOAD" --cli-binary-format raw-in-base64-out \
        --region "$REGION" --query 'FunctionError' --output text "$OUT" 2>&1) || {
        echo "  FAIL  $NAME: invoke error: $ERR"
        FAILED=1
        continue
    }
    if [ "$ERR" = "None" ] || [ -z "$ERR" ]; then
        echo "  ok    $NAME"
        continue
    fi
    ERROR_TYPE=$("${TEST_PYTHON[@]}" - "$OUT" <<'PY'
import json, sys
try:
    print(json.load(open(sys.argv[1])).get("errorType", "unknown"))
except Exception:
    print("unparseable")
PY
    )
    case "$ERROR_TYPE" in
        Runtime.ImportModuleError|Runtime.HandlerNotFound|Runtime.UserCodeSyntaxError)
            echo "  FAIL  $NAME: $ERROR_TYPE (function cannot start)"
            FAILED=1
            ;;
        *)
            # Handler ran and rejected the probe payload — import path is healthy.
            echo "  ok    $NAME (handler error: $ERROR_TYPE)"
            ;;
    esac
done

if [ "$FAILED" = "1" ]; then
    echo "FATAL: import-dead functions detected; fix before calling the deploy good."
    exit 1
fi
echo "Post-deploy INIT sweep passed."
