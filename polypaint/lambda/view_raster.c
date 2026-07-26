/*
 * view_raster: an ASSOCIATED-ARTIFACT view of an existing color render —
 * the architecture drawings of the root cloud. The plan (the artwork
 * itself) plots roots at (Re, Im); a view re-plots the SAME data:
 *
 *   front  : Re rightward,          t upward
 *   rear   : Re mirrored,           t upward
 *   left   : Im mirrored,           t upward
 *   right  : Im rightward,          t upward
 *   radial : r = |root| rightward,  t upward   (all angles collapse —
 *            a palette is (t1,t2); a view is (r, t))
 *
 * Inputs are the artifact's OWN stored derivatives — the post-transform
 * roots dump (u16 sentinel-encoded or f32) and the per-solve score bytes
 * subsampled from stored step_scores — so nothing is re-rendered and no
 * score is re-evaluated. Output is a raw u8 image (pix^2 x channels):
 * per pixel, the score bytes of the FIRST solve root to claim it (the
 * render pipeline's dedup policy), background 0; the caller equalizes and
 * palettes it exactly like the plan raw.
 *
 * t vertical: t = 1 at the top; t = 0 lands ON the bottom row.
 * radial horizontal: r in [0, Rmax], Rmax = the farthest viewport corner
 * from the origin (everything visible in the plan fits in the view).
 */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    const char *scores_path = arg_str(argc, argv, "--scores", NULL);
    const char *out_path = arg_str(argc, argv, "--out", NULL);
    const char *fmt = arg_str(argc, argv, "--roots_format", "u16");
    const char *projection = arg_str(argc, argv, "--projection", NULL);
    const char *vertical = arg_str(argc, argv, "--vertical", "t2");
    int seen_re0 = 0, seen_re1 = 0, seen_im0 = 0, seen_im1 = 0;
    double min_re = arg_num(argc, argv, "--min_re", 0, &seen_re0);
    double max_re = arg_num(argc, argv, "--max_re", 0, &seen_re1);
    double min_im = arg_num(argc, argv, "--min_im", 0, &seen_im0);
    double max_im = arg_num(argc, argv, "--max_im", 0, &seen_im1);
    long grid_n = (long)arg_num(argc, argv, "--grid_n", 0, NULL);
    long degree = (long)arg_num(argc, argv, "--degree", 0, NULL);
    long pix = (long)arg_num(argc, argv, "--pix", 0, NULL);
    long channels = (long)arg_num(argc, argv, "--channels", 1, NULL);

    int proj = -1;
    if (projection) {
        if (strcmp(projection, "front") == 0) proj = 0;
        else if (strcmp(projection, "rear") == 0) proj = 1;
        else if (strcmp(projection, "left") == 0) proj = 2;
        else if (strcmp(projection, "right") == 0) proj = 3;
        else if (strcmp(projection, "radial") == 0) proj = 4;
    }
    int vert_t1 = (vertical && strcmp(vertical, "t1") == 0) ? 1
                : (vertical && strcmp(vertical, "t2") == 0) ? 0 : -1;

    if (!roots_path || !scores_path || !out_path || proj < 0 || vert_t1 < 0
        || grid_n < 2 || degree < 1 || pix < 8 || pix > 8192
        || (channels != 1 && channels != 3)
        || !seen_re0 || !seen_re1 || !seen_im0 || !seen_im1
        || !(max_re > min_re) || !(max_im > min_im)
        || (strcmp(fmt, "u16") != 0 && strcmp(fmt, "f32") != 0)) {
        fprintf(stderr,
            "usage: view_raster --roots=F --scores=F --out=F --roots_format=u16|f32 "
            "--projection=front|rear|left|right|radial --vertical=t2|t1 "
            "--grid_n=N --degree=D --pix=P [--channels=1|3] "
            "--min_re= --max_re= --min_im= --max_im=\n");
        return 2;
    }

    long steps = grid_n * grid_n;             /* pass 0 only */
    long n_roots = steps * degree;
    size_t rec = (strcmp(fmt, "u16") == 0) ? 2 * sizeof(uint16_t) : 2 * sizeof(float);
    FILE *rf = fopen(roots_path, "rb");
    if (!rf) { fprintf(stderr, "view_raster: cannot open %s\n", roots_path); return 1; }
    void *roots = malloc((size_t)n_roots * rec);
    if (!roots) { fprintf(stderr, "view_raster: out of memory\n"); return 1; }
    if (fread(roots, rec, (size_t)n_roots, rf) != (size_t)n_roots) {
        fprintf(stderr, "view_raster: roots file shorter than pass 0 (%ld roots)\n", n_roots);
        return 1;
    }
    fclose(rf);

    /* per-solve score bytes, row-major (row, col) — the palette-raw layout */
    FILE *sf = fopen(scores_path, "rb");
    if (!sf) { fprintf(stderr, "view_raster: cannot open %s\n", scores_path); return 1; }
    uint8_t *scores = malloc((size_t)steps * (size_t)channels);
    if (!scores) { fprintf(stderr, "view_raster: out of memory\n"); return 1; }
    if (fread(scores, (size_t)channels, (size_t)steps, sf) != (size_t)steps) {
        fprintf(stderr, "view_raster: scores file shorter than grid^2 cells\n");
        return 1;
    }
    fclose(sf);

    double span_re = max_re - min_re, span_im = max_im - min_im;
    double xScale = (double)pix / span_re;
    double imScale = (double)pix / span_im;
    /* radial range: the farthest viewport corner from the ORIGIN */
    double rmax = 0;
    for (int cx = 0; cx < 2; cx++) {
        for (int cy = 0; cy < 2; cy++) {
            double cre = cx ? max_re : min_re;
            double cim = cy ? max_im : min_im;
            double d = hypot(cre, cim);
            if (d > rmax) rmax = d;
        }
    }
    double rScale = (double)pix / (rmax > 0 ? rmax : 1.0);

    uint8_t *img = calloc((size_t)pix * (size_t)pix, (size_t)channels);
    uint8_t *claimed = calloc((size_t)pix * (size_t)pix, 1);
    if (!img || !claimed) { fprintf(stderr, "view_raster: out of memory\n"); return 1; }
    long plotted = 0, clipped = 0, deduped = 0;

    for (long s = 0; s < steps; s++) {
        long row = s / grid_n;
        long j = s % grid_n;
        long col = (row & 1) ? (grid_n - 1 - j) : j;   /* serpentine */
        double t = vert_t1 ? (double)row / grid_n : (double)col / grid_n;
        double pyf = (1.0 - t) * (double)pix;
        int py = (int)floor(pyf);
        if (py == pix && t <= 0.0) py = (int)pix - 1;   /* t=0 -> bottom row */
        if (py < 0 || py >= pix) { clipped += degree; continue; }
        const uint8_t *sc = scores + ((size_t)row * grid_n + col) * (size_t)channels;
        for (long r = 0; r < degree; r++) {
            double re, im;
            if (rec == 2 * sizeof(uint16_t)) {
                const uint16_t *q = (const uint16_t *)roots + (s * degree + r) * 2;
                if (q[0] == 0xFFFF && q[1] == 0xFFFF) { clipped++; continue; }
                re = min_re + (double)q[0] / 65534.0 * span_re;
                im = min_im + (double)q[1] / 65534.0 * span_im;
            } else {
                const float *f = (const float *)roots + (s * degree + r) * 2;
                re = f[0];
                im = f[1];
                if (!isfinite(re) || !isfinite(im)
                    || re < min_re || re > max_re || im < min_im || im > max_im) {
                    clipped++;
                    continue;
                }
            }
            double pxf;
            switch (proj) {
            case 0: pxf = (re - min_re) * xScale; break;          /* front */
            case 1: pxf = (max_re - re) * xScale; break;          /* rear */
            case 2: pxf = (max_im - im) * imScale; break;         /* left */
            case 3: pxf = (im - min_im) * imScale; break;         /* right */
            default: pxf = hypot(re, im) * rScale; break;         /* radial */
            }
            if (!isfinite(pxf)) { clipped++; continue; }
            int px = (int)floor(pxf);
            if (px < 0 || px >= pix) { clipped++; continue; }
            size_t idx = (size_t)py * (size_t)pix + (size_t)px;
            if (claimed[idx]) { deduped++; continue; }
            claimed[idx] = 1;
            memcpy(img + idx * (size_t)channels, sc, (size_t)channels);
            plotted++;
        }
    }

    FILE *of = fopen(out_path, "wb");
    if (!of) { fprintf(stderr, "view_raster: cannot open %s for writing\n", out_path); return 1; }
    fwrite(img, (size_t)channels, (size_t)pix * (size_t)pix, of);
    fclose(of);

    printf("{\"pix\":%ld,\"channels\":%ld,\"plotted\":%ld,\"clipped\":%ld,"
           "\"deduped\":%ld,\"rmax\":%.17g}\n",
           pix, channels, plotted, clipped, deduped, rmax);
    return 0;
}
