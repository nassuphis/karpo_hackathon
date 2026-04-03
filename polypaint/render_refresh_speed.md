# Render Refresh Speed

Status: implemented.

This document records the current Render refresh design after the refresh-speed refactor.

## Current Behavior

Render refresh is now driven by one frontend API call:

- `POST /render-summary`

Both of these paths use that same route:

- manual `Refresh`
- post-run automatic refresh

The browser no longer performs render discovery through:

- `/head-keys`
- `/list-prefix`
- `/presign`
- direct `calc.json` fetches

before building the Render panel.

## Response Shape

`/render-summary` now returns:

- `schema_version: 2`
- `calc`
- `families`
- `deepzoom_latest`

`families` is the current source of truth for the immutable Render UI:

- `color`
- `bilevel`
- `coeffs`
- `palette`

The older `artifacts` block is still returned for compatibility, but the current Render UI is driven by `families`.

## Server-Side Work

The storage Lambda now does the S3 work server-side:

- reads `calc.json`
- reads `deepzoom_latest.json`
- lists immutable family prefixes
- reads palette `meta.json` records
- synthesizes legacy `legacy_*` entries when old top-level artifacts still exist

This keeps the browser request shape small even though the server still has to inspect artifact storage.

## What Improved

The big win was removing the browser waterfall.

The old Render refresh shape depended on a sequence of browser-side discovery calls.
The current shape avoids that and renders from one structured summary response.

That also removed the old dependency on listing large DeepZoom prefixes from the Render tab.

## Current Limits

The refactor improved request shape, not the theoretical minimum server-side work.

Current server-side cost still grows with:

- number of immutable render artifacts in each family
- number of saved palettes for the job

So the next optimization, if refresh ever becomes slow again, would be:

- a compact job-level render index

not a return to browser-side probing.

## Related Files

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)
- [tests/e2e/render-refresh.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-refresh.spec.js)
- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)
