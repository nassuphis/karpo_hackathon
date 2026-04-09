/*
 * sweep_cm: companion-matrix polynomial root solver via LAPACK zgeev.
 *
 * Reads coefficient chunks (same format as sweep_cli coeffgen output),
 * builds companion matrix for each polynomial, computes eigenvalues.
 * Output format matches the Aberth solver: interleaved float32 re/im pairs.
 *
 * Usage: sweep_cm output.bin < spec.json
 *
 * Build (dynamic, needs LAPACK/OpenBLAS from Lambda layer):
 *   gcc -O3 -o sweep_cm sweep_cm.c -L/opt/lib -llapack -lopenblas -lm -Wl,-rpath,/opt/lib
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <time.h>

#define HAVE_LAPACK_COMPANION 1
#include "companion_solver.h"

#define READ_BUF_SIZE (256 * 1024)

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

/* Shared companion solver implementation lives in companion_solver.h. */
static int solve_companion(const double *cfRe, const double *cfIm, int nCoeffs,
                           float *out_re, float *out_im) {
    return solve_companion_coeffs(cfRe, cfIm, nCoeffs, out_re, out_im);
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

    int degree = nCoeffs - 1;

    /* Open coefficient file */
    FILE *fin = fopen(coeffsFile, "rb");
    if (!fin) { fprintf(stderr, "Cannot open %s\n", coeffsFile); return 1; }

    FILE *fout = fopen(outPath, "wb");
    if (!fout) { fclose(fin); fprintf(stderr, "Cannot open %s\n", outPath); return 1; }

    float *coeffBuf = malloc(nCoeffs * 2 * sizeof(float));
    float *rootRe = malloc(degree * sizeof(float));
    float *rootIm = malloc(degree * sizeof(float));
    double *cfRe = malloc(nCoeffs * sizeof(double));
    double *cfIm = malloc(nCoeffs * sizeof(double));
    if (!coeffBuf || !rootRe || !rootIm || !cfRe || !cfIm) {
        fprintf(stderr, "Out of memory allocating solver buffers\n");
        free(cfIm);
        free(cfRe);
        free(rootIm);
        free(rootRe);
        free(coeffBuf);
        free(buf);
        fclose(fout);
        fclose(fin);
        return 1;
    }

    long totalSteps = 0;
    long skippedOverflow = 0;
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    while (fread(coeffBuf, sizeof(float), nCoeffs * 2, fin) == (size_t)(nCoeffs * 2)) {
        totalSteps++;

        /* Convert float32 coefficients to double */
        for (int k = 0; k < nCoeffs; k++) {
            cfRe[k] = (double)coeffBuf[k * 2];
            cfIm[k] = (double)coeffBuf[k * 2 + 1];
        }

        int rc = solve_companion(cfRe, cfIm, nCoeffs, rootRe, rootIm);
        if (rc < 0) skippedOverflow++;

        /* Write interleaved float32 re/im pairs */
        for (int k = 0; k < degree; k++) {
            float pair[2] = { rootRe[k], rootIm[k] };
            fwrite(pair, sizeof(float), 2, fout);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    long elapsed_us = (t1.tv_sec - t0.tv_sec) * 1000000L +
                      (t1.tv_nsec - t0.tv_nsec) / 1000L;

    fclose(fin);
    fclose(fout);
    free(coeffBuf);
    free(rootRe);
    free(rootIm);
    free(cfRe);
    free(cfIm);
    free(buf);

    if (skippedOverflow > 0)
        fprintf(stderr, "WARNING: %ld/%ld polynomials skipped (coefficient overflow)\n",
                skippedOverflow, totalSteps);

    printf("{\"mode\":\"solve_cm\",\"n_t\":%ld,\"degree\":%d,\"avg_iterations\":0,\"compute_us\":%ld,\"skipped_overflow\":%ld}\n",
           totalSteps, degree, elapsed_us, skippedOverflow);

    return 0;
}
