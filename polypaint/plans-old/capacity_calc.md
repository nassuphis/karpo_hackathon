**Capacity Calc**

Status: legacy archive. Historical sizing note preserved for reference; current compute and render naming is chunk-based, and `stripes` here is older terminology.

This note describes how to estimate whether a compute job will fit within Lambda storage and memory constraints before dispatch.

The immediate motivation is the coeffgen failure:

- `N=5000`
- `times=10`
- `degree=49`
- `n_coeffs=50`
- `stripes=100`

which failed with:

- `coeffs.bin size mismatch: expected 1000000000, got 538329088`

That error was not just a large-job warning. It exposed a deploy bug and a missing UI preflight.

**Actual Cause**

`polypaint-coeffgen` is currently deployed with the default `512 MB` ephemeral storage, not the intended `10 GB`.

This is visible in [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh):

- `BINARY_TMP=10240` is defined at [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh#L45)
- `create_lambda()` defaults `TMP` to `512` at [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh#L236)
- `update_lambda()` defaults `TMP` to `512` at [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh#L255)
- `polypaint-coeffgen` create/update does not pass `BINARY_TMP` at [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh#L509) and [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh#L580)

So the first required fix is:

- pass `"$BINARY_TMP"` to coeffgen create/update, the same way raster/finalize/bilevel already do

After that, coeffgen will have the intended `10 GB` `/tmp`.

**Why Preflight Still Matters**

Even after fixing coeffgen `/tmp`, the app should still estimate job size when `Calculate` is pressed.

Reason:

- the job may still be too large for `/tmp`
- or large enough to be a bad idea operationally
- and the app already knows the needed inputs:
  - `N`
  - `times`
  - `degree`
  - `n_coeffs`
  - `stripes`
  - Lambda `/tmp` budget

So the app can:

- warn before dispatch
- suggest a safe stripe count
- explain which resource is limiting

**Core Formulas**

Let:

- `N` = grid size
- `times` = repeat count
- `degree` = polynomial degree
- `n_coeffs = degree + 1` unless coeff transforms change length
- `stripes` = number of stripe jobs
- `rows_per_stripe = ceil(N / stripes)`

All current coeff and root binaries are `float32`, so each complex number is:

- `2 * 4 = 8` bytes

**Coefficient Stripe Size**

Each coeffgen stripe writes:

- `rows_per_stripe * N * times * n_coeffs` complex coefficients

So:

- `coeff_bytes_per_stripe = rows_per_stripe * N * times * n_coeffs * 8`

Equivalent expanded form:

- `coeff_bytes_per_stripe = rows_per_stripe * N * times * n_coeffs * 2 * 4`

**Root Stripe Size**

Each solve stripe writes:

- `rows_per_stripe * N * times * degree` complex roots

So:

- `root_bytes_per_stripe = rows_per_stripe * N * times * degree * 8`

Equivalent expanded form:

- `root_bytes_per_stripe = rows_per_stripe * N * times * degree * 2 * 4`

**Param Dump Size**

For param debug, each point writes:

- `t1_re, t1_im, t2_re, t2_im`
- `4 * float32`

So:

- `param_dump_bytes = N * N * 4 * 4`

or:

- `param_dump_bytes = N^2 * 16`

**Worst-Case Compute Estimate**

For compute preflight, the two most important per-stripe estimates are:

- `coeff_bytes_per_stripe`
- `root_bytes_per_stripe`

The job is capacity-safe only if both fit comfortably within the relevant `/tmp` budget.

Use a safety margin. Do not plan to fill `/tmp` to 100%.

Recommended safety budget:

- `safe_tmp_bytes = 0.8 * lambda_tmp_bytes`

That leaves room for:

- JSON specs
- log files
- partial outputs
- process overhead
- future code changes

**Suggested Stripe Count**

Given a safe budget, compute the minimum stripe count needed for coeffgen:

- `min_stripes_for_coeffs = ceil(total_coeff_bytes / safe_tmp_bytes)`

where:

- `total_coeff_bytes = N * N * times * n_coeffs * 8`

And for solve roots:

- `min_stripes_for_roots = ceil(total_root_bytes / safe_tmp_bytes)`

where:

- `total_root_bytes = N * N * times * degree * 8`

Then:

- `recommended_min_stripes = max(min_stripes_for_coeffs, min_stripes_for_roots, 1)`

This gives the minimum stripe count so that no single stripe should exceed the safety budget.

**Worked Example**

For:

- `N=5000`
- `times=10`
- `degree=49`
- `n_coeffs=50`
- `stripes=100`

we have:

- `rows_per_stripe = ceil(5000 / 100) = 50`

Coefficient stripe bytes:

- `50 * 5000 * 10 * 50 * 8 = 1,000,000,000`

So one coeff stripe is about:

- `1.0 GB` decimal
- `953.7 MiB` binary

Root stripe bytes:

- `50 * 5000 * 10 * 49 * 8 = 980,000,000`

So one root stripe is about:

- `980 MB` decimal
- `934.6 MiB` binary

This job is impossible with a `512 MB` coeffgen `/tmp`, which is exactly why it failed.

It should fit comfortably once coeffgen is actually given `10 GB` `/tmp`.

**What The UI Should Do**

On `Calculate`, before dispatch:

1. Determine `degree` and `n_coeffs`.
2. Compute:
   - `rows_per_stripe`
   - `coeff_bytes_per_stripe`
   - `root_bytes_per_stripe`
3. Compare against configured budgets.
4. If over budget:
   - block dispatch or require confirmation
   - suggest a minimum stripe count
5. If near budget:
   - warn that the job is large but still allowed

Useful user-facing messages:

- `Each coeff stripe is estimated at 953 MiB; coeffgen Lambda /tmp budget is 512 MiB. Increase stripes to at least 187 or reduce N/times.`
- `Each coeff stripe is estimated at 953 MiB; coeffgen Lambda /tmp budget is 10 GiB. Job is large but within configured limits.`

**Where To Put The Config**

The frontend should not hardcode guesses independently of deploy.

Recommended:

- define a small capacity config object in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- keep it synchronized with [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)

At minimum:

- `coeffgen_tmp_mb`
- `sweep_tmp_mb`
- safety factor

Better:

- expose a small `/config` endpoint later if this becomes annoying to keep in sync

**What This Does Not Estimate**

This simple preflight is about binary output size and `/tmp`.

It does not directly estimate:

- solver RAM
- coeffgen RAM
- total wall-clock time
- S3 transfer time
- DynamoDB polling overhead

Those can be added later, but `/tmp` is the easiest and most important first gate because it is deterministic from the job parameters.

**Recommended Implementation Order**

1. Fix coeffgen deployment so it actually gets `10 GB` `/tmp`.
2. Add compute preflight in the UI using the formulas above.
3. Show:
   - estimated coeff stripe size
   - estimated root stripe size
   - minimum recommended stripes
4. Warn early for jobs that fit but are unusually large.

**Bottom Line**

Yes, the app can test for this when `Calculate` is pressed.

The formulas are straightforward, the required inputs are already known, and the app can suggest a stripe count that should fit.

But first, coeffgen’s deploy config must be corrected, because right now it is accidentally running with the default `512 MB` `/tmp` instead of the intended `10 GB`.
