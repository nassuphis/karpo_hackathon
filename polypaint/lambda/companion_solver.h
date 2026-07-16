#ifndef POLYPAINT_COMPANION_SOLVER_H
#define POLYPAINT_COMPANION_SOLVER_H

#include <math.h>
#include <stdlib.h>
#include <string.h>

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

/* Per-thread reusable zgeev workspace (CR: sweep_cm threading wave).
 * Buffers grow to the largest matrix seen; the lwork QUERY is cached per
 * matrix dimension and re-issued only when n changes, so zgeev always
 * receives the same lwork it would have been given by the old
 * query-every-call code — its internal blocking choices, and therefore its
 * eigenvalue dust, are byte-identical to the historical behavior. */
typedef struct {
    double _Complex *A;
    double _Complex *W;
    double _Complex *work;
    double *rwork;
    int cap_n;    /* allocated matrix dimension */
    int lwork;    /* capacity of work[] */
    int query_n;  /* dimension the cached lwork_n below was queried for */
    int lwork_n;  /* optimal lwork for query_n (what we pass to zgeev) */
} CompanionWorkspace;

static inline void companion_ws_release(CompanionWorkspace *ws) {
    free(ws->A);
    free(ws->W);
    free(ws->work);
    free(ws->rwork);
    ws->A = NULL; ws->W = NULL; ws->work = NULL; ws->rwork = NULL;
    ws->cap_n = 0; ws->lwork = 0; ws->query_n = 0; ws->lwork_n = 0;
}

static inline int companion_ws_ensure(CompanionWorkspace *ws, int n) {
    if (n > ws->cap_n) {
        companion_ws_release(ws);
        ws->A = (double _Complex *)malloc((size_t)n * (size_t)n * sizeof(double _Complex));
        ws->W = (double _Complex *)malloc((size_t)n * sizeof(double _Complex));
        ws->rwork = (double *)malloc((size_t)(2 * n) * sizeof(double));
        if (!ws->A || !ws->W || !ws->rwork) {
            companion_ws_release(ws);
            return 1;
        }
        ws->cap_n = n;
        ws->query_n = 0;  /* force a fresh lwork query at this size */
    }
    if (ws->query_n != n) {
        int info = 0, lwork = -1;
        int ln = n, ldvl = 1, ldvr = 1;
        char jobvl = 'N', jobvr = 'N';
        double _Complex wkopt;
        zgeev_(&jobvl, &jobvr, &ln, ws->A, &ln, ws->W, NULL, &ldvl, NULL, &ldvr,
               &wkopt, &lwork, ws->rwork, &info);
        if (info != 0) return 1;
        int need = (int)creal(wkopt);
        if (need < 1) return 1;
        if (need > ws->lwork) {
            free(ws->work);
            ws->work = (double _Complex *)malloc((size_t)need * sizeof(double _Complex));
            if (!ws->work) {
                ws->lwork = 0;
                return 1;
            }
            ws->lwork = need;
        }
        ws->query_n = n;
        ws->lwork_n = need;
    }
    return 0;
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
static inline int solve_companion_coeffs_ws(CompanionWorkspace *ws,
                                            const double *cfRe, const double *cfIm, int nCoeffs,
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
    if (companion_ws_ensure(ws, n) != 0) {
        for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
        return nCoeffs - 1;
    }
    double _Complex *A = ws->A;
    /* calloc equivalence: zero the matrix before scattering entries */
    memset(A, 0, (size_t)n * (size_t)n * sizeof(double _Complex));

    double _Complex lead = cfRe[first] + I * cfIm[first];
    if (!isfinite(cabs(lead))) {
        for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
        return -(nCoeffs - 1);
    }

    for (int j = 0; j < n; j++) {
        double _Complex bj = cfRe[first + 1 + j] + I * cfIm[first + 1 + j];
        A[j * n + 0] = -bj / lead;
        if (!isfinite(creal(A[j * n + 0])) || !isfinite(cimag(A[j * n + 0]))) {
            for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
            return -(nCoeffs - 1);
        }
    }
    for (int k = 1; k < n; k++) {
        A[(k - 1) * n + k] = 1.0;
    }

    int info = 0;
    int ln = n, ldvl = 1, ldvr = 1;
    int lwork = ws->lwork_n;   /* the cached per-n optimal, as the old
                                  query-every-call code would have passed */
    char jobvl = 'N', jobvr = 'N';
    zgeev_(&jobvl, &jobvr, &ln, A, &ln, ws->W, NULL, &ldvl, NULL, &ldvr,
           ws->work, &lwork, ws->rwork, &info);
    if (info != 0) {
        for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
        return nCoeffs - 1;
    }

    for (int k = 0; k < n; k++) {
        double re = creal(ws->W[k]), im = cimag(ws->W[k]);
        if (!isfinite(re) || !isfinite(im)) { re = 0.0; im = 0.0; }
        out_re[k] = (float)re;
        out_im[k] = (float)im;
    }
    for (int k = n; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }

    return nCoeffs - 1;
}

/* Back-compat entry point: a persistent PER-THREAD workspace, so every
 * existing caller (sweep_cli's roots_cm under coeffgen worker threads,
 * sweep_cm's solve workers) gets the reuse for free and stays race-free. */
static inline int solve_companion_coeffs(const double *cfRe, const double *cfIm, int nCoeffs,
                                         float *out_re, float *out_im, int strip_exact) {
    static _Thread_local CompanionWorkspace tls_ws;
    return solve_companion_coeffs_ws(&tls_ws, cfRe, cfIm, nCoeffs,
                                     out_re, out_im, strip_exact);
}

#else

static inline int companion_solver_available(void) {
    return 0;
}

typedef struct { int cap_n; } CompanionWorkspace;

static inline void companion_ws_release(CompanionWorkspace *ws) { (void)ws; }

static inline int solve_companion_coeffs_ws(CompanionWorkspace *ws,
                                            const double *cfRe, const double *cfIm, int nCoeffs,
                                            float *out_re, float *out_im, int strip_exact) {
    (void)ws;
    (void)cfRe;
    (void)cfIm;
    (void)strip_exact;
    for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
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
