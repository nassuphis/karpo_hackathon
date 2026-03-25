/*
 * Hand-written polynomial coefficient functions — batch 3.
 *
 * poly_152, poly_153, poly_157, poly_164, poly_167
 *
 * Requires: c_mul, c_div, c_sin, c_cos, c_log, c_abs, c_arg, c_exp2,
 *           c_powr, c_powc helpers from sweep_cli.c
 */

/* ---- poly_152_hand ----
 * Python:
 *   k = np.arange(1, 72)
 *   cf = (t1**k + np.conj(t2)**k) * (-1)**k / (1 + k)
 *   71 coeffs, index 0..70 maps to k=1..71
 */
static void poly_152_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* conj(t2) */
    double ct2r = x2r, ct2i = -x2i;

    /* Accumulate t1^k and conj(t2)^k iteratively */
    double t1pk_r = 1.0, t1pk_i = 0.0;  /* t1^0 */
    double ct2pk_r = 1.0, ct2pk_i = 0.0; /* conj(t2)^0 */

    for (int k = 1; k <= 71; k++) {
        /* t1^k = t1^(k-1) * t1 */
        double nr, ni;
        c_mul(t1pk_r, t1pk_i, x1r, x1i, &nr, &ni);
        t1pk_r = nr; t1pk_i = ni;

        /* conj(t2)^k */
        double cr, ci;
        c_mul(ct2pk_r, ct2pk_i, ct2r, ct2i, &cr, &ci);
        ct2pk_r = cr; ct2pk_i = ci;

        /* (t1^k + conj(t2)^k) * (-1)^k / (1 + k) */
        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        double denom = 1.0 + k;
        cRe[k - 1] = (t1pk_r + ct2pk_r) * sign / denom;
        cIm[k - 1] = (t1pk_i + ct2pk_i) * sign / denom;
    }

    for (int i = 0; i < *nCoeffs; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_153_hand ----
 * Python:
 *   for k in range(1, 72):
 *       if k <= 35:
 *           cf[k-1] = ((np.real(t1)**k + np.imag(t2)**k) * (-1)**k) / k
 *       else:
 *           cf[k-1] = (np.sin(t1 * k) + np.cos(t2 * k)) * (-1)**k / (71 - k + 1)
 *   cf[70] = np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1)
 */
static void poly_153_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double re_t1 = x1r;
    double im_t2 = x2i;

    for (int k = 1; k <= 71; k++) {
        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        if (k <= 35) {
            /* (real(t1)^k + imag(t2)^k) * (-1)^k / k
             * These are real-valued powers of real numbers */
            double re_t1_k = pow(re_t1, (double)k);
            double im_t2_k = pow(im_t2, (double)k);
            cRe[k - 1] = (re_t1_k + im_t2_k) * sign / k;
            cIm[k - 1] = 0.0;
        } else {
            /* (np.sin(t1 * k) + np.cos(t2 * k)) * (-1)^k / (71 - k + 1)
             * t1*k and t2*k are complex */
            double t1k_r = x1r * k, t1k_i = x1i * k;
            double t2k_r = x2r * k, t2k_i = x2i * k;
            double sr, si, cr2, ci2;
            c_sin(t1k_r, t1k_i, &sr, &si);
            c_cos(t2k_r, t2k_i, &cr2, &ci2);
            double denom = 71.0 - k + 1.0;
            cRe[k - 1] = (sr + cr2) * sign / denom;
            cIm[k - 1] = (si + ci2) * sign / denom;
        }
    }

    /* cf[70] = np.log(np.abs(t1) + 1) + np.log(np.abs(t2) + 1)
     * This is a real value overwriting cf[70] */
    double abs_t1 = c_abs(x1r, x1i);
    double abs_t2 = c_abs(x2r, x2i);
    cRe[70] = log(abs_t1 + 1.0) + log(abs_t2 + 1.0);
    cIm[70] = 0.0;

    for (int i = 0; i < *nCoeffs; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_157_hand ----
 * Python:
 *   k = np.arange(71)   # k = 0..70
 *   cf = ((np.real(t1) + np.imag(t2))**(70 - k) + (np.real(t1) - np.imag(t2))**k) * (-1)**k / (k + 1)
 *   71 coeffs
 */
static void poly_157_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double a = x1r + x2i;  /* real(t1) + imag(t2) */
    double b = x1r - x2i;  /* real(t1) - imag(t2) */

    for (int k = 0; k <= 70; k++) {
        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        double term1 = pow(a, 70.0 - k);
        double term2 = pow(b, (double)k);
        /* Result is real since a and b are real */
        cRe[k] = (term1 + term2) * sign / (k + 1.0);
        cIm[k] = 0.0;
    }

    for (int i = 0; i < *nCoeffs; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_164_hand ----
 * Python:
 *   for k in range(1, 72):
 *       if k % 4 == 1:   cf[k-1] = t1**k + t2**(k-1) * np.sin(t1)
 *       elif k % 4 == 2: cf[k-1] = (t1 + t2)**k * np.cos(t2)
 *       elif k % 4 == 3: cf[k-1] = np.log(np.abs(t1) + 1) * t2**k
 *       else:             cf[k-1] = (t1 - t2)**k * np.sin(t1 * t2)
 *   cf[0] = t1 + 2 * t2
 *   cf[70] = t1**35 - t2**35 + 1j * t1 * t2
 */
static void poly_164_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Precompute sin(t1), cos(t2) — complex */
    double sin_t1_r, sin_t1_i;
    c_sin(x1r, x1i, &sin_t1_r, &sin_t1_i);

    double cos_t2_r, cos_t2_i;
    c_cos(x2r, x2i, &cos_t2_r, &cos_t2_i);

    /* log(|t1| + 1) — real scalar */
    double log_abs_t1 = log(c_abs(x1r, x1i) + 1.0);

    /* sin(t1 * t2) — complex */
    double t1t2_r, t1t2_i;
    c_mul(x1r, x1i, x2r, x2i, &t1t2_r, &t1t2_i);
    double sin_t1t2_r, sin_t1t2_i;
    c_sin(t1t2_r, t1t2_i, &sin_t1t2_r, &sin_t1t2_i);

    /* t1 + t2, t1 - t2 */
    double sum_r = x1r + x2r, sum_i = x1i + x2i;
    double dif_r = x1r - x2r, dif_i = x1i - x2i;

    /* Iterative powers */
    double t1pk_r = 1.0, t1pk_i = 0.0;     /* will hold t1^k */
    double t2pk_r = 1.0, t2pk_i = 0.0;     /* will hold t2^k */
    double sumpk_r = 1.0, sumpk_i = 0.0;   /* (t1+t2)^k */
    double difpk_r = 1.0, difpk_i = 0.0;   /* (t1-t2)^k */

    for (int k = 1; k <= 71; k++) {
        double nr, ni;

        /* Save t2^(k-1) before advancing */
        double t2prev_r = t2pk_r, t2prev_i = t2pk_i;

        /* Advance all powers to ^k */
        c_mul(t1pk_r, t1pk_i, x1r, x1i, &nr, &ni);
        t1pk_r = nr; t1pk_i = ni;

        c_mul(t2pk_r, t2pk_i, x2r, x2i, &nr, &ni);
        t2pk_r = nr; t2pk_i = ni;

        c_mul(sumpk_r, sumpk_i, sum_r, sum_i, &nr, &ni);
        sumpk_r = nr; sumpk_i = ni;

        c_mul(difpk_r, difpk_i, dif_r, dif_i, &nr, &ni);
        difpk_r = nr; difpk_i = ni;

        int mod = k % 4;
        if (mod == 1) {
            /* t1^k + t2^(k-1) * sin(t1) */
            double pr, pi;
            c_mul(t2prev_r, t2prev_i, sin_t1_r, sin_t1_i, &pr, &pi);
            cRe[k - 1] = t1pk_r + pr;
            cIm[k - 1] = t1pk_i + pi;
        } else if (mod == 2) {
            /* (t1+t2)^k * cos(t2) */
            double pr, pi;
            c_mul(sumpk_r, sumpk_i, cos_t2_r, cos_t2_i, &pr, &pi);
            cRe[k - 1] = pr;
            cIm[k - 1] = pi;
        } else if (mod == 3) {
            /* log(|t1|+1) * t2^k */
            cRe[k - 1] = log_abs_t1 * t2pk_r;
            cIm[k - 1] = log_abs_t1 * t2pk_i;
        } else {
            /* k % 4 == 0: (t1-t2)^k * sin(t1*t2) */
            double pr, pi;
            c_mul(difpk_r, difpk_i, sin_t1t2_r, sin_t1t2_i, &pr, &pi);
            cRe[k - 1] = pr;
            cIm[k - 1] = pi;
        }
    }

    /* cf[0] = t1 + 2 * t2 (overwrites) */
    cRe[0] = x1r + 2.0 * x2r;
    cIm[0] = x1i + 2.0 * x2i;

    /* cf[70] = t1**35 - t2**35 + 1j * t1 * t2 */
    double t1_35r, t1_35i, t2_35r, t2_35i;
    c_powr(x1r, x1i, 35.0, &t1_35r, &t1_35i);
    c_powr(x2r, x2i, 35.0, &t2_35r, &t2_35i);
    /* 1j * t1 * t2 = 1j * t1t2 = (-t1t2_i, t1t2_r) */
    cRe[70] = (t1_35r - t2_35r) + (-t1t2_i);
    cIm[70] = (t1_35i - t2_35i) + (t1t2_r);

    for (int i = 0; i < *nCoeffs; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_167_hand ----
 * Python:
 *   for k in range(0, 71):
 *       j = k + 1
 *       cf[j-1] = (t1 + t2)**(70 - k) * (t1 - t2)**k * (-1)**k / (k + 1)
 *   i.e. cf[k] = (t1+t2)^(70-k) * (t1-t2)^k * (-1)^k / (k+1), k=0..70
 */
static void poly_167_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double sum_r = x1r + x2r, sum_i = x1i + x2i;
    double dif_r = x1r - x2r, dif_i = x1i - x2i;

    /* Precompute (t1+t2)^n for n=0..70 via iterative multiplication */
    double sumpr[71], sumpi[71];
    sumpr[0] = 1.0; sumpi[0] = 0.0;
    for (int n = 1; n <= 70; n++) {
        c_mul(sumpr[n-1], sumpi[n-1], sum_r, sum_i, &sumpr[n], &sumpi[n]);
    }

    /* Compute (t1-t2)^k iteratively (ascending) */
    double difpk_r = 1.0, difpk_i = 0.0;

    for (int k = 0; k <= 70; k++) {
        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        double denom = k + 1.0;

        /* cf[k] = (t1+t2)^(70-k) * (t1-t2)^k * (-1)^k / (k+1) */
        double pr, pi;
        c_mul(sumpr[70 - k], sumpi[70 - k], difpk_r, difpk_i, &pr, &pi);
        cRe[k] = pr * sign / denom;
        cIm[k] = pi * sign / denom;

        /* Advance: difpk *= (t1-t2) */
        if (k < 70) {
            double nr, ni;
            c_mul(difpk_r, difpk_i, dif_r, dif_i, &nr, &ni);
            difpk_r = nr; difpk_i = ni;
        }
    }

    for (int i = 0; i < *nCoeffs; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}
