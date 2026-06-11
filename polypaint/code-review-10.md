# Code Review 10 — Native Library Dependency Audit

**Date:** 2026-06-11
**Question under review:** are the libvips/LAPACK/libcurl library
dependencies properly exposed, and will more pinned-runtime version errors
appear?
**Method:** enumerated every shipped library in both layers and all staged
closures; resolved every binary's full dependency graph (`ldd`) inside the
Lambda runtime image, with and without the production env; probed the
runtime python's dlopen surface under each `LD_LIBRARY_PATH`; compared
shadowed library versions layer-vs-runtime.

## The dependency matrix

Four link modes exist; each has a different exposure profile:

| Mode | Binaries | Library source | Exposure |
| --- | --- | --- | --- |
| Static musl | sweep, sweep_mt, solve_proximity_stats, bilevel rasters, palette_bins_render, step_scores_to_palette_raw, solve_palette_chunk, solve_palette_debug | none | **immune** |
| libcurl staged (DT_RPATH `$ORIGIN/lib`) | roots2pix_mt, assemble_greyscale, solve_proximity_hist_sectioned, solve_palette_chunk_mt | own June-consistent closure; glibc family from runtime | **fixed this week** (import outage + libresolv DNS) |
| libvips layer (`/opt/lib`, rpath baked) | raw2jpeg, score_raw_render, bilevel_merge, raw_to_bilevel, tiff_compat, png_export, dz_export, autolevels_render | 20 deps from layer, 9 from runtime system | see findings |
| LAPACK layer (`/opt/lib`) | sweep_coeffgen, sweep_cm | lapack/openblas/gfortran/gcc_s from layer, rest runtime | clean |

Verified in the Lambda image: **zero unresolved dependencies in any mode**,
with and without env vars; both layers are internally consistent (each built
in one container run); neither layer ships glibc-family (libresolv/libnss)
or OpenSSL libraries.

## Findings

### [latent] The libarchive→libcrypto edge is the sharpest remaining coupling

The vips binaries resolve 9 libraries from the runtime system — mostly
frozen glibc — but among them, pulled in via libarchive: **libcrypto.so.3,
liblzma, libzstd, liblz4**. The layer deliberately does not ship OpenSSL
(which is why the runtime python survives `/opt/lib`), so vips binaries
lean on the *pinned runtime's* crypto. Today's March-built layer matches
it. A future `build-libvips-layer.sh` run compiles against current Amazon
Linux: if that libarchive wants newer OpenSSL symbols than the pinned
runtime provides, all eight vips binaries fail to load at once. Same
trigger as both incidents: **rebuild against current userland + pinned
runtime**.

### [latent, low] Python-critical shadows in `/opt/lib`

25 layer libraries shadow runtime-owned names. Four are dlopened by the
runtime python itself: **libz, libbz2, libffi** (vips layer) and
**libgcc_s** (LAPACK layer). Probed: every python module touching them
(ssl, zlib, bz2, lzma, ctypes, pyexpat, sqlite3, hashlib, botocore)
imports cleanly under both layers today — because these are AL2023
*frozen-version* packages, byte-version-identical between layer and
runtime (zlib 1.2.11, bz2 1.0.8, ffi 8.1.2). The risk activates only if
AL2023 ever bumps one and a layer is rebuilt across that bump.

### [ok] LAPACK layer is exemplary

Ships exactly its closure (lapack, blas, openblas, gfortran, gcc_s); both
binaries resolve fully even without `LD_LIBRARY_PATH`; nothing
python-critical beyond the stable libgcc_s.

### [fixed in the lead-up to this review] libcurl staged closures

DT_RPATH hermetic closure minus the glibc family; download errors now carry
curl rc/strerror/http *before* the URL so a 1.5 KB presigned URL cannot
truncate the diagnosis.

## Will you see more of these errors?

**Not from what is deployed now** — today's layer/binary/runtime trio is
mutually consistent, and the curl fix removed the one inconsistency.

**The failure family has exactly one trigger: rebuilding any native
artifact against current Amazon Linux while the Lambda runtime stays
pinned.** Ranked exposure at next rebuild:

1. `build-libvips-layer.sh` — the libarchive→system-crypto edge (eight
   functions at once). Highest.
2. `build-lapack-layer.sh` — libgcc_s shadow plus fortran ABI. Low.
3. `build_libcurl_binary` — already hermetic; lowest.
4. AWS bumping the pinned runtime itself — the reverse direction
   (runtime newer than artifacts), historically backward-compatible.

## Guards now in place

- Docker gate: DT_RPATH + staged-closure cleanliness for the four libcurl
  binaries; **new in this review**: layers banned from shipping
  glibc-family/OpenSSL libraries, and the python-critical shadow set
  (zlib/bz2/ffi/gcc_s versions) pinned — a layer rebuild that moves them
  fails the gate with instructions instead of shipping quietly.
- `deploy_manifest.py --check`: `LD_LIBRARY_PATH` entries may only
  reference `/opt/...`.
- Post-deploy INIT sweep catches any import-dead function seconds after
  deploy; checklist §8 now states the layer rule and the
  rebuild→gate→deploy→sweep sequence.

## Recommendation — IMPLEMENTED (same day)

libarchive cannot be dropped (vips 8.15+ implements dzsave on it), and the
chain must not enter `/opt/lib` (python's LD_LIBRARY_PATH — the outage
class). Implemented as the hermetic pattern instead:

- `build-libvips-layer.sh` vendors libcrypto/lzma/zstd/lz4 into
  **`/opt/vipsdeps`** — outside python's search path — and zips it into the
  layer.
- The vips gcc lines link with `--disable-new-dtags
  -rpath /opt/lib:/opt/vipsdeps`, so DT_RPATH resolves libarchive's
  transitive chain from the layer, never the runtime system.
- Layer rebuilt (June userland: crypto 3.2.2, lzma 5.2.5, zstd 1.5.5,
  lz4 1.9.4); all eight binaries rebuilt; in-container vips smoke tests
  passed. The frozen-shadow pins (zlib 1.2.11 / bz2 1.0.8 / ffi 8.1.2)
  survived the June rebuild exactly as predicted — AL2023 keeps them
  frozen, validating the guard design.
- Verified in the Lambda image: crypto chain resolves from
  `/opt/vipsdeps`, zero unresolved deps across all eight binaries, python
  imports clean under the new layer, no crypto in `/opt/lib`.
- Docker gate extended: `vipsdeps/` must carry the four libs (when present)
  while `lib/` stays banned from them.

The vips binaries now have **zero version coupling to the runtime beyond
frozen glibc** — the same guarantee the libcurl binaries got. Deploying
this publishes the new layer version and binaries together
(deploy.sh re-publishes the layer when its zip changes); follow with the
post-deploy INIT sweep and one render/DeepZoom smoke.
