# Fused compute chunk results via S3 references

Status: not implemented.

## Why

`FusedChunkMap` in `stepfunctions/compute_workflow.asl.json.template` aggregates
all chunk workers' return payloads into `$.solve_results` via
`"ResultPath": "$.solve_results"` before transitioning to `SaveMetadataTask`.

Each chunk worker in `handler_compute_chunk_fused.py:222–249` returns a
`result_data` dict with ~25 fields — S3 keys, byte sizes per stage, per-stage
timings, thread counts, `reused_*` flags, optional `skipped_overflow`. Rough
size: 1.5–2 KB per chunk when serialized.

Step Functions standard workflows cap per-state I/O at **262,144 bytes
(256 KB)**. The math:

- 130 chunks × ~2 KB = ~260 KB → right at the limit, intermittent failures
- 317 chunks × ~2 KB = ~630 KB → fails every time

When the limit is exceeded, Step Functions either fails silently with
`States.DataLimitExceeded` or retries indefinitely without surfacing the
error through status reporting. The user sees all chunks complete, then the
workflow hangs with no post-Map log output.

This blocks every render above ~130 chunks. Not latent — production users
are hitting it right now.

## What has to be done

### 1. Chunk worker uploads its own result JSON to S3

**`lambda/handler_compute_chunk_fused.py`**

After the roots upload completes and `result_data` is assembled at L222–249,
upload that dict as a per-chunk JSON to S3:

```python
result_key = f"renders/{job_id}/runs/{run_id}/chunk_results/{chunk_idx:05d}.json"
s3.put_object(
    Bucket=BUCKET,
    Key=result_key,
    Body=json.dumps(result_data, separators=(",", ":")).encode("utf-8"),
    ContentType="application/json",
)
```

Return only a tiny reference from the handler's `ok_response`:

```python
return ok_response({
    "chunk_idx": chunk_idx,
    "result_key": result_key,
    "bin_key": bin_key,   # kept for convenient downstream access
})
```

That's ~150 bytes per chunk in the Map state, down from ~2 KB.

The `result_data` still goes to `report_status` for DDB visibility — that
path is unchanged.

### 2. `finalize_metadata` aggregates from S3, not from payload

**`lambda/handler_compute_plan.py`**

The `finalize_metadata` action today receives `$.solve_results` as a fat list
of chunk results embedded in its payload. Change it to:

- Accept the small list of `{chunk_idx, result_key}` references instead.
- Download each referenced JSON (parallelize with `ThreadPoolExecutor`, ~20
  workers is fine).
- Reconstruct the full aggregated view from the downloaded dicts.
- Write `calc.json` as before.

Payload shape into `SaveMetadataTask` stays the same key (`solve_results`)
but now contains references instead of full dicts. Inside the Lambda:

```python
def _expand_chunk_results(references):
    def load_one(ref):
        obj = s3.get_object(Bucket=BUCKET, Key=ref["result_key"])
        return json.loads(obj["Body"].read())
    with ThreadPoolExecutor(max_workers=20) as pool:
        return list(pool.map(load_one, references))
```

Sort by `chunk_idx` after expansion to restore order (Map return is already
ordered, but belt-and-braces).

### 3. Workflow wiring — no ASL changes required

`FusedChunkMap` keeps `"ResultPath": "$.solve_results"`. `SaveMetadataTask`
keeps receiving `$.solve_results`. Only the _contents_ of the list change:
dicts of ~2 KB each become references of ~150 bytes each.

No new states. No new retries. No new error handling beyond what `s3.get_object`
already has.

### 4. Classic path — same treatment

**`lambda/handler_solve_proximity.py`** (or wherever the classic hires
`SolveMap` worker returns)

The classic `SolveMap` at `compute_workflow.asl.json.template:592–623` has the
same shape: each worker returns a full result dict. Same fix applies:

- Classic solve worker uploads its `result_data` as
  `renders/{job_id}/runs/{run_id}/solve_results/{chunk_idx:05d}.json`.
- Returns `{chunk_idx, result_key, bin_key}`.
- `finalize_metadata` handles both shapes (fused and classic) identically —
  whatever is in `$.solve_results` after either Map state is the list of
  references.

Do both paths at once. Otherwise classic renders with >130 chunks will hit
the same wall.

### 5. Cleanup prefix

Chunk result JSONs live under `renders/{job_id}/runs/{run_id}/...`. They're
small (~2 KB each) but accumulate across runs.

Add to whatever cleanup path handles `runs/` — the render family cleanup
already handles `renders/{job_id}/...` on artifact deletion, so this naturally
falls under existing cleanup if the prefix is nested correctly. Verify that
the cleanup is scoped to delete under `runs/` when a job is removed.

### 6. Tests

- `tests/test_compute_chunk_fused_handler.py` — assert the handler uploads
  `chunk_results/{chunk_idx:05d}.json` with the full `result_data`, and that
  the returned payload is small (strict: `len(json.dumps(payload)) < 500`).

- `tests/test_compute_plan.py::test_finalize_metadata_expands_s3_refs`:
  payload has refs, handler fetches + aggregates, calc.json has all fields.

- `tests/test_compute_plan.py::test_finalize_metadata_handles_317_chunks` (or
  similar large count): construct 317 refs, mock S3 returns, assert
  aggregation completes and calc.json is correct. Pins the fix.

- `tests/test_compute_workflow_definition.py` — spot-check that the
  `FusedChunkMap` / `SolveMap` `ResultPath` wiring is unchanged, and that
  `SaveMetadataTask` still reads `$.solve_results`.

- `tests/test_sweep_smoke.py` — end-to-end small render with e.g. 3 chunks,
  assert the chunk result JSONs are present in S3 at the right prefix, and
  calc.json matches what the old pre-refactor path would have produced.

### 7. `result_data` shape trim (optional but recommended)

While we're touching this code, the per-chunk `result_data` has fields that
are per-run constants repeated per chunk — `execution_method`,
`param_gen_threads`, `coeffgen_threads`, `fused_threads`. These don't vary
across chunks within a run; they should be in `calc.json` at the run level,
not echoed in every chunk result.

Trimming to per-chunk-varying fields only would roughly halve per-chunk JSON
size, which matters for operational costs even though the Map state is now
references. Move run-constants to the finalize_metadata side where they're
written once.

Not required for the main fix. Flag for a follow-up pass if the JSON volume
becomes a concern.

## What this does NOT change

- Lambda invocation model — still one Lambda per chunk, same fan-out shape.
- ASL workflow — `FusedChunkMap` state wiring unchanged, same `ResultPath`,
  same next-state transition. Only the _contents_ of the list change.
- Chunk computation or output artifacts — param/coeff/root bins still upload
  under their current keys. No breaking change to rendering or repalette.
- DDB status reporting — `report_status` still receives full `result_data`
  per chunk. Only the Step Functions payload shrinks.
- Legacy classic behavior on small chunk counts — workflow runs identically
  for renders with <130 chunks either way.

## Sizing note

Post-fix Map state size:

- Per-chunk reference: `{"chunk_idx": 317, "result_key": "renders/.../chunk_results/00316.json", "bin_key": "renders/.../chunk_316.bin"}` ≈ 150 bytes
- 317 chunks × 150 bytes = ~47 KB — comfortably under the 256 KB limit
- Scales to ~1,500 chunks before hitting the limit again

If we ever need to go beyond ~1,500 chunks, the next step is to move the Map
output itself out of the state (e.g., have the Map state write a manifest
file and have `SaveMetadataTask` read it from S3). Not needed for any
realistic workload.

## Operational immediate action

For the stuck `run_1776571217965_pz7dyc` execution:

1. Cancel it from the Step Functions console (it's not recovering).
2. Re-run with chunk count reduced to ≤100 via the Compute MT popup's fused
   tab.
3. Ship the fix before the next large render.

## Summary of changes

| file / area                                      | change                                                              |
|--------------------------------------------------|---------------------------------------------------------------------|
| `lambda/handler_compute_chunk_fused.py`          | upload result_data to `chunk_results/{idx:05d}.json`; return refs   |
| `lambda/handler_solve_proximity.py` (classic solve path) | same treatment: upload + return ref                         |
| `lambda/handler_compute_plan.py::finalize_metadata` | expand refs via parallel S3 GETs; aggregate identically         |
| `stepfunctions/compute_workflow.asl.json.template` | no changes                                                        |
| `tests/test_compute_chunk_fused_handler.py`      | assert upload happens; assert returned payload is small             |
| `tests/test_compute_plan.py`                     | assert finalize expands refs; large-N aggregation test              |
| `tests/test_compute_workflow_definition.py`      | verify wiring unchanged                                             |
| `tests/test_sweep_smoke.py`                      | end-to-end small render shows chunk results on S3                   |
