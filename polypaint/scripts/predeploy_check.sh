#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if command -v uv >/dev/null 2>&1; then
    TEST_PYTHON=(uv run python)
elif [ -x "$ROOT/.venv/bin/python" ]; then
    TEST_PYTHON=("$ROOT/.venv/bin/python")
elif [ -x "$ROOT/../.venv/bin/python" ]; then
    TEST_PYTHON=("$ROOT/../.venv/bin/python")
else
    TEST_PYTHON=(python3)
fi

echo "Running predeploy contract gate..."
"${TEST_PYTHON[@]}" api_manifest.py --check
"${TEST_PYTHON[@]}" -m pytest tests/test_api_route_contracts.py tests/test_deploy_packaging.py -q
bash tests/test_frontend_js.sh
echo "Predeploy contract gate passed."
