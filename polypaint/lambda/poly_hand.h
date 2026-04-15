/*
 * Hand-written polynomial coefficient functions.
 *
 * These replace broken auto-transpiled versions from transpile_poly.py.
 * The transpiler silently emits zeros for:
 *   - np.array([...]) with computed elements in slice assignments
 *   - List comprehensions in slice assignments: cf[a:b] = [expr for ...]
 *
 * Each function has the same signature as the transpiled versions:
 *   void func(double x1r, double x1i, double x2r, double x2i,
 *             double *cRe, double *cIm, int *nCoeffs)
 *
 * Requires: c_mul, c_abs, c_arg, c_exp2 helpers from sweep_cli.c
 *
 * Forward declaration: poly_42_c from poly_generated_funcs.h (used by serendipity wrapper)
 */
#include <complex.h>
#include <stdint.h>
#undef I

static void poly_42_c(double x1r, double x1i, double x2r, double x2i,
                      const double *cfpv, int n_cfpv,
                      double *cRe, double *cIm, int *nCoeffs);

/* ---- poly_29_hand ----
 * Python:
 *   cf[0:5]  = [1, t1, t1^2, t1^3, t1^4]
 *   cf[5:10] = [1, t2, t2^2, t2^3, t2^4]
 *   cf[10:15]= [1, exp(i*t1), exp(2i*t1), exp(3i*t1), exp(4i*t1)]
 *   cf[15:20]= [1, exp(i*t2), exp(2i*t2), exp(3i*t2), exp(4i*t2)]
 *   cf[20:30]= [1, Re(t1+t2), Im(t1+t2), Re(t1*t2), Im(t1*t2),
 *               Re(t1+t2)^2, Im(t1+t2)^2, Re(t1*t2)^2, Im(t1*t2)^2, |t1+t2|]
 *   cf[30:40]= arange(1,11) * |t1| * |t2|
 *   cf[40:50]= [1, log(|t1|+1), log(|t2|+1), log(|t1+t2|+1), log(|t1*t2|+1),
 *               angle(t1), angle(t2), |t1|, |t2|, angle(t1+t2)]
 *   cf[50]   = |t1+t2| * angle(t1*t2)
 */
static void poly_29_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* cf[0:5] = [1, t1, t1^2, t1^3, t1^4] */
    cRe[0] = 1; cIm[0] = 0;
    cRe[1] = x1r; cIm[1] = x1i;
    double t1_2r, t1_2i; c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
    cRe[2] = t1_2r; cIm[2] = t1_2i;
    double t1_3r, t1_3i; c_mul(t1_2r, t1_2i, x1r, x1i, &t1_3r, &t1_3i);
    cRe[3] = t1_3r; cIm[3] = t1_3i;
    double t1_4r, t1_4i; c_mul(t1_3r, t1_3i, x1r, x1i, &t1_4r, &t1_4i);
    cRe[4] = t1_4r; cIm[4] = t1_4i;

    /* cf[5:10] = [1, t2, t2^2, t2^3, t2^4] */
    cRe[5] = 1; cIm[5] = 0;
    cRe[6] = x2r; cIm[6] = x2i;
    double t2_2r, t2_2i; c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
    cRe[7] = t2_2r; cIm[7] = t2_2i;
    double t2_3r, t2_3i; c_mul(t2_2r, t2_2i, x2r, x2i, &t2_3r, &t2_3i);
    cRe[8] = t2_3r; cIm[8] = t2_3i;
    double t2_4r, t2_4i; c_mul(t2_3r, t2_3i, x2r, x2i, &t2_4r, &t2_4i);
    cRe[9] = t2_4r; cIm[9] = t2_4i;

    /* cf[10:15] = [1, exp(i*t1), exp(2i*t1), exp(3i*t1), exp(4i*t1)] */
    cRe[10] = 1; cIm[10] = 0;
    for (int k = 1; k <= 4; k++) {
        double er, ei;
        c_exp2(-k * x1i, k * x1r, &er, &ei);
        cRe[10 + k] = er; cIm[10 + k] = ei;
    }

    /* cf[15:20] = [1, exp(i*t2), exp(2i*t2), exp(3i*t2), exp(4i*t2)] */
    cRe[15] = 1; cIm[15] = 0;
    for (int k = 1; k <= 4; k++) {
        double er, ei;
        c_exp2(-k * x2i, k * x2r, &er, &ei);
        cRe[15 + k] = er; cIm[15 + k] = ei;
    }

    /* cf[20:30] */
    double sumR = x1r + x2r, sumI = x1i + x2i;
    double mulR, mulI; c_mul(x1r, x1i, x2r, x2i, &mulR, &mulI);
    cRe[20] = 1;            cIm[20] = 0;
    cRe[21] = sumR;         cIm[21] = 0;
    cRe[22] = sumI;         cIm[22] = 0;
    cRe[23] = mulR;         cIm[23] = 0;
    cRe[24] = mulI;         cIm[24] = 0;
    cRe[25] = sumR * sumR;  cIm[25] = 0;
    cRe[26] = sumI * sumI;  cIm[26] = 0;
    cRe[27] = mulR * mulR;  cIm[27] = 0;
    cRe[28] = mulI * mulI;  cIm[28] = 0;
    cRe[29] = c_abs(sumR, sumI); cIm[29] = 0;

    /* cf[30:40] = arange(1,11) * |t1| * |t2| */
    double absT1 = c_abs(x1r, x1i);
    double absT2 = c_abs(x2r, x2i);
    double ab12 = absT1 * absT2;
    for (int k = 0; k < 10; k++) {
        cRe[30 + k] = (k + 1) * ab12;
        cIm[30 + k] = 0;
    }

    /* cf[40:50] */
    double absSum = c_abs(sumR, sumI);
    double absMul = c_abs(mulR, mulI);
    cRe[40] = 1;                         cIm[40] = 0;
    cRe[41] = log(absT1 + 1);            cIm[41] = 0;
    cRe[42] = log(absT2 + 1);            cIm[42] = 0;
    cRe[43] = log(absSum + 1);           cIm[43] = 0;
    cRe[44] = log(absMul + 1);           cIm[44] = 0;
    cRe[45] = c_arg(x1r, x1i);           cIm[45] = 0;
    cRe[46] = c_arg(x2r, x2i);           cIm[46] = 0;
    cRe[47] = absT1;                      cIm[47] = 0;
    cRe[48] = absT2;                      cIm[48] = 0;
    cRe[49] = c_arg(sumR, sumI);          cIm[49] = 0;

    /* cf[50] = |t1+t2| * angle(t1*t2) */
    cRe[50] = absSum * c_arg(mulR, mulI); cIm[50] = 0;

    /* NaN guard */
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static double poly_hand_prod_range_inclusive(int start, int end) {
    double prod = 1.0;
    if (start <= end) {
        for (int k = start; k <= end; k++) prod *= (double)k;
    } else {
        for (int k = start; k >= end; k--) prod *= (double)k;
    }
    return prod;
}

static double poly_hand_prod_1_to_n(int n) {
    double prod = 1.0;
    for (int k = 1; k <= n; k++) prod *= (double)k;
    return prod;
}

static double poly_hand_np_int64_prod_1_to_n(int n) {
    union {
        uint64_t u;
        int64_t s;
    } bits;
    bits.u = 1u;
    for (int k = 1; k <= n; k++) {
        bits.u *= (uint64_t)(int64_t)k;
    }
    return (double)bits.s;
}

static double complex poly_hand_cpow_uint(double complex z, int exponent) {
    if (exponent <= 0) return 1.0;
    double complex out = 1.0;
    for (int i = 0; i < exponent; i++) out *= z;
    return out;
}

static void poly_hand_zero(double *cRe, double *cIm, int n) {
    for (int i = 0; i < n; i++) { cRe[i] = 0.0; cIm[i] = 0.0; }
}

static void poly_hand_from_polar(double magnitude, double angle, double *out_r, double *out_i) {
    *out_r = magnitude * cos(angle);
    *out_i = magnitude * sin(angle);
}

static inline double complex poly_hand_z(double r, double i) {
    return CMPLX(r, i);
}

static inline void poly_hand_store_z(double complex z, double *cRe, double *cIm, int idx) {
    cRe[idx] = creal(z);
    cIm[idx] = cimag(z);
}

static void poly_hand_pow_int(double zr, double zi, int exp, double *out_r, double *out_i) {
    double rr = 1.0, ri = 0.0;
    if (exp == 0) {
        *out_r = 1.0;
        *out_i = 0.0;
        return;
    }
    for (int i = 0; i < exp; i++) {
        double nr, ni;
        c_mul(rr, ri, zr, zi, &nr, &ni);
        rr = nr;
        ri = ni;
    }
    *out_r = rr;
    *out_i = ri;
}

static void poly_hand_pow_signed_int(double zr, double zi, int exp, double *out_r, double *out_i) {
    if (exp >= 0) {
        poly_hand_pow_int(zr, zi, exp, out_r, out_i);
        return;
    }
    double pr, pi;
    poly_hand_pow_int(zr, zi, -exp, &pr, &pi);
    c_div(1.0, 0.0, pr, pi, out_r, out_i);
}

static void poly_hand_pow_complex(double ar, double ai, double br, double bi, double *out_r, double *out_i) {
    double lr, li;
    c_log(ar, ai, &lr, &li);
    double exp_r, exp_i;
    c_mul(br, bi, lr, li, &exp_r, &exp_i);
    c_exp2(exp_r, exp_i, out_r, out_i);
}

static void poly_13_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    static const double fib[] = {
        1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181
    };
    *nCoeffs = 51;
    poly_hand_zero(cRe, cIm, 51);

    const double arg1 = c_arg(x1r, x1i);
    const double arg2 = c_arg(x2r, x2i);
    const double cos2 = cos(arg2);
    const double sin2 = sin(arg2);
    const double sin1 = sin(arg1);
    for (int n = 0; n < 19; n++) {
        const double scale_a = fib[n] * cos2;
        const double scale_b = fib[n] * sin2;
        cRe[n] = scale_a * x1r;
        cIm[n] = scale_a * x1i;
        cRe[n + 19] = scale_b * x1r;
        cIm[n + 19] = scale_b * x1i;
        if (n + 38 < 51) {
            const double scale_c = fib[n] * sin1;
            cRe[n + 38] = scale_c * x2r;
            cIm[n + 38] = scale_c * x2i;
        }
    }
    double prod_r, prod_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    cRe[19] = c_abs(prod_r, prod_i);
    cIm[19] = 0.0;
    cRe[49] = log(c_abs(prod_r, prod_i) + 1.0);
    cIm[49] = 0.0;
    cRe[50] = x1r + x2i;
    cIm[50] = 0.0;
}

static void poly_14_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 51;
    poly_hand_zero(cRe, cIm, 51);

    const double log1 = log(c_abs(x1r, x1i) + 1.0);
    const double log2 = log(c_abs(x2r, x2i) + 1.0);
    double base1r = x1r, base1i = x1i;
    double tmp_r = x2r * log1, tmp_i = x2i * log1;
    base1r += tmp_r; base1i += tmp_i;
    double base2r = x2r, base2i = x2i;
    tmp_r = x1r * log2; tmp_i = x1i * log2;
    base2r += tmp_r; base2i += tmp_i;

    cRe[0] = x1r + 3.0 * x2r;
    cIm[0] = x1i + 3.0 * x2i;
    for (int k = 1; k <= 50; k++) {
        const double triangle = (double)(k * (k + 1)) / 2.0;
        double p1r, p1i, p2r, p2i;
        poly_hand_pow_int(base1r, base1i, k, &p1r, &p1i);
        poly_hand_pow_int(base2r, base2i, k, &p2r, &p2i);
        cRe[k] = triangle * (p1r + p2r);
        cIm[k] = triangle * (p1i + p2i);
    }
    cRe[42] = c_abs(x1r, x1i);
    cIm[42] = 0.0;
    cRe[20] = c_abs(x2r, x2i);
    cIm[20] = 0.0;
    double prod_r, prod_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    cRe[31] = c_abs(prod_r, prod_i) - prod_i;
    cIm[31] = 0.0;
    cRe[27] = 2.0 * ((x1r - x2r) + (x1i - x2i));
    cIm[27] = 0.0;
}

static void poly_15_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    static const double primes[] = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73,
        79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163,
        167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241
    };
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);

    const int n_primes = (int)(sizeof(primes) / sizeof(primes[0]));
    const double denom_base = 1.0 + c_abs(x1r, x1i);
    double sum_r = 0.0, sum_i = 0.0;
    for (int i = 0; i < 71; i++) {
        double p_r, p_i;
        poly_hand_pow_int(x2r, x2i, i, &p_r, &p_i);
        const double rotated_r = -p_i;
        const double rotated_i = p_r;
        const double denom = pow(denom_base, (double)i);
        cRe[i] = (primes[i % n_primes] * x1r + rotated_r) / denom;
        cIm[i] = (primes[i % n_primes] * x1i + rotated_i) / denom;
        if (i < 70) {
            sum_r += cRe[i];
            sum_i += cIm[i];
        }
    }
    cRe[70] = sum_r;
    cIm[70] = sum_i;
}

static void poly_19_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[71] = {0};

    cf[0] = creal(t1) + cimag(t2);
    cf[1] = carg(t1);
    cf[2] = cabs(t2);
    cf[3] = csin(t1) + ccos(t2);
    for (int i = 0; i < 6; i++) cf[4 + i] = 1.0 + 0.2 * (double)(i + 1);
    cf[10] = log(cabs(t1) + 1.0) + log(cabs(t2) + 1.0);
    for (int i = 11; i < 72; i++) {
        cf[i - 1] = cf[i - 2] * csin((double)i * cf[i - 3] + cabs(cf[i - 4])) + cf[i - 5];
    }
    for (int i = 0; i < 71; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_22_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    static const double primes[] = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97
    };
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);

    double t1sq_r, t1sq_i, t2sq_r, t2sq_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    const double scalar = x1r * x1r - t1sq_i + x2r * x2r - t2sq_i;
    const double factor = cos(c_arg(x1r + x2r, x1i + x2i)) + sin(c_abs(x1r, x1i) * c_abs(x2r, x2i));
    double sum = 0.0;
    for (int i = 0; i < 25; i++) {
        cRe[i] = primes[i] * scalar;
        cRe[25 + i] = cRe[i] * factor;
        sum += cRe[i] + cRe[25 + i];
    }
    cRe[50] = sum;
}

static void poly_25_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);

    if (c_abs(x2r, x2i) == 0.0) return;

    double prod_r, prod_i, div_r, div_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    c_div(x1r, x1i, x2r, x2i, &div_r, &div_i);
    cRe[0] = prod_r + div_i;
    for (int k = 1; k < 71; k++) {
        double s_r, s_i, c_r, c_i;
        c_sin(x1r + (double)k, x1i, &s_r, &s_i);
        c_cos(x2r + (double)k, x2i, &c_r, &c_i);
        cRe[k] = pow(c_abs(x1r, x1i), (double)k) + pow(c_arg(x2r, x2i), (double)k) + s_r + c_r - log(pow(c_abs(prod_r, prod_i), (double)k) + 1.0);
        cIm[k] = s_i + c_i;
    }
    double tmp_r, tmp_i;
    c_mul(cRe[0], cIm[0], cRe[34], cIm[34], &tmp_r, &tmp_i);
    cRe[35] = tmp_r + prod_i;
    cIm[35] = 0.0;
    cRe[45] = 0.5 * (x1r + cRe[44] + x2r);
    cIm[45] = 0.5 * (x1i - cIm[44] + x2i);
    cRe[50] = cRe[0] + cRe[34] + cRe[44] + x1r + x2i;
    cIm[50] = cIm[0] + cIm[34] + cIm[44];
}

static void poly_26_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[71] = {0};

    cf[0] = t1 + t2;
    cf[1] = t1 * t2;
    for (int k = 3; k < 72; k++) {
        double complex v = csin((double)k * cf[k - 2]) + ccos((double)k * cf[k - 3]);
        cf[k - 1] = v / cabs(v);
    }
    cf[14] = cabs(t1 - t2) * carg(t1 + t2);
    cf[29] = log(cabs(t1 * creal(t2) + 1.0)) - log(cabs(t2 * cimag(t1) + 1.0));
    cf[49] = cabs(t1) * cabs(t2) * cabs(t1 - t2);
    double complex sum1 = 0.0, sum2 = 0.0;
    for (int i = 15; i < 29; i++) sum1 += cf[i];
    for (int i = 30; i < 44; i++) sum2 += cf[i];
    cf[50] = sum1 * sum2 + t1 * t1;
    for (int i = 0; i < 71; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_31_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);

    cRe[0] = x1r + x2r;
    cIm[0] = x1i + x2i;
    const double denom_base = c_abs(x1r, x1i) * c_abs(x2r, x2i);
    for (int i = 1; i < 36; i++) {
        double c_r, c_i, s_r, s_i;
        c_cos((double)i * x1r, (double)i * x1i, &c_r, &c_i);
        c_sin((double)i * x2r, (double)i * x2i, &s_r, &s_i);
        const double denom = pow(denom_base, (double)i);
        cRe[i] = (c_r + s_r) / denom;
        cIm[i] = (c_i + s_i) / denom;
    }
    for (int i = 36; i < 71; i++) {
        double p1r, p1i, p2r, p2i, c_r, c_i, s_r, s_i;
        poly_hand_pow_int(x1r, x1i, i, &p1r, &p1i);
        poly_hand_pow_int(x2r, x2i, i, &p2r, &p2i);
        c_cos(p1r, p1i, &c_r, &c_i);
        c_sin(p2r, p2i, &s_r, &s_i);
        const double scale = log(pow(c_abs(x1r, x1i), (double)i) + 1.0) * log(pow(c_abs(x2r, x2i), (double)i) + 1.0);
        cRe[i] = scale * (c_r + s_r);
        cIm[i] = scale * (c_i + s_i);
    }
}

static void poly_56_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);

    const double ar = x1r - x2i;
    const double ai = x1i + x2r;
    const double br = x2r - x1i;
    const double bi = x2i + x1r;
    for (int k = 1; k < 72; k++) {
        double p1r, p1i, p2r, p2i;
        poly_hand_pow_int(ar, ai, k, &p1r, &p1i);
        poly_hand_pow_int(br, bi, 71 - k, &p2r, &p2i);
        cRe[k - 1] = 0.5 * (p1r + p2r);
        cIm[k - 1] = 0.5 * (p1i + p2i);
    }
    const double angle = c_arg(ar, ai);
    const double scale_mid = 1.0 + sin(angle);
    const double scale_head = 1.0 + cos(angle);
    const double scale_tail = c_abs(ar, ai);
    for (int i = 3; i < 68; i++) { cRe[i] *= scale_mid; cIm[i] *= scale_mid; }
    for (int i = 0; i < 3; i++) { cRe[i] *= scale_head; cIm[i] *= scale_head; }
    for (int i = 68; i < 71; i++) { cRe[i] *= scale_tail; cIm[i] *= scale_tail; }
    const double log_mid = log(fabs(ai) + 1.0);
    cRe[34] *= log_mid;
    cIm[34] *= log_mid;
}

static void poly_63_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[71] = {0};

    for (int j = 1; j < 72; j++) {
        cf[j - 1] = csin(cpow(t1, (double)j)) * ccos(cpow(t2, (double)(71 - j))) *
                    cabs(t1 * cpow(t2, (double)j)) * log(cabs(t1 * t2 + 1.0));
    }
    for (int i = 0; i < 30; i++) cf[i] += cf[30 + i];

    double complex tmp39[39];
    for (int i = 0; i < 39; i++) tmp39[i] = cf[i];
    for (int i = 0; i < 39; i++) cf[32 + i] -= tmp39[i];

    double complex tmp50[50];
    for (int i = 0; i < 50; i++) tmp50[i] = cf[i];
    for (int i = 0; i < 50; i++) cf[10 + i] += (x1r * x2i) * tmp50[i];

    double complex tmp40[40];
    for (int i = 0; i < 40; i++) tmp40[i] = cf[1 + i];
    for (int i = 0; i < 40; i++) cf[30 + i] -= (x1i * x2r) * tmp40[i];

    double angle_a = carg(cpow(t1, t2));
    double complex tmp20[20];
    for (int i = 0; i < 20; i++) tmp20[i] = cf[30 + i];
    for (int i = 0; i < 20; i++) cf[20 + i] += angle_a * tmp20[i];

    double angle_b = carg(cpow(t2, t1));
    double complex tmp31[31];
    for (int i = 0; i < 31; i++) tmp31[i] = cf[i];
    for (int i = 0; i < 31; i++) cf[40 + i] -= angle_b * tmp31[i];

    for (int i = 0; i < 71; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_66_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);

    const double scale1 = x1r * log(c_abs(x2r, x2i) + 1.0);
    const double scale2 = x2i * log(c_abs(x1r, x1i) + 1.0);
    const double scale3 = c_abs(x1r, x1i) * c_abs(x2r, x2i) * log(c_abs(x1r + x2r, x1i + x2i) + 1.0);
    for (int i = 0; i < 25; i++) cRe[i] = scale1 * (double)((i + 1) * (i + 1));
    for (int i = 0; i < 25; i++) cRe[25 + i] = scale2 * (double)((i + 1) * (i + 1) * (i + 1));
    for (int i = 0; i < 20; i++) cRe[50 + i] = scale3 * (double)(i + 1);
    double sum = 0.0;
    for (int i = 0; i < 70; i++) sum += cRe[i];
    cRe[70] = sum * c_arg(x1r + x2r, x1i + x2i);
}

static void poly_75_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[71] = {0};

    cf[0] = 100.0 * cpow(t1, 3.0) + 110.0 * cpow(t1, 2.0) + 120.0 * t2 - 130.0;
    cf[1] = 200.0 * cpow(t2, 3.0) - 210.0 * cpow(t2, 2.0) + 220.0 * t2 - 230.0;
    cf[4] = pow(cabs(t1), 4.0);
    cf[9] = pow(carg(t2), 6.0);
    cf[14] = log(cabs(t1 + poly_hand_z(-cimag(t2), creal(t2)))) + 1.0;
    cf[19] = creal(poly_hand_z(0.0, 1.0) * cf[4] * t1 * t2);
    cf[39] = creal(poly_hand_z(0.0, 1.0) * cf[4] * t1 * t2);
    cf[29] = cimag(cf[1] * conj(cf[0]));
    cf[59] = cimag(cf[1] * conj(cf[0]));
    cf[34] = csin(cf[1]) + ccos(cf[0]);
    cf[2] = pow(cabs(cf[9]), 2.0);
    cf[3] = cf[1] * cf[2];
    cf[8] = cf[19] + cf[39] + cf[59];
    double abs_diff = cabs(t1 - t2);
    for (int i = 15; i < 71; i++) cf[i] = (double)i * abs_diff;
    cf[70] = cf[0] * cf[1] * cf[2] * cf[3];
    for (int i = 0; i < 71; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_79_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);

    const double abs1 = c_abs(x1r, x1i);
    const double abs2 = c_abs(x2r, x2i);
    for (int i = 0; i < 35; i++) {
        const double scale = (double)(i + 1) * pow(abs1, (double)(i + 1));
        cRe[i] = scale * (x1r + x2r);
        cIm[i] = scale * (x1i + x2i);
    }
    for (int i = 0; i < 35; i++) {
        const int p = 35 - i;
        const double scale = (double)p * pow(abs2, (double)p);
        cRe[35 + i] = scale * (x1r - x2r);
        cIm[35 + i] = scale * (x1i - x2i);
    }
    double prod_r, prod_i;
    c_mul(x1r, x1i, x2r, -x2i, &prod_r, &prod_i);
    cRe[70] = abs1 * abs2 + prod_i;
}

static void poly_80_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);

    double p1r, p1i, p2r, p2i;
    poly_hand_pow_int(x1r, x1i, 70, &p1r, &p1i);
    poly_hand_pow_int(x2r, x2i, 70, &p2r, &p2i);
    cRe[0] = p1r + p2r;
    cIm[0] = p1i + p2i;
    double prod_r, prod_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    for (int i = 1; i < 70; i++) {
        double arg_r = prod_r * (double)i;
        double arg_i = prod_i * (double)i;
        double s_r, s_i;
        c_sin(arg_r, arg_i, &s_r, &s_i);
        c_mul(s_r, s_i, s_r, s_i, &cRe[i], &cIm[i]);
    }
    const double scale_t2 = pow(log(c_abs(x2r, x2i) + 1.0), 2.0);
    for (int i = 13; i < 28; i++) { cRe[i] *= scale_t2; cIm[i] *= scale_t2; }
    const double scale_t1 = log(c_abs(x1r, x1i) + 1.0);
    for (int i = 30; i < 46; i++) { cRe[i] *= scale_t1; cIm[i] *= scale_t1; }
    const double bump = c_abs(x1r, x1i) * c_abs(x2r, x2i);
    for (int i = 2; i < 5; i++) cRe[i * 15] += (double)i * bump;
    cRe[70] = x1r * x1r * x1r - x2i * x2i;
}

static void poly_86_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[71] = {0};

    for (int k = 1; k < 72; k++) {
        cf[k - 1] = cpow(ccos((double)k * t1), (double)k) + poly_hand_z(0.0, 1.0) * cpow(csin((double)k * t2), (double)k);
    }
    for (int i = 1; i < 71; i += 2) cf[i] = 1.0 / cf[i];
    for (int i = 2; i < 71; i += 3) cf[i] = 1.0 / (cf[i] * cf[i]);
    for (int r = 5; r < 66; r += 5) cf[r - 1] = cpow(t1 * t2, (double)r);
    cf[70] = pow(cabs(t1), 2.0) + 2.0 * creal(t1) * cimag(t2) + 3.0 * pow(cabs(t2), 2.0);
    for (int i = 0; i < 71; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_102_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[71] = {0};

    cf[0] = 1000.0 * cpow(t1 + t2, 2.0);
    for (int k = 1; k < 20; k++) cf[k] = (double)(k + 1) * cf[k - 1] + csin((double)(k + 1) * t1) + ccos((double)(k + 1) * t2);
    for (int k = 20; k < 40; k++) cf[k] = (double)(k + 1) * cf[k - 1] - csin((double)(k + 1) * t1) - ccos((double)(k + 1) * t2);
    for (int k = 40; k < 60; k++) cf[k] = (double)(k + 1) * cf[k - 1] + csin((double)(k + 1) * t1 * t2) + ccos((double)(k + 1) * t1 * t2);
    for (int k = 60; k < 70; k++) cf[k] = (double)(k + 1) * cf[k - 1] - csin((double)(k + 1) * t1 * t2) - ccos((double)(k + 1) * t1 * t2);
    cf[70] = cabs(cf[69]) + carg(t1) - carg(t2) + creal(t1 * t2) - cimag(conj(t1) * t2);
    for (int i = 0; i < 71; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_110_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    static const double primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59};
    const int n_primes = (int)(sizeof(primes) / sizeof(primes[0]));
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);

    for (int k = 1; k < 36; k++) {
        cRe[k - 1] = x1r * primes[k % n_primes] + x2i * (double)(k * k);
        cRe[70 - k] = x2r * primes[(70 - k) % n_primes] - x1i * (double)(k * k);
    }
    cRe[35] = 440.0 * cos(c_arg(x1r, x1i));
    cIm[35] = 440.0 * sin(c_arg(x2r, x2i));
}

static void poly_113_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);

    const double zr = x1r - x2i;
    const double zi = x1i + x2r;
    const double absz = c_abs(zr, zi);
    const double angz = c_arg(zr, zi);
    for (int k = 1; k < 36; k++) {
        cRe[k - 1] = cos(M_PI * (double)k / 35.0) * (((k % 2) == 0) ? 1.0 : -1.0) * pow(absz, (double)k);
        cRe[70 - k] = sin(M_PI * (double)(35 - k) / 35.0) * (((k % 2) == 0) ? -1.0 : 1.0) * pow(angz, (double)(35 - k));
    }
    cRe[35] = exp(absz);
}

static void poly_114_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    static const double fib13[] = {89, -233, 144, 377, 610, -987, 1597, -2584, 4181, -6765, 10946, -17711, 28657};
    static const double tail_re[] = {0.0, 2.0, -6.0, -5.5};
    static const double tail_im[] = {3.0, -8.0, 11.0, 0.0};
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);

    double t1sq_r, t1sq_i, t2cu_r, t2cu_i, conjprod_r, conjprod_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    poly_hand_pow_int(x2r, x2i, 3, &t2cu_r, &t2cu_i);
    c_mul(x1r, x1i, x2r, -x2i, &conjprod_r, &conjprod_i);
    cRe[0] = t1sq_r + t2cu_r - conjprod_i;
    cIm[0] = 0.0;
    const double scalar = x1r + x2i;
    cRe[1] = -827.0 * scalar;
    cRe[2] = 221.0 * scalar;
    cRe[3] = 653.0 * scalar;
    double shifted_r = x1r + 2.0 * x2i;
    double shifted_i = x1i - 2.0 * x2r;
    cRe[4] = pow(c_abs(shifted_r, shifted_i), 5.0);
    for (int j = 6; j < 29; j++) {
        cRe[j - 1] = cos((double)j * c_arg(x1r + x2r, x1i + x2i)) * sin((double)j * c_abs(t1sq_r + x2r, t1sq_i + x2i)) + (double)j;
    }
    const double diff_abs = c_abs(x1r - x2r, x1i - x2i);
    for (int i = 0; i < 13; i++) cRe[28 + i] = fib13[i] * diff_abs;
    for (int k = 42; k < 62; k++) {
        double scaled_r, scaled_i;
        c_mul((double)k, 0.0, x1r, x1i, &scaled_r, &scaled_i);
        c_mul(scaled_r, scaled_i, x2r, -x2i, &scaled_r, &scaled_i);
        cRe[k - 1] = log(c_abs(scaled_r + 71.0, scaled_i));
    }
    double delta_r, delta_i;
    poly_hand_pow_int(x1r, x1i, 3, &delta_r, &delta_i);
    double t2_3r, t2_3i;
    poly_hand_pow_int(x2r, x2i, 3, &t2_3r, &t2_3i);
    delta_r -= t2_3r;
    delta_i -= t2_3i;
    for (int i = 0; i < 4; i++) {
        double sr, si;
        c_mul(tail_re[i], tail_im[i], delta_r, delta_i, &sr, &si);
        cRe[62 + i] = sr;
        cIm[62 + i] = si;
    }
    for (int i = 66; i < 71; i++) cRe[i] = exp(-(double)i);
    double e1r, e1i, e2r, e2i;
    c_exp2(x1r, x1i, &e1r, &e1i);
    c_exp2(x2r, x2i, &e2r, &e2i);
    cRe[70] = e1r - e2r;
    cIm[70] = e1i - e2i;
}

static void poly_115_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[71] = {0};
    double complex kappa = (1.0 + cpow(t1, 2.0)) * cpow(1.0 - t2, 2.0) * carg(t1) * carg(t2);
    double complex offset = poly_hand_z(creal(t1) - cimag(t2), cimag(t1) + creal(t2));

    cf[0] = kappa - cpow(t1, 3.0) + cpow(t2, 2.0);
    cf[1] = -2.0 * offset + t1 * t2 + cabs(t1 + t2);
    cf[2] = (3.0 + 2.0 * kappa) * (t1 - t2);
    cf[3] = 0.5 * (offset - poly_hand_z(0.0, 1.0) * kappa);
    for (int i = 0; i < 6; i++) cf[4 + i] = cabs(t1) / (double)(i + 1);
    for (int i = 0; i < 10; i++) cf[10 + i] = -cpow(t2, 2.0) * (double)(i + 1);
    for (int k = 21; k < 31; k++) cf[k - 1] = (cpow(t1, (double)k) - cpow(t2, (double)(k - 1))) / ((double)k * (double)k);
    for (int i = 0; i < 21; i++) cf[30 + i] = creal(offset) * (double)(21 + i) + 0.1 * cimag(offset) * (double)(i + 1);
    for (int k = 51; k < 61; k++) cf[k - 1] = pow(cimag(t1 * t2), 2.0) / ((double)k * (double)k);
    cf[61] = cabs(offset) + 0.1 * cpow(t2, 2.0) - 0.1 * cpow(t1, 2.0);
    cf[62] = 0.01 * (cpow(t1, 3.0) - 2.0 * cpow(t2, 3.0));
    cf[63] = 0.001 * (offset * conj(t2));
    for (int i = 64; i < 70; i++) cf[i] = (((double)(i + 1) * creal(offset)) + ((double)(i + 1) * pow(cimag(offset), 2.0))) / 2.0;
    cf[70] = -t1 + 2.0 * poly_hand_z(-cimag(t2), creal(t2));
    for (int i = 0; i < 71; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_307_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);

    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[35] = {0};
    for (int j = 1; j <= 35; j++) {
        double phase = sin((double)j * M_PI / 4.0) + cos((double)j * M_PI / 3.0) + carg(t1) * (double)j / 10.0;
        double magnitude = log(cabs(t1) + (double)j) * (1.0 + sin((double)j * M_PI / 6.0)) +
                           pow(poly_hand_np_int64_prod_1_to_n(j), 0.5) * cos((double)j * M_PI / 8.0);
        cf[j - 1] = magnitude * cexp(poly_hand_z(0.0, phase)) + conj(t2) * (double)(j % 5);
    }
    for (int i = 0; i < 35; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_314_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);

    for (int j = 1; j <= 35; j++) {
        const double r = fmod((double)(j * j) + x1r * x2i, 7.0) + 1.0;
        const double angle = c_arg(x1r, x1i) * sin((double)j * M_PI / r) + c_arg(x2r, x2i) * cos((double)j * M_PI / (r + 1.0));
        const double magnitude = pow(c_abs(x1r, x1i), 0.5 * (double)j) + pow(c_abs(x2r, x2i), 0.3 * (double)(35 - j + 1));
        poly_hand_from_polar(magnitude, angle, &cRe[j - 1], &cIm[j - 1]);
        cRe[j - 1] += x1r * sin((double)j) - x2r * cos((double)j);
        cIm[j - 1] += -x1i * sin((double)j) + x2i * cos((double)j);
        const double scale = log(c_abs(cRe[j - 1], cIm[j - 1]) + 1.0);
        const double extra = poly_hand_prod_1_to_n((j % 5) + 1) + ((double)j + r);
        double nr, ni;
        c_mul(cRe[j - 1], cIm[j - 1], scale, 0.0, &nr, &ni);
        cRe[j - 1] = nr + extra;
        cIm[j - 1] = ni;
    }
}

static void poly_317_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);

    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[35] = {0};
    for (int j = 1; j <= 35; j++) {
        double mag_part = log(cabs(t1) + (double)j) * sin((double)j) + cos((double)(j * j));
        double angle_part = carg(t1) * cos((double)j) + carg(t2) * sin((double)j);
        cf[j - 1] = mag_part * cexp(poly_hand_z(0.0, angle_part)) + conj(t1) * (poly_hand_np_int64_prod_1_to_n(j) / (double)(j + 1));
    }
    for (int k = 1; k <= 35; k++) cf[k - 1] *= poly_hand_z(1.0 + 0.05 * cos((double)k), 0.03 * sin((double)k));
    for (int i = 0; i < 35; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_324_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);

    double complex cf[35] = {0};
    for (int j = 0; j < 35; j++) {
        double r = x1r + (x2r - x1r) * (double)j / 34.0;
        double d = x1i + (x2i - x1i) * (double)j / 34.0;
        double complex z = poly_hand_z(r, d);
        double mag = log(fabs(r * r) + 1.0) * (1.0 + sin(2.0 * M_PI * r * (double)j)) * creal(1.0 + ccos(poly_hand_z(0.0, M_PI * (double)j)));
        double ang = carg(z) + sin((double)j) * log(cabs(poly_hand_z(r, 1.0))) - cos((double)j) * carg(poly_hand_z(r, -1.0));
        cf[j] = mag * poly_hand_z(cos(ang), sin(ang));
    }
    for (int i = 0; i < 35; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_333_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);

    const double log_abs1 = log(c_abs(x1r, x1i) + 1.0);
    const double abs2 = c_abs(x2r, x2i);
    const double angle_prod = c_arg(x1r, x1i) * c_arg(x2r, x2i);
    for (int j = 0; j < 35; j++) {
        const double denom = (double)(j + 1);
        const double angle = sin((double)j * x1i) + cos((double)j * x2r) + angle_prod / denom;
        const double magnitude = log_abs1 * pow((double)j, 1.5) + exp(-(double)j / (abs2 + 1.0)) * sqrt((double)j);
        double a_r, a_i, b_r, b_i;
        poly_hand_from_polar(magnitude, angle, &a_r, &a_i);
        poly_hand_from_polar(magnitude, angle / denom, &b_r, &b_i);
        cRe[j] = a_r + b_r;
        cIm[j] = a_i + b_i;
    }
}

static void poly_345_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);

    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[35] = {0};
    for (int j = 0; j < 35; j++) {
        double rec = x1r + (x2r - x1r) * (double)j / 34.0;
        double mag_part = log(fabs(rec) + 1.0) * poly_hand_np_int64_prod_1_to_n(j) / (double)(j + 2);
        double angle_part = carg(t1) * sin((double)j) + carg(t2) * cos((double)j / 3.0);
        cf[j] = mag_part * poly_hand_z(cos(angle_part), sin(angle_part));
        cf[j] += conj(cf[j]) * sin((double)j * M_PI / 4.0);
    }
    for (int i = 0; i < 35; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_450_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);

    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[35] = {0};
    double ang1 = carg(t1);
    double ang2 = carg(t2);
    double abs_sum = cabs(t1) + cabs(t2);
    for (int j = 1; j <= 35; j++) {
        double angle_part = sin((double)j * ang1 + cos((double)j * ang2));
        double mag_part = log(abs_sum + (double)j) * (1.0 + 0.1 * (double)j);
        double phase_shift = ang1 * cos((double)j / 35.0 * M_PI) - ang2 * sin((double)j / 35.0 * M_PI);
        cf[j - 1] = mag_part * poly_hand_z(cos(angle_part + phase_shift), sin(angle_part - phase_shift));
    }
    for (int k = 1; k <= 5; k++) {
        int idx = 35 - k;
        cf[idx] = cf[idx] * cpow(conj(t1), (double)((k % 3) + 1)) + cpow(conj(t2), (double)(k % 4));
    }
    double cumulative = 1.0;
    for (int r = 1; r <= 35; r++) {
        cumulative *= abs_sum + (double)r;
        cf[r - 1] += cumulative / (double)(r + 1);
    }
    for (int i = 0; i < 35; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_478_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);

    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[35] = {0};
    for (int j = 1; j <= 35; j++) {
        double mag_part1 = log(cabs(t1) + (double)j);
        double mag_part2 = sin((double)j * x1r) * cos((double)j / (x1i + 1.0));
        double magnitude = mag_part1 * mag_part2 + pow(poly_hand_np_int64_prod_1_to_n(j), 0.5);
        double angle = carg(t1) * sin((double)j) + carg(t2) * cos((double)j) + sin((double)j * x1r) - cos((double)j * x2i);
        cf[j - 1] = magnitude * cexp(poly_hand_z(0.0, angle)) + conj(t1) * sin((double)j) - conj(t2) * cos((double)j);
    }
    for (int i = 0; i < 35; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_777_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    poly_hand_zero(cRe, cIm, 25);

    cRe[1] = x1r + x2r;
    cIm[1] = x1i + x2i;
    for (int k = 2; k < 25; k++) {
        const double sum_r = cRe[k - 1] + x2r;
        const double sum_i = cIm[k - 1] + x2i;
        double phase_r, phase_i;
        poly_hand_from_polar(c_abs((double)k * cRe[k - 1] + x1r, (double)k * cIm[k - 1] + x1i), c_arg(sum_r, sum_i), &phase_r, &phase_i);
        cRe[k] = phase_r;
        cIm[k] = phase_i;
    }
    cRe[9] = x1r + x2i;
    cIm[9] = 0.0;
    cRe[14] = 0.0;
    cIm[14] = x1r + x2i;
    double prod_r, prod_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    cRe[19] = prod_r;
    cIm[19] = prod_r;
    cRe[24] = pow(c_abs(x1r, x1i), 2.0) + pow(c_abs(x2r, x2i), 2.0);
    cIm[24] = 0.0;
}

static void poly_782_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    poly_hand_zero(cRe, cIm, 25);

    cRe[0] = x1r + x2r;
    cIm[0] = x1i + x2i;
    double prod_r, prod_i;
    c_mul(x1r, x1i, x2r, -x2i, &prod_r, &prod_i);
    poly_hand_from_polar(1.0, c_arg(prod_r, prod_i), &cRe[1], &cIm[1]);
    cRe[2] = c_abs(x1r, x1i) * c_abs(x2r, x2i);
    for (int k = 3; k < 25; k++) {
        double phase_r, phase_i, nr, ni;
        poly_hand_from_polar(1.0, c_arg(cRe[k - 2], cIm[k - 2]), &phase_r, &phase_i);
        c_mul(cRe[k - 1], cIm[k - 1], phase_r, phase_i, &nr, &ni);
        if (ni == 0.0) nr += 1e-10;
        const double log_term = log(c_abs(nr, ni)) / 2.0;
        cRe[k] = log_term - ni;
        cIm[k] = nr;
    }
}

static void poly_667_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int i = 0; i < 9; i++) { cRe[i] = 0; cIm[i] = 0; }

    const int degree = 8;
    const double abs_t1 = c_abs(x1r, x1i);
    const double ang_t2 = c_arg(x2r, x2i);
    double conj_prod_r, conj_prod_i;
    c_mul(x1r, -x1i, x2r, -x2i, &conj_prod_r, &conj_prod_i);
    const double conj_ang = c_arg(conj_prod_r, conj_prod_i);

    for (int j = 1; j <= degree; j++) {
        double mag_sum = 0.0;
        double angle_sum = 0.0;
        for (int k = 1; k <= j; k++) {
            mag_sum += log(abs_t1 + (double)k) * sin((double)k * ang_t2 + (double)j);
            angle_sum += cos((double)k * M_PI / (double)(j + 1));
        }
        const double magnitude = mag_sum * (1.0 + (double)j);
        const double angle = angle_sum + conj_ang * (double)(j * j);
        double phase_r, phase_i;
        c_exp2(0.0, angle, &phase_r, &phase_i);
        cRe[j] = magnitude * phase_r;
        cIm[j] = magnitude * phase_i;
    }

    double abs_sum = 0.0;
    for (int i = 0; i < degree; i++) abs_sum += c_abs(cRe[i], cIm[i]);
    cRe[degree] = conj_prod_r + abs_sum;
    cIm[degree] = conj_prod_i;

    for (int i = 0; i < 9; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_746_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int i = 0; i < 11; i++) { cRe[i] = 0; cIm[i] = 0; }

    const double seq_start = -1.0 + 2.0 * x1r;
    const double seq_end = 1.0 - 2.0 * x2i;
    const double seq_step = (seq_end - seq_start) / 10.0;
    const double log_scale = log(1.0 + c_abs(x2r, x2i));
    const double weight = log_scale * log_scale;
    double prefix_sum = 0.0;
    for (int i = 0; i < 11; i++) {
        prefix_sum += seq_start + seq_step * (double)i;
        double exp_r, exp_i;
        c_exp2(-(double)(i + 1) * x1i, (double)(i + 1) * x1r, &exp_r, &exp_i);
        cRe[i] = exp_r + prefix_sum * weight;
        cIm[i] = exp_i;
    }

    double tmp_r, tmp_i;
    c_mul(cRe[0], cIm[0], cRe[1], cIm[1], &tmp_r, &tmp_i);
    c_mul(tmp_r, tmp_i, x1r, x1i, &tmp_r, &tmp_i);
    double sqrt_r, sqrt_i;
    c_powr(tmp_r, tmp_i, 0.5, &sqrt_r, &sqrt_i);
    cRe[10] += sqrt_r;
    cIm[10] += sqrt_i;

    c_mul(cRe[9], cIm[9], cRe[10], cIm[10], &tmp_r, &tmp_i);
    c_mul(tmp_r, tmp_i, x2r, x2i, &tmp_r, &tmp_i);
    c_powr(tmp_r, tmp_i, 0.5, &sqrt_r, &sqrt_i);
    cRe[0] -= sqrt_r;
    cIm[0] -= sqrt_i;

    double sum_r = 0.0, sum_i = 0.0;
    for (int i = 0; i < 11; i++) {
        sum_r += cRe[i];
        sum_i += cIm[i];
    }
    cRe[5] = sum_r / 11.0;
    cIm[5] = sum_i / 11.0;

    double div_r, div_i;
    c_div(cRe[7], cIm[7], cRe[5], cIm[5], &div_r, &div_i);
    c_mul(cRe[2], cIm[2], div_r, div_i, &cRe[2], &cIm[2]);

    c_mul(cRe[7], cIm[7], cRe[7], cIm[7], &tmp_r, &tmp_i);
    cRe[7] = tmp_r - cRe[4] + cRe[8];
    cIm[7] = tmp_i - cIm[4] + cIm[8];

    for (int i = 0; i < 11; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_759_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int i = 0; i < 11; i++) { cRe[i] = 0; cIm[i] = 0; }

    double exp_r, exp_i, trig_r, trig_i, tmp_r, tmp_i;
    c_exp2(x1r, x1i, &exp_r, &exp_i);
    c_cos(x2r, x2i, &trig_r, &trig_i);
    c_mul(exp_r, exp_i, trig_r, trig_i, &cRe[0], &cIm[0]);

    c_exp2(-x1i, x1r, &exp_r, &exp_i);
    cRe[1] = -exp_r;
    cIm[1] = -exp_i;

    cRe[2] = log(c_abs(x2r, x2i));

    c_div(x1r + x2r, x1i + x2i, 0.0, 1.0, &cRe[3], &cIm[3]);

    double t1sq_r, t1sq_i, t2sq_r, t2sq_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(t1sq_r, t1sq_i, x1r, x1i, &tmp_r, &tmp_i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    c_mul(t2sq_r, t2sq_i, x2r, x2i, &t2sq_r, &t2sq_i);
    cRe[4] = tmp_r - t2sq_r;
    cIm[4] = tmp_i - t2sq_i;

    c_exp2(x2r, x2i, &exp_r, &exp_i);
    c_sin(x1r, x1i, &trig_r, &trig_i);
    c_mul(-2.0 * exp_r, -2.0 * exp_i, trig_r, trig_i, &cRe[5], &cIm[5]);

    cRe[6] = sqrt(c_abs(x1r - x2r, x1i - x2i));

    double sum_r = x1r + x2r, sum_i = x1i + x2i;
    c_mul(sum_r, sum_i, sum_r, sum_i, &tmp_r, &tmp_i);
    c_exp2(-tmp_i, tmp_r, &cRe[7], &cIm[7]);

    c_mul(x1r, x1i, x2r, x2i, &tmp_r, &tmp_i);
    cRe[8] = log1p(c_abs(tmp_r, tmp_i));

    c_div(x1r - x2r, x1i - x2i, 0.0, 1.0, &cRe[9], &cIm[9]);

    c_exp2(tmp_r, tmp_i, &exp_r, &exp_i);
    c_div(exp_r, exp_i, x1r, x1i, &cRe[10], &cIm[10]);

    for (int i = 0; i < 11; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_766_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int i = 0; i < 11; i++) { cRe[i] = 0; cIm[i] = 0; }

    c_exp2(0.0, c_abs(x1r, x1i), &cRe[0], &cIm[0]);
    cRe[0] *= 100.0;
    cIm[0] *= 100.0;

    cRe[1] = 10.0 * (x1r * x1r - x2i * x2i);
    cIm[2] = (x2r + x1i) * (x1r - x2i);

    double sum_r = x1r + x2r, sum_i = x1i + x2i;
    double diff_r = x1r - x2r, diff_i = x1i - x2i;
    double sq_r, sq_i, tmp_r, tmp_i;
    c_mul(sum_r, sum_i, sum_r, sum_i, &sq_r, &sq_i);
    c_mul(diff_r, diff_i, diff_r, diff_i, &tmp_r, &tmp_i);
    cRe[3] = sq_r - tmp_r;
    cIm[3] = sq_i - tmp_i;

    c_mul(x1r, x1i, x2r, x2i, &tmp_r, &tmp_i);
    const double denom = 1.0 + c_abs(tmp_r, tmp_i);
    cRe[4] = 100.0 * sum_r / denom;
    cIm[4] = 100.0 * sum_i / denom;

    double t1pow_r = x1r, t1pow_i = x1i;
    double t2pow_r = x2r, t2pow_i = x2i;
    for (int k = 1; k <= 5; k++) {
        cRe[5] += t1pow_r * t2pow_i;
        c_mul(t1pow_r, t1pow_i, x1r, x1i, &t1pow_r, &t1pow_i);
        c_mul(t2pow_r, t2pow_i, x2r, x2i, &t2pow_r, &t2pow_i);
    }

    double t1sq_r, t1sq_i, t2sq_r, t2sq_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    cRe[6] = sqrt(c_abs(t1sq_r - t2sq_r, t1sq_i - t2sq_i));

    c_mul(x1r, x1i, x2r, x2i, &tmp_r, &tmp_i);
    const double diff_abs = 1.0 + c_abs(x1r - x2r, x1i - x2i);
    cRe[7] = tmp_r / diff_abs;
    cIm[7] = tmp_i / diff_abs;

    const int a0 = (int)floor(x1r);
    const int a1 = (int)floor(x2i);
    const int b0 = (int)floor(x2r);
    const int b1 = (int)floor(x1i);
    cRe[8] = poly_hand_prod_range_inclusive(a0, a1);
    cIm[8] = poly_hand_prod_range_inclusive(b0, b1);

    c_exp2(0.0, x1r - x2i, &cRe[9], &cIm[9]);
    c_exp2(0.0, x1i - x2r, &cRe[10], &cIm[10]);

    for (int i = 0; i < 11; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_769_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 10;
    for (int i = 0; i < 10; i++) { cRe[i] = 0; cIm[i] = 0; }

    double sin_r, sin_i, cos_r, cos_i;
    c_sin(x1r, x1i, &sin_r, &sin_i);
    double sin_sq_r, sin_sq_i;
    c_mul(sin_r, sin_i, sin_r, sin_i, &sin_sq_r, &sin_sq_i);
    c_mul(sin_sq_r, sin_sq_i, sin_r, sin_i, &sin_r, &sin_i);
    c_cos(x2r, x2i, &cos_r, &cos_i);
    c_mul(cos_r, cos_i, cos_r, cos_i, &cos_r, &cos_i);
    c_mul(100.0 * sin_r, 100.0 * sin_i, cos_r, cos_i, &cRe[0], &cIm[0]);

    double sum_r = x1r + x2r, sum_i = x1i + x2i;
    double diff_r = x1r - x2r, diff_i = x1i - x2i;
    double exp_r, exp_i, sq_r, sq_i, tmp_r, tmp_i;
    c_exp2(-sum_i, sum_r, &exp_r, &exp_i);
    c_mul(diff_r, diff_i, diff_r, diff_i, &sq_r, &sq_i);
    cRe[1] = 100.0 * exp_r - 10.0 * sq_r;
    cIm[1] = 100.0 * exp_i - 10.0 * sq_i;

    double prod_r, prod_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    c_mul(prod_r, prod_i, diff_r, diff_i, &cRe[2], &cIm[2]);
    const double denom = c_abs(x1r, x1i) + c_abs(x2r, x2i) + 1.0;
    cRe[2] /= denom;
    cIm[2] /= denom;

    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    double t1sq_r, t1sq_i, t2sq_r, t2sq_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    c_exp2(-(t1sq_i - t2sq_i), t1sq_r - t2sq_r, &exp_r, &exp_i);
    c_mul(prod_r, prod_i, exp_r, exp_i, &tmp_r, &tmp_i);
    double z_r = tmp_r, z_i = tmp_i;
    c_mul(z_r, z_i, z_r, z_i, &tmp_r, &tmp_i);
    c_mul(tmp_r, tmp_i, z_r, z_i, &cRe[4], &cIm[4]);

    c_sin(prod_r, prod_i, &sin_r, &sin_i);
    cRe[6] = sqrt(c_abs(x1r, x1i)) - sqrt(c_abs(x2r, x2i)) - sin_i;
    cIm[6] = sin_r;

    c_exp2(0.0, c_abs(sum_r, sum_i), &exp_r, &exp_i);
    cRe[7] = 50.0 * c_abs(diff_r, diff_i) * exp_r;
    cIm[7] = 50.0 * c_abs(diff_r, diff_i) * exp_i;

    if (x1i > 0.0) {
        cRe[8] = x1r - c_abs(x2r, x2i);
        cIm[8] = x1i;
    } else {
        cRe[8] = x2r - c_abs(x1r, x1i);
        cIm[8] = x2i;
    }

    c_mul(0.0, 1.0, prod_r, prod_i, &tmp_r, &tmp_i);
    c_powc(tmp_r, tmp_i, 0.1 * prod_r, 0.1 * prod_i, &cRe[9], &cIm[9]);

    for (int i = 0; i < 10; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_773_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int i = 0; i < 11; i++) { cRe[i] = 0; cIm[i] = 0; }

    double sin_r, sin_i, cos_r, cos_i;
    c_sin(x1r, x1i, &sin_r, &sin_i);
    c_cos(x2r, x2i, &cos_r, &cos_i);
    cRe[0] = sin_r - cos_i;
    cIm[0] = sin_i + cos_r;

    cRe[1] = x2r + x1i;
    cIm[1] = x2i - x1r;

    double diff_r = x1r - x2r, diff_i = x1i - x2i;
    double exp_r, exp_i;
    c_exp2(-diff_i, diff_r, &exp_r, &exp_i);
    const double log_term = log(1.0 + c_abs(x1r, x1i) + c_abs(x2r, x2i));
    cRe[2] = exp_r + log_term;
    cIm[2] = exp_i;

    double prod_r, prod_i, prod_sq_r, prod_sq_i, diff_sq_r, diff_sq_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    c_mul(prod_r, prod_i, prod_r, prod_i, &prod_sq_r, &prod_sq_i);
    c_mul(diff_r, diff_i, diff_r, diff_i, &diff_sq_r, &diff_sq_i);
    cRe[3] = prod_sq_r + diff_sq_i;
    cIm[3] = prod_sq_i - diff_sq_r;

    c_sin(x2r, x2i, &sin_r, &sin_i);
    c_cos(x1r, x1i, &cos_r, &cos_i);
    cRe[4] = -100.0 * sin_i + 100.0 * cos_r;
    cIm[4] = 100.0 * sin_r + 100.0 * cos_i;

    double sum_r = x1r + x2r, sum_i = x1i + x2i;
    double sum_sq_r, sum_sq_i, sum_cu_r, sum_cu_i;
    c_mul(sum_r, sum_i, sum_r, sum_i, &sum_sq_r, &sum_sq_i);
    c_mul(sum_sq_r, sum_sq_i, sum_r, sum_i, &sum_cu_r, &sum_cu_i);
    c_mul(diff_sq_r, diff_sq_i, diff_r, diff_i, &diff_sq_r, &diff_sq_i);
    cRe[5] = sum_cu_r + diff_sq_i;
    cIm[5] = sum_cu_i - diff_sq_r;

    c_exp2(-prod_i, prod_r, &cRe[6], &cIm[6]);

    cRe[7] = log1p(c_abs(prod_r, prod_i)) - diff_i;
    cIm[7] = diff_r;

    double t1sq_r, t1sq_i, t2sq_r, t2sq_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    cRe[8] = t1sq_r + t2sq_r;
    cIm[8] = t1sq_i + t2sq_i;

    const double base_r = x1r - x2i;
    const double base_i = x1i + x2r;
    c_mul(base_r, base_i, base_r, base_i, &sum_sq_r, &sum_sq_i);
    c_mul(sum_sq_r, sum_sq_i, base_r, base_i, &cRe[9], &cIm[9]);

    c_exp2(0.0, 550.0, &exp_r, &exp_i);
    c_mul(exp_r, exp_i, prod_r, prod_i, &cRe[10], &cIm[10]);

    for (int i = 0; i < 11; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_785_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int i = 0; i < 25; i++) { cRe[i] = 0; cIm[i] = 0; }

    cRe[0] = 1.0;
    c_mul(x1r, x1i, x1r, x1i, &cRe[1], &cIm[1]);
    c_mul(x2r, x2i, x2r, x2i, &cRe[2], &cIm[2]);
    double prod_r, prod_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    c_mul(prod_r, prod_i, prod_r, prod_i, &cRe[3], &cIm[3]);
    cRe[4] = c_abs(x1r + x2r, x1i + x2i);
    cRe[4] *= cRe[4];

    for (int k = 5; k < 25; k++) {
        cRe[k] = 3.0 * cRe[k - 1] + 2.0 * cRe[k - 5] + 5.0 * (double)(k + 1);
        cIm[k] = 3.0 * cIm[k - 1] + 2.0 * cIm[k - 5];
    }

    for (int i = 0; i < 6; i++) {
        cRe[9 + i] = cRe[i];
        cIm[9 + i] = -cIm[i];
    }
    for (int i = 0; i < 6; i++) {
        double phase_r, phase_i;
        c_exp2(0.0, c_arg(cRe[i], cIm[i]), &phase_r, &phase_i);
        cRe[19 + i] = phase_r;
        cIm[19 + i] = phase_i;
    }

    for (int i = 0; i < 25; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_788_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int i = 0; i < 25; i++) { cRe[i] = 0; cIm[i] = 0; }

    const double base_r = x1r - x2i;
    const double base_i = x1i + x2r;
    cRe[0] = base_r;
    cIm[0] = base_i;
    cRe[1] = 2.0 * base_r * base_r;
    cRe[2] = 3.0 * base_i * base_i * base_i;

    const double ang_sum = c_arg(x1r, x1i) + c_arg(x2r, x2i);
    for (int k = 3; k < 15; k++) {
        double a_r, a_i, b_r, b_i, sum_r, sum_i;
        c_powr(cRe[k - 1], cIm[k - 1], (double)(k + 1), &a_r, &a_i);
        c_powr(cRe[k - 2], cIm[k - 2], (double)k, &b_r, &b_i);
        sum_r = a_r + b_r;
        sum_i = a_i + b_i;
        cRe[k] = c_abs(sum_r, sum_i) / ((double)k * (double)k + 1.0) + ang_sum;
    }

    for (int i = 0; i < 5; i++) {
        cRe[15 + i] = cRe[10 + i] + cRe[i];
        cIm[15 + i] = cIm[10 + i] + cIm[i];
    }

    cRe[20] = c_abs(x1r, x1i) * c_abs(x1r, x1i) - c_abs(x2r, x2i) * c_abs(x2r, x2i);
    cRe[21] = ang_sum;
    double t1sq_r, t1sq_i, t2sq_r, t2sq_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(t1sq_r, t1sq_i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    c_mul(t2sq_r, t2sq_i, x2r, x2i, &t2sq_r, &t2sq_i);
    cRe[22] = t1sq_r - t2sq_r;
    double prod_r, prod_i, diff_r = x1r - x2r, diff_i = x1i - x2i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    c_mul(prod_r, prod_i, diff_r, diff_i, &prod_r, &prod_i);
    cRe[23] = prod_i;
    cRe[24] = sqrt(c_abs(prod_r, prod_i));

    for (int i = 0; i < 25; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_794_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int i = 0; i < 25; i++) { cRe[i] = 0; cIm[i] = 0; }

    const double abs_t1 = c_abs(x1r, x1i);
    const double ang_t2 = c_arg(x2r, x2i);
    const double conj_t2_r = x2r;
    const double conj_t2_i = -x2i;
    for (int k = 0; k < 25; k++) {
        double term_r = pow(abs_t1, (double)k) + pow(ang_t2, (double)k);
        double term_i = 0.0;
        switch (k % 4) {
            case 0: term_r += x1r; term_i += x1i; break;
            case 1: term_r -= x1i; term_i += x1r; break;
            case 2: term_r -= x1r; term_i -= x1i; break;
            default: term_r += x1i; term_i -= x1r; break;
        }
        double pow_r, pow_i;
        c_powr(conj_t2_r, conj_t2_i, 0.5 * (double)(k + 1), &pow_r, &pow_i);
        cRe[k] = term_r + pow_r;
        cIm[k] = term_i + pow_i;
    }

    double prod_r, prod_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    cRe[9] += log(c_abs(prod_r, prod_i));
    if (isfinite(cRe[9]) && isfinite(cIm[9])) {
        const double denom = c_abs(x1r + x2r, x1i + x2i + 1.0);
        cRe[9] /= denom;
        cIm[9] /= denom;
    }

    const double mix_r = x1r - x2i;
    const double mix_i = x1i + x2r;
    double mix_sq_r, mix_sq_i;
    c_mul(mix_r, mix_i, mix_r, mix_i, &mix_sq_r, &mix_sq_i);
    cRe[19] += mix_sq_r;
    cIm[19] += mix_sq_i;
    if (isfinite(cRe[19]) && isfinite(cIm[19])) {
        const double denom2 = c_abs(x1r + x2r, x1i + x2i + 1.0);
        cRe[19] /= denom2 * denom2;
        cIm[19] /= denom2 * denom2;
    }

    double a_r, a_i, b_r, b_i, d_r, d_i;
    c_mul(cRe[4], cIm[4], cRe[4], cIm[4], &a_r, &a_i);
    c_mul(cRe[5], cIm[5], cRe[5], cIm[5], &b_r, &b_i);
    c_mul(cRe[6], cIm[6], cRe[6], cIm[6], &d_r, &d_i);
    c_powr(a_r + b_r + d_r, a_i + b_i + d_i, 0.5, &cRe[24], &cIm[24]);

    for (int i = 0; i < 25; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_810_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int i = 0; i < 25; i++) { cRe[i] = 0; cIm[i] = 0; }

    const double base_even_r = x1r + x2r;
    const double base_even_i = x1i - x2i;
    for (int i = 0; i < 25; i++) {
        if (i == 0) {
            cRe[i] = 1.0;
            continue;
        }
        if (i % 2 == 1) {
            const double scale = 1.0 / ((double)(i + 1) * (double)(i + 1));
            const double base_r = ((double)i * x1r + 3.0 * (double)i * x2r) * scale;
            const double base_i = ((double)i * x1i + 3.0 * (double)i * x2i) * scale;
            c_powr(base_r, base_i, (double)i, &cRe[i], &cIm[i]);
        } else {
            c_powr(base_even_r, base_even_i, (double)i, &cRe[i], &cIm[i]);
        }
    }

    for (int i = 0; i < 25; i++) {
        if (cRe[i] == INFINITY && cIm[i] == 0.0) {
            cRe[i] = 1e10;
            cIm[i] = 0.0;
        } else if (cRe[i] == -INFINITY && cIm[i] == 0.0) {
            cRe[i] = -1e10;
            cIm[i] = 0.0;
        } else if (isnan(cRe[i]) || isnan(cIm[i])) {
            cRe[i] = 0.0;
            cIm[i] = 0.0;
        }
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_108_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double t1sq_r, t1sq_i, t2sq_r, t2sq_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    cRe[0] = x1r + x2r;
    cIm[0] = x1i + x2i;
    cRe[1] = -(t1sq_r + x2r);
    cIm[1] = -(t1sq_i + x2i);
    cRe[2] = t1sq_r - t2sq_r;
    cIm[2] = t1sq_i - t2sq_i - 1.0;

    cRe[3] = 1.0 - x1r; cIm[3] = -x1i;
    cRe[4] = -1.0 + x2r; cIm[4] = x2i;
    cRe[5] = 2.0 - x1r; cIm[5] = -x1i;
    cRe[6] = -2.0 + x2r; cIm[6] = x2i;
    cRe[7] = 3.0 - x1r; cIm[7] = -x1i;
    cRe[8] = -3.0 + x2r; cIm[8] = x2i;
    cRe[9] = 4.0 - x1r; cIm[9] = -x1i;

    cRe[10] = 15.0 * (x1r + x2i);
    cRe[11] = -17.0 * c_arg(x1r, x1i) * c_arg(x2r, x2i);
    cRe[14] = 30.0 * c_abs(x1r, x1i) * c_abs(x2r, x2i);

    double t1cu_r, t1cu_i, t2cu_r, t2cu_i;
    c_mul(t1sq_r, t1sq_i, x1r, x1i, &t1cu_r, &t1cu_i);
    c_mul(t2sq_r, t2sq_i, x2r, x2i, &t2cu_r, &t2cu_i);
    cRe[16] = -(t1cu_r + t2cu_r);
    cIm[16] = -(t1cu_i + t2cu_i);

    cRe[18] = -t1sq_i + t2sq_i;
    cIm[18] = t1sq_r - t2sq_r;
    cRe[19] = 5.0 - x1i;
    cIm[19] = x1r;

    cRe[24] = 50.0 * c_abs(x1r - x2r, x1i - x2i);
    cRe[29] = -40.0 * x1r + 35.0 * x2i;
    cRe[34] = 18.0 * (x1r - x2i);

    double t1_4r, t1_4i, t2_4r, t2_4i;
    c_mul(t1sq_r, t1sq_i, t1sq_r, t1sq_i, &t1_4r, &t1_4i);
    c_mul(t2sq_r, t2sq_i, t2sq_r, t2sq_i, &t2_4r, &t2_4i);
    cRe[39] = -t1_4r + t2_4r - 3.0;
    cIm[39] = -t1_4i + t2_4i;
    cRe[44] = 3.0 * c_arg(x1r, x1i) + 4.0 * c_arg(x2r, x2i);
    cRe[49] = -55.0 * fabs(c_abs(x1r, x1i) - c_abs(x2r, x2i));
    cRe[54] = 33.0 * pow(c_abs(x1r, x1i), 3.0) + pow(c_abs(x2r, x2i), 2.0);

    double t1_5r, t1_5i, t2_5r, t2_5i;
    c_mul(t1_4r, t1_4i, x1r, x1i, &t1_5r, &t1_5i);
    c_mul(t2_4r, t2_4i, x2r, x2i, &t2_5r, &t2_5i);
    cRe[59] = t1_5r + t2_5r - 29.0;
    cIm[59] = t1_5i + t2_5i;
    cRe[64] = -22.0 * t1sq_r + 22.0 * t2sq_i;
    cRe[69] = 15.0 * x1i + 120.0 * x2r;

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_116_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    const double primes[15] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47};
    double p1r[71], p1i[71], p2r[71], p2i[71];
    p1r[0] = 1.0; p1i[0] = 0.0;
    p2r[0] = 1.0; p2i[0] = 0.0;
    for (int i = 1; i <= 70; i++) {
        c_mul(p1r[i - 1], p1i[i - 1], x1r, x1i, &p1r[i], &p1i[i]);
        c_mul(p2r[i - 1], p2i[i - 1], x2r, x2i, &p2r[i], &p2i[i]);
    }

    for (int k = 1; k <= 35; k++) {
        const double s1 = (k % 2) ? -1.0 : 1.0;
        const double a = primes[k % 15] * s1 / (double)(k + 1);
        cRe[k - 1] = a * (p1r[k] + p2r[k]);
        cIm[k - 1] = a * (p1i[k] + p2i[k]);

        const int m = 71 - k;
        const double s2 = (m % 2) ? -1.0 : 1.0;
        const double b = primes[(k + 11) % 15] * s2 / (double)(m + 1);
        cRe[70 - k] = b * (p1r[m] - p2r[m]);
        cIm[70 - k] = b * (p1i[m] - p2i[m]);
    }

    cRe[35] = 28.0 * c_abs(x1r + x2r, x1i + x2i) / (1.0 + c_abs(x1r, x1i));
    cRe[70] = 1.0;
    cIm[70] = 1.0;

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_183_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double p1r[7], p1i[7], p2r[6], p2i[6];
    p1r[0] = 1.0; p1i[0] = 0.0;
    p2r[0] = 1.0; p2i[0] = 0.0;
    for (int i = 1; i <= 6; i++) c_mul(p1r[i - 1], p1i[i - 1], x1r, x1i, &p1r[i], &p1i[i]);
    for (int i = 1; i <= 5; i++) c_mul(p2r[i - 1], p2i[i - 1], x2r, x2i, &p2r[i], &p2i[i]);

    cRe[0] = p1r[6] + 2.0 * p2r[5];
    cIm[0] = p1i[6] + 2.0 * p2i[5];
    cRe[1] = 3.0 * p1r[5] - p2r[4];
    cIm[1] = 3.0 * p1i[5] - p2i[4];
    cRe[2] = 4.0 * p1r[4] + 2.0 * p2r[3];
    cIm[2] = 4.0 * p1i[4] + 2.0 * p2i[3];
    cRe[3] = 5.0 * p1r[3] - 3.0 * p2r[2];
    cIm[3] = 5.0 * p1i[3] - 3.0 * p2i[2];
    cRe[4] = 6.0 * p1r[2] + 4.0 * x2r;
    cIm[4] = 6.0 * p1i[2] + 4.0 * x2i;
    cRe[5] = 7.0 * x1r - 5.0;
    cIm[5] = 7.0 * x1i;

    const double a_r = x1r + 1.0;
    const double a_i = x1i;
    const double b_r = x2r - 1.0;
    const double b_i = x2i;
    double pa_r = 1.0, pa_i = 0.0, pb_r = 1.0, pb_i = 0.0;
    for (int j = 1; j <= 35; j++) {
        c_mul(pa_r, pa_i, a_r, a_i, &pa_r, &pa_i);
        c_mul(pb_r, pb_i, b_r, b_i, &pb_r, &pb_i);
        if (j >= 6) {
            const double sign = (j % 2) ? -1.0 : 1.0;
            cRe[j] = sign * pa_r + pb_r;
            cIm[j] = sign * pa_i + pb_i;
        }
    }

    for (int j = 36; j <= 70; j++) {
        double s_r, s_i, c_r, c_i;
        c_sin((double)j * x1r, (double)j * x1i, &s_r, &s_i);
        c_cos((double)j * x2r, (double)j * x2i, &c_r, &c_i);
        cRe[j] = (double)(j - 35) * s_r - (double)(j - 34) * c_r;
        cIm[j] = (double)(j - 35) * s_i - (double)(j - 34) * c_i;
    }

    double t1sq_r, t1sq_i, t2sq_r, t2sq_i, prod_r, prod_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    cRe[70] = t1sq_r + t2sq_r + prod_r;
    cIm[70] = t1sq_i + t2sq_i + prod_i;

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_545_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double rec[35], imc[35];
    for (int i = 0; i < 35; i++) {
        rec[i] = x1r + (x2r - x1r) * (double)i / 34.0;
        imc[i] = x1i + (x2i - x1i) * (double)i / 34.0;
    }
    const double log_abs1 = log(c_abs(x1r, x1i) + 1.0);
    const double ang1 = c_arg(x1r, x1i);
    const double ang2 = c_arg(x2r, x2i);
    const double diff_abs = c_abs(x1r - x2r, x1i - x2i);
    const double log_sum = log(c_abs(x1r + x2r, x1i + x2i) + 1.0);
    const double conj1_r = x1r, conj1_i = -x1i;
    for (int j = 1; j <= 35; j++) {
        const double rv = rec[j - 1];
        const double iv = imc[j - 1];
        const double mag = (rv * rv * rv + iv * iv) * log_abs1
                         + sin((double)j * M_PI / 4.0) * cos((double)j * M_PI / 3.0);
        const double ang = ang1 + ang2 * (double)j + sin((double)j * x1r * x2i);
        double phase_r, phase_i, pow_r, pow_i;
        c_exp2(0.0, ang, &phase_r, &phase_i);
        if ((j % 5) == 0) {
            pow_r = 1.0;
            pow_i = 0.0;
        } else {
            c_powr(conj1_r, conj1_i, (double)(j % 5), &pow_r, &pow_i);
        }
        const double cs = cos((double)j * iv);
        cRe[j - 1] = mag * phase_r + pow_r * cs;
        cIm[j - 1] = mag * phase_i + pow_i * cs;
    }
    for (int k = 1; k <= 35; k++) {
        double prod = 1.0;
        for (int i = 0; i < k; i++) prod *= (rec[i] + imc[i]);
        const double bump = prod * sin((double)k * ang2);
        cRe[k - 1] += bump;
    }
    for (int r = 1; r <= 35; r++) {
        const double scale = 1.0 + diff_abs / (double)(r + 1);
        cRe[r - 1] = cRe[r - 1] * scale + log_sum * cos((double)r * ang1);
        cIm[r - 1] = cIm[r - 1] * scale;
        if (!isfinite(cRe[r - 1]) || !isfinite(cIm[r - 1])) { cRe[r - 1] = 0; cIm[r - 1] = 0; }
    }
}

static void poly_556_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    double rec[35], imc[35];
    for (int i = 0; i < 35; i++) {
        rec[i] = x1r + (x2r - x1r) * (double)i / 34.0;
        imc[i] = x1i + (x2i - x1i) * (double)i / 34.0;
    }
    const double ang_sum = c_arg(x1r + x2r, x1i + x2i);
    for (int j = 1; j <= 35; j++) {
        double prod = 1.0;
        for (int i = 0; i < j; i++) prod *= (rec[i] + imc[i]);
        const double angle = sin((double)j * rec[j - 1]) + cos((double)j * imc[j - 1]) + ang_sum;
        const double magnitude = log(fabs(rec[j - 1] * rec[j - 1] + imc[j - 1] * imc[j - 1]) + 1.0)
                               * (pow((double)j, 1.5) + prod);
        cRe[j - 1] = magnitude * cos(angle);
        cIm[j - 1] = magnitude * sin(angle);
        if (!isfinite(cRe[j - 1]) || !isfinite(cIm[j - 1])) { cRe[j - 1] = 0; cIm[j - 1] = 0; }
    }
}

static void poly_596_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 26;
    for (int i = 0; i < 26; i++) { cRe[i] = 0; cIm[i] = 0; }

    const double abs1 = c_abs(x1r, x1i);
    const double ang1 = c_arg(x1r, x1i);
    const double ang2 = c_arg(x2r, x2i);
    for (int j = 1; j <= 25; j++) {
        const int k = (j * 3 + 7) % 10;
        const double r = x1r * sin((double)j) + x2i * cos((double)k);
        const double mag = log(abs1 + (double)(j * j)) * sin((double)k * M_PI / 4.0) + cos(r);
        const double angle = ang1 * cos((double)j) + sin((double)k * ang2);
        double conjp_r, conjp_i, phase_r, phase_i;
        if (k == 0) {
            conjp_r = 1.0;
            conjp_i = 0.0;
        } else {
            c_powr(x2r, -x2i, (double)k, &conjp_r, &conjp_i);
        }
        c_exp2(0.0, angle, &phase_r, &phase_i);
        cRe[j - 1] = mag * phase_r + conjp_r;
        cIm[j - 1] = mag * phase_i + conjp_i;
    }
    double sum = 0.0;
    for (int i = 0; i < 25; i++) sum += c_abs(cRe[i], cIm[i]) * cos((double)(i + 1) * M_PI / 6.0);
    cRe[25] = sum + c_abs(x1r, x1i) * c_abs(x2r, x2i);

    for (int i = 0; i < 26; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_749_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 10;
    for (int i = 0; i < 10; i++) { cRe[i] = 0; cIm[i] = 0; }

    double sum_r = x1r + x2r, sum_i = x1i + x2i;
    double diff_r = x1r - x2r, diff_i = x1i - x2i;
    c_exp2(-sum_i, sum_r, &cRe[0], &cIm[0]);

    double s1_r, s1_i, c2_r, c2_i, c1_r, c1_i, s2_r, s2_i;
    c_sin(x1r, x1i, &s1_r, &s1_i);
    c_cos(x2r, x2i, &c2_r, &c2_i);
    c_cos(x1r, x1i, &c1_r, &c1_i);
    c_sin(x2r, x2i, &s2_r, &s2_i);
    double a_r, a_i, b_r, b_i;
    c_mul(s1_r, s1_i, c2_r, c2_i, &a_r, &a_i);
    c_mul(c1_r, c1_i, s2_r, s2_i, &b_r, &b_i);
    cRe[1] = a_r - b_r;
    cIm[1] = a_i - b_i;

    c_sin(sum_r, sum_i, &s1_r, &s1_i);
    for (int j = 2; j < 9; j++) {
        const double scale = exp((double)(j + 1) / 3.0);
        c_exp2(diff_i / (double)(j + 1), -diff_r / (double)(j + 1), &c2_r, &c2_i);
        c_mul(scale * s1_r, scale * s1_i, c2_r, c2_i, &cRe[j], &cIm[j]);
    }

    double t1sq_r, t1sq_i, t2sq_r, t2sq_i, sumsq_r, sumsq_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    sumsq_r = t1sq_r + t2sq_r;
    sumsq_i = t1sq_i + t2sq_i;
    c_powr(sumsq_r, sumsq_i, 0.5, &cRe[9], &cIm[9]);
    cRe[9] -= log(c_abs(x1r, x1i) + c_abs(x2r, x2i)) / log(10.0);

    for (int i = 0; i < 10; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_758_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 10;
    for (int i = 0; i < 10; i++) { cRe[i] = 0; cIm[i] = 0; }

    double t1sq_r, t1sq_i, t2sq_r, t2sq_i, sum_r, sum_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    sum_r = t1sq_r + t2sq_r;
    sum_i = t1sq_i + t2sq_i;
    double phase_r, phase_i;
    c_exp2(0.0, c_arg(x1r + x2r, x1i + x2i), &phase_r, &phase_i);
    c_mul(sum_r, sum_i, phase_r, phase_i, &cRe[0], &cIm[0]);

    double t1cu_r, t1cu_i, diff_r = x1r - x2r, diff_i = x1i - x2i;
    c_mul(t1sq_r, t1sq_i, x1r, x1i, &t1cu_r, &t1cu_i);
    const double scale = cos(c_arg(diff_r, diff_i));
    const double diff2_r = t1cu_r - t2sq_r;
    const double diff2_i = t1cu_i - t2sq_i;
    cRe[1] = -100.0 * scale * diff2_i;
    cIm[1] = 100.0 * scale * diff2_r;

    cRe[2] = diff_r * diff_r + (x1i + x2i) * (x1i + x2i) - 100.0;
    const double gain = 42.0 * (log1p(c_abs(x1r, x1i)) + log1p(c_abs(x2r, x2i)));
    c_exp2(0.0, M_PI / 4.0, &phase_r, &phase_i);
    cRe[3] = gain * phase_r;
    cIm[3] = gain * phase_i;

    const double root = sqrt(c_abs(x1r, x1i) + c_abs(x2r, x2i));
    c_mul(x1r, x1i, x2r, x2i, &sum_r, &sum_i);
    c_exp2(0.0, c_arg(sum_r, sum_i) - M_PI / 3.0, &phase_r, &phase_i);
    cRe[4] = root * phase_r;
    cIm[4] = root * phase_i;

    cRe[5] = sinh(0.1 * (x1r + x2r)) + cosh(0.1 * (x1i - x2i));
    cRe[6] = 1.0 / (1.0 + exp(-c_abs(x1r, x1i)));
    cIm[6] = 1.0 / (1.0 + exp(-c_abs(x2r, x2i)));
    cIm[7] = atan(1.0 / c_abs(x1r + x2r, x1i + x2i));
    cRe[8] = -t1cu_i - 2.0 * x2r - 200.0;
    cIm[8] = t1cu_r - 2.0 * x2i;

    c_mul(x1r, x1i, x2r, x2i, &sum_r, &sum_i);
    c_exp2(-sum_i, sum_r, &phase_r, &phase_i);
    c_div(phase_r, phase_i, 1.0 + x1r + x2r, x1i + x2i, &cRe[9], &cIm[9]);

    for (int i = 0; i < 10; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_787_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int i = 0; i < 25; i++) { cRe[i] = 0; cIm[i] = 0; }

    double prod_r, prod_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    cRe[0] = x1r + x2r;
    cIm[0] = prod_i;
    const double diff_r = x1r - x2r;
    const double diff_i = x1i - x2i;
    for (int i = 1; i < 24; i++) {
        double pow_r, pow_i;
        c_powc(cRe[i - 1], cIm[i - 1], diff_r, diff_i, &pow_r, &pow_i);
        cRe[i] = c_abs(pow_r, pow_i);
        cIm[i] = c_arg(cRe[i - 1], cIm[i - 1]);
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) {
            cRe[i] = 0.0;
            cIm[i] = 1.0;
        }
    }
    double cf0sq_r, cf0sq_i, cf0cu_r, cf0cu_i, cf23sq_r, cf23sq_i;
    c_mul(cRe[0], cIm[0], cRe[0], cIm[0], &cf0sq_r, &cf0sq_i);
    c_mul(cf0sq_r, cf0sq_i, cRe[0], cIm[0], &cf0cu_r, &cf0cu_i);
    c_mul(cRe[23], cIm[23], cRe[23], cIm[23], &cf23sq_r, &cf23sq_i);
    cRe[24] = cf0cu_r + cf23sq_r + cRe[22] - cRe[21] + cRe[20];
    cIm[24] = cf0cu_i + cf23sq_i + cIm[22] - cIm[21] - cIm[20];
}

static void poly_809_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int i = 0; i < 25; i++) { cRe[i] = 0; cIm[i] = 0; }

    cRe[0] = x1r + x2r;
    cIm[0] = x1i + x2i;
    const double denom = c_abs(1.0 + x1r, x1i);
    const double term_r = (denom == 0.0) ? INFINITY : x2r / denom;
    const double term_i = (denom == 0.0) ? INFINITY : -x2i / denom;
    for (int k = 1; k < 25; k++) {
        cRe[k] = cos((double)k * c_arg(cRe[k - 1], cIm[k - 1])) + sin((double)k * c_abs(x1r, x1i)) + term_r;
        cIm[k] = term_i;
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) {
            cRe[k] = cRe[k - 1];
            cIm[k] = cIm[k - 1];
        }
    }

    double a_r, a_i, b_r, b_i, m_r, m_i;
    c_mul(cRe[0], cIm[0], cRe[0], cIm[0], &a_r, &a_i);
    c_mul(a_r, a_i, cRe[0], cIm[0], &a_r, &a_i);
    c_mul(cRe[1], cIm[1], cRe[1], cIm[1], &b_r, &b_i);
    c_mul(cRe[0], cIm[0], cRe[1], cIm[1], &m_r, &m_i);
    cRe[9] = a_r + b_r - m_r;
    cIm[9] = a_i + b_i - m_i;
    if (!isfinite(cRe[9]) || !isfinite(cIm[9])) { cRe[9] = cRe[8]; cIm[9] = cIm[8]; }

    c_mul(x1r, x1i, x1r, x1i, &a_r, &a_i);
    c_mul(x2r, x2i, x2r, x2i, &b_r, &b_i);
    cRe[14] = log(c_abs(cRe[13], cIm[13])) - a_r + b_r;
    cIm[14] = -a_i + b_i;
    if (!isfinite(cRe[14]) || !isfinite(cIm[14])) { cRe[14] = cRe[13]; cIm[14] = cIm[13]; }

    double sum_r = x1r + x2r, sum_i = x1i + x2i;
    c_mul(sum_r, sum_i, sum_r, sum_i, &a_r, &a_i);
    c_mul(cRe[0], cIm[0], a_r, a_i, &a_r, &a_i);
    const double den2 = 1.0 + c_abs(x1r * x2r - x1i * x2i, x1r * x2i + x1i * x2r);
    cRe[19] = a_r - cRe[2] / den2;
    cIm[19] = a_i - cIm[2] / den2;
    if (!isfinite(cRe[19]) || !isfinite(cIm[19])) { cRe[19] = cRe[18]; cIm[19] = cIm[18]; }

    c_mul(sum_r, sum_i, sum_r, sum_i, &a_r, &a_i);
    c_mul(a_r, a_i, sum_r, sum_i, &a_r, &a_i);
    cRe[24] = a_r - cRe[23];
    cIm[24] = a_i - cIm[23];
    if (!isfinite(cRe[24]) || !isfinite(cIm[24])) { cRe[24] = cRe[23]; cIm[24] = cIm[23]; }
}

static void poly_818_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int i = 0; i < 25; i++) { cRe[i] = 0; cIm[i] = 0; }

    const double a0_r = x1r - x2i;
    const double a0_i = x1i + x2r;
    const double b0_r = x2r - x1i;
    const double b0_i = x2i + x1r;
    const double ang_a = c_arg(a0_r, a0_i);
    const double ang_b = c_arg(b0_r, b0_i);
    for (int k = 1; k <= 25; k++) {
        const double shift = 0.5 + (double)k / 25.0;
        const double amp_r = c_abs(a0_r + shift, a0_i);
        const double amp_i = c_abs(b0_r + shift, b0_i);
        const double p1 = pow(ang_a, (double)(k - 1));
        const double p2 = pow(ang_b, (double)(k - 1));
        cRe[k - 1] = amp_r * cos(p1);
        cIm[k - 1] = amp_i * sin(p2);
        if (!isfinite(cRe[k - 1]) || !isfinite(cIm[k - 1])) {
            cRe[k - 1] = 0.0;
            cIm[k - 1] = 0.0;
        }
    }
}

static void poly_101_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0.0; cIm[i] = 0.0; }

    for (int i = 0; i < 10; i++) cRe[i] = 1000.0 * (double)(i + 1);
    for (int i = 10; i < 71; i++) cRe[i] = 1.0;

    const double abs1 = c_abs(x1r, x1i);
    const double abs2 = c_abs(x2r, x2i);
    const double ang1 = c_arg(x1r, x1i);
    const double ang2 = c_arg(x2r, x2i);
    cRe[14] = -pow(abs1, 3.0);
    cRe[29] = -pow(abs2, 4.0);
    cRe[44] = pow(abs1, 5.0);
    cRe[59] = pow(abs2, 6.0);
    cRe[19] = abs1 * abs1 * sin(ang1);
    cRe[39] = pow(abs2, 3.0) * cos(ang2);
    cRe[24] = log(abs1 + 1.0) * abs1;
    cRe[49] = log(abs2 + 1.0) * abs2;

    for (int j = 1; j < 35; j++) {
        const int even_idx = 2 * j;
        const int odd_idx = even_idx + 1;
        const double even_base = cRe[even_idx];
        const double odd_base = cRe[odd_idx];
        double s1r, s1i, c1r, c1i, s2r, s2i, c2r, c2i;
        c_sin((double)j * x1r, (double)j * x1i, &s1r, &s1i);
        c_cos((double)j * x1r, (double)j * x1i, &c1r, &c1i);
        c_sin((double)j * x2r, (double)j * x2i, &s2r, &s2i);
        c_cos((double)j * x2r, (double)j * x2i, &c2r, &c2i);
        cRe[even_idx] = even_base * (s1r + c2r) + odd_base * (c1r + s2r);
        cIm[even_idx] = even_base * (s1i + c2i) + odd_base * (c1i + s2i);
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0.0; cIm[i] = 0.0; }
    }
}

static void poly_106_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0.0; cIm[i] = 0.0; }

    double t1sq_r, t1sq_i, t1cu_r, t1cu_i, t2sq_r, t2sq_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(t1sq_r, t1sq_i, x1r, x1i, &t1cu_r, &t1cu_i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    double sum_r = t1cu_r + t2sq_r;
    double sum_i = t1cu_i + t2sq_i;
    double base_r, base_i;
    c_mul(sum_r, sum_i, sum_r, sum_i, &base_r, &base_i);
    const double diff_abs = c_abs(x1r - x2r, x1i - x2i);
    double prod_r, prod_i, sin_r, sin_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    c_sin(prod_r, prod_i, &sin_r, &sin_i);
    base_r += diff_abs + sin_r;
    base_i += sin_i;

    const double abs_sum = c_abs(x1r + x2r, x1i + x2i);
    for (int k = 1; k <= 71; k++) {
        const double scale = pow(abs_sum, 1.0 / (double)k);
        cRe[k - 1] = base_r * scale;
        cIm[k - 1] = base_i * scale;
    }

    const double scales[10] = {100.0, 90.0, 80.0, 70.0, 60.0, 50.0, 40.0, 30.0, 20.0, 10.0};
    for (int i = 0; i < 10; i++) {
        cRe[i] *= scales[i];
        cIm[i] *= scales[i];
    }
    cRe[11] *= 5.0; cIm[11] *= 5.0;
    cRe[23] *= 4.0; cIm[23] *= 4.0;
    cRe[35] *= 3.0; cIm[35] *= 3.0;
    cRe[47] *= 2.0; cIm[47] *= 2.0;

    for (int k = 15; k < 71; k++) {
        const double factor = -log((double)k);
        cRe[k] *= factor;
        cIm[k] *= factor;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0.0; cIm[i] = 0.0; }
    }
}

static void poly_121_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0.0; cIm[i] = 0.0; }

    const double theta = 2.0 * M_PI / 7.0;
    const double wr = cos(theta);
    const double wi = sin(theta);
    double div_r, div_i;
    c_div(x2r, x2i, wr, wi, &div_r, &div_i);
    double p1r = 1.0, p1i = 0.0, p2r = 1.0, p2i = 0.0;
    for (int k = 1; k <= 7; k++) {
        double nr, ni;
        c_mul(p1r, p1i, x1r, x1i, &nr, &ni);
        p1r = nr; p1i = ni;
        c_mul(p2r, p2i, div_r, div_i, &nr, &ni);
        p2r = nr; p2i = ni;
        const double wkr = cos(theta * (double)k);
        cRe[k - 1] = (p1r - p2r) * wkr;
        cIm[k - 1] = (p1i - p2i) * wkr;
    }

    double t1sq_r, t1sq_i, t2sq_r, t2sq_i, prod_r, prod_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    cRe[7] = -(t1sq_r + t2sq_r) + prod_r + prod_i;
    cIm[7] = -(t1sq_i + t2sq_i);

    const double z = c_arg(x1r, x1i) + c_arg(x2r, x2i);
    for (int k = 9; k < 36; k++) {
        cRe[k - 1] = cos((double)k * z);
        cIm[k - 1] = sin((double)k * z);
    }

    const double abs1 = c_abs(x1r, x1i);
    double base_r = abs1 * x2r + x2i * x1r;
    double base_i = abs1 * x2i + x2i * x1i;
    double sq_r, sq_i;
    c_mul(base_r, base_i, base_r, base_i, &sq_r, &sq_i);
    for (int k = 36; k < 71; k++) {
        cRe[k - 1] = sq_r / (double)(k + 1);
        cIm[k - 1] = sq_i / (double)(k + 1);
    }

    cRe[70] = abs1 - c_abs(x2r, x2i) + log(c_abs(x1r + x2r + 1.0, x1i + x2i) + 1.0);

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0.0; cIm[i] = 0.0; }
    }
}

static void poly_149_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0.0; cIm[i] = 0.0; }

    double t1sq_r, t1sq_i, t1cu_r, t1cu_i, t1_4r, t1_4i, t2sq_r, t2sq_i, t2cu_r, t2cu_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(t1sq_r, t1sq_i, x1r, x1i, &t1cu_r, &t1cu_i);
    c_mul(t1cu_r, t1cu_i, x1r, x1i, &t1_4r, &t1_4i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    c_mul(t2sq_r, t2sq_i, x2r, x2i, &t2cu_r, &t2cu_i);
    cRe[0] = t1_4r + 2.0 * t2cu_r;
    cIm[0] = t1_4i + 2.0 * t2cu_i;
    cRe[1] = -t1cu_r + 3.0 * t2sq_r;
    cIm[1] = -t1cu_i + 3.0 * t2sq_i;
    cRe[2] = t1sq_r - 4.0 * x2r;
    cIm[2] = t1sq_i - 4.0 * x2i;
    cRe[3] = -x1r + 5.0 * t2sq_r;
    cIm[3] = -x1i + 5.0 * t2sq_i;
    double mix_r, mix_i;
    c_mul(t1sq_r, t1sq_i, x2r, x2i, &mix_r, &mix_i);
    cRe[4] = t2cu_r - 6.0 * mix_r;
    cIm[4] = t2cu_i - 6.0 * mix_i;

    for (int j = 6; j < 36; j++) {
        const double sign = (j % 2 == 0) ? 1.0 : -1.0;
        cRe[j - 1] = (pow(x1r, (double)j) - pow(x2i, (double)j)) * sign / (double)j;
    }
    for (int j = 36; j < 71; j++) {
        double s1r, s1i, c2r, c2i;
        c_sin((double)j * x1r, (double)j * x1i, &s1r, &s1i);
        c_cos((double)j * x2r, (double)j * x2i, &c2r, &c2i);
        cRe[j - 1] = (s1r + c2r) / (double)(j + 1);
        cIm[j - 1] = (s1i + c2i) / (double)(j + 1);
    }
    const double scalar = x1r + x2i;
    cRe[70] = scalar * (x1r - x2r);
    cIm[70] = scalar * (x1i - x2i);

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0.0; cIm[i] = 0.0; }
    }
}

/* ---- poly_giga_62_hand_c ----
 * Python:
 *   cf = np.zeros(25, dtype=complex)
 *   cf[0:5] = np.array([abs(t1 + t2)**(i+1) for i in range(5)])
 *   cf[5:10] = ((t1+2j*t2)**3).real * np.log(np.abs(np.conj(t1*t2)))
 *   cf[10:15] = ((t1-t2)**2).imag / np.angle(t1*t2)
 *   cf[15:20] = np.abs(cf[5:10])**0.5 + np.angle(cf[0:5])
 *   cf[20:25] = np.array([abs(t1 * t2)**(i+1) for i in range(5)])
 *   return cf.astype(np.complex128)
 *
 * Notes:
 *   - cf[5:10] and cf[10:15] are scalar-to-slice broadcasts in Python.
 *   - cf[0:5] and cf[20:25] come from list comprehensions in slice assignments.
 *   - np.angle(cf[0:5]) is always 0 here because cf[0:5] are nonnegative reals.
 */
static void poly_giga_62_hand_c(double x1r, double x1i, double x2r, double x2i,
                                const double *cfpv, int n_cfpv,
                                double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int i = 0; i < 25; i++) { cRe[i] = 0; cIm[i] = 0; }

    double sumr = x1r + x2r;
    double sumi = x1i + x2i;
    double abs_sum = c_abs(sumr, sumi);
    for (int i = 0; i < 5; i++) {
        cRe[i] = pow(abs_sum, (double)(i + 1));
    }

    /* z = t1 + 2j*t2 = (x1r - 2*x2i) + i(x1i + 2*x2r) */
    double zr = x1r - 2.0 * x2i;
    double zi = x1i + 2.0 * x2r;
    double z2r, z2i, z3r, z3i;
    c_mul(zr, zi, zr, zi, &z2r, &z2i);
    c_mul(z2r, z2i, zr, zi, &z3r, &z3i);

    double prodr, prodi;
    c_mul(x1r, x1i, x2r, x2i, &prodr, &prodi);
    double abs_prod = c_abs(prodr, prodi);
    double log_abs_prod = log(abs_prod);
    double broadcast_5_10 = z3r * log_abs_prod;
    for (int i = 5; i < 10; i++) {
        cRe[i] = broadcast_5_10;
    }

    double diffr = x1r - x2r;
    double diffi = x1i - x2i;
    double diff2r, diff2i;
    c_mul(diffr, diffi, diffr, diffi, &diff2r, &diff2i);
    double denom = c_arg(prodr, prodi);
    double broadcast_10_15 = diff2i / denom;
    for (int i = 10; i < 15; i++) {
        cRe[i] = broadcast_10_15;
    }

    double broadcast_15_20 = sqrt(fabs(broadcast_5_10));
    for (int i = 15; i < 20; i++) {
        cRe[i] = broadcast_15_20;
    }

    for (int i = 0; i < 5; i++) {
        cRe[20 + i] = pow(abs_prod, (double)(i + 1));
    }

    for (int i = 0; i < 25; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_33_hand ----
 * Python:
 *   f = lambda z, n: z**n - 1
 *   cf[0:35]  = [Re(f(t1,n)) - Im(f(t2,n)) for n in range(1,36)]
 *   cf[35:70] = [log(|f(t2,n)|) + angle(f(t1,n)) + sin(|f(t1,n)|) + cos(angle(f(t2,n))) for n in range(1,36)]
 *   cf[70]    = prod(cf[0:70])
 *   NaN/inf → 0
 */
static void poly_33_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* Precompute powers: t1^n and t2^n iteratively */
    /* f(z, n) = z^n - 1  →  fRe = powRe - 1, fIm = powIm */
    double pow1r = 1, pow1i = 0;  /* t1^0 */
    double pow2r = 1, pow2i = 0;  /* t2^0 */

    for (int n = 1; n <= 35; n++) {
        /* pow1 *= t1 */
        double nr, ni;
        c_mul(pow1r, pow1i, x1r, x1i, &nr, &ni);
        pow1r = nr; pow1i = ni;
        /* pow2 *= t2 */
        c_mul(pow2r, pow2i, x2r, x2i, &nr, &ni);
        pow2r = nr; pow2i = ni;

        /* f(t1, n) = t1^n - 1 */
        double f1r = pow1r - 1, f1i = pow1i;
        /* f(t2, n) = t2^n - 1 */
        double f2r = pow2r - 1, f2i = pow2i;

        /* cf[n-1] = Re(f(t1,n)) - Im(f(t2,n)) */
        cRe[n - 1] = f1r - f2i;
        cIm[n - 1] = 0;

        /* cf[34+n] = log(|f(t2,n)|) + angle(f(t1,n)) + sin(|f(t1,n)|) + cos(angle(f(t2,n))) */
        double abs_f2 = c_abs(f2r, f2i);
        double ang_f1 = c_arg(f1r, f1i);
        double abs_f1 = c_abs(f1r, f1i);
        double ang_f2 = c_arg(f2r, f2i);
        cRe[34 + n] = log(abs_f2) + ang_f1 + sin(abs_f1) + cos(ang_f2);
        cIm[34 + n] = 0;
    }

    /* cf[70] = prod(cf[0:70]) — real product since all coefficients are real */
    double prodR = cRe[0], prodI = 0;
    for (int i = 1; i < 70; i++) {
        /* complex multiply: (prodR + i*prodI) * (cRe[i] + i*cIm[i]) */
        double pr, pi;
        c_mul(prodR, prodI, cRe[i], cIm[i], &pr, &pi);
        prodR = pr; prodI = pi;
    }
    cRe[70] = prodR; cIm[70] = prodI;

    /* NaN guard */
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_55_hand ----
 * Python:
 *   cf[0]     = t1 * t2
 *   cf[1]     = Re(t1) * Im(t2)
 *   cf[2]     = Re(t2) * Im(t1)
 *   cf[3:10]  = linspace(log(|cf[0]|+1), log(|cf[2]|+1), 7)
 *   cf[10:30] = [cos(cf[i-1]) + (t1+t2)^i / (i+1) for i in range(11,31)]
 *   cf[30:50] = [sin(cf[i-1]) + (t1-t2)^i / (i+1) for i in range(31,51)]
 *   cf[50:70] = |cf[0:20]| + |cf[20:40] + t1 + t2|
 *   cf[70]    = prod(cf[0:70])
 */
static void poly_55_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* cf[0] = t1 * t2 */
    double c0r, c0i;
    c_mul(x1r, x1i, x2r, x2i, &c0r, &c0i);
    cRe[0] = c0r; cIm[0] = c0i;

    /* cf[1] = Re(t1) * Im(t2) — real */
    cRe[1] = x1r * x2i; cIm[1] = 0;

    /* cf[2] = Re(t2) * Im(t1) — real */
    cRe[2] = x2r * x1i; cIm[2] = 0;

    /* cf[3:10] = linspace(log(|cf[0]|+1), log(|cf[2]|+1), 7) — real */
    double start_val = log(c_abs(cRe[0], cIm[0]) + 1);
    double end_val = log(c_abs(cRe[2], cIm[2]) + 1);
    for (int k = 0; k < 7; k++) {
        double t = (7 > 1) ? (double)k / 6.0 : 0.0;
        cRe[3 + k] = start_val + t * (end_val - start_val);
        cIm[3 + k] = 0;
    }

    /* Precompute sum = t1+t2, diff = t1-t2 */
    double sR = x1r + x2r, sI = x1i + x2i;
    double dR = x1r - x2r, dI = x1i - x2i;

    /* cf[10:30] = [cos(cf[i-1]) + (t1+t2)^i / (i+1) for i in range(11,31)]
     * Here i ranges 11..30, so cf[i-1] = cf[10..29] (the one just computed before).
     * But cf[10..29] ARE the values being computed! The Python list comprehension
     * uses cf[i-1] which references previously-set values in the SAME array.
     * For i=11: cf[10] = cos(cf[10]) + ... but cf[10] was just set above...
     * Actually cf[10] was 0 (from zeros init), so cf[10-1] = cf[10] = 0 at that point.
     * Wait — cf[i-1] for i=11 is cf[10]. But cf[10] hasn't been assigned yet in
     * the list comprehension (it's building a list). So it reads cf[10] = 0.
     * Actually no — in Python, cf[10:30] = [expr for i in range(11,31)]
     * The list is fully built BEFORE assignment. So cf[i-1] reads from the
     * ORIGINAL array, which has cf[10..29] = 0 at that point.
     * So cf[i-1] for i=11..30 reads cf[10..29] which are all 0.
     */
    /* (t1+t2)^i for i=11..30: build incrementally */
    double spowR = 1, spowI = 0;
    for (int i = 1; i <= 30; i++) {
        double nr, ni;
        c_mul(spowR, spowI, sR, sI, &nr, &ni);
        spowR = nr; spowI = ni;
        if (i >= 11) {
            /* cos(cf[i-1]) — cf[i-1] is 0 for i=11..30, so cos(0) = 1 for real part */
            /* But cf[i-1] is complex. cos(complex) = cos(re)*cosh(im) + i*(-sin(re)*sinh(im)) */
            /* Since cf[i-1] = 0+0i, cos(0) = 1+0i */
            double prev_r = 0, prev_i = 0; /* cf[i-1] was 0 before this assignment */
            double cos_r = cos(prev_r) * cosh(prev_i);
            double cos_i = -sin(prev_r) * sinh(prev_i);
            /* (t1+t2)^i / (i+1) */
            double div = 1.0 / (i + 1);
            cRe[i - 1] = cos_r + spowR * div;
            cIm[i - 1] = cos_i + spowI * div;
        }
    }

    /* cf[30:50] = [sin(cf[i-1]) + (t1-t2)^i / (i+1) for i in range(31,51)]
     * Same logic: list is built before assignment. cf[30..49] are 0 at read time.
     * EXCEPT cf[30] was possibly set... no, cf[30:50] replaces cf[30..49].
     * Actually cf[i-1] for i=31 is cf[30], which WAS set by the cf[10:30] assignment.
     * Wait: cf[10:30] sets cf[10]..cf[29]. For i=31: cf[30] was still 0.
     * For i=32: cf[31] was still 0. All cf[30..49] are 0 at list-build time.
     */
    double dpowR = 1, dpowI = 0;
    for (int i = 1; i <= 50; i++) {
        double nr, ni;
        c_mul(dpowR, dpowI, dR, dI, &nr, &ni);
        dpowR = nr; dpowI = ni;
        if (i >= 31) {
            /* sin(cf[i-1]) where cf[i-1] = 0+0i → sin(0) = 0+0i */
            double prev_r = 0, prev_i = 0;
            double sin_r = sin(prev_r) * cosh(prev_i);
            double sin_i = cos(prev_r) * sinh(prev_i);
            double div = 1.0 / (i + 1);
            cRe[i - 1] = sin_r + dpowR * div;
            cIm[i - 1] = sin_i + dpowI * div;
        }
    }

    /* cf[50:70] = |cf[0:20]| + |cf[20:40] + t1 + t2| */
    for (int k = 0; k < 20; k++) {
        double a = c_abs(cRe[k], cIm[k]);
        double bR = cRe[20 + k] + sR;
        double bI = cIm[20 + k] + sI;
        double b = c_abs(bR, bI);
        cRe[50 + k] = a + b;
        cIm[50 + k] = 0;
    }

    /* cf[70] = prod(cf[0:70]) */
    double prodR = cRe[0], prodI = cIm[0];
    for (int i = 1; i < 70; i++) {
        double pr, pi;
        c_mul(prodR, prodI, cRe[i], cIm[i], &pr, &pi);
        prodR = pr; prodI = pi;
    }
    cRe[70] = prodR; cIm[70] = prodI;

    /* NaN guard */
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_58_hand ----
 * Python:
 *   cf[0] = 100*|t1|*|t2| - 100
 *   cf[1] = 100*t1^3 - t2^3 + 100
 *   cf[2] = 100*t2^3 - t1^3 + 100
 *   cf[3:71] = [cos(k*t1)*sin(k*t2)/log(|k+1|) for k in range(1,69)]
 *   root_coeff = |t1|*|t2| * prod(1..70) / sum(1..70)  (= |t1|*|t2| * 70! / 2485)
 *   cf[4] = root_coeff * sum(cos(k*t1)*sin(k*t2)/log(|k+1|) for k=1..70)
 *   cf[35] = root_coeff * t1^2
 *   cf[34] = root_coeff * t2^2
 *   cf[36:71] = root_coeff * [cos(2k*t1)*sin(2k*t2)/log(|2k+1|) for k=1..35]
 */
static void poly_58_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double absT1 = c_abs(x1r, x1i);
    double absT2 = c_abs(x2r, x2i);

    /* cf[0] = 100*|t1|*|t2| - 100 — real */
    cRe[0] = 100.0 * absT1 * absT2 - 100.0;

    /* t1^3, t2^3 */
    double t1_2r, t1_2i; c_mul(x1r, x1i, x1r, x1i, &t1_2r, &t1_2i);
    double t1_3r, t1_3i; c_mul(t1_2r, t1_2i, x1r, x1i, &t1_3r, &t1_3i);
    double t2_2r, t2_2i; c_mul(x2r, x2i, x2r, x2i, &t2_2r, &t2_2i);
    double t2_3r, t2_3i; c_mul(t2_2r, t2_2i, x2r, x2i, &t2_3r, &t2_3i);

    /* cf[1] = 100*t1^3 - t2^3 + 100 */
    cRe[1] = 100.0 * t1_3r - t2_3r + 100.0;
    cIm[1] = 100.0 * t1_3i - t2_3i;

    /* cf[2] = 100*t2^3 - t1^3 + 100 */
    cRe[2] = 100.0 * t2_3r - t1_3r + 100.0;
    cIm[2] = 100.0 * t2_3i - t1_3i;

    /* cf[3:71] = [cos(k*t1)*sin(k*t2)/log(|k+1|) for k in range(1,69)]
     * cos(k*t1) is complex cos: cos(a+bi) = cos(a)cosh(b) - i*sin(a)sinh(b)
     * sin(k*t2) is complex sin: sin(a+bi) = sin(a)cosh(b) + i*cos(a)sinh(b)
     */
    for (int k = 1; k <= 68; k++) {
        double kt1r = k * x1r, kt1i = k * x1i;
        double kt2r = k * x2r, kt2i = k * x2i;
        /* cos(kt1) */
        double cosr = cos(kt1r) * cosh(kt1i);
        double cosi = -sin(kt1r) * sinh(kt1i);
        /* sin(kt2) */
        double sinr = sin(kt2r) * cosh(kt2i);
        double sini = cos(kt2r) * sinh(kt2i);
        /* cos * sin */
        double pr, pi;
        c_mul(cosr, cosi, sinr, sini, &pr, &pi);
        double logk = log((double)(k + 1));
        cRe[3 + k - 1] = pr / logk;
        cIm[3 + k - 1] = pi / logk;
    }

    /* root_coeff = |t1|*|t2| * prod(1..70) / sum(1..70)
     * prod(1..70) = 70! which overflows double. Python returns inf.
     * sum(1..70) = 2485
     * So root_coeff = |t1|*|t2| * inf / 2485 = inf (or NaN).
     * After NaN guard in Python, this makes cf[4], cf[34], cf[35], cf[36:71]
     * all get set but the root_coeff*... values will be inf → caught by NaN guard → 0.
     * Wait, Python catches this in try/except → returns zeros.
     * Actually let's compute it: 70! overflows double to inf.
     * |t1|*|t2| * inf = inf (if non-zero) or nan (if zero).
     * So any term with root_coeff will be inf/nan → zeroed by NaN guard.
     * But the cf[3:71] assignment happened BEFORE root_coeff overwrites.
     * So the final state is: cf[3:71] from the first loop, but cf[4], cf[34], cf[35]
     * overwritten to 0 (via inf→NaN guard), and cf[36:71] overwritten to 0.
     *
     * Actually: Python evaluates sequentially:
     * 1. cf[3:71] = [...] — sets cf[3]..cf[70]
     * 2. root_coeff = inf
     * 3. cf[4] = inf * sum(...) = inf → caught by except → return zeros
     *
     * Hmm, but the except catches ALL exceptions. np.prod(range(1,71)) doesn't
     * raise, it returns inf. inf * sum() = inf. No exception.
     * So cf[4] = inf. Then cf[35] = inf. cf[34] = inf. cf[36:71] = inf*[...].
     * return cf.astype(...) — no exception.
     * But they have NaN guard? No, the Python doesn't have explicit NaN guard.
     * Wait, the original Python has try/except. So only if an exception occurs.
     * Since inf is not an exception, the function returns cf with inf values.
     * The C NaN guard will zero those out. Let's do the same.
     */
    /* prod(range(1,71)) = 70! */
    double factorial = 1.0;
    for (int k = 1; k <= 70; k++) factorial *= k;  /* overflows to inf */
    double sum_range = 2485.0;  /* sum(1..70) */
    double root_coeff = absT1 * absT2 * factorial / sum_range;

    /* cf[4] = root_coeff * sum(cos(k*t1)*sin(k*t2)/log(|k+1|) for k=1..70) */
    double sumcsr = 0, sumcsi = 0;
    for (int k = 1; k <= 70; k++) {
        double kt1r = k * x1r, kt1i = k * x1i;
        double kt2r = k * x2r, kt2i = k * x2i;
        double cosr = cos(kt1r) * cosh(kt1i);
        double cosi = -sin(kt1r) * sinh(kt1i);
        double sinr = sin(kt2r) * cosh(kt2i);
        double sini = cos(kt2r) * sinh(kt2i);
        double pr, pi;
        c_mul(cosr, cosi, sinr, sini, &pr, &pi);
        double logk = log((double)(k + 1));
        sumcsr += pr / logk;
        sumcsi += pi / logk;
    }
    cRe[4] = root_coeff * sumcsr;
    cIm[4] = root_coeff * sumcsi;

    /* cf[35] = root_coeff * t1^2 */
    cRe[35] = root_coeff * t1_2r;
    cIm[35] = root_coeff * t1_2i;

    /* cf[34] = root_coeff * t2^2 */
    cRe[34] = root_coeff * t2_2r;
    cIm[34] = root_coeff * t2_2i;

    /* cf[36:71] = root_coeff * [cos(2k*t1)*sin(2k*t2)/log(|2k+1|) for k=1..35] */
    for (int k = 1; k <= 35; k++) {
        double kt1r = 2.0 * k * x1r, kt1i = 2.0 * k * x1i;
        double kt2r = 2.0 * k * x2r, kt2i = 2.0 * k * x2i;
        double cosr = cos(kt1r) * cosh(kt1i);
        double cosi = -sin(kt1r) * sinh(kt1i);
        double sinr = sin(kt2r) * cosh(kt2i);
        double sini = cos(kt2r) * sinh(kt2i);
        double pr, pi;
        c_mul(cosr, cosi, sinr, sini, &pr, &pi);
        double logk = log((double)(2 * k + 1));
        cRe[35 + k] = root_coeff * pr / logk;
        cIm[35 + k] = root_coeff * pi / logk;
    }

    /* NaN guard */
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_21_hand ----
 * Lagrange-basis construction using 51st roots of unity.
 */
static void poly_21_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double rootR[51], rootI[51];
    for (int j = 0; j < 51; j++) {
        double ang = 2.0 * M_PI * j / 51.0;
        rootR[j] = cos(ang); rootI[j] = sin(ang);
    }

    for (int k = 0; k < 51; k++) {
        double prodR = 1, prodI = 0;
        for (int j = 0; j < 51; j++) {
            if (j == k) continue;
            double dr = rootR[j] - rootR[k], di = rootI[j] - rootI[k];
            double pr, pi;
            c_mul(prodR, prodI, dr, di, &pr, &pi);
            prodR = pr; prodI = pi;
        }
        double d1r = x1r - rootR[k], d1i = x1i - rootI[k];
        double q1r, q1i;
        c_div(prodR, prodI, d1r, d1i, &q1r, &q1i);
        double d2r = x2r - rootR[k], d2i = x2i - rootI[k];
        c_div(q1r, q1i, d2r, d2i, &cRe[k], &cIm[k]);
    }
    /* cf[0:51] *= (t1 - roots) * (t2 - roots) elementwise */
    for (int j = 0; j < 51; j++) {
        double d1r = x1r - rootR[j], d1i = x1i - rootI[j];
        double d2r = x2r - rootR[j], d2i = x2i - rootI[j];
        double mr, mi, rr, ri;
        c_mul(d1r, d1i, d2r, d2i, &mr, &mi);
        c_mul(cRe[j], cIm[j], mr, mi, &rr, &ri);
        cRe[j] = rr; cIm[j] = ri;
    }
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_35_hand ----
 * cf[k-1] = cos(t1)*t1^k - sin(t2)*t2^k for k=1..70; cf[70] = |t1*t2|
 */
static void poly_35_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double cosT1r, cosT1i; c_cos(x1r, x1i, &cosT1r, &cosT1i);
    double sinT2r, sinT2i; c_sin(x2r, x2i, &sinT2r, &sinT2i);
    double p1r = 1, p1i = 0, p2r = 1, p2i = 0;
    for (int k = 1; k <= 70; k++) {
        double nr, ni;
        c_mul(p1r, p1i, x1r, x1i, &nr, &ni); p1r = nr; p1i = ni;
        c_mul(p2r, p2i, x2r, x2i, &nr, &ni); p2r = nr; p2i = ni;
        double ar, ai, br, bi;
        c_mul(cosT1r, cosT1i, p1r, p1i, &ar, &ai);
        c_mul(sinT2r, sinT2i, p2r, p2i, &br, &bi);
        cRe[k-1] = ar - br; cIm[k-1] = ai - bi;
    }
    double mr, mi; c_mul(x1r, x1i, x2r, x2i, &mr, &mi);
    cRe[70] = c_abs(mr, mi); cIm[70] = 0;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_37_hand ----
 * cf[i-1] = (sin(i*t1)+cos(i*t2))*i^2; then cf[1]+=sum(cf[0:2]), cf[4]+=prod(cf[0:5]),
 * cf[11]+=log(|cf[10]|+1), cf[24]+=angle(cf[23]),
 * cf[[34,44,54,64]] += |t2|^2+Re(t1)^3,
 * cf[[6,13,20,27,34,41,48,55,62,69]] += sin(t1)^71-cos(t2)^71
 */
static void poly_37_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    for (int i = 1; i <= 71; i++) {
        double sr, si, cr, ci;
        c_sin(i*x1r, i*x1i, &sr, &si);
        c_cos(i*x2r, i*x2i, &cr, &ci);
        double i2 = (double)(i*i);
        cRe[i-1] = (sr+cr)*i2; cIm[i-1] = (si+ci)*i2;
    }
    cRe[1] += cRe[0]+cRe[1]; cIm[1] += cIm[0]+cIm[1];
    double prodR = cRe[0], prodI = cIm[0];
    for (int j = 1; j < 5; j++) { double pr,pi; c_mul(prodR,prodI,cRe[j],cIm[j],&pr,&pi); prodR=pr; prodI=pi; }
    cRe[4] += prodR; cIm[4] += prodI;
    cRe[11] += log(c_abs(cRe[10],cIm[10])+1);
    cRe[24] += c_arg(cRe[23],cIm[23]);
    double val = c_abs(x2r,x2i); val=val*val+x1r*x1r*x1r;
    int i1[]={34,44,54,64}; for(int j=0;j<4;j++) cRe[i1[j]]+=val;
    /* sin(t1)^71 - cos(t2)^71 */
    double st1r,st1i; c_sin(x1r,x1i,&st1r,&st1i);
    double ct2r,ct2i; c_cos(x2r,x2i,&ct2r,&ct2i);
    double s71r=1,s71i=0,c71r=1,c71i=0;
    for(int j=0;j<71;j++){double pr,pi;c_mul(s71r,s71i,st1r,st1i,&pr,&pi);s71r=pr;s71i=pi;}
    for(int j=0;j<71;j++){double pr,pi;c_mul(c71r,c71i,ct2r,ct2i,&pr,&pi);c71r=pr;c71i=pi;}
    double dR=s71r-c71r,dI=s71i-c71i;
    int i2[]={6,13,20,27,34,41,48,55,62,69}; for(int j=0;j<10;j++){cRe[i2[j]]+=dR;cIm[i2[j]]+=dI;}
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_40_hand ----
 * cf[k-1] = Re(t1)^(k+1)*sin(angle(t2*k)) + Im(t2)^k*cos(angle(t1/k)); cf[70]=|t1|+|t2|
 */
static void poly_40_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double angT1 = atan2(x1i, x1r);
    for (int k = 1; k <= 70; k++) {
        double rePow = pow(x1r, k+1);
        double angT2k = atan2(k*x2i, k*x2r);
        double imPow = pow(x2i, k);
        cRe[k-1] = rePow*sin(angT2k) + imPow*cos(angT1);
        cIm[k-1] = 0;
    }
    cRe[70] = c_abs(x1r,x1i)+c_abs(x2r,x2i); cIm[70]=0;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_46_hand ----
 * Uses primes array, cf[0:18]=Re(p*(t1+t2)), cf[18:36]=Im(p*(t1-t2)),
 * cf[36:54]=Re(p*t1*conj(t2)+log(p)), cf[54:71]=Im(t1^k)*t2^(k^2),
 * cf[70]=sum(t1^(k+k^2))+sum(t2^k)^2
 */
static void poly_46_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    static const double P[]={2,3,5,7,11,17,19,23,29,31,37,41,43,47,53,59,61,67};
    double sR=x1r+x2r, sI=x1i+x2i, dR=x1r-x2r, dI=x1i-x2i;
    double cjR,cjI; c_mul(x1r,x1i,x2r,-x2i,&cjR,&cjI);
    for(int j=0;j<18;j++){cRe[j]=P[j]*sR;}
    for(int j=0;j<18;j++){cRe[18+j]=P[j]*dI;}
    for(int j=0;j<18;j++){cRe[36+j]=P[j]*cjR+log(P[j]);}
    double t1pR=1,t1pI=0;
    for(int k=1;k<=17;k++){
        double nr,ni;c_mul(t1pR,t1pI,x1r,x1i,&nr,&ni);t1pR=nr;t1pI=ni;
        int kk=k*k; double t2pR=1,t2pI=0;
        for(int j=0;j<kk;j++){c_mul(t2pR,t2pI,x2r,x2i,&nr,&ni);t2pR=nr;t2pI=ni;}
        if(54+k-1<71){cRe[54+k-1]=t1pI*t2pR;cIm[54+k-1]=t1pI*t2pI;}
    }
    double s1R=0,s1I=0;
    for(int k=1;k<=5;k++){int e=k+k*k;double pR=1,pI=0;for(int j=0;j<e;j++){double nr,ni;c_mul(pR,pI,x1r,x1i,&nr,&ni);pR=nr;pI=ni;}s1R+=pR;s1I+=pI;}
    double s2R=0,s2I=0,t2pR2=1,t2pI2=0;
    for(int k=1;k<=10;k++){double nr,ni;c_mul(t2pR2,t2pI2,x2r,x2i,&nr,&ni);t2pR2=nr;t2pI2=ni;s2R+=t2pR2;s2I+=t2pI2;}
    double sq2R,sq2I;c_mul(s2R,s2I,s2R,s2I,&sq2R,&sq2I);
    cRe[70]=s1R+sq2R;cIm[70]=s1I+sq2I;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_72_hand ----
 * cf[0]=t1+t2; cf[1:71]=k*(t1-t2+(sin(k)+i*cos(k)));
 * sort |cf[1:71]| descending → sorted_roots;
 * cf[1:71]=sorted_roots*(t1+t2*i*k);
 * cf = Re(cf)+Im(cf)+(sin(angle(cf+1))+i*cos(angle(cf+1)))
 */
static void poly_72_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    cRe[0]=x1r+x2r; cIm[0]=x1i+x2i;
    double dR=x1r-x2r,dI=x1i-x2i;
    for(int k=1;k<=70;k++){cRe[k]=k*(dR+sin((double)k));cIm[k]=k*(dI+cos((double)k));}
    double mags[70];
    for(int k=0;k<70;k++) mags[k]=c_abs(cRe[k+1],cIm[k+1]);
    for(int i=0;i<69;i++)for(int j=i+1;j<70;j++)if(mags[j]>mags[i]){double t=mags[i];mags[i]=mags[j];mags[j]=t;}
    for(int k=1;k<=70;k++){cRe[k]=mags[k-1]*(x1r-x2i*k);cIm[k]=mags[k-1]*(x1i+x2r*k);}
    for(int k=0;k<71;k++){
        double re=cRe[k],im=cIm[k],ang=atan2(im,re+1.0);
        cRe[k]=re+im+sin(ang);cIm[k]=cos(ang);
    }
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_74_hand ----
 * cf[i-1]=i*(t1+i*t2)^(1/i) for i=1..35; cf[70]=conj(cf[34]); cf[35]=2*t1+3*|t2|
 */
static void poly_74_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    for(int i=1;i<=35;i++){
        double br=x1r+i*x2r,bi=x1i+i*x2i,lr,li,er,ei;
        c_log(br,bi,&lr,&li);lr/=i;li/=i;
        c_exp2(lr,li,&er,&ei);
        cRe[i-1]=i*er;cIm[i-1]=i*ei;
        cRe[70]=cRe[i-1];cIm[70]=-cIm[i-1];
    }
    cRe[35]=2*x1r+3*c_abs(x2r,x2i);cIm[35]=2*x1i;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_94_hand ----
 * Sparse powers of t1/t2 at specific indices, then cf[3:71] *= sin(t1)*sin(t2)-cos(t1)*cos(t2)
 */
static void poly_94_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double t1p[10],t1pi[10]; t1p[1]=x1r;t1pi[1]=x1i;
    for(int k=2;k<=9;k++){c_mul(t1p[k-1],t1pi[k-1],x1r,x1i,&t1p[k],&t1pi[k]);}
    double t2p[9],t2pi[9]; t2p[1]=x2r;t2pi[1]=x2i;
    for(int k=2;k<=8;k++){c_mul(t2p[k-1],t2pi[k-1],x2r,x2i,&t2p[k],&t2pi[k]);}
    cRe[3]=-t1p[9];cIm[3]=-t1pi[9]; cRe[5]=t2p[8];cIm[5]=t2pi[8];
    cRe[7]=-t1p[7];cIm[7]=-t1pi[7]; cRe[9]=t2p[6];cIm[9]=t2pi[6];
    cRe[10]=-t1p[5];cIm[10]=-t1pi[5]; cRe[12]=t2p[4];cIm[12]=t2pi[4];
    cRe[14]=-t1p[3];cIm[14]=-t1pi[3]; cRe[16]=t2p[2];cIm[16]=t2pi[2];
    cRe[20]=-x1r;cIm[20]=-x1i;
    cRe[30]=5e5;cRe[40]=-5e6;cRe[50]=5e7;cRe[60]=-5e8;cRe[70]=5e9;
    double st1r,st1i,st2r,st2i,ct1r,ct1i,ct2r,ct2i;
    c_sin(x1r,x1i,&st1r,&st1i);c_sin(x2r,x2i,&st2r,&st2i);
    c_cos(x1r,x1i,&ct1r,&ct1i);c_cos(x2r,x2i,&ct2r,&ct2i);
    double mr1,mi1,mr2,mi2;
    c_mul(st1r,st1i,st2r,st2i,&mr1,&mi1);c_mul(ct1r,ct1i,ct2r,ct2i,&mr2,&mi2);
    double mulR=mr1-mr2,mulI=mi1-mi2;
    for(int k=3;k<71;k++){double pr,pi;c_mul(cRe[k],cIm[k],mulR,mulI,&pr,&pi);cRe[k]=pr;cIm[k]=pi;}
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_100_hand ----
 * iter=1; for j=1..71: cf[j-1]=iter; iter *= (log(|t1+i*t2|+1)/(71-j+1) + conj(iter))
 */
static void poly_100_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double zr=x1r-x2i,zi=x1i+x2r;
    double logAbsZ=log(c_abs(zr,zi)+1.0);
    double iterR=1.0,iterI=0.0;
    for(int j=1;j<=71;j++){
        cRe[j-1]=iterR;cIm[j-1]=iterI;
        double d=(double)(71-j+1);
        double fR=logAbsZ/d+iterR,fI=-iterI;
        double nr,ni;c_mul(iterR,iterI,fR,fI,&nr,&ni);iterR=nr;iterI=ni;
    }
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_45_hand ----
 * Loop 1: for k=1..71:
 *   if k%2==0: cf[k-1] = k*(t1+Re(t2))*sin(|t1|*k)
 *   else:      cf[k-1] = k*(t2-Im(t1))*cos(angle(t2)*k)
 * Loop 2: for i=2..len(cf)//2-1:
 *   cf[i-1] = cf[i-2]*(|t1|+0.5) + log(|t2|+1)    (sequential dependency!)
 *   cf[70-i] = -cf[71-i]*(|t2|+0.5) - log(|t1|+1)  (sequential from end)
 */
static void poly_45_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double absT1 = c_abs(x1r, x1i);
    double absT2 = c_abs(x2r, x2i);
    double angT2 = c_arg(x2r, x2i);

    /* Loop 1 */
    for (int k = 1; k <= 71; k++) {
        if (k % 2 == 0) {
            /* k*(t1 + Re(t2))*sin(|t1|*k) */
            /* t1 + Re(t2) = (x1r + x2r) + i*x1i */
            double sr = sin(absT1 * k), cr_unused = cos(absT1 * k);
            /* sin is real here since absT1*k is real */
            double ar = x1r + x2r, ai = x1i;
            cRe[k-1] = k * (ar * sr);
            cIm[k-1] = k * (ai * sr);
        } else {
            /* k*(t2 - Im(t1))*cos(angle(t2)*k) */
            /* t2 - Im(t1) = (x2r - x1i) + i*x2i */
            double cv = cos(angT2 * k);
            double ar = x2r - x1i, ai = x2i;
            cRe[k-1] = k * (ar * cv);
            cIm[k-1] = k * (ai * cv);
        }
    }

    /* Loop 2: sequential dependency — each cf[i-1] depends on cf[i-2]
     * len(cf)//2 = 35, so i goes from 2 to 34 */
    double mul1r = absT1 + 0.5;
    double logT2 = log(absT2 + 1.0);
    double mul2r = absT2 + 0.5;
    double logT1 = log(absT1 + 1.0);

    for (int i = 2; i < 35; i++) {
        /* cf[i-1] = cf[i-2] * (|t1|+0.5) + log(|t2|+1)
         * mul by real scalar, add real constant */
        double prevR = cRe[i-2], prevI = cIm[i-2];
        cRe[i-1] = prevR * mul1r + logT2;
        cIm[i-1] = prevI * mul1r;

        /* cf[len(cf)-i] = -cf[len(cf)-i+1] * (|t2|+0.5) - log(|t1|+1)
         * len(cf) = 71, so cf[71-i] = -cf[72-i] * ... */
        int idx = 71 - i;      /* target */
        int src = 72 - i;      /* source = idx + 1 */
        double srcR = cRe[src], srcI = cIm[src];
        cRe[idx] = -srcR * mul2r - logT1;
        cIm[idx] = -srcI * mul2r;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_42_serendipity ----
 * The original broken transpiled version of poly_42, preserved exactly as-is.
 * The transpiler replaced the arange loop variable with 0, producing a
 * beautiful accident. This just delegates to the original transpiled poly_42_c
 * from poly_generated_funcs.h — that function is never modified.
 */
static void poly_42_serendipity(double x1r, double x1i, double x2r, double x2i,
                                const double *cfpv, int n_cfpv,
                                double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    poly_42_c(x1r, x1i, x2r, x2i, cfpv, n_cfpv, cRe, cIm, nCoeffs);
}

/* ---- poly_42_hand ----
 * Correct version:
 * cf[0:35] = |t1| * sin(arange(1,36) * angle(t1))
 * cf[35:70] = Re(t2) * cos(arange(1,36) * Im(t2))
 * cf[70] = t1*t2 + i*sum(log(|cf[0:70]|+1))
 */
static void poly_42_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double absT1 = c_abs(x1r, x1i);
    double angT1 = c_arg(x1r, x1i);
    for (int k = 1; k <= 35; k++) {
        cRe[k-1] = absT1 * sin(k * angT1);
        cIm[k-1] = 0;
    }
    for (int k = 1; k <= 35; k++) {
        cRe[34 + k] = x2r * cos(k * x2i);
        cIm[34 + k] = 0;
    }
    /* cf[70] = t1*t2 + i*sum(log(|cf[0:70]|+1)) */
    double mr, mi;
    c_mul(x1r, x1i, x2r, x2i, &mr, &mi);
    double logsum = 0;
    for (int k = 0; k < 70; k++) logsum += log(c_abs(cRe[k], cIm[k]) + 1);
    cRe[70] = mr;
    cIm[70] = mi + logsum;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_2_hand ----
 * cf[0]=t1+t2; for k=2..36: v=sin(k*cf[k-2])+cos(k*t1)+Re(k*t2)*Im(k*cf[k-2]); cf[k-1]=v/|v|
 * cf[17], cf[31], cf[35] overwritten
 */
static void poly_2_hand(double x1r, double x1i, double x2r, double x2i,
                        const double *cfpv, int n_cfpv,
                        double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 36;
    for (int i = 0; i < 36; i++) { cRe[i] = 0; cIm[i] = 0; }
    cRe[0] = x1r + x2r; cIm[0] = x1i + x2i;
    for (int k = 2; k <= 36; k++) {
        double prevR = cRe[k-2], prevI = cIm[k-2];
        /* sin(k*cf[k-2]) */
        double sr, si; c_sin(k*prevR, k*prevI, &sr, &si);
        /* cos(k*t1) */
        double cr, ci; c_cos(k*x1r, k*x1i, &cr, &ci);
        /* Re(k*t2)*Im(k*cf[k-2]) */
        double reKt2 = k * x2r;
        double imKprev = k * prevI;
        double vr = sr + cr + reKt2 * imKprev;
        double vi = si + ci;
        double mag = c_abs(vr, vi);
        if (mag > 1e-30) { cRe[k-1] = vr/mag; cIm[k-1] = vi/mag; }
    }
    /* cf[17] = t1^2 + Re(t1)*t2 - Im(t2^2) */
    double t1sq_r, t1sq_i; c_mul(x1r,x1i,x1r,x1i,&t1sq_r,&t1sq_i);
    double rt1t2_r = x1r*x2r, rt1t2_i = x1r*x2i;
    double t2sq_r, t2sq_i; c_mul(x2r,x2i,x2r,x2i,&t2sq_r,&t2sq_i);
    cRe[17] = t1sq_r + rt1t2_r - t2sq_i; cIm[17] = t1sq_i + rt1t2_i;
    /* cf[31] = 2*(t1+t2) - Re(t1*t2) + sin(Re(t1))*cos(Im(t2)) */
    double mr, mi; c_mul(x1r,x1i,x2r,x2i,&mr,&mi);
    cRe[31] = 2*(x1r+x2r) - mr + sin(x1r)*cos(x2i);
    cIm[31] = 2*(x1i+x2i);
    /* cf[35] = cf[17]*cf[31] + sin(Re(t1*t2)) - cos(Im(t1*t2)) */
    double p35r, p35i; c_mul(cRe[17],cIm[17],cRe[31],cIm[31],&p35r,&p35i);
    cRe[35] = p35r + sin(mr) - cos(mi); cIm[35] = p35i;
    for (int i = 0; i < 36; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_9_hand ----
 * 51 coefficients. Loop sets cf, then post-loop reads/slice assignments.
 */
static void poly_9_hand(double x1r, double x1i, double x2r, double x2i,
                        const double *cfpv, int n_cfpv,
                        double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 51;
    for (int i = 0; i < 51; i++) { cRe[i] = 0; cIm[i] = 0; }
    cRe[0] = x1r + x2r; cIm[0] = x1i + x2i;
    double absT1 = c_abs(x1r,x1i), angT2 = c_arg(x2r,x2i);
    for (int k = 2; k <= 51; k++) {
        double denom = sqrt((double)(k*k) + 1.0);
        cRe[k-1] = (absT1*sin(k) + angT2*cos(k)) / denom;
        cIm[k-1] = 0;
    }
    /* cf[9] = cf[0]^2 - cf[1]^2 + log(|cf[2]|+1) */
    double c0sq_r,c0sq_i; c_mul(cRe[0],cIm[0],cRe[0],cIm[0],&c0sq_r,&c0sq_i);
    double c1sq_r,c1sq_i; c_mul(cRe[1],cIm[1],cRe[1],cIm[1],&c1sq_r,&c1sq_i);
    cRe[9] = c0sq_r - c1sq_r + log(c_abs(cRe[2],cIm[2])+1);
    cIm[9] = c0sq_i - c1sq_i;
    /* cf[19] = sum(cf[0:19]) * t1 */
    double sumR=0,sumI=0;
    for(int j=0;j<19;j++){sumR+=cRe[j];sumI+=cIm[j];}
    c_mul(sumR,sumI,x1r,x1i,&cRe[19],&cIm[19]);
    /* cf[29] = prod(cf[0:29]) * t2 */
    double pR=cRe[0],pI=cIm[0];
    for(int j=1;j<29;j++){double r,i;c_mul(pR,pI,cRe[j],cIm[j],&r,&i);pR=r;pI=i;}
    c_mul(pR,pI,x2r,x2i,&cRe[29],&cIm[29]);
    /* cf[39] = cf[38]*cf[37] / (1+t1*t2) */
    double n39r,n39i; c_mul(cRe[38],cIm[38],cRe[37],cIm[37],&n39r,&n39i);
    double t12r,t12i; c_mul(x1r,x1i,x2r,x2i,&t12r,&t12i);
    double d39r=1+t12r,d39i=t12i;
    c_div(n39r,n39i,d39r,d39i,&cRe[39],&cIm[39]);
    /* cf[40:50] = Re(cf[30:40]) + i*Im(cf[0:10]) */
    for(int j=0;j<10;j++){cRe[40+j]=cRe[30+j];cIm[40+j]=cIm[j];}
    /* cf[50] = sum(cf[0:50]) */
    sumR=0;sumI=0;
    for(int j=0;j<50;j++){sumR+=cRe[j];sumI+=cIm[j];}
    cRe[50]=sumR;cIm[50]=sumI;
    for (int i = 0; i < 51; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_44_hand ---- */
static void poly_44_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double sr,si,cr,ci;
    c_sin(x1r,x1i,&sr,&si); c_cos(x2r,x2i,&cr,&ci);
    c_mul(sr,si,cr,ci,&cRe[0],&cIm[0]);
    c_cos(x1r,x1i,&cr,&ci); c_sin(x2r,x2i,&sr,&si);
    cRe[1]=cr+sr; cIm[1]=ci+si;
    double a1=c_abs(x1r,x1i),a2=c_abs(x2r,x2i);
    cRe[2]=a1*a1*a1-a2*a2*a2*a2; cIm[2]=0;
    cRe[3]=c_arg(x1r,x1i)-c_arg(x2r,x2i); cIm[3]=0;
    double m0r,m0i; c_mul(x1r,x1i,x2r,x2i,&m0r,&m0i);
    cRe[4]=c_abs(m0r,m0i); cIm[4]=0;
    for(int k=6;k<=35;k++){
        c_sin(k*x1r,k*x1i,&sr,&si); c_cos(k*x2r,k*x2i,&cr,&ci);
        cRe[k-1]=sr+cr; cIm[k-1]=si+ci;
        int k2=70-k;
        c_sin(k2*x1r,k2*x1i,&sr,&si); c_cos(k2*x2r,k2*x2i,&cr,&ci);
        cRe[k+34]=sr-cr; cIm[k+34]=si-ci;
    }
    cRe[35]=c_abs(x1r+x2r,x1i+x2i); cIm[35]=0;
    cRe[70]=log(c_abs(m0r,m0i)+1); cIm[70]=0;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_50_hand ----
 * Loop with conditional cf[k-2], cf[k-3], cf[k-4] dependencies
 */
static void poly_50_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double sr,si,cr,ci;
    c_sin(x1r+x2r,x1i+x2i,&sr,&si); c_cos(x1r-x2r,x1i-x2i,&cr,&ci);
    double cvr=sr+cr, cvi=si+ci;
    double absT1=c_abs(x1r,x1i);
    for(int k=1;k<=71;k++){
        if(k%2==0){
            cRe[k-1]=cvr/(double)k - absT1; cIm[k-1]=cvi/(double)k;
        } else {
            cRe[k-1]=cvr*(double)k + log((double)k+1) + x2i - x1r;
            cIm[k-1]=cvi*(double)k;
        }
        if(k%3==0 && k>=3){cRe[k-1]+=3*cRe[k-2];cIm[k-1]+=3*cIm[k-2];}
        if(k%5==0 && k>=4){cRe[k-1]+=5*cRe[k-3];cIm[k-1]+=5*cIm[k-3];}
        if(k%7==0 && k>=5){cRe[k-1]+=7*cRe[k-4];cIm[k-1]+=7*cIm[k-4];}
    }
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_54_hand ----
 * Loop sets cf, then cf[0:30] *= (|t1|*|t2|)^arange(1,31)
 */
static void poly_54_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    for(int i=1;i<=71;i++){
        /* z = t1*cos(i*t2/15) + t2*sin(i*t1/15) */
        double cr,ci,sr,si;
        c_cos(i*x2r/15.0,i*x2i/15.0,&cr,&ci);
        c_sin(i*x1r/15.0,i*x1i/15.0,&sr,&si);
        double ar,ai,br,bi;
        c_mul(x1r,x1i,cr,ci,&ar,&ai);
        c_mul(x2r,x2i,sr,si,&br,&bi);
        double zr=ar+br, zi=ai+bi;
        double phi=c_arg(zr,zi), r=c_abs(zr,zi);
        /* r*exp(i*phi)^i = r*exp(i*i*phi) */
        double er,ei; c_exp2(0, i*phi, &er, &ei);
        double sign = (i%2==0) ? 1.0 : -1.0;
        cRe[i-1] = r*er + sign*(double)(i*i);
        cIm[i-1] = r*ei;
    }
    /* cf[0:30] *= (|t1|*|t2|)^arange(1,31) */
    double ab=c_abs(x1r,x1i)*c_abs(x2r,x2i);
    double pw=1.0;
    for(int k=0;k<30;k++){
        pw*=ab;
        cRe[k]*=pw; cIm[k]*=pw;
    }
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_61_hand ----
 * cf[0:35] = Re(t1)*arange(1,36)^3 + Im(t2)*sin(arange(1,36))
 * cf[35:70] = Im(t1)*arange(70,35,-1)^2 + Re(t2)*cos(arange(70,35,-1))
 * cf[70] = |t1|*angle(t2) - |t2|*angle(t1)
 */
static void poly_61_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    for(int k=1;k<=35;k++){
        cRe[k-1] = x1r*(double)(k*k*k) + x2i*sin((double)k);
        cIm[k-1] = 0;
    }
    for(int k=70;k>=36;k--){
        cRe[k-1] = x1i*(double)(k*k) + x2r*cos((double)k);
        cIm[k-1] = 0;
    }
    cRe[70] = c_abs(x1r,x1i)*c_arg(x2r,x2i) - c_abs(x2r,x2i)*c_arg(x1r,x1i);
    cIm[70] = 0;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_62_hand ----
 * Sequential: cf[i-1] = i*cf[i-2]^2, then post-loop modifications
 */
static void poly_62_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    /* cf[0] = t1^5 + t2^5 */
    double pr=1,pi=0,qr=1,qi=0;
    for(int j=0;j<5;j++){double r,i;c_mul(pr,pi,x1r,x1i,&r,&i);pr=r;pi=i;}
    double p2r=1,p2i=0;
    for(int j=0;j<5;j++){double r,i;c_mul(p2r,p2i,x2r,x2i,&r,&i);p2r=r;p2i=i;}
    cRe[0]=pr+p2r; cIm[0]=pi+p2i;
    for(int i=2;i<=71;i++){
        double sq_r,sq_i;
        c_mul(cRe[i-2],cIm[i-2],cRe[i-2],cIm[i-2],&sq_r,&sq_i);
        if(i%2==0){
            cRe[i-1]=(double)i*sq_r; cIm[i-1]=(double)i*sq_i;
        } else {
            double fr=(double)i*sq_r, fi=(double)i*sq_i;
            /* *(1+0.1*t2) */
            double mr,mi; c_mul(fr,fi,1+0.1*x2r,-0.1*x2i,&mr,&mi);
            /* wait: 1+0.1*t2 = (1+0.1*x2r) + i*(0.1*x2i) */
            c_mul(fr,fi,1+0.1*x2r,0.1*x2i,&cRe[i-1],&cIm[i-1]);
        }
    }
    /* cf[0] += 2*cf[1] */
    cRe[0]+=2*cRe[1]; cIm[0]+=2*cIm[1];
    /* cf[1] -= 3*cf[2] */
    cRe[1]-=3*cRe[2]; cIm[1]-=3*cIm[2];
    /* for i=3..69: cf[i] += cf[i+1] - cf[i+2] */
    for(int i=3;i<70;i++){cRe[i]+=cRe[i+1]-cRe[i+2]; cIm[i]+=cIm[i+1]-cIm[i+2];}
    /* cf[69] += cf[70] */
    cRe[69]+=cRe[70]; cIm[69]+=cIm[70];
    /* cf[70] = |t1|^2 - |t2|^2 + 2*Im(t1)*Im(t2) - angle(t2) */
    double a1=c_abs(x1r,x1i),a2=c_abs(x2r,x2i);
    cRe[70]=a1*a1-a2*a2+2*x1i*x2i-c_arg(x2r,x2i); cIm[70]=0;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_65_hand ---- */
static void poly_65_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double sumR=x1r+x2r, sumI=x1i+x2i;
    for(int k=1;k<=71;k++){
        /* (t1+t2)^(2k-1) */
        int exp=2*k-1; double pR=1,pI=0;
        for(int j=0;j<exp;j++){double r,i;c_mul(pR,pI,sumR,sumI,&r,&i);pR=r;pI=i;}
        /* sin(k*t1)*cos(k*t2) */
        double sr,si,cr,ci;
        c_sin(k*x1r,k*x1i,&sr,&si);c_cos(k*x2r,k*x2i,&cr,&ci);
        double scr,sci; c_mul(sr,si,cr,ci,&scr,&sci);
        /* log(|k^t2|+1)*Re(t1^t2) — k^t2 = exp(t2*log(k)) */
        double lk=log((double)k);
        double eR,eI; c_exp2(x2r*lk-x2i*0, x2i*lk+x2r*0, &eR, &eI);
        /* wait: k^t2 = exp(t2*log(k)). t2*log(k) = (x2r*lk, x2i*lk) */
        c_exp2(x2r*lk, x2i*lk, &eR, &eI);
        double logAbsKt2 = log(c_abs(eR,eI)+1);
        /* t1^t2 = exp(t2*log(t1)) */
        double lt1r,lt1i; c_log(x1r,x1i,&lt1r,&lt1i);
        double elt1r,elt1i; c_mul(x2r,x2i,lt1r,lt1i,&elt1r,&elt1i);
        double t1t2r,t1t2i; c_exp2(elt1r,elt1i,&t1t2r,&t1t2i);
        double reT1t2 = t1t2r;
        /* |Im(t1^(2k+1) + t2^(2k))| */
        int e1=2*k+1,e2=2*k;
        double p1r=1,p1i=0; for(int j=0;j<e1;j++){double r,i;c_mul(p1r,p1i,x1r,x1i,&r,&i);p1r=r;p1i=i;}
        double p2r=1,p2i=0; for(int j=0;j<e2;j++){double r,i;c_mul(p2r,p2i,x2r,x2i,&r,&i);p2r=r;p2i=i;}
        double absImPow = fabs(p1i+p2i);
        double vr = pR + scr + logAbsKt2*reT1t2 + absImPow;
        double vi = pI + sci;
        /* conj * (-1)^k */
        double sign = (k%2==0) ? 1.0 : -1.0;
        cRe[k-1] = sign*vr; cIm[k-1] = sign*(-vi);
        if(k%2==0){
            /* /= (k + t1) */
            double dr=k+x1r, di=x1i;
            double qr,qi; c_div(cRe[k-1],cIm[k-1],dr,di,&qr,&qi);
            cRe[k-1]=qr; cIm[k-1]=qi;
        }
    }
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_70_hand ----
 * cf[i-1] = Re(t1)*Re(t2)*i^2/exp(|t1|*i) + Im(t1)*Im(t2)*i^3/exp(|t2|*i)
 * cf[1::2] *= -1; cf[k where k^2<=71] += i*|t1|*|t2|
 */
static void poly_70_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double absT1=c_abs(x1r,x1i), absT2=c_abs(x2r,x2i);
    for(int i=1;i<=71;i++){
        /* exp(|t1|*1j) = cos(|t1|) + i*sin(|t1|) */
        double e1r=cos(absT1), e1i=sin(absT1);
        double e2r=cos(absT2), e2i=sin(absT2);
        /* Re(t1)*Re(t2)*i^2 / exp(|t1|*1j) */
        double numR = x1r*x2r*(double)(i*i);
        double ar,ai; c_div(numR,0,e1r,e1i,&ar,&ai);
        /* Im(t1)*Im(t2)*i^3 / exp(|t2|*1j) */
        double numR2 = x1i*x2i*(double)(i*i*i);
        double br,bi; c_div(numR2,0,e2r,e2i,&br,&bi);
        cRe[i-1]=ar+br; cIm[i-1]=ai+bi;
    }
    /* cf[1::2] *= -1 (odd indices) */
    for(int i=1;i<71;i+=2){cRe[i]=-cRe[i];cIm[i]=-cIm[i];}
    /* cf[k where k^2<=71] += i*|t1|*|t2| — k=1..8 (8^2=64<=71, 9^2=81>71) */
    /* p=arange(1,72); cf[p**2<=71] — this means indices where (index+1)^2<=71? No: p=arange(1,72) is [1..71], p**2 is [1,4,9,...]; cf[p**2<=71] selects cf at indices where p^2<=71, i.e. p=1..8, so cf[1],cf[4],cf[9],...cf[64] — but p is used as boolean mask on cf */
    /* Actually: p=np.arange(1,72) gives [1,2,...,71]. p**2 = [1,4,...,5041]. p**2<=71 is True for p=1..8. cf[p**2<=71] selects cf[0:8] (first 8 elements, where mask is True) */
    double ab12=absT1*absT2;
    for(int j=0;j<8;j++){cIm[j]+=ab12;} /* += 1j * |t1|*|t2| */
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_73_hand ----
 * Large-scale coefficients with powers of 10. Multiple loops, each covering different ranges.
 */
static void poly_73_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double sR=x1r+x2r, sI=x1i+x2i, dR=x1r-x2r, dI=x1i-x2i;
    double ct1r,ct1i,st2r,st2i;
    c_cos(x1r,x1i,&ct1r,&ct1i); c_sin(x2r,x2i,&st2r,&st2i);
    cRe[0]=1e30*sR; cIm[0]=1e30*sI;
    cRe[1]=1e28*dR; cIm[1]=1e28*dI;
    cRe[2]=1e26*sR; cIm[2]=1e26*sI;
    for(int k=4;k<=21;k++){
        double s=pow(10.0,30-k);
        cRe[k-1]=s*(ct1r+st2r); cIm[k-1]=s*(ct1i+st2i);
    }
    for(int k=22;k<=31;k++){
        double s=pow(10.0,k-21);
        cRe[k-1]=s*(ct1r-st2r); cIm[k-1]=s*(ct1i-st2i);
    }
    double csR,csI,ssR,ssI;
    c_cos(sR,sI,&csR,&csI); c_sin(dR,dI,&ssR,&ssI);
    for(int k=32;k<=41;k++){
        double s=pow(10.0,42-k);
        double mr,mi; c_mul(sR,sI,csR+ssR,csI+ssI,&mr,&mi);
        cRe[k-1]=s*mr; cIm[k-1]=s*mi;
    }
    cRe[41]=1e21*dR; cIm[41]=1e21*dI;
    double absS=c_abs(sR,sI), angD=c_arg(dR,dI);
    for(int k=43;k<=53;k++){double s=pow(10.0,53-k);cRe[k-1]=s*(absS+angD);cIm[k-1]=0;}
    double absD=c_abs(dR,dI), angS=c_arg(sR,sI);
    for(int k=54;k<=64;k++){double s=pow(10.0,64-k);cRe[k-1]=s*(absD+angS);cIm[k-1]=0;}
    double s1r,s1i,c2r,c2i;
    c_sin(x1r,x1i,&s1r,&s1i); c_cos(x2r,x2i,&c2r,&c2i);
    for(int k=65;k<=71;k++){double s=pow(10.0,71-k);cRe[k-1]=s*(s1r+c2r);cIm[k-1]=s*(s1i+c2i);}
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_78_hand ----
 * 7 slice assignments with fractional powers and alternating signs
 */
static void poly_78_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double a1=c_abs(x1r,x1i), a2=c_abs(x2r,x2i);
    double logA2=log(1+a2), angT2=c_arg(x2r,x2i);
    double aSR=x1r+x2r, aSI=x1i+x2i;
    double angProd=c_arg(x1r*x2r-x1i*x2i, x1r*x2i+x1i*x2r);
    double angDifR=x1r-x2r, angDifI=x1i-x2i;
    double angDif=c_arg(angDifR,angDifI);
    double logAbsSum=log(1+c_abs(aSR,aSI));
    /* cf[0:10] = |t1|^(k/5)*log(1+|t2|) */
    for(int k=1;k<=10;k++){cRe[k-1]=pow(a1,(double)k/5.0)*logA2;cIm[k-1]=0;}
    /* cf[10:20] = Re(t1)^k * angle(t2) * (-1)^k */
    for(int k=1;k<=10;k++){
        double sign=(k%2==0)?1.0:-1.0;
        cRe[9+k]=pow(x1r,(double)k)*angT2*sign; cIm[9+k]=0;
    }
    /* cf[20:30] = Im(t1)^(k/3) * |t2|^(k/4) * (-1)^k */
    for(int k=1;k<=10;k++){
        double sign=(k%2==0)?1.0:-1.0;
        cRe[19+k]=pow(x1i,(double)k/3.0)*pow(a2,(double)k/4.0)*sign; cIm[19+k]=0;
    }
    /* cf[30:40] = |t1*t2|^(k/2) * k */
    double absProd=c_abs(x1r*x2r-x1i*x2i,x1r*x2i+x1i*x2r);
    for(int k=1;k<=10;k++){cRe[29+k]=pow(absProd,(double)k/2.0)*(double)k;cIm[29+k]=0;}
    /* cf[40:50] = Re((t1+t2)^(k/2))*cos(angle(t1*t2))*(-1)^k */
    for(int k=1;k<=10;k++){
        double sign=(k%2==0)?1.0:-1.0;
        /* (t1+t2)^(k/2) = exp(k/2*log(t1+t2)) */
        double lr,li; c_log(aSR,aSI,&lr,&li);
        double er,ei; c_exp2(lr*(double)k/2.0, li*(double)k/2.0, &er, &ei);
        cRe[39+k]=er*cos(angProd)*sign; cIm[39+k]=0;
    }
    /* cf[50:60] = Im((t1+t2)^(k/3))*sin(angle(t1-t2))*(-1)^k */
    for(int k=1;k<=10;k++){
        double sign=(k%2==0)?1.0:-1.0;
        double lr,li; c_log(aSR,aSI,&lr,&li);
        double er,ei; c_exp2(lr*(double)k/3.0, li*(double)k/3.0, &er, &ei);
        cRe[49+k]=ei*sin(angDif)*sign; cIm[49+k]=0;
    }
    /* cf[60:70] = Re(t1^k)*|t2^k|*log(1+|t1+t2|) */
    double t1pR=1,t1pI=0,t2pR=1,t2pI=0;
    for(int k=1;k<=10;k++){
        double nr,ni;
        c_mul(t1pR,t1pI,x1r,x1i,&nr,&ni);t1pR=nr;t1pI=ni;
        c_mul(t2pR,t2pI,x2r,x2i,&nr,&ni);t2pR=nr;t2pI=ni;
        cRe[59+k]=t1pR*c_abs(t2pR,t2pI)*logAbsSum; cIm[59+k]=0;
    }
    cRe[70]=c_abs(angDifR,angDifI)*log(1+a1); cIm[70]=0;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_81_hand ----
 * Loop + fancy indexing (np.arange slices with step)
 */
static void poly_81_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double a1=c_abs(x1r,x1i), a2=c_abs(x2r,x2i);
    for(int k=1;k<=71;k++){
        double sr,si,cr,ci;
        c_sin(k*x1r,k*x1i,&sr,&si); c_cos(k*x2r,k*x2i,&cr,&ci);
        double mr,mi; c_mul(sr,si,cr,ci,&mr,&mi);
        cRe[k-1]=(k+20)*mr + pow(a1,(double)k) + pow(a2,(double)k);
        cIm[k-1]=(k+20)*mi;
    }
    /* cf[arange(2,71,5)] += |t1|*|t2| → indices 2,7,12,...,67 */
    double ab=a1*a2;
    for(int j=2;j<71;j+=5){cRe[j]+=ab;}
    /* cf[arange(3,70,7)] += (-1)^arange(1,11) * angle(t1+t2) — 10 elements */
    double angSum=c_arg(x1r+x2r,x1i+x2i);
    int idx3[]={3,10,17,24,31,38,45,52,59,66};
    for(int j=0;j<10;j++){double sign=(j%2==0)?-1.0:1.0;cRe[idx3[j]]+=sign*angSum;}
    /* cf[arange(6,67,9)] += (-1)^arange(1,8)*log(|t1+t2|+1) — 7 elements */
    double logS=log(c_abs(x1r+x2r,x1i+x2i)+1);
    int idx6[]={6,15,24,33,42,51,60};
    for(int j=0;j<7;j++){double sign=(j%2==0)?-1.0:1.0;cRe[idx6[j]]+=sign*logS;}
    /* cf[arange(5,71,7)] *= Re(t1+t2) — indices 5,12,19,...,68 */
    double reSum=x1r+x2r;
    for(int j=5;j<71;j+=7){cRe[j]*=reSum;cIm[j]*=reSum;}
    /* cf[arange(7,64,11)] *= Im(t1+t2) — indices 7,18,29,40,51,62 */
    double imSum=x1i+x2i;
    for(int j=7;j<64;j+=11){cRe[j]*=imSum;cIm[j]*=imSum;}
    /* cf[arange(1,72,7)] *= conj(t1+t2) — indices 1,8,15,...,71→capped to 70 */
    double cjR=x1r+x2r, cjI=-(x1i+x2i);
    for(int j=1;j<71;j+=7){double r,i;c_mul(cRe[j],cIm[j],cjR,cjI,&r,&i);cRe[j]=r;cIm[j]=i;}
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_82_hand ----
 * Forward dependency: cf[i-1] = i + cf[i-2]*sin + cf[i-3]*cos + cf[i-4]*log
 * Then reverse loop, then cf[70]=sum
 */
static void poly_82_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    double mr,mi; c_mul(x1r,x1i,x2r,x2i,&mr,&mi);
    cRe[0]=1+mr; cIm[0]=mi;
    cRe[1]=2+c_abs(x1r,x1i)*c_abs(x2r,x2i); cIm[1]=0;
    cRe[2]=3+c_abs(x1r+x2r,x1i+x2i); cIm[2]=0;
    for(int i=4;i<=36;i++){
        double s2r,s2i; c_sin(i*x2r,i*x2i,&s2r,&s2i);
        double c1r,c1i; c_cos(i*x1r,i*x1i,&c1r,&c1i);
        double lv=log(c_abs(i*mr+1, i*mi));
        double ar,ai; c_mul(cRe[i-2],cIm[i-2],s2r,s2i,&ar,&ai);
        double br,bi; c_mul(cRe[i-3],cIm[i-3],c1r,c1i,&br,&bi);
        cRe[i-1]=(double)i+ar+br+cRe[i-4]*lv; cIm[i-1]=ai+bi+cIm[i-4]*lv;
    }
    for(int i=37;i<=70;i++){
        int ri=70-i;
        int i2=70-((i<69)?i:69), i3=69-((i<68)?i:68), i4=68-((i<67)?i:67);
        double s1r,s1i; c_sin(ri*x1r,ri*x1i,&s1r,&s1i);
        double c2r,c2i; c_cos(ri*x2r,ri*x2i,&c2r,&c2i);
        double lv=log(c_abs(ri*mr+1, ri*mi));
        double ar,ai; c_mul(cRe[i2],cIm[i2],s1r,s1i,&ar,&ai);
        double br,bi; c_mul(cRe[i3],cIm[i3],c2r,c2i,&br,&bi);
        cRe[i-1]=(double)(70-i)+ar+br+cRe[i4]*lv; cIm[i-1]=ai+bi+cIm[i4]*lv;
    }
    double sumR=0,sumI=0;
    for(int j=0;j<70;j++){sumR+=cRe[j];sumI+=cIm[j];}
    cRe[70]=sumR+c_arg(x1r-x2r,x1i-x2i);
    cIm[70]=sumI+c_arg(x1r+x2r,x1i+x2i);
    /* angle returns real, so imag part correction */
    cIm[70]=sumI; cRe[70]=sumR+c_arg(x1r-x2r,x1i-x2i);
    /* np.real(np.angle(...)) + np.imag(np.angle(...)) — angle is real, so imag=0 */
    cRe[70]=sumR+c_arg(x1r-x2r,x1i-x2i); cIm[70]=sumI;
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_95_hand ----
 * Loop + slice modifications reading loop-set values
 */
static void poly_95_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    cRe[0]=x1r*x1r*x1r - x1i*x1i + 2*x1i*x1r - x2r + x2i*x2i; cIm[0]=0;
    cRe[1]=x1i*x1i*x1i - 5*x1r*x1r + 2*x1r*x1i + 5*x2r - 2*x2i*x2i; cIm[1]=0;
    for(int k=3;k<=71;k++){
        double sr,si; c_sin(k*x1r,k*x1i,&sr,&si);
        double cr,ci; c_cos(k*x2r,k*x2i,&cr,&ci);
        /* |t1^k+t2^(k-1)| */
        double p1r=1,p1i=0; for(int j=0;j<k;j++){double r,i;c_mul(p1r,p1i,x1r,x1i,&r,&i);p1r=r;p1i=i;}
        double p2r=1,p2i=0; for(int j=0;j<k-1;j++){double r,i;c_mul(p2r,p2i,x2r,x2i,&r,&i);p2r=r;p2i=i;}
        cRe[k-1]=c_abs(sr,si)+c_abs(cr,ci)-c_abs(p1r+p2r,p1i+p2i);
        cIm[k-1]=0;
    }
    /* cf[29:40] = |cf[29:40]| / (|t1-t2|^2+1) */
    double dR=x1r-x2r,dI=x1i-x2i; double ad=c_abs(dR,dI); double d1=ad*ad+1;
    for(int j=29;j<40;j++){cRe[j]=c_abs(cRe[j],cIm[j])/d1; cIm[j]=0;}
    /* cf[49:60] = -|cf[49:60]| / (|t1+t2|^2+1) */
    double sR2=x1r+x2r,sI2=x1i+x2i; double as2=c_abs(sR2,sI2); double d2=as2*as2+1;
    for(int j=49;j<60;j++){cRe[j]=-c_abs(cRe[j],cIm[j])/d2; cIm[j]=0;}
    /* cf[64:71] = cf[0:7] * (|t1|^2+|t2|^2) */
    double a1=c_abs(x1r,x1i),a2=c_abs(x2r,x2i); double scale=a1*a1+a2*a2;
    for(int j=0;j<7;j++){cRe[64+j]=cRe[j]*scale; cIm[64+j]=cIm[j]*scale;}
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_96_hand ----
 * Two sequential loops with cf[k-1]=k*cf[k-2]+... dependency
 */
static void poly_96_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    /* cf[0] = t1^5 - t2^4 + t1^2 - t2^2 + |t1| + |t2| */
    double t1p[8],t1pi[8],t2p[7],t2pi[7];
    t1p[1]=x1r;t1pi[1]=x1i;
    for(int j=2;j<=5;j++){c_mul(t1p[j-1],t1pi[j-1],x1r,x1i,&t1p[j],&t1pi[j]);}
    t2p[1]=x2r;t2pi[1]=x2i;
    for(int j=2;j<=6;j++){c_mul(t2p[j-1],t2pi[j-1],x2r,x2i,&t2p[j],&t2pi[j]);}
    double a1=c_abs(x1r,x1i),a2=c_abs(x2r,x2i);
    cRe[0]=t1p[5]-t2p[4]+t1p[2]-t2p[2]+a1+a2;
    cIm[0]=t1pi[5]-t2pi[4]+t1pi[2]-t2pi[2];
    /* cf[50] */
    cRe[50]=t2p[6]-t1p[4]+t2p[3]-t1p[2]+c_arg(x1r,x1i);
    cIm[50]=t2pi[6]-t1pi[4]+t2pi[3]-t1pi[2];
    double sr,si; c_sin(x2r,x2i,&sr,&si);
    cRe[50]+=sr; cIm[50]+=si;
    /* cf[70] */
    double t1_7r=t1p[5],t1_7i=t1pi[5];
    c_mul(t1_7r,t1_7i,t1p[2],t1pi[2],&t1_7r,&t1_7i);
    double t2_5r=t2p[5],t2_5i=t2pi[5];
    double cr,ci; c_cos(x1r,x1i,&cr,&ci);
    cRe[70]=t1_7r+t2_5r-t1p[3]-t2p[2]+cr-sr;
    cIm[70]=t1_7i+t2_5i-t1pi[3]-t2pi[2]+ci-si;
    /* for k=2..50: cf[k-1] = k*cf[k-2] + |cf[0]|/k */
    double absC0=c_abs(cRe[0],cIm[0]);
    for(int k=2;k<=50;k++){
        cRe[k-1]=(double)k*cRe[k-2]+absC0/(double)k;
        cIm[k-1]=(double)k*cIm[k-2];
    }
    /* for r=52..70: cf[r-1] = r*cf[r-2] + |cf[50]|/r */
    double absC50=c_abs(cRe[50],cIm[50]);
    for(int r=52;r<=70;r++){
        cRe[r-1]=(double)r*cRe[r-2]+absC50/(double)r;
        cIm[r-1]=(double)r*cIm[r-2];
    }
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_97_hand ----
 * Three loops: first sets all, second overwrites [0:10], third overwrites [60:71] reading cf[k-2]
 */
static void poly_97_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    /* z = t1 + i*t2 = (x1r - x2i) + i*(x1i + x2r) */
    double zr=x1r-x2i, zi=x1i+x2r;
    /* Loop 1: cf[k-1] = z * k^(-|t1|*log(|k+1|)) */
    double absT1=c_abs(x1r,x1i);
    for(int k=1;k<=71;k++){
        double ex = -absT1*log((double)(k+1));
        double kpow = pow((double)k, ex);
        cRe[k-1]=zr*kpow; cIm[k-1]=zi*kpow;
    }
    /* Loop 2: overwrite cf[0:10] */
    for(int k=1;k<=10;k++){
        double cr,ci,sr2,si2;
        c_cos(x1i+k*x2i, 0, &cr, &ci); /* cos(Im(t1+k*t2)) — Im is real */
        /* actually Im(t1+k*t2) = x1i + k*x2i, this is a real number */
        double imV = x1i + k*x2i;
        double reV = x1r - k*x2r; /* wait: Re(t1-k*t2) = x1r - k*x2r */
        double cosV = cos(imV);
        double sinV = sin(reV);
        double absV = fabs((double)(k*k*k)*cosV - sinV);
        cRe[k-1]=zr*absV; cIm[k-1]=zi*absV;
    }
    /* Loop 3: overwrite cf[60:71], reading cf[k-2] */
    for(int k=61;k<=71;k++){
        double imV=x1i+k*x2i;
        double cosV=cos(imV);
        double denom=fabs((double)(k*k*k)*cosV);
        if(denom<1e-30) denom=1e-30;
        double absPrev=c_abs(cRe[k-2],cIm[k-2]);
        double fac=(double)k*absPrev/denom;
        cRe[k-1]=zr*fac; cIm[k-1]=zi*fac;
    }
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* poly_102_serendipity: clamped recurrence version of poly_102.
 * The original overflows float32 (~k! growth). Clamping |cf[k]| to 1e30
 * creates a different but visually interesting pattern. */
static void poly_102_serendipity(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }
    #define CLAMP102 1e30
    /* cf[0] = 1000 * (t1 + t2)**2 */
    double sr = x1r + x2r, si = x1i + x2i;
    double s2r, s2i;
    c_mul(sr, si, sr, si, &s2r, &s2i);
    cRe[0] = 1000.0 * s2r; cIm[0] = 1000.0 * s2i;

    for (int k = 1; k < 20; k++) {
        double kp1 = (double)(k + 1);
        double pr = kp1 * cRe[k-1], pi_ = kp1 * cIm[k-1];
        double ar = kp1 * x1r, ai = kp1 * x1i;
        double snr, sni; c_sin(ar, ai, &snr, &sni);
        double br = kp1 * x2r, bi = kp1 * x2i;
        double csr, csi; c_cos(br, bi, &csr, &csi);
        cRe[k] = pr + snr + csr;
        cIm[k] = pi_ + sni + csi;
        double m = c_abs(cRe[k], cIm[k]);
        if (m > CLAMP102) { double s = CLAMP102/m; cRe[k]*=s; cIm[k]*=s; }
    }
    for (int k = 20; k < 40; k++) {
        double kp1 = (double)(k + 1);
        double pr = kp1 * cRe[k-1], pi_ = kp1 * cIm[k-1];
        double ar = kp1 * x1r, ai = kp1 * x1i;
        double snr, sni; c_sin(ar, ai, &snr, &sni);
        double br = kp1 * x2r, bi = kp1 * x2i;
        double csr, csi; c_cos(br, bi, &csr, &csi);
        cRe[k] = pr - snr - csr;
        cIm[k] = pi_ - sni - csi;
        double m = c_abs(cRe[k], cIm[k]);
        if (m > CLAMP102) { double s = CLAMP102/m; cRe[k]*=s; cIm[k]*=s; }
    }
    for (int k = 40; k < 60; k++) {
        double kp1 = (double)(k + 1);
        double pr = kp1 * cRe[k-1], pi_ = kp1 * cIm[k-1];
        double t12r, t12i; c_mul(x1r, x1i, x2r, x2i, &t12r, &t12i);
        double ar = kp1 * t12r, ai = kp1 * t12i;
        double snr, sni; c_sin(ar, ai, &snr, &sni);
        double csr, csi; c_cos(ar, ai, &csr, &csi);
        cRe[k] = pr + snr + csr;
        cIm[k] = pi_ + sni + csi;
        double m = c_abs(cRe[k], cIm[k]);
        if (m > CLAMP102) { double s = CLAMP102/m; cRe[k]*=s; cIm[k]*=s; }
    }
    for (int k = 60; k < 70; k++) {
        double kp1 = (double)(k + 1);
        double pr = kp1 * cRe[k-1], pi_ = kp1 * cIm[k-1];
        double t12r, t12i; c_mul(x1r, x1i, x2r, x2i, &t12r, &t12i);
        double ar = kp1 * t12r, ai = kp1 * t12i;
        double snr, sni; c_sin(ar, ai, &snr, &sni);
        double csr, csi; c_cos(ar, ai, &csr, &csi);
        cRe[k] = pr - snr - csr;
        cIm[k] = pi_ - sni - csi;
        double m = c_abs(cRe[k], cIm[k]);
        if (m > CLAMP102) { double s = CLAMP102/m; cRe[k]*=s; cIm[k]*=s; }
    }
    /* cf[70] = |cf[69]| + angle(t1) - angle(t2) + real(t1*t2) - imag(conj(t1)*t2) */
    double abs69 = c_abs(cRe[69], cIm[69]);
    double ang1 = c_arg(x1r, x1i), ang2 = c_arg(x2r, x2i);
    double t12r, t12i; c_mul(x1r, x1i, x2r, x2i, &t12r, &t12i);
    double ct12r, ct12i; c_mul(x1r, -x1i, x2r, x2i, &ct12r, &ct12i);
    cRe[70] = abs69 + ang1 - ang2 + t12r - ct12i;
    cIm[70] = 0;
    #undef CLAMP102

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* poly_117: loop + whole-array cf *= expr + slice augassign
 * cf[k-1] = (t1+t2)^(k-1) + (-1)^k * exp(i*k*pi/71) * k^(1/3)
 * cf *= (1 + log(|cf|+1) / (1 + |t1*t2|))
 * cf[0:10] += (t1^2+t2^2)^(1/3)
 * cf[61:71] *= exp(-i*angle(t1)) */
static void poly_117_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double sr = x1r + x2r, si = x1i + x2i;
    double t12r, t12i; c_mul(x1r, x1i, x2r, x2i, &t12r, &t12i);
    double abs_t12 = c_abs(t12r, t12i);

    /* cf[k-1] = (t1+t2)^(k-1) + (-1)^k * exp(i*k*pi/71) * k^(1/3) */
    for (int k = 1; k <= 71; k++) {
        double pr, pi_;
        c_powr(sr, si, (double)(k - 1), &pr, &pi_);
        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        double ang = (double)k * M_PI / 71.0;
        double er = sign * cos(ang), ei = sign * sin(ang);
        double cbrtk = cbrt((double)k);
        cRe[k-1] = pr + er * cbrtk;
        cIm[k-1] = pi_ + ei * cbrtk;
    }

    /* cf *= (1 + log(|cf|+1) / (1 + |t1*t2|)) — element-wise */
    for (int i = 0; i < 71; i++) {
        double m = c_abs(cRe[i], cIm[i]);
        double fac = 1.0 + log(m + 1.0) / (1.0 + abs_t12);
        cRe[i] *= fac;
        cIm[i] *= fac;
    }

    /* cf[0:10] += (t1^2+t2^2)^(1/3) */
    double t1sq_r, t1sq_i; c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    double sumr = t1sq_r + t2sq_r, sumi = t1sq_i + t2sq_i;
    double cbr, cbi; c_powr(sumr, sumi, 1.0/3.0, &cbr, &cbi);
    for (int i = 0; i < 10; i++) {
        cRe[i] += cbr;
        cIm[i] += cbi;
    }

    /* cf[61:71] *= exp(-i*angle(t1)) */
    double ang1 = c_arg(x1r, x1i);
    double expr = cos(-ang1), expi = sin(-ang1);
    for (int i = 61; i < 71; i++) {
        double tr = cRe[i]*expr - cIm[i]*expi;
        cIm[i] = cRe[i]*expi + cIm[i]*expr;
        cRe[i] = tr;
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* poly_118: primes array used in scalar sums and slice multiplications.
 * Transpiler passes array to c_mul — needs hand-written element-wise loops. */
static void poly_118_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    static const double primes[] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53};
    /* np.sum(primes) = 381, np.sum(primes[:8]) = 77 */
    double sumP = 381.0, sumP8 = 77.0;

    /* f1 = t1 * sumP + t2 */
    double f1r = x1r * sumP + x2r, f1i = x1i * sumP + x2i;
    /* f2 = t2 * sumP8 + conj(t1) */
    double f2r = x2r * sumP8 + x1r, f2i = x2i * sumP8 - x1i;

    /* cf[0:16] = primes[:16] * (t1 - t2) */
    double dr = x1r - x2r, di = x1i - x2i;
    for (int i = 0; i < 16; i++) {
        cRe[i] = primes[i] * dr;
        cIm[i] = primes[i] * di;
    }

    /* cf[16:32] = f1^2 - f2^2 */
    double f1sq_r, f1sq_i; c_mul(f1r, f1i, f1r, f1i, &f1sq_r, &f1sq_i);
    double f2sq_r, f2sq_i; c_mul(f2r, f2i, f2r, f2i, &f2sq_r, &f2sq_i);
    double diffr = f1sq_r - f2sq_r, diffi = f1sq_i - f2sq_i;
    for (int i = 16; i < 32; i++) {
        cRe[i] = diffr;
        cIm[i] = diffi;
    }

    /* cf[32:48] = (t1^3 - t2^3) * (primes[:16] - f1) */
    double t1cu_r, t1cu_i; c_mul(x1r, x1i, x1r, x1i, &t1cu_r, &t1cu_i);
    c_mul(t1cu_r, t1cu_i, x1r, x1i, &t1cu_r, &t1cu_i);
    double t2cu_r, t2cu_i; c_mul(x2r, x2i, x2r, x2i, &t2cu_r, &t2cu_i);
    c_mul(t2cu_r, t2cu_i, x2r, x2i, &t2cu_r, &t2cu_i);
    double cubdr = t1cu_r - t2cu_r, cubdi = t1cu_i - t2cu_i;
    for (int i = 0; i < 16; i++) {
        double pr = primes[i] - f1r, pi_ = -f1i;
        double rr, ri; c_mul(cubdr, cubdi, pr, pi_, &rr, &ri);
        cRe[32 + i] = rr;
        cIm[32 + i] = ri;
    }

    /* cf[48:64] = (primes[:16] * t1^2 + t2^3) - t1 */
    double t1sq_r2, t1sq_i2; c_mul(x1r, x1i, x1r, x1i, &t1sq_r2, &t1sq_i2);
    for (int i = 0; i < 16; i++) {
        cRe[48 + i] = primes[i] * t1sq_r2 + t2cu_r - x1r;
        cIm[48 + i] = primes[i] * t1sq_i2 + t2cu_i - x1i;
    }

    /* cf[64:70] = sin(cf[0:6]*t2) + cos(cf[0:6]*t1) */
    for (int i = 0; i < 6; i++) {
        double ar, ai; c_mul(cRe[i], cIm[i], x2r, x2i, &ar, &ai);
        double snr, sni; c_sin(ar, ai, &snr, &sni);
        double br, bi; c_mul(cRe[i], cIm[i], x1r, x1i, &br, &bi);
        double csr, csi; c_cos(br, bi, &csr, &csi);
        cRe[64 + i] = snr + csr;
        cIm[64 + i] = sni + csi;
    }

    /* cf[70] = prod(primes[:9]) = 2*3*5*7*11*13*17*19*23 = 223092870 */
    cRe[70] = 223092870.0;
    cIm[70] = 0;

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_120_hand ----
 * Python:
 *   theta = angle(t1) * angle(t2)
 *   mult_factors[i] = (i%2==0) ? 1 : -1   (for i=0..69)
 *   cf[0:10]  = (k+1)*t1^2 - (10-k)*t2^2   for k=0..9
 *   cf[10:40] = (k%2)*|t1| + (k%3)*|t2|*exp(k/5*theta*i)  for k=11..40 (1-indexed)
 *   cf[40:60] = ((k+41) + log(|theta|+1)) * conj(t1) * 5 * mult_factors[k]  for k=0..19
 *   cf[60:70] = (k+61) - (k+1)*t2 - (-5)   for k=0..9   [sum = -5]
 *   cf[70]    = 2556 / 15!  ≈ 1.954e-9
 */
static void poly_120_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double theta = c_arg(x1r, x1i) * c_arg(x2r, x2i);
    double abs_t1 = c_abs(x1r, x1i);
    double abs_t2 = c_abs(x2r, x2i);

    /* t1^2, t2^2 */
    double t1sq_r, t1sq_i; c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    double t2sq_r, t2sq_i; c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);

    /* conj(t1) */
    double conj_t1r = x1r, conj_t1i = -x1i;

    /* cf[0:10] = (k+1)*t1^2 - (10-k)*t2^2  for k=0..9 */
    for (int k = 0; k < 10; k++) {
        double a = (double)(k + 1);
        double b = (double)(10 - k);
        cRe[k] = a * t1sq_r - b * t2sq_r;
        cIm[k] = a * t1sq_i - b * t2sq_i;
    }

    /* cf[10:40]: for k=11..40 (1-indexed), cf[k-1] = (k%2)*|t1| + (k%3)*|t2|*exp(k/5*theta*i) */
    for (int k = 11; k <= 40; k++) {
        double base_r = (double)(k % 2) * abs_t1;
        double base_i = 0.0;
        /* (k%3)*|t2|*exp(k/5*theta*i) */
        double angle = ((double)k / 5.0) * theta;
        double er, ei; c_exp2(0.0, angle, &er, &ei);
        double scale = (double)(k % 3) * abs_t2;
        cRe[k - 1] = base_r + scale * er;
        cIm[k - 1] = base_i + scale * ei;
    }

    /* cf[40:60] = ((k+41) + log(|theta|+1)) * conj(t1) * 5 * mult_factors[k]  for k=0..19
     * mult_factors[k] = (k%2==0) ? 1 : -1 */
    double log_term = log(fabs(theta) + 1.0);
    for (int k = 0; k < 20; k++) {
        double scalar = ((double)(k + 41) + log_term) * 5.0;
        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        scalar *= sign;
        cRe[40 + k] = scalar * conj_t1r;
        cIm[40 + k] = scalar * conj_t1i;
    }

    /* cf[60:70] = (k+61) - (k+1)*t2 - (-5)  for k=0..9
     * The constant sum: sum((arange(1,11)) * mult_factors[10:20])
     *   mult_factors[10:20] = [1,-1,1,-1,1,-1,1,-1,1,-1]
     *   1*1 - 2 + 3 - 4 + 5 - 6 + 7 - 8 + 9 - 10 = -5  */
    for (int k = 0; k < 10; k++) {
        double real_part = (double)(k + 61) - (double)(k + 1) * x2r - (-5.0);
        double imag_part = -(double)(k + 1) * x2i;
        cRe[60 + k] = real_part;
        cIm[60 + k] = imag_part;
    }

    /* cf[70] = 2556 / 15! = 2556 / 1307674368000 */
    cRe[70] = 2556.0 / 1307674368000.0;
    cIm[70] = 0.0;

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* ---- poly_128_hand ----
 * Python:
 *   for k=1..71: cf[k-1] = (-1)^k * (t1^k + conj(t2)^(71-k)) * (72-k)
 */
static void poly_128_hand(double x1r, double x1i, double x2r, double x2i,
                           const double *cfpv, int n_cfpv,
                           double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* conj(t2) */
    double ct2r = x2r, ct2i = -x2i;

    /* Build t1^k incrementally and conj(t2)^(71-k) via c_powr */
    double t1pk_r = x1r, t1pk_i = x1i; /* t1^1 */
    for (int k = 1; k <= 71; k++) {
        /* t1^k (already computed for current k) */
        /* conj(t2)^(71-k) */
        double ct2p_r, ct2p_i;
        int exp2 = 71 - k;
        if (exp2 == 0) {
            ct2p_r = 1.0; ct2p_i = 0.0;
        } else {
            c_powr(ct2r, ct2i, (double)exp2, &ct2p_r, &ct2p_i);
        }

        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        double scale = sign * (double)(72 - k);
        double sum_r = t1pk_r + ct2p_r;
        double sum_i = t1pk_i + ct2p_i;
        cRe[k - 1] = scale * sum_r;
        cIm[k - 1] = scale * sum_i;

        /* advance t1^(k+1) = t1^k * t1 */
        if (k < 71) {
            double nr, ni;
            c_mul(t1pk_r, t1pk_i, x1r, x1i, &nr, &ni);
            t1pk_r = nr; t1pk_i = ni;
        }
    }

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* poly_125: 5 list-comp slices + 1 loop + cf[70]=1.
 * Python source has bug: cf[61:70] = [... range(61,71)] — 9 slots, 10 values → crash.
 * Hand-written to match the fixed (intended) behavior. */
static void poly_125_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double abs1 = c_abs(x1r, x1i);
    double abs2 = c_abs(x2r, x2i);

    /* cf[0:15] = [(-1)**j * j**2 * (|t1| + |t2|) for j in range(1,16)] */
    for (int j = 1; j <= 15; j++) {
        double sign = (j % 2 == 0) ? 1.0 : -1.0;
        cRe[j-1] = sign * (double)(j*j) * (abs1 + abs2);
    }

    /* cf[15:30] = [(-1)**(k+1) * k**3 * angle(t1 + 1j*t2) for k in range(16,31)] */
    double sr = x1r - x2i, si = x1i + x2r;  /* t1 + 1j*t2 */
    double ang_s = c_arg(sr, si);
    for (int k = 16; k <= 30; k++) {
        double sign = ((k+1) % 2 == 0) ? 1.0 : -1.0;
        cRe[k-1] = sign * (double)(k*k*k) * ang_s;
    }

    /* cf[30:45] = [(-1)**(r+1)*cos(r*t1) + sin(r*t2) for r in range(31,46)] */
    for (int r = 31; r <= 45; r++) {
        double sign = ((r+1) % 2 == 0) ? 1.0 : -1.0;
        double ar = (double)r*x1r, ai = (double)r*x1i;
        double cosr, cosi; c_cos(ar, ai, &cosr, &cosi);
        double br = (double)r*x2r, bi = (double)r*x2i;
        double sinr, sini; c_sin(br, bi, &sinr, &sini);
        cRe[r-1] = sign*cosr + sinr;
        cIm[r-1] = sign*cosi + sini;
    }

    /* for s in range(46,61): cf[s] = (-1)**s * s**2 * conj(t1) * conj(t2) */
    double cpr, cpi; c_mul(x1r, -x1i, x2r, -x2i, &cpr, &cpi);
    for (int s = 46; s <= 60; s++) {
        double sign = (s % 2 == 0) ? 1.0 : -1.0;
        double fac = sign * (double)(s*s);
        cRe[s] = fac * cpr;
        cIm[s] = fac * cpi;
    }

    /* cf[61:71] = [n**3 * log(|t1*t2| + 1) for n in range(61,71)]
     * Slice indices 61..70, list values n=61..70. cf[61+i] = (61+i)^3 * logval. */
    double t12r, t12i; c_mul(x1r, x1i, x2r, x2i, &t12r, &t12i);
    double logval = log(c_abs(t12r, t12i) + 1.0);
    for (int i = 0; i < 10; i++) {
        int n = 61 + i;
        cRe[61 + i] = (double)(n*n*n) * logval;
    }

    /* cf[70] = 1 */
    cRe[70] = 1.0;

    /* Cap coefficient magnitudes at 1e10 — serendipity version.
     * Original poly_125 has ~16 orders dynamic range which float32 can't handle.
     * Capping creates a different but potentially interesting polynomial. */
    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; continue; }
        double m = c_abs(cRe[i], cIm[i]);
        if (m > 1e10) { double s = 1e10 / m; cRe[i] *= s; cIm[i] *= s; }
    }
}

/* ---- poly_645_hand ----
 * Python:
 *   degree = 8
 *   real_seq = linspace(t1.real, t2.real, 9)
 *   im_seq   = linspace(t1.imag, t2.imag, 9)
 *   for j in 1..9:
 *       mag_component = log(|t1| + j) * sin(j*pi/4) + cos(j*pi/3) * |t2|
 *       angle_component = angle(t1) * j + angle(t2) * (degree + 1 - j)  # unused
 *       intricate_part = exp(1j * (sin(real_seq[j-1]) + cos(im_seq[j-1])))
 *       cf[j-1] = mag_component * intricate_part * conj(t2) + prod(1..j) * sin(j)
 */
static void poly_645_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    for (int i = 0; i < 9; i++) { cRe[i] = 0; cIm[i] = 0; }

    const double abs1 = c_abs(x1r, x1i);
    const double abs2 = c_abs(x2r, x2i);
    double fact = 1.0;

    for (int j = 1; j <= 9; j++) {
        const double mag_component =
            log(abs1 + (double)j) * sin((double)j * M_PI / 4.0) +
            cos((double)j * M_PI / 3.0) * abs2;

        /* Sample along the straight line in real/imag space. */
        const double real_sample = x1r + (x2r - x1r) * (double)(j - 1) / 8.0;
        const double imag_sample = x1i + (x2i - x1i) * (double)(j - 1) / 8.0;
        const double theta = sin(real_sample) + cos(imag_sample);

        double phase_r, phase_i;
        c_exp2(0.0, theta, &phase_r, &phase_i);

        double scaled_r, scaled_i;
        c_mul(mag_component, 0.0, phase_r, phase_i, &scaled_r, &scaled_i);

        double conj_mul_r, conj_mul_i;
        c_mul(scaled_r, scaled_i, x2r, -x2i, &conj_mul_r, &conj_mul_i);

        fact *= (double)j;
        cRe[j - 1] = conj_mul_r + fact * sin((double)j);
        cIm[j - 1] = conj_mul_i;
    }

    for (int i = 0; i < 9; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_795_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int i = 0; i < 25; i++) { cRe[i] = 0; cIm[i] = 0; }

    double x1x2r, x1x2i;
    c_mul(x1r, x1i, x2r, x2i, &x1x2r, &x1x2i);
    const double denom = 1.0 + c_abs(x1x2r, x1x2i);

    for (int i = 0; i < 25; i++) {
        const double base_r = (double)(i * i) + x1r;
        const double base_i = x1i;

        double exp_r, exp_i;
        c_exp2((double)i * x2r, (double)i * x2i, &exp_r, &exp_i);

        double num_r, num_i;
        c_mul(base_r, base_i, exp_r, exp_i, &num_r, &num_i);
        cRe[i] = num_r / denom;
        cIm[i] = num_i / denom;
    }

    for (int i = 4; i < 15; i++) {
        cRe[i] = cRe[i] * cos(cIm[i]);
        cIm[i] = 0.0;
    }

    double phase_r, phase_i;
    c_exp2(0.0, c_arg(x1r + x2r, x1i + x2i), &phase_r, &phase_i);
    for (int i = 16; i < 25; i++) {
        const double mag = c_abs(cRe[i], cIm[i]);
        c_mul(mag, 0.0, phase_r, phase_i, &cRe[i], &cIm[i]);
    }

    double jt2r, jt2i;
    c_mul(0.0, 1.0, x2r, x2i, &jt2r, &jt2i);
    const double zr = x1r + jt2r;
    const double zi = x1i + jt2i;
    const double czr = zr;
    const double czi = -zi;

    double z2r, z2i, z3r, z3i;
    c_mul(zr, zi, zr, zi, &z2r, &z2i);
    c_mul(z2r, z2i, zr, zi, &z3r, &z3i);

    double cz2r, cz2i, cz3r, cz3i;
    c_mul(czr, czi, czr, czi, &cz2r, &cz2i);
    c_mul(cz2r, cz2i, czr, czi, &cz3r, &cz3i);

    cRe[2] = z3r - cz3r;
    cIm[2] = z3i - cz3i;

    const double abs1 = c_abs(x1r, x1i);
    const double abs2 = c_abs(x2r, x2i);
    const double shared_mag = abs1 * abs1 * abs1 + abs2 * abs2 * abs2;
    double shared_r, shared_i;
    c_exp2(0.0, c_arg(x2r - x1r, x2i - x1i), &phase_r, &phase_i);
    c_mul(shared_mag, 0.0, phase_r, phase_i, &shared_r, &shared_i);
    cRe[6] = shared_r;  cIm[6] = shared_i;
    cRe[22] = shared_r; cIm[22] = shared_i;

    cRe[18] = (abs2 > 1.0) ? log(abs2) : 0.0;
    cIm[18] = 0.0;

    for (int i = 0; i < 25; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_504_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    for (int i = 0; i < 35; i++) { cRe[i] = 0; cIm[i] = 0; }

    const int n = 35;
    const double abs1 = c_abs(x1r, x1i);
    const double abs2 = c_abs(x2r, x2i);
    const double ang1 = c_arg(x1r, x1i);
    const double ang2 = c_arg(x2r, x2i);

    for (int j = 1; j <= n; j++) {
        const double a = (double)(j - 1) / (double)(n - 1);
        const double rec = x1r + (x2r - x1r) * a;
        const double imc = x1i + (x2i - x1i) * a;
        const double magnitude =
            log(fabs(rec + imc) + 1.0) *
            (pow(abs1, (double)j) + pow(abs2, (double)(n - j)));
        const double angle = ang1 * sin((double)j) + ang2 * cos((double)j / 2.0);
        cRe[j - 1] = magnitude * cos(angle);
        cIm[j - 1] = magnitude * sin(angle);
    }

    double prod_rec = 1.0;
    double sum_imc = 0.0;
    double conj_pow_r = 1.0, conj_pow_i = 0.0;
    for (int k = 1; k <= n / 2; k++) {
        const double a = (double)(k - 1) / (double)(n - 1);
        const double rec = x1r + (x2r - x1r) * a;
        const double imc = x1i + (x2i - x1i) * a;
        prod_rec *= rec;
        sum_imc += imc;
        c_mul(conj_pow_r, conj_pow_i, x2r, -x2i, &conj_pow_r, &conj_pow_i);
        cRe[k - 1] += prod_rec * conj_pow_r;
        cIm[k - 1] += prod_rec * conj_pow_i;
        cRe[n - k] += sum_imc * sin(abs1 * (double)k) * cos(abs2 / (double)(k + 1));
    }

    for (int i = 0; i < 35; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_741_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 10;
    for (int i = 0; i < 10; i++) { cRe[i] = 0; cIm[i] = 0; }

    double a1r = x1r, a1i = x1i + 7.0;
    double a1sq_r, a1sq_i;
    c_mul(a1r, a1i, a1r, a1i, &a1sq_r, &a1sq_i);
    double a2r = x2r, a2i = x2i + 5.0;
    double a2sq_r, a2sq_i, a2cu_r, a2cu_i;
    c_mul(a2r, a2i, a2r, a2i, &a2sq_r, &a2sq_i);
    c_mul(a2sq_r, a2sq_i, a2r, a2i, &a2cu_r, &a2cu_i);
    cRe[0] = a1sq_r + a2cu_i;

    double prod_r, prod_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    c_exp2(-prod_i, prod_r, &cRe[1], &cIm[1]);

    double exp_t1_r, exp_t1_i, exp_t2_r, exp_t2_i;
    c_exp2(-x1i, x1r, &exp_t1_r, &exp_t1_i);
    c_exp2(x2i, -x2r, &exp_t2_r, &exp_t2_i);
    for (int i = 0; i < 4; i++) {
        const double imag_scale = (double)(i + 2);
        double log_r, log_i;
        c_log(0.0, imag_scale, &log_r, &log_i);
        cRe[2 + i] = log_r + exp_t1_r + exp_t2_r;
        cIm[2 + i] = log_i + exp_t1_i + exp_t2_i;
    }

    cRe[6] = cRe[3]; cIm[6] = cIm[3];
    cRe[7] = cRe[2]; cIm[7] = cIm[2];
    cRe[8] = cRe[1]; cIm[8] = cIm[1];
    cRe[9] = cRe[0]; cIm[9] = cIm[0];

    for (int i = 0; i < 10; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_742_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 10;
    for (int i = 0; i < 10; i++) { cRe[i] = 0; cIm[i] = 0; }

    double jt2r, jt2i;
    c_mul(0.0, 1.0, x2r, x2i, &jt2r, &jt2i);
    const double zr = x1r + jt2r;
    const double zi = x1i + jt2i;
    double pow_r = 1.0, pow_i = 0.0;
    double fact = 1.0;

    for (int k = 1; k <= 10; k++) {
        c_mul(pow_r, pow_i, zr, zi, &pow_r, &pow_i);
        fact *= (double)k;
        double phase_r, phase_i;
        c_exp2(0.0, sin((double)k * x2r), &phase_r, &phase_i);
        double scaled_r = pow_r / fact;
        double scaled_i = pow_i / fact;
        c_mul(scaled_r, scaled_i, phase_r, phase_i, &cRe[k - 1], &cIm[k - 1]);
    }

    double t1sq_r, t1sq_i, t1cu_r, t1cu_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(t1sq_r, t1sq_i, x1r, x1i, &t1cu_r, &t1cu_i);
    double t2sq_r, t2sq_i, t2cu_r, t2cu_i;
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    c_mul(t2sq_r, t2sq_i, x2r, x2i, &t2cu_r, &t2cu_i);

    cRe[0] = t1cu_r + t1sq_i + t2sq_r + x2i;
    cIm[0] = t1cu_i - t1sq_r + t2sq_i - x2r;

    cRe[4] = x2r * x1i + t2cu_i;
    cIm[4] = -t2cu_r;

    double phase_r, phase_i;
    c_exp2(0.0, x1r + x2r, &phase_r, &phase_i);
    const double amp = x1r * x1r * x2r * x2r;
    cRe[9] = amp * phase_r;
    cIm[9] = amp * phase_i;

    for (int i = 0; i < 10; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_760_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int i = 0; i < 11; i++) { cRe[i] = 0; cIm[i] = 0; }

    double sum_r = x1r + x2r, sum_i = x1i + x2i;
    const double abs_sum = c_abs(sum_r, sum_i);
    cRe[0] = sin((x1r + 2.0 * x2i) / (1.0 + abs_sum * abs_sum));
    cRe[1] = 100.0 * x1r * x1i *
             (pow(fabs(cos(x2r)), 2.1) - pow(fabs(sin(x2i)), 2.1));

    const double det = x2r * x1r + x2i * x1i;
    double vv = 0.0;
    if (fabs(det) >= 1e-10) {
        vv = (x1r - x2i + x1i + x2r) / det;
    }
    cRe[2] = 1000.0 * vv;

    /* sum(ifft([t1, t2])) = t1 for the 2-point inverse FFT */
    cRe[3] = x1r;
    cIm[3] = x1i;

    const double abs1 = c_abs(x1r, x1i);
    if (abs1 > 0.0) {
        cRe[4] = 10.0 * x1r;
        cIm[4] = 10.0 * x1i;
    }

    double vals[4] = {x1r, x1i, x2r, x2i};
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            if (vals[j] < vals[i]) {
                double tmp = vals[i];
                vals[i] = vals[j];
                vals[j] = tmp;
            }
        }
    }
    const double med = 0.5 * (vals[1] + vals[2]);
    cRe[5] = 1000.0 * med * med + 500.0 * med * med * med;
    cRe[6] = x2r;
    cIm[6] = 2.0 * sqrt(fabs(x2i));
    cRe[7] = 100.0 * log10(fabs(x2r) + fabs(x2i));

    double prod_r, prod_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    cRe[8] = sqrt(c_abs(prod_r, prod_i));
    cRe[9] = 1000.0 * (x1i * x1i * x1i - 3.0 * x1i * x1r * x1r);
    cRe[10] = 2.0 * (x1r + x1i);
    cIm[10] = 2.0 * (x1r - x1i);

    for (int i = 0; i < 11; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_762_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 10;
    for (int i = 0; i < 10; i++) { cRe[i] = 0; cIm[i] = 0; }

    const double primes[10] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    cRe[0] = x1r + x2r;
    cIm[0] = x1i + x2i;

    double t1sq_r, t1sq_i, t2sq_r, t2sq_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    const double base_r = t1sq_r + t2sq_r;
    const double base_i = t1sq_i + t2sq_i + 1.0;
    for (int i = 1; i < 9; i++) {
        cRe[i] = primes[i] * base_r;
        cIm[i] = primes[i] * base_i;
    }
    cRe[9] = 100.0 + c_abs(base_r, base_i - 1.0);

    for (int i = 0; i < 10; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_765_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    for (int i = 0; i < 11; i++) { cRe[i] = 0; cIm[i] = 0; }

    double pow_r, pow_i;
    c_powc(x1r, x1i, x2r, x2i, &pow_r, &pow_i);
    double t2sq_r, t2sq_i, t2cu_r, t2cu_i;
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    c_mul(t2sq_r, t2sq_i, x2r, x2i, &t2cu_r, &t2cu_i);
    double t1t2cu_r, t1t2cu_i;
    c_mul(x1r, x1i, t2cu_r, t2cu_i, &t1t2cu_r, &t1t2cu_i);
    double exp_r, exp_i;
    c_exp2(-x1i, x1r, &exp_r, &exp_i);
    const double abs2 = c_abs(x2r, x2i);
    double phase_r, phase_i;
    c_exp2(0.0, c_arg(x1r, x1i), &phase_r, &phase_i);

    c_mul(0.0, 1.0, t2cu_r, t2cu_i, &t2cu_r, &t2cu_i);

    cRe[0] = x1r * x2r - x1i * x2i;
    cIm[0] = x1r * x2i + x1i * x2r;
    cRe[1] = 100.0 * pow_r;
    cRe[2] = 10.0 * t1t2cu_r;
    cRe[3] = 200.0 * pow_i;
    cRe[4] = exp_r;
    cIm[4] = exp_i;
    cRe[5] = (x1r > 0.0) ? abs2 : ((x1r < 0.0) ? -abs2 : 0.0);
    const double amp = x1r * x1r + x2i * x2i;
    c_mul(amp, 0.0, phase_r, phase_i, &cRe[6], &cIm[6]);
    if (x1r > x2i) {
        cRe[7] = 1.0 - x2i;
        cIm[7] = x2r;
    } else {
        cRe[7] = x1r;
        cIm[7] = x1i + 100.0;
    }
    cRe[8] = x1r;
    cIm[8] = x1i + 1.0;
    c_exp2(-x2i, x2r, &exp_r, &exp_i);
    cRe[9] = 10.0 * (0.1 + exp_r);
    cIm[9] = 10.0 * exp_i;
    cRe[10] = 0.001 + t2cu_r;
    cIm[10] = t2cu_i;

    for (int i = 0; i < 11; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_776_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 10;
    for (int i = 0; i < 10; i++) { cRe[i] = 0; cIm[i] = 0; }

    double sum_r = x1r + x2r, sum_i = x1i + x2i;
    double diff_r = x1r - x2r, diff_i = x1i - x2i;
    double s_r, s_i, c_r, c_i;
    c_sin(sum_r, sum_i, &s_r, &s_i);
    cRe[0] = 1.0 + s_r;
    cIm[0] = s_i;

    double exp_r, exp_i, prod_r, prod_i;
    c_exp2(x2i, -x2r, &exp_r, &exp_i);
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    c_mul(prod_r, prod_i, exp_r, exp_i, &cRe[1], &cIm[1]);

    if (c_abs(diff_r, diff_i) < 1.0) {
        cRe[2] = 100.0 * x1r;
        cIm[2] = 100.0 * x1i;
    } else {
        cRe[2] = 100.0 * x2r;
        cIm[2] = 100.0 * x2i;
    }

    cRe[3] = 200.0 * (x1r - x1i);

    double t1sq_r, t1sq_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    cRe[4] = 100.0 * t1sq_r + 100.0 * x2i;
    cIm[4] = 100.0 * t1sq_i - 100.0 * x2r;

    double den = 1.0 + c_abs(x2r, x2i);
    double q_r, q_i;
    c_div(x1r, x1i, den, 0.0, &q_r, &q_i);
    double sin_q_r, sin_q_i, cos_q_r, cos_q_i;
    c_sin(q_r, q_i, &sin_q_r, &sin_q_i);
    c_cos(q_r, q_i, &cos_q_r, &cos_q_i);
    c_div(sin_q_r, sin_q_i, cos_q_r, cos_q_i, &cRe[5], &cIm[5]);

    c_exp2(-diff_i, diff_r, &exp_r, &exp_i);
    c_mul(prod_r, prod_i, exp_r, exp_i, &q_r, &q_i);
    cIm[6] = 100.0 * q_r;

    double sin_t2_r, sin_t2_i, sin_t2_sq_r, sin_t2_sq_i, sin_t2_cu_r, sin_t2_cu_i;
    double cos_t2_r, cos_t2_i, sin_t1_r, sin_t1_i;
    c_sin(x2r, x2i, &sin_t2_r, &sin_t2_i);
    c_mul(sin_t2_r, sin_t2_i, sin_t2_r, sin_t2_i, &sin_t2_sq_r, &sin_t2_sq_i);
    c_mul(sin_t2_sq_r, sin_t2_sq_i, sin_t2_r, sin_t2_i, &sin_t2_cu_r, &sin_t2_cu_i);
    c_cos(x2r, x2i, &cos_t2_r, &cos_t2_i);
    c_sin(x1r, x1i, &sin_t1_r, &sin_t1_i);
    c_mul(t1sq_r, t1sq_i, cos_t2_r, cos_t2_i, &q_r, &q_i);
    c_mul(q_r, q_i, sin_t1_r, sin_t1_i, &q_r, &q_i);
    cRe[7] = -100.0 * sin_t2_cu_r + q_r;
    cIm[7] = -100.0 * sin_t2_cu_i + q_i;

    double add_sq_r, add_sq_i, add_4_r, add_4_i;
    c_mul(sum_r, sum_i, sum_r, sum_i, &add_sq_r, &add_sq_i);
    c_mul(add_sq_r, add_sq_i, add_sq_r, add_sq_i, &add_4_r, &add_4_i);
    double diff_sq_r, diff_sq_i, diff_4_r, diff_4_i;
    c_mul(diff_r, diff_i, diff_r, diff_i, &diff_sq_r, &diff_sq_i);
    c_mul(diff_sq_r, diff_sq_i, diff_sq_r, diff_sq_i, &diff_4_r, &diff_4_i);
    cRe[8] = add_4_r - diff_4_r;
    cIm[8] = add_4_i - diff_4_i;

    double t_r, t_i;
    c_mul(cRe[1], cIm[1], cRe[7], cIm[7], &t_r, &t_i);
    double u_r, u_i;
    c_mul(cRe[0], cIm[0], cRe[8], cIm[8], &u_r, &u_i);
    cRe[9] = t_r - u_r;
    cIm[9] = t_i - u_i;

    for (int i = 0; i < 10; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_780_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int i = 0; i < 25; i++) { cRe[i] = 0; cIm[i] = 0; }

    double sum_r = x1r + x2r, sum_i = x1i + x2i;
    double prod_r, prod_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    cRe[0] = x1r * x1r - x1i * x1i + x2r * x2r - x2i * x2i - prod_r;
    cIm[0] = 2.0 * x1r * x1i + 2.0 * x2r * x2i - prod_i;

    for (int k = 1; k < 25; k++) {
        double tmp_r, tmp_i;
        c_mul(cRe[k - 1], cIm[k - 1], sum_r, sum_i, &tmp_r, &tmp_i);
        const double scale = 1.0 / (1.0 + c_abs(cRe[k - 1], cIm[k - 1]));
        cRe[k] = tmp_r * scale;
        cIm[k] = tmp_i * scale;
    }

    const int add_idx[] = {2, 5, 8, 11, 14, 17, 20, 23};
    const int sub_idx[] = {1, 4, 7, 10, 13, 16, 19, 22, 24};
    double jt2r, jt2i;
    c_mul(0.0, 1.0, x2r, x2i, &jt2r, &jt2i);
    const double add_r = x1r + jt2r;
    const double add_i = x1i + jt2i;
    double jt1r, jt1i;
    c_mul(0.0, 1.0, x1r, x1i, &jt1r, &jt1i);
    const double sub_r = x2r + jt1r;
    const double sub_i = x2i + jt1i;
    for (int i = 0; i < 8; i++) {
        cRe[add_idx[i]] += add_r;
        cIm[add_idx[i]] += add_i;
    }
    for (int i = 0; i < 9; i++) {
        cRe[sub_idx[i]] -= sub_r;
        cIm[sub_idx[i]] -= sub_i;
    }

    for (int i = 0; i < 25; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_792_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int i = 0; i < 25; i++) { cRe[i] = 0; cIm[i] = 0; }

    double sum_r = x1r + x2r, sum_i = x1i + x2i;
    double diff_r = x1r - x2r, diff_i = x1i - x2i;
    double sin_r, sin_i, cos_r, cos_i;
    c_sin(sum_r, sum_i, &sin_r, &sin_i);
    c_cos(diff_r, diff_i, &cos_r, &cos_i);
    cRe[0] = sin_r - cos_i;
    cIm[0] = sin_i + cos_r;

    const double log_abs1 = log(c_abs(x1r, x1i));
    const double log_abs2 = log(c_abs(x2r, x2i));
    for (int k = 1; k < 25; k++) {
        double exp_r, exp_i;
        c_exp2((double)k * cRe[k - 1], (double)k * cIm[k - 1], &exp_r, &exp_i);
        double vr = exp_r + log_abs1 - log_abs2;
        double vi = exp_i;
        if (isfinite(vr) && isfinite(vi)) {
            cRe[k] = vr;
            cIm[k] = vi;
        } else {
            cRe[k] = 1.0;
            cIm[k] = 1.0;
        }
    }

    for (int i = 0; i < 25; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_799_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int i = 0; i < 25; i++) { cRe[i] = 0; cIm[i] = 0; }

    double prod_r, prod_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    const double abs1 = c_abs(x1r, x1i);
    const double abs2 = c_abs(x2r, x2i);
    const double a = 2.0 * prod_r + 3.0 * prod_i;
    const double b = 2.0 * c_arg(prod_r, prod_i) - 3.0 * abs2;
    for (int i = 0; i < 25; i++) {
        cRe[i] = a * pow(abs1, (double)(i + 1)) + b * pow(abs2, (double)(i + 1));
        if (!isfinite(cRe[i])) { cRe[i] = 0.0; cIm[i] = 0.0; }
    }

    cRe[0] += prod_r;
    cIm[0] += -prod_i;
    if (!isfinite(cRe[0]) || !isfinite(cIm[0])) { cRe[0] = 0.0; cIm[0] = 0.0; }

    double sum_r = x1r + x2r, sum_i = x1i + x2i;
    const double abs_sum = c_abs(sum_r, sum_i);
    if (abs_sum <= 0.0) {
        cRe[12] = 0.0;
        cIm[12] = 0.0;
    } else {
        cRe[12] *= log(abs_sum);
        cIm[12] *= log(abs_sum);
        if (!isfinite(cRe[12]) || !isfinite(cIm[12])) { cRe[12] = 0.0; cIm[12] = 0.0; }
    }

    if (abs_sum <= 1e-30) {
        cRe[24] = 0.0;
        cIm[24] = 0.0;
    } else {
        double num_r, num_i, div_r, div_i;
        c_mul(cRe[24], cIm[24], prod_r, prod_i, &num_r, &num_i);
        c_div(num_r, num_i, sum_r, sum_i, &div_r, &div_i);
        cRe[24] -= div_r;
        cIm[24] -= div_i;
        if (!isfinite(cRe[24]) || !isfinite(cIm[24])) { cRe[24] = 0.0; cIm[24] = 0.0; }
    }

    for (int i = 0; i < 25; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_802_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int i = 0; i < 25; i++) { cRe[i] = 0; cIm[i] = 0; }

    cRe[0] = 2.0 * x1r + 3.0 * x2r;
    cIm[0] = 2.0 * x1i + 3.0 * x2i;
    cRe[1] = 3.0 * x1r - 2.0 * x2r;
    cIm[1] = 3.0 * x1i - 2.0 * x2i;
    double t1sq_r, t1sq_i, t2sq_r, t2sq_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    cRe[2] = t1sq_r - t2sq_r;
    cIm[2] = t1sq_i - t2sq_i;
    double prod_r, prod_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    cRe[3] = prod_r;
    cRe[4] = prod_i;

    for (int k = 5; k < 20; k++) {
        double s_r, s_i, c_r, c_i;
        c_sin(cRe[k - 1], cIm[k - 1], &s_r, &s_i);
        c_cos(cRe[k - 2], cIm[k - 2], &c_r, &c_i);
        cRe[k] = s_r + c_r;
        cIm[k] = s_i + c_i;
        const double mod = c_abs(cRe[k], cIm[k]);
        if (mod != 0.0) {
            cRe[k] /= mod;
            cIm[k] /= mod;
        } else {
            cRe[k] = 1.0;
            cIm[k] = 0.0;
        }
    }

    for (int i = 0; i < 25; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_812_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int i = 0; i < 25; i++) { cRe[i] = 0; cIm[i] = 0; }

    double prod_r, prod_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    cRe[0] = prod_r;
    cIm[0] = prod_i;
    const double offset = x1r + x2r;

    for (int k = 1; k < 25; k++) {
        double sq_r, sq_i;
        c_mul(cRe[k - 1], cIm[k - 1], cRe[k - 1], cIm[k - 1], &sq_r, &sq_i);
        const double den = 1.0 + c_abs(cRe[k - 1], cIm[k - 1]);
        cRe[k] = (sq_r + offset) / den;
        cIm[k] = sq_i / den;
        if (c_abs(cRe[k], cIm[k]) > 1e6 || !isfinite(cRe[k]) || !isfinite(cIm[k])) {
            cRe[k] = cRe[k - 1];
            cIm[k] = cIm[k - 1];
        }
    }

    for (int i = 0; i < 25; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_111_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    const double primes[20] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
                               31, 37, 41, 43, 47, 53, 59, 61, 67, 71};
    const double abs1 = c_abs(x1r, x1i);
    const double abs2 = c_abs(x2r, x2i);
    double sum_t_r = x1r + x2r, sum_t_i = x1i + x2i;
    double diff_t_r = x1r - x2r, diff_t_i = x1i - x2i;
    const double theta = c_arg(sum_t_r, sum_t_i);

    for (int r = 1; r <= 20; r++) {
        double t1p_r, t1p_i, t2p_r, t2p_i;
        c_powr(x1r, x1i, (double)r, &t1p_r, &t1p_i);
        c_powr(x2r, x2i, (double)r, &t2p_r, &t2p_i);
        double cyc_r = 1.0, cyc_i = 0.0;
        double t1n_r, t1n_i;
        c_powr(x1r, x1i, (double)(r + 1), &t1n_r, &t1n_i);
        for (int m = 0; m < r + 1; m++) {
            double phase_r, phase_i, tmp_r, tmp_i;
            c_exp2(0.0, 2.0 * M_PI * (double)m / (double)(r + 1), &phase_r, &phase_i);
            c_mul(t1n_r, t1n_i, phase_r, phase_i, &tmp_r, &tmp_i);
            tmp_r = 1.0 - tmp_r;
            tmp_i = -tmp_i;
            c_mul(cyc_r, cyc_i, tmp_r, tmp_i, &cyc_r, &cyc_i);
        }
        double num_r = primes[r - 1] * (t1p_r + t2p_r);
        double num_i = primes[r - 1] * (t1p_i + t2p_i);
        c_div(num_r, num_i, cyc_r, cyc_i, &cRe[r - 1], &cIm[r - 1]);
    }

    for (int r = 21; r <= 30; r++) {
        int n = (int)primes[r - 21];
        double cyc_r = 1.0, cyc_i = 0.0;
        double t1n_r, t1n_i;
        c_powr(x1r, x1i, (double)n, &t1n_r, &t1n_i);
        for (int m = 0; m < n; m++) {
            double phase_r, phase_i, tmp_r, tmp_i;
            c_exp2(0.0, 2.0 * M_PI * (double)m / (double)n, &phase_r, &phase_i);
            c_mul(t1n_r, t1n_i, phase_r, phase_i, &tmp_r, &tmp_i);
            tmp_r = 1.0 - tmp_r;
            tmp_i = -tmp_i;
            c_mul(cyc_r, cyc_i, tmp_r, tmp_i, &cyc_r, &cyc_i);
        }
        c_mul(cyc_r, cyc_i, sum_t_r, sum_t_i, &cRe[r - 1], &cIm[r - 1]);
    }

    for (int r = 31; r <= 45; r++) {
        cRe[r - 1] = ((double)(r - 30) * c_abs(diff_t_r, diff_t_i) * cos((double)r * theta)) /
                     (1.0 + pow(abs1, (double)r) + pow(abs2, (double)r));
    }

    for (int r = 46; r <= 60; r++) {
        double s1_r, s1_i, c2_r, c2_i;
        c_sin(primes[r - 46] * x1r, primes[r - 46] * x1i, &s1_r, &s1_i);
        c_cos(primes[r - 46] * x2r, primes[r - 46] * x2i, &c2_r, &c2_i);
        double base_r = s1_r + c2_r;
        double base_i = s1_i + c2_i;
        const int p = r - 45;
        double rot_r = 1.0, rot_i = 0.0;
        switch (p % 4) {
            case 1: rot_r = 0.0; rot_i = -1.0; break;
            case 2: rot_r = -1.0; rot_i = 0.0; break;
            case 3: rot_r = 0.0; rot_i = 1.0; break;
            default: break;
        }
        double tmp_r, tmp_i;
        c_mul(base_r, base_i, rot_r, rot_i, &tmp_r, &tmp_i);
        cRe[r - 1] = tmp_r / tgamma((double)r + 1.0);
        cIm[r - 1] = tmp_i / tgamma((double)r + 1.0);
    }

    double t1sq_r, t1sq_i, t2sq_r, t2sq_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    const double real_diff_sq = t2sq_r - t1sq_r;
    for (int r = 61; r <= 70; r++) {
        int n = r - 50;
        double cyc_r = 1.0, cyc_i = 0.0;
        double t1n_r, t1n_i;
        c_powr(x1r, x1i, (double)n, &t1n_r, &t1n_i);
        for (int m = 0; m < n; m++) {
            double phase_r, phase_i, tmp_r, tmp_i;
            c_exp2(0.0, 2.0 * M_PI * (double)m / (double)n, &phase_r, &phase_i);
            c_mul(t1n_r, t1n_i, phase_r, phase_i, &tmp_r, &tmp_i);
            tmp_r = 1.0 - tmp_r;
            tmp_i = -tmp_i;
            c_mul(cyc_r, cyc_i, tmp_r, tmp_i, &cyc_r, &cyc_i);
        }
        c_div(log(abs1 + (double)r) * real_diff_sq, 0.0,
              primes[r - 61] * cyc_r, primes[r - 61] * cyc_i,
              &cRe[r - 1], &cIm[r - 1]);
    }

    double sum35_r = 0.0, sum35_i = 0.0;
    for (int i = 0; i < 35; i++) {
        sum35_r += cRe[i];
        sum35_i += cIm[i];
    }
    double prod35_r = 1.0, prod35_i = 0.0;
    for (int i = 35; i < 70; i++) {
        double tmp_r, tmp_i;
        c_mul(prod35_r, prod35_i, cRe[i], cIm[i], &tmp_r, &tmp_i);
        prod35_r = tmp_r;
        prod35_i = tmp_i;
    }
    cRe[70] = sum35_r + prod35_r;
    cIm[70] = -sum35_i + prod35_i;

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_112_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    for (int i = 0; i < 71; i++) { cRe[i] = 0; cIm[i] = 0; }

    double t1sq_r, t1sq_i, t2sq_r, t2sq_i;
    c_mul(x1r, x1i, x1r, x1i, &t1sq_r, &t1sq_i);
    c_mul(x2r, x2i, x2r, x2i, &t2sq_r, &t2sq_i);
    const double phi10 = 2.0 * M_PI / 10.0;
    const double phi5 = 2.0 * M_PI / 5.0;
    const double abs_t2 = c_abs(x2r, x2i);
    for (int i = 0; i < 10; i++) {
        double base_r, base_i;
        c_exp2(0.0, phi10 * (double)i, &base_r, &base_i);
        cRe[i] = base_r + t1sq_r + abs_t2;
        cIm[i] = base_i + t1sq_i;
    }
    const double ang1 = c_arg(x1r, x1i);
    for (int i = 0; i < 5; i++) {
        cRe[10 + i] = exp((double)(i + 2) * ang1) - x2r;
    }

    double twojt2_r, twojt2_i;
    c_mul(0.0, 2.0, x2r, x2i, &twojt2_r, &twojt2_i);
    const double mix_r = x1r + twojt2_r;
    const double mix_i = x1i + twojt2_i;
    for (int i = 0; i < 5; i++) {
        double phase_r, phase_i;
        c_exp2(0.0, phi5 * (double)i, &phase_r, &phase_i);
        c_mul(phase_r, phase_i, mix_r, mix_i, &cRe[15 + i], &cIm[15 + i]);
    }

    double t1cu_r, t1cu_i;
    c_mul(t1sq_r, t1sq_i, x1r, x1i, &t1cu_r, &t1cu_i);
    for (int i = 20; i < 30; i++) {
        cRe[i] = 100.0 - t1cu_r;
        cIm[i] = t2sq_i;
    }

    double prod_r, prod_i;
    c_mul(x1r, x1i, x2r, x2i, &prod_r, &prod_i);
    const double abs_prod = c_abs(prod_r, prod_i);
    const double ang_diff = c_arg(x1r - x2r, x1i - x2i);
    for (int i = 30; i < 40; i++) {
        cRe[i] = -40.0 + abs_prod;
        cIm[i] = ang_diff;
    }

    const double offset_r = x1r + x2r;
    const double offset_i = x1i - x2i;
    for (int i = 0; i < 10; i++) {
        double phase_r, phase_i;
        c_exp2(0.0, phi10 * (double)i * (double)(i + 2), &phase_r, &phase_i);
        c_mul(0.0, 1.0, phase_r, phase_i, &phase_r, &phase_i);
        cRe[40 + i] = phase_r - offset_r;
        cIm[40 + i] = phase_i - offset_i;
    }

    const double scale = 2.0 * log(fabs(x1r + x2i) + 1.0);
    for (int i = 0; i < 10; i++) {
        cRe[50 + i] = scale * (double)(i + 2);
    }

    for (int i = 0; i < 10; i++) {
        int k = i + 1;
        double base_r, base_i;
        c_exp2(0.0, phi10 * (double)i, &base_r, &base_i);
        double pow_r, pow_i;
        c_powc(base_r, base_i, (double)k * x1r, (double)k * x1i, &pow_r, &pow_i);
        const double sign = (k % 2 == 0) ? 1.0 : -1.0;
        cRe[60 + i] = sign * pow_r;
        cIm[60 + i] = sign * pow_i;
    }

    double sum_r = 0.0, sum_i = 0.0;
    for (int i = 0; i < 70; i++) {
        sum_r += cRe[i];
        sum_i += cIm[i];
    }
    cRe[70] = sum_r / 71.0;
    cIm[70] = sum_i / 71.0;

    for (int i = 0; i < 71; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

static void poly_316_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[35] = {0};
    for (int j = 1; j <= 35; j++) {
        int k = j + 3;
        double r = x1r * sin((double)j * M_PI / 8.0) + x2r * cos((double)j * M_PI / 5.0);
        double im = x1i * cos((double)j * M_PI / 7.0) - x2i * sin((double)j * M_PI / 9.0);
        double prod = 1.0;
        for (int q = 1; q <= j; q++) prod *= (double)q;
        double mag = log(fabs(r * im) + 1.0) * (1.0 + sin((double)k * M_PI / 4.0)) * prod / 35.0;
        double ang = carg(t1) * cos((double)k * M_PI / 6.0) + carg(t2) * sin((double)k * M_PI / 10.0);
        cf[j - 1] = mag * poly_hand_z(cos(ang), sin(ang));
        double denom = 0.0;
        for (int i = 0; i < 35; i++) denom += cabs(cf[i]);
        for (int i = 0; i < 35; i++) {
            if (denom == 0.0 || !isfinite(denom)) {
                cf[i] = poly_hand_z(NAN, NAN);
            } else {
                cf[i] /= denom;
            }
        }
    }
    for (int i = 0; i < 35; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_335_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[35] = {0};
    for (int j = 0; j < 35; j++) {
        double angle_part = sin((double)j * M_PI / 7.0) * cos((double)j * M_PI / 5.0) + carg(t1) * carg(t2);
        double magnitude_part = log(cabs(t1) + 1.0) * ((double)j * (double)j) / (1.0 + (double)j) + pow(cabs(t2), 1.0 + sin((double)j));
        double phase_arg = angle_part + x1i * x2r / (double)(j + 1);
        double complex phase_shift = poly_hand_z(cos(phase_arg), sin(phase_arg));
        cf[j] = magnitude_part * phase_shift + conj(t1) * conj(t2) / (double)(j + 1);
    }
    for (int k = 0; k < 35; k++) {
        if (k % 5 == 0) cf[k] *= (1.0 + 0.5 * cos((double)k * M_PI / 3.0));
        else if (k % 3 == 0) cf[k] *= (1.0 + 0.3 * sin((double)k * M_PI / 4.0));
        else cf[k] *= (1.0 + 0.2 * log((double)k + 1.0));
    }
    double prod_abs = 1.0;
    double sum_re = 0.0, sum_im = 0.0;
    for (int i = 0; i < 35; i++) {
        prod_abs *= cabs(cf[i]);
        sum_re += creal(cf[i]);
        sum_im += cimag(cf[i]);
    }
    double scale = pow(prod_abs, 1.0 / 35.0);
    double complex offset = poly_hand_z(sum_re, sum_im);
    for (int i = 0; i < 35; i++) cf[i] = cf[i] * scale + offset;
    for (int i = 0; i < 35; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_383_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[35] = {0};
    for (int j = 1; j <= 35; j++) {
        double prod = poly_hand_np_int64_prod_1_to_n(j);
        double mag_part = log(cabs(t1) + (double)j) * sin((double)j * x2r) + prod / (double)(j + 1);
        double angle_part = carg(t1) * cos((double)j * x2i) + sin((double)j) * carg(t2);
        cf[j - 1] = mag_part * poly_hand_z(cos(angle_part), sin(angle_part))
                  + poly_hand_cpow_uint(conj(t1), j)
                  - poly_hand_cpow_uint(conj(t2), 35 - j);
    }
    for (int i = 0; i < 35; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_413_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[35] = {0};
    double log_abs_t2 = log(cabs(t2) + 1.0);
    for (int j = 1; j <= 35; j++) {
        double angle = carg(t1) * sin((double)j) + carg(t2) * cos((double)j / 3.0);
        double magnitude = pow(cabs(t1), (double)j) * pow(log_abs_t2, (double)(35 - j)) * (double)((j % 5) + 1);
        cf[j - 1] = magnitude * poly_hand_z(cos(angle), sin(angle));
    }
    for (int k = 1; k <= 17; k++) {
        double r = (double)(k * k) + sqrt((double)k);
        double complex ph = poly_hand_z(cos(r), sin(r));
        cf[k - 1] *= ph;
        cf[35 - k] *= conj(ph);
    }
    for (int r = 1; r <= 35; r++) cf[r - 1] += 0.1 * (double)r * poly_hand_z(cos(-(double)r / 35.0), sin(-(double)r / 35.0));
    for (int i = 0; i < 35; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_467_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[35] = {0};
    double ang2 = carg(t2);
    for (int j = 1; j <= 35; j++) {
        double mag_sum = 0.0;
        for (int k = 1; k <= j; k++) mag_sum += log(cabs(t1) + (double)k) * sin((double)k * ang2);
        for (int r = 1; r <= 35 - j; r++) mag_sum += log(cabs(t2) + (double)r) * cos((double)r * ang2);
        double mag = log(mag_sum + 1.0);
        double angle = mag_sum / (double)(j + 1) + mag_sum / (double)(35 - j + 1);
        cf[j - 1] = mag * poly_hand_z(cos(angle), sin(angle));
    }
    for (int j = 1; j <= 35; j++) cf[j - 1] = cf[j - 1] * (1.0 + 0.05 * (double)(j * j)) + conj(cf[j - 1]) * 0.02;
    for (int i = 0; i < 35; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_475_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 40;
    poly_hand_zero(cRe, cIm, 40);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[40] = {0};
    for (int j = 1; j <= 40; j++) {
        double prod = poly_hand_np_int64_prod_1_to_n(j);
        double mag = pow(x1r, (double)j) + log(cabs(t2) + (double)j) + pow(prod, 1.0 / 3.0);
        double angle = carg(t1) * sin((double)j * M_PI / 6.0) + carg(t2) * cos((double)j * M_PI / 4.0);
        cf[j - 1] = mag * cexp(poly_hand_z(0.0, angle)) + conj(t1) * sin((double)j / 2.0) - conj(t2) * cos((double)j / 3.0);
    }
    for (int i = 0; i < 40; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_482_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[35] = {0};
    for (int j = 1; j <= 35; j++) {
        double mag_part1 = log(cabs(t1) + (double)j);
        double mag_part2 = sin((double)j * x2r) * cos((double)j / (x1i + 1.0));
        double prod = poly_hand_np_int64_prod_1_to_n(j);
        double magnitude = mag_part1 * mag_part2 + sqrt(prod);
        double angle = carg(t1) * sin((double)j) + carg(t2) * cos((double)j) + sin((double)j * x1r) - cos((double)j * x2i);
        cf[j - 1] = magnitude * cexp(poly_hand_z(0.0, angle)) + conj(t1) * sin((double)j) - conj(t2) * cos((double)j);
    }
    for (int i = 0; i < 35; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_491_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[35] = {0};
    for (int j = 1; j <= 35; j++) {
        double prod = 1.0;
        for (int q = 1; q <= (j % 5) + 1; q++) prod *= (double)q;
        double magnitude = log(cabs(t1) + cabs(t2) + (double)j) * (pow((double)j, 1.5) + prod);
        double angle = carg(t1) * cos((double)j * M_PI / 35.0) + carg(t2) * sin((double)j * M_PI / 35.0);
        cf[j - 1] = magnitude * poly_hand_z(cos(angle), sin(angle));
    }
    for (int i = 0; i < 35; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_513_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double real_seq[35], imag_seq[35];
    for (int i = 0; i < 35; i++) {
        real_seq[i] = x1r + (x2r - x1r) * (double)i / 34.0;
        imag_seq[i] = x1i + (x2i - x1i) * (double)i / 34.0;
    }
    double complex cf[35] = {0};
    for (int j = 1; j <= 35; j++) {
        double prod = 1.0;
        for (int q = 1; q <= (j % 5) + 1; q++) prod *= (double)q;
        double magnitude = log(cabs(t1 + (double)j) + 1.0) * sin((double)j * M_PI / 7.0) + cos((double)j * M_PI / 5.0) * prod;
        double angle = carg(t2) + sin((double)j * M_PI / 3.0) * cos((double)j * M_PI / 4.0) + tan((double)j * M_PI / 6.0);
        cf[j - 1] = magnitude * poly_hand_z(cos(angle), sin(angle));
    }
    for (int k = 1; k <= 17; k++) {
        int idx = k * 2;
        if (idx <= 35) cf[idx - 1] *= cexp(poly_hand_z(0.0, real_seq[k - 1] / (fabs(imag_seq[k - 1]) + 1.0)));
    }
    for (int r = 1; r <= 3; r++) cf[r - 1] = cf[r - 1] * cf[r - 1] / (1.0 + cabs(cf[r - 1]));
    for (int i = 0; i < 35; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_535_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);
    double rec[35], imc[35];
    for (int i = 0; i < 35; i++) {
        rec[i] = x1r + (x2r - x1r) * (double)i / 34.0;
        imc[i] = x1i + (x2i - x1i) * (double)i / 34.0;
    }
    for (int j = 1; j <= 35; j++) {
        double complex z = poly_hand_z(rec[j - 1], imc[j - 1]);
        double mag = log(cabs(z) + 1.0) * sin((double)j * M_PI / 5.0) + cos((double)j * M_PI / ((double)j + 2.0));
        double ang = carg(z) + sin((double)j / 35.0 * M_PI * 4.0) - cos((double)j / 35.0 * M_PI * 3.0);
        cRe[j - 1] = mag * cos(ang);
        cIm[j - 1] = mag * sin(ang);
    }
}

static void poly_551_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 40;
    poly_hand_zero(cRe, cIm, 40);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[40] = {0};
    double prod_base = x1r * x2i;
    for (int j = 1; j <= 40; j++) {
        double mag_part1 = log(cabs(t1) + 1.0) * sin((double)j * M_PI / 40.0);
        double mag_part2 = log(cabs(t2) + 1.0) * cos((double)j * M_PI / 20.0);
        double mag_variation = mag_part1 + mag_part2 + pow(prod_base, 1.0 / (double)j);
        double angle_variation = carg(t1) * sin((double)j / 2.0) + carg(t2) * cos((double)j / 3.0) + sin((double)j) * cos((double)j);
        double complex complex_component = poly_hand_z(cos(angle_variation), sin(angle_variation));
        cf[j - 1] = mag_variation * complex_component + conj(t1) * sin((double)j) + conj(t2) * cos((double)j);
    }
    for (int i = 0; i < 40; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_562_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);
    for (int j = 1; j <= 35; j++) {
        double rec = x1r + (x2r - x1r) * (double)j / 35.0;
        double imc = x1i + (x2i - x1i) * (double)j / 35.0;
        double prod = 1.0;
        for (int q = 1; q <= (j % 5) + 1; q++) prod *= (double)q;
        double mag = log(fabs(rec) + 1.0) * ((double)(j * j) + sqrt(35.0 - (double)j + 1.0)) * sin((double)j) + prod;
        double angle = sin(rec * M_PI / 7.0) + cos(imc * M_PI / 5.0) + c_arg(x1r, x1i) - c_arg(x2r, x2i);
        cRe[j - 1] = mag * cos(angle);
        cIm[j - 1] = mag * sin(angle);
    }
}

static void poly_566_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    poly_hand_zero(cRe, cIm, 25);
    for (int j = 1; j <= 25; j++) {
        double prod = 1.0;
        for (int q = 1; q <= (j % 5) + 1; q++) prod *= (double)q;
        double magnitude = log(c_abs(x1r, x1i) + (double)j) * (prod + sqrt((double)j));
        double angle = sin((double)j * c_arg(x1r, x1i)) + cos(c_arg(x2r, x2i) / ((double)j + 1.0));
        cRe[j - 1] = magnitude * cos(angle);
        cIm[j - 1] = magnitude * sin(angle);
        cRe[j - 1] += (x1i - x2i) * pow(sin((double)j), 2.0);
    }
}

static void poly_570_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 26;
    poly_hand_zero(cRe, cIm, 26);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    for (int j = 1; j <= 26; j++) {
        double sum_mag = 0.0;
        double sum_ang = 0.0;
        for (int k = 1; k <= j; k++) {
            double complex term = t1 * (double)k + t2 / (double)k;
            sum_mag += log(cabs(term) + 1.0);
            double complex angle_term = t1 * sin((double)k * x2r) + t2 * cos((double)k * x1i);
            sum_ang += carg(angle_term);
        }
        cRe[j - 1] = sum_mag * cos(sum_ang);
        cIm[j - 1] = sum_mag * sin(sum_ang);
    }
}

static void poly_604_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    poly_hand_zero(cRe, cIm, 25);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double rec[25], imc[25];
    for (int i = 0; i < 25; i++) {
        rec[i] = x1r + (x2r - x1r) * (double)i / 24.0;
        imc[i] = x1i + (x2i - x1i) * (double)i / 24.0;
    }
    double complex cf[25] = {0};
    for (int j = 1; j <= 25; j++) {
        double magnitude = log(fabs(rec[j - 1]) + 1.0) * pow(cabs(t1), (double)((j % 5) + 1)) + pow(cabs(t2), (double)(((25 - j) % 7) + 1));
        double angle = sin(rec[j - 1] * M_PI * (double)j) + cos(imc[j - 1] * M_PI / ((double)j + 1.0)) + carg(t1) * log((double)j + 2.0) - carg(t2) * sqrt((double)j);
        cf[j - 1] = magnitude * cexp(poly_hand_z(0.0, angle));
    }
    double prod_rec = 1.0, sum_rec = 0.0, sum_imc = 0.0;
    for (int i = 0; i < 25; i++) {
        prod_rec *= rec[i];
        sum_rec += rec[i];
        sum_imc += imc[i];
    }
    double scalar = prod_rec / (sum_imc + 1.0);
    for (int i = 0; i < 25; i++) cf[i] = cf[i] * scalar + sum_rec * conj(t1) - sum_imc * conj(t2);
    for (int i = 0; i < 25; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_727_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    poly_hand_zero(cRe, cIm, 9);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    for (int k = 1; k <= 9; k++) {
        double mag = log(cabs(t1) + cabs(t2) + (double)k) * (double)(k * k);
        double angle = carg(t1) * sin((double)k) + carg(t2) * cos((double)k);
        cRe[k - 1] = mag * cos(angle);
        cIm[k - 1] = mag * sin(angle);
    }
}

static void poly_751_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    poly_hand_zero(cRe, cIm, 11);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    for (int i = 0; i < 11; i++) {
        double angle = 2.0 * M_PI * (double)i / 10.0;
        cRe[i] = 10.0 * cos(angle);
        cIm[i] = 10.0 * sin(angle);
    }
    cRe[0] = cabs(t1 + t2); cIm[0] = 0.0;
    poly_hand_store_z(csin(t1) * cpow(ccos(t2), 2.0), cRe, cIm, 5);
    cRe[7] = -log(cabs(t1) + 1.0);
    cIm[7] = log(cabs(t2) + 1.0);
    poly_hand_store_z((t1 * t1) / (t2 + poly_hand_z(0.0, 1.0)), cRe, cIm, 9);
    poly_hand_store_z(csqrt(t1 * t2), cRe, cIm, 10);
}

static void poly_18_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double arg = carg(t1 * t2 * poly_hand_z(0.0, 1.0));
    double mod = cabs((t1 + poly_hand_z(0.0, 1.0)) * (t2 + poly_hand_z(0.0, 1.0)));
    for (int i = 1; i <= 71; i++) {
        double complex cyclotomic = 0.0;
        for (int k = 1; k <= i; k++) {
            double theta = 2.0 * M_PI * (double)k / (double)i;
            cyclotomic += t1 - cexp(poly_hand_z(0.0, theta));
        }
        poly_hand_store_z(mod * cyclotomic * arg, cRe, cIm, i - 1);
    }
    cRe[70] = log(cabs(t1 * t2)) + 1.0;
    cIm[70] = 0.0;
}

static void poly_39_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    poly_hand_store_z(t1, cRe, cIm, 0);
    poly_hand_store_z(t2, cRe, cIm, 1);
    double ang_abs = fabs(carg(t1 + t2));
    for (int k = 3; k <= 71; k++) {
        double complex prev1 = poly_hand_z(cRe[k - 2], cIm[k - 2]);
        double complex prev2 = poly_hand_z(cRe[k - 3], cIm[k - 3]);
        double complex value = csin((double)k * t1)
                             + ccos((double)k * t2)
                             + log((double)k + 1.0) * cabs(prev1) * cabs(prev2) * ang_abs;
        poly_hand_store_z(value, cRe, cIm, k - 1);
    }
}

static void poly_59_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    for (int i = 1; i <= 71; i++) {
        double complex value = cpow(t1 / (double)(i + 1), (double)i)
                             + (t2 / (double)(i + 1)) * poly_hand_z(0.0, 2.0);
        poly_hand_store_z(value, cRe, cIm, i - 1);
    }
    for (int idx = 1; idx <= 19; idx += 2) {
        double complex z = poly_hand_z(cRe[idx], cIm[idx]) * (t1 + 2.0 * t2);
        poly_hand_store_z(z, cRe, cIm, idx);
    }
    for (int idx = 2; idx <= 29; idx += 3) {
        double complex z = poly_hand_z(cRe[idx], cIm[idx]) * (t1 - 2.0 * t2);
        poly_hand_store_z(z, cRe, cIm, idx);
    }
    for (int idx = 4; idx < 36; idx++) {
        double complex z = poly_hand_z(cRe[idx], cIm[idx]) + 2.0 * t1;
        poly_hand_store_z(z, cRe, cIm, idx);
    }
    for (int idx = 36; idx < 67; idx++) {
        double complex z = poly_hand_z(cRe[idx], cIm[idx]) - 2.0 * t2;
        poly_hand_store_z(z, cRe, cIm, idx);
    }
    double complex add = poly_hand_z(cRe[4], cIm[4]);
    for (int idx = 67; idx < 71; idx++) {
        double complex z = log(cabs(poly_hand_z(cRe[idx], cIm[idx]))) + add;
        poly_hand_store_z(z, cRe, cIm, idx);
    }
}

static void poly_68_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double abs1 = cabs(t1);
    for (int i = 1; i <= 5; i++) {
        cRe[i - 1] = pow(abs1, (double)i);
        cIm[i - 1] = 0.0;
    }
    for (int i = 6; i <= 70; i++) {
        poly_hand_store_z(((double)i * t1 + 2.0 * (double)i * t2) / (double)(i + 1), cRe, cIm, i);
    }
    double complex base = clog(cabs(t1 + t2) + 1.0);
    for (int m = 0; m < 10; m++) {
        double angle = M_PI / 10.0 * (double)(m + 1);
        double complex z = poly_hand_z(cRe[20 + m], cIm[20 + m]) + base * cexp(poly_hand_z(0.0, angle));
        poly_hand_store_z(z, cRe, cIm, 20 + m);
    }
    for (int m = 0; m < 10; m++) {
        double complex z = poly_hand_z(cRe[50 + m], cIm[50 + m]) + poly_hand_z(0.0, 1.0) * (poly_hand_z(cRe[m], cIm[m]) / (double)(m + 11));
        poly_hand_store_z(z, cRe, cIm, 50 + m);
    }
    for (int m = 0; m < 10; m++) {
        double complex z = poly_hand_z(cRe[60 + m], cIm[60 + m]) - csin(poly_hand_z(cRe[m], cIm[m]));
        poly_hand_store_z(z, cRe, cIm, 60 + m);
    }
    double complex cos_sum = ccos(t1 + t2);
    for (int m = 0; m < 10; m++) {
        double complex z = poly_hand_z(cRe[30 + m], cIm[30 + m]) + poly_hand_cpow_uint(cos_sum, m + 1);
        poly_hand_store_z(z, cRe, cIm, 30 + m);
    }
    cRe[70] = cabs(t1) * cabs(t2) * carg(t1) * carg(t2) * sin(cabs(t1 + t2));
    cIm[70] = 0.0;
}

static void poly_69_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double log_term = log(cabs(t1 * t2) + 1.0);
    for (int i = 1; i <= 71; i++) {
        int j = 71 - i;
        double denom = cabs(t2 + (double)i);
        double scalar = ((x1r + x1i * (double)j) / denom) * sin(carg(t1 + (double)i * t2))
                      + log_term * cos(2.0 * M_PI * (double)i / 71.0);
        cRe[i - 1] = scalar;
        cIm[i - 1] = 0.0;
    }
    double fill = x1r * x1r - x1i * x2i;
    for (int i = 0; i < 71; i++) {
        if (cRe[i] == 0.0 && cIm[i] == 0.0) cRe[i] = fill;
    }
}

static void poly_87_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    poly_hand_zero(cRe, cIm, 71);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double prod_re = creal(t1 * t2);
    double prod_im = cimag(t1 * t2);
    for (int k = 1; k < 36; k++) {
        double complex left = cpow(t1 + poly_hand_z(-x2i, x2r), (double)k)
                            + log(cabs(t1 + (double)k * t2) + 1.0) * prod_re;
        double complex right = (double)k * cpow(t1 + poly_hand_z(x2i, -x2r), (double)k)
                             - log(cabs(t2 - (double)k * t1) + 1.0) * prod_im;
        poly_hand_store_z(left, cRe, cIm, k - 1);
        poly_hand_store_z(right, cRe, cIm, 70 - k);
    }
    cRe[35] = 100.0 * cabs(t1) * cabs(t2);
    cIm[35] = 0.0;
    cRe[36] = 200.0 * carg(t1) * carg(t2);
    cIm[36] = 0.0;
    double complex tail[34];
    for (int i = 0; i < 34; i++) tail[i] = poly_hand_z(cRe[i], cIm[i]) - poly_hand_z(cRe[37 + i], cIm[37 + i]);
    for (int i = 0; i < 34; i++) poly_hand_store_z(tail[i], cRe, cIm, 37 + i);
}

static void poly_691_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    poly_hand_zero(cRe, cIm, 9);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    for (int j = 1; j <= 9; j++) {
        double r_part = x1r * pow((double)j, sin((double)j)) + x2r / (double)(j + 1);
        double im_part = x1i * cos((double)j) + x2i * sin((double)j / 2.0);
        double mag = log(cabs(t1) + (double)j) * fabs(sin((double)j)) + log(cabs(t2) + 1.0);
        double angle = carg(t1) * (double)j + carg(t2) * cos((double)j / 3.0);
        double complex coeff = poly_hand_z(r_part, im_part) * cexp(poly_hand_z(0.0, angle)) * mag;
        poly_hand_store_z(coeff + conj(t1) * sin((double)j) + cos((double)j) * sin((double)j / 2.0), cRe, cIm, j - 1);
    }
}

static void poly_694_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    poly_hand_zero(cRe, cIm, 9);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    for (int j = 1; j <= 9; j++) {
        double mag_part1 = log(cabs(t1) + 1.0) * sin((double)j * M_PI / 5.0);
        double mag_part2 = cos((double)j * x1r) * pow(cabs(t2), 0.5);
        double mag_part3 = (j % 2 == 0) ? (x1r + x2i) : (x1r + x2r);
        double magnitude = mag_part1 + mag_part2 + mag_part3;
        double angle_part1 = pow(carg(t1), (double)j);
        double angle_part2 = x2r * (double)j;
        double angle_part3 = (j % 3 == 0) ? sin((double)j * carg(t1)) : cos((double)j * carg(t2));
        double angle = angle_part1 + angle_part2 + angle_part3;
        double complex base = magnitude * poly_hand_z(cos(angle), sin(angle));
        double sum_part = 0.0;
        for (int k = 1; k <= j; k++) sum_part += pow(x1r, (double)k);
        for (int k = 1; k <= (j % 2) + 1; k++) sum_part += pow(x2i, (double)k);
        poly_hand_store_z(base + sum_part * conj(t1 + t2), cRe, cIm, j - 1);
    }
    for (int k = 1; k <= 9; k++) {
        double complex z = poly_hand_z(cRe[k - 1], cIm[k - 1]) * (sin((double)k * x1r) + cos((double)k * x2i)) * cabs(t1 - t2);
        poly_hand_store_z(z, cRe, cIm, k - 1);
    }
}

static void poly_698_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    poly_hand_zero(cRe, cIm, 9);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    for (int j = 1; j <= 9; j++) {
        double mag_part = log(pow(cabs(t1), (double)j) + pow(cabs(t2), (double)(9 - j)) + 1.0);
        double angle_part = sin((double)j * carg(t1)) + cos((double)j * carg(t2));
        cRe[j - 1] = mag_part * cos(angle_part);
        cIm[j - 1] = mag_part * sin(angle_part);
    }
}

static void poly_733_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 9;
    poly_hand_zero(cRe, cIm, 9);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    for (int k = 1; k <= 9; k++) {
        double complex tmp = 0.0;
        for (int j = 1; j <= k; j++) {
            tmp += (pow(x1r, (double)j) / (double)(j + 1)) * cexp(poly_hand_z(0.0, sin((double)j * x2r)));
        }
        for (int r = 1; r <= (k % 3) + 1; r++) {
            tmp += (pow(x2i, (double)r) / (double)(r + 2)) * cexp(poly_hand_z(0.0, cos((double)r * x1i)));
        }
        poly_hand_store_z(tmp, cRe, cIm, k - 1);
    }
}

static void poly_745_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 10;
    poly_hand_zero(cRe, cIm, 10);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[10] = {0};
    cf[9] = x1r * x2i;
    cf[4] = x1i * x2r;
    cf[0] = cexp(poly_hand_z(0.0, 1.0) * (t1 + t2));
    double m = cabs(t1 + t2);
    cf[2] = 1.0 / (m + 1.0);
    double polar_coordinates = hypot(x1r, x1i) * hypot(x2r, x2i);
    cf[6] = cexp(poly_hand_z(0.0, polar_coordinates));
    cf[8] = 285.0 * x1r;
    cf[1] = cpow(t2, 10.0) - cpow(cf[9], 10.0);
    cf[3] = carg(t1) * cf[1];
    cf[5] = cf[2] + cf[6] * cf[8];
    cf[7] = conj(cf[3]) * cf[5];
    for (int i = 0; i < 10; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_755_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    poly_hand_zero(cRe, cIm, 11);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    poly_hand_store_z(cpow(t1 * conj(t2), 3.0) + t1 - t2, cRe, cIm, 0);
    poly_hand_store_z(t2 + poly_hand_z(-x1i, x1r), cRe, cIm, 10);
    double snd0 = sin(2.0 * M_PI * x1r);
    double snd1 = sin(2.0 * M_PI * x2i);
    double csi0 = cos(2.0 * M_PI * x1i);
    double csi1 = cos(2.0 * M_PI * x2r);
    for (int i = 0; i < 9; i++) {
        double w = (double)i / 8.0;
        double snd = snd0 + (snd1 - snd0) * w;
        double csi = csi0 + (csi1 - csi0) * w;
        double complex stat = t1 * (csi * csi) + t2 * (snd * snd)
                            + poly_hand_z(0.0, 1.0) * (t1 * (snd * snd) + t2 * (csi * csi));
        poly_hand_store_z(stat, cRe, cIm, i + 1);
    }
}

static void poly_774_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    poly_hand_zero(cRe, cIm, 11);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex cf[11] = {0};
    double complex complex_sum = t1 + t2;
    double complex complex_product = t1 * t2;
    double complex complex_diff = t1 - t2;
    cf[0] = cexp(poly_hand_z(0.0, carg(complex_sum))) * cabs(complex_sum);
    cf[1] = creal(complex_product) + poly_hand_z(0.0, cimag(complex_diff));
    cf[2] = cabs(complex_product);
    cf[3] = cf[0] + cf[1] + cf[2];
    cf[4] = cf[0] * cf[2] - cf[1] * cf[3];
    cf[5] = 10.0 + csin(cf[4]) + csin(cf[3]) + ccos(cf[2]) + ccos(cf[1]) + csin(cf[0]);
    cf[6] = poly_hand_z(0.0, 2.0) * cf[0] + cf[1] / cf[5];
    cf[7] = cf[1] * cf[2] * cf[3] + cf[0];
    cf[8] = 2.0 * cf[7] - cf[4] * cf[6];
    cf[9] = 1.0;
    for (int i = 0; i < 9; i++) cf[9] *= cf[i];
    cf[10] = (cf[0] + cf[2] - cf[4] + cf[6] - cf[8]) / (cf[1] - cf[3] + cf[5] - cf[7] + cf[9]);
    for (int i = 0; i < 11; i++) poly_hand_store_z(cf[i], cRe, cIm, i);
}

static void poly_789_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    poly_hand_zero(cRe, cIm, 25);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    for (int i = 0; i < 5; i++) cRe[i] = x1r * (double)(i + 1) - x2i * (double)(i + 1);
    cRe[5] = cabs(t1) * cabs(t2);
    for (int i = 0; i < 5; i++) cRe[6 + i] = carg(t1 + t2) * (double)(i + 6);
    poly_hand_store_z(conj(t1) + conj(t2), cRe, cIm, 12);
    double real_t1_plus_it2 = creal(t1 + poly_hand_z(-x2i, x2r));
    for (int i = 0; i < 5; i++) cRe[13 + i] = real_t1_plus_it2 * (double)(i + 1);
    cRe[18] = carg(t1) * carg(t2);
    double imag_t1_minus_it2 = cimag(t1 - poly_hand_z(-x2i, x2r));
    for (int i = 0; i < 5; i++) cRe[19 + i] = imag_t1_minus_it2 * (double)(i + 1);
    poly_hand_store_z(conj(t1 * t2) + cabs(poly_hand_z(cRe[12], cIm[12])) + carg(poly_hand_z(cRe[18], cIm[18])), cRe, cIm, 24);
}

static void poly_800_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    poly_hand_zero(cRe, cIm, 25);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    poly_hand_store_z(t1 * t2, cRe, cIm, 0);
    double angle_it2 = carg(poly_hand_z(-x2i, x2r));
    for (int i = 0; i < 9; i++) {
        cRe[1 + i] = cabs(t1) * pow(angle_it2, (double)(i + 1));
        cIm[1 + i] = 0.0;
    }
    cRe[14] = x1r * x2i + x2r * x1i;
    cIm[14] = 0.0;
    for (int i = 0; i < 5; i++) {
        cRe[15 + i] = pow(cabs(t1 - t2 + poly_hand_z(0.0, 1.0)), (double)(i + 1));
        cIm[15 + i] = 0.0;
    }
    cRe[20] = x1i * x2r + x1r * x2i;
    cIm[20] = 0.0;
    double base = cabs(poly_hand_z(cRe[14], cIm[14]) + t1 * t2);
    for (int i = 0; i < 3; i++) {
        cRe[21 + i] = pow(base, (double)(i + 1));
        cIm[21 + i] = 0.0;
    }
    cRe[24] = fabs(cRe[23]) + log(cabs(t1 + t2 + poly_hand_z(0.0, 0.5)));
    cIm[24] = 0.0;
}

static void poly_806_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    poly_hand_zero(cRe, cIm, 25);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double complex c0 = cexp(poly_hand_z(0.0, 2.0 * M_PI / 3.0));
    double complex c1 = cexp(poly_hand_z(0.0, -2.0 * M_PI / 3.0));
    for (int k = 0; k < 25; k++) {
        double complex z = csin((double)k * t1) * c0 + ccos((double)k * t2) * c1;
        double denom = (cimag(z) == 0.0) ? creal(z) : cimag(z);
        poly_hand_store_z(z / denom, cRe, cIm, k);
    }
    poly_hand_store_z(t1 * t1 + t2 * t2, cRe, cIm, 7);
    cRe[24] = cabs(t1 + t2);
    cIm[24] = 0.0;
}

static void poly_811_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    poly_hand_zero(cRe, cIm, 25);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    poly_hand_store_z(t1 * t2, cRe, cIm, 0);
    poly_hand_store_z((t1 + t2) * conj(t1 - t2), cRe, cIm, 1);
    double arg = carg(t1 + poly_hand_z(-x2i, x2r));
    for (int k = 2; k < 25; k++) {
        double complex z = cabs(t1) * cabs(t2) * sin(pow(arg, (double)k))
                         + log(cabs(cpow(t1, (double)k) / (1.0 + t2)));
        poly_hand_store_z(z, cRe, cIm, k);
    }
}

static void poly_813_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    poly_hand_zero(cRe, cIm, 25);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    poly_hand_store_z(t1 * t2, cRe, cIm, 0);
    for (int k = 1; k < 25; k++) {
        double complex prev = poly_hand_z(cRe[k - 1], cIm[k - 1]);
        double complex v = (csin((double)k * prev) + ccos((double)k * prev)) * creal(t1 + t2);
        if (cabs(v) != 0.0) poly_hand_store_z(log(cabs(v)) + conj(t1 * t2), cRe, cIm, k);
        else poly_hand_store_z(0.0, cRe, cIm, k);
    }
    double complex sum = 0.0;
    for (int i = 0; i < 24; i++) sum += poly_hand_z(cRe[i], cIm[i]);
    poly_hand_store_z(sum + cabs(t1 - t2), cRe, cIm, 24);
}

static void poly_814_hand(double x1r, double x1i, double x2r, double x2i,
                          const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    poly_hand_zero(cRe, cIm, 25);
    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    poly_hand_store_z(carg(t1) * t2, cRe, cIm, 0);
    poly_hand_store_z(t1 * t2, cRe, cIm, 1);
    for (int i = 2; i < 25; i++) {
        double complex v = poly_hand_z(cRe[i - 1], cIm[i - 1]) + poly_hand_z(cRe[i - 2], cIm[i - 2]) + conj(t1 * t2);
        if (cabs(v) != 0.0) poly_hand_store_z(log(cabs(v)), cRe, cIm, i);
        else poly_hand_store_z(0.0, cRe, cIm, i);
    }
    double complex sum = 0.0;
    for (int i = 0; i < 24; i++) sum += poly_hand_z(cRe[i], cIm[i]);
    poly_hand_store_z(cabs(t1 - t2) + sum, cRe, cIm, 24);
}

static void old_379_hand(double x1r, double x1i, double x2r, double x2i,
                         const double *cfpv, int n_cfpv,
                         double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 35;
    poly_hand_zero(cRe, cIm, 35);

    double complex t1 = poly_hand_z(x1r, x1i);
    double complex t2 = poly_hand_z(x2r, x2i);
    double abs_t1 = cabs(t1);
    double abs_t2 = cabs(t2);
    double arg_t1 = carg(t1);
    double arg_t2 = carg(t2);

    for (int j = 1; j <= 35; j++) {
        double jd = (double)j;
        double magnitude = log(abs_t1 + abs_t2 + jd)
                         * (pow(abs_t1, sin(jd)) + pow(abs_t2, cos(jd)));
        double angle = arg_t1 * jd - arg_t2 * (35.0 - jd) + sin(jd) * cos(jd);
        cRe[j - 1] = magnitude * cos(angle);
        cIm[j - 1] = magnitude * sin(angle);
    }

    double complex t2_pow = 1.0;
    for (int k = 1; k <= 35; k++) {
        t2_pow *= t2;
        double complex z = poly_hand_z(cRe[k - 1], cIm[k - 1])
                         + conj(t1) * t2_pow / (double)(k + 1);
        poly_hand_store_z(z, cRe, cIm, k - 1);
    }

    double special = 50.0 * (x1r - x2i);
    const int special_indices[] = {4, 9, 14, 19, 24, 29};
    for (int i = 0; i < 6; i++) cIm[special_indices[i]] += special;
}
