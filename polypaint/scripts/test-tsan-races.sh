#!/usr/bin/env bash
# CR32 F5 gate: ThreadSanitizer over the multi-worker failure paths.
#
# Forces one worker to fail while the others are mid-flight so the
# failure-flag handshake (atomic_int failed + mutex-guarded error string)
# is exercised under contention:
#   1. coeffgen_chunked, 8 workers, params file TRUNCATED to half the claimed
#      step range -> the tail-range workers hit short preads and set the flag
#      while low-range workers are still computing.
#   2. param_gen static scheduler, 8 workers, normal run (clean-path TSan).
#   3. param_gen static scheduler with output at /dev/full (Linux only) ->
#      every worker's first flush fails; skipped where /dev/full is absent.
#
# Any TSan report ("WARNING: ThreadSanitizer") fails the gate.
set -euo pipefail
cd "$(dirname "$0")/.."

CC=${CC:-cc}
WORK=$(mktemp -d /tmp/pp_tsan.XXXXXX)
trap 'rm -rf "$WORK"' EXIT
BIN="$WORK/sweep_tsan"

echo "building sweep_cli with -fsanitize=thread..."
"$CC" -O1 -g -pthread -fsanitize=thread -I lambda lambda/sweep_cli.c -lm -o "$BIN"

TSAN_LOG="$WORK/tsan_log"
export TSAN_OPTIONS="log_path=$TSAN_LOG exitcode=66"

run_case() {
    local label="$1" payload="$2" out="$3" expect_fail="$4"
    set +e
    echo "$payload" | "$BIN" "$out" >"$WORK/$label.stdout" 2>"$WORK/$label.stderr"
    local rc=$?
    set -e
    if [ "$rc" -eq 66 ]; then
        echo "FAIL [$label]: ThreadSanitizer reported a race (exit 66)"
        cat "$TSAN_LOG"* 2>/dev/null || true
        exit 1
    fi
    if [ "$expect_fail" = "yes" ] && [ "$rc" -eq 0 ]; then
        echo "FAIL [$label]: expected the run to fail (forced error) but it passed"
        exit 1
    fi
    if [ "$expect_fail" = "no" ] && [ "$rc" -ne 0 ]; then
        echo "FAIL [$label]: clean run failed rc=$rc"
        cat "$WORK/$label.stderr"
        exit 1
    fi
    echo "ok [$label] rc=$rc"
}

# full params file: 64x64 grid
PARAMS_FULL="$WORK/params_full.bin"
run_case "paramgen_seed" \
    '{"mode":"param_gen","n1":64,"n2":64,"times":1,"n_threads":4}' \
    "$PARAMS_FULL" no

# 1) truncated params -> short pread in tail workers while others compute
PARAMS_TRUNC="$WORK/params_trunc.bin"
head -c "$(( $(stat -f%z "$PARAMS_FULL" 2>/dev/null || stat -c%s "$PARAMS_FULL") / 2 ))" \
    "$PARAMS_FULL" > "$PARAMS_TRUNC"
run_case "chunked_forced_failure" \
    "{\"mode\":\"coeffgen_chunked\",\"function\":\"poly_1\",\"cfpv\":[],\"params_file\":\"$PARAMS_TRUNC\",\"step_start\":0,\"step_count\":4096,\"source_step_start\":0,\"source_n1\":64,\"source_n2\":64,\"n_threads\":8}" \
    "$WORK/chunk_out.bin" yes

# 2) param static scheduler, clean 8-worker run
run_case "param_static_clean" \
    '{"mode":"param_gen","n1":256,"n2":256,"times":1,"n_threads":8}' \
    "$WORK/param_out.bin" no

# 3) param static write failure (Linux /dev/full)
if [ -w /dev/full ] 2>/dev/null; then
    run_case "param_static_forced_failure" \
        '{"mode":"param_gen","n1":64,"n2":64,"times":1,"n_threads":8}' \
        /dev/full yes
else
    echo "skip [param_static_forced_failure]: /dev/full not available on this host"
fi

# 4) sweep_cm threaded solve (CM threading wave): 8 workers over a mixed
#    fixture incl. guard rows. LAPACK needed: Accelerate on macOS; skipped
#    where no LAPACK is linkable (the ARM64 docker gate covers netlib).
CM_BIN="$WORK/sweep_cm_tsan"
CM_BUILT=no
if [ "$(uname)" = "Darwin" ]; then
    if "$CC" -O1 -g -pthread -fsanitize=thread -DPOLYPAINT_ACCELERATE_NEWLAPACK \
        -I lambda lambda/sweep_cm.c -framework Accelerate -lm -o "$CM_BIN" 2>"$WORK/cm_build.err"; then
        CM_BUILT=yes
    fi
elif "$CC" -O1 -g -pthread -fsanitize=thread -I lambda lambda/sweep_cm.c \
        -llapack -lopenblas -lm -o "$CM_BIN" 2>"$WORK/cm_build.err"; then
    CM_BUILT=yes
fi
if [ "$CM_BUILT" = "yes" ]; then
    CM_FIX="$WORK/cm_fixture.bin"
    python3 - "$CM_FIX" <<'PYEOF'
import struct, sys, random
random.seed(31)
rows, nc = 300, 21
with open(sys.argv[1], "wb") as fh:
    for r in range(rows):
        for k in range(nc):
            re, im = random.gauss(0, 1), random.gauss(0, 1)
            if r == 7: re, im = 0.0, 0.0
            if r == 13 and k == 0: re = float("inf")
            fh.write(struct.pack("<ff", re, im))
PYEOF
    OLD_BIN="$BIN"
    BIN="$CM_BIN"
    run_case "sweep_cm_threaded_clean" \
        "{\"mode\":\"solve_cm\",\"coeffs_file\":\"$CM_FIX\",\"n_coeffs\":21,\"n_threads\":8}" \
        "$WORK/cm_out.bin" no
    # solver-brush wave: JT keeps per-worker heap JtState, Newton is
    # stack-local — both must be race-free over the same partition
    run_case "sweep_jt_threaded_clean" \
        "{\"mode\":\"solve_jt\",\"coeffs_file\":\"$CM_FIX\",\"n_coeffs\":21,\"n_threads\":8}" \
        "$WORK/jt_out.bin" no
    run_case "sweep_newton_threaded_clean" \
        "{\"mode\":\"solve_newton\",\"coeffs_file\":\"$CM_FIX\",\"n_coeffs\":21,\"n_threads\":8}" \
        "$WORK/newton_out.bin" no
    BIN="$OLD_BIN"
else
    echo "skip [sweep_cm_threaded_clean]: no LAPACK linkable on this host"
fi

if ls "$TSAN_LOG"* >/dev/null 2>&1; then
    echo "FAIL: ThreadSanitizer wrote reports:"
    cat "$TSAN_LOG"*
    exit 1
fi
echo "TSan gate PASSED (no races reported)"
