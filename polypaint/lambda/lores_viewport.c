/*
 * lores_viewport: low-resolution parameter grid scan to determine viewport.
 * Samples every Nth row/col, solves for roots, uses 2nd/98th percentile
 * bounding box to ignore outliers.
 *
 * Input:  JSON on stdin: {"function":"giga_5","n1":1000,"n2":1000}
 * Output: JSON to stdout: {"center_re":...,"center_im":...,"scale":...,"degree":...}
 *         Scale is computed for a 4096x4096 reference image.
 *
 * Build: aarch64-linux-musl-gcc -O3 -static -o lores_viewport lores_viewport.c -lm
 * Local: cc -O3 -o lores_viewport lores_viewport.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_DEGREE 255
#define MAX_COEFFS 256
#define MAX_ITER 64
#define TOL2 1e-16
#define BUF_SIZE (1024 * 256)
#define REF_SIZE 4096  /* reference image dimension for scale */

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
            double dpM = dpR * dpR + dpI * dpI;
            if (dpM < 1e-60) continue;
            double wR = (pR * dpR + pI * dpI) / dpM;
            double wI = (pI * dpR - pR * dpI) / dpM;
            double sR = 0, sI = 0;
            for (int j = 0; j < degree; j++) {
                if (j == i) continue;
                double dR = zR - rRe[j], dI = zI - rIm[j];
                double dM = dR * dR + dI * dI;
                if (dM < 1e-60) continue;
                sR += dR / dM;
                sI += -dI / dM;
            }
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

/* ---- Coefficient functions ---- */

typedef void (*CoeffFunc)(double, double, double*, double*, int*);

static void giga_5(double x1, double x2,
                   double *cRe, double *cIm, int *nCoeffs)
{
    *nCoeffs = 26;
    memset(cRe, 0, 26 * sizeof(double));
    memset(cIm, 0, 26 * sizeof(double));
    cRe[0]  = 1.0;  cRe[4]  = 4.0;  cRe[12] = 4.0;
    cRe[19] = -9.0; cRe[20] = -1.9; cRe[24] = 0.2;
    double a1 = 2.0 * M_PI * x1, a2 = 2.0 * M_PI * x2;
    double t1R = cos(a1), t1I = sin(a1);
    double t2R = cos(a2), t2I = sin(a2);
    double t2_2R = t2R*t2R - t2I*t2I, t2_2I = 2.0*t2R*t2I;
    double t2_3R = t2_2R*t2R - t2_2I*t2I, t2_3I = t2_2R*t2I + t2_2I*t2R;
    double t1_2R = t1R*t1R - t1I*t1I, t1_2I = 2.0*t1R*t1I;
    double t1_3R = t1_2R*t1R - t1_2I*t1I, t1_3I = t1_2R*t1I + t1_2I*t1R;
    double s6R = t2_3R + t2_2R - t2R - 1.0, s6I = t2_3I + t2_2I - t2I;
    cRe[6] = -100.0*s6I; cIm[6] = 100.0*s6R;
    double s8R = t1_3R + t1_2R + t2R - 1.0, s8I = t1_3I + t1_2I + t2I;
    cRe[8] = -100.0*s8I; cIm[8] = 100.0*s8R;
    double s14R = t2_3R - t2_2R + t2R - 1.0, s14I = t2_3I - t2_2I + t2I;
    cRe[14] = -100.0*s14I; cIm[14] = 100.0*s14R;
}

static void rev_giga_5(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS]; int n;
    giga_5(x1, x2, tmpRe, tmpIm, &n); *nCoeffs = n;
    for (int k = 0; k < n; k++) { cRe[k] = tmpRe[n-1-k]; cIm[k] = tmpIm[n-1-k]; }
}

static void giga_42(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 50;
    memset(cRe, 0, 50*sizeof(double)); memset(cIm, 0, 50*sizeof(double));
    cRe[0]=1.0; cRe[7]=-3.0; cRe[15]=3.0; cRe[31]=-1.0; cRe[39]=2.0;
    double a1=2.0*M_PI*x1, a2=2.0*M_PI*x2;
    double t1R=cos(a1),t1I=sin(a1),t2R=cos(a2),t2I=sin(a2);
    double t1_2R=t1R*t1R-t1I*t1I, t1_2I=2.0*t1R*t1I;
    double t2_2R=t2R*t2R-t2I*t2I, t2_2I=2.0*t2R*t2I;
    double t1_3R=t1_2R*t1R-t1_2I*t1I, t1_3I=t1_2R*t1I+t1_2I*t1R;
    double t2_3R=t2_2R*t2R-t2_2I*t2I, t2_3I=t2_2R*t2I+t2_2I*t2R;
    double sR=t1_2R+t2_2R, sI=t1_2I+t2_2I, eS=exp(sR);
    cRe[11]=-100.0*eS*sin(sI); cIm[11]=100.0*eS*cos(sI);
    cRe[19]=50.0*(t1_3R+t2_3R); cIm[19]=50.0*(t1_3I+t2_3I);
    double dR=t1R-t2R, dI=t1I-t2I, ed=exp(-dI);
    cRe[24]=ed*cos(dR)+10.0*t1_2R; cIm[24]=ed*sin(dR)+10.0*t1_2I;
    double sumR=t1R+t2R, sumI=t1I+t2I;
    double sinSR=sin(sumR)*cosh(sumI), sinSI=cos(sumR)*sinh(sumI);
    double difR=t1R-t2R, difI=t1I-t2I;
    double cosDR=cos(difR)*cosh(difI), cosDI=-sin(difR)*sinh(difI);
    cRe[44]=200.0*sinSR-cosDI; cIm[44]=200.0*sinSI+cosDR;
}

static void rev_giga_42(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS]; int n;
    giga_42(x1, x2, tmpRe, tmpIm, &n); *nCoeffs = n;
    for (int k = 0; k < n; k++) { cRe[k] = tmpRe[n-1-k]; cIm[k] = tmpIm[n-1-k]; }
}

static void giga_43(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 40;
    memset(cRe, 0, 40*sizeof(double)); memset(cIm, 0, 40*sizeof(double));
    cRe[0]=1.0; cRe[4]=-5.0; cRe[14]=10.0; cRe[29]=-20.0;
    double a1=2.0*M_PI*x1, a2=2.0*M_PI*x2;
    double t1R=cos(a1),t1I=sin(a1),t2R=cos(a2),t2I=sin(a2);
    double t1_2R=t1R*t1R-t1I*t1I, t1_2I=2.0*t1R*t1I;
    double t2_2R=t2R*t2R-t2I*t2I, t2_2I=2.0*t2R*t2I;
    double t1_3R=t1_2R*t1R-t1_2I*t1I, t1_3I=t1_2R*t1I+t1_2I*t1R;
    double t2_3R=t2_2R*t2R-t2_2I*t2I, t2_3I=t2_2R*t2I+t2_2I*t2R;
    double d20R=t1_3R-t2_3R, d20I=t1_3I-t2_3I;
    cRe[19]=-100.0*d20I; cIm[19]=100.0*d20R;
    double prodR=t1_2R*t2R-t1_2I*t2I, prodI=t1_2R*t2I+t1_2I*t2R;
    cRe[9]=50.0*(prodR-t2_2I); cIm[9]=50.0*(prodI+t2_2R);
    double e1=exp(-t1I), e2=exp(t2I);
    cRe[24]=e1*cos(t1R)+e2*cos(t2R); cIm[24]=e1*sin(t1R)-e2*sin(t2R);
    double pR=t1R*t2R-t1I*t2I, pI=t1R*t2I+t1I*t2R;
    double sR=t1R+t2R, sI=t1I+t2I;
    double sinSR=sin(sR)*cosh(sI), sinSI=cos(sR)*sinh(sI);
    cRe[34]=200.0*(pR*sinSR-pI*sinSI); cIm[34]=200.0*(pR*sinSI+pI*sinSR);
}

static void rev_giga_43(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS]; int n;
    giga_43(x1, x2, tmpRe, tmpIm, &n); *nCoeffs = n;
    for (int k = 0; k < n; k++) { cRe[k] = tmpRe[n-1-k]; cIm[k] = tmpIm[n-1-k]; }
}

static void giga_87(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 51;
    memset(cRe, 0, 51*sizeof(double)); memset(cIm, 0, 51*sizeof(double));
    double a1=2.0*M_PI*x1, a2=2.0*M_PI*x2;
    double t1R=cos(a1),t1I=sin(a1),t2R=cos(a2),t2I=sin(a2);
    double t1t2R=t1R*t2R-t1I*t2I, t1t2I=t1R*t2I+t1I*t2R;
    #define CABS(re, im) sqrt((re)*(re)+(im)*(im))
    #define CSIN_RE(re, im) (sin(re)*cosh(im))
    #define CSIN_IM(re, im) (cos(re)*sinh(im))
    #define CCOS_RE(re, im) (cos(re)*cosh(im))
    #define CCOS_IM(re, im) (-sin(re)*sinh(im))
    cRe[0]=t1R+t2R; cIm[0]=t1I+t2I;
    double sumR=t1R+t2R, sumI=t1I+t2I;
    double logVal=log(CABS(sumR,sumI)+1.0);
    cRe[1]=1.0+t1t2R+logVal; cIm[1]=t1t2I;
    double dR=1.0-t1t2R, dI=-t1t2I;
    logVal=log(CABS(dR,dI)+1.0);
    cRe[2]=t1R+t2R+logVal; cIm[2]=t1I+t2I;
    for (int i=4; i<=51; i++) {
        int ci=i-1; double di=(double)i;
        double rR=di*t1R+(51.0-di)*t2R, rI=di*t1I+(51.0-di)*t2I;
        double diffR=t1R-t2R*di, diffI=t1I-t2I*di;
        logVal=log(CABS(diffR,diffI)+1.0);
        cRe[ci]=rR+logVal; cIm[ci]=rI;
    }
    double sinT1R=CSIN_RE(t1R,t1I), sinT1I=CSIN_IM(t1R,t1I);
    double cosT2R=CCOS_RE(t2R,t2I), cosT2I=CCOS_IM(t2R,t2I);
    double sinT2R=CSIN_RE(t2R,t2I), sinT2I=CSIN_IM(t2R,t2I);
    double loop30R=cRe[30], loop30I=cIm[30];
    double loop40R=cRe[40], loop40I=cIm[40];
    cRe[10]=cRe[0]+cRe[9]-sinT1R; cIm[10]=cIm[0]+cIm[9]-sinT1I;
    cRe[20]=loop30R+loop40R-cosT2R; cIm[20]=loop30I+loop40I-cosT2I;
    cRe[30]=cRe[20]+loop40R+sinT1R; cIm[30]=cIm[20]+loop40I+sinT1I;
    cRe[40]=cRe[30]+cRe[20]-cosT2R; cIm[40]=cIm[30]+cIm[20]-cosT2I;
    cRe[50]=cRe[40]+cRe[20]+sinT2R; cIm[50]=cIm[40]+cIm[20]+sinT2I;
    #undef CABS
    #undef CSIN_RE
    #undef CSIN_IM
    #undef CCOS_RE
    #undef CCOS_IM
}

static void rev_giga_87(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS]; int n;
    giga_87(x1, x2, tmpRe, tmpIm, &n); *nCoeffs = n;
    for (int k = 0; k < n; k++) { cRe[k] = tmpRe[n-1-k]; cIm[k] = tmpIm[n-1-k]; }
}

static void giga_1(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    memset(cRe, 0, 25*sizeof(double)); memset(cIm, 0, 25*sizeof(double));
    double a1=2.0*M_PI*x1, a2=2.0*M_PI*x2;
    double t1R=cos(a1),t1I=sin(a1),t2R=cos(a2),t2I=sin(a2);
    double t1_2R=t1R*t1R-t1I*t1I, t1_2I=2.0*t1R*t1I;
    double t1_3R=t1_2R*t1R-t1_2I*t1I, t1_3I=t1_2R*t1I+t1_2I*t1R;
    double t1_4R=t1_3R*t1R-t1_3I*t1I, t1_4I=t1_3R*t1I+t1_3I*t1R;
    double t2_2R=t2R*t2R-t2I*t2I, t2_2I=2.0*t2R*t2I;
    double t2_3R=t2_2R*t2R-t2_2I*t2I, t2_3I=t2_2R*t2I+t2_2I*t2R;
    double t2_4R=t2_3R*t2R-t2_3I*t2I, t2_4I=t2_3R*t2I+t2_3I*t2R;
    double t1t2R=t1R*t2R-t1I*t2I, t1t2I=t1R*t2I+t1I*t2R;
    double t1_2t2R=t1_2R*t2R-t1_2I*t2I, t1_2t2I=t1_2R*t2I+t1_2I*t2R;
    double t1t2_2R=t1R*t2_2R-t1I*t2_2I, t1t2_2I=t1R*t2_2I+t1I*t2_2R;
    double t1_4t2_4R=t1_4R*t2_4R-t1_4I*t2_4I, t1_4t2_4I=t1_4R*t2_4I+t1_4I*t2_4R;
    /* R[k] → rev → C[25-k] */
    cRe[24]=30.0*t1_2t2R;  cIm[24]=30.0*t1_2t2I;   /* cf[1] */
    cRe[23]=30.0*t1t2_2R;  cIm[23]=30.0*t1t2_2I;   /* cf[2] */
    cRe[22]=40.0*t1_3R;    cIm[22]=40.0*t1_3I;      /* cf[3] */
    cRe[21]=40.0*t2_3R;    cIm[21]=40.0*t2_3I;      /* cf[4] */
    cRe[20]=-25.0*t1_2R;   cIm[20]=-25.0*t1_2I;     /* cf[5] */
    cRe[19]=-25.0*t2_2R;   cIm[19]=-25.0*t2_2I;     /* cf[6] */
    cRe[18]=10.0*t1t2R;    cIm[18]=10.0*t1t2I;      /* cf[7] */
    cRe[15]=100.0*t1_4t2_4R; cIm[15]=100.0*t1_4t2_4I; /* cf[10] */
    cRe[13]=-5.0*t1R;      cIm[13]=-5.0*t1I;        /* cf[12] */
    cRe[11]=5.0*t2R;       cIm[11]=5.0*t2I;         /* cf[14] */
    cRe[0]=-10.0;                                     /* cf[25] */
}

static void rev_giga_1(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS]; int n;
    giga_1(x1, x2, tmpRe, tmpIm, &n); *nCoeffs = n;
    for (int k = 0; k < n; k++) { cRe[k] = tmpRe[n-1-k]; cIm[k] = tmpIm[n-1-k]; }
}

static void giga_19(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 90;
    double tmpRe[90], tmpIm[90];
    memset(tmpRe, 0, 90*sizeof(double)); memset(tmpIm, 0, 90*sizeof(double));
    double t1 = x1, t2 = x2;  /* raw 0-to-1, no unit circle */
    tmpRe[0] = t1 - t2; tmpIm[0] = 0.0;
    for (int k = 2; k <= 90; k++) {
        int ci = k-1, prev = ci-1;
        double zR = (double)k*tmpRe[prev], zI = (double)k*tmpIm[prev];
        double sinzR = sin(zR)*cosh(zI), sinzI = cos(zR)*sinh(zI);
        double coskt1 = cos((double)k*t1);
        double vR = sinzR + coskt1, vI = sinzI;
        double av = sqrt(vR*vR + vI*vI);
        if (isfinite(av) && av > 1e-10) {
            double nR = vR/av, nI = vI/av;
            tmpRe[ci] = -nI; tmpIm[ci] = nR;
        } else {
            tmpRe[ci] = t1 + t2; tmpIm[ci] = 0.0;
        }
    }
    /* Reverse: R ascending → C leading-first */
    for (int k = 0; k < 90; k++) {
        cRe[k] = tmpRe[89-k]; cIm[k] = tmpIm[89-k];
    }
}

static void rev_giga_19(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS]; int n;
    giga_19(x1, x2, tmpRe, tmpIm, &n); *nCoeffs = n;
    for (int k = 0; k < n; k++) { cRe[k] = tmpRe[n-1-k]; cIm[k] = tmpIm[n-1-k]; }
}

/* giga_30: degree-9, 10 coefficients. Unit circle. R[k] → C[10-k]. */
static void giga_30(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 10;
    memset(cRe, 0, 10*sizeof(double)); memset(cIm, 0, 10*sizeof(double));
    double a1 = 2.0*M_PI*x1, a2 = 2.0*M_PI*x2;
    double t1R = cos(a1), t1I = sin(a1), t2R = cos(a2), t2I = sin(a2);
    double t2_2R = t2R*t2R - t2I*t2I, t2_2I = 2.0*t2R*t2I;
    double t1_2R = t1R*t1R - t1I*t1I, t1_2I = 2.0*t1R*t1I;
    double t1_3R = t1_2R*t1R - t1_2I*t1I, t1_3I = t1_2R*t1I + t1_2I*t1R;
    /* cf[1] = 150i*t2^2 + 100*t1^3 → C[9] */
    cRe[9] = -150.0*t2_2I + 100.0*t1_3R; cIm[9] = 150.0*t2_2R + 100.0*t1_3I;
    /* cf[5] = 150*|t1+t2-2.5*(1+i)| → C[5] */
    double dR = t1R+t2R-2.5, dI = t1I+t2I-2.5;
    cRe[5] = 150.0*sqrt(dR*dR + dI*dI);
    /* cf[10] = 100i*t1^3 + 150*t2^2 → C[0] */
    cRe[0] = -100.0*t1_3I + 150.0*t2_2R; cIm[0] = 100.0*t1_3R + 150.0*t2_2I;
}
static void rev_giga_30(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS]; int n;
    giga_30(x1, x2, tmpRe, tmpIm, &n); *nCoeffs = n;
    for (int k = 0; k < n; k++) { cRe[k] = tmpRe[n-1-k]; cIm[k] = tmpIm[n-1-k]; }
}

/* giga_39: degree-49, 50 coefficients. Unit circle. R[k] → C[50-k]. */
static void giga_39(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 50;
    memset(cRe, 0, 50*sizeof(double)); memset(cIm, 0, 50*sizeof(double));
    double a1 = 2.0*M_PI*x1, a2 = 2.0*M_PI*x2;
    double t1R = cos(a1), t1I = sin(a1), t2R = cos(a2), t2I = sin(a2);
    cRe[49]=1; cRe[40]=2; cRe[30]=-3; cRe[20]=4; cRe[10]=-5; cRe[0]=6;
    double t1_2R=t1R*t1R-t1I*t1I, t1_2I=2*t1R*t1I;
    double t2_2R=t2R*t2R-t2I*t2I, t2_2I=2*t2R*t2I;
    double t1_3R=t1_2R*t1R-t1_2I*t1I, t1_3I=t1_2R*t1I+t1_2I*t1R;
    double t2_3R=t2_2R*t2R-t2_2I*t2I, t2_3I=t2_2R*t2I+t2_2I*t2R;
    double t1t2R=t1R*t2R-t1I*t2I, t1t2I=t1R*t2I+t1I*t2R;
    /* cf[15]=100*(t1^2+t2^2) → C[35] */
    cRe[35]=100*(t1_2R+t2_2R); cIm[35]=100*(t1_2I+t2_2I);
    /* cf[25]=50*(sin(t1)+i*cos(t2)) → C[25] */
    double sint1R=sin(t1R)*cosh(t1I), sint1I=cos(t1R)*sinh(t1I);
    double cost2R_v=cos(t2R)*cosh(t2I), cost2I_v=-sin(t2R)*sinh(t2I);
    cRe[25]=50*(sint1R-cost2I_v); cIm[25]=50*(sint1I+cost2R_v);
    /* cf[35]=200*(t1*t2)+i*(t1^3-t2^3) → C[15] */
    cRe[15]=200*t1t2R-(t1_3I-t2_3I); cIm[15]=200*t1t2I+(t1_3R-t2_3R);
    /* cf[45]=exp(i*(t1+t2))+exp(-i*(t1-t2)) → C[5] */
    double sR=t1R+t2R, sI_v=t1I+t2I;
    double ea=exp(-sI_v), eaR=ea*cos(sR), eaI=ea*sin(sR);
    double dmR=t1R-t2R, dmI=t1I-t2I;
    double eb=exp(dmI), ebR=eb*cos(dmR), ebI=-eb*sin(dmR);
    cRe[5]=eaR+ebR; cIm[5]=eaI+ebI;
}
static void rev_giga_39(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS]; int n;
    giga_39(x1, x2, tmpRe, tmpIm, &n); *nCoeffs = n;
    for (int k = 0; k < n; k++) { cRe[k] = tmpRe[n-1-k]; cIm[k] = tmpIm[n-1-k]; }
}

/* giga_40: degree-34, 35 coefficients. Unit circle. R[k] → C[35-k]. */
static void giga_40(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    memset(cRe, 0, 35*sizeof(double)); memset(cIm, 0, 35*sizeof(double));
    double a1 = 2.0*M_PI*x1, a2 = 2.0*M_PI*x2;
    double t1R = cos(a1), t1I = sin(a1), t2R = cos(a2), t2I = sin(a2);
    cRe[34]=1; cRe[28]=-2; cRe[20]=3; cRe[15]=-4; cRe[8]=5; cRe[0]=-6;
    double t1_2R=t1R*t1R-t1I*t1I, t1_2I=2*t1R*t1I;
    double t2_2R=t2R*t2R-t2I*t2I, t2_2I=2*t2R*t2I;
    double t1_3R=t1_2R*t1R-t1_2I*t1I, t1_3I=t1_2R*t1I+t1_2I*t1R;
    double t2_3R=t2_2R*t2R-t2_2I*t2I, t2_3I=t2_2R*t2I+t2_2I*t2R;
    double t1t2R=t1R*t2R-t1I*t2I, t1t2I=t1R*t2I+t1I*t2R;
    /* cf[12]=50i*sin(t1^2-t2^2) → C[23] */
    double wR=t1_2R-t2_2R, wI=t1_2I-t2_2I;
    double sinwR=sin(wR)*cosh(wI), sinwI=cos(wR)*sinh(wI);
    cRe[23]=-50*sinwI; cIm[23]=50*sinwR;
    /* cf[18]=100*(cos(t1)+i*sin(t2)) → C[17] */
    double cost1R_v=cos(t1R)*cosh(t1I), cost1I_v=-sin(t1R)*sinh(t1I);
    double sint2R_v=sin(t2R)*cosh(t2I), sint2I_v=cos(t2R)*sinh(t2I);
    cRe[17]=100*(cost1R_v-sint2I_v); cIm[17]=100*(cost1I_v+sint2R_v);
    /* cf[25]=50*(t1^3-t2^3+i*t1*t2) → C[10] */
    cRe[10]=50*(t1_3R-t2_3R-t1t2I); cIm[10]=50*(t1_3I-t2_3I+t1t2R);
    /* cf[30]=200*exp(i*t1)+50*exp(-i*t2) → C[5] */
    double e1=exp(-t1I), eit1R=e1*cos(t1R), eit1I=e1*sin(t1R);
    double e2=exp(t2I), emit2R=e2*cos(t2R), emit2I=-e2*sin(t2R);
    cRe[5]=200*eit1R+50*emit2R; cIm[5]=200*eit1I+50*emit2I;
}
static void rev_giga_40(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS]; int n;
    giga_40(x1, x2, tmpRe, tmpIm, &n); *nCoeffs = n;
    for (int k = 0; k < n; k++) { cRe[k] = tmpRe[n-1-k]; cIm[k] = tmpIm[n-1-k]; }
}

/* p7f: degree-22, 23 coefficients. Raw t1=x1, t2=x2 (0 to 1, no unit circle).
 * tt1=exp(i*2π*t1), ttt1=exp(i*2π*tt1), v=linspace(Re(tt1),Re(ttt1),23).
 * f[k]=scale*exp(i*trig(freq*2π*v[k])), branch on t2. f[22]+=211*exp(i*2π/7*t2). */
static void p7f(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 23;
    double t1=x1, t2=x2, pi2=2.0*M_PI;
    double a1=pi2*t1, tt1R=cos(a1), tt1I=sin(a1);
    double ttt1R=exp(-pi2*tt1I)*cos(pi2*tt1R);
    double v0=tt1R, dv=(ttt1R-v0)/22.0;
    double scale, freq; int useCos=0;
    if (t2<0.1) { scale=10*t1; freq=11; }
    else if (t2<0.2) { scale=100; freq=17; }
    else if (t2<0.3) { scale=599; freq=83; useCos=1; }
    else if (t2<0.4) { scale=443; freq=179; }
    else if (t2<0.5) { scale=293; freq=127; }
    else if (t2<0.6) { scale=541; freq=103; }
    else if (t2<0.7) { scale=379; freq=283; }
    else if (t2<0.8) { scale=233; freq=3; }
    else if (t2<0.9) { scale=173; freq=5; }
    else { scale=257; freq=23; }
    double fRe[23], fIm[23];
    for (int k=0; k<23; k++) {
        double vk=v0+k*dv;
        double tv=useCos ? cos(freq*pi2*vk) : sin(freq*pi2*vk);
        fRe[k]=scale*cos(tv); fIm[k]=scale*sin(tv);
    }
    double aa=pi2*(1.0/7.0)*t2;
    fRe[22]+=211*cos(aa); fIm[22]+=211*sin(aa);
    for (int k=0; k<23; k++) { cRe[k]=fRe[22-k]; cIm[k]=fIm[22-k]; }
}
static void rev_p7f(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS]; int n;
    p7f(x1, x2, tmpRe, tmpIm, &n); *nCoeffs = n;
    for (int k = 0; k < n; k++) { cRe[k] = tmpRe[n-1-k]; cIm[k] = tmpIm[n-1-k]; }
}

/* poly_110: degree-70, 71 coefficients. Unit circle + coeff5 transform.
 * coeff5: t1=t1+1/t2, t2=t2+1/t1 (on unit circle: 1/z=conj(z)).
 * Symmetric fill: cf[k-1] and cf[70-k] for k=1..35, cf[35]=middle, cf[70]=0.
 * No reversal (cf[70]=0 → zero leading if reversed). */
static void poly_110(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    memset(cRe, 0, 71*sizeof(double)); memset(cIm, 0, 71*sizeof(double));
    double a1=2.0*M_PI*x1, a2=2.0*M_PI*x2;
    double t1R=cos(a1), t1I=sin(a1), t2R=cos(a2), t2I=sin(a2);
    /* coeff5: t1+conj(t2), t2+conj(t1) */
    double nt1R=t1R+t2R, nt1I=t1I-t2I, nt2R=t2R+t1R, nt2I=t2I-t1I;
    t1R=nt1R; t1I=nt1I; t2R=nt2R; t2I=nt2I;
    static const int P[]={2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59};
    for (int k=1; k<=35; k++) {
        double k2=(double)(k*k);
        cRe[k-1] = t1R*P[k%17] + t2I*k2;
        cRe[70-k] = t2R*P[(70-k)%17] - t1I*k2;
    }
    cRe[35]=440.0*cos(atan2(t1I,t1R)); cIm[35]=440.0*sin(atan2(t2I,t2R));
}
static void rev_poly_110(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS]; int n;
    poly_110(x1, x2, tmpRe, tmpIm, &n); *nCoeffs = n;
    for (int k = 0; k < n; k++) { cRe[k] = tmpRe[n-1-k]; cIm[k] = tmpIm[n-1-k]; }
}

/* giga_227: degree-24, 25 coefficients. Pipeline: uc → coeff3 → poly_giga_62 → rev.
 * coeff3: t=1/(t+2). poly_giga_62: 5 blocks of 5 (real coeffs). rev applied. */
static void giga_227(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    memset(cRe, 0, 25*sizeof(double)); memset(cIm, 0, 25*sizeof(double));
    double a1=2.0*M_PI*x1, a2=2.0*M_PI*x2;
    double t1R=cos(a1), t1I=sin(a1), t2R=cos(a2), t2I=sin(a2);
    /* coeff3: 1/(t+2) */
    double d1R=t1R+2, d1I=t1I, m1=d1R*d1R+d1I*d1I;
    t1R=d1R/m1; t1I=-d1I/m1;
    double d2R=t2R+2, d2I=t2I, m2=d2R*d2R+d2I*d2I;
    t2R=d2R/m2; t2I=-d2I/m2;
    /* |t1+t2| */
    double sR=t1R+t2R, sI=t1I+t2I, absS=sqrt(sR*sR+sI*sI);
    double cf0[5]; double pw=absS;
    for (int i=0;i<5;i++){cf0[i]=pw; pw*=absS;}
    /* (t1+2j*t2)^3 real part */
    double wR=t1R-2*t2I, wI=t1I+2*t2R;
    double w2R=wR*wR-wI*wI, w2I=2*wR*wI;
    double w3R=w2R*wR-w2I*wI;
    /* |t1*t2|, log */
    double pR=t1R*t2R-t1I*t2I, pI=t1R*t2I+t1I*t2R;
    double absP=sqrt(pR*pR+pI*pI);
    double logP=(absP>1e-300)?log(absP):-690.0;
    double val5=w3R*logP;
    /* (t1-t2)^2 imag / angle(t1*t2) */
    double dR=t1R-t2R, ddI=t1I-t2I;
    double diff2I=2*dR*ddI, angleP=atan2(pI,pR);
    double val10=(fabs(angleP)>1e-15)?(diff2I/angleP):0.0;
    double val15=sqrt(fabs(val5));
    double cf20[5]; pw=absP;
    for (int i=0;i<5;i++){cf20[i]=pw; pw*=absP;}
    /* rev: C[k]=cf[24-k] */
    for (int i=0;i<5;i++) cRe[i]=cf20[4-i];
    for (int i=5;i<10;i++) cRe[i]=val15;
    for (int i=10;i<15;i++) cRe[i]=val10;
    for (int i=15;i<20;i++) cRe[i]=val5;
    for (int i=0;i<5;i++) cRe[20+i]=cf0[4-i];
}
static void rev_giga_227(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS]; int n;
    giga_227(x1, x2, tmpRe, tmpIm, &n); *nCoeffs = n;
    for (int k = 0; k < n; k++) { cRe[k] = tmpRe[n-1-k]; cIm[k] = tmpIm[n-1-k]; }
}

/* giga_230: degree-9, 10 coefficients. Pipeline: uc → coeff3 → poly_giga_53 → rev.
 * coeff3: t=1/(t+2). Complex trig, powers, conditional, complex power. */
static void giga_230(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 10;
    memset(cRe, 0, 10*sizeof(double)); memset(cIm, 0, 10*sizeof(double));
    double a1=2.0*M_PI*x1, a2=2.0*M_PI*x2;
    double u1R=cos(a1), u1I=sin(a1), u2R=cos(a2), u2I=sin(a2);
    double dd1R=u1R+2, dd1I=u1I, mm1=dd1R*dd1R+dd1I*dd1I;
    double t1R=dd1R/mm1, t1I=-dd1I/mm1;
    double dd2R=u2R+2, dd2I=u2I, mm2=dd2R*dd2R+dd2I*dd2I;
    double t2R=dd2R/mm2, t2I=-dd2I/mm2;
    double absT1=sqrt(t1R*t1R+t1I*t1I), absT2=sqrt(t2R*t2R+t2I*t2I);
    double pR=t1R*t2R-t1I*t2I, pI=t1R*t2I+t1I*t2R;
    double difR=t1R-t2R, difI=t1I-t2I, sumR=t1R+t2R, sumI=t1I+t2I;
    double cfR[10]={0}, cfI[10]={0};
    /* cf[0]=100*sin(t1)^3*cos(t2)^2 */
    double st1R_v=sin(t1R)*cosh(t1I), st1I_v=cos(t1R)*sinh(t1I);
    double s2R=st1R_v*st1R_v-st1I_v*st1I_v, s2I=2*st1R_v*st1I_v;
    double s3R=s2R*st1R_v-s2I*st1I_v, s3I=s2R*st1I_v+s2I*st1R_v;
    double c2R_v=cos(t2R)*cosh(t2I), c2I_v=-sin(t2R)*sinh(t2I);
    double cc2R=c2R_v*c2R_v-c2I_v*c2I_v, cc2I=2*c2R_v*c2I_v;
    cfR[0]=100*(s3R*cc2R-s3I*cc2I); cfI[0]=100*(s3R*cc2I+s3I*cc2R);
    /* cf[1]=100*exp(i*(t1+t2))-10*(t1-t2)^2 */
    double ev1=exp(-sumI);
    cfR[1]=100*ev1*cos(sumR)-10*(difR*difR-difI*difI);
    cfI[1]=100*ev1*sin(sumR)-10*2*difR*difI;
    /* cf[2]=t1*t2*(t1-t2)/(|t1|+|t2|+1) */
    double pdRv=pR*difR-pI*difI, pdIv=pR*difI+pI*difR, dn=absT1+absT2+1;
    cfR[2]=pdRv/dn; cfI[2]=pdIv/dn;
    /* cf[4]=(t1*t2*exp(i*(t1^2-t2^2)))^3 */
    double t12R=t1R*t1R-t1I*t1I, t12I=2*t1R*t1I;
    double t22R=t2R*t2R-t2I*t2I, t22I=2*t2R*t2I;
    double dqR=t12R-t22R, dqI=t12I-t22I;
    double ev4=exp(-dqI), e4R=ev4*cos(dqR), e4I=ev4*sin(dqR);
    double q4R=pR*e4R-pI*e4I, q4I=pR*e4I+pI*e4R;
    double q42R=q4R*q4R-q4I*q4I, q42I=2*q4R*q4I;
    cfR[4]=q42R*q4R-q42I*q4I; cfI[4]=q42R*q4I+q42I*q4R;
    /* cf[6]=sqrt(|t1|)-sqrt(|t2|)+i*sin(t1*t2) */
    double spR=sin(pR)*cosh(pI), spI=cos(pR)*sinh(pI);
    cfR[6]=sqrt(absT1)-sqrt(absT2)-spI; cfI[6]=spR;
    /* cf[7]=50*|t1-t2|*exp(i*|t1+t2|) */
    double ad=sqrt(difR*difR+difI*difI), as=sqrt(sumR*sumR+sumI*sumI);
    cfR[7]=50*ad*cos(as); cfI[7]=50*ad*sin(as);
    /* cf[8] */
    if (t1I>0) {cfR[8]=t1R-absT2; cfI[8]=t1I;}
    else {cfR[8]=t2R-absT1; cfI[8]=t2I;}
    /* cf[9]=(i*t1*t2)^(0.1*t1*t2) */
    double zzR=-pI, zzI=pR, wwR=0.1*pR, wwI=0.1*pI;
    double azz=sqrt(zzR*zzR+zzI*zzI);
    if (azz>1e-300) {
        double lR=log(azz), lI=atan2(zzI,zzR);
        double wlR=wwR*lR-wwI*lI, wlI=wwR*lI+wwI*lR;
        double ew=exp(wlR); cfR[9]=ew*cos(wlI); cfI[9]=ew*sin(wlI);
    }
    for (int k=0;k<10;k++) {
        double re=cfR[9-k], im=cfI[9-k];
        cRe[k]=isfinite(re)?re:0; cIm[k]=isfinite(im)?im:0;
    }
}
static void rev_giga_230(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS]; int n;
    giga_230(x1, x2, tmpRe, tmpIm, &n); *nCoeffs = n;
    for (int k = 0; k < n; k++) { cRe[k] = tmpRe[n-1-k]; cIm[k] = tmpIm[n-1-k]; }
}

/* giga_232: degree-8, 9 coefficients. Pipeline: uc → coeff2 → poly_729 → rev → safe.
 * coeff2: t1=t1+t2, t2=t1*t2. poly_729: loop j=0..8 with complex coeff formula. */
static void giga_232(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 9;
    memset(cRe, 0, 9*sizeof(double)); memset(cIm, 0, 9*sizeof(double));
    double a1=2.0*M_PI*x1, a2=2.0*M_PI*x2;
    double u1R=cos(a1), u1I=sin(a1), u2R=cos(a2), u2I=sin(a2);
    double t1R=u1R+u2R, t1I=u1I+u2I;
    double t2R=u1R*u2R-u1I*u2I, t2I=u1R*u2I+u1I*u2R;
    double absT1=sqrt(t1R*t1R+t1I*t1I), absT2=sqrt(t2R*t2R+t2I*t2I);
    double angT1=atan2(t1I,t1R), angT2=atan2(t2I,t2R);
    double a1p[4]; a1p[1]=absT1; a1p[2]=absT1*absT1; a1p[3]=a1p[2]*absT1;
    double a2p[9]; a2p[0]=1;
    for (int i=1;i<=8;i++) a2p[i]=a2p[i-1]*absT2;
    double cfR[9], cfI[9];
    for (int j=0;j<=8;j++) {
        double jd=(double)j;
        double rp=t1R*(jd*jd)-t2R*sqrt(jd+1);
        double ip=(t1I+t2I)*log(jd+2);
        double mag=a1p[(j%3)+1]+a2p[8-j];
        double ang=angT1*sin(jd)+angT2*cos(jd);
        double zmR=rp*mag, zmI=ip*mag, eR=cos(ang), eI=sin(ang);
        cfR[j]=zmR*eR-zmI*eI; cfI[j]=zmR*eI+zmI*eR;
    }
    for (int k=0;k<9;k++) {
        double re=cfR[8-k], im=cfI[8-k];
        cRe[k]=isfinite(re)?re:0; cIm[k]=isfinite(im)?im:0;
    }
}
static void rev_giga_232(double x1, double x2, double *cRe, double *cIm, int *nCoeffs) {
    double tmpRe[MAX_COEFFS], tmpIm[MAX_COEFFS]; int n;
    giga_232(x1, x2, tmpRe, tmpIm, &n); *nCoeffs = n;
    for (int k = 0; k < n; k++) { cRe[k] = tmpRe[n-1-k]; cIm[k] = tmpIm[n-1-k]; }
}

static CoeffFunc lookupFunction(const char *name) {
    if (strcmp(name, "giga_1") == 0) return giga_1;
    if (strcmp(name, "rev_giga_1") == 0) return rev_giga_1;
    if (strcmp(name, "giga_5") == 0) return giga_5;
    if (strcmp(name, "rev_giga_5") == 0) return rev_giga_5;
    if (strcmp(name, "giga_19") == 0) return giga_19;
    if (strcmp(name, "rev_giga_19") == 0) return rev_giga_19;
    if (strcmp(name, "giga_42") == 0) return giga_42;
    if (strcmp(name, "rev_giga_42") == 0) return rev_giga_42;
    if (strcmp(name, "giga_43") == 0) return giga_43;
    if (strcmp(name, "rev_giga_43") == 0) return rev_giga_43;
    if (strcmp(name, "giga_87") == 0) return giga_87;
    if (strcmp(name, "rev_giga_87") == 0) return rev_giga_87;
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

/* ---- Main ---- */

int main(int argc, char **argv) {
    /* Read JSON from stdin */
    char buf[BUF_SIZE];
    int len = 0;
    while (len < BUF_SIZE - 1) {
        int ch = fgetc(stdin);
        if (ch == EOF) break;
        buf[len++] = (char)ch;
    }
    buf[len] = '\0';

    /* Parse */
    char funcName[64] = "";
    const char *cp = findKey(buf, "function");
    if (cp) parseString(cp, funcName, sizeof(funcName));

    int n1 = 100, n2 = 100;
    cp = findKey(buf, "n1");
    if (cp) n1 = (int)parseNum(&cp);
    cp = findKey(buf, "n2");
    if (cp) n2 = (int)parseNum(&cp);
    if (n1 < 1) n1 = 1;
    if (n2 < 1) n2 = 1;

    double quantile = 0.0;  /* default 0 = true bounding box */
    cp = findKey(buf, "quantile");
    if (cp) quantile = parseNum(&cp);
    if (quantile < 0) quantile = 0;
    if (quantile > 0.5) quantile = 0.5;

    double shim = 0.05;  /* viewport widening fraction, 5% margin */
    cp = findKey(buf, "shim");
    if (cp) shim = parseNum(&cp);
    if (shim < 0) shim = 0;
    if (shim > 1.0) shim = 1.0;

    CoeffFunc coeffFunc = lookupFunction(funcName);
    if (!coeffFunc) {
        fprintf(stderr, "Unknown function: %s\n", funcName);
        return 1;
    }

    /* Probe degree */
    double coeffRe[MAX_COEFFS], coeffIm[MAX_COEFFS];
    int nCoeffs;
    coeffFunc(0.0, 0.0, coeffRe, coeffIm, &nCoeffs);
    int degree = nCoeffs - 1;

    /* Sample grid and collect all root positions.
     * Target ~100 samples per axis (10K solves max).
     * Viewport is a rough quantile estimate — high resolution is wasteful. */
    int sampleSkip = 1;
    if (n1 > 100 || n2 > 100) {
        int maxDim = n1 > n2 ? n1 : n2;
        sampleSkip = (maxDim + 99) / 100;  /* ceil(maxDim/100) */
    }
    int sampN1 = (n1 + sampleSkip - 1) / sampleSkip;
    int sampN2 = (n2 + sampleSkip - 1) / sampleSkip;
    int maxRoots = sampN1 * sampN2 * degree;

    double *allRe = malloc(maxRoots * sizeof(double));
    double *allIm = malloc(maxRoots * sizeof(double));
    if (!allRe || !allIm) {
        fprintf(stderr, "Cannot allocate %d root positions\n", maxRoots);
        return 1;
    }

    double rootRe[MAX_DEGREE], rootIm[MAX_DEGREE];
    int nRoots = 0;

    for (int i1 = 0; i1 < n1; i1 += sampleSkip) {
        double x1 = (double)i1 / (double)n1;
        for (int i2 = 0; i2 < n2; i2 += sampleSkip) {
            double x2 = (double)i2 / (double)n2;
            coeffFunc(x1, x2, coeffRe, coeffIm, &nCoeffs);

            /* Fresh initial guesses for each sample (no warm-start —
               sampleSkip makes consecutive points too far apart) */
            int deg0 = nCoeffs - 1;
            for (int k = 0; k < deg0; k++) {
                double ang = 2.0 * M_PI * k / deg0 + 0.3;
                double r = 1.0 + 0.1 * k / deg0;
                rootRe[k] = r * cos(ang);
                rootIm[k] = r * sin(ang);
            }

            int start = 0;
            while (start < nCoeffs - 1 &&
                   coeffRe[start]*coeffRe[start] + coeffIm[start]*coeffIm[start] < 1e-30)
                start++;
            int effN = nCoeffs - start;
            int effDeg = effN - 1;
            if (effDeg <= 0) continue;

            if (effDeg == 1) {
                double aR = coeffRe[start], aI = coeffIm[start];
                double bR = coeffRe[start+1], bI = coeffIm[start+1];
                double d = aR*aR + aI*aI;
                if (d > 1e-30) {
                    rootRe[0] = -(bR*aR + bI*aI) / d;
                    rootIm[0] = -(bI*aR - bR*aI) / d;
                }
            } else {
                solveEA(coeffRe + start, coeffIm + start, effN,
                        rootRe, rootIm, effDeg);
            }

            for (int r = 0; r < effDeg; r++) {
                if (nRoots < maxRoots) {
                    allRe[nRoots] = rootRe[r];
                    allIm[nRoots] = rootIm[r];
                    nRoots++;
                }
            }
        }
    }

    /* Compute viewport using quantiles */
    double centerRe = 0, centerIm = 0, scale = 1.0;
    double qMinRe = 0, qMaxRe = 0, qMinIm = 0, qMaxIm = 0;

    double shimMul = 1.0 + shim;  /* e.g. shim=0.05 → multiply range by 1.05 */
    if (nRoots > 10) {
        qsort(allRe, nRoots, sizeof(double), cmpDouble);
        qsort(allIm, nRoots, sizeof(double), cmpDouble);
        int lo = (int)(nRoots * quantile);
        int hi = (int)(nRoots * (1.0 - quantile)) - 1;
        if (lo < 0) lo = 0;
        if (hi >= nRoots) hi = nRoots - 1;
        if (hi <= lo) { lo = 0; hi = nRoots - 1; }
        qMinRe = allRe[lo]; qMaxRe = allRe[hi];
        qMinIm = allIm[lo]; qMaxIm = allIm[hi];
        if (qMaxRe > qMinRe && qMaxIm > qMinIm) {
            centerRe = (qMinRe + qMaxRe) / 2.0;
            centerIm = (qMinIm + qMaxIm) / 2.0;
            double rangeRe = (qMaxRe - qMinRe) * shimMul;
            double rangeIm = (qMaxIm - qMinIm) * shimMul;
            double scaleRe = REF_SIZE / rangeRe;
            double scaleIm = REF_SIZE / rangeIm;
            scale = scaleRe < scaleIm ? scaleRe : scaleIm;
        }
    } else if (nRoots > 0) {
        qMinRe = allRe[0]; qMaxRe = allRe[0];
        qMinIm = allIm[0]; qMaxIm = allIm[0];
        for (int i = 1; i < nRoots; i++) {
            if (allRe[i] < qMinRe) qMinRe = allRe[i];
            if (allRe[i] > qMaxRe) qMaxRe = allRe[i];
            if (allIm[i] < qMinIm) qMinIm = allIm[i];
            if (allIm[i] > qMaxIm) qMaxIm = allIm[i];
        }
        if (qMaxRe > qMinRe && qMaxIm > qMinIm) {
            centerRe = (qMinRe + qMaxRe) / 2.0;
            centerIm = (qMinIm + qMaxIm) / 2.0;
            double rangeRe = (qMaxRe - qMinRe) * shimMul;
            double rangeIm = (qMaxIm - qMinIm) * shimMul;
            double scaleRe = REF_SIZE / rangeRe;
            double scaleIm = REF_SIZE / rangeIm;
            scale = scaleRe < scaleIm ? scaleRe : scaleIm;
        }
    }

    free(allRe);
    free(allIm);

    printf("{\"center_re\":%.15g,\"center_im\":%.15g,\"scale\":%.15g,\"degree\":%d,"
           "\"n_roots\":%d,\"q_re\":[%.6g,%.6g],\"q_im\":[%.6g,%.6g]}\n",
           centerRe, centerIm, scale, degree,
           nRoots, qMinRe, qMaxRe, qMinIm, qMaxIm);

    return 0;
}
