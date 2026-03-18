/* poly_hand_300c.h — Hand-written C for poly_241 through poly_260 */

static void poly_241_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    /* conj(t1) = (x1r, -x1i) */
    double conj1r = x1r, conj1i = -x1i;

    /* First loop: magnitude/angle based coefficients */
    for (int j = 1; j <= 35; j++) {
        double magnitude = log(abs1 + abs2 + j) * (pow(abs1, sin(j)) + pow(abs2, cos(j)));
        double angle = ang1 * j - ang2 * (35 - j) + sin(j) * cos(j);
        cRe[j - 1] = magnitude * cos(angle);
        cIm[j - 1] = magnitude * sin(angle);
    }

    /* Second loop: cf[k-1] += conj(t1) * t2^k / (k+1) */
    /* t2^k iteratively */
    double t2pk_r = x2r, t2pk_i = x2i; /* t2^1 */
    for (int k = 1; k <= 35; k++) {
        /* conj(t1) * t2^k */
        double mr, mi;
        c_mul(conj1r, conj1i, t2pk_r, t2pk_i, &mr, &mi);
        cRe[k - 1] += mr / (k + 1.0);
        cIm[k - 1] += mi / (k + 1.0);
        /* t2^(k+1) */
        double nr, ni;
        c_mul(t2pk_r, t2pk_i, x2r, x2i, &nr, &ni);
        t2pk_r = nr; t2pk_i = ni;
    }

    /* cf[[4,9,14,19,24,29]] += 50*(real(t1) - imag(t2))*1j */
    {
        double val = 50.0 * (x1r - x2i);
        int indices[] = {4, 9, 14, 19, 24, 29};
        for (int i = 0; i < 6; i++) {
            cIm[indices[i]] += val;
        }
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_242_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double conj1r = x1r, conj1i = -x1i;

    /* t2^j iteratively */
    double t2j_r = 1.0, t2j_i = 0.0;
    /* conj(t1)^j iteratively */
    double c1j_r = 1.0, c1j_i = 0.0;

    for (int j = 1; j <= 35; j++) {
        /* advance t2^j */
        { double nr, ni; c_mul(t2j_r, t2j_i, x2r, x2i, &nr, &ni); t2j_r = nr; t2j_i = ni; }
        /* advance conj(t1)^j */
        { double nr, ni; c_mul(c1j_r, c1j_i, conj1r, conj1i, &nr, &ni); c1j_r = nr; c1j_i = ni; }

        int m = j % 5;
        if (m == 1) {
            double val = sin(abs1 * j) + cos(ang2 * j);
            cRe[j - 1] = val;
        } else if (m == 2) {
            /* log(abs(t1)+1) * t2^j */
            double s = log(abs1 + 1.0);
            cRe[j - 1] = s * t2j_r;
            cIm[j - 1] = s * t2j_i;
        } else if (m == 3) {
            /* conj(t1)^j - real(t2)*j */
            cRe[j - 1] = c1j_r - x2r * j;
            cIm[j - 1] = c1j_i;
        } else if (m == 4) {
            cRe[j - 1] = x1i + abs2 * sin(j * ang1);
        } else {
            /* t1*t2^j + cos(j) - sin(j) */
            double mr, mi;
            c_mul(x1r, x1i, t2j_r, t2j_i, &mr, &mi);
            cRe[j - 1] = mr + cos(j) - sin(j);
            cIm[j - 1] = mi;
        }
    }

    /* cf[6] = 50j*t1^2 - 30j*t2 + 20 */
    { double t1sq_r, t1sq_i; c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
      cRe[6] = -50.0*t1sq_i + 30.0*x2i + 20.0;
      cIm[6] = 50.0*t1sq_r - 30.0*x2r; }
    /* cf[13] = 80*t1 - 60j*t2^2 + 10 */
    { double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
      cRe[13] = 80.0*x1r + 60.0*t2sq_i + 10.0;
      cIm[13] = 80.0*x1i - 60.0*t2sq_r; }
    /* cf[20] = 40j*t1^3 + 25*conj(t2) - 15 */
    { double t1sq_r, t1sq_i; c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
      double t1cu_r, t1cu_i; c_mul(t1sq_r, t1sq_i, x1r, x1i, &t1cu_r, &t1cu_i);
      cRe[20] = -40.0*t1cu_i + 25.0*x2r - 15.0;
      cIm[20] = 40.0*t1cu_r - 25.0*x2i; }
    /* cf[27] = 70*abs(t1) + 35j*angle(t2) + 5 */
    cRe[27] = 70.0*abs1 + 5.0;
    cIm[27] = 35.0*ang2;
    /* cf[34] = 90j*t1*t2 - 45*real(t1) + 22.5 */
    { double mr, mi; c_mul(x1r, x1i, x2r, x2i, &mr, &mi);
      cRe[34] = -90.0*mi - 45.0*x1r + 22.5;
      cIm[34] = 90.0*mr; }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_243_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* Fixed coefficients */
    cRe[0] = 2.0;
    cRe[5] = -3.0; cIm[5] = 2.0;
    cRe[9] = 4.5;
    cIm[14] = -5.2;
    cRe[21] = 3.3;
    cRe[27] = -1.1;

    /* Loop j=2..5: cf[j-1] = (real(t1)^j + imag(t2)^j)*sin(angle(t1)*j)/(1+j) */
    for (int j = 2; j <= 5; j++) {
        double val = (pow(x1r, j) + pow(x2i, j)) * sin(ang1 * j) / (1.0 + j);
        cRe[j - 1] = val;
    }

    /* Loop k=7..14: cf[k-1] = abs(t1)^k*cos(angle(t2)*k) + conj(t2)*log(abs(t1*t2)+1) */
    {
        double t1t2_r, t1t2_i;
        c_mul(x1r, x1i, x2r, x2i, &t1t2_r, &t1t2_i);
        double logabs = log(c_abs(t1t2_r, t1t2_i) + 1.0);
        double conj2r = x2r, conj2i = -x2i;
        for (int k = 7; k <= 14; k++) {
            double term1 = pow(abs1, k) * cos(ang2 * k);
            cRe[k - 1] = term1 + conj2r * logabs;
            cIm[k - 1] = conj2i * logabs;
        }
    }

    /* Loop r=16..25: cf[r-1] = (real(t1^r) - imag(t2^r)*1j)*sin(t1+t2) + cos(t1*t2) */
    {
        /* sin(t1+t2) complex */
        double sr, si;
        c_sin(x1r + x2r, x1i + x2i, &sr, &si);
        /* cos(t1*t2) complex */
        double t1t2r, t1t2i;
        c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        double cosr, cosi;
        c_cos(t1t2r, t1t2i, &cosr, &cosi);

        /* t1^r iteratively */
        double t1r_r = 1.0, t1r_i = 0.0;
        double t2r_r = 1.0, t2r_i = 0.0;
        for (int r = 1; r <= 25; r++) {
            { double nr, ni; c_mul(t1r_r, t1r_i, x1r, x1i, &nr, &ni); t1r_r = nr; t1r_i = ni; }
            { double nr, ni; c_mul(t2r_r, t2r_i, x2r, x2i, &nr, &ni); t2r_r = nr; t2r_i = ni; }
            if (r >= 16) {
                /* (real(t1^r) - imag(t2^r)*1j) = (t1r_r, -t2r_i) */
                double ar = t1r_r, ai = -t2r_i;
                /* * sin(t1+t2) */
                double mr, mi;
                c_mul(ar, ai, sr, si, &mr, &mi);
                cRe[r - 1] = mr + cosr;
                cIm[r - 1] = mi + cosi;
            }
        }
    }

    /* cf[25] = prod([abs(t1),abs(t2)]) + sum([real(t1),imag(t2)])*conj(t1+t2) */
    {
        double prod = abs1 * abs2;
        double s = x1r + x2i;
        double sumr = x1r + x2r, sumi = x1i + x2i; /* t1+t2 */
        cRe[25] = prod + s * sumr;
        cIm[25] = -s * sumi; /* conj negates imag */
    }

    /* cf[26] = log(abs(t1)+1) + log(abs(t2)+1)*1j */
    cRe[26] = log(abs1 + 1.0);
    cIm[26] = log(abs2 + 1.0);

    /* cf[28] = real(t1*t2) - imag(t1/t2)*1j */
    {
        double mr, mi; c_mul(x1r, x1i, x2r, x2i, &mr, &mi);
        double dr, di; c_div(x1r, x1i, x2r, x2i, &dr, &di);
        cRe[28] = mr;
        cIm[28] = -di;
    }

    /* cf[29] = sin(t1^2) + cos(t2^3)*1j */
    {
        double t1sq_r, t1sq_i; c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
        double sr, si; c_sin(t1sq_r, t1sq_i, &sr, &si);
        double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
        double t2cu_r, t2cu_i; c_mul(t2sq_r, t2sq_i, x2r, x2i, &t2cu_r, &t2cu_i);
        double cr, ci; c_cos(t2cu_r, t2cu_i, &cr, &ci);
        cRe[29] = sr - ci;
        cIm[29] = si + cr;
    }

    /* cf[31] = abs(t1+t2)*exp(-real(t1-t2)) */
    {
        double absum = c_abs(x1r + x2r, x1i + x2i);
        double expval = exp(-(x1r - x2r));
        cRe[31] = absum * expval;
    }

    /* cf[33] = angle(t1) + angle(t2)*1j */
    cRe[33] = ang1;
    cIm[33] = ang2;

    /* cf[34] = (t1^3+t2^3)/(1+abs(t1)+abs(t2)) */
    {
        double t1sq_r, t1sq_i; c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
        double t1cu_r, t1cu_i; c_mul(t1sq_r, t1sq_i, x1r, x1i, &t1cu_r, &t1cu_i);
        double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
        double t2cu_r, t2cu_i; c_mul(t2sq_r, t2sq_i, x2r, x2i, &t2cu_r, &t2cu_i);
        double denom = 1.0 + abs1 + abs2;
        cRe[34] = (t1cu_r + t2cu_r) / denom;
        cIm[34] = (t1cu_i + t2cu_i) / denom;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_244_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);

    for (int j = 1; j <= 35; j++) {
        int k = (j % 6) + 1;
        int r = (j % 4) + 1;
        double angle_part = sin(j * x1r) * cos(j * x2i) + ang1 / (k + 1.0);
        double mag_part = pow(abs1, k) * pow(abs2, r) + log(abs1 + abs2 + j);
        cRe[j - 1] = cos(angle_part) * mag_part;
        cIm[j - 1] = sin(angle_part) * mag_part;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_245_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* Fixed coefficients at indices 1,5,9,13,17,21,25,29,33 */
    double fixed_re[] = {2, -3, 4, -5, 6, -7, 8, -9, 10};
    double fixed_im[] = {3, 2, -1, 4, -3, 5, -4, 6, -5};
    int fixed_idx[] = {1, 5, 9, 13, 17, 21, 25, 29, 33};
    for (int i = 0; i < 9; i++) {
        cRe[fixed_idx[i]] = fixed_re[i];
        cIm[fixed_idx[i]] = fixed_im[i];
    }

    /* j_indices: 0,4,8,12,16,20,24,28,32 */
    /* sin(t1+t2) complex, cos(t2) complex, log(abs(t1)+1) real */
    {
        double st_r, st_i; c_sin(x1r + x2r, x1i + x2i, &st_r, &st_i);
        double ct_r, ct_i; c_cos(x2r, x2i, &ct_r, &ct_i);
        double logval = log(abs1 + 1.0);
        double cross = x1r * x2i + x1i * x2r;
        double absdiff = abs1 * abs1 - abs2 * abs2;
        /* absdiff * sin(t1+t2): complex * real = (absdiff*st_r, absdiff*st_i) */
        /* log(abs(t1)+1) * cos(t2): real * complex = (logval*ct_r, logval*ct_i) */
        int j_indices[] = {0, 4, 8, 12, 16, 20, 24, 28, 32};
        for (int i = 0; i < 9; i++) {
            int idx = j_indices[i];
            cRe[idx] = cross + absdiff * st_r + logval * ct_r;
            cIm[idx] = absdiff * st_i + logval * ct_i;
        }
    }

    /* k_indices: 2,6,10,14,18,22,26,30,34 */
    {
        /* sin(t1*t2) complex */
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        double st_r, st_i; c_sin(t1t2r, t1t2i, &st_r, &st_i);
        /* cos(t1/(abs(t2)+1)) */
        double denom = abs2 + 1.0;
        double dr = x1r / denom, di = x1i / denom;
        double ct_r, ct_i; c_cos(dr, di, &ct_r, &ct_i);
        /* ct * conj(t2) */
        double mr, mi; c_mul(ct_r, ct_i, x2r, -x2i, &mr, &mi);
        /* angle(t1+t2) */
        double ang_sum = c_arg(x1r + x2r, x1i + x2i);
        /* abs(t1-t2) */
        double abs_diff = c_abs(x1r - x2r, x1i - x2i);
        double prod = x1r * x2i;

        int k_indices[] = {2, 6, 10, 14, 18, 22, 26, 30, 34};
        for (int i = 0; i < 9; i++) {
            int idx = k_indices[i];
            cRe[idx] = st_r + mr + ang_sum * abs_diff + prod;
            cIm[idx] = st_i + mi;
        }
    }

    /* r_indices: 3,7,11,15,19,23,27,31 */
    {
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        double logprod = log(c_abs(t1t2r, t1t2i) + 1.0);
        double val = x1r * x1r * x1r - x2i * x2i * x2i + t1t2r + x1i + x2i + logprod;
        int r_indices[] = {3, 7, 11, 15, 19, 23, 27, 31};
        for (int i = 0; i < 8; i++) {
            cRe[r_indices[i]] = val;
        }
    }

    /* Overwrite specific coefficients */
    /* cf[18] = 100j*t1^3 + 50j*t2^2 - 75*t1*t2 + 25 */
    {
        double t1sq_r, t1sq_i; c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
        double t1cu_r, t1cu_i; c_mul(t1sq_r, t1sq_i, x1r, x1i, &t1cu_r, &t1cu_i);
        double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[18] = -100.0*t1cu_i - 50.0*t2sq_i - 75.0*t1t2r + 25.0;
        cIm[18] = 100.0*t1cu_r + 50.0*t2sq_r - 75.0*t1t2i;
    }
    /* cf[22] = 80j*t2^3 - 60j*t1^2 + 40*sin(t1+t2) - 20 */
    {
        double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
        double t2cu_r, t2cu_i; c_mul(t2sq_r, t2sq_i, x2r, x2i, &t2cu_r, &t2cu_i);
        double t1sq_r, t1sq_i; c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
        double sr, si; c_sin(x1r + x2r, x1i + x2i, &sr, &si);
        cRe[22] = -80.0*t2cu_i + 60.0*t1sq_i + 40.0*sr - 20.0;
        cIm[22] = 80.0*t2cu_r - 60.0*t1sq_r + 40.0*si;
    }
    /* cf[26] = 90j*t1*t2^2 - 70*cos(t1) + 50*log(abs(t2)+1) */
    {
        double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
        double p_r, p_i; c_mul(x1r, x1i, t2sq_r, t2sq_i, &p_r, &p_i);
        double cr, ci; c_cos(x1r, x1i, &cr, &ci);
        double logval = 50.0 * log(abs2 + 1.0);
        cRe[26] = -90.0*p_i - 70.0*cr + logval;
        cIm[26] = 90.0*p_r - 70.0*ci;
    }
    /* cf[30] = 110j*sin(t1^2) - 95*abs(t2)*t1 + 85j*angle(t1+t2) */
    {
        double t1sq_r, t1sq_i; c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
        double sr, si; c_sin(t1sq_r, t1sq_i, &sr, &si);
        double ang_sum = c_arg(x1r + x2r, x1i + x2i);
        cRe[30] = -110.0*si - 95.0*abs2*x1r;
        cIm[30] = 110.0*sr - 95.0*abs2*x1i + 85.0*ang_sum;
    }
    /* cf[34] = 120j*cos(t1*t2) - 100*sin(t2) + 75*log(abs(t1)+1) */
    {
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        double cr, ci; c_cos(t1t2r, t1t2i, &cr, &ci);
        double s2r, s2i; c_sin(x2r, x2i, &s2r, &s2i);
        double logval = 75.0 * log(abs1 + 1.0);
        cRe[34] = -120.0*ci - 100.0*s2r + logval;
        cIm[34] = 120.0*cr - 100.0*s2i;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_246_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* Fixed coefficients at indices 0,3,7,11,15,19,23,27,31 */
    /* cf[0] = 2+3j */
    cRe[0] = 2.0; cIm[0] = 3.0;

    /* cf[3] = conj(t1)*sin(t2) */
    { double sr, si; c_sin(x2r, x2i, &sr, &si);
      double mr, mi; c_mul(x1r, -x1i, sr, si, &mr, &mi);
      cRe[3] = mr; cIm[3] = mi; }

    /* cf[7] = log(abs(t1)+1) + log(abs(t2)+1)*1j */
    cRe[7] = log(abs1 + 1.0); cIm[7] = log(abs2 + 1.0);

    /* cf[11] = real(t1)^2 - imag(t2)^2 + (real(t2)*imag(t1))*1j */
    cRe[11] = x1r*x1r - x2i*x2i; cIm[11] = x2r*x1i;

    /* cf[15] = sin(t1*t2) + cos(t1+t2)*1j */
    { double pr, pi; c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
      double sr, si; c_sin(pr, pi, &sr, &si);
      double cr, ci; c_cos(x1r+x2r, x1i+x2i, &cr, &ci);
      cRe[15] = sr - ci; cIm[15] = si + cr; }

    /* cf[19] = prod([t1,t2]) + sum([real(t1),imag(t2)])*1j = t1*t2 + (x1r+x2i)*1j */
    { double pr, pi; c_mul(x1r, x1i, x2r, x2i, &pr, &pi);
      cRe[19] = pr; cIm[19] = pi + x1r + x2i; }

    /* cf[23] = abs(t1)^3 - abs(t2)^3 + angle(t1)*angle(t2)*1j */
    cRe[23] = abs1*abs1*abs1 - abs2*abs2*abs2; cIm[23] = ang1*ang2;

    /* cf[27] = real(t2)*sin(angle(t1)) + imag(t1)*cos(angle(t2))*1j */
    cRe[27] = x2r*sin(ang1); cIm[27] = x1i*cos(ang2);

    /* cf[31] = real(t1+t2) + imag(t1-t2)*1j */
    cRe[31] = x1r + x2r; cIm[31] = x1i - x2i;

    /* Loop for remaining indices */
    int loop_indices[] = {2, 3, 5, 6, 8, 9, 10, 12, 13, 14, 16, 17, 18, 20, 21, 22, 24, 25, 26, 27, 28, 29, 30, 32, 33, 34};
    for (int ii = 0; ii < 26; ii++) {
        int j = loop_indices[ii];
        int k = j * 3;
        int r = j % 4;
        double term1 = pow(x1r, k) * sin(k * ang1);
        double term2_re = pow(x2i, k) * sin(k * ang1); /* part of first term */
        /* Actually: (real(t1)^k + imag(t2)^k)*sin(k*angle(t1)) */
        double t1part = (pow(x1r, k) + pow(x2i, k)) * sin(k * ang1);
        /* (real(t2)^r - imag(t1)^r)*cos(r*angle(t2))*1j */
        double t2part = (pow(x2r, r) - pow(x1i, r)) * cos(r * ang2);
        double logpart = log(abs1 + abs2 + j);
        cRe[j] = t1part + logpart;
        cIm[j] = t2part + logpart;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_247_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);
    double conj1r = x1r, conj1i = -x1i;

    /* First loop: magnitude * (cos(angle) + j*sin(angle)) */
    for (int j = 1; j <= 35; j++) {
        double angle = pow(ang1, j) + pow(ang2, (j % 5) + 1);
        double magnitude = pow(abs1, j % 7) * pow(abs2, j / 5 + 1);
        cRe[j - 1] = magnitude * cos(angle);
        cIm[j - 1] = magnitude * sin(angle);
    }

    /* While loop: k=1,5,9,...,33: cf[k-1] += conj(t1)*t2^(k%3) - log(abs(t1+t2)+1) */
    {
        double logval = log(c_abs(x1r + x2r, x1i + x2i) + 1.0);
        /* Precompute t2^0, t2^1, t2^2 */
        double t2p[3][2];
        t2p[0][0] = 1.0; t2p[0][1] = 0.0;
        t2p[1][0] = x2r; t2p[1][1] = x2i;
        c_mul(x2r, x2i, x2r, x2i, &t2p[2][0], &t2p[2][1]);
        for (int k = 1; k <= 35; k += 4) {
            int idx = k % 3;
            double mr, mi;
            c_mul(conj1r, conj1i, t2p[idx][0], t2p[idx][1], &mr, &mi);
            cRe[k - 1] += mr - logval;
            cIm[k - 1] += mi;
        }
    }

    /* Third loop: r=2,5,8,...,34: cf[r-1] *= (sin(t1*r) + cos(t2/(r+1))) */
    for (int r = 2; r <= 34; r += 3) {
        double sr, si; c_sin(x1r * r, x1i * r, &sr, &si);
        double cr, ci; c_cos(x2r / (r + 1.0), x2i / (r + 1.0), &cr, &ci);
        double factor_r = sr + cr, factor_i = si + ci;
        double old_r = cRe[r - 1], old_i = cIm[r - 1];
        double nr, ni;
        c_mul(old_r, old_i, factor_r, factor_i, &nr, &ni);
        cRe[r - 1] = nr;
        cIm[r - 1] = ni;
    }

    /* cf[9] = sum([abs(t1),abs(t2)])*exp(1j*angle(t1+t2)) */
    {
        double s = abs1 + abs2;
        double a = c_arg(x1r + x2r, x1i + x2i);
        cRe[9] = s * cos(a);
        cIm[9] = s * sin(a);
    }
    /* cf[19] = prod([abs(t1),abs(t2)])/(1+abs(t1-t2)) */
    {
        double p = abs1 * abs2;
        double d = c_abs(x1r - x2r, x1i - x2i);
        cRe[19] = p / (1.0 + d);
        cIm[19] = 0.0;
    }
    /* cf[34] = real(t1)^3 - imag(t2)^2 + 2j*real(t2)*imag(t1) */
    cRe[34] = x1r*x1r*x1r - x2i*x2i;
    cIm[34] = 2.0*x2r*x1i;

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_248_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* Fixed coefficients */
    cRe[1] = 3.0; cRe[6] = -5.0; cRe[12] = 8.0;
    cRe[18] = -12.0; cRe[24] = 20.0; cRe[30] = -25.0;

    /* Main loop */
    for (int j = 1; j <= 35; j++) {
        double angle, magnitude;
        if (j % 4 == 1) {
            angle = ang1 * j + sin(j * ang2);
            magnitude = log(abs1 + abs2 + j) * (j % 3 + 1);
            cRe[j - 1] = magnitude * cos(angle);
            cIm[j - 1] = magnitude * sin(angle);
        } else if (j % 4 == 2) {
            angle = ang2 * j + cos(j * ang1);
            magnitude = abs1*abs1 + abs2*abs2 + j;
            cRe[j - 1] = magnitude * cos(angle);
            cIm[j - 1] = -magnitude * sin(angle);
        } else if (j % 4 == 3) {
            double ang_sum = c_arg(x1r + x2r, x1i + x2i);
            angle = sin(j * ang_sum);
            double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
            magnitude = log(c_abs(t1t2r, t1t2i) + 1.0) * (j + 2);
            /* magnitude * exp(1j*angle) */
            cRe[j - 1] = magnitude * cos(angle);
            cIm[j - 1] = magnitude * sin(angle);
        } else {
            double ang_diff = c_arg(x1r - x2r, x1i - x2i);
            angle = cos(j * ang_diff);
            magnitude = pow(abs1 + abs2, j) / (j + 1.0);
            cRe[j - 1] = magnitude;
            cIm[j - 1] = magnitude * angle;
        }
    }

    /* cf[k-1] += (real(t1)^k - imag(t2)^k)*1j for k=5,10,15,20,25,30,35 */
    for (int k = 5; k <= 35; k += 5) {
        double val = pow(x1r, k) - pow(x2i, k);
        cIm[k - 1] += val;
    }

    /* cf[r-1] *= (1 + 0.5j*real(t1+t2)) for r=10..15 */
    {
        double rsum = x1r + x2r;
        double fr = 1.0, fi = 0.5 * rsum;
        for (int r = 10; r <= 15; r++) {
            double old_r = cRe[r - 1], old_i = cIm[r - 1];
            double nr, ni;
            c_mul(old_r, old_i, fr, fi, &nr, &ni);
            cRe[r - 1] = nr;
            cIm[r - 1] = ni;
        }
    }

    /* cf[19] = prod(abs(cf[0:10]))^(1/5)*(sin(angle(t1))+cos(angle(t2))) */
    {
        double prod = 1.0;
        for (int i = 0; i < 10; i++) {
            prod *= c_abs(cRe[i], cIm[i]);
        }
        double val = pow(prod, 0.2) * (sin(ang1) + cos(ang2));
        cRe[19] = val;
        cIm[19] = 0.0;
    }

    /* cf[33] = conj(cf[33]) + t1^3 - t2^3 */
    {
        double t1sq_r, t1sq_i; c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
        double t1cu_r, t1cu_i; c_mul(t1sq_r, t1sq_i, x1r, x1i, &t1cu_r, &t1cu_i);
        double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
        double t2cu_r, t2cu_i; c_mul(t2sq_r, t2sq_i, x2r, x2i, &t2cu_r, &t2cu_i);
        double old_r = cRe[33], old_i = cIm[33];
        cRe[33] = old_r + t1cu_r - t2cu_r;
        cIm[33] = -old_i + t1cu_i - t2cu_i;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_249_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    for (int j = 1; j <= 35; j++) {
        int m = j % 5;
        if (m == 1) {
            cRe[j - 1] = pow(x1r, j) + pow(x2i, j % 3) * sin(j * ang1);
        } else if (m == 2) {
            /* conj(t1)*cos(j*angle(t2)) + abs(t2)^2/(j+1) */
            double cv = cos(j * ang2);
            cRe[j - 1] = x1r * cv + abs2*abs2 / (j + 1.0);
            cIm[j - 1] = -x1i * cv;
        } else if (m == 3) {
            cRe[j - 1] = log(abs1 + 1.0) + x1r*x1r - x2i*x2i;
            cIm[j - 1] = log(abs2 + 1.0);
        } else if (m == 4) {
            cRe[j - 1] = pow(x1r * x2i, j) + (abs1 + abs2) * sin(j);
        } else {
            /* sum([real(t1),imag(t2)])*cos(j*angle(t1)*angle(t2)) + 1j*prod([abs(t1),abs(t2)]) */
            double s = x1r + x2i;
            cRe[j - 1] = s * cos(j * ang1 * ang2);
            cIm[j - 1] = abs1 * abs2;
        }
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_250_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* Main loop */
    for (int j = 1; j <= 35; j++) {
        int m = j % 4;
        if (m == 1) {
            cRe[j - 1] = pow(x1r, j) + sin(j * ang1);
            cIm[j - 1] = pow(x1i, j) + cos(j * abs1);
        } else if (m == 2) {
            double logval = log(abs2 + 1.0);
            cRe[j - 1] = logval * (pow(x2r, j) - pow(x2i, j));
            cIm[j - 1] = pow(ang2, j) + pow(abs2, j);
        } else if (m == 3) {
            /* sin(t1*j)*cos(t2*j) + conj(t1)*conj(t2) */
            double sr, si; c_sin(x1r * j, x1i * j, &sr, &si);
            double cr, ci; c_cos(x2r * j, x2i * j, &cr, &ci);
            double mr, mi; c_mul(sr, si, cr, ci, &mr, &mi);
            /* conj(t1)*conj(t2) = conj(t1*t2) */
            double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
            cRe[j - 1] = mr + t1t2r;
            cIm[j - 1] = mi - t1t2i;
        } else {
            /* abs(t1+t2)^j + 1j*angle(t1-t2) */
            double abs_sum = c_abs(x1r + x2r, x1i + x2i);
            double ang_diff = c_arg(x1r - x2r, x1i - x2i);
            cRe[j - 1] = pow(abs_sum, j);
            cIm[j - 1] = ang_diff;
        }
    }

    /* cf[idx-1] *= (sin(k)+1j*cos(k)) for k=1..7, idx=k*5 */
    for (int k = 1; k <= 7; k++) {
        int idx = k * 5;
        if (idx <= 35) {
            double fr = sin(k), fi = cos(k);
            double old_r = cRe[idx - 1], old_i = cIm[idx - 1];
            double nr, ni;
            c_mul(old_r, old_i, fr, fi, &nr, &ni);
            cRe[idx - 1] = nr;
            cIm[idx - 1] = ni;
        }
    }

    /* cf[7] = sum(abs(cf[0:7])) + 1j*prod(abs(cf[0:7])) */
    {
        double s = 0.0, p = 1.0;
        for (int i = 0; i < 7; i++) {
            double a = c_abs(cRe[i], cIm[i]);
            s += a; p *= a;
        }
        cRe[7] = s; cIm[7] = p;
    }

    /* cf[15] = cos(t1+t2) + 1j*sin(t1-t2) */
    {
        double cr, ci; c_cos(x1r + x2r, x1i + x2i, &cr, &ci);
        double sr, si; c_sin(x1r - x2r, x1i - x2i, &sr, &si);
        cRe[15] = cr - si; cIm[15] = ci + sr;
    }

    /* cf[23] = log(abs(t1^2-t2^2)+1) + 1j*angle(t1*t2) */
    {
        double t1sq_r, t1sq_i; c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
        double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
        double diff_r = t1sq_r - t2sq_r, diff_i = t1sq_i - t2sq_i;
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[23] = log(c_abs(diff_r, diff_i) + 1.0);
        cIm[23] = c_arg(t1t2r, t1t2i);
    }

    /* cf[31] = conj(t1)^3 + conj(t2)^2 + sin(t1*t2) */
    {
        double c1r = x1r, c1i = -x1i;
        double c1sq_r, c1sq_i; c_mul(c1r, c1i, c1r, c1i, &c1sq_r, &c1sq_i);
        double c1cu_r, c1cu_i; c_mul(c1sq_r, c1sq_i, c1r, c1i, &c1cu_r, &c1cu_i);
        double c2r = x2r, c2i = -x2i;
        double c2sq_r, c2sq_i; c_mul(c2r, c2i, c2r, c2i, &c2sq_r, &c2sq_i);
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        double sr, si; c_sin(t1t2r, t1t2i, &sr, &si);
        cRe[31] = c1cu_r + c2sq_r + sr;
        cIm[31] = c1cu_i + c2sq_i + si;
    }

    /* cf[34] = real(t1)*real(t2) + imag(t1)*imag(t2) + 1j*(real(t1)-imag(t2)) */
    cRe[34] = x1r*x2r + x1i*x2i;
    cIm[34] = x1r - x2i;

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_251_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* Main loop: (abs(t1)^j + abs(t2)^(35-j))*exp(1j*angle)*sin(j*real(t1)-imag(t2)) */
    for (int j = 1; j <= 35; j++) {
        double angle = ang1 * j - ang2;
        double mag = (pow(abs1, j) + pow(abs2, 35 - j)) * sin(j * x1r - x2i);
        cRe[j - 1] = mag * cos(angle);
        cIm[j - 1] = mag * sin(angle);
    }

    /* cf[4] = conj(t1)*t2^2 - log(abs(t1)+1) + 2j*real(t2) */
    {
        double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
        double mr, mi; c_mul(x1r, -x1i, t2sq_r, t2sq_i, &mr, &mi);
        cRe[4] = mr - log(abs1 + 1.0);
        cIm[4] = mi + 2.0 * x2r;
    }

    /* cf[9] = sin(t1) + cos(t2)*conj(t1) */
    {
        double sr, si; c_sin(x1r, x1i, &sr, &si);
        double cr, ci; c_cos(x2r, x2i, &cr, &ci);
        double mr, mi; c_mul(cr, ci, x1r, -x1i, &mr, &mi);
        cRe[9] = sr + mr;
        cIm[9] = si + mi;
    }

    /* cf[14] = (t1*t2)^3 - real(t1)^2 + imag(t2)^3 */
    {
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        double sq_r, sq_i; c_mul(t1t2r, t1t2i, t1t2r, t1t2i, &sq_r, &sq_i);
        double cu_r, cu_i; c_mul(sq_r, sq_i, t1t2r, t1t2i, &cu_r, &cu_i);
        cRe[14] = cu_r - x1r * x1r + x2i * x2i * x2i;
        cIm[14] = cu_i;
    }

    /* cf[19] = exp(1j*angle(t1))*log(abs(t2)+1) + abs(t1+t2) */
    {
        double logval = log(abs2 + 1.0);
        double abs_sum = c_abs(x1r + x2r, x1i + x2i);
        cRe[19] = cos(ang1) * logval + abs_sum;
        cIm[19] = sin(ang1) * logval;
    }

    /* cf[24] = sin(t1+t2)*cos(t1-t2) + 1j*(real(t1)*imag(t2)) */
    {
        double sr, si; c_sin(x1r + x2r, x1i + x2i, &sr, &si);
        double cr, ci; c_cos(x1r - x2r, x1i - x2i, &cr, &ci);
        double mr, mi; c_mul(sr, si, cr, ci, &mr, &mi);
        cRe[24] = mr;
        cIm[24] = mi + x1r * x2i;
    }

    /* cf[29] = prod([real(t1),imag(t2),abs(t1+t2)]) + sum([real(t2),imag(t1)]) */
    {
        double abs_sum = c_abs(x1r + x2r, x1i + x2i);
        cRe[29] = x1r * x2i * abs_sum + x2r + x1i;
        cIm[29] = 0.0;
    }

    /* cf[34] = conj(t1)^2 + conj(t2)^3 - t1*t2 */
    {
        double c1sq_r, c1sq_i; c_mul(x1r, -x1i, x1r, -x1i, &c1sq_r, &c1sq_i);
        double c2sq_r, c2sq_i; c_mul(x2r, -x2i, x2r, -x2i, &c2sq_r, &c2sq_i);
        double c2cu_r, c2cu_i; c_mul(c2sq_r, c2sq_i, x2r, -x2i, &c2cu_r, &c2cu_i);
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[34] = c1sq_r + c2cu_r - t1t2r;
        cIm[34] = c1sq_i + c2cu_i - t1t2i;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_252_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    double sum_r = x1r + x2r, sum_i = x1i + x2i;

    for (int j = 1; j <= 35; j++) {
        int k = j % 6;
        int r = j / 6 + 1;
        if (k == 1) {
            double s = log(abs1 + 1.0) + sin(ang2);
            double pr, pi; c_powr(x1r, x1i, r, &pr, &pi);
            cRe[j - 1] = s * pr;
            cIm[j - 1] = s * pi;
        } else if (k == 2) {
            double s = cos(ang1) - sin(abs2);
            double pr, pi; c_powr(x2r, -x2i, r, &pr, &pi);
            cRe[j - 1] = s * pr;
            cIm[j - 1] = s * pi;
        } else if (k == 3) {
            double s = x1r * x2i + x2r * x1i;
            double pr, pi; c_powr(sum_r, sum_i, r, &pr, &pi);
            cRe[j - 1] = s * pr;
            cIm[j - 1] = s * pi;
        } else if (k == 4) {
            double diff = abs1*abs1 - abs2*abs2;
            double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
            double ang = c_arg(t1t2r, t1t2i);
            cRe[j - 1] = diff * cos(ang) * r;
            cIm[j - 1] = diff * sin(ang) * r;
        } else if (k == 5) {
            double sr, si; c_sin(x1r * r, x1i * r, &sr, &si);
            double cr, ci; c_cos(x2r / (double)r, x2i / (double)r, &cr, &ci);
            double fr = sr + cr, fi = si + ci;
            double dr = x1r - x2r, di = x1i - x2i;
            double dsq_r, dsq_i; c_mul(dr, di, dr, di, &dsq_r, &dsq_i);
            double mr, mi; c_mul(fr, fi, dsq_r, dsq_i, &mr, &mi);
            cRe[j - 1] = mr;
            cIm[j - 1] = mi;
        } else {
            double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
            double s = log(c_abs(t1t2r, t1t2i) + 1.0) + c_arg(sum_r, sum_i);
            double tcr = x1r + x2r, tci = x1i - x2i;
            double pr, pi; c_powr(tcr, tci, r, &pr, &pi);
            cRe[j - 1] = s * pr;
            cIm[j - 1] = s * pi;
        }
    }

    /* cf[4] = 100j*t1^4 - 50*t2^2 + 25j */
    {
        double pr, pi; c_powr(x1r, x1i, 4, &pr, &pi);
        double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
        cRe[4] = -100.0*pi - 50.0*t2sq_r;
        cIm[4] = 100.0*pr - 50.0*t2sq_i + 25.0;
    }
    /* cf[11] = 75*conj(t1) - 60j*t2 + 30 */
    cRe[11] = 75.0*x1r + 60.0*x2i + 30.0;
    cIm[11] = -75.0*x1i - 60.0*x2r;

    /* cf[18] = (t1^3+t2^3)/(real(t1)+real(t2)+1) */
    {
        double t1cu_r, t1cu_i; c_powr(x1r, x1i, 3, &t1cu_r, &t1cu_i);
        double t2cu_r, t2cu_i; c_powr(x2r, x2i, 3, &t2cu_r, &t2cu_i);
        double denom = x1r + x2r + 1.0;
        cRe[18] = (t1cu_r + t2cu_r) / denom;
        cIm[18] = (t1cu_i + t2cu_i) / denom;
    }

    /* cf[25] = sin(t1+t2)*cos(t1-t2)*1j */
    {
        double sr, si; c_sin(x1r + x2r, x1i + x2i, &sr, &si);
        double cr, ci; c_cos(x1r - x2r, x1i - x2i, &cr, &ci);
        double mr, mi; c_mul(sr, si, cr, ci, &mr, &mi);
        cRe[25] = -mi;
        cIm[25] = mr;
    }

    /* cf[32] = log(abs(t1+t2)+1)*(t1^2-t2^2) */
    {
        double logval = log(c_abs(x1r + x2r, x1i + x2i) + 1.0);
        double t1sq_r, t1sq_i; c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
        double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
        cRe[32] = logval * (t1sq_r - t2sq_r);
        cIm[32] = logval * (t1sq_i - t2sq_i);
    }

    /* cf[34] = real(t1*t2) + imag(t1-t2)*1j */
    {
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[34] = t1t2r;
        cIm[34] = x1i - x2i;
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_253_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);

    for (int j = 1; j <= 35; j++) {
        if (j % 4 == 0) {
            int k = j / 4;
            double angle = c_arg(x1r + x2r, x1i + x2i) * k;
            double base = pow(x1r, k) + pow(x2i, k);
            cRe[j - 1] = base * cos(angle);
            cIm[j - 1] = base * sin(angle);
        } else if (j % 5 == 0) {
            int r = j / 5;
            double logval = log(abs1 * r + 1.0);
            double pr, pi; c_powr(x2r, -x2i, r, &pr, &pi);
            cRe[j - 1] = logval + pr;
            cIm[j - 1] = pi;
        } else if (j % 3 == 1) {
            double sr, si; c_sin(x1r * j, x1i * j, &sr, &si);
            double cr, ci; c_cos(x2r * j, x2i * j, &cr, &ci);
            cRe[j - 1] = sr - ci;
            cIm[j - 1] = si + cr;
        } else {
            double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
            double dr, di; c_div(x1r, x1i, x2r, x2i, &dr, &di);
            cRe[j - 1] = t1t2r;
            cIm[j - 1] = di;
        }
    }

    /* cf[6] = prod([real(t1),imag(t2)]) + sum([abs(t1),abs(t2)])*1j */
    {
        double abs2 = c_abs(x2r, x2i);
        cRe[6] = x1r * x2i;
        cIm[6] = abs1 + abs2;
    }

    /* cf[13] = t1^3 + t2^2 - 5*t1*t2*1j */
    {
        double t1cu_r, t1cu_i; c_powr(x1r, x1i, 3, &t1cu_r, &t1cu_i);
        double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[13] = t1cu_r + t2sq_r + 5.0*t1t2i;
        cIm[13] = t1cu_i + t2sq_i - 5.0*t1t2r;
    }

    /* cf[20] = sin(t1+t2) + cos(t1-t2)*1j */
    {
        double sr, si; c_sin(x1r + x2r, x1i + x2i, &sr, &si);
        double cr, ci; c_cos(x1r - x2r, x1i - x2i, &cr, &ci);
        cRe[20] = sr - ci;
        cIm[20] = si + cr;
    }

    /* cf[27] = log(abs(t1)+1)*conj(t2) - sin(t1*t2) */
    {
        double logval = log(abs1 + 1.0);
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        double sr, si; c_sin(t1t2r, t1t2i, &sr, &si);
        cRe[27] = logval * x2r - sr;
        cIm[27] = -logval * x2i - si;
    }

    /* cf[34] = real(t1)^2 - imag(t2)^2 + 2*real(t1)*imag(t2)*1j */
    cRe[34] = x1r*x1r - x2i*x2i;
    cIm[34] = 2.0*x1r*x2i;

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_254_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    for (int j = 1; j <= 35; j++) {
        int k = j % 5 + 1;
        int r = j / 5 + 1;
        double angle = ang1 * sin(j) + ang2 * cos(j);
        double magnitude = pow(abs1, k) + pow(abs2, r) + log(abs1 + 1.0) * log(abs2 + 1.0);
        cRe[j - 1] = magnitude * cos(angle);
        cIm[j - 1] = magnitude * sin(angle);
    }

    /* cf[[2,7,13,21,28]] = conj(t1)*t2^2 - t1^2*conj(t2) */
    {
        double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
        double mr, mi; c_mul(x1r, -x1i, t2sq_r, t2sq_i, &mr, &mi);
        double t1sq_r, t1sq_i; c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
        double mr2, mi2; c_mul(t1sq_r, t1sq_i, x2r, -x2i, &mr2, &mi2);
        double vr = mr - mr2, vi = mi - mi2;
        int idx[] = {2, 7, 13, 21, 28};
        for (int i = 0; i < 5; i++) { cRe[idx[i]] = vr; cIm[idx[i]] = vi; }
    }

    /* cf[[4,10,18,26,34]] = sin(t1*t2) + cos(t1+t2)*1j */
    {
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        double sr, si; c_sin(t1t2r, t1t2i, &sr, &si);
        double cr, ci; c_cos(x1r + x2r, x1i + x2i, &cr, &ci);
        double vr = sr - ci, vi = si + cr;
        int idx[] = {4, 10, 18, 26, 34};
        for (int i = 0; i < 5; i++) { cRe[idx[i]] = vr; cIm[idx[i]] = vi; }
    }

    /* cf[16] = prod([abs(t1),abs(t2)])*exp(1j*(angle(t1)-angle(t2))) */
    {
        double p = abs1 * abs2;
        double a = ang1 - ang2;
        cRe[16] = p * cos(a);
        cIm[16] = p * sin(a);
    }

    /* cf[24] = sum([abs(t1+t2),real(t1)^2,imag(t2)^2])*(1+1j) */
    {
        double abs_sum = c_abs(x1r + x2r, x1i + x2i);
        double s = abs_sum + x1r*x1r + x2i*x2i;
        cRe[24] = s;
        cIm[24] = s;
    }

    /* cf[34] = log(abs(t1)+1) + log(abs(t2)+1)*1j (overwrite) */
    cRe[34] = log(abs1 + 1.0);
    cIm[34] = log(abs2 + 1.0);

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_255_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* Fixed coefficients at 1-based indices 3,8,14,19,26,34 -> 0-based 2,7,13,18,25,33 */
    cRe[2] = 2.0; cIm[2] = 3.0;
    cRe[7] = -4.0; cIm[7] = 1.0;
    cRe[13] = 5.0; cIm[13] = -2.0;
    cRe[18] = -3.0; cIm[18] = 4.0;
    cRe[25] = 1.5; cIm[25] = -0.5;
    cRe[33] = -2.2; cIm[33] = 2.0;

    /* Main loop: skip indices that were set as fixed */
    for (int j = 1; j <= 35; j++) {
        if (j == 3 || j == 8 || j == 14 || j == 19 || j == 26 || j == 34) continue;
        int k = j % 7 + 1;
        int r = j / 5 + 1;
        double magnitude = sin(j * ang1) * cos(k * abs2) + log(abs1 + 1.0) * r;
        double angle = ang2 * k - ang1 * r + sin(j * x1i);
        cRe[j - 1] = magnitude * cos(angle);
        cIm[j - 1] = magnitude * sin(angle);
    }

    /* Additional: k=1..5, idx=7*k (1-based) */
    for (int k = 1; k <= 5; k++) {
        int idx = 7 * k;
        if (idx <= 35) {
            double decay = exp(-(double)k / (abs1 + abs2 + 1.0));
            double c1r, c1i; c_powr(x1r, -x1i, k, &c1r, &c1i);
            double t2r, t2i; c_powr(x2r, x2i, k, &t2r, &t2i);
            double sr, si; c_sin(x1r * k, x1i * k, &sr, &si);
            double cr, ci; c_cos(x2r * k, x2i * k, &cr, &ci);
            /* (conj(t1)^k + t2^k)*decay + (sin(t1*k)+cos(t2*k))*1j */
            cRe[idx - 1] = (c1r + t2r) * decay - si - ci;
            cIm[idx - 1] = (c1i + t2i) * decay + sr + cr;
        }
    }

    /* Additional: r=1..3, start=10*r, j=start..start+3 */
    for (int r = 1; r <= 3; r++) {
        int start = 10 * r;
        double pr, pi; c_powr(x1r + x2r, x1i + x2i, r, &pr, &pi);
        double sq = (x1r - x2i) * (x1r - x2i);
        for (int j = start; j <= start + 3; j++) {
            if (j <= 35) {
                double sj = sin(j);
                cRe[j - 1] = pr * sj;
                cIm[j - 1] = pi * sj + sq;
            }
        }
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_256_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* First loop */
    for (int j = 1; j <= 35; j++) {
        double abs_t1j = c_abs(x1r + j, x1i);
        double mag_part = log(abs_t1j + 1.0) * pow(abs2, j % 5 + 1);
        double angle_part = ang1 * sin(j) - ang2 * cos(j);
        cRe[j - 1] = mag_part * cos(angle_part);
        cIm[j - 1] = mag_part * sin(angle_part);
    }

    /* Second loop */
    for (int k = 1; k <= 35; k++) {
        if (k % 4 == 0) {
            double c1r, c1i; c_powr(x1r, -x1i, k, &c1r, &c1i);
            double sr, si; c_sin(x2r * k, x2i * k, &sr, &si);
            double mr, mi; c_mul(c1r, c1i, sr, si, &mr, &mi);
            cRe[k - 1] += mr;
            cIm[k - 1] += mi;
        } else if (k % 3 == 0) {
            double factor = x1r + x2i * log(k + 1.0);
            cRe[k - 1] *= factor;
            cIm[k - 1] *= factor;
        } else {
            double val = abs1 * abs2 / (k + 1.0);
            cRe[k - 1] += val;
        }
    }

    /* Third loop: r=1..7, idx=r*5 */
    for (int r = 1; r <= 7; r++) {
        int idx = r * 5;
        if (idx <= 35) {
            double t2r_r, t2r_i; c_powr(x2r, x2i, r, &t2r_r, &t2r_i);
            double t1r_r, t1r_i; c_powr(x1r, x1i, r, &t1r_r, &t1r_i);
            cRe[idx - 1] += -100.0*t2r_i - 50.0*t1r_r;
            cIm[idx - 1] += 100.0*t2r_r - 50.0*t1r_i;
        }
    }

    /* cf[9] = sum(abs(cf[0:9]))*sin(real(t1)) - cos(imag(t2)) */
    {
        double s = 0.0;
        for (int i = 0; i < 9; i++) s += c_abs(cRe[i], cIm[i]);
        cRe[9] = s * sin(x1r) - cos(x2i);
        cIm[9] = 0.0;
    }

    /* cf[19] = prod(abs(cf[14:19]+1))/(1+abs(t1*t2)) */
    {
        double p = 1.0;
        for (int i = 14; i < 19; i++) {
            p *= c_abs(cRe[i] + 1.0, cIm[i]);
        }
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[19] = p / (1.0 + c_abs(t1t2r, t1t2i));
        cIm[19] = 0.0;
    }

    /* cf[29] = conj(t1) + sin(t2)*log(abs(t1)+1) */
    {
        double sr, si; c_sin(x2r, x2i, &sr, &si);
        double logval = log(abs1 + 1.0);
        cRe[29] = x1r + sr * logval;
        cIm[29] = -x1i + si * logval;
    }

    /* cf[34] = real(t1)^2 - imag(t2)^2 + 1j*(real(t2)*imag(t1)) */
    cRe[34] = x1r*x1r - x2i*x2i;
    cIm[34] = x2r*x1i;

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_257_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* Main loop */
    for (int j = 1; j <= 35; j++) {
        int k = j % 5 + 1;
        double r = ((double)j*j + sin(x1r * j) - cos(x2i * k)) / (log(abs1 + 1.0) + 1.0);
        double angle = pow(ang1, k) + pow(ang2, j % 3);
        double mag = r * (pow(x1r, k) + pow(x2i, j));
        cRe[j - 1] = mag * cos(angle);
        cIm[j - 1] = mag * sin(angle);
    }

    /* Second loop: k=1..5, index=5*k */
    for (int k = 1; k <= 5; k++) {
        double r = (x1r + k) - (x2i * k);
        double angle = c_arg(x1r + k, x1i) - c_arg(x2r + k, x2i);
        int index = 5 * k;
        if (index <= 35) {
            double sr, si; c_sin(x1r * k, x1i * k, &sr, &si);
            double cr, ci; c_cos(x2r * k, x2i * k, &cr, &ci);
            double fr = sr + cr, fi = si + ci;
            double er = r * cos(angle), ei = r * sin(angle);
            double mr, mi; c_mul(er, ei, fr, fi, &mr, &mi);
            cRe[index - 1] = mr;
            cIm[index - 1] = mi;
        }
    }

    /* cf[6] = conj(t1)*t2^2 + sin(t1+t2) */
    {
        double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
        double mr, mi; c_mul(x1r, -x1i, t2sq_r, t2sq_i, &mr, &mi);
        double sr, si; c_sin(x1r + x2r, x1i + x2i, &sr, &si);
        cRe[6] = mr + sr;
        cIm[6] = mi + si;
    }

    /* cf[13] = log(abs(t1)+1)*cos(t2) - 1j*sin(t1*t2) */
    {
        double logval = log(abs1 + 1.0);
        double cr, ci; c_cos(x2r, x2i, &cr, &ci);
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        double sr, si; c_sin(t1t2r, t1t2i, &sr, &si);
        cRe[13] = logval * cr + si;
        cIm[13] = logval * ci - sr;
    }

    /* cf[20] = abs(t1)^3 - abs(t2)^2 + 1j*angle(t1*t2) */
    {
        double abs2v = c_abs(x2r, x2i);
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[20] = abs1*abs1*abs1 - abs2v*abs2v;
        cIm[20] = c_arg(t1t2r, t1t2i);
    }

    /* cf[27] = real(t1^2) + imag(t2^3) - 2j*real(t1*t2) */
    {
        double t1sq_r, t1sq_i; c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
        double t2cu_r, t2cu_i; c_powr(x2r, x2i, 3, &t2cu_r, &t2cu_i);
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[27] = t1sq_r + t2cu_i;
        cIm[27] = -2.0 * t1t2r;
    }

    /* cf[34] = prod([real(t1),real(t2)]) + sum([imag(t1),imag(t2)])*1j */
    cRe[34] = x1r * x2r;
    cIm[34] = x1i + x2i;

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_258_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    /* Main loop */
    for (int j = 1; j <= 35; j++) {
        double angle = sin(j * ang1 + cos(j * ang2)) + x1r * x2i;
        double magnitude = log(abs1 + abs2 + j) + pow(x1r, (j % 4) + 1) - pow(x2i, (j % 3) + 1) + x1r * x2i * j;
        cRe[j - 1] = magnitude * cos(angle);
        cIm[j - 1] = magnitude * sin(angle);
    }

    /* cf[2] = conj(t1)*t2^2 + sin(t1*t2)*cos(t1-t2) */
    {
        double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
        double mr, mi; c_mul(x1r, -x1i, t2sq_r, t2sq_i, &mr, &mi);
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        double sr, si; c_sin(t1t2r, t1t2i, &sr, &si);
        double cr, ci; c_cos(x1r - x2r, x1i - x2i, &cr, &ci);
        double pr, pi; c_mul(sr, si, cr, ci, &pr, &pi);
        cRe[2] = mr + pr;
        cIm[2] = mi + pi;
    }

    /* cf[7] = real(t1^2+t2^2) + 1j*imag(t1*t2) */
    {
        double t1sq_r, t1sq_i; c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
        double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[7] = t1sq_r + t2sq_r;
        cIm[7] = t1t2i;
    }

    /* cf[14] = log(abs(t1+t2)+1) + 1j*angle(t1-t2) */
    {
        double abs_sum = c_abs(x1r + x2r, x1i + x2i);
        double ang_diff = c_arg(x1r - x2r, x1i - x2i);
        cRe[14] = log(abs_sum + 1.0);
        cIm[14] = ang_diff;
    }

    /* cf[21] = sin(t1)^3 - cos(t2)^3 + 1j*(sin(t1)*cos(t2)) */
    {
        double s1r, s1i; c_sin(x1r, x1i, &s1r, &s1i);
        double c2r, c2i; c_cos(x2r, x2i, &c2r, &c2i);
        double s1sq_r, s1sq_i; c_mul(s1r, s1i, s1r, s1i, &s1sq_r, &s1sq_i);
        double s1cu_r, s1cu_i; c_mul(s1sq_r, s1sq_i, s1r, s1i, &s1cu_r, &s1cu_i);
        double c2sq_r, c2sq_i; c_mul(c2r, c2i, c2r, c2i, &c2sq_r, &c2sq_i);
        double c2cu_r, c2cu_i; c_mul(c2sq_r, c2sq_i, c2r, c2i, &c2cu_r, &c2cu_i);
        double pr, pi; c_mul(s1r, s1i, c2r, c2i, &pr, &pi);
        cRe[21] = s1cu_r - c2cu_r - pi;
        cIm[21] = s1cu_i - c2cu_i + pr;
    }

    /* cf[28] = real(t1*t2) + imag(t1+t2)*1j */
    {
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        cRe[28] = t1t2r;
        cIm[28] = x1i + x2i;
    }

    /* cf[34] = prod([abs(t1),abs(t2),j]) + 1j*sum([real(t1),imag(t2)])
       Note: j=35 (last loop value) */
    cRe[34] = abs1 * abs2 * 35.0;
    cIm[34] = x1r + x2i;

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_259_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    double logval = log(abs1 + abs2 + 1.0);
    for (int j = 1; j <= 35; j++) {
        double ang_sum = c_arg(x1r + x2r, x1i + x2i);
        double sv = sin(j * ang2);
        double cterm_r = x1r * sv, cterm_i = -x1i * sv;
        double costerm = logval * cos(j * ang_sum);
        cRe[j - 1] = pow(x1r, j) * pow(x2i, 35 - j) + cterm_r + costerm;
        cIm[j - 1] = cterm_i;
    }

    for (int k = 1; k <= 5; k++) {
        int r = k + 5;
        double ang_diff = ang1 - ang2;
        double mag = pow(abs1, k) * pow(abs2, 5 - k);
        cRe[r - 1] += mag * cos(ang_diff);
        cIm[r - 1] += mag * sin(ang_diff);
    }

    for (int m = 6; m <= 10; m++) {
        cRe[m - 1] += sin(x1r * m) + cos(x2i * m);
    }

    /* cf[11] = real(t1*t2) + 1j*imag(t1/t2) */
    {
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        double dr, di; c_div(x1r, x1i, x2r, x2i, &dr, &di);
        cRe[11] = t1t2r;
        cIm[11] = di;
    }

    /* cf[19] = log(abs(t1+t2)) + 1j*angle(t1-t2) */
    {
        double abs_sum = c_abs(x1r + x2r, x1i + x2i);
        double ang_diff = c_arg(x1r - x2r, x1i - x2i);
        cRe[19] = log(abs_sum);
        cIm[19] = ang_diff;
    }

    /* cf[24] = conj(t1)^2 - conj(t2)^3 + sin(t1*t2) */
    {
        double c1sq_r, c1sq_i; c_mul(x1r, -x1i, x1r, -x1i, &c1sq_r, &c1sq_i);
        double c2sq_r, c2sq_i; c_mul(x2r, -x2i, x2r, -x2i, &c2sq_r, &c2sq_i);
        double c2cu_r, c2cu_i; c_mul(c2sq_r, c2sq_i, x2r, -x2i, &c2cu_r, &c2cu_i);
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        double sr, si; c_sin(t1t2r, t1t2i, &sr, &si);
        cRe[24] = c1sq_r - c2cu_r + sr;
        cIm[24] = c1sq_i - c2cu_i + si;
    }

    /* cf[29] = abs(t1)^3*abs(t2)^2 + cos(angle(t1)*angle(t2)) */
    cRe[29] = abs1*abs1*abs1 * abs2*abs2 + cos(ang1 * ang2);
    cIm[29] = 0.0;

    /* cf[34] = prod([abs(t1),abs(t2)]) + sum([real(t1),imag(t2)])*1j */
    cRe[34] = abs1 * abs2;
    cIm[34] = x1r + x2i;

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}


static void poly_260_hand(double x1r, double x1i, double x2r, double x2i,
                          double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);
    double ang1 = c_arg(x1r, x1i);
    double ang2 = c_arg(x2r, x2i);

    for (int j = 1; j <= 35; j++) {
        int k = j % 7 + 1;
        int r = j / 7 + 1;
        cRe[j - 1] = (pow(x1r, k) - pow(x2i, r)) * cos(ang1 * j) + sin(ang2 * r) / (abs1 + abs2 + j);
    }

    /* cf[3] = conj(t1)*t2^2 + log(abs(t1)+1)*sin(t2) */
    {
        double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
        double mr, mi; c_mul(x1r, -x1i, t2sq_r, t2sq_i, &mr, &mi);
        double logval = log(abs1 + 1.0);
        double sr, si; c_sin(x2r, x2i, &sr, &si);
        cRe[3] = mr + logval * sr;
        cIm[3] = mi + logval * si;
    }

    /* cf[7] = real(t1*t2) + imag(t1)^2 - cos(t2) */
    {
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        double cr, ci; c_cos(x2r, x2i, &cr, &ci);
        cRe[7] = t1t2r + x1i*x1i - cr;
        cIm[7] = -ci;
    }

    /* cf[12] = abs(t1+t2)^2 - real(t1)^3 + imag(t2) */
    {
        double abs_sum = c_abs(x1r + x2r, x1i + x2i);
        cRe[12] = abs_sum*abs_sum - x1r*x1r*x1r + x2i;
    }

    /* cf[16] = sin(t1)*cos(t2) + real(t2)^2 - imag(t1)^2 */
    {
        double sr, si; c_sin(x1r, x1i, &sr, &si);
        double cr, ci; c_cos(x2r, x2i, &cr, &ci);
        double mr, mi; c_mul(sr, si, cr, ci, &mr, &mi);
        cRe[16] = mr + x2r*x2r - x1i*x1i;
        cIm[16] = mi;
    }

    /* cf[21] = log(abs(t1*t2)+1) + conj(t1) - conj(t2) */
    {
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        double logval = log(c_abs(t1t2r, t1t2i) + 1.0);
        cRe[21] = logval + x1r - x2r;
        cIm[21] = -x1i + x2i;
    }

    /* cf[25] = real(t1)^2*imag(t2) - real(t2)*imag(t1) + sin(angle(t1+t2)) */
    {
        double ang_sum = c_arg(x1r + x2r, x1i + x2i);
        cRe[25] = x1r*x1r*x2i - x2r*x1i + sin(ang_sum);
    }

    /* cf[30] = (real(t1)+imag(t1))*(real(t2)-imag(t2)) + cos(angle(t1*t2)) */
    {
        double t1t2r, t1t2i; c_mul(x1r, x1i, x2r, x2i, &t1t2r, &t1t2i);
        double ang = c_arg(t1t2r, t1t2i);
        cRe[30] = (x1r + x1i) * (x2r - x2i) + cos(ang);
    }

    /* cf[33] = real(t1)^3 - imag(t1)^3 + real(t2)^3 - imag(t2)^3 */
    cRe[33] = x1r*x1r*x1r - x1i*x1i*x1i + x2r*x2r*x2r - x2i*x2i*x2i;

    /* cf[34] = sum([real(t1),real(t2),imag(t1),imag(t2)]) + prod([abs(t1),abs(t2)]) */
    cRe[34] = x1r + x2r + x1i + x2i + abs1 * abs2;

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}
