/*
 * sweep_cm: companion-matrix polynomial root solver via LAPACK zgeev.
 *
 * Reads coefficient chunks (same format as sweep_cli coeffgen output),
 * builds companion matrix for each polynomial, computes eigenvalues.
 * Output format matches the Aberth solver: interleaved float32 re/im pairs.
 *
 * Threading (CM threading wave): rows are independent and uniform-cost, so
 * the spec's n_threads (default 1) statically partitions the row range
 * across pthread workers, each with its own persistent zgeev workspace,
 * writing to disjoint slices of one preallocated output buffer. Row order
 * and per-row arithmetic are unchanged, so the output is byte-identical to
 * the historical sequential path at any thread count.
 *
 * Usage: sweep_cm output.bin < spec.json
 *
 * Build (dynamic, needs LAPACK/OpenBLAS from Lambda layer):
 *   gcc -O3 -pthread -o sweep_cm sweep_cm.c -L/opt/lib -llapack -lopenblas -lm -Wl,-rpath,/opt/lib
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include <pthread.h>

#define HAVE_LAPACK_COMPANION 1
#include "companion_solver.h"
#include "jt_solver.h"
#include "newton_solver.h"

/* solver brushes: which per-row algorithm this run paints with */
enum { CM_KIND_ZGEEV = 0, CM_KIND_JT = 1, CM_KIND_NEWTON = 2 };

#define READ_BUF_SIZE (256 * 1024)
#define CM_MAX_THREADS 64

/* Simple JSON parser helpers */
static char *read_stdin(void) {
    size_t cap = 4096, len = 0;
    char *buf = malloc(cap);
    if (!buf) return NULL;
    char chunk[READ_BUF_SIZE];
    size_t nread;
    while ((nread = fread(chunk, 1, sizeof(chunk), stdin)) > 0) {
        if (len + nread + 1 > cap) {
            size_t new_cap = cap;
            while (len + nread + 1 > new_cap) new_cap *= 2;
            char *new_buf = realloc(buf, new_cap);
            if (!new_buf) {
                free(buf);
                return NULL;
            }
            buf = new_buf;
            cap = new_cap;
        }
        memcpy(buf + len, chunk, nread);
        len += nread;
    }
    buf[len] = '\0';
    return buf;
}

static const char *find_key(const char *json, const char *key) {
    char pat[128];
    snprintf(pat, sizeof(pat), "\"%s\"", key);
    const char *p = strstr(json, pat);
    if (!p) return NULL;
    p += strlen(pat);
    while (*p && (*p == ' ' || *p == ':' || *p == '\t')) p++;
    return p;
}

static double parse_num(const char **p) {
    while (**p && (**p == ' ' || **p == '"')) (*p)++;
    return strtod(*p, (char **)p);
}

static void parse_string(const char *p, char *out, int max) {
    while (*p && *p != '"') p++;
    if (*p == '"') p++;
    int i = 0;
    while (*p && *p != '"' && i < max - 1) out[i++] = *p++;
    out[i] = '\0';
}

typedef struct {
    const float *coeffData;   /* rows * nCoeffs * 2 interleaved f32 */
    float *outData;           /* rows * degree  * 2 interleaved f32 */
    long rowStart;
    long rowEnd;
    int nCoeffs;
    int kind;                 /* CM_KIND_* */
    int newtonMaxSteps;       /* solver-brush knob; 0 = Newton default (50) */
    long skippedOverflow;
    int failed;
} CmWorkerArg;

static void *cmWorkerMain(void *vp) {
    CmWorkerArg *arg = (CmWorkerArg *)vp;
    int nCoeffs = arg->nCoeffs;
    int degree = nCoeffs - 1;
    double *cfRe = malloc((size_t)nCoeffs * sizeof(double));
    double *cfIm = malloc((size_t)nCoeffs * sizeof(double));
    float *rootRe = malloc((size_t)degree * sizeof(float));
    float *rootIm = malloc((size_t)degree * sizeof(float));
    if (!cfRe || !cfIm || !rootRe || !rootIm) {
        free(cfRe); free(cfIm); free(rootRe); free(rootIm);
        arg->failed = 1;
        return NULL;
    }
    CompanionWorkspace ws;
    memset(&ws, 0, sizeof(ws));
    JtState *jt = NULL;
    if (arg->kind == CM_KIND_JT) {
        jt = malloc(sizeof(JtState));
        if (!jt) {
            free(cfRe); free(cfIm); free(rootRe); free(rootIm);
            arg->failed = 1;
            return NULL;
        }
    }
    for (long r = arg->rowStart; r < arg->rowEnd; r++) {
        const float *src = arg->coeffData + (size_t)r * nCoeffs * 2;
        for (int k = 0; k < nCoeffs; k++) {
            cfRe[k] = (double)src[k * 2];
            cfIm[k] = (double)src[k * 2 + 1];
        }
        int rc;
        if (arg->kind == CM_KIND_JT) {
            rc = solve_jt_coeffs(jt, cfRe, cfIm, nCoeffs, rootRe, rootIm);
        } else if (arg->kind == CM_KIND_NEWTON) {
            rc = solve_newton_coeffs(cfRe, cfIm, nCoeffs, rootRe, rootIm,
                                     arg->newtonMaxSteps);
        } else {
            rc = solve_companion_coeffs_ws(&ws, cfRe, cfIm, nCoeffs,
                                           rootRe, rootIm, 0);
        }
        if (rc < 0) arg->skippedOverflow++;
        float *dst = arg->outData + (size_t)r * degree * 2;
        for (int k = 0; k < degree; k++) {
            dst[k * 2] = rootRe[k];
            dst[k * 2 + 1] = rootIm[k];
        }
    }
    free(jt);
    companion_ws_release(&ws);
    free(cfRe);
    free(cfIm);
    free(rootRe);
    free(rootIm);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: sweep_cm output.bin < spec.json\n");
        return 1;
    }
    const char *outPath = argv[1];

    char *buf = read_stdin();
    if (!buf) {
        fprintf(stderr, "Out of memory reading spec from stdin\n");
        return 1;
    }

    /* Parse spec */
    char coeffsFile[256] = "";
    const char *cp = find_key(buf, "coeffs_file");
    if (cp) parse_string(cp, coeffsFile, sizeof(coeffsFile));
    if (!coeffsFile[0]) { fprintf(stderr, "Missing coeffs_file\n"); return 1; }

    int nCoeffs = 0;
    cp = find_key(buf, "n_coeffs");
    if (cp) nCoeffs = (int)parse_num(&cp);
    if (nCoeffs < 2) { fprintf(stderr, "Invalid n_coeffs: %d\n", nCoeffs); return 1; }

    int kind = CM_KIND_ZGEEV;
    char modeStr[64] = "";
    cp = find_key(buf, "mode");
    if (cp) parse_string(cp, modeStr, sizeof(modeStr));
    if (strcmp(modeStr, "solve_jt") == 0) kind = CM_KIND_JT;
    else if (strcmp(modeStr, "solve_newton") == 0) kind = CM_KIND_NEWTON;
    else strcpy(modeStr, "solve_cm");

    /* capped-Newton brush knob: fewer steps = faster and MORE textured
     * (fewer roots fully converge). Ignored by solve_cm / solve_jt. */
    int newtonMaxSteps = 0;
    cp = find_key(buf, "max_iter");
    if (cp) {
        int mi = (int)parse_num(&cp);
        if (mi >= 1 && mi <= 64) newtonMaxSteps = mi;
    }

    int nThreads = 1;
    cp = find_key(buf, "n_threads");
    if (cp) nThreads = (int)parse_num(&cp);
    if (nThreads < 1) nThreads = 1;
    if (nThreads > CM_MAX_THREADS) nThreads = CM_MAX_THREADS;

    int degree = nCoeffs - 1;

    /* Read the whole coefficient file; the historical streaming loop
     * stopped at the first short read, so a trailing partial row is
     * ignored the same way here (rows = floor). */
    FILE *fin = fopen(coeffsFile, "rb");
    if (!fin) { fprintf(stderr, "Cannot open %s\n", coeffsFile); return 1; }
    if (fseek(fin, 0, SEEK_END) != 0) { fclose(fin); fprintf(stderr, "Cannot seek %s\n", coeffsFile); return 1; }
    long fileBytes = ftell(fin);
    if (fileBytes < 0) { fclose(fin); fprintf(stderr, "Cannot size %s\n", coeffsFile); return 1; }
    rewind(fin);

    size_t rowBytes = (size_t)nCoeffs * 2 * sizeof(float);
    long totalSteps = (long)((size_t)fileBytes / rowBytes);

    float *coeffData = NULL;
    float *outData = NULL;
    if (totalSteps > 0) {
        coeffData = malloc((size_t)totalSteps * rowBytes);
        outData = malloc((size_t)totalSteps * (size_t)degree * 2 * sizeof(float));
        if (!coeffData || !outData) {
            fprintf(stderr, "Out of memory allocating solver buffers\n");
            free(outData);
            free(coeffData);
            free(buf);
            fclose(fin);
            return 1;
        }
        if (fread(coeffData, 1, (size_t)totalSteps * rowBytes, fin)
                != (size_t)totalSteps * rowBytes) {
            fprintf(stderr, "Short read on %s\n", coeffsFile);
            free(outData);
            free(coeffData);
            free(buf);
            fclose(fin);
            return 1;
        }
    }
    fclose(fin);

    if (nThreads > totalSteps && totalSteps > 0) nThreads = (int)totalSteps;
    if (totalSteps == 0) nThreads = 1;

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    long skippedOverflow = 0;
    int workerFailed = 0;
    if (totalSteps > 0) {
        pthread_t threads[CM_MAX_THREADS];
        int joinable[CM_MAX_THREADS] = {0};
        CmWorkerArg args[CM_MAX_THREADS];
        long rowsPer = totalSteps / nThreads;
        long extra = totalSteps % nThreads;
        long cursor = 0;
        for (int i = 0; i < nThreads; i++) {
            long count = rowsPer + (i < extra ? 1 : 0);
            args[i].coeffData = coeffData;
            args[i].outData = outData;
            args[i].rowStart = cursor;
            args[i].rowEnd = cursor + count;
            args[i].nCoeffs = nCoeffs;
            args[i].kind = kind;
            args[i].newtonMaxSteps = newtonMaxSteps;
            args[i].skippedOverflow = 0;
            args[i].failed = 0;
            cursor += count;
        }
        for (int i = 0; i < nThreads; i++) {
            if (i == nThreads - 1) {
                /* run the last range on the main thread: one fewer spawn,
                 * and the single-thread case never touches pthreads */
                cmWorkerMain(&args[i]);
            } else if (pthread_create(&threads[i], NULL, cmWorkerMain, &args[i]) == 0) {
                joinable[i] = 1;
            } else {
                fprintf(stderr, "sweep_cm pthread_create failed for worker %d\n", i);
                cmWorkerMain(&args[i]);  /* fall back: run this range inline */
            }
        }
        for (int i = 0; i < nThreads - 1; i++) {
            if (joinable[i]) pthread_join(threads[i], NULL);
        }
        for (int i = 0; i < nThreads; i++) {
            skippedOverflow += args[i].skippedOverflow;
            workerFailed |= args[i].failed;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    long elapsed_us = (t1.tv_sec - t0.tv_sec) * 1000000L +
                      (t1.tv_nsec - t0.tv_nsec) / 1000L;

    if (workerFailed) {
        fprintf(stderr, "sweep_cm worker allocation failed\n");
        free(outData);
        free(coeffData);
        free(buf);
        return 1;
    }

    FILE *fout = fopen(outPath, "wb");
    if (!fout) {
        fprintf(stderr, "Cannot open %s\n", outPath);
        free(outData);
        free(coeffData);
        free(buf);
        return 1;
    }
    if (totalSteps > 0) {
        size_t outCount = (size_t)totalSteps * (size_t)degree * 2;
        if (fwrite(outData, sizeof(float), outCount, fout) != outCount) {
            fprintf(stderr, "Short write on %s\n", outPath);
            fclose(fout);
            free(outData);
            free(coeffData);
            free(buf);
            return 1;
        }
    }
    fclose(fout);

    free(outData);
    free(coeffData);
    free(buf);

    if (skippedOverflow > 0)
        fprintf(stderr, "WARNING: %ld/%ld polynomials skipped (coefficient overflow)\n",
                skippedOverflow, totalSteps);

    int newtonEffectiveSteps = (kind == CM_KIND_NEWTON)
        ? ((newtonMaxSteps >= 1 && newtonMaxSteps <= 50) ? newtonMaxSteps : 50)
        : 0;
    printf("{\"mode\":\"%s\",\"n_t\":%ld,\"degree\":%d,\"avg_iterations\":0,\"compute_us\":%ld,\"skipped_overflow\":%ld,\"n_threads\":%d,\"max_iter\":%d}\n",
           modeStr, totalSteps, degree, elapsed_us, skippedOverflow, nThreads, newtonEffectiveSteps);

    return 0;
}
