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
 * No rev() — R index k maps to C index k-1.
 */
static void giga_19(double x1, double x2,
                    double *cRe, double *cIm, int *nCoeffs)
{
    *nCoeffs = 90;
    memset(cRe, 0, 90 * sizeof(double));
    memset(cIm, 0, 90 * sizeof(double));

    /* t1, t2 are real, 0 to 1 — no exp(2πi·x) */
    double t1 = x1, t2 = x2;

    /* cf[1] = t1 - t2  →  C[0] */
    cRe[0] = t1 - t2;
    cIm[0] = 0.0;

    for (int k = 2; k <= 90; k++) {
        int ci = k - 1;    /* C 0-based index */
        int prev = ci - 1; /* previous coefficient */

        /* z = k * cf[k-1]  (real × complex) */
        double zR = (double)k * cRe[prev];
        double zI = (double)k * cIm[prev];

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
            cRe[ci] = -nI;
            cIm[ci] = nR;
        } else {
            /* fallback: cf[k] = t1 + t2 (real) */
            cRe[ci] = t1 + t2;
            cIm[ci] = 0.0;
        }
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
