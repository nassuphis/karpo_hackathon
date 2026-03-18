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

/* ---- poly_21_hand ----
 * Lagrange-basis construction using 51st roots of unity.
 */
static void poly_21_hand(double x1r, double x1i, double x2r, double x2i,
                         double *cRe, double *cIm, int *nCoeffs) {
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
                         double *cRe, double *cIm, int *nCoeffs) {
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
                         double *cRe, double *cIm, int *nCoeffs) {
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
                         double *cRe, double *cIm, int *nCoeffs) {
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
                         double *cRe, double *cIm, int *nCoeffs) {
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
                         double *cRe, double *cIm, int *nCoeffs) {
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
                         double *cRe, double *cIm, int *nCoeffs) {
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
                         double *cRe, double *cIm, int *nCoeffs) {
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
                          double *cRe, double *cIm, int *nCoeffs) {
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
                         double *cRe, double *cIm, int *nCoeffs) {
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
 * The broken transpiled version of poly_42. The transpiler replaced the loop
 * variable k (from np.arange) with 0 in the sin/cos arguments, producing
 * |t1|*sin(0*angle(t1)) and Re(t2)*cos(0*Im(t2)) — a beautiful accident.
 * Preserved for posterity because the resulting root image is gorgeous.
 */
static void poly_42_serendipity(double x1r, double x1i, double x2r, double x2i,
                                double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double absT1 = c_abs(x1r, x1i);
    double angT1 = c_arg(x1r, x1i);
    /* The "bug": k=0 for all elements, so sin(0)=0, making cf[0:35] all zero */
    double sv = sin(0 * angT1);  /* always 0 */
    for (int k = 0; k < 35; k++) {
        cRe[k] = absT1 * sv;
        cIm[k] = 0;
    }
    /* cos(0*Im(t2)) = cos(0) = 1, so cf[35:70] = Re(t2) * 1 = Re(t2) */
    double cv = cos(0 * x2i);  /* always 1 */
    for (int k = 0; k < 35; k++) {
        cRe[35 + k] = x2r * cv;
        cIm[35 + k] = 0;
    }
    /* cf[70] = t1*t2 + i*sum(log(|cf[0:70]|+1)) */
    double mr, mi;
    c_mul(x1r, x1i, x2r, x2i, &mr, &mi);
    double logsum = 0;
    for (int k = 0; k < 70; k++) logsum += log(c_abs(cRe[k], cIm[k]) + 1);
    cRe[70] = mr - logsum;  /* real part: Re(t1*t2) - logsum (from i*logsum imag) */
    cIm[70] = mi + logsum;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_42_hand ----
 * Correct version:
 * cf[0:35] = |t1| * sin(arange(1,36) * angle(t1))
 * cf[35:70] = Re(t2) * cos(arange(1,36) * Im(t2))
 * cf[70] = t1*t2 + i*sum(log(|cf[0:70]|+1))
 */
static void poly_42_hand(double x1r, double x1i, double x2r, double x2i,
                         double *cRe, double *cIm, int *nCoeffs) {
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
