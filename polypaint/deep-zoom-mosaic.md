<!-- LEGACY: superseded by deep-zoom-palette-modaic.md and the generic AllCol/AllPal artifact mosaic implementation. -->
# Deep Zoom Thumbnail Mosaic with OpenSeadragon and S3

This note describes how to display a large collection of square, same-sized thumbnails as a single pan/zoom mosaic using OpenSeadragon. The thumbnails are treated as if they were tiles in one giant virtual image.

This is not a normal Deep Zoom Image pyramid. It is a deliberately simple, single-resolution tile source:

- every thumbnail is one tile;
- all thumbnails are the same square size;
- the viewer can pan around the mosaic;
- zooming can be constrained so that users cannot zoom out to a tiny overview or zoom in beyond the native thumbnail resolution;
- the image files can be hosted on Amazon S3.

## 0. PolyPaint application: the AllRenders tab (verified against the codebase)

This note backs an **AllRenders** tab — one pan/zoom wall of every color render. The facts below are confirmed against the current tree and **override the generic guidance in §10–§13 for PolyPaint**.

**The single-tile-source hack works here because renders are square.** Treating each thumbnail as one tile distorts non-square images. PolyPaint is safe: the render plan rejects `width`/`height` and emits only square output (`lambda/handler_render_plan.py:338` — *"render plan no longer accepts width/height; pass pix for square output"*; `view_mode == "square"`, `square_extent` default 2.0). Previews are aspect-preserving (`vipsthumbnail -s {N}x{N}`), so a square source yields a square thumbnail, and OSD scales each into the fixed `tileSize` slot with no distortion.

**OpenSeadragon is already loaded by the app.** `index.html` loads OpenSeadragon 4.1.1 from cdnjs before the split app scripts, and the DeepZoom viewer already builds `OpenSeadragon({ tileSources })` (`js/12-deepzoom-boot.js:270`). Reuse the global `OpenSeadragon` dependency and the boot pattern — no new library and no vendored copy.

**The bucket is already fully public-read — use stable URLs, not presigned.** `deploy.sh:131-145` installs a bucket policy allowing anonymous `s3:GetObject` on `arn:aws:s3:::polypaint/*`, with Block-Public-Access fully off (`deploy.sh:125-128`). Consequences:

- Every color thumbnail already has a stable public URL:
  `https://polypaint.s3.us-east-1.amazonaws.com/renders/<job>/color/<artifact>/preview.png`
- Use **Option A** (§3): a key-only manifest plus one base URL. No presigned URLs, no 1-hour expiry, no ~1.5 KB-per-entry bloat. A wall left open all day keeps working.
- Use the **HTTPS REST endpoint** (`…s3.us-east-1.amazonaws.com/…`, the form `deploy.sh:402/1985` already use), **not** the `s3-website-…` website endpoint (HTTP-only → mixed content from the HTTPS app).
- **CORS:** `deploy.sh` installs no `put-bucket-cors`; plain tile display does not need it. Add a `GET`/`HEAD` rule only if OSD canvas operations report tainted-canvas errors.

**"Public" already includes the recipe, not just the pictures.** The `polypaint/*` policy makes everything under `renders/<job>/` anonymously readable — including `calc.json` and `color/<id>/meta.json`, which carry the param/coeff/solve-score **program source** (`handler_compute_orchestrator.py:93-96`), plus raw root data (`lores.bin`, `chunk_*.bin`, `coeffs_*.bin`), bilevel TIFs, PDFs, and deepzoom tiles. Fine for a single-user art project — just a conscious note that the *algorithms* are public, not only the renders. (Separately, per the `deploy.sh:120-123` comment, the HTTP API is unauthenticated **including the `/delete*` routes** — the real exposure to revisit before sharing the stack; unrelated to the mosaic.)

**Existing color previews are square, but not all the same physical pixel size.** Current production color artifacts create `preview.png` through `render_score_raw(...)`, which passes `--preview_max=512` (`lambda/raw_score_render.py:115`) into the native renderer; the native path preserves aspect ratio and downsizes the long edge (`lambda/score_raw_render.c:508-520`). Since color renders are square, new color previews are `512 × 512`. Autolevel-derived color artifacts also use `vipsthumbnail -s 512x512` (`lambda/handler_autolevels.py:276`).

**Bucket scan, 2026-06-23:** a targeted PNG-header scan of public S3 objects found:

- `renders/<job>/color/<artifact>/preview.png`: 1,483 total.
- Color previews at `512 × 512`: 728.
- Color previews at `1024 × 1024`: 755.
- `1024 × 1024` color previews are older `color_run_*` and `color_repalette_*` artifacts, not a separate root-render class.
- Newest 30 color previews by S3 `LastModified` were all `512 × 512`.
- Example `compute_mqlacwaq` color previews were both `512 × 512`.
- Palette previews under `renders/<job>/palettes/<palette>/preview.png`: 960 total; 956 are `512 × 512`, 4 are `500 × 500`.

**AllRenders v1 should not regenerate thumbnails.** Use the existing square `preview.png` files and set the *virtual* OSD tile size to `512`. Existing `1024 × 1024` previews will be scaled down by the browser into a 512 logical tile. Include actual `preview_width` / `preview_height` in the cached manifest for audit/debugging. Accept square previews; skip or flag any future non-square preview because it would distort in a fixed tile slot.

## 1. Basic idea

Assume you have `N` square thumbnails, all `tileSize × tileSize` pixels.

For example:

```text
thumb_000001.jpg
thumb_000002.jpg
thumb_000003.jpg
...
```

Choose how many thumbnails you want per row:

```js
const tileSize = 160;
const cols = 100;
const rows = Math.ceil(files.length / cols);
```

The virtual mosaic then has dimensions:

```js
const mosaicWidth  = cols * tileSize;
const mosaicHeight = rows * tileSize;
```

OpenSeadragon thinks this is one huge image. In reality, each tile request is mapped directly to one thumbnail.

The tile coordinate `(x, y)` maps to the thumbnail index:

```js
const index = y * cols + x;
```

So tile `(0, 0)` is the first thumbnail, tile `(1, 0)` is the second, tile `(0, 1)` is the first thumbnail on the second row, and so on.

## 2. Directory layout

A simple static layout:

```text
site/
  index.html
  manifest.json
  blank.jpg
  openseadragon/
    openseadragon.min.js
    images/
      home_rest.png
      home_grouphover.png
      ...
```

Your thumbnails may live separately on S3:

```text
s3://my-bucket/thumbs/thumb_000001.jpg
s3://my-bucket/thumbs/thumb_000002.jpg
s3://my-bucket/thumbs/thumb_000003.jpg
...
```

They do not need to be in the same bucket as the HTML page, provided browser access and CORS are configured correctly.

## 3. Manifest file

The viewer needs a list of thumbnail filenames or URLs.

### Option A: manifest contains filenames

Use this if all thumbnails share a common base URL:

```json
[
  "thumb_000001.jpg",
  "thumb_000002.jpg",
  "thumb_000003.jpg"
]
```

Then the JavaScript can prepend the S3 base URL.

### Option B: manifest contains full URLs

Use this if thumbnails are spread across prefixes, buckets, or signed URLs:

```json
[
  "https://my-bucket.s3.eu-central-1.amazonaws.com/thumbs/thumb_000001.jpg",
  "https://my-bucket.s3.eu-central-1.amazonaws.com/thumbs/thumb_000002.jpg",
  "https://my-bucket.s3.eu-central-1.amazonaws.com/thumbs/thumb_000003.jpg"
]
```

This is more explicit but can make the manifest large.

## 4. Generate a manifest from local files

If the thumbnails are local before upload:

```bash
find thumbs -maxdepth 1 -type f \( -name '*.jpg' -o -name '*.png' -o -name '*.webp' \) \
  | sort \
  | sed 's|thumbs/||' \
  | jq -R . \
  | jq -s . > manifest.json
```

If your files are already on S3, you can generate a manifest using the AWS CLI:

```bash
aws s3 ls s3://my-bucket/thumbs/ \
  | awk '{print $4}' \
  | sort \
  | jq -R . \
  | jq -s . > manifest.json
```

For recursive prefixes:

```bash
aws s3 ls s3://my-bucket/thumbs/ --recursive \
  | awk '{print $4}' \
  | sort \
  | jq -R . \
  | jq -s . > manifest.json
```

If using recursive output, the filenames may include the prefix, for example:

```json
[
  "thumbs/a/thumb_000001.jpg",
  "thumbs/a/thumb_000002.jpg",
  "thumbs/b/thumb_000003.jpg"
]
```

In that case, set the base URL to the bucket root rather than the `thumbs/` prefix.

## 5. Minimal OpenSeadragon viewer

This version assumes:

- the thumbnails are all square;
- the manifest contains filenames only;
- the images are public S3 objects;
- all images live under one S3 prefix.

```html
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <title>Deep Zoom Thumbnail Mosaic</title>
  <style>
    html, body {
      margin: 0;
      width: 100%;
      height: 100%;
      overflow: hidden;
      background: #111;
    }

    #viewer {
      width: 100vw;
      height: 100vh;
    }
  </style>
</head>
<body>
  <div id="viewer"></div>

  <script src="openseadragon/openseadragon.min.js"></script>
  <script>
    const tileSize = 160;
    const cols = 100;

    // Change this to your own bucket / region / prefix.
    const s3BaseUrl = "https://my-bucket.s3.eu-central-1.amazonaws.com/thumbs/";

    fetch("manifest.json")
      .then(r => r.json())
      .then(files => {
        const rows = Math.ceil(files.length / cols);

        const viewer = OpenSeadragon({
          id: "viewer",
          prefixUrl: "openseadragon/images/",

          showNavigator: true,
          animationTime: 0.15,
          blendTime: 0,

          constrainDuringPan: true,
          visibilityRatio: 1.0,

          // Do not allow the whole mosaic to shrink too far.
          minZoomImageRatio: 1.0,

          // Do not zoom past native thumbnail resolution.
          maxZoomPixelRatio: 1.0,

          tileSources: {
            width: cols * tileSize,
            height: rows * tileSize,
            tileSize: tileSize,
            tileOverlap: 0,

            // Single-level tile source. No real image pyramid.
            minLevel: 0,
            maxLevel: 0,

            getTileUrl: function(level, x, y) {
              const index = y * cols + x;

              if (
                x < 0 ||
                y < 0 ||
                x >= cols ||
                y >= rows ||
                index >= files.length
              ) {
                return "blank.jpg";
              }

              return s3BaseUrl + files[index];
            }
          }
        });
      });
  </script>
</body>
</html>
```

## 6. Version with full S3 URLs in the manifest

If `manifest.json` contains full URLs, simplify `getTileUrl`:

```js
getTileUrl: function(level, x, y) {
  const index = y * cols + x;

  if (
    x < 0 ||
    y < 0 ||
    x >= cols ||
    y >= rows ||
    index >= files.length
  ) {
    return "blank.jpg";
  }

  return files[index];
}
```

This is useful if some URLs are CloudFront URLs, some are S3 URLs, or some are pre-signed URLs.

## 7. Blank tile

The last row of the mosaic may not be completely filled. OpenSeadragon may request tile coordinates that do not correspond to a real thumbnail. Return a blank image for those cases.

Create a blank tile with ImageMagick:

```bash
magick -size 160x160 xc:white blank.jpg
```

Use the same dimensions as your thumbnails.

If you want a dark background:

```bash
magick -size 160x160 xc:'#111111' blank.jpg
```

## 8. Is S3 an issue?

S3 is not a problem. It is actually a good fit because this design is just static HTTP image serving.

The only things to watch are:

1. **Object access**: the browser must be allowed to fetch the thumbnail objects.
2. **CORS**: if your HTML page is served from a different origin than the bucket, S3 may need a CORS rule.
3. **Caching**: set cache headers so that thumbnails are not repeatedly downloaded.
4. **Request volume**: OpenSeadragon loads many small files; for very large mosaics, CloudFront may be useful.
5. **Signed URLs**: if objects are private and URLs expire, the manifest must be refreshed when the URLs expire.

## 9. Public S3 objects

If the thumbnails are public, each file should be directly accessible by URL, for example:

```text
https://my-bucket.s3.eu-central-1.amazonaws.com/thumbs/thumb_000001.jpg
```

Opening that URL in a browser should show the image.

If this works, OpenSeadragon can usually load the image.

## 10. CORS configuration for S3

If the viewer HTML is served from another domain, for example:

```text
https://my-site.com/index.html
```

and the thumbnails are loaded from:

```text
https://my-bucket.s3.eu-central-1.amazonaws.com/thumbs/...
```

then this is a cross-origin request.

Image tags often load cross-origin images without trouble, but CORS becomes important if the viewer, browser, canvas, plugins, or any processing code needs proper cross-origin permission. It is safer to configure CORS on the bucket.

A permissive read-only CORS configuration:

```json
[
  {
    "AllowedHeaders": ["*"],
    "AllowedMethods": ["GET", "HEAD"],
    "AllowedOrigins": ["*"],
    "ExposeHeaders": ["ETag"],
    "MaxAgeSeconds": 3000
  }
]
```

A stricter version for a known site:

```json
[
  {
    "AllowedHeaders": ["*"],
    "AllowedMethods": ["GET", "HEAD"],
    "AllowedOrigins": ["https://my-site.com"],
    "ExposeHeaders": ["ETag"],
    "MaxAgeSeconds": 3000
  }
]
```

In the S3 console:

1. Open the bucket.
2. Go to **Permissions**.
3. Find **Cross-origin resource sharing (CORS)**.
4. Paste the JSON configuration.
5. Save.

S3 CORS rules do not override normal permissions. The object still needs to be public, or the request still needs valid authorization.

## 11. Bucket policy for public read access

If this is a public thumbnail bucket, the bucket policy might look like this:

```json
{
  "Version": "2012-10-17",
  "Statement": [
    {
      "Sid": "PublicReadThumbs",
      "Effect": "Allow",
      "Principal": "*",
      "Action": "s3:GetObject",
      "Resource": "arn:aws:s3:::my-bucket/thumbs/*"
    }
  ]
}
```

This allows public reads only for objects under `thumbs/`.

Whether this works also depends on the bucket's **Block Public Access** settings. If public access is blocked at the account or bucket level, the bucket policy will not make the files publicly readable.

## 12. Private buckets and pre-signed URLs

> **PolyPaint note:** Not applicable. The `polypaint` bucket is already public-read (`deploy.sh:131-145`), so the AllRenders tab uses stable public URLs (Option A, §3) with no presigning and no expiry. This section matters only if the bucket is later locked down.

If the thumbnails are private, you have two common choices.

### Option A: pre-signed URLs in the manifest

Generate a manifest containing temporary signed URLs:

```json
[
  "https://my-bucket.s3.eu-central-1.amazonaws.com/thumbs/thumb_000001.jpg?X-Amz-Algorithm=...",
  "https://my-bucket.s3.eu-central-1.amazonaws.com/thumbs/thumb_000002.jpg?X-Amz-Algorithm=..."
]
```

Then `getTileUrl` returns `files[index]` directly.

The drawback is that the viewer breaks when the URLs expire. You need to regenerate the manifest or fetch signed URLs dynamically.

### Option B: serve through CloudFront

Use CloudFront in front of S3. For private content, use signed CloudFront URLs or signed cookies. This is usually cleaner for a serious browser-based gallery.

## 13. Cache headers

For thumbnails that do not change, set long cache headers:

```bash
aws s3 cp thumbs/ s3://my-bucket/thumbs/ \
  --recursive \
  --cache-control "public, max-age=31536000, immutable"
```

For the manifest, use a shorter cache time, because it may change:

```bash
aws s3 cp manifest.json s3://my-bucket/mosaic/manifest.json \
  --cache-control "public, max-age=60"
```

This prevents the browser from repeatedly downloading the same thumbnails while still allowing the manifest to update quickly.

## 14. Request volume and CloudFront

OpenSeadragon loads only visible tiles plus a buffer, so it will not fetch every thumbnail at once. Still, panning quickly over a huge mosaic can generate many small HTTP requests.

For a small private project, direct S3 access is fine.

For a public or high-traffic project, use CloudFront:

```js
const s3BaseUrl = "https://d123example.cloudfront.net/thumbs/";
```

CloudFront helps with:

- lower latency;
- better edge caching;
- less direct load on S3;
- cleaner custom domains;
- signed cookies / signed URLs if needed.

## 15. Controlling zoom behavior

This project is not trying to build a full deep zoom pyramid. The thumbnails themselves are already the tile grid.

The important options are:

```js
minLevel: 0,
maxLevel: 0,
minZoomImageRatio: 1.0,
maxZoomPixelRatio: 1.0,
visibilityRatio: 1.0,
constrainDuringPan: true
```

Meaning:

- `minLevel: 0` and `maxLevel: 0` make this a single-level tile source.
- `minZoomImageRatio: 1.0` discourages zooming out until the whole mosaic is tiny.
- `maxZoomPixelRatio: 1.0` discourages zooming in beyond the native thumbnail pixels.
- `visibilityRatio: 1.0` and `constrainDuringPan: true` keep the viewport constrained to the image area.

You can loosen these if you want a more free-floating feel.

For example, to allow some zoom-out overview:

```js
minZoomImageRatio: 0.1
```

To allow zooming in beyond native thumbnail resolution:

```js
maxZoomPixelRatio: 2.0
```

## 16. Picking the number of columns

The number of columns controls the shape of the mosaic.

For a roughly square mosaic:

```js
const cols = Math.ceil(Math.sqrt(files.length));
const rows = Math.ceil(files.length / cols);
```

For a wide wall:

```js
const cols = 200;
```

For a tall contact-sheet style layout:

```js
const cols = 20;
```

The grid does not need to match the browser shape. OpenSeadragon handles panning.

## 17. Adding click behavior

You can map a click position back to a thumbnail index.

```js
viewer.addHandler("canvas-click", function(event) {
  const viewportPoint = viewer.viewport.pointFromPixel(event.position);
  const imagePoint = viewer.viewport.viewportToImageCoordinates(viewportPoint);

  const x = Math.floor(imagePoint.x / tileSize);
  const y = Math.floor(imagePoint.y / tileSize);
  const index = y * cols + x;

  if (index >= 0 && index < files.length) {
    console.log("Clicked thumbnail", index, files[index]);
  }
});
```

You could use this to open the original high-resolution image, show metadata, select images, or build a lightbox.

## 18. Common problems

### Images do not load

Check:

- Does the image URL work directly in the browser?
- Is the bucket or prefix public?
- Is Block Public Access preventing public reads?
- Is the S3 URL using the correct region?
- Is CORS configured if needed?
- Are filenames in the manifest URL-encoded correctly?

### The last row shows broken images

Return `blank.jpg` for out-of-range indices.

### The manifest loads but images fail

Open the browser dev tools and inspect the Network tab. Look for:

- `403 Forbidden`: permissions problem;
- `404 Not Found`: wrong filename, prefix, or URL encoding;
- CORS error: bucket CORS rule problem;
- mixed content error: trying to load `http://` images from an `https://` page.

### It is slow

Try:

- smaller thumbnails;
- WebP instead of JPEG/PNG;
- CloudFront in front of S3;
- long cache headers;
- fewer columns if too many tiles are visible at once;
- avoid very tiny thumbnails, because that increases the number of HTTP requests per screen.

## 19. Practical recommended setup

For a clean implementation:

1. Normalize all thumbnails to the same square size, for example `160 × 160` or `256 × 256`.
2. Upload thumbnails to S3 under a stable prefix, for example `s3://my-bucket/thumbs/`.
3. Generate `manifest.json` in deterministic sorted order.
4. Serve the viewer HTML either from the same S3 bucket, another static host, or CloudFront.
5. Add S3 CORS allowing `GET` and `HEAD` from the viewer origin.
6. Put CloudFront in front if the project is public or large.
7. Set long cache headers on thumbnails and short cache headers on the manifest.
8. Use OpenSeadragon with a custom single-level tile source.

## 20. References

- OpenSeadragon custom tile sources: https://openseadragon.github.io/examples/tilesource-custom/
- OpenSeadragon advanced custom tile sources: https://openseadragon.github.io/examples/tilesource-custom-advanced/
- Amazon S3 CORS overview: https://docs.aws.amazon.com/AmazonS3/latest/userguide/cors.html
- Amazon S3 CORS configuration elements: https://docs.aws.amazon.com/AmazonS3/latest/userguide/ManageCorsUsing.html
- Amazon S3 CORS examples: https://docs.aws.amazon.com/AmazonS3/latest/userguide/enabling-cors-examples.html

## 21. PolyPaint implementation plan

**Treat the manifest as a cached, Refresh-recomputed index — never compute it on every open and never compute it synchronously behind the browser request.** Enumerating all color artifacts across all jobs is the O(jobs × artifacts) S3 crawl that the Results `/list` path deliberately avoids (see the `/render-count` lazy-per-job decision). The Results tab parallelizes one `calc.json` read per job; AllRenders has a deeper fanout: job prefixes, color-artifact prefixes, key HEADs, and preview-dimension checks per artifact. Use the Results parallelism model inside the builder, but run the builder asynchronously so API Gateway never waits for the crawl.

### 21.1 Refresh contract: async DDB status, not synchronous crawl

`POST /list-color-mosaic` is a control endpoint on the storage Lambda, placed near `/render-summary` / `/render-count`.

- `{ "refresh": true }` conditionally acquires a DynamoDB refresh row, starts the background crawl, and returns immediately.
- `{ "refresh": false }` or an empty body returns the current DynamoDB status. This is a cheap status read, not a crawl.
- The browser polls `/list-color-mosaic` every ~2s while `state:"computing"`.
- On `state:"ready"`, the browser fetches the public S3 manifest from `status.manifest_url`.
- On `state:"error"`, the tab shows the error and keeps the last loaded manifest usable. If there is no loaded manifest but `last_ready_manifest_url` exists, offer/load that instead of the failed refresh.

Use DynamoDB, not S3, as the authoritative status/ownership mechanism. S3 object writes are not a reliable compare-and-swap lock, so a stale worker can otherwise overwrite a newer status or manifest. The existing `JOBS_TABLE` is available to storage handlers through `_get_ddb()`.

DDB key:

```text
job_id  = "__allrenders_mosaic__"
task_id = "color_mosaic_status"
```

Use low-level DynamoDB client calls, matching existing storage code. The storage Lambda role currently grants `dynamodb:PutItem`, `GetItem`, `DeleteItem`, `Query`, and `BatchWriteItem`, but not `UpdateItem` (`deploy.sh` DDB policy). Therefore every transition must use conditional full-row `put_item`; do not use `update_item` unless the IAM policy is deliberately changed. This matches the existing favorites conditional-write precedent in `handler_storage.py`.

```python
ddb.put_item(
    TableName=JOBS_TABLE,
    Item=full_status_item,
    ConditionExpression=(
        "attribute_not_exists(job_id) OR "
        "#state <> :computing OR "
        "updated_at_ms < :stale_before"
    ),
    ExpressionAttributeNames={"#state": "state"},
    ExpressionAttributeValues={
        ":computing": {"S": "computing"},
        ":stale_before": {"N": str(stale_before_ms)},
    },
)
```

Acquire rule for Refresh:

- Generate `refresh_id = "mosaic_<utc>_<short-random>"`.
- Read the existing row once with `GetItem` so the new full-row item can preserve any `last_ready_*` fields.
- `put_item` the full DDB row to `state:"computing"` with a condition:
  - row does not exist, or
  - `state` is not `computing`, or
  - `updated_at_ms < now_ms - STALE_MS`.
- If the conditional write fails, return the existing non-stale `computing` status and do not start another worker.
- Use a conservative stale threshold, e.g. 30 minutes.

Publish rule for the worker:

- Write the manifest to a refresh-specific key, not a stable alias:
  `renders/_index/color_mosaic/<refresh_id>/all.json`.
- Then `put_item` the full ready row with condition `refresh_id = :refresh_id AND #state = :computing`.
- If that conditional put fails, the worker is stale; exit without publishing a public "ready" status. The refresh-specific manifest it wrote is harmless garbage.
- On exception, `put_item` the full error row with the same owner condition.
- Optionally mirror the current DDB status to `renders/_index/color_mosaic_status.json` for debugging/shareability, but the UI must treat DDB/API status as authoritative.
- Preserve the last good manifest across failed refreshes. When acquiring a new `computing` row, copy the previous ready manifest fields into `last_ready_manifest_key`, `last_ready_manifest_url`, `last_ready_completed_at`, and `last_ready_count`. On successful ready, set both the current `manifest_*` fields and the `last_ready_*` fields. On error, leave current `manifest_key` / `manifest_url` empty for the failed refresh but keep the `last_ready_*` fields.

Route response shapes:

- Missing state: `{"schema_version":1,"state":"missing","manifest_key":"","manifest_url":"","count":0}`.
- Computing: return the DDB status row, including `refresh_id`, `started_at`, `updated_at`, and any partial counts if later added.
- Ready: return the DDB status row with `manifest_key`, `manifest_url`, counts, and skipped counts.
- Error: return the DDB status row with `error`, empty current `manifest_key` / `manifest_url`, and populated `last_ready_*` fields if a previous ready manifest exists. The frontend may keep showing the loaded manifest or offer "Load last ready"; it must not fetch a manifest from the failed refresh.

Status vocabulary:

- Existing task status rows use `task_status`, `error_msg`, `result_data`, and `updated_at_ms` (for example the `/check-status` path queries those fields). Before implementing, either reuse that vocabulary for this singleton row or deliberately keep the simpler `state` / `error` / top-level manifest fields documented here.
- If keeping `state`, do it because this is not a chunk/task row and will be consumed only by `/list-color-mosaic`; add a small conversion helper so the frontend never sees DynamoDB typed attributes.
- Do not accidentally mix both forms in one row.

Use storage self-invocation for the background job. The deployed Lambda role already has `lambda:InvokeFunction` on `polypaint-*` (`deploy.sh` installs `polypaint-lambda-invoke`), so the storage handler can invoke its own function asynchronously:

```python
lambda_client.invoke(
    FunctionName=os.environ["AWS_LAMBDA_FUNCTION_NAME"],
    InvocationType="Event",
    Payload=json.dumps({
        "internal_action": "build_color_mosaic",
        "refresh_id": refresh_id,
    }).encode("utf-8"),
)
```

Do not use `context.function_name` in the route handler. `handler_storage._handle_storage_route` currently calls route functions as `fn(event)`, so `context` is not threaded into storage route handlers. The Lambda runtime always provides `AWS_LAMBDA_FUNCTION_NAME`; use that environment variable. At the top of `handler_storage.handler`, check `event.get("internal_action") == "build_color_mosaic"` before route dispatch and call the worker directly. This avoids a new Lambda, avoids a Step Functions workflow for a UI index rebuild, and avoids routing the crawl through API Gateway.

Status object shape returned by `/list-color-mosaic`:

```json
{
  "schema_version": 1,
  "state": "ready",
  "refresh_id": "mosaic_20260623T000000Z_ab12cd",
  "started_at": "2026-06-23T00:00:00Z",
  "updated_at": "2026-06-23T00:00:10Z",
  "completed_at": "2026-06-23T00:00:10Z",
  "manifest_key": "renders/_index/color_mosaic/mosaic_20260623T000000Z_ab12cd/all.json",
  "manifest_url": "https://polypaint.s3.us-east-1.amazonaws.com/renders/_index/color_mosaic/mosaic_20260623T000000Z_ab12cd/all.json",
  "last_ready_manifest_key": "renders/_index/color_mosaic/mosaic_20260623T000000Z_ab12cd/all.json",
  "last_ready_manifest_url": "https://polypaint.s3.us-east-1.amazonaws.com/renders/_index/color_mosaic/mosaic_20260623T000000Z_ab12cd/all.json",
  "last_ready_completed_at": "2026-06-23T00:00:10Z",
  "last_ready_count": 1483,
  "count": 1483,
  "source_counts": {"512x512": 728, "1024x1024": 755, "unknown": 0},
  "skipped_non_square": 0,
  "skipped_missing_preview": 0,
  "skipped_missing_image": 0,
  "skipped_legacy": 0,
  "unknown_dimensions": 0,
  "error": ""
}
```

This is intentionally DDB-backed even though the manifest is public S3. Polling a tiny DDB status row through the API is cheap and avoids stale public-status cache/race bugs.

Async invocation retries:

- Lambda async `InvocationType:"Event"` can retry the same payload after an unhandled worker error. The design is idempotent: refresh-specific manifest keys plus conditional DDB `put_item` mean duplicate workers can waste work but cannot publish stale status.
- Prefer catching worker exceptions and writing `state:"error"` so the invocation completes successfully. If duplicate retries become noisy, explicitly configure async max retries for `polypaint-storage` to `0` in deploy, but that is not required for correctness.

### 21.2 Backend builder

Targeted scan shape:

1. List job prefixes under `renders/` using `Delimiter="/"`.
2. For each job, list only `renders/<job>/color/` artifact prefixes.
3. Apply the same validity rule as the Render tab: a displayable immutable color artifact has `image.jpeg` or `image.png`, and the preferred tile is `preview.png` if present.
4. Reuse/refactor `_list_render_family_variants(job_id, "color")` and `_legacy_render_variant(job_id, "color")` rather than writing a second artifact-validity rule.
5. Thread a required `presign=False` option through `_list_render_family_variants`, `_legacy_render_variant`, and `_head_artifact_keys`. Today `_list_render_family_variants` hardcodes `presign=True`; AllRenders must not mint throwaway presigned URLs because the bucket is public and the manifest stores keys.
6. Use a pooled S3 client like `_results_list_s3_client(max_workers)`. Avoid nested `ThreadPoolExecutor`s: first build a flattened `(job_id, artifact_prefix)` work-list, then process that list through one bounded pool. Nested job-level plus artifact-level pools multiply thread counts and make S3 throttling harder to reason about.

Concrete helper split:

- `_render_family_artifact_prefixes(job_id, family, *, s3_client=s3) -> list[str]`: lists `renders/<job>/<family-dir>/` prefixes.
- `_render_family_entry_from_prefix(job_id, family, prefix, *, presign=True, s3_client=s3) -> dict | None`: contains the current image/preview HEAD + `_render_artifact_entry` logic.
- `_list_render_family_variants(job_id, family, *, presign=True, s3_client=s3)` becomes a thin wrapper around those helpers for existing Render summary behavior.
- The mosaic builder uses the prefix helper to flatten work, then calls `_render_family_entry_from_prefix(..., presign=False, s3_client=list_s3)` in one bounded pool.
- Read `calc.json` once per job and cache `{function, degree, N, times}` by job id before enriching tile entries. Do not reread `calc.json` per artifact.

Dimension capture:

- Prefer HEAD metadata first. `_head_artifact_keys` already surfaces user metadata width/height when present, and several color paths write it.
- If preview dimensions are absent from HEAD metadata, use a small PNG-header range read (`Range: bytes=0-32`) as fallback.
- PNG fallback is deterministic: validate the PNG signature and read IHDR width/height from bytes 16..23 as big-endian uint32. If the range body is short, not PNG, or IHDR is invalid, treat dimensions as unknown.
- Do not download image bodies.
- Include square color previews (`512 × 512`, `1024 × 1024`, or future square sizes).
- Skip or count non-square previews because a fixed-tile mosaic would distort them.
- If both HEAD metadata and PNG-header fallback fail, include the tile in `All` only with `preview_width:null` / `preview_height:null` if the preview key exists, and exclude it from strict `512` / `1024` filters. Count these in `unknown_dimensions`. Do not guess `512`.
- If a color artifact lacks `preview.png`, count `skipped_missing_preview`; v1 does not use full render images as tiles.

Legacy color scope:

- Immutable artifacts live under `renders/<job>/color/<artifact>/preview.png`; these are v1’s primary scope.
- Legacy root color previews live at `renders/<job>/preview_color.png` through `RENDER_FAMILY_SHAPES["color"].legacy_preview_candidates`.
- Choose explicitly during implementation: either include legacy as `legacy:true` entries, or skip and count `skipped_legacy`. Prefer skip for v1 unless there is a clear need to include pre-artifact renders.

### 21.3 Manifest strategy

Use one canonical manifest shape for v1, stored under a refresh-specific key:

```text
https://polypaint.s3.us-east-1.amazonaws.com/renders/_index/color_mosaic/<refresh_id>/all.json
```

The UI gets the exact `manifest_key` / `manifest_url` from `/list-color-mosaic` status. Do not hardcode a latest manifest URL in the viewer. The `All | 512 | 1024` selector is a client-side filter over this one manifest. This is simpler than writing three manifest objects because every tile already carries `preview_width` / `preview_height`, and switching filters becomes instant. If diagnostic split objects are useful later, they can be added without changing the viewer contract.

**Manifest shape (key-only, Option A):**

```json
{
  "schema_version": 1,
  "computed_at": "2026-06-23T00:00:00Z",
  "base": "https://polypaint.s3.us-east-1.amazonaws.com/",
  "refresh_id": "mosaic_20260623T000000Z_ab12cd",
  "manifest_key": "renders/_index/color_mosaic/mosaic_20260623T000000Z_ab12cd/all.json",
  "manifest_kind": "all",
  "dimension_filter": "all-square",
  "tile_size": 512,
  "count": 1483,
  "source_counts": {
    "512x512": 728,
    "1024x1024": 755,
    "unknown": 0
  },
  "skipped_non_square": 0,
  "skipped_missing_preview": 0,
  "skipped_missing_image": 0,
  "skipped_legacy": 0,
  "unknown_dimensions": 0,
  "tiles": [
    {
      "key": "renders/<job>/color/<artifact>/preview.png",
      "job_id": "compute_...",
      "artifact_id": "...",
      "created_at": "...",
      "function": "giga_39",
      "degree": 39,
      "N": 10000,
      "times": 1,
      "preview_width": 512,
      "preview_height": 512,
      "image_key": "renders/<job>/color/<artifact>/image.jpeg"
    }
  ]
}
```

`function`, `degree`, `N`, and `times` come from `calc.json` and are refresh-only costs. Include them so the UI can arrange/filter without another backend call.

Manifest retention:

- Refresh-specific manifests are immutable-ish public JSON snapshots. They will accumulate unless cleaned.
- After a successful ready publish, prune old prefixes under `renders/_index/color_mosaic/`, keeping at least the current `refresh_id` and the previous `last_ready_*` refresh id. A simple keep-last-N policy, e.g. last 10 manifests by prefix timestamp, is safer for local debugging.
- If pruning is not implemented in v1, add an explicit skipped cleanup note in the status response and create a follow-up. Do not leave retention undefined.
- An S3 lifecycle rule on `renders/_index/color_mosaic/` would also work, but it is an infra/deploy change; code-level pruning is enough for v1.
- Manifest size grows linearly. At current scale the JSON is small; at tens of thousands of tiles it becomes multi-MB and should be sharded or paged. This is a known v1 ceiling alongside the OSD tile-burst ceiling.

### 21.4 Viewer and arrangement

Add a real tab and module:

- `index.html`: add `<div class="tab-btn" onclick="switchTab('allrenders')">AllRenders</div>`. The label lowercased must equal the `switchTab` argument because `switchTab` toggles active buttons with `b.textContent.toLowerCase() === name`.
- `index.html`: add `<div id="tab-allrenders" class="tab-content">...</div>`.
- `js/01-core-compute.js`: add `if (name === 'allrenders') loadAllRenders();` in `switchTab`.
- `index.html`: add `<script src="js/13-allrenders.js"></script>` after `js/12-deepzoom-boot.js`.
- Update the part-consistency check at the bottom of `index.html` to include `13-allrenders`.
- Add `;(window.__ppParts = window.__ppParts || []).push('13-allrenders');` at the end of the new JS part.
- `tests/test_frontend_js.sh` loads JS tags from `index.html`; adding the script tag plus registration is enough for the sequential-load harness. `tests/test_frontend_parts_contract.py` will enforce declaration-only top-level code.
- `loadAllRenders()` must be idempotent because `switchTab('allrenders')` runs on every tab selection. If the loaded manifest `refresh_id` matches the ready status and no refresh is in progress, do not recreate the viewer, refetch the manifest, or restart polling; only resize/reopen when the container or controls changed. If status is `error`, keep the loaded manifest, or fetch `last_ready_manifest_url` only when no manifest is loaded.

Toolbar:

- Refresh button: disabled while `state:"computing"`, shows spinner/status text, starts async refresh via `/list-color-mosaic`.
- Status readout: `ready`, `computing`, `error`, `last refreshed`, `refresh_id`, tile count, skipped counts.
- Size filter: `All | 512 | 1024`. In v1 this is a filter, not a resolution switch. `1024` means older 1024-preview artifacts; they are still rendered into a 512 logical tile.
- Sort mode: `Date`, `Job`, `Function`, `Degree`, `N`, `Random`.
- Column-count control: presets or slider. Default `cols = Math.ceil(Math.sqrt(count))`; user can widen into a contact-sheet or narrow into a tall wall.
- Home/Fit button: `viewer.viewport.goHome()`.

Client arrangement:

- Start from `manifest.tiles`.
- Apply size filter.
- Apply optional text filter, e.g. function/job substring, after v1 if desired.
- Apply sort:
  - Date: `created_at` descending, tiebreak `job_id`, `artifact_id`.
  - Job: group by `job_id`, then `created_at`/`artifact_id`.
  - Function: `function`, then date.
  - Degree/N: numeric, then date.
  - Random: deterministic seeded shuffle per loaded manifest, not `Math.random()` every repaint.
- Recompute `cols` and `rows`.
- Call `viewer.open(newTileSource)` on filter/sort/column changes; do not destroy/recreate the viewer for ordinary rearrangement.

OSD lifecycle:

- Replicate `js/12-deepzoom-boot.js:viewDeepZoom`: set the viewer container visible before creating OpenSeadragon, otherwise OSD can measure a `0 × 0` viewport.
- Use a separate viewer variable and container, e.g. `_allRendersViewer`, not `window._osdViewer`. The DeepZoom module owns `_osdViewer` and destroys/recreates it; sharing that global would make the DeepZoom and AllRenders tabs tear each other down.
- Create the AllRenders viewer once on first `loadAllRenders()` after the tab is active.
- On new manifest/filter/sort/column layout, call `viewer.open(tileSource)`.
- Use a single-level custom tile source: `minLevel:0`, `maxLevel:0`, `tileSize:512`, `tileOverlap:0`.
- Be explicit about the scaling ceiling: with `minLevel:0` / `maxLevel:0`, there is no lower-resolution overview. A fit-all/home view can request every visible tile because the source has only one level. `minZoomImageRatio` does not fix this; it only prevents zooming out past home. For v1, set OpenSeadragon `imageLoaderLimit` to bound concurrent requests and accept the burst at current scale (~1.5k tiles). The real future fix for much larger walls is a true multi-level pyramid or a separately rendered low-resolution overview/background layer.
- Use a 1×1 transparent data URI for out-of-range blank tiles; do not request `blank.jpg`.

Tile-source mapping:

```js
const cols = selectedCols;
const rows = Math.ceil(filteredTiles.length / cols);
const tileSize = 512;

const tileSource = {
  width: cols * tileSize,
  height: rows * tileSize,
  tileSize,
  tileOverlap: 0,
  minLevel: 0,
  maxLevel: 0,
  getTileUrl(level, x, y) {
    const index = y * cols + x;
    const tile = filteredTiles[index];
    return tile ? manifest.base + encodeURI(tile.key) : TRANSPARENT_TILE_URL;
  },
};
```

### 21.5 Click-to-open

Click should select the exact color artifact, not merely the result set. Use the existing Render/Favorites pattern:

```js
await _ensureResultsSelection(tile.job_id);  // or selectResult(tile.job_id) if already loaded
switchTab('render');
await refreshRenderArtifacts(tile.job_id, {
  selectFamily: 'color',
  selectArtifactId: tile.artifact_id,
});
```

This mirrors `goRenderSelectedFavorite()` / `goColorFromPalette()` and uses the already-supported `selectFamily` / `selectArtifactId` path in `refreshRenderArtifacts`.

Add a hover/selected overlay in AllRenders before navigation if it is cheap; it is usability polish, not required for v1.

**Shareable:** since the bucket is already a public website (and the DeepZoom share-link pattern exists), the AllRenders wall can also be a standalone public page, not only an in-app tab.

### 21.6 Deployment and routing

- Add `/list-color-mosaic` to the storage route list in `deploy_manifest.json`.
- Add route dispatch in `lambda/handler_storage.py`.
- Add `os` and a Lambda client in `lambda/handler_storage.py` for storage self-invocation.
- Regenerate `api_manifest.json` with `uv run python api_manifest.py --write` if using the project virtualenv workflow. If the deployment checklist still says `python3 api_manifest.py --write`, update the checklist in the same change so the source of truth does not drift.
- API Gateway routes are created from `deploy_manifest.json` by `deploy.sh`; do not edit route wiring directly in `deploy.sh`.
- Refresh-specific manifest objects live under `renders/_index/color_mosaic/<refresh_id>/` and are read directly by the browser from S3 after `/list-color-mosaic` reports `ready`. Direct public S3 is v1; CloudFront would be a separate infra task, not a drop-in code change.
- Write manifest objects with `ContentType: application/json`. Use `CacheControl: no-cache, max-age=0` unless/until historical manifests get immutable cache policy. If a debug public status mirror is added, also write it with `CacheControl: no-cache, max-age=0`; do not make it the authoritative status source.

### 21.7 Tests/gates

- Storage unit test: fake S3 with 512, 1024, non-square, missing-preview, missing-image, missing-metadata, and optional legacy artifacts; assert only square displayable immutable color previews enter the manifest unless legacy inclusion is deliberately enabled.
- Storage unit test: HEAD metadata dimensions are used when present; PNG range-read fallback is used only when metadata dimensions are absent.
- Storage unit test: unknown preview dimensions are included only in `All`, excluded from strict `512` / `1024`, and counted in `unknown_dimensions`.
- Storage unit test: `/list-color-mosaic {refresh:true}` writes `computing` status, self-invokes asynchronously, and returns immediately without building the manifest inline.
- Storage unit test: DDB transitions use conditional `put_item`, not `update_item`; mocks should fail if `update_item` is called.
- Storage unit test: internal worker writes `renders/_index/color_mosaic/<refresh_id>/all.json` then conditionally flips DDB status to `ready` via full-row `put_item`.
- Storage unit test: stale `computing` can be replaced; non-stale `computing` returns the existing refresh id; older worker cannot `put_item` over a newer refresh id.
- Storage unit test: failed refresh preserves `last_ready_manifest_url` and does not expose the failed refresh's `manifest_url`.
- Storage unit test: stale worker writes a refresh-specific manifest but fails the DDB conditional update; current status remains owned by the newer refresh.
- Storage unit test: successful ready prunes old `renders/_index/color_mosaic/<refresh_id>/` prefixes according to the keep-last-N policy while keeping current and last-ready manifests.
- Regression test: existing `/render-summary` behavior still returns presigned URLs by default and is unchanged by the `_list_render_family_variants(..., presign=False)` refactor.
- Manifest determinism test: same fixtures in different S3 listing order produce identical `tiles` order.
- Manifest schema test: entries include `job_id`, `artifact_id`, `created_at`, `function`, `degree`, `N`, `times`, dimensions, `key`, and `image_key`.
- API route contract: `/list-color-mosaic` appears in `deploy_manifest.json` and regenerated `api_manifest.json`.
- Frontend harness: AllRenders tab exists; `switchTab('allrenders')` loads it; script registration includes `13-allrenders`; Refresh calls `/list-color-mosaic`; status polling uses `/list-color-mosaic` and fetches `status.manifest_url` only after `ready`; selector filters `All | 512 | 1024` in memory; sort/column controls rebuild the tile source with `viewer.open`.
- Frontend harness: `state:"error"` keeps the currently loaded manifest; if none is loaded and `last_ready_manifest_url` exists, it fetches that fallback instead.
- Frontend harness: OSD tile source maps `x,y` to `tiles[y * cols + x]`; out-of-range returns the transparent data URI; click maps back to the same tile entry and calls `refreshRenderArtifacts(... selectFamily:'color', selectArtifactId)`.
- Add any new storage test file to `scripts/predeploy_check.sh` if it is not in an already-gated file. `tests/test_storage_handler.py` is already gated; route contracts are covered by `tests/test_api_route_contracts.py`.

### 21.8 Ordered implementation steps

1. **Storage helper refactor.** In `lambda/handler_storage.py`, thread `presign=False` through `_list_render_family_variants`, `_legacy_render_variant`, and their `_head_artifact_keys` calls. Existing render-summary behavior must stay unchanged by default (`presign=True`).
2. **Dimension helper.** Add a small helper that returns preview dimensions from HEAD metadata first, then PNG range fallback. Keep it independent and unit-test it with fake S3 objects.
3. **Manifest builder core.** Add `_build_color_mosaic_manifest(refresh_id, *, include_legacy=False)` that returns the manifest dict and counts. It should use the same validity helpers as Render summary, enrich entries from `calc.json`, sort deterministically, and never generate presigned URLs.
4. **Status helpers.** Add helpers to serialize/deserialize the singleton DDB row, write `computing` / `ready` / `error` with conditional full-row `put_item`, and check refresh-id ownership before final writes. Do not use `update_item` unless IAM is changed deliberately.
5. **Async route.** Add `handle_list_color_mosaic(event)`. On `{refresh:true}`, read existing status, conditionally `put_item` the new `computing` row, self-invoke the storage Lambda using `os.environ["AWS_LAMBDA_FUNCTION_NAME"]` with `internal_action:"build_color_mosaic"`, and return the status. On no refresh, return current status or a `missing`/`empty` state.
6. **Internal worker path.** At the start of `handler_storage.handler`, handle `event["internal_action"] == "build_color_mosaic"` by calling the worker. The worker writes `renders/_index/color_mosaic/<refresh_id>/all.json`, then conditionally flips DDB status to `ready` with full-row `put_item`; on exception, write `error` only if the refresh id still owns the DDB row.
7. **Manifest retention.** After a successful ready publish, prune old refresh-specific manifest prefixes with a keep-last-N policy, preserving the current and last-ready refresh ids.
8. **Route contracts.** Add `/list-color-mosaic` to `deploy_manifest.json`, regenerate `api_manifest.json`, and add/update route tests if needed.
9. **Frontend skeleton.** Add the AllRenders tab button/panel in `index.html`, add `switchTab('allrenders')`, add `js/13-allrenders.js`, update script tags and the `__ppParts` expected list.
10. **Frontend status/manifest loading.** Implement idempotent `loadAllRenders()`: call `/list-color-mosaic` for status, show empty/computing/ready/error states, poll the same endpoint during computing, fetch `status.manifest_url` on ready, and keep the last loaded manifest usable on refresh errors. If there is no loaded manifest and status has `last_ready_manifest_url`, fetch that as the fallback wall. Guard against duplicate polls/viewer recreation on repeated tab opens.
11. **OSD wall.** Implement the single-level tile source with a dedicated `_allRendersViewer`, create OSD only after the tab is visible, set `imageLoaderLimit`, and use `viewer.open(tileSource)` for filter/sort/column changes.
12. **Controls.** Add Refresh, status readout, `All | 512 | 1024`, sort mode, column-count control, Home/Fit. Make 512/1024 labels clear that they filter by stored preview size.
13. **Click-to-open.** Map click to tile index, highlight/preview the selected tile if cheap, then call `_ensureResultsSelection`, `switchTab('render')`, and `refreshRenderArtifacts(... selectFamily:'color', selectArtifactId)`.
14. **Tests.** Add backend unit tests before frontend tests. Then add frontend harness assertions for tab/module registration, status polling, tile mapping, filters, sort/columns, and click-to-open. Run targeted tests plus `tests/test_frontend_js.sh`.
