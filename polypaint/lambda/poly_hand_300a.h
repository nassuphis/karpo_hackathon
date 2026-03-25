/* poly_hand_300a.h — Hand-written C for poly_201 through poly_220 */

static void poly_201_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (vectorized over j=0..70):
     *   cf = (np.real(t1)**j * np.sin(j * np.angle(t2)) +
     *         np.real(t2)**j * np.cos(j * np.angle(t1))) +
     *        (np.imag(t1)**j * np.cos(j * np.angle(t2)) -
     *         np.imag(t2)**j * np.sin(j * np.angle(t1))) * 1j
     *
     * Real part: real(t1)^j * sin(j*angle(t2)) + real(t2)^j * cos(j*angle(t1))
     * Imag part: imag(t1)^j * cos(j*angle(t2)) - imag(t2)^j * sin(j*angle(t1))
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    double r1_pow = 1.0;  /* x1r^j */
    double r2_pow = 1.0;  /* x2r^j */
    double i1_pow = 1.0;  /* x1i^j */
    double i2_pow = 1.0;  /* x2i^j */

    for (int j = 0; j < 71; j++) {
        cRe[j] = r1_pow * sin(j * ang2) + r2_pow * cos(j * ang1);
        cIm[j] = i1_pow * cos(j * ang2) - i2_pow * sin(j * ang1);

        r1_pow *= x1r;
        r2_pow *= x2r;
        i1_pow *= x1i;
        i2_pow *= x2i;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_202_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (vectorized over k=0..70):
     *   cf = (t1**k * np.sin(k * np.angle(t1) + np.real(t2)) +
     *         t2**k * np.cos(k * np.angle(t2) - np.real(t1)))
     *        * np.log(np.abs(t1 * t2) + 1) / (1 + k**2)
     *      + (np.sin(k * np.real(t1)) - np.cos(k * np.imag(t2)))
     *        * (np.abs(t1) + np.abs(t2)) / (2 + k)
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    /* t1*t2 */
    double pr, pi;
    c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
    double logprod = log(c_abs(pr, pi) + 1.0);
    double absum = c_abs(x1r, x1i) + c_abs(x2r, x2i);

    /* iterative t1^k and t2^k */
    double t1kr = 1.0, t1ki = 0.0;  /* t1^k */
    double t2kr = 1.0, t2ki = 0.0;  /* t2^k */

    for (int k = 0; k < 71; k++) {
        /* t1^k * sin(k*angle(t1) + real(t2)) -- multiply complex by real scalar */
        double s1 = sin(k * ang1 + x2r);
        double a1r = t1kr * s1;
        double a1i = t1ki * s1;

        /* t2^k * cos(k*angle(t2) - real(t1)) */
        double c2 = cos(k * ang2 - x1r);
        double a2r = t2kr * c2;
        double a2i = t2ki * c2;

        double scale1 = logprod / (1.0 + (double)k * k);
        double termRe = (a1r + a2r) * scale1;
        double termIm = (a1i + a2i) * scale1;

        /* second part is real */
        double term2 = (sin(k * x1r) - cos(k * x2i)) * absum / (2.0 + k);

        cRe[k] = termRe + term2;
        cIm[k] = termIm;

        /* advance powers */
        double nr, ni;
        c_mul(t1kr, t1ki, x1r, x1i, &nr, &ni);
        t1kr = nr; t1ki = ni;
        c_mul(t2kr, t2ki, x2r, x2i, &nr, &ni);
        t2kr = nr; t2ki = ni;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_203_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (k=1..71, so index j = k-1 = 0..70):
     *   real_part = real(t1)^k * sin(k*angle(t2)) + real(t2)^k * cos(k*angle(t1))
     *              + log(abs(t1) + k) + real(t1+t2)^k / (k+1)
     *   imag_part = imag(t1)^k * cos(k*angle(t2)) + imag(t2)^k * sin(k*angle(t1))
     *              + sin(k) + cos(k)
     *   cf = (real_part + 1j * imag_part) * ((-1)^k * log(k + abs(t1)))
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double abs1 = c_abs(x1r, x1i);
    double rsum = x1r + x2r;  /* real(t1+t2) */

    double r1_pow = x1r;   /* x1r^k, starts at k=1 */
    double r2_pow = x2r;   /* x2r^k */
    double i1_pow = x1i;   /* x1i^k */
    double i2_pow = x2i;   /* x2i^k */
    double rs_pow = rsum;  /* rsum^k */

    for (int k = 1; k <= 71; k++) {
        int j = k - 1;
        double rp = r1_pow * sin(k * ang2) + r2_pow * cos(k * ang1)
                  + log(abs1 + k) + rs_pow / (k + 1.0);
        double ip = i1_pow * cos(k * ang2) + i2_pow * sin(k * ang1)
                  + sin((double)k) + cos((double)k);
        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        double logk = log(k + abs1);
        double scale = sign * logk;
        /* (rp + ip*1j) * scale => (rp*scale, ip*scale) */
        cRe[j] = rp * scale;
        cIm[j] = ip * scale;

        r1_pow *= x1r;
        r2_pow *= x2r;
        i1_pow *= x1i;
        i2_pow *= x2i;
        rs_pow *= rsum;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_204_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (j=1..71, index = j-1 = 0..70):
     *   cf = real(t1)^j + real(t2)^(71-j) * cos(j*angle(t1) + (71-j)*angle(t2))
     *        + log(abs(t1)+1) * sin(j*angle(t2))
     *   cf += imag(t1)^j - imag(t2)^(71-j) * sin(j*angle(t1) - (71-j)*angle(t2))
     *         + log(abs(t2)+1) * cos(j*angle(t1))
     *
     * Result is real (no 1j multiplier).
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double log1 = log(c_abs(x1r, x1i) + 1.0);
    double log2 = log(c_abs(x2r, x2i) + 1.0);

    for (int j = 1; j <= 71; j++) {
        int idx = j - 1;
        int mj = 71 - j;
        double r1j = pow(x1r, (double)j);
        double r2mj = pow(x2r, (double)mj);
        double i1j = pow(x1i, (double)j);
        double i2mj = pow(x2i, (double)mj);

        double line1 = r1j + r2mj * cos(j * ang1 + mj * ang2) + log1 * sin(j * ang2);
        double line2 = i1j - i2mj * sin(j * ang1 - mj * ang2) + log2 * cos(j * ang1);

        cRe[idx] = line1 + line2;
        /* cIm stays 0 — result is real */
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_205_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python:
     *   cf[j] = (real(t1)*(j+1)^2 + imag(t2)*(j+1)) * sin((j+1)*angle(t1+t2))
     *         + (cos((j+1)*angle(t1)) + log(abs(t1*t2)+1)) * (cos(j+1) + sin(j+1)*1j)
     *
     * The second term has a complex factor: cos(j+1) + sin(j+1)*1j
     * Let A = (real(t1)*(j+1)^2 + imag(t2)*(j+1)) * sin((j+1)*angle(t1+t2))  [real]
     * Let B = cos((j+1)*angle(t1)) + log(abs(t1*t2)+1)                        [real]
     * cf[j] = A + B*(cos(j+1) + sin(j+1)*1j)
     *       = (A + B*cos(j+1)) + B*sin(j+1)*1j
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double ang_sum = c_arg(x1r + x2r, x1i + x2i);  /* angle(t1+t2) */
    double ang1 = c_arg(x1r, x1i);
    double pr, pi;
    c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
    double logprod = log(c_abs(pr, pi) + 1.0);

    for (int j = 0; j < 71; j++) {
        double jp1 = j + 1.0;
        double A = (x1r * jp1 * jp1 + x2i * jp1) * sin(jp1 * ang_sum);
        double B = cos(jp1 * ang1) + logprod;
        cRe[j] = A + B * cos(jp1);
        cIm[j] = B * sin(jp1);
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_206_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python:
     *   real_part = real(t1)^(j+1) * sin((j+1)*angle(t2) + log(abs(t1)+1))
     *   imag_part = imag(t2)^(j+1) * cos((j+1)*angle(t1) + log(abs(t2)+1))
     *   cf[j] = real_part + imag(t1) * real(t2) * imag_part
     *
     * Result is real.
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double loga1 = log(c_abs(x1r, x1i) + 1.0);
    double loga2 = log(c_abs(x2r, x2i) + 1.0);
    double scale = x1i * x2r;  /* imag(t1) * real(t2) */

    double r1_pow = x1r;   /* x1r^(j+1) starting at j=0 */
    double i2_pow = x2i;   /* x2i^(j+1) */

    for (int j = 0; j < 71; j++) {
        double jp1 = j + 1.0;
        double rp = r1_pow * sin(jp1 * ang2 + loga1);
        double ip = i2_pow * cos(jp1 * ang1 + loga2);
        cRe[j] = rp + scale * ip;

        r1_pow *= x1r;
        i2_pow *= x2i;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_207_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python:
     *   Loop 1 (j=0..70):
     *     k = (j+1)^2 * real(t1) - imag(t2)
     *     r = sin((j+1)*angle(t1)) + cos((j+1)*angle(t2))
     *     magnitude = log(abs(t1)+1) * (k + r^2)
     *     angle_val = abs(t2)*(j+1) + real(t1)*sin(j+1)
     *     cf[j] = magnitude * (cos(angle_val) + sin(angle_val)*1j)
     *   Loop 2 (j=0..70):
     *     cf[j] += conj(t1) * conj(t2) * (j+1)
     *
     * conj(t1)*conj(t2) = conj(t1*t2) → compute once.
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double log_abs1 = log(c_abs(x1r, x1i) + 1.0);
    double abs2 = c_abs(x2r, x2i);

    /* conj(t1)*conj(t2) = (x1r - x1i*1j)*(x2r - x2i*1j) */
    double cjr, cji;
    c_mul(x1r, -x1i, x2r, -x2i, &cjr, &cji);

    for (int j = 0; j < 71; j++) {
        double jp1 = j + 1.0;
        double kv = jp1 * jp1 * x1r - x2i;
        double rv = sin(jp1 * ang1) + cos(jp1 * ang2);
        double magnitude = log_abs1 * (kv + rv * rv);
        double angle_val = abs2 * jp1 + x1r * sin(jp1);
        cRe[j] = magnitude * cos(angle_val) + cjr * jp1;
        cIm[j] = magnitude * sin(angle_val) + cji * jp1;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_208_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python:
     *   mag = log(abs(t1)*(j+1) + abs(t2)*(j+1)^2 + 1)
     *   angle = angle(t1)*sqrt(j+1) + angle(t2)*log(j+2)
     *   cf[j] = mag * (cos(angle) + sin(angle)*1j)
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    for (int j = 0; j < 71; j++) {
        double jp1 = j + 1.0;
        double mag = log(abs1 * jp1 + abs2 * jp1 * jp1 + 1.0);
        double angle_val = ang1 * sqrt(jp1) + ang2 * log(j + 2.0);
        cRe[j] = mag * cos(angle_val);
        cIm[j] = mag * sin(angle_val);
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_209_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python:
     *   for j in range(71):
     *     mag = 0; ang = 0
     *     for k in range(1, min(j+1, 11)):
     *       mag += real(t1)^k * real(t2)^(j-k) * log(abs(t1)+abs(t2)+1)
     *       ang += angle(t1)*k - angle(t2)*(j-k) + sin(k)*angle(conj(t1+t2))
     *     cf[j] = mag * (cos(ang) + sin(ang)*1j)
     *
     * Note: angle(conj(t1+t2)) = -angle(t1+t2)
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double logsum = log(c_abs(x1r, x1i) + c_abs(x2r, x2i) + 1.0);
    double ang_conj_sum = -c_arg(x1r + x2r, x1i + x2i);  /* angle(conj(t1+t2)) */

    for (int j = 0; j < 71; j++) {
        double mag = 0.0;
        double ang = 0.0;
        int kmax = (j + 1 < 11) ? j + 1 : 11;
        for (int k = 1; k < kmax; k++) {
            mag += pow(x1r, (double)k) * pow(x2r, (double)(j - k)) * logsum;
            ang += ang1 * k - ang2 * (j - k) + sin((double)k) * ang_conj_sum;
        }
        cRe[j] = mag * cos(ang);
        cIm[j] = mag * sin(ang);
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_210_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python:
     *   mag = log(abs(t1)^(j+1) + abs(t2)^(70-j) + 1)
     *         * (1 + sin((j+1)*real(t1)) - cos((j+1)*imag(t2)))
     *   angle = angle(t1)*(j+1) + angle(t2)*(j+1)^2
     *           + sin((j+1)*real(t1)) - cos((j+1)*imag(t2))
     *   cf[j] = mag * (cos(angle) + sin(angle)*1j)
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    for (int j = 0; j < 71; j++) {
        double jp1 = j + 1.0;
        double s = sin(jp1 * x1r);
        double c = cos(jp1 * x2i);
        double mag = log(pow(abs1, jp1) + pow(abs2, 70.0 - j) + 1.0)
                   * (1.0 + s - c);
        double angle_val = ang1 * jp1 + ang2 * jp1 * jp1 + s - c;
        cRe[j] = mag * cos(angle_val);
        cIm[j] = mag * sin(angle_val);
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_211_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python:
     *   mag = log(abs(t1)*(j+1) + abs(t2)*sqrt(j+1) + 1)
     *         + sin((j+1)*real(t1)) * cos((j+1)*imag(t2))
     *   angle = angle(t1)^2 / (j+2) + angle(t2)*cos(j+1) + real(t1*t2)
     *   cf[j] = mag * (cos(angle) + sin(angle)*1j)
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    /* real(t1*t2) */
    double pr, pi;
    c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
    double real_prod = pr;

    for (int j = 0; j < 71; j++) {
        double jp1 = j + 1.0;
        double mag = log(abs1 * jp1 + abs2 * sqrt(jp1) + 1.0)
                   + sin(jp1 * x1r) * cos(jp1 * x2i);
        double angle_val = ang1 * ang1 / (j + 2.0) + ang2 * cos(jp1) + real_prod;
        cRe[j] = mag * cos(angle_val);
        cIm[j] = mag * sin(angle_val);
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_212_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python:
     *   cf[j] = (real(t1)^j + imag(t2)^(70-j)) * cos(j*angle(t1+t2))
     *           + sin(j*angle(t1*t2))
     *           + log(abs(t1)+abs(t2)+1)^j
     *
     * Result is real.
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double ang_sum = c_arg(x1r + x2r, x1i + x2i);  /* angle(t1+t2) */
    double pr, pi;
    c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
    double ang_prod = c_arg(pr, pi);  /* angle(t1*t2) */
    double logab = log(c_abs(x1r, x1i) + c_abs(x2r, x2i) + 1.0);

    double r1_pow = 1.0;   /* x1r^j */
    double log_pow = 1.0;  /* logab^j */

    for (int j = 0; j < 71; j++) {
        double i2_70mj = pow(x2i, 70.0 - j);
        cRe[j] = (r1_pow + i2_70mj) * cos(j * ang_sum)
               + sin(j * ang_prod)
               + log_pow;

        r1_pow *= x1r;
        log_pow *= logab;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_213_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python:
     *   k = (j+1)*3 + (j // 7)
     *   r = (j % 5) + 2
     *   mag = abs(t1)^k + abs(t2)^r + (real(t1) + imag(t2)) * log(abs(t1)+1)
     *   angle = angle(t1)*k - angle(t2)*r + sin(j+1)*cos(j+1)
     *   cf[j] = mag * (cos(angle) + sin(angle)*1j)
     *
     * np.sum([real(t1), imag(t2)]) = real(t1) + imag(t2)
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double sum_ri = x1r + x2i;
    double log1 = log(abs1 + 1.0);

    for (int j = 0; j < 71; j++) {
        int kv = (j + 1) * 3 + (j / 7);
        int rv = (j % 5) + 2;
        double mag = pow(abs1, (double)kv) + pow(abs2, (double)rv)
                   + sum_ri * log1;
        double angle_val = ang1 * kv - ang2 * rv
                         + sin(j + 1.0) * cos(j + 1.0);
        cRe[j] = mag * cos(angle_val);
        cIm[j] = mag * sin(angle_val);
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_214_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python:
     *   cf[0] = real(t1) + imag(t2)*1j
     *   prev = t1 * t2
     *   for j in range(70):
     *     magnitude = log(abs(prev)+1) + real(prev)^2 - imag(prev)^2
     *     angle = angle(prev) + sin(real(prev)) - cos(imag(prev))
     *     cf[j+1] = magnitude * (cos(angle) + sin(angle)*1j)
     *     prev = prev * t1 - t2 / (j+1)
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    cRe[0] = x1r;
    cIm[0] = x2i;

    /* prev = t1 * t2 */
    double prevr, previ;
    c_mul(x1r, x1i, x2r, x2i, &prevr, &previ);

    for (int j = 0; j < 70; j++) {
        double magnitude = log(c_abs(prevr, previ) + 1.0)
                         + prevr * prevr - previ * previ;
        double angle_val = c_arg(prevr, previ) + sin(prevr) - cos(previ);
        cRe[j + 1] = magnitude * cos(angle_val);
        cIm[j + 1] = magnitude * sin(angle_val);

        /* prev = prev * t1 - t2 / (j+1) */
        double nr, ni;
        c_mul(prevr, previ, x1r, x1i, &nr, &ni);
        double inv = 1.0 / (j + 1.0);
        prevr = nr - x2r * inv;
        previ = ni - x2i * inv;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_215_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python:
     *   cf[j] = (real(t1) + (j+1)) * sin((j+1)*angle(t1))
     *           + conj(t2) * (imag(t1) + (j+1)) * cos((j+1)*angle(t2)) * 1j
     *
     * First term is real scalar.
     * Second term: conj(t2) * B * 1j where B is real.
     *   conj(t2) = (x2r, -x2i)
     *   conj(t2) * B * 1j = B * (x2r + (-x2i)*1j) * 1j
     *                      = B * (x2r*1j + (-x2i)*1j*1j)
     *                      = B * (x2i*(-1)*(-1) + x2r*1j)  wait let me be careful
     *   conj(t2) * 1j = (x2r - x2i*1j) * 1j = x2r*1j - x2i*1j*1j = x2i + x2r*1j
     *   So: conj(t2) * B * 1j = B*x2i + B*x2r*1j
     *
     * cf[j] real = A + B*x2i
     * cf[j] imag = B*x2r
     * where A = (x1r + jp1) * sin(jp1 * ang1)
     *       B = (x1i + jp1) * cos(jp1 * ang2)
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    for (int j = 0; j < 71; j++) {
        double jp1 = j + 1.0;
        double A = (x1r + jp1) * sin(jp1 * ang1);
        double B = (x1i + jp1) * cos(jp1 * ang2);
        cRe[j] = A + B * x2i;
        cIm[j] = B * x2r;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_216_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python:
     *   k = ((j+1)*5) % 20 + 1
     *   r = (j // 6) + 1
     *   cf[j] = (real(t1)^k + imag(t2)^r) * cos((j+1)*angle(t1))
     *           + conj(t2) * sin((j+1)*angle(t2))
     *           - real(t1*t2) * cos(j+1)
     *
     * conj(t2) = (x2r, -x2i)
     * real(t1*t2) = x1r*x2r - x1i*x2i
     *
     * Let S = sin((j+1)*ang2), C = cos((j+1)*ang1)
     * Let T = (x1r^k + x2i^r) * C  [real]
     * cf[j] = T + (x2r,-x2i)*S - rp*cos(j+1)
     *       = (T + x2r*S - rp*cos(j+1)) + (-x2i*S)*1j
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double real_prod = x1r * x2r - x1i * x2i;  /* real(t1*t2) */

    for (int j = 0; j < 71; j++) {
        double jp1 = j + 1.0;
        int kv = ((j + 1) * 5) % 20 + 1;
        int rv = (j / 6) + 1;
        double T = (pow(x1r, (double)kv) + pow(x2i, (double)rv)) * cos(jp1 * ang1);
        double S = sin(jp1 * ang2);
        cRe[j] = T + x2r * S - real_prod * cos(jp1);
        cIm[j] = -x2i * S;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_217_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python:
     *   angle = angle(t1)*(j+1) + angle(t2)*(71-(j+1))
     *   magnitude = abs(t1)^(j+1) * abs(t2)^(71-(j+1))
     *               + log(abs(t1)+1)*sin(j+1) + cos(j+1)
     *   cf[j] = magnitude * (cos(angle) + sin(angle)*1j)
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double log1 = log(abs1 + 1.0);

    for (int j = 0; j < 71; j++) {
        double jp1 = j + 1.0;
        double mj = 71.0 - jp1;
        double angle_val = ang1 * jp1 + ang2 * mj;
        double magnitude = pow(abs1, jp1) * pow(abs2, mj)
                         + log1 * sin(jp1) + cos(jp1);
        cRe[j] = magnitude * cos(angle_val);
        cIm[j] = magnitude * sin(angle_val);
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_218_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python:
     *   mag = sin((k+1)*abs(t1)) + cos((k+1)*abs(t2)) + log(abs(t1)+(k+1))
     *   angle = angle(t1)*(k+1) + angle(t2)*(71-(k+1))
     *   cf[k] = mag * (cos(angle) + sin(angle)*1j)
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    for (int k = 0; k < 71; k++) {
        double kp1 = k + 1.0;
        double mag = sin(kp1 * abs1) + cos(kp1 * abs2) + log(abs1 + kp1);
        double angle_val = ang1 * kp1 + ang2 * (71.0 - kp1);
        cRe[k] = mag * cos(angle_val);
        cIm[k] = mag * sin(angle_val);
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_219_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python:
     *   angle = angle(t1)*(k+1) - angle(t2)*(71-(k+1))
     *   magnitude = abs(t1)^(k+1) + log(abs(t2)+1)^(k+1)
     *   cf[k] = magnitude * (cos(angle) + sin(angle)*1j)
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double log2 = log(c_abs(x2r, x2i) + 1.0);

    for (int k = 0; k < 71; k++) {
        double kp1 = k + 1.0;
        double angle_val = ang1 * kp1 - ang2 * (71.0 - kp1);
        double magnitude = pow(abs1, kp1) + pow(log2, kp1);
        cRe[k] = magnitude * cos(angle_val);
        cIm[k] = magnitude * sin(angle_val);
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_220_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python:
     *   cf[j] = (real(t1)*(j+1) + imag(t2)*(j+1)^2) * sin(angle(t1)*(j+1))
     *           + cos(angle(t2)*(j+1)) * log(abs(t1) + abs(t2)*(j+1))
     *           + real(conj(t1)*t2)^(j+1)
     *           - imag(t1*conj(t2))^(j+1)
     *
     * conj(t1)*t2 = (x1r,-x1i)*(x2r,x2i) = (x1r*x2r+x1i*x2i, x1r*x2i-x1i*x2r)
     * real(conj(t1)*t2) = x1r*x2r + x1i*x2i
     *
     * t1*conj(t2) = (x1r,x1i)*(x2r,-x2i) = (x1r*x2r+x1i*x2i, x1i*x2r-x1r*x2i)
     * imag(t1*conj(t2)) = x1i*x2r - x1r*x2i
     *
     * Result is real.
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double rc = x1r * x2r + x1i * x2i;  /* real(conj(t1)*t2) */
    double ic = x1i * x2r - x1r * x2i;  /* imag(t1*conj(t2)) */

    for (int j = 0; j < 71; j++) {
        double jp1 = j + 1.0;
        double term1 = (x1r * jp1 + x2i * jp1 * jp1) * sin(ang1 * jp1);
        double term2 = cos(ang2 * jp1) * log(abs1 + abs2 * jp1);
        double term3 = pow(rc, jp1);
        double term4 = pow(ic, jp1);
        cRe[j] = term1 + term2 + term3 - term4;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}
