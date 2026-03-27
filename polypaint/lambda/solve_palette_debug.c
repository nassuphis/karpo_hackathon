/*
 * solve_palette_debug — generate a palette debug JPEG from lores root data.
 *
 * Reads lores.bin, computes one solve-score per sample, derives lores-only
 * clip bounds and equal-density cuts, colors each sample, deserpentines,
 * expands to NxN via nearest-neighbor, writes raw RGB (raw2jpeg format).
 *
 * Usage:
 *   solve_palette_debug input.bin output.raw \
 *     --degree=D --lores_n=L --full_n=N --times=T \
 *     --metric=proximity --palette=inferno \
 *     --quantile_lo=Q --quantile_hi=1-Q \
 *     [--root_xforms=file.json]
 *
 * Output: JSON metadata to stdout. Raw RGB (12-byte header + pixels) to output.raw.
 *
 * Build: aarch64-linux-musl-gcc -O3 -static -o solve_palette_debug solve_palette_debug.c -lm
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "root_xforms.h"
#include "solve_score.h"
#include "palette_lut.h"

#define MAXDEG 1024

/* ---- Argument parsing ---- */

static const char *getArg(int argc, char **argv, const char *key) {
    size_t klen = strlen(key);
    for (int i = 1; i < argc; i++)
        if (strncmp(argv[i], key, klen) == 0 && argv[i][klen] == '=')
            return argv[i] + klen + 1;
    return NULL;
}
static int getArgInt(int argc, char **argv, const char *key, int def) {
    const char *v = getArg(argc, argv, key);
    return v ? atoi(v) : def;
}
static double getArgDouble(int argc, char **argv, const char *key, double def) {
    const char *v = getArg(argc, argv, key);
    return v ? atof(v) : def;
}
static const char *getArgStr(int argc, char **argv, const char *key, const char *def) {
    const char *v = getArg(argc, argv, key);
    return v ? v : def;
}

/* ---- qsort comparator ---- */
static int cmp_double(const void *a, const void *b) {
    double da = *(const double *)a, db = *(const double *)b;
    return (da > db) - (da < db);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: solve_palette_debug input.bin output.raw --degree=D "
                "--lores_n=L --full_n=N --times=T --metric=... --palette=... "
                "--quantile_lo=Q --quantile_hi=1-Q [--root_xforms=file.json]\n");
        return 1;
    }

    const char *inPath = argv[1];
    const char *outPath = argv[2];
    int degree = getArgInt(argc, argv, "--degree", 0);
    int loresN = getArgInt(argc, argv, "--lores_n", 0);
    int fullN = getArgInt(argc, argv, "--full_n", 0);
    int times = getArgInt(argc, argv, "--times", 1);
    double quantileLo = getArgDouble(argc, argv, "--quantile_lo", 0.001);
    double quantileHi = getArgDouble(argc, argv, "--quantile_hi", 0.999);
    const char *metricStr = getArgStr(argc, argv, "--metric", "proximity");
    const char *palName = getArgStr(argc, argv, "--palette", "inferno");

    if (degree < 2 || degree > MAXDEG) { fprintf(stderr, "Invalid degree: %d\n", degree); return 1; }
    if (loresN < 1) { fprintf(stderr, "Invalid lores_n: %d\n", loresN); return 1; }
    if (fullN < 1) { fprintf(stderr, "Invalid full_n: %d\n", fullN); return 1; }
    if (times < 1) { fprintf(stderr, "Invalid times: %d\n", times); return 1; }

    enum SolveMetric metric;
    if (!parse_solve_metric(metricStr, &metric)) {
        fprintf(stderr, "Invalid metric: %s\n", metricStr);
        return 1;
    }
    const RGB *pal = findPalette(palName);
    /* Validate palette name — reject unknown instead of silent fallback */
    {
        int found = 0;
        for (int i = 0; PALETTES[i].name; i++)
            if (strcmp(PALETTES[i].name, palName) == 0) { found = 1; break; }
        if (!found) {
            fprintf(stderr, "Invalid palette: %s\n", palName);
            return 1;
        }
    }

    /* Read input file */
    FILE *f = fopen(inPath, "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", inPath); return 1; }
    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    int stride = degree * 2;
    long totalSolves = fileSize / (stride * (long)sizeof(float));
    long expectedSolves = (long)times * loresN * loresN;
    if (totalSolves != expectedSolves) {
        fprintf(stderr, "Solve count mismatch: file has %ld, expected %ld (times=%d, loresN=%d)\n",
                totalSolves, expectedSolves, times, loresN);
        fclose(f);
        return 1;
    }

    float *buf = malloc(fileSize);
    if (!buf) { fprintf(stderr, "Out of memory\n"); fclose(f); return 1; }
    if ((long)fread(buf, 1, fileSize, f) != fileSize) {
        fprintf(stderr, "Short read\n"); free(buf); fclose(f); return 1;
    }
    fclose(f);

    /* Parse root transforms */
    const char *rtPath = getArgStr(argc, argv, "--root_xforms", NULL);
    RootXformEntry rtChain[MAX_RT_CHAIN];
    int nRt = 0;
    if (rtPath) {
        nRt = parse_root_xform_file(rtPath, rtChain, MAX_RT_CHAIN);
        if (nRt == 0) {
            fprintf(stderr, "Failed to parse root transforms from %s\n", rtPath);
            free(buf); return 1;
        }
    }

    float wkRe[MAXDEG], wkIm[MAXDEG];
    long perPass = (long)loresN * loresN;

    /* ---- Compute scores for pass 0 only ---- */
    double *scores = malloc(perPass * sizeof(double));
    if (!scores) { fprintf(stderr, "Out of memory for scores\n"); free(buf); return 1; }

    for (long s = 0; s < perPass; s++) {
        const float *roots = buf + s * stride;
        if (nRt > 0) {
            for (int k = 0; k < degree; k++) { wkRe[k] = roots[k*2]; wkIm[k] = roots[k*2+1]; }
            apply_root_xforms(rtChain, nRt, wkRe, wkIm, degree);
            float xf[MAXDEG * 2];
            for (int k = 0; k < degree; k++) { xf[k*2] = wkRe[k]; xf[k*2+1] = wkIm[k]; }
            scores[s] = compute_solve_metric_score(xf, degree, metric);
        } else {
            scores[s] = compute_solve_metric_score(roots, degree, metric);
        }
    }
    free(buf);

    /* ---- Clip bounds (same logic as solve_proximity_stats --mode=clip) ---- */
    double *sorted = malloc(perPass * sizeof(double));
    memcpy(sorted, scores, perPass * sizeof(double));
    qsort(sorted, perPass, sizeof(double), cmp_double);

    double clipLo, clipHi;
    const char *fallbackReason = NULL;
    int clipFallback = 0;

    if (perPass < 100) {
        clipLo = sorted[0]; clipHi = sorted[perPass - 1];
        clipFallback = 1; fallbackReason = "small_sample";
    } else {
        long loIdx = (long)((perPass - 1) * quantileLo);
        long hiIdx = (long)((perPass - 1) * quantileHi);
        if (hiIdx <= loIdx) {
            clipLo = sorted[0]; clipHi = sorted[perPass - 1];
            clipFallback = 1; fallbackReason = "degenerate_quantile_range";
        } else {
            clipLo = sorted[loIdx]; clipHi = sorted[hiIdx];
            if (clipHi - clipLo < 1e-12) {
                clipLo = sorted[0]; clipHi = sorted[perPass - 1];
                clipFallback = 1; fallbackReason = "degenerate_quantile_range";
            }
        }
    }
    if (clipHi - clipLo < 1e-12) {
        clipLo = sorted[0] - 0.5; clipHi = sorted[0] + 0.5;
        clipFallback = 1; fallbackReason = "zero_full_range_expanded";
    }

    /* ---- Equal-density cuts from lores (100-bin histogram → 9 cuts) ---- */
    double clipRange = clipHi - clipLo;
    int histBins = 100;
    long *hist = calloc(histBins, sizeof(long));
    for (long s = 0; s < perPass; s++) {
        double u = (scores[s] - clipLo) / clipRange;
        if (u < 0) u = 0; if (u > 1) u = 1;
        int h = (int)(u * histBins);
        if (h >= histBins) h = histBins - 1;
        hist[h]++;
    }

    int finalBins = 10;
    double cutsNorm[9];
    long totalCount = perPass;
    for (int k = 0; k < 9; k++) {
        long target = totalCount * (k + 1) / finalBins;
        long cum = 0;
        double cut = 1.0;
        for (int i = 0; i < histBins; i++) {
            long cumBefore = cum;
            cum += hist[i];
            if (cum >= target) {
                double frac = hist[i] > 0 ? (double)(target - cumBefore) / hist[i] : 1.0;
                cut = (i + frac) / histBins;
                break;
            }
        }
        if (cut < 0) cut = 0; if (cut > 1) cut = 1;
        if (k > 0 && cut < cutsNorm[k - 1]) cut = cutsNorm[k - 1];
        cutsNorm[k] = cut;
    }
    free(hist);

    /* ---- Build loresN x loresN RGB grid with serpentine deshuffle ---- */
    unsigned char *srcRGB = calloc(perPass * 3, 1);

    for (long s = 0; s < perPass; s++) {
        /* Bin assignment */
        double u = (scores[s] - clipLo) / clipRange;
        if (u < 0) u = 0; if (u > 1) u = 1;
        int bin = 0;
        for (int k = 0; k < 9; k++) {
            if (u > cutsNorm[k]) bin = k + 1;
        }
        double t = (bin + 0.5) / (double)finalBins;
        unsigned char r, g, b;
        paletteRGB(pal, t, &r, &g, &b);

        /* Serpentine deshuffle: s = i1 * loresN + j */
        int i1 = (int)(s / loresN);
        int j = (int)(s % loresN);
        int i2 = (i1 % 2 == 0) ? j : (loresN - 1 - j);

        long dstIdx = ((long)i1 * loresN + i2) * 3;
        srcRGB[dstIdx] = r;
        srcRGB[dstIdx + 1] = g;
        srcRGB[dstIdx + 2] = b;
    }
    free(scores);
    free(sorted);

    /* ---- Nearest-neighbor expansion to fullN x fullN ---- */
    long outPixels = (long)fullN * fullN;
    unsigned char *dst = malloc(outPixels * 3);
    if (!dst) { fprintf(stderr, "Out of memory for output image\n"); free(srcRGB); return 1; }

    for (int y = 0; y < fullN; y++) {
        int srcI = y * loresN / fullN;
        if (srcI >= loresN) srcI = loresN - 1;
        for (int x = 0; x < fullN; x++) {
            int srcJ = x * loresN / fullN;
            if (srcJ >= loresN) srcJ = loresN - 1;
            long si = ((long)srcI * loresN + srcJ) * 3;
            long di = ((long)y * fullN + x) * 3;
            dst[di] = srcRGB[si];
            dst[di + 1] = srcRGB[si + 1];
            dst[di + 2] = srcRGB[si + 2];
        }
    }
    free(srcRGB);

    /* ---- Write raw RGB with 12-byte header (raw2jpeg format) ---- */
    FILE *out = fopen(outPath, "wb");
    if (!out) { fprintf(stderr, "Cannot open output %s\n", outPath); free(dst); return 1; }
    uint32_t hdr[3] = { (uint32_t)fullN, (uint32_t)fullN, 3 };
    fwrite(hdr, sizeof(uint32_t), 3, out);
    fwrite(dst, 1, outPixels * 3, out);
    fclose(out);
    free(dst);

    /* ---- Emit JSON metadata ---- */
    printf("{\"mode\":\"palette_debug\",\"metric\":\"%s\",\"palette\":\"%s\","
           "\"n_samples_used\":%ld,\"degree\":%d,\"lores_n\":%d,\"full_n\":%d,"
           "\"times\":%d,\"using_pass\":0,",
           solve_metric_name(metric), palName, perPass, degree, loresN, fullN, times);
    printf("\"clip_lo\":%.15g,\"clip_hi\":%.15g,\"clip_range\":%.15g,",
           clipLo, clipHi, clipRange);
    printf("\"clip_fallback\":%s,", clipFallback ? "true" : "false");
    if (fallbackReason)
        printf("\"clip_fallback_reason\":\"%s\",", fallbackReason);
    else
        printf("\"clip_fallback_reason\":null,");
    printf("\"cuts_norm\":[");
    for (int k = 0; k < 9; k++) {
        if (k > 0) printf(",");
        printf("%.6f", cutsNorm[k]);
    }
    printf("]}\n");

    return 0;
}
