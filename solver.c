/*
 * Ehrlich-Aberth polynomial root solver for WASM.
 * No stdlib, no malloc, no math.h — pure f64 arithmetic.
 * Compiled with: clang --target=wasm32-unknown-unknown -O3 -nostdlib
 *
 * All pointer arguments are byte offsets into WASM linear memory.
 * The caller (JS) sets up Float64Array/Uint8Array views at those offsets.
 */

#define MAX_ITER 64
#define TOL2 1e-16

/* WASM exports this function */
__attribute__((export_name("solveEA")))
void solveEA(double *cRe, double *cIm, int nCoeffs,
             double *warmRe, double *warmIm, int nRoots,
             int trackIter, unsigned char *iterCounts)
{
    /* Strip leading near-zero coefficients */
    int start = 0;
    while (start < nCoeffs - 1 &&
           cRe[start] * cRe[start] + cIm[start] * cIm[start] < 1e-30)
        start++;

    int degree = nCoeffs - 1 - start;
    if (degree <= 0)
        return;

    /* Degree 1: linear case  a*z + b = 0  =>  z = -b/a */
    if (degree == 1) {
        double aR = cRe[start], aI = cIm[start];
        double bR = cRe[start + 1], bI = cIm[start + 1];
        double d = aR * aR + aI * aI;
        if (d < 1e-30)
            return;
        warmRe[0] = -(bR * aR + bI * aI) / d;
        warmIm[0] = -(bI * aR - bR * aI) / d;
        if (trackIter && iterCounts)
            iterCounts[0] = 1;
        return;
    }

    /* Copy stripped coefficients to stack arrays */
    int n = nCoeffs - start;
    double cr[256], ci[256];
    for (int k = 0; k < n; k++) {
        cr[k] = cRe[start + k];
        ci[k] = cIm[start + k];
    }

    /* Copy warm-start roots to local arrays */
    double rRe[255], rIm[255];
    for (int i = 0; i < degree; i++) {
        rRe[i] = warmRe[i];
        rIm[i] = warmIm[i];
    }

    /* Convergence tracking */
    unsigned char conv[255];
    if (trackIter) {
        for (int i = 0; i < degree; i++)
            conv[i] = 0;
    }

    /* Main Ehrlich-Aberth iteration */
    for (int iter = 0; iter < MAX_ITER; iter++) {
        double maxCorr2 = 0;

        for (int i = 0; i < degree; i++) {
            if (trackIter && conv[i])
                continue;

            double zR = rRe[i], zI = rIm[i];

            /* Horner: evaluate p(z) and p'(z) simultaneously */
            double pR = cr[0], pI = ci[0];
            double dpR = 0, dpI = 0;
            for (int k = 1; k < n; k++) {
                /* dp = dp * z + p */
                double ndR = dpR * zR - dpI * zI + pR;
                double ndI = dpR * zI + dpI * zR + pI;
                dpR = ndR;
                dpI = ndI;
                /* p = p * z + c[k] */
                double npR = pR * zR - pI * zI + cr[k];
                double npI = pR * zI + pI * zR + ci[k];
                pR = npR;
                pI = npI;
            }

            /* Newton step: w = p(z) / p'(z) */
            double dpM = dpR * dpR + dpI * dpI;
            if (dpM < 1e-60)
                continue;
            double wR = (pR * dpR + pI * dpI) / dpM;
            double wI = (pI * dpR - pR * dpI) / dpM;

            /* Aberth sum: S = sum_{j!=i} 1/(z_i - z_j) */
            double sR = 0, sI = 0;
            for (int j = 0; j < degree; j++) {
                if (j == i)
                    continue;
                double dR = zR - rRe[j];
                double dI = zI - rIm[j];
                double dM = dR * dR + dI * dI;
                if (dM < 1e-60)
                    continue;
                sR += dR / dM;
                sI += -dI / dM;
            }

            /* Correction: z -= w / (1 - w * S) */
            double wsR = wR * sR - wI * sI;
            double wsI = wR * sI + wI * sR;
            double dnR = 1 - wsR;
            double dnI = -wsI;
            double dnM = dnR * dnR + dnI * dnI;
            if (dnM < 1e-60)
                continue;

            double crrR = (wR * dnR + wI * dnI) / dnM;
            double crrI = (wI * dnR - wR * dnI) / dnM;

            rRe[i] -= crrR;
            rIm[i] -= crrI;

            double h2 = crrR * crrR + crrI * crrI;
            if (h2 > maxCorr2)
                maxCorr2 = h2;

            if (trackIter && h2 < TOL2) {
                conv[i] = 1;
                if (iterCounts)
                    iterCounts[i] = (unsigned char)(iter + 1);
            }
        }

        if (maxCorr2 < TOL2) {
            if (trackIter && iterCounts) {
                for (int i = 0; i < degree; i++)
                    if (!conv[i]) {
                        conv[i] = 1;
                        iterCounts[i] = (unsigned char)(iter + 1);
                    }
            }
            break;
        }
    }

    /* Final: fill in unconverged iteration counts */
    if (trackIter && iterCounts) {
        for (int i = 0; i < degree; i++)
            if (!conv[i])
                iterCounts[i] = MAX_ITER;
    }

    /* Write results back to warm-start buffers (only finite values) */
    for (int i = 0; i < degree; i++) {
        /* NaN check: x != x is true for NaN (IEEE 754) */
        if (rRe[i] == rRe[i] && rIm[i] == rIm[i]) {
            warmRe[i] = rRe[i];
            warmIm[i] = rIm[i];
        }
        /* If NaN, leave warm-start unchanged — JS handles rescue */
    }
}
