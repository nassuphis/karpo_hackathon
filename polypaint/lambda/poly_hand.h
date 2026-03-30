/*
 * Hand-written polynomial coefficient functions.
 *
 * These replace broken auto-transpiled versions from transpile_poly.py.
 * The transpiler silently emits zeros for:
 *   - np.array([...]) with computed elements in slice assignments
 *   - List comprehensions in slice assignments: cf[a:b] = [expr for ...]
 *
 * Each function has the same signature as the transpiled versions:
 *   void func(double x1r, double x1i, double x2r, double x2i,
 *             double *cRe, double *cIm, int *nCoeffs)
 *
 * Requires: c_mul, c_abs, c_arg, c_exp2 helpers from sweep_cli.c
 *
 * Forward declaration: poly_42_c from poly_generated_funcs.h (used by serendipity wrapper)
 */
static void poly_42_c(double x1r, double x1i, double x2r, double x2i,
                      const double *cfpv, int n_cfpv,
                      double *cRe, double *cIm, int *nCoeffs);

/* ---- poly_29_hand ----
 * Python:
 *   cf[0:5]  = [1, t1, t1^2, t1^3, t1^4]
 *   cf[5:10] = [1, t2, t2^2, t2^3, t2^4]
 *   cf[10:15]= [1, exp(i*t1), exp(2i*t1), exp(3i*t1), exp(4i*t1)]
 *   cf[15:20]= [1, exp(i*t2), exp(2i*t2), exp(3i*t2), exp(4i*t2)]
 *   cf[20:30]= [1, Re(t1+t2), Im(t1+t2), Re(t1*t2), Im(t1*t2),
 *               Re(t1+t2)^2, Im(t1+t2)^2, Re(t1*t2)^2, Im(t1*t2)^2, |t1+t2|]
 *   cf[30:40]= arange(1,11) * |t1| * |t2|
 *   cf[40:50]= [1, log(|t1|+1), log(|t2|+1), log(|t1+t2|+1), log(|t1*t2|+1),
 *               angle(t1), angle(t2), |t1|, |t2|, angle(t1+t2)]
 *   cf[50]   = |t1+t2| * angle(t1*t2)
 */
static void poly_29_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* cf[0:5] = [1, t1, t1^2, t1^3, t1^4] */
    cRe[0] = 1; cIm[0] = 0;
    cRe[1] = x1r; cIm[1] = x1i;
    double t1_2r, t1_2i; c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
    cRe[2] = t1_2r; cIm[2] = t1_2i;
    double t1_3r, t1_3i; c_mul(t1_2r, t1_2i, x1r, x1i, &t1_3r, &t1_3i);
    cRe[3] = t1_3r; cIm[3] = t1_3i;
    double t1_4r, t1_4i; c_mul(t1_3r, t1_3i, x1r, x1i, &t1_4r, &t1_4i);
    cRe[4] = t1_4r; cIm[4] = t1_4i;

    /* cf[5:10] = [1, t2, t2^2, t2^3, t2^4] */
    cRe[5] = 1; cIm[5] = 0;
    cRe[6] = x2r; cIm[6] = x2i;
    double t2_2r, t2_2i; c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
    cRe[7] = t2_2r; cIm[7] = t2_2i;
    double t2_3r, t2_3i; c_mul(t2_2r, t2_2i, x2r, x2i, &t2_3r, &t2_3i);
    cRe[8] = t2_3r; cIm[8] = t2_3i;
    double t2_4r, t2_4i; c_mul(t2_3r, t2_3i, x2r, x2i, &t2_4r, &t2_4i);
    cRe[9] = t2_4r; cIm[9] = t2_4i;

    /* cf[10:15] = [1, exp(i*t1), exp(2i*t1), exp(3i*t1), exp(4i*t1)] */
    cRe[10] = 1; cIm[10] = 0;
    for (int k = 1; k <= 4; k++) {
        double er, ei;
        c_exp2(-k * x1i, k * x1r, &er, &ei);
        cRe[10 + k] = er; cIm[10 + k] = ei;
    }

    /* cf[15:20] = [1, exp(i*t2), exp(2i*t2), exp(3i*t2), exp(4i*t2)] */
    cRe[15] = 1; cIm[15] = 0;
    for (int k = 1; k <= 4; k++) {
        double er, ei;
        c_exp2(-k * x2i, k * x2r, &er, &ei);
        cRe[15 + k] = er; cIm[15 + k] = ei;
    }

    /* cf[20:30] */
    double sumR = x1r + x2r, sumI = x1i + x2i;
    double mulR, mulI; c_mul(x1r, x1i, x2r, x2i, &mulR, &mulI);
    cRe[20] = 1;            cIm[20] = 0;
    cRe[21] = sumR;         cIm[21] = 0;
    cRe[22] = sumI;         cIm[22] = 0;
    cRe[23] = mulR;         cIm[23] = 0;
    cRe[24] = mulI;         cIm[24] = 0;
    cRe[25] = sumR * sumR;  cIm[25] = 0;
    cRe[26] = sumI * sumI;  cIm[26] = 0;
    cRe[27] = mulR * mulR;  cIm[27] = 0;
    cRe[28] = mulI * mulI;  cIm[28] = 0;
    cRe[29] = c_abs(sumR, sumI); cIm[29] = 0;

    /* cf[30:40] = arange(1,11) * |t1| * |t2| */
    double absT1 = c_abs(x1r, x1i);
    double absT2 = c_abs(x2r, x2i);
    double ab12 = absT1 * absT2;
    for (int k = 0; k < 10; k++) {
        cRe[30 + k] = (k + 1) * ab12;
        cIm[30 + k] = 0;
    }

    /* cf[40:50] */
    double absSum = c_abs(sumR, sumI);
    double absMul = c_abs(mulR, mulI);
    cRe[40] = 1;                         cIm[40] = 0;
    cRe[41] = log(absT1 + 1);            cIm[41] = 0;
    cRe[42] = log(absT2 + 1);            cIm[42] = 0;
    cRe[43] = log(absSum + 1);           cIm[43] = 0;
    cRe[44] = log(absMul + 1);           cIm[44] = 0;
    cRe[45] = c_arg(x1r, x1i);           cIm[45] = 0;
    cRe[46] = c_arg(x2r, x2i);           cIm[46] = 0;
    cRe[47] = absT1;                      cIm[47] = 0;
    cRe[48] = absT2;                      cIm[48] = 0;
    cRe[49] = c_arg(sumR, sumI);          cIm[49] = 0;

    /* cf[50] = |t1+t2| * angle(t1*t2) */
    cRe[50] = absSum * c_arg(mulR, mulI); cIm[50] = 0;

    /* NaN guard */
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_giga_62_hand_c ----
 * Python:
 *   cf = np.zeros(25, dtype=complex)
 *   cf[0:5] = np.array([abs(t1 + t2)**(i+1) for i in range(5)])
 *   cf[5:10] = ((t1+2j*t2)**3).real * np.log(np.abs(np.conj(t1*t2)))
 *   cf[10:15] = ((t1-t2)**2).imag / np.angle(t1*t2)
 *   cf[15:20] = np.abs(cf[5:10])**0.5 + np.angle(cf[0:5])
 *   cf[20:25] = np.array([abs(t1 * t2)**(i+1) for i in range(5)])
 *   return cf.astype(np.complex128)
 *
 * Notes:
 *   - cf[5:10] and cf[10:15] are scalar-to-slice broadcasts in Python.
 *   - cf[0:5] and cf[20:25] come from list comprehensions in slice assignments.
 *   - np.angle(cf[0:5]) is always 0 here because cf[0:5] are nonnegative reals.
 */
static void poly_giga_62_hand_c(double x1r, double x1i, double x2r, double x2i,
                                const double *cfpv, int n_cfpv,
                                double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int i = 0; i < 25; i++) { cRe[i] = 0; cIm[i] = 0; }

    double sumr = x1r + x2r;
    double sumi = x1i + x2i;
    double abs_sum = c_abs(sumr, sumi);
    for (int i = 0; i < 5; i++) {
        cRe[i] = pow(abs_sum, (double)(i + 1));
    }

    /* z = t1 + 2j*t2 = (x1r - 2*x2i) + i(x1i + 2*x2r) */
    double zr = x1r - 2.0 * x2i;
    double zi = x1i + 2.0 * x2r;
    double z2r, z2i, z3r, z3i;
    c_mul(zr, zi, zr, zi, &z2r, &z2i);
    c_mul(z2r, z2i, zr, zi, &z3r, &z3i);

    double prodr, prodi;
    c_mul(x1r, x1i, x2r, x2i, &prodr, &prodi);
    double abs_prod = c_abs(prodr, prodi);
    double log_abs_prod = log(abs_prod);
    double broadcast_5_10 = z3r * log_abs_prod;
    for (int i = 5; i < 10; i++) {
        cRe[i] = broadcast_5_10;
    }

    double diffr = x1r - x2r;
    double diffi = x1i - x2i;
    double diff2r, diff2i;
    c_mul(diffr, diffi, diffr, diffi, &diff2r, &diff2i);
    double denom = c_arg(prodr, prodi);
    double broadcast_10_15 = diff2i / denom;
    for (int i = 10; i < 15; i++) {
        cRe[i] = broadcast_10_15;
    }

    double broadcast_15_20 = sqrt(fabs(broadcast_5_10));
    for (int i = 15; i < 20; i++) {
        cRe[i] = broadcast_15_20;
    }

    for (int i = 0; i < 5; i++) {
        cRe[20 + i] = pow(abs_prod, (double)(i + 1));
    }

    for (int i = 0; i < 25; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_33_hand ----
 * Python:
 *   f = lambda z, n: z**n - 1
 *   cf[0:35]  = [Re(f(t1,n)) - Im(f(t2,n)) for n in range(1,36)]
 *   cf[35:70] = [log(|f(t2,n)|) + angle(f(t1,n)) + sin(|f(t1,n)|) + cos(angle(f(t2,n))) for n in range(1,36)]
 *   cf[70]    = prod(cf[0:70])
 *   NaN/inf → 0
 */
static void poly_33_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Precompute powers: t1^n and t2^n iteratively */
    /* f(z, n) = z^n - 1  →  fRe = powRe - 1, fIm = powIm */
    double pow1r = 1, pow1i = 0;  /* t1^0 */
    double pow2r = 1, pow2i = 0;  /* t2^0 */

    for (int n = 1; n <= 35; n++) {
        /* pow1 *= t1 */
        double nr, ni;
        c_mul(pow1r, pow1i, x1r, x1i, &nr, &ni);
        pow1r = nr; pow1i = ni;
        /* pow2 *= t2 */
        c_mul(pow2r, pow2i, x2r, x2i, &nr, &ni);
        pow2r = nr; pow2i = ni;

        /* f(t1, n) = t1^n - 1 */
        double f1r = pow1r - 1, f1i = pow1i;
        /* f(t2, n) = t2^n - 1 */
        double f2r = pow2r - 1, f2i = pow2i;

        /* cf[n-1] = Re(f(t1,n)) - Im(f(t2,n)) */
        cRe[n - 1] = f1r - f2i;
        cIm[n - 1] = 0;

        /* cf[34+n] = log(|f(t2,n)|) + angle(f(t1,n)) + sin(|f(t1,n)|) + cos(angle(f(t2,n))) */
        double abs_f2 = c_abs(f2r, f2i);
        double ang_f1 = c_arg(f1r, f1i);
        double abs_f1 = c_abs(f1r, f1i);
        double ang_f2 = c_arg(f2r, f2i);
        cRe[34 + n] = log(abs_f2) + ang_f1 + sin(abs_f1) + cos(ang_f2);
        cIm[34 + n] = 0;
    }

    /* cf[70] = prod(cf[0:70]) — real product since all coefficients are real */
    double prodR = cRe[0], prodI = 0;
    for (int i = 1; i < 70; i++) {
        /* complex multiply: (prodR + i*prodI) * (cRe[i] + i*cIm[i]) */
        double pr, pi;
        c_mul(prodR, prodI, cRe[i], cIm[i], &pr, &pi);
        prodR = pr; prodI = pi;
    }
    cRe[70] = prodR; cIm[70] = prodI;

    /* NaN guard */
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_55_hand ----
 * Python:
 *   cf[0]     = t1 * t2
 *   cf[1]     = Re(t1) * Im(t2)
 *   cf[2]     = Re(t2) * Im(t1)
 *   cf[3:10]  = linspace(log(|cf[0]|+1), log(|cf[2]|+1), 7)
 *   cf[10:30] = [cos(cf[i-1]) + (t1+t2)^i / (i+1) for i in range(11,31)]
 *   cf[30:50] = [sin(cf[i-1]) + (t1-t2)^i / (i+1) for i in range(31,51)]
 *   cf[50:70] = |cf[0:20]| + |cf[20:40] + t1 + t2|
 *   cf[70]    = prod(cf[0:70])
 */
static void poly_55_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* cf[0] = t1 * t2 */
    double c0r, c0i;
    c_mul(x1r, x1i, x2r, x2i, &c0r, &c0i);
    cRe[0] = c0r; cIm[0] = c0i;

    /* cf[1] = Re(t1) * Im(t2) — real */
    cRe[1] = x1r * x2i; cIm[1] = 0;

    /* cf[2] = Re(t2) * Im(t1) — real */
    cRe[2] = x2r * x1i; cIm[2] = 0;

    /* cf[3:10] = linspace(log(|cf[0]|+1), log(|cf[2]|+1), 7) — real */
    double start_val = log(c_abs(cRe[0], cIm[0]) + 1);
    double end_val = log(c_abs(cRe[2], cIm[2]) + 1);
    for (int k = 0; k < 7; k++) {
        double t = (7 > 1) ? (double)k / 6.0 : 0.0;
        cRe[3 + k] = start_val + t * (end_val - start_val);
        cIm[3 + k] = 0;
    }

    /* Precompute sum = t1+t2, diff = t1-t2 */
    double sR = x1r + x2r, sI = x1i + x2i;
    double dR = x1r - x2r, dI = x1i - x2i;

    /* cf[10:30] = [cos(cf[i-1]) + (t1+t2)^i / (i+1) for i in range(11,31)]
     * Here i ranges 11..30, so cf[i-1] = cf[10..29] (the one just computed before).
     * But cf[10..29] ARE the values being computed! The Python list comprehension
     * uses cf[i-1] which references previously-set values in the SAME array.
     * For i=11: cf[10] = cos(cf[10]) + ... but cf[10] was just set above...
     * Actually cf[10] was 0 (from zeros init), so cf[10-1] = cf[10] = 0 at that point.
     * Wait — cf[i-1] for i=11 is cf[10]. But cf[10] hasn't been assigned yet in
     * the list comprehension (it's building a list). So it reads cf[10] = 0.
     * Actually no — in Python, cf[10:30] = [expr for i in range(11,31)]
     * The list is fully built BEFORE assignment. So cf[i-1] reads from the
     * ORIGINAL array, which has cf[10..29] = 0 at that point.
     * So cf[i-1] for i=11..30 reads cf[10..29] which are all 0.
     */
    /* (t1+t2)^i for i=11..30: build incrementally */
    double spowR = 1, spowI = 0;
    for (int i = 1; i <= 30; i++) {
        double nr, ni;
        c_mul(spowR, spowI, sR, sI, &nr, &ni);
        spowR = nr; spowI = ni;
        if (i >= 11) {
            /* cos(cf[i-1]) — cf[i-1] is 0 for i=11..30, so cos(0) = 1 for real part */
            /* But cf[i-1] is complex. cos(complex) = cos(re)*cosh(im) + i*(-sin(re)*sinh(im)) */
            /* Since cf[i-1] = 0+0i, cos(0) = 1+0i */
            double prev_r = 0, prev_i = 0; /* cf[i-1] was 0 before this assignment */
            double cos_r = cos(prev_r) * cosh(prev_i);
            double cos_i = -sin(prev_r) * sinh(prev_i);
            /* (t1+t2)^i / (i+1) */
            double div = 1.0 / (i + 1);
            cRe[i - 1] = cos_r + spowR * div;
            cIm[i - 1] = cos_i + spowI * div;
        }
    }

    /* cf[30:50] = [sin(cf[i-1]) + (t1-t2)^i / (i+1) for i in range(31,51)]
     * Same logic: list is built before assignment. cf[30..49] are 0 at read time.
     * EXCEPT cf[30] was possibly set... no, cf[30:50] replaces cf[30..49].
     * Actually cf[i-1] for i=31 is cf[30], which WAS set by the cf[10:30] assignment.
     * Wait: cf[10:30] sets cf[10]..cf[29]. For i=31: cf[30] was still 0.
     * For i=32: cf[31] was still 0. All cf[30..49] are 0 at list-build time.
     */
    double dpowR = 1, dpowI = 0;
    for (int i = 1; i <= 50; i++) {
        double nr, ni;
        c_mul(dpowR, dpowI, dR, dI, &nr, &ni);
        dpowR = nr; dpowI = ni;
        if (i >= 31) {
            /* sin(cf[i-1]) where cf[i-1] = 0+0i → sin(0) = 0+0i */
            double prev_r = 0, prev_i = 0;
            double sin_r = sin(prev_r) * cosh(prev_i);
            double sin_i = cos(prev_r) * sinh(prev_i);
            double div = 1.0 / (i + 1);
            cRe[i - 1] = sin_r + dpowR * div;
            cIm[i - 1] = sin_i + dpowI * div;
        }
    }

    /* cf[50:70] = |cf[0:20]| + |cf[20:40] + t1 + t2| */
    for (int k = 0; k < 20; k++) {
        double a = c_abs(cRe[k], cIm[k]);
        double bR = cRe[20 + k] + sR;
        double bI = cIm[20 + k] + sI;
        double b = c_abs(bR, bI);
        cRe[50 + k] = a + b;
        cIm[50 + k] = 0;
    }

    /* cf[70] = prod(cf[0:70]) */
    double prodR = cRe[0], prodI = cIm[0];
    for (int i = 1; i < 70; i++) {
        double pr, pi;
        c_mul(prodR, prodI, cRe[i], cIm[i], &pr, &pi);
        prodR = pr; prodI = pi;
    }
    cRe[70] = prodR; cIm[70] = prodI;

    /* NaN guard */
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_58_hand ----
 * Python:
 *   cf[0] = 100*|t1|*|t2| - 100
 *   cf[1] = 100*t1^3 - t2^3 + 100
 *   cf[2] = 100*t2^3 - t1^3 + 100
 *   cf[3:71] = [cos(k*t1)*sin(k*t2)/log(|k+1|) for k in range(1,69)]
 *   root_coeff = |t1|*|t2| * prod(1..70) / sum(1..70)  (= |t1|*|t2| * 70! / 2485)
 *   cf[4] = root_coeff * sum(cos(k*t1)*sin(k*t2)/log(|k+1|) for k=1..70)
 *   cf[35] = root_coeff * t1^2
 *   cf[34] = root_coeff * t2^2
 *   cf[36:71] = root_coeff * [cos(2k*t1)*sin(2k*t2)/log(|2k+1|) for k=1..35]
 */
static void poly_58_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double absT1 = c_abs(x1r, x1i);
    double absT2 = c_abs(x2r, x2i);

    /* cf[0] = 100*|t1|*|t2| - 100 — real */
    cRe[0] = 100.0 * absT1 * absT2 - 100.0;

    /* t1^3, t2^3 */
    double t1_2r, t1_2i; c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
    double t1_3r, t1_3i; c_mul(t1_2r, t1_2i, x1r, x1i, &t1_3r, &t1_3i);
    double t2_2r, t2_2i; c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
    double t2_3r, t2_3i; c_mul(t2_2r, t2_2i, x2r, x2i, &t2_3r, &t2_3i);

    /* cf[1] = 100*t1^3 - t2^3 + 100 */
    cRe[1] = 100.0 * t1_3r - t2_3r + 100.0;
    cIm[1] = 100.0 * t1_3i - t2_3i;

    /* cf[2] = 100*t2^3 - t1^3 + 100 */
    cRe[2] = 100.0 * t2_3r - t1_3r + 100.0;
    cIm[2] = 100.0 * t2_3i - t1_3i;

    /* cf[3:71] = [cos(k*t1)*sin(k*t2)/log(|k+1|) for k in range(1,69)]
     * cos(k*t1) is complex cos: cos(a+bi) = cos(a)cosh(b) - i*sin(a)sinh(b)
     * sin(k*t2) is complex sin: sin(a+bi) = sin(a)cosh(b) + i*cos(a)sinh(b)
     */
    for (int k = 1; k <= 68; k++) {
        double kt1r = k * x1r, kt1i = k * x1i;
        double kt2r = k * x2r, kt2i = k * x2i;
        /* cos(kt1) */
        double cosr = cos(kt1r) * cosh(kt1i);
        double cosi = -sin(kt1r) * sinh(kt1i);
        /* sin(kt2) */
        double sinr = sin(kt2r) * cosh(kt2i);
        double sini = cos(kt2r) * sinh(kt2i);
        /* cos * sin */
        double pr, pi;
        c_mul(cosr, cosi, sinr, sini, &pr, &pi);
        double logk = log((double)(k + 1));
        cRe[3 + k - 1] = pr / logk;
        cIm[3 + k - 1] = pi / logk;
    }

    /* root_coeff = |t1|*|t2| * prod(1..70) / sum(1..70)
     * prod(1..70) = 70! which overflows double. Python returns inf.
     * sum(1..70) = 2485
     * So root_coeff = |t1|*|t2| * inf / 2485 = inf (or NaN).
     * After NaN guard in Python, this makes cf[4], cf[34], cf[35], cf[36:71]
     * all get set but the root_coeff*... values will be inf → caught by NaN guard → 0.
     * Wait, Python catches this in try/except → returns zeros.
     * Actually let's compute it: 70! overflows double to inf.
     * |t1|*|t2| * inf = inf (if non-zero) or nan (if zero).
     * So any term with root_coeff will be inf/nan → zeroed by NaN guard.
     * But the cf[3:71] assignment happened BEFORE root_coeff overwrites.
     * So the final state is: cf[3:71] from the first loop, but cf[4], cf[34], cf[35]
     * overwritten to 0 (via inf→NaN guard), and cf[36:71] overwritten to 0.
     *
     * Actually: Python evaluates sequentially:
     * 1. cf[3:71] = [...] — sets cf[3]..cf[70]
     * 2. root_coeff = inf
     * 3. cf[4] = inf * sum(...) = inf → caught by except → return zeros
     *
     * Hmm, but the except catches ALL exceptions. np.prod(range(1,71)) doesn't
     * raise, it returns inf. inf * sum() = inf. No exception.
     * So cf[4] = inf. Then cf[35] = inf. cf[34] = inf. cf[36:71] = inf*[...].
     * return cf.astype(...) — no exception.
     * But they have NaN guard? No, the Python doesn't have explicit NaN guard.
     * Wait, the original Python has try/except. So only if an exception occurs.
     * Since inf is not an exception, the function returns cf with inf values.
     * The C NaN guard will zero those out. Let's do the same.
     */
    /* prod(range(1,71)) = 70! */
    double factorial = 1.0;
    for (int k = 1; k <= 70; k++) factorial *= k;  /* overflows to inf */
    double sum_range = 2485.0;  /* sum(1..70) */
    double root_coeff = absT1 * absT2 * factorial / sum_range;

    /* cf[4] = root_coeff * sum(cos(k*t1)*sin(k*t2)/log(|k+1|) for k=1..70) */
    double sumcsr = 0, sumcsi = 0;
    for (int k = 1; k <= 70; k++) {
        double kt1r = k * x1r, kt1i = k * x1i;
        double kt2r = k * x2r, kt2i = k * x2i;
        double cosr = cos(kt1r) * cosh(kt1i);
        double cosi = -sin(kt1r) * sinh(kt1i);
        double sinr = sin(kt2r) * cosh(kt2i);
        double sini = cos(kt2r) * sinh(kt2i);
        double pr, pi;
        c_mul(cosr, cosi, sinr, sini, &pr, &pi);
        double logk = log((double)(k + 1));
        sumcsr += pr / logk;
        sumcsi += pi / logk;
    }
    cRe[4] = root_coeff * sumcsr;
    cIm[4] = root_coeff * sumcsi;

    /* cf[35] = root_coeff * t1^2 */
    cRe[35] = root_coeff * t1_2r;
    cIm[35] = root_coeff * t1_2i;

    /* cf[34] = root_coeff * t2^2 */
    cRe[34] = root_coeff * t2_2r;
    cIm[34] = root_coeff * t2_2i;

    /* cf[36:71] = root_coeff * [cos(2k*t1)*sin(2k*t2)/log(|2k+1|) for k=1..35] */
    for (int k = 1; k <= 35; k++) {
        double kt1r = 2.0 * k * x1r, kt1i = 2.0 * k * x1i;
        double kt2r = 2.0 * k * x2r, kt2i = 2.0 * k * x2i;
        double cosr = cos(kt1r) * cosh(kt1i);
        double cosi = -sin(kt1r) * sinh(kt1i);
        double sinr = sin(kt2r) * cosh(kt2i);
        double sini = cos(kt2r) * sinh(kt2i);
        double pr, pi;
        c_mul(cosr, cosi, sinr, sini, &pr, &pi);
        double logk = log((double)(2 * k + 1));
        cRe[35 + k] = root_coeff * pr / logk;
        cIm[35 + k] = root_coeff * pi / logk;
    }

    /* NaN guard */
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_21_hand ----
 * Lagrange-basis construction using 51st roots of unity.
 */
static void poly_21_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double rootR[51], rootI[51];
    for (int j = 0; j < 51; j++) {
        double ang = 2.0 * M_PI * j / 51.0;
        rootR[j] = cos(ang); rootI[j] = sin(ang);
    }

    for (int k = 0; k < 51; k++) {
        double prodR = 1, prodI = 0;
        for (int j = 0; j < 51; j++) {
            if (j == k) continue;
            double dr = rootR[j] - rootR[k], di = rootI[j] - rootI[k];
            double pr, pi;
            c_mul(prodR, prodI, dr, di, &pr, &pi);
            prodR = pr; prodI = pi;
        }
        double d1r = x1r - rootR[k], d1i = x1i - rootI[k];
        double q1r, q1i;
        c_div(prodR, prodI, d1r, d1i, &q1r, &q1i);
        double d2r = x2r - rootR[k], d2i = x2i - rootI[k];
        c_div(q1r, q1i, d2r, d2i, &cRe[k], &cIm[k]);
    }
    /* cf[0:51] *= (t1 - roots) * (t2 - roots) elementwise */
    for (int j = 0; j < 51; j++) {
        double d1r = x1r - rootR[j], d1i = x1i - rootI[j];
        double d2r = x2r - rootR[j], d2i = x2i - rootI[j];
        double mr, mi, rr, ri;
        c_mul(d1r, d1i, d2r, d2i, &mr, &mi);
        c_mul(cRe[j], cIm[j], mr, mi, &rr, &ri);
        cRe[j] = rr; cIm[j] = ri;
    }
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_35_hand ----
 * cf[k-1] = cos(t1)*t1^k - sin(t2)*t2^k for k=1..70; cf[70] = |t1*t2|
 */
static void poly_35_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double cosT1r, cosT1i; c_cos(x1r, x1i, &cosT1r, &cosT1i);
    double sinT2r, sinT2i; c_sin(x2r, x2i, &sinT2r, &sinT2i);
    double p1r = 1, p1i = 0, p2r = 1, p2i = 0;
    for (int k = 1; k <= 70; k++) {
        double nr, ni;
        c_mul(p1r, p1i, x1r, x1i, &nr, &ni); p1r = nr; p1i = ni;
        c_mul(p2r, p2i, x2r, x2i, &nr, &ni); p2r = nr; p2i = ni;
        double ar, ai, br, bi;
        c_mul(cosT1r, cosT1i, p1r, p1i, &ar, &ai);
        c_mul(sinT2r, sinT2i, p2r, p2i, &br, &bi);
        cRe[k-1] = ar - br; cIm[k-1] = ai - bi;
    }
    double mr, mi; c_mul(x1r, x1i, x2r, x2i, &mr, &mi);
    cRe[70] = c_abs(mr, mi); cIm[70] = 0;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_37_hand ----
 * cf[i-1] = (sin(i*t1)+cos(i*t2))*i^2; then cf[1]+=sum(cf[0:2]), cf[4]+=prod(cf[0:5]),
 * cf[11]+=log(|cf[10]|+1), cf[24]+=angle(cf[23]),
 * cf[[34,44,54,64]] += |t2|^2+Re(t1)^3,
 * cf[[6,13,20,27,34,41,48,55,62,69]] += sin(t1)^71-cos(t2)^71
 */
static void poly_37_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    for (int i = 1; i <= 71; i++) {
        double sr, si, cr, ci;
        c_sin(i*x1r, i*x1i, &sr, &si);
        c_cos(i*x2r, i*x2i, &cr, &ci);
        double i2 = (double)(i*i);
        cRe[i-1] = (sr+cr)*i2; cIm[i-1] = (si+ci)*i2;
    }
    cRe[1] += cRe[0]+cRe[1]; cIm[1] += cIm[0]+cIm[1];
    double prodR = cRe[0], prodI = cIm[0];
    for (int j = 1; j < 5; j++) { double pr,pi; c_mul(prodR,prodI,cRe[j],cIm[j],&pr,&pi); prodR=pr; prodI=pi; }
    cRe[4] += prodR; cIm[4] += prodI;
    cRe[11] += log(c_abs(cRe[10],cIm[10])+1);
    cRe[24] += c_arg(cRe[23],cIm[23]);
    double val = c_abs(x2r,x2i); val=val*val+x1r*x1r*x1r;
    int i1[]={34,44,54,64}; for(int j=0;j<4;j++) cRe[i1[j]]+=val;
    /* sin(t1)^71 - cos(t2)^71 */
    double st1r,st1i; c_sin(x1r,x1i,&st1r,&st1i);
    double ct2r,ct2i; c_cos(x2r,x2i,&ct2r,&ct2i);
    double s71r=1,s71i=0,c71r=1,c71i=0;
    for(int j=0;j<71;j++){double pr,pi;c_mul(s71r,s71i,st1r,st1i,&pr,&pi);s71r=pr;s71i=pi;}
    for(int j=0;j<71;j++){double pr,pi;c_mul(c71r,c71i,ct2r,ct2i,&pr,&pi);c71r=pr;c71i=pi;}
    double dR=s71r-c71r,dI=s71i-c71i;
    int i2[]={6,13,20,27,34,41,48,55,62,69}; for(int j=0;j<10;j++){cRe[i2[j]]+=dR;cIm[i2[j]]+=dI;}
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_40_hand ----
 * cf[k-1] = Re(t1)^(k+1)*sin(angle(t2*k)) + Im(t2)^k*cos(angle(t1/k)); cf[70]=|t1|+|t2|
 */
static void poly_40_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double angT1 = atan2(x1i, x1r);
    for (int k = 1; k <= 70; k++) {
        double rePow = pow(x1r, k+1);
        double angT2k = atan2(k*x2i, k*x2r);
        double imPow = pow(x2i, k);
        cRe[k-1] = rePow*sin(angT2k) + imPow*cos(angT1);
        cIm[k-1] = 0;
    }
    cRe[70] = c_abs(x1r,x1i)+c_abs(x2r,x2i); cIm[70]=0;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_46_hand ----
 * Uses primes array, cf[0:18]=Re(p*(t1+t2)), cf[18:36]=Im(p*(t1-t2)),
 * cf[36:54]=Re(p*t1*conj(t2)+log(p)), cf[54:71]=Im(t1^k)*t2^(k^2),
 * cf[70]=sum(t1^(k+k^2))+sum(t2^k)^2
 */
static void poly_46_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    static const double P[]={2,3,5,7,11,17,19,23,29,31,37,41,43,47,53,59,61,67};
    double sR=x1r+x2r, sI=x1i+x2i, dR=x1r-x2r, dI=x1i-x2i;
    double cjR,cjI; c_mul(x1r,x1i,x2r,-x2i,&cjR,&cjI);
    for(int j=0;j<18;j++){cRe[j]=P[j]*sR;}
    for(int j=0;j<18;j++){cRe[18+j]=P[j]*dI;}
    for(int j=0;j<18;j++){cRe[36+j]=P[j]*cjR+log(P[j]);}
    double t1pR=1,t1pI=0;
    for(int k=1;k<=17;k++){
        double nr,ni;c_mul(t1pR,t1pI,x1r,x1i,&nr,&ni);t1pR=nr;t1pI=ni;
        int kk=k*k; double t2pR=1,t2pI=0;
        for(int j=0;j<kk;j++){c_mul(t2pR,t2pI,x2r,x2i,&nr,&ni);t2pR=nr;t2pI=ni;}
        if(54+k-1<71){cRe[54+k-1]=t1pI*t2pR;cIm[54+k-1]=t1pI*t2pI;}
    }
    double s1R=0,s1I=0;
    for(int k=1;k<=5;k++){int e=k+k*k;double pR=1,pI=0;for(int j=0;j<e;j++){double nr,ni;c_mul(pR,pI,x1r,x1i,&nr,&ni);pR=nr;pI=ni;}s1R+=pR;s1I+=pI;}
    double s2R=0,s2I=0,t2pR2=1,t2pI2=0;
    for(int k=1;k<=10;k++){double nr,ni;c_mul(t2pR2,t2pI2,x2r,x2i,&nr,&ni);t2pR2=nr;t2pI2=ni;s2R+=t2pR2;s2I+=t2pI2;}
    double sq2R,sq2I;c_mul(s2R,s2I,s2R,s2I,&sq2R,&sq2I);
    cRe[70]=s1R+sq2R;cIm[70]=s1I+sq2I;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_72_hand ----
 * cf[0]=t1+t2; cf[1:71]=k*(t1-t2+(sin(k)+i*cos(k)));
 * sort |cf[1:71]| descending → sorted_roots;
 * cf[1:71]=sorted_roots*(t1+t2*i*k);
 * cf = Re(cf)+Im(cf)+(sin(angle(cf+1))+i*cos(angle(cf+1)))
 */
static void poly_72_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    cRe[0]=x1r+x2r; cIm[0]=x1i+x2i;
    double dR=x1r-x2r,dI=x1i-x2i;
    for(int k=1;k<=70;k++){cRe[k]=k*(dR+sin((double)k));cIm[k]=k*(dI+cos((double)k));}
    double mags[70];
    for(int k=0;k<70;k++) mags[k]=c_abs(cRe[k+1],cIm[k+1]);
    for(int i=0;i<69;i++)for(int j=i+1;j<70;j++)if(mags[j]>mags[i]){double t=mags[i];mags[i]=mags[j];mags[j]=t;}
    for(int k=1;k<=70;k++){cRe[k]=mags[k-1]*(x1r-x2i*k);cIm[k]=mags[k-1]*(x1i+x2r*k);}
    for(int k=0;k<71;k++){
        double re=cRe[k],im=cIm[k],ang=atan2(im,re+1.0);
        cRe[k]=re+im+sin(ang);cIm[k]=cos(ang);
    }
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_74_hand ----
 * cf[i-1]=i*(t1+i*t2)^(1/i) for i=1..35; cf[70]=conj(cf[34]); cf[35]=2*t1+3*|t2|
 */
static void poly_74_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    for(int i=1;i<=35;i++){
        double br=x1r+i*x2r,bi=x1i+i*x2i,lr,li,er,ei;
        c_log(br,bi,&lr,&li);lr/=i;li/=i;
        c_exp2(lr,li,&er,&ei);
        cRe[i-1]=i*er;cIm[i-1]=i*ei;
        cRe[70]=cRe[i-1];cIm[70]=-cIm[i-1];
    }
    cRe[35]=2*x1r+3*c_abs(x2r,x2i);cIm[35]=2*x1i;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_94_hand ----
 * Sparse powers of t1/t2 at specific indices, then cf[3:71] *= sin(t1)*sin(t2)-cos(t1)*cos(t2)
 */
static void poly_94_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double t1p[10],t1pi[10]; t1p[1]=x1r;t1pi[1]=x1i;
    for(int k=2;k<=9;k++){c_mul(t1p[k-1],t1pi[k-1],x1r,x1i,&t1p[k],&t1pi[k]);}
    double t2p[9],t2pi[9]; t2p[1]=x2r;t2pi[1]=x2i;
    for(int k=2;k<=8;k++){c_mul(t2p[k-1],t2pi[k-1],x2r,x2i,&t2p[k],&t2pi[k]);}
    cRe[3]=-t1p[9];cIm[3]=-t1pi[9]; cRe[5]=t2p[8];cIm[5]=t2pi[8];
    cRe[7]=-t1p[7];cIm[7]=-t1pi[7]; cRe[9]=t2p[6];cIm[9]=t2pi[6];
    cRe[10]=-t1p[5];cIm[10]=-t1pi[5]; cRe[12]=t2p[4];cIm[12]=t2pi[4];
    cRe[14]=-t1p[3];cIm[14]=-t1pi[3]; cRe[16]=t2p[2];cIm[16]=t2pi[2];
    cRe[20]=-x1r;cIm[20]=-x1i;
    cRe[30]=5e5;cRe[40]=-5e6;cRe[50]=5e7;cRe[60]=-5e8;cRe[70]=5e9;
    double st1r,st1i,st2r,st2i,ct1r,ct1i,ct2r,ct2i;
    c_sin(x1r,x1i,&st1r,&st1i);c_sin(x2r,x2i,&st2r,&st2i);
    c_cos(x1r,x1i,&ct1r,&ct1i);c_cos(x2r,x2i,&ct2r,&ct2i);
    double mr1,mi1,mr2,mi2;
    c_mul(st1r,st1i,st2r,st2i,&mr1,&mi1);c_mul(ct1r,ct1i,ct2r,ct2i,&mr2,&mi2);
    double mulR=mr1-mr2,mulI=mi1-mi2;
    for(int k=3;k<71;k++){double pr,pi;c_mul(cRe[k],cIm[k],mulR,mulI,&pr,&pi);cRe[k]=pr;cIm[k]=pi;}
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_100_hand ----
 * iter=1; for j=1..71: cf[j-1]=iter; iter *= (log(|t1+i*t2|+1)/(71-j+1) + conj(iter))
 */
static void poly_100_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double zr=x1r-x2i,zi=x1i+x2r;
    double logAbsZ=log(c_abs(zr,zi)+1.0);
    double iterR=1.0,iterI=0.0;
    for(int j=1;j<=71;j++){
        cRe[j-1]=iterR;cIm[j-1]=iterI;
        double d=(double)(71-j+1);
        double fR=logAbsZ/d+iterR,fI=-iterI;
        double nr,ni;c_mul(iterR,iterI,fR,fI,&nr,&ni);iterR=nr;iterI=ni;
    }
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_45_hand ----
 * Loop 1: for k=1..71:
 *   if k%2==0: cf[k-1] = k*(t1+Re(t2))*sin(|t1|*k)
 *   else:      cf[k-1] = k*(t2-Im(t1))*cos(angle(t2)*k)
 * Loop 2: for i=2..len(cf)//2-1:
 *   cf[i-1] = cf[i-2]*(|t1|+0.5) + log(|t2|+1)    (sequential dependency!)
 *   cf[70-i] = -cf[71-i]*(|t2|+0.5) - log(|t1|+1)  (sequential from end)
 */
static void poly_45_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double absT1 = c_abs(x1r, x1i);
    double absT2 = c_abs(x2r, x2i);
    double angT2 = c_arg(x2r, x2i);

    /* Loop 1 */
    for (int k = 1; k <= 71; k++) {
        if (k % 2 == 0) {
            /* k*(t1 + Re(t2))*sin(|t1|*k) */
            /* t1 + Re(t2) = (x1r + x2r) + i*x1i */
            double sr = sin(absT1 * k), cr_unused = cos(absT1 * k);
            /* sin is real here since absT1*k is real */
            double ar = x1r + x2r, ai = x1i;
            cRe[k-1] = k * (ar * sr);
            cIm[k-1] = k * (ai * sr);
        } else {
            /* k*(t2 - Im(t1))*cos(angle(t2)*k) */
            /* t2 - Im(t1) = (x2r - x1i) + i*x2i */
            double cv = cos(angT2 * k);
            double ar = x2r - x1i, ai = x2i;
            cRe[k-1] = k * (ar * cv);
            cIm[k-1] = k * (ai * cv);
        }
    }

    /* Loop 2: sequential dependency — each cf[i-1] depends on cf[i-2]
     * len(cf)//2 = 35, so i goes from 2 to 34 */
    double mul1r = absT1 + 0.5;
    double logT2 = log(absT2 + 1.0);
    double mul2r = absT2 + 0.5;
    double logT1 = log(absT1 + 1.0);

    for (int i = 2; i < 35; i++) {
        /* cf[i-1] = cf[i-2] * (|t1|+0.5) + log(|t2|+1)
         * mul by real scalar, add real constant */
        double prevR = cRe[i-2], prevI = cIm[i-2];
        cRe[i-1] = prevR * mul1r + logT2;
        cIm[i-1] = prevI * mul1r;

        /* cf[len(cf)-i] = -cf[len(cf)-i+1] * (|t2|+0.5) - log(|t1|+1)
         * len(cf) = 71, so cf[71-i] = -cf[72-i] * ... */
        int idx = 71 - i;      /* target */
        int src = 72 - i;      /* source = idx + 1 */
        double srcR = cRe[src], srcI = cIm[src];
        cRe[idx] = -srcR * mul2r - logT1;
        cIm[idx] = -srcI * mul2r;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_42_serendipity ----
 * The original broken transpiled version of poly_42, preserved exactly as-is.
 * The transpiler replaced the arange loop variable with 0, producing a
 * beautiful accident. This just delegates to the original transpiled poly_42_c
 * from poly_generated_funcs.h — that function is never modified.
 */
static void poly_42_serendipity(double x1r, double x1i, double x2r, double x2i,
                                const double *cfpv, int n_cfpv,
                                double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    poly_42_c(x1r, x1i, x2r, x2i, cfpv, n_cfpv, cRe, cIm, nCoeffs);
}

/* ---- poly_42_hand ----
 * Correct version:
 * cf[0:35] = |t1| * sin(arange(1,36) * angle(t1))
 * cf[35:70] = Re(t2) * cos(arange(1,36) * Im(t2))
 * cf[70] = t1*t2 + i*sum(log(|cf[0:70]|+1))
 */
static void poly_42_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double absT1 = c_abs(x1r, x1i);
    double angT1 = c_arg(x1r, x1i);
    for (int k = 1; k <= 35; k++) {
        cRe[k-1] = absT1 * sin(k * angT1);
        cIm[k-1] = 0;
    }
    for (int k = 1; k <= 35; k++) {
        cRe[34 + k] = x2r * cos(k * x2i);
        cIm[34 + k] = 0;
    }
    /* cf[70] = t1*t2 + i*sum(log(|cf[0:70]|+1)) */
    double mr, mi;
    c_mul(x1r, x1i, x2r, x2i, &mr, &mi);
    double logsum = 0;
    for (int k = 0; k < 70; k++) logsum += log(c_abs(cRe[k], cIm[k]) + 1);
    cRe[70] = mr;
    cIm[70] = mi + logsum;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_2_hand ----
 * cf[0]=t1+t2; for k=2..36: v=sin(k*cf[k-2])+cos(k*t1)+Re(k*t2)*Im(k*cf[k-2]); cf[k-1]=v/|v|
 * cf[17], cf[31], cf[35] overwritten
 */
static void poly_2_hand(double x1r, double x1i, double x2r, double x2i,
                        const double *cfpv, int n_cfpv,
                        double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int i = 0; i < 36; i++) { cRe[i] = 0; cIm[i] = 0; }
    cRe[0] = x1r + x2r; cIm[0] = x1i + x2i;
    for (int k = 2; k <= 36; k++) {
        double prevR = cRe[k-2], prevI = cIm[k-2];
        /* sin(k*cf[k-2]) */
        double sr, si; c_sin(k*prevR, k*prevI, &sr, &si);
        /* cos(k*t1) */
        double cr, ci; c_cos(k*x1r, k*x1i, &cr, &ci);
        /* Re(k*t2)*Im(k*cf[k-2]) */
        double reKt2 = k * x2r;
        double imKprev = k * prevI;
        double vr = sr + cr + reKt2 * imKprev;
        double vi = si + ci;
        double mag = c_abs(vr, vi);
        if (mag > 1e-30) { cRe[k-1] = vr/mag; cIm[k-1] = vi/mag; }
    }
    /* cf[17] = t1^2 + Re(t1)*t2 - Im(t2^2) */
    double t1sq_r, t1sq_i; c_mul(x1r,x1i,x1r,x1i,&t1sq_r,&t1sq_i);
    double rt1t2_r = x1r*x2r, rt1t2_i = x1r*x2i;
    double t2sq_r, t2sq_i; c_mul(x2r,x2i,x2r,x2i,&t2sq_r,&t2sq_i);
    cRe[17] = t1sq_r + rt1t2_r - t2sq_i; cIm[17] = t1sq_i + rt1t2_i;
    /* cf[31] = 2*(t1+t2) - Re(t1*t2) + sin(Re(t1))*cos(Im(t2)) */
    double mr, mi; c_mul(x1r,x1i,x2r,x2i,&mr,&mi);
    cRe[31] = 2*(x1r+x2r) - mr + sin(x1r)*cos(x2i);
    cIm[31] = 2*(x1i+x2i);
    /* cf[35] = cf[17]*cf[31] + sin(Re(t1*t2)) - cos(Im(t1*t2)) */
    double p35r, p35i; c_mul(cRe[17],cIm[17],cRe[31],cIm[31],&p35r,&p35i);
    cRe[35] = p35r + sin(mr) - cos(mi); cIm[35] = p35i;
    for (int i = 0; i < 36; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_9_hand ----
 * 51 coefficients. Loop sets cf, then post-loop reads/slice assignments.
 */
static void poly_9_hand(double x1r, double x1i, double x2r, double x2i,
                        const double *cfpv, int n_cfpv,
                        double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 51;
    for (int i = 0; i < 51; i++) { cRe[i] = 0; cIm[i] = 0; }
    cRe[0] = x1r + x2r; cIm[0] = x1i + x2i;
    double absT1 = c_abs(x1r,x1i), angT2 = c_arg(x2r,x2i);
    for (int k = 2; k <= 51; k++) {
        double denom = sqrt((double)(k*k) + 1.0);
        cRe[k-1] = (absT1*sin(k) + angT2*cos(k)) / denom;
        cIm[k-1] = 0;
    }
    /* cf[9] = cf[0]^2 - cf[1]^2 + log(|cf[2]|+1) */
    double c0sq_r,c0sq_i; c_mul(cRe[0],cIm[0],cRe[0],cIm[0],&c0sq_r,&c0sq_i);
    double c1sq_r,c1sq_i; c_mul(cRe[1],cIm[1],cRe[1],cIm[1],&c1sq_r,&c1sq_i);
    cRe[9] = c0sq_r - c1sq_r + log(c_abs(cRe[2],cIm[2])+1);
    cIm[9] = c0sq_i - c1sq_i;
    /* cf[19] = sum(cf[0:19]) * t1 */
    double sumR=0,sumI=0;
    for(int j=0;j<19;j++){sumR+=cRe[j];sumI+=cIm[j];}
    c_mul(sumR,sumI,x1r,x1i,&cRe[19],&cIm[19]);
    /* cf[29] = prod(cf[0:29]) * t2 */
    double pR=cRe[0],pI=cIm[0];
    for(int j=1;j<29;j++){double r,i;c_mul(pR,pI,cRe[j],cIm[j],&r,&i);pR=r;pI=i;}
    c_mul(pR,pI,x2r,x2i,&cRe[29],&cIm[29]);
    /* cf[39] = cf[38]*cf[37] / (1+t1*t2) */
    double n39r,n39i; c_mul(cRe[38],cIm[38],cRe[37],cIm[37],&n39r,&n39i);
    double t12r,t12i; c_mul(x1r,x1i,x2r,x2i,&t12r,&t12i);
    double d39r=1+t12r,d39i=t12i;
    c_div(n39r,n39i,d39r,d39i,&cRe[39],&cIm[39]);
    /* cf[40:50] = Re(cf[30:40]) + i*Im(cf[0:10]) */
    for(int j=0;j<10;j++){cRe[40+j]=cRe[30+j];cIm[40+j]=cIm[j];}
    /* cf[50] = sum(cf[0:50]) */
    sumR=0;sumI=0;
    for(int j=0;j<50;j++){sumR+=cRe[j];sumI+=cIm[j];}
    cRe[50]=sumR;cIm[50]=sumI;
    for (int i = 0; i < 51; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_44_hand ---- */
static void poly_44_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double sr,si,cr,ci;
    c_sin(x1r,x1i,&sr,&si); c_cos(x2r,x2i,&cr,&ci);
    c_mul(sr,si,cr,ci,&cRe[0],&cIm[0]);
    c_cos(x1r,x1i,&cr,&ci); c_sin(x2r,x2i,&sr,&si);
    cRe[1]=cr+sr; cIm[1]=ci+si;
    double a1=c_abs(x1r,x1i),a2=c_abs(x2r,x2i);
    cRe[2]=a1*a1*a1-a2*a2*a2*a2; cIm[2]=0;
    cRe[3]=c_arg(x1r,x1i)-c_arg(x2r,x2i); cIm[3]=0;
    double m0r,m0i; c_mul(x1r,x1i,x2r,x2i,&m0r,&m0i);
    cRe[4]=c_abs(m0r,m0i); cIm[4]=0;
    for(int k=6;k<=35;k++){
        c_sin(k*x1r,k*x1i,&sr,&si); c_cos(k*x2r,k*x2i,&cr,&ci);
        cRe[k-1]=sr+cr; cIm[k-1]=si+ci;
        int k2=70-k;
        c_sin(k2*x1r,k2*x1i,&sr,&si); c_cos(k2*x2r,k2*x2i,&cr,&ci);
        cRe[k+34]=sr-cr; cIm[k+34]=si-ci;
    }
    cRe[35]=c_abs(x1r+x2r,x1i+x2i); cIm[35]=0;
    cRe[70]=log(c_abs(m0r,m0i)+1); cIm[70]=0;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_50_hand ----
 * Loop with conditional cf[k-2], cf[k-3], cf[k-4] dependencies
 */
static void poly_50_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double sr,si,cr,ci;
    c_sin(x1r+x2r,x1i+x2i,&sr,&si); c_cos(x1r-x2r,x1i-x2i,&cr,&ci);
    double cvr=sr+cr, cvi=si+ci;
    double absT1=c_abs(x1r,x1i);
    for(int k=1;k<=71;k++){
        if(k%2==0){
            cRe[k-1]=cvr/(double)k - absT1; cIm[k-1]=cvi/(double)k;
        } else {
            cRe[k-1]=cvr*(double)k + log((double)k+1) + x2i - x1r;
            cIm[k-1]=cvi*(double)k;
        }
        if(k%3==0 && k>=3){cRe[k-1]+=3*cRe[k-2];cIm[k-1]+=3*cIm[k-2];}
        if(k%5==0 && k>=4){cRe[k-1]+=5*cRe[k-3];cIm[k-1]+=5*cIm[k-3];}
        if(k%7==0 && k>=5){cRe[k-1]+=7*cRe[k-4];cIm[k-1]+=7*cIm[k-4];}
    }
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_54_hand ----
 * Loop sets cf, then cf[0:30] *= (|t1|*|t2|)^arange(1,31)
 */
static void poly_54_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    for(int i=1;i<=71;i++){
        /* z = t1*cos(i*t2/15) + t2*sin(i*t1/15) */
        double cr,ci,sr,si;
        c_cos(i*x2r/15.0,i*x2i/15.0,&cr,&ci);
        c_sin(i*x1r/15.0,i*x1i/15.0,&sr,&si);
        double ar,ai,br,bi;
        c_mul(x1r,x1i,cr,ci,&ar,&ai);
        c_mul(x2r,x2i,sr,si,&br,&bi);
        double zr=ar+br, zi=ai+bi;
        double phi=c_arg(zr,zi), r=c_abs(zr,zi);
        /* r*exp(i*phi)^i = r*exp(i*i*phi) */
        double er,ei; c_exp2(0, i*phi, &er, &ei);
        double sign = (i%2==0) ? 1.0 : -1.0;
        cRe[i-1] = r*er + sign*(double)(i*i);
        cIm[i-1] = r*ei;
    }
    /* cf[0:30] *= (|t1|*|t2|)^arange(1,31) */
    double ab=c_abs(x1r,x1i)*c_abs(x2r,x2i);
    double pw=1.0;
    for(int k=0;k<30;k++){
        pw*=ab;
        cRe[k]*=pw; cIm[k]*=pw;
    }
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_61_hand ----
 * cf[0:35] = Re(t1)*arange(1,36)^3 + Im(t2)*sin(arange(1,36))
 * cf[35:70] = Im(t1)*arange(70,35,-1)^2 + Re(t2)*cos(arange(70,35,-1))
 * cf[70] = |t1|*angle(t2) - |t2|*angle(t1)
 */
static void poly_61_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    for(int k=1;k<=35;k++){
        cRe[k-1] = x1r*(double)(k*k*k) + x2i*sin((double)k);
        cIm[k-1] = 0;
    }
    for(int k=70;k>=36;k--){
        cRe[k-1] = x1i*(double)(k*k) + x2r*cos((double)k);
        cIm[k-1] = 0;
    }
    cRe[70] = c_abs(x1r,x1i)*c_arg(x2r,x2i) - c_abs(x2r,x2i)*c_arg(x1r,x1i);
    cIm[70] = 0;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_62_hand ----
 * Sequential: cf[i-1] = i*cf[i-2]^2, then post-loop modifications
 */
static void poly_62_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    /* cf[0] = t1^5 + t2^5 */
    double pr=1,pi=0,qr=1,qi=0;
    for(int j=0;j<5;j++){double r,i;c_mul(pr,pi,x1r,x1i,&r,&i);pr=r;pi=i;}
    double p2r=1,p2i=0;
    for(int j=0;j<5;j++){double r,i;c_mul(p2r,p2i,x2r,x2i,&r,&i);p2r=r;p2i=i;}
    cRe[0]=pr+p2r; cIm[0]=pi+p2i;
    for(int i=2;i<=71;i++){
        double sq_r,sq_i;
        c_mul(cRe[i-2],cIm[i-2],cRe[i-2],cIm[i-2],&sq_r,&sq_i);
        if(i%2==0){
            cRe[i-1]=(double)i*sq_r; cIm[i-1]=(double)i*sq_i;
        } else {
            double fr=(double)i*sq_r, fi=(double)i*sq_i;
            /* *(1+0.1*t2) */
            double mr,mi; c_mul(fr,fi,1+0.1*x2r,-0.1*x2i,&mr,&mi);
            /* wait: 1+0.1*t2 = (1+0.1*x2r) + i*(0.1*x2i) */
            c_mul(fr,fi,1+0.1*x2r,0.1*x2i,&cRe[i-1],&cIm[i-1]);
        }
    }
    /* cf[0] += 2*cf[1] */
    cRe[0]+=2*cRe[1]; cIm[0]+=2*cIm[1];
    /* cf[1] -= 3*cf[2] */
    cRe[1]-=3*cRe[2]; cIm[1]-=3*cIm[2];
    /* for i=3..69: cf[i] += cf[i+1] - cf[i+2] */
    for(int i=3;i<70;i++){cRe[i]+=cRe[i+1]-cRe[i+2]; cIm[i]+=cIm[i+1]-cIm[i+2];}
    /* cf[69] += cf[70] */
    cRe[69]+=cRe[70]; cIm[69]+=cIm[70];
    /* cf[70] = |t1|^2 - |t2|^2 + 2*Im(t1)*Im(t2) - angle(t2) */
    double a1=c_abs(x1r,x1i),a2=c_abs(x2r,x2i);
    cRe[70]=a1*a1-a2*a2+2*x1i*x2i-c_arg(x2r,x2i); cIm[70]=0;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_65_hand ---- */
static void poly_65_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double sumR=x1r+x2r, sumI=x1i+x2i;
    for(int k=1;k<=71;k++){
        /* (t1+t2)^(2k-1) */
        int exp=2*k-1; double pR=1,pI=0;
        for(int j=0;j<exp;j++){double r,i;c_mul(pR,pI,sumR,sumI,&r,&i);pR=r;pI=i;}
        /* sin(k*t1)*cos(k*t2) */
        double sr,si,cr,ci;
        c_sin(k*x1r,k*x1i,&sr,&si);c_cos(k*x2r,k*x2i,&cr,&ci);
        double scr,sci; c_mul(sr,si,cr,ci,&scr,&sci);
        /* log(|k^t2|+1)*Re(t1^t2) — k^t2 = exp(t2*log(k)) */
        double lk=log((double)k);
        double eR,eI; c_exp2(x2r*lk-x2i*0, x2i*lk+x2r*0, &eR, &eI);
        /* wait: k^t2 = exp(t2*log(k)). t2*log(k) = (x2r*lk, x2i*lk) */
        c_exp2(x2r*lk, x2i*lk, &eR, &eI);
        double logAbsKt2 = log(c_abs(eR,eI)+1);
        /* t1^t2 = exp(t2*log(t1)) */
        double lt1r,lt1i; c_log(x1r,x1i,&lt1r,&lt1i);
        double elt1r,elt1i; c_mul(x2r,x2i,lt1r,lt1i,&elt1r,&elt1i);
        double t1t2r,t1t2i; c_exp2(elt1r,elt1i,&t1t2r,&t1t2i);
        double reT1t2 = t1t2r;
        /* |Im(t1^(2k+1) + t2^(2k))| */
        int e1=2*k+1,e2=2*k;
        double p1r=1,p1i=0; for(int j=0;j<e1;j++){double r,i;c_mul(p1r,p1i,x1r,x1i,&r,&i);p1r=r;p1i=i;}
        double p2r=1,p2i=0; for(int j=0;j<e2;j++){double r,i;c_mul(p2r,p2i,x2r,x2i,&r,&i);p2r=r;p2i=i;}
        double absImPow = fabs(p1i+p2i);
        double vr = pR + scr + logAbsKt2*reT1t2 + absImPow;
        double vi = pI + sci;
        /* conj * (-1)^k */
        double sign = (k%2==0) ? 1.0 : -1.0;
        cRe[k-1] = sign*vr; cIm[k-1] = sign*(-vi);
        if(k%2==0){
            /* /= (k + t1) */
            double dr=k+x1r, di=x1i;
            double qr,qi; c_div(cRe[k-1],cIm[k-1],dr,di,&qr,&qi);
            cRe[k-1]=qr; cIm[k-1]=qi;
        }
    }
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_70_hand ----
 * cf[i-1] = Re(t1)*Re(t2)*i^2/exp(|t1|*i) + Im(t1)*Im(t2)*i^3/exp(|t2|*i)
 * cf[1::2] *= -1; cf[k where k^2<=71] += i*|t1|*|t2|
 */
static void poly_70_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double absT1=c_abs(x1r,x1i), absT2=c_abs(x2r,x2i);
    for(int i=1;i<=71;i++){
        /* exp(|t1|*1j) = cos(|t1|) + i*sin(|t1|) */
        double e1r=cos(absT1), e1i=sin(absT1);
        double e2r=cos(absT2), e2i=sin(absT2);
        /* Re(t1)*Re(t2)*i^2 / exp(|t1|*1j) */
        double numR = x1r*x2r*(double)(i*i);
        double ar,ai; c_div(numR,0,e1r,e1i,&ar,&ai);
        /* Im(t1)*Im(t2)*i^3 / exp(|t2|*1j) */
        double numR2 = x1i*x2i*(double)(i*i*i);
        double br,bi; c_div(numR2,0,e2r,e2i,&br,&bi);
        cRe[i-1]=ar+br; cIm[i-1]=ai+bi;
    }
    /* cf[1::2] *= -1 (odd indices) */
    for(int i=1;i<71;i+=2){cRe[i]=-cRe[i];cIm[i]=-cIm[i];}
    /* cf[k where k^2<=71] += i*|t1|*|t2| — k=1..8 (8^2=64<=71, 9^2=81>71) */
    /* p=arange(1,72); cf[p**2<=71] — this means indices where (index+1)^2<=71? No: p=arange(1,72) is [1..71], p**2 is [1,4,9,...]; cf[p**2<=71] selects cf at indices where p^2<=71, i.e. p=1..8, so cf[1],cf[4],cf[9],...cf[64] — but p is used as boolean mask on cf */
    /* Actually: p=np.arange(1,72) gives [1,2,...,71]. p**2 = [1,4,...,5041]. p**2<=71 is True for p=1..8. cf[p**2<=71] selects cf[0:8] (first 8 elements, where mask is True) */
    double ab12=absT1*absT2;
    for(int j=0;j<8;j++){cIm[j]+=ab12;} /* += 1j * |t1|*|t2| */
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_73_hand ----
 * Large-scale coefficients with powers of 10. Multiple loops, each covering different ranges.
 */
static void poly_73_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double sR=x1r+x2r, sI=x1i+x2i, dR=x1r-x2r, dI=x1i-x2i;
    double ct1r,ct1i,st2r,st2i;
    c_cos(x1r,x1i,&ct1r,&ct1i); c_sin(x2r,x2i,&st2r,&st2i);
    cRe[0]=1e30*sR; cIm[0]=1e30*sI;
    cRe[1]=1e28*dR; cIm[1]=1e28*dI;
    cRe[2]=1e26*sR; cIm[2]=1e26*sI;
    for(int k=4;k<=21;k++){
        double s=pow(10.0,30-k);
        cRe[k-1]=s*(ct1r+st2r); cIm[k-1]=s*(ct1i+st2i);
    }
    for(int k=22;k<=31;k++){
        double s=pow(10.0,k-21);
        cRe[k-1]=s*(ct1r-st2r); cIm[k-1]=s*(ct1i-st2i);
    }
    double csR,csI,ssR,ssI;
    c_cos(sR,sI,&csR,&csI); c_sin(dR,dI,&ssR,&ssI);
    for(int k=32;k<=41;k++){
        double s=pow(10.0,42-k);
        double mr,mi; c_mul(sR,sI,csR+ssR,csI+ssI,&mr,&mi);
        cRe[k-1]=s*mr; cIm[k-1]=s*mi;
    }
    cRe[41]=1e21*dR; cIm[41]=1e21*dI;
    double absS=c_abs(sR,sI), angD=c_arg(dR,dI);
    for(int k=43;k<=53;k++){double s=pow(10.0,53-k);cRe[k-1]=s*(absS+angD);cIm[k-1]=0;}
    double absD=c_abs(dR,dI), angS=c_arg(sR,sI);
    for(int k=54;k<=64;k++){double s=pow(10.0,64-k);cRe[k-1]=s*(absD+angS);cIm[k-1]=0;}
    double s1r,s1i,c2r,c2i;
    c_sin(x1r,x1i,&s1r,&s1i); c_cos(x2r,x2i,&c2r,&c2i);
    for(int k=65;k<=71;k++){double s=pow(10.0,71-k);cRe[k-1]=s*(s1r+c2r);cIm[k-1]=s*(s1i+c2i);}
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_78_hand ----
 * 7 slice assignments with fractional powers and alternating signs
 */
static void poly_78_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double a1=c_abs(x1r,x1i), a2=c_abs(x2r,x2i);
    double logA2=log(1+a2), angT2=c_arg(x2r,x2i);
    double aSR=x1r+x2r, aSI=x1i+x2i;
    double angProd=c_arg(x1r*x2r-x1i*x2i, x1r*x2i+x1i*x2r);
    double angDifR=x1r-x2r, angDifI=x1i-x2i;
    double angDif=c_arg(angDifR,angDifI);
    double logAbsSum=log(1+c_abs(aSR,aSI));
    /* cf[0:10] = |t1|^(k/5)*log(1+|t2|) */
    for(int k=1;k<=10;k++){cRe[k-1]=pow(a1,(double)k/5.0)*logA2;cIm[k-1]=0;}
    /* cf[10:20] = Re(t1)^k * angle(t2) * (-1)^k */
    for(int k=1;k<=10;k++){
        double sign=(k%2==0)?1.0:-1.0;
        cRe[9+k]=pow(x1r,(double)k)*angT2*sign; cIm[9+k]=0;
    }
    /* cf[20:30] = Im(t1)^(k/3) * |t2|^(k/4) * (-1)^k */
    for(int k=1;k<=10;k++){
        double sign=(k%2==0)?1.0:-1.0;
        cRe[19+k]=pow(x1i,(double)k/3.0)*pow(a2,(double)k/4.0)*sign; cIm[19+k]=0;
    }
    /* cf[30:40] = |t1*t2|^(k/2) * k */
    double absProd=c_abs(x1r*x2r-x1i*x2i,x1r*x2i+x1i*x2r);
    for(int k=1;k<=10;k++){cRe[29+k]=pow(absProd,(double)k/2.0)*(double)k;cIm[29+k]=0;}
    /* cf[40:50] = Re((t1+t2)^(k/2))*cos(angle(t1*t2))*(-1)^k */
    for(int k=1;k<=10;k++){
        double sign=(k%2==0)?1.0:-1.0;
        /* (t1+t2)^(k/2) = exp(k/2*log(t1+t2)) */
        double lr,li; c_log(aSR,aSI,&lr,&li);
        double er,ei; c_exp2(lr*(double)k/2.0, li*(double)k/2.0, &er, &ei);
        cRe[39+k]=er*cos(angProd)*sign; cIm[39+k]=0;
    }
    /* cf[50:60] = Im((t1+t2)^(k/3))*sin(angle(t1-t2))*(-1)^k */
    for(int k=1;k<=10;k++){
        double sign=(k%2==0)?1.0:-1.0;
        double lr,li; c_log(aSR,aSI,&lr,&li);
        double er,ei; c_exp2(lr*(double)k/3.0, li*(double)k/3.0, &er, &ei);
        cRe[49+k]=ei*sin(angDif)*sign; cIm[49+k]=0;
    }
    /* cf[60:70] = Re(t1^k)*|t2^k|*log(1+|t1+t2|) */
    double t1pR=1,t1pI=0,t2pR=1,t2pI=0;
    for(int k=1;k<=10;k++){
        double nr,ni;
        c_mul(t1pR,t1pI,x1r,x1i,&nr,&ni);t1pR=nr;t1pI=ni;
        c_mul(t2pR,t2pI,x2r,x2i,&nr,&ni);t2pR=nr;t2pI=ni;
        cRe[59+k]=t1pR*c_abs(t2pR,t2pI)*logAbsSum; cIm[59+k]=0;
    }
    cRe[70]=c_abs(angDifR,angDifI)*log(1+a1); cIm[70]=0;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_81_hand ----
 * Loop + fancy indexing (np.arange slices with step)
 */
static void poly_81_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double a1=c_abs(x1r,x1i), a2=c_abs(x2r,x2i);
    for(int k=1;k<=71;k++){
        double sr,si,cr,ci;
        c_sin(k*x1r,k*x1i,&sr,&si); c_cos(k*x2r,k*x2i,&cr,&ci);
        double mr,mi; c_mul(sr,si,cr,ci,&mr,&mi);
        cRe[k-1]=(k+20)*mr + pow(a1,(double)k) + pow(a2,(double)k);
        cIm[k-1]=(k+20)*mi;
    }
    /* cf[arange(2,71,5)] += |t1|*|t2| → indices 2,7,12,...,67 */
    double ab=a1*a2;
    for(int j=2;j<71;j+=5){cRe[j]+=ab;}
    /* cf[arange(3,70,7)] += (-1)^arange(1,11) * angle(t1+t2) — 10 elements */
    double angSum=c_arg(x1r+x2r,x1i+x2i);
    int idx3[]={3,10,17,24,31,38,45,52,59,66};
    for(int j=0;j<10;j++){double sign=(j%2==0)?-1.0:1.0;cRe[idx3[j]]+=sign*angSum;}
    /* cf[arange(6,67,9)] += (-1)^arange(1,8)*log(|t1+t2|+1) — 7 elements */
    double logS=log(c_abs(x1r+x2r,x1i+x2i)+1);
    int idx6[]={6,15,24,33,42,51,60};
    for(int j=0;j<7;j++){double sign=(j%2==0)?-1.0:1.0;cRe[idx6[j]]+=sign*logS;}
    /* cf[arange(5,71,7)] *= Re(t1+t2) — indices 5,12,19,...,68 */
    double reSum=x1r+x2r;
    for(int j=5;j<71;j+=7){cRe[j]*=reSum;cIm[j]*=reSum;}
    /* cf[arange(7,64,11)] *= Im(t1+t2) — indices 7,18,29,40,51,62 */
    double imSum=x1i+x2i;
    for(int j=7;j<64;j+=11){cRe[j]*=imSum;cIm[j]*=imSum;}
    /* cf[arange(1,72,7)] *= conj(t1+t2) — indices 1,8,15,...,71→capped to 70 */
    double cjR=x1r+x2r, cjI=-(x1i+x2i);
    for(int j=1;j<71;j+=7){double r,i;c_mul(cRe[j],cIm[j],cjR,cjI,&r,&i);cRe[j]=r;cIm[j]=i;}
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_82_hand ----
 * Forward dependency: cf[i-1] = i + cf[i-2]*sin + cf[i-3]*cos + cf[i-4]*log
 * Then reverse loop, then cf[70]=sum
 */
static void poly_82_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double mr,mi; c_mul(x1r,x1i,x2r,x2i,&mr,&mi);
    cRe[0]=1+mr; cIm[0]=mi;
    cRe[1]=2+c_abs(x1r,x1i)*c_abs(x2r,x2i); cIm[1]=0;
    cRe[2]=3+c_abs(x1r+x2r,x1i+x2i); cIm[2]=0;
    for(int i=4;i<=36;i++){
        double s2r,s2i; c_sin(i*x2r,i*x2i,&s2r,&s2i);
        double c1r,c1i; c_cos(i*x1r,i*x1i,&c1r,&c1i);
        double lv=log(c_abs(i*mr+1, i*mi));
        double ar,ai; c_mul(cRe[i-2],cIm[i-2],s2r,s2i,&ar,&ai);
        double br,bi; c_mul(cRe[i-3],cIm[i-3],c1r,c1i,&br,&bi);
        cRe[i-1]=(double)i+ar+br+cRe[i-4]*lv; cIm[i-1]=ai+bi+cIm[i-4]*lv;
    }
    for(int i=37;i<=70;i++){
        int ri=70-i;
        int i2=70-((i<69)?i:69), i3=69-((i<68)?i:68), i4=68-((i<67)?i:67);
        double s1r,s1i; c_sin(ri*x1r,ri*x1i,&s1r,&s1i);
        double c2r,c2i; c_cos(ri*x2r,ri*x2i,&c2r,&c2i);
        double lv=log(c_abs(ri*mr+1, ri*mi));
        double ar,ai; c_mul(cRe[i2],cIm[i2],s1r,s1i,&ar,&ai);
        double br,bi; c_mul(cRe[i3],cIm[i3],c2r,c2i,&br,&bi);
        cRe[i-1]=(double)(70-i)+ar+br+cRe[i4]*lv; cIm[i-1]=ai+bi+cIm[i4]*lv;
    }
    double sumR=0,sumI=0;
    for(int j=0;j<70;j++){sumR+=cRe[j];sumI+=cIm[j];}
    cRe[70]=sumR+c_arg(x1r-x2r,x1i-x2i);
    cIm[70]=sumI+c_arg(x1r+x2r,x1i+x2i);
    /* angle returns real, so imag part correction */
    cIm[70]=sumI; cRe[70]=sumR+c_arg(x1r-x2r,x1i-x2i);
    /* np.real(np.angle(...)) + np.imag(np.angle(...)) — angle is real, so imag=0 */
    cRe[70]=sumR+c_arg(x1r-x2r,x1i-x2i); cIm[70]=sumI;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_95_hand ----
 * Loop + slice modifications reading loop-set values
 */
static void poly_95_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    cRe[0]=x1r*x1r*x1r - x1i*x1i + 2*x1i*x1r - x2r + x2i*x2i; cIm[0]=0;
    cRe[1]=x1i*x1i*x1i - 5*x1r*x1r + 2*x1r*x1i + 5*x2r - 2*x2i*x2i; cIm[1]=0;
    for(int k=3;k<=71;k++){
        double sr,si; c_sin(k*x1r,k*x1i,&sr,&si);
        double cr,ci; c_cos(k*x2r,k*x2i,&cr,&ci);
        /* |t1^k+t2^(k-1)| */
        double p1r=1,p1i=0; for(int j=0;j<k;j++){double r,i;c_mul(p1r,p1i,x1r,x1i,&r,&i);p1r=r;p1i=i;}
        double p2r=1,p2i=0; for(int j=0;j<k-1;j++){double r,i;c_mul(p2r,p2i,x2r,x2i,&r,&i);p2r=r;p2i=i;}
        cRe[k-1]=c_abs(sr,si)+c_abs(cr,ci)-c_abs(p1r+p2r,p1i+p2i);
        cIm[k-1]=0;
    }
    /* cf[29:40] = |cf[29:40]| / (|t1-t2|^2+1) */
    double dR=x1r-x2r,dI=x1i-x2i; double ad=c_abs(dR,dI); double d1=ad*ad+1;
    for(int j=29;j<40;j++){cRe[j]=c_abs(cRe[j],cIm[j])/d1; cIm[j]=0;}
    /* cf[49:60] = -|cf[49:60]| / (|t1+t2|^2+1) */
    double sR2=x1r+x2r,sI2=x1i+x2i; double as2=c_abs(sR2,sI2); double d2=as2*as2+1;
    for(int j=49;j<60;j++){cRe[j]=-c_abs(cRe[j],cIm[j])/d2; cIm[j]=0;}
    /* cf[64:71] = cf[0:7] * (|t1|^2+|t2|^2) */
    double a1=c_abs(x1r,x1i),a2=c_abs(x2r,x2i); double scale=a1*a1+a2*a2;
    for(int j=0;j<7;j++){cRe[64+j]=cRe[j]*scale; cIm[64+j]=cIm[j]*scale;}
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_96_hand ----
 * Two sequential loops with cf[k-1]=k*cf[k-2]+... dependency
 */
static void poly_96_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    /* cf[0] = t1^5 - t2^4 + t1^2 - t2^2 + |t1| + |t2| */
    double t1p[8],t1pi[8],t2p[7],t2pi[7];
    t1p[1]=x1r;t1pi[1]=x1i;
    for(int j=2;j<=5;j++){c_mul(t1p[j-1],t1pi[j-1],x1r,x1i,&t1p[j],&t1pi[j]);}
    t2p[1]=x2r;t2pi[1]=x2i;
    for(int j=2;j<=6;j++){c_mul(t2p[j-1],t2pi[j-1],x2r,x2i,&t2p[j],&t2pi[j]);}
    double a1=c_abs(x1r,x1i),a2=c_abs(x2r,x2i);
    cRe[0]=t1p[5]-t2p[4]+t1p[2]-t2p[2]+a1+a2;
    cIm[0]=t1pi[5]-t2pi[4]+t1pi[2]-t2pi[2];
    /* cf[50] */
    cRe[50]=t2p[6]-t1p[4]+t2p[3]-t1p[2]+c_arg(x1r,x1i);
    cIm[50]=t2pi[6]-t1pi[4]+t2pi[3]-t1pi[2];
    double sr,si; c_sin(x2r,x2i,&sr,&si);
    cRe[50]+=sr; cIm[50]+=si;
    /* cf[70] */
    double t1_7r=t1p[5],t1_7i=t1pi[5];
    c_mul(t1_7r,t1_7i,t1p[2],t1pi[2],&t1_7r,&t1_7i);
    double t2_5r=t2p[5],t2_5i=t2pi[5];
    double cr,ci; c_cos(x1r,x1i,&cr,&ci);
    cRe[70]=t1_7r+t2_5r-t1p[3]-t2p[2]+cr-sr;
    cIm[70]=t1_7i+t2_5i-t1pi[3]-t2pi[2]+ci-si;
    /* for k=2..50: cf[k-1] = k*cf[k-2] + |cf[0]|/k */
    double absC0=c_abs(cRe[0],cIm[0]);
    for(int k=2;k<=50;k++){
        cRe[k-1]=(double)k*cRe[k-2]+absC0/(double)k;
        cIm[k-1]=(double)k*cIm[k-2];
    }
    /* for r=52..70: cf[r-1] = r*cf[r-2] + |cf[50]|/r */
    double absC50=c_abs(cRe[50],cIm[50]);
    for(int r=52;r<=70;r++){
        cRe[r-1]=(double)r*cRe[r-2]+absC50/(double)r;
        cIm[r-1]=(double)r*cIm[r-2];
    }
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_97_hand ----
 * Three loops: first sets all, second overwrites [0:10], third overwrites [60:71] reading cf[k-2]
 */
static void poly_97_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    /* z = t1 + i*t2 = (x1r - x2i) + i*(x1i + x2r) */
    double zr=x1r-x2i, zi=x1i+x2r;
    /* Loop 1: cf[k-1] = z * k^(-|t1|*log(|k+1|)) */
    double absT1=c_abs(x1r,x1i);
    for(int k=1;k<=71;k++){
        double ex = -absT1*log((double)(k+1));
        double kpow = pow((double)k, ex);
        cRe[k-1]=zr*kpow; cIm[k-1]=zi*kpow;
    }
    /* Loop 2: overwrite cf[0:10] */
    for(int k=1;k<=10;k++){
        double cr,ci,sr2,si2;
        c_cos(x1i+k*x2i, 0, &cr, &ci); /* cos(Im(t1+k*t2)) — Im is real */
        /* actually Im(t1+k*t2) = x1i + k*x2i, this is a real number */
        double imV = x1i + k*x2i;
        double reV = x1r - k*x2r; /* wait: Re(t1-k*t2) = x1r - k*x2r */
        double cosV = cos(imV);
        double sinV = sin(reV);
        double absV = fabs((double)(k*k*k)*cosV - sinV);
        cRe[k-1]=zr*absV; cIm[k-1]=zi*absV;
    }
    /* Loop 3: overwrite cf[60:71], reading cf[k-2] */
    for(int k=61;k<=71;k++){
        double imV=x1i+k*x2i;
        double cosV=cos(imV);
        double denom=fabs((double)(k*k*k)*cosV);
        if(denom<1e-30) denom=1e-30;
        double absPrev=c_abs(cRe[k-2],cIm[k-2]);
        double fac=(double)k*absPrev/denom;
        cRe[k-1]=zr*fac; cIm[k-1]=zi*fac;
    }
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* poly_102_serendipity: clamped recurrence version of poly_102.
 * The original overflows float32 (~k! growth). Clamping |cf[k]| to 1e30
 * creates a different but visually interesting pattern. */
static void poly_102_serendipity(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    #define CLAMP102 1e30
    /* cf[0] = 1000 * (t1 + t2)**2 */
    double sr = x1r + x2r, si = x1i + x2i;
    double s2r, s2i;
    c_mul(sr, si, sr, si, &s2r, &s2i);
    cRe[0] = 1000.0 * s2r; cIm[0] = 1000.0 * s2i;

    for (int k = 1; k < 20; k++) {
        double kp1 = (double)(k + 1);
        double pr = kp1 * cRe[k-1], pi_ = kp1 * cIm[k-1];
        double ar = kp1 * x1r, ai = kp1 * x1i;
        double snr, sni; c_sin(ar, ai, &snr, &sni);
        double br = kp1 * x2r, bi = kp1 * x2i;
        double csr, csi; c_cos(br, bi, &csr, &csi);
        cRe[k] = pr + snr + csr;
        cIm[k] = pi_ + sni + csi;
        double m = c_abs(cRe[k], cIm[k]);
        if (m > CLAMP102) { double s = CLAMP102/m; cRe[k]*=s; cIm[k]*=s; }
    }
    for (int k = 20; k < 40; k++) {
        double kp1 = (double)(k + 1);
        double pr = kp1 * cRe[k-1], pi_ = kp1 * cIm[k-1];
        double ar = kp1 * x1r, ai = kp1 * x1i;
        double snr, sni; c_sin(ar, ai, &snr, &sni);
        double br = kp1 * x2r, bi = kp1 * x2i;
        double csr, csi; c_cos(br, bi, &csr, &csi);
        cRe[k] = pr - snr - csr;
        cIm[k] = pi_ - sni - csi;
        double m = c_abs(cRe[k], cIm[k]);
        if (m > CLAMP102) { double s = CLAMP102/m; cRe[k]*=s; cIm[k]*=s; }
    }
    for (int k = 40; k < 60; k++) {
        double kp1 = (double)(k + 1);
        double pr = kp1 * cRe[k-1], pi_ = kp1 * cIm[k-1];
        double t12r, t12i; c_mul(x1r, x1i, x2r, x2i, &t12r, &t12i);
        double ar = kp1 * t12r, ai = kp1 * t12i;
        double snr, sni; c_sin(ar, ai, &snr, &sni);
        double csr, csi; c_cos(ar, ai, &csr, &csi);
        cRe[k] = pr + snr + csr;
        cIm[k] = pi_ + sni + csi;
        double m = c_abs(cRe[k], cIm[k]);
        if (m > CLAMP102) { double s = CLAMP102/m; cRe[k]*=s; cIm[k]*=s; }
    }
    for (int k = 60; k < 70; k++) {
        double kp1 = (double)(k + 1);
        double pr = kp1 * cRe[k-1], pi_ = kp1 * cIm[k-1];
        double t12r, t12i; c_mul(x1r, x1i, x2r, x2i, &t12r, &t12i);
        double ar = kp1 * t12r, ai = kp1 * t12i;
        double snr, sni; c_sin(ar, ai, &snr, &sni);
        double csr, csi; c_cos(ar, ai, &csr, &csi);
        cRe[k] = pr - snr - csr;
        cIm[k] = pi_ - sni - csi;
        double m = c_abs(cRe[k], cIm[k]);
        if (m > CLAMP102) { double s = CLAMP102/m; cRe[k]*=s; cIm[k]*=s; }
    }
    /* cf[70] = |cf[69]| + angle(t1) - angle(t2) + real(t1*t2) - imag(conj(t1)*t2) */
    double abs69 = c_abs(cRe[69], cIm[69]);
    double ang1 = c_arg(x1r, x1i), ang2 = c_arg(x2r, x2i);
    double t12r, t12i; c_mul(x1r, x1i, x2r, x2i, &t12r, &t12i);
    double ct12r, ct12i; c_mul(x1r, -x1i, x2r, x2i, &ct12r, &ct12i);
    cRe[70] = abs69 + ang1 - ang2 + t12r - ct12i;
    cIm[70] = 0;
    #undef CLAMP102

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* poly_117: loop + whole-array cf *= expr + slice augassign
 * cf[k-1] = (t1+t2)^(k-1) + (-1)^k * exp(i*k*pi/71) * k^(1/3)
 * cf *= (1 + log(|cf|+1) / (1 + |t1*t2|))
 * cf[0:10] += (t1^2+t2^2)^(1/3)
 * cf[61:71] *= exp(-i*angle(t1)) */
static void poly_117_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double sr = x1r + x2r, si = x1i + x2i;
    double t12r, t12i; c_mul(x1r, x1i, x2r, x2i, &t12r, &t12i);
    double abs_t12 = c_abs(t12r, t12i);

    /* cf[k-1] = (t1+t2)^(k-1) + (-1)^k * exp(i*k*pi/71) * k^(1/3) */
    for (int k = 1; k <= 71; k++) {
        double pr, pi_;
        c_powr(sr, si, (double)(k - 1), &pr, &pi_);
        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        double ang = (double)k * M_PI / 71.0;
        double er = sign * cos(ang), ei = sign * sin(ang);
        double cbrtk = cbrt((double)k);
        cRe[k-1] = pr + er * cbrtk;
        cIm[k-1] = pi_ + ei * cbrtk;
    }

    /* cf *= (1 + log(|cf|+1) / (1 + |t1*t2|)) — element-wise */
    for (int i = 0; i < 71; i++) {
        double m = c_abs(cRe[i], cIm[i]);
        double fac = 1.0 + log(m + 1.0) / (1.0 + abs_t12);
        cRe[i] *= fac;
        cIm[i] *= fac;
    }

    /* cf[0:10] += (t1^2+t2^2)^(1/3) */
    double t1sq_r, t1sq_i; c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    double sumr = t1sq_r + t2sq_r, sumi = t1sq_i + t2sq_i;
    double cbr, cbi; c_powr(sumr, sumi, 1.0/3.0, &cbr, &cbi);
    for (int i = 0; i < 10; i++) {
        cRe[i] += cbr;
        cIm[i] += cbi;
    }

    /* cf[61:71] *= exp(-i*angle(t1)) */
    double ang1 = c_arg(x1r, x1i);
    double expr = cos(-ang1), expi = sin(-ang1);
    for (int i = 61; i < 71; i++) {
        double tr = cRe[i]*expr - cIm[i]*expi;
        cIm[i] = cRe[i]*expi + cIm[i]*expr;
        cRe[i] = tr;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* poly_118: primes array used in scalar sums and slice multiplications.
 * Transpiler passes array to c_mul — needs hand-written element-wise loops. */
static void poly_118_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    static const double primes[] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53};
    /* np.sum(primes) = 381, np.sum(primes[:8]) = 77 */
    double sumP = 381.0, sumP8 = 77.0;

    /* f1 = t1 * sumP + t2 */
    double f1r = x1r * sumP + x2r, f1i = x1i * sumP + x2i;
    /* f2 = t2 * sumP8 + conj(t1) */
    double f2r = x2r * sumP8 + x1r, f2i = x2i * sumP8 - x1i;

    /* cf[0:16] = primes[:16] * (t1 - t2) */
    double dr = x1r - x2r, di = x1i - x2i;
    for (int i = 0; i < 16; i++) {
        cRe[i] = primes[i] * dr;
        cIm[i] = primes[i] * di;
    }

    /* cf[16:32] = f1^2 - f2^2 */
    double f1sq_r, f1sq_i; c_mul(f1r, f1i, f1r, f1i, &f1sq_r, &f1sq_i);
    double f2sq_r, f2sq_i; c_mul(f2r, f2i, f2r, f2i, &f2sq_r, &f2sq_i);
    double diffr = f1sq_r - f2sq_r, diffi = f1sq_i - f2sq_i;
    for (int i = 16; i < 32; i++) {
        cRe[i] = diffr;
        cIm[i] = diffi;
    }

    /* cf[32:48] = (t1^3 - t2^3) * (primes[:16] - f1) */
    double t1cu_r, t1cu_i; c_mul(x1r, x1i, x1r, x1i, &t1cu_r, &t1cu_i);
    c_mul(t1cu_r, t1cu_i, x1r, x1i, &t1cu_r, &t1cu_i);
    double t2cu_r, t2cu_i; c_mul(x2r, x2i, x2r, x2i, &t2cu_r, &t2cu_i);
    c_mul(t2cu_r, t2cu_i, x2r, x2i, &t2cu_r, &t2cu_i);
    double cubdr = t1cu_r - t2cu_r, cubdi = t1cu_i - t2cu_i;
    for (int i = 0; i < 16; i++) {
        double pr = primes[i] - f1r, pi_ = -f1i;
        double rr, ri; c_mul(cubdr, cubdi, pr, pi_, &rr, &ri);
        cRe[32 + i] = rr;
        cIm[32 + i] = ri;
    }

    /* cf[48:64] = (primes[:16] * t1^2 + t2^3) - t1 */
    double t1sq_r2, t1sq_i2; c_mul(x1r, x1i, x1r, x1i, &t1sq_r2, &t1sq_i2);
    for (int i = 0; i < 16; i++) {
        cRe[48 + i] = primes[i] * t1sq_r2 + t2cu_r - x1r;
        cIm[48 + i] = primes[i] * t1sq_i2 + t2cu_i - x1i;
    }

    /* cf[64:70] = sin(cf[0:6]*t2) + cos(cf[0:6]*t1) */
    for (int i = 0; i < 6; i++) {
        double ar, ai; c_mul(cRe[i], cIm[i], x2r, x2i, &ar, &ai);
        double snr, sni; c_sin(ar, ai, &snr, &sni);
        double br, bi; c_mul(cRe[i], cIm[i], x1r, x1i, &br, &bi);
        double csr, csi; c_cos(br, bi, &csr, &csi);
        cRe[64 + i] = snr + csr;
        cIm[64 + i] = sni + csi;
    }

    /* cf[70] = prod(primes[:9]) = 2*3*5*7*11*13*17*19*23 = 223092870 */
    cRe[70] = 223092870.0;
    cIm[70] = 0;

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_120_hand ----
 * Python:
 *   theta = angle(t1) * angle(t2)
 *   mult_factors[i] = (i%2==0) ? 1 : -1   (for i=0..69)
 *   cf[0:10]  = (k+1)*t1^2 - (10-k)*t2^2   for k=0..9
 *   cf[10:40] = (k%2)*|t1| + (k%3)*|t2|*exp(k/5*theta*i)  for k=11..40 (1-indexed)
 *   cf[40:60] = ((k+41) + log(|theta|+1)) * conj(t1) * 5 * mult_factors[k]  for k=0..19
 *   cf[60:70] = (k+61) - (k+1)*t2 - (-5)   for k=0..9   [sum = -5]
 *   cf[70]    = 2556 / 15!  ≈ 1.954e-9
 */
static void poly_120_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double theta = c_arg(x1r, x1i) * c_arg(x2r, x2i);
    double abs_t1 = c_abs(x1r, x1i);
    double abs_t2 = c_abs(x2r, x2i);

    /* t1^2, t2^2 */
    double t1sq_r, t1sq_i; c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);

    /* conj(t1) */
    double conj_t1r = x1r, conj_t1i = -x1i;

    /* cf[0:10] = (k+1)*t1^2 - (10-k)*t2^2  for k=0..9 */
    for (int k = 0; k < 10; k++) {
        double a = (double)(k + 1);
        double b = (double)(10 - k);
        cRe[k] = a * t1sq_r - b * t2sq_r;
        cIm[k] = a * t1sq_i - b * t2sq_i;
    }

    /* cf[10:40]: for k=11..40 (1-indexed), cf[k-1] = (k%2)*|t1| + (k%3)*|t2|*exp(k/5*theta*i) */
    for (int k = 11; k <= 40; k++) {
        double base_r = (double)(k % 2) * abs_t1;
        double base_i = 0.0;
        /* (k%3)*|t2|*exp(k/5*theta*i) */
        double angle = ((double)k / 5.0) * theta;
        double er, ei; c_exp2(0.0, angle, &er, &ei);
        double scale = (double)(k % 3) * abs_t2;
        cRe[k - 1] = base_r + scale * er;
        cIm[k - 1] = base_i + scale * ei;
    }

    /* cf[40:60] = ((k+41) + log(|theta|+1)) * conj(t1) * 5 * mult_factors[k]  for k=0..19
     * mult_factors[k] = (k%2==0) ? 1 : -1 */
    double log_term = log(fabs(theta) + 1.0);
    for (int k = 0; k < 20; k++) {
        double scalar = ((double)(k + 41) + log_term) * 5.0;
        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        scalar *= sign;
        cRe[40 + k] = scalar * conj_t1r;
        cIm[40 + k] = scalar * conj_t1i;
    }

    /* cf[60:70] = (k+61) - (k+1)*t2 - (-5)  for k=0..9
     * The constant sum: sum((arange(1,11)) * mult_factors[10:20])
     *   mult_factors[10:20] = [1,-1,1,-1,1,-1,1,-1,1,-1]
     *   1*1 - 2 + 3 - 4 + 5 - 6 + 7 - 8 + 9 - 10 = -5  */
    for (int k = 0; k < 10; k++) {
        double real_part = (double)(k + 61) - (double)(k + 1) * x2r - (-5.0);
        double imag_part = -(double)(k + 1) * x2i;
        cRe[60 + k] = real_part;
        cIm[60 + k] = imag_part;
    }

    /* cf[70] = 2556 / 15! = 2556 / 1307674368000 */
    cRe[70] = 2556.0 / 1307674368000.0;
    cIm[70] = 0.0;

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_128_hand ----
 * Python:
 *   for k=1..71: cf[k-1] = (-1)^k * (t1^k + conj(t2)^(71-k)) * (72-k)
 */
static void poly_128_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* conj(t2) */
    double ct2r = x2r, ct2i = -x2i;

    /* Build t1^k incrementally and conj(t2)^(71-k) via c_powr */
    double t1pk_r = x1r, t1pk_i = x1i; /* t1^1 */
    for (int k = 1; k <= 71; k++) {
        /* t1^k (already computed for current k) */
        /* conj(t2)^(71-k) */
        double ct2p_r, ct2p_i;
        int exp2 = 71 - k;
        if (exp2 == 0) {
            ct2p_r = 1.0; ct2p_i = 0.0;
        } else {
            c_powr(ct2r, ct2i, (double)exp2, &ct2p_r, &ct2p_i);
        }

        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        double scale = sign * (double)(72 - k);
        double sum_r = t1pk_r + ct2p_r;
        double sum_i = t1pk_i + ct2p_i;
        cRe[k - 1] = scale * sum_r;
        cIm[k - 1] = scale * sum_i;

        /* advance t1^(k+1) = t1^k * t1 */
        if (k < 71) {
            double nr, ni;
            c_mul(t1pk_r, t1pk_i, x1r, x1i, &nr, &ni);
            t1pk_r = nr; t1pk_i = ni;
        }
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* poly_125: 5 list-comp slices + 1 loop + cf[70]=1.
 * Python source has bug: cf[61:70] = [... range(61,71)] — 9 slots, 10 values → crash.
 * Hand-written to match the fixed (intended) behavior. */
static void poly_125_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);

    /* cf[0:15] = [(-1)**j * j**2 * (|t1| + |t2|) for j in range(1,16)] */
    for (int j = 1; j <= 15; j++) {
        double sign = (j % 2 == 0) ? 1.0 : -1.0;
        cRe[j-1] = sign * (double)(j*j) * (abs1 + abs2);
    }

    /* cf[15:30] = [(-1)**(k+1) * k**3 * angle(t1 + 1j*t2) for k in range(16,31)] */
    double sr = x1r - x2i, si = x1i + x2r;  /* t1 + 1j*t2 */
    double ang_s = c_arg(sr, si);
    for (int k = 16; k <= 30; k++) {
        double sign = ((k+1) % 2 == 0) ? 1.0 : -1.0;
        cRe[k-1] = sign * (double)(k*k*k) * ang_s;
    }

    /* cf[30:45] = [(-1)**(r+1)*cos(r*t1) + sin(r*t2) for r in range(31,46)] */
    for (int r = 31; r <= 45; r++) {
        double sign = ((r+1) % 2 == 0) ? 1.0 : -1.0;
        double ar = (double)r*x1r, ai = (double)r*x1i;
        double cosr, cosi; c_cos(ar, ai, &cosr, &cosi);
        double br = (double)r*x2r, bi = (double)r*x2i;
        double sinr, sini; c_sin(br, bi, &sinr, &sini);
        cRe[r-1] = sign*cosr + sinr;
        cIm[r-1] = sign*cosi + sini;
    }

    /* for s in range(46,61): cf[s] = (-1)**s * s**2 * conj(t1) * conj(t2) */
    double cpr, cpi; c_mul(x1r, -x1i, x2r, -x2i, &cpr, &cpi);
    for (int s = 46; s <= 60; s++) {
        double sign = (s % 2 == 0) ? 1.0 : -1.0;
        double fac = sign * (double)(s*s);
        cRe[s] = fac * cpr;
        cIm[s] = fac * cpi;
    }

    /* cf[61:71] = [n**3 * log(|t1*t2| + 1) for n in range(61,71)]
     * Slice indices 61..70, list values n=61..70. cf[61+i] = (61+i)^3 * logval. */
    double t12r, t12i; c_mul(x1r, x1i, x2r, x2i, &t12r, &t12i);
    double logval = log(c_abs(t12r, t12i) + 1.0);
    for (int i = 0; i < 10; i++) {
        int n = 61 + i;
        cRe[61 + i] = (double)(n*n*n) * logval;
    }

    /* cf[70] = 1 */
    cRe[70] = 1.0;

    /* Cap coefficient magnitudes at 1e10 — serendipity version.
     * Original poly_125 has ~16 orders dynamic range which float32 can't handle.
     * Capping creates a different but potentially interesting polynomial. */
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; continue; }
        double m = c_abs(cRe[i], cIm[i]);
        if (m > 1e10) { double s = 1e10 / m; cRe[i] *= s; cIm[i] *= s; }
    }
}
