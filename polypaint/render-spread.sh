#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PY="$ROOT/.venv/bin/python"
if [[ ! -x "$PY" ]]; then
  PY="python3"
fi

if [[ $# -lt 1 ]]; then
  cat >&2 <<'USAGE'
Usage:
  ./render-spread.sh COLOR_ARTIFACT_ID [COMPUTE_JOB_ID] [--render-preview] [--keep-work]

Examples:
  ./render-spread.sh color_run_1781711887328_v21ejh
  ./render-spread.sh color_run_1781711887328_v21ejh compute_mqi0y5o0 --render-preview
  ./render-spread.sh color_run_1781711887328_v21ejh compute_mqi0y5o0 --keep-work

Outputs are written to test-pdf/. By default only the PDF is kept. With
--render-preview, rendered page PNGs are kept too. Use --keep-work to keep
downloaded images, prepared images, and JSON sidecars for debugging.
USAGE
  exit 2
fi

export AWS_DEFAULT_REGION="${AWS_DEFAULT_REGION:-us-east-1}"

exec "$PY" "$ROOT/scripts/render_spread_pdf.py" "$@" --out-dir "$ROOT/test-pdf"
