/*
 * Hand-written polynomial coefficient functions — batch 4.
 *
 * poly_171, poly_179, poly_180, poly_184, poly_187
 *
 * Requires: c_mul, c_div, c_sin, c_cos, c_log, c_abs, c_arg,
 *           c_exp2, c_powr, c_powc helpers from sweep_cli.c
 */

/* ---- poly_171_hand ----
 * Python:
 *   k = np.arange(71)
 *   cf = (t1**k + t2**(70 - k)) * (-1)**k * (71 - k)
 */
static void poly_171_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Build t1**k powers: t1pow[k] = t1^k */
    double t1pr[71], t1pi[71];
    t1pr[0] = 1.0; t1pi[0] = 0.0;
    for (int k = 1; k < 71; k++) {
        c_mul(t1pr[k-1], t1pi[k-1], x1r, x1i, &t1pr[k], &t1pi[k]);
    }

    /* Build t2**k powers: t2pow[k] = t2^k */
    double t2pr[71], t2pi[71];
    t2pr[0] = 1.0; t2pi[0] = 0.0;
    for (int k = 1; k < 71; k++) {
        c_mul(t2pr[k-1], t2pi[k-1], x2r, x2i, &t2pr[k], &t2pi[k]);
    }

    for (int k = 0; k < 71; k++) {
        /* (t1**k + t2**(70-k)) */
        double sr = t1pr[k] + t2pr[70 - k];
        double si = t1pi[k] + t2pi[70 - k];
        /* * (-1)**k * (71 - k) */
        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        double scale = sign * (71 - k);
        cRe[k] = sr * scale;
        cIm[k] = si * scale;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_179_hand ----
 * Python:
 *   cf[0] = (t1 + t2) * (np.real(t1) - np.imag(t2))
 *   for j in range(2, 71):
 *     if j % 2 == 0:
 *       cf[j-1] = (np.abs(t1)**j - np.abs(t2)**j) / (j + 1) * (-1)**j
 *     else:
 *       cf[j-1] = (np.real(t1)**j + np.imag(t2)**j) / (j + 2) * np.sin(j * np.angle(t1 + t2))
 *   cf[70] = sum((t1**n).real * (t2**n).imag for n in range(1, 6)) + np.log(np.abs(t1) + np.abs(t2) + 1)
 */
static void poly_179_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* cf[0] = (t1 + t2) * (real(t1) - imag(t2))
     * This is complex * real = scale the complex number */
    double sumr = x1r + x2r;
    double sumi = x1i + x2i;
    double factor = x1r - x2i;
    cRe[0] = sumr * factor;
    cIm[0] = sumi * factor;

    double abs_t1 = c_abs(x1r, x1i);
    double abs_t2 = c_abs(x2r, x2i);
    double angle_sum = c_arg(x1r + x2r, x1i + x2i);

    for (int j = 2; j < 71; j++) {
        if (j % 2 == 0) {
            /* cf[j-1] = (|t1|^j - |t2|^j) / (j+1) * (-1)^j */
            double at1j = pow(abs_t1, (double)j);
            double at2j = pow(abs_t2, (double)j);
            double sign = (j % 2 == 0) ? 1.0 : -1.0;
            cRe[j - 1] = (at1j - at2j) / (j + 1) * sign;
            cIm[j - 1] = 0.0;
        } else {
            /* cf[j-1] = (real(t1)^j + imag(t2)^j) / (j+2) * sin(j * angle(t1+t2)) */
            double r1j = pow(x1r, (double)j);
            double i2j = pow(x2i, (double)j);
            double s = sin(j * angle_sum);
            cRe[j - 1] = (r1j + i2j) / (j + 2) * s;
            cIm[j - 1] = 0.0;
        }
    }

    /* cf[70] = sum((t1**n).real * (t2**n).imag for n in range(1,6))
     *        + log(|t1| + |t2| + 1) */
    double t1pr = x1r, t1pi = x1i;
    double t2pr = x2r, t2pi = x2i;
    double acc = 0.0;
    for (int n = 1; n <= 5; n++) {
        if (n > 1) {
            double nr, ni;
            c_mul(t1pr, t1pi, x1r, x1i, &nr, &ni);
            t1pr = nr; t1pi = ni;
            c_mul(t2pr, t2pi, x2r, x2i, &nr, &ni);
            t2pr = nr; t2pi = ni;
        }
        acc += t1pr * t2pi;  /* real(t1^n) * imag(t2^n) */
    }
    cRe[70] = acc + log(abs_t1 + abs_t2 + 1.0);
    cIm[70] = 0.0;

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_180_hand ----
 * Python:
 *   cf0 = t1 ** ((np.arange(71) % 6) + 1)
 *   cf1 = t2 ** ((np.arange(71) % 4) + 1)
 *   cf2 = (-1)**np.arange(71)
 *   cf3 = np.log(np.arange(71) + 1)
 *   cf = (cf0 + cf1) * cf2 * cf3
 */
static void poly_180_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Precompute t1^1..t1^6 */
    double t1pr[7], t1pi[7];
    t1pr[0] = 1.0; t1pi[0] = 0.0;
    for (int p = 1; p <= 6; p++) {
        c_mul(t1pr[p-1], t1pi[p-1], x1r, x1i, &t1pr[p], &t1pi[p]);
    }

    /* Precompute t2^1..t2^4 */
    double t2pr[5], t2pi[5];
    t2pr[0] = 1.0; t2pi[0] = 0.0;
    for (int p = 1; p <= 4; p++) {
        c_mul(t2pr[p-1], t2pi[p-1], x2r, x2i, &t2pr[p], &t2pi[p]);
    }

    for (int k = 0; k < 71; k++) {
        int p1 = (k % 6) + 1;  /* 1..6 */
        int p2 = (k % 4) + 1;  /* 1..4 */
        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        double lg = log((double)(k + 1));

        /* (t1^p1 + t2^p2) * sign * lg */
        double sr = t1pr[p1] + t2pr[p2];
        double si = t1pi[p1] + t2pi[p2];
        double scale = sign * lg;
        cRe[k] = sr * scale;
        cIm[k] = si * scale;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_184_hand ----
 * Python:
 *   cf[0] = t1**5 + 2*t2**4
 *   cf[1] = -3*t1**4 + 4*t2**3
 *   cf[2] = 5*t1**3 - 6*t2**2
 *   cf[3] = -7*t1**2 + 8*t2
 *   cf[4] = 9*t1 - 10
 *   cf[5:10]  = (t1*t2)**arange(1,6) * [11,-12,13,-14,15]
 *   cf[10:20] = (t1+t2)**(arange(6,16)/2) * [-16,17,-18,19,-20,21,-22,23,-24,25]
 *   cf[20:30] = (t1-t2)**(arange(16,26)/3) * [26,-27,28,-29,30,31,-32,33,-34,35]
 *   cf[30:40] = (t1*t2)**(arange(26,36)/4) * [36,-37,38,-39,40,41,-42,43,-44,45]
 *   cf[40:50] = (t1+conj(t2))**(arange(36,46)/5) * [-46,47,-48,49,-50,51,-52,53,-54,55]
 *   cf[50:60] = (conj(t1)-t2)**(arange(46,56)/6) * [56,-57,58,-59,60,61,-62,63,-64,65]
 *   cf[60:70] = (|t1|+|t2|)**(arange(56,66)/7) * [-66,67,-68,69,-70,71,-72,73,-74,75]
 *   cf[70]    = log(|t1|+1) + log(|t2|+1)
 */
static void poly_184_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Precompute t1 powers up to 5 */
    double t1pr[6], t1pi[6];
    t1pr[0] = 1.0; t1pi[0] = 0.0;
    for (int p = 1; p <= 5; p++)
        c_mul(t1pr[p-1], t1pi[p-1], x1r, x1i, &t1pr[p], &t1pi[p]);

    /* Precompute t2 powers up to 4 */
    double t2pr[5], t2pi[5];
    t2pr[0] = 1.0; t2pi[0] = 0.0;
    for (int p = 1; p <= 4; p++)
        c_mul(t2pr[p-1], t2pi[p-1], x2r, x2i, &t2pr[p], &t2pi[p]);

    /* cf[0] = t1^5 + 2*t2^4 */
    cRe[0] = t1pr[5] + 2.0 * t2pr[4];
    cIm[0] = t1pi[5] + 2.0 * t2pi[4];

    /* cf[1] = -3*t1^4 + 4*t2^3 */
    cRe[1] = -3.0 * t1pr[4] + 4.0 * t2pr[3];
    cIm[1] = -3.0 * t1pi[4] + 4.0 * t2pi[3];

    /* cf[2] = 5*t1^3 - 6*t2^2 */
    cRe[2] = 5.0 * t1pr[3] - 6.0 * t2pr[2];
    cIm[2] = 5.0 * t1pi[3] - 6.0 * t2pi[2];

    /* cf[3] = -7*t1^2 + 8*t2 */
    cRe[3] = -7.0 * t1pr[2] + 8.0 * x2r;
    cIm[3] = -7.0 * t1pi[2] + 8.0 * x2i;

    /* cf[4] = 9*t1 - 10 */
    cRe[4] = 9.0 * x1r - 10.0;
    cIm[4] = 9.0 * x1i;

    /* cf[5:10] = (t1*t2)^arange(1,6) * [11,-12,13,-14,15] */
    {
        double prodR, prodI;
        c_mul(x1r, x1i, x2r, x2i, &prodR, &prodI);
        double ppR = 1.0, ppI = 0.0;
        double coeffs5[5] = {11.0, -12.0, 13.0, -14.0, 15.0};
        for (int j = 0; j < 5; j++) {
            double nr, ni;
            c_mul(ppR, ppI, prodR, prodI, &nr, &ni);
            ppR = nr; ppI = ni;
            cRe[5 + j] = ppR * coeffs5[j];
            cIm[5 + j] = ppI * coeffs5[j];
        }
    }

    /* cf[10:20] = (t1+t2)^(arange(6,16)/2) * [-16,17,-18,19,-20,21,-22,23,-24,25] */
    {
        double br = x1r + x2r, bi = x1i + x2i;
        double coeffs10[10] = {-16.0, 17.0, -18.0, 19.0, -20.0, 21.0, -22.0, 23.0, -24.0, 25.0};
        for (int j = 0; j < 10; j++) {
            double pw = (6.0 + j) / 2.0;
            double rr, ri;
            c_powr(br, bi, pw, &rr, &ri);
            cRe[10 + j] = rr * coeffs10[j];
            cIm[10 + j] = ri * coeffs10[j];
        }
    }

    /* cf[20:30] = (t1-t2)^(arange(16,26)/3) * [26,-27,28,-29,30,31,-32,33,-34,35] */
    {
        double br = x1r - x2r, bi = x1i - x2i;
        double coeffs20[10] = {26.0, -27.0, 28.0, -29.0, 30.0, 31.0, -32.0, 33.0, -34.0, 35.0};
        for (int j = 0; j < 10; j++) {
            double pw = (16.0 + j) / 3.0;
            double rr, ri;
            c_powr(br, bi, pw, &rr, &ri);
            cRe[20 + j] = rr * coeffs20[j];
            cIm[20 + j] = ri * coeffs20[j];
        }
    }

    /* cf[30:40] = (t1*t2)^(arange(26,36)/4) * [36,-37,38,-39,40,41,-42,43,-44,45] */
    {
        double pr, pi;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
        double coeffs30[10] = {36.0, -37.0, 38.0, -39.0, 40.0, 41.0, -42.0, 43.0, -44.0, 45.0};
        for (int j = 0; j < 10; j++) {
            double pw = (26.0 + j) / 4.0;
            double rr, ri;
            c_powr(pr, pi, pw, &rr, &ri);
            cRe[30 + j] = rr * coeffs30[j];
            cIm[30 + j] = ri * coeffs30[j];
        }
    }

    /* cf[40:50] = (t1+conj(t2))^(arange(36,46)/5) * [-46,47,-48,49,-50,51,-52,53,-54,55] */
    {
        double br = x1r + x2r, bi = x1i + (-x2i);
        double coeffs40[10] = {-46.0, 47.0, -48.0, 49.0, -50.0, 51.0, -52.0, 53.0, -54.0, 55.0};
        for (int j = 0; j < 10; j++) {
            double pw = (36.0 + j) / 5.0;
            double rr, ri;
            c_powr(br, bi, pw, &rr, &ri);
            cRe[40 + j] = rr * coeffs40[j];
            cIm[40 + j] = ri * coeffs40[j];
        }
    }

    /* cf[50:60] = (conj(t1)-t2)^(arange(46,56)/6) * [56,-57,58,-59,60,61,-62,63,-64,65] */
    {
        double br = x1r - x2r, bi = (-x1i) - x2i;
        double coeffs50[10] = {56.0, -57.0, 58.0, -59.0, 60.0, 61.0, -62.0, 63.0, -64.0, 65.0};
        for (int j = 0; j < 10; j++) {
            double pw = (46.0 + j) / 6.0;
            double rr, ri;
            c_powr(br, bi, pw, &rr, &ri);
            cRe[50 + j] = rr * coeffs50[j];
            cIm[50 + j] = ri * coeffs50[j];
        }
    }

    /* cf[60:70] = (|t1|+|t2|)^(arange(56,66)/7) * [-66,67,-68,69,-70,71,-72,73,-74,75]
     * Note: |t1|+|t2| is real, so result is purely real */
    {
        double base = c_abs(x1r, x1i) + c_abs(x2r, x2i);
        double coeffs60[10] = {-66.0, 67.0, -68.0, 69.0, -70.0, 71.0, -72.0, 73.0, -74.0, 75.0};
        for (int j = 0; j < 10; j++) {
            double pw = (56.0 + j) / 7.0;
            double val = pow(base, pw);
            cRe[60 + j] = val * coeffs60[j];
            cIm[60 + j] = 0.0;
        }
    }

    /* cf[70] = log(|t1|+1) + log(|t2|+1) */
    cRe[70] = log(c_abs(x1r, x1i) + 1.0) + log(c_abs(x2r, x2i) + 1.0);
    cIm[70] = 0.0;

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_187_hand ----
 * Python:
 *   k = np.arange(71)
 *   cf = t1**k * np.sin(t2 * k) + t2**k * np.cos(t1 * k)
 *
 * t1, t2 are complex, so t2*k and t1*k are complex => use c_sin, c_cos
 */
static void poly_187_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Build t1^k and t2^k iteratively */
    double t1pr = 1.0, t1pi = 0.0;
    double t2pr = 1.0, t2pi = 0.0;

    for (int k = 0; k < 71; k++) {
        /* sin(t2 * k) where t2*k is complex */
        double skr = (double)k * x2r;
        double ski = (double)k * x2i;
        double sinr, sini;
        c_sin(skr, ski, &sinr, &sini);

        /* cos(t1 * k) where t1*k is complex */
        double ckr = (double)k * x1r;
        double cki = (double)k * x1i;
        double cosr, cosi;
        c_cos(ckr, cki, &cosr, &cosi);

        /* t1^k * sin(t2*k) */
        double mr, mi;
        c_mul(t1pr, t1pi, sinr, sini, &mr, &mi);

        /* t2^k * cos(t1*k) */
        double nr, ni;
        c_mul(t2pr, t2pi, cosr, cosi, &nr, &ni);

        cRe[k] = mr + nr;
        cIm[k] = mi + ni;

        /* Advance powers: t1^(k+1) = t1^k * t1, t2^(k+1) = t2^k * t2 */
        double nxr, nxi;
        c_mul(t1pr, t1pi, x1r, x1i, &nxr, &nxi);
        t1pr = nxr; t1pi = nxi;
        c_mul(t2pr, t2pi, x2r, x2i, &nxr, &nxi);
        t2pr = nxr; t2pi = nxi;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}
