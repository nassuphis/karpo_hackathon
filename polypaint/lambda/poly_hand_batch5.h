/*
 * Hand-written polynomial coefficient functions: poly_188 through poly_192.
 *
 * Each function has the same signature as the transpiled versions:
 *   void func(double x1r, double x1i, double x2r, double x2i,
 *             double *cRe, double *cIm, int *nCoeffs)
 *
 * Requires: c_mul, c_abs, c_arg, c_exp2, c_sin, c_cos, c_powr, c_log helpers.
 */

/* ---- poly_188_hand ----
 * Python:
 *   k = np.arange(71)
 *   cf = (np.real(t1)**k + np.imag(t2) * k) * (-1)**k
 *      + np.log(np.abs(t1 + t2 * k) + 1)
 *      + np.sin(t1 * k) * np.cos(t2 * k)
 *      + (np.angle(t1) * k - np.angle(t2)) * 1j
 */
static void poly_188_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double angle_t1 = c_arg(x1r, x1i);
    double angle_t2 = c_arg(x2r, x2i);

    /* t1_real_pow[k] = np.real(t1)**k  (real number raised to int power) */
    double t1r_pow = 1.0;  /* x1r^0 */

    for (int k = 0; k < 71; k++) {
        double sign = (k % 2 == 0) ? 1.0 : -1.0;

        /* Term 1: (np.real(t1)**k + np.imag(t2) * k) * (-1)**k */
        double term1 = (t1r_pow + x2i * (double)k) * sign;

        /* Term 2: np.log(np.abs(t1 + t2 * k) + 1)  — real log of real positive */
        double sum_r = x1r + x2r * (double)k;
        double sum_i = x1i + x2i * (double)k;
        double abs_sum = c_abs(sum_r, sum_i);
        double term2 = log(abs_sum + 1.0);

        /* Term 3: np.sin(t1 * k) * np.cos(t2 * k)  — complex * complex */
        double s1r, s1i, c2r, c2i;
        c_sin(x1r * (double)k, x1i * (double)k, &s1r, &s1i);
        c_cos(x2r * (double)k, x2i * (double)k, &c2r, &c2i);
        double t3r, t3i;
        c_mul(s1r, s1i, c2r, c2i, &t3r, &t3i);

        /* Term 4: (np.angle(t1) * k - np.angle(t2)) * 1j — purely imaginary */
        double term4_im = angle_t1 * (double)k - angle_t2;

        cRe[k] = term1 + term2 + t3r;
        cIm[k] = t3i + term4_im;

        t1r_pow *= x1r;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_189_hand ----
 * Python:
 *   degrees = np.arange(71)
 *   cf = t1**(70 - degrees) * (np.cos(degrees) + 1j * np.sin(degrees))
 *      + t2**degrees * (np.cos(degrees) - 1j * np.sin(degrees))
 *
 * Note: np.cos(degrees) and np.sin(degrees) are real cos/sin of integer values.
 * t1**(70-k) and t2**k are complex powers.
 */
static void poly_189_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Precompute t1 powers: t1pow[k] = t1^k for k=0..70 */
    double t1pR[71], t1pI[71];
    t1pR[0] = 1.0; t1pI[0] = 0.0;
    for (int k = 1; k <= 70; k++) {
        c_mul(t1pR[k-1], t1pI[k-1], x1r, x1i, &t1pR[k], &t1pI[k]);
    }

    /* Precompute t2 powers: t2pow[k] = t2^k for k=0..70 */
    double t2pR[71], t2pI[71];
    t2pR[0] = 1.0; t2pI[0] = 0.0;
    for (int k = 1; k <= 70; k++) {
        c_mul(t2pR[k-1], t2pI[k-1], x2r, x2i, &t2pR[k], &t2pI[k]);
    }

    for (int k = 0; k < 71; k++) {
        double ck = cos((double)k);
        double sk = sin((double)k);

        /* t1^(70-k) * (cos(k) + i*sin(k)) */
        int exp1 = 70 - k;
        double aR, aI;
        c_mul(t1pR[exp1], t1pI[exp1], ck, sk, &aR, &aI);

        /* t2^k * (cos(k) - i*sin(k)) */
        double bR, bI;
        c_mul(t2pR[k], t2pI[k], ck, -sk, &bR, &bI);

        cRe[k] = aR + bR;
        cIm[k] = aI + bI;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_190_hand ----
 * Python:
 *   exponents = np.arange(71)
 *   cf = t1**exponents + (-1)**exponents * t2**(exponents + 1)
 */
static void poly_190_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Precompute t1 powers: t1^k for k=0..70 */
    double t1pR[71], t1pI[71];
    t1pR[0] = 1.0; t1pI[0] = 0.0;
    for (int k = 1; k <= 70; k++) {
        c_mul(t1pR[k-1], t1pI[k-1], x1r, x1i, &t1pR[k], &t1pI[k]);
    }

    /* Precompute t2 powers: t2^k for k=0..71 (need up to k+1=71) */
    double t2pR[72], t2pI[72];
    t2pR[0] = 1.0; t2pI[0] = 0.0;
    for (int k = 1; k <= 71; k++) {
        c_mul(t2pR[k-1], t2pI[k-1], x2r, x2i, &t2pR[k], &t2pI[k]);
    }

    for (int k = 0; k < 71; k++) {
        double sign = (k % 2 == 0) ? 1.0 : -1.0;

        /* t1^k + (-1)^k * t2^(k+1) */
        cRe[k] = t1pR[k] + sign * t2pR[k + 1];
        cIm[k] = t1pI[k] + sign * t2pI[k + 1];
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_191_hand ----
 * Python:
 *   exponents = np.arange(71)
 *   cf = (t1**exponents) * np.sin(exponents * np.angle(t2))
 *      + (np.conj(t2)**exponents) * np.cos(exponents * np.real(t1))
 *
 * np.angle(t2) is real, np.real(t1) is real,
 * so np.sin(...) and np.cos(...) produce real scalars per element.
 */
static void poly_191_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double angle_t2 = c_arg(x2r, x2i);
    /* conj(t2) = (x2r, -x2i) */
    double ct2r = x2r, ct2i = -x2i;

    /* Precompute t1 powers */
    double t1pR[71], t1pI[71];
    t1pR[0] = 1.0; t1pI[0] = 0.0;
    for (int k = 1; k <= 70; k++) {
        c_mul(t1pR[k-1], t1pI[k-1], x1r, x1i, &t1pR[k], &t1pI[k]);
    }

    /* Precompute conj(t2) powers */
    double ct2pR[71], ct2pI[71];
    ct2pR[0] = 1.0; ct2pI[0] = 0.0;
    for (int k = 1; k <= 70; k++) {
        c_mul(ct2pR[k-1], ct2pI[k-1], ct2r, ct2i, &ct2pR[k], &ct2pI[k]);
    }

    for (int k = 0; k < 71; k++) {
        double s = sin((double)k * angle_t2);  /* real */
        double c = cos((double)k * x1r);       /* real */

        /* (t1^k) * sin(k * angle(t2)) + (conj(t2)^k) * cos(k * real(t1)) */
        cRe[k] = t1pR[k] * s + ct2pR[k] * c;
        cIm[k] = t1pI[k] * s + ct2pI[k] * c;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_192_hand ----
 * Python:
 *   degrees = np.arange(71)
 *   cf = (t1**degrees) * (np.conj(t2)**(degrees % 7)) * (-1)**(degrees // 6) * (1 + degrees / 70)
 */
static void poly_192_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* conj(t2) = (x2r, -x2i) */
    double ct2r = x2r, ct2i = -x2i;

    /* Precompute t1 powers: t1^k for k=0..70 */
    double t1pR[71], t1pI[71];
    t1pR[0] = 1.0; t1pI[0] = 0.0;
    for (int k = 1; k <= 70; k++) {
        c_mul(t1pR[k-1], t1pI[k-1], x1r, x1i, &t1pR[k], &t1pI[k]);
    }

    /* Precompute conj(t2) powers: conj(t2)^k for k=0..6 (degrees % 7) */
    double ct2pR[7], ct2pI[7];
    ct2pR[0] = 1.0; ct2pI[0] = 0.0;
    for (int k = 1; k <= 6; k++) {
        c_mul(ct2pR[k-1], ct2pI[k-1], ct2r, ct2i, &ct2pR[k], &ct2pI[k]);
    }

    for (int k = 0; k < 71; k++) {
        double sign = ((k / 6) % 2 == 0) ? 1.0 : -1.0;  /* (-1)^(k // 6) */
        double scale = (1.0 + (double)k / 70.0) * sign;
        int mod7 = k % 7;

        /* (t1^k) * (conj(t2)^(k%7)) * scale */
        double pR, pI;
        c_mul(t1pR[k], t1pI[k], ct2pR[mod7], ct2pI[mod7], &pR, &pI);

        cRe[k] = pR * scale;
        cIm[k] = pI * scale;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}
