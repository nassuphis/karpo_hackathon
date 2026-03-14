/*
 * Sweep solver: evaluate a polynomial family across n_t parameter steps.
 * Ehrlich-Aberth with warm-start + greedy root matching for trajectory continuity.
 *
 * Reads JSON sweep spec from stdin.
 * Writes packed f32 binary (root positions) to a file path given as argv[1].
 * Writes metadata JSON to stdout.
 *
 * Build: aarch64-linux-musl-gcc -O3 -static -o sweep sweep_cli.c -lm
 * Local: cc -O3 -o sweep sweep_cli.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_DEGREE 255
#define MAX_COEFFS 256
#define MAX_ANIM 64
#define MAX_ITER 64
#define TOL2 1e-16
#define BUF_SIZE (1024 * 256)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ---- qsort comparator for doubles ---- */
static int cmpDouble(const void *a, const void *b) {
    double da = *(const double *)a, db = *(const double *)b;
    return (da > db) - (da < db);
}

/* ---- Ehrlich-Aberth solver ---- */

static int solveEA(double *cr, double *ci, int n,
                   double *rRe, double *rIm, int degree)
{
    for (int iter = 0; iter < MAX_ITER; iter++) {
        double maxCorr2 = 0;
        for (int i = 0; i < degree; i++) {
            double zR = rRe[i], zI = rIm[i];

            /* Horner: p(z) and p'(z) */
            double pR = cr[0], pI = ci[0];
            double dpR = 0, dpI = 0;
            for (int k = 1; k < n; k++) {
                double ndR = dpR * zR - dpI * zI + pR;
                double ndI = dpR * zI + dpI * zR + pI;
                dpR = ndR; dpI = ndI;
                double npR = pR * zR - pI * zI + cr[k];
                double npI = pR * zI + pI * zR + ci[k];
                pR = npR; pI = npI;
            }

            /* Newton: w = p/p' */
            double dpM = dpR * dpR + dpI * dpI;
            if (dpM < 1e-60) continue;
            double wR = (pR * dpR + pI * dpI) / dpM;
            double wI = (pI * dpR - pR * dpI) / dpM;

            /* Aberth sum */
            double sR = 0, sI = 0;
            for (int j = 0; j < degree; j++) {
                if (j == i) continue;
                double dR = zR - rRe[j], dI = zI - rIm[j];
                double dM = dR * dR + dI * dI;
                if (dM < 1e-60) continue;
                sR += dR / dM;
                sI += -dI / dM;
            }

            /* Correction */
            double wsR = wR * sR - wI * sI;
            double wsI = wR * sI + wI * sR;
            double dnR = 1 - wsR, dnI = -wsI;
            double dnM = dnR * dnR + dnI * dnI;
            if (dnM < 1e-60) continue;

            double crrR = (wR * dnR + wI * dnI) / dnM;
            double crrI = (wI * dnR - wR * dnI) / dnM;
            rRe[i] -= crrR;
            rIm[i] -= crrI;

            double h2 = crrR * crrR + crrI * crrI;
            if (h2 > maxCorr2) maxCorr2 = h2;
        }
        if (maxCorr2 < TOL2) return iter + 1;
    }
    return MAX_ITER;
}

/* ---- Greedy root matching ---- */

static void matchRoots(double *newRe, double *newIm,
                       double *oldRe, double *oldIm, int n)
{
    double tmpRe[MAX_DEGREE], tmpIm[MAX_DEGREE];
    int used[MAX_DEGREE];
    memset(used, 0, n * sizeof(int));

    for (int i = 0; i < n; i++) {
        int bestJ = 0;
        double bestD = 1e30;
        for (int j = 0; j < n; j++) {
            if (used[j]) continue;
            double dr = newRe[j] - oldRe[i];
            double di = newIm[j] - oldIm[i];
            double d = dr * dr + di * di;
            if (d < bestD) { bestD = d; bestJ = j; }
        }
        tmpRe[i] = newRe[bestJ];
        tmpIm[i] = newIm[bestJ];
        used[bestJ] = 1;
    }
    memcpy(newRe, tmpRe, n * sizeof(double));
    memcpy(newIm, tmpIm, n * sizeof(double));
}

/* ---- Minimal JSON parsing ---- */

static const char *skip(const char *p) {
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    return p;
}

static double parseNum(const char **pp) {
    const char *p = skip(*pp);
    char *end;
    double v = strtod(p, &end);
    *pp = end;
    return v;
}

static const char *findKey(const char *json, const char *key) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *p = strstr(json, pattern);
    if (!p) return NULL;
    p += strlen(pattern);
    p = skip(p);
    if (*p == ':') p++;
    return skip(p);
}

/* Find key within a bounded region (for nested objects) */
static const char *findKeyIn(const char *start, const char *end, const char *key) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *p = start;
    while (p < end) {
        p = strstr(p, pattern);
        if (!p || p >= end) return NULL;
        p += strlen(pattern);
        p = skip(p);
        if (*p == ':') p++;
        return skip(p);
    }
    return NULL;
}

static int parseCoefficients(const char *p, double *re, double *im) {
    p = skip(p);
    if (*p != '[') return 0;
    p++;
    int count = 0;
    while (count < MAX_COEFFS) {
        p = skip(p);
        if (*p == ']') break;
        if (*p == ',') { p++; p = skip(p); }
        if (*p != '[') break;
        p++;
        re[count] = parseNum(&p);
        p = skip(p); if (*p == ',') p++;
        im[count] = parseNum(&p);
        p = skip(p); if (*p == ']') p++;
        count++;
    }
    return count;
}

/* Find matching brace/bracket end */
static const char *findClosing(const char *p, char open, char close) {
    int depth = 1;
    p++;
    while (*p && depth > 0) {
        if (*p == open) depth++;
        else if (*p == close) depth--;
        if (*p == '"') { p++; while (*p && *p != '"') { if (*p == '\\') p++; p++; } }
        p++;
    }
    return p;
}

/* Animation entry */
typedef struct {
    int coeff_index;
    double radius;
    double speed;
    double angle;
    int ccw;
    /* center is the coefficient's base position */
    double centerRe, centerIm;
} Anim;

static int parseAnimations(const char *p, Anim *anims) {
    p = skip(p);
    if (*p != '[') return 0;
    p++;
    int count = 0;
    while (count < MAX_ANIM) {
        p = skip(p);
        if (*p == ']') break;
        if (*p == ',') { p++; p = skip(p); }
        if (*p != '{') break;
        const char *objStart = p;
        const char *objEnd = findClosing(p, '{', '}');

        Anim *a = &anims[count];
        a->coeff_index = 0; a->radius = 0.5; a->speed = 1.0;
        a->angle = 0.0; a->ccw = 0;

        const char *v;
        v = findKeyIn(objStart, objEnd, "coeff_index");
        if (v) a->coeff_index = (int)parseNum(&v);
        v = findKeyIn(objStart, objEnd, "radius");
        if (v) a->radius = parseNum(&v);
        v = findKeyIn(objStart, objEnd, "speed");
        if (v) a->speed = parseNum(&v);
        v = findKeyIn(objStart, objEnd, "angle");
        if (v) a->angle = parseNum(&v);
        v = findKeyIn(objStart, objEnd, "ccw");
        if (v) a->ccw = (*v == 't' || *v == '1');

        p = objEnd;
        count++;
    }
    return count;
}

static int parseBool(const char *p) {
    p = skip(p);
    return (*p == 't' || *p == '1');
}

/* ---- Parse a quoted string value ---- */

static int parseString(const char *p, char *out, int maxLen) {
    p = skip(p);
    if (*p != '"') return 0;
    p++;
    int i = 0;
    while (*p && *p != '"' && i < maxLen - 1) {
        out[i++] = *p++;
    }
    out[i] = '\0';
    return i;
}

/* ---- Coefficient functions for grid mode ---- */

/*
 * giga_5: degree-25 polynomial with 2 unit-circle parameters t1, t2.
 * R code (1-based):
 *   cf[c(1,5,13,20,21,25)] = c(1,4,4,-9,-1.9,0.2)
 *   cf[7]  = 100i * t2^3 + 100i * t2^2 - 100i * t2 - 100i
 *   cf[9]  = 100i * t1^3 + 100i * t1^2 + 100i * t2 - 100i
 *   cf[15] = 100i * t2^3 - 100i * t2^2 + 100i * t2 - 100i
 *
 * C uses 0-based indices, so R index k → C index k-1.
 * t1 = exp(2πi·x1), t2 = exp(2πi·x2)
 */
static void giga_5(double x1, double x2,
                   double *cRe, double *cIm, int *nCoeffs)
{
    *nCoeffs = 26;
    memset(cRe, 0, 26 * sizeof(double));
    memset(cIm, 0, 26 * sizeof(double));

    /* Fixed real coefficients (R 1-based → C 0-based) */
    cRe[0]  = 1.0;   /* R index 1 */
    cRe[4]  = 4.0;   /* R index 5 */
    cRe[12] = 4.0;   /* R index 13 */
    cRe[19] = -9.0;  /* R index 20 */
    cRe[20] = -1.9;  /* R index 21 */
    cRe[24] = 0.2;   /* R index 25 */

    /* t1 = exp(2πi·x1), t2 = exp(2πi·x2) */
    double a1 = 2.0 * M_PI * x1, a2 = 2.0 * M_PI * x2;
    double t1R = cos(a1), t1I = sin(a1);
    double t2R = cos(a2), t2I = sin(a2);

    /* Complex powers of t2: t2^2, t2^3 */
    double t2_2R = t2R * t2R - t2I * t2I;
    double t2_2I = 2.0 * t2R * t2I;
    double t2_3R = t2_2R * t2R - t2_2I * t2I;
    double t2_3I = t2_2R * t2I + t2_2I * t2R;

    /* Complex powers of t1: t1^2, t1^3 */
    double t1_2R = t1R * t1R - t1I * t1I;
    double t1_2I = 2.0 * t1R * t1I;
    double t1_3R = t1_2R * t1R - t1_2I * t1I;
    double t1_3I = t1_2R * t1I + t1_2I * t1R;

    /* cf[7] (C index 6) = 100i * (t2^3 + t2^2 - t2 - 1)
     * Let S = t2^3 + t2^2 - t2 - 1 (complex)
     * 100i * S = (-100*S_im, 100*S_re) */
    double s6R = t2_3R + t2_2R - t2R - 1.0;
    double s6I = t2_3I + t2_2I - t2I;
    cRe[6] = -100.0 * s6I;
    cIm[6] =  100.0 * s6R;

    /* cf[9] (C index 8) = 100i * (t1^3 + t1^2 + t2 - 1) */
    double s8R = t1_3R + t1_2R + t2R - 1.0;
    double s8I = t1_3I + t1_2I + t2I;
    cRe[8] = -100.0 * s8I;
    cIm[8] =  100.0 * s8R;

    /* cf[15] (C index 14) = 100i * (t2^3 - t2^2 + t2 - 1) */
    double s14R = t2_3R - t2_2R + t2R - 1.0;
    double s14I = t2_3I - t2_2I + t2I;
    cRe[14] = -100.0 * s14I;
    cIm[14] =  100.0 * s14R;
}

/*
 * rev_giga_5: same as giga_5 but coefficients reversed.
 * Original giga_5 has coeff[0..25] (highest power first).
 * Reversed: coeff[k] → coeff[25-k], so lowest power becomes highest.
 */
static void rev_giga_5(double x1, double x2,
                       double *cRe, double *cIm, int *nCoeffs)
{
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS];
    int n;
    giga_5(x1, x2, tmpRe, tmpIm, &n);
    *nCoeffs = n;
    for (int k = 0; k < n; k++) {
        cRe[k] = tmpRe[n - 1 - k];
        cIm[k] = tmpIm[n - 1 - k];
    }
}

/*
 * giga_42: degree-49 polynomial with 50 coefficients.
 * t1 = exp(2πi·x1), t2 = exp(2πi·x2) (complex, on unit circle).
 * R definition (1-based):
 *   cf[c(1,8,16,32,40)] = c(1,-3,3,-1,2)
 *   cf[12] = 100i * exp(t1^2 + t2^2)
 *   cf[20] = 50 * (t1^3 + t2^3)
 *   cf[25] = exp(1i*(t1-t2)) + 10*t1^2
 *   cf[45] = 200*sin(t1+t2) + 1i*cos(t1-t2)
 */
static void giga_42(double x1, double x2,
                    double *cRe, double *cIm, int *nCoeffs)
{
    *nCoeffs = 50;
    memset(cRe, 0, 50 * sizeof(double));
    memset(cIm, 0, 50 * sizeof(double));

    /* Fixed real coefficients (R 1-based → C 0-based) */
    cRe[0]  =  1.0;  /* R index 1  */
    cRe[7]  = -3.0;  /* R index 8  */
    cRe[15] =  3.0;  /* R index 16 */
    cRe[31] = -1.0;  /* R index 32 */
    cRe[39] =  2.0;  /* R index 40 */

    /* t1 = exp(2πi·x1), t2 = exp(2πi·x2) */
    double a1 = 2.0 * M_PI * x1, a2 = 2.0 * M_PI * x2;
    double t1R = cos(a1), t1I = sin(a1);
    double t2R = cos(a2), t2I = sin(a2);

    /* t1^2, t2^2 */
    double t1_2R = t1R * t1R - t1I * t1I;
    double t1_2I = 2.0 * t1R * t1I;
    double t2_2R = t2R * t2R - t2I * t2I;
    double t2_2I = 2.0 * t2R * t2I;

    /* t1^3, t2^3 */
    double t1_3R = t1_2R * t1R - t1_2I * t1I;
    double t1_3I = t1_2R * t1I + t1_2I * t1R;
    double t2_3R = t2_2R * t2R - t2_2I * t2I;
    double t2_3I = t2_2R * t2I + t2_2I * t2R;

    /* cf[12] = 100i * exp(t1^2 + t2^2)
     * s = t1^2 + t2^2 (complex), exp(s) = exp(sR)*(cos(sI)+i*sin(sI))
     * 100i * exp(s) = (-100*eR*sin(sI), 100*eR*cos(sI)) */
    double sR = t1_2R + t2_2R;
    double sI = t1_2I + t2_2I;
    double eS = exp(sR);
    cRe[11] = -100.0 * eS * sin(sI);
    cIm[11] =  100.0 * eS * cos(sI);

    /* cf[20] = 50 * (t1^3 + t2^3) — complex */
    cRe[19] = 50.0 * (t1_3R + t2_3R);
    cIm[19] = 50.0 * (t1_3I + t2_3I);

    /* cf[25] = exp(1i*(t1-t2)) + 10*t1^2
     * d = t1-t2 (complex), 1i*d = (-dI, dR)
     * exp(-dI + i*dR) = exp(-dI)*(cos(dR) + i*sin(dR)) */
    double dR = t1R - t2R;
    double dI = t1I - t2I;
    double ed = exp(-dI);
    cRe[24] = ed * cos(dR) + 10.0 * t1_2R;
    cIm[24] = ed * sin(dR) + 10.0 * t1_2I;

    /* cf[45] = 200*sin(t1+t2) + 1i*cos(t1-t2)
     * Complex sin(z) for z=a+bi: sin(a)*cosh(b) + i*cos(a)*sinh(b)
     * Complex cos(z) for z=a+bi: cos(a)*cosh(b) - i*sin(a)*sinh(b) */
    double sumR = t1R + t2R, sumI = t1I + t2I;
    double sinSR = sin(sumR) * cosh(sumI);
    double sinSI = cos(sumR) * sinh(sumI);
    double difR = t1R - t2R, difI = t1I - t2I;
    double cosDR = cos(difR) * cosh(difI);
    double cosDI = -sin(difR) * sinh(difI);
    /* 200*sin(sum) + i*cos(dif) */
    cRe[44] = 200.0 * sinSR - cosDI;
    cIm[44] = 200.0 * sinSI + cosDR;
}

/* rev_giga_42: reversed coefficients of giga_42. */
static void rev_giga_42(double x1, double x2,
                        double *cRe, double *cIm, int *nCoeffs)
{
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS];
    int n;
    giga_42(x1, x2, tmpRe, tmpIm, &n);
    *nCoeffs = n;
    for (int k = 0; k < n; k++) {
        cRe[k] = tmpRe[n - 1 - k];
        cIm[k] = tmpIm[n - 1 - k];
    }
}

/*
 * giga_43: degree-39 polynomial with 40 coefficients.
 * t1 = exp(2*pi*i*x1), t2 = exp(2*pi*i*x2).
 * R definition (1-based):
 *   cf[c(1,5,15,30)] = c(1,-5,10,-20)
 *   cf[20] = 100i * (t1^3 - t2^3)
 *   cf[10] = 50 * (t1^2*t2 + 1i*t2^2)
 *   cf[25] = exp(1i*t1) + exp(-1i*t2)
 *   cf[35] = 200 * t1 * t2 * sin(t1+t2)
 */
static void giga_43(double x1, double x2,
                    double *cRe, double *cIm, int *nCoeffs)
{
    *nCoeffs = 40;
    memset(cRe, 0, 40 * sizeof(double));
    memset(cIm, 0, 40 * sizeof(double));

    /* Fixed real coefficients (R 1-based -> C 0-based) */
    cRe[0]  =   1.0;  /* R index 1  */
    cRe[4]  =  -5.0;  /* R index 5  */
    cRe[14] =  10.0;  /* R index 15 */
    cRe[29] = -20.0;  /* R index 30 */

    /* t1 = exp(2*pi*i*x1), t2 = exp(2*pi*i*x2) */
    double a1 = 2.0 * M_PI * x1, a2 = 2.0 * M_PI * x2;
    double t1R = cos(a1), t1I = sin(a1);
    double t2R = cos(a2), t2I = sin(a2);

    /* t1^2, t2^2 */
    double t1_2R = t1R * t1R - t1I * t1I;
    double t1_2I = 2.0 * t1R * t1I;
    double t2_2R = t2R * t2R - t2I * t2I;
    double t2_2I = 2.0 * t2R * t2I;

    /* t1^3, t2^3 */
    double t1_3R = t1_2R * t1R - t1_2I * t1I;
    double t1_3I = t1_2R * t1I + t1_2I * t1R;
    double t2_3R = t2_2R * t2R - t2_2I * t2I;
    double t2_3I = t2_2R * t2I + t2_2I * t2R;

    /* cf[20] = 100i * (t1^3 - t2^3)
     * d = t1^3 - t2^3, then 100i*d = (-100*dI, 100*dR) */
    double d20R = t1_3R - t2_3R;
    double d20I = t1_3I - t2_3I;
    cRe[19] = -100.0 * d20I;
    cIm[19] =  100.0 * d20R;

    /* cf[10] = 50 * (t1^2*t2 + 1i*t2^2)
     * t1^2*t2: (t1_2R*t2R - t1_2I*t2I, t1_2R*t2I + t1_2I*t2R)
     * 1i*t2^2: (-t2_2I, t2_2R)
     * sum, then multiply by 50 */
    double prodR = t1_2R * t2R - t1_2I * t2I;
    double prodI = t1_2R * t2I + t1_2I * t2R;
    cRe[9] = 50.0 * (prodR - t2_2I);
    cIm[9] = 50.0 * (prodI + t2_2R);

    /* cf[25] = exp(1i*t1) + exp(-1i*t2)
     * 1i*t1 = (-t1I, t1R), so exp(1i*t1) = exp(-t1I)*(cos(t1R)+i*sin(t1R))
     * -1i*t2 = (t2I, -t2R), so exp(-1i*t2) = exp(t2I)*(cos(-t2R)+i*sin(-t2R))
     *        = exp(t2I)*(cos(t2R) - i*sin(t2R)) */
    double e1 = exp(-t1I);
    double e2 = exp(t2I);
    cRe[24] = e1 * cos(t1R) + e2 * cos(t2R);
    cIm[24] = e1 * sin(t1R) - e2 * sin(t2R);

    /* cf[35] = 200 * t1 * t2 * sin(t1+t2)
     * t1*t2: (t1R*t2R - t1I*t2I, t1R*t2I + t1I*t2R)
     * sin(t1+t2) complex: sin(sR)*cosh(sI) + i*cos(sR)*sinh(sI)
     *   where s = t1+t2
     * Then multiply the two complex numbers and scale by 200 */
    double pR = t1R * t2R - t1I * t2I;
    double pI = t1R * t2I + t1I * t2R;
    double sR = t1R + t2R, sI = t1I + t2I;
    double sinSR = sin(sR) * cosh(sI);
    double sinSI = cos(sR) * sinh(sI);
    /* (pR + i*pI) * (sinSR + i*sinSI) */
    cRe[34] = 200.0 * (pR * sinSR - pI * sinSI);
    cIm[34] = 200.0 * (pR * sinSI + pI * sinSR);
}

/* rev_giga_43: reversed coefficients of giga_43. */
static void rev_giga_43(double x1, double x2,
                        double *cRe, double *cIm, int *nCoeffs)
{
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS];
    int n;
    giga_43(x1, x2, tmpRe, tmpIm, &n);
    *nCoeffs = n;
    for (int k = 0; k < n; k++) {
        cRe[k] = tmpRe[n - 1 - k];
        cIm[k] = tmpIm[n - 1 - k];
    }
}

/*
 * giga_87: degree-50 polynomial with 51 coefficients.
 * t1 = exp(2*pi*i*x1), t2 = exp(2*pi*i*x2).
 * R definition (1-based, sequential overwrites):
 *   cf[1] = t1 + t2
 *   cf[2] = 1 + t1*t2 + log(abs(t1+t2)+1)
 *   cf[3] = t1 + t2 + log(abs(1-t1*t2)+1)
 *   for i in 4:51: cf[i] = i*t1 + (51-i)*t2 + log(abs(t1-t2*i)+1)
 *   cf[11] = cf[1] + cf[10] - sin(t1)
 *   cf[21] = cf[31] + cf[41] - cos(t2)     (loop values of 31,41)
 *   cf[31] = cf[21] + cf[41] + sin(t1)     (new 21, loop 41)
 *   cf[41] = cf[31] + cf[21] - cos(t2)     (new 31, new 21)
 *   cf[51] = cf[41] + cf[21] + sin(t2)     (new 41, new 21)
 */
static void giga_87(double x1, double x2,
                    double *cRe, double *cIm, int *nCoeffs)
{
    *nCoeffs = 51;
    memset(cRe, 0, 51 * sizeof(double));
    memset(cIm, 0, 51 * sizeof(double));

    /* t1 = exp(2*pi*i*x1), t2 = exp(2*pi*i*x2) */
    double a1 = 2.0 * M_PI * x1, a2 = 2.0 * M_PI * x2;
    double t1R = cos(a1), t1I = sin(a1);
    double t2R = cos(a2), t2I = sin(a2);

    /* t1*t2 */
    double t1t2R = t1R * t2R - t1I * t2I;
    double t1t2I = t1R * t2I + t1I * t2R;

    /* Complex abs: |z| = sqrt(re^2 + im^2) */
    #define CABS(re, im) sqrt((re)*(re) + (im)*(im))

    /* Complex sin/cos helpers:
     * sin(a+bi) = sin(a)*cosh(b) + i*cos(a)*sinh(b)
     * cos(a+bi) = cos(a)*cosh(b) - i*sin(a)*sinh(b) */
    #define CSIN_RE(re, im) (sin(re) * cosh(im))
    #define CSIN_IM(re, im) (cos(re) * sinh(im))
    #define CCOS_RE(re, im) (cos(re) * cosh(im))
    #define CCOS_IM(re, im) (-sin(re) * sinh(im))

    /* cf[1] = t1 + t2  (C index 0) */
    cRe[0] = t1R + t2R;
    cIm[0] = t1I + t2I;

    /* cf[2] = 1 + t1*t2 + log(|t1+t2| + 1)  (C index 1) */
    double sumR = t1R + t2R, sumI = t1I + t2I;
    double logVal = log(CABS(sumR, sumI) + 1.0);
    cRe[1] = 1.0 + t1t2R + logVal;
    cIm[1] = t1t2I;

    /* cf[3] = t1 + t2 + log(|1 - t1*t2| + 1)  (C index 2) */
    double dR = 1.0 - t1t2R, dI = -t1t2I;
    logVal = log(CABS(dR, dI) + 1.0);
    cRe[2] = t1R + t2R + logVal;
    cIm[2] = t1I + t2I;

    /* for i in 4:51: cf[i] = i*t1 + (51-i)*t2 + log(|t1-t2*i| + 1)
     * C index = i-1, so for ci=3..50 */
    for (int i = 4; i <= 51; i++) {
        int ci = i - 1;  /* C 0-based index */
        double di = (double)i;
        /* i*t1 + (51-i)*t2 */
        double rR = di * t1R + (51.0 - di) * t2R;
        double rI = di * t1I + (51.0 - di) * t2I;
        /* t1 - t2*i */
        double diffR = t1R - t2R * di;
        double diffI = t1I - t2I * di;
        logVal = log(CABS(diffR, diffI) + 1.0);
        cRe[ci] = rR + logVal;
        cIm[ci] = rI;
    }

    /* Complex sin(t1) and cos(t2) — needed for overwrites */
    double sinT1R = CSIN_RE(t1R, t1I);
    double sinT1I = CSIN_IM(t1R, t1I);
    double cosT2R = CCOS_RE(t2R, t2I);
    double cosT2I = CCOS_IM(t2R, t2I);
    double sinT2R = CSIN_RE(t2R, t2I);
    double sinT2I = CSIN_IM(t2R, t2I);

    /* Save loop values of cf[31] and cf[41] (C indices 30 and 40) */
    double loop30R = cRe[30], loop30I = cIm[30];
    double loop40R = cRe[40], loop40I = cIm[40];

    /* cf[11] = cf[1] + cf[10] - sin(t1)  (C: [10] = [0] + [9] - sin(t1)) */
    cRe[10] = cRe[0] + cRe[9] - sinT1R;
    cIm[10] = cIm[0] + cIm[9] - sinT1I;

    /* cf[21] = cf[31] + cf[41] - cos(t2)  (C: [20] = loop[30] + loop[40] - cos(t2)) */
    cRe[20] = loop30R + loop40R - cosT2R;
    cIm[20] = loop30I + loop40I - cosT2I;

    /* cf[31] = cf[21] + cf[41] + sin(t1)  (C: [30] = new[20] + loop[40] + sin(t1)) */
    cRe[30] = cRe[20] + loop40R + sinT1R;
    cIm[30] = cIm[20] + loop40I + sinT1I;

    /* cf[41] = cf[31] + cf[21] - cos(t2)  (C: [40] = new[30] + new[20] - cos(t2)) */
    cRe[40] = cRe[30] + cRe[20] - cosT2R;
    cIm[40] = cIm[30] + cIm[20] - cosT2I;

    /* cf[51] = cf[41] + cf[21] + sin(t2)  (C: [50] = new[40] + new[20] + sin(t2)) */
    cRe[50] = cRe[40] + cRe[20] + sinT2R;
    cIm[50] = cIm[40] + cIm[20] + sinT2I;

    #undef CABS
    #undef CSIN_RE
    #undef CSIN_IM
    #undef CCOS_RE
    #undef CCOS_IM
}

/*
 * giga_1: degree-24 polynomial with 25 coefficients.
 * t1 = exp(2πi·x1), t2 = exp(2πi·x2) (complex, on unit circle).
 * R definition (1-based, reversed at end):
 *   cf[1]  = 30 * (t1^2 * t2)
 *   cf[2]  = 30 * (t1 * t2^2)
 *   cf[3]  = 40 * (t1^3)
 *   cf[4]  = 40 * (t2^3)
 *   cf[5]  = -25 * (t1^2)
 *   cf[6]  = -25 * (t2^2)
 *   cf[7]  = 10 * (t1 * t2)
 *   cf[10] = 100 * (t1^4 * t2^4)
 *   cf[12] = -5 * (t1)
 *   cf[14] = 5 * (t2)
 *   cf[25] = -10
 *   cf = rev(cf)
 *
 * After rev: R index k → C index 25-k (0-based).
 */
static void giga_1(double x1, double x2,
                   double *cRe, double *cIm, int *nCoeffs)
{
    *nCoeffs = 25;
    memset(cRe, 0, 25 * sizeof(double));
    memset(cIm, 0, 25 * sizeof(double));

    double a1 = 2.0 * M_PI * x1, a2 = 2.0 * M_PI * x2;
    double t1R = cos(a1), t1I = sin(a1);
    double t2R = cos(a2), t2I = sin(a2);

    /* Powers of t1 */
    double t1_2R = t1R * t1R - t1I * t1I;
    double t1_2I = 2.0 * t1R * t1I;
    double t1_3R = t1_2R * t1R - t1_2I * t1I;
    double t1_3I = t1_2R * t1I + t1_2I * t1R;
    double t1_4R = t1_3R * t1R - t1_3I * t1I;
    double t1_4I = t1_3R * t1I + t1_3I * t1R;

    /* Powers of t2 */
    double t2_2R = t2R * t2R - t2I * t2I;
    double t2_2I = 2.0 * t2R * t2I;
    double t2_3R = t2_2R * t2R - t2_2I * t2I;
    double t2_3I = t2_2R * t2I + t2_2I * t2R;
    double t2_4R = t2_3R * t2R - t2_3I * t2I;
    double t2_4I = t2_3R * t2I + t2_3I * t2R;

    /* Products */
    double t1t2R = t1R * t2R - t1I * t2I;
    double t1t2I = t1R * t2I + t1I * t2R;

    double t1_2t2R = t1_2R * t2R - t1_2I * t2I;  /* t1^2 * t2 */
    double t1_2t2I = t1_2R * t2I + t1_2I * t2R;

    double t1t2_2R = t1R * t2_2R - t1I * t2_2I;   /* t1 * t2^2 */
    double t1t2_2I = t1R * t2_2I + t1I * t2_2R;

    double t1_4t2_4R = t1_4R * t2_4R - t1_4I * t2_4I;  /* t1^4 * t2^4 */
    double t1_4t2_4I = t1_4R * t2_4I + t1_4I * t2_4R;

    /* R index k → after rev → C index (25 - k) */

    /* cf[1] = 30*(t1^2*t2) → C[24] */
    cRe[24] = 30.0 * t1_2t2R;
    cIm[24] = 30.0 * t1_2t2I;

    /* cf[2] = 30*(t1*t2^2) → C[23] */
    cRe[23] = 30.0 * t1t2_2R;
    cIm[23] = 30.0 * t1t2_2I;

    /* cf[3] = 40*(t1^3) → C[22] */
    cRe[22] = 40.0 * t1_3R;
    cIm[22] = 40.0 * t1_3I;

    /* cf[4] = 40*(t2^3) → C[21] */
    cRe[21] = 40.0 * t2_3R;
    cIm[21] = 40.0 * t2_3I;

    /* cf[5] = -25*(t1^2) → C[20] */
    cRe[20] = -25.0 * t1_2R;
    cIm[20] = -25.0 * t1_2I;

    /* cf[6] = -25*(t2^2) → C[19] */
    cRe[19] = -25.0 * t2_2R;
    cIm[19] = -25.0 * t2_2I;

    /* cf[7] = 10*(t1*t2) → C[18] */
    cRe[18] = 10.0 * t1t2R;
    cIm[18] = 10.0 * t1t2I;

    /* cf[10] = 100*(t1^4*t2^4) → C[15] */
    cRe[15] = 100.0 * t1_4t2_4R;
    cIm[15] = 100.0 * t1_4t2_4I;

    /* cf[12] = -5*(t1) → C[13] */
    cRe[13] = -5.0 * t1R;
    cIm[13] = -5.0 * t1I;

    /* cf[14] = 5*(t2) → C[11] */
    cRe[11] = 5.0 * t2R;
    cIm[11] = 5.0 * t2I;

    /* cf[25] = -10 → C[0] */
    cRe[0] = -10.0;
}

/* rev_giga_1: reversed coefficients of giga_1. */
static void rev_giga_1(double x1, double x2,
                       double *cRe, double *cIm, int *nCoeffs)
{
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS];
    int n;
    giga_1(x1, x2, tmpRe, tmpIm, &n);
    *nCoeffs = n;
    for (int k = 0; k < n; k++) {
        cRe[k] = tmpRe[n - 1 - k];
        cIm[k] = tmpIm[n - 1 - k];
    }
}

/* rev_giga_87: reversed coefficients of giga_87. */
static void rev_giga_87(double x1, double x2,
                        double *cRe, double *cIm, int *nCoeffs)
{
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS];
    int n;
    giga_87(x1, x2, tmpRe, tmpIm, &n);
    *nCoeffs = n;
    for (int k = 0; k < n; k++) {
        cRe[k] = tmpRe[n - 1 - k];
        cIm[k] = tmpIm[n - 1 - k];
    }
}

/*
 * giga_30: degree-9 polynomial with 10 coefficients.
 * t1 = exp(2πi·x1), t2 = exp(2πi·x2)  (unit circle)
 * R code (ascending, no rev):
 *   n=10; cf = rep(0,n)
 *   cf[1]   = 150i * t2^2 + 100 * t1^3
 *   cf[n/2] = 150 * abs(t1 + t2 - 2.5*(1i+1))
 *   cf[n]   = 100i * t1^3 + 150 * t2^2
 * Mapping: R[k] → C[10-k]
 */
static void giga_30(double x1, double x2,
                    double *cRe, double *cIm, int *nCoeffs)
{
    *nCoeffs = 10;
    memset(cRe, 0, 10 * sizeof(double));
    memset(cIm, 0, 10 * sizeof(double));

    double a1 = 2.0 * M_PI * x1, a2 = 2.0 * M_PI * x2;
    double t1R = cos(a1), t1I = sin(a1);
    double t2R = cos(a2), t2I = sin(a2);

    /* Powers */
    double t2_2R = t2R * t2R - t2I * t2I;
    double t2_2I = 2.0 * t2R * t2I;
    double t1_2R = t1R * t1R - t1I * t1I;
    double t1_2I = 2.0 * t1R * t1I;
    double t1_3R = t1_2R * t1R - t1_2I * t1I;
    double t1_3I = t1_2R * t1I + t1_2I * t1R;

    /* cf[1] = 150i*t2^2 + 100*t1^3 → C[9]
     * 150i*(t2_2R + i*t2_2I) = -150*t2_2I + i*150*t2_2R */
    cRe[9] = -150.0 * t2_2I + 100.0 * t1_3R;
    cIm[9] =  150.0 * t2_2R + 100.0 * t1_3I;

    /* cf[5] = 150*|t1 + t2 - 2.5*(1+i)| → C[5]
     * abs() gives real result (complex modulus) */
    double dR = t1R + t2R - 2.5;
    double dI = t1I + t2I - 2.5;
    cRe[5] = 150.0 * sqrt(dR * dR + dI * dI);

    /* cf[10] = 100i*t1^3 + 150*t2^2 → C[0]
     * 100i*(t1_3R + i*t1_3I) = -100*t1_3I + i*100*t1_3R */
    cRe[0] = -100.0 * t1_3I + 150.0 * t2_2R;
    cIm[0] =  100.0 * t1_3R + 150.0 * t2_2I;
}

/* rev_giga_30: reversed coefficients of giga_30. */
static void rev_giga_30(double x1, double x2,
                        double *cRe, double *cIm, int *nCoeffs)
{
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS];
    int n;
    giga_30(x1, x2, tmpRe, tmpIm, &n);
    *nCoeffs = n;
    for (int k = 0; k < n; k++) {
        cRe[k] = tmpRe[n - 1 - k];
        cIm[k] = tmpIm[n - 1 - k];
    }
}

/*
 * giga_39: degree-49 polynomial with 50 coefficients.
 * t1 = exp(2πi·x1), t2 = exp(2πi·x2)  (unit circle)
 * R code (ascending, no rev):
 *   cf = complex(50)
 *   cf[c(1,10,20,30,40,50)] = c(1,2,-3,4,-5,6)
 *   cf[15] = 100*(t1^2 + t2^2)
 *   cf[25] = 50*(sin(t1) + 1i*cos(t2))
 *   cf[35] = 200*(t1*t2) + 1i*(t1^3 - t2^3)
 *   cf[45] = exp(1i*(t1+t2)) + exp(-1i*(t1-t2))
 * Mapping: R[k] → C[50-k]
 */
static void giga_39(double x1, double x2,
                    double *cRe, double *cIm, int *nCoeffs)
{
    *nCoeffs = 50;
    memset(cRe, 0, 50 * sizeof(double));
    memset(cIm, 0, 50 * sizeof(double));

    double a1 = 2.0 * M_PI * x1, a2 = 2.0 * M_PI * x2;
    double t1R = cos(a1), t1I = sin(a1);
    double t2R = cos(a2), t2I = sin(a2);

    /* Fixed real coefficients: R[k] → C[50-k] */
    cRe[49] = 1.0;   /* R[1] */
    cRe[40] = 2.0;   /* R[10] */
    cRe[30] = -3.0;  /* R[20] */
    cRe[20] = 4.0;   /* R[30] */
    cRe[10] = -5.0;  /* R[40] */
    cRe[0]  = 6.0;   /* R[50] */

    /* Powers */
    double t1_2R = t1R * t1R - t1I * t1I;
    double t1_2I = 2.0 * t1R * t1I;
    double t2_2R = t2R * t2R - t2I * t2I;
    double t2_2I = 2.0 * t2R * t2I;
    double t1_3R = t1_2R * t1R - t1_2I * t1I;
    double t1_3I = t1_2R * t1I + t1_2I * t1R;
    double t2_3R = t2_2R * t2R - t2_2I * t2I;
    double t2_3I = t2_2R * t2I + t2_2I * t2R;

    /* t1*t2 */
    double t1t2R = t1R * t2R - t1I * t2I;
    double t1t2I = t1R * t2I + t1I * t2R;

    /* cf[15] = 100*(t1^2 + t2^2) → C[35] */
    cRe[35] = 100.0 * (t1_2R + t2_2R);
    cIm[35] = 100.0 * (t1_2I + t2_2I);

    /* cf[25] = 50*(sin(t1) + i*cos(t2)) → C[25]
     * sin(a+bi) = sin(a)cosh(b) + i*cos(a)sinh(b)
     * cos(c+di) = cos(c)cosh(d) - i*sin(c)sinh(d)
     * i*cos(t2) = i*(cost2R + i*cost2I) = -cost2I + i*cost2R */
    double sint1R = sin(t1R) * cosh(t1I);
    double sint1I = cos(t1R) * sinh(t1I);
    double cost2R = cos(t2R) * cosh(t2I);
    double cost2I = -sin(t2R) * sinh(t2I);
    cRe[25] = 50.0 * (sint1R - cost2I);
    cIm[25] = 50.0 * (sint1I + cost2R);

    /* cf[35] = 200*(t1*t2) + i*(t1^3 - t2^3) → C[15]
     * i*(t1^3 - t2^3) = -(t1_3I - t2_3I) + i*(t1_3R - t2_3R) */
    cRe[15] = 200.0 * t1t2R - (t1_3I - t2_3I);
    cIm[15] = 200.0 * t1t2I + (t1_3R - t2_3R);

    /* cf[45] = exp(i*(t1+t2)) + exp(-i*(t1-t2)) → C[5]
     * Let s = t1+t2, d = t1-t2
     * i*s = -sI + i*sR → exp(-sI) * (cos(sR) + i*sin(sR))
     * -i*d = dI - i*dR → exp(dI) * (cos(dR) - i*sin(dR)) */
    double sR = t1R + t2R, sI = t1I + t2I;
    double ea = exp(-sI);
    double eaR = ea * cos(sR), eaI = ea * sin(sR);
    double dmR = t1R - t2R, dmI = t1I - t2I;
    double eb = exp(dmI);
    double ebR = eb * cos(dmR), ebI = -eb * sin(dmR);
    cRe[5] = eaR + ebR;
    cIm[5] = eaI + ebI;
}

/* rev_giga_39: reversed coefficients of giga_39. */
static void rev_giga_39(double x1, double x2,
                        double *cRe, double *cIm, int *nCoeffs)
{
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS];
    int n;
    giga_39(x1, x2, tmpRe, tmpIm, &n);
    *nCoeffs = n;
    for (int k = 0; k < n; k++) {
        cRe[k] = tmpRe[n - 1 - k];
        cIm[k] = tmpIm[n - 1 - k];
    }
}

/*
 * giga_40: degree-34 polynomial with 35 coefficients.
 * t1 = exp(2πi·x1), t2 = exp(2πi·x2)  (unit circle)
 * R code (ascending, no rev):
 *   cf = complex(35)
 *   cf[c(1,7,15,20,27,35)] = c(1,-2,3,-4,5,-6)
 *   cf[12] = 50i * sin(t1^2 - t2^2)
 *   cf[18] = 100 * (cos(t1) + 1i*sin(t2))
 *   cf[25] = 50 * (t1^3 - t2^3 + 1i*t1*t2)
 *   cf[30] = 200*exp(1i*t1) + 50*exp(-1i*t2)
 * Mapping: R[k] → C[35-k]
 */
static void giga_40(double x1, double x2,
                    double *cRe, double *cIm, int *nCoeffs)
{
    *nCoeffs = 35;
    memset(cRe, 0, 35 * sizeof(double));
    memset(cIm, 0, 35 * sizeof(double));

    double a1 = 2.0 * M_PI * x1, a2 = 2.0 * M_PI * x2;
    double t1R = cos(a1), t1I = sin(a1);
    double t2R = cos(a2), t2I = sin(a2);

    /* Fixed real coefficients: R[k] → C[35-k] */
    cRe[34] = 1.0;   /* R[1] */
    cRe[28] = -2.0;  /* R[7] */
    cRe[20] = 3.0;   /* R[15] */
    cRe[15] = -4.0;  /* R[20] */
    cRe[8]  = 5.0;   /* R[27] */
    cRe[0]  = -6.0;  /* R[35] */

    /* Powers */
    double t1_2R = t1R * t1R - t1I * t1I;
    double t1_2I = 2.0 * t1R * t1I;
    double t2_2R = t2R * t2R - t2I * t2I;
    double t2_2I = 2.0 * t2R * t2I;
    double t1_3R = t1_2R * t1R - t1_2I * t1I;
    double t1_3I = t1_2R * t1I + t1_2I * t1R;
    double t2_3R = t2_2R * t2R - t2_2I * t2I;
    double t2_3I = t2_2R * t2I + t2_2I * t2R;

    /* t1*t2 */
    double t1t2R = t1R * t2R - t1I * t2I;
    double t1t2I = t1R * t2I + t1I * t2R;

    /* cf[12] = 50i * sin(t1^2 - t2^2) → C[23]
     * w = t1^2 - t2^2
     * sin(a+bi) = sin(a)cosh(b) + i*cos(a)sinh(b)
     * 50i * sin(w) = -50*sinwI + i*50*sinwR */
    double wR = t1_2R - t2_2R, wI = t1_2I - t2_2I;
    double sinwR = sin(wR) * cosh(wI);
    double sinwI = cos(wR) * sinh(wI);
    cRe[23] = -50.0 * sinwI;
    cIm[23] =  50.0 * sinwR;

    /* cf[18] = 100*(cos(t1) + i*sin(t2)) → C[17]
     * cos(a+bi) = cos(a)cosh(b) - i*sin(a)sinh(b)
     * sin(c+di) = sin(c)cosh(d) + i*cos(c)sinh(d)
     * i*sin(t2) = -sint2I + i*sint2R */
    double cost1R = cos(t1R) * cosh(t1I);
    double cost1I = -sin(t1R) * sinh(t1I);
    double sint2R = sin(t2R) * cosh(t2I);
    double sint2I = cos(t2R) * sinh(t2I);
    cRe[17] = 100.0 * (cost1R - sint2I);
    cIm[17] = 100.0 * (cost1I + sint2R);

    /* cf[25] = 50*(t1^3 - t2^3 + i*t1*t2) → C[10]
     * i*(t1t2R + i*t1t2I) = -t1t2I + i*t1t2R */
    cRe[10] = 50.0 * (t1_3R - t2_3R - t1t2I);
    cIm[10] = 50.0 * (t1_3I - t2_3I + t1t2R);

    /* cf[30] = 200*exp(i*t1) + 50*exp(-i*t2) → C[5]
     * i*t1 = -t1I + i*t1R → exp(-t1I)*(cos(t1R) + i*sin(t1R))
     * -i*t2 = t2I - i*t2R → exp(t2I)*(cos(t2R) - i*sin(t2R)) */
    double e1 = exp(-t1I);
    double eit1R = e1 * cos(t1R), eit1I = e1 * sin(t1R);
    double e2 = exp(t2I);
    double emit2R = e2 * cos(t2R), emit2I = -e2 * sin(t2R);
    cRe[5] = 200.0 * eit1R + 50.0 * emit2R;
    cIm[5] = 200.0 * eit1I + 50.0 * emit2I;
}

/* rev_giga_40: reversed coefficients of giga_40. */
static void rev_giga_40(double x1, double x2,
                        double *cRe, double *cIm, int *nCoeffs)
{
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS];
    int n;
    giga_40(x1, x2, tmpRe, tmpIm, &n);
    *nCoeffs = n;
    for (int k = 0; k < n; k++) {
        cRe[k] = tmpRe[n - 1 - k];
        cIm[k] = tmpIm[n - 1 - k];
    }
}

/*
 * p7f: degree-22 polynomial with 23 coefficients.
 * t1 = x1, t2 = x2 (raw 0-to-1, NO unit circle transform).
 *
 * tt1  = exp(i*2π*t1)           (unit circle point)
 * ttt1 = exp(i*2π*tt1)          (nested exp, tt1 is complex)
 * v    = linspace(Re(tt1), Re(ttt1), 23)   (23 real values)
 *
 * Coefficients: f[k] = scale * exp(i * trig(freq * 2π * v[k]))
 * where scale, freq, and trig (sin/cos) depend on t2 bracket.
 * f[22] += 211 * exp(i * 2π * (1/7) * t2)
 *
 * Python 0-indexed → reverse for C leading-first.
 */
static void p7f(double x1, double x2,
                double *cRe, double *cIm, int *nCoeffs)
{
    *nCoeffs = 23;
    double t1 = x1, t2 = x2;
    double pi2 = 2.0 * M_PI;

    /* tt1 = exp(i*2π*t1) */
    double a1 = pi2 * t1;
    double tt1R = cos(a1), tt1I = sin(a1);

    /* ttt1 = exp(i*2π*tt1), tt1 is complex
     * i*2π*tt1 = 2π*(-tt1I + i*tt1R)
     * exp(-2π*tt1I) * (cos(2π*tt1R) + i*sin(2π*tt1R)) */
    double eArg = -pi2 * tt1I;
    double aArg = pi2 * tt1R;
    double ttt1R = exp(eArg) * cos(aArg);
    /* ttt1I not needed — only Re(ttt1) used for linspace */

    /* v = linspace(Re(tt1), Re(ttt1), 23) */
    double v0 = tt1R;
    double vEnd = ttt1R;
    double dv = (vEnd - v0) / 22.0;

    /* Determine scale, freq, and trig function from t2 bracket */
    double scale;
    double freq;
    int useCos = 0;  /* 0 = sin, 1 = cos */

    if (t2 < 0.1) {
        scale = 10.0 * t1; freq = 11.0;
    } else if (t2 < 0.2) {
        scale = 100.0; freq = 17.0;
    } else if (t2 < 0.3) {
        scale = 599.0; freq = 83.0; useCos = 1;
    } else if (t2 < 0.4) {
        scale = 443.0; freq = 179.0;
    } else if (t2 < 0.5) {
        scale = 293.0; freq = 127.0;
    } else if (t2 < 0.6) {
        scale = 541.0; freq = 103.0;
    } else if (t2 < 0.7) {
        scale = 379.0; freq = 283.0;
    } else if (t2 < 0.8) {
        scale = 233.0; freq = 3.0;
    } else if (t2 < 0.9) {
        scale = 173.0; freq = 5.0;
    } else {
        scale = 257.0; freq = 23.0;
    }

    /* Compute f[k] = scale * exp(i * trig(freq * 2π * v[k]))
     * trig(x) is real → exp(i * real) = cos(real) + i*sin(real) */
    double fRe[23], fIm[23];
    for (int k = 0; k < 23; k++) {
        double vk = v0 + k * dv;
        double trigArg = freq * pi2 * vk;
        double trigVal = useCos ? cos(trigArg) : sin(trigArg);
        fRe[k] = scale * cos(trigVal);
        fIm[k] = scale * sin(trigVal);
    }

    /* f[22] += 211 * exp(i * 2π * (1/7) * t2) */
    double addAngle = pi2 * (1.0 / 7.0) * t2;
    fRe[22] += 211.0 * cos(addAngle);
    fIm[22] += 211.0 * sin(addAngle);

    /* Reverse: Python ascending → C leading-first */
    for (int k = 0; k < 23; k++) {
        cRe[k] = fRe[22 - k];
        cIm[k] = fIm[22 - k];
    }
}

/* rev_p7f: reversed coefficients of p7f. */
static void rev_p7f(double x1, double x2,
                    double *cRe, double *cIm, int *nCoeffs)
{
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS];
    int n;
    p7f(x1, x2, tmpRe, tmpIm, &n);
    *nCoeffs = n;
    for (int k = 0; k < n; k++) {
        cRe[k] = tmpRe[n - 1 - k];
        cIm[k] = tmpIm[n - 1 - k];
    }
}

/*
 * giga_230: degree-9 polynomial with 10 coefficients.
 * Pipeline: uc → coeff3 → poly_giga_53 → rev.
 *
 * uc:      t1 = exp(i*2π*x1), t2 = exp(i*2π*x2)
 * coeff3:  t1 = 1/(t1+2),     t2 = 1/(t2+2)
 *
 * poly_giga_53(t1, t2):
 *   cf[0] = 100 * sin(t1)³ * cos(t2)²
 *   cf[1] = 100 * exp(i*(t1+t2)) - 10*(t1-t2)²
 *   cf[2] = t1*t2*(t1-t2) / (|t1|+|t2|+1)
 *   cf[4] = (t1*t2*exp(i*(t1²-t2²)))³
 *   cf[6] = sqrt(|t1|) - sqrt(|t2|) + i*sin(t1*t2)
 *   cf[7] = 50*|t1-t2|*exp(i*|t1+t2|)
 *   cf[8] = t1-|t2| if Im(t1)>0 else t2-|t1|
 *   cf[9] = (i*t1*t2)^(0.1*t1*t2)
 *
 * rev: C[k] = cf[9-k]
 */
static void giga_230(double x1, double x2,
                     double *cRe, double *cIm, int *nCoeffs)
{
    *nCoeffs = 10;
    memset(cRe, 0, 10 * sizeof(double));
    memset(cIm, 0, 10 * sizeof(double));

    /* uc */
    double a1 = 2.0 * M_PI * x1, a2 = 2.0 * M_PI * x2;
    double u1R = cos(a1), u1I = sin(a1);
    double u2R = cos(a2), u2I = sin(a2);

    /* coeff3: 1/(t+2) */
    double d1R = u1R + 2.0, d1I = u1I, m1 = d1R * d1R + d1I * d1I;
    double t1R = d1R / m1, t1I = -d1I / m1;
    double d2R = u2R + 2.0, d2I = u2I, m2 = d2R * d2R + d2I * d2I;
    double t2R = d2R / m2, t2I = -d2I / m2;

    /* Common values */
    double absT1 = sqrt(t1R * t1R + t1I * t1I);
    double absT2 = sqrt(t2R * t2R + t2I * t2I);
    double pR = t1R * t2R - t1I * t2I, pI = t1R * t2I + t1I * t2R;
    double difR = t1R - t2R, difI = t1I - t2I;
    double sumR = t1R + t2R, sumI = t1I + t2I;

    double cfR[10], cfI[10];
    memset(cfR, 0, sizeof(cfR));
    memset(cfI, 0, sizeof(cfI));

    /* cf[0] = 100 * sin(t1)^3 * cos(t2)^2 */
    double st1R = sin(t1R) * cosh(t1I), st1I = cos(t1R) * sinh(t1I);
    double st1_2R = st1R * st1R - st1I * st1I, st1_2I = 2.0 * st1R * st1I;
    double st1_3R = st1_2R * st1R - st1_2I * st1I;
    double st1_3I = st1_2R * st1I + st1_2I * st1R;
    double ct2R_v = cos(t2R) * cosh(t2I), ct2I_v = -sin(t2R) * sinh(t2I);
    double ct2_2R = ct2R_v * ct2R_v - ct2I_v * ct2I_v;
    double ct2_2I = 2.0 * ct2R_v * ct2I_v;
    cfR[0] = 100.0 * (st1_3R * ct2_2R - st1_3I * ct2_2I);
    cfI[0] = 100.0 * (st1_3R * ct2_2I + st1_3I * ct2_2R);

    /* cf[1] = 100*exp(i*(t1+t2)) - 10*(t1-t2)^2
     * i*(sumR+i*sumI) = (-sumI, sumR) → exp(-sumI)*(cos(sumR)+i*sin(sumR)) */
    double e1v = exp(-sumI);
    double ex1R = e1v * cos(sumR), ex1I = e1v * sin(sumR);
    double dif2R = difR * difR - difI * difI, dif2I = 2.0 * difR * difI;
    cfR[1] = 100.0 * ex1R - 10.0 * dif2R;
    cfI[1] = 100.0 * ex1I - 10.0 * dif2I;

    /* cf[2] = t1*t2*(t1-t2) / (|t1|+|t2|+1) */
    double pdR = pR * difR - pI * difI, pdI = pR * difI + pI * difR;
    double den2 = absT1 + absT2 + 1.0;
    cfR[2] = pdR / den2;
    cfI[2] = pdI / den2;

    /* cf[4] = (t1*t2*exp(i*(t1^2-t2^2)))^3 */
    double t1_2R = t1R * t1R - t1I * t1I, t1_2I = 2.0 * t1R * t1I;
    double t2_2R = t2R * t2R - t2I * t2I, t2_2I = 2.0 * t2R * t2I;
    double dsqR = t1_2R - t2_2R, dsqI = t1_2I - t2_2I;
    double e4v = exp(-dsqI);
    double ex4R = e4v * cos(dsqR), ex4I = e4v * sin(dsqR);
    double pe4R = pR * ex4R - pI * ex4I, pe4I = pR * ex4I + pI * ex4R;
    double pe4_2R = pe4R * pe4R - pe4I * pe4I, pe4_2I = 2.0 * pe4R * pe4I;
    cfR[4] = pe4_2R * pe4R - pe4_2I * pe4I;
    cfI[4] = pe4_2R * pe4I + pe4_2I * pe4R;

    /* cf[6] = sqrt(|t1|) - sqrt(|t2|) + i*sin(t1*t2) */
    double sinpR = sin(pR) * cosh(pI), sinpI = cos(pR) * sinh(pI);
    cfR[6] = sqrt(absT1) - sqrt(absT2) - sinpI;
    cfI[6] = sinpR;

    /* cf[7] = 50*|t1-t2|*exp(i*|t1+t2|) */
    double absDif = sqrt(difR * difR + difI * difI);
    double absSm = sqrt(sumR * sumR + sumI * sumI);
    cfR[7] = 50.0 * absDif * cos(absSm);
    cfI[7] = 50.0 * absDif * sin(absSm);

    /* cf[8] = t1-|t2| if Im(t1)>0 else t2-|t1| */
    if (t1I > 0) { cfR[8] = t1R - absT2; cfI[8] = t1I; }
    else         { cfR[8] = t2R - absT1; cfI[8] = t2I; }

    /* cf[9] = (i*t1*t2)^(0.1*t1*t2) = exp(w*log(z)), z=i*p, w=0.1*p */
    double zR = -pI, zI = pR;
    double wR = 0.1 * pR, wI = 0.1 * pI;
    double absZ = sqrt(zR * zR + zI * zI);
    if (absZ > 1e-300) {
        double logZR = log(absZ), logZI = atan2(zI, zR);
        double wlR = wR * logZR - wI * logZI;
        double wlI = wR * logZI + wI * logZR;
        double ewl = exp(wlR);
        cfR[9] = ewl * cos(wlI);
        cfI[9] = ewl * sin(wlI);
    }

    /* rev + safe: C[k] = cf[9-k], NaN/Inf → 0 */
    for (int k = 0; k < 10; k++) {
        double re = cfR[9 - k], im = cfI[9 - k];
        cRe[k] = isfinite(re) ? re : 0.0;
        cIm[k] = isfinite(im) ? im : 0.0;
    }
}

/* rev_giga_230: reversed coefficients of giga_230. */
static void rev_giga_230(double x1, double x2,
                         double *cRe, double *cIm, int *nCoeffs)
{
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS];
    int n;
    giga_230(x1, x2, tmpRe, tmpIm, &n);
    *nCoeffs = n;
    for (int k = 0; k < n; k++) {
        cRe[k] = tmpRe[n - 1 - k];
        cIm[k] = tmpIm[n - 1 - k];
    }
}

/*
 * giga_232: degree-8 polynomial with 9 coefficients.
 * Pipeline: uc → coeff2 → poly_729 → rev → safe.
 *
 * uc:     t1 = exp(i*2π*x1), t2 = exp(i*2π*x2)
 * coeff2: t1_new = t1+t2, t2_new = t1*t2
 *
 * poly_729(t1, t2):
 *   for j in 0..8:
 *     r_part    = Re(t1)*j² - Re(t2)*sqrt(j+1)
 *     im_part   = (Im(t1)+Im(t2))*log(j+2)
 *     magnitude = |t1|^(j%3+1) + |t2|^(8-j)
 *     angle     = arg(t1)*sin(j) + arg(t2)*cos(j)
 *     cf[j]     = (r_part + i*im_part) * magnitude * exp(i*angle)
 *
 * rev: C[k] = cf[8-k]
 * safe: NaN/Inf → 0
 */
static void giga_232(double x1, double x2,
                     double *cRe, double *cIm, int *nCoeffs)
{
    *nCoeffs = 9;
    memset(cRe, 0, 9 * sizeof(double));
    memset(cIm, 0, 9 * sizeof(double));

    /* uc */
    double a1 = 2.0 * M_PI * x1, a2 = 2.0 * M_PI * x2;
    double u1R = cos(a1), u1I = sin(a1);
    double u2R = cos(a2), u2I = sin(a2);

    /* coeff2: t1 = u1+u2, t2 = u1*u2 */
    double t1R = u1R + u2R, t1I = u1I + u2I;
    double t2R = u1R * u2R - u1I * u2I, t2I = u1R * u2I + u1I * u2R;

    /* Precompute moduli, arguments, powers */
    double absT1 = sqrt(t1R * t1R + t1I * t1I);
    double absT2 = sqrt(t2R * t2R + t2I * t2I);
    double angT1 = atan2(t1I, t1R);
    double angT2 = atan2(t2I, t2R);

    double absT1p[4]; /* absT1^1, ^2, ^3 (index 1..3) */
    absT1p[1] = absT1;
    absT1p[2] = absT1 * absT1;
    absT1p[3] = absT1p[2] * absT1;

    double absT2p[9]; /* absT2^0 .. ^8 */
    absT2p[0] = 1.0;
    for (int i = 1; i <= 8; i++) absT2p[i] = absT2p[i - 1] * absT2;

    double cfR[9], cfI[9];
    for (int j = 0; j <= 8; j++) {
        double jd = (double)j;
        double r_part = t1R * (jd * jd) - t2R * sqrt(jd + 1.0);
        double im_part = (t1I + t2I) * log(jd + 2.0);
        double magnitude = absT1p[(j % 3) + 1] + absT2p[8 - j];
        double ang = angT1 * sin(jd) + angT2 * cos(jd);

        /* (r_part + i*im_part) * magnitude * exp(i*angle) */
        double zmR = r_part * magnitude, zmI = im_part * magnitude;
        double eR = cos(ang), eI = sin(ang);
        cfR[j] = zmR * eR - zmI * eI;
        cfI[j] = zmR * eI + zmI * eR;
    }

    /* safe + rev: C[k] = cf[8-k], NaN/Inf → 0 */
    for (int k = 0; k < 9; k++) {
        double re = cfR[8 - k], im = cfI[8 - k];
        cRe[k] = isfinite(re) ? re : 0.0;
        cIm[k] = isfinite(im) ? im : 0.0;
    }
}

/* rev_giga_232: reversed coefficients of giga_232. */
static void rev_giga_232(double x1, double x2,
                         double *cRe, double *cIm, int *nCoeffs)
{
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS];
    int n;
    giga_232(x1, x2, tmpRe, tmpIm, &n);
    *nCoeffs = n;
    for (int k = 0; k < n; k++) {
        cRe[k] = tmpRe[n - 1 - k];
        cIm[k] = tmpIm[n - 1 - k];
    }
}

/*
 * giga_227: degree-24 polynomial with 25 coefficients.
 * Pipeline: uc → coeff3 → poly_giga_62 → rev.
 *
 * uc:      t1 = exp(i*2π*x1), t2 = exp(i*2π*x2)
 * coeff3:  t1 = 1/(t1+2),     t2 = 1/(t2+2)
 *
 * poly_giga_62(t1, t2):
 *   cf[0:5]   = |t1+t2|^(i+1) for i=0..4
 *   cf[5:10]  = Re((t1+2j*t2)^3) * log(|t1*t2|)  (all 5 same)
 *   cf[10:15] = Im((t1-t2)^2) / angle(t1*t2)     (all 5 same)
 *   cf[15:20] = sqrt(|cf[5:10]|) + angle(cf[0:5]) (all 5 same, angle=0)
 *   cf[20:25] = |t1*t2|^(i+1) for i=0..4
 *
 * rev: C[k] = cf[24-k]
 * All coefficients are real.
 */
static void giga_227(double x1, double x2,
                     double *cRe, double *cIm, int *nCoeffs)
{
    *nCoeffs = 25;
    memset(cRe, 0, 25 * sizeof(double));
    memset(cIm, 0, 25 * sizeof(double));

    /* uc: unit circle */
    double a1 = 2.0 * M_PI * x1, a2 = 2.0 * M_PI * x2;
    double t1R = cos(a1), t1I = sin(a1);
    double t2R = cos(a2), t2I = sin(a2);

    /* coeff3: t = 1/(t+2)
     * 1/((a+2) + bi) = ((a+2) - bi) / ((a+2)² + b²) */
    double d1R = t1R + 2.0, d1I = t1I;
    double m1 = d1R * d1R + d1I * d1I;
    t1R = d1R / m1; t1I = -d1I / m1;

    double d2R = t2R + 2.0, d2I = t2I;
    double m2 = d2R * d2R + d2I * d2I;
    t2R = d2R / m2; t2I = -d2I / m2;

    /* --- poly_giga_62 --- */

    /* |t1+t2| */
    double sR = t1R + t2R, sI = t1I + t2I;
    double absSum = sqrt(sR * sR + sI * sI);

    /* cf[0:5] = |t1+t2|^(i+1) */
    double cf0[5];
    double pw = absSum;
    for (int i = 0; i < 5; i++) { cf0[i] = pw; pw *= absSum; }

    /* (t1 + 2j*t2)^3:  2j*t2 = -2*t2I + 2i*t2R */
    double wR = t1R - 2.0 * t2I, wI = t1I + 2.0 * t2R;
    double w2R = wR * wR - wI * wI, w2I = 2.0 * wR * wI;
    double w3R = w2R * wR - w2I * wI;
    /* Re((t1+2j*t2)^3) = w3R */

    /* |t1*t2| */
    double pR = t1R * t2R - t1I * t2I, pI = t1R * t2I + t1I * t2R;
    double absProd = sqrt(pR * pR + pI * pI);

    /* log(|conj(t1*t2)|) = log(|t1*t2|) */
    double logAbsP = (absProd > 1e-300) ? log(absProd) : -690.0;

    /* cf[5:10] = Re((t1+2j*t2)^3) * log(|t1*t2|) */
    double val5 = w3R * logAbsP;

    /* (t1-t2)^2 */
    double dR = t1R - t2R, dI = t1I - t2I;
    double diff2I = 2.0 * dR * dI;  /* Im((t1-t2)^2) */

    /* angle(t1*t2) */
    double angleP = atan2(pI, pR);

    /* cf[10:15] = Im((t1-t2)^2) / angle(t1*t2) */
    double val10 = (fabs(angleP) > 1e-15) ? (diff2I / angleP) : 0.0;

    /* cf[15:20] = sqrt(|cf[5:10]|) + angle(cf[0:5])
     * cf[0:5] are positive reals → angle = 0 */
    double val15 = sqrt(fabs(val5));

    /* cf[20:25] = |t1*t2|^(i+1) */
    double cf20[5];
    pw = absProd;
    for (int i = 0; i < 5; i++) { cf20[i] = pw; pw *= absProd; }

    /* rev: C[k] = cf[24-k] */
    /* C[0..4]   = cf[24..20] = |t1*t2|^5..1 */
    for (int i = 0; i < 5; i++) cRe[i] = cf20[4 - i];
    /* C[5..9]   = cf[19..15] = val15 */
    for (int i = 5; i < 10; i++) cRe[i] = val15;
    /* C[10..14] = cf[14..10] = val10 */
    for (int i = 10; i < 15; i++) cRe[i] = val10;
    /* C[15..19] = cf[9..5]   = val5 */
    for (int i = 15; i < 20; i++) cRe[i] = val5;
    /* C[20..24] = cf[4..0]   = |t1+t2|^5..1 */
    for (int i = 0; i < 5; i++) cRe[20 + i] = cf0[4 - i];
}

/* rev_giga_227: reversed coefficients of giga_227. */
static void rev_giga_227(double x1, double x2,
                         double *cRe, double *cIm, int *nCoeffs)
{
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS];
    int n;
    giga_227(x1, x2, tmpRe, tmpIm, &n);
    *nCoeffs = n;
    for (int k = 0; k < n; k++) {
        cRe[k] = tmpRe[n - 1 - k];
        cIm[k] = tmpIm[n - 1 - k];
    }
}

/*
 * poly_110: degree-70 polynomial with 71 coefficients.
 * Pipeline: t1=x1, t2=x2 → unit_circle → coeff5 → poly_110.
 *
 * unit_circle: t1 = exp(i*2π*x1), t2 = exp(i*2π*x2)
 * coeff5:      t1_new = t1 + 1/t2,  t2_new = t2 + 1/t1
 *              (on unit circle: 1/z = conj(z))
 *
 * poly_110(t1, t2):
 *   cf = zeros(71, complex)
 *   primes = [2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59]
 *   for k in 1..35:
 *     cf[k-1]  = Re(t1)*primes[k%17]    + Im(t2)*k²
 *     cf[70-k] = Re(t2)*primes[(70-k)%17] - Im(t1)*k²
 *   cf[35] = 440 * (cos(angle(t1)) + i*sin(angle(t2)))
 *   cf[70] = 0 always
 *
 * No reversal: cf[0] = leading coeff (cf[70]=0 would be zero leading if reversed).
 */
static void poly_110(double x1, double x2,
                     double *cRe, double *cIm, int *nCoeffs)
{
    *nCoeffs = 71;
    memset(cRe, 0, 71 * sizeof(double));
    memset(cIm, 0, 71 * sizeof(double));

    /* Unit circle transform */
    double a1 = 2.0 * M_PI * x1, a2 = 2.0 * M_PI * x2;
    double t1R = cos(a1), t1I = sin(a1);
    double t2R = cos(a2), t2I = sin(a2);

    /* coeff5: t1_new = t1 + 1/t2, t2_new = t2 + 1/t1
     * On unit circle: 1/z = conj(z) = (Re(z), -Im(z)) */
    double nt1R = t1R + t2R, nt1I = t1I - t2I;
    double nt2R = t2R + t1R, nt2I = t2I - t1I;
    t1R = nt1R; t1I = nt1I;
    t2R = nt2R; t2I = nt2I;

    /* Prime sequence (17 primes) */
    static const int P[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
                            31, 37, 41, 43, 47, 53, 59};

    /* Fill cf[0..34] and cf[36..69] symmetrically */
    for (int k = 1; k <= 35; k++) {
        double k2 = (double)(k * k);
        /* cf[k-1] = Re(t1) * prime[k%17] + Im(t2) * k²  (real) */
        cRe[k - 1] = t1R * P[k % 17] + t2I * k2;
        /* cf[70-k] = Re(t2) * prime[(70-k)%17] - Im(t1) * k²  (real) */
        cRe[70 - k] = t2R * P[(70 - k) % 17] - t1I * k2;
    }

    /* cf[35] = sum(primes) * (cos(angle(t1)) + i*sin(angle(t2)))
     * sum = 2+3+5+7+11+13+17+19+23+29+31+37+41+43+47+53+59 = 440 */
    double angle_t1 = atan2(t1I, t1R);
    double angle_t2 = atan2(t2I, t2R);
    cRe[35] = 440.0 * cos(angle_t1);
    cIm[35] = 440.0 * sin(angle_t2);
    /* cf[70] = 0: z=0 is always a root */
}

/* rev_poly_110: reversed coefficients of poly_110. */
static void rev_poly_110(double x1, double x2,
                         double *cRe, double *cIm, int *nCoeffs)
{
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS];
    int n;
    poly_110(x1, x2, tmpRe, tmpIm, &n);
    *nCoeffs = n;
    for (int k = 0; k < n; k++) {
        cRe[k] = tmpRe[n - 1 - k];
        cIm[k] = tmpIm[n - 1 - k];
    }
}

/*
 * giga_19: degree-89 polynomial with 90 coefficients.
 * t1 = x1, t2 = x2 (raw 0-to-1, NO unit circle transform).
 * R definition (1-based, NO rev):
 *   cf[1] = t1 - t2
 *   for k in 2:90:
 *     v = sin(k * cf[k-1]) + cos(k * t1)         (complex sin, real cos)
 *     av = |v|
 *     if finite(av) && av > 1e-10: cf[k] = 1i * v / av
 *     else:                        cf[k] = t1 + t2
 *
 * No rev() — R returns ascending order (constant first), but C solver
 * expects leading-first, so we reverse after iterative computation.
 */
static void giga_19(double x1, double x2,
                    double *cRe, double *cIm, int *nCoeffs)
{
    *nCoeffs = 90;

    /* Compute in temp arrays (R ascending order: index 0 = constant) */
    double tmpRe[90], tmpIm[90];
    memset(tmpRe, 0, 90 * sizeof(double));
    memset(tmpIm, 0, 90 * sizeof(double));

    /* t1, t2 are real, 0 to 1 — no exp(2πi·x) */
    double t1 = x1, t2 = x2;

    /* cf[1] = t1 - t2  →  tmp[0] (constant term) */
    tmpRe[0] = t1 - t2;
    tmpIm[0] = 0.0;

    for (int k = 2; k <= 90; k++) {
        int ci = k - 1;    /* temp index */
        int prev = ci - 1; /* previous coefficient */

        /* z = k * cf[k-1]  (real × complex) */
        double zR = (double)k * tmpRe[prev];
        double zI = (double)k * tmpIm[prev];

        /* Complex sin: sin(a+bi) = sin(a)cosh(b) + i·cos(a)sinh(b) */
        double sinzR = sin(zR) * cosh(zI);
        double sinzI = cos(zR) * sinh(zI);

        /* cos(k * t1) is real since t1 is real */
        double coskt1 = cos((double)k * t1);

        /* v = sin(z) + cos(k*t1) */
        double vR = sinzR + coskt1;
        double vI = sinzI;

        /* av = |v| */
        double av = sqrt(vR * vR + vI * vI);

        if (isfinite(av) && av > 1e-10) {
            /* cf[k] = 1i * v / av
             * v/av = (vR/av, vI/av)
             * 1i * (a, b) = (-b, a) */
            double nR = vR / av;
            double nI = vI / av;
            tmpRe[ci] = -nI;
            tmpIm[ci] = nR;
        } else {
            /* fallback: cf[k] = t1 + t2 (real) */
            tmpRe[ci] = t1 + t2;
            tmpIm[ci] = 0.0;
        }
    }

    /* Reverse: R ascending → C leading-first */
    for (int k = 0; k < 90; k++) {
        cRe[k] = tmpRe[89 - k];
        cIm[k] = tmpIm[89 - k];
    }
}

/* rev_giga_19: reversed coefficients of giga_19. */
static void rev_giga_19(double x1, double x2,
                        double *cRe, double *cIm, int *nCoeffs)
{
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS];
    int n;
    giga_19(x1, x2, tmpRe, tmpIm, &n);
    *nCoeffs = n;
    for (int k = 0; k < n; k++) {
        cRe[k] = tmpRe[n - 1 - k];
        cIm[k] = tmpIm[n - 1 - k];
    }
}

/* ---- Function dispatch ---- */

typedef void (*CoeffFunc)(double, double, double*, double*, int*);

static CoeffFunc lookupFunction(const char *name) {
    if (strcmp(name, "giga_1") == 0) return giga_1;
    if (strcmp(name, "rev_giga_1") == 0) return rev_giga_1;
    if (strcmp(name, "giga_5") == 0) return giga_5;
    if (strcmp(name, "rev_giga_5") == 0) return rev_giga_5;
    if (strcmp(name, "giga_42") == 0) return giga_42;
    if (strcmp(name, "rev_giga_42") == 0) return rev_giga_42;
    if (strcmp(name, "giga_43") == 0) return giga_43;
    if (strcmp(name, "rev_giga_43") == 0) return rev_giga_43;
    if (strcmp(name, "giga_87") == 0) return giga_87;
    if (strcmp(name, "rev_giga_87") == 0) return rev_giga_87;
    if (strcmp(name, "giga_19") == 0) return giga_19;
    if (strcmp(name, "rev_giga_19") == 0) return rev_giga_19;
    if (strcmp(name, "giga_30") == 0) return giga_30;
    if (strcmp(name, "rev_giga_30") == 0) return rev_giga_30;
    if (strcmp(name, "giga_39") == 0) return giga_39;
    if (strcmp(name, "rev_giga_39") == 0) return rev_giga_39;
    if (strcmp(name, "giga_40") == 0) return giga_40;
    if (strcmp(name, "rev_giga_40") == 0) return rev_giga_40;
    if (strcmp(name, "p7f") == 0) return p7f;
    if (strcmp(name, "rev_p7f") == 0) return rev_p7f;
    if (strcmp(name, "poly_110") == 0) return poly_110;
    if (strcmp(name, "rev_poly_110") == 0) return rev_poly_110;
    if (strcmp(name, "giga_227") == 0) return giga_227;
    if (strcmp(name, "rev_giga_227") == 0) return rev_giga_227;
    if (strcmp(name, "giga_230") == 0) return giga_230;
    if (strcmp(name, "rev_giga_230") == 0) return rev_giga_230;
    if (strcmp(name, "giga_232") == 0) return giga_232;
    if (strcmp(name, "rev_giga_232") == 0) return rev_giga_232;
    return NULL;
}

/* ---- Grid sweep (2D parameter scan) ---- */

static int runGrid(const char *buf, const char *outPath) {
    /* Parse function name */
    char funcName[64] = "";
    const char *cp = findKey(buf, "function");
    if (cp) parseString(cp, funcName, sizeof(funcName));

    /* Parse grid dimensions */
    int n1 = 100, n2 = 100;
    cp = findKey(buf, "n1");
    if (cp) n1 = (int)parseNum(&cp);
    cp = findKey(buf, "n2");
    if (cp) n2 = (int)parseNum(&cp);
    if (n1 < 1) n1 = 1;
    if (n2 < 1) n2 = 1;

    /* Optional stripe range: i1_start..i1_end (for parallel fan-out) */
    int i1_start = 0, i1_end = n1;
    cp = findKey(buf, "i1_start");
    if (cp) i1_start = (int)parseNum(&cp);
    cp = findKey(buf, "i1_end");
    if (cp) i1_end = (int)parseNum(&cp);
    if (i1_start < 0) i1_start = 0;
    if (i1_end > n1) i1_end = n1;
    if (i1_start >= i1_end) {
        fprintf(stderr, "Empty stripe: i1_start=%d >= i1_end=%d\n", i1_start, i1_end);
        return 1;
    }
    int stripeRows = i1_end - i1_start;

    if ((long)stripeRows * n2 > 10000000) {
        fprintf(stderr, "Stripe too large: %d x %d\n", stripeRows, n2);
        return 1;
    }

    int doMatch = 1;
    cp = findKey(buf, "match_roots");
    if (cp) doMatch = parseBool(cp);

    /* Look up coefficient function */
    CoeffFunc coeffFunc = lookupFunction(funcName);
    if (!coeffFunc) {
        fprintf(stderr, "Unknown function: %s\n", funcName);
        return 1;
    }

    /* Probe degree by evaluating at (0,0) */
    double coeffRe[MAX_COEFFS], coeffIm[MAX_COEFFS];
    int nCoeffs;
    coeffFunc(0.0, 0.0, coeffRe, coeffIm, &nCoeffs);
    int degree = nCoeffs - 1;

    /* Open output */
    FILE *fout = fopen(outPath, "wb");
    if (!fout) {
        fprintf(stderr, "Cannot open %s for writing\n", outPath);
        return 1;
    }

    float *stepBuf = malloc(degree * 2 * sizeof(float));
    double rootRe[MAX_DEGREE], rootIm[MAX_DEGREE];
    double prevRe[MAX_DEGREE], prevIm[MAX_DEGREE];

    /* Initial guesses */
    for (int k = 0; k < degree; k++) {
        double ang = 2.0 * M_PI * k / degree + 0.3;
        double r = 1.0 + 0.1 * k / degree;
        rootRe[k] = r * cos(ang);
        rootIm[k] = r * sin(ang);
    }

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    long totalIters = 0;
    long totalSteps = (long)stripeRows * n2;

    for (int i1 = i1_start; i1 < i1_end; i1++) {
        double x1 = (double)i1 / (double)n1;

        for (int j = 0; j < n2; j++) {
            /* Serpentine: even rows go forward, odd rows go backward */
            int i2 = (i1 & 1) ? (n2 - 1 - j) : j;
            double x2 = (double)i2 / (double)n2;

            /* Evaluate coefficient function */
            coeffFunc(x1, x2, coeffRe, coeffIm, &nCoeffs);

            /* Strip leading zeros */
            int start = 0;
            while (start < nCoeffs - 1 &&
                   coeffRe[start] * coeffRe[start] + coeffIm[start] * coeffIm[start] < 1e-30)
                start++;
            int effN = nCoeffs - start;
            int effDeg = effN - 1;

            /* Solve */
            int iters;
            if (effDeg <= 0) {
                for (int i = 0; i < degree; i++) { rootRe[i] = 0; rootIm[i] = 0; }
                iters = 0;
            } else if (effDeg == 1) {
                double aR = coeffRe[start], aI = coeffIm[start];
                double bR = coeffRe[start+1], bI = coeffIm[start+1];
                double d = aR*aR + aI*aI;
                if (d > 1e-30) {
                    rootRe[0] = -(bR*aR + bI*aI) / d;
                    rootIm[0] = -(bI*aR - bR*aI) / d;
                }
                iters = 1;
            } else {
                iters = solveEA(coeffRe + start, coeffIm + start, effN,
                                rootRe, rootIm, effDeg);
            }
            totalIters += iters;

            /* Match roots */
            int stepIdx = (i1 - i1_start) * n2 + j;
            if (doMatch && stepIdx > 0 && effDeg > 1) {
                matchRoots(rootRe, rootIm, prevRe, prevIm, effDeg);
            }

            /* Save for warm-start */
            memcpy(prevRe, rootRe, degree * sizeof(double));
            memcpy(prevIm, rootIm, degree * sizeof(double));

            /* Pack and write */
            for (int i = 0; i < degree; i++) {
                stepBuf[i * 2]     = (float)rootRe[i];
                stepBuf[i * 2 + 1] = (float)rootIm[i];
            }
            fwrite(stepBuf, sizeof(float), degree * 2, fout);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    long elapsed_us = (t1.tv_sec - t0.tv_sec) * 1000000L +
                      (t1.tv_nsec - t0.tv_nsec) / 1000L;

    fclose(fout);
    free(stepBuf);

    long dataBytes = totalSteps * degree * 2 * sizeof(float);
    double avgIters = totalSteps > 0 ? (double)totalIters / totalSteps : 0;

    printf("{\"mode\":\"grid\",\"function\":\"%s\","
           "\"degree\":%d,\"n1\":%d,\"n2\":%d,"
           "\"i1_start\":%d,\"i1_end\":%d,"
           "\"n_t\":%ld,\"stride\":%d,\"matched\":%s,"
           "\"data_bytes\":%ld,\"elapsed_us\":%ld,"
           "\"avg_iterations\":%.2f}\n",
           funcName, degree, n1, n2,
           i1_start, i1_end,
           totalSteps, degree * 2, doMatch ? "true" : "false",
           dataBytes, elapsed_us, avgIters);

    return 0;
}



/* ---- Main ---- */

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: sweep <output.bin>\n");
        return 1;
    }
    const char *outPath = argv[1];

    /* Read stdin */
    char *buf = malloc(BUF_SIZE);
    if (!buf) { fprintf(stderr, "malloc failed\n"); return 1; }
    int len = 0, n;
    while ((n = fread(buf + len, 1, BUF_SIZE - len - 1, stdin)) > 0)
        len += n;
    buf[len] = '\0';

    /* Check for grid mode */
    {
        char mode[32] = "";
        const char *mp = findKey(buf, "mode");
        if (mp) parseString(mp, mode, sizeof(mode));
        if (strcmp(mode, "grid") == 0) {
            int rc = runGrid(buf, outPath);
            free(buf);
            return rc;
        }
    }

    /* Parse spec (animation sweep mode) */
    double baseRe[MAX_COEFFS], baseIm[MAX_COEFFS];
    int nCoeffs = 0;
    const char *cp = findKey(buf, "coefficients");
    if (cp) nCoeffs = parseCoefficients(cp, baseRe, baseIm);
    if (nCoeffs < 2) {
        fprintf(stderr, "Need at least 2 coefficients\n");
        return 1;
    }
    int degree = nCoeffs - 1;

    Anim anims[MAX_ANIM];
    int nAnims = 0;
    cp = findKey(buf, "animations");
    if (cp) nAnims = parseAnimations(cp, anims);

    int n_t = 1000;
    cp = findKey(buf, "n_t");
    if (cp) n_t = (int)parseNum(&cp);
    if (n_t < 1) n_t = 1;
    if (n_t > 10000000) n_t = 10000000;

    int doMatch = 1;
    cp = findKey(buf, "match_roots");
    if (cp) doMatch = parseBool(cp);

    /* Set animation centers from base coefficients */
    for (int a = 0; a < nAnims; a++) {
        int idx = anims[a].coeff_index;
        if (idx >= 0 && idx < nCoeffs) {
            anims[a].centerRe = baseRe[idx];
            anims[a].centerIm = baseIm[idx];
        }
    }

    /* Open output file */
    FILE *fout = fopen(outPath, "wb");
    if (!fout) {
        fprintf(stderr, "Cannot open %s for writing\n", outPath);
        return 1;
    }

    /* Allocate output buffer for one step */
    float *stepBuf = malloc(degree * 2 * sizeof(float));

    /* Root arrays */
    double rootRe[MAX_DEGREE], rootIm[MAX_DEGREE];
    double prevRe[MAX_DEGREE], prevIm[MAX_DEGREE];
    double coeffRe[MAX_COEFFS], coeffIm[MAX_COEFFS];

    /* Initial guesses */
    for (int k = 0; k < degree; k++) {
        double ang = 2.0 * M_PI * k / degree + 0.3;
        double r = 1.0 + 0.1 * k / degree;
        rootRe[k] = r * cos(ang);
        rootIm[k] = r * sin(ang);
    }

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    long totalIters = 0;

    for (int step = 0; step < n_t; step++) {
        double t = (double)step / (double)n_t;  /* t in [0, 1) */

        /* Start with base coefficients */
        memcpy(coeffRe, baseRe, nCoeffs * sizeof(double));
        memcpy(coeffIm, baseIm, nCoeffs * sizeof(double));

        /* Apply animations: circle path */
        for (int a = 0; a < nAnims; a++) {
            int idx = anims[a].coeff_index;
            if (idx < 0 || idx >= nCoeffs) continue;

            double dir = anims[a].ccw ? -1.0 : 1.0;
            double phase = 2.0 * M_PI * (t * anims[a].speed * dir + anims[a].angle);
            coeffRe[idx] = anims[a].centerRe + anims[a].radius * cos(phase);
            coeffIm[idx] = anims[a].centerIm + anims[a].radius * sin(phase);
        }

        /* Strip leading zeros */
        int start = 0;
        while (start < nCoeffs - 1 &&
               coeffRe[start] * coeffRe[start] + coeffIm[start] * coeffIm[start] < 1e-30)
            start++;
        int effN = nCoeffs - start;
        int effDeg = effN - 1;

        /* Solve */
        int iters;
        if (effDeg <= 0) {
            /* Degenerate: zero roots */
            for (int i = 0; i < degree; i++) { rootRe[i] = 0; rootIm[i] = 0; }
            iters = 0;
        } else if (effDeg == 1) {
            /* Linear */
            double aR = coeffRe[start], aI = coeffIm[start];
            double bR = coeffRe[start+1], bI = coeffIm[start+1];
            double d = aR*aR + aI*aI;
            if (d > 1e-30) {
                rootRe[0] = -(bR*aR + bI*aI) / d;
                rootIm[0] = -(bI*aR - bR*aI) / d;
            }
            iters = 1;
        } else {
            iters = solveEA(coeffRe + start, coeffIm + start, effN,
                            rootRe, rootIm, effDeg);
        }
        totalIters += iters;

        /* Match roots to previous step */
        if (doMatch && step > 0 && effDeg > 1) {
            matchRoots(rootRe, rootIm, prevRe, prevIm, effDeg);
        }

        /* Save for next step's warm-start and matching */
        memcpy(prevRe, rootRe, degree * sizeof(double));
        memcpy(prevIm, rootIm, degree * sizeof(double));

        /* Pack as f32 and write */
        for (int i = 0; i < degree; i++) {
            stepBuf[i * 2]     = (float)rootRe[i];
            stepBuf[i * 2 + 1] = (float)rootIm[i];
        }
        fwrite(stepBuf, sizeof(float), degree * 2, fout);
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    long elapsed_us = (t1.tv_sec - t0.tv_sec) * 1000000L +
                      (t1.tv_nsec - t0.tv_nsec) / 1000L;

    fclose(fout);
    free(stepBuf);
    free(buf);

    /* Output metadata to stdout */
    long dataBytes = (long)n_t * degree * 2 * sizeof(float);
    double avgIters = n_t > 0 ? (double)totalIters / n_t : 0;

    printf("{\"degree\":%d,\"n_t\":%d,\"stride\":%d,"
           "\"matched\":%s,\"data_bytes\":%ld,"
           "\"elapsed_us\":%ld,\"avg_iterations\":%.2f}\n",
           degree, n_t, degree * 2,
           doMatch ? "true" : "false",
           dataBytes, elapsed_us, avgIters);

    return 0;
}
