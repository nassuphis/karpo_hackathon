/*
 * Sweep solver: evaluate a polynomial family across n_t parameter steps.
 * Ehrlich-Aberth with warm-start + greedy root matching for trajectory continuity.
 *
 * Reads JSON sweep spec from stdin.
 * Writes packed f32 binary (root positions) to a file path given as argv[1].
 * Writes metadata JSON to stdout.
 *
 * Build: aarch64-linux-musl-gcc -O3 -static -pthread -o sweep sweep_cli.c -lm
 * Local: cc -O3 -pthread -o sweep sweep_cli.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "companion_solver.h"

#define MAX_DEGREE 255
#define MAX_COEFFS 256
#define MAX_ANIM 64
#define MAX_ITER 64
#define TOL2 1e-16
#define BUF_SIZE (1024 * 256)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#define PP_THREAD_LOCAL _Thread_local
#else
#define PP_THREAD_LOCAL __thread
#endif

/* ---- qsort comparator for doubles ---- */
static int cmpDouble(const void *a, const void *b) {
    double da = *(const double *)a, db = *(const double *)b;
    return (da > db) - (da < db);
}

/* ---- Ehrlich-Aberth solver ---- */

static int solveEALimited(double *cr, double *ci, int n,
                          double *rRe, double *rIm, int degree,
                          int maxIter)
{
    if (maxIter < 1) maxIter = 1;
    if (maxIter > MAX_ITER) maxIter = MAX_ITER;
    for (int iter = 0; iter < maxIter; iter++) {
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
    return maxIter;
}

static int solveEA(double *cr, double *ci, int n,
                   double *rRe, double *rIm, int degree)
{
    return solveEALimited(cr, ci, n, rRe, rIm, degree, MAX_ITER);
}

static void seedEAInitialGuess(double *rootRe, double *rootIm, int degree) {
    for (int i = 0; i < degree; i++) {
        double ang = 2.0 * M_PI * i / degree + 0.3;
        double r = 1.0 + 0.1 * i / degree;
        rootRe[i] = r * cos(ang);
        rootIm[i] = r * sin(ang);
    }
}

static int warmStartNeedsReseed(const double *rootRe, const double *rootIm, int effDeg) {
    if (effDeg <= 0) return 0;

    double warmMag = 0;
    for (int i = 0; i < effDeg; i++) {
        double re = rootRe[i], im = rootIm[i];
        if (!isfinite(re) || !isfinite(im)) return 1;
        warmMag += re * re + im * im;
    }
    if (warmMag < 1e-20) return 1;

    for (int i = 0; i < effDeg; i++) {
        for (int j = i + 1; j < effDeg; j++) {
            double dR = rootRe[i] - rootRe[j];
            double dI = rootIm[i] - rootIm[j];
            if (dR * dR + dI * dI < 1e-18) return 1;
        }
    }
    return 0;
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

/* Parse a JSON array of numbers: [1.0, 2.0, ...] */
static int parseNumArray(const char *p, double *out, int maxCount) {
    p = skip(p);
    if (*p != '[') return 0;
    p++;
    int count = 0;
    while (*p && *p != ']' && count < maxCount) {
        p = skip(p);
        if (*p == ']') break;
        out[count++] = parseNum(&p);
        p = skip(p);
        if (*p == ',') p++;
    }
    return count;
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
#define MAX_CT_ARGS 8

typedef struct {
    char name[64];
    char args[MAX_CT_ARGS][64];
    int nArgs;
} CtEntry;

/* Match the persisted param_gen representation so monolithic coeffgen and
 * streamed coeffgen_chunked see the same transformed inputs. */
static inline void quantize_params_f32(double *z1r, double *z1i,
                                       double *z2r, double *z2i) {
    float q1r = (float)(*z1r), q1i = (float)(*z1i);
    float q2r = (float)(*z2r), q2i = (float)(*z2i);
    *z1r = (double)q1r; *z1i = (double)q1i;
    *z2r = (double)q2r; *z2i = (double)q2i;
}

static int coeff_transform_chain_needs_quantized_params(const CtEntry *entries, int count) {
    for (int i = 0; i < count; i++) {
        if (entries[i].nArgs > 0) return 1;
    }
    return 0;
}

static int ct_arg_int(const CtEntry *e, int idx, int fallback) {
    if (!e || idx < 0 || idx >= e->nArgs) return fallback;
    char *end = NULL;
    long v = strtol(e->args[idx], &end, 10);
    if (end == e->args[idx]) return fallback;
    return (int)v;
}

static int ct_parse_double_expr(const char *raw, double *out) {
    if (!raw || !out) return 0;
    const char *p = raw;
    double total = 0.0;
    int saw = 0;
    for (;;) {
        p = skip(p);
        if (!*p) break;
        char *end = NULL;
        double v = strtod(p, &end);
        if (end == p) return 0;
        total += v;
        if (!isfinite(total)) return 0;
        saw = 1;
        p = skip(end);
        if (!*p) break;
        if (*p != '+' && *p != '-') return 0;
    }
    if (!saw) return 0;
    *out = total;
    return 1;
}

static int ct_parse_complex_literal(const char *raw, double *outRe, double *outIm) {
    if (!raw || !outRe || !outIm) return 0;
    char buf[128];
    int n = 0;
    for (const char *p = raw; *p && n < (int)sizeof(buf) - 1; p++) {
        if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') continue;
        buf[n++] = (*p == 'i' || *p == 'I') ? 'j' : *p;
    }
    buf[n] = '\0';
    if (n <= 0) return 0;
    const char *p = buf;
    double rr = 0.0, ri = 0.0;
    int saw = 0;
    while (*p) {
        char *end = NULL;
        double value = strtod(p, &end);
        if (end != p) {
            if (*end == 'j') {
                ri += value;
                p = end + 1;
            } else {
                rr += value;
                p = end;
            }
            saw = 1;
        } else if (*p == 'j') {
            ri += 1.0;
            p++;
            saw = 1;
        } else if ((*p == '+' || *p == '-') && p[1] == 'j') {
            ri += (*p == '-') ? -1.0 : 1.0;
            p += 2;
            saw = 1;
        } else {
            return 0;
        }
        if (!isfinite(rr) || !isfinite(ri)) return 0;
        if (!*p) break;
        if (*p != '+' && *p != '-') return 0;
    }
    if (!saw) return 0;
    *outRe = rr;
    *outIm = ri;
    return 1;
}

static int ct_arg_has_complex_unit(const CtEntry *e, int idx) {
    if (!e || idx < 0 || idx >= e->nArgs) return 0;
    return strchr(e->args[idx], 'j') || strchr(e->args[idx], 'J') ||
           strchr(e->args[idx], 'i') || strchr(e->args[idx], 'I');
}

static double ct_arg_double(const CtEntry *e, int idx, double fallback) {
    if (!e || idx < 0 || idx >= e->nArgs) return fallback;
    double v = fallback;
    if (!ct_parse_double_expr(e->args[idx], &v)) return fallback;
    return v;
}

static int ct_arg_pad_lo(const CtEntry *e, int idx, int fallbackLo) {
    if (!e || idx < 0 || idx >= e->nArgs) return fallbackLo;
    const char *arg = e->args[idx];
    if (strcmp(arg, "lo") == 0) return 1;
    if (strcmp(arg, "hi") == 0) return 0;
    fprintf(stderr, "Invalid roots pad mode: %s (expected hi or lo)\n", arg);
    return -1;
}

static int ct_base_arg_count(const char *name) {
    if (strcmp(name, "scale100") == 0) return 4;
    if (strcmp(name, "power") == 0) return 1;
    if (strcmp(name, "invpower") == 0) return 1;
    if (strcmp(name, "exp") == 0) return 2;
    if (strcmp(name, "round") == 0) return 2;
    if (strcmp(name, "pow") == 0) return 4;
    if (strcmp(name, "roots_cm") == 0) return 1;
    if (strcmp(name, "roots") == 0) return 2;
    return 0;
}

static double ct_arg_andy(const CtEntry *e) {
    if (!e) return 0.0;
    int idx = ct_base_arg_count(e->name);
    if (idx < 0 || idx >= e->nArgs) return 0.0;
    return ct_arg_double(e, idx, 0.0);
}

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
static inline void c_mul(double ar, double ai, double br, double bi, double *rr, double *ri);
static inline void c_div(double ar, double ai, double br, double bi, double *rr, double *ri);

static void pt_none(double *z1r, double *z1i, double *z2r, double *z2i) {
    (void)z1r; (void)z1i; (void)z2r; (void)z2i;
}

/* unit_circle: z -> exp(i * 2*pi * z).  Full complex: exp(-2*pi*b) * exp(i*2*pi*a).
 * Backward-compatible: for real inputs (b=0), reduces to cos(2*pi*a) + i*sin(2*pi*a). */
static void pt_unit_circle(double *z1r, double *z1i, double *z2r, double *z2i) {
    double u1 = 2.0 * M_PI * (*z1r), s1 = exp(-2.0 * M_PI * (*z1i));
    double u2 = 2.0 * M_PI * (*z2r), s2 = exp(-2.0 * M_PI * (*z2i));
    *z1r = s1 * cos(u1); *z1i = s1 * sin(u1);
    *z2r = s2 * cos(u2); *z2i = s2 * sin(u2);
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

static void pt_apply_moebius(double *zr, double *zi,
                             double ar, double ai, double br, double bi,
                             double cr, double ci, double dr, double di) {
    double numr, numi, denr, deni;
    c_mul(ar, ai, *zr, *zi, &numr, &numi);
    numr += br;
    numi += bi;
    c_mul(cr, ci, *zr, *zi, &denr, &deni);
    denr += dr;
    deni += di;
    c_div(numr, numi, denr, deni, zr, zi);
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

static void pt_moebius_abcd(double *z1r, double *z1i, double *z2r, double *z2i,
                            double ar, double ai, double br, double bi,
                            double cr, double ci, double dr, double di) {
    pt_apply_moebius(z1r, z1i, ar, ai, br, bi, cr, ci, dr, di);
    pt_apply_moebius(z2r, z2i, ar, ai, br, bi, cr, ci, dr, di);
}

/* inv_t_plus_2(re1, im1, re2, im2):
 *   t1 = 1 / (t1 + re1 + i*im1)
 *   t2 = 1 / (t2 + re2 + i*im2)
 * Defaults (2,0,2,0) match the old fixed inv_t_plus_2 behavior. */
static void pt_inv_t_plus_2(double *z1r, double *z1i, double *z2r, double *z2i,
                            double re1, double im1, double re2, double im2) {
    double d;
    double ar = *z1r + re1, ai = *z1i + im1;
    d = ar * ar + ai * ai;
    if (d > 1e-30) { *z1r = ar / d; *z1i = -ai / d; }
    else { *z1r = 0; *z1i = 0; }

    ar = *z2r + re2; ai = *z2i + im2;
    d = ar * ar + ai * ai;
    if (d > 1e-30) { *z2r = ar / d; *z2i = -ai / d; }
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

/* zzold: t1' = t2' = t1 + i*t2.  Both outputs identical, from original inputs. */
static void pt_zzold(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a = *z1r, b = *z1i, c = *z2r, d = *z2i;
    double nr = a - d, ni = b + c;
    *z1r = nr; *z1i = ni; *z2r = nr; *z2i = ni;
}

/* zz1: t1' = t1 + i*t2,  t2' = t1*t2 + i*(t1 + t2).  Both from originals. */
static void pt_zz1(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a = *z1r, b = *z1i, c = *z2r, d = *z2i;
    /* t1' = t1 + i*t2 = (a - d) + i(b + c) */
    double n1r = a - d, n1i = b + c;
    /* t1*t2 = (ac - bd) + i(ad + bc) */
    double pr = a*c - b*d, pi = a*d + b*c;
    /* t1 + t2 = (a + c) + i(b + d) */
    double sr = a + c, si = b + d;
    /* i*(t1+t2) = -si + i*sr = -(b+d) + i(a+c) */
    /* t2' = t1*t2 + i*(t1+t2) = (pr - si) + i(pi + sr) */
    double n2r = pr - si, n2i = pi + sr;
    *z1r = n1r; *z1i = n1i; *z2r = n2r; *z2i = n2i;
}

/* zz2: t1' = t1 + i*t2,  t2' = t1 - i*t2.  Both from originals. */
static void pt_zz2(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a = *z1r, b = *z1i, c = *z2r, d = *z2i;
    /* t1' = t1 + i*t2 = (a-d) + i(b+c) */
    /* t2' = t1 - i*t2 = (a+d) + i(b-c) */
    *z1r = a - d; *z1i = b + c;
    *z2r = a + d; *z2i = b - c;
}

/* zz3: t1' = t1 + i*t2,  t2' = t2 + i*t1.  Both from originals. */
static void pt_zz3(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a = *z1r, b = *z1i, c = *z2r, d = *z2i;
    *z1r = a - d; *z1i = b + c;
    *z2r = c - b; *z2i = d + a;
}

/* crd(n, size): cardioid mapping on one parameter. n=0→t1, n=1→t2.
 * x' = size * (1 + cos(theta)) * exp(i*theta), theta = 2*pi*Re(x). */
static void pt_crd_one(double *xr, double *xi, double size) {
    double theta = 2.0 * M_PI * (*xr);
    double r = size * (1.0 + cos(theta));
    *xr = r * cos(theta);
    *xi = r * sin(theta);
}

/* crd(n, size): cardioid curve. n=0→t1, n=1→t2, n=2→both. */
static void pt_crd(double *z1r, double *z1i, double *z2r, double *z2i, int n, double size) {
    if (n == 0)      pt_crd_one(z1r, z1i, size);
    else if (n == 1) pt_crd_one(z2r, z2i, size);
    else if (n == 2) { pt_crd_one(z1r, z1i, size); pt_crd_one(z2r, z2i, size); }
}

static void pt_hrt_one(double *xr, double *xi, double size, double turns) {
    double u = *xr;
    double t = 2.0 * M_PI * u + M_PI / 2.0;
    double st = sin(t);
    double xh = 16.0 * st * st * st;
    double yh = 13.0*cos(t) - 5.0*cos(2.0*t) - 2.0*cos(3.0*t) - cos(4.0*t);
    double hr = xh / 40.0;
    double hi = yh / 40.0 + 0.1;
    double ra = 2.0 * M_PI * turns;
    double rotr = cos(ra), roti = sin(ra);
    double sr = size * hr, si = size * hi;
    *xr = rotr * sr - roti * si;
    *xi = rotr * si + roti * sr;
}

/* hrt(n, size, turns): heart curve. n=0→t1, n=1→t2, n=2→both. */
static void pt_hrt(double *z1r, double *z1i, double *z2r, double *z2i, int n, double size, double turns) {
    if (n == 0)      pt_hrt_one(z1r, z1i, size, turns);
    else if (n == 1) pt_hrt_one(z2r, z2i, size, turns);
    else if (n == 2) { pt_hrt_one(z1r, z1i, size, turns); pt_hrt_one(z2r, z2i, size, turns); }
}

/* spdl(n, va, vb, vp): spindle / superellipse-like. */
static void pt_spdl_one(double *xr, double *xi, double va, double vb, double vp) {
    double theta = 2.0 * M_PI * (*xr);
    double ct = cos(theta), st = sin(theta);
    double e = 2.0 / (vp > 0.01 ? vp : 0.01);
    *xr = va * (ct >= 0 ? 1 : -1) * pow(fabs(ct), e);
    *xi = vb * (st >= 0 ? 1 : -1) * pow(fabs(st), e);
}
static void pt_spdl(double *z1r, double *z1i, double *z2r, double *z2i, int n, double va, double vb, double vp) {
    if (n == 0)      pt_spdl_one(z1r, z1i, va, vb, vp);
    else if (n == 1) pt_spdl_one(z2r, z2i, va, vb, vp);
    else if (n == 2) { pt_spdl_one(z1r, z1i, va, vb, vp); pt_spdl_one(z2r, z2i, va, vb, vp); }
}

/* lmc(n, a, b): limacon. r = a + b*cos(theta). */
static void pt_lmc_one(double *xr, double *xi, double a, double b) {
    double theta = 2.0 * M_PI * (*xr);
    double r = a + b * cos(theta);
    *xr = r * cos(theta);
    *xi = r * sin(theta);
}
static void pt_lmc(double *z1r, double *z1i, double *z2r, double *z2i, int n, double a, double b) {
    if (n == 0)      pt_lmc_one(z1r, z1i, a, b);
    else if (n == 1) pt_lmc_one(z2r, z2i, a, b);
    else if (n == 2) { pt_lmc_one(z1r, z1i, a, b); pt_lmc_one(z2r, z2i, a, b); }
}

/* rsc(n, amp, k): rose curve. r = amp*cos(k*theta). */
static void pt_rsc_one(double *xr, double *xi, double amp, double k) {
    double theta = 2.0 * M_PI * (*xr);
    double r = amp * cos(k * theta);
    *xr = r * cos(theta);
    *xi = r * sin(theta);
}
static void pt_rsc(double *z1r, double *z1i, double *z2r, double *z2i, int n, double amp, double k) {
    if (n == 0)      pt_rsc_one(z1r, z1i, amp, k);
    else if (n == 1) pt_rsc_one(z2r, z2i, amp, k);
    else if (n == 2) { pt_rsc_one(z1r, z1i, amp, k); pt_rsc_one(z2r, z2i, amp, k); }
}

/* lss(n, A, B, a, b, phase): Lissajous. X = A*sin(a*theta+delta), Y = B*sin(b*theta). */
static void pt_lss_one(double *xr, double *xi, double A, double B, double a, double b, double phase) {
    double theta = 2.0 * M_PI * (*xr);
    double delta = M_PI * phase;
    *xr = A * sin(a * theta + delta);
    *xi = B * sin(b * theta);
}
static void pt_lss(double *z1r, double *z1i, double *z2r, double *z2i, int n, double A, double B, double a, double b, double phase) {
    if (n == 0)      pt_lss_one(z1r, z1i, A, B, a, b, phase);
    else if (n == 1) pt_lss_one(z2r, z2i, A, B, a, b, phase);
    else if (n == 2) { pt_lss_one(z1r, z1i, A, B, a, b, phase); pt_lss_one(z2r, z2i, A, B, a, b, phase); }
}

/* ast(n, scale): astroid. X = scale*cos^3, Y = scale*sin^3. */
static void pt_ast_one(double *xr, double *xi, double scale) {
    double theta = 2.0 * M_PI * (*xr);
    double ct = cos(theta), st = sin(theta);
    *xr = scale * ct * ct * ct;
    *xi = scale * st * st * st;
}
static void pt_ast(double *z1r, double *z1i, double *z2r, double *z2i, int n, double scale) {
    if (n == 0)      pt_ast_one(z1r, z1i, scale);
    else if (n == 1) pt_ast_one(z2r, z2i, scale);
    else if (n == 2) { pt_ast_one(z1r, z1i, scale); pt_ast_one(z2r, z2i, scale); }
}

/* asp(n, a, b): Archimedean spiral. r = a + b*theta. */
static void pt_asp_one(double *xr, double *xi, double a, double b) {
    double theta = 2.0 * M_PI * (*xr);
    double r = a + b * theta;
    *xr = r * cos(theta);
    *xi = r * sin(theta);
}
static void pt_asp(double *z1r, double *z1i, double *z2r, double *z2i, int n, double a, double b) {
    if (n == 0)      pt_asp_one(z1r, z1i, a, b);
    else if (n == 1) pt_asp_one(z2r, z2i, a, b);
    else if (n == 2) { pt_asp_one(z1r, z1i, a, b); pt_asp_one(z2r, z2i, a, b); }
}

/* lsp(n, a, b): logarithmic spiral. r = a*exp(b*theta). */
static void pt_lsp_one(double *xr, double *xi, double a, double b) {
    double theta = 2.0 * M_PI * (*xr);
    double r = a * exp(b * theta);
    *xr = r * cos(theta);
    *xi = r * sin(theta);
}
static void pt_lsp(double *z1r, double *z1i, double *z2r, double *z2i, int n, double a, double b) {
    if (n == 0)      pt_lsp_one(z1r, z1i, a, b);
    else if (n == 1) pt_lsp_one(z2r, z2i, a, b);
    else if (n == 2) { pt_lsp_one(z1r, z1i, a, b); pt_lsp_one(z2r, z2i, a, b); }
}

/* dlt(n, R): deltoid. X = R*(2cos + cos2)/3, Y = R*(2sin - sin2)/3. */
static void pt_dlt_one(double *xr, double *xi, double R) {
    double theta = 2.0 * M_PI * (*xr);
    *xr = R * (2.0 * cos(theta) + cos(2.0 * theta)) / 3.0;
    *xi = R * (2.0 * sin(theta) - sin(2.0 * theta)) / 3.0;
}
static void pt_dlt(double *z1r, double *z1i, double *z2r, double *z2i, int n, double R) {
    if (n == 0)      pt_dlt_one(z1r, z1i, R);
    else if (n == 1) pt_dlt_one(z2r, z2i, R);
    else if (n == 2) { pt_dlt_one(z1r, z1i, R); pt_dlt_one(z2r, z2i, R); }
}

/* rply(n, sides, radius, turns): regular polygon perimeter walk. */
static void pt_rply_one(double *xr, double *xi, double sides, double radius, double turns) {
    int ns = (int)sides;
    if (ns < 3) ns = 3;
    double t = fmod(*xr, 1.0);
    if (t < 0) t += 1.0;
    double pos = t * ns;           /* position along edges: [0, ns) */
    int edge = (int)pos;
    if (edge >= ns) edge = ns - 1;
    double frac = pos - edge;      /* fraction along this edge */
    /* vertices: v_k at angle 2*pi*k/ns */
    double a0 = 2.0 * M_PI * edge / ns;
    double a1 = 2.0 * M_PI * (edge + 1) / ns;
    double px = radius * ((1 - frac) * cos(a0) + frac * cos(a1));
    double py = radius * ((1 - frac) * sin(a0) + frac * sin(a1));
    /* apply rotation */
    double ra = 2.0 * M_PI * turns;
    *xr = cos(ra) * px - sin(ra) * py;
    *xi = sin(ra) * px + cos(ra) * py;
}
static void pt_rply(double *z1r, double *z1i, double *z2r, double *z2i, int n, double sides, double radius, double turns) {
    if (n == 0)      pt_rply_one(z1r, z1i, sides, radius, turns);
    else if (n == 1) pt_rply_one(z2r, z2i, sides, radius, turns);
    else if (n == 2) { pt_rply_one(z1r, z1i, sides, radius, turns); pt_rply_one(z2r, z2i, sides, radius, turns); }
}

/* star(n, points, outer, inner_ratio): star perimeter walk. */
static void pt_star_one(double *xr, double *xi, double points, double outer, double inner_ratio) {
    int np = (int)points;
    if (np < 3) np = 3;
    int nv = 2 * np;               /* alternating outer/inner vertices */
    double t = fmod(*xr, 1.0);
    if (t < 0) t += 1.0;
    double pos = t * nv;
    int edge = (int)pos;
    if (edge >= nv) edge = nv - 1;
    double frac = pos - edge;
    /* vertex k: even=outer at angle pi*k/np, odd=inner at same angle scheme */
    double r0 = (edge % 2 == 0) ? outer : outer * inner_ratio;
    double r1 = ((edge + 1) % 2 == 0) ? outer : outer * inner_ratio;
    double a0 = 2.0 * M_PI * edge / nv;
    double a1 = 2.0 * M_PI * (edge + 1) / nv;
    *xr = (1 - frac) * r0 * cos(a0) + frac * r1 * cos(a1);
    *xi = (1 - frac) * r0 * sin(a0) + frac * r1 * sin(a1);
}
static void pt_star(double *z1r, double *z1i, double *z2r, double *z2i, int n, double points, double outer, double inner_ratio) {
    if (n == 0)      pt_star_one(z1r, z1i, points, outer, inner_ratio);
    else if (n == 1) pt_star_one(z2r, z2i, points, outer, inner_ratio);
    else if (n == 2) { pt_star_one(z1r, z1i, points, outer, inner_ratio); pt_star_one(z2r, z2i, points, outer, inner_ratio); }
}

/* rect(n, width, height, turns): rectangle perimeter walk by arc length, then rotate. */
static void pt_rect_one(double *xr, double *xi, double w, double h, double turns) {
    double perim = 2.0 * (w + h);
    double t = fmod(*xr, 1.0);
    if (t < 0) t += 1.0;
    double d = t * perim;           /* distance along perimeter */
    double px, py;
    double hw = w / 2.0, hh = h / 2.0;
    if (d < w) {                    /* bottom: left to right */
        px = -hw + d;  py = -hh;
    } else if (d < w + h) {         /* right: bottom to top */
        px = hw;  py = -hh + (d - w);
    } else if (d < 2 * w + h) {     /* top: right to left */
        px = hw - (d - w - h);  py = hh;
    } else {                         /* left: top to bottom */
        px = -hw;  py = hh - (d - 2 * w - h);
    }
    double ra = 2.0 * M_PI * turns;
    *xr = cos(ra) * px - sin(ra) * py;
    *xi = sin(ra) * px + cos(ra) * py;
}
static void pt_rect(double *z1r, double *z1i, double *z2r, double *z2i, int n, double w, double h, double turns) {
    if (n == 0)      pt_rect_one(z1r, z1i, w, h, turns);
    else if (n == 1) pt_rect_one(z2r, z2i, w, h, turns);
    else if (n == 2) { pt_rect_one(z1r, z1i, w, h, turns); pt_rect_one(z2r, z2i, w, h, turns); }
}

/* rrect(n, width, height, m): rounded rectangle via superellipse. */
static void pt_rrect_one(double *xr, double *xi, double w, double h, double m) {
    double theta = 2.0 * M_PI * fmod(*xr, 1.0);
    double ct = cos(theta), st = sin(theta);
    double e = 2.0 / (m > 0.01 ? m : 0.01);
    *xr = (w / 2.0) * (ct >= 0 ? 1 : -1) * pow(fabs(ct), e);
    *xi = (h / 2.0) * (st >= 0 ? 1 : -1) * pow(fabs(st), e);
}
static void pt_rrect(double *z1r, double *z1i, double *z2r, double *z2i, int n, double w, double h, double m) {
    if (n == 0)      pt_rrect_one(z1r, z1i, w, h, m);
    else if (n == 1) pt_rrect_one(z2r, z2i, w, h, m);
    else if (n == 2) { pt_rrect_one(z1r, z1i, w, h, m); pt_rrect_one(z2r, z2i, w, h, m); }
}

static int pt_target_value(const double *args, int nArgs, int idx, int fallback) {
    if (!args || idx < 0 || idx >= nArgs) return fallback;
    int n = (int)args[idx];
    return (n == 0 || n == 1 || n == 2) ? n : fallback;
}

static void pt_apply_independent_target(ParamTransform fn,
                                        double *z1r, double *z1i,
                                        double *z2r, double *z2i,
                                        int n) {
    if (!fn) return;
    if (n == 2) {
        fn(z1r, z1i, z2r, z2i);
        return;
    }
    if (n == 0) {
        double t2r = *z2r, t2i = *z2i;
        fn(z1r, z1i, &t2r, &t2i);
        return;
    }
    if (n == 1) {
        double t1r = *z1r, t1i = *z1i;
        fn(&t1r, &t1i, z2r, z2i);
    }
}

static int pt_is_targetable_independent(const char *name) {
    return strcmp(name, "unit_circle") == 0 ||
           strcmp(name, "square") == 0 ||
           strcmp(name, "cube") == 0 ||
           strcmp(name, "reciprocal") == 0 ||
           strcmp(name, "conjugate") == 0 ||
           strcmp(name, "negate") == 0 ||
           strcmp(name, "exp") == 0 ||
           strcmp(name, "xim") == 0;
}

static void pt_rtheta_target(double *z1r, double *z1i, double *z2r, double *z2i,
                             int n, double p) {
    double in1r = *z1r, in2r = *z2r;
    double r1 = pow(in1r, p), r2 = pow(in2r, p);
    double a1 = 2.0 * M_PI * in2r, a2 = 2.0 * M_PI * in1r;
    double out1r = r1 * cos(a1), out1i = r1 * sin(a1);
    double out2r = r2 * cos(a2), out2i = r2 * sin(a2);
    if (n == 0 || n == 2) { *z1r = out1r; *z1i = out1i; }
    if (n == 1 || n == 2) { *z2r = out2r; *z2i = out2i; }
}

/* xim: t1' = i*Re(t1), t2' = i*Re(t2). Real parts become imaginary, imag discarded. */
static void pt_xim(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a = *z1r, c = *z2r;
    *z1r = 0; *z1i = a;
    *z2r = 0; *z2i = c;
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
static inline double c_scale_safe(double scale, double factor) {
    if (!isfinite(scale)) {
        if (fabs(factor) < 1e-300) return 0.0;
        return copysign(INFINITY, factor);
    }
    return scale * factor;
}
static inline void c_sin(double ar, double ai, double *rr, double *ri) {
    double ay = fabs(ai);
    if (ay > 350.0) {
        double scale = 0.5 * exp(ay);
        *rr = c_scale_safe(scale, sin(ar));
        *ri = c_scale_safe(scale, ai >= 0.0 ? cos(ar) : -cos(ar));
        return;
    }
    *rr = sin(ar) * cosh(ai);
    *ri = cos(ar) * sinh(ai);
}
static inline void c_cos(double ar, double ai, double *rr, double *ri) {
    double ay = fabs(ai);
    if (ay > 350.0) {
        double scale = 0.5 * exp(ay);
        *rr = c_scale_safe(scale, cos(ar));
        *ri = c_scale_safe(scale, ai >= 0.0 ? -sin(ar) : sin(ar));
        return;
    }
    *rr = cos(ar) * cosh(ai);
    *ri = -sin(ar) * sinh(ai);
}
static inline void c_tan(double ar, double ai, double *rr, double *ri) {
    double x2 = 2.0 * ar, y2 = 2.0 * ai;
    double ay2 = fabs(y2);
    if (ay2 > 350.0) {
        *rr = 0.0;
        *ri = copysign(1.0, ai);
        return;
    }
    double denom = cos(x2) + cosh(y2);
    if (fabs(denom) < 1e-300) {
        *rr = 0.0;
        *ri = 0.0;
        return;
    }
    *rr = sin(x2) / denom;
    *ri = sinh(y2) / denom;
}
static inline void c_sinh(double ar, double ai, double *rr, double *ri) {
    double ax = fabs(ar);
    if (ax > 350.0) {
        double scale = 0.5 * exp(ax);
        *rr = c_scale_safe(scale, ar >= 0.0 ? cos(ai) : -cos(ai));
        *ri = c_scale_safe(scale, sin(ai));
        return;
    }
    *rr = sinh(ar) * cos(ai);
    *ri = cosh(ar) * sin(ai);
}
static inline void c_cosh(double ar, double ai, double *rr, double *ri) {
    double ax = fabs(ar);
    if (ax > 350.0) {
        double scale = 0.5 * exp(ax);
        *rr = c_scale_safe(scale, cos(ai));
        *ri = c_scale_safe(scale, ar >= 0.0 ? sin(ai) : -sin(ai));
        return;
    }
    *rr = cosh(ar) * cos(ai);
    *ri = sinh(ar) * sin(ai);
}
static inline void c_tanh(double ar, double ai, double *rr, double *ri) {
    double x2 = 2.0 * ar, y2 = 2.0 * ai;
    double ax2 = fabs(x2);
    if (ax2 > 350.0) {
        *rr = copysign(1.0, ar);
        *ri = 0.0;
        return;
    }
    double denom = cosh(x2) + cos(y2);
    if (fabs(denom) < 1e-300) {
        *rr = 0.0;
        *ri = 0.0;
        return;
    }
    *rr = sinh(x2) / denom;
    *ri = sin(y2) / denom;
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

/* Complex inverse trig (needs c_powr defined above) */
static inline void c_atan(double ar, double ai, double *rr, double *ri) {
    double p1r = 1 - ai, p1i = ar;
    double p2r = 1 + ai, p2i = -ar;
    double qr, qi; c_div(p1r, p1i, p2r, p2i, &qr, &qi);
    double lr, li; c_log(qr, qi, &lr, &li);
    *rr = li / 2; *ri = -lr / 2;
}
static inline void c_asinh(double ar, double ai, double *rr, double *ri) {
    double z2r, z2i; c_mul(ar, ai, ar, ai, &z2r, &z2i);
    z2r += 1;
    double sr, si; c_powr(z2r, z2i, 0.5, &sr, &si);
    sr += ar; si += ai;
    c_log(sr, si, rr, ri);
}
static inline void c_asin(double ar, double ai, double *rr, double *ri) {
    double z2r, z2i; c_mul(ar, ai, ar, ai, &z2r, &z2i);
    double wr = 1 - z2r, wi = -z2i;
    double sr, si; c_powr(wr, wi, 0.5, &sr, &si);
    sr += -ai; si += ar;
    double lr, li; c_log(sr, si, &lr, &li);
    *rr = li; *ri = -lr;
}
static inline void c_acos(double ar, double ai, double *rr, double *ri) {
    double asr, asi; c_asin(ar, ai, &asr, &asi);
    *rr = M_PI/2 - asr; *ri = -asi;
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
    if (strcmp(name, "xim") == 0)        return pt_xim;
    if (strcmp(name, "zzold") == 0)      return pt_zzold;
    if (strcmp(name, "zz1") == 0)        return pt_zz1;
    if (strcmp(name, "zz2") == 0)        return pt_zz2;
    if (strcmp(name, "zz3") == 0)        return pt_zz3;
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

static void ct_blend_with_original(double *cRe, double *cIm, int *nCoeffs,
                                   const double *origRe, const double *origIm,
                                   int origN, double andy) {
    if (!isfinite(andy) || andy == 0.0) return;
    int n = *nCoeffs < origN ? *nCoeffs : origN;
    double fWeight = 1.0 - andy;
    for (int k = 0; k < n; k++) {
        cRe[k] = cRe[k] * fWeight + origRe[k] * andy;
        cIm[k] = cIm[k] * fWeight + origIm[k] * andy;
    }
}

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

static void ct_linear_affine(double *cRe, double *cIm, int *nCoeffs,
                             double x, double y, double w, double u) {
    int n = *nCoeffs;
    for (int k = 0; k < n; k++) {
        double zr = cRe[k], zi = cIm[k];
        cRe[k] = zr * x - zi * y + w;
        cIm[k] = zr * y + zi * x + u;
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) {
            cRe[k] = 0.0;
            cIm[k] = 0.0;
        }
    }
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

static void ct_swirler(double *cRe, double *cIm, int *nCoeffs) {
    for (int k = 0; k < *nCoeffs; k++) {
        double zr = cRe[k], zi = cIm[k];
        /* a = |z*100| % 1 */
        double mag100 = sqrt((zr*100)*(zr*100) + (zi*100)*(zi*100));
        double a = fmod(mag100, 1.0);
        if (a < 0) a += 1.0;
        /* b = |z*10| % 1 */
        double mag10 = sqrt((zr*10)*(zr*10) + (zi*10)*(zi*10));
        double b = fmod(mag10, 1.0);
        if (b < 0) b += 1.0;
        /* exp(a^4 + b^4 + i*2*pi*b*a) */
        double realExp = a*a*a*a + b*b*b*b;
        double imagExp = 2.0 * M_PI * b * a;
        double er = exp(realExp) * cos(imagExp);
        double ei = exp(realExp) * sin(imagExp);
        /* z * exp(...) */
        cRe[k] = zr * er - zi * ei;
        cIm[k] = zr * ei + zi * er;
    }
}

/* sort_mod: sort moduli ascending, keep original angles.
 * cf[k] = sorted_moduli[k] * exp(i * angle(cf[k])). */
static void ct_sort_mod(double *cRe, double *cIm, int *nCoeffs) {
    int n = *nCoeffs;
    /* Extract moduli */
    double mods[MAX_COEFFS];
    for (int k = 0; k < n; k++) mods[k] = sqrt(cRe[k]*cRe[k] + cIm[k]*cIm[k]);
    /* Sort moduli ascending (simple insertion sort — n is small) */
    for (int i = 1; i < n; i++) {
        double key = mods[i];
        int j = i - 1;
        while (j >= 0 && mods[j] > key) { mods[j+1] = mods[j]; j--; }
        mods[j+1] = key;
    }
    /* Reconstruct: sorted modulus with original angle */
    for (int k = 0; k < n; k++) {
        double angle = atan2(cIm[k], cRe[k]);
        cRe[k] = mods[k] * cos(angle);
        cIm[k] = mods[k] * sin(angle);
    }
}

/* sort_angle_keep_mod: sort angles ascending, keep original moduli.
 * cf[k] = |cf[k]| * exp(i * sorted_angles[k]). */
static void ct_sort_angle_keep_mod(double *cRe, double *cIm, int *nCoeffs) {
    int n = *nCoeffs;
    double angles[MAX_COEFFS];
    for (int k = 0; k < n; k++) angles[k] = atan2(cIm[k], cRe[k]);
    for (int i = 1; i < n; i++) {
        double key = angles[i];
        int j = i - 1;
        while (j >= 0 && angles[j] > key) { angles[j+1] = angles[j]; j--; }
        angles[j+1] = key;
    }
    for (int k = 0; k < n; k++) {
        double mod = sqrt(cRe[k]*cRe[k] + cIm[k]*cIm[k]);
        cRe[k] = mod * cos(angles[k]);
        cIm[k] = mod * sin(angles[k]);
    }
}

/* sort_abs: reorder coefficients by ascending magnitude. */
static void ct_sort_abs(double *cRe, double *cIm, int *nCoeffs) {
    int n = *nCoeffs;
    /* Insertion sort by magnitude — n is small */
    for (int i = 1; i < n; i++) {
        double kr = cRe[i], ki = cIm[i];
        double km = kr*kr + ki*ki;
        int j = i - 1;
        while (j >= 0 && (cRe[j]*cRe[j] + cIm[j]*cIm[j]) > km) {
            cRe[j+1] = cRe[j]; cIm[j+1] = cIm[j]; j--;
        }
        cRe[j+1] = kr; cIm[j+1] = ki;
    }
}

static void ct_cumsum(double *cRe, double *cIm, int *nCoeffs) {
    for (int k = 1; k < *nCoeffs; k++) { cRe[k] += cRe[k-1]; cIm[k] += cIm[k-1]; }
}

/* cummax: running maximum by coefficient magnitude, preserving the winning value. */
static void ct_cummax(double *cRe, double *cIm, int *nCoeffs) {
    int n = *nCoeffs;
    if (n <= 0) return;
    double bestRe = cRe[0], bestIm = cIm[0];
    double bestMag2 = bestRe * bestRe + bestIm * bestIm;
    for (int k = 0; k < n; k++) {
        double mag2 = cRe[k] * cRe[k] + cIm[k] * cIm[k];
        if (mag2 >= bestMag2) {
            bestRe = cRe[k];
            bestIm = cIm[k];
            bestMag2 = mag2;
        }
        cRe[k] = bestRe;
        cIm[k] = bestIm;
    }
}

/* sort_cumsum: reorder coefficients by ascending magnitude of cumulative sum. */
static void ct_sort_cumsum(double *cRe, double *cIm, int *nCoeffs) {
    int n = *nCoeffs;
    /* Compute cumsum magnitudes */
    double cumR = 0, cumI = 0;
    double mags[MAX_COEFFS];
    int idx[MAX_COEFFS];
    for (int k = 0; k < n; k++) {
        cumR += cRe[k]; cumI += cIm[k];
        mags[k] = cumR * cumR + cumI * cumI;
        idx[k] = k;
    }
    /* Sort indices by cumsum magnitude (insertion sort) */
    for (int i = 1; i < n; i++) {
        double km = mags[i]; int ki = idx[i];
        int j = i - 1;
        while (j >= 0 && mags[j] > km) { mags[j+1] = mags[j]; idx[j+1] = idx[j]; j--; }
        mags[j+1] = km; idx[j+1] = ki;
    }
    /* Reorder coefficients */
    double tmpR[MAX_COEFFS], tmpI[MAX_COEFFS];
    for (int k = 0; k < n; k++) { tmpR[k] = cRe[k]; tmpI[k] = cIm[k]; }
    for (int k = 0; k < n; k++) { cRe[k] = tmpR[idx[k]]; cIm[k] = tmpI[idx[k]]; }
}

/* power(k): cf[i] = (i+1) * sum_{j=0..k} cf[i]^j */
static void ct_power(double *cRe, double *cIm, int *nCoeffs, int power) {
    int n = *nCoeffs;
    if (power < 0) power = 0;
    for (int i = 0; i < n; i++) {
        double zr = cRe[i], zi = cIm[i];
        double sumR = 1.0, sumI = 0.0;
        double powR = 1.0, powI = 0.0;
        for (int p = 1; p <= power; p++) {
            double nextPowR = powR * zr - powI * zi;
            double nextPowI = powR * zi + powI * zr;
            powR = nextPowR;
            powI = nextPowI;
            sumR += powR;
            sumI += powI;
        }
        double scale = (double)(i + 1);
        cRe[i] = sumR * scale;
        cIm[i] = sumI * scale;
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) {
            cRe[i] = 0.0;
            cIm[i] = 0.0;
        }
    }
}

static void ct_exp_affine(double *cRe, double *cIm, int *nCoeffs, double a, double b) {
    int n = *nCoeffs;
    for (int i = 0; i < n; i++) {
        double mr, mi;
        c_mul(cRe[i], cIm[i], a, b, &mr, &mi);
        c_exp2(mr, mi, &cRe[i], &cIm[i]);
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) {
            cRe[i] = 0.0;
            cIm[i] = 0.0;
        }
    }
}

static void ct_apply_unary_complex(double *cRe, double *cIm, int *nCoeffs,
                                   void (*fn)(double, double, double *, double *)) {
    int n = *nCoeffs;
    for (int i = 0; i < n; i++) {
        double rr, ri;
        fn(cRe[i], cIm[i], &rr, &ri);
        if (!isfinite(rr) || !isfinite(ri)) {
            cRe[i] = 0.0;
            cIm[i] = 0.0;
        } else {
            cRe[i] = rr;
            cIm[i] = ri;
        }
    }
}

static void ct_cos_apply(double *cRe, double *cIm, int *nCoeffs)  { ct_apply_unary_complex(cRe, cIm, nCoeffs, c_cos); }
static void ct_sin_apply(double *cRe, double *cIm, int *nCoeffs)  { ct_apply_unary_complex(cRe, cIm, nCoeffs, c_sin); }
static void ct_tan_apply(double *cRe, double *cIm, int *nCoeffs)  { ct_apply_unary_complex(cRe, cIm, nCoeffs, c_tan); }
static void ct_cosh_apply(double *cRe, double *cIm, int *nCoeffs) { ct_apply_unary_complex(cRe, cIm, nCoeffs, c_cosh); }
static void ct_sinh_apply(double *cRe, double *cIm, int *nCoeffs) { ct_apply_unary_complex(cRe, cIm, nCoeffs, c_sinh); }
static void ct_tanh_apply(double *cRe, double *cIm, int *nCoeffs) { ct_apply_unary_complex(cRe, cIm, nCoeffs, c_tanh); }

static void ct_round_affine(double *cRe, double *cIm, int *nCoeffs, double a, double b) {
    int n = *nCoeffs;
    for (int i = 0; i < n; i++) {
        double mr, mi;
        c_mul(cRe[i], cIm[i], a, b, &mr, &mi);
        cRe[i] = round(mr);
        cIm[i] = round(mi);
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) {
            cRe[i] = 0.0;
            cIm[i] = 0.0;
        }
    }
}

static void ct_pow_affine(double *cRe, double *cIm, int *nCoeffs,
                          double a, double b, double pr, double pi) {
    int n = *nCoeffs;
    for (int i = 0; i < n; i++) {
        double br, bi, rr, ri;
        c_mul(cRe[i], cIm[i], a, b, &br, &bi);
        if (br * br + bi * bi < 1e-60) {
            rr = 0.0;
            ri = 0.0;
        } else {
            c_powc(br, bi, pr, pi, &rr, &ri);
        }
        if (!isfinite(rr) || !isfinite(ri)) {
            cRe[i] = 0.0;
            cIm[i] = 0.0;
        } else {
            cRe[i] = rr;
            cIm[i] = ri;
        }
    }
}

/* invpower(k): let s = sum_{j=0..k} cf[i]^j.
 * If |s| > 1, cf[i] = 1/s, else cf[i] = 1. */
static void ct_invpower(double *cRe, double *cIm, int *nCoeffs, int power) {
    int n = *nCoeffs;
    if (power < 0) power = 0;
    for (int i = 0; i < n; i++) {
        double zr = cRe[i], zi = cIm[i];
        double sumR = 1.0, sumI = 0.0;
        double powR = 1.0, powI = 0.0;
        for (int p = 1; p <= power; p++) {
            double nextPowR = powR * zr - powI * zi;
            double nextPowI = powR * zi + powI * zr;
            powR = nextPowR;
            powI = nextPowI;
            sumR += powR;
            sumI += powI;
        }
        double sumMag = hypot(sumR, sumI);
        if (isfinite(sumMag) && sumMag > 1.0) {
            double denom = sumR * sumR + sumI * sumI;
            if (denom > 1e-300) {
                cRe[i] =  sumR / denom;
                cIm[i] = -sumI / denom;
            } else {
                cRe[i] = 1.0;
                cIm[i] = 0.0;
            }
        } else {
            cRe[i] = 1.0;
            cIm[i] = 0.0;
        }
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) {
            cRe[i] = 1.0;
            cIm[i] = 0.0;
        }
    }
}

static void ct_write_roots_padded(double *cRe, double *cIm, int totalCoeffs,
                                  const double *rootRe, const double *rootIm,
                                  int nRoots, int padLo) {
    if (totalCoeffs <= 0) return;
    for (int i = 0; i < totalCoeffs; i++) {
        cRe[i] = 0.0;
        cIm[i] = 0.0;
    }
    if (nRoots <= 0) return;
    if (padLo) {
        for (int i = 0; i < nRoots && i < totalCoeffs - 1; i++) {
            cRe[i] = rootRe[i];
            cIm[i] = rootIm[i];
        }
        return;
    }
    for (int i = 0; i < nRoots && i + 1 < totalCoeffs; i++) {
        cRe[i + 1] = rootRe[i];
        cIm[i + 1] = rootIm[i];
    }
}

/* roots_cm(mode): compute roots via companion matrix and treat those roots as
 * the next coefficient vector. Output length stays constant by padding one zero
 * either at the highest-order side ("hi") or constant side ("lo"). */
static int ct_roots_cm(double *cRe, double *cIm, int *nCoeffs, int padLo) {
    int n = *nCoeffs;
    int degree = n - 1;
    if (n <= 0) return 0;
    if (degree <= 0) {
        cRe[0] = 0.0;
        cIm[0] = 0.0;
        return 0;
    }
    if (!companion_solver_available()) {
        fprintf(stderr, "roots_cm unavailable in this build\n");
        return 1;
    }

    float rootRe[MAX_DEGREE], rootIm[MAX_DEGREE];
    int rc = solve_companion_coeffs(cRe, cIm, n, rootRe, rootIm);
    if (rc == 0) {
        fprintf(stderr, "roots_cm companion solve failed\n");
        return 1;
    }
    double outRe[MAX_DEGREE], outIm[MAX_DEGREE];
    for (int i = 0; i < degree; i++) {
        outRe[i] = (double)rootRe[i];
        outIm[i] = (double)rootIm[i];
    }
    ct_write_roots_padded(cRe, cIm, n, outRe, outIm, degree, padLo);
    return 0;
}

/* roots(k,mode): run k Aberth-Ehrlich iterations on the current coefficient
 * vector, then treat the resulting roots as the next coefficient vector.
 * Output length stays constant by padding one zero at the highest-order side
 * ("hi") or constant side ("lo"). */
static void ct_roots(double *cRe, double *cIm, int *nCoeffs, int iters, int padLo) {
    int n = *nCoeffs;
    int degree = n - 1;
    if (n <= 0) return;
    if (iters < 1) iters = 1;
    if (degree <= 0) {
        cRe[0] = 0.0;
        cIm[0] = 0.0;
        return;
    }

    int start = 0;
    while (start < n - 1 &&
           cRe[start] * cRe[start] + cIm[start] * cIm[start] < 1e-30)
        start++;
    int effN = n - start;

    int trailingZeros = 0;
    while (trailingZeros < effN - 1) {
        int idx = start + effN - 1 - trailingZeros;
        if (cRe[idx] * cRe[idx] + cIm[idx] * cIm[idx] >= 1e-30) break;
        trailingZeros++;
    }
    effN -= trailingZeros;
    int effDeg = effN - 1;

    double rootRe[MAX_DEGREE], rootIm[MAX_DEGREE];
    for (int i = 0; i < degree; i++) { rootRe[i] = 0.0; rootIm[i] = 0.0; }
    if (effDeg > 0) {
        seedEAInitialGuess(rootRe, rootIm, effDeg);
        solveEALimited(cRe + start, cIm + start, effN, rootRe, rootIm, effDeg, iters);
    }
    ct_write_roots_padded(cRe, cIm, n, rootRe, rootIm, effDeg, padLo);
}

static CoeffTransform lookupCoeffTransform(const char *name) {
    if (strcmp(name, "none") == 0)        return ct_none;
    if (strcmp(name, "rev") == 0)         return ct_rev;
    if (strcmp(name, "conj") == 0)        return ct_conj;
    if (strcmp(name, "normalize") == 0)   return ct_normalize;
    if (strcmp(name, "deriv") == 0)       return ct_deriv;
    if (strcmp(name, "safe") == 0)        return ct_safe;
    if (strcmp(name, "negate_odd") == 0)  return ct_negate_odd;
    if (strcmp(name, "max2one") == 0)    return ct_max2one;
    if (strcmp(name, "swirler") == 0)   return ct_swirler;
    if (strcmp(name, "sort_mod_keep_angle") == 0)  return ct_sort_mod;
    if (strcmp(name, "sort_angle_keep_mod") == 0)  return ct_sort_angle_keep_mod;
    if (strcmp(name, "sort_abs") == 0)  return ct_sort_abs;
    if (strcmp(name, "cumsum") == 0)   return ct_cumsum;
    if (strcmp(name, "cummax") == 0)   return ct_cummax;
    if (strcmp(name, "sort_cumsum") == 0) return ct_sort_cumsum;
    if (strcmp(name, "cos") == 0)      return ct_cos_apply;
    if (strcmp(name, "sin") == 0)      return ct_sin_apply;
    if (strcmp(name, "tan") == 0)      return ct_tan_apply;
    if (strcmp(name, "cosh") == 0)     return ct_cosh_apply;
    if (strcmp(name, "sinh") == 0)     return ct_sinh_apply;
    if (strcmp(name, "tanh") == 0)     return ct_tanh_apply;
    return NULL;
}

static int dispatchCt(const CtEntry *e, double *cRe, double *cIm, int *nCoeffs) {
    double origRe[MAX_COEFFS], origIm[MAX_COEFFS];
    int origN = *nCoeffs;
    double andy = ct_arg_andy(e);
    if (isfinite(andy) && andy != 0.0) {
        for (int i = 0; i < origN && i < MAX_COEFFS; i++) {
            origRe[i] = cRe[i];
            origIm[i] = cIm[i];
        }
    }
    int rc = 0;
    if (strcmp(e->name, "scale100") == 0) {
        double x = 100.0, y = 0.0, w = 0.0, u = 0.0;
        if (ct_arg_has_complex_unit(e, 0)) {
            if (!ct_parse_complex_literal(e->args[0], &x, &y)) {
                fprintf(stderr, "Invalid scale100 multiplier: %s\n", e->args[0]);
                return 1;
            }
        } else {
            x = ct_arg_double(e, 0, 100.0);
            y = ct_arg_double(e, 1, 0.0);
        }
        if (ct_arg_has_complex_unit(e, 2)) {
            if (!ct_parse_complex_literal(e->args[2], &w, &u)) {
                fprintf(stderr, "Invalid scale100 offset: %s\n", e->args[2]);
                return 1;
            }
        } else {
            w = ct_arg_double(e, 2, 0.0);
            u = ct_arg_double(e, 3, 0.0);
        }
        ct_linear_affine(cRe, cIm, nCoeffs, x, y, w, u);
        goto done;
    }
    if (strcmp(e->name, "roots_cm") == 0) {
        int padLo = ct_arg_pad_lo(e, 0, 0);
        if (padLo < 0) return 1;
        rc = ct_roots_cm(cRe, cIm, nCoeffs, padLo);
        goto done;
    }
    if (strcmp(e->name, "power") == 0) {
        int k = ct_arg_int(e, 0, 8);
        ct_power(cRe, cIm, nCoeffs, k);
        goto done;
    }
    if (strcmp(e->name, "invpower") == 0) {
        int k = ct_arg_int(e, 0, 4);
        ct_invpower(cRe, cIm, nCoeffs, k);
        goto done;
    }
    if (strcmp(e->name, "exp") == 0) {
        double a = ct_arg_double(e, 0, 1.0);
        double b = ct_arg_double(e, 1, 0.0);
        ct_exp_affine(cRe, cIm, nCoeffs, a, b);
        goto done;
    }
    if (strcmp(e->name, "round") == 0) {
        double a = ct_arg_double(e, 0, 1.0);
        double b = ct_arg_double(e, 1, 0.0);
        ct_round_affine(cRe, cIm, nCoeffs, a, b);
        goto done;
    }
    if (strcmp(e->name, "pow") == 0) {
        double a = ct_arg_double(e, 0, 1.0);
        double b = ct_arg_double(e, 1, 0.0);
        double pr = ct_arg_double(e, 2, 1.0);
        double pi = ct_arg_double(e, 3, 0.0);
        ct_pow_affine(cRe, cIm, nCoeffs, a, b, pr, pi);
        goto done;
    }
    if (strcmp(e->name, "roots") == 0) {
        int k = ct_arg_int(e, 0, 8);
        int padLo = ct_arg_pad_lo(e, 1, 0);
        if (padLo < 0) return 1;
        ct_roots(cRe, cIm, nCoeffs, k, padLo);
        goto done;
    }
    CoeffTransform fn = lookupCoeffTransform(e->name);
    if (!fn) {
        fprintf(stderr, "Unknown coeff transform: %s\n", e->name);
        return 1;
    }
    fn(cRe, cIm, nCoeffs);
done:
    if (rc == 0) ct_blend_with_original(cRe, cIm, nCoeffs, origRe, origIm, origN, andy);
    return rc;
}

/* ==== Fast xorshift64 RNG for dithering ==== */

static PP_THREAD_LOCAL uint64_t _rng_state = 0x123456789abcdef0ULL;
static inline uint64_t xorshift64(void) {
    uint64_t x = _rng_state;
    x ^= x << 13; x ^= x >> 7; x ^= x << 17;
    _rng_state = x;
    return x;
}
static inline double rng_uniform(void) {
    return (xorshift64() >> 11) * (1.0 / 9007199254740992.0);
}

static inline uint64_t paramGenRowSeed(long globalRow, int i1, int pass) {
    uint64_t seed = 0x123456789abcdef0ULL;
    seed ^= ((uint64_t)(pass + 1) * 0x9e3779b97f4a7c15ULL);
    seed ^= ((uint64_t)(i1 + 1) * 0xbf58476d1ce4e5b9ULL);
    seed ^= ((uint64_t)(globalRow + 1) * 0x94d049bb133111ebULL);
    return seed ? seed : 1ULL;
}

/* sdith(d): square dither — adds uniform noise to both re and im.
 * Width = d/N where N is grid size. d=1 means one grid spacing, d=2 means twice. */
static void pt_sdith(double *z1r, double *z1i, double *z2r, double *z2i, double d, int gridN) {
    if (d <= 0.0) d = 1.0;
    double w = d / (gridN > 0 ? gridN : 1);
    *z1r += w * (rng_uniform() - 0.5);
    *z1i += w * (rng_uniform() - 0.5);
    *z2r += w * (rng_uniform() - 0.5);
    *z2i += w * (rng_uniform() - 0.5);
}

/* ddith(n, d, exp): disk dither — random offset inside disk of radius d/N.
 * n=0→t1, n=1→t2, n=2→both. exp controls radial distribution (0.5 = uniform area). */
static void pt_ddith_one(double *xr, double *xi, double rmax, double ex) {
    double u1 = rng_uniform(), u2 = rng_uniform();
    double theta = 2.0 * M_PI * u1;
    double r = pow(u2, ex) * rmax;
    *xr += r * cos(theta);
    *xi += r * sin(theta);
}
static void pt_ddith(double *z1r, double *z1i, double *z2r, double *z2i, int n, double d, double ex, int gridN) {
    if (d <= 0.0) d = 1.0;
    if (ex <= 0.0) ex = 0.5;
    double rmax = d / (gridN > 0 ? gridN : 1);
    if (n == 0 || n == 2) pt_ddith_one(z1r, z1i, rmax, ex);
    if (n == 1 || n == 2) pt_ddith_one(z2r, z2i, rmax, ex);
}

/* ndith(d): normal dither — independent Gaussian jitter, sigma = d/N.
 * Uses Box-Muller transform for normal samples. */
static void pt_ndith(double *z1r, double *z1i, double *z2r, double *z2i, double d, int gridN) {
    if (d <= 0.0) d = 1.0;
    double sigma = d / (gridN > 0 ? gridN : 1);
    double u1, u2, z0, z1_bm;
    /* t1 real + imag */
    u1 = rng_uniform(); u2 = rng_uniform();
    if (u1 < 1e-30) u1 = 1e-30;
    z0 = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
    z1_bm = sqrt(-2.0 * log(u1)) * sin(2.0 * M_PI * u2);
    *z1r += sigma * z0; *z1i += sigma * z1_bm;
    /* t2 real + imag */
    u1 = rng_uniform(); u2 = rng_uniform();
    if (u1 < 1e-30) u1 = 1e-30;
    z0 = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
    z1_bm = sqrt(-2.0 * log(u1)) * sin(2.0 * M_PI * u2);
    *z2r += sigma * z0; *z2i += sigma * z1_bm;
}

/* adth(n, d, inner): annulus dither — uniform by area between inner*rmax and rmax. */
static void pt_adth_one(double *xr, double *xi, double rmax, double inner_frac) {
    if (inner_frac < 0) inner_frac = 0;
    if (inner_frac > 1) inner_frac = 1;
    double r0 = inner_frac * rmax, r1 = rmax;
    double u = rng_uniform();
    double r = sqrt(r0 * r0 + u * (r1 * r1 - r0 * r0));
    double theta = 2.0 * M_PI * rng_uniform();
    *xr += r * cos(theta);
    *xi += r * sin(theta);
}
static void pt_adth(double *z1r, double *z1i, double *z2r, double *z2i, int n, double d, double inner_frac, int gridN) {
    if (d <= 0.0) d = 1.0;
    double rmax = d / (gridN > 0 ? gridN : 1);
    if (n == 0 || n == 2) pt_adth_one(z1r, z1i, rmax, inner_frac);
    if (n == 1 || n == 2) pt_adth_one(z2r, z2i, rmax, inner_frac);
}

/* ldth(n, d, len, angle): line dither — uniform on a rotated line segment. */
static void pt_ldth_one(double *xr, double *xi, double half_len, double angle) {
    double t = (2.0 * rng_uniform() - 1.0) * half_len;
    *xr += t * cos(angle);
    *xi += t * sin(angle);
}
static void pt_ldth(double *z1r, double *z1i, double *z2r, double *z2i, int n, double d, double len_frac, double angle, int gridN) {
    if (d <= 0.0) d = 1.0;
    double half_len = (d / (gridN > 0 ? gridN : 1)) * len_frac;
    if (n == 0 || n == 2) pt_ldth_one(z1r, z1i, half_len, angle);
    if (n == 1 || n == 2) pt_ldth_one(z2r, z2i, half_len, angle);
}

/* crdth(n, d): cross dither — 50% horizontal, 50% vertical, uniform along arm. */
static void pt_crdth_one(double *xr, double *xi, double half_len) {
    double t = (2.0 * rng_uniform() - 1.0) * half_len;
    if (rng_uniform() < 0.5)
        *xr += t;
    else
        *xi += t;
}
static void pt_crdth(double *z1r, double *z1i, double *z2r, double *z2i, int n, double d, int gridN) {
    if (d <= 0.0) d = 1.0;
    double half_len = d / (gridN > 0 ? gridN : 1);
    if (n == 0 || n == 2) pt_crdth_one(z1r, z1i, half_len);
    if (n == 1 || n == 2) pt_crdth_one(z2r, z2i, half_len);
}

/* scdth(n, d, half_ap, center): sector dither — area-uniform in circular sector. */
static void pt_scdth_one(double *xr, double *xi, double rmax, double half_ap, double center) {
    double r = sqrt(rng_uniform()) * rmax;
    double theta = center + (2.0 * rng_uniform() - 1.0) * half_ap;
    *xr += r * cos(theta);
    *xi += r * sin(theta);
}
static void pt_scdth(double *z1r, double *z1i, double *z2r, double *z2i, int n, double d, double half_ap_frac, double center, int gridN) {
    if (d <= 0.0) d = 1.0;
    if (half_ap_frac < 0) half_ap_frac = 0;
    if (half_ap_frac > 1) half_ap_frac = 1;
    double rmax = d / (gridN > 0 ? gridN : 1);
    double half_ap = M_PI * half_ap_frac;
    if (n == 0 || n == 2) pt_scdth_one(z1r, z1i, rmax, half_ap, center);
    if (n == 1 || n == 2) pt_scdth_one(z2r, z2i, rmax, half_ap, center);
}

/* roots5: cubic root parameter transform.
 * Solves a*z^3 + b*z^2 + c*z + d = 0 via Cardano's formula,
 * returns (smallest root, largest root) by magnitude. */
static void _cbrt_c(double zr, double zi, double *rr, double *ri) {
    double m2 = zr*zr + zi*zi;
    if (m2 < 1e-60) { *rr = 0; *ri = 0; return; }
    double r = pow(sqrt(m2), 1.0/3.0);
    double th = atan2(zi, zr) / 3.0;
    *rr = r * cos(th); *ri = r * sin(th);
}

/* Polynomial root solvers (complex coefficients). Returns roots in out[] arrays, returns count. */
#define MAX_POLY_ROOTS 4
static const double _omega_r = -0.5, _omega_i = 0.86602540378443864676;  /* exp(2pi*i/3) */
static const double _omega2_r = -0.5, _omega2_i = -0.86602540378443864676;

static int _solve_linear(double ar, double ai, double br, double bi,
                         double *outr, double *outi) {
    double m = ar*ar + ai*ai;
    if (m < 1e-60) return 0;
    c_div(-br, -bi, ar, ai, &outr[0], &outi[0]);
    return 1;
}

static int _solve_quadratic(double ar, double ai, double br, double bi,
                            double cr, double ci, double *outr, double *outi) {
    double m = ar*ar + ai*ai;
    if (m < 1e-60) return _solve_linear(br, bi, cr, ci, outr, outi);
    /* disc = b^2 - 4ac */
    double b2r, b2i; c_mul(br, bi, br, bi, &b2r, &b2i);
    double acr, aci; c_mul(ar, ai, cr, ci, &acr, &aci);
    double dr = b2r - 4*acr, di = b2i - 4*aci;
    double sr, si; c_powr(dr, di, 0.5, &sr, &si);
    double a2r = 2*ar, a2i = 2*ai;
    c_div(-br + sr, -bi + si, a2r, a2i, &outr[0], &outi[0]);
    c_div(-br - sr, -bi - si, a2r, a2i, &outr[1], &outi[1]);
    return 2;
}

static int _solve_cubic(double ar, double ai, double br, double bi,
                        double cr, double ci, double dr, double di,
                        double *outr, double *outi) {
    double m = ar*ar + ai*ai;
    if (m < 1e-60) return _solve_quadratic(br, bi, cr, ci, dr, di, outr, outi);
    /* A=b/a, B=c/a, C=d/a */
    double Ar, Ai; c_div(br, bi, ar, ai, &Ar, &Ai);
    double Br, Bi; c_div(cr, ci, ar, ai, &Br, &Bi);
    double Cr, Ci; c_div(dr, di, ar, ai, &Cr, &Ci);
    /* p = B - A^2/3 */
    double A2r, A2i; c_mul(Ar, Ai, Ar, Ai, &A2r, &A2i);
    double pr = Br - A2r/3, pi_ = Bi - A2i/3;
    /* q = 2A^3/27 - AB/3 + C */
    double A3r, A3i; c_mul(A2r, A2i, Ar, Ai, &A3r, &A3i);
    double ABr, ABi; c_mul(Ar, Ai, Br, Bi, &ABr, &ABi);
    double qr = 2*A3r/27 - ABr/3 + Cr, qi = 2*A3i/27 - ABi/3 + Ci;
    /* delta = (q/2)^2 + (p/3)^3 */
    double q2r, q2i; c_mul(qr/2, qi/2, qr/2, qi/2, &q2r, &q2i);
    double p3r, p3i; c_mul(pr/3, pi_/3, pr/3, pi_/3, &p3r, &p3i);
    c_mul(p3r, p3i, pr/3, pi_/3, &p3r, &p3i);
    double delr = q2r + p3r, deli = q2i + p3i;
    double sdr, sdi; c_powr(delr, deli, 0.5, &sdr, &sdi);
    double ur, ui, vr, vi;
    _cbrt_c(-qr/2 + sdr, -qi/2 + sdi, &ur, &ui);
    /* Enforce Cardano relation: u*v = -p/3. Independent cbrt is wrong for complex. */
    double umag = ur*ur + ui*ui;
    if (umag > 1e-24) {
        c_div(-pr/3, -pi_/3, ur, ui, &vr, &vi);
    } else {
        _cbrt_c(-qr/2 - sdr, -qi/2 - sdi, &vr, &vi);
    }
    double shr = Ar/3, shi = Ai/3;
    outr[0] = ur + vr - shr; outi[0] = ui + vi - shi;
    double t1r, t1i; c_mul(_omega_r, _omega_i, ur, ui, &t1r, &t1i);
    double t2r, t2i; c_mul(_omega2_r, _omega2_i, vr, vi, &t2r, &t2i);
    outr[1] = t1r + t2r - shr; outi[1] = t1i + t2i - shi;
    c_mul(_omega2_r, _omega2_i, ur, ui, &t1r, &t1i);
    c_mul(_omega_r, _omega_i, vr, vi, &t2r, &t2i);
    outr[2] = t1r + t2r - shr; outi[2] = t1i + t2i - shi;
    return 3;
}

static int _solve_quartic(double ar, double ai, double br, double bi,
                          double cr, double ci, double dr_, double di_,
                          double er, double ei, double *outr, double *outi) {
    double m = ar*ar + ai*ai;
    if (m < 1e-60) return _solve_cubic(br, bi, cr, ci, dr_, di_, er, ei, outr, outi);
    /* Monic: A=b/a, B=c/a, C=d/a, D=e/a */
    double Ar, Ai; c_div(br, bi, ar, ai, &Ar, &Ai);
    double Br, Bi; c_div(cr, ci, ar, ai, &Br, &Bi);
    double Cr, Ci; c_div(dr_, di_, ar, ai, &Cr, &Ci);
    double Dr, Di; c_div(er, ei, ar, ai, &Dr, &Di);
    /* Depress: x = y - A/4 => y^4 + p y^2 + q y + r */
    double A2r, A2i; c_mul(Ar, Ai, Ar, Ai, &A2r, &A2i);
    double A3r, A3i; c_mul(A2r, A2i, Ar, Ai, &A3r, &A3i);
    double A4r, A4i; c_mul(A2r, A2i, A2r, A2i, &A4r, &A4i);
    double ABr, ABi; c_mul(Ar, Ai, Br, Bi, &ABr, &ABi);
    double ACr, ACi; c_mul(Ar, Ai, Cr, Ci, &ACr, &ACi);
    double A2Br, A2Bi; c_mul(A2r, A2i, Br, Bi, &A2Br, &A2Bi);
    double pr = Br - 3*A2r/8, pi_ = Bi - 3*A2i/8;
    double qr = Cr - ABr/2 + A3r/8, qi = Ci - ABi/2 + A3i/8;
    double rr = Dr - ACr/4 + A2Br/16 - 3*A4r/256;
    double ri = Di - ACi/4 + A2Bi/16 - 3*A4i/256;
    /* Biquadratic case: q≈0 */
    double qmag = qr*qr + qi*qi;
    double shift_r = Ar/4, shift_i = Ai/4;
    if (qmag < 1e-60) {
        double zr[2], zi[2];
        int nz = _solve_quadratic(1, 0, pr, pi_, rr, ri, zr, zi);
        int n = 0;
        for (int j = 0; j < nz; j++) {
            double sr2, si2; c_powr(zr[j], zi[j], 0.5, &sr2, &si2);
            outr[n] = sr2 - shift_r; outi[n] = si2 - shift_i; n++;
            outr[n] = -sr2 - shift_r; outi[n] = -si2 - shift_i; n++;
        }
        return n;
    }
    /* Resolvent cubic: m^3 - (p/2)m^2 - r*m + (r*p/2 - q^2/8) */
    double rpr, rpi; c_mul(rr, ri, pr, pi_, &rpr, &rpi);
    double q2r2, q2i2; c_mul(qr, qi, qr, qi, &q2r2, &q2i2);
    double rc_r[3], rc_i[3];
    int nrc = _solve_cubic(1, 0, -pr/2, -pi_/2, -rr, -ri,
                           rpr/2 - q2r2/8, rpi/2 - q2i2/8, rc_r, rc_i);
    /* Pick resolvent root with largest magnitude */
    int best = 0; double bestm = 0;
    for (int j = 0; j < nrc; j++) {
        double mm = rc_r[j]*rc_r[j] + rc_i[j]*rc_i[j];
        if (mm > bestm) { bestm = mm; best = j; }
    }
    double mr2 = rc_r[best], mi2 = rc_i[best];
    /* s = sqrt(2m - p) */
    double sr, si; c_powr(2*mr2 - pr, 2*mi2 - pi_, 0.5, &sr, &si);
    double smag = sr*sr + si*si;
    if (smag < 1e-60 && nrc > 0) {
        mr2 = rc_r[0]; mi2 = rc_i[0];
        c_powr(2*mr2 - pr, 2*mi2 - pi_, 0.5, &sr, &si);
    }
    /* Final s guard — if still tiny after fallback, bail */
    smag = sr*sr + si*si;
    if (smag < 1e-60) return 0;
    /* t = -q / (2s) */
    double tr, ti; c_div(-qr, -qi, 2*sr, 2*si, &tr, &ti);
    int n = 0;
    n += _solve_quadratic(1, 0, sr, si, mr2 + tr, mi2 + ti, outr + n, outi + n);
    n += _solve_quadratic(1, 0, -sr, -si, mr2 - tr, mi2 - ti, outr + n, outi + n);
    for (int j = 0; j < n; j++) { outr[j] -= shift_r; outi[j] -= shift_i; }
    return n;
}

/* Helper: sort roots by magnitude, return (smallest, largest) */
static void _roots_minmax(double *rr, double *ri, int n,
                          double *sml_r, double *sml_i, double *lrg_r, double *lrg_i) {
    if (n <= 0) { *sml_r=0; *sml_i=0; *lrg_r=0; *lrg_i=0; return; }
    int imin = 0, imax = 0;
    double mmin = rr[0]*rr[0]+ri[0]*ri[0], mmax = mmin;
    for (int j = 1; j < n; j++) {
        double m = rr[j]*rr[j] + ri[j]*ri[j];
        if (m < mmin) { mmin = m; imin = j; }
        if (m > mmax) { mmax = m; imax = j; }
    }
    *sml_r = rr[imin]; *sml_i = ri[imin];
    *lrg_r = rr[imax]; *lrg_i = ri[imax];
}

/* z01: mirror remix of real parts. t1' = Re(t1) + i*Re(t2), t2' = Re(t2) + i*Re(t1). */
static void pt_z01(double *z1r, double *z1i, double *z2r, double *z2i) {
    double r1 = *z1r, r2 = *z2r;
    *z1r = r1; *z1i = r2;
    *z2r = r2; *z2i = r1;
}

/* sum_prod: t1' = t1+t2, t2' = t1*t2. */
static void pt_sum_prod(double *z1r, double *z1i, double *z2r, double *z2i) {
    double sr = *z1r + *z2r, si = *z1i + *z2i;
    double pr, pi; c_mul(*z1r, *z1i, *z2r, *z2i, &pr, &pi);
    *z1r = sr; *z1i = si; *z2r = pr; *z2i = pi;
}

/* roots2: quadratic root param transform. Solves (9/64)z^2 + t1*z + t2 = 0. */
static void pt_roots2(double *z1r, double *z1i, double *z2r, double *z2i) {
    double rr[2], ri[2];
    int n = _solve_quadratic(9.0 / 64.0, 0.0, *z1r, *z1i, *z2r, *z2i, rr, ri);
    if (n > 1) {
        *z1r = rr[0]; *z1i = ri[0];
        *z2r = rr[1]; *z2i = ri[1];
    } else if (n == 1) {
        *z1r = rr[0]; *z1i = ri[0];
        *z2r = rr[0]; *z2i = ri[0];
    } else {
        *z1r = 0.0; *z1i = 0.0;
        *z2r = 0.0; *z2i = 0.0;
    }
    if (!isfinite(*z1r) || !isfinite(*z1i)) { *z1r = 0.0; *z1i = 0.0; }
    if (!isfinite(*z2r) || !isfinite(*z2i)) { *z2r = 0.0; *z2i = 0.0; }
}

/* roots3: cubic root param transform. a=t1+t2, b=1, c=1, d=t1*t2. */
static void pt_roots3(double *z1r, double *z1i, double *z2r, double *z2i) {
    double ar = *z1r + *z2r, ai = *z1i + *z2i;
    double dr, di; c_mul(*z1r, *z1i, *z2r, *z2i, &dr, &di);
    double rr[3], ri[3];
    int n = _solve_cubic(ar, ai, 1, 0, 1, 0, dr, di, rr, ri);
    double sr, si, lr, li;
    _roots_minmax(rr, ri, n, &sr, &si, &lr, &li);
    *z1r = sr; *z1i = si; *z2r = lr; *z2i = li;
    if (!isfinite(*z1r)||!isfinite(*z1i)) { *z1r=0; *z1i=0; }
    if (!isfinite(*z2r)||!isfinite(*z2i)) { *z2r=0; *z2i=0; }
}

/* roots5: cubic root param transform. a=cos(100t1), b=it1, c=it2, d=sin(100t2). */
static void pt_roots5(double *z1r, double *z1i, double *z2r, double *z2i) {
    double ar, ai; c_cos(100.0*(*z1r), 100.0*(*z1i), &ar, &ai);
    double br = -(*z1i), bi = *z1r;
    double cr = -(*z2i), ci = *z2r;
    double dr, di; c_sin(100.0*(*z2r), 100.0*(*z2i), &dr, &di);
    double rr[3], ri[3];
    int n = _solve_cubic(ar, ai, br, bi, cr, ci, dr, di, rr, ri);
    double sr, si, lr, li;
    _roots_minmax(rr, ri, n, &sr, &si, &lr, &li);
    *z1r = sr; *z1i = si; *z2r = lr; *z2i = li;
    if (!isfinite(*z1r)||!isfinite(*z1i)) { *z1r=0; *z1i=0; }
    if (!isfinite(*z2r)||!isfinite(*z2i)) { *z2r=0; *z2i=0; }
}

/* roots6: quartic root param transform.
 * a=sin(5t1), b=it1, c=(t1-t2)^3+(t1+t2)^2+t1*t2+1, d=it2, e=sin(t2). */
static void pt_roots6(double *z1r, double *z1i, double *z2r, double *z2i) {
    double ar, ai; c_sin(5.0*(*z1r), 5.0*(*z1i), &ar, &ai);
    double br = -(*z1i), bi = *z1r;
    /* c = (t1-t2)^3 + (t1+t2)^2 + t1*t2 + 1 */
    double diffr = *z1r - *z2r, diffi = *z1i - *z2i;
    double sumr = *z1r + *z2r, sumi = *z1i + *z2i;
    double d3r, d3i; c_mul(diffr, diffi, diffr, diffi, &d3r, &d3i);
    c_mul(d3r, d3i, diffr, diffi, &d3r, &d3i); /* (t1-t2)^3 */
    double s2r, s2i; c_mul(sumr, sumi, sumr, sumi, &s2r, &s2i); /* (t1+t2)^2 */
    double t1t2r, t1t2i; c_mul(*z1r, *z1i, *z2r, *z2i, &t1t2r, &t1t2i);
    double cr = d3r + s2r + t1t2r + 1.0, ci = d3i + s2i + t1t2i;
    double dr = -(*z2i), di = *z2r; /* i*t2 */
    double er, ei; c_sin(*z2r, *z2i, &er, &ei);
    double rr[MAX_POLY_ROOTS], ri[MAX_POLY_ROOTS];
    int n = _solve_quartic(ar, ai, br, bi, cr, ci, dr, di, er, ei, rr, ri);
    double sr, si, lr, li;
    _roots_minmax(rr, ri, n, &sr, &si, &lr, &li);
    *z1r = sr; *z1i = si; *z2r = lr; *z2i = li;
    if (!isfinite(*z1r)||!isfinite(*z1i)) { *z1r=0; *z1i=0; }
    if (!isfinite(*z2r)||!isfinite(*z2i)) { *z2r=0; *z2i=0; }
}

/* ==== Parameter transform dispatch (array-of-arrays format) ==== */

#define MAX_PT_ARGS 12

typedef struct {
    char name[64];
    double args[MAX_PT_ARGS];
    char rawArgs[MAX_PT_ARGS][64];
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
                snprintf(entries[count].rawArgs[entries[count].nArgs], sizeof(entries[count].rawArgs[entries[count].nArgs]), "%s", tmp);
                entries[count].args[entries[count].nArgs++] = atof(tmp);
            } else if (*p == '-' || (*p >= '0' && *p <= '9')) {
                /* Bare number */
                int argIdx = entries[count].nArgs;
                int j = 0;
                const char *q = p;
                while (*q && *q != ',' && *q != ']' && j < 63) {
                    entries[count].rawArgs[argIdx][j++] = *q++;
                }
                while (j > 0 && (entries[count].rawArgs[argIdx][j - 1] == ' ' ||
                                 entries[count].rawArgs[argIdx][j - 1] == '\t' ||
                                 entries[count].rawArgs[argIdx][j - 1] == '\n' ||
                                 entries[count].rawArgs[argIdx][j - 1] == '\r')) {
                    j--;
                }
                entries[count].rawArgs[argIdx][j] = '\0';
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

/* ==== Coefficient transform dispatch (string-or-array format) ==== */

/* Parse coeff_transforms: ["rev", ["roots", "8"], ...] */
static int parseCtChain(const char *p, CtEntry *entries, int maxCount) {
    p = skip(p);
    if (*p != '[') return 0;
    p++;
    int count = 0;
    while (count < maxCount) {
        p = skip(p);
        if (*p == ']') break;
        if (*p == ',') { p++; p = skip(p); }

        entries[count].nArgs = 0;
        if (*p == '"') {
            p++;
            int i = 0;
            while (*p && *p != '"' && i < 63) entries[count].name[i++] = *p++;
            entries[count].name[i] = '\0';
            if (*p == '"') p++;
            count++;
            continue;
        }

        if (*p != '[') break;
        p++;
        p = skip(p);
        if (*p != '"') break;
        p++;
        int i = 0;
        while (*p && *p != '"' && i < 63) entries[count].name[i++] = *p++;
        entries[count].name[i] = '\0';
        if (*p == '"') p++;

        while (entries[count].nArgs < MAX_CT_ARGS) {
            p = skip(p);
            if (*p == ']') break;
            if (*p == ',') { p++; p = skip(p); }
            if (*p == '"') {
                p++;
                int j = 0;
                while (*p && *p != '"' && j < 63) entries[count].args[entries[count].nArgs][j++] = *p++;
                entries[count].args[entries[count].nArgs][j] = '\0';
                if (*p == '"') p++;
                entries[count].nArgs++;
            } else if (*p == '-' || (*p >= '0' && *p <= '9')) {
                int j = 0;
                while (*p && *p != ',' && *p != ']' && j < 63) {
                    entries[count].args[entries[count].nArgs][j++] = *p++;
                }
                while (j > 0 && (entries[count].args[entries[count].nArgs][j - 1] == ' ' ||
                                 entries[count].args[entries[count].nArgs][j - 1] == '\t' ||
                                 entries[count].args[entries[count].nArgs][j - 1] == '\n' ||
                                 entries[count].args[entries[count].nArgs][j - 1] == '\r')) {
                    j--;
                }
                entries[count].args[entries[count].nArgs][j] = '\0';
                entries[count].nArgs++;
            } else {
                break;
            }
        }
        p = skip(p);
        if (*p == ']') p++;
        count++;
    }
    return count;
}

static int pt_arg_complex(const PtEntry *e, int idx, double fallbackRe, double fallbackIm,
                          double *outRe, double *outIm) {
    if (!outRe || !outIm) return 0;
    if (!e || idx < 0 || idx >= e->nArgs) {
        *outRe = fallbackRe;
        *outIm = fallbackIm;
        return 1;
    }
    const char *raw = e->rawArgs[idx];
    if (!raw || !*raw) {
        *outRe = fallbackRe;
        *outIm = fallbackIm;
        return 1;
    }
    if (ct_parse_complex_literal(raw, outRe, outIm)) return 1;
    if (ct_parse_double_expr(raw, outRe)) {
        *outIm = 0.0;
        return 1;
    }
    return 0;
}

/* Dispatch a single param transform entry */
static int dispatchPt(const PtEntry *e, double *z1r, double *z1i, double *z2r, double *z2i, int gridN) {
    if (strcmp(e->name, "sdith") == 0) {
        double d = e->nArgs > 0 ? e->args[0] : 1.0;
        pt_sdith(z1r, z1i, z2r, z2i, d, gridN);
        return 0;
    }
    if (strcmp(e->name, "ddith") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 2;
        double d = e->nArgs > 1 ? e->args[1] : 1.0;
        double ex = e->nArgs > 2 ? e->args[2] : 0.5;
        pt_ddith(z1r, z1i, z2r, z2i, n, d, ex, gridN);
        return 0;
    }
    if (strcmp(e->name, "ndith") == 0) {
        double d = e->nArgs > 0 ? e->args[0] : 1.0;
        pt_ndith(z1r, z1i, z2r, z2i, d, gridN);
        return 0;
    }
    if (strcmp(e->name, "adth") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 2;
        double d = e->nArgs > 1 ? e->args[1] : 1.0;
        double inner = e->nArgs > 2 ? e->args[2] : 0.4;
        pt_adth(z1r, z1i, z2r, z2i, n, d, inner, gridN);
        return 0;
    }
    if (strcmp(e->name, "ldth") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 2;
        double d = e->nArgs > 1 ? e->args[1] : 1.0;
        double len = e->nArgs > 2 ? e->args[2] : 1.0;
        double angle = e->nArgs > 3 ? e->args[3] : 0.0;
        pt_ldth(z1r, z1i, z2r, z2i, n, d, len, angle, gridN);
        return 0;
    }
    if (strcmp(e->name, "crdth") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 2;
        double d = e->nArgs > 1 ? e->args[1] : 1.0;
        pt_crdth(z1r, z1i, z2r, z2i, n, d, gridN);
        return 0;
    }
    if (strcmp(e->name, "scdth") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 2;
        double d = e->nArgs > 1 ? e->args[1] : 1.0;
        double half_ap = e->nArgs > 2 ? e->args[2] : 0.25;
        double center = e->nArgs > 3 ? e->args[3] : 0.0;
        pt_scdth(z1r, z1i, z2r, z2i, n, d, half_ap, center, gridN);
        return 0;
    }
    /* t1radd(v): add v to real part of t1 only */
    if (strcmp(e->name, "t1radd") == 0) {
        double v = e->nArgs > 0 ? e->args[0] : 0.0;
        *z1r += v;
        return 0;
    }
    /* t1iadd(v): add v to imaginary part of t1 only */
    if (strcmp(e->name, "t1iadd") == 0) {
        double v = e->nArgs > 0 ? e->args[0] : 0.0;
        *z1i += v;
        return 0;
    }
    /* t2radd(v): add v to real part of t2 only */
    if (strcmp(e->name, "t2radd") == 0) {
        double v = e->nArgs > 0 ? e->args[0] : 0.0;
        *z2r += v;
        return 0;
    }
    /* t2iadd(v): add v to imaginary part of t2 only */
    if (strcmp(e->name, "t2iadd") == 0) {
        double v = e->nArgs > 0 ? e->args[0] : 0.0;
        *z2i += v;
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
    /* add(v): legacy scalar add-to-all-components.
     * add(c1, c2): z1 += c1, z2 += c2 with complex literals. */
    if (strcmp(e->name, "add") == 0) {
        if (e->nArgs >= 2) {
            double c1r, c1i, c2r, c2i;
            if (!pt_arg_complex(e, 0, 0.0, 0.0, &c1r, &c1i) ||
                !pt_arg_complex(e, 1, 0.0, 0.0, &c2r, &c2i)) {
                fprintf(stderr, "Invalid add param transform offset\n");
                return 1;
            }
            *z1r += c1r; *z1i += c1i;
            *z2r += c2r; *z2i += c2i;
            return 0;
        }
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
    /* rtheta(p): z1 = pow(x1,p)*exp(2*pi*x2*i), z2 = pow(x2,p)*exp(2*pi*x1*i) */
    if (strcmp(e->name, "rtheta") == 0) {
        double p = e->nArgs > 0 ? e->args[0] : 1.0;
        int n = pt_target_value(e->args, e->nArgs, 1, 2);
        pt_rtheta_target(z1r, z1i, z2r, z2i, n, p);
        return 0;
    }
    if (strcmp(e->name, "moebius") == 0) {
        if (e->nArgs <= 0) {
            pt_moebius(z1r, z1i, z2r, z2i);
            return 0;
        }
        double ar, ai, br, bi, cr, ci, dr, di;
        if (!pt_arg_complex(e, 0, 1.0, 0.0, &ar, &ai) ||
            !pt_arg_complex(e, 1, 0.0, 0.0, &br, &bi) ||
            !pt_arg_complex(e, 2, 0.0, 0.0, &cr, &ci) ||
            !pt_arg_complex(e, 3, 1.0, 0.0, &dr, &di)) {
            fprintf(stderr, "Invalid moebius param transform coefficient\n");
            return 1;
        }
        pt_moebius_abcd(z1r, z1i, z2r, z2i, ar, ai, br, bi, cr, ci, dr, di);
        return 0;
    }
    if (strcmp(e->name, "inv_t_plus_2") == 0) {
        double re1 = e->nArgs > 0 ? e->args[0] : 2.0;
        double im1 = e->nArgs > 1 ? e->args[1] : 0.0;
        double re2 = e->nArgs > 2 ? e->args[2] : 2.0;
        double im2 = e->nArgs > 3 ? e->args[3] : 0.0;
        pt_inv_t_plus_2(z1r, z1i, z2r, z2i, re1, im1, re2, im2);
        return 0;
    }
    if (strcmp(e->name, "crd") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 0;
        double size = e->nArgs > 1 ? e->args[1] : 1.0;
        pt_crd(z1r, z1i, z2r, z2i, n, size);
        return 0;
    }
    if (strcmp(e->name, "hrt") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 0;
        double size = e->nArgs > 1 ? e->args[1] : 1.0;
        double turns = e->nArgs > 2 ? e->args[2] : 0.0;
        pt_hrt(z1r, z1i, z2r, z2i, n, size, turns);
        return 0;
    }
    if (strcmp(e->name, "spdl") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 0;
        double va = e->nArgs > 1 ? e->args[1] : 0.5;
        double vb = e->nArgs > 2 ? e->args[2] : 0.2;
        double vp = e->nArgs > 3 ? e->args[3] : 1.5;
        pt_spdl(z1r, z1i, z2r, z2i, n, va, vb, vp);
        return 0;
    }
    if (strcmp(e->name, "lmc") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 0;
        double a = e->nArgs > 1 ? e->args[1] : 0.3;
        double b = e->nArgs > 2 ? e->args[2] : 0.5;
        pt_lmc(z1r, z1i, z2r, z2i, n, a, b);
        return 0;
    }
    if (strcmp(e->name, "rsc") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 0;
        double amp = e->nArgs > 1 ? e->args[1] : 0.5;
        double k = e->nArgs > 2 ? e->args[2] : 2.0;
        pt_rsc(z1r, z1i, z2r, z2i, n, amp, k);
        return 0;
    }
    if (strcmp(e->name, "lss") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 0;
        double A = e->nArgs > 1 ? e->args[1] : 0.5;
        double B = e->nArgs > 2 ? e->args[2] : 0.5;
        double a = e->nArgs > 3 ? e->args[3] : 3.0;
        double b = e->nArgs > 4 ? e->args[4] : 2.0;
        double phase = e->nArgs > 5 ? e->args[5] : 0.5;
        pt_lss(z1r, z1i, z2r, z2i, n, A, B, a, b, phase);
        return 0;
    }
    if (strcmp(e->name, "ast") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 0;
        double scale = e->nArgs > 1 ? e->args[1] : 1.0;
        pt_ast(z1r, z1i, z2r, z2i, n, scale);
        return 0;
    }
    if (strcmp(e->name, "asp") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 0;
        double a = e->nArgs > 1 ? e->args[1] : 0.0;
        double b = e->nArgs > 2 ? e->args[2] : 0.1;
        pt_asp(z1r, z1i, z2r, z2i, n, a, b);
        return 0;
    }
    if (strcmp(e->name, "lsp") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 0;
        double a = e->nArgs > 1 ? e->args[1] : 0.1;
        double b = e->nArgs > 2 ? e->args[2] : 0.15;
        pt_lsp(z1r, z1i, z2r, z2i, n, a, b);
        return 0;
    }
    if (strcmp(e->name, "dlt") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 0;
        double R = e->nArgs > 1 ? e->args[1] : 1.0;
        pt_dlt(z1r, z1i, z2r, z2i, n, R);
        return 0;
    }
    if (strcmp(e->name, "rply") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 0;
        double sides = e->nArgs > 1 ? e->args[1] : 5.0;
        double radius = e->nArgs > 2 ? e->args[2] : 1.0;
        double turns = e->nArgs > 3 ? e->args[3] : 0.0;
        pt_rply(z1r, z1i, z2r, z2i, n, sides, radius, turns);
        return 0;
    }
    if (strcmp(e->name, "star") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 0;
        double points = e->nArgs > 1 ? e->args[1] : 5.0;
        double outer = e->nArgs > 2 ? e->args[2] : 1.0;
        double inner_ratio = e->nArgs > 3 ? e->args[3] : 0.5;
        pt_star(z1r, z1i, z2r, z2i, n, points, outer, inner_ratio);
        return 0;
    }
    if (strcmp(e->name, "rect") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 0;
        double w = e->nArgs > 1 ? e->args[1] : 2.0;
        double h = e->nArgs > 2 ? e->args[2] : 1.0;
        double turns = e->nArgs > 3 ? e->args[3] : 0.0;
        pt_rect(z1r, z1i, z2r, z2i, n, w, h, turns);
        return 0;
    }
    if (strcmp(e->name, "rrect") == 0) {
        int n = e->nArgs > 0 ? (int)e->args[0] : 0;
        double w = e->nArgs > 1 ? e->args[1] : 2.0;
        double h = e->nArgs > 2 ? e->args[2] : 1.0;
        double m = e->nArgs > 3 ? e->args[3] : 4.0;
        pt_rrect(z1r, z1i, z2r, z2i, n, w, h, m);
        return 0;
    }
    if (strcmp(e->name, "z01") == 0) {
        pt_z01(z1r, z1i, z2r, z2i);
        return 0;
    }
    if (strcmp(e->name, "sum_prod") == 0) {
        pt_sum_prod(z1r, z1i, z2r, z2i);
        return 0;
    }
    if (strcmp(e->name, "roots2") == 0) {
        pt_roots2(z1r, z1i, z2r, z2i);
        return 0;
    }
    if (strcmp(e->name, "roots3") == 0) {
        pt_roots3(z1r, z1i, z2r, z2i);
        return 0;
    }
    if (strcmp(e->name, "roots5") == 0) {
        pt_roots5(z1r, z1i, z2r, z2i);
        return 0;
    }
    if (strcmp(e->name, "roots6") == 0) {
        pt_roots6(z1r, z1i, z2r, z2i);
        return 0;
    }
    if (pt_is_targetable_independent(e->name)) {
        ParamTransform fn = lookupParamTransform(e->name);
        int n = pt_target_value(e->args, e->nArgs, 0, 2);
        pt_apply_independent_target(fn, z1r, z1i, z2r, z2i, n);
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

/* CoeffFuncC: coefficient function ABI.
 * (x1r, x1i, x2r, x2i, cfpv, n_cfpv, cRe, cIm, nCoeffs) */
typedef void (*CoeffFuncC)(double, double, double, double,
                           const double*, int,
                           double*, double*, int*);

#define MAX_CFPV 16

#define WRAP_OLD(fname) \
    static void fname##_c(double x1r, double x1i, double x2r, double x2i, \
                          const double *cfpv, int n_cfpv, \
                          double *cRe, double *cIm, int *nCoeffs) { \
        (void)x1i; (void)x2i; (void)cfpv; (void)n_cfpv; \
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
#include "poly_hand_batch2.h"
#include "poly_hand_batch3.h"
#include "poly_hand_batch4.h"
#include "poly_hand_batch5.h"
#include "poly_hand_batch6.h"
#include "poly_hand_batch7.h"
#include "poly_hand_300a.h"
#include "poly_hand_300b.h"
#include "poly_hand_300c.h"
#include "poly_hand_300d.h"
#include "poly_hand_300e.h"

#include "poly_generated_funcs.h"
#include "poly_generated_200.c"
#include "poly_generated_200_funcs.h"
/* poly_generated_300.c not included — all 100 functions hand-written in poly_hand_300[a-e].h */
#include "poly_generated_400.c"
#include "poly_generated_400_funcs.h"
#include "poly_generated_500.c"
#include "poly_generated_500_funcs.h"
#include "poly_generated_600.c"
#include "poly_generated_600_funcs.h"
#include "poly_generated_700.c"
#include "poly_generated_700_funcs.h"
#include "poly_generated_800.c"
#include "poly_generated_800_funcs.h"
#include "poly_generated_900.c"
#include "poly_generated_900_funcs.h"

/* p821: hand-written, sequential cf[k] = cf[k-1] * ... with conditionals */
static void p821_c(double x1r, double x1i, double x2r, double x2i,
                   const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 25;
    for (int i = 0; i < 25; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* cf[0] = 3*t1 + 5j*t2 */
    cRe[0] = 3.0*x1r - 5.0*x2i;
    cIm[0] = 3.0*x1i + 5.0*x2r;

    double mod_t1 = sqrt(x1r*x1r + x1i*x1i);
    double arg_t2 = atan2(x2i, x2r);
    double mr = mod_t1 + arg_t2;  /* real scalar */

    for (int k = 1; k < 25; k++) {
        /* cf[k] = cf[k-1] * (mod_t1 + arg_t2)  — real scalar multiply */
        cRe[k] = cRe[k-1] * mr;
        cIm[k] = cIm[k-1] * mr;

        /* if cf[k].real < 0 and cf[k].imag < 0: cf[k] = conj(cf[k]) */
        if (cRe[k] < 0 && cIm[k] < 0) {
            cIm[k] = -cIm[k];
        }

        /* if |cf[k].real| > 10: cf[k] = cf[k] / mod_t1 */
        if (fabs(cRe[k]) > 10.0 && mod_t1 > 1e-30) {
            cRe[k] /= mod_t1;
            cIm[k] /= mod_t1;
        }

        /* if |cf[k].imag| > 10: cf[k] = cf[k] / (1j * arg_t2) */
        /* 1j * arg_t2 = (0 + arg_t2*j), so dividing by it:
           (a+bi)/(0+ci) = (a+bi)*(-ci)/(c²) = (b/c) + (-a/c)i  when c=arg_t2 */
        if (fabs(cIm[k]) > 10.0 && fabs(arg_t2) > 1e-30) {
            double a = cRe[k], b = cIm[k];
            double c = arg_t2;
            cRe[k] = b / c;
            cIm[k] = -a / c;
        }
    }

    /* NaN guard */
    for (int i = 0; i < 25; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* moth4: 50 coefficients, sequential with complex sin/cos normalization */
static void moth4_c(double x1r, double x1i, double x2r, double x2i,
                    const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 50;
    for (int i = 0; i < 50; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* cf[0] = t1 + t2 */
    double sumr = x1r + x2r, sumi = x1i + x2i;
    cRe[0] = sumr; cIm[0] = sumi;

    for (int k = 1; k < 50; k++) {
        /* v = sin(((k+4)%10) * cf[k-1]) + cos((k%10) * t1) */
        int idx1 = (k + 4) % 10;
        int idx2 = k % 10;

        /* idx1 * cf[k-1] */
        double mr = (double)idx1 * cRe[k-1], mi = (double)idx1 * cIm[k-1];
        /* sin(mr + mi*i) */
        double sr, si;
        c_sin(mr, mi, &sr, &si);

        /* idx2 * t1 */
        double cr2 = (double)idx2 * x1r, ci2 = (double)idx2 * x1i;
        /* cos(cr2 + ci2*i) */
        double cosr, cosi;
        c_cos(cr2, ci2, &cosr, &cosi);

        /* v = sin_result + cos_result */
        double vr = sr + cosr, vi = si + cosi;

        /* av = |v| */
        double av = sqrt(vr * vr + vi * vi);

        if (isfinite(av) && av > 1e-10) {
            /* cf[k] = v / av (normalize to unit circle) */
            cRe[k] = vr / av;
            cIm[k] = vi / av;
        } else {
            /* fallback: cf[k] = t1 + t2 */
            cRe[k] = sumr;
            cIm[k] = sumi;
        }
    }

    /* NaN guard */
    for (int i = 0; i < 50; i++) {
        if (!isfinite(cRe[i]) || !isfinite(cIm[i])) { cRe[i] = 0; cIm[i] = 0; }
    }
}

/* p11b2: 71 coefficients, cubic ramp over normalized denominator. */
static void p11a_c(double x1r, double x1i, double x2r, double x2i,
                   const double *cfpv, int n_cfpv,
                   double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    const int n = 40;
    *nCoeffs = n;

    double sumr = x1r + x2r;
    double sumi = x1i + x2i;
    double a = sqrt(sumr * sumr + sumi * sumi) / 2.0;
    int m = ((int)(10.0 * a)) % 13 + 3;
    double denr = (double)(m > 3 ? m : 3) + sumr;
    double deni = sumi;
    double d2 = denr * denr + deni * deni;
    if (d2 < 1e-30) d2 = 1e-30;
    int mod_base = m * 2;

    for (int k = 0; k < n; k++) {
        double vr = (double)k * denr / d2;
        double vi = -(double)k * deni / d2;
        double er = -M_PI * vi;
        double ei = M_PI * vr;
        double scale = exp(er);
        double ucr = scale * cos(ei);
        double uci = scale * sin(ei);
        double coeff_scale = mod_base > 0 ? (double)(k % mod_base) : 0.0;
        cRe[k] = coeff_scale * ucr;
        cIm[k] = coeff_scale * uci;
    }

    for (int k = 0; k < n; k++) {
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) { cRe[k] = 0.0; cIm[k] = 0.0; }
    }
}

static void p11a_v1_c(double x1r, double x1i, double x2r, double x2i,
                      const double *cfpv, int n_cfpv,
                      double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    const int n = 40;
    *nCoeffs = n;

    double sumr = x1r + x2r;
    double sumi = x1i + x2i;
    double a = sqrt(sumr * sumr + sumi * sumi) / 2.0;
    int m = ((int)(5.0 * a)) % 21 + 3;
    double denr = (double)(m > 3 ? m : 3) + sumr;
    double deni = sumi;
    double d2 = denr * denr + deni * deni;
    if (d2 < 1e-30) d2 = 1e-30;
    int mod_base = m + 10;

    for (int k = 0; k < n; k++) {
        double v0 = (double)k / (double)(n - 1);
        double numer = pow(v0, 0.75);
        double vr = numer * denr / d2;
        double vi = -numer * deni / d2;
        double er = -50.0 * M_PI * vi;
        double ei = 50.0 * M_PI * vr;
        double scale = exp(er);
        double ucr = scale * cos(ei);
        double uci = scale * sin(ei);
        double coeff_scale = mod_base > 0 ? (double)(k % mod_base) : 0.0;
        cRe[k] = coeff_scale * ucr;
        cIm[k] = coeff_scale * uci;
    }

    for (int k = 0; k < n; k++) {
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) { cRe[k] = 0.0; cIm[k] = 0.0; }
    }
}

static void p11a1_c(double x1r, double x1i, double x2r, double x2i,
                    const double *cfpv, int n_cfpv,
                    double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    const int n = 40;
    *nCoeffs = n;

    double sumr = x1r + x2r;
    double sumi = x1i + x2i;
    double a = sqrt(sumr * sumr + sumi * sumi) / 2.0;
    int m = ((int)(10.0 * a)) % 13;
    double denr = (double)(m + 3) + sumr;
    double deni = sumi;
    double d2 = denr * denr + deni * deni;
    if (d2 < 1e-30) d2 = 1e-30;
    int mod_base = m * 2;

    for (int k = 0; k < n; k++) {
        double v0 = (double)k / (double)(n - 1);
        double numer = (double)n * v0;
        double vr = numer * denr / d2;
        double vi = -numer * deni / d2;
        double er = -M_PI * vi;
        double ei = M_PI * vr;
        double scale = exp(er);
        double ucr = scale * cos(ei);
        double uci = scale * sin(ei);
        double coeff_scale = mod_base > 0 ? (double)(k % mod_base) : 0.0;
        cRe[k] = coeff_scale * ucr;
        cIm[k] = coeff_scale * uci;
    }

    for (int k = 0; k < n; k++) {
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) { cRe[k] = 0.0; cIm[k] = 0.0; }
    }
}

static void p11a2_c(double x1r, double x1i, double x2r, double x2i,
                    const double *cfpv, int n_cfpv,
                    double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    const int n = 40;
    *nCoeffs = n;

    double sumr = x1r + x2r;
    double sumi = x1i + x2i;
    double a = sqrt(sumr * sumr + sumi * sumi) / 2.0;
    int m = ((int)(101.0 * a)) % 17;
    double denr = (double)(m + 3) + sumr;
    double deni = sumi;
    double d2 = denr * denr + deni * deni;
    if (d2 < 1e-30) d2 = 1e-30;
    int mod_base = m * 2;

    for (int k = 0; k < n; k++) {
        double v0 = (double)k / (double)(n - 1);
        double numer = (double)n * v0;
        double vr = numer * denr / d2;
        double vi = -numer * deni / d2;
        double er = -M_PI * vi;
        double ei = M_PI * vr;
        double scale = exp(er);
        double ucr = scale * cos(ei);
        double uci = scale * sin(ei);
        double coeff_scale = mod_base > 0 ? (double)(k % mod_base) : 0.0;
        cRe[k] = coeff_scale * ucr;
        cIm[k] = coeff_scale * uci;
    }

    for (int k = 0; k < n; k++) {
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) { cRe[k] = 0.0; cIm[k] = 0.0; }
    }
}

/* p11b2: 71 coefficients, cubic ramp over normalized denominator. */
static void p11b2_c(double x1r, double x1i, double x2r, double x2i,
                    const double *cfpv, int n_cfpv,
                    double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    const int n = 71;
    *nCoeffs = n;

    double sumr = x1r + x2r;
    double sumi = x1i + x2i;
    double denr = sumr + 3.0;
    double deni = sumi;
    double adenom = sqrt(denr * denr + deni * deni);
    if (adenom < 1.0 && adenom > 1e-30) {
        denr /= adenom;
        deni /= adenom;
    }

    double d2 = denr * denr + deni * deni;
    if (d2 < 1e-30) d2 = 1e-30;

    int mod_base = ((int)(101.0 * sqrt(sumr * sumr + sumi * sumi))) % 51 + 1;
    for (int k = 0; k < n; k++) {
        double v = (double)k / (double)(n - 1);
        double numer = 7.0 * (double)n * pow(v, 3.0);
        double ur = numer * denr / d2;
        double ui = -numer * deni / d2;

        double er = -M_PI * ui;
        double ei = M_PI * ur;
        double scale = exp(er);
        double ucr = scale * cos(ei);
        double uci = scale * sin(ei);

        double coeff_scale = (double)(k % mod_base);
        cRe[k] = coeff_scale * ucr;
        cIm[k] = coeff_scale * uci;
    }

    for (int k = 0; k < n; k++) {
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) { cRe[k] = 0.0; cIm[k] = 0.0; }
    }
}

/* p11b2_v1: 71 coefficients, power-15 ramp over normalized denominator. */
static void p11b2_v1_c(double x1r, double x1i, double x2r, double x2i,
                       const double *cfpv, int n_cfpv,
                       double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    const int n = 71;
    *nCoeffs = n;

    double sumr = x1r + x2r;
    double sumi = x1i + x2i;
    double denr = sumr + 3.0;
    double deni = sumi;
    double adenom = sqrt(denr * denr + deni * deni);
    if (adenom < 1.0 && adenom > 1e-30) {
        denr /= adenom;
        deni /= adenom;
    }

    double d2 = denr * denr + deni * deni;
    if (d2 < 1e-30) d2 = 1e-30;

    int mod_base = ((int)(4583.0 * sqrt(sumr * sumr + sumi * sumi))) % 71 + 1;
    for (int k = 0; k < n; k++) {
        double v = (double)k / (double)(n - 1);
        double numer = 7.0 * (double)n * pow(v, 15.0);
        double ur = numer * denr / d2;
        double ui = -numer * deni / d2;

        double er = -M_PI * ui;
        double ei = M_PI * ur;
        double scale = exp(er);
        double ucr = scale * cos(ei);
        double uci = scale * sin(ei);

        int sf = ((k + 1) % mod_base);
        double coeff_scale = (double)(sf + 1);
        cRe[k] = coeff_scale * ucr;
        cIm[k] = coeff_scale * uci;
    }

    for (int k = 0; k < n; k++) {
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) { cRe[k] = 0.0; cIm[k] = 0.0; }
    }
}

/* giga_11a: 25 coefficients, powers of t1+t2 divided by index. */
static void giga_11a_c(double x1r, double x1i, double x2r, double x2i,
                       const double *cfpv, int n_cfpv,
                       double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    const int n = 25;
    *nCoeffs = n;

    double br = x1r + x2r;
    double bi = x1i + x2i;
    double pr = br;
    double pi = bi;

    for (int k = 1; k <= n; k++) {
        cRe[k - 1] = pr / (double)k;
        cIm[k - 1] = pi / (double)k;
        if (k < n) {
            double nr = pr * br - pi * bi;
            double ni = pr * bi + pi * br;
            pr = nr;
            pi = ni;
        }
    }

    for (int k = 0; k < n; k++) {
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) { cRe[k] = 0.0; cIm[k] = 0.0; }
    }
}

/* giga_11b: 40 coefficients, modular scale over exponential reciprocal ramp. */
static void giga_11b_c(double x1r, double x1i, double x2r, double x2i,
                       const double *cfpv, int n_cfpv,
                       double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    const int n = 40;
    *nCoeffs = n;

    double sumr = x1r + x2r;
    double sumi = x1i + x2i;
    int m = ((int)fmod(5.0 * sqrt(sumr * sumr + sumi * sumi), 17.0)) + 1;
    if (m < 1) m = 1;

    for (int k = 0; k < n; k++) {
        double scale_factor = (double)(k % m);
        double denr = sumr + (double)m;
        double deni = sumi;
        double d2 = denr * denr + deni * deni;
        if (d2 < 1e-30) d2 = 1e-30;

        double numer = (double)(k + 1);
        double ur = numer * denr / d2;
        double ui = -numer * deni / d2;

        double er = -M_PI * ui;
        double ei = M_PI * ur;
        double escale = exp(er);
        double expr = escale * cos(ei);
        double expi = escale * sin(ei);
        cRe[k] = scale_factor * expr;
        cIm[k] = scale_factor * expi;
    }

    for (int k = 0; k < n; k++) {
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) { cRe[k] = 0.0; cIm[k] = 0.0; }
    }
}

/* p11b3: 11 coefficients, array mutation with integer indexing */
static void p11b3_c(double x1r, double x1i, double x2r, double x2i,
                    const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 11;
    /* t = t1 + t2 */
    double tr = x1r + x2r, ti = x1i + x2i;
    /* a = |t1+t2| / 2 */
    double a = sqrt(tr * tr + ti * ti) / 2.0;
    /* m = int(a * 251) % 37 */
    int m = ((int)(a * 251.0)) % 37;
    /* v[k] = (k+1) / (t + 4) for k=0..10 */
    double vr[11], vi[11];
    double dr = tr + 4.0, di = ti;
    double d2 = dr * dr + di * di;
    if (d2 < 1e-30) d2 = 1e-30;
    for (int k = 0; k < 11; k++) {
        double nr = (double)(k + 1);
        /* (k+1) / (t+4) — real numerator, complex denominator */
        vr[k] = nr * dr / d2;
        vi[k] = -nr * di / d2;
    }
    /* p1 = int(7 * a * 11) % 11; v[p1] = (p1+1) / (t + |t|*2 + 1 + m) */
    double abst = sqrt(tr * tr + ti * ti);
    int p1 = ((int)(7.0 * a * 11.0)) % 11;
    {
        double denr = tr + abst * 2.0 + 1.0 + m, deni = ti;
        double dd = denr * denr + deni * deni;
        if (dd < 1e-30) dd = 1e-30;
        double nr = (double)(p1 + 1);
        vr[p1] = nr * denr / dd;
        vi[p1] = -nr * deni / dd;
    }
    /* p2 = int(619 * a * 11) % 11; v[p2] = (p2+1) / (t + |t|*2 + 1 + m//2) */
    int p2 = ((int)(619.0 * a * 11.0)) % 11;
    {
        double denr = tr + abst * 2.0 + 1.0 + (m / 2), deni = ti;
        double dd = denr * denr + deni * deni;
        if (dd < 1e-30) dd = 1e-30;
        double nr = (double)(p2 + 1);
        vr[p2] = nr * denr / dd;
        vi[p2] = -nr * deni / dd;
    }
    /* cf = exp(i * pi * v) */
    for (int k = 0; k < 11; k++) {
        /* i*pi*v = -pi*vi + i*pi*vr */
        double er = -M_PI * vi[k], ei = M_PI * vr[k];
        double e = exp(er);
        cRe[k] = e * cos(ei);
        cIm[k] = e * sin(ei);
    }
    /* NaN guard */
    for (int k = 0; k < 11; k++) {
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) { cRe[k] = 0; cIm[k] = 0; }
    }
}

/* p11b2_v2: 11 coefficients, unit-interval power ramp over normalized denominator. */
static void p11b2_v2_c(double x1r, double x1i, double x2r, double x2i,
                       const double *cfpv, int n_cfpv,
                       double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    const int n = 11;
    *nCoeffs = n;

    /* denom = t1 + t2 + 3 */
    double sumr = x1r + x2r;
    double sumi = x1i + x2i;
    double denr = sumr + 3.0;
    double deni = sumi;
    double adenom = sqrt(denr * denr + deni * deni);
    if (adenom < 1.0 && adenom > 1e-30) {
        denr /= adenom;
        deni /= adenom;
    }

    double d2 = denr * denr + deni * deni;
    if (d2 < 1e-30) d2 = 1e-30;

    int mod_base = ((int)(4583.0 * sqrt(sumr * sumr + sumi * sumi))) % 11 + 1;
    for (int k = 0; k < n; k++) {
        double v = (double)k / (double)(n - 1); /* linspace(0,1,11) */
        double numer = 7.0 * (double)n * pow(v, 15.0);

        /* u = numer / denom */
        double ur = numer * denr / d2;
        double ui = -numer * deni / d2;

        /* uc = exp(i * pi * u) */
        double er = -M_PI * ui;
        double ei = M_PI * ur;
        double scale = exp(er);
        double ucr = scale * cos(ei);
        double uci = scale * sin(ei);

        int sf = ((k + 1) % mod_base);
        double coeff_scale = (double)(sf + 1);
        cRe[k] = coeff_scale * ucr;
        cIm[k] = coeff_scale * uci;
    }

    for (int k = 0; k < n; k++) {
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) { cRe[k] = 0.0; cIm[k] = 0.0; }
    }
}

/* p11b2_v3: 11 coefficients, complex-cosine envelope with +0.5i shift. */
static void p11b2_v3_c(double x1r, double x1i, double x2r, double x2i,
                       const double *cfpv, int n_cfpv,
                       double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    const int n = 11;
    *nCoeffs = n;

    double sumr = x1r + x2r;
    double sumi = x1i + x2i;
    double denr = sumr + 3.0;
    double deni = sumi;
    double adenom = sqrt(denr * denr + deni * deni);
    if (adenom < 1.0 && adenom > 1e-30) {
        denr /= adenom;
        deni /= adenom;
    }

    double d2 = denr * denr + deni * deni;
    if (d2 < 1e-30) d2 = 1e-30;

    for (int k = 0; k < n; k++) {
        double v = (double)k / (double)(n - 1);
        double numer = 7.0 * (double)n * pow(v, 15.0);
        double ur = numer * denr / d2;
        double ui = -numer * deni / d2;

        double er = -M_PI * ui;
        double ei = M_PI * ur;
        double scale = exp(er);
        double ucr = scale * cos(ei);
        double uci = scale * sin(ei);

        double a = 2.0 * M_PI * v;
        double b = M_PI;
        double zr = cos(a) * cosh(b);
        double zi = -sin(a) * sinh(b);
        double z2r = zr * zr - zi * zi;
        double z2i = 2.0 * zr * zi;
        double z3r = z2r * zr - z2i * zi;
        double z3i = z2r * zi + z2i * zr;
        double sfr = z3r * (double)(n - 1);
        double sfi = z3i * (double)(n - 1);
        double coeffr = sfr + 1.0;
        double coeffi = sfi;
        cRe[k] = coeffr * ucr - coeffi * uci;
        cIm[k] = coeffr * uci + coeffi * ucr;
    }

    for (int k = 0; k < n; k++) {
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) { cRe[k] = 0.0; cIm[k] = 0.0; }
    }
}

/* p11b2_v4: 11 coefficients, complex-cosine envelope with -0.5i shift. */
static void p11b2_v4_c(double x1r, double x1i, double x2r, double x2i,
                       const double *cfpv, int n_cfpv,
                       double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    const int n = 11;
    *nCoeffs = n;

    double sumr = x1r + x2r;
    double sumi = x1i + x2i;
    double denr = sumr + 3.0;
    double deni = sumi;
    double adenom = sqrt(denr * denr + deni * deni);
    if (adenom < 1.0 && adenom > 1e-30) {
        denr /= adenom;
        deni /= adenom;
    }

    double d2 = denr * denr + deni * deni;
    if (d2 < 1e-30) d2 = 1e-30;

    for (int k = 0; k < n; k++) {
        double v = (double)k / (double)(n - 1);
        double numer = 7.0 * (double)n * pow(v, 15.0);
        double ur = numer * denr / d2;
        double ui = -numer * deni / d2;

        double er = -M_PI * ui;
        double ei = M_PI * ur;
        double scale = exp(er);
        double ucr = scale * cos(ei);
        double uci = scale * sin(ei);

        double a = 2.0 * M_PI * v;
        double b = -M_PI;
        double zr = cos(a) * cosh(b);
        double zi = -sin(a) * sinh(b);
        double z2r = zr * zr - zi * zi;
        double z2i = 2.0 * zr * zi;
        double z3r = z2r * zr - z2i * zi;
        double z3i = z2r * zi + z2i * zr;
        double sfr = z3r * (double)(n - 1);
        double sfi = z3i * (double)(n - 1);
        double coeffr = sfr + 1.0;
        double coeffi = sfi;
        cRe[k] = coeffr * ucr - coeffi * uci;
        cIm[k] = coeffr * uci + coeffi * ucr;
    }

    for (int k = 0; k < n; k++) {
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) { cRe[k] = 0.0; cIm[k] = 0.0; }
    }
}

/* p11b2_v5: 11 coefficients, exponential line interpolation family.
 * This is the same underlying construction as giga_138, exposed under its older name. */
static void p11b2_v5_c(double x1r, double x1i, double x2r, double x2i,
                       const double *cfpv, int n_cfpv,
                       double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    const int n = 11;
    *nCoeffs = n;

    double denr = x1r + x2r + 3.0;
    double deni = x1i + x2i;
    double adenom = sqrt(denr * denr + deni * deni);
    if (adenom < 1.0 && adenom > 1e-30) {
        denr /= adenom;
        deni /= adenom;
    }
    double d2 = denr * denr + deni * deni;
    if (d2 < 1e-30) d2 = 1e-30;

    double z1r = x1r, z1i = x1i;
    double z2r = x1r + x2r, z2i = x1i + x2i;
    double step1r = (x2r - x1r) / (double)(n - 1);
    double step1i = (x2i - x1i) / (double)(n - 1);
    double prod_r = x1r * x2r - x1i * x2i;
    double prod_i = x1r * x2i + x1i * x2r;
    double step2r = (prod_r - z2r) / (double)(n - 1);
    double step2i = (prod_i - z2i) / (double)(n - 1);

    for (int k = 0; k < n; k++) {
        double v1_scale = exp(-2.0 * M_PI * z1i);
        double v1_angle = 2.0 * M_PI * z1r;
        double v1r = v1_scale * cos(v1_angle);
        double v1i = v1_scale * sin(v1_angle);

        double v2_scale = exp(-2.0 * M_PI * z2i);
        double v2_angle = 2.0 * M_PI * z2r;
        double v2r = v2_scale * cos(v2_angle);
        double v2i = v2_scale * sin(v2_angle);

        double vr = v1r - v2i;
        double vi = v1i + v2r;

        double numer_r = (double)n * vr;
        double numer_i = (double)n * vi;
        double ur = (numer_r * denr + numer_i * deni) / d2;
        double ui = (numer_i * denr - numer_r * deni) / d2;

        double er = -M_PI * ui;
        double ei = M_PI * ur;
        double scale = exp(er);
        cRe[k] = scale * cos(ei);
        cIm[k] = scale * sin(ei);

        z1r += step1r;
        z1i += step1i;
        z2r += step2r;
        z2i += step2i;
    }

    for (int k = 0; k < n; k++) {
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) { cRe[k] = 0.0; cIm[k] = 0.0; }
    }
}

/* giga_139: parametric p11b3 variant.
 * CFPV: int1, int2, int3, deg
 * Defaults reproduce the original Python implementation. */
static void giga_139_c(double x1r, double x1i, double x2r, double x2i,
                    const double *cfpv, int n_cfpv,
                    double *cRe, double *cIm, int *nCoeffs) {
    int int1 = (n_cfpv > 0) ? (int)cfpv[0] : 251;
    int int2 = (n_cfpv > 1) ? (int)cfpv[1] : 37;
    int int3 = (n_cfpv > 2) ? (int)cfpv[2] : 619;
    int deg = (n_cfpv > 3 && cfpv[3] >= 2 && cfpv[3] <= MAX_COEFFS) ? (int)cfpv[3] : 11;
    if (int2 <= 0) int2 = 1;
    *nCoeffs = deg;
    for (int i = 0; i < deg; i++) { cRe[i] = 0; cIm[i] = 0; }

    /* t = t1 + t2 */
    double tr = x1r + x2r, ti = x1i + x2i;
    /* a = |t1+t2| / 2 */
    double a = sqrt(tr * tr + ti * ti) / 2.0;
    /* m = int(a * int1) % int2 */
    int m = ((int)(a * (double)int1)) % int2;

    double vr[MAX_COEFFS], vi[MAX_COEFFS];
    /* v[k] = (k+1) / (t+4) */
    double dr = tr + 4.0, di = ti;
    double d2 = dr * dr + di * di;
    if (d2 < 1e-30) d2 = 1e-30;
    for (int k = 0; k < deg; k++) {
        double nr = (double)(k + 1);
        vr[k] = nr * dr / d2;
        vi[k] = -nr * di / d2;
    }

    /* p1 = int(7 * a * deg) % deg; v[p1] = (p1+1) / (t + |t|*2 + 1 + m) */
    double abst = sqrt(tr * tr + ti * ti);
    int p1 = ((int)(7.0 * a * (double)deg)) % deg;
    {
        double denr = tr + abst * 2.0 + 1.0 + m, deni = ti;
        double dd = denr * denr + deni * deni;
        if (dd < 1e-30) dd = 1e-30;
        double nr = (double)(p1 + 1);
        vr[p1] = nr * denr / dd;
        vi[p1] = -nr * deni / dd;
    }

    /* p2 = int(int3 * a * deg) % deg; v[p2] = (p2+1) / (t + |t|*2 + 1 + m//2) */
    int p2 = ((int)((double)int3 * a * (double)deg)) % deg;
    {
        double denr = tr + abst * 2.0 + 1.0 + (m / 2), deni = ti;
        double dd = denr * denr + deni * deni;
        if (dd < 1e-30) dd = 1e-30;
        double nr = (double)(p2 + 1);
        vr[p2] = nr * denr / dd;
        vi[p2] = -nr * deni / dd;
    }

    /* cf = exp(i * pi * v) */
    for (int k = 0; k < deg; k++) {
        double er = -M_PI * vi[k], ei = M_PI * vr[k];
        double e = exp(er);
        cRe[k] = e * cos(ei);
        cIm[k] = e * sin(ei);
    }
    for (int k = 0; k < deg; k++) {
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) { cRe[k] = 0; cIm[k] = 0; }
    }
}

/* poly_creative10: geometric algebra product terms with alternating signs.
 * gp = dot(t1,t2) + i*wedge(t1,t2), cf[k] = gp^(k+1), cf[even] *= -1. */
static void creative10_hand_c(double x1r, double x1i, double x2r, double x2i,
                               const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    (void)cfpv; (void)n_cfpv;
    *nCoeffs = 71;
    double dot = x1r * x2r + x1i * x2i;
    double wedge = x1r * x2i - x1i * x2r;
    /* gp = dot + i*wedge */
    double gr = dot, gi = wedge;
    /* gp^1 */
    double pr = gr, pi_ = gi;
    for (int k = 0; k < 71; k++) {
        cRe[k] = pr; cIm[k] = pi_;
        /* Negate even indices */
        if (k % 2 == 0) { cRe[k] = -cRe[k]; cIm[k] = -cIm[k]; }
        /* gp^(k+2) = gp^(k+1) * gp */
        double nr = pr * gr - pi_ * gi;
        double ni = pr * gi + pi_ * gr;
        pr = nr; pi_ = ni;
    }
    for (int k = 0; k < 71; k++) {
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) { cRe[k] = 0; cIm[k] = 0; }
    }
}

/* creative8: Hamiltonian-like terms with position/momentum mixing.
 * CFPV[0] = n (degree+1, default 71). */
static void creative8_c(double x1r, double x1i, double x2r, double x2i,
                        const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    int n = (n_cfpv > 0 && cfpv[0] >= 2 && cfpv[0] <= MAX_COEFFS)
            ? (int)cfpv[0] : 71;
    *nCoeffs = n;
    /* Pass 1: even k → q-terms, odd k → p-terms */
    for (int k = 0; k < n; k++) {
        if (k % 2 == 0) {
            double q = (k / 2 + 1) * x1r;
            cRe[k] = q * q;
            cIm[k] = q * x2i;
        } else {
            double p = (k / 2 + 1) * x1i;
            cRe[k] = p * p;
            cIm[k] = -(p * x2r);
        }
    }
    /* Pass 2: cf[even] += conj(cf[odd]), cf[odd] -= conj(cf[even]) */
    double tmpR[MAX_COEFFS], tmpI[MAX_COEFFS];
    for (int k = 0; k < n; k++) { tmpR[k] = cRe[k]; tmpI[k] = cIm[k]; }
    for (int k = 0; k < n; k += 2) {
        if (k + 1 < n) {
            cRe[k] = tmpR[k] + tmpR[k + 1];
            cIm[k] = tmpI[k] + (-tmpI[k + 1]);
        }
    }
    for (int k = 1; k < n; k += 2) {
        cRe[k] = tmpR[k] - tmpR[k - 1];
        cIm[k] = tmpI[k] - (-tmpI[k - 1]);
    }
    for (int k = 0; k < n; k++) {
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) { cRe[k] = 0; cIm[k] = 0; }
    }
}

/* creative9: Fourier series with frequency decay and neighbor mixing.
 * CFPV[0] = n (degree+1, default 71). */
static void creative9_c(double x1r, double x1i, double x2r, double x2i,
                        const double *cfpv, int n_cfpv,
                          double *cRe, double *cIm, int *nCoeffs) {
    int n = (n_cfpv > 0 && cfpv[0] >= 2 && cfpv[0] <= MAX_COEFFS) ? (int)cfpv[0] : 71;
    *nCoeffs = n;
    double phase_t1 = atan2(x1i, x1r);
    double phase_t2 = atan2(x2i, x2r);
    /* |t1*t2| */
    double pr, pi_; c_mul(x1r, x1i, x2r, x2i, &pr, &pi_);
    double abs_t1t2 = sqrt(pr*pr + pi_*pi_);

    /* Pass 1: cf[k] = (sin(freq_t1) + i*cos(freq_t2)) * exp(-|t1*t2|*k/n) */
    for (int k = 0; k < n; k++) {
        double freq_t1 = (k + 1) * phase_t1;
        double freq_t2 = (k + 1) * phase_t2;
        double sr = sin(freq_t1), si = cos(freq_t2);
        double decay = exp(-abs_t1t2 * k / (double)n);
        cRe[k] = sr * decay;
        cIm[k] = si * decay;
    }

    /* Pass 2: neighbor mixing (in-place) */
    double sumr = x1r + x2r, sumi = x1i + x2i;
    for (int k = 1; k < n - 1; k++) {
        double avgr = (cRe[k-1] + cRe[k+1]) * 0.5;
        double avgi = (cIm[k-1] + cIm[k+1]) * 0.5;
        c_mul(avgr, avgi, sumr, sumi, &cRe[k], &cIm[k]);
    }

    for (int k = 0; k < n; k++) {
        if (!isfinite(cRe[k]) || !isfinite(cIm[k])) { cRe[k] = 0; cIm[k] = 0; }
    }
}

/* Auto-generated g-functions from ops_poly.py (g1-g99+) */
#include "g_generated.c"

/* Auto-generated giga functions from giga.py */
#include "giga_generated.c"

#include "coeff_func_lookup.h"

/* ==== Coeffgen mode: generate coefficient vectors for the grid ==== */

/* param_dump: output transformed parameter pairs only.
 * Same grid + transform pipeline as coeffgen, but stops before calling
 * the coefficient function. Output: raw f32 pairs (z1r, z1i, z2r, z2i). */
static int runParamDump(const char *buf, const char *outPath) {
    int n1 = 100, n2 = 100;
    const char *cp;
    cp = findKey(buf, "n1"); if (cp) n1 = (int)parseNum(&cp);
    cp = findKey(buf, "n2"); if (cp) n2 = (int)parseNum(&cp);
    if (n1 < 1) n1 = 1;
    if (n2 < 1) n2 = 1;

    PtEntry ptEntries[MAX_CHAIN];
    int nPt = 0;
    cp = findKey(buf, "param_transforms");
    if (cp) nPt = parsePtChain(cp, ptEntries, MAX_CHAIN);

    FILE *fout = fopen(outPath, "wb");
    if (!fout) { fprintf(stderr, "Cannot open %s\n", outPath); return 1; }

    long nPoints = (long)n1 * n2;
    for (int i1 = 0; i1 < n1; i1++) {
        double x1 = (double)i1 / (double)n1;
        for (int i2 = 0; i2 < n2; i2++) {
            double x2 = (double)i2 / (double)n2;
            double z1r = x1, z1i = 0.0, z2r = x2, z2i = 0.0;
            for (int t = 0; t < nPt; t++) dispatchPt(&ptEntries[t], &z1r, &z1i, &z2r, &z2i, n1);
            float out[4] = { (float)z1r, (float)z1i, (float)z2r, (float)z2i };
            fwrite(out, sizeof(float), 4, fout);
        }
    }
    fclose(fout);

    long dataBytes = nPoints * 4 * sizeof(float);
    printf("{\"mode\":\"param_dump\",\"n_points\":%ld,\"data_bytes\":%ld,\"n1\":%d,\"n2\":%d}\n",
           nPoints, dataBytes, n1, n2);
    return 0;
}

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
    CtEntry ctEntries[MAX_CHAIN];
    int nCt = 0;
    cp = findKey(buf, "coeff_transforms");
    if (cp) nCt = parseCtChain(cp, ctEntries, MAX_CHAIN);
    int quantizeParams = coeff_transform_chain_needs_quantized_params(ctEntries, nCt);

    /* Look up coefficient function spec */
    const CoeffFuncSpec *spec = lookupCoeffFuncSpec(funcName);
    if (!spec) {
        fprintf(stderr, "Unknown function: %s\n", funcName);
        return 1;
    }
    CoeffFuncC coeffFunc = spec->func;

    /* Parse and normalize CFPV from spec */
    double cfpv[MAX_CFPV];
    int n_cfpv = 0;
    cp = findKey(buf, "cfpv");
    if (cp) n_cfpv = parseNumArray(cp, cfpv, MAX_CFPV);
    if (spec->n_params == 0) {
        n_cfpv = 0;
    } else {
        if (n_cfpv > spec->n_params) {
            fprintf(stderr, "Too many cfpv params for %s: got %d, expected %d\n",
                    spec->name, n_cfpv, spec->n_params);
            return 1;
        }
        for (int i = n_cfpv; i < spec->n_params; i++)
            cfpv[i] = spec->defaults[i];
        n_cfpv = spec->n_params;
    }

    /* Probe degree at (0,0) with transforms applied */
    double probeRe[MAX_COEFFS], probeIm[MAX_COEFFS];
    int probeN;
    {
        double z1r = 0, z1i = 0, z2r = 0, z2i = 0;
        for (int t = 0; t < nPt; t++) dispatchPt(&ptEntries[t], &z1r, &z1i, &z2r, &z2i, n1);
        if (quantizeParams) quantize_params_f32(&z1r, &z1i, &z2r, &z2i);
        coeffFunc(z1r, z1i, z2r, z2i, cfpv, n_cfpv, probeRe, probeIm, &probeN);
        for (int t = 0; t < nCt; t++) {
            if (dispatchCt(&ctEntries[t], probeRe, probeIm, &probeN) != 0) return 1;
        }
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
            if (quantizeParams) quantize_params_f32(&z1r, &z1i, &z2r, &z2i);

            double cRe[MAX_COEFFS], cIm[MAX_COEFFS];
            int nCoeffs;
            coeffFunc(z1r, z1i, z2r, z2i, cfpv, n_cfpv, cRe, cIm, &nCoeffs);
            for (int t = 0; t < nCt; t++) {
                if (dispatchCt(&ctEntries[t], cRe, cIm, &nCoeffs) != 0) {
                    fclose(fout);
                    free(stepBuf);
                    return 1;
                }
            }

            if (nCoeffs != nCoeffsOut) {
                fprintf(stderr, "nCoeffs mismatch: probe returned %d but step (%d,%d) pass %d returned %d\n",
                        nCoeffsOut, i1, i2, pass, nCoeffs);
                fclose(fout);
                free(stepBuf);
                return 1;
            }

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

/* ==== Param-gen mode: generate full unrolled parameter stream ==== */
/* Output: N*N*times records of (t1r, t1i, t2r, t2i) as float32.
 * Order: pass-major, i1 ascending, serpentine i2. Matches coeffgen traversal exactly. */
typedef struct {
    long rowIndex;
    int ready;
    int inUse;
    float *data;
} ParamGenRowSlot;

typedef struct {
    int n1;
    int n2;
    int gridN;
    int nPt;
    int slotCount;
    const PtEntry *ptEntries;
    long totalRows;
    long nextRow;
    long stepStart;
    long stepEnd;
    ParamGenRowSlot *slots;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} ParamGenThreadCtx;

typedef struct {
    ParamGenThreadCtx *ctx;
} ParamGenWorkerArg;

static void computeParamGenRow(long globalRow, int n1, int n2, int gridN,
                               const PtEntry *ptEntries, int nPt, float *outRow) {
    int pass = (int)(globalRow / n1);
    int i1 = (int)(globalRow % n1);
    double x1 = (double)i1 / (double)n1;
    _rng_state = paramGenRowSeed(globalRow, i1, pass);
    for (int j = 0; j < n2; j++) {
        int i2 = (i1 & 1) ? (n2 - 1 - j) : j;
        double x2 = (double)i2 / (double)n2;
        double z1r = x1, z1i = 0.0, z2r = x2, z2i = 0.0;
        for (int t = 0; t < nPt; t++) dispatchPt(&ptEntries[t], &z1r, &z1i, &z2r, &z2i, gridN);
        outRow[j * 4] = (float)z1r;
        outRow[j * 4 + 1] = (float)z1i;
        outRow[j * 4 + 2] = (float)z2r;
        outRow[j * 4 + 3] = (float)z2i;
    }
}

static void *paramGenWorkerMain(void *vp) {
    ParamGenWorkerArg *arg = (ParamGenWorkerArg *)vp;
    ParamGenThreadCtx *ctx = arg->ctx;
    while (1) {
        long row = -1;
        int slotIdx = -1;
        pthread_mutex_lock(&ctx->mutex);
        for (;;) {
            if (ctx->nextRow >= ctx->totalRows) {
                pthread_mutex_unlock(&ctx->mutex);
                return NULL;
            }
            row = ctx->nextRow;
            slotIdx = (int)(row % ctx->slotCount);
            if (!ctx->slots[slotIdx].inUse) {
                ctx->slots[slotIdx].inUse = 1;
                ctx->slots[slotIdx].ready = 0;
                ctx->slots[slotIdx].rowIndex = row;
                ctx->nextRow++;
                break;
            }
            pthread_cond_wait(&ctx->cond, &ctx->mutex);
        }
        pthread_mutex_unlock(&ctx->mutex);

        computeParamGenRow(
            row,
            ctx->n1,
            ctx->n2,
            ctx->gridN,
            ctx->ptEntries,
            ctx->nPt,
            ctx->slots[slotIdx].data
        );

        pthread_mutex_lock(&ctx->mutex);
        ctx->slots[slotIdx].ready = 1;
        pthread_cond_broadcast(&ctx->cond);
        pthread_mutex_unlock(&ctx->mutex);
    }
}

static int writeParamGenRowSlice(FILE *fout, const float *rowData, long row, int n2,
                                 long stepStart, long stepEnd) {
    long rowFirstStep = row * (long)n2;
    long rowEndStep = rowFirstStep + (long)n2;
    long overlapStart = rowFirstStep > stepStart ? rowFirstStep : stepStart;
    long overlapEnd = rowEndStep < stepEnd ? rowEndStep : stepEnd;
    if (overlapEnd <= overlapStart) return 0;
    size_t j0 = (size_t)(overlapStart - rowFirstStep);
    size_t count = (size_t)(overlapEnd - overlapStart);
    size_t wrote = fwrite(rowData + j0 * 4u, sizeof(float), count * 4u, fout);
    return wrote == count * 4u ? 0 : 1;
}

static int runParamGenRangeSerial(FILE *fout, int n1, int n2, int gridN,
                                  const PtEntry *ptEntries, int nPt,
                                  long stepStart, long stepCount) {
    long stepEnd = stepStart + stepCount;
    long rowStart = stepStart / (long)n2;
    long rowEnd = (stepEnd + (long)n2 - 1L) / (long)n2;
    size_t rowValueCount = (size_t)n2 * 4u;
    float *rowData = (float *)malloc(rowValueCount * sizeof(float));
    if (!rowData) {
        fprintf(stderr, "param_gen range row buffer alloc failed\n");
        return 1;
    }
    for (long row = rowStart; row < rowEnd; row++) {
        computeParamGenRow(row, n1, n2, gridN, ptEntries, nPt, rowData);
        if (writeParamGenRowSlice(fout, rowData, row, n2, stepStart, stepEnd) != 0) {
            free(rowData);
            fprintf(stderr, "param_gen range write failed\n");
            return 1;
        }
    }
    free(rowData);
    return 0;
}

static int runParamGenSerial(FILE *fout, int n1, int n2, int gridN, int times,
                             const PtEntry *ptEntries, int nPt) {
    for (int pass = 0; pass < times; pass++) {
        /* Seed RNG per pass — matches coeffgen exactly */
        _rng_state = 0x123456789abcdef0ULL ^ ((uint64_t)pass * 2654435761ULL);
        if (!_rng_state) _rng_state = 1;
        for (int i1 = 0; i1 < n1; i1++) {
            double x1 = (double)i1 / (double)n1;
            for (int j = 0; j < n2; j++) {
                int i2 = (i1 & 1) ? (n2 - 1 - j) : j;
                double x2 = (double)i2 / (double)n2;

                double z1r = x1, z1i = 0.0, z2r = x2, z2i = 0.0;
                for (int t = 0; t < nPt; t++) dispatchPt(&ptEntries[t], &z1r, &z1i, &z2r, &z2i, gridN);

                float out[4] = { (float)z1r, (float)z1i, (float)z2r, (float)z2i };
                fwrite(out, sizeof(float), 4, fout);
            }
        }
    }
    return 0;
}

static int runParamGenThreadedRange(FILE *fout, int n1, int n2, int gridN,
                                    const PtEntry *ptEntries, int nPt, int nThreads,
                                    long stepStart, long stepCount) {
    long stepEnd = stepStart + stepCount;
    long rowStart = stepStart / (long)n2;
    long rowEnd = (stepEnd + (long)n2 - 1L) / (long)n2;
    long totalRows = rowEnd;
    long rowCount = rowEnd - rowStart;
    size_t rowValueCount = (size_t)n2 * 4u;
    int slotCount = nThreads * 2;
    if (slotCount < 2) slotCount = 2;
    if ((long)slotCount > rowCount) slotCount = (int)rowCount;
    if (slotCount < 1) slotCount = 1;

    ParamGenRowSlot *slots = (ParamGenRowSlot *)calloc((size_t)slotCount, sizeof(ParamGenRowSlot));
    pthread_t *threads = (pthread_t *)calloc((size_t)nThreads, sizeof(pthread_t));
    ParamGenWorkerArg *args = (ParamGenWorkerArg *)calloc((size_t)nThreads, sizeof(ParamGenWorkerArg));
    if (!slots || !threads || !args) {
        free(slots);
        free(threads);
        free(args);
        fprintf(stderr, "param_gen threaded alloc failed\n");
        return 1;
    }
    for (int i = 0; i < slotCount; i++) {
        slots[i].rowIndex = -1;
        slots[i].data = (float *)malloc(rowValueCount * sizeof(float));
        if (!slots[i].data) {
            for (int j = 0; j < i; j++) free(slots[j].data);
            free(slots);
            free(threads);
            free(args);
            fprintf(stderr, "param_gen threaded row buffer alloc failed\n");
            return 1;
        }
    }

    ParamGenThreadCtx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.n1 = n1;
    ctx.n2 = n2;
    ctx.gridN = gridN;
    ctx.nPt = nPt;
    ctx.slotCount = slotCount;
    ctx.ptEntries = ptEntries;
    ctx.totalRows = totalRows;
    ctx.nextRow = rowStart;
    ctx.stepStart = stepStart;
    ctx.stepEnd = stepEnd;
    ctx.slots = slots;
    pthread_mutex_init(&ctx.mutex, NULL);
    pthread_cond_init(&ctx.cond, NULL);

    int created = 0;
    for (int i = 0; i < nThreads; i++) {
        args[i].ctx = &ctx;
        if (pthread_create(&threads[i], NULL, paramGenWorkerMain, &args[i]) != 0) {
            fprintf(stderr, "param_gen pthread_create failed for worker %d\n", i);
            pthread_mutex_lock(&ctx.mutex);
            ctx.nextRow = ctx.totalRows;
            pthread_cond_broadcast(&ctx.cond);
            pthread_mutex_unlock(&ctx.mutex);
            for (int j = 0; j < created; j++) pthread_join(threads[j], NULL);
            pthread_cond_destroy(&ctx.cond);
            pthread_mutex_destroy(&ctx.mutex);
            for (int j = 0; j < slotCount; j++) free(slots[j].data);
            free(slots);
            free(threads);
            free(args);
            return 1;
        }
        created++;
    }

    for (long row = rowStart; row < rowEnd; row++) {
        int slotIdx = (int)(row % slotCount);
        pthread_mutex_lock(&ctx.mutex);
        while (!(slots[slotIdx].inUse && slots[slotIdx].rowIndex == row && slots[slotIdx].ready)) {
            pthread_cond_wait(&ctx.cond, &ctx.mutex);
        }
        pthread_mutex_unlock(&ctx.mutex);

        if (writeParamGenRowSlice(fout, slots[slotIdx].data, row, n2, stepStart, stepEnd) != 0) {
            pthread_mutex_lock(&ctx.mutex);
            ctx.nextRow = ctx.totalRows;
            slots[slotIdx].inUse = 0;
            slots[slotIdx].ready = 0;
            slots[slotIdx].rowIndex = -1;
            pthread_cond_broadcast(&ctx.cond);
            pthread_mutex_unlock(&ctx.mutex);
            for (int i = 0; i < created; i++) pthread_join(threads[i], NULL);
            pthread_cond_destroy(&ctx.cond);
            pthread_mutex_destroy(&ctx.mutex);
            for (int i = 0; i < slotCount; i++) free(slots[i].data);
            free(slots);
            free(threads);
            free(args);
            fprintf(stderr, "param_gen threaded write failed\n");
            return 1;
        }

        pthread_mutex_lock(&ctx.mutex);
        slots[slotIdx].inUse = 0;
        slots[slotIdx].ready = 0;
        slots[slotIdx].rowIndex = -1;
        pthread_cond_broadcast(&ctx.cond);
        pthread_mutex_unlock(&ctx.mutex);
    }

    for (int i = 0; i < created; i++) pthread_join(threads[i], NULL);
    pthread_cond_destroy(&ctx.cond);
    pthread_mutex_destroy(&ctx.mutex);
    for (int i = 0; i < slotCount; i++) free(slots[i].data);
    free(slots);
    free(threads);
    free(args);
    return 0;
}

static int runParamGenThreaded(FILE *fout, int n1, int n2, int gridN, int times,
                               const PtEntry *ptEntries, int nPt, int nThreads) {
    long totalSteps = (long)n1 * (long)n2 * (long)times;
    return runParamGenThreadedRange(fout, n1, n2, gridN, ptEntries, nPt, nThreads, 0, totalSteps);
}

static int runParamGen(const char *buf, const char *outPath) {
    int n1 = 100, n2 = 100;
    const char *cp = findKey(buf, "n1"); if (cp) n1 = (int)parseNum(&cp);
    cp = findKey(buf, "n2"); if (cp) n2 = (int)parseNum(&cp);

    /* gridN: override for dither/transform scaling. If not set, defaults to n1.
     * Used by lores preview to match hires dither amplitude. */
    int gridN = n1;
    cp = findKey(buf, "gridN"); if (cp) gridN = (int)parseNum(&cp);

    int times = 1;
    cp = findKey(buf, "times"); if (cp) times = (int)parseNum(&cp);
    if (times < 1) times = 1;

    int requestedThreads = 1;
    cp = findKey(buf, "n_threads"); if (cp) requestedThreads = (int)parseNum(&cp);
    if (requestedThreads < 1) requestedThreads = 1;

    PtEntry ptEntries[MAX_CHAIN];
    int nPt = 0;
    cp = findKey(buf, "param_transforms");
    if (cp) nPt = parsePtChain(cp, ptEntries, MAX_CHAIN);

    /* outPath "-" means write binary data to stdout (for streaming to S3).
     * Metadata JSON goes to stderr in that case. */
    int streamMode = (strcmp(outPath, "-") == 0);
    FILE *fout = streamMode ? stdout : fopen(outPath, "wb");
    if (!fout) { fprintf(stderr, "Cannot open %s\n", outPath); return 1; }

    long totalSteps = (long)n1 * (long)n2 * (long)times;
    long stepStart = 0;
    long stepCount = totalSteps;
    int isRange = 0;
    cp = findKey(buf, "step_start");
    if (cp) {
        stepStart = (long)parseNum(&cp);
        isRange = 1;
    }
    cp = findKey(buf, "step_count");
    if (cp) {
        stepCount = (long)parseNum(&cp);
        isRange = 1;
    } else if (isRange) {
        stepCount = totalSteps - stepStart;
    }
    if (stepStart < 0 || stepCount < 1 || stepStart > totalSteps || stepCount > totalSteps - stepStart) {
        fprintf(stderr, "invalid param_gen range: step_start=%ld step_count=%ld total_steps=%ld\n",
                stepStart, stepCount, totalSteps);
        if (!streamMode) fclose(fout);
        return 1;
    }

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    int threadsUsed = requestedThreads;
    long rowStart = stepStart / (long)n2;
    long rowEnd = (stepStart + stepCount + (long)n2 - 1L) / (long)n2;
    long rowCount = rowEnd - rowStart;
    if (threadsUsed > rowCount) threadsUsed = (int)rowCount;
    if (threadsUsed < 1) threadsUsed = 1;

    int rc = 0;
    if (threadsUsed <= 1) {
        threadsUsed = 1;
        if (isRange) {
            rc = runParamGenRangeSerial(fout, n1, n2, gridN, ptEntries, nPt, stepStart, stepCount);
        } else {
            rc = runParamGenSerial(fout, n1, n2, gridN, times, ptEntries, nPt);
        }
    } else {
        rc = runParamGenThreadedRange(fout, n1, n2, gridN, ptEntries, nPt, threadsUsed, stepStart, stepCount);
    }
    if (rc != 0) {
        if (!streamMode) fclose(fout);
        else fflush(stdout);
        return rc;
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    long elapsed_us = (t1.tv_sec - t0.tv_sec) * 1000000L +
                      (t1.tv_nsec - t0.tv_nsec) / 1000L;
    if (!streamMode) fclose(fout);
    else fflush(stdout);

    long dataBytes = stepCount * 4 * (long)sizeof(float);
    /* In stream mode, metadata goes to stderr (stdout is binary data) */
    FILE *metaOut = streamMode ? stderr : stdout;
    fprintf(metaOut, "{\"mode\":\"param_gen\",\"n1\":%d,\"n2\":%d,\"times\":%d,"
           "\"n_steps\":%ld,\"total_steps\":%ld,\"step_start\":%ld,\"step_count\":%ld,"
           "\"data_bytes\":%ld,\"threads\":%d,\"elapsed_us\":%ld}\n",
           n1, n2, times, stepCount, totalSteps, stepStart, stepCount, dataBytes, threadsUsed, elapsed_us);
    return 0;
}

/* ==== Coeffgen-chunked mode: read params slice, generate coefficients ==== */
/* Reads step_count records from params_file starting at step_start,
 * runs coefficient function + coeff transforms on each, writes coefficient output. */
typedef struct {
    int paramsFd;
    int outFd;
    long paramBaseOffset;
    long globalStepStart;
    int nCoeffsOut;
    long outRowBytes;
    CoeffFuncC coeffFunc;
    const CtEntry *ctEntries;
    int nCt;
    const double *cfpv;
    int n_cfpv;
    pthread_mutex_t mutex;
    int failed;
    char error[256];
} CoeffGenThreadCtx;

typedef struct {
    CoeffGenThreadCtx *ctx;
    long stepLo;
    long stepHi;
} CoeffGenWorkerArg;

static void coeffGenSetThreadError(CoeffGenThreadCtx *ctx, const char *msg) {
    pthread_mutex_lock(&ctx->mutex);
    if (!ctx->failed) {
        ctx->failed = 1;
        snprintf(ctx->error, sizeof(ctx->error), "%s", msg);
    }
    pthread_mutex_unlock(&ctx->mutex);
}

static void *coeffGenWorkerMain(void *vp) {
    CoeffGenWorkerArg *arg = (CoeffGenWorkerArg *)vp;
    CoeffGenThreadCtx *ctx = arg->ctx;
    float params[4];
    double cRe[MAX_COEFFS], cIm[MAX_COEFFS];
    float *stepBuf = (float *)malloc((size_t)ctx->outRowBytes);
    if (!stepBuf) {
        coeffGenSetThreadError(ctx, "coeffgen threaded step buffer alloc failed");
        return NULL;
    }

    for (long s = arg->stepLo; s < arg->stepHi; s++) {
        if (ctx->failed) break;

        off_t paramOff = (off_t)(ctx->paramBaseOffset + s * (long)(4 * sizeof(float)));
        ssize_t got = pread(ctx->paramsFd, params, sizeof(params), paramOff);
        if (got != (ssize_t)sizeof(params)) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Short read at step %ld", ctx->globalStepStart + s);
            coeffGenSetThreadError(ctx, msg);
            break;
        }

        int nCoeffs = 0;
        ctx->coeffFunc((double)params[0], (double)params[1],
                       (double)params[2], (double)params[3],
                       ctx->cfpv, ctx->n_cfpv, cRe, cIm, &nCoeffs);
        for (int t = 0; t < ctx->nCt; t++) {
            if (dispatchCt(&ctx->ctEntries[t], cRe, cIm, &nCoeffs) != 0) {
                coeffGenSetThreadError(ctx, "coeffgen threaded coeff transform failed");
                break;
            }
        }
        if (ctx->failed) break;

        if (nCoeffs != ctx->nCoeffsOut) {
            char msg[256];
            snprintf(msg, sizeof(msg),
                     "nCoeffs mismatch: probe returned %d but step %ld returned %d",
                     ctx->nCoeffsOut, ctx->globalStepStart + s, nCoeffs);
            coeffGenSetThreadError(ctx, msg);
            break;
        }

        for (int k = 0; k < ctx->nCoeffsOut; k++) {
            stepBuf[k * 2]     = (float)cRe[k];
            stepBuf[k * 2 + 1] = (float)cIm[k];
        }

        off_t outOff = (off_t)(s * ctx->outRowBytes);
        ssize_t wrote = pwrite(ctx->outFd, stepBuf, (size_t)ctx->outRowBytes, outOff);
        if (wrote != (ssize_t)ctx->outRowBytes) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Short write at step %ld", ctx->globalStepStart + s);
            coeffGenSetThreadError(ctx, msg);
            break;
        }
    }

    free(stepBuf);
    return NULL;
}

static int runCoeffGenChunked(const char *buf, const char *outPath) {
    char funcName[64] = "";
    const char *cp = findKey(buf, "function");
    if (cp) parseString(cp, funcName, sizeof(funcName));

    char paramsFile[256] = "";
    cp = findKey(buf, "params_file");
    if (cp) parseString(cp, paramsFile, sizeof(paramsFile));

    long stepStart = 0, stepCount = 0;
    cp = findKey(buf, "step_start"); if (cp) stepStart = (long)parseNum(&cp);
    cp = findKey(buf, "step_count"); if (cp) stepCount = (long)parseNum(&cp);
    int requestedThreads = 1;
    cp = findKey(buf, "n_threads"); if (cp) requestedThreads = (int)parseNum(&cp);
    if (requestedThreads < 1) requestedThreads = 1;

    if (stepCount <= 0) {
        fprintf(stderr, "Empty chunk: step_count=%ld\n", stepCount);
        return 1;
    }

    /* Parse coefficient transform chain */
    CtEntry ctEntries[MAX_CHAIN];
    int nCt = 0;
    cp = findKey(buf, "coeff_transforms");
    if (cp) nCt = parseCtChain(cp, ctEntries, MAX_CHAIN);

    /* Look up coefficient function spec */
    const CoeffFuncSpec *spec = lookupCoeffFuncSpec(funcName);
    if (!spec) {
        fprintf(stderr, "Unknown function: %s\n", funcName);
        return 1;
    }
    CoeffFuncC coeffFunc = spec->func;

    /* Parse and normalize CFPV from spec */
    double cfpv[MAX_CFPV];
    int n_cfpv = 0;
    cp = findKey(buf, "cfpv");
    if (cp) n_cfpv = parseNumArray(cp, cfpv, MAX_CFPV);
    if (spec->n_params == 0) {
        n_cfpv = 0;
    } else {
        if (n_cfpv > spec->n_params) {
            fprintf(stderr, "Too many cfpv params for %s: got %d, expected %d\n",
                    spec->name, n_cfpv, spec->n_params);
            return 1;
        }
        for (int i = n_cfpv; i < spec->n_params; i++)
            cfpv[i] = spec->defaults[i];
        n_cfpv = spec->n_params;
    }

    /* Open params file and probe first record. */
    int paramsFd = open(paramsFile, O_RDONLY);
    if (paramsFd < 0) {
        fprintf(stderr, "Cannot open params file: %s\n", paramsFile);
        return 1;
    }
    long recordBytes = 4 * sizeof(float);  /* t1r, t1i, t2r, t2i */

    /* Probe degree from first record */
    float probe[4];
    if (pread(paramsFd, probe, sizeof(probe), (off_t)(stepStart * recordBytes)) != (ssize_t)sizeof(probe)) {
        fprintf(stderr, "Cannot read probe record\n");
        close(paramsFd);
        return 1;
    }

    double probeRe[MAX_COEFFS], probeIm[MAX_COEFFS];
    int probeN;
    coeffFunc((double)probe[0], (double)probe[1],
              (double)probe[2], (double)probe[3],
              cfpv, n_cfpv, probeRe, probeIm, &probeN);
    for (int t = 0; t < nCt; t++) {
        if (dispatchCt(&ctEntries[t], probeRe, probeIm, &probeN) != 0) {
            close(paramsFd);
            return 1;
        }
    }
    int nCoeffsOut = probeN;
    int degree = nCoeffsOut - 1;
    int threadsUsed = requestedThreads;
    if (threadsUsed > stepCount) threadsUsed = (int)stepCount;
    if (threadsUsed < 1) threadsUsed = 1;
    long outRowBytes = (long)nCoeffsOut * 2 * (long)sizeof(float);

    int outFd = open(outPath, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (outFd < 0) {
        fprintf(stderr, "Cannot open %s\n", outPath);
        close(paramsFd);
        return 1;
    }
    if (ftruncate(outFd, (off_t)(stepCount * outRowBytes)) != 0) {
        fprintf(stderr, "Cannot size %s\n", outPath);
        close(paramsFd);
        close(outFd);
        return 1;
    }

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    int rc = 0;
    if (threadsUsed <= 1) {
        float *stepBuf = (float *)malloc((size_t)outRowBytes);
        if (!stepBuf) {
            fprintf(stderr, "coeffgen step buffer alloc failed\n");
            close(paramsFd);
            close(outFd);
            return 1;
        }
        for (long s = 0; s < stepCount; s++) {
            float params[4];
            if (pread(paramsFd, params, sizeof(params), (off_t)(stepStart * recordBytes + s * recordBytes)) != (ssize_t)sizeof(params)) {
                fprintf(stderr, "Short read at step %ld\n", stepStart + s);
                rc = 1;
                break;
            }

            double cRe[MAX_COEFFS], cIm[MAX_COEFFS];
            int nCoeffs;
            coeffFunc((double)params[0], (double)params[1],
                      (double)params[2], (double)params[3],
                      cfpv, n_cfpv, cRe, cIm, &nCoeffs);
            for (int t = 0; t < nCt; t++) {
                if (dispatchCt(&ctEntries[t], cRe, cIm, &nCoeffs) != 0) {
                    rc = 1;
                    break;
                }
            }
            if (rc != 0) break;

            if (nCoeffs != nCoeffsOut) {
                fprintf(stderr, "nCoeffs mismatch: probe returned %d but step %ld returned %d\n",
                        nCoeffsOut, stepStart + s, nCoeffs);
                rc = 1;
                break;
            }

            for (int k = 0; k < nCoeffsOut; k++) {
                stepBuf[k * 2]     = (float)cRe[k];
                stepBuf[k * 2 + 1] = (float)cIm[k];
            }
            if (pwrite(outFd, stepBuf, (size_t)outRowBytes, (off_t)(s * outRowBytes)) != (ssize_t)outRowBytes) {
                fprintf(stderr, "Short write at step %ld\n", stepStart + s);
                rc = 1;
                break;
            }
        }
        free(stepBuf);
    } else {
        CoeffGenThreadCtx ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.paramsFd = paramsFd;
        ctx.outFd = outFd;
        ctx.paramBaseOffset = stepStart * recordBytes;
        ctx.globalStepStart = stepStart;
        ctx.nCoeffsOut = nCoeffsOut;
        ctx.outRowBytes = outRowBytes;
        ctx.coeffFunc = coeffFunc;
        ctx.ctEntries = ctEntries;
        ctx.nCt = nCt;
        ctx.cfpv = cfpv;
        ctx.n_cfpv = n_cfpv;
        pthread_mutex_init(&ctx.mutex, NULL);

        pthread_t *threads = (pthread_t *)calloc((size_t)threadsUsed, sizeof(pthread_t));
        CoeffGenWorkerArg *args = (CoeffGenWorkerArg *)calloc((size_t)threadsUsed, sizeof(CoeffGenWorkerArg));
        if (!threads || !args) {
            fprintf(stderr, "coeffgen threaded alloc failed\n");
            free(threads);
            free(args);
            pthread_mutex_destroy(&ctx.mutex);
            close(paramsFd);
            close(outFd);
            return 1;
        }

        long baseSteps = stepCount / threadsUsed;
        long remSteps = stepCount % threadsUsed;
        long cursor = 0;
        int created = 0;
        for (int i = 0; i < threadsUsed; i++) {
            long len = baseSteps + (i < remSteps ? 1 : 0);
            args[i].ctx = &ctx;
            args[i].stepLo = cursor;
            args[i].stepHi = cursor + len;
            cursor += len;
            if (pthread_create(&threads[i], NULL, coeffGenWorkerMain, &args[i]) != 0) {
                coeffGenSetThreadError(&ctx, "coeffgen pthread_create failed");
                rc = 1;
                break;
            }
            created++;
        }
        for (int i = 0; i < created; i++) pthread_join(threads[i], NULL);
        if (ctx.failed) {
            fprintf(stderr, "%s\n", ctx.error[0] ? ctx.error : "coeffgen threaded failure");
            rc = 1;
        }
        free(threads);
        free(args);
        pthread_mutex_destroy(&ctx.mutex);
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    long elapsed_us = (t1.tv_sec - t0.tv_sec) * 1000000L +
                      (t1.tv_nsec - t0.tv_nsec) / 1000L;
    close(paramsFd);
    close(outFd);
    if (rc != 0) return rc;

    long dataBytes = stepCount * nCoeffsOut * 2 * (long)sizeof(float);
    printf("{\"mode\":\"coeffgen_chunked\",\"function\":\"%s\","
           "\"n_coeffs\":%d,\"degree\":%d,"
           "\"step_start\":%ld,\"step_count\":%ld,"
           "\"n_t\":%ld,\"data_bytes\":%ld,\"threads\":%d,"
           "\"elapsed_us\":%ld}\n",
           funcName, nCoeffsOut, degree,
           stepStart, stepCount,
           stepCount, dataBytes, threadsUsed, elapsed_us);
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

        if (effDeg <= 0) {
            for (int i = 0; i < degree; i++) { rootRe[i] = 0; rootIm[i] = 0; }
            iters = 0;
        } else if (effDeg == 1) {
            for (int i = 1; i < degree; i++) { rootRe[i] = 0; rootIm[i] = 0; }
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
            /* Set trailing-zero roots to 0 (z=0 with multiplicity trailingZeros),
             * but preserve the active solver slots as the warm-start chain. */
            for (int i = effDeg; i < degree; i++) { rootRe[i] = 0; rootIm[i] = 0; }
            /* Re-init when the preserved warm start has collapsed (all zero,
             * non-finite, or duplicate roots from a repeated-root step). */
            if (warmStartNeedsReseed(rootRe, rootIm, effDeg))
                seedEAInitialGuess(rootRe, rootIm, effDeg);
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

            if (effDeg <= 0) {
                for (int i = 0; i < degree; i++) { rootRe[i] = 0; rootIm[i] = 0; }
                iters = 0;
            } else if (effDeg == 1) {
                for (int i = 1; i < degree; i++) { rootRe[i] = 0; rootIm[i] = 0; }
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
                for (int i = effDeg; i < degree; i++) { rootRe[i] = 0; rootIm[i] = 0; }
                if (warmStartNeedsReseed(rootRe, rootIm, effDeg))
                    seedEAInitialGuess(rootRe, rootIm, effDeg);
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
        if (strcmp(mode, "param_dump") == 0) {
            int rc = runParamDump(buf, outPath);
            free(buf);
            return rc;
        }
        if (strcmp(mode, "param_gen") == 0) {
            int rc = runParamGen(buf, outPath);
            free(buf);
            return rc;
        }
        if (strcmp(mode, "coeffgen_chunked") == 0) {
            int rc = runCoeffGenChunked(buf, outPath);
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

        if (effDeg <= 0) {
            for (int i = 0; i < degree; i++) { rootRe[i] = 0; rootIm[i] = 0; }
            iters = 0;
        } else if (effDeg == 1) {
            /* Linear */
            for (int i = 1; i < degree; i++) { rootRe[i] = 0; rootIm[i] = 0; }
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
            for (int i = effDeg; i < degree; i++) { rootRe[i] = 0; rootIm[i] = 0; }
            if (warmStartNeedsReseed(rootRe, rootIm, effDeg))
                seedEAInitialGuess(rootRe, rootIm, effDeg);
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
