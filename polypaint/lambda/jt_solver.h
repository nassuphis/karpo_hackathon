#ifndef POLYPAINT_JT_SOLVER_H
#define POLYPAINT_JT_SOLVER_H

/*
 * Jenkins-Traub CPOLY (TOMS Algorithm 419, Comm. ACM 15 (1972) 97-99),
 * ported to re-entrant C from pyroots/src/polyroot.cpp — itself Ross
 * Ihaka's 1997 C translation for R (src/appl/cpoly.c), later C++-ified.
 *
 * Ported faithfully: the shift sequence (94-degree rotations from
 * 1/sqrt(2)), the Cauchy lower bound, the power-of-2 coefficient
 * scaling, the two-pass/9-shift search, and the stage-2/3 convergence
 * tests are verbatim. Roots are stored IN THE ORDER FOUND and the
 * polynomial is DEFLATED after each — the error-compounding gradient
 * across the constellation is the point (solver-brush semantics; see
 * docs/solver-brushes.md).
 *
 * Two deliberate departures, both documented:
 *  - The original used file-scope globals and C-static locals (a
 *    Fortran-translation habit); all state lives in JtState so worker
 *    threads can solve concurrently. One static local was READ-before-
 *    assign in a rare branch (vrshft's relstp when h(s) is essentially
 *    zero on the first stage-3 iteration): R inherits a stale value
 *    from the previous call there; this port initializes it to 0.0.
 *  - Exactly-zero LEADING coefficients are stripped before the solve
 *    (the pipeline's convention; R's polyroot fails the whole call).
 *
 * JT_MAX_NN bounds the working polynomial (matches MAX_COEFFS).
 */

#include <math.h>
#include <float.h>
#include <string.h>

#define JT_MAX_NN 256

typedef struct {
    int nn;
    double sr, si, tr, ti, pvr, pvi;
    double pr[JT_MAX_NN], pi_[JT_MAX_NN];
    double hr[JT_MAX_NN], hi[JT_MAX_NN];
    double qpr[JT_MAX_NN], qpi[JT_MAX_NN];
    double qhr[JT_MAX_NN], qhi[JT_MAX_NN];
    double shr[JT_MAX_NN], shi[JT_MAX_NN];
} JtState;

static const double jt_eta = DBL_EPSILON;
static const double jt_are = DBL_EPSILON;
#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2 0.70710678118654752440
#endif
static const double jt_mre = 2.0 * M_SQRT2 * DBL_EPSILON;
static const double jt_infin = DBL_MAX;

static inline void jt_cdivid(double ar, double ai, double br, double bi,
                             double *cr, double *ci) {
    double d, r;
    if (br == 0.0 && bi == 0.0) {
        *cr = *ci = INFINITY;
    } else if (fabs(br) >= fabs(bi)) {
        r = bi / br;
        d = br + r * bi;
        *cr = (ar + ai * r) / d;
        *ci = (ai - ar * r) / d;
    } else {
        r = br / bi;
        d = bi + r * br;
        *cr = (ar * r + ai) / d;
        *ci = (ai * r - ar) / d;
    }
}

static inline void jt_polyev(int n, double s_r, double s_i,
                             const double *p_r, const double *p_i,
                             double *q_r, double *q_i,
                             double *v_r, double *v_i) {
    int i;
    double t;
    q_r[0] = p_r[0];
    q_i[0] = p_i[0];
    *v_r = q_r[0];
    *v_i = q_i[0];
    for (i = 1; i < n; i++) {
        t = *v_r * s_r - *v_i * s_i + p_r[i];
        q_i[i] = *v_i = *v_r * s_i + *v_i * s_r + p_i[i];
        q_r[i] = *v_r = t;
    }
}

static inline double jt_errev(int n, const double *qr, const double *qi,
                              double ms, double mp, double a_re, double m_re) {
    double e;
    int i;
    e = hypot(qr[0], qi[0]) * m_re / (a_re + m_re);
    for (i = 0; i < n; i++)
        e = e * ms + hypot(qr[i], qi[i]);
    return e * (a_re + m_re) - mp * m_re;
}

static inline double jt_cauchy(int n, double *pot, double *q) {
    double f, x, delf, dx, xm;
    int i, n1 = n - 1;
    pot[n1] = -pot[n1];
    x = exp((log(-pot[n1]) - log(pot[0])) / (double)n1);
    if (pot[n1 - 1] != 0.0) {
        xm = -pot[n1] / pot[n1 - 1];
        if (xm < x)
            x = xm;
    }
    for (;;) {
        xm = x * 0.1;
        f = pot[0];
        for (i = 1; i < n; i++)
            f = f * xm + pot[i];
        if (f <= 0.0)
            break;
        x = xm;
    }
    dx = x;
    while (fabs(dx / x) > 0.005) {
        q[0] = pot[0];
        for (i = 1; i < n; i++)
            q[i] = q[i - 1] * x + pot[i];
        f = q[n1];
        delf = q[0];
        for (i = 1; i < n1; i++)
            delf = delf * x + q[i];
        dx = f / delf;
        x -= dx;
    }
    return x;
}

static inline double jt_pow_di(double x, int n) {
    double p = 1.0;
    if (n != 0) {
        if (n < 0) { n = -n; x = 1 / x; }
        for (;;) {
            if (n & 01) p *= x;
            if (n >>= 1) x *= x; else break;
        }
    }
    return p;
}

static inline double jt_scale(int n, const double *pot,
                              double eps, double BIG, double small, double base) {
    int i, ell;
    double x, high, sc, lo, min_, max_;
    high = sqrt(BIG);
    lo = small / eps;
    max_ = 0.0;
    min_ = BIG;
    for (i = 0; i < n; i++) {
        x = pot[i];
        if (x > max_) max_ = x;
        if (x != 0.0 && x < min_)
            min_ = x;
    }
    if (min_ < lo || max_ > high) {
        x = lo / min_;
        if (x <= 1.0)
            sc = 1.0 / (sqrt(max_) * sqrt(min_));
        else {
            sc = x;
            if (BIG / sc > max_)
                sc = 1.0;
        }
        ell = (int)(log(sc) / log(base) + 0.5);
        return jt_pow_di(base, ell);
    }
    return 1.0;
}

static inline void jt_noshft(JtState *st, int l1) {
    int i, j, jj, n = st->nn - 1, nm1 = n - 1;
    double t1, t2, xni;
    for (i = 0; i < n; i++) {
        xni = (double)(st->nn - i - 1);
        st->hr[i] = xni * st->pr[i] / n;
        st->hi[i] = xni * st->pi_[i] / n;
    }
    for (jj = 1; jj <= l1; jj++) {
        if (hypot(st->hr[n - 1], st->hi[n - 1]) <=
            jt_eta * 10.0 * hypot(st->pr[n - 1], st->pi_[n - 1])) {
            for (i = 1; i <= nm1; i++) {
                j = st->nn - i;
                st->hr[j - 1] = st->hr[j - 2];
                st->hi[j - 1] = st->hi[j - 2];
            }
            st->hr[0] = 0.0;
            st->hi[0] = 0.0;
        } else {
            jt_cdivid(-st->pr[st->nn - 1], -st->pi_[st->nn - 1],
                      st->hr[n - 1], st->hi[n - 1], &st->tr, &st->ti);
            for (i = 1; i <= nm1; i++) {
                j = st->nn - i;
                t1 = st->hr[j - 2];
                t2 = st->hi[j - 2];
                st->hr[j - 1] = st->tr * t1 - st->ti * t2 + st->pr[j - 1];
                st->hi[j - 1] = st->tr * t2 + st->ti * t1 + st->pi_[j - 1];
            }
            st->hr[0] = st->pr[0];
            st->hi[0] = st->pi_[0];
        }
    }
}

static inline void jt_calct(JtState *st, int *bol) {
    int n = st->nn - 1;
    double hvi, hvr;
    jt_polyev(n, st->sr, st->si, st->hr, st->hi, st->qhr, st->qhi, &hvr, &hvi);
    *bol = hypot(hvr, hvi) <= jt_are * 10.0 * hypot(st->hr[n - 1], st->hi[n - 1]);
    if (!*bol) {
        jt_cdivid(-st->pvr, -st->pvi, hvr, hvi, &st->tr, &st->ti);
    } else {
        st->tr = 0.0;
        st->ti = 0.0;
    }
}

static inline void jt_nexth(JtState *st, int bol) {
    int j, n = st->nn - 1;
    double t1, t2;
    if (!bol) {
        for (j = 1; j < n; j++) {
            t1 = st->qhr[j - 1];
            t2 = st->qhi[j - 1];
            st->hr[j] = st->tr * t1 - st->ti * t2 + st->qpr[j];
            st->hi[j] = st->tr * t2 + st->ti * t1 + st->qpi[j];
        }
        st->hr[0] = st->qpr[0];
        st->hi[0] = st->qpi[0];
    } else {
        for (j = 1; j < n; j++) {
            st->hr[j] = st->qhr[j - 1];
            st->hi[j] = st->qhi[j - 1];
        }
        st->hr[0] = 0.0;
        st->hi[0] = 0.0;
    }
}

static inline int jt_vrshft(JtState *st, int l3, double *zr, double *zi) {
    int bol, b;
    int i, j;
    double r1, r2, mp, ms, tp, relstp = 0.0;
    double omp = 0.0;
    b = 0;
    st->sr = *zr;
    st->si = *zi;
    for (i = 1; i <= l3; i++) {
        jt_polyev(st->nn, st->sr, st->si, st->pr, st->pi_,
                  st->qpr, st->qpi, &st->pvr, &st->pvi);
        mp = hypot(st->pvr, st->pvi);
        ms = hypot(st->sr, st->si);
        if (mp <= 20.0 * jt_errev(st->nn, st->qpr, st->qpi, ms, mp, jt_eta, jt_mre)) {
            *zr = st->sr;
            *zi = st->si;
            return 1;
        }
        if (i != 1) {
            if (!b && mp >= omp && relstp < .05) {
                tp = relstp;
                b = 1;
                if (relstp < jt_eta)
                    tp = jt_eta;
                r1 = sqrt(tp);
                r2 = st->sr * (r1 + 1.) - st->si * r1;
                st->si = st->sr * r1 + st->si * (r1 + 1.);
                st->sr = r2;
                jt_polyev(st->nn, st->sr, st->si, st->pr, st->pi_,
                          st->qpr, st->qpi, &st->pvr, &st->pvi);
                for (j = 1; j <= 5; ++j) {
                    jt_calct(st, &bol);
                    jt_nexth(st, bol);
                }
                omp = jt_infin;
                goto L10;
            }
            if (mp * .1 > omp)
                return 0;
        }
        omp = mp;
    L10:
        jt_calct(st, &bol);
        jt_nexth(st, bol);
        jt_calct(st, &bol);
        if (!bol) {
            relstp = hypot(st->tr, st->ti) / hypot(st->sr, st->si);
            st->sr += st->tr;
            st->si += st->ti;
        }
    }
    return 0;
}

static inline int jt_fxshft(JtState *st, int l2, double *zr, double *zi) {
    int pasd, bol, test;
    double svsi, svsr;
    int i, j, n;
    double oti, otr;
    n = st->nn - 1;
    jt_polyev(st->nn, st->sr, st->si, st->pr, st->pi_,
              st->qpr, st->qpi, &st->pvr, &st->pvi);
    test = 1;
    pasd = 0;
    jt_calct(st, &bol);
    for (j = 1; j <= l2; j++) {
        otr = st->tr;
        oti = st->ti;
        jt_nexth(st, bol);
        jt_calct(st, &bol);
        *zr = st->sr + st->tr;
        *zi = st->si + st->ti;
        if (!bol && test && j != l2) {
            if (hypot(st->tr - otr, st->ti - oti) >= hypot(*zr, *zi) * 0.5) {
                pasd = 0;
            } else if (!pasd) {
                pasd = 1;
            } else {
                for (i = 0; i < n; i++) {
                    st->shr[i] = st->hr[i];
                    st->shi[i] = st->hi[i];
                }
                svsr = st->sr;
                svsi = st->si;
                if (jt_vrshft(st, 10, zr, zi)) {
                    return 1;
                }
                test = 0;
                for (i = 1; i <= n; i++) {
                    st->hr[i - 1] = st->shr[i - 1];
                    st->hi[i - 1] = st->shi[i - 1];
                }
                st->sr = svsr;
                st->si = svsi;
                jt_polyev(st->nn, st->sr, st->si, st->pr, st->pi_,
                          st->qpr, st->qpi, &st->pvr, &st->pvi);
                jt_calct(st, &bol);
            }
        }
    }
    return jt_vrshft(st, 10, zr, zi);
}

/* Core driver on descending coefficients opr/opi[0..degree] with a
 * NONZERO leading coefficient. zeror/zeroi receive `degree` roots in
 * the order found. Returns 0 on success, 1 on the (rare) two-pass
 * failure — partial zeros are whatever was stored before the failure,
 * remaining slots zeroed by the caller. */
static inline int jt_polyroot(JtState *st,
                              const double *opr, const double *opi, int degree,
                              double *zeror, double *zeroi) {
    const double smalno = DBL_MIN;
    const double base = (double)FLT_RADIX;
    int d_n, i, i1, i2;
    double zi, zr, xx, yy;
    double bnd, xxx;
    int conv;
    int d1;
    const double cosr = -0.06975647374412529990;   /* cos 94deg */
    const double sinr = 0.99756405025982424767;    /* sin 94deg */
    xx = M_SQRT1_2;
    yy = -xx;

    st->nn = degree;
    d1 = st->nn - 1;

    if (opr[0] == 0.0 && opi[0] == 0.0)
        return 1;

    /* remove zeros at the origin, exactly as the original */
    while (opr[st->nn] == 0.0 && opi[st->nn] == 0.0) {
        d_n = d1 - st->nn + 1;
        zeror[d_n] = 0.0;
        zeroi[d_n] = 0.0;
        st->nn--;
    }
    st->nn++;

    if (st->nn == 1)
        return 0;
    if (st->nn > JT_MAX_NN)
        return 1;

    for (i = 0; i < st->nn; i++) {
        st->pr[i] = opr[i];
        st->pi_[i] = opi[i];
        st->shr[i] = hypot(st->pr[i], st->pi_[i]);
    }
    bnd = jt_scale(st->nn, st->shr, jt_eta, jt_infin, smalno, base);
    if (bnd != 1.0) {
        for (i = 0; i < st->nn; i++) {
            st->pr[i] *= bnd;
            st->pi_[i] *= bnd;
        }
    }

    while (st->nn > 2) {
        for (i = 0; i < st->nn; i++)
            st->shr[i] = hypot(st->pr[i], st->pi_[i]);
        bnd = jt_cauchy(st->nn, st->shr, st->shi);

        conv = 0;
        for (i1 = 1; i1 <= 2 && !conv; i1++) {
            jt_noshft(st, 5);
            for (i2 = 1; i2 <= 9; i2++) {
                xxx = cosr * xx - sinr * yy;
                yy = sinr * xx + cosr * yy;
                xx = xxx;
                st->sr = bnd * xx;
                st->si = bnd * yy;
                if (jt_fxshft(st, i2 * 10, &zr, &zi)) {
                    conv = 1;
                    break;
                }
            }
        }
        if (!conv)
            return 1;

        d_n = d1 + 2 - st->nn;
        zeror[d_n] = zr;
        zeroi[d_n] = zi;
        --st->nn;
        for (i = 0; i < st->nn; i++) {
            st->pr[i] = st->qpr[i];
            st->pi_[i] = st->qpi[i];
        }
    }

    jt_cdivid(-st->pr[1], -st->pi_[1], st->pr[0], st->pi_[0],
              &zeror[d1], &zeroi[d1]);
    return 0;
}

/* Pipeline entry point, mirroring solve_companion_coeffs' contract:
 * cf[0..n-1] descending, f32 outputs, always nCoeffs-1 slots written.
 * Exactly-zero leading coefficients strip first (their giant-root
 * content is undefined for JT; R would fail the whole call); the
 * missing degrees pad with zeros at the tail, and total failure
 * returns negative with all-zero output (the skip counter's signal). */
static inline int solve_jt_coeffs(JtState *st,
                                  const double *cfRe, const double *cfIm, int nCoeffs,
                                  float *out_re, float *out_im) {
    int first = 0;
    while (first < nCoeffs - 1 && cfRe[first] == 0.0 && cfIm[first] == 0.0)
        first++;
    int degree = nCoeffs - 1 - first;
    for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
    if (degree <= 0 || degree + 1 > JT_MAX_NN)
        return nCoeffs - 1;
    /* overflow-row skip, mirroring solve_companion_coeffs: non-finite
     * coefficients would spin CPOLY's scaling/bound loops on NaN */
    for (int k = first; k < nCoeffs; k++) {
        if (!isfinite(cfRe[k]) || !isfinite(cfIm[k]))
            return -(nCoeffs - 1);
    }

    double zr[JT_MAX_NN], zi[JT_MAX_NN];
    memset(zr, 0, sizeof(zr));
    memset(zi, 0, sizeof(zi));
    int fail = jt_polyroot(st, cfRe + first, cfIm + first, degree, zr, zi);
    if (fail)
        return -(nCoeffs - 1);
    for (int k = 0; k < degree; k++) {
        double re = zr[k], im = zi[k];
        if (!isfinite(re) || !isfinite(im)) { re = 0.0; im = 0.0; }
        out_re[k] = (float)re;
        out_im[k] = (float)im;
    }
    return nCoeffs - 1;
}

/* Full-width twin of solve_jt_coeffs for the fused JT64 pipeline: the
 * same contract with f64 outputs, so the finished double-precision
 * roots reach the caller without the f32 exit cast (the 5.3e-8 floor
 * of the float interface). The float entry point above stays untouched
 * so every existing paint path remains byte-identical; twin drift is
 * pinned by tests/test_fused_solvers_jt64_cm64.py. */
static inline int solve_jt_coeffs_f64(JtState *st,
                                      const double *cfRe, const double *cfIm, int nCoeffs,
                                      double *out_re, double *out_im) {
    int first = 0;
    while (first < nCoeffs - 1 && cfRe[first] == 0.0 && cfIm[first] == 0.0)
        first++;
    int degree = nCoeffs - 1 - first;
    for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0; out_im[k] = 0.0; }
    if (degree <= 0 || degree + 1 > JT_MAX_NN)
        return nCoeffs - 1;
    for (int k = first; k < nCoeffs; k++) {
        if (!isfinite(cfRe[k]) || !isfinite(cfIm[k]))
            return -(nCoeffs - 1);
    }

    double zr[JT_MAX_NN], zi[JT_MAX_NN];
    memset(zr, 0, sizeof(zr));
    memset(zi, 0, sizeof(zi));
    int fail = jt_polyroot(st, cfRe + first, cfIm + first, degree, zr, zi);
    if (fail)
        return -(nCoeffs - 1);
    for (int k = 0; k < degree; k++) {
        double re = zr[k], im = zi[k];
        if (!isfinite(re) || !isfinite(im)) { re = 0.0; im = 0.0; }
        out_re[k] = re;
        out_im[k] = im;
    }
    return nCoeffs - 1;
}

#endif
