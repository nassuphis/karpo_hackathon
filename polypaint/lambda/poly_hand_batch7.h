/* poly_hand_batch7.h — Hand-written C for poly_198, poly_199, poly_123, poly_161, poly_103 */

static void poly_198_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    /*
     * Python:
     *   k = np.arange(71)
     *   cf = (np.real(t1) + np.imag(t2))**(70-k) * (np.abs(t1)+np.abs(t2))**k
     *        * np.sin(k*np.angle(t1) - np.angle(t2))
     *      + (np.real(t2) - np.imag(t1))**k * np.cos(k*np.angle(t2) + np.angle(t1))
     *      + np.log(np.abs(t1)+1) * np.log(np.abs(t2)+1) / (k+1)
     *
     * All operations are real (angle, abs, sin/cos of reals).
     * Result is real-valued coefficients.
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double a = x1r + x2i;                          /* real(t1) + imag(t2) */
    double b = c_abs(x1r, x1i) + c_abs(x2r, x2i); /* abs(t1) + abs(t2)  */
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double c = x2r - x1i;                          /* real(t2) - imag(t1) */
    double logTerm = log(c_abs(x1r, x1i) + 1.0) * log(c_abs(x2r, x2i) + 1.0);

    double b_pow = 1.0; /* b^k, start at k=0 */
    double c_pow = 1.0; /* c^k, start at k=0 */

    for (int k = 0; k < 71; k++) {
        double a70mk = pow(a, 70 - k);
        double bk    = b_pow;
        double ck    = c_pow;

        double term1 = a70mk * bk * sin(k * ang1 - ang2);
        double term2 = ck * cos(k * ang2 + ang1);
        double term3 = logTerm / (k + 1.0);

        cRe[k] = term1 + term2 + term3;
        /* cIm stays 0 — result is real */

        b_pow *= b;
        c_pow *= c;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_199_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    /*
     * Python:
     *   j = np.arange(71)
     *   cf = (np.real(t1)**j * np.sin(j * np.real(t2)))
     *      + (np.imag(t2)**j * np.cos(j * np.imag(t1))) / (j + 1)
     *
     * All real operations. Result is real-valued.
     * Note operator precedence: division binds to second term only.
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double r1 = x1r;           /* real(t1) */
    double r2 = x2r;           /* real(t2) */
    double im1 = x1i;          /* imag(t1) */
    double im2 = x2i;          /* imag(t2) */

    double r1_pow = 1.0;       /* real(t1)^j */
    double im2_pow = 1.0;      /* imag(t2)^j */

    for (int j = 0; j < 71; j++) {
        double term1 = r1_pow * sin(j * r2);
        double term2 = im2_pow * cos(j * im1) / (j + 1.0);
        cRe[j] = term1 + term2;

        r1_pow  *= r1;
        im2_pow *= im2;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_123_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    /*
     * Python (no try/except — always succeeds):
     *   cf[0]     = t1.real**2 - t2.imag**2
     *   cf[1]     = (t1+t2)**2 - 7
     *   cf[2]     = t1**2 - t2**2
     *   cf[3:10]  = arange(3,30,4) * abs(t1 + 1j*t2)   [7 elements]
     *   cf[10:20] = (t1-t2).real * arange(11,21)        [10 elements]
     *   cf[20:30] = 1/(1+arange(21,31)) * (t1+t2).real  [10 elements]
     *   cf[30]    = angle(t1) * t2.imag
     *   cf[31:50] = 1000 * (-1)**arange(32,51) * t1*t2  [19 elements]
     *   cf[50:60] = 2000 * (-1)**arange(51,61) * log(abs(t1)+1)  [10 elements]
     *   cf[60:65] = 1j * conj(t1*t2) * sqrt(arange(61,66))       [5 elements]
     *   cf[65:70] = arange(66,71)*(arange(66,71)-1) / (abs(t1)+abs(t2)+1)  [5 elements]
     *   cf[70]    = prod(arange(1,6)) = 120
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* cf[0] = t1.real**2 - t2.imag**2 */
    cRe[0] = x1r * x1r - x2i * x2i;

    /* cf[1] = (t1+t2)**2 - 7   (complex) */
    double sr = x1r + x2r, si = x1i + x2i;
    double s2r, s2i;
    c_mul(sr, si, sr, si, &s2r, &s2i);
    cRe[1] = s2r - 7.0;
    cIm[1] = s2i;

    /* cf[2] = t1**2 - t2**2  (complex) */
    double t1sq_r, t1sq_i, t2sq_r, t2sq_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    cRe[2] = t1sq_r - t2sq_r;
    cIm[2] = t1sq_i - t2sq_i;

    /* cf[3:10] = arange(3,30,4) * abs(t1 + 1j*t2)
     * t1 + 1j*t2 = (x1r + i*x1i) + i*(x2r + i*x2i) = (x1r - x2i) + i*(x1i + x2r)
     * arange(3,30,4) = [3,7,11,15,19,23,27] — 7 elements for indices 3..9 */
    double vr = x1r - x2i, vi = x1i + x2r;
    double vabs = c_abs(vr, vi);
    {
        int vals[] = {3, 7, 11, 15, 19, 23, 27};
        for (int i = 0; i < 7; i++) {
            cRe[3 + i] = vals[i] * vabs;
        }
    }

    /* cf[10:20] = (t1-t2).real * arange(11,21)
     * (t1-t2).real = x1r - x2r */
    double diffReal = x1r - x2r;
    for (int i = 0; i < 10; i++) {
        cRe[10 + i] = diffReal * (11 + i);
    }

    /* cf[20:30] = 1/(1+arange(21,31)) * (t1+t2).real
     * (t1+t2).real = x1r + x2r */
    double sumReal = x1r + x2r;
    for (int i = 0; i < 10; i++) {
        cRe[20 + i] = sumReal / (1.0 + (21 + i));
    }

    /* cf[30] = angle(t1) * t2.imag */
    cRe[30] = c_arg(x1r, x1i) * x2i;

    /* cf[31:50] = 1000 * (-1)**arange(32,51) * t1*t2
     * t1*t2 is complex */
    double pr, pi;
    c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
    for (int i = 0; i < 19; i++) {
        int exp = 32 + i;
        double sign = (exp % 2 == 0) ? 1.0 : -1.0;
        cRe[31 + i] = 1000.0 * sign * pr;
        cIm[31 + i] = 1000.0 * sign * pi;
    }

    /* cf[50:60] = 2000 * (-1)**arange(51,61) * log(abs(t1)+1)
     * log(abs(t1)+1) is real, result is real */
    double logAbs1 = log(c_abs(x1r, x1i) + 1.0);
    for (int i = 0; i < 10; i++) {
        int exp = 51 + i;
        double sign = (exp % 2 == 0) ? 1.0 : -1.0;
        cRe[50 + i] = 2000.0 * sign * logAbs1;
    }

    /* cf[60:65] = 1j * conj(t1*t2) * sqrt(arange(61,66))
     * conj(t1*t2) = (pr, -pi)
     * 1j * (pr, -pi) = (pi, pr)  [since i*(a-bi) = ai - bi² = b + ai → wait]
     * Actually: 1j * (pr - pi*i) = 1j*pr + 1j*(-pi*i) = pr*i + pi = (pi, pr) */
    /* Wait: conj(t1*t2) = (pr, -pi). Multiply by 1j = (0,1):
     * (0+1i)*(pr - pi*i) = -(-pi) + pr*i = pi + pr*i  → (pi, pr) */
    for (int i = 0; i < 5; i++) {
        double sqrtVal = sqrt(61.0 + i);
        cRe[60 + i] = pi * sqrtVal;
        cIm[60 + i] = pr * sqrtVal;
    }

    /* cf[65:70] = arange(66,71)*(arange(66,71)-1) / (abs(t1)+abs(t2)+1) */
    double denom = c_abs(x1r, x1i) + c_abs(x2r, x2i) + 1.0;
    for (int i = 0; i < 5; i++) {
        double n = 66.0 + i;
        cRe[65 + i] = n * (n - 1.0) / denom;
    }

    /* cf[70] = prod(arange(1,6)) = 1*2*3*4*5 = 120 */
    cRe[70] = 120.0;

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_161_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    /*
     * Python:
     *   k = np.arange(1, 71)
     *   cf[:70] = (t1**((k%4)+1) * t2**((k%3)+1))
     *           + (-1)**k * log(abs(t1)+1) * sin(k*t2)
     *   cf[70]  = t1*t2 / (1 + t1**2 + t2**2)
     *
     * t1**n and t2**n are complex powers (integer).
     * sin(k*t2) is complex sin since t2 is complex.
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double logAbs1 = log(c_abs(x1r, x1i) + 1.0);

    /* Pre-compute t1^1..t1^4 and t2^1..t2^3 */
    double t1p_r[5], t1p_i[5]; /* t1^0 .. t1^4 */
    double t2p_r[4], t2p_i[4]; /* t2^0 .. t2^3 */
    t1p_r[0] = 1.0; t1p_i[0] = 0.0;
    t2p_r[0] = 1.0; t2p_i[0] = 0.0;
    for (int n = 1; n <= 4; n++) {
        c_mul(t1p_r[n-1], t1p_i[n-1], x1r, x1i, &t1p_r[n], &t1p_i[n]);
    }
    for (int n = 1; n <= 3; n++) {
        c_mul(t2p_r[n-1], t2p_i[n-1], x2r, x2i, &t2p_r[n], &t2p_i[n]);
    }

    for (int k = 1; k <= 70; k++) {
        int p1 = (k % 4) + 1;  /* 1..4 */
        int p2 = (k % 3) + 1;  /* 1..3 */

        /* t1^p1 * t2^p2 */
        double mr, mi;
        c_mul(t1p_r[p1], t1p_i[p1], t2p_r[p2], t2p_i[p2], &mr, &mi);

        /* sin(k * t2) — complex sin */
        double kt2r = k * x2r, kt2i = k * x2i;
        double sinr, sini;
        c_sin(kt2r, kt2i, &sinr, &sini);

        double sign = (k % 2 == 0) ? 1.0 : -1.0;

        cRe[k - 1] = mr + sign * logAbs1 * sinr;
        cIm[k - 1] = mi + sign * logAbs1 * sini;
    }

    /* cf[70] = t1*t2 / (1 + t1^2 + t2^2) */
    double numr, numi;
    c_mul(x1r, x1i, x2r, x2i, &numr, &numi);

    double denr = 1.0 + t1p_r[2] + t2p_r[2];  /* 1 + t1^2 + t2^2 (real parts) */
    double deni = t1p_i[2] + t2p_i[2];         /* imaginary parts */

    /* But actually 1 + t1^2 + t2^2 is complex: (1 + re(t1^2) + re(t2^2)) + i*(im(t1^2) + im(t2^2)) */
    c_div(numr, numi, denr, deni, &cRe[70], &cIm[70]);

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_103_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    /*
     * Python:
     *   cf[0] = t2 * log(abs(t1)+1)
     *   cf[1] = t1 * log(abs(t2)+1)
     *   for k in range(2,32,2):
     *       cf[k]   = sin(k*t1) * log(abs(t2)+1)
     *       cf[k+1] = cos(k*t2) * log(abs(t1)+1)
     *   for k in range(32,52,2):
     *       cf[k]   = cos(k*t1) * log(abs(t2)+1)
     *       cf[k+1] = sin(k*t2) * log(abs(t1)+1)
     *   for k in range(52,72,2):
     *       cf[k]   = t1 * log(abs(t2*(k+1))+1)
     *       cf[k+1] = t2 * log(abs(t1*(k+1))+1)
     *   (Note: k=70 → cf[71] is OOB in Python, caught by except.
     *    We cap at k=68 so last pair is cf[68],cf[69]; cf[70]=0.)
     *
     *   Post-processing:
     *   mod_cf = (71 - arange(1,72)) * abs(cf)
     *   arg_cf = arange(1,72)/71 * angle(cf)
     *   cf = mod_cf * exp(1j * arg_cf)
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double logAbs1 = log(c_abs(x1r, x1i) + 1.0);
    double logAbs2 = log(c_abs(x2r, x2i) + 1.0);

    /* cf[0] = t2 * log(abs(t1)+1)  — complex * real */
    cRe[0] = x2r * logAbs1;
    cIm[0] = x2i * logAbs1;

    /* cf[1] = t1 * log(abs(t2)+1) */
    cRe[1] = x1r * logAbs2;
    cIm[1] = x1i * logAbs2;

    /* for k in range(2,32,2): sin(k*t1)*log(abs(t2)+1), cos(k*t2)*log(abs(t1)+1) */
    for (int k = 2; k < 32; k += 2) {
        double sr, si;
        c_sin(k * x1r, k * x1i, &sr, &si);
        cRe[k]   = sr * logAbs2;
        cIm[k]   = si * logAbs2;

        double cr, ci;
        c_cos(k * x2r, k * x2i, &cr, &ci);
        cRe[k+1] = cr * logAbs1;
        cIm[k+1] = ci * logAbs1;
    }

    /* for k in range(32,52,2): cos(k*t1)*log(abs(t2)+1), sin(k*t2)*log(abs(t1)+1) */
    for (int k = 32; k < 52; k += 2) {
        double cr, ci;
        c_cos(k * x1r, k * x1i, &cr, &ci);
        cRe[k]   = cr * logAbs2;
        cIm[k]   = ci * logAbs2;

        double sr, si;
        c_sin(k * x2r, k * x2i, &sr, &si);
        cRe[k+1] = sr * logAbs1;
        cIm[k+1] = si * logAbs1;
    }

    /* for k in range(52,72,2): cap at k=68 to avoid cf[71] OOB
     * cf[k]   = t1 * log(abs(t2*(k+1))+1)
     * cf[k+1] = t2 * log(abs(t1*(k+1))+1)
     * t2*(k+1) means scalar multiply: magnitude scales by (k+1) */
    for (int k = 52; k <= 68; k += 2) {
        double mag_t2_k1 = c_abs(x2r, x2i) * (k + 1);
        double logVal2 = log(mag_t2_k1 + 1.0);
        cRe[k]   = x1r * logVal2;
        cIm[k]   = x1i * logVal2;

        double mag_t1_k1 = c_abs(x1r, x1i) * (k + 1);
        double logVal1 = log(mag_t1_k1 + 1.0);
        cRe[k+1] = x2r * logVal1;
        cIm[k+1] = x2i * logVal1;
    }
    /* cf[70] stays 0 (would be OOB pair in Python) */

    /* Post-processing:
     * mod_cf[i] = (71 - (i+1)) * abs(cf[i])     = (70-i) * abs(cf[i])
     * arg_cf[i] = (i+1)/71 * angle(cf[i])
     * cf[i] = mod_cf[i] * exp(1j * arg_cf[i])
     *       = mod_cf[i] * (cos(arg_cf[i]) + i*sin(arg_cf[i]))
     */
    for (int i = 0; i < 71; i++) {
        double mag = c_abs(cRe[i], cIm[i]);
        double ang = c_arg(cRe[i], cIm[i]);
        double mod_cf = (70.0 - i) * mag;
        double arg_cf = (i + 1.0) / 71.0 * ang;
        cRe[i] = mod_cf * cos(arg_cf);
        cIm[i] = mod_cf * sin(arg_cf);
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}
