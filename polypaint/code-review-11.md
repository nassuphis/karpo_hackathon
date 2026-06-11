# Code Review 11 — libvips/vipsdeps Follow-Up

Reviewed: 2026-06-11  
Commit reviewed: `475f771 Fix render DNS failure + vendor libarchive crypto chain into /opt/vipsdeps`

## Findings

### [blocker] `/opt/bin/vips` and `/opt/bin/vipsthumbnail` still do not use `/opt/vipsdeps`

`lambda/build-libvips-layer.sh:83-89` copies the `vips` and `vipsthumbnail` CLI binaries from the libvips install tree directly into the layer. Nothing patches their RPATH/RUNPATH afterward. The new RPATH work in `deploy.sh:720-724` applies only to the custom C binaries compiled in deploy (`raw2jpeg`, `score_raw_render`, `dz_export`, etc.), not to the layer-provided CLI tools.

I checked the actual ELF dynamic tags in the current tree:

```text
lambda/raw2jpeg                           RPATH=/opt/lib:/opt/vipsdeps
lambda/score_raw_render                   RPATH=/opt/lib:/opt/vipsdeps
lambda/bilevel_merge                      RPATH=/opt/lib:/opt/vipsdeps
lambda/raw_to_bilevel                     RPATH=/opt/lib:/opt/vipsdeps
lambda/tiff_compat                        RPATH=/opt/lib:/opt/vipsdeps
lambda/png_export                         RPATH=/opt/lib:/opt/vipsdeps
lambda/dz_export                          RPATH=/opt/lib:/opt/vipsdeps
lambda/autolevels_render                  RPATH=/opt/lib:/opt/vipsdeps

lambda/layer-build/bin/vips               RPATH=<empty>, RUNPATH=<empty>
lambda/layer-build/bin/vipsthumbnail      RPATH=<empty>, RUNPATH=<empty>
lambda/layer-build/lib/libvips.so.42.18.1 RPATH=<empty>, RUNPATH=<empty>
lambda/layer-build/lib/libarchive.so.13.7.4 RPATH=<empty>, RUNPATH=<empty>
```

This matters because `libvips.so` eagerly needs `libarchive.so.13`, and `libarchive.so.13` needs `libcrypto.so.3`, `liblzma.so.5`, `libzstd.so.1`, and `liblz4.so.1`. With only `LD_LIBRARY_PATH=/opt/lib`, the CLI path can still resolve libarchive's crypto chain from the pinned runtime system instead of `/opt/vipsdeps`.

This is not theoretical: multiple handlers execute these CLI tools directly:

- `lambda/handler_render_preview.py:103-113` runs `/opt/bin/vipsthumbnail`.
- `lambda/handler_palette_finalize.py:307-313` runs `/opt/bin/vipsthumbnail`.
- `lambda/handler_resize_artifact.py:230-260` runs `/opt/bin/vips`.
- `lambda/handler_resize_artifact.py:418-424` runs `/opt/bin/vipsthumbnail`.
- `lambda/handler_tiff_compat.py:105-117` runs `/opt/bin/vipsthumbnail`.
- `lambda/handler_png_export.py:105-117` runs `/opt/bin/vipsthumbnail`.
- `lambda/handler_repalette.py:319-327` runs `/opt/bin/vipsthumbnail`.
- `lambda/handler_palette_debug.py:204-212` runs `/opt/bin/vipsthumbnail`.

`lambda/shared.py:142-148` only ensures `/opt/lib` is in `LD_LIBRARY_PATH`; it does not include `/opt/vipsdeps`.

Impact: the custom binaries are now hermetic for libarchive's crypto chain, but the CLI-based preview/resize paths are still exposed to the same pinned-runtime coupling class CR10 was trying to eliminate. A render preview or resize path could still fail after a future layer rebuild if runtime libcrypto is older than the libarchive chain.

Recommended fix: make the CLI tools hermetic too. Preferred options:

- Patch `/out/bin/vips` and `/out/bin/vipsthumbnail` in `build-libvips-layer.sh` with DT_RPATH `/opt/lib:/opt/vipsdeps` after copying them, using `patchelf --force-rpath --set-rpath /opt/lib:/opt/vipsdeps`.
- Or patch the relevant shared object in the layer (`libvips.so` or `libarchive.so`) so its transitive chain resolves `/opt/vipsdeps`.
- Or, less cleanly, update `imgpipe_env()` to put `/opt/vipsdeps:/opt/lib` into the subprocess environment. This keeps Python parent processes safe only if it is used strictly for child processes, not Lambda function env vars.

Add a Docker assertion that `/opt/bin/vips` and `/opt/bin/vipsthumbnail` resolve `libcrypto`, `liblzma`, `libzstd`, and `liblz4` from `/opt/vipsdeps`, not from `/lib64` or `/usr/lib64`.

### [concern] Docker runtime harness does not install `/opt/vipsdeps` into the simulated Lambda `/opt`

`scripts/test-docker-runtime.sh:60-66` merges layers into the container like this:

```bash
cp -a /opt-lapack/lib /opt/
cp -a /opt-vips/lib/* /opt/lib/
mkdir -p /opt/bin
cp -a /opt-vips/bin/* /opt/bin/
export LD_LIBRARY_PATH=/src/assemble_greyscale_lib:/src/solve_palette_chunk_mt_lib:/src/solve_proximity_hist_sectioned_lib:/src/roots2pix_mt_lib:/opt/lib
```

It does not copy `/opt-vips/vipsdeps` to `/opt/vipsdeps`. That means the test container is not exercising the deployed layer layout. The custom binaries' new RPATH points at `/opt/vipsdeps`, but that directory is absent in the harness; if tests pass, they are passing by resolving libarchive's crypto chain from the container's system libraries, not by proving the vendored chain works.

`tests/docker_runtime_regression.py:3423-3428` checks that `/src/layer-build/vipsdeps` exists in the mounted source tree, but that is not the runtime path the binary loader uses. It verifies the build output exists, not that the simulated Lambda runtime can load from it.

Recommended fix:

- In `scripts/test-docker-runtime.sh`, copy `cp -a /opt-vips/vipsdeps /opt/` before running tests.
- Add an explicit `ldd`/loader-path assertion for each custom libvips binary and both CLI tools: `libcrypto.so.3`, `liblzma.so.5`, `libzstd.so.1`, and `liblz4.so.1` must resolve under `/opt/vipsdeps`.
- Make missing `/opt/vipsdeps` fatal when the current build expects it.

### [concern] Docker test still contaminates the Python test process with staged libcurl closures

`scripts/test-docker-runtime.sh:65` puts `/src/assemble_greyscale_lib`, `/src/solve_palette_chunk_mt_lib`, `/src/solve_proximity_hist_sectioned_lib`, and `/src/roots2pix_mt_lib` on `LD_LIBRARY_PATH` for the whole Python regression process.

That is the exact class of environment contamination the deploy fix is trying to avoid. The actual Lambda functions should rely on binary DT_RPATH for these staged closures, not `LD_LIBRARY_PATH`. The Docker test asserts the binaries have DT_RPATH later, but the process environment still makes staged OpenSSL visible to Python during the whole test run.

Recommended fix:

- Remove the `/src/*_lib` directories from the global Docker `LD_LIBRARY_PATH`.
- Let the libcurl binaries load their adjacent closures through DT_RPATH.
- If a specific test must run `ldd` with extra paths, scope that to the one subprocess, not the global Python environment.

This would make the Docker runtime harness match the production rule more closely: `/opt/lib` for layers only, no bundle paths in `LD_LIBRARY_PATH`.

### [concern] `vipsdeps` is optional in the Docker assertion even though the current deploy design requires it

`tests/docker_runtime_regression.py:3423-3430` treats a missing `/src/layer-build/vipsdeps` as a non-fatal pre-vendoring state:

```python
if os.path.isdir(vipsdeps):
    ...
else:
    print("  layer-build/vipsdeps: ABSENT (pre-vendoring layer; rebuild to adopt CR10)")
```

For the current commit, `vipsdeps` is no longer optional. `deploy.sh:720-724` links custom libvips binaries with `RPATH=/opt/lib:/opt/vipsdeps`, and `lambda/build-libvips-layer.sh:146-182` packages `vipsdeps/` into the layer. A missing `vipsdeps` directory should fail the Docker gate.

Recommended fix: make absence fatal and verify the deployed-path copy under `/opt/vipsdeps`, not only the source mount under `/src/layer-build/vipsdeps`.

## Confirmed Good

The libcurl staged-closure fix is correctly reflected in deploy. `deploy.sh:654-665` excludes glibc-family libraries including `libresolv.so.*` and `libnss_*.so.*`. The current staged libcurl directories contain no `libresolv`/`libnss` files. They still contain `libssl`/`libcrypto`, which is acceptable for DT_RPATH self-resolution as long as those directories are not on Lambda `LD_LIBRARY_PATH`.

The custom libvips binaries are correctly rebuilt with DT_RPATH. The current ELF tags for all eight custom binaries show `RPATH=/opt/lib:/opt/vipsdeps` and no RUNPATH.

The layer zip does include `vipsdeps/` with the expected chain:

```text
vipsdeps/libcrypto.so.3
vipsdeps/liblzma.so.5
vipsdeps/libzstd.so.1
vipsdeps/liblz4.so.1
```

`lambda/layer-build/lib` does not contain `libcrypto`, `libssl`, `libresolv`, or `libnss_*`, which preserves the Python parent-process safety rule for `/opt/lib`.

The `assemble_greyscale` error-message change is correct. `lambda/assemble_greyscale.c` now puts curl rc, strerror, HTTP status, and curl error detail before the URL, so long presigned URLs cannot truncate away the useful diagnostic.

## Verification Run

- `python3 lambda/gen_coeff_vocab.py --check`  
  Result: `coeff_vocab_js.js: OK`

- `python3 -m py_compile lambda/handler_render_preview.py lambda/handler_palette_finalize.py lambda/handler_resize_artifact.py lambda/handler_autolevels.py lambda/handler_tiff_compat.py lambda/handler_png_export.py lambda/handler_repalette.py lambda/handler_palette_debug.py`  
  Result: passed

- `bash -n deploy.sh lambda/build-libvips-layer.sh scripts/test-docker-runtime.sh scripts/predeploy_check.sh scripts/postdeploy_init_check.sh`  
  Result: passed

- `bash scripts/predeploy_check.sh`  
  Result: `403 passed, 2 subtests passed in 2.28s`, frontend source/runtime checks passed, predeploy gate passed

- `bash scripts/test-docker-runtime.sh`  
  Result: passed. Caveat: this pass is not decisive for the new `/opt/vipsdeps` guarantee because the harness does not copy `/opt-vips/vipsdeps` to `/opt/vipsdeps` and still exposes staged libcurl closures through global `LD_LIBRARY_PATH`.

- `git diff --check`  
  Result: clean

## Verdict

Do not treat this as deploy-clean yet. The custom libvips binaries are fixed, but the layer CLI tools (`vips`, `vipsthumbnail`) remain coupled to runtime-system resolution for libarchive's crypto chain, and the Docker harness currently cannot catch that because it does not reproduce the deployed `/opt/vipsdeps` layout.

The next change should make CLI libvips tools hermetic, copy `vipsdeps/` into the Docker `/opt`, remove staged bundle dirs from global test `LD_LIBRARY_PATH`, and assert actual loader resolution paths. After that, rerun the Docker runtime regression and predeploy gate.

---

## Resolution — 2026-06-11

**[blocker] CLI tools — ✅ fixed.** `/opt/bin/vips` and `/opt/bin/vipsthumbnail`
now carry `RPATH=/opt/lib:/opt/vipsdeps`: the existing layer's tools were
patched in place with patchelf (verified via readelf) and the layer re-zipped,
and `build-libvips-layer.sh` gained a permanent patchelf step so every future
rebuild produces hermetic CLI tools.

**[concern] harness layout — ✅ fixed.** `test-docker-runtime.sh` now copies
`vipsdeps/` into the simulated `/opt` and treats its absence as fatal.

**[concern] env contamination — ✅ fixed.** The staged `*_lib` dirs are gone
from the global `LD_LIBRARY_PATH`; the test process runs with the production
rule (`/opt/lib` only). The staged closures are instead exercised exactly as
production loads them: each libcurl binary is staged into a zip layout
(binary + `lib/`) and `ldd` with **no environment** must resolve the whole
closure from `$ORIGIN/lib`.

**[concern] optional vipsdeps — ✅ fixed.** Absence is now fatal in both the
python gate and the harness, and the harness asserts the deployed-path copy.

New loader assertions in the gate, all passing: 4 hermetic zip-layout
closures; libcrypto/lzma/zstd/lz4 resolving from `/opt/vipsdeps` for all 8
compiled vips binaries **and both CLI tools**. Full Docker regression,
predeploy gate, and frontend harness green after the changes.
