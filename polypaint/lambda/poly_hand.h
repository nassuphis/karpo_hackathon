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
 */

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
                         double *cRe, double *cIm, int *nCoeffs) {
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

/* ---- poly_33_hand ----
 * Python:
 *   f = lambda z, n: z**n - 1
 *   cf[0:35]  = [Re(f(t1,n)) - Im(f(t2,n)) for n in range(1,36)]
 *   cf[35:70] = [log(|f(t2,n)|) + angle(f(t1,n)) + sin(|f(t1,n)|) + cos(angle(f(t2,n))) for n in range(1,36)]
 *   cf[70]    = prod(cf[0:70])
 *   NaN/inf → 0
 */
static void poly_33_hand(double x1r, double x1i, double x2r, double x2i,
                         double *cRe, double *cIm, int *nCoeffs) {
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
                         double *cRe, double *cIm, int *nCoeffs) {
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
                         double *cRe, double *cIm, int *nCoeffs) {
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
