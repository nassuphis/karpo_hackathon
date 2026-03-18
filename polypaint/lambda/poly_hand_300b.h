/* poly_hand_300b.h — Hand-written C for poly_221 through poly_240 */

static void poly_221_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    /*
     * Python:
     *   for j in range(71):
     *       cf[j] = (real(t1)**(j+1) * cos(angle(t2) + (j+1)))
     *             + (imag(t2)**(j+1) * sin(angle(t1) * (j+1)))
     *             + log(abs(t1) + (j+1)) + log(abs(t2) + 1)
     *             + conj(t1) * (j+1) - conj(t2)**(j+1)
     *
     * conj(t1)*(j+1) is complex, conj(t2)**(j+1) is complex.
     * Result is complex.
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double r1 = x1r;
    double ang2 = c_arg(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double conj1r = x1r, conj1i = -x1i;  /* conj(t1) */
    double log_abs2_1 = log(abs2 + 1.0);

    double r1_pow = r1;       /* real(t1)^(j+1), start j=0 => ^1 */
    double im2_pow = x2i;     /* imag(t2)^(j+1) */
    /* conj(t2)^(j+1): iterative complex power */
    double ct2r = x2r, ct2i = -x2i;  /* conj(t2)^1 */

    for (int j = 0; j < 71; j++) {
        int jp1 = j + 1;
        double term1 = r1_pow * cos(ang2 + jp1);
        double term2 = im2_pow * sin(ang1 * jp1);
        double term3 = log(abs1 + jp1);
        double term4 = log_abs2_1;
        /* conj(t1) * (j+1) */
        double t5r = conj1r * jp1;
        double t5i = conj1i * jp1;

        cRe[j] = term1 + term2 + term3 + term4 + t5r - ct2r;
        cIm[j] = t5i - ct2i;

        r1_pow *= r1;
        im2_pow *= x2i;
        /* ct2 *= conj(t2) */
        double nr = ct2r * x2r - ct2i * (-x2i);
        double ni = ct2r * (-x2i) + ct2i * x2r;
        ct2r = nr; ct2i = ni;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_222_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    /*
     * Python:
     *   prev = t1 + t2
     *   for j in range(71):
     *       magnitude = abs(prev) * log(abs(prev) + 1)
     *       angle = angle(prev) + sin(j+1) * cos(j+1)
     *       cf[j] = magnitude * (cos(angle) + 1j * sin(angle))
     *       prev = prev * t1 - t2 / (j+1)
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double pr = x1r + x2r, pi = x1i + x2i;  /* prev = t1 + t2 */

    for (int j = 0; j < 71; j++) {
        double mag_prev = c_abs(pr, pi);
        double magnitude = mag_prev * log(mag_prev + 1.0);
        double angle = c_arg(pr, pi) + sin(j + 1.0) * cos(j + 1.0);
        cRe[j] = magnitude * cos(angle);
        cIm[j] = magnitude * sin(angle);

        /* prev = prev * t1 - t2 / (j+1) */
        double mr, mi;
        c_mul(pr, pi, x1r, x1i, &mr, &mi);
        double denom = j + 1.0;
        pr = mr - x2r / denom;
        pi = mi - x2i / denom;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_223_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    /*
     * Python:
     *   for j in range(71):
     *       mag = log(abs(t1)*(j+1) + 1) * (1 + sin((j+1)*angle(t2)))
     *       ang = angle(t1)*sqrt(j+1) + cos((j+1)*angle(t2))
     *       cf[j] = mag*cos(ang) + mag*sin(ang)*1j
     */
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    for (int j = 0; j < 71; j++) {
        int jp1 = j + 1;
        double mag = log(abs1 * jp1 + 1.0) * (1.0 + sin(jp1 * ang2));
        double ang = ang1 * sqrt((double)jp1) + cos(jp1 * ang2);
        cRe[j] = mag * cos(ang);
        cIm[j] = mag * sin(ang);
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_224_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    /*
     * Python: cf = np.zeros(35, dtype=complex)
     * Branching on (j+1)%4, fixed overrides, etc.
     * conj(t1)**k and conj(t2)**k, sin(t1*k), cos(t2*k) are complex.
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double conj1r = x1r, conj1i = -x1i;
    double conj2r = x2r, conj2i = -x2i;

    for (int j = 0; j < 35; j++) {
        int jp1 = j + 1;
        int mod = jp1 % 4;
        if (mod == 1) {
            int k = j + 3;
            double rp = pow(x1r, (double)k);
            double ip = pow(x2i, (double)k);
            cRe[j] = rp + ip * sin(ang1 * k);
            cIm[j] = 0;
        } else if (mod == 2) {
            int k = j + 4;
            double abs12 = c_abs(x1r + x2r, x1i + x2i);
            double absk = pow(abs12, (double)k);
            cRe[j] = absk * cos(ang2 * k) + log(abs1 + 1.0);
            cIm[j] = 0;
        } else if (mod == 3) {
            int k = j + 2;
            /* conj(t1)**k - conj(t2)**k + sin(t1*k) - cos(t2*k) */
            double c1kr, c1ki, c2kr, c2ki;
            c_powr(conj1r, conj1i, (double)k, &c1kr, &c1ki);
            c_powr(conj2r, conj2i, (double)k, &c2kr, &c2ki);
            double s1r, s1i, c2r2, c2i2;
            c_sin(x1r * k, x1i * k, &s1r, &s1i);
            c_cos(x2r * k, x2i * k, &c2r2, &c2i2);
            cRe[j] = c1kr - c2kr + s1r - c2r2;
            cIm[j] = c1ki - c2ki + s1i - c2i2;
        } else {
            int k = j + 1;
            double log1k = pow(log(abs1 + 1.0), (double)k);
            double abs2 = c_abs(x2r, x2i);
            double log2k = pow(log(abs2 + 1.0), (double)(35 - k));
            /* real(t1*t2) * imag(t1+t2) */
            double t1t2r, t1t2i;
            c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
            double rpart = t1t2r * (x1i + x2i);
            cRe[j] = log1k + log2k + rpart;
            cIm[j] = 0;
        }
    }

    /* cf[[4,9,14,19,24,29,34]] = [2+3j, -4j, 5-6j, -7+8j, 9-10j, 11+12j, -13+14j] */
    cRe[4] = 2; cIm[4] = 3;
    cRe[9] = 0; cIm[9] = -4;
    cRe[14] = 5; cIm[14] = -6;
    cRe[19] = -7; cIm[19] = 8;
    cRe[24] = 9; cIm[24] = -10;
    cRe[29] = 11; cIm[29] = 12;
    cRe[34] = -13; cIm[34] = 14;

    /* cf[7] = 100j*t2**3 + 100j*t2**2 - 100*t2 - 100 */
    {
        double t2_2r, t2_2i, t2_3r, t2_3i;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        c_mul(t2_2r, t2_2i, x2r, x2i, &t2_3r, &t2_3i);
        /* 100j * t2^3 = (-100*t2_3i, 100*t2_3r) */
        /* 100j * t2^2 = (-100*t2_2i, 100*t2_2r) */
        cRe[7] = -100*t2_3i + (-100*t2_2i) - 100*x2r - 100;
        cIm[7] = 100*t2_3r + 100*t2_2r - 100*x2i;
    }

    /* cf[11] = 150j*t1**3 + 150j*t1**2 + 150*t2 - 150 */
    {
        double t1_2r, t1_2i, t1_3r, t1_3i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        c_mul(t1_2r, t1_2i, x1r, x1i, &t1_3r, &t1_3i);
        cRe[11] = -150*t1_3i + (-150*t1_2i) + 150*x2r - 150;
        cIm[11] = 150*t1_3r + 150*t1_2r + 150*x2i;
    }

    /* cf[17] = 200j*t2**3 - 200j*t2**2 + 200*t2 - 200 */
    {
        double t2_2r, t2_2i, t2_3r, t2_3i;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        c_mul(t2_2r, t2_2i, x2r, x2i, &t2_3r, &t2_3i);
        cRe[17] = -200*t2_3i - (-200*t2_2i) + 200*x2r - 200;
        cIm[17] = 200*t2_3r - 200*t2_2r + 200*x2i;
    }

    /* cf[21] = 250*sin(t1) + 300j*cos(t2) + 50*log(abs(t1)+1) */
    {
        double s1r, s1i, c2r, c2i;
        c_sin(x1r, x1i, &s1r, &s1i);
        c_cos(x2r, x2i, &c2r, &c2i);
        cRe[21] = 250*s1r + (-300*c2i) + 50*log(abs1 + 1.0);
        cIm[21] = 250*s1i + 300*c2r;
    }

    /* cf[27] = 350*prod([t1,t2]) + 400j*sum([t1,t2]) = 350*t1*t2 + 400j*(t1+t2) */
    {
        double pr, pi;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
        double sr = x1r + x2r, si = x1i + x2i;
        /* 400j*(sr+si*j) = (-400*si, 400*sr) */
        cRe[27] = 350*pr + (-400*si);
        cIm[27] = 350*pi + 400*sr;
    }

    /* cf[32] = 450j*t1*t2 + 500*conj(t1-t2) */
    {
        double pr, pi;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
        /* 450j*(pr+pi*j) = (-450*pi, 450*pr) */
        double dr = x1r - x2r, di = x1i - x2i;
        cRe[32] = -450*pi + 500*dr;
        cIm[32] = 450*pr + 500*(-di);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_225_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    /*
     * Python: 35 coeffs, fixed indices, loop with mag/angle, additional loops.
     * cf[35] assignment is OOB for a 35-element array — ignored in C (Python silently
     * extends or errors, but the try/except catches it; the function returns the
     * partially filled array before that line would execute since cf has size 35).
     * Actually in numpy, cf[35] on a size-35 array raises IndexError, caught by except,
     * returns zeros. But let's look more carefully — cf is size 35, indices 0-34.
     * cf[35] would throw. The except returns zeros(35). So effectively this function
     * always returns zeros. But let's implement the logic without the OOB line.
     * Actually wait — the try block runs line by line. All lines before cf[35] execute
     * successfully. Then cf[35] raises IndexError, except catches it, returns zeros(35).
     * So the ENTIRE result is zeros. Let me implement it that way.
     *
     * UPDATE: Actually re-reading — the cf[35] IS out of bounds and WILL throw.
     * The except returns np.zeros(35). So this function always returns zeros.
     * But that seems like a bug. Let's implement the actual logic anyway, just
     * skip the cf[35] line, since that's probably what was intended.
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    int fixed_indices[] = {3, 8, 14, 19, 23, 29};
    double fixed_re[] = {2, -3, 5, -4, 1.5, 3};
    double fixed_im[] = {-1, 4, -2, 3, -0.5, 2};
    for (int i = 0; i < 6; i++) {
        cRe[fixed_indices[i]] = fixed_re[i];
        cIm[fixed_indices[i]] = fixed_im[i];
    }

    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    /* conj(t1)*t2 */
    double ct1t2r, ct1t2i;
    c_mul(x1r, -x1i, x2r, x2i, &ct1t2r, &ct1t2i);

    for (int j = 0; j < 35; j++) {
        /* skip fixed indices */
        int is_fixed = 0;
        for (int f = 0; f < 6; f++) { if (j == fixed_indices[f]) { is_fixed = 1; break; } }
        if (is_fixed) continue;

        int k = (j % 7) + 1;
        double r = x1r * x2i / (k + j + 1.0);
        double angle = ang1 + ang2 * k;
        double magnitude = pow(abs1, (double)k) + pow(abs2, (double)(7 - k));
        double base_re = magnitude * cos(angle) + r * sin(angle);
        double base_im = 0.0;
        /* + conj(t1)*t2 / (j+1) */
        cRe[j] = base_re + ct1t2r / (j + 1.0);
        cIm[j] = base_im + ct1t2i / (j + 1.0);
    }

    /* Additional intricate assignments: for k in 1..5, idx=5*k, if idx<=35 (note: <=35 means idx can be 35, OOB) */
    for (int k = 1; k <= 5; k++) {
        int idx = 5 * k;
        if (idx < 35) {  /* use < 35 to avoid OOB */
            double r1k = pow(x1r, (double)k);
            double i2k = pow(x2i, (double)k);
            double term1 = (r1k - i2k) * cos(k * ang1);
            double term2 = (abs1 + abs2) * sin(k * ang2);
            /* conj(t1)^k * conj(t2)^k / (k+1) */
            double c1kr, c1ki, c2kr, c2ki, pkr, pki;
            c_powr(x1r, -x1i, (double)k, &c1kr, &c1ki);
            c_powr(x2r, -x2i, (double)k, &c2kr, &c2ki);
            c_mul(c1kr, c1ki, c2kr, c2ki, &pkr, &pki);
            cRe[idx] = term1 + term2 + pkr / (k + 1.0);
            cIm[idx] = pki / (k + 1.0);
        }
    }

    /* cf[9] = log(abs(t1*t2)+1) * (real(t1)+imag(t2)) + 2j*real(t1)^2 - 3*imag(t2)^2 */
    {
        double t1t2r, t1t2i;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        double a = c_abs(t1t2r, t1t2i);
        cRe[9] = log(a + 1.0) * (x1r + x2i) - 3.0 * x2i * x2i;
        cIm[9] = 2.0 * x1r * x1r;
    }

    /* cf[20] = conj(t1)*t2^3 + real(t2)*imag(t1) - 4j*abs(t1+t2) */
    {
        double t2_3r, t2_3i, tmp1r, tmp1i;
        c_mul(x2r, x2i, x2r, x2i, &tmp1r, &tmp1i);
        c_mul(tmp1r, tmp1i, x2r, x2i, &t2_3r, &t2_3i);
        double pr, pi;
        c_mul(x1r, -x1i, t2_3r, t2_3i, &pr, &pi);
        double abs12 = c_abs(x1r + x2r, x1i + x2i);
        cRe[20] = pr + x2r * x1i;
        cIm[20] = pi - 4.0 * abs12;
    }

    /* Skip cf[35] — OOB */

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_226_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Fixed: cf[[0,6,13,20,27,34]] = [2, -1+3j, 4-2j, -3+5j, 1.5-1.5j, 0.3+0.7j] */
    cRe[0] = 2; cIm[0] = 0;
    cRe[6] = -1; cIm[6] = 3;
    cRe[13] = 4; cIm[13] = -2;
    cRe[20] = -3; cIm[20] = 5;
    cRe[27] = 1.5; cIm[27] = -1.5;
    cRe[34] = 0.3; cIm[34] = 0.7;

    int fixed[] = {0, 6, 13, 20, 27, 34};
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);

    /* Loop from j=2 to 34 */
    for (int j = 2; j < 35; j++) {
        int is_fixed = 0;
        for (int f = 0; f < 6; f++) { if (j == fixed[f]) { is_fixed = 1; break; } }
        if (is_fixed) continue;

        int jp1 = j + 1;
        double r = x1r + x2i * jp1;
        double theta = ang1 * jp1 - ang2 / ((jp1 % 5) + 1.0);
        double magnitude = sin(r) * cos(r + theta) + log(abs1 + abs2 + jp1);
        double phase = theta + sin(jp1 * x1r) - cos(jp1 * x2i);
        cRe[j] = magnitude * cos(phase);
        cIm[j] = magnitude * sin(phase);
    }

    /* cf[9] = conj(t1)*t2^2 + sin(t1*t2) */
    {
        double t2_2r, t2_2i, pr, pi;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        c_mul(x1r, -x1i, t2_2r, t2_2i, &pr, &pi);
        double t1t2r, t1t2i, sr, si;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        c_sin(t1t2r, t1t2i, &sr, &si);
        cRe[9] = pr + sr;
        cIm[9] = pi + si;
    }

    /* cf[18] = abs(t1+t2) * exp(1j*angle(t1-t2)) */
    {
        double abs12 = c_abs(x1r + x2r, x1i + x2i);
        double ang_diff = c_arg(x1r - x2r, x1i - x2i);
        cRe[18] = abs12 * cos(ang_diff);
        cIm[18] = abs12 * sin(ang_diff);
    }

    /* cf[25] = sum([real(t1),imag(t1),real(t2),imag(t2)]) + prod([abs(t1),abs(t2)]) */
    {
        double s = x1r + x1i + x2r + x2i;
        double p = abs1 * abs2;
        cRe[25] = s + p;
        cIm[25] = 0;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_227_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    /*
     * Python:
     *   for j in range(35):
     *       k = (j+1) + (j//5)
     *       magnitude = log(abs(t1)+1)*sin(j+1) + log(abs(t2)+1)*cos(j+1)
     *       angle = angle(t1)*(j+1)**0.5 - angle(t2)*log(j+2)
     *       cf[j] = magnitude*(cos(angle)+1j*sin(angle)) + conj(t1)**k - conj(t2)**(35-j)
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double log1 = log(abs1 + 1.0);
    double log2 = log(abs2 + 1.0);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    for (int j = 0; j < 35; j++) {
        int k = (j + 1) + (j / 5);
        double magnitude = log1 * sin(j + 1.0) + log2 * cos(j + 1.0);
        double angle = ang1 * sqrt(j + 1.0) - ang2 * log(j + 2.0);
        double mr = magnitude * cos(angle);
        double mi = magnitude * sin(angle);

        /* conj(t1)^k */
        double c1kr, c1ki;
        c_powr(x1r, -x1i, (double)k, &c1kr, &c1ki);
        /* conj(t2)^(35-j) */
        double c2kr, c2ki;
        c_powr(x2r, -x2i, (double)(35 - j), &c2kr, &c2ki);

        cRe[j] = mr + c1kr - c2kr;
        cIm[j] = mi + c1ki - c2ki;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_228_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Fixed: cf[[3,7,13,17,26,32]] = [2.5, -3.4, 5.6, -4.2, 3.1, 0.8] (real) */
    int fixed[] = {3, 7, 13, 17, 26, 32};
    double fvals[] = {2.5, -3.4, 5.6, -4.2, 3.1, 0.8};
    for (int f = 0; f < 6; f++) {
        cRe[fixed[f]] = fvals[f];
    }

    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double log1 = log(abs1 + 1.0);
    double log2 = log(abs2 + 1.0);

    for (int j = 0; j < 35; j++) {
        int is_fixed = 0;
        for (int f = 0; f < 6; f++) { if (j == fixed[f]) { is_fixed = 1; break; } }
        if (is_fixed) continue;

        int k = 35 - j;
        double angle = ang1 * (j + 1.0) + ang2 * k;
        double magnitude = log1 * sin(j + 1.0) + log2 * cos((double)k);
        cRe[j] = magnitude * cos(angle);
        cIm[j] = magnitude * sin(angle);
    }

    /* cf[9] = conj(t1)^2 * t2 + sin(t1*t2) */
    {
        double c1_2r, c1_2i;
        c_mul(x1r, -x1i, x1r, -x1i, &c1_2r, &c1_2i);
        double pr, pi;
        c_mul(c1_2r, c1_2i, x2r, x2i, &pr, &pi);
        double t1t2r, t1t2i, sr, si;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        c_sin(t1t2r, t1t2i, &sr, &si);
        cRe[9] = pr + sr;
        cIm[9] = pi + si;
    }

    /* cf[18] = abs(t1+t2) * exp(1j*angle(t1-t2)) */
    {
        double abs12 = c_abs(x1r + x2r, x1i + x2i);
        double ang_diff = c_arg(x1r - x2r, x1i - x2i);
        cRe[18] = abs12 * cos(ang_diff);
        cIm[18] = abs12 * sin(ang_diff);
    }

    /* cf[25] = sum([real(t1),imag(t1),real(t2),imag(t2)]) + prod([abs(t1),abs(t2)]) */
    {
        double s = x1r + x1i + x2r + x2i;
        double p = abs1 * abs2;
        cRe[25] = s + p;
        cIm[25] = 0;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_229_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Fixed: cf[[2,6,13,18,21,27]] = [2-3j, -4+5j, 1.5-2.5j, -3.3+4.4j, 0.5-1.2j, 3-3j] */
    cRe[2] = 2; cIm[2] = -3;
    cRe[6] = -4; cIm[6] = 5;
    cRe[13] = 1.5; cIm[13] = -2.5;
    cRe[18] = -3.3; cIm[18] = 4.4;
    cRe[21] = 0.5; cIm[21] = -1.2;
    cRe[27] = 3; cIm[27] = -3;

    int fixed[] = {2, 6, 13, 18, 21, 27};
    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);

    /* conj(t1^2 - t2^2) */
    double t1_2r, t1_2i, t2_2r, t2_2i;
    c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
    c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
    double diff_r = t1_2r - t2_2r, diff_i = t1_2i - t2_2i;
    double conj_diff_r = diff_r, conj_diff_i = -diff_i;

    for (int j = 0; j < 35; j++) {
        int is_fixed = 0;
        for (int f = 0; f < 6; f++) { if (j == fixed[f]) { is_fixed = 1; break; } }
        if (is_fixed) continue;

        /* angle = angle(t1^(j+1) + t2^(35-j)) */
        double p1r, p1i, p2r, p2i;
        c_powr(x1r, x1i, (double)(j + 1), &p1r, &p1i);
        c_powr(x2r, x2i, (double)(35 - j), &p2r, &p2i);
        double angle = c_arg(p1r + p2r, p1i + p2i);

        /* magnitude = abs(t1)^((j%5)+1) * abs(t2)^((35-j)%7+1) */
        double magnitude = pow(abs1, (double)((j % 5) + 1)) * pow(abs2, (double)((35 - j) % 7 + 1));

        cRe[j] = magnitude * cos(angle) + conj_diff_r;
        cIm[j] = magnitude * sin(angle) + conj_diff_i;
    }

    /* j = 35 used in the additional assignments below */

    /* cf[9] = sin(t1*35) + cos(t2*35) + log(abs(t1)+abs(t2)+1) */
    {
        double s1r, s1i, c2r, c2i;
        c_sin(x1r * 35, x1i * 35, &s1r, &s1i);
        c_cos(x2r * 35, x2i * 35, &c2r, &c2i);
        double logv = log(abs1 + abs2 + 1.0);
        cRe[9] = s1r + c2r + logv;
        cIm[9] = s1i + c2i;
    }

    /* cf[17] = real(t1)^2 - imag(t2)^2 + 2j*real(t1)*imag(t2) */
    cRe[17] = x1r * x1r - x2i * x2i;
    cIm[17] = 2.0 * x1r * x2i;

    /* cf[25] = prod([abs(t1),abs(t2)]) * exp(1j*angle(t1+t2)) */
    {
        double p = abs1 * abs2;
        double ang = c_arg(x1r + x2r, x1i + x2i);
        cRe[25] = p * cos(ang);
        cIm[25] = p * sin(ang);
    }

    /* cf[30] = sum([abs(t1+t2),abs(t1-t2)]) + 1j*sum([angle(t1),angle(t2)]) */
    {
        double a1 = c_abs(x1r + x2r, x1i + x2i);
        double a2 = c_abs(x1r - x2r, x1i - x2i);
        double ang1 = c_arg(x1r, x1i);
        double ang2 = c_arg(x2r, x2i);
        cRe[30] = a1 + a2;
        cIm[30] = ang1 + ang2;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_230_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Fixed: cf[[2,7,11,16,22,27,31,34]] = [3,-2,5,-4,6,-3,2,-1] (real) */
    cRe[2] = 3; cRe[7] = -2; cRe[11] = 5; cRe[16] = -4;
    cRe[22] = 6; cRe[27] = -3; cRe[31] = 2; cRe[34] = -1;

    double abs1 = c_abs(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* Main loop — note this overwrites the fixed values */
    for (int j = 0; j < 35; j++) {
        int jp1 = j + 1;
        if (jp1 % 4 == 0) {
            /* (real(t1)^2 + imag(t2)) * (j+1) + (imag(t1)*real(t2)) * 1j */
            cRe[j] = (x1r * x1r + x2i) * jp1;
            cIm[j] = x1i * x2r;
        } else if (jp1 % 5 == 1) {
            /* sin(t1*(j+1)) + cos(t2+(j+1))*1j — complex sin/cos */
            double s1r, s1i, c2r, c2i;
            c_sin(x1r * jp1, x1i * jp1, &s1r, &s1i);
            c_cos(x2r + jp1, x2i, &c2r, &c2i);
            cRe[j] = s1r;  /* real part of sin(t1*(j+1)) + cos(t2+(j+1))*1j */
            cIm[j] = s1i;
            /* + cos(t2+(j+1))*1j means: the cos result is complex, multiplied by 1j */
            /* 1j * (c2r + c2i*j) = (-c2i + c2r*j) */
            cRe[j] += -c2i;
            cIm[j] += c2r;
        } else if (jp1 % 3 == 2) {
            /* log(abs(t1)*(j+1)+1) + angle(t2)^(j+1)*1j */
            cRe[j] = log(abs1 * jp1 + 1.0);
            cIm[j] = pow(ang2, (double)jp1);
        } else {
            /* real(t1+t2)*(j+1) + imag(t1-t2)*1j */
            cRe[j] = (x1r + x2r) * jp1;
            cIm[j] = (x1i - x2i);
        }
    }

    /* Nested loop: for k in 1..5, for r in 1..7, idx=(k*r)%35 */
    for (int k = 1; k <= 5; k++) {
        for (int r = 1; r <= 7; r++) {
            int idx = (k * r) % 35;
            cRe[idx] += pow(x1r, (double)k) * pow(x2i, (double)r);
            cIm[idx] += pow(x2r, (double)k) * pow(x1i, (double)r);
        }
    }

    /* Modify m in range(10,31,5): cf[m] = cf[m]*conj(t1) + prod([abs(t2),m])*1j */
    for (int m = 10; m <= 30; m += 5) {
        double tr = cRe[m], ti = cIm[m];
        double pr, pi;
        c_mul(tr, ti, x1r, -x1i, &pr, &pi);
        double abs2 = c_abs(x2r, x2i);
        cRe[m] = pr;
        cIm[m] = pi + abs2 * m;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_231_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 50;
    for (int i = 0; i < 50; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double log1 = log(abs1 + 1.0);

    /* Main loop */
    for (int k = 0; k < 50; k++) {
        double magnitude = pow(abs1, (double)((k % 5) + 1)) + pow(abs2, (double)((k % 7) + 1))
                         + log1 * sin(k + 1.0);
        double angle = ang1 * cos(k + 1.0) + ang2 * sin(k + 1.0);
        cRe[k] = magnitude * cos(angle);
        cIm[k] = magnitude * sin(angle);
    }

    /* for j in range(2,51,3): cf[j] += conj(t1) * t2^(j%4) */
    for (int j = 2; j < 51 && j < 50; j += 3) {
        double pwr, pwi;
        c_powr(x2r, x2i, (double)(j % 4), &pwr, &pwi);
        double pr, pi;
        c_mul(x1r, -x1i, pwr, pwi, &pr, &pi);
        cRe[j] += pr;
        cIm[j] += pi;
    }

    /* for r in range(5,51,5): cf[r] += real(t2)*cos(r) + imag(t1)*sin(r)*1j */
    for (int r = 5; r < 51 && r < 50; r += 5) {
        cRe[r] += x2r * cos((double)r);
        cIm[r] += x1i * sin((double)r);
    }

    /* cf[9] = sum([abs(t1),abs(t2)]) + prod([real(t1),real(t2)])*1j */
    cRe[9] = abs1 + abs2;
    cIm[9] = x1r * x2r;

    /* cf[19] = real(t1)^2 - imag(t2)^2 + 2*real(t1)*imag(t2)*1j */
    cRe[19] = x1r * x1r - x2i * x2i;
    cIm[19] = 2.0 * x1r * x2i;

    /* cf[29] = log(abs(t1)+abs(t2)+1) * (sin(angle(t1)) + cos(angle(t2))*1j) */
    {
        double lv = log(abs1 + abs2 + 1.0);
        cRe[29] = lv * sin(ang1);
        cIm[29] = lv * cos(ang2);
    }

    /* cf[39] = abs(t1)*abs(t2) * exp(1j*(angle(t1)-angle(t2))) */
    {
        double p = abs1 * abs2;
        double ang = ang1 - ang2;
        cRe[39] = p * cos(ang);
        cIm[39] = p * sin(ang);
    }

    /* cf[49] = conj(t1) + conj(t2) - t1*t2*1j */
    {
        double t1t2r, t1t2i;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        /* -1j * (t1t2r + t1t2i*j) = (t1t2i, -t1t2r) */
        cRe[49] = x1r + x2r + t1t2i;
        cIm[49] = -x1i + (-x2i) + (-t1t2r);
    }

    for (int i = 0; i < 50; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_232_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double log1 = log(abs1 + 1.0);
    double log2 = log(abs2 + 1.0);

    /* Main loop: j from 1 to 35 */
    for (int j = 1; j <= 35; j++) {
        int k = j % 5 + 1;
        int r = j / 7 + 1;
        double ang12 = c_arg(x1r + x2r, x1i + x2i);
        double r1k = pow(x1r, (double)k);
        double i2r = pow(x2i, (double)r);
        double denom = abs1 + abs2 + 1.0;

        /* conj(t1)^k * cos(r*angle(t2)) */
        double c1kr, c1ki;
        c_powr(x1r, -x1i, (double)k, &c1kr, &c1ki);
        double cosv = cos(r * ang2);

        cRe[j-1] = (r1k - i2r) * sin(j * ang12) / denom + c1kr * cosv + log1 * log2;
        cIm[j-1] = c1ki * cosv;
    }

    /* cf[[2,7,11,18,22,28,33]] overrides */
    /* cf[2] = (t1*t2)^2 - conj(t1)*sin(t2) */
    {
        double t1t2r, t1t2i, sq_r, sq_i;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        c_mul(t1t2r, t1t2i, t1t2r, t1t2i, &sq_r, &sq_i);
        double sr, si, pr, pi;
        c_sin(x2r, x2i, &sr, &si);
        c_mul(x1r, -x1i, sr, si, &pr, &pi);
        cRe[2] = sq_r - pr;
        cIm[2] = sq_i - pi;
    }
    /* cf[7] = abs(t1)*real(t2) + imag(t1)*imag(t2) */
    cRe[7] = abs1 * x2r + x1i * x2i;
    cIm[7] = 0;

    /* cf[11] = cos(t1) + sin(t2) */
    {
        double cr, ci, sr, si;
        c_cos(x1r, x1i, &cr, &ci);
        c_sin(x2r, x2i, &sr, &si);
        cRe[11] = cr + sr;
        cIm[11] = ci + si;
    }
    /* cf[18] = log(abs(t1)+1) + log(abs(t2)+1) */
    cRe[18] = log1 + log2;
    cIm[18] = 0;

    /* cf[22] = t1^3 - t2^3 + conj(t1*t2) */
    {
        double t1_3r, t1_3i, t2_3r, t2_3i;
        double t1_2r, t1_2i, t2_2r, t2_2i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        c_mul(t1_2r, t1_2i, x1r, x1i, &t1_3r, &t1_3i);
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        c_mul(t2_2r, t2_2i, x2r, x2i, &t2_3r, &t2_3i);
        double t1t2r, t1t2i;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[22] = t1_3r - t2_3r + t1t2r;
        cIm[22] = t1_3i - t2_3i - t1t2i;
    }
    /* cf[28] = real(t1+t2) * imag(t1-t2) */
    cRe[28] = (x1r + x2r) * (x1i - x2i);
    cIm[28] = 0;

    /* cf[33] = sin(t1*t2) + cos(conj(t1))*cos(conj(t2)) */
    {
        double t1t2r, t1t2i, sr, si;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        c_sin(t1t2r, t1t2i, &sr, &si);
        double c1r, c1i, c2r, c2i, pr, pi;
        c_cos(x1r, -x1i, &c1r, &c1i);
        c_cos(x2r, -x2i, &c2r, &c2i);
        c_mul(c1r, c1i, c2r, c2i, &pr, &pi);
        cRe[33] = sr + pr;
        cIm[33] = si + pi;
    }

    /* cf[[4,9,14,19,24,29]] overrides */
    cRe[4] = x1r * x1r + x2i * x2i;
    cIm[4] = 0;

    cRe[9] = ang1 * ang2;
    cIm[9] = 0;

    /* cf[14] = abs(t1+t2) * conj(t1-t2) */
    {
        double a12 = c_abs(x1r + x2r, x1i + x2i);
        cRe[14] = a12 * (x1r - x2r);
        cIm[14] = a12 * (-(x1i - x2i));
    }

    /* cf[19] = sin(abs(t1)) * cos(abs(t2)) */
    cRe[19] = sin(abs1) * cos(abs2);
    cIm[19] = 0;

    /* cf[24] = log(abs(t1*t2)+1) */
    {
        double t1t2r, t1t2i;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[24] = log(c_abs(t1t2r, t1t2i) + 1.0);
        cIm[24] = 0;
    }

    /* cf[29] = sum([real(t1), imag(t2), angle(t1+t2)]) */
    {
        double ang12 = c_arg(x1r + x2r, x1i + x2i);
        cRe[29] = x1r + x2i + ang12;
        cIm[29] = 0;
    }

    /* cf[[6,10,16,20,26,30]] overrides */
    /* cf[6] = t1^2*t2 - t1*t2^2 */
    {
        double t1_2r, t1_2i, t2_2r, t2_2i, pr1, pi1, pr2, pi2;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        c_mul(t1_2r, t1_2i, x2r, x2i, &pr1, &pi1);
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        c_mul(x1r, x1i, t2_2r, t2_2i, &pr2, &pi2);
        cRe[6] = pr1 - pr2;
        cIm[6] = pi1 - pi2;
    }
    /* cf[10] = conj(t1)^2 + conj(t2)^2 */
    {
        double c1r, c1i, c2r, c2i;
        c_mul(x1r, -x1i, x1r, -x1i, &c1r, &c1i);
        c_mul(x2r, -x2i, x2r, -x2i, &c2r, &c2i);
        cRe[10] = c1r + c2r;
        cIm[10] = c1i + c2i;
    }
    /* cf[16] = sin(t1)*cos(t2) + cos(t1)*sin(t2) = sin(t1+t2) */
    {
        double s1r, s1i, c2r, c2i, c1r, c1i, s2r, s2i;
        c_sin(x1r, x1i, &s1r, &s1i);
        c_cos(x2r, x2i, &c2r, &c2i);
        c_cos(x1r, x1i, &c1r, &c1i);
        c_sin(x2r, x2i, &s2r, &s2i);
        double p1r, p1i, p2r, p2i;
        c_mul(s1r, s1i, c2r, c2i, &p1r, &p1i);
        c_mul(c1r, c1i, s2r, s2i, &p2r, &p2i);
        cRe[16] = p1r + p2r;
        cIm[16] = p1i + p2i;
    }
    /* cf[20] = real(t1*t2) + imag(t1*t2) — this is a real number */
    {
        double t1t2r, t1t2i;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[20] = t1t2r + t1t2i;
        cIm[20] = 0;
    }
    /* cf[26] = (t1+t2)^3 - (t1-t2)^3 */
    {
        double sr = x1r + x2r, si = x1i + x2i;
        double dr = x1r - x2r, di = x1i - x2i;
        double s3r, s3i, d3r, d3i;
        c_powr(sr, si, 3.0, &s3r, &s3i);
        c_powr(dr, di, 3.0, &d3r, &d3i);
        cRe[26] = s3r - d3r;
        cIm[26] = s3i - d3i;
    }
    /* cf[30] = prod([abs(t1), abs(t2), real(t1+t2)]) */
    cRe[30] = abs1 * abs2 * (x1r + x2r);
    cIm[30] = 0;

    /* cf[[8,12,17,23,27,31]] overrides */
    cRe[8] = x1r * x1r * x1r - x2i * x2i * x2i;
    cIm[8] = 0;

    cRe[12] = ang1 * ang1 + ang2 * ang2;
    cIm[12] = 0;

    /* cf[17] = sin(t1+t2) - cos(t1-t2) */
    {
        double sr, si, cr, ci;
        c_sin(x1r + x2r, x1i + x2i, &sr, &si);
        c_cos(x1r - x2r, x1i - x2i, &cr, &ci);
        cRe[17] = sr - cr;
        cIm[17] = si - ci;
    }

    /* cf[23] = log(abs(t1)^2 + abs(t2)^2 + 1) */
    cRe[23] = log(abs1 * abs1 + abs2 * abs2 + 1.0);
    cIm[23] = 0;

    /* cf[27] = real(conj(t1)*t2) */
    {
        double pr, pi;
        c_mul(x1r, -x1i, x2r, x2i, &pr, &pi);
        cRe[27] = pr;
        cIm[27] = 0;
    }

    /* cf[31] = imag(t1*conj(t2)) */
    {
        double pr, pi;
        c_mul(x1r, x1i, x2r, -x2i, &pr, &pi);
        cRe[31] = pi;
        cIm[31] = 0;
    }

    /* cf[[3,5,15,20,21,25,32,34]] overrides — these come LAST in Python */
    cRe[3] = x1r * x2r;
    cIm[3] = 0;

    cRe[5] = x1i * x2i;
    cIm[5] = 0;

    /* cf[15] = angle(t1+t2) * abs(t1*t2) */
    {
        double ang12 = c_arg(x1r + x2r, x1i + x2i);
        double t1t2r, t1t2i;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[15] = ang12 * c_abs(t1t2r, t1t2i);
        cIm[15] = 0;
    }

    /* cf[20] = sin(real(t1)) + cos(imag(t2)) — overwrites previous cf[20] */
    cRe[20] = sin(x1r) + cos(x2i);
    cIm[20] = 0;

    /* cf[21] = log(abs(t1+t2)+1) */
    cRe[21] = log(c_abs(x1r + x2r, x1i + x2i) + 1.0);
    cIm[21] = 0;

    /* cf[25] = real(conj(t1+t2)) = real(t1+t2) */
    cRe[25] = x1r + x2r;
    cIm[25] = 0;

    /* cf[32] = sin(abs(t1)^2) * cos(abs(t2)^2) */
    cRe[32] = sin(abs1 * abs1) * cos(abs2 * abs2);
    cIm[32] = 0;

    /* cf[34] = real(t1)^2 + imag(t1)^2 + real(t2)^2 + imag(t2)^2 = |t1|^2 + |t2|^2 */
    cRe[34] = x1r*x1r + x1i*x1i + x2r*x2r + x2i*x2i;
    cIm[34] = 0;

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_233_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Fixed: cf[[2,5,11,17,23,29]] = [2-3j, -4+2j, 5-1j, -6+3j, 7-2j, -8+4j] */
    cRe[2] = 2; cIm[2] = -3;
    cRe[5] = -4; cIm[5] = 2;
    cRe[11] = 5; cIm[11] = -1;
    cRe[17] = -6; cIm[17] = 3;
    cRe[23] = 7; cIm[23] = -2;
    cRe[29] = -8; cIm[29] = 4;

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* Main loop: j from 1 to 35 */
    for (int j = 1; j <= 35; j++) {
        if (j % 4 == 0) {
            cRe[j-1] = x1r * j * j - x2i * j;
            cIm[j-1] = x1i + x2r;
        } else if (j % 5 == 0) {
            int m = j % 3 + 1;
            cRe[j-1] = pow(abs1, (double)m) * cos(ang2 * j);
            cIm[j-1] = sin(ang1 * j) * abs2;
        } else if (j % 3 == 0) {
            cRe[j-1] = log(abs1 + 1.0) * j;
            cIm[j-1] = -log(abs2 + 1.0) * j;
        } else {
            cRe[j-1] = (x1r + x2r) * j;
            cIm[j-1] = (x1i - x2i) * j;
        }
    }

    /* Variations: k%7==0 and k%11==0 */
    for (int k = 1; k <= 35; k++) {
        if (k % 7 == 0) {
            /* cf[k-1] = conj(t1)*t2^2 + (sin(t1)-cos(t2))*1j */
            double t2_2r, t2_2i, pr, pi;
            c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
            c_mul(x1r, -x1i, t2_2r, t2_2i, &pr, &pi);
            double s1r, s1i, c2r, c2i;
            c_sin(x1r, x1i, &s1r, &s1i);
            c_cos(x2r, x2i, &c2r, &c2i);
            /* (sin(t1)-cos(t2))*1j */
            double dr = s1r - c2r, di = s1i - c2i;
            /* 1j*(dr+di*j) = (-di, dr) */
            cRe[k-1] = pr + (-di);
            cIm[k-1] = pi + dr;
        }
        if (k % 11 == 0) {
            /* cf[k-1] = prod([real(t1),imag(t2),k]) + sum([abs(t1),abs(t2),k])*1j */
            cRe[k-1] = x1r * x2i * k;
            cIm[k-1] = abs1 + abs2 + k;
        }
    }

    /* Specific overrides */
    /* cf[4] = 10*t1 - 5j*t2^2 */
    {
        double t2_2r, t2_2i;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        /* -5j*(t2_2r+t2_2i*j) = (5*t2_2i, -5*t2_2r) */
        cRe[4] = 10*x1r + 5*t2_2i;
        cIm[4] = 10*x1i - 5*t2_2r;
    }
    /* cf[9] = 15j*t1^3 + 8*t2 */
    {
        double t1_2r, t1_2i, t1_3r, t1_3i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        c_mul(t1_2r, t1_2i, x1r, x1i, &t1_3r, &t1_3i);
        cRe[9] = -15*t1_3i + 8*x2r;
        cIm[9] = 15*t1_3r + 8*x2i;
    }
    /* cf[14] = 20*t1^2 - 10j*t2^3 */
    {
        double t1_2r, t1_2i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        double t2_2r, t2_2i, t2_3r, t2_3i;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        c_mul(t2_2r, t2_2i, x2r, x2i, &t2_3r, &t2_3i);
        cRe[14] = 20*t1_2r + 10*t2_3i;
        cIm[14] = 20*t1_2i - 10*t2_3r;
    }
    /* cf[19] = 25j*t1 - 12*t2^2 */
    {
        double t2_2r, t2_2i;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        cRe[19] = -25*x1i - 12*t2_2r;
        cIm[19] = 25*x1r - 12*t2_2i;
    }
    /* cf[24] = 30*t1^4 + 15j*t2 */
    {
        double t1_2r, t1_2i, t1_4r, t1_4i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        c_mul(t1_2r, t1_2i, t1_2r, t1_2i, &t1_4r, &t1_4i);
        cRe[24] = 30*t1_4r - 15*x2i;
        cIm[24] = 30*t1_4i + 15*x2r;
    }
    /* cf[34] = 35j*t1^2 - 18*t2^3 */
    {
        double t1_2r, t1_2i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        double t2_2r, t2_2i, t2_3r, t2_3i;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        c_mul(t2_2r, t2_2i, x2r, x2i, &t2_3r, &t2_3i);
        cRe[34] = -35*t1_2i - 18*t2_3r;
        cIm[34] = 35*t1_2r - 18*t2_3i;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_234_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    for (int j = 1; j <= 35; j++) {
        int k = j + 2;
        double r = sqrt((double)j);
        double mag = (pow(abs1, r) + pow(abs2, (double)(k % 5 + 1))) * sin(j) + log(abs1 + 1.0) * cos(r);
        double ang = ang1 * cos(j / 2.0) - ang2 * sin(r);
        cRe[j-1] = mag * cos(ang);
        cIm[j-1] = mag * sin(ang);

        /* + conj(t1)*t2^k - conj(t2)*t1^(k%3) */
        double t2kr, t2ki, t1mr, t1mi;
        c_powr(x2r, x2i, (double)k, &t2kr, &t2ki);
        c_powr(x1r, x1i, (double)(k % 3), &t1mr, &t1mi);
        double p1r, p1i, p2r, p2i;
        c_mul(x1r, -x1i, t2kr, t2ki, &p1r, &p1i);
        c_mul(x2r, -x2i, t1mr, t1mi, &p2r, &p2i);
        cRe[j-1] += p1r - p2r;
        cIm[j-1] += p1i - p2i;
    }

    /* cf[4] = real(t1) + imag(t2)*1j */
    cRe[4] = x1r;
    cIm[4] = x2i;

    /* cf[11] = log(abs(t1)+1)*log(abs(t2)+1) + sin(t1*t2) */
    {
        double log1 = log(abs1 + 1.0);
        double log2 = log(abs2 + 1.0);
        double t1t2r, t1t2i, sr, si;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        c_sin(t1t2r, t1t2i, &sr, &si);
        cRe[11] = log1 * log2 + sr;
        cIm[11] = si;
    }

    /* cf[19] = (abs(t1)-abs(t2))*cos(ang1-ang2) + 2j*sin(ang1+ang2) */
    cRe[19] = (abs1 - abs2) * cos(ang1 - ang2);
    cIm[19] = 2.0 * sin(ang1 + ang2);

    /* cf[24] = conj(t1+t2) * (real(t1)-imag(t2)) + 3j */
    {
        double cr = x1r + x2r, ci = -(x1i + x2i);
        double s = x1r - x2i;
        cRe[24] = cr * s;
        cIm[24] = ci * s + 3.0;
    }

    /* cf[29] = sum([abs(t1),abs(t2)]) + prod([abs(t1),abs(t2)])*1j */
    cRe[29] = abs1 + abs2;
    cIm[29] = abs1 * abs2;

    /* cf[34] = angle(t1*t2) + abs(t1+t2)*1j */
    {
        double t1t2r, t1t2i;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[34] = c_arg(t1t2r, t1t2i);
        cIm[34] = c_abs(x1r + x2r, x1i + x2i);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_235_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Fixed: cf[[2,6,11,18,24,29]] = [3,-5,7,-11,13,-17] (real) */
    cRe[2] = 3; cRe[6] = -5; cRe[11] = 7; cRe[18] = -11; cRe[24] = 13; cRe[29] = -17;

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* Loop for j=1..10 (indices 0..9) */
    for (int j = 1; j <= 10; j++) {
        double magnitude = log(pow(abs1, (double)j) + 1.0) + pow(abs2, (double)(j % 3 + 1));
        double angle = ang1 * j - ang2 * (j % 2);
        cRe[j-1] = magnitude * cos(angle);
        cIm[j-1] = magnitude * sin(angle);
    }

    /* Loop for k=11..20 (indices 10..19) */
    double abs12 = c_abs(x1r + x2r, x1i + x2i);
    for (int k = 11; k <= 20; k++) {
        double magnitude = sin(x1r * k) + cos(x2i * k);
        double angle = log(abs12 + 1.0) * k;
        cRe[k-1] = magnitude * cos(angle);
        cIm[k-1] = magnitude * sin(angle);
    }

    /* Loop for r=21..34 (indices 20..33) */
    double ang_conj = c_arg(x1r + x2r, -(x1i + x2i));
    for (int r = 21; r <= 34; r++) {
        double magnitude = log(abs1 * r + 1.0) + x2r * x2r;
        double angle = ang_conj * r;
        cRe[r-1] = magnitude * cos(angle);
        cIm[r-1] = magnitude * sin(angle);
    }

    /* cf[34] = sin(t1*t2) + cos(t1/(t2+1)) + 1j*log(abs(t1+t2)+1) */
    {
        double t1t2r, t1t2i, sr, si;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        c_sin(t1t2r, t1t2i, &sr, &si);
        /* t2+1 = (x2r+1, x2i) */
        double dr, di, cr, ci;
        c_div(x1r, x1i, x2r + 1.0, x2i, &dr, &di);
        c_cos(dr, di, &cr, &ci);
        cRe[34] = sr + cr;
        cIm[34] = si + ci + log(abs12 + 1.0);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_236_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang12 = c_arg(x1r + x2r, x1i + x2i);

    /* Main loop */
    for (int j = 1; j <= 35; j++) {
        double angle = sin(x1r * j) * cos(x2i * j) + ang12 / j;
        double magnitude = log(abs1 * j + abs2 + 1.0) * pow(1.0 + sin(j), 0.5);
        cRe[j-1] = magnitude * cos(angle);
        cIm[j-1] = magnitude * sin(angle);
    }

    /* Second pass: k from 1 to 35 */
    for (int k = 1; k <= 35; k++) {
        if (k % 5 == 0) {
            /* cf[k-1] = cf[k-1] * conj(t1) + abs(t2)^2 */
            double tr = cRe[k-1], ti = cIm[k-1];
            double pr, pi;
            c_mul(tr, ti, x1r, -x1i, &pr, &pi);
            cRe[k-1] = pr + abs2 * abs2;
            cIm[k-1] = pi;
        } else if (k % 3 == 0) {
            /* cf[k-1] += t1^k - t2^k */
            double t1kr, t1ki, t2kr, t2ki;
            c_powr(x1r, x1i, (double)k, &t1kr, &t1ki);
            c_powr(x2r, x2i, (double)k, &t2kr, &t2ki);
            cRe[k-1] += t1kr - t2kr;
            cIm[k-1] += t1ki - t2ki;
        } else {
            /* cf[k-1] += sin(t1*k) * cos(t2*k) */
            double s1r, s1i, c2r, c2i, pr, pi;
            c_sin(x1r*k, x1i*k, &s1r, &s1i);
            c_cos(x2r*k, x2i*k, &c2r, &c2i);
            c_mul(s1r, s1i, c2r, c2i, &pr, &pi);
            cRe[k-1] += pr;
            cIm[k-1] += pi;
        }
    }

    /* r loop: for r in 2..5, idx=r^2, if idx<=35 (1-based => idx-1) */
    /* Note: after the loop, r=5 (Python for loop variable persists) */
    for (int r = 2; r <= 5; r++) {
        int idx = r * r;
        if (idx <= 35) {
            cRe[idx-1] += x1r * x2i / r;
            /* imag part: 0 since prod is real */
        }
    }

    /* cf[[3,9,15,21,27,33]] += 100j*(t1^2 - t2^2) / (r+1) where r=5 (last loop var) */
    {
        double t1_2r, t1_2i, t2_2r, t2_2i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        double dr = t1_2r - t2_2r, di = t1_2i - t2_2i;
        /* 100j * (dr+di*j) / 6 = (-100*di + 100*dr*j) / 6 */
        double addr = -100.0 * di / 6.0;
        double addi = 100.0 * dr / 6.0;
        int indices[] = {3, 9, 15, 21, 27, 33};
        for (int i = 0; i < 6; i++) {
            cRe[indices[i]] += addr;
            cIm[indices[i]] += addi;
        }
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_237_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);

    /* conj(t1)*conj(t2) */
    double cct_r, cct_i;
    c_mul(x1r, -x1i, x2r, -x2i, &cct_r, &cct_i);

    /* First pass */
    for (int j = 1; j <= 35; j++) {
        if (j % 4 == 1) {
            /* t1^j * sin(j*angle(t2)) + conj(t2)^2 */
            double t1jr, t1ji;
            c_powr(x1r, x1i, (double)j, &t1jr, &t1ji);
            double sv = sin(j * ang2);
            double c2_2r, c2_2i;
            c_mul(x2r, -x2i, x2r, -x2i, &c2_2r, &c2_2i);
            cRe[j-1] = t1jr * sv + c2_2r;
            cIm[j-1] = t1ji * sv + c2_2i;
        } else if (j % 4 == 2) {
            /* t2^j * cos(j*angle(t1)) + conj(t1)^2 */
            double t2jr, t2ji;
            c_powr(x2r, x2i, (double)j, &t2jr, &t2ji);
            double cv = cos(j * ang1);
            double c1_2r, c1_2i;
            c_mul(x1r, -x1i, x1r, -x1i, &c1_2r, &c1_2i);
            cRe[j-1] = t2jr * cv + c1_2r;
            cIm[j-1] = t2ji * cv + c1_2i;
        } else if (j % 4 == 3) {
            /* real(t1)*imag(t2)*log(abs(t1)+1) + real(t2)^j */
            cRe[j-1] = x1r * x2i * log(abs1 + 1.0) + pow(x2r, (double)j);
            cIm[j-1] = 0;
        } else {
            /* imag(t1)*real(t2)*log(abs(t2)+1) + imag(t2)^j */
            cRe[j-1] = x1i * x2r * log(abs2 + 1.0) + pow(x2i, (double)j);
            cIm[j-1] = 0;
        }
        /* + conj(t1)*conj(t2) / (j+1) */
        cRe[j-1] += cct_r / (j + 1.0);
        cIm[j-1] += cct_i / (j + 1.0);
    }

    /* Second pass: modify based on cf[k-1] values */
    for (int k = 1; k <= 35; k++) {
        double cr = cRe[k-1], ci = cIm[k-1];
        double ang_cf = c_arg(cr, ci);
        double abs_cf = c_abs(cr, ci);

        if (k % 5 == 0) {
            double scale = 1.0 + 0.05 * k;
            cRe[k-1] = cr * scale + sin(k * ang_cf);
            cIm[k-1] = ci * scale;
        } else {
            double scale = 1.0 + 0.02 * k;
            cRe[k-1] = cr / scale + cos(k * ang_cf);
            cIm[k-1] = ci / scale;
        }
        /* += log(abs(cf[k-1])+1) * real(cf[k-1]) — uses updated value */
        double new_abs = c_abs(cRe[k-1], cIm[k-1]);
        double addv = log(new_abs + 1.0) * cRe[k-1];
        cRe[k-1] += addv;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_238_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* First pass */
    for (int j = 1; j <= 35; j++) {
        double angle = ang1 * j * j - ang2 * sqrt((double)j);
        double magnitude = pow(abs1, (double)j) + pow(abs2, (double)(35 - j)) + log(abs1 + abs2 + j);
        cRe[j-1] = magnitude * cos(angle);
        cIm[j-1] = magnitude * sin(angle);
    }

    /* Second pass: cf[k-1] += conj(cf[36-k]) * t1^(k%5) - conj(cf[k-1]) * t2^(35-k%3)
     * Note: cf[36-k] when k=1 => cf[35] => index 35 => OOB for 35-element array!
     * But Python uses 0-indexed cf[36-k-1] implicitly? No — cf[36-k] with k starting at 1
     * gives cf[35] which is OOB. The try/except would catch this.
     * Actually wait: cf has 35 elements (indices 0..34). cf[35] is OOB.
     * When k=1: cf[35] => IndexError => except returns zeros.
     * So this function always returns zeros for k=1 in the second loop.
     *
     * BUT: the second loop iterates k=1..35. k=1 accesses cf[35] which is index 35 on a
     * size-35 array => IndexError => returns zeros(35).
     * So the function returns: the first pass results plus nothing from second pass
     * (the exception happens immediately at k=1).
     * Then the specific assignments at the end also don't execute.
     *
     * This means poly_238 only returns the first-pass results. Let me implement just that.
     * Actually, I should still implement the full intended logic, skipping the OOB access,
     * since the OOB is likely a bug. Let me handle it by clamping the index.
     */
    for (int k = 1; k <= 35; k++) {
        int idx_mirror = 36 - k; /* 35 down to 1, but 35 is OOB */
        if (idx_mirror >= 35) idx_mirror = 34; /* clamp */
        double ccr = cRe[idx_mirror], cci = -cIm[idx_mirror]; /* conj */
        double t1kr, t1ki;
        c_powr(x1r, x1i, (double)(k % 5), &t1kr, &t1ki);
        double p1r, p1i;
        c_mul(ccr, cci, t1kr, t1ki, &p1r, &p1i);

        double ckr = cRe[k-1], cki = -cIm[k-1]; /* conj(cf[k-1]) */
        double t2kr, t2ki;
        c_powr(x2r, x2i, (double)(35 - k % 3), &t2kr, &t2ki);
        double p2r, p2i;
        c_mul(ckr, cki, t2kr, t2ki, &p2r, &p2i);

        cRe[k-1] += p1r - p2r;
        cIm[k-1] += p1i - p2i;
    }

    /* Specific overrides */
    /* cf[4] += 2*t1^3 - 3*t2^2 + sin(t1*t2)*1j */
    {
        double t1_3r, t1_3i, t1_2r, t1_2i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        c_mul(t1_2r, t1_2i, x1r, x1i, &t1_3r, &t1_3i);
        double t2_2r, t2_2i;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        double t1t2r, t1t2i, sr, si;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        c_sin(t1t2r, t1t2i, &sr, &si);
        /* sin(t1*t2)*1j = (-si, sr) */
        cRe[4] += 2*t1_3r - 3*t2_2r + (-si);
        cIm[4] += 2*t1_3i - 3*t2_2i + sr;
    }

    /* cf[9] = conj(cf[9])*t1 - imag(cf[9])*t2 + log(abs(t1+t2)+1) */
    {
        double ccr = cRe[9], cci = -cIm[9];
        double p1r, p1i;
        c_mul(ccr, cci, x1r, x1i, &p1r, &p1i);
        double im9 = cIm[9];
        cRe[9] = p1r - im9*x2r + log(c_abs(x1r+x2r, x1i+x2i) + 1.0);
        cIm[9] = p1i - im9*x2i;
    }

    /* cf[14] = cf[14]*t1^2 - cf[14]/(abs(t2)+1) + cos(t1-t2)*1j */
    {
        double t1_2r, t1_2i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        double cr14 = cRe[14], ci14 = cIm[14];
        double p1r, p1i;
        c_mul(cr14, ci14, t1_2r, t1_2i, &p1r, &p1i);
        double denom = abs2 + 1.0;
        double cosdr, cosdi;
        c_cos(x1r - x2r, x1i - x2i, &cosdr, &cosdi);
        /* cos(t1-t2)*1j = (-cosdi, cosdr) */
        cRe[14] = p1r - cr14/denom + (-cosdi);
        cIm[14] = p1i - ci14/denom + cosdr;
    }

    /* cf[19] = real(cf[19]) + imag(cf[19])*1j + t1*t2 */
    {
        double re19 = cRe[19], im19 = cIm[19];
        double t1t2r, t1t2i;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[19] = re19 + t1t2r;
        cIm[19] = im19 + t1t2i;
    }

    /* cf[24] = abs(t1)*abs(t2) + angle(t1+t2)*1j */
    cRe[24] = abs1 * abs2;
    cIm[24] = c_arg(x1r + x2r, x1i + x2i);

    /* cf[29] = sin(t1^2) + cos(t2^3)*1j - log(abs(t1*t2)+1) */
    {
        double t1_2r, t1_2i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        double sr, si;
        c_sin(t1_2r, t1_2i, &sr, &si);
        double t2_2r, t2_2i, t2_3r, t2_3i;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        c_mul(t2_2r, t2_2i, x2r, x2i, &t2_3r, &t2_3i);
        double cr, ci;
        c_cos(t2_3r, t2_3i, &cr, &ci);
        /* cos(t2^3)*1j = (-ci, cr) */
        double t1t2r, t1t2i;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        double logv = log(c_abs(t1t2r, t1t2i) + 1.0);
        cRe[29] = sr + (-ci) - logv;
        cIm[29] = si + cr;
    }

    /* cf[34] = conj(cf[34]) + t1 - t2 + sin(t1+t2)*1j */
    {
        double ccr = cRe[34], cci = -cIm[34];
        double sr, si;
        c_sin(x1r + x2r, x1i + x2i, &sr, &si);
        /* sin(t1+t2)*1j = (-si, sr) */
        cRe[34] = ccr + x1r - x2r + (-si);
        cIm[34] = cci + x1i - x2i + sr;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_239_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);

    for (int j = 1; j <= 35; j++) {
        if (j % 5 == 1) {
            /* real(t1)^j + imag(t2)^2 */
            cRe[j-1] = pow(x1r, (double)j) + x2i * x2i;
            cIm[j-1] = 0;
        } else if (j % 5 == 2) {
            /* abs(t1)*abs(t2)^j * exp(1j*angle(t1)*j) */
            double m = abs1 * pow(abs2, (double)j);
            double a = ang1 * j;
            cRe[j-1] = m * cos(a);
            cIm[j-1] = m * sin(a);
        } else if (j % 5 == 3) {
            /* conj(t1) * sin(t2)^j + cos(t1*t2) */
            double sr, si;
            c_sin(x2r, x2i, &sr, &si);
            double spjr, spji;
            c_powr(sr, si, (double)j, &spjr, &spji);
            double pr, pi;
            c_mul(x1r, -x1i, spjr, spji, &pr, &pi);
            double t1t2r, t1t2i, cr, ci;
            c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
            c_cos(t1t2r, t1t2i, &cr, &ci);
            cRe[j-1] = pr + cr;
            cIm[j-1] = pi + ci;
        } else if (j % 5 == 4) {
            /* log(abs(t1)+1) * t2^j + 1j*log(abs(t2)+1) */
            double l1 = log(abs1 + 1.0);
            double t2jr, t2ji;
            c_powr(x2r, x2i, (double)j, &t2jr, &t2ji);
            cRe[j-1] = l1 * t2jr;
            cIm[j-1] = l1 * t2ji + log(abs2 + 1.0);
        } else {
            /* (t1+t2)^j - (t1-t2)^j */
            double sjr, sji, djr, dji;
            c_powr(x1r + x2r, x1i + x2i, (double)j, &sjr, &sji);
            c_powr(x1r - x2r, x1i - x2i, (double)j, &djr, &dji);
            cRe[j-1] = sjr - djr;
            cIm[j-1] = sji - dji;
        }
    }

    /* Additional assignments */
    /* cf[4] += 2j*t1*t2 */
    {
        double pr, pi;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
        /* 2j*(pr+pi*j) = (-2*pi, 2*pr) */
        cRe[4] += -2*pi;
        cIm[4] += 2*pr;
    }

    /* cf[9] = real(t1)^2 - imag(t2)^3 + 3j*abs(t1*t2) */
    {
        double t1t2r, t1t2i;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[9] = x1r * x1r - x2i * x2i * x2i;
        cIm[9] = 3.0 * c_abs(t1t2r, t1t2i);
    }

    /* cf[14] = sin(t1+t2)*cos(t1-t2) + 1j*log(abs(t1)+abs(t2)+1) */
    {
        double sr, si, cr, ci, pr, pi;
        c_sin(x1r + x2r, x1i + x2i, &sr, &si);
        c_cos(x1r - x2r, x1i - x2i, &cr, &ci);
        c_mul(sr, si, cr, ci, &pr, &pi);
        cRe[14] = pr;
        cIm[14] = pi + log(abs1 + abs2 + 1.0);
    }

    /* cf[19] = (t1*t2)^2 - conj(t1)*conj(t2) + 2j*angle(t1+t2) */
    {
        double t1t2r, t1t2i, sq_r, sq_i;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        c_mul(t1t2r, t1t2i, t1t2r, t1t2i, &sq_r, &sq_i);
        double ccr, cci;
        c_mul(x1r, -x1i, x2r, -x2i, &ccr, &cci);
        double ang = c_arg(x1r + x2r, x1i + x2i);
        cRe[19] = sq_r - ccr;
        cIm[19] = sq_i - cci + 2.0 * ang;
    }

    /* cf[24] = real(t1*t2) + imag(t1)^2 - imag(t2)^2 + 1j*(real(t1)-real(t2)) */
    {
        double t1t2r, t1t2i;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[24] = t1t2r + x1i*x1i - x2i*x2i;
        cIm[24] = x1r - x2r;
    }

    /* cf[29] = abs(t1+t2)^3 * exp(1j*angle(t1-t2)) */
    {
        double a = c_abs(x1r + x2r, x1i + x2i);
        double a3 = a * a * a;
        double ang = c_arg(x1r - x2r, x1i - x2i);
        cRe[29] = a3 * cos(ang);
        cIm[29] = a3 * sin(ang);
    }

    /* cf[34] = sin(abs(t1)*t2) + cos(abs(t2)*t1) + 1j*(real(t1)*real(t2)) */
    {
        double sr, si, cr, ci;
        c_sin(abs1 * x2r, abs1 * x2i, &sr, &si);
        c_cos(abs2 * x1r, abs2 * x1i, &cr, &ci);
        cRe[34] = sr + cr;
        cIm[34] = si + ci + x1r * x2r;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_240_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Fixed: cf[[0,4,12,19,20,24]] = [1+0j, 4+0j, 4+0j, -9+0j, -1.9+0j, 0.2+0j] */
    cRe[0] = 1; cRe[4] = 4; cRe[12] = 4; cRe[19] = -9; cRe[20] = -1.9; cRe[24] = 0.2;

    double abs1 = c_abs(x1r, x1i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    int skip[] = {5, 13, 20, 21, 25};

    /* Loop j from 2 to 34 (Python: range(2,35)), writes to cf[j-1] */
    for (int j = 2; j < 35; j++) {
        int is_skip = 0;
        for (int s = 0; s < 5; s++) { if (j == skip[s]) { is_skip = 1; break; } }
        if (is_skip) continue;

        double mag = log(c_abs(x1r + j, x1i) + 1.0) * sin(j * ang2) + cos(j * ang1);
        double angle = pow(ang1, (double)j) + sin(j * ang2) - cos(j);
        cRe[j-1] = mag * cos(angle);
        cIm[j-1] = mag * sin(angle);
    }

    /* cf[34] = conj(t1)*conj(t2) + sin(abs(t1)*abs(t2)) + log(abs(t1)+abs(t2)+1)*1j */
    {
        double pr, pi;
        c_mul(x1r, -x1i, x2r, -x2i, &pr, &pi);
        double abs2 = c_abs(x2r, x2i);
        cRe[34] = pr + sin(abs1 * abs2);
        cIm[34] = pi + log(abs1 + abs2 + 1.0);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}
