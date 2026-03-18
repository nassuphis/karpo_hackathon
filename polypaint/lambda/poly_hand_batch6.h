/*
 * Hand-written polynomial coefficient functions: poly_193 through poly_197.
 *
 * Translated from lambda/poly200.py.
 * Requires: c_mul, c_div, c_sin, c_cos, c_log, c_abs, c_arg, c_exp2, c_powr, c_powc helpers.
 */

/* ---- poly_193_hand ----
 * Python:
 *   j = np.arange(71)
 *   cf = (t1**j * t2**(70 - j)) * ((-1)**j + np.real(t1) * np.imag(t2) / (j + 1))
 */
static void poly_193_hand(double x1r, double x1i, double x2r, double x2i,
                           double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Build t1^j powers: pow1[j] = t1^j */
    double pow1r[71], pow1i[71];
    pow1r[0] = 1.0; pow1i[0] = 0.0;
    for (int j = 1; j < 71; j++) {
        c_mul(pow1r[j-1], pow1i[j-1], x1r, x1i, &pow1r[j], &pow1i[j]);
    }

    /* Build t2^j powers: pow2[j] = t2^j */
    double pow2r[71], pow2i[71];
    pow2r[0] = 1.0; pow2i[0] = 0.0;
    for (int j = 1; j < 71; j++) {
        c_mul(pow2r[j-1], pow2i[j-1], x2r, x2i, &pow2r[j], &pow2i[j]);
    }

    double re_t1 = x1r;
    double im_t2 = x2i;
    double scale_base = re_t1 * im_t2;  /* np.real(t1) * np.imag(t2) */

    for (int j = 0; j < 71; j++) {
        /* t1^j * t2^(70-j) */
        double pr, pi;
        c_mul(pow1r[j], pow1i[j], pow2r[70 - j], pow2i[70 - j], &pr, &pi);

        /* (-1)^j + real(t1)*imag(t2) / (j+1) */
        double sign = (j % 2 == 0) ? 1.0 : -1.0;
        double factor = sign + scale_base / (j + 1.0);

        /* multiply complex by real scalar */
        cRe[j] = pr * factor;
        cIm[j] = pi * factor;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_194_hand ----
 * Python:
 *   exponents = np.arange(71)
 *   cf = (t1**exponents) * np.real(t2)
 *      + (np.conj(t1)**exponents) * np.imag(t2)
 *      - np.log(np.abs(t1) + 1)**exponents
 *      + np.sin(t1 * exponents)
 *      - np.cos(t2 * exponents)
 */
static void poly_194_hand(double x1r, double x1i, double x2r, double x2i,
                           double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double re_t2 = x2r;
    double im_t2 = x2i;

    /* t1^j powers */
    double pow1r[71], pow1i[71];
    pow1r[0] = 1.0; pow1i[0] = 0.0;
    for (int j = 1; j < 71; j++) {
        c_mul(pow1r[j-1], pow1i[j-1], x1r, x1i, &pow1r[j], &pow1i[j]);
    }

    /* conj(t1)^j powers */
    double cx1r = x1r, cx1i = -x1i;
    double cpow1r[71], cpow1i[71];
    cpow1r[0] = 1.0; cpow1i[0] = 0.0;
    for (int j = 1; j < 71; j++) {
        c_mul(cpow1r[j-1], cpow1i[j-1], cx1r, cx1i, &cpow1r[j], &cpow1i[j]);
    }

    /* log(|t1| + 1) — this is a real number */
    double log_abs_t1 = log(c_abs(x1r, x1i) + 1.0);
    /* log_abs_t1^j powers (real) */
    double lpow[71];
    lpow[0] = 1.0;
    for (int j = 1; j < 71; j++) {
        lpow[j] = lpow[j-1] * log_abs_t1;
    }

    for (int j = 0; j < 71; j++) {
        /* term1: t1^j * real(t2) */
        double t1r = pow1r[j] * re_t2;
        double t1i = pow1i[j] * re_t2;

        /* term2: conj(t1)^j * imag(t2) */
        double t2r = cpow1r[j] * im_t2;
        double t2i = cpow1i[j] * im_t2;

        /* term3: -log(|t1|+1)^j  (real, subtracted) */
        double t3r = -lpow[j];
        double t3i = 0.0;

        /* term4: sin(t1 * j) — complex sin of complex arg */
        double argr = x1r * j, argi = x1i * j;
        double s4r, s4i;
        c_sin(argr, argi, &s4r, &s4i);

        /* term5: -cos(t2 * j) — complex cos of complex arg */
        double bargr = x2r * j, bargi = x2i * j;
        double c5r, c5i;
        c_cos(bargr, bargi, &c5r, &c5i);

        cRe[j] = t1r + t2r + t3r + s4r - c5r;
        cIm[j] = t1i + t2i + t3i + s4i - c5i;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_195_hand ----
 * Python:
 *   degrees = np.arange(71)
 *   cf = (t1**degrees) * np.sin(t2 * degrees) + (t2**degrees) * np.cos(t1 * degrees)
 */
static void poly_195_hand(double x1r, double x1i, double x2r, double x2i,
                           double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* t1^j powers */
    double pow1r[71], pow1i[71];
    pow1r[0] = 1.0; pow1i[0] = 0.0;
    for (int j = 1; j < 71; j++) {
        c_mul(pow1r[j-1], pow1i[j-1], x1r, x1i, &pow1r[j], &pow1i[j]);
    }

    /* t2^j powers */
    double pow2r[71], pow2i[71];
    pow2r[0] = 1.0; pow2i[0] = 0.0;
    for (int j = 1; j < 71; j++) {
        c_mul(pow2r[j-1], pow2i[j-1], x2r, x2i, &pow2r[j], &pow2i[j]);
    }

    for (int j = 0; j < 71; j++) {
        /* sin(t2 * j) — complex sin */
        double s_argr = x2r * j, s_argi = x2i * j;
        double sr, si;
        c_sin(s_argr, s_argi, &sr, &si);

        /* t1^j * sin(t2*j) */
        double a_r, a_i;
        c_mul(pow1r[j], pow1i[j], sr, si, &a_r, &a_i);

        /* cos(t1 * j) — complex cos */
        double c_argr = x1r * j, c_argi = x1i * j;
        double cr, ci;
        c_cos(c_argr, c_argi, &cr, &ci);

        /* t2^j * cos(t1*j) */
        double b_r, b_i;
        c_mul(pow2r[j], pow2i[j], cr, ci, &b_r, &b_i);

        cRe[j] = a_r + b_r;
        cIm[j] = a_i + b_i;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_196_hand ----
 * Python:
 *   exponents1 = np.arange(1, 72)       # j+1 for j in 0..70
 *   exponents2 = np.arange(71, 0, -1)   # 71-j for j in 0..70
 *   terms1 = t1**exponents1              # t1^(j+1)
 *   terms2 = (-1)**np.arange(71) * t2**exponents2   # (-1)^j * t2^(71-j)
 *   terms3 = np.sin(t1 * np.arange(71)) * np.cos(t2 * np.arange(1, 72))  # sin(t1*j)*cos(t2*(j+1))
 *   cf = terms1 + terms2 + terms3
 */
static void poly_196_hand(double x1r, double x1i, double x2r, double x2i,
                           double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* t1^j powers for j=0..71 (need up to j=71) */
    double pow1r[72], pow1i[72];
    pow1r[0] = 1.0; pow1i[0] = 0.0;
    for (int j = 1; j < 72; j++) {
        c_mul(pow1r[j-1], pow1i[j-1], x1r, x1i, &pow1r[j], &pow1i[j]);
    }

    /* t2^j powers for j=0..71 (need up to j=71) */
    double pow2r[72], pow2i[72];
    pow2r[0] = 1.0; pow2i[0] = 0.0;
    for (int j = 1; j < 72; j++) {
        c_mul(pow2r[j-1], pow2i[j-1], x2r, x2i, &pow2r[j], &pow2i[j]);
    }

    for (int j = 0; j < 71; j++) {
        /* term1: t1^(j+1) */
        double t1r = pow1r[j + 1];
        double t1i = pow1i[j + 1];

        /* term2: (-1)^j * t2^(71-j) */
        double sign = (j % 2 == 0) ? 1.0 : -1.0;
        int exp2 = 71 - j;
        double t2r = sign * pow2r[exp2];
        double t2i = sign * pow2i[exp2];

        /* term3: sin(t1 * j) * cos(t2 * (j+1)) — complex sin * complex cos */
        double s_argr = x1r * j, s_argi = x1i * j;
        double sr, si;
        c_sin(s_argr, s_argi, &sr, &si);

        double c_argr = x2r * (j + 1), c_argi = x2i * (j + 1);
        double cr, ci;
        c_cos(c_argr, c_argi, &cr, &ci);

        double t3r, t3i;
        c_mul(sr, si, cr, ci, &t3r, &t3i);

        cRe[j] = t1r + t2r + t3r;
        cIm[j] = t1i + t2i + t3i;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_197_hand ----
 * Python:
 *   j = np.arange(71)
 *   cf = (np.real(t1)**j * np.sin(j * np.angle(t2))
 *        + np.real(t2)**(70 - j) * np.cos((70 - j) * np.angle(t2)))
 *       + (np.imag(t1)**j * np.cos(j * np.angle(t2))
 *        - np.imag(t2)**(j / 2) * np.sin((70 - j) * np.angle(t1)))
 *   cf = cf * (np.log(np.abs(t1) + 1) * np.log(np.abs(t2) + 1))
 *
 * Note: all operations here are real**real and sin/cos of real args,
 * so cf is real-valued (imag = 0), then multiplied by a real scalar.
 * We use pow() for real**real since exponents can be fractional (j/2).
 */
static void poly_197_hand(double x1r, double x1i, double x2r, double x2i,
                           double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double re_t1 = x1r;
    double im_t1 = x1i;
    double re_t2 = x2r;
    double im_t2 = x2i;

    double angle_t1 = c_arg(x1r, x1i);
    double angle_t2 = c_arg(x2r, x2i);

    double log_scale = log(c_abs(x1r, x1i) + 1.0) * log(c_abs(x2r, x2i) + 1.0);

    for (int j = 0; j < 71; j++) {
        int k = 70 - j;

        /* np.real(t1)**j — real^int, use pow for generality */
        double re_t1_j = pow(re_t1, (double)j);

        /* np.real(t2)**(70-j) */
        double re_t2_k = pow(re_t2, (double)k);

        /* np.imag(t1)**j */
        double im_t1_j = pow(im_t1, (double)j);

        /* np.imag(t2)**(j/2) — fractional exponent */
        double im_t2_jhalf = pow(im_t2, j / 2.0);

        /* sin/cos with real args */
        double sin_j_at2 = sin(j * angle_t2);
        double cos_k_at2 = cos(k * angle_t2);
        double cos_j_at2 = cos(j * angle_t2);
        double sin_k_at1 = sin(k * angle_t1);

        double val = (re_t1_j * sin_j_at2 + re_t2_k * cos_k_at2)
                   + (im_t1_j * cos_j_at2 - im_t2_jhalf * sin_k_at1);

        /* cf is real, multiplied by real scalar */
        cRe[j] = val * log_scale;
        cIm[j] = 0.0;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}
