/*
 * Ehrlich-Aberth polynomial root solver — standalone CLI.
 * Reads JSON from stdin, writes JSON to stdout.
 * Statically linked for Lambda (ARM64 Linux).
 *
 * Input:  {"coefficients":[[re,im],...]}
 * Output: {"roots":[[re,im],...], "iterations":N, "elapsed_us":N}
 *
 * Build: gcc -O3 -static -o solver solver_cli.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_DEGREE 255
#define MAX_COEFFS 256
#define MAX_ITER 64
#define TOL2 1e-16
#define BUF_SIZE (1024 * 64)

/* ---- Ehrlich-Aberth solver (from solver.c) ---- */

static int solveEA(double *cRe, double *cIm, int nCoeffs,
                   double *rootRe, double *rootIm)
{
    /* Strip leading near-zero coefficients */
    int start = 0;
    while (start < nCoeffs - 1 &&
           cRe[start] * cRe[start] + cIm[start] * cIm[start] < 1e-30)
        start++;

    int degree = nCoeffs - 1 - start;
    if (degree <= 0)
        return 0;

    /* Degree 1: linear case */
    if (degree == 1) {
        double aR = cRe[start], aI = cIm[start];
        double bR = cRe[start + 1], bI = cIm[start + 1];
        double d = aR * aR + aI * aI;
        if (d < 1e-30) return 0;
        rootRe[0] = -(bR * aR + bI * aI) / d;
        rootIm[0] = -(bI * aR - bR * aI) / d;
        return 1;
    }

    /* Copy stripped coefficients */
    int n = nCoeffs - start;
    double cr[MAX_COEFFS], ci[MAX_COEFFS];
    for (int k = 0; k < n; k++) {
        cr[k] = cRe[start + k];
        ci[k] = cIm[start + k];
    }

    /* Initial guesses on a slightly off-center circle */
    for (int k = 0; k < degree; k++) {
        double angle = 2.0 * M_PI * k / degree + 0.3;
        double r = 1.0 + 0.1 * k / degree;
        rootRe[k] = r * cos(angle);
        rootIm[k] = r * sin(angle);
    }

    /* Main Ehrlich-Aberth iteration */
    int totalIter = 0;
    for (int iter = 0; iter < MAX_ITER; iter++) {
        totalIter = iter + 1;
        double maxCorr2 = 0;

        for (int i = 0; i < degree; i++) {
            double zR = rootRe[i], zI = rootIm[i];

            /* Horner: evaluate p(z) and p'(z) simultaneously */
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

            /* Newton step: w = p(z) / p'(z) */
            double dpM = dpR * dpR + dpI * dpI;
            if (dpM < 1e-60) continue;
            double wR = (pR * dpR + pI * dpI) / dpM;
            double wI = (pI * dpR - pR * dpI) / dpM;

            /* Aberth sum: S = sum_{j!=i} 1/(z_i - z_j) */
            double sR = 0, sI = 0;
            for (int j = 0; j < degree; j++) {
                if (j == i) continue;
                double dR = zR - rootRe[j];
                double dI = zI - rootIm[j];
                double dM = dR * dR + dI * dI;
                if (dM < 1e-60) continue;
                sR += dR / dM;
                sI += -dI / dM;
            }

            /* Correction: z -= w / (1 - w * S) */
            double wsR = wR * sR - wI * sI;
            double wsI = wR * sI + wI * sR;
            double dnR = 1 - wsR;
            double dnI = -wsI;
            double dnM = dnR * dnR + dnI * dnI;
            if (dnM < 1e-60) continue;

            double crrR = (wR * dnR + wI * dnI) / dnM;
            double crrI = (wI * dnR - wR * dnI) / dnM;

            rootRe[i] -= crrR;
            rootIm[i] -= crrI;

            double h2 = crrR * crrR + crrI * crrI;
            if (h2 > maxCorr2) maxCorr2 = h2;
        }

        if (maxCorr2 < TOL2) break;
    }

    return totalIter;
}

/* ---- Minimal JSON parsing ---- */

/* Skip whitespace */
static const char *skip(const char *p) {
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    return p;
}

/* Parse a number, advance pointer */
static double parseNum(const char **pp) {
    const char *p = skip(*pp);
    char *end;
    double v = strtod(p, &end);
    *pp = end;
    return v;
}

/* Find key in JSON object, return pointer after the colon */
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

/* Parse [[re,im],[re,im],...] */
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
        p++; /* skip [ */
        re[count] = parseNum(&p);
        p = skip(p);
        if (*p == ',') p++;
        im[count] = parseNum(&p);
        p = skip(p);
        if (*p == ']') p++;
        count++;
    }
    return count;
}

int main(int argc, char **argv) {
    /* Read all of stdin */
    char buf[BUF_SIZE];
    int len = 0;
    int n;
    while ((n = fread(buf + len, 1, BUF_SIZE - len - 1, stdin)) > 0)
        len += n;
    buf[len] = '\0';

    /* Parse coefficients */
    double cRe[MAX_COEFFS], cIm[MAX_COEFFS];
    int nCoeffs = 0;

    const char *cp = findKey(buf, "coefficients");
    if (cp) {
        nCoeffs = parseCoefficients(cp, cRe, cIm);
    }

    if (nCoeffs < 2) {
        fprintf(stderr, "Need at least 2 coefficients\n");
        return 1;
    }

    /* Solve */
    double rootRe[MAX_DEGREE], rootIm[MAX_DEGREE];
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    int iters = solveEA(cRe, cIm, nCoeffs, rootRe, rootIm);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    long elapsed_us = (t1.tv_sec - t0.tv_sec) * 1000000L +
                      (t1.tv_nsec - t0.tv_nsec) / 1000L;

    int degree = nCoeffs - 1;

    /* Output JSON */
    printf("{\"roots\":[");
    for (int i = 0; i < degree; i++) {
        if (i > 0) printf(",");
        printf("[%.17g,%.17g]", rootRe[i], rootIm[i]);
    }
    printf("],\"degree\":%d,\"iterations\":%d,\"elapsed_us\":%ld}\n",
           degree, iters, elapsed_us);

    return 0;
}
