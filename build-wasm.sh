#!/bin/bash
set -e

CLANG=/opt/homebrew/opt/llvm/bin/clang

echo "Compiling solver.c → solver.wasm..."
$CLANG --target=wasm32-unknown-unknown -O3 -nostdlib \
  -Wl,--no-entry \
  -Wl,--export=solveEA \
  -Wl,--initial-memory=65536 \
  -Wl,-z,stack-size=32768 \
  -o solver.wasm solver.c

echo "Base64 encoding..."
node -e "console.log(require('fs').readFileSync('solver.wasm').toString('base64'))" > solver.wasm.b64

echo "WASM size: $(wc -c < solver.wasm | tr -d ' ') bytes"
echo "Base64 size: $(wc -c < solver.wasm.b64 | tr -d ' ') bytes"
echo "Done. Base64 string saved to solver.wasm.b64"
