# Code Review 13

Reviewed current pushed `HEAD`: `bc0ee74 CR12: fix split load order, add sequential-load gate + parts contract`.

## Findings

### [concern] Frontend deploy is improved, but still not truly mixed-set safe

The CR12 deploy race is partially fixed: `upload_frontend_assets()` now uploads all non-index assets first and uploads stamped `index.html` last.

- `deploy.sh:189-205`

This fixes the direct "new index points at missing/old JS" failure from the previous review. However, the comment still claims cache-busting means caches can never serve a stale or mixed set:

- `deploy.sh:173-177`

That is still not true while assets use stable S3 object keys. Query strings change browser cache behavior, but S3 still serves `js/01-core-compute.js` from the same object key. During deploy, there is still a window where:

- The old `index.html` is live.
- Some `js/*.js` / generated catalog objects have already been overwritten with new bodies.
- A browser fetching the old index can request `js/foo.js?v=<old-build-id>` and receive the new object body.

For this specific commit, old-index + new-JS is probably okay because the HTML/script tag set did not materially change from `43af142` to `bc0ee74`. But the general invariant in the comment is false, and future frontend splits can break in the opposite direction.

Fix options:

- Use build-specific asset keys, for example `assets/${BUILD_ID}/js/...`, and upload the new index last.
- Or use content-hashed filenames for all generated catalogs and JS parts.
- At minimum, soften the comment and add an explicit test that `index.html` uploads last so the fixed direction does not regress.

### [concern] The parts contract is weaker than its docstring says

The new structural test says it "closes" the load-order gap by allowing only declarations outside the `js/12` boot block:

- `tests/test_frontend_parts_contract.py:1-11`

But the scanner treats every column-zero `const`, `let`, and `var` line as safe:

- `tests/test_frontend_parts_contract.py:26-31`
- `tests/test_frontend_parts_contract.py:136`

That does not actually mean "non-executable." A line like this passes the structural test but executes immediately during part load:

```js
const x = laterPartFunction();
```

The current code already relies on top-level computed declarations/IIFEs, for example catalog construction and enrichment in `js/07-transform-catalogs.js`. That is acceptable when the dependencies are earlier in load order, but the test's claim is stronger than what it enforces. The sequential runtime gate catches the current bug class under its DOM stub, but it will not reliably catch a conditional top-level initializer path.

There is a second small hole: any file containing a line that starts with `/* ---- Boot sequence ----` becomes exempt after that line.

- `tests/test_frontend_parts_contract.py:122-130`

The test should explicitly assert that the boot block exists only in `js/12-deepzoom-boot.js`.

Fix:

- Either weaken the docstring/error text to say the test is a lightweight top-level statement scanner, not a full guarantee.
- Or tighten it: allow boot only in `js/12`, reject top-level declaration initializers that contain obvious call/IIFE syntax unless they are listed in a small whitelist, and keep the sequential load gate as the runtime backstop.

### [nit] Upload-order fix is not pinned by deploy packaging tests

The production code now uploads `index.html` last, but `tests/test_deploy_packaging.py` only checks that upload/verify helpers and cache-busting exist; it does not assert the index-last ordering.

- `tests/test_deploy_packaging.py:548-559`
- `deploy.sh:196-205`

Given this was a real deployment race, the ordering should have a small regression test. A text-order assertion is enough: within `upload_frontend_assets()`, the loop that skips `index.html` must appear before the final `aws s3 cp "$STAMPED" "s3://$BUCKET/index.html"`.

## Confirmed Good

- The CR12 browser-load blocker is fixed. The boot/init calls now run from `js/12-deepzoom-boot.js` after all split parts are parsed:
  - `js/12-deepzoom-boot.js:1021-1273`
- `_coeffProgramLegacyInputDefs` moved into `js/07-transform-catalogs.js` before `_coeffProgramCatalog` is constructed, closing the specific `js/07` forward-reference failure:
  - `js/07-transform-catalogs.js:539-546`
  - `js/07-transform-catalogs.js:534`
- The frontend sequential-load gate now executes generated catalogs and each `js/*.js` part separately in `index.html` order:
  - `tests/test_frontend_js.sh:1023-1075`
- The new parts contract catches accidental top-level statements in non-boot parts and verifies tag/disk agreement:
  - `tests/test_frontend_parts_contract.py:106-146`
- The normal frontend manifest extraction still sees split files through script-tag order.
- No native code changed in this commit, so skipping Docker runtime is reasonable for this review.

## Verification Run

Passed:

```text
python3 api_manifest.py --check
bash tests/test_frontend_js.sh
uv run python -m pytest tests/test_frontend_parts_contract.py tests/test_deploy_packaging.py tests/test_deepzoom_viewport_math.py tests/test_api_route_contracts.py -q
bash scripts/predeploy_check.sh
```

Results:

```text
api_manifest.json: OK
Frontend fused render source checks: OK
Frontend fused render runtime checks: OK
Frontend sequential load checks: OK (16 scripts)
40 passed in 1.95s
403 passed, 2 subtests passed in 2.30s
Predeploy contract gate passed.
```

The first non-escalated `bash scripts/predeploy_check.sh` failed only because the sandbox blocked `uv` cache access under `~/.cache`; the escalated run passed.

## Verdict

The actual CR12 blocker is resolved. I do not see a current browser-load showstopper in `bc0ee74`, and the full predeploy gate passes. The remaining issues are hardening/documentation gaps: stable-key frontend deploys are still not atomically mixed-set safe, and the structural parts contract overstates what it proves. I would deploy this commit if you need the load-order fix, but I would tighten the upload-order test and update the deploy-cache comment soon.

---

## Resolution — 2026-06-12

**[concern] mixed-set safety — ✅ fixed at the root.** The reviewer is right
that query strings cannot make stable S3 keys safe, so the minimum option
was skipped in favor of the real one: scripts now deploy to
**build-versioned keys** (`assets/${BUILD_ID}/js/...` and
`assets/${BUILD_ID}/<generated>_js.js`) via a `deployed_asset_key` helper;
the stamped index.html references those keys and uploads **last**. A live
old index keeps resolving the old build's objects untouched for its whole
lifetime; the flip to the new set is the single index.html PUT. The false
comment is gone, replaced by one stating exactly this (and that old
`assets/` prefixes accumulate and may be pruned manually — live old tabs
reference them). `verify_frontend_assets` fetches the deployed keys.

**[concern] contract overstatement — ✅ tightened and truthed.** New
assertion: the boot banner may exist only in js/12, exactly once (the
exemption is no longer claimable by earlier parts). The docstring now
states precisely what is proven: statements (including conditional ones)
are fully covered; unconditional declaration initializers are covered by
the runtime sequential-load gate; the residual — a *conditional* expression
inside an initializer calling a later part — is named as not
machine-checked, with the whitelist-churn rationale for not flagging every
initializer with call syntax.

**[nit] index-last not pinned — ✅ pinned.** The packaging test now asserts,
inside the `upload_frontend_assets` body, that the index-skip loop precedes
the final stamped-index upload, plus pins on `assets/${BUILD_ID}/` and
`deployed_asset_key`.
