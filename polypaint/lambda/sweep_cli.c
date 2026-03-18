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
#include <stdint.h>
#include <unistd.h>

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

/* ---- Parse a JSON array of strings (e.g. ["unit_circle","rev"]) ---- */

#define MAX_CHAIN 16

static int parseStringArray(const char *p, char names[][64], int maxCount) {
    p = skip(p);
    if (*p != '[') return 0;
    p++;
    int count = 0;
    while (count < maxCount) {
        p = skip(p);
        if (*p == ']') break;
        if (*p == ',') { p++; p = skip(p); }
        if (*p != '"') break;
        p++;
        int i = 0;
        while (*p && *p != '"' && i < 63) names[count][i++] = *p++;
        names[count][i] = '\0';
        if (*p == '"') p++;
        count++;
    }
    return count;
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

/* ==== Parameter transforms (composable pipeline) ==== */

typedef void (*ParamTransform)(double *z1r, double *z1i, double *z2r, double *z2i);

static void pt_none(double *z1r, double *z1i, double *z2r, double *z2i) {
    (void)z1r; (void)z1i; (void)z2r; (void)z2i;
}

static void pt_unit_circle(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a1 = 2.0 * M_PI * (*z1r), a2 = 2.0 * M_PI * (*z2r);
    *z1r = cos(a1); *z1i = sin(a1);
    *z2r = cos(a2); *z2i = sin(a2);
}

static void pt_square(double *z1r, double *z1i, double *z2r, double *z2i) {
    double r, i;
    r = (*z1r)*(*z1r) - (*z1i)*(*z1i); i = 2.0*(*z1r)*(*z1i);
    *z1r = r; *z1i = i;
    r = (*z2r)*(*z2r) - (*z2i)*(*z2i); i = 2.0*(*z2r)*(*z2i);
    *z2r = r; *z2i = i;
}

static void pt_cube(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a, b, r, i;
    a = *z1r; b = *z1i;
    r = a*a*a - 3.0*a*b*b; i = 3.0*a*a*b - b*b*b;
    *z1r = r; *z1i = i;
    a = *z2r; b = *z2i;
    r = a*a*a - 3.0*a*b*b; i = 3.0*a*a*b - b*b*b;
    *z2r = r; *z2i = i;
}

static void pt_reciprocal(double *z1r, double *z1i, double *z2r, double *z2i) {
    double d;
    d = (*z1r)*(*z1r) + (*z1i)*(*z1i);
    if (d > 1e-30) { *z1r = (*z1r)/d; *z1i = -(*z1i)/d; }
    else { *z1r = 0; *z1i = 0; }
    d = (*z2r)*(*z2r) + (*z2i)*(*z2i);
    if (d > 1e-30) { *z2r = (*z2r)/d; *z2i = -(*z2i)/d; }
    else { *z2r = 0; *z2i = 0; }
}

static void pt_conjugate(double *z1r, double *z1i, double *z2r, double *z2i) {
    *z1i = -(*z1i); *z2i = -(*z2i);
}

static void pt_swap(double *z1r, double *z1i, double *z2r, double *z2i) {
    double tr = *z1r, ti = *z1i;
    *z1r = *z2r; *z1i = *z2i;
    *z2r = tr; *z2i = ti;
}

static void pt_add_sub(double *z1r, double *z1i, double *z2r, double *z2i) {
    double ar = *z1r, ai = *z1i, br = *z2r, bi = *z2i;
    *z1r = ar + br; *z1i = ai + bi;
    *z2r = ar - br; *z2i = ai - bi;
}

static void pt_mul_div(double *z1r, double *z1i, double *z2r, double *z2i) {
    double ar = *z1r, ai = *z1i, br = *z2r, bi = *z2i;
    *z1r = ar*br - ai*bi; *z1i = ar*bi + ai*br;
    double d = br*br + bi*bi;
    if (d > 1e-30) { *z2r = (ar*br + ai*bi)/d; *z2i = (ai*br - ar*bi)/d; }
    else { *z2r = 0; *z2i = 0; }
}

static void pt_moebius(double *z1r, double *z1i, double *z2r, double *z2i) {
    double d;
    double ar = *z1r + 2.0, ai = *z1i;
    d = ar*ar + ai*ai;
    if (d > 1e-30) { *z1r = ar/d; *z1i = -ai/d; }
    else { *z1r = 0; *z1i = 0; }
    ar = *z2r + 2.0; ai = *z2i;
    d = ar*ar + ai*ai;
    if (d > 1e-30) { *z2r = ar/d; *z2i = -ai/d; }
    else { *z2r = 0; *z2i = 0; }
}

static void pt_shift1(double *z1r, double *z1i, double *z2r, double *z2i) {
    *z1r += 1.0; *z2r += 1.0; (void)z1i; (void)z2i;
}

static void pt_scale10(double *z1r, double *z1i, double *z2r, double *z2i) {
    *z1r *= 10.0; *z1i *= 10.0; *z2r *= 10.0; *z2i *= 10.0;
}

static void pt_negate(double *z1r, double *z1i, double *z2r, double *z2i) {
    *z1r = -(*z1r); *z1i = -(*z1i); *z2r = -(*z2r); *z2i = -(*z2i);
}
static void pt_zz(double *z1r, double *z1i, double *z2r, double *z2i) {
    /* (x1.real + i*x2.real, x2.real + i*x1.real) */
    double a = *z1r, b = *z2r;
    *z1r = a; *z1i = b; *z2r = b; *z2i = a;
}

static void pt_exp(double *z1r, double *z1i, double *z2r, double *z2i) {
    double e, r, i;
    e = exp(*z1r); r = e * cos(*z1i); i = e * sin(*z1i);
    *z1r = r; *z1i = i;
    e = exp(*z2r); r = e * cos(*z2i); i = e * sin(*z2i);
    *z2r = r; *z2i = i;
}

/* Complex arithmetic helpers for composable parameter transforms */
static inline void c_mul(double ar, double ai, double br, double bi, double *rr, double *ri) {
    *rr = ar*br - ai*bi;
    *ri = ar*bi + ai*br;
}
static inline void c_div(double ar, double ai, double br, double bi, double *rr, double *ri) {
    double d = br*br + bi*bi;
    if (d < 1e-30) { *rr = 0; *ri = 0; return; }
    *rr = (ar*br + ai*bi) / d;
    *ri = (ai*br - ar*bi) / d;
}
static inline void c_sin(double ar, double ai, double *rr, double *ri) {
    *rr = sin(ar) * cosh(ai);
    *ri = cos(ar) * sinh(ai);
}
static inline void c_cos(double ar, double ai, double *rr, double *ri) {
    *rr = cos(ar) * cosh(ai);
    *ri = -sin(ar) * sinh(ai);
}
static inline void c_log(double ar, double ai, double *rr, double *ri) {
    double m2 = ar*ar + ai*ai;
    *rr = (m2 > 0) ? 0.5 * log(m2) : -700.0;
    *ri = atan2(ai, ar);
}

/* Additional complex helpers for coefficient functions */
static inline double c_abs(double r, double i) { return sqrt(r*r + i*i); }
static inline double c_arg(double r, double i) { return atan2(i, r); }
static inline void c_exp2(double r, double i, double *rr, double *ri) {
    double e = exp(r); *rr = e * cos(i); *ri = e * sin(i);
}
static inline void c_powr(double r, double i, double p, double *rr, double *ri) {
    double m2 = r*r + i*i;
    if (m2 < 1e-60) { *rr = 0; *ri = 0; return; }
    double m = sqrt(m2), a = atan2(i, r), mp = pow(m, p);
    *rr = mp * cos(p * a); *ri = mp * sin(p * a);
}
static inline void c_powc(double ar, double ai, double br, double bi, double *rr, double *ri) {
    double lr, li, mr, mi;
    c_log(ar, ai, &lr, &li);
    c_mul(br, bi, lr, li, &mr, &mi);
    c_exp2(mr, mi, rr, ri);
}

/* coeff2: (t1+t2, t1*t2) */
static void pt_coeff2(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a1r=*z1r, a1i=*z1i, a2r=*z2r, a2i=*z2i;
    double pr, pi;
    c_mul(a1r, a1i, a2r, a2i, &pr, &pi);
    *z1r = a1r + a2r; *z1i = a1i + a2i;
    *z2r = pr; *z2i = pi;
}

/* coeff3: (1/(t1+2), 1/(t2+2)) */
static void pt_coeff3(double *z1r, double *z1i, double *z2r, double *z2i) {
    c_div(1.0, 0.0, *z1r + 2.0, *z1i, z1r, z1i);
    c_div(1.0, 0.0, *z2r + 2.0, *z2i, z2r, z2i);
}

/* coeff3a: (1/(t1+1), 1/(t2+1)) */
static void pt_coeff3a(double *z1r, double *z1i, double *z2r, double *z2i) {
    c_div(1.0, 0.0, *z1r + 1.0, *z1i, z1r, z1i);
    c_div(1.0, 0.0, *z2r + 1.0, *z2i, z2r, z2i);
}

/* coeff4: (cos(t1), sin(t2)) */
static void pt_coeff4(double *z1r, double *z1i, double *z2r, double *z2i) {
    double rr, ri;
    c_cos(*z1r, *z1i, &rr, &ri); *z1r = rr; *z1i = ri;
    c_sin(*z2r, *z2i, &rr, &ri); *z2r = rr; *z2i = ri;
}

/* coeff5: (t1 + 1/t2, t2 + 1/t1) */
static void pt_coeff5(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a1r=*z1r, a1i=*z1i, a2r=*z2r, a2i=*z2i;
    double rr, ri;
    c_div(1.0, 0.0, a2r, a2i, &rr, &ri);
    *z1r = a1r + rr; *z1i = a1i + ri;
    c_div(1.0, 0.0, a1r, a1i, &rr, &ri);
    *z2r = a2r + rr; *z2i = a2i + ri;
}

/* coeff5a: (t1 + 1/t1, t2 + 1/t2) */
static void pt_coeff5a(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a1r=*z1r, a1i=*z1i, a2r=*z2r, a2i=*z2i;
    double rr, ri;
    c_div(1.0, 0.0, a1r, a1i, &rr, &ri);
    *z1r = a1r + rr; *z1i = a1i + ri;
    c_div(1.0, 0.0, a2r, a2i, &rr, &ri);
    *z2r = a2r + rr; *z2i = a2i + ri;
}

/* coeff6: ((t1^3+i)/(t1^3-i), (t2^3+i)/(t2^3-i)) */
static void pt_coeff6(double *z1r, double *z1i, double *z2r, double *z2i) {
    double tr, ti, s2r, s2i, c3r, c3i;
    tr = *z1r; ti = *z1i;
    c_mul(tr, ti, tr, ti, &s2r, &s2i);
    c_mul(s2r, s2i, tr, ti, &c3r, &c3i);
    c_div(c3r, c3i + 1.0, c3r, c3i - 1.0, z1r, z1i);
    tr = *z2r; ti = *z2i;
    c_mul(tr, ti, tr, ti, &s2r, &s2i);
    c_mul(s2r, s2i, tr, ti, &c3r, &c3i);
    c_div(c3r, c3i + 1.0, c3r, c3i - 1.0, z2r, z2i);
}

/* coeff7: ((t+sin(t))/(t+cos(t))) for each */
static void pt_coeff7(double *z1r, double *z1i, double *z2r, double *z2i) {
    double sr, si, cr, ci, tr, ti;
    tr = *z1r; ti = *z1i;
    c_sin(tr, ti, &sr, &si);
    c_cos(tr, ti, &cr, &ci);
    c_div(tr + sr, ti + si, tr + cr, ti + ci, z1r, z1i);
    tr = *z2r; ti = *z2i;
    c_sin(tr, ti, &sr, &si);
    c_cos(tr, ti, &cr, &ci);
    c_div(tr + sr, ti + si, tr + cr, ti + ci, z2r, z2i);
}

/* coeff8: ((t1+sin(t2))/(t2+cos(t1)), (t2+sin(t1))/(t1+cos(t2))) */
static void pt_coeff8(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a1r=*z1r, a1i=*z1i, a2r=*z2r, a2i=*z2i;
    double s1r, s1i, s2r, s2i, c1r, c1i, c2r, c2i;
    c_sin(a1r, a1i, &s1r, &s1i);
    c_sin(a2r, a2i, &s2r, &s2i);
    c_cos(a1r, a1i, &c1r, &c1i);
    c_cos(a2r, a2i, &c2r, &c2i);
    c_div(a1r + s2r, a1i + s2i, a2r + c1r, a2i + c1i, z1r, z1i);
    c_div(a2r + s1r, a2i + s1i, a1r + c2r, a1i + c2i, z2r, z2i);
}

/* coeff9: ((t1^2+i*t2)/(t1^2-i*t2), (t2^2+i*t1)/(t2^2-i*t1)) */
static void pt_coeff9(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a1r=*z1r, a1i=*z1i, a2r=*z2r, a2i=*z2i;
    double sq1r, sq1i, sq2r, sq2i, it1r, it1i, it2r, it2i;
    c_mul(a1r, a1i, a1r, a1i, &sq1r, &sq1i);
    c_mul(a2r, a2i, a2r, a2i, &sq2r, &sq2i);
    it2r = -a2i; it2i = a2r;  /* i*t2 */
    it1r = -a1i; it1i = a1r;  /* i*t1 */
    c_div(sq1r + it2r, sq1i + it2i, sq1r - it2r, sq1i - it2i, z1r, z1i);
    c_div(sq2r + it1r, sq2i + it1i, sq2r - it1r, sq2i - it1i, z2r, z2i);
}

/* coeff10: ((t1^4-t2)/(t1^4+t2), (t2^4-t1)/(t2^4+t1)) */
static void pt_coeff10(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a1r=*z1r, a1i=*z1i, a2r=*z2r, a2i=*z2i;
    double s1r, s1i, s2r, s2i, q1r, q1i, q2r, q2i;
    c_mul(a1r, a1i, a1r, a1i, &s1r, &s1i);
    c_mul(s1r, s1i, s1r, s1i, &q1r, &q1i);
    c_mul(a2r, a2i, a2r, a2i, &s2r, &s2i);
    c_mul(s2r, s2i, s2r, s2i, &q2r, &q2i);
    c_div(q1r - a2r, q1i - a2i, q1r + a2r, q1i + a2i, z1r, z1i);
    c_div(q2r - a1r, q2i - a1i, q2r + a1r, q2i + a1i, z2r, z2i);
}

/* coeff11: (log(t1^4+2), log(t2^4+2)) */
static void pt_coeff11(double *z1r, double *z1i, double *z2r, double *z2i) {
    double sqr, sqi, qr, qi;
    c_mul(*z1r, *z1i, *z1r, *z1i, &sqr, &sqi);
    c_mul(sqr, sqi, sqr, sqi, &qr, &qi);
    c_log(qr + 2.0, qi, z1r, z1i);
    c_mul(*z2r, *z2i, *z2r, *z2i, &sqr, &sqi);
    c_mul(sqr, sqi, sqr, sqi, &qr, &qi);
    c_log(qr + 2.0, qi, z2r, z2i);
}

/* coeff12: (2*t1^4-3*t2^3+4*t1^2-5*t2, 2*t2^4-3*t1^3+4*t2^2-5*t1) */
static void pt_coeff12(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a1r=*z1r, a1i=*z1i, a2r=*z2r, a2i=*z2i;
    double s1r, s1i, s2r, s2i, c1r, c1i, c2r, c2i, q1r, q1i, q2r, q2i;
    c_mul(a1r, a1i, a1r, a1i, &s1r, &s1i);
    c_mul(s1r, s1i, a1r, a1i, &c1r, &c1i);
    c_mul(s1r, s1i, s1r, s1i, &q1r, &q1i);
    c_mul(a2r, a2i, a2r, a2i, &s2r, &s2i);
    c_mul(s2r, s2i, a2r, a2i, &c2r, &c2i);
    c_mul(s2r, s2i, s2r, s2i, &q2r, &q2i);
    *z1r = 2*q1r - 3*c2r + 4*s1r - 5*a2r;
    *z1i = 2*q1i - 3*c2i + 4*s1i - 5*a2i;
    *z2r = 2*q2r - 3*c1r + 4*s2r - 5*a1r;
    *z2i = 2*q2i - 3*c1i + 4*s2i - 5*a1i;
}

static ParamTransform lookupParamTransform(const char *name) {
    if (strcmp(name, "none") == 0)        return pt_none;
    if (strcmp(name, "unit_circle") == 0) return pt_unit_circle;
    if (strcmp(name, "square") == 0)      return pt_square;
    if (strcmp(name, "cube") == 0)        return pt_cube;
    if (strcmp(name, "reciprocal") == 0)  return pt_reciprocal;
    if (strcmp(name, "conjugate") == 0)   return pt_conjugate;
    if (strcmp(name, "swap") == 0)        return pt_swap;
    if (strcmp(name, "add_sub") == 0)     return pt_add_sub;
    if (strcmp(name, "mul_div") == 0)     return pt_mul_div;
    if (strcmp(name, "moebius") == 0)     return pt_moebius;
    if (strcmp(name, "shift1") == 0)      return pt_shift1;
    if (strcmp(name, "scale10") == 0)     return pt_scale10;
    if (strcmp(name, "negate") == 0)      return pt_negate;
    if (strcmp(name, "zz") == 0)          return pt_zz;
    if (strcmp(name, "exp") == 0)         return pt_exp;
    if (strcmp(name, "coeff2") == 0)     return pt_coeff2;
    if (strcmp(name, "coeff3") == 0)     return pt_coeff3;
    if (strcmp(name, "coeff3a") == 0)    return pt_coeff3a;
    if (strcmp(name, "coeff4") == 0)     return pt_coeff4;
    if (strcmp(name, "coeff5") == 0)     return pt_coeff5;
    if (strcmp(name, "coeff5a") == 0)    return pt_coeff5a;
    if (strcmp(name, "coeff6") == 0)     return pt_coeff6;
    if (strcmp(name, "coeff7") == 0)     return pt_coeff7;
    if (strcmp(name, "coeff8") == 0)     return pt_coeff8;
    if (strcmp(name, "coeff9") == 0)     return pt_coeff9;
    if (strcmp(name, "coeff10") == 0)    return pt_coeff10;
    if (strcmp(name, "coeff11") == 0)    return pt_coeff11;
    if (strcmp(name, "coeff12") == 0)    return pt_coeff12;
    return NULL;
}

/* ==== Coefficient transforms (composable pipeline) ==== */

typedef void (*CoeffTransform)(double *cRe, double *cIm, int *nCoeffs);

static void ct_none(double *cRe, double *cIm, int *nCoeffs) {
    (void)cRe; (void)cIm; (void)nCoeffs;
}

static void ct_rev(double *cRe, double *cIm, int *nCoeffs) {
    int n = *nCoeffs;
    for (int k = 0; k < n / 2; k++) {
        double tr = cRe[k]; cRe[k] = cRe[n-1-k]; cRe[n-1-k] = tr;
        double ti = cIm[k]; cIm[k] = cIm[n-1-k]; cIm[n-1-k] = ti;
    }
}

static void ct_conj(double *cRe, double *cIm, int *nCoeffs) {
    for (int k = 0; k < *nCoeffs; k++) cIm[k] = -cIm[k];
    (void)cRe;
}

static void ct_normalize(double *cRe, double *cIm, int *nCoeffs) {
    double d = cRe[0]*cRe[0] + cIm[0]*cIm[0];
    if (d < 1e-30) return;
    double invR = cRe[0] / d, invI = -cIm[0] / d;
    for (int k = 0; k < *nCoeffs; k++) {
        double r = cRe[k]*invR - cIm[k]*invI;
        double i = cRe[k]*invI + cIm[k]*invR;
        cRe[k] = r; cIm[k] = i;
    }
}

static void ct_deriv(double *cRe, double *cIm, int *nCoeffs) {
    int n = *nCoeffs;
    if (n <= 1) { *nCoeffs = 1; cRe[0] = 0; cIm[0] = 0; return; }
    /* Leading-first: c[0]*z^(n-1) + c[1]*z^(n-2) + ... + c[n-1]
       Derivative: (n-1)*c[0]*z^(n-2) + (n-2)*c[1]*z^(n-3) + ... + 1*c[n-2] */
    for (int k = 0; k < n - 1; k++) {
        double deg = (double)(n - 1 - k);
        cRe[k] = cRe[k] * deg;
        cIm[k] = cIm[k] * deg;
    }
    *nCoeffs = n - 1;
}

static void ct_scale100(double *cRe, double *cIm, int *nCoeffs) {
    for (int k = 0; k < *nCoeffs; k++) { cRe[k] *= 100.0; cIm[k] *= 100.0; }
}

static void ct_safe(double *cRe, double *cIm, int *nCoeffs) {
    for (int k = 0; k < *nCoeffs; k++) {
        if (!isfinite(cRe[k])) cRe[k] = 0;
        if (!isfinite(cIm[k])) cIm[k] = 0;
    }
}

static void ct_negate_odd(double *cRe, double *cIm, int *nCoeffs) {
    for (int k = 1; k < *nCoeffs; k += 2) { cRe[k] = -cRe[k]; cIm[k] = -cIm[k]; }
}

static void ct_max2one(double *cRe, double *cIm, int *nCoeffs) {
    int best = 0;
    double bestMag = cRe[0]*cRe[0] + cIm[0]*cIm[0];
    for (int k = 1; k < *nCoeffs; k++) {
        double m = cRe[k]*cRe[k] + cIm[k]*cIm[k];
        if (m > bestMag) { bestMag = m; best = k; }
    }
    cRe[best] = 1.0; cIm[best] = 0.0;
}

static CoeffTransform lookupCoeffTransform(const char *name) {
    if (strcmp(name, "none") == 0)        return ct_none;
    if (strcmp(name, "rev") == 0)         return ct_rev;
    if (strcmp(name, "conj") == 0)        return ct_conj;
    if (strcmp(name, "normalize") == 0)   return ct_normalize;
    if (strcmp(name, "deriv") == 0)       return ct_deriv;
    if (strcmp(name, "scale100") == 0)    return ct_scale100;
    if (strcmp(name, "safe") == 0)        return ct_safe;
    if (strcmp(name, "negate_odd") == 0)  return ct_negate_odd;
    if (strcmp(name, "max2one") == 0)    return ct_max2one;
    return NULL;
}

/* ==== Fast xorshift64 RNG for dithering ==== */

static uint64_t _rng_state = 0x123456789abcdef0ULL;
static inline uint64_t xorshift64(void) {
    uint64_t x = _rng_state;
    x ^= x << 13; x ^= x >> 7; x ^= x << 17;
    _rng_state = x;
    return x;
}
static inline double rng_uniform(void) {
    return (xorshift64() >> 11) * (1.0 / 9007199254740992.0);
}

/* sdith: square dither — adds uniform noise with width 1/(d*N) to z1r, z2r */
static void pt_sdith(double *z1r, double *z1i, double *z2r, double *z2i, double d, int gridN) {
    (void)z1i; (void)z2i;
    if (d <= 0.0) d = 1.0;
    double w = 1.0 / (d * gridN);
    *z1r += w * (rng_uniform() - 0.5);
    *z2r += w * (rng_uniform() - 0.5);
}

/* ==== Parameter transform dispatch (array-of-arrays format) ==== */

#define MAX_PT_ARGS 4

typedef struct {
    char name[64];
    double args[MAX_PT_ARGS];
    int nArgs;
} PtEntry;

/* Parse param_transforms: [["unit_circle"], ["sdith", "3"], ...] */
static int parsePtChain(const char *p, PtEntry *entries, int maxCount) {
    p = skip(p);
    if (*p != '[') return 0;
    p++; /* outer [ */
    int count = 0;
    while (count < maxCount) {
        p = skip(p);
        if (*p == ']') break;
        if (*p == ',') { p++; p = skip(p); }
        if (*p != '[') break;
        p++; /* inner [ */
        /* First element: the name (string) */
        p = skip(p);
        if (*p != '"') break;
        p++;
        int i = 0;
        while (*p && *p != '"' && i < 63) entries[count].name[i++] = *p++;
        entries[count].name[i] = '\0';
        if (*p == '"') p++;
        /* Remaining elements: numeric or string args */
        entries[count].nArgs = 0;
        while (entries[count].nArgs < MAX_PT_ARGS) {
            p = skip(p);
            if (*p == ']') break;
            if (*p == ',') { p++; p = skip(p); }
            if (*p == '"') {
                /* String arg — parse as double */
                p++;
                char tmp[64]; int j = 0;
                while (*p && *p != '"' && j < 63) tmp[j++] = *p++;
                tmp[j] = '\0';
                if (*p == '"') p++;
                entries[count].args[entries[count].nArgs++] = atof(tmp);
            } else if (*p == '-' || (*p >= '0' && *p <= '9')) {
                /* Bare number */
                entries[count].args[entries[count].nArgs++] = atof(p);
                while (*p && *p != ',' && *p != ']') p++;
            } else {
                break;
            }
        }
        p = skip(p);
        if (*p == ']') p++; /* close inner ] */
        count++;
    }
    return count;
}

/* Dispatch a single param transform entry */
static int dispatchPt(const PtEntry *e, double *z1r, double *z1i, double *z2r, double *z2i, int gridN) {
    if (strcmp(e->name, "sdith") == 0) {
        double d = e->nArgs > 0 ? e->args[0] : 1.0;
        pt_sdith(z1r, z1i, z2r, z2i, d, gridN);
        return 0;
    }
    /* radd(v): add v to real parts of both */
    if (strcmp(e->name, "radd") == 0) {
        double v = e->nArgs > 0 ? e->args[0] : 0.0;
        *z1r += v; *z2r += v;
        return 0;
    }
    /* iadd(v): add v to imaginary parts of both */
    if (strcmp(e->name, "iadd") == 0) {
        double v = e->nArgs > 0 ? e->args[0] : 0.0;
        *z1i += v; *z2i += v;
        return 0;
    }
    /* add(v): add v to both real and imaginary of both */
    if (strcmp(e->name, "add") == 0) {
        double v = e->nArgs > 0 ? e->args[0] : 0.0;
        *z1r += v; *z1i += v; *z2r += v; *z2i += v;
        return 0;
    }
    /* cadd(re, im): add complex (re+im*i) to both */
    if (strcmp(e->name, "cadd") == 0) {
        double re = e->nArgs > 0 ? e->args[0] : 0.0;
        double im = e->nArgs > 1 ? e->args[1] : 0.0;
        *z1r += re; *z1i += im; *z2r += re; *z2i += im;
        return 0;
    }
    /* rscale(v): multiply real parts by v */
    if (strcmp(e->name, "rscale") == 0) {
        double v = e->nArgs > 0 ? e->args[0] : 1.0;
        *z1r *= v; *z2r *= v;
        return 0;
    }
    /* iscale(v): multiply imaginary parts by v */
    if (strcmp(e->name, "iscale") == 0) {
        double v = e->nArgs > 0 ? e->args[0] : 1.0;
        *z1i *= v; *z2i *= v;
        return 0;
    }
    /* scale(v): multiply both (re,im) by v */
    if (strcmp(e->name, "scale") == 0) {
        double v = e->nArgs > 0 ? e->args[0] : 1.0;
        *z1r *= v; *z1i *= v; *z2r *= v; *z2i *= v;
        return 0;
    }
    /* rtheta: z1 = x1*exp(2*pi*x2*i), z2 = x2*exp(2*pi*x1*i) */
    if (strcmp(e->name, "rtheta") == 0) {
        double r1 = *z1r, r2 = *z2r;
        double a1 = 2.0 * M_PI * *z2r, a2 = 2.0 * M_PI * *z1r;
        *z1r = r1 * cos(a1); *z1i = r1 * sin(a1);
        *z2r = r2 * cos(a2); *z2i = r2 * sin(a2);
        return 0;
    }
    /* Fall back to standard param transforms (no extra args) */
    ParamTransform fn = lookupParamTransform(e->name);
    if (!fn) {
        fprintf(stderr, "Unknown param transform: %s\n", e->name);
        return 1;
    }
    fn(z1r, z1i, z2r, z2i);
    return 0;
}

/* ==== Wrapped coefficient functions (accept complex inputs) ==== */

typedef void (*CoeffFuncC)(double, double, double, double, double*, double*, int*);

#define WRAP_OLD(fname) \
    static void fname##_c(double x1r, double x1i, double x2r, double x2i, \
                          double *cRe, double *cIm, int *nCoeffs) { \
        (void)x1i; (void)x2i; \
        fname(x1r, x2r, cRe, cIm, nCoeffs); \
    }

WRAP_OLD(giga_1)
WRAP_OLD(giga_5)
WRAP_OLD(giga_19)
WRAP_OLD(giga_30)
WRAP_OLD(giga_39)
WRAP_OLD(giga_40)
WRAP_OLD(giga_42)
WRAP_OLD(giga_43)
WRAP_OLD(giga_87)
WRAP_OLD(giga_227)
WRAP_OLD(giga_230)
WRAP_OLD(giga_232)
WRAP_OLD(p7f)
WRAP_OLD(poly_110)

/* Hand-written poly functions: replace broken transpiled versions */
#include "poly_hand.h"

#include "poly_generated_funcs.h"

static CoeffFuncC lookupCoeffFuncC(const char *name) {
    if (strcmp(name, "giga_1") == 0)   return giga_1_c;
    if (strcmp(name, "giga_5") == 0)   return giga_5_c;
    if (strcmp(name, "giga_19") == 0)  return giga_19_c;
    if (strcmp(name, "giga_30") == 0)  return giga_30_c;
    if (strcmp(name, "giga_39") == 0)  return giga_39_c;
    if (strcmp(name, "giga_40") == 0)  return giga_40_c;
    if (strcmp(name, "giga_42") == 0)  return giga_42_c;
    if (strcmp(name, "giga_43") == 0)  return giga_43_c;
    if (strcmp(name, "giga_87") == 0)  return giga_87_c;
    if (strcmp(name, "giga_227") == 0) return giga_227_c;
    if (strcmp(name, "giga_230") == 0) return giga_230_c;
    if (strcmp(name, "giga_232") == 0) return giga_232_c;
    if (strcmp(name, "p7f") == 0)      return p7f_c;
    if (strcmp(name, "poly_110") == 0) return poly_110_c;
    if (strcmp(name, "poly_21") == 0)  return poly_21_hand;
    if (strcmp(name, "poly_29") == 0)  return poly_29_hand;
    if (strcmp(name, "poly_33") == 0)  return poly_33_hand;
    if (strcmp(name, "poly_35") == 0)  return poly_35_hand;
    if (strcmp(name, "poly_37") == 0)  return poly_37_hand;
    if (strcmp(name, "poly_40") == 0)  return poly_40_hand;
    if (strcmp(name, "poly_46") == 0)  return poly_46_hand;
    if (strcmp(name, "poly_55") == 0)  return poly_55_hand;
    if (strcmp(name, "poly_58") == 0)  return poly_58_hand;
    if (strcmp(name, "poly_72") == 0)  return poly_72_hand;
    if (strcmp(name, "poly_74") == 0)  return poly_74_hand;
    if (strcmp(name, "poly_94") == 0)  return poly_94_hand;
    if (strcmp(name, "poly_100") == 0) return poly_100_hand;
#include "poly_generated_lookups.h"
    return NULL;
}

/* ==== Coeffgen mode: generate coefficient vectors for the grid ==== */

static int runCoeffGen(const char *buf, const char *outPath) {
    char funcName[64] = "";
    const char *cp = findKey(buf, "function");
    if (cp) parseString(cp, funcName, sizeof(funcName));

    int n1 = 100, n2 = 100;
    cp = findKey(buf, "n1"); if (cp) n1 = (int)parseNum(&cp);
    cp = findKey(buf, "n2"); if (cp) n2 = (int)parseNum(&cp);
    if (n1 < 1) n1 = 1;
    if (n2 < 1) n2 = 1;

    int i1_start = 0, i1_end = n1;
    cp = findKey(buf, "i1_start"); if (cp) i1_start = (int)parseNum(&cp);
    cp = findKey(buf, "i1_end");   if (cp) i1_end = (int)parseNum(&cp);
    if (i1_start < 0) i1_start = 0;
    if (i1_end > n1) i1_end = n1;
    if (i1_start >= i1_end) {
        fprintf(stderr, "Empty stripe: i1_start=%d >= i1_end=%d\n", i1_start, i1_end);
        return 1;
    }

    /* Parse parameter transform chain (array-of-arrays format) */
    PtEntry ptEntries[MAX_CHAIN];
    int nPt = 0;
    cp = findKey(buf, "param_transforms");
    if (cp) nPt = parsePtChain(cp, ptEntries, MAX_CHAIN);

    /* Parse coefficient transform chain */
    char ctNames[MAX_CHAIN][64];
    int nCt = 0;
    cp = findKey(buf, "coeff_transforms");
    if (cp) nCt = parseStringArray(cp, ctNames, MAX_CHAIN);
    CoeffTransform ctChain[MAX_CHAIN];
    for (int t = 0; t < nCt; t++) {
        ctChain[t] = lookupCoeffTransform(ctNames[t]);
        if (!ctChain[t]) {
            fprintf(stderr, "Unknown coeff transform: %s\n", ctNames[t]);
            return 1;
        }
    }

    /* Look up coefficient function */
    CoeffFuncC coeffFunc = lookupCoeffFuncC(funcName);
    if (!coeffFunc) {
        fprintf(stderr, "Unknown function: %s\n", funcName);
        return 1;
    }

    /* Probe degree at (0,0) with transforms applied */
    double probeRe[MAX_COEFFS], probeIm[MAX_COEFFS];
    int probeN;
    {
        double z1r = 0, z1i = 0, z2r = 0, z2i = 0;
        for (int t = 0; t < nPt; t++) dispatchPt(&ptEntries[t], &z1r, &z1i, &z2r, &z2i, n1);
        coeffFunc(z1r, z1i, z2r, z2i, probeRe, probeIm, &probeN);
        for (int t = 0; t < nCt; t++) ctChain[t](probeRe, probeIm, &probeN);
    }
    int nCoeffsOut = probeN;
    int degree = nCoeffsOut - 1;

    FILE *fout = fopen(outPath, "wb");
    if (!fout) { fprintf(stderr, "Cannot open %s\n", outPath); return 1; }

    float *stepBuf = malloc(nCoeffsOut * 2 * sizeof(float));
    int stripeRows = i1_end - i1_start;
    /* Parse times (repeat count for dithering) */
    int times = 1;
    cp = findKey(buf, "times");
    if (cp) times = (int)parseNum(&cp);
    if (times < 1) times = 1;

    long totalSteps = (long)stripeRows * n2 * times;

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (int pass = 0; pass < times; pass++) {
    /* Re-seed RNG each pass so every (pass, stripe) combo gets unique dither */
    _rng_state = 0x123456789abcdef0ULL ^ ((uint64_t)pass * 2654435761ULL) ^ ((uint64_t)i1_start * 40503ULL);
    if (!_rng_state) _rng_state = 1;
    for (int i1 = i1_start; i1 < i1_end; i1++) {
        double x1 = (double)i1 / (double)n1;
        for (int j = 0; j < n2; j++) {
            int i2 = (i1 & 1) ? (n2 - 1 - j) : j;
            double x2 = (double)i2 / (double)n2;

            double z1r = x1, z1i = 0.0, z2r = x2, z2i = 0.0;
            for (int t = 0; t < nPt; t++) dispatchPt(&ptEntries[t], &z1r, &z1i, &z2r, &z2i, n1);

            double cRe[MAX_COEFFS], cIm[MAX_COEFFS];
            int nCoeffs;
            coeffFunc(z1r, z1i, z2r, z2i, cRe, cIm, &nCoeffs);
            for (int t = 0; t < nCt; t++) ctChain[t](cRe, cIm, &nCoeffs);

            /* Pad or truncate to nCoeffsOut */
            for (int k = nCoeffs; k < nCoeffsOut; k++) { cRe[k] = 0; cIm[k] = 0; }

            for (int k = 0; k < nCoeffsOut; k++) {
                stepBuf[k * 2]     = (float)cRe[k];
                stepBuf[k * 2 + 1] = (float)cIm[k];
            }
            fwrite(stepBuf, sizeof(float), nCoeffsOut * 2, fout);
        }
    } /* end i1 loop */
    } /* end pass loop */

    clock_gettime(CLOCK_MONOTONIC, &t1);
    long elapsed_us = (t1.tv_sec - t0.tv_sec) * 1000000L +
                      (t1.tv_nsec - t0.tv_nsec) / 1000L;
    fclose(fout);
    free(stepBuf);

    long dataBytes = totalSteps * nCoeffsOut * 2 * (long)sizeof(float);
    printf("{\"mode\":\"coeffgen\",\"function\":\"%s\","
           "\"n_coeffs\":%d,\"degree\":%d,"
           "\"n1\":%d,\"n2\":%d,"
           "\"i1_start\":%d,\"i1_end\":%d,"
           "\"n_t\":%ld,\"data_bytes\":%ld,"
           "\"elapsed_us\":%ld}\n",
           funcName, nCoeffsOut, degree,
           n1, n2, i1_start, i1_end,
           totalSteps, dataBytes, elapsed_us);
    return 0;
}

/* ==== Solve-from-coefficients mode ==== */

static int runSolveFromCoeffs(const char *buf, const char *outPath) {
    char coeffsFile[256] = "";
    const char *cp = findKey(buf, "coeffs_file");
    if (cp) parseString(cp, coeffsFile, sizeof(coeffsFile));
    if (!coeffsFile[0]) {
        fprintf(stderr, "Missing coeffs_file\n");
        return 1;
    }

    int nCoeffs = 0;
    cp = findKey(buf, "n_coeffs"); if (cp) nCoeffs = (int)parseNum(&cp);
    if (nCoeffs < 2 || nCoeffs > MAX_COEFFS) {
        fprintf(stderr, "Invalid n_coeffs: %d\n", nCoeffs);
        return 1;
    }
    int degree = nCoeffs - 1;

    int n1 = 100, n2 = 100;
    cp = findKey(buf, "n1"); if (cp) n1 = (int)parseNum(&cp);
    cp = findKey(buf, "n2"); if (cp) n2 = (int)parseNum(&cp);

    int i1_start = 0, i1_end = n1;
    cp = findKey(buf, "i1_start"); if (cp) i1_start = (int)parseNum(&cp);
    cp = findKey(buf, "i1_end");   if (cp) i1_end = (int)parseNum(&cp);
    if (i1_start < 0) i1_start = 0;
    if (i1_end > n1) i1_end = n1;

    int doMatch = 1;
    cp = findKey(buf, "match_roots"); if (cp) doMatch = parseBool(cp);

    FILE *fin = fopen(coeffsFile, "rb");
    if (!fin) { fprintf(stderr, "Cannot open %s\n", coeffsFile); return 1; }

    FILE *fout = fopen(outPath, "wb");
    if (!fout) { fclose(fin); fprintf(stderr, "Cannot open %s\n", outPath); return 1; }

    float *coeffBuf = malloc(nCoeffs * 2 * sizeof(float));
    float *rootBuf = malloc(degree * 2 * sizeof(float));
    double rootRe[MAX_DEGREE], rootIm[MAX_DEGREE];
    double prevRe[MAX_DEGREE], prevIm[MAX_DEGREE];
    double coeffRe[MAX_COEFFS], coeffIm[MAX_COEFFS];

    for (int k = 0; k < degree; k++) {
        double ang = 2.0 * M_PI * k / degree + 0.3;
        double r = 1.0 + 0.1 * k / degree;
        rootRe[k] = r * cos(ang);
        rootIm[k] = r * sin(ang);
    }

    long totalSteps = 0;
    long totalIters = 0;

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    while (fread(coeffBuf, sizeof(float), nCoeffs * 2, fin) == (size_t)(nCoeffs * 2)) {
        totalSteps++;
        for (int k = 0; k < nCoeffs; k++) {
            coeffRe[k] = (double)coeffBuf[k * 2];
            coeffIm[k] = (double)coeffBuf[k * 2 + 1];
        }

        /* Strip leading zeros */
        int start = 0;
        while (start < nCoeffs - 1 &&
               coeffRe[start]*coeffRe[start] + coeffIm[start]*coeffIm[start] < 1e-30)
            start++;
        int effN = nCoeffs - start;

        /* Strip trailing zeros — trailing zeros mean z=0 is a root with that multiplicity.
         * Factor them out so the solver only works on the reduced polynomial. */
        int trailingZeros = 0;
        while (trailingZeros < effN - 1) {
            int k = start + effN - 1 - trailingZeros;
            if (coeffRe[k]*coeffRe[k] + coeffIm[k]*coeffIm[k] >= 1e-30) break;
            trailingZeros++;
        }
        effN -= trailingZeros;
        int effDeg = effN - 1;

        int iters;
        /* Set trailing-zero roots to 0 (z=0 with multiplicity trailingZeros) */
        for (int i = 0; i < degree; i++) { rootRe[i] = 0; rootIm[i] = 0; }

        if (effDeg <= 0) {
            iters = 0;
        } else if (effDeg == 1) {
            rootRe[0] = 0; rootIm[0] = 0;
            double aR = coeffRe[start], aI = coeffIm[start];
            double bR = coeffRe[start+1], bI = coeffIm[start+1];
            double d = aR*aR + aI*aI;
            if (d > 1e-30) {
                rootRe[0] = -(bR*aR + bI*aI) / d;
                rootIm[0] = -(bI*aR - bR*aI) / d;
            }
            iters = 1;
        } else {
            /* Re-init warm-start if previous roots are all zero (e.g. after a
             * degenerate step). EA needs non-degenerate starting points. */
            double warmMag = 0;
            for (int i = 0; i < effDeg; i++)
                warmMag += rootRe[i]*rootRe[i] + rootIm[i]*rootIm[i];
            if (warmMag < 1e-20) {
                for (int i = 0; i < effDeg; i++) {
                    double ang = 2.0 * M_PI * i / effDeg + 0.3;
                    double r = 1.0 + 0.1 * i / effDeg;
                    rootRe[i] = r * cos(ang);
                    rootIm[i] = r * sin(ang);
                }
            }
            iters = solveEA(coeffRe + start, coeffIm + start, effN,
                            rootRe, rootIm, effDeg);
        }
        totalIters += iters;

        if (doMatch && totalSteps > 1 && effDeg > 1) {
            matchRoots(rootRe, rootIm, prevRe, prevIm, effDeg);
        }
        memcpy(prevRe, rootRe, degree * sizeof(double));
        memcpy(prevIm, rootIm, degree * sizeof(double));

        for (int i = 0; i < degree; i++) {
            rootBuf[i * 2]     = (float)rootRe[i];
            rootBuf[i * 2 + 1] = (float)rootIm[i];
        }
        fwrite(rootBuf, sizeof(float), degree * 2, fout);
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    long elapsed_us = (t1.tv_sec - t0.tv_sec) * 1000000L +
                      (t1.tv_nsec - t0.tv_nsec) / 1000L;

    fclose(fin);
    fclose(fout);
    free(coeffBuf);
    free(rootBuf);

    long dataBytes = totalSteps * degree * 2 * (long)sizeof(float);
    double avgIters = totalSteps > 0 ? (double)totalIters / totalSteps : 0;

    printf("{\"mode\":\"solve\",\"degree\":%d,"
           "\"n1\":%d,\"n2\":%d,"
           "\"i1_start\":%d,\"i1_end\":%d,"
           "\"n_t\":%ld,\"stride\":%d,\"matched\":%s,"
           "\"data_bytes\":%ld,\"elapsed_us\":%ld,"
           "\"avg_iterations\":%.2f}\n",
           degree, n1, n2, i1_start, i1_end,
           totalSteps, degree * 2, doMatch ? "true" : "false",
           dataBytes, elapsed_us, avgIters);
    return 0;
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

            /* Strip trailing zeros (z=0 roots) */
            int trailingZeros = 0;
            while (trailingZeros < effN - 1) {
                int k = start + effN - 1 - trailingZeros;
                if (coeffRe[k]*coeffRe[k] + coeffIm[k]*coeffIm[k] >= 1e-30) break;
                trailingZeros++;
            }
            effN -= trailingZeros;
            int effDeg = effN - 1;

            /* Solve */
            int iters;
            for (int i = 0; i < degree; i++) { rootRe[i] = 0; rootIm[i] = 0; }

            if (effDeg <= 0) {
                iters = 0;
            } else if (effDeg == 1) {
                rootRe[0] = 0; rootIm[0] = 0;
                double aR = coeffRe[start], aI = coeffIm[start];
                double bR = coeffRe[start+1], bI = coeffIm[start+1];
                double d = aR*aR + aI*aI;
                if (d > 1e-30) {
                    rootRe[0] = -(bR*aR + bI*aI) / d;
                    rootIm[0] = -(bI*aR - bR*aI) / d;
                }
                iters = 1;
            } else {
                double warmMag = 0;
                for (int ii = 0; ii < effDeg; ii++)
                    warmMag += rootRe[ii]*rootRe[ii] + rootIm[ii]*rootIm[ii];
                if (warmMag < 1e-20) {
                    for (int ii = 0; ii < effDeg; ii++) {
                        double ang = 2.0 * M_PI * ii / effDeg + 0.3;
                        double r = 1.0 + 0.1 * ii / effDeg;
                        rootRe[ii] = r * cos(ang);
                        rootIm[ii] = r * sin(ang);
                    }
                }
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

    /* Seed xorshift RNG */
    _rng_state = (uint64_t)time(NULL) ^ ((uint64_t)getpid() << 32);

    /* Read stdin */
    char *buf = malloc(BUF_SIZE);
    if (!buf) { fprintf(stderr, "malloc failed\n"); return 1; }
    int len = 0, n;
    while ((n = fread(buf + len, 1, BUF_SIZE - len - 1, stdin)) > 0)
        len += n;
    buf[len] = '\0';

    /* Dispatch on mode */
    {
        char mode[32] = "";
        const char *mp = findKey(buf, "mode");
        if (mp) parseString(mp, mode, sizeof(mode));
        if (strcmp(mode, "grid") == 0) {
            int rc = runGrid(buf, outPath);
            free(buf);
            return rc;
        }
        if (strcmp(mode, "coeffgen") == 0) {
            int rc = runCoeffGen(buf, outPath);
            free(buf);
            return rc;
        }
        if (strcmp(mode, "solve") == 0) {
            int rc = runSolveFromCoeffs(buf, outPath);
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

        /* Strip trailing zeros (z=0 roots) */
        int trailingZeros = 0;
        while (trailingZeros < effN - 1) {
            int k = start + effN - 1 - trailingZeros;
            if (coeffRe[k]*coeffRe[k] + coeffIm[k]*coeffIm[k] >= 1e-30) break;
            trailingZeros++;
        }
        effN -= trailingZeros;
        int effDeg = effN - 1;

        /* Solve */
        int iters;
        for (int i = 0; i < degree; i++) { rootRe[i] = 0; rootIm[i] = 0; }

        if (effDeg <= 0) {
            iters = 0;
        } else if (effDeg == 1) {
            /* Linear */
            rootRe[0] = 0; rootIm[0] = 0;
            double aR = coeffRe[start], aI = coeffIm[start];
            double bR = coeffRe[start+1], bI = coeffIm[start+1];
            double d = aR*aR + aI*aI;
            if (d > 1e-30) {
                rootRe[0] = -(bR*aR + bI*aI) / d;
                rootIm[0] = -(bI*aR - bR*aI) / d;
            }
            iters = 1;
        } else {
            double warmMag = 0;
            for (int i = 0; i < effDeg; i++)
                warmMag += rootRe[i]*rootRe[i] + rootIm[i]*rootIm[i];
            if (warmMag < 1e-20) {
                for (int i = 0; i < effDeg; i++) {
                    double ang = 2.0 * M_PI * i / effDeg + 0.3;
                    double r = 1.0 + 0.1 * i / effDeg;
                    rootRe[i] = r * cos(ang);
                    rootIm[i] = r * sin(ang);
                }
            }
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
