#ifndef POLYPAINT_COMPANION_SOLVER_H
#define POLYPAINT_COMPANION_SOLVER_H

#include <math.h>
#include <stdlib.h>

#ifdef HAVE_LAPACK_COMPANION
#include <complex.h>

/* LAPACK zgeev: complex double general matrix eigenvalues.
 * POLYPAINT_ACCELERATE_NEWLAPACK (local macOS test builds only): bind to
 * Accelerate's modern $NEWLAPACK symbol instead of the legacy CLAPACK-3.2
 * one — numpy links the modern interface, and the two vintages emit
 * eigenvalues in different QR deflation orders. Lambda builds link netlib
 * LAPACK 3.10 (same modern lineage) and never define this. */
#if defined(__APPLE__) && defined(POLYPAINT_ACCELERATE_NEWLAPACK)
extern void zgeev_(char *jobvl, char *jobvr, int *n,
                   double _Complex *a, int *lda,
                   double _Complex *w,
                   double _Complex *vl, int *ldvl,
                   double _Complex *vr, int *ldvr,
                   double _Complex *work, int *lwork,
                   double *rwork, int *info) __asm__("_zgeev$NEWLAPACK");
#else
extern void zgeev_(char *jobvl, char *jobvr, int *n,
                   double _Complex *a, int *lda,
                   double _Complex *w,
                   double _Complex *vl, int *ldvl,
                   double _Complex *vr, int *ldvr,
                   double _Complex *work, int *lwork,
                   double *rwork, int *info);
#endif

static inline int companion_solver_available(void) {
    return 1;
}

/* cf[0..n-1] are complex coefficients: cf[0]*z^(n-1) + ... + cf[n-1].
 * Returns roots in out_re/out_im. Positive return means success and indicates
 * format degree slots written (always nCoeffs-1). Negative return means the
 * polynomial was skipped due to overflow and zeros were written instead.
 * strip_exact = 0: legacy behavior, leading coefficients below a relative
 * threshold (|cf|^2 < max^2 * 1e-15) are treated as zero — protective for
 * garbage rows but it deletes deliberately tiny leading structure.
 * strip_exact = 1: np.roots semantics — only EXACTLY zero leading
 * coefficients are stripped; tiny leads keep their (giant) roots. */
static inline int solve_companion_coeffs(const double *cfRe, const double *cfIm, int nCoeffs,
                                         float *out_re, float *out_im, int strip_exact) {
    int first = 0;
    double maxMag = 0.0;
    for (int k = 0; k < nCoeffs; k++) {
        double m = cfRe[k] * cfRe[k] + cfIm[k] * cfIm[k];
        if (m > maxMag) maxMag = m;
    }
    if (maxMag < 1e-60) {
        for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
        return nCoeffs - 1;
    }

    double thr = strip_exact ? 0.0 : maxMag * 1e-15;
    while (first < nCoeffs - 1 &&
           (strip_exact
                ? (cfRe[first] == 0.0 && cfIm[first] == 0.0)
                : (cfRe[first] * cfRe[first] + cfIm[first] * cfIm[first]) < thr)) {
        first++;
    }

    int degree = nCoeffs - 1 - first;
    if (degree <= 0) {
        for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
        return nCoeffs - 1;
    }

    if (degree == 1) {
        double _Complex a = cfRe[first] + I * cfIm[first];
        double _Complex b = cfRe[first + 1] + I * cfIm[first + 1];
        double _Complex root = -b / a;
        out_re[0] = (float)creal(root);
        out_im[0] = (float)cimag(root);
        for (int k = 1; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
        return nCoeffs - 1;
    }

    int n = degree;
    double _Complex *A = calloc((size_t)n * (size_t)n, sizeof(double _Complex));
    if (!A) return 0;

    double _Complex lead = cfRe[first] + I * cfIm[first];
    if (!isfinite(cabs(lead))) {
        for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
        free(A);
        return -(nCoeffs - 1);
    }

    for (int j = 0; j < n; j++) {
        double _Complex bj = cfRe[first + 1 + j] + I * cfIm[first + 1 + j];
        A[j * n + 0] = -bj / lead;
        if (!isfinite(creal(A[j * n + 0])) || !isfinite(cimag(A[j * n + 0]))) {
            for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
            free(A);
            return -(nCoeffs - 1);
        }
    }
    for (int k = 1; k < n; k++) {
        A[(k - 1) * n + k] = 1.0;
    }

    double _Complex *W = malloc((size_t)n * sizeof(double _Complex));
    double *rwork = malloc((size_t)(2 * n) * sizeof(double));
    if (!W || !rwork) {
        for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
        free(rwork);
        free(W);
        free(A);
        return nCoeffs - 1;
    }

    int info, lwork = -1;
    double _Complex wkopt;
    char jobvl = 'N', jobvr = 'N';
    int ldvl = 1, ldvr = 1;

    zgeev_(&jobvl, &jobvr, &n, A, &n, W, NULL, &ldvl, NULL, &ldvr,
           &wkopt, &lwork, rwork, &info);
    if (info != 0) {
        for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
        free(rwork);
        free(W);
        free(A);
        return nCoeffs - 1;
    }

    lwork = (int)creal(wkopt);
    if (lwork < 1) {
        for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
        free(rwork);
        free(W);
        free(A);
        return nCoeffs - 1;
    }

    double _Complex *work = malloc((size_t)lwork * sizeof(double _Complex));
    if (!work) {
        for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
        free(rwork);
        free(W);
        free(A);
        return nCoeffs - 1;
    }

    zgeev_(&jobvl, &jobvr, &n, A, &n, W, NULL, &ldvl, NULL, &ldvr,
           work, &lwork, rwork, &info);
    if (info != 0) {
        for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
        free(work);
        free(rwork);
        free(W);
        free(A);
        return nCoeffs - 1;
    }

    for (int k = 0; k < n; k++) {
        double re = creal(W[k]), im = cimag(W[k]);
        if (!isfinite(re) || !isfinite(im)) { re = 0.0; im = 0.0; }
        out_re[k] = (float)re;
        out_im[k] = (float)im;
    }
    for (int k = n; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }

    free(work);
    free(rwork);
    free(W);
    free(A);
    return nCoeffs - 1;
}

#else

static inline int companion_solver_available(void) {
    return 0;
}

static inline int solve_companion_coeffs(const double *cfRe, const double *cfIm, int nCoeffs,
                                         float *out_re, float *out_im, int strip_exact) {
    (void)cfRe;
    (void)cfIm;
    (void)strip_exact;
    for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
    return 0;
}

#endif

#endif
