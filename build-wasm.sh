#!/bin/bash
set -e

CLANG=/opt/homebrew/opt/llvm/bin/clang

# --- Build the full step-loop WASM ---
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
