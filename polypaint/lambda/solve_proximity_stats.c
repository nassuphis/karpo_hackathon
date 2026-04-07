/*
 * solve_proximity_stats — compute solve-level metric scores from root .bin files.
 *
 * Supports multiple metrics via --metric (default: proximity).
 * Metrics: proximity, crowding, spread, anisotropy, area,
 *          clusteriness, shelliness, outlierness, nn_variation, real_axis_proximity,
 *          centroid_re, centroid_im, centroid_dist, dist_unit_circle, asymmetry_re.
 *
 * Three modes:
 *   --mode=clip      Compute score array, sort, emit clip bounds (quantiles).
 *   --mode=hist      Compute per-solve scores, emit 100-bin histogram using given clip bounds.
 *   --mode=summary   Compute full debug summary: quantiles, stats, clip, occupancy, actual 10 solve-score color bins.
 *
 * Usage:
 *   solve_proximity_stats input.bin --mode=clip --degree=D [--metric=proximity] [--quantile_lo=0.001] [--quantile_hi=0.999] [--root_xforms=file.json]
 *   solve_proximity_stats - --input_size=BYTES --mode=clip --degree=D [--metric=proximity] [--quantile_lo=0.001] [--quantile_hi=0.999] [--root_xforms=file.json]
 *   solve_proximity_stats input.bin --mode=hist --degree=D [--metric=proximity] --clip_lo=X --clip_hi=Y --hist_bins=100 [--omega=1] [--root_xforms=file.json]
 *   solve_proximity_stats - --input_size=BYTES --mode=hist --degree=D [--metric=proximity] --clip_lo=X --clip_hi=Y --hist_bins=100 [--omega=1] [--root_xforms=file.json]
 *
 * Output: JSON to stdout.
 *
 * Build: aarch64-linux-musl-gcc -O3 -static -o solve_proximity_stats solve_proximity_stats.c -lm
 */

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

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

static long long getArgLongLong(int argc, char **argv, const char *key, long long def) {
    const char *v = getArg(argc, argv, key);
    return v ? atoll(v) : def;
}

static const char *getArgStr(int argc, char **argv, const char *key, const char *def) {
    const char *v = getArg(argc, argv, key);
    return v ? v : def;
}

/* ---- Input loading ---- */

static int read_exact_stdin(float **bufOut, long *fileSizeOut, long long inputSizeHint) {
    if (inputSizeHint <= 0) return 0;
    if (inputSizeHint > LONG_MAX) {
        fprintf(stderr, "input_size too large: %lld\n", inputSizeHint);
        return -1;
    }
    long fileSize = (long)inputSizeHint;
    float *buf = malloc((size_t)fileSize);
    if (!buf) {
        fprintf(stderr, "Out of memory: %ld bytes\n", fileSize);
        return -1;
    }
    char *dst = (char *)buf;
    long total = 0;
    while (total < fileSize) {
        size_t want = (size_t)(fileSize - total);
        size_t got = fread(dst + total, 1, want, stdin);
        if (got == 0) {
            if (ferror(stdin)) {
                fprintf(stderr, "stdin read error\n");
            } else {
                fprintf(stderr, "Short stdin read: got %ld of %ld bytes\n", total, fileSize);
            }
            free(buf);
            return -1;
        }
        total += (long)got;
    }
    *bufOut = buf;
    *fileSizeOut = fileSize;
    return 1;
}

static int read_grow_stdin(float **bufOut, long *fileSizeOut) {
    size_t cap = 1024 * 1024;
    char *buf = malloc(cap);
    if (!buf) {
        fprintf(stderr, "Out of memory for stdin buffer\n");
        return -1;
    }
    size_t size = 0;
    while (1) {
        if (size == cap) {
            size_t nextCap = cap * 2;
            char *next = realloc(buf, nextCap);
            if (!next) {
                fprintf(stderr, "Out of memory growing stdin buffer\n");
                free(buf);
                return -1;
            }
            buf = next;
            cap = nextCap;
        }
        size_t got = fread(buf + size, 1, cap - size, stdin);
        size += got;
        if (got == 0) {
            if (ferror(stdin)) {
                fprintf(stderr, "stdin read error\n");
                free(buf);
                return -1;
            }
            break;
        }
    }
    if (size > LONG_MAX) {
        fprintf(stderr, "stdin buffer too large\n");
        free(buf);
        return -1;
    }
    *bufOut = (float *)buf;
    *fileSizeOut = (long)size;
    return 1;
}

static int read_input(const char *inPath, long long inputSizeHint, float **bufOut, long *fileSizeOut) {
    if (strcmp(inPath, "-") == 0) {
        int rc = read_exact_stdin(bufOut, fileSizeOut, inputSizeHint);
        if (rc != 0) return rc > 0 ? 0 : -1;
        rc = read_grow_stdin(bufOut, fileSizeOut);
        return rc > 0 ? 0 : -1;
    }

    FILE *f = fopen(inPath, "rb");
    if (!f) {
        fprintf(stderr, "Cannot open %s\n", inPath);
        return -1;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fprintf(stderr, "fseek failed for %s\n", inPath);
        fclose(f);
        return -1;
    }
    long fileSize = ftell(f);
    if (fileSize < 0) {
        fprintf(stderr, "ftell failed for %s\n", inPath);
        fclose(f);
        return -1;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fprintf(stderr, "fseek rewind failed for %s\n", inPath);
        fclose(f);
        return -1;
    }

    float *buf = malloc((size_t)fileSize);
    if (!buf) {
        fprintf(stderr, "Out of memory: %ld bytes\n", fileSize);
        fclose(f);
        return -1;
    }
    if ((long)fread(buf, 1, (size_t)fileSize, f) != fileSize) {
        fprintf(stderr, "Short read\n");
        free(buf);
        fclose(f);
        return -1;
    }
    fclose(f);

    *bufOut = buf;
    *fileSizeOut = fileSize;
    return 0;
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

static int count_finite_roots_in_solve(const float *roots, int degree) {
    int count = 0;
    for (int i = 0; i < degree; i++) {
        if (isfinite((double)roots[i * 2]) && isfinite((double)roots[i * 2 + 1])) {
            count++;
        }
    }
    return count;
}

static int clamp_threads(int requested, long n_items) {
    int threads = requested < 1 ? 1 : requested;
    if (n_items > 0 && threads > (int)n_items) threads = (int)n_items;
    if (threads < 1) threads = 1;
    return threads;
}

typedef struct {
    const float *buf;
    long start;
    long end;
    int stride;
    int degree;
    enum SolveMetric metric;
    RootXformEntry *rtChain;
    int nRt;
    double *scores;
    int collect_stats;
    int minFiniteRootsRequired;
    long finiteRootCount;
    long fullyFiniteSolveCount;
    long partialFiniteSolveCount;
    long zeroFiniteSolveCount;
    long usableSolveCount;
    int minFiniteRootsPerSolve;
    int maxFiniteRootsPerSolve;
} ScoreWorkerArgs;

static void *score_worker_main(void *arg_) {
    ScoreWorkerArgs *arg = (ScoreWorkerArgs *)arg_;
    float wkRe[MAXDEG], wkIm[MAXDEG];
    for (long s = arg->start; s < arg->end; s++) {
        const float *roots = arg->buf + s * arg->stride;
        if (arg->collect_stats) {
            int finiteRoots = count_finite_roots_in_solve(roots, arg->degree);
            arg->finiteRootCount += finiteRoots;
            if (finiteRoots == arg->degree) arg->fullyFiniteSolveCount++;
            else if (finiteRoots == 0) arg->zeroFiniteSolveCount++;
            else arg->partialFiniteSolveCount++;
            if (finiteRoots >= arg->minFiniteRootsRequired) arg->usableSolveCount++;
            if (finiteRoots < arg->minFiniteRootsPerSolve) arg->minFiniteRootsPerSolve = finiteRoots;
            if (finiteRoots > arg->maxFiniteRootsPerSolve) arg->maxFiniteRootsPerSolve = finiteRoots;
        }
        arg->scores[s] = (arg->nRt > 0)
            ? score_xformed(roots, arg->degree, arg->metric, arg->rtChain, arg->nRt, wkRe, wkIm)
            : compute_solve_metric_score(roots, arg->degree, arg->metric);
    }
    return NULL;
}

static void compute_scores_parallel(
    const float *buf,
    long nSolves,
    int stride,
    int degree,
    enum SolveMetric metric,
    RootXformEntry *rtChain,
    int nRt,
    double *scores,
    int requestedThreads,
    int collectStats,
    int minFiniteRootsRequired,
    long *finiteRootCount,
    long *fullyFiniteSolveCount,
    long *partialFiniteSolveCount,
    long *zeroFiniteSolveCount,
    long *usableSolveCount,
    int *minFiniteRootsPerSolve,
    int *maxFiniteRootsPerSolve,
    int *threadsUsed
) {
    int threads = clamp_threads(requestedThreads, nSolves);
    if (threadsUsed) *threadsUsed = threads;
    ScoreWorkerArgs *args = calloc((size_t)threads, sizeof(ScoreWorkerArgs));
    pthread_t *workers = calloc((size_t)threads, sizeof(pthread_t));
    if (!args || !workers) {
        fprintf(stderr, "Out of memory for score threads\n");
        free(args);
        free(workers);
        exit(1);
    }

    long base = nSolves / threads;
    long extra = nSolves % threads;
    long start = 0;
    for (int i = 0; i < threads; i++) {
        long width = base + (i < extra ? 1 : 0);
        args[i].buf = buf;
        args[i].start = start;
        args[i].end = start + width;
        args[i].stride = stride;
        args[i].degree = degree;
        args[i].metric = metric;
        args[i].rtChain = rtChain;
        args[i].nRt = nRt;
        args[i].scores = scores;
        args[i].collect_stats = collectStats;
        args[i].minFiniteRootsRequired = minFiniteRootsRequired;
        args[i].minFiniteRootsPerSolve = degree;
        args[i].maxFiniteRootsPerSolve = 0;
        pthread_create(&workers[i], NULL, score_worker_main, &args[i]);
        start += width;
    }

    long finiteRootsTotal = 0;
    long fullyFiniteTotal = 0;
    long partialFiniteTotal = 0;
    long zeroFiniteTotal = 0;
    long usableTotal = 0;
    int minFinite = degree;
    int maxFinite = 0;

    for (int i = 0; i < threads; i++) {
        pthread_join(workers[i], NULL);
        if (collectStats) {
            finiteRootsTotal += args[i].finiteRootCount;
            fullyFiniteTotal += args[i].fullyFiniteSolveCount;
            partialFiniteTotal += args[i].partialFiniteSolveCount;
            zeroFiniteTotal += args[i].zeroFiniteSolveCount;
            usableTotal += args[i].usableSolveCount;
            if (args[i].minFiniteRootsPerSolve < minFinite) minFinite = args[i].minFiniteRootsPerSolve;
            if (args[i].maxFiniteRootsPerSolve > maxFinite) maxFinite = args[i].maxFiniteRootsPerSolve;
        }
    }

    if (finiteRootCount) *finiteRootCount = finiteRootsTotal;
    if (fullyFiniteSolveCount) *fullyFiniteSolveCount = fullyFiniteTotal;
    if (partialFiniteSolveCount) *partialFiniteSolveCount = partialFiniteTotal;
    if (zeroFiniteSolveCount) *zeroFiniteSolveCount = zeroFiniteTotal;
    if (usableSolveCount) *usableSolveCount = usableTotal;
    if (minFiniteRootsPerSolve) *minFiniteRootsPerSolve = collectStats ? minFinite : degree;
    if (maxFiniteRootsPerSolve) *maxFiniteRootsPerSolve = collectStats ? maxFinite : degree;

    free(args);
    free(workers);
}

typedef struct {
    const float *buf;
    long start;
    long end;
    int stride;
    int degree;
    enum SolveMetric metric;
    RootXformEntry *rtChain;
    int nRt;
    double clipLo;
    double clipHi;
    double omega;
    int omegaEnabled;
    int histBins;
    long *hist;
} HistWorkerArgs;

static void *hist_worker_main(void *arg_) {
    HistWorkerArgs *arg = (HistWorkerArgs *)arg_;
    float wkRe[MAXDEG], wkIm[MAXDEG];
    double range = arg->clipHi - arg->clipLo;
    for (long s = arg->start; s < arg->end; s++) {
        const float *roots = arg->buf + s * arg->stride;
        double score = (arg->nRt > 0)
            ? score_xformed(roots, arg->degree, arg->metric, arg->rtChain, arg->nRt, wkRe, wkIm)
            : compute_solve_metric_score(roots, arg->degree, arg->metric);
        double u = (score - arg->clipLo) / range;
        if (u < 0) u = 0;
        if (u > 1) u = 1;
        u = apply_solve_score_transfer(u, arg->omegaEnabled, arg->omega);
        int h = (int)(u * arg->histBins);
        if (h >= arg->histBins) h = arg->histBins - 1;
        arg->hist[h]++;
    }
    return NULL;
}

static void compute_hist_parallel(
    const float *buf,
    long nSolves,
    int stride,
    int degree,
    enum SolveMetric metric,
    RootXformEntry *rtChain,
    int nRt,
    double clipLo,
    double clipHi,
    int histBins,
    double omega,
    int omegaEnabled,
    int requestedThreads,
    long *hist,
    int *threadsUsed
) {
    int threads = clamp_threads(requestedThreads, nSolves);
    if (threadsUsed) *threadsUsed = threads;
    HistWorkerArgs *args = calloc((size_t)threads, sizeof(HistWorkerArgs));
    pthread_t *workers = calloc((size_t)threads, sizeof(pthread_t));
    if (!args || !workers) {
        fprintf(stderr, "Out of memory for hist threads\n");
        free(args);
        free(workers);
        exit(1);
    }

    long base = nSolves / threads;
    long extra = nSolves % threads;
    long start = 0;
    for (int i = 0; i < threads; i++) {
        long width = base + (i < extra ? 1 : 0);
        args[i].buf = buf;
        args[i].start = start;
        args[i].end = start + width;
        args[i].stride = stride;
        args[i].degree = degree;
        args[i].metric = metric;
        args[i].rtChain = rtChain;
        args[i].nRt = nRt;
        args[i].clipLo = clipLo;
        args[i].clipHi = clipHi;
        args[i].omega = omega;
        args[i].omegaEnabled = omegaEnabled;
        args[i].histBins = histBins;
        args[i].hist = calloc((size_t)histBins, sizeof(long));
        if (!args[i].hist) {
            fprintf(stderr, "Out of memory for hist worker bins\n");
            exit(1);
        }
        pthread_create(&workers[i], NULL, hist_worker_main, &args[i]);
        start += width;
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(workers[i], NULL);
        for (int h = 0; h < histBins; h++) hist[h] += args[i].hist[h];
        free(args[i].hist);
    }

    free(args);
    free(workers);
}

/* ---- Main ---- */

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: solve_proximity_stats input.bin| - --mode=clip|hist|summary --degree=D "
                "[--metric=proximity|crowding|spread|anisotropy|area|clusteriness|shelliness|outlierness|nn_variation|real_axis_proximity|centroid_re|centroid_im|centroid_dist|dist_unit_circle|asymmetry_re] [options]\n");
        return 1;
    }

    const char *inPath = argv[1];
    long long inputSizeHint = getArgLongLong(argc, argv, "--input_size", -1);
    const char *mode = getArgStr(argc, argv, "--mode", "");
    int degree = getArgInt(argc, argv, "--degree", 0);
    const char *metricStr = getArgStr(argc, argv, "--metric", "proximity");
    double omega = getArgDouble(argc, argv, "--omega", 1.0);
    int omegaEnabled = getArgInt(argc, argv, "--omega_enabled", 1);
    int requestedThreads = getArgInt(argc, argv, "--threads", 1);

    if (degree < 1 || degree > MAXDEG) {
        fprintf(stderr, "Invalid degree: %d (must be 1-%d)\n", degree, MAXDEG);
        return 1;
    }

    enum SolveMetric metric;
    if (!parse_solve_metric(metricStr, &metric)) {
        fprintf(stderr, "Invalid metric: %s (use proximity|crowding|spread|anisotropy|area|clusteriness|shelliness|outlierness|nn_variation|real_axis_proximity|centroid_re|centroid_im|centroid_dist|dist_unit_circle|asymmetry_re)\n", metricStr);
        return 1;
    }

    /* Read input file/stdin */
    long fileSize = 0;
    float *buf = NULL;
    if (read_input(inPath, inputSizeHint, &buf, &fileSize) != 0) return 1;

    int stride = degree * 2;
    long nSolves = fileSize / (stride * (long)sizeof(float));
    if (nSolves <= 0) {
        fprintf(stderr, "Empty or invalid file: %ld bytes, degree=%d\n", fileSize, degree);
        free(buf);
        return 1;
    }

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

    const char *metricName = solve_metric_name(metric);

    if (strcmp(mode, "clip") == 0) {
        /* ---- CLIP MODE ---- */
        double quantileLo = getArgDouble(argc, argv, "--quantile_lo", 0.001);
        double quantileHi = getArgDouble(argc, argv, "--quantile_hi", 0.999);

        double *scores = malloc(nSolves * sizeof(double));
        if (!scores) { fprintf(stderr, "Out of memory for scores\n"); free(buf); return 1; }

        int threadsUsed = 1;
        compute_scores_parallel(
            buf, nSolves, stride, degree, metric, rtChain, nRt, scores,
            requestedThreads, 0, 0,
            NULL, NULL, NULL, NULL, NULL, NULL, NULL, &threadsUsed
        );

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

        printf("{\"mode\":\"clip\",\"metric\":\"%s\",\"n_solves\":%ld,\"degree\":%d,\"threads\":%d,"
               "\"omega\":%.15g,\"omega_enabled\":%s,\"clip_lo\":%.15g,\"clip_hi\":%.15g,"
               "\"min_score\":%.15g,\"max_score\":%.15g}\n",
               metricName, nSolves, degree, threadsUsed, omega, omegaEnabled ? "true" : "false", clipLo, clipHi,
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

        int threadsUsed = 1;
        compute_hist_parallel(
            buf, nSolves, stride, degree, metric, rtChain, nRt,
            clipLo, clipHi, histBins, omega, omegaEnabled, requestedThreads,
            hist, &threadsUsed
        );

        printf("{\"mode\":\"hist\",\"metric\":\"%s\",\"n_solves\":%ld,\"degree\":%d,\"threads\":%d,"
               "\"hist_bins\":%d,\"omega\":%.15g,\"omega_enabled\":%s,\"clip_lo\":%.15g,\"clip_hi\":%.15g,"
               "\"hist\":[", metricName, nSolves, degree, threadsUsed, histBins, omega, omegaEnabled ? "true" : "false", clipLo, clipHi);
        for (int i = 0; i < histBins; i++) {
            if (i > 0) printf(",");
            printf("%ld", hist[i]);
        }
        printf("]}\n");

        free(hist);

    } else if (strcmp(mode, "summary") == 0) {
        /* ---- SUMMARY MODE ---- */
        double quantileLo = getArgDouble(argc, argv, "--quantile_lo", 0.001);
        double quantileHi = getArgDouble(argc, argv, "--quantile_hi", 0.999);

        double *scores = malloc(nSolves * sizeof(double));
        if (!scores) { fprintf(stderr, "Out of memory for scores\n"); free(buf); return 1; }

        long totalRootSlots = nSolves * (long)degree;
        long finiteRootCount = 0;
        long fullyFiniteSolveCount = 0;
        long partialFiniteSolveCount = 0;
        long zeroFiniteSolveCount = 0;
        int minFiniteRootsRequired = solve_metric_min_roots(metric);
        long usableSolveCount = 0;
        int minFiniteRootsPerSolve = degree;
        int maxFiniteRootsPerSolve = 0;

        int threadsUsed = 1;
        compute_scores_parallel(
            buf, nSolves, stride, degree, metric, rtChain, nRt, scores,
            requestedThreads, 1, minFiniteRootsRequired,
            &finiteRootCount, &fullyFiniteSolveCount, &partialFiniteSolveCount,
            &zeroFiniteSolveCount, &usableSolveCount,
            &minFiniteRootsPerSolve, &maxFiniteRootsPerSolve,
            &threadsUsed
        );
        qsort(scores, nSolves, sizeof(double), cmp_double);

        /* Extremes */
        double minScore = scores[0];
        double maxScore = scores[nSolves - 1];

        /* Mean + stddev */
        double sum = 0;
        for (long s = 0; s < nSolves; s++) sum += scores[s];
        double meanScore = sum / nSolves;
        double var = 0;
        for (long s = 0; s < nSolves; s++) {
            double d = scores[s] - meanScore;
            var += d * d;
        }
        double stddevScore = sqrt(var / nSolves);

        /* Quantiles (same convention as clip mode) */
        #define QI(q) ((long)((nSolves - 1) * (q)))
        double q05 = scores[QI(0.05)];
        double q10 = scores[QI(0.10)];
        double q25 = scores[QI(0.25)];
        double q50 = scores[QI(0.50)];
        double q75 = scores[QI(0.75)];
        double q90 = scores[QI(0.90)];
        double q95 = scores[QI(0.95)];

        /* Clip bounds using same logic as clip mode */
        double clipLo, clipHi;
        const char *fallbackReason = NULL;
        int clipFallback = 0;
        if (nSolves < 100) {
            clipLo = minScore; clipHi = maxScore;
            clipFallback = 1; fallbackReason = "small_sample";
        } else {
            long loIdx = (long)((nSolves - 1) * quantileLo);
            long hiIdx = (long)((nSolves - 1) * quantileHi);
            if (hiIdx <= loIdx) {
                clipLo = minScore; clipHi = maxScore;
                clipFallback = 1; fallbackReason = "degenerate_quantile_range";
            } else {
                clipLo = scores[loIdx]; clipHi = scores[hiIdx];
                if (clipHi - clipLo < 1e-12) {
                    clipLo = minScore; clipHi = maxScore;
                    clipFallback = 1; fallbackReason = "degenerate_quantile_range";
                }
            }
        }
        if (clipHi - clipLo < 1e-12) {
            clipLo = minScore - 0.5; clipHi = minScore + 0.5;
            clipFallback = 1; fallbackReason = "zero_full_range_expanded";
        }

        double fullRange = maxScore - minScore;
        double clipRange = clipHi - clipLo;

        /* Clip occupancy */
        long belowCount = 0, inrangeCount = 0, aboveCount = 0;
        for (long s = 0; s < nSolves; s++) {
            if (scores[s] < clipLo) belowCount++;
            else if (scores[s] > clipHi) aboveCount++;
            else inrangeCount++;
        }

        /* ---- 100-bin intermediate histogram over clipped range (mirrors real pipeline) ---- */
        int intBins = 100;
        long *intHist = calloc(intBins, sizeof(long));
        if (!intHist) { fprintf(stderr, "Out of memory\n"); free(scores); free(buf); return 1; }
        for (long s = 0; s < nSolves; s++) {
            if (scores[s] < clipLo || scores[s] > clipHi) continue;
            double u = (scores[s] - clipLo) / clipRange;
            if (u < 0) u = 0; if (u > 1) u = 1;
            u = apply_solve_score_transfer(u, omegaEnabled, omega);
            int h = (int)(u * intBins);
            if (h >= intBins) h = intBins - 1;
            intHist[h]++;
        }

        /* ---- 9 equal-density cuts from 100-bin histogram (mirrors merge logic) ---- */
        int finalBins = 10;
        double cutsNorm[9];
        long totalInRange = inrangeCount;
        for (int k = 0; k < 9; k++) {
            long target = totalInRange * (k + 1) / finalBins;
            long cum = 0;
            double cut = 1.0;
            for (int i = 0; i < intBins; i++) {
                long cumBefore = cum;
                cum += intHist[i];
                if (cum >= target) {
                    double frac = intHist[i] > 0 ? (double)(target - cumBefore) / intHist[i] : 1.0;
                    cut = (i + frac) / intBins;
                    break;
                }
            }
            if (cut < 0) cut = 0; if (cut > 1) cut = 1;
            if (k > 0 && cut < cutsNorm[k - 1]) cut = cutsNorm[k - 1];
            cutsNorm[k] = cut;
        }
        free(intHist);

        /* Score-space cuts */
        double cutsScore[9];
        for (int k = 0; k < 9; k++)
            cutsScore[k] = clipLo + cutsNorm[k] * clipRange;

        /* ---- Final 10-bin counts (in-range solves only) ---- */
        long finalBinCounts[10] = {0};
        for (long s = 0; s < nSolves; s++) {
            if (scores[s] < clipLo || scores[s] > clipHi) continue;
            double u = (scores[s] - clipLo) / clipRange;
            if (u < 0) u = 0; if (u > 1) u = 1;
            u = apply_solve_score_transfer(u, omegaEnabled, omega);
            int bin = 0;
            for (int k = 0; k < 9; k++) {
                if (u > cutsNorm[k]) bin = k + 1;
            }
            finalBinCounts[bin]++;
        }

        /* ---- Outlier/saturation counts ---- */
        long minScoreCount = 0, maxScoreCount = 0, clipLoCount = 0, clipHiCount = 0;
        long nUnique = 0;
        double lastUnique = scores[0] - 1;
        for (long s = 0; s < nSolves; s++) {
            if (scores[s] == minScore) minScoreCount++;
            if (scores[s] == maxScore) maxScoreCount++;
            if (scores[s] == clipLo) clipLoCount++;
            if (scores[s] == clipHi) clipHiCount++;
            if (scores[s] != lastUnique) { nUnique++; lastUnique = scores[s]; }
        }

        /* ---- Emit JSON ---- */
        printf("{\"mode\":\"summary\",\"metric\":\"%s\",\"n_solves\":%ld,\"degree\":%d,\"threads\":%d,",
               metricName, nSolves, degree, threadsUsed);
        printf("\"min_score\":%.15g,\"max_score\":%.15g,", minScore, maxScore);
        printf("\"mean_score\":%.15g,\"stddev_score\":%.15g,", meanScore, stddevScore);
        printf("\"q05\":%.15g,\"q10\":%.15g,\"q25\":%.15g,\"q50\":%.15g,", q05, q10, q25, q50);
        printf("\"q75\":%.15g,\"q90\":%.15g,\"q95\":%.15g,", q75, q90, q95);
        printf("\"omega\":%.15g,\"omega_enabled\":%s,\"clip_quantile\":%.15g,\"clip_lo\":%.15g,\"clip_hi\":%.15g,",
               omega, omegaEnabled ? "true" : "false", quantileLo, clipLo, clipHi);
        printf("\"full_range\":%.15g,\"clip_range\":%.15g,", fullRange, clipRange);
        printf("\"clip_below_count\":%ld,\"clip_inrange_count\":%ld,\"clip_above_count\":%ld,",
               belowCount, inrangeCount, aboveCount);
        printf("\"clip_below_frac\":%.6f,\"clip_inrange_frac\":%.6f,\"clip_above_frac\":%.6f,",
               (double)belowCount / nSolves, (double)inrangeCount / nSolves, (double)aboveCount / nSolves);
        printf("\"clip_fallback\":%s,", clipFallback ? "true" : "false");
        if (fallbackReason)
            printf("\"clip_fallback_reason\":\"%s\",", fallbackReason);
        else
            printf("\"clip_fallback_reason\":null,");
        printf("\"metric_validity_policy\":\"%s\",", solve_metric_validity_policy_name());
        printf("\"metric_min_finite_roots\":%d,", minFiniteRootsRequired);
        printf("\"total_root_slots\":%ld,\"finite_root_count\":%ld,", totalRootSlots, finiteRootCount);
        printf("\"fully_finite_solve_count\":%ld,\"partial_finite_solve_count\":%ld,\"zero_finite_solve_count\":%ld,",
               fullyFiniteSolveCount, partialFiniteSolveCount, zeroFiniteSolveCount);
        printf("\"usable_solve_count\":%ld,", usableSolveCount);
        printf("\"forced_zero_score_count\":%ld,", nSolves - usableSolveCount);
        printf("\"finite_root_frac\":%.6f,", totalRootSlots > 0 ? (double)finiteRootCount / totalRootSlots : 0.0);
        printf("\"fully_finite_solve_frac\":%.6f,\"partial_finite_solve_frac\":%.6f,\"zero_finite_solve_frac\":%.6f,\"usable_solve_frac\":%.6f,",
               nSolves > 0 ? (double)fullyFiniteSolveCount / nSolves : 0.0,
               nSolves > 0 ? (double)partialFiniteSolveCount / nSolves : 0.0,
               nSolves > 0 ? (double)zeroFiniteSolveCount / nSolves : 0.0,
               nSolves > 0 ? (double)usableSolveCount / nSolves : 0.0);
        printf("\"mean_finite_roots_per_solve\":%.6f,\"min_finite_roots_per_solve\":%d,\"max_finite_roots_per_solve\":%d,",
               nSolves > 0 ? (double)finiteRootCount / nSolves : 0.0,
               minFiniteRootsPerSolve, maxFiniteRootsPerSolve);
        printf("\"intermediate_hist_bins\":100,\"final_bins\":10,");
        printf("\"cuts_norm\":[");
        for (int k = 0; k < 9; k++) { if (k) printf(","); printf("%.6f", cutsNorm[k]); }
        printf("],\"cuts_score\":[");
        for (int k = 0; k < 9; k++) { if (k) printf(","); printf("%.15g", cutsScore[k]); }
        printf("],\"final_bin_counts\":[");
        for (int k = 0; k < 10; k++) { if (k) printf(","); printf("%ld", finalBinCounts[k]); }
        printf("],\"final_bin_fracs\":[");
        for (int k = 0; k < 10; k++) { if (k) printf(","); printf("%.6f", totalInRange > 0 ? (double)finalBinCounts[k] / totalInRange : 0); }
        printf("],");
        printf("\"min_score_count\":%ld,\"max_score_count\":%ld,", minScoreCount, maxScoreCount);
        printf("\"clip_lo_count\":%ld,\"clip_hi_count\":%ld,", clipLoCount, clipHiCount);
        printf("\"n_unique_scores\":%ld}\n", nUnique);

        free(scores);

    } else {
        fprintf(stderr, "Unknown mode: %s (use clip, hist, or summary)\n", mode);
        free(buf);
        return 1;
    }

    free(buf);
    return 0;
}
