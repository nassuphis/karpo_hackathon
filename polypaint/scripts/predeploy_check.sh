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
"${TEST_PYTHON[@]}" -m pytest \
    tests/test_api_route_contracts.py \
    tests/test_deploy_packaging.py \
    tests/test_render_workflow_definition.py \
    tests/test_render_plan.py \
    tests/test_finalize_mt_handler.py \
    tests/test_raster_mt.py \
    tests/test_bilevel_handler.py \
    tests/test_coeff_bilevel_stitch_handler.py \
    tests/test_solve_proximity_handler.py \
    -q
bash tests/test_frontend_js.sh
echo "Predeploy contract gate passed."
