/* poly_hand_300e.h — Hand-written C for poly_281 through poly_300 */

static void poly_281_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Python: sparse assignment at indices [0,4,9,14,19,24,29,34],
     * then loop over specific indices with branching on j%5.
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs_t1 = c_abs(x1r, x1i);
    double abs_t2 = c_abs(x2r, x2i);
    double ang_t1 = c_arg(x1r, x1i);
    double ang_t2 = c_arg(x2r, x2i);

    /* cf[0] = real(t1) + imag(t2)*1j */
    cRe[0] = x1r;  cIm[0] = x2i;

    /* cf[4] = abs(t1)**2 - abs(t2)**2 * 1j */
    cRe[4] = abs_t1 * abs_t1;  cIm[4] = -(abs_t2 * abs_t2);

    /* cf[9] = sin(angle(t1)) + cos(angle(t2))*1j */
    cRe[9] = sin(ang_t1);  cIm[9] = cos(ang_t2);

    /* cf[14] = log(abs(t1)+1) - log(abs(t2)+1)*1j */
    cRe[14] = log(abs_t1 + 1.0);  cIm[14] = -log(abs_t2 + 1.0);

    /* cf[19] = conj(t1) + conj(t2)*1j  =>  (x1r - x1i*j) + (x2r - x2i*j)*1j
       = x1r + x2r*1j  + (-x1i + (-x2i)*1j) ... wait, let's be careful:
       conj(t1) = x1r - x1i*1j, conj(t2) = x2r - x2i*1j
       conj(t1) + conj(t2)*1j = (x1r - x1i*1j) + (x2r - x2i*1j)*1j
       = x1r - x1i*1j + x2r*1j - x2i*1j*1j = x1r + x2i + (x2r - x1i)*1j */
    cRe[19] = x1r + x2i;  cIm[19] = x2r - x1i;

    /* cf[24] = real(t1)**3 - imag(t2)**3 * 1j */
    cRe[24] = x1r * x1r * x1r;  cIm[24] = -(x2i * x2i * x2i);

    /* cf[29] = abs(t1)**4 + abs(t2)**4 * 1j */
    { double a4 = abs_t1*abs_t1*abs_t1*abs_t1; double b4 = abs_t2*abs_t2*abs_t2*abs_t2;
      cRe[29] = a4;  cIm[29] = b4; }

    /* cf[34] = sin(angle(t1)*2) - cos(angle(t2)*2)*1j */
    cRe[34] = sin(ang_t1 * 2.0);  cIm[34] = -cos(ang_t2 * 2.0);

    /* Loop over specific indices */
    int loop_indices[] = {2,3,4,6,7,8,9,11,12,13,16,17,18,19,21,22,23,24,26,27,28,29,31,32,33,34};
    int nloop = 26;
    for (int li = 0; li < nloop; li++) {
        int j = loop_indices[li];
        int k = j * 3;
        int r = j % 5;
        if (r == 0) {
            /* cf[j] = (real(t1)+imag(t2))*sin(k) + (real(t2)-imag(t1))*cos(k)*1j */
            cRe[j] = (x1r + x2i) * sin((double)k);
            cIm[j] = (x2r - x1i) * cos((double)k);
        } else if (r == 1) {
            /* cf[j] = abs(t1+t2)**k * exp(1j*angle(t1-t2)) */
            double sumr = x1r + x2r, sumi = x1i + x2i;
            double abs_sum = c_abs(sumr, sumi);
            double diffr = x1r - x2r, diffi = x1i - x2i;
            double ang_diff = c_arg(diffr, diffi);
            double mag = pow(abs_sum, (double)k);
            cRe[j] = mag * cos(ang_diff);
            cIm[j] = mag * sin(ang_diff);
        } else if (r == 2) {
            /* cf[j] = log(abs(t1)**k + 1) + log(abs(t2)**k + 1)*1j */
            cRe[j] = log(pow(abs_t1, (double)k) + 1.0);
            cIm[j] = log(pow(abs_t2, (double)k) + 1.0);
        } else if (r == 3) {
            /* cf[j] = conj(t1)**k - conj(t2)**k * 1j */
            /* conj(t1) = (x1r, -x1i), conj(t2) = (x2r, -x2i) */
            double cr1r, cr1i, cr2r, cr2i;
            c_powr(x1r, -x1i, (double)k, &cr1r, &cr1i);
            c_powr(x2r, -x2i, (double)k, &cr2r, &cr2i);
            /* result = (cr1r + cr1i*1j) - (cr2r + cr2i*1j)*1j
               = cr1r + cr2i + (cr1i - cr2r)*1j */
            cRe[j] = cr1r + cr2i;
            cIm[j] = cr1i - cr2r;
        } else {
            /* r == 4: cf[j] = sum(real(t1), imag(t2)) * prod(abs(t1), abs(t2)) + 1j*sum(imag(t1), real(t2))
               np.sum with two scalar args = addition, np.prod with two scalar args = multiplication */
            cRe[j] = (x1r + x2i) * (abs_t1 * abs_t2);
            cIm[j] = x1i + x2r;
        }
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_282_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Fixed coefficients */
    cRe[0] = 2.5;  cRe[6] = -4.2;  cRe[13] = 3.8;  cRe[20] = -16.5;  cRe[27] = 5.3;  cRe[34] = 0.6;

    double abs_t1 = c_abs(x1r, x1i);
    double abs_t2 = c_abs(x2r, x2i);
    double ang_t1 = c_arg(x1r, x1i);
    double ang_t2 = c_arg(x2r, x2i);

    /* conj(t2) = (x2r, -x2i) */
    double ct2r = x2r, ct2i = -x2i;

    for (int j = 2; j < 35; j++) {
        if (j % 4 == 0) {
            int k = j / 2;
            /* cf[j] = (150j*t1**k + 75*conj(t2)) * sin(k*angle(t1)) - 50*log(abs(t2)+1) */
            double t1kr, t1ki;
            c_powr(x1r, x1i, (double)k, &t1kr, &t1ki);
            /* 150j * t1**k = (0+150j)*(t1kr+t1ki*j) = -150*t1ki + 150*t1kr*j */
            double ar = -150.0 * t1ki + 75.0 * ct2r;
            double ai =  150.0 * t1kr + 75.0 * ct2i;
            double s = sin((double)k * ang_t1);
            double logval = 50.0 * log(abs_t2 + 1.0);
            cRe[j] = ar * s - logval;
            cIm[j] = ai * s;
        } else if (j % 3 == 0) {
            int k = j % 5;
            /* cf[j] = (200*real(t1*t2**k) + 100j*imag(t1-t2)) * cos(k*angle(t2)) */
            double t2kr, t2ki;
            c_powr(x2r, x2i, (double)k, &t2kr, &t2ki);
            double mulr, muli;
            c_mul(x1r, x1i, t2kr, t2ki, &mulr, &muli);
            double im_diff = x1i - x2i;
            /* 200*mulr + 100j*im_diff = (200*mulr, 100*im_diff) */
            double cosval = cos((double)k * ang_t2);
            cRe[j] = 200.0 * mulr * cosval;
            cIm[j] = 100.0 * im_diff * cosval;
        } else {
            int r = j % 7;
            /* cf[j] = conj(t1)**r * t2**j + abs(t1**j)*abs(t2**r) */
            double ct1r_pow, ct1i_pow;
            c_powr(x1r, -x1i, (double)r, &ct1r_pow, &ct1i_pow);
            double t2jr, t2ji;
            c_powr(x2r, x2i, (double)j, &t2jr, &t2ji);
            double pr, pi;
            c_mul(ct1r_pow, ct1i_pow, t2jr, t2ji, &pr, &pi);
            /* abs(t1**j) = abs(t1)**j, abs(t2**r) = abs(t2)**r */
            double absprod = pow(abs_t1, (double)j) * pow(abs_t2, (double)r);
            cRe[j] = pr + absprod;
            cIm[j] = pi;
        }
    }

    /* Overwrite specific indices */
    /* cf[9] = 180j*t1**3 - 120*t2**2 + 90*sin(t1)*cos(t2) */
    {
        double t1_3r, t1_3i;
        c_powr(x1r, x1i, 3.0, &t1_3r, &t1_3i);
        double t2_2r, t2_2i;
        c_powr(x2r, x2i, 2.0, &t2_2r, &t2_2i);
        double sinr, sini, cosr, cosi;
        c_sin(x1r, x1i, &sinr, &sini);
        c_cos(x2r, x2i, &cosr, &cosi);
        double scr, sci;
        c_mul(sinr, sini, cosr, cosi, &scr, &sci);
        /* 180j*t1**3 = (-180*t1_3i, 180*t1_3r) */
        cRe[9] = -180.0*t1_3i - 120.0*t2_2r + 90.0*scr;
        cIm[9] =  180.0*t1_3r - 120.0*t2_2i + 90.0*sci;
    }

    /* cf[19] = 220j*t2**4 + 130*real(t1**3) - 100*imag(t2) */
    {
        double t2_4r, t2_4i;
        c_powr(x2r, x2i, 4.0, &t2_4r, &t2_4i);
        double t1_3r, t1_3i;
        c_powr(x1r, x1i, 3.0, &t1_3r, &t1_3i);
        /* 220j*t2**4 = (-220*t2_4i, 220*t2_4r) */
        cRe[19] = -220.0*t2_4i + 130.0*t1_3r - 100.0*x2i;
        cIm[19] =  220.0*t2_4r;
    }

    /* cf[29] = 260j*t1**2*t2 + 160*log(abs(t1*t2)+1) - 110*conj(t1) */
    {
        double t1_2r, t1_2i;
        c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
        double prodr, prodi;
        c_mul(t1_2r, t1_2i, x2r, x2i, &prodr, &prodi);
        /* 260j * prod = (-260*prodi, 260*prodr) */
        double mulr2, muli2;
        c_mul(x1r, x1i, x2r, x2i, &mulr2, &muli2);
        double abs_prod = c_abs(mulr2, muli2);
        double logval = 160.0 * log(abs_prod + 1.0);
        cRe[29] = -260.0*prodi + logval - 110.0*x1r;
        cIm[29] =  260.0*prodr + 110.0*x1i;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_283_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs_t1 = c_abs(x1r, x1i);
    double abs_t2 = c_abs(x2r, x2i);
    double ang_t1 = c_arg(x1r, x1i);
    double ang_t2 = c_arg(x2r, x2i);

    for (int j = 1; j <= 35; j++) {
        double angle_part = ang_t1 * sin((double)j) + ang_t2 * cos((double)j);
        int m5 = (j % 5) + 1;
        int m7 = (j % 7) + 1;
        double magnitude_part = pow(abs_t1, (double)m5) + pow(abs_t2, (double)m7);
        cRe[j-1] = magnitude_part * cos(angle_part);
        cIm[j-1] = magnitude_part * sin(angle_part);

        if (j % 4 == 0) {
            /* += conj(t1)*t2**2 - log(abs(t1)+1) */
            double t2_2r, t2_2i;
            c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
            double pr, pi;
            c_mul(x1r, -x1i, t2_2r, t2_2i, &pr, &pi);
            double logval = log(abs_t1 + 1.0);
            cRe[j-1] += pr - logval;
            cIm[j-1] += pi;
        }
        if (j % 6 == 0) {
            /* *= sin(t1*j) + cos(t2/(j+1)) */
            double tjr = x1r * (double)j, tji = x1i * (double)j;
            double sinr, sini;
            c_sin(tjr, tji, &sinr, &sini);
            double divr = x2r / (double)(j + 1), divi = x2i / (double)(j + 1);
            double cosr, cosi;
            c_cos(divr, divi, &cosr, &cosi);
            /* complex multiply cf[j-1] * (sin + cos) */
            double sr = sinr + cosr, si = sini + cosi;
            double oldr = cRe[j-1], oldi = cIm[j-1];
            c_mul(oldr, oldi, sr, si, &cRe[j-1], &cIm[j-1]);
        }
    }

    /* specific_indices = [3,8,15,22,29,35] */
    int spec[] = {3, 8, 15, 22, 29, 35};
    for (int si = 0; si < 6; si++) {
        int k = spec[si];
        /* += (real(t1)+imag(t2))*t1**k - (real(t2)-imag(t1))*t2**k */
        double a = x1r + x2i;
        double b = x2r - x1i;
        double t1kr, t1ki, t2kr, t2ki;
        c_powr(x1r, x1i, (double)k, &t1kr, &t1ki);
        c_powr(x2r, x2i, (double)k, &t2kr, &t2ki);
        cRe[k-1] += a * t1kr - b * t2kr;
        cIm[k-1] += a * t1ki - b * t2ki;
    }

    /* cf[[4,11,18,25,32]] = [5,-10,15,-20,25] + 1j*[-5,10,-15,20,-25] */
    int fix_idx[] = {4, 11, 18, 25, 32};
    double fix_re[] = {5, -10, 15, -20, 25};
    double fix_im[] = {-5, 10, -15, 20, -25};
    for (int i = 0; i < 5; i++) {
        cRe[fix_idx[i]] = fix_re[i];
        cIm[fix_idx[i]] = fix_im[i];
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_284_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs_t1 = c_abs(x1r, x1i);
    double abs_t2 = c_abs(x2r, x2i);
    double ang_t1 = c_arg(x1r, x1i);
    double ang_t2 = c_arg(x2r, x2i);
    double sumr = x1r + x2r, sumi = x1i + x2i;
    double log_abs_sum = log(c_abs(sumr, sumi) + 1.0);

    for (int j = 1; j <= 35; j++) {
        double phase = ang_t1 * sin((double)j) + ang_t2 * cos((double)j);
        int m5 = (j % 5) + 1;
        int d7 = j / 7 + 1;
        double magnitude = pow(abs_t1, (double)m5) + pow(abs_t2, (double)d7);
        double perturb = log_abs_sum * cos((double)j * M_PI / 3.0) + sin((double)j * M_PI / 4.0);
        /* magnitude * exp(1j*phase) + perturb */
        cRe[j-1] = magnitude * cos(phase) + perturb;
        cIm[j-1] = magnitude * sin(phase);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_285_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * For each j in 1..35:
     *   real_seq = linspace(real(t1), real(t2), j)  -- j elements
     *   imag_seq = linspace(imag(t1), imag(t2), j)  -- j elements
     *   mag = sum(log(abs(real_seq)+1)*sin(real_seq*j)) + prod(imag_seq+1)
     *   angle = sum(cos(imag_seq*j)) - sum(sin(real_seq/(j+1)))
     *   cf[j-1] = mag * (cos(angle) + 1j*sin(angle))
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double r1 = x1r, r2 = x2r, i1 = x1i, i2 = x2i;

    for (int j = 1; j <= 35; j++) {
        /* linspace of j elements from r1 to r2, i1 to i2 */
        double mag_sum = 0.0;
        double prod_val = 1.0;
        double angle_cos_sum = 0.0;
        double angle_sin_sum = 0.0;

        for (int idx = 0; idx < j; idx++) {
            double rval, ival;
            if (j == 1) {
                rval = r1; ival = i1;
            } else {
                double t = (double)idx / (double)(j - 1);
                rval = r1 + t * (r2 - r1);
                ival = i1 + t * (i2 - i1);
            }
            mag_sum += log(fabs(rval) + 1.0) * sin(rval * (double)j);
            prod_val *= (ival + 1.0);
            angle_cos_sum += cos(ival * (double)j);
            angle_sin_sum += sin(rval / (double)(j + 1));
        }

        double mag = mag_sum + prod_val;
        double angle = angle_cos_sum - angle_sin_sum;
        cRe[j-1] = mag * cos(angle);
        cIm[j-1] = mag * sin(angle);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_286_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs_t1 = c_abs(x1r, x1i);
    double abs_t2 = c_abs(x2r, x2i);
    double ang_t1 = c_arg(x1r, x1i);
    double ang_t2 = c_arg(x2r, x2i);

    for (int k = 1; k <= 35; k++) {
        double a = x1r * sin((double)k * x2r) + x1i * cos((double)k * x2i);
        double b = log(abs_t1 + 1.0) * sin((double)k * ang_t2 / (double)(k + 1));
        double cv = pow(abs_t2, (double)k) * cos((double)k * x1r);
        double d = sin((double)k * x1i) + cos((double)k * x2r);
        double angle = ang_t1 * sin((double)k) + ang_t2 * cos((double)k);
        double magnitude = a + b + cv + d;

        double mr = magnitude * cos(angle);
        double mi = magnitude * sin(angle);

        /* + conj(t1) * conj(t2)**k */
        double ct2kr, ct2ki;
        c_powr(x2r, -x2i, (double)k, &ct2kr, &ct2ki);
        double pr, pi;
        c_mul(x1r, -x1i, ct2kr, ct2ki, &pr, &pi);

        cRe[k-1] = mr + pr;
        cIm[k-1] = mi + pi;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_287_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    int n = 35;
    double ang_t1 = c_arg(x1r, x1i);
    double ang_t2 = c_arg(x2r, x2i);
    /* conj(t1)*conj(t2) = conj(t1*t2) */
    double ct1r = x1r, ct1i = -x1i;
    double ct2r = x2r, ct2i = -x2i;
    double cpr, cpi;
    c_mul(ct1r, ct1i, ct2r, ct2i, &cpr, &cpi);

    for (int j = 1; j <= n; j++) {
        /* linspace element: rec[j-1] and imc[j-1] */
        double t = (double)(j - 1) / (double)(n - 1);
        double r = x1r + t * (x2r - x1r);
        double m = x1i + t * (x2i - x1i);

        double mag_part = log(fabs(r * m) + 1.0) * ((double)(j*j) + sin((double)j) * cos((double)j));
        double angle_part = ang_t1 * sin((double)j / 3.0) + ang_t2 * cos((double)j / 4.0) + sin(m * M_PI / 5.0);

        /* coeff = mag_part * exp(1j*angle_part) + conj(t1)*conj(t2)/(j+1) */
        cRe[j-1] = mag_part * cos(angle_part) + cpr / (double)(j + 1);
        cIm[j-1] = mag_part * sin(angle_part) + cpi / (double)(j + 1);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_288_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Uses linspace, nested loop, and np.prod(np.repeat(abs(t1)+k, j%3+1)).
     * np.repeat(val, count) creates array of val repeated count times.
     * np.prod of that = val^count.
     * Note: k in the prod expression is the loop variable from the inner loop
     *       at its final value (k = j at end of inner loop).
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    int n = 35;
    double abs_t1 = c_abs(x1r, x1i);

    /* precompute linspace */
    double rec[35], imc[35];
    for (int i = 0; i < n; i++) {
        double t = (double)i / (double)(n - 1);
        rec[i] = x1r + t * (x2r - x1r);
        imc[i] = x1i + t * (x2i - x1i);
    }

    for (int j = 1; j <= n; j++) {
        double mag_sum = 0.0;
        double angle_sum = 0.0;
        int last_k = j; /* k goes 1..j, final k=j */

        for (int k = 1; k <= j; k++) {
            double term_mag = log(fabs(rec[k-1] * imc[j-1]) + 1.0) * sin((double)k * M_PI / (double)n);
            double term_angle = atan2(imc[j-1], rec[k-1]) + cos((double)k * M_PI / (double)(n + 1));
            mag_sum += term_mag;
            angle_sum += term_angle;
        }

        /* prod = (abs(t1) + k)^(j%3+1) where k = last_k = j */
        int rep_count = j % 3 + 1;
        double base = abs_t1 + (double)last_k;
        double prod_val = pow(base, (double)rep_count);
        double magnitude = mag_sum * prod_val;

        double angle = angle_sum / (double)(j + 1)
                      + sin((double)j * M_PI / (double)(n + 2))
                      * cos((double)j * M_PI / (double)(n + 3));

        double variation = (j % 2 == 0) ? sin((double)j) : cos((double)j);

        /* conj(t2)**j */
        double ct2jr, ct2ji;
        c_powr(x2r, -x2i, (double)j, &ct2jr, &ct2ji);

        /* magnitude * exp(1j*angle) + variation * conj(t2)**j */
        cRe[j-1] = magnitude * cos(angle) + variation * ct2jr;
        cIm[j-1] = magnitude * sin(angle) + variation * ct2ji;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_289_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    int n = 35;
    for (int j = 1; j <= n; j++) {
        double t = (double)(j - 1) / (double)(n - 1);
        double rec_j = x1r + t * (x2r - x1r);
        double imc_j = x1i + t * (x2i - x1i);

        /* mag_factor = log(abs(rec+imc*1j)+1) * (1+sin(j*pi/4)) */
        double abs_val = c_abs(rec_j, imc_j);
        double mag_factor = log(abs_val + 1.0) * (1.0 + sin((double)j * M_PI / 4.0));

        /* angle_factor = angle(rec+1j*imc) + cos(j*pi/3)*sin(j*pi/5) */
        double angle_factor = atan2(imc_j, rec_j) + cos((double)j * M_PI / 3.0) * sin((double)j * M_PI / 5.0);

        cRe[j-1] = mag_factor * cos(angle_factor);
        cIm[j-1] = mag_factor * sin(angle_factor);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_290_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs_t1 = c_abs(x1r, x1i);
    double abs_t2 = c_abs(x2r, x2i);
    double ang_t1 = c_arg(x1r, x1i);
    double ang_t2 = c_arg(x2r, x2i);
    double sumr = x1r + x2r, sumi = x1i + x2i;
    double diffr = x1r - x2r, diffi = x1i - x2i;
    double ang_prod_r, ang_prod_i;
    c_mul(x1r, x1i, x2r, x2i, &ang_prod_r, &ang_prod_i);
    double ang_prod = c_arg(ang_prod_r, ang_prod_i);

    for (int j = 1; j <= 35; j++) {
        double mag, angle;
        if (j % 4 == 1) {
            mag = log(abs_t1 + (double)(j*j)) + sin((double)j * M_PI / 6.0) * cos((double)j * M_PI / 4.0);
            angle = ang_t1 * (double)j + sin((double)j * M_PI / 5.0) - cos((double)j * M_PI / 3.0);
        } else if (j % 4 == 2) {
            /* np.prod(np.arange(1, (j%5)+2)) = factorial of (j%5+1) */
            int fac_n = (j % 5) + 1;
            double fac = 1.0;
            for (int f = 2; f <= fac_n; f++) fac *= (double)f;
            mag = log(abs_t2 + (double)j) * fac;
            angle = ang_t2 / (double)(j + 1) + sin((double)j * M_PI / 7.0);
        } else if (j % 4 == 3) {
            mag = x1r * (double)j - x2i + log(c_abs(sumr, sumi) + 1.0);
            angle = ang_prod + cos((double)j * M_PI / 2.0);
        } else {
            /* j%4 == 0 */
            double re_diff_r, re_diff_i;
            re_diff_r = diffr; re_diff_i = diffi;
            double re_diff = x1r - x2r; /* real(t1-t2) = real part of diff */
            mag = fabs(re_diff) * pow((double)j, 1.5) + sin((double)j * M_PI / 3.0);
            angle = c_arg(diffr, diffi) + sin((double)j * M_PI / 4.0);
        }
        cRe[j-1] = mag * cos(angle);
        cIm[j-1] = mag * sin(angle);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_291_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * Complex function with factorial, trig sums, and perturbation.
     * np.prod(np.arange(1,j+1))**0.5 = sqrt(j!)
     * sum(sin(arange(1,j+1)*pi/6)) and sum(cos(arange(1,j+1)*pi/8))
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    int n = 35;
    double abs_t1 = c_abs(x1r, x1i);
    double abs_t2 = c_abs(x2r, x2i);
    double ang_t1 = c_arg(x1r, x1i);
    double ang_t2 = c_arg(x2r, x2i);
    double abs_re_diff = fabs(x1r - x2r);

    double factorial = 1.0;

    /* running sums for sin and cos series */
    double sin_sum = 0.0, cos_sum = 0.0;

    for (int j = 1; j <= n; j++) {
        factorial *= (double)j;

        sin_sum += sin((double)j * M_PI / 6.0);
        cos_sum += cos((double)j * M_PI / 8.0);

        double mag_part1 = log(abs_t1 + abs_t2 + (double)j) * (1.0 + sin((double)j * M_PI / 7.0));
        double mag_part2 = sqrt(factorial) / (1.0 + abs_re_diff / (double)(j + 1));
        double magnitude = mag_part1 * mag_part2 * (1.0 + cos((double)j * M_PI / 5.0));

        double angle_part1 = ang_t1 * sin((double)j / 3.0) + ang_t2 * cos((double)j / 4.0);
        double angle_part2 = sin_sum - cos_sum;
        double angle = angle_part1 + angle_part2;

        double real_component = x1r * cos((double)j) - x2i * sin((double)j);
        double imag_component = x2r * sin((double)j) + x1i * cos((double)j);
        double perturbation = sin(real_component) + cos(imag_component);

        /* magnitude * exp(1j*angle) * perturbation */
        cRe[j-1] = magnitude * cos(angle) * perturbation;
        cIm[j-1] = magnitude * sin(angle) * perturbation;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_292_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    int n = 35;
    double abs_t1_05 = pow(c_abs(x1r, x1i), 0.5);
    double abs_t2_03 = pow(c_abs(x2r, x2i), 0.3);
    double ang_t1 = c_arg(x1r, x1i);
    double ang_t2 = c_arg(x2r, x2i);

    for (int j = 1; j <= n; j++) {
        int k = (j*j + 3*j + 1) % n + 1;
        double r = sin((double)j * x1r) * cos((double)k * x2i);
        double angle = ang_t1 * (double)j - ang_t2 * (double)k + log((double)(j + 1));
        double magnitude = abs_t1_05 * abs_t2_03 * fabs(r) + (double)j;
        cRe[j-1] = magnitude * cos(angle);
        cIm[j-1] = magnitude * sin(angle);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_293_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    int n = 35;
    double ang_t1 = c_arg(x1r, x1i);
    double ang_t2 = c_arg(x2r, x2i);

    for (int j = 1; j <= n; j++) {
        int k = (j * 2 + 5) % 12;
        /* r = j // 6 + 1 (unused in the formula but declared in Python) */
        double term_re = x1r * sin((double)j) + x2r * cos((double)k);
        double term_im = x1i * cos((double)j / 4.0) - x2i * sin((double)k / 3.0);
        double magnitude = (fabs(term_re) + fabs(term_im)) * log(1.0 + (double)j) * pow((double)j, 0.4);
        double angle = ang_t1 * sin((double)j / 2.0) + ang_t2 * cos((double)k / 4.0) + log((double)(j + 2));

        cRe[j-1] = magnitude * cos(angle);
        cIm[j-1] = magnitude * sin(angle);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_294_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    int n = 35;
    double ang_t1 = c_arg(x1r, x1i);
    double ang_t2 = c_arg(x2r, x2i);
    /* conj(t1*t2) */
    double pr, pi;
    c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
    double conj_prod_r = pr, conj_prod_i = -pi;

    for (int j = 1; j <= n; j++) {
        double t = (double)(j - 1) / (double)(n - 1);
        double rec_j = x1r + t * (x2r - x1r);
        double imc_j = x1i + t * (x2i - x1i);

        int k = (j % 7) + 1;
        double r = rec_j * cos((double)j) - imc_j * sin((double)j);
        double i_part = rec_j * sin((double)j) + imc_j * cos((double)j);
        double mag = log(fabs(r + 1.0) + fabs(i_part + 1.0))
                   * (1.0 + sin((double)j * M_PI / (double)k))
                   * (1.0 + cos((double)j * M_PI / (double)(k + 1)));
        double angle = ang_t1 + ang_t2 + sin((double)j * M_PI / (double)k) + cos((double)j * M_PI / (double)(k + 2));

        cRe[j-1] = mag * cos(angle) + conj_prod_r / (double)(j + 2);
        cIm[j-1] = mag * sin(angle) + conj_prod_i / (double)(j + 2);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_295_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    int n = 35;
    for (int j = 1; j <= n; j++) {
        double t = (double)(j - 1) / (double)(n - 1);
        double rec_j = x1r + t * (x2r - x1r);
        double imc_j = x1i + t * (x2i - x1i);

        double r = rec_j + imc_j;
        /* r + 1j is (r, 1.0) */
        double magnitude = log(c_abs(r, 1.0) + 1.0) * pow((double)j, sin((double)j) + 1.0);
        double angle = atan2(1.0, r) + sin((double)j * M_PI / 4.0) * cos((double)j * M_PI / 3.0);

        /* main term: magnitude * exp(1j*angle) */
        double mr = magnitude * cos(angle);
        double mi = magnitude * sin(angle);

        /* conjugate term: conj(magnitude*exp(1j*(angle/2))) * cos(j*pi/5)
           magnitude is real, so conj(mag*exp(1j*a2)) = mag*exp(-1j*a2) */
        double a2 = angle / 2.0;
        double cos_j5 = cos((double)j * M_PI / 5.0);
        double cr = magnitude * cos(a2) * cos_j5;  /* conj has -sin for imag part */
        double ci = -magnitude * sin(a2) * cos_j5;

        cRe[j-1] = mr + cr;
        cIm[j-1] = mi + ci;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_296_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs_t1 = c_abs(x1r, x1i);
    double abs_t2 = c_abs(x2r, x2i);

    /* conj(t1)*conj(t2) */
    double cpr, cpi;
    c_mul(x1r, -x1i, x2r, -x2i, &cpr, &cpi);
    double conj_angle = c_arg(cpr, cpi);

    /* precompute linspace */
    double rec[35], imc[35];
    for (int i = 0; i < 35; i++) {
        double t = (double)i / 34.0;
        rec[i] = x1r + t * (x2r - x1r);
        imc[i] = x1i + t * (x2i - x1i);
    }

    for (int k = 1; k <= 35; k++) {
        int j = (k + 7) % 12 + 1;  /* j is 1..12, used as index into rec/imc (1-based) */
        double term1 = rec[k-1] * cos(imc[j-1] * M_PI / 5.0);
        double term2 = imc[k-1] * sin(rec[j-1] * M_PI / 4.0);

        /* angle = np.angle(term1 + term2 + np.angle(conj_part))
           Note: term1+term2+conj_angle is a real number,
           so angle = atan2(0, term1+term2+conj_angle) = 0 or pi */
        double ang_input = term1 + term2 + conj_angle;
        double angle = atan2(0.0, ang_input);

        double magnitude = log(fabs(term1 + term2) + 1.0)
                         * pow(abs_t1, (double)((k % 4) + 1))
                         * pow(abs_t2, (double)((j % 3) + 1));

        cRe[k-1] = magnitude * cos(angle);
        cIm[k-1] = magnitude * sin(angle);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_297_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    int n = 35;
    double abs_t1 = c_abs(x1r, x1i);
    double abs_t2 = c_abs(x2r, x2i);
    double ang_t1 = c_arg(x1r, x1i);
    double ang_t2 = c_arg(x2r, x2i);

    for (int j = 1; j <= n; j++) {
        double mag_part = log(abs_t1 + (double)j) * sin((double)j * ang_t2) + cos((double)(j*j) * x1r);
        double angle_part = ang_t1 * log((double)(j + 1)) + ang_t2 * sqrt((double)j);

        double mr = mag_part * cos(angle_part);
        double mi = mag_part * sin(angle_part);

        /* + conj(t1)**j / (1 + abs(t2 + j)) */
        double ct1jr, ct1ji;
        c_powr(x1r, -x1i, (double)j, &ct1jr, &ct1ji);
        /* abs(t2 + j) = abs((x2r+j) + x2i*1j) */
        double denom = 1.0 + c_abs(x2r + (double)j, x2i);
        cRe[j-1] = mr + ct1jr / denom;
        cIm[j-1] = mi + ct1ji / denom;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_298_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    int n = 35;
    double abs_t1 = c_abs(x1r, x1i);
    double abs_t2 = c_abs(x2r, x2i);
    double ang_t1 = c_arg(x1r, x1i);
    double ang_t2 = c_arg(x2r, x2i);
    double abs_sum = c_abs(x1r + x2r, x1i + x2i);

    /* conj(t1*t2) */
    double pr, pi;
    c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
    double conj_pr = pr, conj_pi = -pi;

    for (int j = 1; j <= n; j++) {
        double t = (double)(j - 1) / (double)(n - 1);
        double rec_j = x1r + t * (x2r - x1r);

        double mag_part = log(fabs(rec_j) + 1.0) * sin((double)j * M_PI / 7.0) + cos((double)j * M_PI / 5.0);
        double angle_part = ang_t1 * sqrt((double)j) - ang_t2 / (double)(j + 2);
        double fluctuation;
        if (j % 3 == 0) {
            fluctuation = abs_t1 * abs_t2;
        } else {
            fluctuation = abs_sum / (double)(j + 1);
        }

        double mr = (mag_part + fluctuation) * cos(angle_part);
        double mi = (mag_part + fluctuation) * sin(angle_part);

        /* + conj(t1*t2)**j */
        double cpjr, cpji;
        c_powr(conj_pr, conj_pi, (double)j, &cpjr, &cpji);

        cRe[j-1] = mr + cpjr;
        cIm[j-1] = mi + cpji;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_299_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    /*
     * mag = abs(t1)**j + abs(t2)**(n-j+1) + sum(sin(j*pi/(arange(1,6)+1)))
     * ang = angle(t1)*log(j+1) + angle(t2)*arctan(j) + sum(cos(arange(1,4)*pi/j))
     * The sin sum: sum over m=2..6 of sin(j*pi/m)
     * The cos sum: sum over m=1..3 of cos(m*pi/j)
     */
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    int n = 35;
    double abs_t1 = c_abs(x1r, x1i);
    double abs_t2 = c_abs(x2r, x2i);
    double ang_t1 = c_arg(x1r, x1i);
    double ang_t2 = c_arg(x2r, x2i);

    for (int j = 1; j <= n; j++) {
        /* sum(sin(j*pi/(arange(1,6)+1))) = sin(j*pi/2)+sin(j*pi/3)+sin(j*pi/4)+sin(j*pi/5)+sin(j*pi/6) */
        double sin_sum = 0.0;
        for (int m = 2; m <= 6; m++) {
            sin_sum += sin((double)j * M_PI / (double)m);
        }

        double mag = pow(abs_t1, (double)j) + pow(abs_t2, (double)(n - j + 1)) + sin_sum;

        /* sum(cos(arange(1,4)*pi/j)) = cos(pi/j)+cos(2*pi/j)+cos(3*pi/j) */
        double cos_sum = 0.0;
        for (int m = 1; m <= 3; m++) {
            cos_sum += cos((double)m * M_PI / (double)j);
        }

        double ang = ang_t1 * log((double)(j + 1)) + ang_t2 * atan((double)j) + cos_sum;

        cRe[j-1] = mag * cos(ang);
        cIm[j-1] = mag * sin(ang);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_300_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    int n = 35;
    double abs_t1 = c_abs(x1r, x1i);
    double ang_t1 = c_arg(x1r, x1i);
    double ang_t2 = c_arg(x2r, x2i);
    double r1 = x1r, r2 = x2r, i1 = x1i, i2 = x2i;

    for (int j = 1; j <= n; j++) {
        /* term_mag = log(abs(t1)+j) * abs(r1*j - i2/(j+1)) + prod([r1,i2,j]) */
        double inner = r1 * (double)j - i2 / (double)(j + 1);
        double term_mag = log(abs_t1 + (double)j) * fabs(inner) + r1 * i2 * (double)j;

        /* term_angle = angle(t1)*j - angle(t2)*(n-j) + sin(j*r2)*cos(j*i1) */
        double term_angle = ang_t1 * (double)j - ang_t2 * (double)(n - j)
                          + sin((double)j * r2) * cos((double)j * i1);

        cRe[j-1] = term_mag * cos(term_angle);
        cIm[j-1] = term_mag * sin(term_angle);
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}
