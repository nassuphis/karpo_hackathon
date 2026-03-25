/*
 * Hand-written polynomial coefficient functions — batch 2.
 *
 * poly_134, poly_135, poly_142, poly_146, poly_150
 *
 * Each function has the same signature as the transpiled versions:
 *   void func(double x1r, double x1i, double x2r, double x2i,
 *             double *cRe, double *cIm, int *nCoeffs)
 *
 * Requires: c_mul, c_div, c_abs, c_arg, c_exp2, c_sin, c_cos,
 *           c_log, c_powr helpers from sweep_cli.c
 */

/* ---- poly_134_hand ----
 * Python:
 *   cf = np.zeros(71, dtype=complex)
 *   for k in range(1, 72):
 *       cf[k-1] = (np.sin(t1 * k) + np.cos(np.conj(t2) * k)) * (-1)**k / (k + 1)
 */
static void poly_134_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* conj(t2) = (x2r, -x2i) */
    double ct2r = x2r, ct2i = -x2i;

    for (int k = 1; k <= 71; k++) {
        /* t1 * k  (complex * real) */
        double akr = x1r * k, aki = x1i * k;
        /* sin(t1 * k) */
        double sr, si;
        c_sin(akr, aki, &sr, &si);

        /* conj(t2) * k */
        double bkr = ct2r * k, bki = ct2i * k;
        /* cos(conj(t2) * k) */
        double cr, ci;
        c_cos(bkr, bki, &cr, &ci);

        /* (sin + cos) */
        double sumr = sr + cr, sumi = si + ci;

        /* * (-1)^k / (k + 1) */
        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        double scale = sign / (k + 1.0);

        cRe[k - 1] = sumr * scale;
        cIm[k - 1] = sumi * scale;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_135_hand ----
 * Python:
 *   cf = np.zeros(71, dtype=complex)
 *   for k in range(1, 71):
 *       cf[k-1] = (t1**k * np.sin(k * t2) + (-1)**k * t2**(k-1) * np.cos(k * t1)) / k
 *   cf[70] = (np.log(np.abs(t1) + np.abs(t2) + 1) + np.sin(t1 * t2)) / 71
 */
static void poly_135_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    for (int k = 1; k <= 70; k++) {
        /* t1**k */
        double t1kr, t1ki;
        c_powr(x1r, x1i, (double)k, &t1kr, &t1ki);

        /* sin(k * t2) */
        double sr, si;
        c_sin(x2r * k, x2i * k, &sr, &si);

        /* t1**k * sin(k * t2) */
        double ar, ai;
        c_mul(t1kr, t1ki, sr, si, &ar, &ai);

        /* t2**(k-1) */
        double t2kr, t2ki;
        if (k == 1) {
            t2kr = 1.0; t2ki = 0.0;
        } else {
            c_powr(x2r, x2i, (double)(k - 1), &t2kr, &t2ki);
        }

        /* cos(k * t1) */
        double cosR, cosI;
        c_cos(x1r * k, x1i * k, &cosR, &cosI);

        /* t2**(k-1) * cos(k * t1) */
        double br, bi;
        c_mul(t2kr, t2ki, cosR, cosI, &br, &bi);

        /* (-1)**k * that product */
        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        br *= sign;
        bi *= sign;

        /* sum / k */
        double inv_k = 1.0 / k;
        cRe[k - 1] = (ar + br) * inv_k;
        cIm[k - 1] = (ai + bi) * inv_k;
    }

    /* cf[70] = (np.log(np.abs(t1) + np.abs(t2) + 1) + np.sin(t1 * t2)) / 71 */
    double logVal = log(c_abs(x1r, x1i) + c_abs(x2r, x2i) + 1.0);

    /* sin(t1 * t2) */
    double pr, pi;
    c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
    double sinR, sinI;
    c_sin(pr, pi, &sinR, &sinI);

    cRe[70] = (logVal + sinR) / 71.0;
    cIm[70] = sinI / 71.0;

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_142_hand ----
 * Python:
 *   cf = np.zeros(71, dtype=complex)
 *   cf[0] = t1 + 2*t2
 *   cf[1] = t1**2 - t2
 *   cf[2] = np.conj(t1) + t2**3
 *   for k in range(4, 36):
 *       cf[k-1] = (t1**k + t2**(k-1)) / (k * np.sin(k))
 *       cf[72 - k - 1] = (t2**k - t1**(k-1)) / (k * np.cos(k))
 *   for k in range(36, 71):
 *       cf[k-1] = np.log(np.abs(t1) + 1) * np.sin(k * t2) + np.cos(k * t1)
 *   cf[70] = (t1**5 + t2**5) / (1 + np.abs(t1*t2))
 */
static void poly_142_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* cf[0] = t1 + 2*t2 */
    cRe[0] = x1r + 2.0 * x2r;
    cIm[0] = x1i + 2.0 * x2i;

    /* cf[1] = t1**2 - t2 */
    double t1sq_r, t1sq_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    cRe[1] = t1sq_r - x2r;
    cIm[1] = t1sq_i - x2i;

    /* cf[2] = conj(t1) + t2**3 */
    double t2cu_r, t2cu_i;
    c_powr(x2r, x2i, 3.0, &t2cu_r, &t2cu_i);
    cRe[2] = x1r + t2cu_r;
    cIm[2] = -x1i + t2cu_i;

    /* for k in range(4, 36): */
    for (int k = 4; k <= 35; k++) {
        /* t1**k */
        double t1kr, t1ki;
        c_powr(x1r, x1i, (double)k, &t1kr, &t1ki);

        /* t2**(k-1) */
        double t2km1r, t2km1i;
        c_powr(x2r, x2i, (double)(k - 1), &t2km1r, &t2km1i);

        /* (t1**k + t2**(k-1)) / (k * sin(k))  — sin(k) is real since k is integer */
        double denom1 = k * sin((double)k);
        if (denom1 != 0.0) {
            double inv1 = 1.0 / denom1;
            cRe[k - 1] = (t1kr + t2km1r) * inv1;
            cIm[k - 1] = (t1ki + t2km1i) * inv1;
        }

        /* t2**k */
        double t2kr, t2ki;
        c_powr(x2r, x2i, (double)k, &t2kr, &t2ki);

        /* t1**(k-1) */
        double t1km1r, t1km1i;
        c_powr(x1r, x1i, (double)(k - 1), &t1km1r, &t1km1i);

        /* cf[72 - k - 1] = (t2**k - t1**(k-1)) / (k * cos(k)) */
        int idx2 = 72 - k - 1;  /* = 71 - k */
        double denom2 = k * cos((double)k);
        if (denom2 != 0.0) {
            double inv2 = 1.0 / denom2;
            cRe[idx2] = (t2kr - t1km1r) * inv2;
            cIm[idx2] = (t2ki - t1km1i) * inv2;
        }
    }

    /* for k in range(36, 71): */
    double logAbs1 = log(c_abs(x1r, x1i) + 1.0);
    for (int k = 36; k <= 70; k++) {
        /* np.sin(k * t2) — complex sin */
        double sr, si;
        c_sin(x2r * k, x2i * k, &sr, &si);

        /* np.cos(k * t1) — complex cos */
        double cr, ci;
        c_cos(x1r * k, x1i * k, &cr, &ci);

        /* logAbs1 * sin(k*t2) + cos(k*t1) */
        /* logAbs1 is real, so multiply real * complex */
        cRe[k - 1] = logAbs1 * sr + cr;
        cIm[k - 1] = logAbs1 * si + ci;
    }

    /* cf[70] = (t1**5 + t2**5) / (1 + |t1*t2|) */
    double t1_5r, t1_5i, t2_5r, t2_5i;
    c_powr(x1r, x1i, 5.0, &t1_5r, &t1_5i);
    c_powr(x2r, x2i, 5.0, &t2_5r, &t2_5i);

    double prodr, prodi;
    c_mul(x1r, x1i, x2r, x2i, &prodr, &prodi);
    double denom = 1.0 + c_abs(prodr, prodi);

    cRe[70] = (t1_5r + t2_5r) / denom;
    cIm[70] = (t1_5i + t2_5i) / denom;

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_146_hand ----
 * Python:
 *   cf = np.zeros(71, dtype=complex)
 *   cf[0:10]  = (t1**arange(1,11) + t2**arange(1,11)) * (-1)**arange(1,11)
 *   cf[10:20] = sin(t1 * arange(11,21)) - cos(t2 * arange(11,21))
 *   cf[20:30] = log(|t1|+1) * arange(21,31) - log(|t2|+1)
 *   cf[30:40] = (t1*t2)**arange(31,41) / (1 + arange(31,41))
 *   cf[40:71] = Re(t1)*Im(t2) - Im(t1)*Re(t2) + angle(t1+t2)*|t1-t2|*arange(40,71)
 *   cf[70]    = sum(cf[0:70])
 */
static void poly_146_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* cf[0:10] = (t1**k + t2**k) * (-1)**k  for k in 1..10 */
    for (int k = 1; k <= 10; k++) {
        double t1kr, t1ki, t2kr, t2ki;
        c_powr(x1r, x1i, (double)k, &t1kr, &t1ki);
        c_powr(x2r, x2i, (double)k, &t2kr, &t2ki);

        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        cRe[k - 1] = (t1kr + t2kr) * sign;
        cIm[k - 1] = (t1ki + t2ki) * sign;
    }

    /* cf[10:20] = sin(t1 * k) - cos(t2 * k)  for k in 11..20 */
    for (int k = 11; k <= 20; k++) {
        double sr, si;
        c_sin(x1r * k, x1i * k, &sr, &si);

        double cr, ci;
        c_cos(x2r * k, x2i * k, &cr, &ci);

        int idx = k - 11 + 10;  /* 10..19 */
        cRe[idx] = sr - cr;
        cIm[idx] = si - ci;
    }

    /* cf[20:30] = log(|t1|+1) * k - log(|t2|+1)  for k in 21..30 */
    /* These are real-valued coefficients */
    double logA = log(c_abs(x1r, x1i) + 1.0);
    double logB = log(c_abs(x2r, x2i) + 1.0);
    for (int k = 21; k <= 30; k++) {
        int idx = k - 21 + 20;  /* 20..29 */
        cRe[idx] = logA * k - logB;
        cIm[idx] = 0.0;
    }

    /* cf[30:40] = (t1*t2)**k / (1 + k)  for k in 31..40 */
    double prodr, prodi;
    c_mul(x1r, x1i, x2r, x2i, &prodr, &prodi);
    for (int k = 31; k <= 40; k++) {
        double pkr, pki;
        c_powr(prodr, prodi, (double)k, &pkr, &pki);
        double inv = 1.0 / (1.0 + k);

        int idx = k - 31 + 30;  /* 30..39 */
        cRe[idx] = pkr * inv;
        cIm[idx] = pki * inv;
    }

    /* cf[40:71] = crossTerm + angle(t1+t2)*|t1-t2|*j  for j in 40..70 */
    double crossTerm = x1r * x2i - x1i * x2r;  /* Re(t1)*Im(t2) - Im(t1)*Re(t2) */

    /* angle(t1 + t2) */
    double sumr = x1r + x2r, sumi = x1i + x2i;
    double ang = c_arg(sumr, sumi);

    /* |t1 - t2| */
    double diffr = x1r - x2r, diffi = x1i - x2i;
    double absDiff = c_abs(diffr, diffi);

    double angAbs = ang * absDiff;

    for (int j = 40; j <= 70; j++) {
        int idx = j;  /* cf[40] through cf[70] */
        cRe[idx] = crossTerm + angAbs * j;
        cIm[idx] = 0.0;
    }

    /* cf[70] = sum(cf[0:70]) */
    double sumRe = 0.0, sumIm = 0.0;
    for (int i = 0; i < 70; i++) {
        sumRe += cRe[i];
        sumIm += cIm[i];
    }
    cRe[70] = sumRe;
    cIm[70] = sumIm;

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_150_hand ----
 * Python:
 *   j = np.arange(0, 71).astype(complex)
 *   cf = (t1 + j * t2) * (-1)**j * log(|t1| + |t2| + 1)**(|j| % 5 + 1) * (j + 1)
 *
 * Since j is non-negative integer, |j| = j, so exponent = (j % 5 + 1).
 * log(|t1|+|t2|+1) is a real scalar; raised to integer power is still real.
 * (t1 + j*t2) is complex, (-1)**j is real, (j+1) is real.
 */
static void poly_150_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double L = log(c_abs(x1r, x1i) + c_abs(x2r, x2i) + 1.0);

    for (int j = 0; j < 71; j++) {
        /* t1 + j * t2 */
        double vr = x1r + j * x2r;
        double vi = x1i + j * x2i;

        /* (-1)**j */
        double sign = (j % 2 == 0) ? 1.0 : -1.0;

        /* L**(j % 5 + 1) */
        int exp = (j % 5) + 1;
        double Lpow = 1.0;
        for (int e = 0; e < exp; e++) Lpow *= L;

        /* (j + 1) */
        double jp1 = (double)(j + 1);

        /* full scale = sign * Lpow * jp1 */
        double scale = sign * Lpow * jp1;

        cRe[j] = vr * scale;
        cIm[j] = vi * scale;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}
