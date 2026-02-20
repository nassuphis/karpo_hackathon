/*
 * Full worker step loop for WASM — replaces per-step JS↔WASM boundary crossing.
 * Contains: EA solver, curve interpolation, root matching, pixel output.
 * Compiled with: clang --target=wasm32-unknown-unknown -O3 -nostdlib
 *                -Wl,--import-memory -Wl,--stack-first -Wl,-z,stack-size=65536
 *
 * Memory layout:
 *   [0 .. 64KB)   C shadow stack (grows downward)
 *   [64KB .. end)  Config + data sections (JS-computed offsets)
 */

#define MAX_DEG    255
#define MAX_COEFFS 256
#define HUNGARIAN_MAX 32   /* cap for Hungarian O(n³) — 32×32×8 = 8KB on stack */
#define SOLVER_MAX_ITER 64
#define SOLVER_TOL2     1e-16
#define PROGRESS_INTERVAL 2000
#define PI 3.14159265358979323846

/* ================================================================
 * Imported functions from JS environment
 * ================================================================ */
__attribute__((import_module("env"), import_name("cos")))
extern double js_cos(double);

__attribute__((import_module("env"), import_name("sin")))
extern double js_sin(double);

__attribute__((import_module("env"), import_name("log")))
extern double js_log(double);

__attribute__((import_module("env"), import_name("reportProgress")))
extern void js_reportProgress(int step);

/* ================================================================
 * Config layout — flat int32 and float64 arrays in WASM memory
 * ================================================================ */

/* Config int32 indices */
#define CI_NCOEFFS        0
#define CI_NROOTS         1
#define CI_CANVAS_W       2
#define CI_CANVAS_H       3
#define CI_TOTAL_STEPS    4
#define CI_COLOR_MODE     5   /* 0=uniform, 1=index, 2=proximity, 3=derivative */
#define CI_MATCH_STRATEGY 6   /* 0=assign4, 1=assign1, 2=hungarian1 */
#define CI_MORPH_ENABLED  7
#define CI_N_ENTRIES      8
#define CI_N_DENTRIES     9
#define CI_N_FOLLOWC      10
#define CI_N_SEL_INDICES  11
#define CI_HAS_JIGGLE     12
#define CI_UNIFORM_R      13
#define CI_UNIFORM_G      14
#define CI_UNIFORM_B      15
#define CI_RNG_SEED0      16
#define CI_RNG_SEED1      17
#define CI_RNG_SEED2      18
#define CI_RNG_SEED3      19
/* Data section offsets (byte offsets into WASM memory) */
#define CI_OFF_COEFFS_RE     20
#define CI_OFF_COEFFS_IM     21
#define CI_OFF_COLORS_R      22
#define CI_OFF_COLORS_G      23
#define CI_OFF_COLORS_B      24
#define CI_OFF_JIGGLE_RE     25
#define CI_OFF_JIGGLE_IM     26
#define CI_OFF_MORPH_TGT_RE  27
#define CI_OFF_MORPH_TGT_IM  28
#define CI_OFF_PROX_PAL_R    29
#define CI_OFF_PROX_PAL_G    30
#define CI_OFF_PROX_PAL_B    31
#define CI_OFF_DERIV_PAL_R   32
#define CI_OFF_DERIV_PAL_G   33
#define CI_OFF_DERIV_PAL_B   34
#define CI_OFF_SEL_INDICES   35
#define CI_OFF_FOLLOWC_IDX   36
/* C-curve entry parallel arrays */
#define CI_OFF_ENTRY_IDX     37
#define CI_OFF_ENTRY_SPEED   38
#define CI_OFF_ENTRY_CCW     39
#define CI_OFF_ENTRY_DITHER  40
#define CI_OFF_CURVE_OFFSETS 41
#define CI_OFF_CURVE_LENGTHS 42
#define CI_OFF_CURVE_ISCLOUD 43
/* D-curve entry parallel arrays */
#define CI_OFF_DENTRY_IDX     44
#define CI_OFF_DENTRY_SPEED   45
#define CI_OFF_DENTRY_CCW     46
#define CI_OFF_DENTRY_DITHER  47
#define CI_OFF_DCURVE_OFFSETS 48
#define CI_OFF_DCURVE_LENGTHS 49
#define CI_OFF_DCURVE_ISCLOUD 50
/* Curve data + working arrays + output */
#define CI_OFF_CURVES_FLAT    51
#define CI_OFF_DCURVES_FLAT   52
#define CI_OFF_WORK_COEFFS_RE 53
#define CI_OFF_WORK_COEFFS_IM 54
#define CI_OFF_TMP_RE         55
#define CI_OFF_TMP_IM         56
#define CI_OFF_MORPH_WORK_RE  57
#define CI_OFF_MORPH_WORK_IM  58
#define CI_OFF_PASS_ROOTS_RE  59
#define CI_OFF_PASS_ROOTS_IM  60
#define CI_OFF_PAINT_IDX      61
#define CI_OFF_PAINT_R        62
#define CI_OFF_PAINT_G        63
#define CI_OFF_PAINT_B        64
#define CI_MORPH_PATH_TYPE    65   /* 0=line, 1=circle, 2=ellipse, 3=figure8 */
#define CI_MORPH_CCW          66   /* 0=CW (default), 1=CCW */
#define CI_OFF_ENTRY_DITHER_DIST  67  /* per-entry int32 array: 0=normal, 1=uniform */
#define CI_OFF_DENTRY_DITHER_DIST 68  /* per-D-entry int32 array: 0=normal, 1=uniform */
/* Total: 69 int32 config values */

/* Config float64 indices */
#define CD_RANGE              0
#define CD_FPS                1
#define CD_MORPH_RATE         2
#define CD_MORPH_ELLIPSE_MINOR 3   /* minor axis fraction (0.1–1.0), ellipse only */
#define CD_MORPH_DITHER_START  4   /* C/start dither sigma (absolute, max(cosθ,0)² envelope) */
#define CD_MORPH_DITHER_MID    5   /* midpoint dither sigma (absolute, sin²θ envelope) */
#define CD_MORPH_DITHER_END    6   /* D/end dither sigma (absolute, max(-cosθ,0)² envelope) */
#define CD_CENTER_X            7   /* bitmap viewport center X */
#define CD_CENTER_Y            8   /* bitmap viewport center Y */
/* Total: 9 float64 config values */

/* ================================================================
 * Global state (set by init)
 * ================================================================ */
static int *cfgI;
static double *cfgD;

static int nCoeffs, nRoots, canvasW, canvasH, totalSteps;
static int colorMode, matchStrategy, morphEnabled;
static int nEntries, nDEntries, nFollowC, nSelIndices, hasJiggle;
static int morphPathType, morphCcw;
static double bitmapRange, FPS, morphRate, morphEllipseMinor, morphDitherStart, morphDitherMid, morphDitherEnd;
static double centerX, centerY;

/* Data pointers */
static double *coeffsRe, *coeffsIm;
static unsigned char *colorsR, *colorsG, *colorsB;
static double *jiggleRe, *jiggleIm;
static double *morphTargetRe, *morphTargetIm;
static unsigned char *proxPalR, *proxPalG, *proxPalB;
static unsigned char *derivPalR, *derivPalG, *derivPalB;
static int *selIndices, *followCIdx;

/* C-curve entry arrays */
static int *entryIdx, *entryCcw, *entryDitherDist;
static double *entrySpeed, *entryDither;
static int *curveOffsets, *curveLengths, *curveIsCloud;
static double *curvesFlat;

/* D-curve entry arrays */
static int *dEntryIdx, *dEntryCcw, *dEntryDitherDist;
static double *dEntrySpeed, *dEntryDither;
static int *dCurveOffsets, *dCurveLengths, *dCurveIsCloud;
static double *dCurvesFlat;

/* Working arrays */
static double *workCoeffsRe, *workCoeffsIm;
static double *tmpRe, *tmpIm;
static double *morphWorkRe, *morphWorkIm;
static double *passRootsRe, *passRootsIm;

/* Output buffers */
static int *paintIdx;
static unsigned char *paintR, *paintG, *paintB;

/* ================================================================
 * Utility functions
 * ================================================================ */

/* Fractional part in [0, 1): equivalent to ((t%1)+1)%1 */
static double frac01(double t) {
    double f = t - (double)(long long)t;
    if (f < 0.0) f += 1.0;
    return f;
}

/* Integer power: base^n for non-negative int n */
static double ipow(double base, int n) {
    double result = 1.0;
    for (int i = 0; i < n; i++) result *= base;
    return result;
}

/* ---- PRNG: xorshift128 ---- */
static unsigned int rngS[4];

static unsigned int xorshift128(void) {
    unsigned int t = rngS[3];
    unsigned int s = rngS[0];
    rngS[3] = rngS[2]; rngS[2] = rngS[1]; rngS[1] = s;
    t ^= t << 11;
    t ^= t >> 8;
    rngS[0] = t ^ s ^ (s >> 19);
    return rngS[0];
}

static double rngUniform(void) {
    return (double)(xorshift128() >> 1) / 2147483648.0; /* [0, 1) */
}

static double gaussSpare;
static int gaussHasSpare;

static double rngGauss(void) {
    if (gaussHasSpare) { gaussHasSpare = 0; return gaussSpare; }
    double u, v;
    do { u = rngUniform(); } while (u == 0.0);
    v = rngUniform();
    double r = __builtin_sqrt(-2.0 * js_log(u));
    double theta = 2.0 * PI * v;
    gaussSpare = r * js_sin(theta);
    gaussHasSpare = 1;
    return r * js_cos(theta);
}

static double rngDither(int dist) {
    return dist ? (rngUniform() - 0.5) * 2.0 : rngGauss();
}

/* ================================================================
 * Ehrlich-Aberth solver (ported from solver.c)
 * ================================================================ */
static void solveEA(double *cRe, double *cIm, int nc,
                    double *warmRe, double *warmIm, int nr)
{
    int start = 0;
    while (start < nc - 1 &&
           cRe[start] * cRe[start] + cIm[start] * cIm[start] < 1e-30)
        start++;

    int degree = nc - 1 - start;
    if (degree <= 0) return;

    /* Linear case */
    if (degree == 1) {
        double aR = cRe[start], aI = cIm[start];
        double bR = cRe[start + 1], bI = cIm[start + 1];
        double d = aR * aR + aI * aI;
        if (d < 1e-30) return;
        warmRe[0] = -(bR * aR + bI * aI) / d;
        warmIm[0] = -(bI * aR - bR * aI) / d;
        return;
    }

    /* Copy stripped coefficients to stack */
    int n = nc - start;
    double cr[MAX_COEFFS], ci[MAX_COEFFS];
    for (int k = 0; k < n; k++) {
        cr[k] = cRe[start + k];
        ci[k] = cIm[start + k];
    }

    double rRe[MAX_DEG], rIm[MAX_DEG];
    for (int i = 0; i < degree; i++) {
        rRe[i] = warmRe[i];
        rIm[i] = warmIm[i];
    }

    for (int iter = 0; iter < SOLVER_MAX_ITER; iter++) {
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

        if (maxCorr2 < SOLVER_TOL2) break;
    }

    /* Write back (NaN check) */
    for (int i = 0; i < degree; i++) {
        if (rRe[i] == rRe[i] && rIm[i] == rIm[i]) {
            warmRe[i] = rRe[i];
            warmIm[i] = rIm[i];
        }
    }
}

/* ================================================================
 * Root matching — greedy O(n²)
 * ================================================================ */
static void matchRootsGreedy(double *newRe, double *newIm,
                             double *oldRe, double *oldIm, int n)
{
    unsigned char used[MAX_DEG];
    double tRe[MAX_DEG], tIm[MAX_DEG];
    for (int i = 0; i < n; i++) used[i] = 0;

    for (int i = 0; i < n; i++) {
        int bestJ = 0;
        double bestD = 1e300;
        for (int j = 0; j < n; j++) {
            if (used[j]) continue;
            double dx = newRe[j] - oldRe[i], dy = newIm[j] - oldIm[i];
            double d2 = dx * dx + dy * dy;
            if (d2 < bestD) { bestD = d2; bestJ = j; }
        }
        tRe[i] = newRe[bestJ]; tIm[i] = newIm[bestJ];
        used[bestJ] = 1;
    }
    for (int i = 0; i < n; i++) { newRe[i] = tRe[i]; newIm[i] = tIm[i]; }
}

/* ================================================================
 * Root matching — Hungarian (Kuhn-Munkres) O(n³)
 * ================================================================ */
static void hungarianMatch(double *newRe, double *newIm,
                           double *oldRe, double *oldIm, int n)
{
    /* Fall back to greedy if n exceeds Hungarian cap (stack safety) */
    if (n > HUNGARIAN_MAX) {
        matchRootsGreedy(newRe, newIm, oldRe, oldIm, n);
        return;
    }

    /* Cost matrix — stack allocated, max 32×32×8 = 8KB */
    double cost[HUNGARIAN_MAX * HUNGARIAN_MAX];
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            double dr = newRe[j] - oldRe[i], di = newIm[j] - oldIm[i];
            cost[i * n + j] = dr * dr + di * di;
        }

    double INF = 1e18;
    double u[HUNGARIAN_MAX + 2], v[HUNGARIAN_MAX + 2];
    int p[HUNGARIAN_MAX + 2], way[HUNGARIAN_MAX + 2];
    for (int i = 0; i <= n; i++) { u[i] = 0; v[i] = 0; p[i] = 0; }

    for (int i = 1; i <= n; i++) {
        p[0] = i;
        int j0 = 0;
        double minv[HUNGARIAN_MAX + 2];
        unsigned char used[HUNGARIAN_MAX + 2];
        for (int jj = 0; jj <= n; jj++) { minv[jj] = INF; used[jj] = 0; }

        do {
            used[j0] = 1;
            int i0 = p[j0], j1 = -1;
            double delta = INF;
            for (int j = 1; j <= n; j++) {
                if (used[j]) continue;
                double cur = cost[(i0 - 1) * n + (j - 1)] - u[i0] - v[j];
                if (cur < minv[j]) { minv[j] = cur; way[j] = j0; }
                if (minv[j] < delta) { delta = minv[j]; j1 = j; }
            }
            for (int j = 0; j <= n; j++) {
                if (used[j]) { u[p[j]] += delta; v[j] -= delta; }
                else { minv[j] -= delta; }
            }
            j0 = j1;
        } while (p[j0] != 0);

        do { int jj = way[j0]; p[j0] = p[jj]; j0 = jj; } while (j0);
    }

    double tRe[HUNGARIAN_MAX], tIm[HUNGARIAN_MAX];
    for (int j = 1; j <= n; j++) {
        tRe[p[j] - 1] = newRe[j - 1];
        tIm[p[j] - 1] = newIm[j - 1];
    }
    for (int i = 0; i < n; i++) { newRe[i] = tRe[i]; newIm[i] = tIm[i]; }
}

/* ================================================================
 * Derivative sensitivity + rankNorm
 * ================================================================ */

/* Insertion sort for rankNorm (n <= 30) */
static void rankNorm(double *raw, double *result, int n)
{
    double maxFinite = -1e300;
    for (int i = 0; i < n; i++) {
        if (raw[i] == raw[i] && raw[i] < 1e300 && raw[i] > maxFinite)
            maxFinite = raw[i];
    }
    if (maxFinite <= -1e300) { /* all NaN/inf — fill 0.5 */
        for (int i = 0; i < n; i++) result[i] = 0.5;
        return;
    }

    /* Build (value, index) pairs */
    double vals[MAX_DEG];
    int idxs[MAX_DEG];
    for (int i = 0; i < n; i++) {
        vals[i] = (raw[i] == raw[i] && raw[i] < 1e300) ? raw[i] : maxFinite;
        idxs[i] = i;
    }

    /* Insertion sort */
    for (int i = 1; i < n; i++) {
        double v = vals[i];
        int ix = idxs[i];
        int j = i - 1;
        while (j >= 0 && vals[j] > v) {
            vals[j + 1] = vals[j];
            idxs[j + 1] = idxs[j];
            j--;
        }
        vals[j + 1] = v;
        idxs[j + 1] = ix;
    }

    /* Assign ranks */
    int rank = 0;
    for (int p = 0; p < n; p++) {
        if (p > 0 && vals[p] != vals[p - 1]) rank = p;
        result[idxs[p]] = (double)rank;
    }
    int maxRank = n - 1;
    if (maxRank == 0) {
        for (int i = 0; i < n; i++) result[i] = 0.5;
        return;
    }
    for (int i = 0; i < n; i++) result[i] /= (double)maxRank;
}

static void computeSens(double *cRe, double *cIm, int nc,
                        double *rRe, double *rIm, int nr,
                        int *selIdx, int nSel,
                        double *sens)
{
    int deg = nc - 1;
    for (int j = 0; j < nr; j++) {
        double zRe = rRe[j], zIm = rIm[j];
        double pRe = cRe[0], pIm = cIm[0];
        double dpRe = 0, dpIm = 0;
        for (int k = 1; k <= deg; k++) {
            double ndR = dpRe * zRe - dpIm * zIm + pRe;
            double ndI = dpRe * zIm + dpIm * zRe + pIm;
            dpRe = ndR; dpIm = ndI;
            double npR = pRe * zRe - pIm * zIm + cRe[k];
            double npI = pRe * zIm + pIm * zRe + cIm[k];
            pRe = npR; pIm = npI;
        }
        double dpMag2 = dpRe * dpRe + dpIm * dpIm;
        if (dpMag2 < 1e-60) { sens[j] = 1e300; continue; }
        double dpMag = __builtin_sqrt(dpMag2);
        double rMag = __builtin_sqrt(zRe * zRe + zIm * zIm);
        /* Power ladder: pows[k] = rMag^k, O(deg) once instead of O(deg*nSel) */
        double pows[MAX_COEFFS];
        pows[0] = 1.0;
        for (int k = 1; k <= deg; k++) pows[k] = pows[k - 1] * rMag;
        double sum = 0;
        for (int s = 0; s < nSel; s++) {
            sum += pows[deg - selIdx[s]];
        }
        sens[j] = sum / dpMag;
    }
}

/* ================================================================
 * Exported: init
 * ================================================================ */
__attribute__((export_name("init")))
void init(int cfgIntOffset, int cfgDblOffset)
{
    cfgI = (int *)((unsigned long)cfgIntOffset);
    cfgD = (double *)((unsigned long)cfgDblOffset);

    nCoeffs       = cfgI[CI_NCOEFFS];
    nRoots        = cfgI[CI_NROOTS];
    canvasW       = cfgI[CI_CANVAS_W];
    canvasH       = cfgI[CI_CANVAS_H];
    totalSteps    = cfgI[CI_TOTAL_STEPS];
    colorMode     = cfgI[CI_COLOR_MODE];
    matchStrategy = cfgI[CI_MATCH_STRATEGY];
    morphEnabled  = cfgI[CI_MORPH_ENABLED];
    nEntries      = cfgI[CI_N_ENTRIES];
    nDEntries     = cfgI[CI_N_DENTRIES];
    nFollowC      = cfgI[CI_N_FOLLOWC];
    nSelIndices    = cfgI[CI_N_SEL_INDICES];
    hasJiggle     = cfgI[CI_HAS_JIGGLE];

    bitmapRange      = cfgD[CD_RANGE];
    FPS              = cfgD[CD_FPS];
    morphRate        = cfgD[CD_MORPH_RATE];
    morphEllipseMinor = cfgD[CD_MORPH_ELLIPSE_MINOR];
    morphDitherStart = cfgD[CD_MORPH_DITHER_START];
    morphDitherMid = cfgD[CD_MORPH_DITHER_MID];
    morphDitherEnd = cfgD[CD_MORPH_DITHER_END];
    centerX          = cfgD[CD_CENTER_X];
    centerY          = cfgD[CD_CENTER_Y];
    morphPathType    = cfgI[CI_MORPH_PATH_TYPE];
    morphCcw         = cfgI[CI_MORPH_CCW];

    /* Seed PRNG + reset Gaussian spare */
    gaussHasSpare = 0;
    rngS[0] = (unsigned int)cfgI[CI_RNG_SEED0];
    rngS[1] = (unsigned int)cfgI[CI_RNG_SEED1];
    rngS[2] = (unsigned int)cfgI[CI_RNG_SEED2];
    rngS[3] = (unsigned int)cfgI[CI_RNG_SEED3];
    if (rngS[0] == 0 && rngS[1] == 0 && rngS[2] == 0 && rngS[3] == 0) {
        rngS[0] = 0xDEADBEEF; rngS[1] = 0x12345678;
        rngS[2] = 0xABCDEF01; rngS[3] = 0x87654321;
    }

    /* Set data pointers from config offsets */
    #define PTR(type, idx) ((type *)((unsigned long)cfgI[idx]))
    coeffsRe      = PTR(double, CI_OFF_COEFFS_RE);
    coeffsIm      = PTR(double, CI_OFF_COEFFS_IM);
    colorsR       = PTR(unsigned char, CI_OFF_COLORS_R);
    colorsG       = PTR(unsigned char, CI_OFF_COLORS_G);
    colorsB       = PTR(unsigned char, CI_OFF_COLORS_B);
    jiggleRe      = PTR(double, CI_OFF_JIGGLE_RE);
    jiggleIm      = PTR(double, CI_OFF_JIGGLE_IM);
    morphTargetRe = PTR(double, CI_OFF_MORPH_TGT_RE);
    morphTargetIm = PTR(double, CI_OFF_MORPH_TGT_IM);
    proxPalR      = PTR(unsigned char, CI_OFF_PROX_PAL_R);
    proxPalG      = PTR(unsigned char, CI_OFF_PROX_PAL_G);
    proxPalB      = PTR(unsigned char, CI_OFF_PROX_PAL_B);
    derivPalR     = PTR(unsigned char, CI_OFF_DERIV_PAL_R);
    derivPalG     = PTR(unsigned char, CI_OFF_DERIV_PAL_G);
    derivPalB     = PTR(unsigned char, CI_OFF_DERIV_PAL_B);
    selIndices    = PTR(int, CI_OFF_SEL_INDICES);
    followCIdx    = PTR(int, CI_OFF_FOLLOWC_IDX);

    entryIdx      = PTR(int, CI_OFF_ENTRY_IDX);
    entrySpeed    = PTR(double, CI_OFF_ENTRY_SPEED);
    entryCcw      = PTR(int, CI_OFF_ENTRY_CCW);
    entryDither   = PTR(double, CI_OFF_ENTRY_DITHER);
    entryDitherDist = PTR(int, CI_OFF_ENTRY_DITHER_DIST);
    curveOffsets  = PTR(int, CI_OFF_CURVE_OFFSETS);
    curveLengths  = PTR(int, CI_OFF_CURVE_LENGTHS);
    curveIsCloud  = PTR(int, CI_OFF_CURVE_ISCLOUD);
    curvesFlat    = PTR(double, CI_OFF_CURVES_FLAT);

    dEntryIdx     = PTR(int, CI_OFF_DENTRY_IDX);
    dEntrySpeed   = PTR(double, CI_OFF_DENTRY_SPEED);
    dEntryCcw     = PTR(int, CI_OFF_DENTRY_CCW);
    dEntryDither  = PTR(double, CI_OFF_DENTRY_DITHER);
    dEntryDitherDist = PTR(int, CI_OFF_DENTRY_DITHER_DIST);
    dCurveOffsets = PTR(int, CI_OFF_DCURVE_OFFSETS);
    dCurveLengths = PTR(int, CI_OFF_DCURVE_LENGTHS);
    dCurveIsCloud = PTR(int, CI_OFF_DCURVE_ISCLOUD);
    dCurvesFlat   = PTR(double, CI_OFF_DCURVES_FLAT);

    workCoeffsRe  = PTR(double, CI_OFF_WORK_COEFFS_RE);
    workCoeffsIm  = PTR(double, CI_OFF_WORK_COEFFS_IM);
    tmpRe         = PTR(double, CI_OFF_TMP_RE);
    tmpIm         = PTR(double, CI_OFF_TMP_IM);
    morphWorkRe   = PTR(double, CI_OFF_MORPH_WORK_RE);
    morphWorkIm   = PTR(double, CI_OFF_MORPH_WORK_IM);
    passRootsRe   = PTR(double, CI_OFF_PASS_ROOTS_RE);
    passRootsIm   = PTR(double, CI_OFF_PASS_ROOTS_IM);

    paintIdx      = PTR(int, CI_OFF_PAINT_IDX);
    paintR        = PTR(unsigned char, CI_OFF_PAINT_R);
    paintG        = PTR(unsigned char, CI_OFF_PAINT_G);
    paintB        = PTR(unsigned char, CI_OFF_PAINT_B);
    #undef PTR
}

/* ================================================================
 * Exported: runStepLoop
 * Returns: number of pixels written to output buffers
 * ================================================================ */
__attribute__((export_name("runStepLoop")))
int runStepLoop(int stepStart, int stepEnd, double elapsedOffset)
{
    int pc = 0;  /* paint count */
    int W = canvasW, H = canvasH;
    double range = bitmapRange;
    int nr = nRoots, nc = nCoeffs;
    double proxRunMax = 1.0;

    /* Copy pass roots to working roots */
    for (int i = 0; i < nr; i++) {
        tmpRe[i] = passRootsRe[i];
        tmpIm[i] = passRootsIm[i];
    }
    /* rootsRe/Im are the "old" roots for matching; tmpRe/Im are solver output */
    double *rootsRe = passRootsRe;
    double *rootsIm = passRootsIm;

    /* Morph angle recurrence: replace per-step JS trig with multiply */
    double morphCosT = 1.0, morphSinT = 0.0;
    double morphCosD = 1.0, morphSinD = 0.0;
    if (morphEnabled) {
        double dTheta = 2.0 * PI * morphRate * FPS / (double)totalSteps;
        double theta0 = 2.0 * PI * morphRate *
            (elapsedOffset + ((double)stepStart / (double)totalSteps) * FPS);
        morphCosT = js_cos(theta0); morphSinT = js_sin(theta0);
        morphCosD = js_cos(dTheta); morphSinD = js_sin(dTheta);
    }

    for (int step = stepStart; step < stepEnd; step++) {
        double elapsed = elapsedOffset + ((double)step / (double)totalSteps) * FPS;

        /* 1. Reset coefficients to base values (if jiggle) */
        if (hasJiggle) {
            for (int i = 0; i < nc; i++) {
                workCoeffsRe[i] = coeffsRe[i];
                workCoeffsIm[i] = coeffsIm[i];
            }
        }

        /* 2. Interpolate C-curves */
        for (int a = 0; a < nEntries; a++) {
            int idx = entryIdx[a];
            double dir = entryCcw[a] ? -1.0 : 1.0;
            double t = elapsed * entrySpeed[a] * dir;
            double u = frac01(t);
            int N = curveLengths[a];
            double rawIdx = u * N;
            int base = curveOffsets[a] * 2;

            if (curveIsCloud[a]) {
                int k = (int)rawIdx;
                if (k >= N) k = N - 1;  /* safety clamp */
                workCoeffsRe[idx] = curvesFlat[base + k * 2];
                workCoeffsIm[idx] = curvesFlat[base + k * 2 + 1];
            } else {
                int lo = (int)rawIdx;
                if (lo >= N) lo = N - 1;
                int hi = lo + 1; if (hi == N) hi = 0;
                double frac = rawIdx - (double)lo;
                workCoeffsRe[idx] = curvesFlat[base + lo * 2] * (1 - frac) + curvesFlat[base + hi * 2] * frac;
                workCoeffsIm[idx] = curvesFlat[base + lo * 2 + 1] * (1 - frac) + curvesFlat[base + hi * 2 + 1] * frac;
            }

            /* 3. Apply dither */
            if (entryDither[a] > 0) {
                workCoeffsRe[idx] += rngDither(entryDitherDist[a]) * entryDither[a];
                workCoeffsIm[idx] += rngDither(entryDitherDist[a]) * entryDither[a];
            }
        }

        /* 4. Interpolate D-curves (only when morph active) */
        if (morphEnabled && nDEntries > 0 && dCurvesFlat) {
            for (int da = 0; da < nDEntries; da++) {
                int dIdx = dEntryIdx[da];
                double dDir = dEntryCcw[da] ? -1.0 : 1.0;
                double dT = elapsed * dEntrySpeed[da] * dDir;
                double dU = frac01(dT);
                int dN = dCurveLengths[da];
                double dRawIdx = dU * dN;
                int dBase = dCurveOffsets[da] * 2;

                if (dCurveIsCloud[da]) {
                    int dK = (int)dRawIdx;
                    if (dK >= dN) dK = dN - 1;
                    morphWorkRe[dIdx] = dCurvesFlat[dBase + dK * 2];
                    morphWorkIm[dIdx] = dCurvesFlat[dBase + dK * 2 + 1];
                } else {
                    int dLo = (int)dRawIdx;
                    if (dLo >= dN) dLo = dN - 1;
                    int dHi = dLo + 1; if (dHi == dN) dHi = 0;
                    double dFrac = dRawIdx - (double)dLo;
                    morphWorkRe[dIdx] = dCurvesFlat[dBase + dLo * 2] * (1 - dFrac) + dCurvesFlat[dBase + dHi * 2] * dFrac;
                    morphWorkIm[dIdx] = dCurvesFlat[dBase + dLo * 2 + 1] * (1 - dFrac) + dCurvesFlat[dBase + dHi * 2 + 1] * dFrac;
                }

                if (dEntryDither[da] > 0) {
                    morphWorkRe[dIdx] += rngDither(dEntryDitherDist[da]) * dEntryDither[da];
                    morphWorkIm[dIdx] += rngDither(dEntryDitherDist[da]) * dEntryDither[da];
                }
            }
        }

        /* 5. Follow-C: D-nodes that mirror C-node position (only when morph active) */
        if (morphEnabled && nFollowC > 0) {
            for (int fc = 0; fc < nFollowC; fc++) {
                int fci = followCIdx[fc];
                morphWorkRe[fci] = workCoeffsRe[fci];
                morphWorkIm[fci] = workCoeffsIm[fci];
            }
        }

        /* 6. Morph blend (sin/cos via recurrence — no JS trig calls) */
        if (morphEnabled && !(morphCosT >= 1.0 - 1e-14 && morphSinT > -1e-14 && morphSinT < 1e-14)) {
            /* Skip blend when theta≈0 — avoids fp noise at home position */
            double cosT = morphCosT, sinT = morphSinT;
            if (morphPathType == 0) {
                /* Line: mu = (1 - cos(theta)) / 2 */
                double mu = 0.5 - 0.5 * cosT;
                double omu = 1.0 - mu;
                for (int m = 0; m < nc; m++) {
                    workCoeffsRe[m] = workCoeffsRe[m] * omu + morphWorkRe[m] * mu;
                    workCoeffsIm[m] = workCoeffsIm[m] * omu + morphWorkIm[m] * mu;
                }
            } else {
                /* Non-linear: circle (1), ellipse (2), figure-8 (3) */
                double sign = morphCcw ? 1.0 : -1.0;
                double sin2T = 2.0 * sinT * cosT; /* sin(2θ) for figure-8 */
                for (int m = 0; m < nc; m++) {
                    double cR = workCoeffsRe[m], cI = workCoeffsIm[m];
                    double dR = morphWorkRe[m], dI = morphWorkIm[m];
                    double dx = dR - cR, dy = dI - cI;
                    double len2 = dx * dx + dy * dy;
                    if (len2 < 1e-30) continue; /* C ≈ D, keep C */
                    double len = __builtin_sqrt(len2);
                    double ux = dx / len, uy = dy / len;
                    double vx = -uy, vy = ux;
                    double midR = (cR + dR) * 0.5, midI = (cI + dI) * 0.5;
                    double semi = len * 0.5;
                    double lx = -semi * cosT;
                    double ly;
                    if (morphPathType == 1) {          /* circle */
                        ly = sign * semi * sinT;
                    } else if (morphPathType == 2) {   /* ellipse */
                        ly = sign * (morphEllipseMinor * semi) * sinT;
                    } else {                           /* figure-8 */
                        ly = sign * (semi * 0.5) * sin2T;
                    }
                    workCoeffsRe[m] = midR + lx * ux + ly * vx;
                    workCoeffsIm[m] = midI + lx * uy + ly * vy;
                }
            }
            /* Morph path dither: start max(cos,0)² + mid sin² + end max(-cos,0)² */
            if (morphDitherStart > 0.0 || morphDitherMid > 0.0 || morphDitherEnd > 0.0) {
                double startEnv = cosT > 0.0 ? cosT * cosT : 0.0;
                double endEnv   = cosT < 0.0 ? cosT * cosT : 0.0;
                double ds = morphDitherStart * startEnv + morphDitherMid * sinT * sinT + morphDitherEnd * endEnv;
                if (ds > 0.0) for (int m = 0; m < nc; m++) {
                    workCoeffsRe[m] += (rngUniform() - 0.5) * 2.0 * ds;
                    workCoeffsIm[m] += (rngUniform() - 0.5) * 2.0 * ds;
                }
            }
            /* Advance morph angle recurrence */
            {
                double nc2 = morphCosT * morphCosD - morphSinT * morphSinD;
                double ns  = morphSinT * morphCosD + morphCosT * morphSinD;
                morphCosT = nc2; morphSinT = ns;
            }
            /* Renormalize every 1024 steps to prevent drift */
            if (((step - stepStart) & 1023) == 0) {
                double invLen = 1.0 / __builtin_sqrt(
                    morphCosT * morphCosT + morphSinT * morphSinT);
                morphCosT *= invLen; morphSinT *= invLen;
            }
        }

        /* 7. Apply jiggle offsets */
        if (hasJiggle) {
            for (int j = 0; j < nc; j++) {
                workCoeffsRe[j] += jiggleRe[j];
                workCoeffsIm[j] += jiggleIm[j];
            }
        }

        /* 8. Solve */
        for (int i = 0; i < nr; i++) {
            tmpRe[i] = rootsRe[i];
            tmpIm[i] = rootsIm[i];
        }
        solveEA(workCoeffsRe, workCoeffsIm, nc, tmpRe, tmpIm, nr);

        /* NaN rescue */
        for (int i = 0; i < nr; i++) {
            if (tmpRe[i] != tmpRe[i] || tmpIm[i] != tmpIm[i]) {
                double angle = (2.0 * PI * i) / nr + 0.37;
                tmpRe[i] = js_cos(angle);
                tmpIm[i] = js_sin(angle);
            }
        }

        /* 9. Post-solve: color-mode dependent processing + pixel output */
        if (colorMode == 3) {
            /* Derivative mode */
            if ((step - stepStart) % 4 == 0) {
                matchRootsGreedy(tmpRe, tmpIm, rootsRe, rootsIm, nr);
            }
            double rawSens[MAX_DEG], normSens[MAX_DEG];
            computeSens(workCoeffsRe, workCoeffsIm, nc, tmpRe, tmpIm, nr,
                        selIndices, nSelIndices, rawSens);
            rankNorm(rawSens, normSens, nr);
            for (int i = 0; i < nr; i++) {
                rootsRe[i] = tmpRe[i]; rootsIm[i] = tmpIm[i];
            }
            for (int i = 0; i < nr; i++) {
                int ix = (int)(((rootsRe[i] - centerX) / range + 1.0) * 0.5 * W);
                int iy = (int)((1.0 - (rootsIm[i] - centerY) / range) * 0.5 * H);
                if (ix < 0 || ix >= W || iy < 0 || iy >= H) continue;
                int palIdx = (int)(normSens[i] * 15.0 + 0.5);
                if (palIdx > 15) palIdx = 15;
                paintIdx[pc] = iy * W + ix;
                paintR[pc] = derivPalR[palIdx];
                paintG[pc] = derivPalG[palIdx];
                paintB[pc] = derivPalB[palIdx];
                pc++;
            }
        } else if (colorMode == 2) {
            /* Proximity mode — symmetric O(n²/2) */
            double minDists[MAX_DEG];
            for (int i = 0; i < nr; i++) minDists[i] = 1e300;
            for (int i = 0; i < nr; i++) {
                for (int j = i + 1; j < nr; j++) {
                    double dx = tmpRe[i] - tmpRe[j], dy = tmpIm[i] - tmpIm[j];
                    double d2 = dx * dx + dy * dy;
                    if (d2 < minDists[i]) minDists[i] = d2;
                    if (d2 < minDists[j]) minDists[j] = d2;
                }
            }
            for (int i = 0; i < nr; i++) minDists[i] = __builtin_sqrt(minDists[i]);
            for (int i = 0; i < nr; i++) {
                if (minDists[i] > proxRunMax) proxRunMax = minDists[i];
            }
            proxRunMax *= 0.999;
            for (int i = 0; i < nr; i++) {
                rootsRe[i] = tmpRe[i]; rootsIm[i] = tmpIm[i];
            }
            for (int i = 0; i < nr; i++) {
                int ix = (int)(((rootsRe[i] - centerX) / range + 1.0) * 0.5 * W);
                int iy = (int)((1.0 - (rootsIm[i] - centerY) / range) * 0.5 * H);
                if (ix < 0 || ix >= W || iy < 0 || iy >= H) continue;
                double t = 1.0;
                if (proxRunMax > 0) {
                    t = minDists[i] / proxRunMax;
                    if (t > 1.0) t = 1.0;
                    t = 1.0 - t;
                }
                int palIdx = (int)(t * 15.0);
                if (palIdx > 15) palIdx = 15;
                paintIdx[pc] = iy * W + ix;
                paintR[pc] = proxPalR[palIdx];
                paintG[pc] = proxPalG[palIdx];
                paintB[pc] = proxPalB[palIdx];
                pc++;
            }
        } else if (colorMode == 0) {
            /* Uniform mode */
            for (int i = 0; i < nr; i++) {
                rootsRe[i] = tmpRe[i]; rootsIm[i] = tmpIm[i];
            }
            int ur = cfgI[CI_UNIFORM_R], ug = cfgI[CI_UNIFORM_G], ub = cfgI[CI_UNIFORM_B];
            for (int i = 0; i < nr; i++) {
                int ix = (int)(((rootsRe[i] - centerX) / range + 1.0) * 0.5 * W);
                int iy = (int)((1.0 - (rootsIm[i] - centerY) / range) * 0.5 * H);
                if (ix < 0 || ix >= W || iy < 0 || iy >= H) continue;
                paintIdx[pc] = iy * W + ix;
                paintR[pc] = (unsigned char)ur;
                paintG[pc] = (unsigned char)ug;
                paintB[pc] = (unsigned char)ub;
                pc++;
            }
        } else {
            /* Index-rainbow mode (colorMode == 1) */
            if (matchStrategy == 2) {
                hungarianMatch(tmpRe, tmpIm, rootsRe, rootsIm, nr);
            } else if (matchStrategy == 1) {
                matchRootsGreedy(tmpRe, tmpIm, rootsRe, rootsIm, nr);
            } else {
                /* assign4: every 4th step */
                if ((step - stepStart) % 4 == 0) {
                    matchRootsGreedy(tmpRe, tmpIm, rootsRe, rootsIm, nr);
                }
            }
            for (int i = 0; i < nr; i++) {
                rootsRe[i] = tmpRe[i]; rootsIm[i] = tmpIm[i];
            }
            for (int i = 0; i < nr; i++) {
                int ix = (int)(((rootsRe[i] - centerX) / range + 1.0) * 0.5 * W);
                int iy = (int)((1.0 - (rootsIm[i] - centerY) / range) * 0.5 * H);
                if (ix < 0 || ix >= W || iy < 0 || iy >= H) continue;
                paintIdx[pc] = iy * W + ix;
                paintR[pc] = colorsR[i];
                paintG[pc] = colorsG[i];
                paintB[pc] = colorsB[i];
                pc++;
            }
        }

        /* 10. Progress report */
        if ((step - stepStart) % PROGRESS_INTERVAL == 0) {
            js_reportProgress(step - stepStart);
        }
    }

    /* Write final roots back to pass roots for JS to read */
    /* (rootsRe/Im already points to passRootsRe/Im, so they're already there) */

    return pc;
}
