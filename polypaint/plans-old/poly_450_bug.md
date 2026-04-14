# poly_450 sweep_cm Bug Investigation

Status: legacy archive. Historical bug-investigation note preserved for debugging history; it still references migration-era hypotheses that are not current render/storage guidance.

## 1. Symptom

Job `compute_mndo3ju1` — `poly_450 [rev] N=2000, chunks=400, degree=35, companion_matrix solver` — fails at the sweep (solve) phase.

Error from browser log:
```
[11:59:14 PM] Render failed: An error occurred (NoSuchKey)...
[11:58:57 PM] Compute: [unit_circle,coeff10] poly_450 [rev] N=2000, chunks=400...
```

And on a second attempt (Color render):
```
sweep-cm HTTP 500: {"message":"Internal Server Error"}
```

Both the **lores solve** and the **hires fan-out** fail.

## 2. Evidence Gathered

### 2.1 CloudWatch Logs (polypaint-sweep-cm)

**Error**: `JSONDecodeError: Expecting value: line 1 column 2 (char 1)`
- At `handler_sweep_cm.py:66`: `compute_meta = json.loads(result.stdout)`
- The binary exits with returncode 0 but stdout is not valid JSON
- "column 2 (char 1)" means stdout has exactly 1 byte before the parse fails — likely a null byte or single newline, not truly empty

**REPORT lines**:
- Duration: 269ms / 94ms — far too fast for degree-35 eigenvalue decomposition on thousands of steps
- Memory: 109MB / 113MB of 4096MB — not OOM
- No stderr output from the binary logged

### 2.2 Binary Behavior (sweep_cm.c)

- Line 209: reads coefficients in a `while(fread(...))` loop
- Line 240: **unconditionally** prints JSON metadata after the loop and `fclose`
- If the binary reaches line 240, stdout WILL have valid JSON
- Therefore: the binary is **crashing before line 240** (segfault, abort, or linker error)

### 2.3 Why returncode might be 0 despite a crash

- Python `subprocess.run` with `capture_output=True` — if the process is killed by a signal (SIGSEGV, SIGABRT), the returncode should be negative (e.g. -11). The handler only checks `returncode != 0`, so this SHOULD be caught.
- **However**: if the LAPACK layer is missing or incompatible, the dynamic linker may print to stderr and exit with code 127 or similar — which WOULD be caught.
- **Alternative**: the binary is actually writing partial output. `char 1` in the JSONDecodeError suggests stdout is 1+ bytes, not empty. This could be a `\n` or partial printf from a crash during the `fread` loop.

### 2.4 Recent Code Changes

`handler_sweep_cm.py` was modified in commit `6eeb579`:
- `stripe_idx` → `chunk_idx` rename
- Default s3_key changed from `stripe_` to `chunk_` prefix
- **These changes are cosmetic and don't affect the solve path**

`handler_render_plan.py` field priority was flipped:
- Before: `calc.get("n_stripes", calc.get("n_chunks", 10))`
- After: `calc.get("n_chunks", calc.get("n_stripes", 10))`
- **This affects the render pipeline (Step Functions) but NOT the compute pipeline** (browser-driven sweep-cm)

### 2.5 What Works

- Preview generation works for this job (uses lores data)
- The Aberth-Ehrlich solver (`sweep`) works for other jobs
- Lower-degree jobs likely work with `sweep_cm`
- Coeffgen succeeds: degree=35, 400 chunks, 1152MB total

## 3. Hypotheses

### H1: LAPACK layer missing or incompatible (MOST LIKELY)

The `sweep_cm` binary links against LAPACK (`zgeev_`). This requires the `polypaint-lapack` Lambda layer.

**Check**: Was the Lambda deployed with the correct layer? Did a recent deploy drop the LAPACK layer? The binary would crash immediately on the first `zgeev_` call with a dynamic linker error.

**Evidence supporting**: 269ms runtime is consistent with "read coeffs, call zgeev, crash" — no actual eigenvalue computation happened.

### H2: Binary crashes on degree=35 input

The companion matrix for degree 35 is 35×35 complex doubles. `calloc(35*35, 16)` = ~20KB. LAPACK workspace query should handle this. But maybe:
- A specific coefficient pattern causes NaN/Inf in the companion matrix
- LAPACK `zgeev_` fails with `info < 0` (bad argument) due to a subtle bug in matrix construction
- The error handler at line 145 returns but the binary continues — it shouldn't crash

**Evidence against**: The binary handles `info != 0` gracefully (zeros output, returns). It wouldn't crash.

### H3: Empty or corrupt coefficients file

If the coefficients file downloaded from S3 is empty or truncated:
- The `fread` loop runs 0 times
- `totalSteps = 0`
- Binary reaches line 240 and prints JSON normally

**Evidence against**: This wouldn't cause a crash. The binary would print `{"mode":"solve_cm","n_t":0,...}` and exit 0.

### H4: stdout captured incorrectly

The handler uses `subprocess.run(capture_output=True, text=True)`. If the binary writes to stdout in a way that doesn't flush (e.g. crash before `printf` buffer is flushed), `result.stdout` could be partial.

**Evidence supporting**: "char 1" in the error means stdout is not empty but not valid JSON. This is consistent with a crash during or after the computation loop but before/during the `printf` at line 240.

## 4. Investigation Plan

### Step 1: Check deployed Lambda layer configuration

```bash
aws lambda get-function-configuration --function-name polypaint-sweep-cm --region us-east-1 \
    --query '{Layers: Layers[*].Arn, Runtime: Runtime, Architecture: Architectures}'
```

Verify the LAPACK layer ARN is present. Compare with `polypaint-sweep` (Aberth solver) which should have a different layer setup.

### Step 2: Check if sweep (Aberth) works for this job

Run a compute with the same job parameters but using the Aberth-Ehrlich solver instead of companion matrix. If it succeeds, the issue is specific to sweep_cm.

### Step 3: Reproduce locally with Docker

```bash
# Build sweep_cm for local testing
cd lambda
# Run with a test coefficients file
echo '{"mode":"solve_cm","coeffs_file":"/tmp/test_coeffs.bin","n_coeffs":36}' | ./sweep_cm /tmp/test_roots.bin
```

Create a small test coefficients file with degree=35 (36 complex coefficients = 288 bytes per step). Verify the binary produces valid JSON.

### Step 4: Add diagnostic logging to handler

Before `json.loads(result.stdout)`, log:
```python
print(f"sweep_cm returncode={result.returncode} stdout_len={len(result.stdout)} stderr_len={len(result.stderr)}")
print(f"sweep_cm stdout={repr(result.stdout[:200])}")
print(f"sweep_cm stderr={repr(result.stderr[:500])}")
```

Deploy and re-run. The CloudWatch output will show exactly what the binary produced.

### Step 5: Check the actual stdout content

The JSONDecodeError says "char 1" — the first byte is not a valid JSON start character. Log `repr(result.stdout)` to see exactly what byte(s) are present. This distinguishes:
- `\n` only (binary crashed after a newline but before printf)
- Partial JSON (binary crashed during printf)
- Dynamic linker error message (missing .so)
- LAPACK error output

### Step 6: Test LAPACK linkage on Lambda

Add a trivial LAPACK test to the binary startup or create a test Lambda that just calls `zgeev_` on a 2×2 matrix. If this fails, the layer is the problem.

### Step 7: Check binary compilation

Verify the deployed `sweep_cm` binary was compiled with the correct LAPACK linkage:
```bash
ldd lambda/sweep_cm  # check shared library dependencies
file lambda/sweep_cm  # verify ARM64
```

If it's statically linked, LAPACK should be embedded. If dynamically linked, it needs the layer at runtime.

## 5. Most Efficient Path

**Do Step 4 first.** Adding 3 lines of diagnostic logging to `handler_sweep_cm.py` and redeploying will immediately reveal:
- Whether the binary crashed (returncode != 0, stderr has crash info)
- Whether LAPACK is missing (stderr has "cannot open shared object file")
- Whether the binary ran but produced garbage (stdout has partial content)
- Whether the coefficients file was empty (n_t=0 in output)

This is the fastest way to narrow from 4 hypotheses to 1.

## 6. Quick Fix Path (if LAPACK layer is missing)

If Step 1 confirms the layer is missing:
```bash
aws lambda update-function-configuration --function-name polypaint-sweep-cm \
    --layers <LAPACK_LAYER_ARN> <OTHER_LAYERS> --region us-east-1
```

Or re-run `deploy.sh update` which should attach all required layers.
