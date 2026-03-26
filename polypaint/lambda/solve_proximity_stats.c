/*
 * solve_proximity_stats — compute solve-level metric scores from root .bin files.
 *
 * Supports multiple metrics via --metric (default: proximity).
 * Metrics: proximity, crowding, spread, anisotropy, area.
 *
 * Two modes:
 *   --mode=clip   Compute score array, sort, emit clip bounds (quantiles).
 *   --mode=hist   Compute per-solve scores, emit 100-bin histogram using given clip bounds.
 *
 * Usage:
 *   solve_proximity_stats input.bin --mode=clip --degree=D [--metric=proximity] [--quantile_lo=0.001] [--quantile_hi=0.999] [--root_xforms=file.json]
 *   solve_proximity_stats input.bin --mode=hist --degree=D [--metric=proximity] --clip_lo=X --clip_hi=Y --hist_bins=100 [--root_xforms=file.json]
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
#include "solve_score.h"

#define MAXDEG 1024

/* ---- Argument parsing ---- */

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

/* ---- Score with root transforms applied in working buffer ---- */

static double score_xformed(const float *roots, int degree, enum SolveMetric metric,
                            RootXformEntry *rtChain, int nRt,
                            float *wkRe, float *wkIm) {
    for (int k = 0; k < degree; k++) {
        wkRe[k] = roots[k * 2];
        wkIm[k] = roots[k * 2 + 1];
    }
    apply_root_xforms(rtChain, nRt, wkRe, wkIm, degree);
    /* Pack back into interleaved format for the shared helper */
    float xformed[MAXDEG * 2];
    for (int k = 0; k < degree; k++) {
        xformed[k * 2] = wkRe[k];
        xformed[k * 2 + 1] = wkIm[k];
    }
    return compute_solve_metric_score(xformed, degree, metric);
}

/* ---- Comparison for qsort ---- */

static int cmp_double(const void *a, const void *b) {
    double da = *(const double *)a, db = *(const double *)b;
    return (da > db) - (da < db);
}

/* ---- Main ---- */

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: solve_proximity_stats input.bin --mode=clip|hist --degree=D "
                "[--metric=proximity|crowding|spread|anisotropy|area] [options]\n");
        return 1;
    }

    const char *inPath = argv[1];
    const char *mode = getArgStr(argc, argv, "--mode", "");
    int degree = getArgInt(argc, argv, "--degree", 0);
    const char *metricStr = getArgStr(argc, argv, "--metric", "proximity");

    if (degree < 2 || degree > MAXDEG) {
        fprintf(stderr, "Invalid degree: %d (must be 2-%d)\n", degree, MAXDEG);
        return 1;
    }

    enum SolveMetric metric;
    if (!parse_solve_metric(metricStr, &metric)) {
        fprintf(stderr, "Invalid metric: %s (use proximity|crowding|spread|anisotropy|area)\n", metricStr);
        return 1;
    }

    /* Read input file */
    FILE *f = fopen(inPath, "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", inPath); return 1; }
    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    int stride = degree * 2;
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

    float wkRe[MAXDEG], wkIm[MAXDEG];
    const char *metricName = solve_metric_name(metric);

    if (strcmp(mode, "clip") == 0) {
        /* ---- CLIP MODE ---- */
        double quantileLo = getArgDouble(argc, argv, "--quantile_lo", 0.001);
        double quantileHi = getArgDouble(argc, argv, "--quantile_hi", 0.999);

        double *scores = malloc(nSolves * sizeof(double));
        if (!scores) { fprintf(stderr, "Out of memory for scores\n"); free(buf); return 1; }

        for (long s = 0; s < nSolves; s++) {
            const float *roots = buf + s * stride;
            scores[s] = (nRt > 0)
                ? score_xformed(roots, degree, metric, rtChain, nRt, wkRe, wkIm)
                : compute_solve_metric_score(roots, degree, metric);
        }

        qsort(scores, nSolves, sizeof(double), cmp_double);

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

        printf("{\"mode\":\"clip\",\"metric\":\"%s\",\"n_solves\":%ld,\"degree\":%d,"
               "\"clip_lo\":%.15g,\"clip_hi\":%.15g,"
               "\"min_score\":%.15g,\"max_score\":%.15g}\n",
               metricName, nSolves, degree, clipLo, clipHi,
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
                ? score_xformed(roots, degree, metric, rtChain, nRt, wkRe, wkIm)
                : compute_solve_metric_score(roots, degree, metric);
            double u = (score - clipLo) / range;
            if (u < 0) u = 0;
            if (u > 1) u = 1;
            int h = (int)(u * histBins);
            if (h >= histBins) h = histBins - 1;
            hist[h]++;
        }

        printf("{\"mode\":\"hist\",\"metric\":\"%s\",\"n_solves\":%ld,\"degree\":%d,"
               "\"hist_bins\":%d,\"clip_lo\":%.15g,\"clip_hi\":%.15g,"
               "\"hist\":[", metricName, nSolves, degree, histBins, clipLo, clipHi);
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
