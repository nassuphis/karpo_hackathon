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

**OpenSeadragon is already vendored.** The DeepZoom viewer already builds `OpenSeadragon({ tileSources })` (`js/12-deepzoom-boot.js:270`). Reuse the dependency and the boot pattern — no new library.

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

**Treat the manifest as a cached, Refresh-recomputed index — never compute it on every open.** Enumerating all color artifacts across all jobs is the O(jobs × artifacts) S3 crawl that the Results `/list` path deliberately avoids (see the `/render-count` lazy-per-job decision). A naive full `renders/` walk is too broad because it traverses chunks and unrelated artifacts; the targeted scan shape is job-prefix first, then `renders/<job>/color/`. So:

1. **Compute (on Refresh only).** List job prefixes under `renders/` (`Delimiter="/"` → `CommonPrefixes`). For each job, list only `renders/<job>/color/` artifact prefixes and apply the same validity rule as the Render tab: a displayable color artifact must have an `image.jpeg` or `image.png`, and the preferred tile is `preview.png` if present. Reuse/refactor `_list_render_family_variants(job_id, "color")` and `_legacy_render_variant(job_id, "color")` rather than writing a second artifact-validity rule. If necessary, add a `presign=False` option so the mosaic builder returns keys without generating throwaway presigned URLs.
2. **Capture preview dimensions during Refresh.** Read only the PNG header (`Range: bytes=0-32`) for each `preview.png` and store `preview_width` / `preview_height`. This proved cheap enough for ~1.5k previews and is refresh-only. Do not download image bodies.
3. **Filter safely.** Include square color previews (`512 × 512`, `1024 × 1024`, or future square sizes). Skip or mark non-square previews because a fixed-tile mosaic would distort them. If a color artifact lacks `preview.png`, either omit it from v1 or include a `missing_preview` count; do not use full render images as tiles.
4. **Sort deterministically** (e.g. `created_at` desc, tiebreak `job_id` + `artifact_id`) so tiles do not reshuffle between refreshes — the mosaic index `y*cols + x` must be stable.
5. **Cache** the manifest to a public object: `renders/_index/color_mosaic.json`.

**The display path needs no Lambda.** Because the manifest is itself a public S3 object, the viewer fetches `https://polypaint.s3.us-east-1.amazonaws.com/renders/_index/color_mosaic.json` directly, optionally with a cache-buster. Only the **Refresh** button calls a Lambda, which recomputes and overwrites it.

**Endpoint:** `POST /list-color-mosaic` on the storage lambda, placed near `/render-summary` / `/render-count`. `{ "refresh": true }` recomputes + re-caches and returns the manifest. Without `refresh`, it may return the cached manifest if present, but the normal tab-open path should fetch the public manifest object directly. Add `/list-color-mosaic` to `deploy_manifest.json` storage routes and regenerate the contract with `.venv/bin/python api_manifest.py --write`.

**Manifest shape (key-only, Option A):**

```json
{
  "computed_at": "2026-06-23T00:00:00Z",
  "base": "https://polypaint.s3.us-east-1.amazonaws.com/",
  "tile_size": 512,
  "count": 18742,
  "skipped_non_square": 0,
  "skipped_missing_preview": 0,
  "tiles": [
    {
      "key": "renders/<job>/color/<artifact>/preview.png",
      "job_id": "compute_...",
      "artifact_id": "...",
      "created_at": "...",
      "preview_width": 512,
      "preview_height": 512,
      "image_key": "renders/<job>/color/<artifact>/image.jpeg"
    }
  ]
}
```

The client computes `cols = Math.ceil(Math.sqrt(count))` and `rows = Math.ceil(count / cols)`; `getTileUrl(level, x, y)` returns `base + encodeURI(tiles[y*cols + x].key)`, or `blank.jpg` / a tiny local data-URI tile for out-of-range indices (§7). Use `tileSize = manifest.tile_size || 512`; the physical PNG may be 512 or 1024, but the virtual tile is 512.

**Viewer module:** new `js/NN-allrenders.js`, single-level custom tile source (§5 / §15: `minLevel: 0`, `maxLevel: 0`, `tileSize: 512`, `tileOverlap: 0`, `minZoomImageRatio`, `maxZoomPixelRatio`). Constrain min zoom so a fully zoomed-out wall does not request thousands of tiles at once; front with CloudFront if a large wall feels sluggish (objects are public, so this is a drop-in — §14).

**Click-to-open (§17):** map click → `index` → `tiles[index]` → open that artifact in the Render tab (`job_id` / `artifact_id`), or DeepZoom the full image via the existing viewer.

**Shareable:** since the bucket is already a public website (and the DeepZoom share-link pattern exists), the AllRenders wall can also be a standalone public page, not only an in-app tab.

**Tests/gates:**

- Storage unit test: fake S3 with 512, 1024, non-square, missing-preview, and missing-image artifacts; assert only square displayable color previews enter the manifest.
- Manifest determinism test: same fixtures in different S3 listing order produce identical `tiles` order.
- API route contract: `/list-color-mosaic` appears in `deploy_manifest.json` and regenerated `api_manifest.json`.
- Frontend harness: AllRenders tab exists; Refresh calls `/list-color-mosaic`; OSD tile source maps `x,y` to `tiles[y * cols + x]`; click maps back to the same tile entry.
