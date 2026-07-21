/* ---- Poly-Sheets tab: parameter-scan mosaic artifacts (poly-sheet.md) ----
 * One async lambda renders every frame of a sheet (the pipeline triple
 * with a $-token substituted per frame), stitched into one bilevel PNG.
 * The pipeline comes from the SAME shared controls the compute preview
 * reads; this tab adds the scan/frame/grid knobs, the run/cancel
 * buttons, and the gallery. */

let _sheetsInventory = [];
let _activeSheetRun = null;   // {sheetId, jobId, taskId}
let _sheetResumeTimer = null;
// round-13 finding 1/2: cancellation is an IDENTITY-KEYED state machine.
// The intent set is the authoritative "this run is being cancelled" record
// (decoupled from the single descriptor, so a direct rail command can
// establish intent for ANY run); the timer map gives each run its OWN retry
// slot, so a stale retry can never clear or replace a newer run's timer.
// round-14 finding 2: the intent set is PERSISTED to localStorage under its
// own identity-keyed store (NOT piggy-backed on the single Sheets
// descriptor), so a rail cancellation of a run that is not the descriptor
// survives a reload.
const SHEET_CANCEL_INTENTS_KEY = 'polypaint_sheet_cancel_intents_v1';
const _sheetCancelIntents = _sheetCancelIntentsLoad();  // "sheetId::generation"
const _sheetCancelTimers = new Map();           // "sheetId::generation" -> handle
function _sheetCancelKey(sheetId, generation) { return sheetId + '::' + generation; }
function _sheetCancelIntentsLoad() {
    try {
        const raw = localStorage.getItem(SHEET_CANCEL_INTENTS_KEY);
        return new Set(raw ? JSON.parse(raw) : []);
    } catch (e) { return new Set(); }
}
function _sheetCancelIntentsPersist() {
    try {
        localStorage.setItem(SHEET_CANCEL_INTENTS_KEY,
                             JSON.stringify([..._sheetCancelIntents]));
    } catch (e) {}
}
function _sheetCancelIntentAdd(key) { _sheetCancelIntents.add(key); _sheetCancelIntentsPersist(); }
function _sheetCancelIntentDelete(key) { _sheetCancelIntents.delete(key); _sheetCancelIntentsPersist(); }
function _sheetClearCancelTimer(sheetId, generation) {
    const key = _sheetCancelKey(sheetId, generation);
    const h = _sheetCancelTimers.get(key);
    if (h !== undefined) { clearTimeout(h); _sheetCancelTimers.delete(key); }
}
async function _sheetResumePersistedCancels() {
    /* round-14 finding 2: on boot, re-issue a DIRECT cancel for every
     * persisted intent (its in-memory retry timer was lost on reload). Each
     * is identity-scoped, so this is safe regardless of the descriptor. */
    for (const key of [..._sheetCancelIntents]) {
        const idx = key.lastIndexOf('::');
        if (idx < 0) { _sheetCancelIntentDelete(key); continue; }
        const sheetId = key.slice(0, idx), generation = key.slice(idx + 2);
        void _cancelSheetRun(sheetId, generation, { direct: true });
    }
}
// round-10 finding 6: only these EXPLICIT statuses count as terminal — a
// malformed/unknown run.status must not be mistaken for a confirmed end
const SHEET_TERMINAL_STATUSES = new Set(
    ['cancelled', 'done', 'abandoned', 'failed', 'superseded']);
// generations this client just abandoned — discovery must not re-grab
// them in the window before the server's terminal mark propagates
const _sheetAbandonedGenerations = new Set();

function _sheetNewId() {
    return 'sheet_' + Date.now().toString(36) + '_' + Math.random().toString(36).slice(2, 8);
}

function _sheetVal(id, fallback) {
    const el = document.getElementById(id);
    if (!el) return fallback;
    const v = el.value;
    return v === '' || v == null ? fallback : v;
}

function _sheetSpacingChanged() {
    for (const sfx of ['', '2']) {
        const isStep = String(_sheetVal('sheet-spacing' + sfx, 'linear')) === 'step';
        for (const id of [`sheet-step${sfx}`, `sheet-step${sfx}-label`]) {
            const el = document.getElementById(id);
            if (el) el.style.display = isStep ? '' : 'none';
        }
        const toEl = document.getElementById('sheet-to' + sfx);
        if (toEl) toEl.disabled = isStep;
    }
}

function _sheetScanLine(sfx) {
    const token = String(_sheetVal('sheet-token' + sfx, '')).trim();
    if (!token) return null;   // blank token = inactive line
    return {
        token,
        from: parseFloat(_sheetVal('sheet-from' + sfx, 0)),
        to: parseFloat(_sheetVal('sheet-to' + sfx, 1)),
        step: parseFloat(_sheetVal('sheet-step' + sfx, 1)),
        steps: Math.max(1, Math.min(256, parseInt(_sheetVal('sheet-steps' + sfx, 16), 10) || 16)),
        spacing: String(_sheetVal('sheet-spacing' + sfx, 'linear')),
    };
}

function _sheetActiveScans() {
    return ['', '2'].map(_sheetScanLine).filter(Boolean);
}

function _sheetPreviewTilePx() {
    /* The Compute Preview input owns the browser-side pixel range. Reading
     * its declared bounds keeps Sheet inheritance aligned with the control
     * instead of maintaining a second, stale resolution ceiling here. */
    const input = document.getElementById('compute-preview-size');
    const parsed = Number.parseInt(input?.value ?? '', 10);
    const parsedMin = Number.parseInt(input?.min ?? '', 10);
    const parsedMax = Number.parseInt(input?.max ?? '', 10);
    const value = Number.isFinite(parsed) ? parsed : 1000;
    const min = Number.isFinite(parsedMin) ? parsedMin : 64;
    const max = Number.isFinite(parsedMax) ? parsedMax : 4096;
    return Math.max(min, Math.min(max, value));
}

function _sheetInheritedFrame() {
    /* N, pixels, viewport mode/bounds, and rotation all come from the
     * COMPUTE preview controls — the sheet renders what Preview shows.
     * Throws with a user-facing message when explicit bounds are missing. */
    const n = Math.max(8, Math.min(256, parseInt(_sheetVal('compute-preview-n', 96), 10) || 96));
    const tile = _sheetPreviewTilePx();
    const rotate = parseInt(_sheetVal('compute-preview-rotate', 0), 10) || 0;
    const previewMode = (typeof _computePreviewViewportMode !== 'undefined') ? _computePreviewViewportMode : 'quantile';
    const freeze = !!document.getElementById('sheet-freeze')?.checked;
    let viewport;
    if (previewMode === 'quantile') {
        viewport = {
            mode: freeze ? 'frozen' : 'quantile',
            quantile: Math.max(0, Math.min(0.1, _numFieldOr('compute-preview-quantile', 0) / 100)),
            shim: Math.max(0, Math.min(0.5, _numFieldOr('compute-preview-shim', 5) / 100)),
        };
    } else {
        const b = _computePreviewExplicitBounds();
        if (!b) {
            throw new Error(previewMode === 'marquee'
                ? 'Marquee viewport: drag a rectangle on the Compute preview image first.'
                : 'Square viewport: enter a positive extent on the Compute preview.');
        }
        viewport = { mode: 'explicit', min_re: b.min_re, max_re: b.max_re, min_im: b.min_im, max_im: b.max_im };
    }
    return {
        n, tile_px: tile, rotate, viewport,
        solver_mode: String(_sheetVal('compute-preview-solver', 'ae64')),
        solver_iters: Math.max(0, Math.min(64, parseInt(_sheetVal('compute-preview-iters', 0), 10) || 0)),
        polarity: String(_sheetVal('sheet-polarity', 'white_on_black')),
        margin_px: Math.max(0, Math.min(64, parseInt(_sheetVal('sheet-margin', 4), 10) || 0)),
        label: !!document.getElementById('sheet-label')?.checked,
    };
}

/* ---- durable run descriptor (CR35-F4): the browser can die at any
 * point after `begin`; everything needed to resume — including the
 * full dispatch payload — is persisted until the run is terminal. */
const SHEET_RUN_STORAGE_KEY = 'polypaint_sheet_run_v1';

function _sheetRunSave(desc) {
    try { localStorage.setItem(SHEET_RUN_STORAGE_KEY, JSON.stringify(desc)); } catch (e) {}
}

function _sheetRunLoad() {
    try {
        const raw = localStorage.getItem(SHEET_RUN_STORAGE_KEY);
        return raw ? JSON.parse(raw) : null;
    } catch (e) { return null; }
}

function _sheetRunClear() {
    try { localStorage.removeItem(SHEET_RUN_STORAGE_KEY); } catch (e) {}
}

const SHEET_MAX_WORKERS = 8;

function _sheetCanvasGeometry(scans, steps, frame, requestedCols) {
    /* Mirror server geometry for a truthful pre-run size/layout summary.
     * Admission remains server-authoritative. */
    const twoDimensional = Array.isArray(scans) && scans.length === 2;
    const cols = twoDimensional
        ? Number(scans[0].steps)
        : (Number(requestedCols) || Math.ceil(Math.sqrt(steps)));
    const rows = twoDimensional ? Number(scans[1].steps) : Math.ceil(steps / cols);
    const tilePx = Number(frame.tile_px);
    const marginPx = Number(frame.margin_px);
    const width = cols * tilePx + (cols + 1) * marginPx;
    const height = rows * tilePx + (rows + 1) * marginPx;
    return {
        cols, rows, width, height, pixels: width * height,
        tilePx, marginPx, twoDimensional,
    };
}

function _sheetCanvasSummary(geometry, steps) {
    return `${geometry.cols} columns x ${geometry.rows} rows = ${steps} frames; ` +
        `${geometry.tilePx}px tiles + ${geometry.marginPx}px margins -> ` +
        `${geometry.width}x${geometry.height}px ` +
        `(${(geometry.pixels / 1e6).toFixed(1)}MP, ` +
        `~${(geometry.pixels / (8 * 1024 * 1024)).toFixed(1)}MiB uncompressed at 1 bit; ` +
        `streamed, no full canvas allocation)`;
}

function _sheetGeometryChanged() {
    const summaryEl = document.getElementById('sheet-layout-summary');
    const colsEl = document.getElementById('sheet-cols');
    const scans = _sheetActiveScans();
    const twoDimensional = scans.length === 2;
    if (colsEl) {
        colsEl.disabled = twoDimensional;
        colsEl.title = twoDimensional
            ? 'Ignored for a 2-D scan: line 1 Steps is columns and line 2 Steps is rows.'
            : '0 = auto (square-ish)';
    }
    if (!summaryEl) return;
    if (!scans.length) {
        summaryEl.textContent = 'No active scan. Enter a token on at least one line.';
        summaryEl.style.color = '#ff8a80';
        return;
    }
    const steps = scans.reduce((total, scan) => total * scan.steps, 1);
    if (steps > 256) {
        summaryEl.textContent = `Invalid layout: ${steps} frames exceeds the 256-frame limit. ` +
            (twoDimensional ? 'Reduce one or both Steps fields.' : 'Reduce Steps.');
        summaryEl.style.color = '#ff8a80';
        return;
    }
    const frame = {
        tile_px: _sheetPreviewTilePx(),
        margin_px: Math.max(0, Math.min(64,
            parseInt(_sheetVal('sheet-margin', 4), 10) || 0)),
    };
    const requestedCols = twoDimensional
        ? undefined
        : (parseInt(_sheetVal('sheet-cols', 0), 10) || Math.ceil(Math.sqrt(steps)));
    const geometry = _sheetCanvasGeometry(scans, steps, frame, requestedCols);
    const mode = twoDimensional
        ? '2-D scan (the two Steps fields set columns and rows; Cols is ignored): '
        : '1-D scan: ';
    summaryEl.textContent = mode + _sheetCanvasSummary(geometry, steps) +
        '. The 1-bit PNG is stitched by libvips.';
    summaryEl.style.color = '#9aa0b4';
}

function _sheetBindGeometryControls() {
    const ids = [
        'sheet-token', 'sheet-token2', 'sheet-steps', 'sheet-steps2',
        'sheet-cols', 'sheet-margin', 'compute-preview-size',
    ];
    for (const id of ids) {
        const el = document.getElementById(id);
        if (!el || el._sheetGeometryBound) continue;
        el._sheetGeometryBound = true;
        el.addEventListener('input', _sheetGeometryChanged);
        el.addEventListener('change', _sheetGeometryChanged);
    }
    _sheetGeometryChanged();
}

function _sheetFrameRanges(steps, workers) {
    // contiguous, balanced: frame k goes to worker floor(k*W/steps)
    const ranges = Array.from({ length: workers }, () => []);
    for (let k = 0; k < steps; k++) ranges[Math.floor(k * workers / steps)].push(k);
    return ranges.filter(r => r.length > 0);
}

async function runPolySheet() {
    const btn = document.getElementById('btn-sheet-run');
    const statusEl = document.getElementById('sheets-status');
    const existing = _sheetRunLoad();
    if (existing) {
        // round-3 finding 6: a transient failure re-enabled Run, and a
        // new run would overwrite the single resumable descriptor
        if (statusEl) {
            statusEl.textContent = `Sheet ${existing.sheetId} is still resumable — resuming it instead (Cancel it first to start fresh).`;
            statusEl.className = 'status';
        }
        void resumeSheetRun();
        return;
    }
    const funcName = document.getElementById('render-function')?.value || '';
    if (!funcName) {
        if (statusEl) { statusEl.textContent = 'Select a function on the Render tab first.'; statusEl.className = 'status error'; }
        return;
    }
    const scans = _sheetActiveScans();
    if (!scans.length) {
        if (statusEl) { statusEl.textContent = 'Enter a scan token on at least one line (blank token = inactive).'; statusEl.className = 'status error'; }
        return;
    }
    const steps = scans.reduce((a, sc) => a * sc.steps, 1);
    if (steps > 256) {
        if (statusEl) { statusEl.textContent = `Total frames ${steps} > 256 — reduce steps (product of active lines).`; statusEl.className = 'status error'; }
        return;
    }
    let frame;
    try {
        frame = _sheetInheritedFrame();
    } catch (e) {
        if (statusEl) { statusEl.textContent = e.message; statusEl.className = 'status error'; }
        return;
    }

    const requestedCols = scans.length === 2
        ? undefined
        : (parseInt(_sheetVal('sheet-cols', 0), 10) || Math.ceil(Math.sqrt(steps)));
    const geometry = _sheetCanvasGeometry(scans, steps, frame, requestedCols);

    const sheetId = _sheetNewId();
    const jobId = sheetId;
    const taskId = 'sheet_run_' + sheetId;

    const payload = _attachProgramSourcePayload({
        job_id: jobId,
        task_id: taskId,
        sheet_id: sheetId,
        function: funcName,
        cfpv: _cfpv.length > 0 ? [..._cfpv] : [],
        param_transforms: _effectiveParamTransformsForCompute(),
        param_program_chain: _effectiveParamProgramChainForCompute(),
        coeff_transforms: _effectiveCoeffTransformsForCompute(),
        coeff_program_chain: _effectiveCoeffProgramChainForCompute(),
        scans,
        frame,
        // Cross product: the grid IS steps1 x steps2.
        grid_cols: requestedCols,
    });

    const orig = btn ? btn.textContent : 'Run Sheet';
    if (btn) { btn.disabled = true; btn.textContent = 'Validating...'; }
    if (statusEl) {
        statusEl.textContent = `Sheet ${sheetId}: validating ${_sheetCanvasSummary(geometry, steps)}; probing frame 0...`;
        statusEl.className = 'status';
    }
    let admitted = false;
    try {
        // synchronous admission: the server validates everything, probes
        // frame 0 (real degree -> honest budget), mints the generation,
        // and pre-writes every status row BEFORE any async work
        const run = await lambdaPost(
            'poly_sheet', { ...payload, action: 'begin' }, '/sheet-begin',
            { idempotent: false });
        if (!run || !run.generation || !Array.isArray(run.workers)) {
            throw new Error('sheet-begin returned an invalid admission record');
        }
        admitted = true;
        const desc = {
            sheetId, jobId, generation: run.generation,
            steps, funcName, n: frame.n,
            workers: run.workers, stitchTask: run.stitch_task_id,
            startedAtS: Date.now() / 1000,
            payload: { ...payload, generation: run.generation },
        };
        _sheetRunSave(desc);
        _activeSheetRun = { sheetId, jobId, generation: run.generation };
        _jobsRailUpsert({
            id: 'sheet:' + sheetId, kind: 'sheet',
            label: `Sheet ${steps}f N=${frame.n} · ${funcName}`,
            jobId, tab: 'sheets', state: 'running', startedAt: Date.now(),
            generation: run.generation,
            detail: `accepted (${run.workers.length} workers)`,
        });
        if (btn) btn.textContent = 'Rendering...';
        await _sheetDriveRun(desc, statusEl, { dispatchWorkers: true });
        _sheetRunClear();                       // terminal success
        if (btn) { btn.textContent = '✓ Done'; setTimeout(() => { btn.textContent = orig; btn.disabled = false; }, 1500); }
    } catch (e) {
        if (!admitted) {
            // No generation was returned, so there is nothing the resume
            // machinery can safely redispatch or the rail can track. The old
            // shared catch created a permanent running card and claimed it
            // would retry even though no descriptor existed.
            if (statusEl) {
                statusEl.textContent = 'Sheet admission failed: ' + e.message;
                statusEl.className = 'status error';
            }
            if (btn) { btn.textContent = orig; btn.disabled = false; }
            return;
        }
        // only a SERVER-reported terminal outcome retires the descriptor;
        // a transient drive failure (network, poll timeout) keeps it AND
        // schedules a retry — round-7 finding 4: the initial drive used to
        // preserve the descriptor but never re-attempt, so recovery needed
        // a manual reload. Resume shares this backoff/give-up machinery.
        if (e && e.sheetTerminal) {
            _sheetRunClear();
        } else {
            _sheetScheduleResume(1);
        }
        _jobsRailUpsert({ id: 'sheet:' + sheetId, state: e && e.sheetTerminal ? 'error' : 'running', detail: e.message });
        if (statusEl) { statusEl.textContent = 'Sheet failed: ' + e.message + (e && e.sheetTerminal ? '' : ' — will retry'); statusEl.className = 'status error'; }
        if (btn) { btn.textContent = orig; btn.disabled = false; }
        return;
    } finally {
        _activeSheetRun = null;
    }
    void loadSheetsTab();
}

function _sheetScheduleResume(attemptHint) {
    /* Schedule the next resume attempt after a backoff proportional to
     * the lease (round-7 finding 4). Shared by the initial drive failure
     * and resume so a lost/expired lease is actually retried. */
    const desc = _sheetRunLoad();
    if (!desc) return;
    desc.resumeAttempts = desc.resumeAttempts || attemptHint || 1;
    const backoff = SHEET_RESUME_BACKOFF_MS * desc.resumeAttempts;
    desc.nextResumeAt = Date.now() + backoff;
    _sheetRunSave(desc);
    if (_sheetResumeTimer) clearTimeout(_sheetResumeTimer);
    _sheetResumeTimer = setTimeout(() => { void resumeSheetRun(); }, backoff);
}

function _sheetTerminalError(message) {
    const err = new Error(message);
    err.sheetTerminal = true;
    return err;
}

async function _sheetDispatchWorkers(desc) {
    const jobs = desc.workers.map((w) => ({
        ...desc.payload,
        action: 'frames',
        task_id: w.task_id,
        frame_indices: w.frames,
    }));
    // one retry: workers are idempotent per generation, so re-dispatching
    // every job after a partial acceptance cannot corrupt anything
    for (let attempt = 0; attempt < 2; attempt++) {
        const disp = await lambdaPost('dispatch', {
            target: 'poly_sheet', jobs, expected_keys: [],
        });
        if ((disp.fired || 0) === jobs.length) return;
    }
    throw new Error('poly-sheet worker dispatch failed after retry');
}

async function _sheetDriveRun(desc, statusEl, opts) {
    /* Drive an admitted run to a terminal state. Resumable: everything
     * it needs is in the persisted descriptor, and every step is
     * idempotent per generation (CR35-F4). */
    const { sheetId, jobId, steps } = desc;
    // round-6 finding 2: run.json is the AUTHORITATIVE completion signal.
    // The stitch's terminal DDB write is best-effort (it swallows a
    // throttle after the pointer commit), so a published sheet can show
    // no 'done' DDB row. Reconcile against run.json first — if this
    // generation is already published, we're done regardless of DDB.
    if (await _sheetGenerationPublished(sheetId, desc.generation)) {
        _jobsRailUpsert({ id: 'sheet:' + sheetId, state: 'done', detail: 'published' });
        if (statusEl) { statusEl.textContent = `Sheet ${sheetId}: published — generating DeepZoom...`; statusEl.className = 'status ok'; }
        void _sheetGenerateDeepZoom(sheetId, statusEl).catch(() => {});
        return;
    }
    if (opts && opts.dispatchWorkers) await _sheetDispatchWorkers(desc);
    await _pollSheetWorkers(sheetId, jobId, steps, desc.workers.length, statusEl, desc.generation);

    const stitchState = await _sheetTaskPhase(jobId, desc.stitchTask);
    if (stitchState === null) {
        // UNKNOWN (poll failure) must not double-dispatch the stitch
        // (review round-2 finding 7): fail transiently — the descriptor
        // survives and resume retries with a real answer
        throw new Error('could not determine stitch state; will retry on resume');
    }
    // round-9 finding 2: redispatch the stitch in ANY non-terminal state,
    // not just 'accepted'. A stitch that crashed after reporting 'stitch'
    // or 'Publishing' would otherwise never be relaunched. claim_task
    // gates duplicates — a live stitch's lease no-ops the redispatch, a
    // crashed stitch's expired lease lets the redispatch reclaim it.
    if (stitchState !== 'done' && stitchState !== 'error') {
        const stitch = await lambdaPost('dispatch', {
            target: 'poly_sheet',
            jobs: [{ ...desc.payload, action: 'stitch', task_id: desc.stitchTask,
                     started_at_s: desc.startedAtS }],
            expected_keys: [],
        });
        if ((stitch.fired || 0) !== 1) throw new Error('poly-sheet stitch dispatch failed');
        // round-9 finding 4: report a REAL accepted stitch redispatch so a
        // stitch-only recovery records an actual dispatch, not intent
        if (opts && opts.onDispatch) opts.onDispatch();
    }
    const doneRd = await _pollSheetTask(sheetId, jobId, desc.stitchTask, statusEl, 'stitching', 'sheet:' + sheetId, desc.generation);
    _jobsRailUpsert({ id: 'sheet:' + sheetId, state: 'done', detail: `${doneRd.frames} frames` });
    if (statusEl) { statusEl.textContent = `Sheet ${sheetId}: done (${doneRd.frames} frames, ${doneRd.elapsed_ms}ms) — generating DeepZoom...`; statusEl.className = 'status ok'; }
    // DeepZoom pyramid right away — the sheet viewer of choice
    void _sheetGenerateDeepZoom(sheetId, statusEl).catch(() => {});
}

async function _sheetGenerationPublished(sheetId, generation) {
    /* Authoritative completion (round-6 finding 2): fetch run.json and
     * report whether THIS generation is the published pointer. Best-
     * effort — a fetch failure just means "not confirmed here". */
    try {
        const resp = await fetch(_publicStorageUrl(`sheets/${sheetId}/run.json`) + '?t=' + Date.now());
        if (!resp.ok) return false;
        const run = await resp.json();
        return !!run && run.published_generation === generation;
    } catch (e) { return false; }
}

async function _sheetTaskPhase(jobId, taskId) {
    for (let attempt = 0; attempt < 3; attempt++) {
        if (attempt) await new Promise(r => setTimeout(r, 1000 * attempt));
        try {
            const check = await lambdaPost('storage', {
                job_id: jobId, task_prefix: taskId, expected: 1,
            }, '/check-status');
            if ((check.errors || 0) > 0) return 'error';
            const rd = check.results?.[0];
            return rd ? (rd.phase || 'accepted') : 'accepted';
        } catch (e) { /* transient; retry */ }
    }
    return null;   // UNKNOWN after retries — caller must not guess
}

const SHEET_RESUME_MAX_ATTEMPTS = 6;
const SHEET_RESUME_BACKOFF_MS = 30000;
// Must track the server's CLAIM_LEASE_SECONDS (shared.py). A crashed
// worker is only reclaimable AFTER its lease expires, so the resume loop
// must not abandon a run until it has actually redispatched at least once
// past this horizon — otherwise every retry races a still-live lease and
// the crashed worker is never recovered (round-8 finding 4).
const SHEET_LEASE_MS = 420000;

async function _sheetWorkersNeedingDispatch(desc) {
    /* Round-6 finding 1: redispatch every NON-DONE worker. The server's
     * lease claim is the gate — a redispatch of a live worker loses the
     * claim and no-ops, while a CRASHED worker (running row, expired
     * lease) is reclaimed by its redispatch. Blindly skipping 'running'
     * (round-3) stranded crashed workers forever. */
    const check = await lambdaPost('storage', {
        job_id: desc.jobId,
        task_prefix: `sheet_tiles_${desc.sheetId}_${desc.generation}`,
        expected: desc.workers.length,
    }, '/check-status');
    if ((check.errors || 0) > 0) {
        const detail = check.error_details?.[0] || {};
        throw _sheetTerminalError(detail.error_msg || 'sheet worker failed');
    }
    const phases = {};
    for (const r of check.results || []) {
        if (r.task_id) phases[r.task_id] = r.phase || 'accepted';
    }
    return desc.workers.filter(w => (phases[w.task_id] || 'accepted') !== 'done');
}

async function resumeSheetRun() {
    /* Reload recovery (CR35-F4, hardened round-3): bounded attempts with
     * backoff, no recursion into loadSheetsTab. Round-6 finding 1: EVERY
     * non-terminal worker is redispatched (the server lease gates
     * duplicates — a live worker no-ops, a crashed one is reclaimed);
     * round-9 finding 7: this comment previously said "only never-started
     * workers", contradicting that fix. */
    const desc = _sheetRunLoad();
    if (!desc || _activeSheetRun) return;
    // round-10 finding 3: a persisted cancellation intent (set before a
    // reload lost the in-page retry timer) takes precedence — re-establish
    // the intent and re-issue the authoritative cancel (direct) instead of
    // driving the run forward.
    if (desc.cancelRequested) {
        void _cancelSheetRun(desc.sheetId, desc.generation, { direct: true });
        return;
    }
    // anchor the lease clock on the first resume so give-up can require a
    // real post-lease-expiry dispatch (round-8 finding 4). Persisted, so
    // it survives reloads across the whole retry window.
    const nowAnchor = Date.now();
    if (!desc.firstResumeAt) { desc.firstResumeAt = nowAnchor; _sheetRunSave(desc); }
    const pastLeaseHorizon =
        (nowAnchor - desc.firstResumeAt) >= SHEET_LEASE_MS;
    // give-up check FIRST (round-5 finding 4: the max-attempt gate must
    // not sit behind the backoff early-return, or a stranded run never
    // reaches it). Round-8 finding 4: give up ONLY once we have both
    // exhausted the attempt ceiling AND made a dispatch past the lease
    // horizon (desc.hadPostLeaseDispatch) — abandoning before the lease
    // could expire strands a crashed worker that was never actually
    // reclaim-attempted.
    if ((desc.resumeAttempts || 0) >= SHEET_RESUME_MAX_ATTEMPTS
            && desc.hadPostLeaseDispatch) {
        const statusEl = document.getElementById('sheets-status');
        if (statusEl) {
            statusEl.textContent = `Sheet ${desc.sheetId}: resume gave up after ${desc.resumeAttempts} attempts — abandoning the run`;
            statusEl.className = 'status error';
        }
        // finding 5: tell the server so /list-sheets discovery stops
        // rediscovering this run, plus a local guard for the propagation
        // window (the mark is async and may not have landed yet). The
        // local guard is added regardless so THIS client stops
        // rediscovering immediately.
        _sheetAbandonedGenerations.add(desc.generation);
        // round-8 finding 5: dispatch the abandon until ACCEPTED, don't
        // just fire-and-swallow. If it is never accepted, keep the
        // descriptor and schedule a retry (the give-up branch re-runs and
        // re-dispatches) so the server-side hide is not silently dropped.
        const abandoned = await _sheetDispatchControl(
            desc.sheetId, desc.generation, 'abandon');
        // round-9 finding 5: clear local state ONLY once run.json is
        // confirmed terminal — a bare 202 does not prove the server hide
        // landed, and clearing on intent leaves a ghost 'running' run for
        // discovery to resurrect. If unaccepted OR unconfirmed, keep the
        // descriptor and reschedule so the hide is retried.
        const confirmed = abandoned
            && await _sheetConfirmRunTerminal(desc.sheetId, desc.generation);
        if (!confirmed) {
            if (_sheetResumeTimer) clearTimeout(_sheetResumeTimer);
            _sheetResumeTimer = setTimeout(() => { void resumeSheetRun(); },
                                           SHEET_RESUME_BACKOFF_MS);
            if (statusEl) {
                statusEl.textContent = `Sheet ${desc.sheetId}: abandoned locally; confirming the server hide...`;
                statusEl.className = 'status';
            }
            return;
        }
        _sheetRunClear();
        return;
    }
    const now = Date.now();
    if (desc.nextResumeAt && now < desc.nextResumeAt) {
        // round-5 finding 4: a reload during backoff destroyed the only
        // retry timer. Rebuild it from the persisted deadline so the run
        // resumes automatically without waiting for a manual refresh.
        if (_sheetResumeTimer) clearTimeout(_sheetResumeTimer);
        _sheetResumeTimer = setTimeout(() => { void resumeSheetRun(); },
                                       Math.max(0, desc.nextResumeAt - now));
        return;
    }
    desc.resumeAttempts = (desc.resumeAttempts || 0) + 1;
    desc.nextResumeAt = now + SHEET_RESUME_BACKOFF_MS * desc.resumeAttempts;
    _sheetRunSave(desc);
    // NB: hadPostLeaseDispatch is deliberately NOT set here (round-9
    // finding 4) — recording it before any dispatch let a single
    // status-request failure satisfy the give-up gate without a recovery
    // invocation ever being issued. It is set below, only after a real
    // post-horizon dispatch/observation.

    const statusEl = document.getElementById('sheets-status');
    _activeSheetRun = { sheetId: desc.sheetId, jobId: desc.jobId, generation: desc.generation };
    _jobsRailUpsert({
        id: 'sheet:' + desc.sheetId, kind: 'sheet',
        label: `Sheet ${desc.steps}f N=${desc.n} · ${desc.funcName}`,
        jobId: desc.jobId, tab: 'sheets', state: 'running',
        startedAt: (desc.startedAtS || 0) * 1000 || Date.now(),
        generation: desc.generation,
        detail: `resume attempt ${desc.resumeAttempts}`,
    });
    if (statusEl) { statusEl.textContent = `Sheet ${desc.sheetId}: resuming (attempt ${desc.resumeAttempts})...`; statusEl.className = 'status'; }
    // round-9 finding 4: record the post-lease reclaim ONLY when a real
    // recovery invocation is ACCEPTED (a fired worker or stitch redispatch),
    // never on a mere status read. A status-probe success with no pending
    // workers, followed by a failing stitch probe, must NOT satisfy give-up.
    const markPostLeaseDispatch = () => {
        if (pastLeaseHorizon && !desc.hadPostLeaseDispatch) {
            desc.hadPostLeaseDispatch = true;
            _sheetRunSave(desc);
        }
    };
    try {
        const pending = await _sheetWorkersNeedingDispatch(desc);
        if (pending.length) {
            await _sheetDispatchWorkers({ ...desc, workers: pending });
            markPostLeaseDispatch();   // a REAL accepted worker redispatch
        }
        // the stitch redispatch (a stitch-only recovery) reports via the
        // callback, so it too records an actual accepted dispatch
        await _sheetDriveRun(desc, statusEl, { dispatchWorkers: false,
                                               onDispatch: markPostLeaseDispatch });
        _sheetRunClear();
        void loadSheetsTab();
    } catch (e) {
        if (e && e.sheetTerminal) {
            _sheetRunClear();
        } else {
            // finding 4: actually SCHEDULE the next attempt — reopening
            // the tab was the only retry before
            const backoff = SHEET_RESUME_BACKOFF_MS * (desc.resumeAttempts || 1);
            if (_sheetResumeTimer) clearTimeout(_sheetResumeTimer);
            _sheetResumeTimer = setTimeout(() => { void resumeSheetRun(); }, backoff);
            if (statusEl) { statusEl.textContent = `Sheet ${desc.sheetId} resume failed: ${e.message} — retrying in ${Math.round(backoff / 1000)}s`; statusEl.className = 'status error'; }
        }
        _jobsRailUpsert({ id: 'sheet:' + desc.sheetId, state: e && e.sheetTerminal ? 'error' : 'running', detail: e.message });
    } finally {
        _activeSheetRun = null;
    }
}

const SHEET_POLL_MS = 3000;
const SHEET_POLL_MAX_CONSECUTIVE_FAILURES = 5;
// round-9 finding 6: the server allows one worker/frame up to 720s
// (BUDGET_US) of native work between tile writes, so the client must not
// declare a stall before that legal deadline plus margin — 600s wrongly
// killed a healthy worker mid-frame. 15 min clears the 720s budget.
const SHEET_STALL_DEADLINE_MS = 15 * 60 * 1000;

async function _pollSheetWorkers(sheetId, jobId, steps, workers, statusEl, generation) {
    /* CR35-F4/F5 poll contract: transient /check-status failures are
     * retried (a single network error must not abandon a healthy run);
     * a run whose progress signature stalls past the deadline is
     * declared dead (an accepted row whose worker never reported would
     * otherwise poll forever). */
    let failures = 0;
    let lastSignature = '';
    let lastProgressAt = Date.now();
    while (true) {
        await new Promise(r => setTimeout(r, SHEET_POLL_MS));
        let check;
        try {
            check = await lambdaPost('storage', {
                job_id: jobId,
                task_prefix: `sheet_tiles_${sheetId}_${generation}`,
                expected: workers,
            }, '/check-status');
            failures = 0;
        } catch (e) {
            failures += 1;
            if (failures >= SHEET_POLL_MAX_CONSECUTIVE_FAILURES) {
                throw new Error(`status polling failed ${failures}x: ${e.message}`);
            }
            continue;
        }
        if (check.errors > 0) {
            const detail = check.error_details?.[0] || {};
            throw _sheetTerminalError(detail.error_msg || 'sheet worker failed');
        }
        const rows = check.results || [];
        const doneFrames = rows.reduce((a, r) => a + (Number(r.frame) || 0), 0);
        const doneRows = rows.filter(r => r.phase === 'done').length;
        const signature = `${doneFrames}:${doneRows}:${rows.length}`;
        if (signature !== lastSignature) {
            lastSignature = signature;
            lastProgressAt = Date.now();
        } else if (Date.now() - lastProgressAt > SHEET_STALL_DEADLINE_MS) {
            throw new Error(
                `no worker progress for ${Math.round(SHEET_STALL_DEADLINE_MS / 60000)} minutes — ` +
                'a worker likely died before reporting; re-run the sheet');
        }
        const label = `${doneFrames}/${steps} frames (${doneRows}/${workers} workers)`;
        if (statusEl) { statusEl.textContent = `Sheet ${sheetId}: ${label}`; statusEl.className = 'status'; }
        _jobsRailUpsert({ id: 'sheet:' + sheetId, state: 'running', detail: label });
        if (doneRows >= workers) return;
    }
}

async function _pollSheetTask(sheetId, jobId, taskId, statusEl, phaseLabel, railId = null, publishedGeneration = null) {
    // railId: rail card to keep updating, or null — the deepzoom poll must
    // NOT touch the sheet's card (it would resurrect a done card to running)
    let failures = 0;
    let lastLabel = '';
    let lastProgressAt = Date.now();
    while (true) {
        await new Promise(r => setTimeout(r, SHEET_POLL_MS));
        // round-6 finding 2: reconcile the STITCH poll against run.json —
        // a swallowed terminal DDB write must not hang the poll on a sheet
        // that is already published
        if (publishedGeneration && await _sheetGenerationPublished(sheetId, publishedGeneration)) {
            return { phase: 'done', frames: 0, elapsed_ms: 0, reconciled: true };
        }
        let check;
        try {
            check = await lambdaPost('storage', {
                job_id: jobId, task_prefix: taskId, expected: 1,
            }, '/check-status');
            failures = 0;
        } catch (e) {
            failures += 1;
            if (failures >= SHEET_POLL_MAX_CONSECUTIVE_FAILURES) {
                throw new Error(`status polling failed ${failures}x: ${e.message}`);
            }
            continue;
        }
        if (check.errors > 0) {
            const detail = check.error_details?.[0] || {};
            throw _sheetTerminalError(detail.error_msg || `sheet ${phaseLabel} failed`);
        }
        const rd = check.results?.[0] || {};
        const label = rd.phase_label || rd.phase || phaseLabel;
        if (label !== lastLabel) {
            lastLabel = label;
            lastProgressAt = Date.now();
        } else if (Date.now() - lastProgressAt > SHEET_STALL_DEADLINE_MS) {
            throw new Error(`${phaseLabel} made no progress for ${Math.round(SHEET_STALL_DEADLINE_MS / 60000)} minutes`);
        }
        if (statusEl) { statusEl.textContent = `Sheet ${sheetId}: ${label}`; statusEl.className = 'status'; }
        if (railId) _jobsRailUpsert({ id: railId, state: 'running', detail: label });
        if (rd.phase === 'done') return rd;
    }
}

const _sheetDzExports = {};   // sheet_id -> {dzi_url, share_url}
const _sheetDzInflight = {};  // sheet_id -> pending generation promise

function _sheetGenerateDeepZoom(sheetId, statusEl) {
    // dedupe: rapid re-clicks on a legacy sheet must not fire twice
    if (!_sheetDzInflight[sheetId]) {
        _sheetDzInflight[sheetId] = _sheetGenerateDeepZoomInner(sheetId, statusEl)
            .finally(() => { delete _sheetDzInflight[sheetId]; });
    }
    return _sheetDzInflight[sheetId];
}

async function _sheetGenerateDeepZoomInner(sheetId, statusEl) {
    const taskId = `sheet_dz_${sheetId}_${Date.now().toString(36)}`;
    const railId = 'sheetdz:' + sheetId;
    _jobsRailUpsert({
        id: railId, kind: 'sheet', label: `DeepZoom ${sheetId}`,
        jobId: sheetId, tab: 'sheets', state: 'running',
        startedAt: Date.now(), detail: 'dispatched',
    });
    try {
        const disp = await lambdaPost('dispatch', {
            target: 'deepzoom_export',
            jobs: [{ action: 'sheet', sheet_id: sheetId, job_id: sheetId,
                     export_id: 'dz_' + Date.now(), task_id: taskId }],
            expected_keys: [],
        });
        if ((disp.fired || 0) !== 1) throw new Error('deepzoom dispatch failed');
        const rd = await _pollSheetTask(sheetId, sheetId, taskId, statusEl, 'deepzoom', railId);
        if (rd.dzi_url) {
            _sheetDzExports[sheetId] = { dzi_url: rd.dzi_url, share_url: rd.share_url };
            if (statusEl) { statusEl.textContent = `Sheet ${sheetId}: DeepZoom ready`; statusEl.className = 'status ok'; }
        }
        _jobsRailUpsert({ id: railId, state: 'done', detail: 'ready' });
        return _sheetDzExports[sheetId] || null;
    } catch (e) {
        _jobsRailUpsert({ id: railId, state: 'error', detail: e.message });
        throw e;
    }
}

async function _sheetFindDeepZoom(sheetId) {
    /* CR35-F21: a listing FAILURE is not a miss. Only a successful
     * empty listing may trigger generation; transient failures retry
     * with backoff and then THROW so the caller reports instead of
     * launching a duplicate export. */
    if (_sheetDzExports[sheetId]) return _sheetDzExports[sheetId];
    let lastError = null;
    for (let attempt = 0; attempt < 3; attempt++) {
        if (attempt) await new Promise(r => setTimeout(r, 1500 * attempt));
        try {
            const resp = await lambdaPost('storage', {}, '/list-deepzoom');
            const hit = (resp.exports || []).find(e => {
                if (e.job_id !== sheetId || !e.dzi_url) return false;
                // The short-lived 1-bit pyramid format thresholded reduced
                // levels and made sparse sheets coarse and slow. Exclude it
                // so affected immutable exports regenerate once. Exports
                // predating tile_bitdepth used the correct 8-bit default.
                return e.tile_bitdepth == null || Number(e.tile_bitdepth) === 8;
            });
            if (hit) {
                _sheetDzExports[sheetId] = { dzi_url: hit.dzi_url, share_url: hit.share_url };
                return _sheetDzExports[sheetId];
            }
            return null;                     // authoritative miss
        } catch (e) {
            lastError = e;
        }
    }
    throw new Error(`DeepZoom listing failed: ${lastError && lastError.message}`);
}

async function _sheetDispatchControl(sheetId, generation, action) {
    /* Round-8 finding 5: dispatch a control op (cancel/abandon) until the
     * async event is ACCEPTED (fired === 1). Poly-Sheet's async lambda has
     * retries disabled server-side, so a dropped enqueue must be re-sent
     * by the client rather than silently lost. Returns true once accepted,
     * false after the bounded retries are exhausted. */
    for (let attempt = 0; attempt < 3; attempt++) {
        if (attempt) await new Promise(r => setTimeout(r, 500 * attempt));
        try {
            const resp = await lambdaPost('dispatch', {
                target: 'poly_sheet',
                jobs: [{ action, sheet_id: sheetId, generation }],
                expected_keys: [],
            });
            if ((resp.fired || 0) === 1) return true;
        } catch (e) { /* transient; retry */ }
    }
    return false;
}

async function _sheetConfirmRunTerminal(sheetId, generation) {
    /* Round-9 finding 5: an async 202 only proves the control event was
     * queued, not that run.json reached a terminal state. Confirm the run
     * record actually left 'running' for THIS generation (bounded
     * retries) before we clear local state — otherwise a discovery from
     * this or another client resurrects a ghost 'running' run. */
    return (await _sheetResolveRunStatus(sheetId, generation)) !== null;
}

async function _sheetResolveRunStatus(sheetId, generation) {
    /* Round-9 finding 1/2: run.json is the AUTHORITATIVE terminal record —
     * cancellation and publication compete through its CAS. Return the
     * resolved status for THIS generation once it leaves 'running'
     * ('cancelled'/'done'/'abandoned'/'failed'), 'superseded' if a newer
     * generation took over, or null while it is still running / unconfirmed
     * (bounded retries). The marker is only a worker-stop hint, never the
     * proof — confirming it (as the old code did) could clear a run that a
     * publish actually won. */
    for (let attempt = 0; attempt < 4; attempt++) {
        if (attempt) await new Promise(r => setTimeout(r, 400 * attempt));
        try {
            const resp = await fetch(
                _publicStorageUrl(`sheets/${sheetId}/run.json`) + '?t=' + Date.now(),
                { cache: 'no-store' });
            if (resp.ok) {
                const run = await resp.json();
                if (!run || run.generation !== generation) return 'superseded';
                // round-10 finding 6: require an EXPLICIT terminal status;
                // 'running' or any unknown/malformed value -> not-yet-terminal
                if (run.status && run.status !== 'running') {
                    return SHEET_TERMINAL_STATUSES.has(run.status)
                        ? run.status : null;
                }
            }
        } catch (e) { /* transient */ }
    }
    return null;
}

async function cancelPolySheet() {
    // thin wrapper: resolve the CURRENT run's identity, then issue a DIRECT
    // cancel for THAT specific run (a user command establishes intent).
    const statusEl = document.getElementById('sheets-status');
    // a run mid-backoff has no _activeSheetRun but IS persisted — cancel
    // must reach it (round-3 finding 4)
    const target = _activeSheetRun || _sheetRunLoad();
    if (!target) {
        if (statusEl) statusEl.textContent = 'No active sheet run.';
        return { ok: false, reason: 'no-active-run' };
    }
    return _cancelSheetRun(target.sheetId, target.generation, { direct: true });
}

async function _cancelSheetRun(sheetId, generation, opts) {
    /* Identity-keyed cancellation (round-13 findings 1/2). Returns a
     * STRUCTURED result so callers (the rail) can react rather than trusting
     * a silent undefined.
     *   opts.direct === true  -> a USER command: ESTABLISH the cancel intent
     *                            (even without a matching descriptor).
     *   opts.direct falsy     -> a DEFERRED RETRY: proceed only while the
     *                            intent is still active for this identity
     *                            (a superseded/cleared run must not be
     *                            re-cancelled). */
    const direct = !!(opts && opts.direct);
    const key = _sheetCancelKey(sheetId, generation);
    const statusEl = document.getElementById('sheets-status');
    if (direct) {
        // round-14 finding 2: PERSIST the intent (identity-keyed store), so a
        // rail cancel of a non-descriptor run survives a reload.
        _sheetCancelIntentAdd(key);
        // also mirror on the descriptor if it IS this run (keeps the existing
        // descriptor-driven resume path working)
        const d = _sheetRunLoad();
        if (d && d.sheetId === sheetId && d.generation === generation && !d.cancelRequested) {
            d.cancelRequested = true; _sheetRunSave(d);
        }
    } else if (!_sheetCancelIntents.has(key)) {
        // the run this retry targets is no longer being cancelled (cleared
        // or superseded) — do nothing, and do NOT touch any other run.
        return { ok: false, reason: 'intent-cleared' };
    }
    // round-16 finding 5 (status text): every status write from this
    // cancellation is suppressed while a NEWER run of the same sheet owns
    // the descriptor — a stale retry must never overwrite the live run's
    // status line (re-evaluated per write; the descriptor can change
    // across the awaits below).
    const staleForStatus = () => {
        const cur = _sheetRunLoad();
        return !!(cur && (cur.sheetId !== sheetId || cur.generation !== generation));
    };
    if (statusEl && !staleForStatus()) { statusEl.textContent = `Sheet ${sheetId}: cancelling...`; statusEl.className = 'status'; }
    // round-8/9 finding 5/1/2: confirm through the AUTHORITATIVE run.json,
    // not the best-effort marker. Order: dispatch-until-fired -> resolve
    // run.json status -> clear.
    const fired = await _sheetDispatchControl(sheetId, generation, 'cancel');
    if (!fired) {
        _sheetScheduleCancelRetry(sheetId, generation);
        if (statusEl && !staleForStatus()) { statusEl.textContent = `Sheet ${sheetId}: cancel not accepted — retrying...`; statusEl.className = 'status'; }
        return { ok: false, dispatched: false, pending: true, reason: 'not-accepted' };
    }
    const status = await _sheetResolveRunStatus(sheetId, generation);
    if (status === null) {
        // fired but run.json is still 'running' — keep retrying until the
        // run actually goes terminal (the cancel takes effect between frames)
        _sheetScheduleCancelRetry(sheetId, generation);
        if (statusEl && !staleForStatus()) { statusEl.textContent = `Sheet ${sheetId}: cancel dispatched — taking effect between frames...`; statusEl.className = 'status'; }
        return { ok: true, dispatched: true, pending: true, status: null };
    }
    // TERMINAL: retire this identity's intent (persisted) + its OWN timer
    // (never another run's), and clear the descriptor only if it is still
    // THIS run.
    _sheetCancelIntentDelete(key);
    _sheetClearCancelTimer(sheetId, generation);
    const still = _sheetRunLoad();
    if (still && still.sheetId === sheetId && still.generation === generation) {
        if (_sheetResumeTimer) { clearTimeout(_sheetResumeTimer); _sheetResumeTimer = null; }
        _sheetAbandonedGenerations.add(generation);
        _sheetRunClear();
    }
    // round-15 finding 3: resolve any sheet rail card for this run — a
    // DURABLE retry (not the original _jobsRailKill call) that reaches
    // terminal here must clear the card, not leave it "cancelling…" forever.
    // round-16 finding 5: generation-exact, so an old run's completion can
    // never corrupt a newer run's card.
    if (typeof _jobsRailResolveSheet === 'function') {
        _jobsRailResolveSheet(sheetId, generation, status);
    }
    // round-16 finding 5 (status text): a stale completion message must not
    // misdescribe the live run — stay silent when a newer run owns the line.
    if (statusEl && !staleForStatus()) {
        if (status === 'cancelled') {
            statusEl.textContent = `Sheet ${sheetId}: cancelled.`;
            statusEl.className = 'status';
        } else if (status === 'done') {
            statusEl.textContent = `Sheet ${sheetId}: already published before the cancel took effect.`;
            statusEl.className = 'status ok';
        } else {
            statusEl.textContent = `Sheet ${sheetId}: run ended (${status}).`;
            statusEl.className = 'status';
        }
    }
    return { ok: true, dispatched: true, status };
}

function _sheetScheduleCancelRetry(sheetId, generation) {
    /* Round-13 finding 1: each run has its OWN timer slot (keyed by
     * identity), so a stale retry can never clear or replace a NEWER run's
     * timer. Only schedule while THIS identity's intent is still active. */
    const key = _sheetCancelKey(sheetId, generation);
    if (!_sheetCancelIntents.has(key)) return;
    const existing = _sheetCancelTimers.get(key);
    if (existing !== undefined) clearTimeout(existing);
    _sheetCancelTimers.set(key, setTimeout(
        () => { void _cancelSheetRun(sheetId, generation); }, SHEET_RESUME_BACKOFF_MS));
}

async function _sheetDiscoverServerRun(rows) {
    /* Round-3 finding 6: a crash between admission and _sheetRunSave (or
     * cleared storage) loses the local descriptor, but run.json now
     * carries the admitted payload — rebuild the descriptor from the
     * server record and resume. */
    if (_sheetRunLoad() || _activeSheetRun) return;
    const running = (rows || []).find(r => r.run_status === 'running' && r.run_key
        && !_sheetAbandonedGenerations.has(r.run_generation));
    if (!running) return;
    try {
        const resp = await fetch(_publicStorageUrl(running.run_key) + '?t=' + Date.now());
        if (!resp.ok) return;
        const run = await resp.json();
        if (!run || run.status !== 'running' || !run.payload) return;
        const desc = {
            sheetId: run.sheet_id,
            jobId: run.job_id,
            generation: run.generation,
            steps: run.steps,
            funcName: run.payload.function || '',
            n: run.payload.frame?.n || 0,
            workers: run.workers,
            stitchTask: run.stitch_task_id,
            startedAtS: run.created_at_s || Date.now() / 1000,
            payload: { ...run.payload, job_id: run.job_id, sheet_id: run.sheet_id,
                       generation: run.generation },
        };
        _sheetRunSave(desc);
        const statusEl = document.getElementById('sheets-status');
        if (statusEl) {
            statusEl.textContent = `Sheet ${run.sheet_id}: found an in-flight server run — resuming.`;
            statusEl.className = 'status';
        }
        void resumeSheetRun();
    } catch (e) { /* discovery is best-effort */ }
}

async function loadSheetsTab() {
    _sheetGeometryChanged();
    void resumeSheetRun();
    // round-14 finding 2: re-issue any cancellation intents that a reload
    // stripped of their in-memory retry timers (identity-scoped, safe).
    void _sheetResumePersistedCancels();
    const invEl = document.getElementById('sheets-inventory');
    try {
        const resp = await lambdaPost('storage', {}, '/list-sheets');
        _sheetsInventory = resp.sheets || [];
        void _sheetDiscoverServerRun(_sheetsInventory);
    } catch (e) {
        if (invEl) invEl.textContent = 'Failed to list sheets: ' + e.message;
        return;
    }
    if (!invEl) return;
    if (_sheetsInventory.length === 0) {
        invEl.innerHTML = '<div style="padding:8px; color:#888">No sheets yet.</div>';
        return;
    }
    invEl.innerHTML = _sheetsInventory.map(row =>
        `<div class="sheet-row" data-sheet-id="${_escapeHtml(row.sheet_id)}" ` +
        `style="padding:4px 8px; cursor:pointer; border-bottom:1px solid #222" ` +
        `onclick="_viewSheet('${_escapeHtml(row.sheet_id)}')">` +
        `${_escapeHtml(row.sheet_id)} <span style="color:#666">${_escapeHtml(row.modified.slice(0, 19))}</span></div>`
    ).join('');
    _sheetMarkSelectedRow();
}

function _sheetMarkSelectedRow() {
    const invEl = document.getElementById('sheets-inventory');
    if (!invEl) return;
    for (const rowEl of invEl.querySelectorAll('.sheet-row')) {
        rowEl.classList.toggle('selected', rowEl.dataset.sheetId === _sheetViewerId);
    }
}

let _sheetViewerId = null;
let _sheetViewSeq = 0;
let _sheetOsd = null;
let _sheetViewerManifest = null;
let _sheetContext = null;   // {sheetId, frame} while the popup is open

function _sheetShowOsd(dziUrl) {
    const el = document.getElementById('sheet-osd');
    if (!el) return;
    if (_sheetOsd) {
        _sheetOsd.destroy();
        _sheetOsd = null;
    }
    el.innerHTML = '';
    if (!el.dataset.sheetCtxBound) {
        el.addEventListener('contextmenu', _sheetOsdContextMenu);
        el.dataset.sheetCtxBound = '1';
    }
    _sheetOsd = OpenSeadragon({
        element: el,
        tileSources: dziUrl,
        prefixUrl: 'https://cdnjs.cloudflare.com/ajax/libs/openseadragon/4.1.1/images/',
        showNavigator: true,
        navigatorPosition: 'BOTTOM_RIGHT',
        navigatorMaintainSizeRatio: false,
        navigatorWidth: '160px',
        navigatorHeight: '160px',
        maxZoomPixelRatio: 4,
    });
}

async function _sheetDownload(btn) {
    if (!_sheetViewerId) return;
    const orig = btn ? btn.textContent : 'Download';
    if (btn) { btn.disabled = true; btn.textContent = 'Fetching...'; }
    try {
        const resp = await fetch(_publicStorageUrl(_sheetResolvedKeys(_sheetViewerId).pngKey));
        if (!resp.ok) throw new Error('HTTP ' + resp.status);
        const blob = await resp.blob();
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `${_sheetViewerId}.png`;
        document.body.appendChild(a);
        a.click();
        a.remove();
        setTimeout(() => URL.revokeObjectURL(url), 10000);
        if (btn) { btn.textContent = '✓ Saved'; setTimeout(() => { btn.textContent = orig; btn.disabled = false; }, 1500); }
    } catch (e) {
        if (btn) { btn.textContent = 'Failed: ' + e.message; setTimeout(() => { btn.textContent = orig; btn.disabled = false; }, 2500); }
    }
}

function _sheetResolvedKeys(sheetId) {
    // the inventory row carries the pointer-resolved keys (list-sheets
    // follows run.json.published_*); legacy sheets fall back to fixed keys
    const row = (_sheetsInventory || []).find(r => r.sheet_id === sheetId);
    return {
        pngKey: (row && row.png_key) || `sheets/${sheetId}/sheet.png`,
        manifestKey: (row && row.manifest_key) || `sheets/${sheetId}/sheet.json`,
    };
}

function _viewSheet(sheetId) {
    const viewer = document.getElementById('sheet-viewer');
    const meta = document.getElementById('sheet-viewer-meta');
    if (!viewer) return;
    _sheetViewerId = sheetId;
    const seq = ++_sheetViewSeq;   // rapid list clicks: only the latest wins
    _sheetViewerManifest = null;
    _sheetContextClose();
    _sheetMarkSelectedRow();
    viewer.style.display = '';
    const keys = _sheetResolvedKeys(sheetId);
    if (meta) {
        meta.textContent = sheetId + ' (loading manifest...)';
        fetch(_publicStorageUrl(keys.manifestKey) + '?t=' + Date.now())
            .then(r => r.json())
            .then(m => {
                if (seq !== _sheetViewSeq) return;
                _sheetViewerManifest = m;
                const scansDesc = (m.scans || [m.scan]).map(sc =>
                    `${sc.token} ${sc.from}..${sc.spacing === 'step' ? `+${sc.step}·k` : sc.to} (${sc.spacing})`).join(' × ');
                meta.textContent = `${sheetId}: ${m.frames} frames · ${m.grid.cols}x${m.grid.rows} · ` +
                    `N=${m.n} tile=${m.tile_px}px · ${m.solver_mode} · ` +
                    `${scansDesc} · ` +
                    `viewport ${m.viewport.mode} · ${m.elapsed_ms}ms`;
            })
            .catch(() => { if (seq === _sheetViewSeq) meta.textContent = sheetId; });
    }
    void _sheetViewDeepZoom(sheetId, seq);
}

async function _sheetViewDeepZoom(sheetId, seq) {
    const statusEl = document.getElementById('sheets-status');
    const osdEl = document.getElementById('sheet-osd');
    try {
        let exp = await _sheetFindDeepZoom(sheetId);
        if (!exp) {
            // legacy sheet with no pyramid yet: generate it in place
            if (seq === _sheetViewSeq && osdEl && !_sheetOsd) {
                osdEl.innerHTML = '<div style="color:#888; font-size:12px; padding:16px">Generating DeepZoom for ' + _escapeHtml(sheetId) + '...</div>';
            }
            if (statusEl) { statusEl.textContent = `Sheet ${sheetId}: generating DeepZoom...`; statusEl.className = 'status'; }
            exp = await _sheetGenerateDeepZoom(sheetId, statusEl);
        }
        if (!exp || !exp.dzi_url) throw new Error('no DZI produced');
        if (seq !== _sheetViewSeq) return;   // user moved on to another sheet
        _sheetShowOsd(exp.dzi_url);
    } catch (e) {
        if (seq !== _sheetViewSeq) return;
        if (statusEl) { statusEl.textContent = `Sheet ${sheetId}: DeepZoom failed — ${e.message}`; statusEl.className = 'status error'; }
    }
}

function _setInputValue(id, value) {
    const el = document.getElementById(id);
    if (el != null && value != null) el.value = String(value);
}

async function populateSelectedSheet(btn) {
    const statusEl = document.getElementById('sheets-status');
    if (!_sheetViewerId) {
        if (statusEl) { statusEl.textContent = 'Select a sheet first.'; statusEl.className = 'status error'; }
        return;
    }
    const sheetId = _sheetViewerId;
    const orig = btn ? btn.textContent : 'Populate';
    if (btn) { btn.disabled = true; btn.textContent = 'Populating...'; }
    try {
        const resp = await fetch(_publicStorageUrl(_sheetResolvedKeys(sheetId).manifestKey) + '?t=' + Date.now());
        if (!resp.ok) throw new Error('manifest HTTP ' + resp.status);
        const m = await resp.json();

        // sheet scan + frame knobs first (the compute populate switches tabs)
        const scans = m.scans || (m.scan ? [m.scan] : []);
        ['', '2'].forEach((sfx, i) => {
            const sc = scans[i];
            _setInputValue('sheet-token' + sfx, sc ? sc.token : '');
            if (!sc) {
                const tok = document.getElementById('sheet-token' + sfx);
                if (tok) tok.value = '';
                return;
            }
            _setInputValue('sheet-from' + sfx, sc.from);
            _setInputValue('sheet-to' + sfx, sc.to);
            if (sc.spacing === 'step') _setInputValue('sheet-step' + sfx, sc.step);
            _setInputValue('sheet-steps' + sfx, sc.steps);
            _setInputValue('sheet-spacing' + sfx, sc.spacing);
        });
        _sheetSpacingChanged();
        _setInputValue('sheet-cols', m.grid?.cols);
        _setInputValue('sheet-polarity', m.polarity);
        _setInputValue('sheet-margin', m.margin_px);
        const freezeEl = document.getElementById('sheet-freeze');
        if (freezeEl) freezeEl.checked = m.viewport?.mode === 'frozen';
        const labelEl = document.getElementById('sheet-label');
        if (labelEl) labelEl.checked = !!m.label;

        // the inherited compute-preview controls (presence-aware: an
        // explicit zero shim/quantile round-trips as zero, CR35-F15)
        _setInputValue('compute-preview-n', m.n);
        _setInputValue('compute-preview-size', m.tile_px);
        _setInputValue('compute-preview-rotate', m.rotate);
        _setInputValue('compute-preview-solver', m.solver_mode);
        if (m.solver_iters != null) _setInputValue('compute-preview-iters', m.solver_iters);
        if (m.viewport) {
            if (m.viewport.quantile != null) _setInputValue('compute-preview-quantile', m.viewport.quantile * 100);
            if (m.viewport.shim != null) _setInputValue('compute-preview-shim', m.viewport.shim * 100);
        }
        if (m.viewport?.mode === 'explicit' && Array.isArray(m.viewport.explicit)) {
            // full round-trip: stored numeric bounds seed the marquee
            _seedComputePreviewMarquee(m.viewport.explicit);
        } else if (m.viewport?.mode !== 'explicit') {
            const radio = document.querySelector('input[name="compute-preview-viewport-mode"][value="quantile"]');
            if (radio) { radio.checked = true; _setComputePreviewViewportMode('quantile'); }
        }
        if (typeof _applyComputePreviewRotation === 'function') _applyComputePreviewRotation();
        _sheetGeometryChanged();

        // pipeline (function + programs + cfpv) into the Compute tab
        _populateComputeFromDetail(sheetId, {
            pipeline: m.pipeline || {},
            calc: { solver: m.solver_mode },
        });
        if (statusEl) {
            statusEl.textContent = `Populated Compute + Sheets controls from ${sheetId}.`;
            statusEl.className = 'status ok';
        }
        if (btn) { btn.textContent = '✓ Populated'; setTimeout(() => { btn.textContent = orig; btn.disabled = false; }, 1500); }
    } catch (e) {
        if (statusEl) { statusEl.textContent = 'Populate failed: ' + e.message; statusEl.className = 'status error'; }
        if (btn) { btn.textContent = orig; btn.disabled = false; }
    }
}

function _sheetsArrowNav(e) {
    // same convention as the Results / Palette / Render artifact tables
    // (bound from js/12-deepzoom-boot.js — parts are declaration-only)
    const tab = document.getElementById('tab-sheets');
    if (!tab || !tab.classList.contains('active')) return;
    if (e.key !== 'ArrowUp' && e.key !== 'ArrowDown') return;
    if (_isTextInputFocused()) return;
    if (!_sheetsInventory.length) return;
    e.preventDefault();

    const idx = _sheetViewerId
        ? _sheetsInventory.findIndex(r => r.sheet_id === _sheetViewerId)
        : -1;
    let next;
    if (e.key === 'ArrowDown') next = idx < _sheetsInventory.length - 1 ? idx + 1 : idx;
    else next = idx > 0 ? idx - 1 : 0;
    if (next < 0) next = 0;
    const row = _sheetsInventory[next];
    if (!row || row.sheet_id === _sheetViewerId) return;
    _viewSheet(row.sheet_id);
    const rowEl = document.querySelector(
        `#sheets-inventory .sheet-row[data-sheet-id="${CSS.escape(row.sheet_id)}"]`);
    if (rowEl && typeof rowEl.scrollIntoView === 'function') rowEl.scrollIntoView({ block: 'nearest' });
}

function _sheetFramePickFromImagePoint(m, x, y) {
    /* Image-pixel point -> frame index, or null on gutters/borders/
     * out-of-grid. Pure: exercised directly by the e2e harness. */
    if (!m || !m.grid || !m.tile_px) return null;
    const margin = m.margin_px || 0;
    const span = m.tile_px + margin;
    const cx = x - margin;
    const cy = y - margin;
    if (cx < 0 || cy < 0) return null;
    const col = Math.floor(cx / span);
    const row = Math.floor(cy / span);
    if (col >= m.grid.cols || row >= m.grid.rows) return null;
    if (cx - col * span >= m.tile_px || cy - row * span >= m.tile_px) return null;
    const k = row * m.grid.cols + col;
    return k < m.frames ? k : null;
}

function _sheetValueLiteral(v) {
    // mirror of the server's _value_literal: integral values spell as
    // integers, negatives as (0-x) — the grammar has no unary minus
    const body = Number.isInteger(v) ? String(Math.abs(v)) : String(Math.abs(v));
    return v < 0 ? `(0-${body})` : body;
}

function _sheetOsdContextMenu(e) {
    e.preventDefault();
    const m = _sheetViewerManifest;
    const el = document.getElementById('sheet-osd');
    if (!m || !_sheetOsd || !el) { _sheetContextClose(); return; }
    const rect = el.getBoundingClientRect();
    const web = new OpenSeadragon.Point(e.clientX - rect.left, e.clientY - rect.top);
    const img = _sheetOsd.viewport.viewportToImageCoordinates(_sheetOsd.viewport.pointFromPixel(web));
    const k = _sheetFramePickFromImagePoint(m, img.x, img.y);
    if (k == null) { _sheetContextClose(); return; }
    _sheetContext = { sheetId: _sheetViewerId, frame: k };
    _sheetRenderContextMenu(e.clientX, e.clientY);
}

function _sheetContextClose() {
    _sheetContext = null;
    const menu = document.getElementById('sheet-context-menu');
    if (menu) {
        menu.style.display = 'none';
        menu.setAttribute('aria-hidden', 'true');
        menu.innerHTML = '';
    }
}

function _sheetContextDismissClick(e) {
    if (!_sheetContext) return;
    const menu = document.getElementById('sheet-context-menu');
    if (menu && e && e.target && menu.contains(e.target)) return;
    _sheetContextClose();
}

function _sheetContextEscape(e) {
    if (e && e.key === 'Escape') _sheetContextClose();
}

function _sheetRenderContextMenu(x, y) {
    const menu = document.getElementById('sheet-context-menu');
    const m = _sheetViewerManifest;
    if (!menu || !m || !_sheetContext) return;
    const k = _sheetContext.frame;
    const rec = (m.frame_records || [])[k] || {};
    const values = rec.values || (rec.value != null ? [rec.value] : []);
    const scans = m.scans || (m.scan ? [m.scan] : []);
    const canPopulate = !!m.pipeline;
    const rows = [
        `<div class="artifact-mosaic-menu-row"><span>frame</span><code>${k} of ${m.frames}</code></div>`,
        ...scans.map((sc, i) =>
            `<div class="artifact-mosaic-menu-row"><span>${_escapeHtml(sc.token)}</span><code>${_escapeHtml(String(values[i]))}</code></div>`),
        rec.bounds
            ? `<div class="artifact-mosaic-menu-row"><span>bounds</span><code>${rec.bounds.map(b => Number(b).toPrecision(4)).join(', ')}</code></div>`
            : '',
    ].filter(Boolean).join('');
    menu.innerHTML = `
        <div class="artifact-mosaic-menu-head">
            <div class="artifact-mosaic-menu-title">Sheet frame</div>
            <button type="button" class="artifact-mosaic-menu-close" data-sheet-action="close" aria-label="Close">x</button>
        </div>
        <div class="artifact-mosaic-menu-meta">${rows}</div>
        <div class="artifact-mosaic-menu-actions">
            <button type="button" class="artifact-mosaic-menu-action" data-sheet-action="populate-frame"${canPopulate ? '' : ' disabled'}>Populate Frame</button>
            <button type="button" class="artifact-mosaic-menu-action" data-sheet-action="close">Close</button>
        </div>
        ${canPopulate ? '' : '<div class="artifact-mosaic-menu-note err">manifest has no pipeline (sheet predates v1.5) — re-run it to enable Populate</div>'}
    `;
    menu.style.display = 'block';
    menu.setAttribute('aria-hidden', 'false');
    const rect = menu.getBoundingClientRect();
    const vw = window.innerWidth || 1200;
    const vh = window.innerHeight || 800;
    menu.style.left = `${Math.max(8, Math.min(x, vw - (rect.width || 260) - 8))}px`;
    menu.style.top = `${Math.max(8, Math.min(y, vh - (rect.height || 220) - 8))}px`;
    if (!menu._sheetBound) {
        menu.addEventListener('click', (ev) => {
            const btn = ev.target && ev.target.closest ? ev.target.closest('[data-sheet-action]') : null;
            if (!btn) return;
            ev.preventDefault();
            const action = btn.getAttribute('data-sheet-action');
            if (action === 'close') _sheetContextClose();
            else if (action === 'populate-frame') _sheetPopulateFrame();
        });
        menu._sheetBound = true;
    }
}

function _sheetPopulateFrame() {
    const statusEl = document.getElementById('sheets-status');
    const m = _sheetViewerManifest;
    const ctx = _sheetContext;
    _sheetContextClose();
    if (!m || !ctx || !m.pipeline) return;
    const k = ctx.frame;
    const rec = (m.frame_records || [])[k] || {};
    const values = rec.values || (rec.value != null ? [rec.value] : []);
    const scans = m.scans || (m.scan ? [m.scan] : []);

    // the clicked frame's exact sources: every token replaced by the
    // literal value that rendered it (same spelling the server used)
    const pipeline = { ...m.pipeline };
    // ONE lexical longest-match pass, mirroring the backend: split/join
    // per token let $T corrupt $T2 (review round-2 finding)
    const tokenMap = {};
    scans.forEach((sc, i) => {
        if (values[i] != null) tokenMap[sc.token] = _sheetValueLiteral(values[i]);
    });
    const tokens = Object.keys(tokenMap).sort((a, b) => b.length - a.length);
    if (tokens.length) {
        const esc = tokens.map(t => t.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'));
        const pattern = new RegExp('(?:' + esc.join('|') + ')(?![A-Za-z0-9_])', 'g');
        for (const field of ['param_program_source_text', 'coeff_program_source_text']) {
            const text = pipeline[field];
            if (typeof text !== 'string' || !text) continue;
            pipeline[field] = text.replace(pattern, (mtok) => tokenMap[mtok]);
        }
    }

    // preview controls, as in populateSelectedSheet
    _setInputValue('compute-preview-n', m.n);
    _setInputValue('compute-preview-size', m.tile_px);
    _setInputValue('compute-preview-rotate', m.rotate);
    _setInputValue('compute-preview-solver', m.solver_mode);
    if (m.solver_iters != null) _setInputValue('compute-preview-iters', m.solver_iters);
    if (m.viewport) {
        if (m.viewport.quantile != null) _setInputValue('compute-preview-quantile', m.viewport.quantile * 100);
        if (m.viewport.shim != null) _setInputValue('compute-preview-shim', m.viewport.shim * 100);
    }
    // the clicked frame's ACTUAL bounds become the preview viewport —
    // Populate Frame reconstructs the tile exactly, whatever mode
    // framed it (CR35-F22)
    if (Array.isArray(rec.bounds)) _seedComputePreviewMarquee(rec.bounds);
    if (typeof _applyComputePreviewRotation === 'function') _applyComputePreviewRotation();

    _populateComputeFromDetail(ctx.sheetId, {
        pipeline,
        calc: { solver: m.solver_mode },
    });
    if (statusEl) {
        const desc = scans.map((sc, i) => `${sc.token}=${values[i]}`).join(', ');
        statusEl.textContent = `Populated Compute from ${ctx.sheetId} frame ${k} (${desc} substituted into the sources).`;
        statusEl.className = 'status ok';
    }
}

// round-15 finding 4: the boot call that resumes persisted cancellation
// intents lives in the js/12 boot block (parts are declarations-only;
// see tests/test_frontend_parts_contract.py) — search "resumePersistedCancels".

;(window.__ppParts = window.__ppParts || []).push('16-poly-sheets');
