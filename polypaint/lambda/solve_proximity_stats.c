/*
 * solve_proximity_stats — compute solve-level proximity metrics from root .bin files.
 *
 * Two modes:
 *   --mode=clip   Compute score array, sort, emit clip bounds (quantiles).
 *   --mode=hist   Compute per-solve scores, emit 100-bin histogram using given clip bounds.
 *
 * Score: -0.5 * log10(max(d2_min, EPS2)) where d2_min = min_{i<j} |r_i - r_j|^2
 *
 * Usage:
 *   solve_proximity_stats input.bin --mode=clip --degree=D [--quantile_lo=0.001] [--quantile_hi=0.999] [--root_xforms=file.json]
 *   solve_proximity_stats input.bin --mode=hist --degree=D --clip_lo=X --clip_hi=Y --hist_bins=100 [--root_xforms=file.json]
 *
 * Output: JSON to stdout.
 *
 * Build: aarch64-linux-musl-gcc -O3 -static -o solve_proximity_stats solve_proximity_stats.c -lm
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "root_xforms.h"

#define EPS2 1e-300
#define MAXDEG 1024

/* ---- Argument parsing (same style as roots2pix.c) ---- */

static const char *getArg(int argc, char **argv, const char *key) {
    size_t klen = strlen(key);
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], key, klen) == 0 && argv[i][klen] == '=')
            return argv[i] + klen + 1;
    }
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

/* ---- Score computation ---- */

static double solve_score(const float *roots, int degree) {
    /* Compute d2_min = min_{i<j} ((re_i-re_j)^2 + (im_i-im_j)^2) */
    double d2_min = 1e300;
    for (int i = 0; i < degree; i++) {
        double ri_re = roots[i * 2];
        double ri_im = roots[i * 2 + 1];
        for (int j = i + 1; j < degree; j++) {
            double dr = ri_re - roots[j * 2];
            double di = ri_im - roots[j * 2 + 1];
            double d2 = dr * dr + di * di;
            if (d2 < d2_min) d2_min = d2;
        }
    }
    return -0.5 * log10(d2_min > EPS2 ? d2_min : EPS2);
}

/* ---- Score with root transforms applied in working buffer ---- */

static double solve_score_xformed(const float *roots, int degree,
                                   RootXformEntry *rtChain, int nRt,
                                   float *wkRe, float *wkIm) {
    for (int k = 0; k < degree; k++) {
        wkRe[k] = roots[k * 2];
        wkIm[k] = roots[k * 2 + 1];
    }
    if (nRt > 0) apply_root_xforms(rtChain, nRt, wkRe, wkIm, degree);
    /* Compute d2_min on transformed roots */
    double d2_min = 1e300;
    for (int i = 0; i < degree; i++) {
        for (int j = i + 1; j < degree; j++) {
            double dr = (double)wkRe[i] - (double)wkRe[j];
            double di = (double)wkIm[i] - (double)wkIm[j];
            double d2 = dr * dr + di * di;
            if (d2 < d2_min) d2_min = d2;
        }
    }
    return -0.5 * log10(d2_min > EPS2 ? d2_min : EPS2);
}

/* ---- Comparison for qsort ---- */

static int cmp_double(const void *a, const void *b) {
    double da = *(const double *)a, db = *(const double *)b;
    return (da > db) - (da < db);
}

/* ---- Main ---- */

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: solve_proximity_stats input.bin --mode=clip|hist --degree=D [options]\n");
        return 1;
    }

    const char *inPath = argv[1];
    const char *mode = getArgStr(argc, argv, "--mode", "");
    int degree = getArgInt(argc, argv, "--degree", 0);

    if (degree < 2 || degree > MAXDEG) {
        fprintf(stderr, "Invalid degree: %d (must be 2-%d)\n", degree, MAXDEG);
        return 1;
    }

    /* Read input file */
    FILE *f = fopen(inPath, "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", inPath); return 1; }
    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    int stride = degree * 2;  /* float pairs per solve */
    long nSolves = fileSize / (stride * (long)sizeof(float));
    if (nSolves <= 0) {
        fprintf(stderr, "Empty or invalid file: %ld bytes, degree=%d\n", fileSize, degree);
        fclose(f);
        return 1;
    }

    float *buf = malloc(fileSize);
    if (!buf) { fprintf(stderr, "Out of memory: %ld bytes\n", fileSize); fclose(f); return 1; }
    if ((long)fread(buf, 1, fileSize, f) != fileSize) {
        fprintf(stderr, "Short read\n");
        free(buf); fclose(f);
        return 1;
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
            free(buf);
            return 1;
        }
    }

    /* Working buffers for transformed roots (float to match root_xforms.h) */
    float wkRe[MAXDEG], wkIm[MAXDEG];

    if (strcmp(mode, "clip") == 0) {
        /* ---- CLIP MODE ---- */
        double quantileLo = getArgDouble(argc, argv, "--quantile_lo", 0.001);
        double quantileHi = getArgDouble(argc, argv, "--quantile_hi", 0.999);

        /* Compute all scores */
        double *scores = malloc(nSolves * sizeof(double));
        if (!scores) { fprintf(stderr, "Out of memory for scores\n"); free(buf); return 1; }

        for (long s = 0; s < nSolves; s++) {
            const float *roots = buf + s * stride;
            scores[s] = (nRt > 0)
                ? solve_score_xformed(roots, degree, rtChain, nRt, wkRe, wkIm)
                : solve_score(roots, degree);
        }

        /* Sort */
        qsort(scores, nSolves, sizeof(double), cmp_double);

        /* Clip bounds with fallback rules */
        double clipLo, clipHi;
        if (nSolves < 100) {
            clipLo = scores[0];
            clipHi = scores[nSolves - 1];
        } else {
            long loIdx = (long)((nSolves - 1) * quantileLo);
            long hiIdx = (long)((nSolves - 1) * quantileHi);
            if (hiIdx <= loIdx) {
                clipLo = scores[0];
                clipHi = scores[nSolves - 1];
            } else {
                clipLo = scores[loIdx];
                clipHi = scores[hiIdx];
                if (clipHi - clipLo < 1e-12) {
                    clipLo = scores[0];
                    clipHi = scores[nSolves - 1];
                }
            }
        }
        if (clipHi - clipLo < 1e-12) {
            clipLo = scores[0] - 0.5;
            clipHi = scores[0] + 0.5;
        }

        printf("{\"mode\":\"clip\",\"n_solves\":%ld,\"degree\":%d,"
               "\"score\":\"-0.5*log10(d2_min)\","
               "\"clip_lo\":%.15g,\"clip_hi\":%.15g,"
               "\"min_score\":%.15g,\"max_score\":%.15g}\n",
               nSolves, degree, clipLo, clipHi,
               scores[0], scores[nSolves - 1]);

        free(scores);

    } else if (strcmp(mode, "hist") == 0) {
        /* ---- HIST MODE ---- */
        double clipLo = getArgDouble(argc, argv, "--clip_lo", 0);
        double clipHi = getArgDouble(argc, argv, "--clip_hi", 0);
        int histBins = getArgInt(argc, argv, "--hist_bins", 100);

        if (clipHi - clipLo < 1e-12) {
            fprintf(stderr, "Invalid clip range: lo=%.15g hi=%.15g\n", clipLo, clipHi);
            free(buf);
            return 1;
        }
        if (histBins < 1 || histBins > 10000) {
            fprintf(stderr, "Invalid hist_bins: %d\n", histBins);
            free(buf);
            return 1;
        }

        long *hist = calloc(histBins, sizeof(long));
        if (!hist) { fprintf(stderr, "Out of memory for histogram\n"); free(buf); return 1; }

        double range = clipHi - clipLo;
        for (long s = 0; s < nSolves; s++) {
            const float *roots = buf + s * stride;
            double score = (nRt > 0)
                ? solve_score_xformed(roots, degree, rtChain, nRt, wkRe, wkIm)
                : solve_score(roots, degree);
            double u = (score - clipLo) / range;
            if (u < 0) u = 0;
            if (u > 1) u = 1;
            int h = (int)(u * histBins);
            if (h >= histBins) h = histBins - 1;
            hist[h]++;
        }

        /* Print JSON */
        printf("{\"mode\":\"hist\",\"n_solves\":%ld,\"degree\":%d,"
               "\"hist_bins\":%d,\"clip_lo\":%.15g,\"clip_hi\":%.15g,"
               "\"hist\":[", nSolves, degree, histBins, clipLo, clipHi);
        for (int i = 0; i < histBins; i++) {
            if (i > 0) printf(",");
            printf("%ld", hist[i]);
        }
        printf("]}\n");

        free(hist);

    } else {
        fprintf(stderr, "Unknown mode: %s (use clip or hist)\n", mode);
        free(buf);
        return 1;
    }

    free(buf);
    return 0;
}
