# DynamoDB: Job Status Tracking

DynamoDB coordinates async Lambda workflows. Each Lambda writes its progress; the frontend polls for completion.

## Table Schema

- **Table**: `polypaint-jobs` (on-demand billing, auto-scales)
- **Partition key**: `job_id` (String) — e.g., `compute_mmvtc0zf`
- **Sort key**: `task_id` (String) — e.g., `raster_0`, `tile_3`, `encode`
- **TTL**: `ttl` attribute, set to `now + 86400` (24h auto-cleanup)

## Attributes Per Item

| Attribute | Type | Description |
|-----------|------|-------------|
| `job_id` | S | Partition key — the compute job ID |
| `task_id` | S | Sort key — identifies the specific task |
| `task_status` | S | Current status (`started`, `done`, `error`, etc.) |
| `ttl` | N | Unix timestamp for auto-deletion (24h from write) |
| `error_msg` | S | Error message if status=error (truncated to 1000 chars) |
| `result_data` | S | Optional JSON-encoded dict of task results (e.g., sweep metadata) |

## Task ID Conventions

| Phase | Task ID Format | Count per job |
|-------|---------------|---------------|
| Coefficient gen | `coeffgen_{stripe}` | nStripes (e.g., 50) |
| Root solving | `sweep_{stripe}` | nStripes |
| Rasterization | `raster_{stripe}` | nStripes |
| Tile assembly | `tile_{tile}` | nTiles (e.g., 16) |
| Image encoding | `encode` | 1 |

The sweep Lambda writes status to DynamoDB, and stores `result_data` (sweep metadata) in completed items.

## Status Progression

```
coeffgen:   started → done
sweep:      started → done (with result_data)
raster:     started → bin_downloaded → rasterized → done
finalize:   started → reading_N/M_Ppix_Bbytes → assembled → done
encode:     started → stitching → encoding → done

Any phase:  → error (with error_msg)
```

## Writing Status: `report_status()`

Defined in `shared.py`. Called by every Lambda that has DynamoDB access:

```python
report_status(job_id, task_id, status, error_msg=None, result_data=None)
```

- Uses `PutItem` (overwrites previous status for same job_id+task_id)
- Sets TTL to 24h from now
- Error messages truncated to 1000 characters
- `result_data` is an optional dict, stored as a JSON string in the DynamoDB item (used by sweep to persist metadata like `bin_size`, `compute_us`, `n_t`, `avg_iterations`)
- No error handling — exceptions propagate to Lambda runtime

## Reading Status: `/check-status` Endpoint

Called by the frontend every 3 seconds during each pipeline phase.

**Request:**
```json
{
  "job_id": "compute_mmvtc0zf",
  "task_prefix": "raster_",
  "expected": 50
}
```

**Query:** `job_id = :jid AND begins_with(task_id, :pfx)` with pagination.

**Response:**
```json
{
  "done": 48,
  "errors": 0,
  "error_details": [],
  "stuck": [
    {"task_id": "raster_12", "status": "bin_downloaded"},
    {"task_id": "raster_37", "status": "started"}
  ],
  "status_counts": {"done": 48, "started": 1, "bin_downloaded": 1},
  "results": [
    {"task_id": "sweep_0", "result_data": {"bin_size": 21600, "compute_us": 507, "n_t": 300, "avg_iterations": 5.18}},
    ...
  ],
  "total": 48,
  "expected": 50,
  "complete": false
}
```

The `results` array contains `result_data` from completed tasks that stored it. The frontend uses this to collect sweep metadata (e.g., `bin_size`, `compute_us`, `n_t`, `avg_iterations`) without making additional requests.

`complete` is `true` when `done + errors >= expected`.

## Frontend Polling Pattern

```
Dispatch N async Lambdas (fire-and-forget via /dispatch-render)
  └── Poll /check-status every 3 seconds
        ├── check.complete → proceed to next phase
        ├── check.errors > 0 → throw with error_details
        ├── No progress for 30s → log warning
        └── No progress for 600s → throw timeout error
```

**Polling constants** (index.html):
- Poll interval: 3 seconds
- Timeout: 600 seconds (10 minutes)
- Stall warning: 30 seconds

## Cleanup

### TTL Auto-Cleanup
Items automatically deleted 24 hours after creation. No manual cleanup needed for normal operation.

### `/clean-render` Endpoint
Manually deletes all DynamoDB items for a job:
- Queries all items matching `job_id`
- Batch deletes in groups of 25 (DynamoDB limit)
- Best-effort (exceptions silently caught)
- Also deletes S3 render artifacts (`.raw`, `.pix`, `.png`, `.jpeg`)

## IAM Permissions

Lambdas with DynamoDB access (via `JOBS_TABLE` env var):
- `polypaint-coeffgen` — writes coeffgen status
- `polypaint-sweep` — writes sweep status (with result_data)
- `polypaint-raster` — writes raster status
- `polypaint-finalize` — writes finalize status
- `polypaint-encode` — writes encode status
- `polypaint-storage` — reads status (check-status), batch deletes (clean-render)

Actions granted: `PutItem`, `Query`, `BatchWriteItem`

## Sweep and DynamoDB

The sweep Lambda now reports status to DynamoDB, matching the pattern used by all other async phases. Sweep jobs are dispatched via the dispatch Lambda (fire-and-forget), and the frontend polls `/check-status` with `task_prefix: 'sweep_'` every 3 seconds. Sweep stores `result_data` in its `done` status, which the frontend extracts from the `results` array in the check-status response to collect metadata like `bin_size`, `compute_us`, `n_t`, and `avg_iterations`. The `deploy.sh` script grants the sweep Lambda DynamoDB permissions via the `JOBS_TABLE` environment variable.
