#!/bin/bash
set -e

CLANG=/opt/homebrew/opt/llvm/bin/clang

# --- Build the old solver-only WASM (kept for reference) ---
echo "Compiling solver.c → solver.wasm..."
$CLANG --target=wasm32-unknown-unknown -O3 -nostdlib \
  -Wl,--no-entry \
  -Wl,--export=solveEA \
  -Wl,--initial-memory=65536 \
  -Wl,-z,stack-size=32768 \
  -o solver.wasm solver.c

node -e "console.log(require('fs').readFileSync('solver.wasm').toString('base64'))" > solver.wasm.b64
echo "solver.wasm: $(wc -c < solver.wasm | tr -d ' ') bytes (B64: $(wc -c < solver.wasm.b64 | tr -d ' '))"

# --- Build the full step-loop WASM ---
echo ""
echo "Compiling step_loop.c → step_loop.wasm..."
$CLANG --target=wasm32-unknown-unknown -O3 -nostdlib \
  -Wl,--no-entry \
  -Wl,--export=init \
  -Wl,--export=runStepLoop \
  -Wl,--export=__heap_base \
  -Wl,--import-memory \
  -Wl,--stack-first \
  -Wl,-z,stack-size=65536 \
  -o step_loop.wasm step_loop.c

node -e "console.log(require('fs').readFileSync('step_loop.wasm').toString('base64'))" > step_loop.wasm.b64
echo "step_loop.wasm: $(wc -c < step_loop.wasm | tr -d ' ') bytes (B64: $(wc -c < step_loop.wasm.b64 | tr -d ' '))"
echo ""
echo "Done. Paste step_loop.wasm.b64 contents into WASM_STEP_LOOP_B64 in index.html"
