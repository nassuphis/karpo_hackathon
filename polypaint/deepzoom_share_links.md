# DeepZoom Share Links

Status: implemented.

This document describes the current standalone DeepZoom share-link feature.

## Current Behavior

Each DeepZoom export writes a standalone share page:

- `deepzoom/{job_id}/{export_id}/viewer.html`

The share page is independent from the main app.

It:

- loads OpenSeadragon from a CDN
- opens the export’s own `image.dzi`
- renders a full-page viewer
- includes a small header with job/export information

## Output Files

Each export now includes:

- `deepzoom/{job_id}/{export_id}/image.dzi`
- `deepzoom/{job_id}/{export_id}/image_files/...`
- `deepzoom/{job_id}/{export_id}/viewer.html`
- `deepzoom/{job_id}/{export_id}/meta.json`

`meta.json` includes:

- `share_url`

`renders/{job_id}/deepzoom_latest.json` also includes:

- `share_url`

## Viewer Implementation

Viewer template:

- [lambda/deepzoom_viewer_template.html](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/deepzoom_viewer_template.html)

Key current implementation details:

- `tileSources: 'image.dzi'`
- relative path usage for the exported descriptor
- page layout gives the viewer a real height
- standalone page does not depend on `index.html`

Important historical fix:

- the viewer used to collapse to a zero-height render area
- current template uses a layout that keeps the viewer container non-zero-height

## Upload Order

Implementation detail that matters:

- `viewer.html` is uploaded before `meta.json`

Reason:

- `meta.json` advertises `share_url`
- it must not advertise a `viewer.html` object that does not exist yet

## Inventory Loading

The DeepZoom tab now loads inventory server-side through:

- `POST /list-deepzoom`

This replaced the older browser-side multi-request inventory build.

## UI Contract

DeepZoom inventory rows now show a `Share` column.

When `share_url` exists, the row renders:

- `Open`

The link uses:

- `target="_blank"`
- `rel="noopener noreferrer"`

When `share_url` is missing, the row shows a placeholder instead of an active link.

## Scope

The feature applies only to newly generated exports.

Old exports are not backfilled.
Regenerating DeepZoom is the migration path for old jobs.

## Tests

Current coverage lives in:

- [tests/test_pipeline.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pipeline.py)
- [tests/e2e/deepzoom-inventory.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/deepzoom-inventory.spec.js)

Those tests cover:

- `viewer.html` upload
- `share_url` in manifests
- `tileSources: 'image.dzi'`
- inventory rendering of `Open` links
- `noopener noreferrer`
