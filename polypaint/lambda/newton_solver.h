#ifndef POLYPAINT_NEWTON_SOLVER_H
#define POLYPAINT_NEWTON_SOLVER_H

/*
 * Newton-Raphson + forward deflation: the naive classical rootfinder,
 * kept deliberately naive — its failure modes are the brush (see
 * docs/solver-brushes.md).
 *
 * For each root: Newton iterates z <- z - p(z)/p'(z) from the FIXED
 * seed 0.4 + 0.9i (every root, every polynomial — which basin the seed
 * falls in is a fractal function of the coefficients, and those basin
 * boundaries paint). Convergence is |step| <= 1e-14 * |z| (or an exact
 * zero of p); after max_steps (the solver_iters brush knob, default
 * and ceiling NEWTON_MAX_STEPS) the CURRENT iterate is accepted
 * as-is — non-convergence is not an error here, it is texture. The
 * found root then deflates the polynomial by synthetic division
 * (forward Horner), compounding its error into every later root.
 *
 * Determinism: fixed seed, fixed iteration cap, fixed tolerances, a
 * fixed nudge (z += 0.5) when p'(z) lands exactly on 0, and roots
 * emitted in the order found.
 */

#include <math.h>
#include <string.h>

#define NEWTON_MAX_NN 256
#define NEWTON_MAX_STEPS 50

static inline void newton_cdiv(double ar, double ai, double br, double bi,
                               double *cr, double *ci) {
    double d, r;
    if (br == 0.0 && bi == 0.0) {
        *cr = *ci = INFINITY;
        return;
    }
    if (fabs(br) >= fabs(bi)) {
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

/* Horner: value and derivative of the descending-coefficient poly at z. */
static inline void newton_eval(const double *cr, const double *ci, int n,
                               double zr, double zi,
                               double *pr, double *pi_,
                               double *dr, double *di) {
    double vr = cr[0], vi = ci[0];
    double wr = 0.0, wi = 0.0;
    for (int k = 1; k < n; k++) {
        double t = wr * zr - wi * zi + vr;
        wi = wr * zi + wi * zr + vi;
        wr = t;
        t = vr * zr - vi * zi + cr[k];
        vi = vr * zi + vi * zr + ci[k];
        vr = t;
    }
    *pr = vr;
    *pi_ = vi;
    *dr = wr;
    *di = wi;
}

/* Pipeline entry point, mirroring solve_companion_coeffs' contract:
 * cf[0..n-1] descending, f32 outputs, always nCoeffs-1 slots written.
 * Exactly-zero leading coefficients strip first; missing degrees pad
 * with zeros at the tail. Non-finite coefficient rows skip (zeros,
 * negative return -> skipped_overflow). Otherwise never "fails":
 * whatever Newton produced is the answer. */
static inline int solve_newton_coeffs(const double *cfRe, const double *cfIm, int nCoeffs,
                                      float *out_re, float *out_im, int max_steps) {
    if (max_steps < 1 || max_steps > NEWTON_MAX_STEPS) max_steps = NEWTON_MAX_STEPS;
    int first = 0;
    while (first < nCoeffs - 1 && cfRe[first] == 0.0 && cfIm[first] == 0.0)
        first++;
    int degree = nCoeffs - 1 - first;
    for (int k = 0; k < nCoeffs - 1; k++) { out_re[k] = 0.0f; out_im[k] = 0.0f; }
    if (degree <= 0 || degree + 1 > NEWTON_MAX_NN)
        return nCoeffs - 1;
    /* overflow-row skip, mirroring solve_companion_coeffs: non-finite
     * coefficients turn every iterate into seed-restart churn */
    for (int k = first; k < nCoeffs; k++) {
        if (!isfinite(cfRe[k]) || !isfinite(cfIm[k]))
            return -(nCoeffs - 1);
    }

    double wr_[NEWTON_MAX_NN], wi_[NEWTON_MAX_NN];
    for (int k = 0; k <= degree; k++) {
        wr_[k] = cfRe[first + k];
        wi_[k] = cfIm[first + k];
    }

    int found = 0;
    int n = degree + 1;   /* current coefficient count */
    while (n > 2) {
        double zr = 0.4, zi = 0.9;
        for (int it = 0; it < max_steps; it++) {
            double pr, pi_, dr, di;
            newton_eval(wr_, wi_, n, zr, zi, &pr, &pi_, &dr, &di);
            if (pr == 0.0 && pi_ == 0.0)
                break;
            if (dr == 0.0 && di == 0.0) {
                zr += 0.5;   /* deterministic nudge off the critical point */
                continue;
            }
            double sr_, si_;
            newton_cdiv(pr, pi_, dr, di, &sr_, &si_);
            zr -= sr_;
            zi -= si_;
            if (!isfinite(zr) || !isfinite(zi)) {
                zr = 0.4; zi = 0.9;   /* diverged: restart once from the seed */
                continue;
            }
            if (hypot(sr_, si_) <= 1e-14 * hypot(zr, zi))
                break;
        }
        if (!isfinite(zr) || !isfinite(zi)) { zr = 0.0; zi = 0.0; }
        {
            /* guard the f32 cast too: a capped 1-step budget can accept a
             * double-finite iterate beyond f32 range (caught by the
             * max_iter=1 brush test) */
            float fr = (float)zr, fi = (float)zi;
            if (!isfinite(fr) || !isfinite(fi)) { fr = 0.0f; fi = 0.0f; }
            out_re[found] = fr;
            out_im[found] = fi;
        }
        found++;
        /* forward deflation by (z - root): synthetic division */
        double br = wr_[0], bi = wi_[0];
        for (int k = 1; k < n - 1; k++) {
            double t = br * zr - bi * zi + wr_[k];
            bi = br * zi + bi * zr + wi_[k];
            br = t;
            wr_[k] = br;
            wi_[k] = bi;
        }
        n--;
    }
    if (n == 2) {
        double zr, zi;
        newton_cdiv(-wr_[1], -wi_[1], wr_[0], wi_[0], &zr, &zi);
        if (!isfinite(zr) || !isfinite(zi)) { zr = 0.0; zi = 0.0; }
        {
            float fr = (float)zr, fi = (float)zi;
            if (!isfinite(fr) || !isfinite(fi)) { fr = 0.0f; fi = 0.0f; }
            out_re[found] = fr;
            out_im[found] = fi;
        }
        found++;
    }
    return nCoeffs - 1;
}

#endif
