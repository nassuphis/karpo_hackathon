# Meditations in Color: Internal Data Extraction Plan

> **STATUS (2026-07-23): investigation COMPLETE — extraction verified against
> the live site.** The site turned out to be the easiest case in the decision
> tree (all data embedded in one static page), so the original phased plan
> collapses to a one-request pipeline. This revision leads with the verified
> findings and the working recipe; the original investigation phases are
> summarized in the appendix with their resolutions.

## 1. Objective

Collect the artist / artwork / palette data behind Meditations in Color
(meditationsincolor.com) as a structured dataset. **Done in principle:** the
full dataset (367 artists, 20,361 works with palettes) has been extracted and
parsed end-to-end; what remains is normalization, validation, and two optional
follow-ups (curated flags, per-work weights).

---

# Verified Findings (probed 2026-07-23)

## Architecture — Outcome 3/5: everything is embedded in one static page

- `GET https://meditationsincolor.com/colorists` → HTTP 200, **9,932,443
  bytes**, ~2.3 s. No cookies, tokens, referer, or special headers required —
  a bare `curl` works.
- The page is **hand-rolled HTML + vanilla JS**. It is NOT Next.js: zero
  `__NEXT_DATA__`, zero `self.__next_f.push`, zero `application/ld+json`.
  There are no API calls, no GraphQL, no fetch/XHR data traffic at all.
- Three inline scripts; the big one is **9,826,484 chars** and begins:

  ```js
  // ── Mock Data ────────────────────────────────
  const ARTISTS = [
  { id: "wassily-kandinsky", name: "Wassily ...
  ```

  The entire archive ships inside this literal. The UI renders client-side
  from it.
- **There is no pagination.** The archive's "load more" is client-side lazy
  *rendering* of already-loaded data (`DIRECTORY_BATCH_SIZE` + an
  IntersectionObserver sentinel). Phase 5 of the original plan is moot.
- No `robots.txt` exists (the URL returns the host's HTML 404 page). Nothing
  is disallowed; no access controls are involved anywhere.
- External JS assets are versioned (`mic-analytics.js?v=20260520-ga-v1`,
  `artist_quotes.js`, `mic-brand-colors.js`, …) — the site is redeployed as
  static files, so **the blob's shape can drift on any redeploy**. Pin the
  fetch date and keep the raw HTML.

## Data model (measured, not inferred)

`const ARTISTS = [...]` — **367 artist records**, each:

```text
id             URL slug ("wassily-kandinsky")
name, dates, nationality, movement, preset
worksScanned, thumbnailIdx, bio, colorQuote
stats          {warmCool, saturation, consistency, density}
palette        artist-level profile palette (hex array)
decades        per-decade aggregates: {decade, colors: [hex...], shares: [0..1 weights]}
works          array of work records
```

Work records — **20,361 total**, fields exactly:

```js
{ title: "Points", date: "1920", source: "WIKIDATA",
  palette: ["#BC9F41", "#292B24", "#9A8A44", "#454833",
            "#552C2F", "#5F5337", "#776A3D", "#155C52"],   // ordered hex
  pageUrl: "http://www.wikidata.org/entity/Q3693176",
  imageUrl: "https://upload.wikimedia.org/.../600px-....jpg" }
```

- **Palette length distribution:** 8 colors for 16,589 works (81%); the rest
  1–12 (`{1:9, 2:17, 3:62, 4:279, 5:583, 6:1072, 7:1376, 8:16589, 9:102,
  10:91, 11:56, 12:125}`). Every work has at least one color.
- **Sources (17):** WIKIDATA, COMMONS, EUROPEANA, GETTY, CMA, MET, RIJKS,
  SMK, NGA, TATE, DIMU, WALTERS, V&A, SMITHSONIAN, NATGALLERY, WHITNEY, AIC.
- Images are hosted externally (Wikimedia/museum hosts). Palette extraction
  needs **no image downloads**.
- The plan's snapshot numbers were stale: **367 artists / 20,361 works** now
  vs 287 / 19,507 in the original text. Treat counts as moving.

## What the desired dataset gets — field by field

| Wanted | Available? | Where |
|---|---|---|
| Artist name / slug | yes | `name` / `id` |
| Artwork title / date | yes | work `title` / `date` |
| Museum / source collection | yes | work `source` (17 codes) |
| Artwork page URL | yes | work `pageUrl` (Wikidata/museum entity) |
| Source image URL | yes | work `imageUrl` (600px thumb) |
| Palette colors | yes | work `palette`, ordered hex |
| **Color proportions / weights** | **NO per work** | only artist-level per-decade `shares`; per-work weights would require recomputing from images (Outcome 6 — not guaranteed identical) |
| Curated-status indicator | not in this page | no `curated` field on works; `DIRECTORY_PALETTE_READING_IDS = ['dominant','diverse','spectrum','accents']` is palette *reading modes*, not curation. The ~459 curated works presumably live on the Drawing System page — one more fetch if needed (open item) |
| Internal artwork identifier | none exists | works carry no id; use `pageUrl` as the natural key (dedup on it), scoped by artist `id` |
| Original metadata | yes | keep the raw slice/HTML |

## The one real technical trap

**The embedded data is JavaScript, not JSON** — keys are unquoted
(`title:`, `palette:`), so `json.loads` fails. The original plan's Step 5
only anticipated `<script type="application/json">`. Parse it either by
evaluating in a node `vm` sandbox (verified below) or with `chompjs` in
Python. This is the only step where the naive approach breaks.

---

# Verified Extraction Recipe (ran successfully 2026-07-23)

```bash
curl -s 'https://meditationsincolor.com/colorists' -o colorists.html
```

Slice the literal out of the biggest inline script:

```python
import re
html = open('colorists.html', encoding='utf-8').read()
scripts = re.findall(r'<script(?![^>]*src)[^>]*>(.*?)</script>', html, re.S)
js = max(scripts, key=len)                      # 9.8 MB data script
start = js.find('const ARTISTS = [')
end = start + re.search(r'^\];', js[start:], re.M).end()
open('artists_slice.js', 'w', encoding='utf-8').write(js[start:end])
```

Evaluate the literal and emit JSON (no DOM references inside the slice, so a
bare vm context suffices):

```js
// node extract.js
const fs = require("fs"), vm = require("vm");
const src = fs.readFileSync("artists_slice.js", "utf8");
const ctx = {}; vm.createContext(ctx);
vm.runInContext(src + "\nglobalThis.__out = ARTISTS;", ctx);
fs.writeFileSync("mic_dataset.json", JSON.stringify(ctx.__out));
```

Measured result: `artists_slice.js` = 9,656,527 chars →
`mic_dataset.json` = 9,258,274 bytes; 367 artists; 20,361 works
(Kandinsky alone: 166 works). Colors match the site by construction —
the page renders from this same literal.

(Python-only alternative: `chompjs.parse_js_object()` on the slice.)

---

# Remaining Work

## Normalize (kept from the original plan, adapted)

Layout:

```text
data/
  raw/colorists.html            # keep the exact fetched page (shape drifts on redeploy)
  raw/artists_slice.js
  normalized/artists.jsonl      # one artist per line, works stripped
  normalized/works.jsonl        # one work per line, + artist id
  normalized/palettes.jsonl     # {artist, pageUrl, position, hex, rgb, weight: null}
```

Rules (unchanged): hex uppercase 6-digit, `#ABC` → `#AABBCC`, rgb 0–255
ints, preserve palette order (do not sort), missing weights stay `null`.
Dedup works on `pageUrl`; keep duplicate titles (same title recurs
legitimately within an artist).

## Validate (updated targets)

- artists = 367, works = 20,361 at fetch time — treat as **at-least**, not
  exact constants; re-derive from the parsed data on every refresh.
- Assert: every work has non-empty `palette`, valid hex; every artist has
  `id` and ≥1 work; `shares` arrays in `decades` sum to ≈1 where present
  (a couple of decade entries lack `shares` — tolerate, don't crash).
- Spot-check ten works across artists against the rendered site.

## Open items (optional)

1. **Curated flag**: fetch the Drawing System page and look for its own
   embedded list; join on `pageUrl`/title.
2. **Per-work weights**: only via recomputing from `imageUrl` images —
   an independent pipeline whose palettes will NOT exactly match the site's.

---

# Responsibility notes (superseding the original throttling section)

- The whole dataset arrives in **one request** — no crawler, no throttling
  problem, nothing resembling load on the site.
- No robots.txt exists; no access controls are circumvented; the data is
  what the site sends every visitor.
- If the extracted dataset is ever **republished** (rather than used
  privately for palette work), add a courtesy attribution to
  meditationsincolor.com — the palettes are their computed contribution on
  top of public-domain artworks.

---

# Appendix: original phased plan → resolution

| Original phase | Resolution |
|---|---|
| P1 Understand data flow (DevTools) | Done — no Fetch/XHR data traffic exists; everything is in the initial HTML |
| P2 Identify JSON requests | N/A — no API. Step 5 ("check main HTML") was the winning branch, but as a JS literal, not JSON |
| P3 Reproduce one request | Trivial — bare `curl`, no headers/cookies needed |
| P4 Data model | Resolved — see "Data model" above; palette = Case A (inline in records), plus artist-decade aggregates |
| P5 Pagination | None — client-side lazy rendering only |
| P6 Playwright network capture | Unnecessary here. NOTE if reused elsewhere: the script dedups responses by URL, which collapses GraphQL/POST traffic (same URL, different bodies) — key by URL+body hash instead |
| P7 Bulk downloader (resume/throttle) | Unnecessary — one request. Keep only the normalize step |
| P8 Validation | Still applies — see "Validate" above |
| Decision tree | **Outcome 3/5 materialized** (embedded static data). Outcome 6 applies only to the optional per-work-weights follow-up |

## Definition of Initial Success — all met 2026-07-23

1. ✅ Data located: `const ARTISTS = [...]` inline in `/colorists`
2. ✅ Works outside the browser: bare curl
3. ✅ Palette location: per-work `palette` array (+ artist/decade aggregates)
4. ✅ Linking identifiers: artist `id`; works keyed by `pageUrl`
5. ✅ Pagination: none exists
6. ✅ Script saved records: full 9.3 MB `mic_dataset.json` extracted
7. ✅ Colors match display: identical by construction (site renders from the same literal)
