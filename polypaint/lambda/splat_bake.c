/*
 * splat_bake: voxel-binned anisotropic gaussian splats from a sculpture
 * roots dump — the server-side twin of the viewer's splat worker. The
 * splats are a pure function of (roots dump x per-solve colors x a small
 * parameter set), all of which the server already holds, so baking a
 * shareable viewer must never require a browser upload.
 *
 * Mirrors sculpture.html EXACTLY:
 *   - u16 dump decode: 0..65534 spans each viewport axis, the pair
 *     (65535,65535) is the non-finite/out-of-view sentinel;
 *   - f32 dump decode: drop non-finite and outside-viewport roots;
 *   - normalize: X = (re-cx)/side, Z = -(im-cy)/side, side = max span;
 *   - pass 0 only, serpentine step order: col = (row&1) ? N-1-j : j,
 *     t2 = col/N, t1 = row/N;
 *   - Y = t - 0.5 after the slices binning (level/(S-1), 0.5 for S==1);
 *   - res^3 voxel grid over [-0.5,0.5]^3 (indices clamped), 13-moment
 *     accumulators per occupied cell;
 *   - cyclic-Jacobi 3x3 eigen; axes = top-2 eigenvectors x 2*sqrt(lambda),
 *     floored at 0.35/res; weight sqrt(count/max); colors = byte means;
 *   - the bake fold: center.y and axis y components scale by yscale,
 *     all axis components scale by scalemul;
 *   - the pack: centers u16x3 (per-axis bounds), axisA i16x3n, axisB
 *     i16x3n (shared amax), colors u8x3n, weights u8xn = 22 bytes/splat,
 *     quantized with round(x) = floor(x+0.5) to mirror JS Math.round.
 *
 * Output: the pack to --out, a JSON summary on stdout. Splats are written
 * sorted by voxel id — deterministic regardless of hash iteration.
 */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static double js_round(double v) { return floor(v + 0.5); }

typedef struct { int64_t key; int32_t slot; } SbPair;

static int splat_bake_cmp_pair(const void *pa, const void *pb) {
    const SbPair *a = pa;
    const SbPair *b = pb;
    if (a->key < b->key) return -1;
    if (a->key > b->key) return 1;
    return 0;
}

/* symmetric 3x3 eigen via cyclic Jacobi — top-2 eigenpairs (mirrors the
 * viewer worker's jacobi3, machine-precision verified there) */
static void jacobi3(double xx, double xy, double xz, double yy, double yz,
                    double zz, double *l1, double *l2, double v1[3], double v2[3]) {
    double a[9] = { xx, xy, xz, xy, yy, yz, xz, yz, zz };
    double v[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
    for (int sweep = 0; sweep < 10; sweep++) {
        if (fabs(a[1]) + fabs(a[2]) + fabs(a[5]) < 1e-15) break;
        for (int pi = 0; pi < 3; pi++) {
            int p = (pi == 2) ? 1 : 0;
            int q = (pi == 0) ? 1 : 2;
            double apq = a[p * 3 + q];
            if (fabs(apq) < 1e-18) continue;
            double th = 0.5 * atan2(2.0 * apq, a[q * 3 + q] - a[p * 3 + p]);
            double c = cos(th), sn = sin(th);
            for (int k = 0; k < 3; k++) {
                double akp = a[k * 3 + p], akq = a[k * 3 + q];
                a[k * 3 + p] = c * akp - sn * akq;
                a[k * 3 + q] = sn * akp + c * akq;
            }
            for (int k = 0; k < 3; k++) {
                double apk = a[p * 3 + k], aqk = a[q * 3 + k];
                a[p * 3 + k] = c * apk - sn * aqk;
                a[q * 3 + k] = sn * apk + c * aqk;
            }
            for (int k = 0; k < 3; k++) {
                double vkp = v[k * 3 + p], vkq = v[k * 3 + q];
                v[k * 3 + p] = c * vkp - sn * vkq;
                v[k * 3 + q] = sn * vkp + c * vkq;
            }
        }
    }
    double lam[3] = { a[0], a[4], a[8] };
    int ord[3] = { 0, 1, 2 };
    for (int i = 0; i < 2; i++) {
        for (int j = i + 1; j < 3; j++) {
            if (lam[ord[j]] > lam[ord[i]]) { int t = ord[i]; ord[i] = ord[j]; ord[j] = t; }
        }
    }
    *l1 = lam[ord[0]];
    *l2 = lam[ord[1]];
    for (int k = 0; k < 3; k++) {
        v1[k] = v[k * 3 + ord[0]];
        v2[k] = v[k * 3 + ord[1]];
    }
}

/* open-addressed voxel table: key = voxel id, value = accumulator slot */
typedef struct {
    int64_t *keys;
    int32_t *slots;
    size_t cap;      /* power of two */
    size_t used;
} VoxMap;

static void vox_init(VoxMap *m, size_t cap) {
    m->cap = cap;
    m->used = 0;
    m->keys = malloc(cap * sizeof(int64_t));
    m->slots = malloc(cap * sizeof(int32_t));
    if (!m->keys || !m->slots) { fprintf(stderr, "splat_bake: out of memory\n"); exit(1); }
    for (size_t i = 0; i < cap; i++) m->keys[i] = -1;
}

static int32_t vox_lookup(VoxMap *m, int64_t key, int32_t next_slot, int *inserted);

static void vox_grow(VoxMap *m) {
    VoxMap bigger;
    vox_init(&bigger, m->cap * 2);
    for (size_t i = 0; i < m->cap; i++) {
        if (m->keys[i] < 0) continue;
        int ins = 0;
        int32_t s = vox_lookup(&bigger, m->keys[i], m->slots[i], &ins);
        (void)s;
    }
    free(m->keys);
    free(m->slots);
    *m = bigger;
}

static int32_t vox_lookup(VoxMap *m, int64_t key, int32_t next_slot, int *inserted) {
    if (m->used * 10 >= m->cap * 7) vox_grow(m);
    size_t mask = m->cap - 1;
    size_t i = ((uint64_t)key * 0x9E3779B97F4A7C15ULL) & mask;
    for (;;) {
        if (m->keys[i] < 0) {
            m->keys[i] = key;
            m->slots[i] = next_slot;
            m->used++;
            *inserted = 1;
            return next_slot;
        }
        if (m->keys[i] == key) {
            *inserted = 0;
            return m->slots[i];
        }
        i = (i + 1) & mask;
    }
}

static const char *arg_str(int argc, char **argv, const char *name, const char *dflt) {
    size_t len = strlen(name);
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], name, len) == 0 && argv[i][len] == '=') return argv[i] + len + 1;
    }
    return dflt;
}

static double arg_num(int argc, char **argv, const char *name, double dflt, int *seen) {
    const char *raw = arg_str(argc, argv, name, NULL);
    if (!raw) { if (seen) *seen = 0; return dflt; }
    if (seen) *seen = 1;
    return atof(raw);
}

int main(int argc, char **argv) {
    const char *roots_path = arg_str(argc, argv, "--roots", NULL);
    const char *colors_path = arg_str(argc, argv, "--colors", NULL);
    const char *out_path = arg_str(argc, argv, "--out", NULL);
    const char *fmt = arg_str(argc, argv, "--roots_format", "u16");
    const char *zaxis = arg_str(argc, argv, "--zaxis", "t2");
    int seen_re0 = 0, seen_re1 = 0, seen_im0 = 0, seen_im1 = 0;
    double min_re = arg_num(argc, argv, "--min_re", 0, &seen_re0);
    double max_re = arg_num(argc, argv, "--max_re", 0, &seen_re1);
    double min_im = arg_num(argc, argv, "--min_im", 0, &seen_im0);
    double max_im = arg_num(argc, argv, "--max_im", 0, &seen_im1);
    long grid_n = (long)arg_num(argc, argv, "--grid_n", 0, NULL);
    long degree = (long)arg_num(argc, argv, "--degree", 0, NULL);
    long res = (long)arg_num(argc, argv, "--res", 96, NULL);
    long slices = (long)arg_num(argc, argv, "--slices", 0, NULL);
    double yscale = arg_num(argc, argv, "--yscale", 1.0, NULL);
    double scalemul = arg_num(argc, argv, "--scalemul", 1.0, NULL);

    if (!roots_path || !colors_path || !out_path || grid_n < 2 || degree < 1
        || !seen_re0 || !seen_re1 || !seen_im0 || !seen_im1
        || !(max_re > min_re) || !(max_im > min_im)
        || res < 8 || res > 256 || slices < 0 || slices > 64
        || !(yscale >= 0.0) || !(scalemul > 0.0)
        || (strcmp(fmt, "u16") != 0 && strcmp(fmt, "f32") != 0)
        || (strcmp(zaxis, "t1") != 0 && strcmp(zaxis, "t2") != 0)) {
        fprintf(stderr,
            "usage: splat_bake --roots=F --colors=F --out=F --roots_format=u16|f32 "
            "--grid_n=N --degree=D --min_re= --max_re= --min_im= --max_im= "
            "[--res=96] [--zaxis=t2] [--slices=0] [--yscale=1] [--scalemul=1]\n");
        return 2;
    }

    long steps = grid_n * grid_n;             /* pass 0 only */
    long n_roots = steps * degree;
    size_t rec = (strcmp(fmt, "u16") == 0) ? 2 * sizeof(uint16_t) : 2 * sizeof(float);
    FILE *rf = fopen(roots_path, "rb");
    if (!rf) { fprintf(stderr, "splat_bake: cannot open %s\n", roots_path); return 1; }
    void *roots = malloc((size_t)n_roots * rec);
    if (!roots) { fprintf(stderr, "splat_bake: out of memory\n"); return 1; }
    if (fread(roots, rec, (size_t)n_roots, rf) != (size_t)n_roots) {
        fprintf(stderr, "splat_bake: roots file shorter than pass 0 (%ld roots)\n", n_roots);
        return 1;
    }
    fclose(rf);

    FILE *cf = fopen(colors_path, "rb");
    if (!cf) { fprintf(stderr, "splat_bake: cannot open %s\n", colors_path); return 1; }
    uint8_t *cell_rgb = malloc((size_t)steps * 3);
    if (!cell_rgb) { fprintf(stderr, "splat_bake: out of memory\n"); return 1; }
    if (fread(cell_rgb, 3, (size_t)steps, cf) != (size_t)steps) {
        fprintf(stderr, "splat_bake: colors file shorter than grid^2 RGB cells\n");
        return 1;
    }
    fclose(cf);

    double cx = (min_re + max_re) / 2.0, cy = (min_im + max_im) / 2.0;
    double span_re = max_re - min_re, span_im = max_im - min_im;
    double side = span_re > span_im ? span_re : span_im;
    int use_t1 = (strcmp(zaxis, "t1") == 0);

    VoxMap map;
    vox_init(&map, 1 << 14);
    size_t acap = 1 << 12;
    double *acc = malloc(acap * 13 * sizeof(double));
    int64_t *slot_key = malloc(acap * sizeof(int64_t));
    if (!acc || !slot_key) { fprintf(stderr, "splat_bake: out of memory\n"); return 1; }
    int32_t used = 0;
    long pts_used = 0, pts_clipped = 0;

    for (long s = 0; s < steps; s++) {
        long row = s / grid_n;
        long j = s % grid_n;
        long col = (row & 1) ? (grid_n - 1 - j) : j;   /* serpentine */
        double t = use_t1 ? (double)row / grid_n : (double)col / grid_n;
        double y;
        if (slices > 0) {
            long level = (long)floor(t * slices);
            if (level > slices - 1) level = slices - 1;
            y = slices > 1 ? (double)level / (slices - 1) : 0.5;
        } else {
            y = t;
        }
        y -= 0.5;
        /* cell color: the palette raw is row-major (row, col) */
        const uint8_t *rgb = cell_rgb + ((size_t)row * grid_n + col) * 3;
        for (long r = 0; r < degree; r++) {
            double re, im;
            if (rec == 2 * sizeof(uint16_t)) {
                const uint16_t *q = (const uint16_t *)roots + (s * degree + r) * 2;
                if (q[0] == 0xFFFF && q[1] == 0xFFFF) { pts_clipped++; continue; }
                re = min_re + (double)q[0] / 65534.0 * span_re;
                im = min_im + (double)q[1] / 65534.0 * span_im;
            } else {
                const float *f = (const float *)roots + (s * degree + r) * 2;
                re = f[0];
                im = f[1];
                if (!isfinite(re) || !isfinite(im)
                    || re < min_re || re > max_re || im < min_im || im > max_im) {
                    pts_clipped++;
                    continue;
                }
            }
            double x = (re - cx) / side;
            double z = -(im - cy) / side;
            long vx = (long)floor((x + 0.5) * res);
            long vy = (long)floor((y + 0.5) * res);
            long vz = (long)floor((z + 0.5) * res);
            if (vx < 0) vx = 0; if (vx > res - 1) vx = res - 1;
            if (vy < 0) vy = 0; if (vy > res - 1) vy = res - 1;
            if (vz < 0) vz = 0; if (vz > res - 1) vz = res - 1;
            int64_t key = (vx * res + vy) * res + vz;
            int inserted = 0;
            int32_t slot = vox_lookup(&map, key, used, &inserted);
            if (inserted) {
                if ((size_t)used >= acap) {
                    acap *= 2;
                    acc = realloc(acc, acap * 13 * sizeof(double));
                    slot_key = realloc(slot_key, acap * sizeof(int64_t));
                    if (!acc || !slot_key) { fprintf(stderr, "splat_bake: out of memory\n"); return 1; }
                }
                memset(acc + (size_t)used * 13, 0, 13 * sizeof(double));
                slot_key[used] = key;
                used++;
            }
            double *a = acc + (size_t)slot * 13;
            a[0] += 1.0;
            a[1] += x; a[2] += y; a[3] += z;
            a[4] += x * x; a[5] += x * y; a[6] += x * z;
            a[7] += y * y; a[8] += y * z; a[9] += z * z;
            a[10] += rgb[0]; a[11] += rgb[1]; a[12] += rgb[2];
            pts_used++;
        }
    }
    free(roots);
    free(cell_rgb);
    if (used == 0) { fprintf(stderr, "splat_bake: no points landed in the cube\n"); return 1; }

    /* deterministic output order: sort slots by voxel id */
    SbPair *pairs = malloc((size_t)used * sizeof(SbPair));
    if (!pairs) { fprintf(stderr, "splat_bake: out of memory\n"); return 1; }
    for (int32_t i = 0; i < used; i++) { pairs[i].key = slot_key[i]; pairs[i].slot = i; }
    qsort(pairs, (size_t)used, sizeof(SbPair), splat_bake_cmp_pair);

    double minS = 0.35 / (double)res;
    double *centers = malloc((size_t)used * 3 * sizeof(double));
    double *axisA = malloc((size_t)used * 3 * sizeof(double));
    double *axisB = malloc((size_t)used * 3 * sizeof(double));
    double *colmean = malloc((size_t)used * 3 * sizeof(double));
    double *wcnt = malloc((size_t)used * sizeof(double));
    if (!centers || !axisA || !axisB || !colmean || !wcnt) {
        fprintf(stderr, "splat_bake: out of memory\n");
        return 1;
    }
    double wmax = 0;
    for (int32_t i = 0; i < used; i++) {
        const double *a = acc + (size_t)pairs[i].slot * 13;
        double n = a[0];
        double mx = a[1] / n, my = a[2] / n, mz = a[3] / n;
        double l1, l2, v1[3], v2[3];
        jacobi3(a[4] / n - mx * mx, a[5] / n - mx * my, a[6] / n - mx * mz,
                a[7] / n - my * my, a[8] / n - my * mz, a[9] / n - mz * mz,
                &l1, &l2, v1, v2);
        double la = 2.0 * sqrt(l1 > 0 ? l1 : 0);
        double lb = 2.0 * sqrt(l2 > 0 ? l2 : 0);
        if (la < minS) la = minS;
        if (lb < minS) lb = minS;
        /* the bake fold: yscale on y components, scalemul on all axes */
        centers[i * 3] = mx;
        centers[i * 3 + 1] = my * yscale;
        centers[i * 3 + 2] = mz;
        axisA[i * 3] = v1[0] * la * scalemul;
        axisA[i * 3 + 1] = v1[1] * la * scalemul * yscale;
        axisA[i * 3 + 2] = v1[2] * la * scalemul;
        axisB[i * 3] = v2[0] * lb * scalemul;
        axisB[i * 3 + 1] = v2[1] * lb * scalemul * yscale;
        axisB[i * 3 + 2] = v2[2] * lb * scalemul;
        colmean[i * 3] = a[10] / n;
        colmean[i * 3 + 1] = a[11] / n;
        colmean[i * 3 + 2] = a[12] / n;
        wcnt[i] = n;
        if (n > wmax) wmax = n;
    }

    double cmin[3] = { 1e300, 1e300, 1e300 }, cmax[3] = { -1e300, -1e300, -1e300 };
    double amax = 1e-6;
    for (int32_t i = 0; i < used; i++) {
        for (int k = 0; k < 3; k++) {
            double v = centers[i * 3 + k];
            if (v < cmin[k]) cmin[k] = v;
            if (v > cmax[k]) cmax[k] = v;
            double aa = fabs(axisA[i * 3 + k]);
            double ab = fabs(axisB[i * 3 + k]);
            if (aa > amax) amax = aa;
            if (ab > amax) amax = ab;
        }
    }

    FILE *of = fopen(out_path, "wb");
    if (!of) { fprintf(stderr, "splat_bake: cannot open %s for writing\n", out_path); return 1; }
    /* centers u16x3 per splat, then axisA i16x3n, axisB i16x3n, colors
     * u8x3n, weights u8xn — byte-identical to the viewer-side pack */
    for (int32_t i = 0; i < used; i++) {
        for (int k = 0; k < 3; k++) {
            double span = cmax[k] - cmin[k];
            double g = (span != 0.0) ? span : 1.0;
            double q = js_round((centers[i * 3 + k] - cmin[k]) / g * 65535.0);
            if (q < 0) q = 0; if (q > 65535) q = 65535;
            uint16_t u = (uint16_t)q;
            fwrite(&u, sizeof(u), 1, of);
        }
    }
    for (int32_t i = 0; i < used * 3; i++) {
        double q = js_round(axisA[i] / amax * 32767.0);
        if (q < -32767) q = -32767; if (q > 32767) q = 32767;
        int16_t u = (int16_t)q;
        fwrite(&u, sizeof(u), 1, of);
    }
    for (int32_t i = 0; i < used * 3; i++) {
        double q = js_round(axisB[i] / amax * 32767.0);
        if (q < -32767) q = -32767; if (q > 32767) q = 32767;
        int16_t u = (int16_t)q;
        fwrite(&u, sizeof(u), 1, of);
    }
    for (int32_t i = 0; i < used * 3; i++) {
        double q = js_round(colmean[i]);
        if (q < 0) q = 0; if (q > 255) q = 255;
        uint8_t u = (uint8_t)q;
        fwrite(&u, sizeof(u), 1, of);
    }
    for (int32_t i = 0; i < used; i++) {
        double q = js_round(sqrt(wcnt[i] / (wmax > 0 ? wmax : 1)) * 255.0);
        if (q < 0) q = 0; if (q > 255) q = 255;
        uint8_t u = (uint8_t)q;
        fwrite(&u, sizeof(u), 1, of);
    }
    fclose(of);

    printf("{\"count\":%d,\"points_used\":%ld,\"points_clipped\":%ld,"
           "\"cmin\":[%.17g,%.17g,%.17g],\"cmax\":[%.17g,%.17g,%.17g],"
           "\"amax\":%.17g}\n",
           used, pts_used, pts_clipped,
           cmin[0], cmin[1], cmin[2], cmax[0], cmax[1], cmax[2], amax);
    return 0;
}
