#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORKDIR="${WORKDIR:-$(mktemp -d "${TMPDIR:-/tmp}/polypaint-oracle.XXXXXX")}"
BINARY="$WORKDIR/sweep_oracle"

cd "$ROOT"
python3 tests/oracle_runner.py \
  --compile-binary "$BINARY" \
  --workdir "$WORKDIR/runs" \
  --write-baselines \
  --baseline-dir tests/fixtures/oracle/baseline

echo "Oracle baselines updated in tests/fixtures/oracle/baseline"
