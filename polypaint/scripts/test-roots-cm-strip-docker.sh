#!/bin/bash
# ARM64 Docker gate for the roots_cm strip=exact mode (giga_2880 wave).
#
# Builds sweep_cli.c inside Amazon Linux 2023 with the SAME netlib
# LAPACK 3.10 layer the deploy uses, then runs probe rows of the
# giga_2880 program and checks them against oracle rows computed by the
# HOST python (numpy/Accelerate-new). This proves, for the deployed
# LAPACK lineage:
#   1. strip=exact keeps all 32 roots (no relative-threshold stripping);
#   2. the zgeev emission ORDER matches numpy's (order-positional parity
#      at f32 tolerance) — i.e. lambda renders pair roots to slots the
#      same way the reference's np.roots did.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

LAPACK_BUILD="$ROOT/lambda/layer-build-lapack"
if [ ! -d "$LAPACK_BUILD/lib" ]; then
    echo "ERROR: lambda/layer-build-lapack missing. Run lambda/build-lapack-layer.sh first."
    exit 1
fi

WORK="$(mktemp -d /tmp/roots-cm-strip-gate.XXXXXX)"
trap 'rm -rf "$WORK"' EXIT

if command -v uv >/dev/null 2>&1; then
    TEST_PYTHON=(uv run python)
else
    TEST_PYTHON=(python3)
fi

# Host side: emit probe payload + oracle rows (numpy zgeev order).
"${TEST_PYTHON[@]}" - "$WORK" <<'EOF'
import json, sys, os
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)) if '__file__' in dir() else '.', 'lambda'))
sys.path.insert(0, 'lambda')
import numpy as np
from coeff_program_source import compile_coeff_program_source
from cp437_font import FONT_ROWS

work = sys.argv[1]
rows = FONT_ROWS[2]
pts = np.array([complex(x-3.5, (7-y)-3.5)
                for y in range(8) for x in range(8) if rows[y] & (1 << (7-x))],
               dtype=np.complex128)
M = "floor(71*t1) - 71*floor(floor(71*t1)/71)"
N = "floor(101*t2) - 101*floor(floor(101*t2)/101)"
DELTA = ("(0.05+0.09*(71*t1-floor(71*t1))) + 1i*(0.05+0.09*(101*t2-floor(101*t2)))"
         " + (poly[0]/5)*exp(1i*6.283185307179586*(poly[1]/3))")
SRC = f"""poly = fill(2, 0)
poly[0] = {M}
poly[1] = {N}
poly = translate_roots(roots_ascii_literal(2), {DELTA})
poly
poly = sort_mod_keep_angle(poly)
poly = multiply(poly, 0.1)
poly = add(pop, poly)
poly
poly = sort_abs(poly)
poly = divide(peek, abs(poly[32]))
poly = multiply(poly, 6.283185307179586i)
poly = exp(poly)
poly = divide(pop, poly)
poly
poly = sort_abs(poly)
poly = divide(pop, abs(poly[32]))
poly
poly = roots_cm(poly, lo, exact)
poly[32] = 1
poly = multiply(poly, 0.1)
poly = add(pop, poly)
emit"""
compiled = compile_coeff_program_source(SRC)
payload = {"version": 1, "fingerprint": compiled["fingerprint"], "tokens": compiled["tokens"],
           "stack_max": compiled["stack_max"], "scalar_exprs": compiled["scalar_exprs"],
           "vector_constants": compiled["vector_constants"]}

def oracle(t1, t2):
    m = np.floor(71*t1) % 71; j1 = 71*t1 - np.floor(71*t1)
    n = np.floor(101*t2) % 101; j2 = 101*t2 - np.floor(101*t2)
    rts = pts + complex(0.05+0.09*j1, 0.05+0.09*j2) + (m/5)*np.exp(1j*2*np.pi*(n/3))
    cf = np.poly(rts).astype(complex)
    cf = (np.sort(np.abs(cf)) * np.exp(1j*np.angle(cf))) * 0.1 + cf
    sa = np.max(np.abs(cf)); cf = cf / np.exp(1j*2*np.pi*(cf/sa))
    cf = cf / np.max(np.abs(cf))
    return 0.1*np.append(np.roots(cf), 1) + cf

probes = [(0.137, 0.823), (0.75, 0.31), (0.031, 0.977), (0.9137, 0.412),
          (0.5192, 0.2652), (0.2364, 0.9452)]
spec = []
for t1, t2 in probes:
    q = oracle(t1, t2)
    spec.append({"t1": t1, "t2": t2,
                 "expected": [[z.real, z.imag] for z in q]})
with open(os.path.join(work, "probes.json"), "w") as fh:
    json.dump({"payload": payload, "rows": spec}, fh)
print(f"emitted {len(spec)} oracle rows")
EOF

# Container side: build with the deploy LAPACK and run the probes.
docker run --rm --platform linux/arm64 \
    -v "$ROOT/lambda:/src" \
    -v "$LAPACK_BUILD:/opt" \
    -v "$WORK:/work" \
    public.ecr.aws/amazonlinux/amazonlinux:2023 \
    bash -c '
        set -euo pipefail
        dnf install -y gcc python3 2>&1 | tail -1
        export LD_LIBRARY_PATH=/opt/lib
        gcc -O2 -pthread -DHAVE_LAPACK_COMPANION -o /work/sweep_test /src/sweep_cli.c \
            -L/opt/lib -llapack -lopenblas -lm -Wl,-rpath,/opt/lib
        python3 - <<PYEOF
import json, subprocess
spec = json.load(open("/work/probes.json"))
payload = spec["payload"]
worst = 0.0
for row in spec["rows"]:
    proc = subprocess.run(["/work/sweep_test", "/tmp/gate_row.bin"],
        input=json.dumps({"mode": "compute_debug", "function": "const",
                          "cfpv": [1, 0, 0], "u": row["t1"], "v": row["t2"],
                          "grid_n": 1000, "coeff_transforms": [],
                          "coeff_program": payload}),
        capture_output=True, text=True, timeout=60)
    assert proc.returncode == 0, proc.stderr[:400]
    meta = json.loads(proc.stdout)
    actual = [complex(re, im) for re, im in meta["coeff"]["poly"]]
    expected = [complex(re, im) for re, im in row["expected"]]
    assert len(actual) == len(expected) == 33, (len(actual), len(expected))
    for a, e in zip(actual, expected):
        rel = abs(a - e) / max(1.0, abs(e))
        worst = max(worst, rel)
print(f"ARM64 netlib-LAPACK order-positional parity: worst rel = {worst:.3e}")
# f32 cast bounds the residual; order mismatches would show as O(1).
assert worst < 1e-4, f"order/parity broke: {worst}"
print("roots_cm strip=exact ARM64 gate PASSED")
PYEOF
    '
