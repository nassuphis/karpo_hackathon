/*
 * Multithreaded solve-from-coefficients binary for AE root solving.
 *
 * Reads JSON solve spec from stdin.
 * Writes packed f32 roots to argv[1].
 * Writes metadata JSON to stdout.
 *
 * This mirrors sweep_cli.c "solve" mode, but parallelizes across coefficient
 * records in contiguous worker blocks. It preserves warm starts within each
 * block, but block boundaries cold-start from the same perturbed-circle seed
 * used by single-thread AE. match_roots=true remains unsupported.
 *
 * Build: aarch64-linux-musl-gcc -O3 -static -pthread -o sweep_mt sweep_mt.c -lm
 * Local: cc -O3 -pthread -o sweep_mt sweep_mt.c -lm
 */

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_DEGREE 255
#define MAX_COEFFS 256
#define MAX_ITER 64
#define TOL2 1e-16
#define BUF_SIZE (1024 * 256)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int solveEA(const double *cr, const double *ci, int n,
                   double *rRe, double *rIm, int degree)
{
    for (int iter = 0; iter < MAX_ITER; iter++) {
        double maxCorr2 = 0;
        for (int i = 0; i < degree; i++) {
            double zR = rRe[i], zI = rIm[i];

            double pR = cr[0], pI = ci[0];
            double dpR = 0, dpI = 0;
            for (int k = 1; k < n; k++) {
                double ndR = dpR * zR - dpI * zI + pR;
                double ndI = dpR * zI + dpI * zR + pI;
                dpR = ndR;
                dpI = ndI;
                double npR = pR * zR - pI * zI + cr[k];
                double npI = pR * zI + pI * zR + ci[k];
                pR = npR;
                pI = npI;
            }

            double dpM = dpR * dpR + dpI * dpI;
            if (dpM < 1e-60) continue;
            double wR = (pR * dpR + pI * dpI) / dpM;
            double wI = (pI * dpR - pR * dpI) / dpM;

            double sR = 0, sI = 0;
            for (int j = 0; j < degree; j++) {
                if (j == i) continue;
                double dR = zR - rRe[j], dI = zI - rIm[j];
                double dM = dR * dR + dI * dI;
                if (dM < 1e-60) continue;
                sR += dR / dM;
                sI += -dI / dM;
            }

            double wsR = wR * sR - wI * sI;
            double wsI = wR * sI + wI * sR;
            double dnR = 1 - wsR, dnI = -wsI;
            double dnM = dnR * dnR + dnI * dnI;
            if (dnM < 1e-60) continue;

            double crrR = (wR * dnR + wI * dnI) / dnM;
            double crrI = (wI * dnR - wR * dnI) / dnM;
            rRe[i] -= crrR;
            rIm[i] -= crrI;

            double h2 = crrR * crrR + crrI * crrI;
            if (h2 > maxCorr2) maxCorr2 = h2;
        }
        if (maxCorr2 < TOL2) return iter + 1;
    }
    return MAX_ITER;
}

static const char *skip(const char *p) {
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    return p;
}

static double parseNum(const char **pp) {
    const char *p = skip(*pp);
    char *end;
    double v = strtod(p, &end);
    *pp = end;
    return v;
}

static const char *findKey(const char *json, const char *key) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *p = strstr(json, pattern);
    if (!p) return NULL;
    p += strlen(pattern);
    p = skip(p);
    if (*p == ':') p++;
    return skip(p);
}

static int parseBool(const char *p) {
    p = skip(p);
    return (*p == 't' || *p == '1');
}

static int parseString(const char *p, char *out, int maxLen) {
    p = skip(p);
    if (*p != '"') return 0;
    p++;
    int i = 0;
    while (*p && *p != '"' && i < maxLen - 1) out[i++] = *p++;
    out[i] = '\0';
    return i;
}

static void seedEAInitialGuess(double *rootRe, double *rootIm, int degree) {
    for (int i = 0; i < degree; i++) {
        double ang = 2.0 * M_PI * i / degree + 0.3;
        double r = 1.0 + 0.1 * i / degree;
        rootRe[i] = r * cos(ang);
        rootIm[i] = r * sin(ang);
    }
}

static int warmStartNeedsReseed(const double *rootRe, const double *rootIm, int effDeg) {
    if (effDeg <= 0) return 0;

    double warmMag = 0;
    for (int i = 0; i < effDeg; i++) {
        double re = rootRe[i], im = rootIm[i];
        if (!isfinite(re) || !isfinite(im)) return 1;
        warmMag += re * re + im * im;
    }
    if (warmMag < 1e-20) return 1;

    for (int i = 0; i < effDeg; i++) {
        for (int j = i + 1; j < effDeg; j++) {
            double dR = rootRe[i] - rootRe[j];
            double dI = rootIm[i] - rootIm[j];
            if (dR * dR + dI * dI < 1e-18) return 1;
        }
    }
    return 0;
}

static unsigned long long readUllFile(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;
    char buf[128] = {0};
    size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
    fclose(fp);
    if (n == 0) return 0;
    if (strncmp(buf, "max", 3) == 0) return 0;
    errno = 0;
    unsigned long long v = strtoull(buf, NULL, 10);
    if (errno != 0) return 0;
    return v;
}

static unsigned long long detectMemoryLimitBytes(void) {
    unsigned long long v = readUllFile("/sys/fs/cgroup/memory.max");
    if (v > 0 && v < (1ULL << 60)) return v;
    v = readUllFile("/sys/fs/cgroup/memory/memory.limit_in_bytes");
    if (v > 0 && v < (1ULL << 60)) return v;

    long pages = sysconf(_SC_PHYS_PAGES);
    long pageSize = sysconf(_SC_PAGESIZE);
    if (pages > 0 && pageSize > 0) {
        return (unsigned long long)pages * (unsigned long long)pageSize;
    }
    return 0;
}

typedef struct {
    long stepStart;
    long stepEnd;
    int nCoeffs;
    int degree;
    const float *coeffData;
    float *rootData;
    long totalIters;
} SolveThread;

static int solveOneRecordWarm(const float *coeffStep, float *rootStep, int nCoeffs, int degree,
                              double *rootRe, double *rootIm) {
    double coeffRe[MAX_COEFFS], coeffIm[MAX_COEFFS];

    for (int k = 0; k < nCoeffs; k++) {
        coeffRe[k] = (double)coeffStep[k * 2];
        coeffIm[k] = (double)coeffStep[k * 2 + 1];
    }

    int start = 0;
    while (start < nCoeffs - 1 &&
           coeffRe[start] * coeffRe[start] + coeffIm[start] * coeffIm[start] < 1e-30) {
        start++;
    }
    int effN = nCoeffs - start;

    int trailingZeros = 0;
    while (trailingZeros < effN - 1) {
        int k = start + effN - 1 - trailingZeros;
        if (coeffRe[k] * coeffRe[k] + coeffIm[k] * coeffIm[k] >= 1e-30) break;
        trailingZeros++;
    }
    effN -= trailingZeros;
    int effDeg = effN - 1;

    int iters;
    if (effDeg <= 0) {
        for (int i = 0; i < degree; i++) {
            rootRe[i] = 0;
            rootIm[i] = 0;
        }
        iters = 0;
    } else if (effDeg == 1) {
        for (int i = 1; i < degree; i++) {
            rootRe[i] = 0;
            rootIm[i] = 0;
        }
        rootRe[0] = 0;
        rootIm[0] = 0;
        double aR = coeffRe[start], aI = coeffIm[start];
        double bR = coeffRe[start + 1], bI = coeffIm[start + 1];
        double d = aR * aR + aI * aI;
        if (d > 1e-30) {
            rootRe[0] = -(bR * aR + bI * aI) / d;
            rootIm[0] = -(bI * aR - bR * aI) / d;
        }
        iters = 1;
    } else {
        for (int i = effDeg; i < degree; i++) {
            rootRe[i] = 0;
            rootIm[i] = 0;
        }
        if (warmStartNeedsReseed(rootRe, rootIm, effDeg)) {
            seedEAInitialGuess(rootRe, rootIm, effDeg);
        }
        iters = solveEA(coeffRe + start, coeffIm + start, effN, rootRe, rootIm, effDeg);
    }

    for (int i = 0; i < degree; i++) {
        rootStep[i * 2] = (float)rootRe[i];
        rootStep[i * 2 + 1] = (float)rootIm[i];
    }
    return iters;
}

static void *solveThreadMain(void *arg) {
    SolveThread *job = (SolveThread *)arg;
    long totalIters = 0;
    long coeffStride = (long)job->nCoeffs * 2;
    long rootStride = (long)job->degree * 2;
    double rootRe[MAX_DEGREE], rootIm[MAX_DEGREE];

    seedEAInitialGuess(rootRe, rootIm, job->degree);

    for (long step = job->stepStart; step < job->stepEnd; step++) {
        const float *coeffStep = job->coeffData + step * coeffStride;
        float *rootStep = job->rootData + step * rootStride;
        totalIters += solveOneRecordWarm(coeffStep, rootStep, job->nCoeffs, job->degree, rootRe, rootIm);
    }
    job->totalIters = totalIters;
    return NULL;
}

static int runSolveMT(const char *buf, const char *outPath) {
    char coeffsFile[256] = "";
    const char *cp = findKey(buf, "coeffs_file");
    if (cp) parseString(cp, coeffsFile, sizeof(coeffsFile));
    if (!coeffsFile[0]) {
        fprintf(stderr, "Missing coeffs_file\n");
        return 1;
    }

    int nCoeffs = 0;
    cp = findKey(buf, "n_coeffs");
    if (cp) nCoeffs = (int)parseNum(&cp);
    if (nCoeffs < 2 || nCoeffs > MAX_COEFFS) {
        fprintf(stderr, "Invalid n_coeffs: %d\n", nCoeffs);
        return 1;
    }
    int degree = nCoeffs - 1;

    int n1 = 100, n2 = 100;
    cp = findKey(buf, "n1");
    if (cp) n1 = (int)parseNum(&cp);
    cp = findKey(buf, "n2");
    if (cp) n2 = (int)parseNum(&cp);

    int i1_start = 0, i1_end = n1;
    cp = findKey(buf, "i1_start");
    if (cp) i1_start = (int)parseNum(&cp);
    cp = findKey(buf, "i1_end");
    if (cp) i1_end = (int)parseNum(&cp);
    if (i1_start < 0) i1_start = 0;
    if (i1_end > n1) i1_end = n1;

    int doMatch = 1;
    cp = findKey(buf, "match_roots");
    if (cp) doMatch = parseBool(cp);
    if (doMatch) {
        fprintf(stderr, "solve_mt only supports match_roots=false\n");
        return 1;
    }

    int requestedThreads = 0;
    cp = findKey(buf, "n_threads");
    if (cp) requestedThreads = (int)parseNum(&cp);
    if (requestedThreads <= 0) {
        const char *envThreads = getenv("SWEEP_MT_THREADS");
        if (envThreads && *envThreads) requestedThreads = atoi(envThreads);
    }

    FILE *fin = fopen(coeffsFile, "rb");
    if (!fin) {
        fprintf(stderr, "Cannot open %s\n", coeffsFile);
        return 1;
    }
    if (fseek(fin, 0, SEEK_END) != 0) {
        fclose(fin);
        fprintf(stderr, "Cannot seek %s\n", coeffsFile);
        return 1;
    }
    long fileBytes = ftell(fin);
    if (fileBytes < 0) {
        fclose(fin);
        fprintf(stderr, "Cannot stat %s\n", coeffsFile);
        return 1;
    }
    rewind(fin);

    long coeffStrideBytes = (long)nCoeffs * 2L * (long)sizeof(float);
    if (coeffStrideBytes <= 0 || (fileBytes % coeffStrideBytes) != 0) {
        fclose(fin);
        fprintf(stderr, "Coeff file size %ld is not divisible by record size %ld\n",
                fileBytes, coeffStrideBytes);
        return 1;
    }
    long totalSteps = fileBytes / coeffStrideBytes;
    if (totalSteps <= 0) {
        fclose(fin);
        fprintf(stderr, "Empty coeff file\n");
        return 1;
    }

    long declaredSteps = (long)(i1_end - i1_start) * (long)n2;
    if (declaredSteps > 0 && declaredSteps != totalSteps) {
        fclose(fin);
        fprintf(stderr, "Declared steps %ld do not match coeff records %ld\n",
                declaredSteps, totalSteps);
        return 1;
    }

    long rootBytes = totalSteps * (long)degree * 2L * (long)sizeof(float);
    unsigned long long scratchBytes = 0;
    unsigned long long memLimit = detectMemoryLimitBytes();

    long hwThreads = sysconf(_SC_NPROCESSORS_ONLN);
    if (hwThreads < 1) hwThreads = 1;
    int nThreads = requestedThreads > 0 ? requestedThreads : (int)hwThreads;
    if (nThreads < 1) nThreads = 1;
    if ((long)nThreads > totalSteps) nThreads = (int)totalSteps;
    scratchBytes = (unsigned long long)nThreads *
                   (unsigned long long)((MAX_COEFFS * 2 + MAX_DEGREE * 2) * sizeof(double));

    unsigned long long totalBytes = (unsigned long long)fileBytes +
                                    (unsigned long long)rootBytes +
                                    scratchBytes;
    if (memLimit > 0 && totalBytes > (memLimit * 85ULL) / 100ULL) {
        fclose(fin);
        fprintf(stderr,
                "solve_mt memory estimate too large: need ~%llu bytes, limit ~%llu bytes\n",
                totalBytes, memLimit);
        return 1;
    }

    float *coeffData = (float *)malloc((size_t)fileBytes);
    float *rootData = (float *)malloc((size_t)rootBytes);
    if (!coeffData || !rootData) {
        fclose(fin);
        free(coeffData);
        free(rootData);
        fprintf(stderr, "Out of memory for coeff/root buffers (%ld + %ld bytes)\n",
                fileBytes, rootBytes);
        return 1;
    }
    if (fread(coeffData, 1, (size_t)fileBytes, fin) != (size_t)fileBytes) {
        fclose(fin);
        free(coeffData);
        free(rootData);
        fprintf(stderr, "Failed to read coeff data\n");
        return 1;
    }
    fclose(fin);

    pthread_t *threads = (pthread_t *)calloc((size_t)nThreads, sizeof(pthread_t));
    SolveThread *jobs = (SolveThread *)calloc((size_t)nThreads, sizeof(SolveThread));
    if (!threads || !jobs) {
        free(threads);
        free(jobs);
        free(coeffData);
        free(rootData);
        fprintf(stderr, "Out of memory for thread metadata\n");
        return 1;
    }

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    long cursor = 0;
    for (int i = 0; i < nThreads; i++) {
        long remaining = totalSteps - cursor;
        long workersLeft = nThreads - i;
        long block = (remaining + workersLeft - 1) / workersLeft;
        jobs[i].stepStart = cursor;
        jobs[i].stepEnd = cursor + block;
        jobs[i].nCoeffs = nCoeffs;
        jobs[i].degree = degree;
        jobs[i].coeffData = coeffData;
        jobs[i].rootData = rootData;
        cursor += block;
        if (pthread_create(&threads[i], NULL, solveThreadMain, &jobs[i]) != 0) {
            fprintf(stderr, "pthread_create failed for worker %d\n", i);
            for (int j = 0; j < i; j++) pthread_join(threads[j], NULL);
            free(threads);
            free(jobs);
            free(coeffData);
            free(rootData);
            return 1;
        }
    }

    long totalIters = 0;
    for (int i = 0; i < nThreads; i++) {
        pthread_join(threads[i], NULL);
        totalIters += jobs[i].totalIters;
    }

    FILE *fout = fopen(outPath, "wb");
    if (!fout) {
        free(threads);
        free(jobs);
        free(coeffData);
        free(rootData);
        fprintf(stderr, "Cannot open %s\n", outPath);
        return 1;
    }
    if (fwrite(rootData, 1, (size_t)rootBytes, fout) != (size_t)rootBytes) {
        fclose(fout);
        free(threads);
        free(jobs);
        free(coeffData);
        free(rootData);
        fprintf(stderr, "Failed to write %s\n", outPath);
        return 1;
    }
    fclose(fout);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    long elapsed_us = (t1.tv_sec - t0.tv_sec) * 1000000L +
                      (t1.tv_nsec - t0.tv_nsec) / 1000L;

    free(threads);
    free(jobs);
    free(coeffData);
    free(rootData);

    double avgIters = totalSteps > 0 ? (double)totalIters / (double)totalSteps : 0.0;
    printf("{\"mode\":\"solve_mt\",\"degree\":%d,"
           "\"n1\":%d,\"n2\":%d,"
           "\"i1_start\":%d,\"i1_end\":%d,"
           "\"n_t\":%ld,\"stride\":%d,\"matched\":false,"
           "\"data_bytes\":%ld,\"elapsed_us\":%ld,"
           "\"avg_iterations\":%.2f,\"n_threads\":%d}\n",
           degree, n1, n2, i1_start, i1_end,
           totalSteps, degree * 2, rootBytes, elapsed_us,
           avgIters, nThreads);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s out.bin\n", argv[0]);
        return 1;
    }

    char *buf = malloc(BUF_SIZE);
    if (!buf) {
        fprintf(stderr, "OOM\n");
        return 1;
    }
    size_t n = fread(buf, 1, BUF_SIZE - 1, stdin);
    buf[n] = '\0';

    char mode[32] = "";
    const char *cp = findKey(buf, "mode");
    if (cp) parseString(cp, mode, sizeof(mode));
    int rc = 0;
    if (strcmp(mode, "solve_mt") == 0 || strcmp(mode, "solve") == 0 || mode[0] == '\0') {
        rc = runSolveMT(buf, argv[1]);
    } else {
        fprintf(stderr, "Unknown mode: %s\n", mode);
        rc = 1;
    }
    free(buf);
    return rc;
}
