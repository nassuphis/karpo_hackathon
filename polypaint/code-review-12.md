# Code Review 12

Reviewed current pushed `HEAD`: `43af142 Split index.html monolith into 12 ordered js/ parts (D1 stage 1)`.

## Findings

### [blocker] The split frontend does not load as separate browser scripts

The browser executes the new files as separate classic scripts in the order listed in `index.html`:

- `index.html:2930-2941` loads `js/01-core-compute.js` through `js/12-deepzoom-boot.js`.
- `js/06-popup-init.js:1320-1334` executes initializer calls at top level before later files have loaded.
- `js/06-popup-init.js:1159` reads `_closeBilevelPopup`, but `_closeBilevelPopup` is not defined until `js/09-render-orchestration.js:1638`.
- `js/06-popup-init.js:1169` reads `_syncBilevelPopupControls`, but that is not defined until `js/09-render-orchestration.js:1651`.
- `js/06-popup-init.js:1334` calls `_clearPaletteCanvas`, but that is not defined until `js/09-render-orchestration.js:638`.
- `js/07-transform-catalogs.js:722` and `js/07-transform-catalogs.js:731` call `_coeffProgramLegacyInputDefs`, but that is not defined until `js/09-render-orchestration.js:183`.

I verified this by running the scripts sequentially in tag order with a browser-like VM context. The first real failure is:

```text
FAILED js/06-popup-init.js js/06-popup-init.js:1159
ReferenceError: _closeBilevelPopup is not defined
```

The current tests miss this because they concatenate the JS parts into one string:

- `tests/test_frontend_js.sh:50`
- `tests/test_frontend_js.sh:554`

Concatenation preserves the old single-script hoisting behavior. It is not equivalent to browser execution for top-level code that uses functions declared in later files.

Fix before deploy:

- Move all top-level UI initialization out of `js/06-popup-init.js` into the last boot file after every function definition is loaded, or move the required definitions before the init calls.
- Move `_coeffProgramLegacyInputDefs` into `js/07-transform-catalogs.js` before `_coeffProgramCatalog` is constructed, or move catalog construction later.
- Add a gate that runs generated/catalog scripts plus each `js/*.js` file separately in `index.html` tag order. Do not rely only on concatenated `vm.runInContext(src, ctx)`.

### [concern] Frontend deploy can expose a mixed asset set

`frontend_asset_keys()` emits `index.html` before the generated catalogs and `js/*.js` assets:

- `deploy.sh:73-85`

`upload_frontend_assets()` uploads in that exact order:

- `deploy.sh:189-200`

After this split, the stamped `index.html` contains `?v=${BUILD_ID}` cache-busting for all JS parts:

- `deploy.sh:173-185`

That means a user can fetch the new stamped `index.html` before the new `js/*.js` objects are uploaded. On the first split deploy this can mean missing JS objects. On later deploys it can mean the new index points at old same-key JS bodies until the loop overwrites them. The query string busts browser cache, but S3 still serves the object at the same key.

The boot check helps only partially:

- `index.html:2942-2953` checks part names/order and logs `console.error`.

It does not include a build id/content check, and it does not stop execution. If an old same-name part set is served, `__ppParts` can still match.

Fix:

- Upload non-index frontend assets first, verify them, then upload stamped `index.html` last.
- Prefer adding the build id to each part registration or a generated manifest so the boot check can detect same-name stale parts.

## Confirmed Good

- The monolith split is mechanically faithful. After removing only the new part headers, `__ppParts` registrations, and chunk-boundary blank lines, the old inline script and the concatenated new parts have no logic differences.
- Every `js/*.js` part registers exactly one expected `__ppParts` name.
- `api_manifest.py` now reads `index.html` plus `js/` parts in script-tag order, so API-contract extraction still sees the frontend calls.
- `tests/test_deepzoom_viewport_math.py` also reads the split parts in tag order.
- Deploy packaging now includes `js/*.js` assets and uses `application/javascript` for them.
- Existing deploy/test gates pass, but they do not catch the separate-script load-order blocker above.

## Verification Run

Passed:

```text
python3 api_manifest.py --check
bash tests/test_frontend_js.sh
uv run python -m pytest tests/test_deploy_packaging.py tests/test_deepzoom_viewport_math.py tests/test_api_route_contracts.py -q
bash scripts/predeploy_check.sh
```

Results:

```text
api_manifest.json: OK
Frontend fused render source checks: OK
Frontend fused render runtime checks: OK
38 passed in 1.87s
403 passed, 2 subtests passed in 2.48s
Predeploy contract gate passed.
```

Failed, intentionally added as review-only browser semantics check:

```text
sequential classic-script VM load in index.html tag order
```

First failure:

```text
js/06-popup-init.js:1159 ReferenceError: _closeBilevelPopup is not defined
```

## Verdict

Do not deploy this commit as-is. The split is mechanically clean, and the normal gates pass, but the actual browser load path is broken by forward references across split files. Fix the load-order/init placement first, then add a sequential-script frontend gate so this class of regression cannot recur.

---

## Resolution — 2026-06-12

**[blocker] split load order — ✅ fixed, gate added.** Both verified causes
relocated: the monolith's top-level initializer block (the ~20 `_init*()` /
`buildPaletteCircles` / `_clearPaletteCanvas` calls at js/06's tail) moved to
the end of js/12 as an explicit boot sequence — those calls reference
functions across several parts and must run only after every part is parsed
— and `_coeffProgramLegacyInputDefs` moved from js/09 into js/07 ahead of
the catalog IIFE that calls it at load. The review's recommended gate is now
permanent: the frontend harness executes every script tag **separately** in
index.html order under a lenient DOM stub (browser semantics, no
concatenation hoisting) and verifies the part registrations afterward.
Sequential load passes for all 16 scripts; flip-verified (re-adding one init
call to js/06 fails the gate with the exact ReferenceError).

**[concern] mixed asset set — ✅ fixed.** `upload_frontend_assets` now ships
every js part and generated catalog first and the stamped index.html
**last**, so a loading browser can never observe a new index referencing
missing or old-body part keys (S3 strong read-after-write makes index-last
sufficient). The boot check remains as the runtime alarm; per-part build
registration was considered and skipped — source-controlled parts cannot
carry a deploy-time id, and index-last ordering plus ?v= stamping closes
the window the registration would have detected.
