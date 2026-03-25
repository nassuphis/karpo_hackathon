/* poly_hand_300d.h — Hand-written C for poly_261 through poly_280 */

static void poly_261_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs, j in 1..35):
     *   mag = log(1 + abs(t1)**j + abs(t2)**(35-j)) + sin(j*angle(t1) + angle(t2))
     *   angle_val = cos(j*angle(t1)) - sin((35-j)*angle(t2))
     *   cf[j-1] = mag * (cos(angle_val) + 1j*sin(angle_val))
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    for (int j = 1; j <= 35; j++) {
        double mag = log(1.0 + pow(abs1, j) + pow(abs2, 35 - j))
                   + sin(j * ang1 + ang2);
        double angle_val = cos(j * ang1) - sin((35 - j) * ang2);
        cRe[j - 1] = mag * cos(angle_val);
        cIm[j - 1] = mag * sin(angle_val);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_262_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs):
     *   cf[[2,7,15,23,29]] = [2+1j, -3+2j, 4-1.5j, -2.2+0.8j, 0.6-0.4j]
     *   Loop j in 1..35: if j not in {3,8,16,24,30}:
     *     k=(j*3)%7+1; r=(j+4)%5+1
     *     mag = log(abs(t1)**k + abs(t2)**r + j)
     *     ang = angle(t1)*k - angle(t2)*r + sin(j)*pi/6
     *     cf[j-1] = mag*(cos(ang) + sin(ang)*1j)
     *   cf[11] = conj(t1)**2 * t2 - t1 * conj(t2)
     *   cf[18] = sin(t1*t2) + cos(t1+t2)*1j
     *   cf[26] = log(abs(t1+t2)+1) + angle(t1-t2)*1j
     *   cf[33] = (real(t1)+imag(t2))*cos(angle(t1)) + (imag(t1)-real(t2))*sin(angle(t2))*1j
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* Fixed coefficients: indices 2,7,15,23,29 (0-based) */
    cRe[2] = 2.0;   cIm[2] = 1.0;
    cRe[7] = -3.0;  cIm[7] = 2.0;
    cRe[15] = 4.0;  cIm[15] = -1.5;
    cRe[23] = -2.2; cIm[23] = 0.8;
    cRe[29] = 0.6;  cIm[29] = -0.4;

    /* The Python check is j not in {3,8,16,24,30} which maps to 0-based {2,7,15,23,29} */
    for (int j = 1; j <= 35; j++) {
        if (j == 3 || j == 8 || j == 16 || j == 24 || j == 30) continue;
        int k = (j * 3) % 7 + 1;
        int r = (j + 4) % 5 + 1;
        double mag = log(pow(abs1, k) + pow(abs2, r) + (double)j);
        double ang = ang1 * k - ang2 * r + sin((double)j) * M_PI / 6.0;
        cRe[j - 1] = mag * cos(ang);
        cIm[j - 1] = mag * sin(ang);
    }

    /* cf[11] = conj(t1)**2 * t2 - t1 * conj(t2) */
    {
        /* conj(t1)^2 */
        double c1r, c1i;
        c_mul(x1r, -x1i, x1r, -x1i, &c1r, &c1i);
        /* conj(t1)^2 * t2 */
        double a1r, a1i;
        c_mul(c1r, c1i, x2r, x2i, &a1r, &a1i);
        /* t1 * conj(t2) */
        double a2r, a2i;
        c_mul(x1r, x1i, x2r, -x2i, &a2r, &a2i);
        cRe[11] = a1r - a2r;
        cIm[11] = a1i - a2i;
    }

    /* cf[18] = sin(t1*t2) + cos(t1+t2)*1j */
    {
        double pr, pi;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
        double sr, si;
        c_sin(pr, pi, &sr, &si);
        double cr, ci;
        c_cos(x1r + x2r, x1i + x2i, &cr, &ci);
        /* result = sr+si*1j + (cr+ci*1j)*1j = sr+si*1j + cr*1j + ci*1j*1j = (sr - ci) + (si + cr)*1j */
        cRe[18] = sr - ci;
        cIm[18] = si + cr;
    }

    /* cf[26] = log(abs(t1+t2)+1) + angle(t1-t2)*1j */
    {
        double sum_abs = c_abs(x1r + x2r, x1i + x2i);
        double diff_ang = c_arg(x1r - x2r, x1i - x2i);
        cRe[26] = log(sum_abs + 1.0);
        cIm[26] = diff_ang;
    }

    /* cf[33] = (real(t1)+imag(t2))*cos(angle(t1)) + (imag(t1)-real(t2))*sin(angle(t2))*1j */
    {
        cRe[33] = (x1r + x2i) * cos(ang1);
        cIm[33] = (x1i - x2r) * sin(ang2);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_263_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs): Complex branching with j%4, j%3, j%5, else.
     * Plus overlay loops and fixed overrides.
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* Main loop j in 1..35 */
    for (int j = 1; j <= 35; j++) {
        if (j % 4 == 0) {
            /* cf = (real(t1)*j + imag(t2)*(35-j)) + (abs(t1)**0.5 * angle(t2))*1j */
            cRe[j - 1] = x1r * j + x2i * (35 - j);
            cIm[j - 1] = sqrt(abs1) * ang2;
        } else if (j % 3 == 0) {
            /* cf = sin(t1*j) + cos(t2+j) + log(abs(t1*t2)+1) -- complex ops */
            double sr, si;
            c_sin(x1r * j, x1i * j, &sr, &si);
            double cr, ci;
            c_cos(x2r + j, x2i, &cr, &ci);
            double pr, pi;
            c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
            double logv = log(c_abs(pr, pi) + 1.0);
            cRe[j - 1] = sr + cr + logv;
            cIm[j - 1] = si + ci;
        } else if (j % 5 == 0) {
            /* cf = (real(t2)**j - imag(t1)**(j%3)) + conj(t1)*imag(t2)*1j */
            double term1 = pow(x2r, j) - pow(x1i, j % 3);
            double conjt1_times_im2_r = x1r * x2i;
            double conjt1_times_im2_i = -x1i * x2i;
            cRe[j - 1] = term1 - conjt1_times_im2_i;
            cIm[j - 1] = conjt1_times_im2_r;
        } else {
            /* cf = real(t1)**2 + imag(t2)**2 + sin(t1+t2)*cos(t1-t2)*1j */
            double realpart = x1r * x1r + x2i * x2i;
            double sr, si;
            c_sin(x1r + x2r, x1i + x2i, &sr, &si);
            double cr, ci;
            c_cos(x1r - x2r, x1i - x2i, &cr, &ci);
            double pr, pi;
            c_mul(sr, si, cr, ci, &pr, &pi);
            /* result = realpart + pr*1j => but pr,pi is complex, so *1j means... */
            /* (pr + pi*1j)*1j = -pi + pr*1j */
            cRe[j - 1] = realpart - pi;
            cIm[j - 1] = pr;
        }
    }

    /* Second loop: k in 1..7, index = k*5 if <= 35 */
    for (int k = 1; k <= 7; k++) {
        int index = k * 5;
        if (index <= 35) {
            /* cf[index-1] += (t1**k - t2**k)*(k%2) + log(abs(t1+t2)+1)*1j */
            double t1kr, t1ki, t2kr, t2ki;
            c_powr(x1r, x1i, (double)k, &t1kr, &t1ki);
            c_powr(x2r, x2i, (double)k, &t2kr, &t2ki);
            double diff_r = t1kr - t2kr;
            double diff_i = t1ki - t2ki;
            double factor = (k % 2);
            double logv = log(c_abs(x1r + x2r, x1i + x2i) + 1.0);
            cRe[index - 1] += diff_r * factor;
            cIm[index - 1] += diff_i * factor + logv;
        }
    }

    /* Third loop: r in 1..5, idx = 7 + r*6 if <= 35 */
    /* cf[idx-1] += prod(cf[0:r]) * sin(t1*r) + cos(t2*r)*1j */
    for (int r = 1; r <= 5; r++) {
        int idx = 7 + r * 6;
        if (idx <= 35) {
            /* prod(cf[0:r]) = product of cf[0]..cf[r-1] */
            double prodr = cRe[0], prodi = cIm[0];
            for (int m = 1; m < r; m++) {
                double tr, ti;
                c_mul(prodr, prodi, cRe[m], cIm[m], &tr, &ti);
                prodr = tr; prodi = ti;
            }
            /* sin(t1*r) is complex sin */
            double sr, si;
            c_sin(x1r * r, x1i * r, &sr, &si);
            /* prod * sin(t1*r) */
            double psr, psi;
            c_mul(prodr, prodi, sr, si, &psr, &psi);
            /* cos(t2*r) is complex cos */
            double cr2, ci2;
            c_cos(x2r * r, x2i * r, &cr2, &ci2);
            /* cos(t2*r)*1j = (cr2+ci2*1j)*1j = -ci2 + cr2*1j */
            cRe[idx - 1] += psr - ci2;
            cIm[idx - 1] += psi + cr2;
        }
    }

    /* Fixed overrides */
    /* cf[9] = 100j*t2**3 + 100j*t2**2 - 100*t2 - 100 */
    {
        double t2_3r, t2_3i;
        c_powr(x2r, x2i, 3.0, &t2_3r, &t2_3i);
        double t2_2r, t2_2i;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        /* 100j*(a+bi) = -100b + 100a*j */
        cRe[9] = -100.0 * t2_3i + (-100.0 * t2_2i) - 100.0 * x2r - 100.0;
        cIm[9] = 100.0 * t2_3r + 100.0 * t2_2r - 100.0 * x2i;
    }

    /* cf[14] = 100j*t1**3 - 100j*t1**2 + 100*t2 - 100 */
    {
        double t1_3r, t1_3i;
        c_powr(x1r, x1i, 3.0, &t1_3r, &t1_3i);
        double t1_2r, t1_2i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        cRe[14] = -100.0 * t1_3i - (-100.0 * t1_2i) + 100.0 * x2r - 100.0;
        cIm[14] = 100.0 * t1_3r - 100.0 * t1_2r + 100.0 * x2i;
    }

    /* cf[24] = real(t1*t2) + imag(t1+t2)*1j */
    {
        double pr, pi;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
        cRe[24] = pr;
        cIm[24] = x1i + x2i;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_264_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs):
     *   Main loop k in 1..35:
     *     magnitude = (real(t1)+imag(t2))**k/(k+2) + (real(t2)-imag(t1))**(k%5+1)
     *     theta = angle(t1)**2/(k+1) + angle(t2)*log(abs(t2)+1)
     *     cf[k-1] = magnitude*(cos(theta)+sin(theta)*1j)
     *   Every 5th: cf[j-1] = conj(cf[j-1])*t1**2 - t2**3
     *   Every 3rd: cf[r-1] = real(t1)*cf[r-1] + imag(t2)*cf[r-1]**2
     *   cf[0] = 1+t1-t2; cf[34] = sin(t1*t2)+cos(t1/(abs(t2)+1))
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double abs2 = c_abs(x2r, x2i);
    double a = x1r + x2i;   /* real(t1) + imag(t2) */
    double b = x2r - x1i;   /* real(t2) - imag(t1) */

    for (int k = 1; k <= 35; k++) {
        double magnitude = pow(a, k) / (k + 2.0) + pow(b, k % 5 + 1);
        double theta = ang1 * ang1 / (k + 1.0) + ang2 * log(abs2 + 1.0);
        cRe[k - 1] = magnitude * cos(theta);
        cIm[k - 1] = magnitude * sin(theta);
    }

    /* Every 5th: j in 5,10,...,35 */
    {
        double t1_2r, t1_2i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        double t2_3r, t2_3i;
        c_powr(x2r, x2i, 3.0, &t2_3r, &t2_3i);
        for (int j = 5; j <= 35; j += 5) {
            /* conj(cf[j-1]) * t1^2 - t2^3 */
            double cr, ci;
            c_mul(cRe[j-1], -cIm[j-1], t1_2r, t1_2i, &cr, &ci);
            cRe[j - 1] = cr - t2_3r;
            cIm[j - 1] = ci - t2_3i;
        }
    }

    /* Every 3rd: r in 3,6,...,35 */
    for (int r = 3; r <= 35; r += 3) {
        /* cf[r-1] = real(t1)*cf[r-1] + imag(t2)*cf[r-1]**2 */
        double oldr = cRe[r - 1], oldi = cIm[r - 1];
        double sq_r, sq_i;
        c_mul(oldr, oldi, oldr, oldi, &sq_r, &sq_i);
        cRe[r - 1] = x1r * oldr + x2i * sq_r;
        cIm[r - 1] = x1r * oldi + x2i * sq_i;
    }

    /* cf[0] = 1 + t1 - t2 */
    cRe[0] = 1.0 + x1r - x2r;
    cIm[0] = x1i - x2i;

    /* cf[34] = sin(t1*t2) + cos(t1/(abs(t2)+1)) */
    {
        double pr, pi;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
        double sr, si;
        c_sin(pr, pi, &sr, &si);
        double denom = abs2 + 1.0;
        double dr = x1r / denom, di = x1i / denom;
        double cr, ci;
        c_cos(dr, di, &cr, &ci);
        cRe[34] = sr + cr;
        cIm[34] = si + ci;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_265_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs, j in 1..35):
     *   angle = angle(t1)*j + angle(t2)*(35-j)
     *   magnitude = abs(t1)**(j%5+1) + abs(t2)**(j%7+1) + log(abs(t1*t2)+1)
     *   phase = sin(j*real(t1)) + cos(j*imag(t2)) + angle(t1+t2)
     *   cf[j-1] = magnitude*(cos(phase) + 1j*sin(phase))
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double pr, pi;
    c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
    double log_abs_prod = log(c_abs(pr, pi) + 1.0);
    double ang_sum = c_arg(x1r + x2r, x1i + x2i);

    for (int j = 1; j <= 35; j++) {
        double magnitude = pow(abs1, j % 5 + 1) + pow(abs2, j % 7 + 1) + log_abs_prod;
        double phase = sin(j * x1r) + cos(j * x2i) + ang_sum;
        cRe[j - 1] = magnitude * cos(phase);
        cIm[j - 1] = magnitude * sin(phase);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_266_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs, k in 1..35):
     *   mag = sin(abs(t1)*(k**2)) + cos(abs(t2)/k) + log(abs(t1)+1)*log(abs(t2)+1)
     *   ang = angle(t1)*k + angle(t2)*(35-k) + sin(k)*cos(k)
     *   cf[k-1] = mag*(cos(ang) + 1j*sin(ang))
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double log12 = log(abs1 + 1.0) * log(abs2 + 1.0);

    for (int k = 1; k <= 35; k++) {
        double mag = sin(abs1 * (double)k * k) + cos(abs2 / k) + log12;
        double ang = ang1 * k + ang2 * (35 - k) + sin((double)k) * cos((double)k);
        cRe[k - 1] = mag * cos(ang);
        cIm[k - 1] = mag * sin(ang);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_267_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs):
     *   Main loop j in 1..35:
     *     angle = angle(t1)*sin(j) + angle(t2)*cos(j)
     *     magnitude = log(abs(t1)+1)*real(t2)**0.5 + imag(t1)**2/(j+1)
     *     cf[j-1] = magnitude*(cos(angle)+sin(angle)*1j) + conj(t1)*t2**j
     *   cf[4] = real(t1) + imag(t2)*1j
     *   cf[11] = abs(t1)**2 - abs(t2)**2 + (real(t1)*imag(t2))*1j
     *   cf[19] = sin(t1) + cos(t2)*1j
     *   for r in 25..35: cf[r-1] += prod([t1,t2])**r / (r+1)
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double log_abs1_p1 = log(abs1 + 1.0);
    double sqrt_re_t2 = sqrt(fabs(x2r));  /* real(t2)**0.5 -- needs fabs for safety */
    /* Note: np.real(t2)**0.5 with negative real part would give NaN in Python too,
       but we use fabs to get the magnitude for sqrt */

    /* conj(t1) = (x1r, -x1i) */
    /* t2^j iteratively */
    double t2jr = 1.0, t2ji = 0.0; /* t2^0 */

    for (int j = 1; j <= 35; j++) {
        /* t2^j */
        double nr, ni;
        c_mul(t2jr, t2ji, x2r, x2i, &nr, &ni);
        t2jr = nr; t2ji = ni;

        double angle_val = ang1 * sin((double)j) + ang2 * cos((double)j);
        double magnitude = log_abs1_p1 * sqrt_re_t2 + x1i * x1i / (j + 1.0);

        /* magnitude*(cos(angle)+sin(angle)*1j) */
        double mr = magnitude * cos(angle_val);
        double mi = magnitude * sin(angle_val);

        /* conj(t1)*t2^j */
        double cr, ci;
        c_mul(x1r, -x1i, t2jr, t2ji, &cr, &ci);

        cRe[j - 1] = mr + cr;
        cIm[j - 1] = mi + ci;
    }

    /* Overrides */
    cRe[4] = x1r;
    cIm[4] = x2i;

    cRe[11] = abs1 * abs1 - abs2 * abs2;
    cIm[11] = x1r * x2i;

    /* cf[19] = sin(t1) + cos(t2)*1j */
    {
        double sr, si;
        c_sin(x1r, x1i, &sr, &si);
        double cr, ci;
        c_cos(x2r, x2i, &cr, &ci);
        /* (sr+si*1j) + (cr+ci*1j)*1j = sr - ci + (si + cr)*1j */
        cRe[19] = sr - ci;
        cIm[19] = si + cr;
    }

    /* for r in 25..35: cf[r-1] += (t1*t2)**r / (r+1) */
    {
        double pr, pi;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
        for (int r = 25; r <= 35; r++) {
            double pwr, pwi;
            c_powr(pr, pi, (double)r, &pwr, &pwi);
            cRe[r - 1] += pwr / (r + 1.0);
            cIm[r - 1] += pwi / (r + 1.0);
        }
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_268_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs):
     *   Main loop j in 1..35 (all real result):
     *     k=j+2; r=(j%5)+1
     *     cf[j-1] = (real(t1)**k - imag(t2)**r)*sin(abs(t1)*j) + (angle(t2)+j)*cos(log(abs(t1)+1))
     *   Then 11 fixed complex overrides at indices 3,6,9,12,15,18,21,24,27,30,33
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double log_abs1 = log(abs1 + 1.0);
    double cos_log = cos(log_abs1);

    for (int j = 1; j <= 35; j++) {
        int k = j + 2;
        int r = (j % 5) + 1;
        double term1 = (pow(x1r, k) - pow(x2i, r)) * sin(abs1 * j);
        double term2 = (ang2 + j) * cos_log;
        cRe[j - 1] = term1 + term2;
        /* cIm stays 0 -- this is a real-valued expression */
    }

    /* cf[3] = conj(t1)*t2**2 - abs(t2)*cos(t1) */
    {
        double t2_2r, t2_2i;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        double ar, ai;
        c_mul(x1r, -x1i, t2_2r, t2_2i, &ar, &ai);
        double cr, ci;
        c_cos(x1r, x1i, &cr, &ci);
        double abs_t2 = c_abs(x2r, x2i);
        cRe[3] = ar - abs_t2 * cr;
        cIm[3] = ai - abs_t2 * ci;
    }

    /* cf[6] = sin(t1*t2) + cos(t1+t2)*t1 */
    {
        double pr, pi;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
        double sr, si;
        c_sin(pr, pi, &sr, &si);
        double cr, ci;
        c_cos(x1r + x2r, x1i + x2i, &cr, &ci);
        double mr, mi;
        c_mul(cr, ci, x1r, x1i, &mr, &mi);
        cRe[6] = sr + mr;
        cIm[6] = si + mi;
    }

    /* cf[9] = log(abs(t1)+1) + real(t2)**3 - imag(t1)*imag(t2) -- all real */
    cRe[9] = log_abs1 + x2r * x2r * x2r - x1i * x2i;
    cIm[9] = 0.0;

    /* cf[12] = (t1**2 + t2**2)*sin(t1) - cos(t2) */
    {
        double t1_2r, t1_2i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        double t2_2r, t2_2i;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        double sumr = t1_2r + t2_2r, sumi = t1_2i + t2_2i;
        double sr, si;
        c_sin(x1r, x1i, &sr, &si);
        double mr, mi;
        c_mul(sumr, sumi, sr, si, &mr, &mi);
        double cr, ci;
        c_cos(x2r, x2i, &cr, &ci);
        cRe[12] = mr - cr;
        cIm[12] = mi - ci;
    }

    /* cf[15] = real(t1)*real(t2) + imag(t1)*imag(t2) + angle(t1*t2) -- all real */
    {
        double pr, pi;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
        cRe[15] = x1r * x2r + x1i * x2i + c_arg(pr, pi);
        cIm[15] = 0.0;
    }

    /* cf[18] = abs(t1+t2)*sin(angle(t1)) - cos(abs(t2)) -- all real */
    {
        double abs_sum = c_abs(x1r + x2r, x1i + x2i);
        double ang1 = c_arg(x1r, x1i);
        double abs2 = c_abs(x2r, x2i);
        cRe[18] = abs_sum * sin(ang1) - cos(abs2);
        cIm[18] = 0.0;
    }

    /* cf[21] = conj(t1**3) + t2**3 - log(abs(t1*t2)+1) */
    {
        double t1_3r, t1_3i;
        c_powr(x1r, x1i, 3.0, &t1_3r, &t1_3i);
        double t2_3r, t2_3i;
        c_powr(x2r, x2i, 3.0, &t2_3r, &t2_3i);
        double pr, pi;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
        double logv = log(c_abs(pr, pi) + 1.0);
        cRe[21] = t1_3r + t2_3r - logv;  /* conj flips imag */
        cIm[21] = -t1_3i + t2_3i;
    }

    /* cf[24] = sin(t1**2) + cos(t2**2) - real(t1*t2) */
    {
        double t1_2r, t1_2i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        double t2_2r, t2_2i;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        double sr, si;
        c_sin(t1_2r, t1_2i, &sr, &si);
        double cr, ci;
        c_cos(t2_2r, t2_2i, &cr, &ci);
        double pr, pi;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
        cRe[24] = sr + cr - pr;
        cIm[24] = si + ci;
    }

    /* cf[27] = imag(t1**2) - real(t2**2) - real(t1*t2) -- all real */
    {
        double t1_2r, t1_2i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        double t2_2r, t2_2i;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        double pr, pi;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
        cRe[27] = t1_2i - t2_2r - pr;
        cIm[27] = 0.0;
    }

    /* cf[30] = abs(t1)**2 * cos(t2) - sin(abs(t2)) */
    {
        double abs1_sq = abs1 * abs1;
        double cr, ci;
        c_cos(x2r, x2i, &cr, &ci);
        double abs2 = c_abs(x2r, x2i);
        cRe[30] = abs1_sq * cr - sin(abs2);
        cIm[30] = abs1_sq * ci;
    }

    /* cf[33] = real(t1**3) - imag(t2**3) + log(abs(t1+t2)+1) -- all real */
    {
        double t1_3r, t1_3i;
        c_powr(x1r, x1i, 3.0, &t1_3r, &t1_3i);
        double t2_3r, t2_3i;
        c_powr(x2r, x2i, 3.0, &t2_3r, &t2_3i);
        double abs_sum = c_abs(x1r + x2r, x1i + x2i);
        cRe[33] = t1_3r - t2_3i + log(abs_sum + 1.0);
        cIm[33] = 0.0;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_269_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs):
     *   Main loop j in 1..35 with j%3 branches:
     *     j%3==1: (real(t1)**j + imag(t2)**(j%5+1))*sin(j*angle(t1)) + cos(j*angle(t2))
     *     j%3==2: (abs(t1)*abs(t2))**((j+1)/7) + log(abs(t1)+1)*log(abs(t2)+1)
     *     j%3==0: conj(t1)*t2**(j%4) - conj(t2)*t1**(j%3)
     *   Then 6 overrides at 3,9,15,21,27,33
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double log1 = log(abs1 + 1.0);
    double log2 = log(abs2 + 1.0);

    for (int j = 1; j <= 35; j++) {
        if (j % 3 == 1) {
            /* real result: (real(t1)**j + imag(t2)**(j%5+1))*sin(j*angle(t1)) + cos(j*angle(t2)) */
            double v = (pow(x1r, j) + pow(x2i, j % 5 + 1)) * sin(j * ang1) + cos(j * ang2);
            cRe[j - 1] = v;
            cIm[j - 1] = 0.0;
        } else if (j % 3 == 2) {
            /* real result: (abs(t1)*abs(t2))**((j+1)/7.0) + log(abs(t1)+1)*log(abs(t2)+1) */
            double v = pow(abs1 * abs2, (j + 1.0) / 7.0) + log1 * log2;
            cRe[j - 1] = v;
            cIm[j - 1] = 0.0;
        } else {
            /* j%3==0: conj(t1)*t2**(j%4) - conj(t2)*t1**(j%3) */
            /* j%3==0 so t1**(j%3) = t1**0 = 1 */
            double t2pr, t2pi;
            c_powr(x2r, x2i, (double)(j % 4), &t2pr, &t2pi);
            double ar, ai;
            c_mul(x1r, -x1i, t2pr, t2pi, &ar, &ai);
            /* conj(t2)*1 = conj(t2) */
            cRe[j - 1] = ar - x2r;
            cIm[j - 1] = ai - (-x2i);
        }
    }

    /* Overrides */
    /* cf[3] = sum([real(t1), imag(t2)]) + prod([abs(t1), abs(t2)]) -- real */
    cRe[3] = (x1r + x2i) + (abs1 * abs2);
    cIm[3] = 0.0;

    /* cf[9] = sin(t1*t2) + cos(t1-t2) + log(abs(t1+t2)+1) */
    {
        double pr, pi;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
        double sr, si;
        c_sin(pr, pi, &sr, &si);
        double cr, ci;
        c_cos(x1r - x2r, x1i - x2i, &cr, &ci);
        double logv = log(c_abs(x1r + x2r, x1i + x2i) + 1.0);
        cRe[9] = sr + cr + logv;
        cIm[9] = si + ci;
    }

    /* cf[15] = (real(t1)**2 - imag(t1)**2) + (real(t2)**2 - imag(t2)**2) -- real */
    cRe[15] = (x1r * x1r - x1i * x1i) + (x2r * x2r - x2i * x2i);
    cIm[15] = 0.0;

    /* cf[21] = abs(t1*t2)*angle(t1+t2) + conj(t1-t2) */
    {
        double pr, pi;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
        double abs_prod = c_abs(pr, pi);
        double ang_sum = c_arg(x1r + x2r, x1i + x2i);
        cRe[21] = abs_prod * ang_sum + (x1r - x2r);
        cIm[21] = -(x1i - x2i);
    }

    /* cf[27] = sin(real(t1)*imag(t2)) + cos(imag(t1)*real(t2)) -- real */
    cRe[27] = sin(x1r * x2i) + cos(x1i * x2r);
    cIm[27] = 0.0;

    /* cf[33] = log(abs(t1)**3 + abs(t2)**3 + 1) + real(t1*conj(t2)) -- real */
    {
        double pr, pi;
        c_mul(x1r, x1i, x2r, -x2i, &pr, &pi);
        cRe[33] = log(abs1 * abs1 * abs1 + abs2 * abs2 * abs2 + 1.0) + pr;
        cIm[33] = 0.0;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_270_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs):
     *   Main loop j in 1..35:
     *     term1 = log(abs(t1)+j)*cos(j*pi/7) * exp(1j*(angle(t1)*j+sin(j*angle(t2))))
     *     term2 = log(abs(t2)+(35-j))*sin(j*pi/5) * exp(1j*(angle(t2)*(35-j)+cos(j*angle(t1))))
     *     cf[j-1] = term1 + term2 + conj(t1)**(j%5) * conj(t2)**(j%3)
     *   Multiply loop: k in 2,5,8,...,34: cf[k-1] *= (sin(abs(t1*k)) + cos(abs(t2+k)))
     *   Add loop: r in 1,6,11,...,31: cf[r-1] += 1j*log(abs(t1+r)+1)*sin(angle(t2)*r)
     *   cf[0] = real(t1)+real(t2); cf[34] = imag(t1)-imag(t2)+conj(t1*t2)
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    for (int j = 1; j <= 35; j++) {
        double phase1 = ang1 * j + sin(j * ang2);
        double mag1 = log(abs1 + j) * cos(j * M_PI / 7.0);

        double phase2 = ang2 * (35 - j) + cos(j * ang1);
        double mag2 = log(abs2 + (35 - j)) * sin(j * M_PI / 5.0);

        double t1r = mag1 * cos(phase1);
        double t1i = mag1 * sin(phase1);
        double t2r_v = mag2 * cos(phase2);
        double t2i_v = mag2 * sin(phase2);

        /* conj(t1)**(j%5) * conj(t2)**(j%3) */
        double c1r, c1i;
        c_powr(x1r, -x1i, (double)(j % 5), &c1r, &c1i);
        double c2r, c2i;
        c_powr(x2r, -x2i, (double)(j % 3), &c2r, &c2i);
        double cr, ci;
        c_mul(c1r, c1i, c2r, c2i, &cr, &ci);

        cRe[j - 1] = t1r + t2r_v + cr;
        cIm[j - 1] = t1i + t2i_v + ci;
    }

    /* Multiply loop: k in 2,5,8,...,34 */
    for (int k = 2; k <= 34; k += 3) {
        /* sin(abs(t1*k)) -- abs(t1*k) = abs(t1)*k since k is real */
        double factor = sin(abs1 * k) + cos(abs2 + k);
        cRe[k - 1] *= factor;
        cIm[k - 1] *= factor;
    }

    /* Add loop: r in 1,6,11,...,31 (step 5, up to 35) */
    for (int r = 1; r <= 35; r += 5) {
        double logv = log(c_abs(x1r + r, x1i) + 1.0);
        double sinv = sin(ang2 * r);
        /* += 1j * logv * sinv */
        cIm[r - 1] += logv * sinv;
    }

    /* cf[0] = real(t1) + real(t2) */
    cRe[0] = x1r + x2r;
    cIm[0] = 0.0;

    /* cf[34] = imag(t1) - imag(t2) + conj(t1*t2) */
    {
        double pr, pi;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
        cRe[34] = x1i - x2i + pr;
        cIm[34] = -pi;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_271_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs, j in 1..35):
     *   mag = log(abs(t1)+j)*sin(j*angle(t2)) + cos(abs(t2)*j)
     *   angle_val = real(t1)*j + imag(t2)/(j+1)
     *   cf[j-1] = mag*(cos(angle_val) + sin(angle_val)*1j)
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang2 = c_arg(x2r, x2i);

    for (int j = 1; j <= 35; j++) {
        double mag = log(abs1 + j) * sin(j * ang2) + cos(abs2 * j);
        double angle_val = x1r * j + x2i / (j + 1.0);
        cRe[j - 1] = mag * cos(angle_val);
        cIm[j - 1] = mag * sin(angle_val);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_272_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs, k in 1..35):
     *   j = (k+3)%6+1; r = k//4+1
     *   mag_part = log(abs(t1)+k)*sin(j*angle(t2)) + cos(r*angle(t1))
     *   angle_part = angle(t1)**j - angle(t2)**r + sin(k)*cos(k)
     *   cf[k-1] = mag_part*(cos(angle_part)+1j*sin(angle_part)) + conj(t1)**j * conj(t2)**r
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    for (int k = 1; k <= 35; k++) {
        int j = (k + 3) % 6 + 1;
        int r = k / 4 + 1;
        double mag_part = log(abs1 + k) * sin(j * ang2) + cos(r * ang1);
        double angle_part = pow(ang1, j) - pow(ang2, r) + sin((double)k) * cos((double)k);

        double mr = mag_part * cos(angle_part);
        double mi = mag_part * sin(angle_part);

        /* conj(t1)**j * conj(t2)**r */
        double c1r, c1i;
        c_powr(x1r, -x1i, (double)j, &c1r, &c1i);
        double c2r, c2i;
        c_powr(x2r, -x2i, (double)r, &c2r, &c2i);
        double cr, ci;
        c_mul(c1r, c1i, c2r, c2i, &cr, &ci);

        cRe[k - 1] = mr + cr;
        cIm[k - 1] = mi + ci;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_273_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs):
     *   cf[[0,5,11,17,23,29]] = [2, -3+1j, 4, -5j, 6+2j, -7]
     *   Loop j in 2..34: if cf[j]==0:
     *     cf[j] = (real(t1)**j - imag(t2)**j) + (angle(t1)*j + abs(t2))*1j
     *   Loop k in 3..33: cf[k] += sin(t1*k)*cos(t2/k) + log(abs(t1)+1)*sin(angle(t2))*1j
     *   Then 7 overrides at 9,14,19,24,27,31,34
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double log_abs1 = log(c_abs(x1r, x1i) + 1.0);

    /* Fixed coefficients */
    cRe[0] = 2.0;  cIm[0] = 0.0;
    cRe[5] = -3.0; cIm[5] = 1.0;
    cRe[11] = 4.0; cIm[11] = 0.0;
    cRe[17] = 0.0; cIm[17] = -5.0;
    cRe[23] = 6.0; cIm[23] = 2.0;
    cRe[29] = -7.0; cIm[29] = 0.0;

    /* Loop j in 2..34 (0-based): if cf[j]==0 */
    for (int j = 2; j <= 34; j++) {
        if (cRe[j] == 0.0 && cIm[j] == 0.0) {
            cRe[j] = pow(x1r, j) - pow(x2i, j);
            cIm[j] = ang1 * j + abs2;
        }
    }

    /* Loop k in 3..33: cf[k] += sin(t1*k)*cos(t2/k) + log(abs(t1)+1)*sin(angle(t2))*1j */
    for (int k = 3; k <= 33; k++) {
        double sr, si;
        c_sin(x1r * k, x1i * k, &sr, &si);
        double cr, ci;
        c_cos(x2r / k, x2i / k, &cr, &ci);
        double mr, mi;
        c_mul(sr, si, cr, ci, &mr, &mi);
        double imag_add = log_abs1 * sin(ang2);
        cRe[k] += mr;
        cIm[k] += mi + imag_add;
    }

    /* cf[9] = conj(t1)*t2**2 + abs(t2)*1j */
    {
        double t2_2r, t2_2i;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        double mr, mi;
        c_mul(x1r, -x1i, t2_2r, t2_2i, &mr, &mi);
        cRe[9] = mr;
        cIm[9] = mi + abs2;
    }

    /* cf[14] = real(t1**3) + imag(t2**3)*1j */
    {
        double t1_3r, t1_3i;
        c_powr(x1r, x1i, 3.0, &t1_3r, &t1_3i);
        double t2_3r, t2_3i;
        c_powr(x2r, x2i, 3.0, &t2_3r, &t2_3i);
        cRe[14] = t1_3r;
        cIm[14] = t2_3i;
    }

    /* cf[19] = real(t1)*real(t2) + imag(t1)*imag(t2)*1j */
    cRe[19] = x1r * x2r;
    cIm[19] = x1i * x2i;

    /* cf[24] = abs(t1)+abs(t2) + angle(t1+t2)*1j */
    {
        double abs1 = c_abs(x1r, x1i);
        cRe[24] = abs1 + abs2;
        cIm[24] = c_arg(x1r + x2r, x1i + x2i);
    }

    /* cf[27] = sin(abs(t1)) + cos(abs(t2))*1j */
    {
        double abs1 = c_abs(x1r, x1i);
        cRe[27] = sin(abs1);
        cIm[27] = cos(abs2);
    }

    /* cf[31] = log(abs(t1)+1) + log(abs(t2)+1)*1j */
    {
        double log2 = log(abs2 + 1.0);
        cRe[31] = log_abs1;
        cIm[31] = log2;
    }

    /* cf[34] = conj(t1+t2) */
    cRe[34] = x1r + x2r;
    cIm[34] = -(x1i + x2i);

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_274_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs):
     *   Main loop j in 1..35:
     *     mag = log(abs(t1)+1)**j * sin(j*angle(t1)) + abs(t2)**(j%4+1)
     *     ang = angle(t1)*j + angle(t2)*(j%5)
     *     cf[j-1] = mag*(cos(ang)+1j*sin(ang))
     *   Every 7th (k=1,8,15,22,29): cf[k-1] += 100j*t1**k - 50*t2**(k%3)
     *   Loop r in 2..34: cf[r-1] = cf[r-1]*(1+0.1*r) + conj(t1)*sin(r*angle(t2))
     *   cf[0] = 1+real(t1)-real(t2); cf[34] = 2-imag(t1)+imag(t2)
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double log_abs1_p1 = log(abs1 + 1.0);

    for (int j = 1; j <= 35; j++) {
        double mag = pow(log_abs1_p1, j) * sin(j * ang1) + pow(abs2, j % 4 + 1);
        double ang = ang1 * j + ang2 * (j % 5);
        cRe[j - 1] = mag * cos(ang);
        cIm[j - 1] = mag * sin(ang);
    }

    /* Every 7th: k in 1,8,15,22,29 */
    for (int k = 1; k <= 35; k += 7) {
        double t1kr, t1ki;
        c_powr(x1r, x1i, (double)k, &t1kr, &t1ki);
        double t2kr, t2ki;
        c_powr(x2r, x2i, (double)(k % 3), &t2kr, &t2ki);
        /* 100j*(t1kr+t1ki*j) = -100*t1ki + 100*t1kr*j */
        cRe[k - 1] += -100.0 * t1ki - 50.0 * t2kr;
        cIm[k - 1] += 100.0 * t1kr - 50.0 * t2ki;
    }

    /* Loop r in 2..34: cf[r-1] = cf[r-1]*(1+0.1*r) + conj(t1)*sin(r*angle(t2)) */
    for (int r = 2; r <= 34; r++) {
        double scale = 1.0 + 0.1 * r;
        double sinv = sin(r * ang2);
        /* conj(t1)*sinv is (x1r*sinv, -x1i*sinv) */
        cRe[r - 1] = cRe[r - 1] * scale + x1r * sinv;
        cIm[r - 1] = cIm[r - 1] * scale + (-x1i) * sinv;
    }

    /* Overrides */
    cRe[0] = 1.0 + x1r - x2r;
    cIm[0] = 0.0;
    cRe[34] = 2.0 - x1i + x2i;
    cIm[34] = 0.0;

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_275_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs, j in 1..35):
     *   k = (j*4)%8+1; r = j//5+2
     *   angle = angle(t1)*j + angle(t2)*k + sin(j)*cos(k)
     *   mag = abs(t1)**j + abs(t2)**k + log(abs(t1*t2)+1)*r
     *   cf[j-1] = mag*(cos(angle)+1j*sin(angle)) + conj(t1)**r * conj(t2)**k
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double pr, pi_v;
    c_mul(x1r, x1i, x2r, x2i, &pr, &pi_v);
    double log_prod = log(c_abs(pr, pi_v) + 1.0);

    for (int j = 1; j <= 35; j++) {
        int k = (j * 4) % 8 + 1;
        int r = j / 5 + 2;
        double angle_val = ang1 * j + ang2 * k + sin((double)j) * cos((double)k);
        double mag = pow(abs1, j) + pow(abs2, k) + log_prod * r;

        double mr = mag * cos(angle_val);
        double mi = mag * sin(angle_val);

        /* conj(t1)**r * conj(t2)**k */
        double c1r, c1i;
        c_powr(x1r, -x1i, (double)r, &c1r, &c1i);
        double c2r, c2i;
        c_powr(x2r, -x2i, (double)k, &c2r, &c2i);
        double cr, ci;
        c_mul(c1r, c1i, c2r, c2i, &cr, &ci);

        cRe[j - 1] = mr + cr;
        cIm[j - 1] = mi + ci;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_276_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs): j%5 branches + 7 overrides
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    for (int j = 1; j <= 35; j++) {
        if (j % 5 == 1) {
            /* real(t1)**j + imag(t2)**(j%3+1) * conj(t1) */
            double pow_val = pow(x2i, j % 3 + 1);
            /* pow_val * conj(t1) */
            double cr = pow_val * x1r;
            double ci = pow_val * (-x1i);
            cRe[j - 1] = pow(x1r, j) + cr;
            cIm[j - 1] = ci;
        } else if (j % 5 == 2) {
            /* abs(t1+t2)**j * sin(angle(t1)*j) + 1j*cos(angle(t2)*j) */
            double abs_sum = c_abs(x1r + x2r, x1i + x2i);
            double v = pow(abs_sum, j) * sin(ang1 * j);
            cRe[j - 1] = v;
            cIm[j - 1] = cos(ang2 * j);
        } else if (j % 5 == 3) {
            /* log(abs(t1)+1)*real(t2)**j - 1j*log(abs(t2)+1)*imag(t1)**j */
            double log1 = log(abs1 + 1.0);
            double log2 = log(abs2 + 1.0);
            cRe[j - 1] = log1 * pow(x2r, j);
            cIm[j - 1] = -log2 * pow(x1i, j);
        } else if (j % 5 == 4) {
            /* (t1**2 + t2**3)*sin(j) + 1j*(t1*t2)**2 * cos(j) */
            double t1_2r, t1_2i;
            c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
            double t2_3r, t2_3i;
            c_powr(x2r, x2i, 3.0, &t2_3r, &t2_3i);
            double sr = sin((double)j);
            double ar = (t1_2r + t2_3r) * sr;
            double ai = (t1_2i + t2_3i) * sr;

            /* (t1*t2)**2 */
            double pr, pi_v;
            c_mul(x1r, x1i, x2r, x2i, &pr, &pi_v);
            double p2r, p2i;
            c_mul(pr, pi_v, pr, pi_v, &p2r, &p2i);
            double cv = cos((double)j);
            /* 1j*(p2r+p2i*1j)*cv = (-p2i*cv + p2r*cv*1j) */
            cRe[j - 1] = ar + (-p2i * cv);
            cIm[j - 1] = ai + (p2r * cv);
        } else {
            /* j%5==0: prod([real(t1),imag(t2),j]) + 1j*sum([abs(t1),abs(t2),j]) */
            cRe[j - 1] = x1r * x2i * j;
            cIm[j - 1] = abs1 + abs2 + j;
        }
    }

    /* Overrides */
    /* cf[4] = 100*t1**4 - 50j*t2**2 + 25 */
    {
        double t1_4r, t1_4i;
        c_powr(x1r, x1i, 4.0, &t1_4r, &t1_4i);
        double t2_2r, t2_2i;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        /* -50j*(t2_2r+t2_2i*j) = 50*t2_2i - 50*t2_2r*j */
        cRe[4] = 100.0 * t1_4r + 50.0 * t2_2i + 25.0;
        cIm[4] = 100.0 * t1_4i - 50.0 * t2_2r;
    }

    /* cf[9] = 200j*sin(t1) + 150*cos(t2) */
    {
        double sr, si;
        c_sin(x1r, x1i, &sr, &si);
        double cr, ci;
        c_cos(x2r, x2i, &cr, &ci);
        /* 200j*(sr+si*j) = -200*si + 200*sr*j */
        cRe[9] = -200.0 * si + 150.0 * cr;
        cIm[9] = 200.0 * sr + 150.0 * ci;
    }

    /* cf[14] = 300*log(abs(t1)+1) + 100j*log(abs(t2)+1) -- real+pure_imag */
    cRe[14] = 300.0 * log(abs1 + 1.0);
    cIm[14] = 100.0 * log(abs2 + 1.0);

    /* cf[19] = conj(t1)*t2**3 - t1**2*conj(t2) */
    {
        double t2_3r, t2_3i;
        c_powr(x2r, x2i, 3.0, &t2_3r, &t2_3i);
        double ar, ai;
        c_mul(x1r, -x1i, t2_3r, t2_3i, &ar, &ai);
        double t1_2r, t1_2i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        double br, bi;
        c_mul(t1_2r, t1_2i, x2r, -x2i, &br, &bi);
        cRe[19] = ar - br;
        cIm[19] = ai - bi;
    }

    /* cf[24] = abs(t1)**3 + abs(t2)**2 * 1j */
    cRe[24] = abs1 * abs1 * abs1;
    cIm[24] = abs2 * abs2;

    /* cf[29] = sin(t1*t2) + cos(t1+t2)*1j */
    {
        double pr, pi_v;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi_v);
        double sr, si;
        c_sin(pr, pi_v, &sr, &si);
        double cr, ci;
        c_cos(x1r + x2r, x1i + x2i, &cr, &ci);
        /* (sr+si*j) + (cr+ci*j)*j = sr-ci + (si+cr)*j */
        cRe[29] = sr - ci;
        cIm[29] = si + cr;
    }

    /* cf[34] = log(abs(t1*t2)+1) + 1j*angle(t1+t2) */
    {
        double pr, pi_v;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi_v);
        cRe[34] = log(c_abs(pr, pi_v) + 1.0);
        cIm[34] = c_arg(x1r + x2r, x1i + x2i);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_277_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs): 7 ranges of 5, each with different formula.
     * Plus 4 overrides at 11,17,26,33.
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double ang_sum = c_arg(x1r + x2r, x1i + x2i);

    for (int j = 0; j < 35; j++) {
        if (j < 5) {
            int k = j + 1;
            /* (real(t1)**k + imag(t2)**k) * exp(1j*angle(t1+t2)) */
            double base = pow(x1r, k) + pow(x2i, k);
            cRe[j] = base * cos(ang_sum);
            cIm[j] = base * sin(ang_sum);
        } else if (j < 10) {
            int k = j - 4;
            /* (abs(t1)**k * abs(t2)**5) / (k+1) + 1j*sin(k*angle(t1)) */
            double v = pow(abs1, k) * pow(abs2, 5) / (k + 1.0);
            cRe[j] = v;
            cIm[j] = sin(k * ang1);
        } else if (j < 15) {
            int k = j - 9;
            /* real(t1*t2) + 1j*imag(t1**k + t2**k) */
            double pr, pi_v;
            c_mul(x1r, x1i, x2r, x2i, &pr, &pi_v);
            double t1kr, t1ki;
            c_powr(x1r, x1i, (double)k, &t1kr, &t1ki);
            double t2kr, t2ki;
            c_powr(x2r, x2i, (double)k, &t2kr, &t2ki);
            cRe[j] = pr;
            cIm[j] = t1ki + t2ki;
        } else if (j < 20) {
            int k = j - 14;
            /* log(abs(t1)+1)*cos(k*angle(t2)) + 1j*log(abs(t2)+1)*sin(k*angle(t1)) */
            cRe[j] = log(abs1 + 1.0) * cos(k * ang2);
            cIm[j] = log(abs2 + 1.0) * sin(k * ang1);
        } else if (j < 25) {
            int k = j - 19;
            /* (t1 + conj(t2))**k + (conj(t1) - t2)**k */
            double ar, ai;
            c_powr(x1r + x2r, x1i - x2i, (double)k, &ar, &ai);
            double br, bi;
            c_powr(x1r - x2r, -x1i - x2i, (double)k, &br, &bi);
            cRe[j] = ar + br;
            cIm[j] = ai + bi;
        } else if (j < 30) {
            int k = j - 24;
            /* real(t1)**k * imag(t2)**k + 1j*(abs(t1+t2)**k) */
            double abs_sum = c_abs(x1r + x2r, x1i + x2i);
            cRe[j] = pow(x1r, k) * pow(x2i, k);
            cIm[j] = pow(abs_sum, k);
        } else {
            int k = j - 29;
            /* (real(t1)*imag(t2))**k + conj(t1*t2)**k */
            double base = x1r * x2i;
            double pr, pi_v;
            c_mul(x1r, x1i, x2r, x2i, &pr, &pi_v);
            double cr, ci;
            c_powr(pr, -pi_v, (double)k, &cr, &ci);
            cRe[j] = pow(base, k) + cr;
            cIm[j] = ci;
        }
    }

    /* Overrides */
    /* cf[11] = 100*t1**3 - 50j*t2**2 + 25*t1*t2 */
    {
        double t1_3r, t1_3i;
        c_powr(x1r, x1i, 3.0, &t1_3r, &t1_3i);
        double t2_2r, t2_2i;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        double pr, pi_v;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi_v);
        /* -50j*(t2_2r+t2_2i*j) = 50*t2_2i - 50*t2_2r*j */
        cRe[11] = 100.0 * t1_3r + 50.0 * t2_2i + 25.0 * pr;
        cIm[11] = 100.0 * t1_3i - 50.0 * t2_2r + 25.0 * pi_v;
    }

    /* cf[17] = 200j*sin(t1) + 150*cos(t2) */
    {
        double sr, si;
        c_sin(x1r, x1i, &sr, &si);
        double cr, ci;
        c_cos(x2r, x2i, &cr, &ci);
        cRe[17] = -200.0 * si + 150.0 * cr;
        cIm[17] = 200.0 * sr + 150.0 * ci;
    }

    /* cf[26] = 300*log(abs(t1)+1) + 100j*abs(t2)**2 */
    cRe[26] = 300.0 * log(abs1 + 1.0);
    cIm[26] = 100.0 * abs2 * abs2;

    /* cf[33] = 400*real(t1*t2) - 200j*imag(t1+t2) */
    {
        double pr, pi_v;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi_v);
        cRe[33] = 400.0 * pr;
        cIm[33] = -200.0 * (x1i + x2i);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_278_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs):
     *   Fixed: cf[0]=2, cf[3]=-3, cf[9]=5+2j, cf[15]=-4+1j, cf[21]=3.5, cf[29]=-2.2
     *   Loop j in 1..34 (0-based): if j not in fixed_indices:
     *     angle = angle(t1)**0.5*(j+1) + angle(t2)**0.3*(35-(j+1))
     *     magnitude = abs(t1)**((j+1)/3) + abs(t2)**(35-(j+1))/2
     *     cf[j] = magnitude*(cos(angle)+sin(angle)*1j)
     *   4 overrides at 6,13,20,27
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* Fixed coefficients */
    cRe[0] = 2.0;
    cRe[3] = -3.0;
    cRe[9] = 5.0;  cIm[9] = 2.0;
    cRe[15] = -4.0; cIm[15] = 1.0;
    cRe[21] = 3.5;
    cRe[29] = -2.2;

    int fixed[] = {0, 3, 9, 15, 21, 29};
    for (int j = 1; j < 35; j++) {
        int is_fixed = 0;
        for (int f = 0; f < 6; f++) {
            if (j == fixed[f]) { is_fixed = 1; break; }
        }
        if (is_fixed) continue;

        double jj = j + 1.0; /* j+1 in 1-based */
        /* angle(t1)**0.5 — need fabs for fractional power of potentially negative angle */
        double ang1_sqrt = (ang1 >= 0) ? sqrt(ang1) : -sqrt(-ang1);
        /* Actually Python: np.angle(t1)**0.5 with negative angle gives NaN,
           but let's handle it with pow to match Python behavior */
        double angle_val = pow(fabs(ang1), 0.5) * (ang1 >= 0 ? 1 : -1) * jj
                         + pow(fabs(ang2), 0.3) * (ang2 >= 0 ? 1 : -1) * (35.0 - jj);
        /* Actually simpler: Python pow of negative base with fractional exp gives NaN.
           Let's just use pow which will return NaN and isfinite check will zero it.
           But to match better, use the sign-preserving approach. */
        double magnitude = pow(abs1, jj / 3.0) + pow(abs2, 35.0 - jj) / 2.0;
        cRe[j] = magnitude * cos(angle_val);
        cIm[j] = magnitude * sin(angle_val);
    }

    /* cf[6] = (100*t1**2 - 50*conj(t2)) + (25*sin(t1)+75*cos(t2))*1j */
    {
        double t1_2r, t1_2i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        /* Real part: 100*t1^2 - 50*conj(t2) */
        double rr = 100.0 * t1_2r - 50.0 * x2r;
        double ri = 100.0 * t1_2i - 50.0 * (-x2i);
        /* Imaginary coefficient: 25*sin(t1) + 75*cos(t2) */
        double sr, si;
        c_sin(x1r, x1i, &sr, &si);
        double cr, ci;
        c_cos(x2r, x2i, &cr, &ci);
        double ir = 25.0 * sr + 75.0 * cr;
        double ii = 25.0 * si + 75.0 * ci;
        /* *1j: (ir+ii*j)*j = -ii + ir*j */
        cRe[6] = rr + (-ii);
        cIm[6] = ri + ir;
    }

    /* cf[13] = (200*t2**3 + 100*real(t1)) + (50*imag(t2) - 30*log(abs(t1)+1))*1j */
    {
        double t2_3r, t2_3i;
        c_powr(x2r, x2i, 3.0, &t2_3r, &t2_3i);
        double rr = 200.0 * t2_3r + 100.0 * x1r;
        double ri = 200.0 * t2_3i;
        double iv = 50.0 * x2i - 30.0 * log(abs1 + 1.0);
        cRe[13] = rr;
        cIm[13] = ri + iv;
    }

    /* cf[20] = (abs(t1)+abs(t2)) + (real(t1)*real(t2))*1j */
    cRe[20] = abs1 + abs2;
    cIm[20] = x1r * x2r;

    /* cf[27] = (log(abs(t1)+1)*real(t1)) - (real(t2)**2) + (imag(t1)*imag(t2))*1j */
    cRe[27] = log(abs1 + 1.0) * x1r - x2r * x2r;
    cIm[27] = x1i * x2i;

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_279_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs):
     *   Loop j in 0..34:
     *     mag = log(abs(t1)**(j+1) + abs(t2)**(35-(j+1)) + 1) * ((j+1)%7+1) * (1+sin(j+1))
     *     ang = angle(t1)*(j+1)**0.5 - angle(t2)*(35-(j+1))**0.3
     *     cf[j] = mag*(cos(ang)+1j*sin(ang))
     *   Loop k in 0..34:
     *     if (k+1)%5==0: cf[k] = cf[k]*conj(t1) + real(t2)**2
     *     elif (k+1)%3==0: cf[k] += imag(t1)*imag(t2)
     *     else: cf[k] = cf[k]*real(t1+t2) - imag(t1-t2)
     *   indices [2,7,14,22,28,34]: cf[idx] += 100*t1**2 - 50*t2
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* First loop */
    for (int j = 0; j < 35; j++) {
        int jp1 = j + 1;
        double mag = log(pow(abs1, jp1) + pow(abs2, 35 - jp1) + 1.0)
                   * (jp1 % 7 + 1) * (1.0 + sin((double)jp1));
        double ang = ang1 * sqrt((double)jp1) - ang2 * pow(35.0 - jp1, 0.3);
        cRe[j] = mag * cos(ang);
        cIm[j] = mag * sin(ang);
    }

    /* Second loop: modifications */
    double re_sum = x1r + x2r;  /* real(t1+t2) */
    double im_diff = x1i - x2i; /* imag(t1-t2) */
    double re_t2_sq = x2r * x2r; /* real(t2)**2 */
    double im_prod = x1i * x2i;  /* imag(t1)*imag(t2) */

    for (int k = 0; k < 35; k++) {
        int kp1 = k + 1;
        if (kp1 % 5 == 0) {
            /* cf[k] = cf[k]*conj(t1) + real(t2)**2 */
            double mr, mi;
            c_mul(cRe[k], cIm[k], x1r, -x1i, &mr, &mi);
            cRe[k] = mr + re_t2_sq;
            cIm[k] = mi;
        } else if (kp1 % 3 == 0) {
            /* cf[k] += imag(t1)*imag(t2) -- real addition */
            cRe[k] += im_prod;
        } else {
            /* cf[k] = cf[k]*real(t1+t2) - imag(t1-t2) */
            cRe[k] = cRe[k] * re_sum - im_diff;
            cIm[k] = cIm[k] * re_sum;
        }
    }

    /* cf[indices] += 100*t1**2 - 50*t2 */
    {
        double t1_2r, t1_2i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        double addr = 100.0 * t1_2r - 50.0 * x2r;
        double addi = 100.0 * t1_2i - 50.0 * x2i;
        int indices[] = {2, 7, 14, 22, 28, 34};
        for (int n = 0; n < 6; n++) {
            cRe[indices[n]] += addr;
            cIm[indices[n]] += addi;
        }
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_280_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python (35 coeffs): (j+1)%6 branches + fixed overrides
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);

    for (int j = 0; j < 35; j++) {
        int jp1 = j + 1;
        int mod = jp1 % 6;

        if (mod == 1) {
            /* (t1**(j+1) + conj(t2)**(j+1)) * log(abs(t1)+1) */
            double t1pr, t1pi;
            c_powr(x1r, x1i, (double)jp1, &t1pr, &t1pi);
            double ct2r, ct2i;
            c_powr(x2r, -x2i, (double)jp1, &ct2r, &ct2i);
            double logv = log(abs1 + 1.0);
            cRe[j] = (t1pr + ct2r) * logv;
            cIm[j] = (t1pi + ct2i) * logv;
        } else if (mod == 2) {
            /* (sin(t1*(j+1)) + cos(t2*(j+1))) * ((j+1)**2 + real(t1)) */
            double sr, si;
            c_sin(x1r * jp1, x1i * jp1, &sr, &si);
            double cr, ci;
            c_cos(x2r * jp1, x2i * jp1, &cr, &ci);
            double factor = (double)jp1 * jp1 + x1r;
            cRe[j] = (sr + cr) * factor;
            cIm[j] = (si + ci) * factor;
        } else if (mod == 3) {
            /* (real(t1)*imag(t2))**(j+1) + conj(t1*t2) */
            double base = x1r * x2i;
            double base_pow = pow(base, jp1);
            double pr, pi_v;
            c_mul(x1r, x1i, x2r, x2i, &pr, &pi_v);
            cRe[j] = base_pow + pr;
            cIm[j] = -pi_v;
        } else if (mod == 4) {
            /* log(abs(t1+t2)+1) * (j+1)**1.5 * angle(t1+t2) -- real */
            double logv = log(c_abs(x1r + x2r, x1i + x2i) + 1.0);
            double ang_sum = c_arg(x1r + x2r, x1i + x2i);
            cRe[j] = logv * pow((double)jp1, 1.5) * ang_sum;
            cIm[j] = 0.0;
        } else if (mod == 5) {
            /* (real(t1)**2 - imag(t2)**2)*(j+1) + 1j*(imag(t1)+real(t2)) */
            cRe[j] = (x1r * x1r - x2i * x2i) * jp1;
            cIm[j] = x1i + x2r;
        } else {
            /* mod==0: (abs(t1)+abs(t2))*(j+1)**3 * sin(angle(t1*t2)) */
            double pr, pi_v;
            c_mul(x1r, x1i, x2r, x2i, &pr, &pi_v);
            double ang_prod = c_arg(pr, pi_v);
            cRe[j] = (abs1 + abs2) * (double)jp1 * jp1 * jp1 * sin(ang_prod);
            cIm[j] = 0.0;
        }
    }

    /* Fixed overrides at indices [2,7,14,21,28,33] */
    cRe[2] = 2.0;   cIm[2] = 3.0;
    cRe[7] = -1.0;  cIm[7] = 4.0;
    cRe[14] = 0.5;  cIm[14] = -2.0;
    cRe[21] = 3.0;  cIm[21] = 0.0;
    cRe[28] = 0.0;  cIm[28] = -2.5;
    cRe[33] = 1.0;  cIm[33] = 1.0;

    /* cf[9] = 100j*t1**3 - 50*t2**2 + 25*conj(t1) */
    {
        double t1_3r, t1_3i;
        c_powr(x1r, x1i, 3.0, &t1_3r, &t1_3i);
        double t2_2r, t2_2i;
        c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
        /* 100j*(t1_3r+t1_3i*j) = -100*t1_3i + 100*t1_3r*j */
        cRe[9] = -100.0 * t1_3i - 50.0 * t2_2r + 25.0 * x1r;
        cIm[9] = 100.0 * t1_3r - 50.0 * t2_2i + 25.0 * (-x1i);
    }

    /* cf[19] = 75*t2**3 + 50j*conj(t2) - 25*t1 */
    {
        double t2_3r, t2_3i;
        c_powr(x2r, x2i, 3.0, &t2_3r, &t2_3i);
        /* 50j*conj(t2) = 50j*(x2r - x2i*j) = 50*x2i + 50*x2r*j */
        cRe[19] = 75.0 * t2_3r + 50.0 * x2i - 25.0 * x1r;
        cIm[19] = 75.0 * t2_3i + 50.0 * x2r - 25.0 * x1i;
    }

    /* cf[24] = 60j*sin(t1)*cos(t2) + 40*log(abs(t1*t2)+1) */
    {
        double sr, si;
        c_sin(x1r, x1i, &sr, &si);
        double cr, ci;
        c_cos(x2r, x2i, &cr, &ci);
        double mr, mi;
        c_mul(sr, si, cr, ci, &mr, &mi);
        /* 60j*(mr+mi*j) = -60*mi + 60*mr*j */
        double pr, pi_v;
        c_mul(x1r, x1i, x2r, x2i, &pr, &pi_v);
        double logv = 40.0 * log(c_abs(pr, pi_v) + 1.0);
        cRe[24] = -60.0 * mi + logv;
        cIm[24] = 60.0 * mr;
    }

    /* cf[34] = 150*real(t1+t2) - 100j*imag(t1-t2) */
    {
        double re_sum = x1r + x2r;
        double im_diff = x1i - x2i;
        cRe[34] = 150.0 * re_sum;
        cIm[34] = -100.0 * im_diff;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}
