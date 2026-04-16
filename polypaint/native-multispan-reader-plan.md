# Native Multi-Span Reader Plan

Status
------

Implemented through Phase 5.

Current state:

- Phase 1: compact section contract + `solve_source_manifest` is implemented
- Phase 2: associated palette uses native multispan for logical `sectioned`
  sections
- Phase 3: full render histogram uses native multispan for logical
  `sectioned` sections and explicitly rejects logical `tmpfile`
- Phase 4: render UI and status/log wording are section-first and expose
  logical solve sizes plus requested/effective input-mode clarity
- Phase 5: raster uses native multispan for logical `sectioned` sections, with
  direct tmpfile-vs-multispan parity coverage

This is the target architecture for logical render-time sections.

It supersedes the old recommendation in
[logical-sectioning-plan.md](/Users/nicknassuphis/karpo_hackathon/polypaint/logical-sectioning-plan.md)
that the tmpfile stitch adaptor should be the main rollout target.


Objective
---------

The render side of the app should operate on logical solve sections, not on the
physical chunk layout produced by compute.

The worker contract should be:

- `process logical section 31/64`

not:

- `process physical chunk 31`

That means:

- render planning owns the mapping from global solve ranges to physical chunk
  files
- native readers own remote range access across that mapping
- render workers and native scoring/raster code do not branch on solver chunk
  layout


Current Problem
---------------

Today logical sections exist in the plan/runtime layer, but they still fall
back to tmpfile stitching inside the worker:

- [lambda/handler_palette_chunk.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_chunk.py)
- [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)
- [lambda/logical_sections.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/logical_sections.py)

The current behavior is:

- logical section selected
- worker computes `root_spans` / `coeff_spans` / `param_spans`
- Python downloads and stitches those spans into `/tmp`
- native binary runs in `tmpfile` mode

So the UI can say `sectioned`, but logical sections still execute as `tmpfile`.

That is the wrong end state.

Important scope clarification:

- the lores debug/summary histogram path already reads one file:
  - `renders/{job}/lores.bin`
- that path is not the problem this plan is solving
- the problem is the full render prepass over full-solve data


Required Outcome
----------------

Once this plan is implemented:

- logical sections stay `sectioned`
- no full logical-section tmpfile is materialized first
- render workers pass a compact manifest to native code
- native code reads a logical contiguous input assembled from many physical
  spans on demand

The render side should not care whether a section:

- is fully inside one compute chunk
- crosses two chunk files
- needs one source or three sources

That is the reader’s job.

Also:

- finalize is not part of this refactor
- finalize consumes tile/fragments, not solve-aligned row stores
- the native multi-span reader is only for solve-aligned consumers


Current Code Boundaries
-----------------------

Relevant current code:

- compact logical section planning
  - [lambda/logical_sections.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/logical_sections.py)
- render plan section item generation
  - [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
- logical associated-palette worker
  - [lambda/handler_palette_chunk.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_chunk.py)
- logical solve-score hist worker
  - [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)

Relevant non-target path:

- lores debug/summary histogram
  - same handler, but reading `lores.bin`
  - already single-file
  - does not need multi-span support

Current native single-object sectioned readers:

- palette chunk
  - [lambda/solve_palette_chunk_mt.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_palette_chunk_mt.c)
- solve histogram
  - [lambda/solve_proximity_hist_sectioned.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_hist_sectioned.c)
- raster
  - [lambda/roots2pix_mt.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix_mt.c)

Important fact:

- the native sectioned path already exists
- it already uses libcurl with presigned URLs and per-thread ranged GETs
- the missing piece is multi-span logical addressing, not remote I/O from
  scratch


Hard Invariants
---------------

1. Physical compute chunks are a storage detail.
2. Render workers operate on logical solve ranges.
3. Render-facing payloads use section language, not chunk language.
4. Step Functions plans stay compact.
5. Logical sections do not inline large per-section span arrays into the plan.
6. Native readers, not Python tmpfile stitching, resolve logical ranges across
   chunk objects.
7. `sectioned` means actual native remote reading for both physical and logical
   sections.
8. The render UI exposes section sizing only for solve-aligned phases, not for
   finalize.

In practice that means:

- no `chunk_idx` in render worker contracts
- no `chunk_items` as the render-facing unit of work
- no UI wording that implies render is consuming compute chunks
- render receives:
  - `section_idx`
  - `section_count`
  - `step_start`
  - `step_count`
  - an opaque solve-source manifest

The solve-source manifest may still be derived from compute chunk artifacts, but
that storage layout is hidden behind the manifest boundary.

And in the UI:

- hist, palette, and later raster are section-counted solve-aligned phases
- finalize remains a worker/tile stage, not a section-sized solve-aligned phase


Non-Goals
---------

Not in this plan:

- redesigning compute chunking
- redesigning the raster gather/finalize model
- rewriting render around one monolithic logical object in S3
- introducing the AWS C++ SDK into Lambda binaries

This plan should reuse the current presigned-URL + libcurl model.


Native Reader Model
-------------------

Introduce a shared native multi-span reader library:

- `lambda/multispan_reader.h`
- `lambda/multispan_reader.c`

Responsibilities:

1. accept a logical manifest describing one source stream
2. map logical byte ranges onto one or more physical spans
3. issue libcurl ranged GETs against the referenced object URLs
4. copy the bytes into the caller’s destination buffer as if the source were
   one contiguous file
5. hide all physical chunk boundaries from the caller

Core API shape:

- `multispan_reader_open(manifest_path, retries, ...)`
- `multispan_reader_read_exact(reader, logical_offset, length, dst, err)`
- `multispan_reader_close(reader)`

This is conceptually a virtual `pread` over a logical file.


Manifest Contract
-----------------

The Step Functions plan should stay compact.

So the render worker input should be:

- `section_idx`
- `section_count`
- `step_start`
- `step_count`
- `logical_section=true`
- `solve_source_manifest`

`solve_source_manifest` is an opaque storage-layout description.

It may still be built from compute chunk metadata, but render code must not
treat that as the unit of work.

`solve_source_manifest` must be versioned and frozen before implementation.

Recommended Python-side schema:

```json
{
  "version": 1,
  "job_id": "compute_abc123",
  "total_solves": 25000000,
  "degree": 35,
  "n_coeffs": 36,
  "sources": {
    "slv": {
      "row_bytes": 280,
      "segments": [
        {
          "storage_id": "slv_0000",
          "key": "renders/compute_abc123/chunk_0.bin",
          "solve_start": 0,
          "solve_count": 1250000,
          "source_solve_start": 0,
          "byte_size": 350000000
        }
      ]
    },
    "cf": {
      "row_bytes": 288,
      "segments": [
        {
          "storage_id": "cf_0000",
          "key": "renders/compute_abc123/coeffs_0000.bin",
          "solve_start": 0,
          "solve_count": 1250000,
          "source_solve_start": 0,
          "byte_size": 360000000
        }
      ]
    },
    "pm": {
      "row_bytes": 16,
      "segments": [
        {
          "storage_id": "pm_0000",
          "key": "renders/compute_abc123/params_0000.bin",
          "solve_start": 0,
          "solve_count": 1250000,
          "source_solve_start": 0,
          "byte_size": 20000000
        }
      ]
    }
  }
}
```

Field rules:

- `version` is mandatory
- `total_solves`, `degree`, and `n_coeffs` are top-level metadata for sizing and
  validation
- `sources.slv` is always present for solve-aligned render phases
- `sources.cf` and `sources.pm` are present only when those artifacts exist
- `row_bytes` is stored explicitly to keep worker/native manifest generation
  honest and avoid re-deriving it inconsistently
- `segments` are sorted by `solve_start`
- `solve_start` / `solve_count` define the logical solve coverage of the stored
  object
- `source_solve_start` defines the offset inside that stored object
  - chunk-local files usually use `0`
  - any future non-zero offset storage layout is still representable
- `storage_id` is only for dedupe/stable logging and must not leak into the UI

Render workers should treat this object as opaque except for:

- validating required source presence
- asking the shared helper to map one logical section to source spans
- reporting source-family diagnostics

Render workers must not branch on:

- number of segments
- specific segment keys
- storage_id naming

The worker then derives the actual span list locally from
[lambda/logical_sections.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/logical_sections.py).

Before invoking native code, the worker writes a compact native manifest file
per required source:

- roots manifest
- coeff manifest when needed
- params manifest when needed

Recommended manifest structure:

```json
{
  "logical_size": 12345678,
  "row_bytes": 280,
  "solve_start": 390625,
  "solve_count": 390625,
  "sources": [
    {"id": 0, "url": "https://...", "key": "renders/job/chunk_0.bin"},
    {"id": 1, "url": "https://...", "key": "renders/job/chunk_1.bin"}
  ],
  "spans": [
    {
      "source_id": 0,
      "logical_byte_start": 0,
      "byte_start": 109375000,
      "byte_length": 8750000
    },
    {
      "source_id": 1,
      "logical_byte_start": 8750000,
      "byte_start": 0,
      "byte_length": 21875000
    }
  ]
}
```

Important rules:

- spans are sorted by `logical_byte_start`
- sources are deduplicated, so one URL is not repeated per span
- manifest is local to the worker, not embedded in the Step Functions plan
- render code does not inspect or reason about physical chunk boundaries

Worker-local derived span schema should also be frozen:

```json
{
  "source_family": "slv",
  "solve_start": 390625,
  "solve_count": 390625,
  "row_bytes": 280,
  "spans": [
    {
      "storage_id": "slv_0000",
      "key": "renders/compute_abc123/chunk_0.bin",
      "solve_start": 390625,
      "solve_count": 218750,
      "local_solve_start": 0,
      "byte_start": 109375000,
      "byte_length": 61250000
    }
  ]
}
```

This intermediate span structure remains worker-local. It must never be
serialized into the Step Functions plan.


Section-Only Render Contract
----------------------------

This rename is intentional and required.

Current names such as:

- `chunk_items`
- `chunk_manifest`
- `chunk_idx`

are acceptable only inside compute/storage compatibility code.

They are not acceptable as the render-side contract.

Target render-side names:

- `section_items`
- `solve_source_manifest`
- `section_idx`
- `section_count`

The point is not cosmetic. The point is to force the code to respect the right
abstraction boundary:

- compute/storage owns physical layout
- render owns logical solve sections


How Native Readers Use It
-------------------------

Current sectioned binaries already assign solve-aligned sections to threads.

That shape should stay.

For each thread section:

1. compute logical byte range from solve range
2. ask the multi-span reader for that logical byte range
3. reader resolves one or more physical spans
4. reader fills the thread-local input buffer
5. the scoring/raster logic runs exactly as if the bytes had come from one
   normal contiguous object

That preserves:

- current thread model
- current compute loops
- current score/raster semantics

The only change is where the bytes come from.


Initial Adoption Order
----------------------

Do not try to make every native binary support every source on day one.

Roll out a shared reader and adopt it incrementally.

First adoption target:

- associated palette
  - roots always
  - coeffs/params only when score program requires them

Next adoption target:

- solve histogram
  - same source-aware pattern

Later adoption target:

- optional raster adoption if and when logical raster sections become worth it


Transport Choice
----------------

Keep the current sectioned transport model:

- presigned URLs
- libcurl
- ranged GETs

Do not introduce a local HTTP proxy or the AWS SDK as the primary path.

Reason:

- the codebase already has working sectioned-native readers on libcurl
- Lambda packaging/runtime is already set up for that
- the missing feature is logical multi-span addressing, not transport


Reader Semantics
----------------

The multi-span reader must support:

- exact reads across span boundaries
- empty-range rejection
- retries on the same HTTP/network failures current sectioned readers retry
- clear error reporting with:
  - logical offset
  - requested length
  - source key / URL
  - physical byte range

The first implementation does not need a global block cache.

V1 is allowed to:

- resolve a caller read into one or more direct range GETs
- copy those bytes into the destination buffer

That is already enough to remove the tmpfile stitch path and hide chunk layout.

If profiling later shows too much repeated overlap:

- add a shared block cache
- add readahead windows
- add request coalescing per source

Those are performance improvements, not required for correctness.


Worker Contract Changes
-----------------------

Workers should stop forcing tmpfile mode for logical sections once their native
binary supports multi-span.

Associated palette:

- [lambda/handler_palette_chunk.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_chunk.py)

Solve histogram:

- [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)

New worker behavior:

- build needed spans from `solve_source_manifest`
- generate presigned URLs for the unique source objects
- write local manifest files
- invoke native binary in new input mode:
  - `multispan_sectioned`

Temporary compatibility during rollout:

- keep `tmpfile` path behind an explicit fallback
- log both:
  - `requested_input_mode`
  - `effective_input_mode`
- remove the fallback after both palette and hist are stable


Native Binary Changes
---------------------

Palette chunk binary:

- extend [lambda/solve_palette_chunk_mt.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_palette_chunk_mt.c)
- replace single `--url` / `--input_size` assumption with:
  - `--input_mode=tmpfile|sectioned|multispan_sectioned`
  - `--input_manifest=/tmp/...json`
- do the same for optional coeff source manifest
- if param source becomes native-remote later, add param manifest support too

Solve histogram binary:

- extend [lambda/solve_proximity_hist_sectioned.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_hist_sectioned.c)
- same manifest-driven input mode
- same optional coeff/param manifest support

Shared library adoption:

- both binaries should call the same multi-span reader code
- do not duplicate the span-resolution logic in each binary


Plan-Size Rule
--------------

The Step Functions plan must not carry expanded span lists.

The plan already hit size limits once.

So:

- continue sending compact `solve_source_manifest`
- continue sending compact logical section items
- derive expanded span lists inside the worker only
- write native manifests locally in `/tmp`

This is not optional.


UI / Logging Contract
---------------------

Once a phase supports native multi-span:

- `sectioned` should remain valid for:
  - `physical_chunks`
  - `logical_sections`
  - `logical_sections_auto`

The UI should stop implying that logical sections require tmpfile.

Runtime logging should show:

- section mode
- requested input mode
- effective input mode
- logical section count
- span count per source
- bytes read per source
- retry counts

If the worker falls back to tmpfile for any reason, the reason must be logged
explicitly.


UI Contract
-----------

Render-side UI should be section-oriented for solve-aligned phases.

The popup should show, per phase:

- active sources
  - `slv`
  - `slv + cf`
  - `slv + pm`
  - `slv + cf + pm`
- threads
- Lambda memory
- total solves
- logical roots size
- logical coeff size
- logical params size
- `min safe sections`

The key controls are:

- `Hist sections`
- `Palette sections`
- `Raster sections` only after raster adopts the same reader

Each section-count control should show:

- current manual value
- computed minimum safe value
- a `minsection` action that copies the safe minimum into the field

Important exclusions:

- do not show finalize in this section-sizing model
- do not present compute chunk count as the render-side work-size knob
- if compute chunk count is shown at all, it should be diagnostics only
- do not show a `Raster sections` control before raster actually supports the
  multi-span section reader

Safety computation should remain phase-specific:

- hist budget uses the histogram worker Lambda memory and thread count
- palette budget uses the palette worker Lambda memory and thread count
- raster budget uses the raster worker Lambda memory and thread count

The min safe section count is computed from:

- total solves
- roots row bytes
- coeff row bytes when needed
- params row bytes when needed
- fixed overhead
- per-thread overhead
- phase Lambda memory

This is solve-count and row-width math, not chunk-count math.


Histogram Scope Clarification
-----------------------------

There are two different histogram contexts in the app and the plan must keep
them separate.

1. Lores debug / summary histogram
   - reads `lores.bin`
   - already a single file
   - does not need the native multi-span reader
   - does not need section auto-sizing

2. Full solve render prepass histogram
   - reads the full solve-aligned data set
   - currently uses physical chunks or logical sections
   - does need the native multi-span reader if logical sections are to remain
     truly sectioned
   - does need phase-specific safe section sizing in the UI

All references to `Hist sections` and histogram auto sizing in this plan refer
to case 2, not case 1.


Rollout Phases
--------------

1. Shared native library + manifest format
   - add `multispan_reader.c/h`
   - add native manifest writer in Python workers
   - no behavior change yet

2. Associated palette native adoption
   - support roots manifest
   - support coeff manifest when score program uses `cf`
   - keep tmpfile fallback temporarily for logical sections only

3. Solve histogram native adoption
   - same source-aware adoption
   - applies only to the full render prepass histogram path
   - does not touch the lores single-file debug summary path
   - remove silent logical-section tmpfile coercion for hist

4. Logging + UI cleanup
   - requested/effective input mode clarity
   - remove misleading tmpfile-only messaging

5. Optional raster adoption
   - only if logical raster sections become a real requirement
   - reuse the same reader library and manifest model


Code Task List
--------------

This is the implementation sequence.

1. Rename the render-side contract to sections only.
   Files:
   - [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
   - [stepfunctions/render_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/render_workflow.asl.json.template)
   - [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
   Work:
   - replace render-facing `chunk_manifest` with `solve_source_manifest`
   - replace render-facing `chunk_items` usage with `section_items`
   - remove `chunk_idx` from render worker payloads where section identity is
     the real unit
   - update logs/UI text so render talks about sections, not chunks

2. Extract source-layout helpers so Python can build native manifests without
   leaking chunk semantics into workers.
   Files:
   - [lambda/logical_sections.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/logical_sections.py)
   Work:
   - add a neutral source-layout API over the current compute artifact list
   - keep compute compatibility inside this helper
   - emit source spans from `solve_source_manifest`, not from render-visible
     chunk objects

3. Add a native manifest writer on the Python side.
   Files:
   - [lambda/handler_palette_chunk.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_chunk.py)
   - [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)
   Work:
   - generate presigned URLs for the unique backing objects
   - write local roots/coeff/params manifests in `/tmp`
   - add explicit `requested_input_mode` and `effective_input_mode`
   - keep current tmpfile stitch path only as a temporary fallback

4. Add the shared native multi-span reader library.
   Files:
   - `lambda/multispan_reader.h`
   - `lambda/multispan_reader.c`
   Work:
   - parse local manifest JSON
   - map logical ranges to physical spans
   - issue ranged GETs with current retry rules
   - fill caller buffers exactly across span boundaries
   - expose one clean `read_exact` interface to callers

5. Adopt the reader in associated palette first.
   Files:
   - [lambda/solve_palette_chunk_mt.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_palette_chunk_mt.c)
   - [lambda/handler_palette_chunk.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_chunk.py)
   Work:
   - add `multispan_sectioned` input mode
   - support roots manifest
   - support coeff manifest when score program uses `cf`
   - support params manifest when score program uses `pm`
   - remove unconditional logical-section tmpfile coercion once parity is
     proven

6. Adopt the reader in solve histogram second.
   Files:
   - [lambda/solve_proximity_hist_sectioned.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_hist_sectioned.c)
   - [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)
   Work:
   - same `multispan_sectioned` support for the full render prepass path
   - same optional coeff/param source manifests
   - remove logical-section tmpfile coercion from the full render hist path
   - leave the lores single-file summary path alone

7. Clean up UI and status reporting around sectioned mode.
   Files:
   - [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
   Work:
   - if a phase supports native multi-span, `sectioned` remains valid in all
     section modes
   - stop implying that logical sections require tmpfile
   - display section counts and effective input mode clearly
   - show per-phase logical solve sizes and min safe sections
   - do not include finalize in this section-sizing block

8. Remove the tmpfile logical-section fallback after parity is proven.
   Files:
   - [lambda/handler_palette_chunk.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_chunk.py)
   - [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)
   Work:
   - delete silent fallback
   - fail clearly if multi-span input cannot be constructed
   - keep tmpfile mode only as an explicit user-selected non-sectioned mode


Fallback Rule
-------------

The fallback boundary should be explicit.

Allowed during rollout:

- associated palette logical sections may temporarily fall back to tmpfile
- full render histogram logical sections may temporarily fall back to tmpfile
- the fallback must be explicit in logs and status payloads

Not allowed:

- silent fallback for physical `sectioned`
- silent fallback after parity has been proven for a phase
- using tmpfile fallback as a permanent implementation for logical sections

End-state rule:

- `input_mode=sectioned` on a logical section means native multi-span sectioned
  reading
- associated palette may still honor `input_mode=tmpfile` explicitly
- full render histogram logical sections do not have a separate tmpfile native
  path; they require native multi-span sectioned reading and reject
  `input_mode=tmpfile` explicitly

9. Optional later task: adopt the same reader in raster.
   Files:
   - [lambda/roots2pix_mt.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix_mt.c)
   - [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)
   Work:
   - only if raster logical sections become necessary
   - use the same section-only render contract


Required Tests
--------------

Implementation rule:

- no phase is considered implemented until its tests are implemented in the same
  change
- code-only rollout is not allowed
- fallback removal is not allowed until parity tests are green

Test files to update or add:

- [tests/test_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_plan.py)
- [tests/test_render_workflow_definition.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_workflow_definition.py)
- [tests/test_palette_chunk_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_chunk_handler.py)
- [tests/test_solve_proximity_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_handler.py)
- [tests/test_solve_proximity_hist_sectioned.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_hist_sectioned.py)
- [tests/test_raster_mt_parity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_raster_mt_parity.py)
- [tests/test_raster_pixel_bins.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_raster_pixel_bins.py)
- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)
- new native reader test target
  - recommended:
    - `tests/test_multispan_reader.py` as a Python harness around a small test
      binary
    - or `lambda/test_multispan_reader.c` if a pure native harness is easier to
      keep deterministic

Phase 1 required tests:

1. Render contract rename and compact source-manifest planning.
   Files:
   - [tests/test_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_plan.py)
   - [tests/test_render_workflow_definition.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_workflow_definition.py)
   Cases:
   - render plan emits `section_items`, not render-facing `chunk_items`
   - render plan emits `solve_source_manifest`, not render-facing
     `chunk_manifest`
   - `solve_source_manifest` contains:
     - `version`
     - `total_solves`
     - `degree`
     - `n_coeffs`
     - `sources.slv`
   - `sources.cf` is present only when coeff artifacts exist
   - `sources.pm` is present only when param artifacts exist
   - no per-section expanded span arrays are serialized into the Step Functions
     plan
   - render workflow consumes `section_items` and `solve_source_manifest`
   - no render-facing worker payloads expose:
     - `chunk_idx`
     - `chunk_items`
     - `chunk_manifest`

2. Native multi-span reader unit behavior.
   Files:
   - new native reader harness test file
   Cases:
   - logical read fully inside one span
   - logical read ending exactly on a span boundary
   - logical read starting exactly on a span boundary
   - logical read crossing two spans in one source
   - logical read crossing more than two spans
   - logical read across repeated reads returns identical bytes
   - empty-range request is rejected
   - gap in logical coverage is rejected
   - short ranged download is rejected with source/range detail
   - retryable HTTP/network failure retries and reports the final failing range

Phase 2 required tests:

1. Associated palette worker manifest generation and mode selection.
   Files:
   - [tests/test_palette_chunk_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_chunk_handler.py)
   Cases:
   - logical section + `input_mode=sectioned` writes roots manifest and invokes
     `multispan_sectioned`
   - score program using `cf` writes coeff manifest
   - score program using `pm` writes params manifest
   - score program using only `slv` does not write coeff/params manifests
   - `requested_input_mode` and `effective_input_mode` are both reported
   - temporary logical tmpfile fallback, if still present in this phase, is
     explicit in status/log payloads

2. Associated palette parity against tmpfile stitch path.
   Files:
   - [tests/test_palette_chunk_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_chunk_handler.py)
   - [tests/test_palette_finalize_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_finalize_handler.py)
   Cases:
   - same logical section produces identical score output in:
     - tmpfile stitch path
     - `multispan_sectioned` path
   - same logical section produces identical bins output in:
     - tmpfile stitch path
     - `multispan_sectioned` path
   - associated-palette finalize output is unchanged when fed palette bins from
     either path

Phase 3 required tests:

1. Full render histogram worker manifest generation and mode selection.
   Files:
   - [tests/test_solve_proximity_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_handler.py)
   - [tests/test_solve_proximity_hist_sectioned.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_hist_sectioned.py)
   Cases:
   - full render hist logical section + `input_mode=sectioned` invokes
     `multispan_sectioned`
   - coeff manifest is emitted only when program uses coeff sources
   - params manifest is emitted only when program uses param sources
   - lores summary path still reads `lores.bin` and does not require multi-span
   - full render hist path no longer silently coerces logical sections to
     tmpfile

2. Full render histogram parity against tmpfile stitch path.
   Files:
   - [tests/test_solve_proximity_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_handler.py)
   - [tests/test_solve_proximity_hist_sectioned.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_hist_sectioned.py)
   Cases:
   - same logical section yields identical hist counts in:
     - tmpfile stitch path
     - `multispan_sectioned` path
   - same logical section yields identical source-dependent behavior for:
     - `slv`
     - `slv + cf`
     - `slv + pm`
     - `slv + cf + pm`

Phase 4 required tests:

1. UI contract tests.
   Files:
   - [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)
   Cases:
   - render UI shows section-oriented controls for:
     - hist
     - palette
   - render UI does not show finalize in the section-sizing block
   - render UI does not use compute chunk count as the render work-size knob
   - render UI shows:
     - logical roots size
     - logical coeff size
     - logical params size
     - per-phase `min safe sections`
   - `Raster sections` is present once raster adopts the reader
   - if a phase supports native multi-span, `sectioned` remains valid for
     logical sections
   - logs/status text for render phases refers to sections, not chunks

Phase 5 required tests, if raster adoption happens:

1. Raster worker mode and parity.
   Files:
   - [tests/test_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_raster_mt.py)
   - [tests/test_raster_mt_parity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_raster_mt_parity.py)
   Cases:
   - logical raster section + `input_mode=sectioned` invokes
     `multispan_sectioned`
   - coeff manifests are emitted only when raster solve-score program requires
     coeff sources
   - params manifests are emitted only when raster solve-score program requires
     param sources
   - raster output parity between tmpfile stitch path and `multispan_sectioned`

Acceptance commands for each phase:

- targeted pytest suites for the files changed in that phase
- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)
  when UI or status text changes
- `git diff --check`

Before removing tmpfile fallback for a phase:

- parity tests for that phase must exist
- parity tests for that phase must pass
- the phase-specific handler tests must prove no silent fallback remains


Success Criteria
----------------

This plan is done when:

- logical associated-palette sections no longer force tmpfile
- logical solve hist sections no longer force tmpfile
- render workers execute logical solve sections without knowing physical chunk
  layout
- compute chunk count can change without changing render-side reader contracts

That is the real end state:

- compute chooses physical storage layout
- render consumes logical solve sections
- native readers bridge the two
