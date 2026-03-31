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

/* LAPACK zgeev: complex double general matrix eigenvalues */
extern void zgeev_(char *jobvl, char *jobvr, int *n,
                   double _Complex *a, int *lda,
                   double _Complex *w,
                   double _Complex *vl, int *ldvl,
                   double _Complex *vr, int *ldvr,
                   double _Complex *work, int *lwork,
                   double *rwork, int *info);

/* Simple JSON parser helpers */
static char *read_stdin(void) {
    size_t cap = 4096, len = 0;
    char *buf = malloc(cap);
    int c;
    while ((c = fgetc(stdin)) != EOF) {
        if (len + 1 >= cap) { cap *= 2; buf = realloc(buf, cap); }
        buf[len++] = (char)c;
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

/* Solve one polynomial via companion matrix.
 * cf[0..n-1] are complex coefficients: cf[0]*z^(n-1) + cf[1]*z^(n-2) + ... + cf[n-1].
 * Returns roots in out_re/out_im, returns actual degree (number of roots). */
static int solve_companion(const double *cfRe, const double *cfIm, int nCoeffs,
                           float *out_re, float *out_im) {
    /* Find effective degree (skip leading zeros) */
    int first = 0;
    double maxMag = 0;
    for (int k = 0; k < nCoeffs; k++) {
        double m = cfRe[k] * cfRe[k] + cfIm[k] * cfIm[k];
        if (m > maxMag) maxMag = m;
    }
    /* All-zero polynomial: no roots */
    if (maxMag < 1e-60) {
        for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0; out_im[k] = 0; }
        return nCoeffs - 1;
    }
    double thr = maxMag * 1e-15;
    while (first < nCoeffs - 1 && (cfRe[first] * cfRe[first] + cfIm[first] * cfIm[first]) < thr)
        first++;

    int degree = nCoeffs - 1 - first;
    if (degree <= 0) {
        for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0; out_im[k] = 0; }
        return nCoeffs - 1;
    }

    /* Linear case: -cf[1]/cf[0] */
    if (degree == 1) {
        double _Complex a = cfRe[first] + I * cfIm[first];
        double _Complex b = cfRe[first + 1] + I * cfIm[first + 1];
        double _Complex root = -b / a;
        out_re[0] = (float)creal(root);
        out_im[0] = (float)cimag(root);
        /* Fill remaining with zeros */
        for (int k = 1; k < nCoeffs - 1; k++) { out_re[k] = 0; out_im[k] = 0; }
        return nCoeffs - 1;
    }

    /* Build companion matrix (column-major for LAPACK) */
    int n = degree;
    double _Complex *A = calloc(n * n, sizeof(double _Complex));
    if (!A) return 0;

    /* Normalize to monic: divide by leading coefficient */
    double _Complex lead = cfRe[first] + I * cfIm[first];
    for (int j = 0; j < n; j++) {
        /* First row: -b[j]/lead where b[j] = cf[first+1+j] */
        double _Complex bj = cfRe[first + 1 + j] + I * cfIm[first + 1 + j];
        A[j * n + 0] = -bj / lead;  /* A[0][j] in column-major = A[j*n + 0] */
    }
    /* Sub-diagonal of 1s */
    for (int k = 1; k < n; k++) {
        A[(k - 1) * n + k] = 1.0;  /* A[k][k-1] in column-major */
    }

    /* Compute eigenvalues */
    double _Complex *W = malloc(n * sizeof(double _Complex));
    double *rwork = malloc(2 * n * sizeof(double));
    int info, lwork = -1;
    double _Complex wkopt;
    char jobvl = 'N', jobvr = 'N';
    int ldvl = 1, ldvr = 1;

    /* Diagnostic: log state before first LAPACK call */
    double leadMag = cabs(lead);
    fprintf(stderr, "DIAG zgeev pre: first=%d degree=%d n=%d lda=%d leadMag=%.6e nCoeffs=%d\n",
            first, degree, n, n, leadMag, nCoeffs);

    /* Workspace query */
    zgeev_(&jobvl, &jobvr, &n, A, &n, W, NULL, &ldvl, NULL, &ldvr,
           &wkopt, &lwork, rwork, &info);
    if (info != 0) {
        /* Workspace query failed — zero output */
        for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0; out_im[k] = 0; }
        free(rwork); free(W); free(A);
        return nCoeffs - 1;
    }
    lwork = (int)creal(wkopt);
    double _Complex *work = malloc(lwork * sizeof(double _Complex));

    /* Actual eigensolve */
    zgeev_(&jobvl, &jobvr, &n, A, &n, W, NULL, &ldvl, NULL, &ldvr,
           work, &lwork, rwork, &info);
    if (info != 0) {
        /* Eigensolve failed or partially converged — zero output */
        for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0; out_im[k] = 0; }
        free(work); free(rwork); free(W); free(A);
        return nCoeffs - 1;
    }

    /* Write roots */
    for (int k = 0; k < n; k++) {
        double re = creal(W[k]), im = cimag(W[k]);
        if (!isfinite(re) || !isfinite(im)) { re = 0; im = 0; }
        out_re[k] = (float)re;
        out_im[k] = (float)im;
    }
    /* Pad remaining slots with zeros */
    for (int k = n; k < nCoeffs - 1; k++) { out_re[k] = 0; out_im[k] = 0; }

    free(work);
    free(rwork);
    free(W);
    free(A);

    return nCoeffs - 1;  /* Always return full degree for format compatibility */
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: sweep_cm output.bin < spec.json\n");
        return 1;
    }
    const char *outPath = argv[1];

    char *buf = read_stdin();

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

    long totalSteps = 0;
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    while (fread(coeffBuf, sizeof(float), nCoeffs * 2, fin) == (size_t)(nCoeffs * 2)) {
        totalSteps++;

        /* Convert float32 coefficients to double */
        for (int k = 0; k < nCoeffs; k++) {
            cfRe[k] = (double)coeffBuf[k * 2];
            cfIm[k] = (double)coeffBuf[k * 2 + 1];
        }

        solve_companion(cfRe, cfIm, nCoeffs, rootRe, rootIm);

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

    printf("{\"mode\":\"solve_cm\",\"n_t\":%ld,\"degree\":%d,\"avg_iterations\":0,\"compute_us\":%ld}\n",
           totalSteps, degree, elapsed_us);

    return 0;
}
