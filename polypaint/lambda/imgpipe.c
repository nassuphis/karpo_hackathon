/*
 * imgpipe: libvips-based image pipeline for polynomial root rendering.
 *
 * Intermediate format: .raw files with 12-byte header (uint32 W, H, bands)
 * followed by raw uint8 pixel data. Avoids PNG encode/decode overhead
 * for intermediate stages; only --encode produces final JPEG/PNG.
 *
 * Three modes:
 *   --roots2image stripe.bin out.raw --width=W --height=H
 *                 --center_re=X --center_im=Y --scale=S --degree=D
 *                 [--color=rainbow|proximity] [--match=none|greedy|hungarian]
 *                 [--palette=inferno|viridis|magma|plasma|turbo|cividis|warm|cool]
 *     Reads f32 root positions from .bin, renders to raw image.
 *
 *   --reduce acc.raw next.raw out.raw [--gamma=2.2]
 *     Gamma-correct additive merge of two images (gamma=0 for raw saturating add).
 *
 *   --encode input.raw out.jpeg --quality=Q
 *     Convert raw image to JPEG or PNG with specified quality.
 *
 * Build (must link against libvips from Lambda layer):
 *   gcc -O3 -o imgpipe imgpipe.c -I/opt/include \
 *     -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
 *     -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm \
 *     -Wl,-rpath,/opt/lib
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vips/vips.h>

#define MAXDEG 256

/* ---- RGB type and palette definitions (16-step) ---- */

typedef struct { unsigned char r, g, b; } RGB;

/* Inferno: black → purple → orange → yellow */
static const RGB PAL_INFERNO[16] = {
    {0,0,4}, {16,11,53}, {43,15,95}, {72,12,119},
    {101,14,118}, {126,34,102}, {148,56,81}, {168,81,60},
    {186,108,41}, {203,137,25}, {217,169,13}, {228,201,27},
    {235,232,68}, {247,249,115}, {252,254,164}, {252,255,164}
};

/* Viridis: purple → teal → green → yellow */
static const RGB PAL_VIRIDIS[16] = {
    {68,1,84}, {72,20,103}, {71,40,120}, {63,57,131},
    {55,72,137}, {46,87,140}, {38,102,141}, {31,116,140},
    {26,131,137}, {27,146,130}, {40,161,119}, {65,175,102},
    {102,187,79}, {149,198,50}, {201,206,22}, {253,231,37}
};

/* Magma: black → purple → pink → cream */
static const RGB PAL_MAGMA[16] = {
    {0,0,4}, {13,7,49}, {38,11,93}, {65,8,123},
    {93,14,126}, {119,31,114}, {142,52,98}, {163,75,82},
    {184,101,68}, {204,130,56}, {221,162,47}, {234,196,53},
    {242,228,82}, {249,249,121}, {253,254,168}, {252,253,191}
};

/* Plasma: purple → magenta → orange → yellow */
static const RGB PAL_PLASMA[16] = {
    {13,8,135}, {47,5,146}, {79,2,150}, {107,2,145},
    {132,9,133}, {153,21,117}, {171,38,98}, {187,58,79},
    {201,81,59}, {213,107,39}, {223,135,22}, {231,165,11},
    {237,196,8}, {240,225,15}, {243,249,40}, {240,249,33}
};

/* Turbo: dark blue → cyan → green → yellow → red → dark red */
static const RGB PAL_TURBO[16] = {
    {48,18,59}, {57,68,148}, {43,118,196}, {28,163,206},
    {20,200,178}, {44,222,128}, {96,237,79}, {156,240,43},
    {208,230,30}, {242,204,20}, {255,170,14}, {252,130,15},
    {236,89,16}, {210,49,14}, {175,18,8}, {122,4,3}
};

/* Cividis: blue → yellow (colorblind-friendly) */
static const RGB PAL_CIVIDIS[16] = {
    {0,32,76}, {0,46,96}, {23,60,108}, {48,73,113},
    {72,85,116}, {93,97,119}, {113,110,121}, {132,122,119},
    {151,134,115}, {170,147,108}, {189,160,98}, {208,174,84},
    {226,189,65}, {242,205,43}, {254,222,19}, {253,238,6}
};

/* Warm: magenta → red → orange → yellow */
static const RGB PAL_WARM[16] = {
    {110,64,170}, {138,60,162}, {163,62,143}, {182,72,121},
    {196,87,97}, {208,107,75}, {216,130,56}, {222,155,42},
    {225,180,36}, {225,205,41}, {220,226,56}, {208,243,81},
    {190,252,108}, {168,254,139}, {145,253,168}, {122,250,196}
};

/* Cool: green → blue → purple */
static const RGB PAL_COOL[16] = {
    {110,64,170}, {100,82,192}, {88,101,207}, {75,119,215},
    {62,137,217}, {52,154,213}, {44,170,203}, {40,185,188},
    {42,199,168}, {53,211,145}, {71,222,119}, {96,230,91},
    {126,236,65}, {160,240,44}, {194,241,32}, {228,238,29}
};

/* Named palette table */
typedef struct { const char *name; const RGB *colors; } PalEntry;
static const PalEntry PALETTES[] = {
    {"inferno", PAL_INFERNO},
    {"viridis", PAL_VIRIDIS},
    {"magma",   PAL_MAGMA},
    {"plasma",  PAL_PLASMA},
    {"turbo",   PAL_TURBO},
    {"cividis", PAL_CIVIDIS},
    {"warm",    PAL_WARM},
    {"cool",    PAL_COOL},
    {NULL, NULL}
};

#define N_PALETTES 8

static const RGB *findPalette(const char *name) {
    if (!name) return PAL_INFERNO;
    for (int i = 0; PALETTES[i].name; i++)
        if (strcmp(PALETTES[i].name, name) == 0)
            return PALETTES[i].colors;
    return PAL_INFERNO;
}

/* Interpolate 16-step palette at t in [0,1] */
static void paletteRGB(const RGB *pal, double t,
                       unsigned char *r, unsigned char *g, unsigned char *b) {
    if (t <= 0) { *r = pal[0].r; *g = pal[0].g; *b = pal[0].b; return; }
    if (t >= 1) { *r = pal[15].r; *g = pal[15].g; *b = pal[15].b; return; }
    double idx = t * 15.0;
    int lo = (int)idx;
    double f = idx - lo;
    int hi = lo + 1;
    *r = (unsigned char)(pal[lo].r * (1-f) + pal[hi].r * f + 0.5);
    *g = (unsigned char)(pal[lo].g * (1-f) + pal[hi].g * f + 0.5);
    *b = (unsigned char)(pal[lo].b * (1-f) + pal[hi].b * f + 0.5);
}

/* ---- Rainbow palette (HSL, same as sweep_cli.c) ---- */

static void rainbowRGB(int index, int total,
                       unsigned char *r, unsigned char *g, unsigned char *b) {
    double hue = (double)index / (total > 0 ? total : 1);
    double h6 = hue * 6.0;
    int hi = (int)h6;
    double f = h6 - hi;
    double q = 1.0 - f;
    switch (hi % 6) {
        case 0: *r = 255; *g = (unsigned char)(f * 255); *b = 0; break;
        case 1: *r = (unsigned char)(q * 255); *g = 255; *b = 0; break;
        case 2: *r = 0; *g = 255; *b = (unsigned char)(f * 255); break;
        case 3: *r = 0; *g = (unsigned char)(q * 255); *b = 255; break;
        case 4: *r = (unsigned char)(f * 255); *g = 0; *b = 255; break;
        case 5: *r = 255; *g = 0; *b = (unsigned char)(q * 255); break;
    }
}

/* ---- Parse --key=value from argv ---- */

static const char *getArg(int argc, char **argv, const char *key) {
    int klen = strlen(key);
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], key, klen) == 0 && argv[i][klen] == '=')
            return argv[i] + klen + 1;
    }
    return NULL;
}

static int getArgInt(int argc, char **argv, const char *key, int def) {
    const char *v = getArg(argc, argv, key);
    return v ? atoi(v) : def;
}

static double getArgDouble(int argc, char **argv, const char *key, double def) {
    const char *v = getArg(argc, argv, key);
    return v ? atof(v) : def;
}

static const char *getArgStr(int argc, char **argv, const char *key, const char *def) {
    const char *v = getArg(argc, argv, key);
    return v ? v : def;
}

/* ---- Raw image I/O (12-byte header: uint32 W, H, bands + pixel data) ---- */

static int raw_write(const char *path, const unsigned char *data,
                     unsigned int w, unsigned int h, unsigned int bands) {
    FILE *f = fopen(path, "wb");
    if (!f) { fprintf(stderr, "Cannot create %s\n", path); return -1; }
    fwrite(&w, 4, 1, f);
    fwrite(&h, 4, 1, f);
    fwrite(&bands, 4, 1, f);
    fwrite(data, 1, (size_t)w * h * bands, f);
    fclose(f);
    return 0;
}

static unsigned char *raw_read(const char *path,
                               unsigned int *w, unsigned int *h, unsigned int *bands) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", path); return NULL; }
    if (fread(w, 4, 1, f) != 1 || fread(h, 4, 1, f) != 1 || fread(bands, 4, 1, f) != 1) {
        fprintf(stderr, "Bad raw header in %s\n", path);
        fclose(f); return NULL;
    }
    size_t n = (size_t)*w * *h * *bands;
    unsigned char *data = malloc(n);
    if (!data) { fprintf(stderr, "Cannot allocate %zu bytes\n", n); fclose(f); return NULL; }
    if (fread(data, 1, n, f) != n) {
        fprintf(stderr, "Short read in %s\n", path);
        free(data); fclose(f); return NULL;
    }
    fclose(f);
    return data;
}

/* ---- Root matching: greedy ---- */

static void greedyMatch(const float *prevStep, const float *currStep,
                        int degree, int *perm) {
    /* perm[i] = which current root index is assigned to old root i.
     * Cost = squared Euclidean distance. Greedy: for each old root,
     * pick the closest unassigned new root. */
    unsigned char used[MAXDEG];
    memset(used, 0, degree);
    for (int i = 0; i < degree; i++) {
        double oldRe = prevStep[i * 2], oldIm = prevStep[i * 2 + 1];
        double bestD = 1e30;
        int bestJ = 0;
        for (int j = 0; j < degree; j++) {
            if (used[j]) continue;
            double dr = currStep[j * 2] - oldRe;
            double di = currStep[j * 2 + 1] - oldIm;
            double d = dr * dr + di * di;
            if (d < bestD) { bestD = d; bestJ = j; }
        }
        perm[i] = bestJ;
        used[bestJ] = 1;
    }
}

/* ---- Root matching: Hungarian (Kuhn-Munkres O(n³)) ---- */

static void hungarianMatch(const float *prevStep, const float *currStep,
                           int degree, int *perm) {
    /* Build squared-distance cost matrix */
    double cost[MAXDEG * MAXDEG];
    for (int i = 0; i < degree; i++)
        for (int j = 0; j < degree; j++) {
            double dr = currStep[j * 2] - prevStep[i * 2];
            double di = currStep[j * 2 + 1] - prevStep[i * 2 + 1];
            cost[i * degree + j] = dr * dr + di * di;
        }

    /* Kuhn-Munkres with 1-indexed potentials */
    int n = degree;
    double u[MAXDEG + 1], v[MAXDEG + 1];
    int p[MAXDEG + 1], way[MAXDEG + 1];
    memset(u, 0, sizeof(u));
    memset(v, 0, sizeof(v));
    memset(p, 0, sizeof(p));

    for (int i = 1; i <= n; i++) {
        p[0] = i;
        int j0 = 0;
        double minv[MAXDEG + 1];
        unsigned char used[MAXDEG + 1];
        for (int j = 0; j <= n; j++) { minv[j] = 1e30; used[j] = 0; }

        do {
            used[j0] = 1;
            int i0 = p[j0], j1 = -1;
            double delta = 1e30;
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

    /* p[j] = row i assigned to column j (1-indexed).
     * perm[old_i] = new_j: old root i matched to new root j. */
    for (int j = 1; j <= n; j++)
        perm[p[j] - 1] = j - 1;
}

/* ---- Gamma LUTs for reduce ---- */

static float srgb2lin[256];
static unsigned char lin2srgb[4096];

static void buildGammaLUT(double gamma) {
    for (int i = 0; i < 256; i++)
        srgb2lin[i] = (float)pow(i / 255.0, gamma);
    double inv_gamma = 1.0 / gamma;
    for (int i = 0; i < 4096; i++) {
        double v = pow(i / 4095.0, inv_gamma) * 255.0;
        lin2srgb[i] = v > 255.0 ? 255 : (unsigned char)(v + 0.5);
    }
}

/* ---- roots2image mode ---- */

enum ColorMode { COLOR_RAINBOW = 0, COLOR_PROXIMITY = 1 };
enum MatchMode { MATCH_NONE = 0, MATCH_GREEDY = 1, MATCH_HUNGARIAN = 2 };

static int do_roots2image(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: imgpipe --roots2image stripe.bin out.png "
                "--width=W --height=H --center_re=X --center_im=Y --scale=S "
                "--degree=D [--color=rainbow|proximity] "
                "[--match=none|greedy|hungarian] [--palette=inferno|...]\n");
        return 1;
    }
    const char *binPath = argv[2];
    const char *outPath = argv[3];
    int W = getArgInt(argc, argv, "--width", 4096);
    int H = getArgInt(argc, argv, "--height", 4096);
    double centerRe = getArgDouble(argc, argv, "--center_re", 0.0);
    double centerIm = getArgDouble(argc, argv, "--center_im", 0.0);
    double scale = getArgDouble(argc, argv, "--scale", 100.0);
    int degree = getArgInt(argc, argv, "--degree", 25);
    const char *colorStr = getArgStr(argc, argv, "--color", "rainbow");
    const char *matchStr = getArgStr(argc, argv, "--match", "none");
    const char *palName = getArgStr(argc, argv, "--palette", "inferno");

    enum ColorMode colorMode = COLOR_RAINBOW;
    if (strcmp(colorStr, "proximity") == 0) colorMode = COLOR_PROXIMITY;

    enum MatchMode matchMode = MATCH_NONE;
    if (strcmp(matchStr, "greedy") == 0) matchMode = MATCH_GREEDY;
    else if (strcmp(matchStr, "hungarian") == 0) matchMode = MATCH_HUNGARIAN;

    const RGB *proxPal = findPalette(palName);

    if (W < 1 || W > 16384 || H < 1 || H > 16384) {
        fprintf(stderr, "Invalid dimensions: %dx%d\n", W, H);
        return 1;
    }
    if (degree < 1 || degree > MAXDEG) {
        fprintf(stderr, "Invalid degree: %d\n", degree);
        return 1;
    }

    /* Read binary root data */
    FILE *fin = fopen(binPath, "rb");
    if (!fin) { fprintf(stderr, "Cannot open %s\n", binPath); return 1; }
    fseek(fin, 0, SEEK_END);
    long fileSize = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    int stride = degree * 2;  /* f32 per step: re,im for each root */
    long nPoints = fileSize / (stride * sizeof(float));
    if (nPoints <= 0) { fprintf(stderr, "Empty root file\n"); fclose(fin); return 1; }

    float *roots = malloc(fileSize);
    if (!roots) { fprintf(stderr, "Cannot allocate %ld bytes\n", fileSize); fclose(fin); return 1; }
    fread(roots, 1, fileSize, fin);
    fclose(fin);

    /* Build rainbow palette (used for rainbow mode) */
    unsigned char rbPalR[MAXDEG], rbPalG[MAXDEG], rbPalB[MAXDEG];
    for (int i = 0; i < degree; i++)
        rainbowRGB(i, degree, &rbPalR[i], &rbPalG[i], &rbPalB[i]);

    /* Allocate pixel buffer */
    long pixelBytes = (long)W * H * 3;
    unsigned char *pixels = calloc(pixelBytes, 1);
    if (!pixels) {
        fprintf(stderr, "Cannot allocate %ldMB\n", pixelBytes / (1024 * 1024));
        free(roots);
        return 1;
    }

    long rootsPlotted = 0, rootsClipped = 0;
    double halfW = W / 2.0, halfH = H / 2.0;

    if (colorMode == COLOR_PROXIMITY) {
        /* --- Proximity coloring: two-pass --- */

        /* Pass 1: compute global min/max of min-pairwise-distances */
        double globalMin = 1e30, globalMax = 0.0;
        for (long p = 0; p < nPoints; p++) {
            float *step = roots + p * stride;
            for (int i = 0; i < degree; i++) {
                double re_i = step[i * 2], im_i = step[i * 2 + 1];
                double d2min = 1e30;
                for (int j = 0; j < degree; j++) {
                    if (j == i) continue;
                    double dr = re_i - step[j * 2];
                    double di = im_i - step[j * 2 + 1];
                    double d2 = dr * dr + di * di;
                    if (d2 < d2min) d2min = d2;
                }
                double d = sqrt(d2min);
                if (d < globalMin) globalMin = d;
                if (d > globalMax) globalMax = d;
            }
        }
        double range = globalMax - globalMin;
        if (range < 1e-15) range = 1.0;

        /* Pass 2: render with normalized distances */
        for (long p = 0; p < nPoints; p++) {
            float *step = roots + p * stride;
            for (int i = 0; i < degree; i++) {
                double re = step[i * 2], im = step[i * 2 + 1];
                int px = (int)(halfW + (re - centerRe) * scale);
                int py = (int)(halfH - (im - centerIm) * scale);
                if (px < 0 || px >= W || py < 0 || py >= H) {
                    rootsClipped++;
                    continue;
                }

                /* Compute min distance to any other root */
                double d2min = 1e30;
                for (int j = 0; j < degree; j++) {
                    if (j == i) continue;
                    double dr = re - step[j * 2];
                    double di = im - step[j * 2 + 1];
                    double d2 = dr * dr + di * di;
                    if (d2 < d2min) d2min = d2;
                }
                double t = (sqrt(d2min) - globalMin) / range;
                if (t < 0) t = 0;
                if (t > 1) t = 1;

                unsigned char cr, cg, cb;
                paletteRGB(proxPal, t, &cr, &cg, &cb);

                long idx = ((long)py * W + px) * 3;
                int v;
                v = pixels[idx]   + cr; pixels[idx]   = v > 255 ? 255 : v;
                v = pixels[idx+1] + cg; pixels[idx+1] = v > 255 ? 255 : v;
                v = pixels[idx+2] + cb; pixels[idx+2] = v > 255 ? 255 : v;
                rootsPlotted++;
            }
        }
    } else {
        /* --- Rainbow coloring (with optional matching) --- */
        int colorMap[MAXDEG];
        for (int i = 0; i < degree; i++) colorMap[i] = i;

        float prevStep[MAXDEG * 2];
        int havePrev = 0;
        int perm[MAXDEG];

        for (long p = 0; p < nPoints; p++) {
            float *step = roots + p * stride;

            /* Root matching */
            if (matchMode != MATCH_NONE && havePrev) {
                if (matchMode == MATCH_HUNGARIAN)
                    hungarianMatch(prevStep, step, degree, perm);
                else
                    greedyMatch(prevStep, step, degree, perm);

                /* Update colorMap: old root i had color colorMap[i],
                 * matched to new root perm[i].
                 * New root j should get color of the old root it was matched from. */
                int newColorMap[MAXDEG];
                for (int i = 0; i < degree; i++)
                    newColorMap[perm[i]] = colorMap[i];
                memcpy(colorMap, newColorMap, degree * sizeof(int));
            }

            /* Plot roots */
            for (int r = 0; r < degree; r++) {
                double re = step[r * 2];
                double im = step[r * 2 + 1];
                int px = (int)(halfW + (re - centerRe) * scale);
                int py = (int)(halfH - (im - centerIm) * scale);
                if (px >= 0 && px < W && py >= 0 && py < H) {
                    long idx = ((long)py * W + px) * 3;
                    int ci = colorMap[r];
                    int v;
                    v = pixels[idx]   + rbPalR[ci]; pixels[idx]   = v > 255 ? 255 : v;
                    v = pixels[idx+1] + rbPalG[ci]; pixels[idx+1] = v > 255 ? 255 : v;
                    v = pixels[idx+2] + rbPalB[ci]; pixels[idx+2] = v > 255 ? 255 : v;
                    rootsPlotted++;
                } else {
                    rootsClipped++;
                }
            }

            /* Save current step for next iteration's matching */
            if (matchMode != MATCH_NONE) {
                memcpy(prevStep, step, stride * sizeof(float));
                havePrev = 1;
            }
        }
    }

    /* Write raw image (12-byte header + pixel data) */
    if (raw_write(outPath, pixels, W, H, 3) != 0) {
        free(pixels); free(roots);
        return 1;
    }

    free(pixels);
    free(roots);

    /* Output metadata as JSON */
    printf("{\"roots_plotted\":%ld,\"roots_clipped\":%ld,\"n_points\":%ld,"
           "\"degree\":%d,\"color\":\"%s\",\"match\":\"%s\"",
           rootsPlotted, rootsClipped, nPoints, degree, colorStr, matchStr);
    if (colorMode == COLOR_PROXIMITY)
        printf(",\"palette\":\"%s\"", palName);
    printf("}\n");

    return 0;
}

/* ---- reduce mode ---- */

static int do_reduce(int argc, char **argv) {
    if (argc < 5) {
        fprintf(stderr, "Usage: imgpipe --reduce acc.raw next.raw out.raw [--gamma=2.2]\n");
        return 1;
    }
    const char *accPath = argv[2];
    const char *nextPath = argv[3];
    const char *outPath = argv[4];
    double gamma = getArgDouble(argc, argv, "--gamma", 2.2);

    /* Load both raw images */
    unsigned int W, H, bands;
    unsigned int W2, H2, bands2;
    unsigned char *accData = raw_read(accPath, &W, &H, &bands);
    if (!accData) return 1;
    unsigned char *nextData = raw_read(nextPath, &W2, &H2, &bands2);
    if (!nextData) { free(accData); return 1; }

    if (W != W2 || H != H2) {
        fprintf(stderr, "Image dimension mismatch: %ux%u vs %ux%u\n", W, H, W2, H2);
        free(accData); free(nextData);
        return 1;
    }

    size_t n = (size_t)W * H * bands;

    if (gamma > 0.01) {
        /* Gamma-correct blending via LUTs */
        buildGammaLUT(gamma);
        for (size_t i = 0; i < n; i++) {
            float sum = srgb2lin[accData[i]] + srgb2lin[nextData[i]];
            if (sum >= 1.0f) {
                accData[i] = 255;
            } else {
                int idx = (int)(sum * 4095.0f + 0.5f);
                if (idx > 4095) idx = 4095;
                accData[i] = lin2srgb[idx];
            }
        }
    } else {
        /* Raw saturating add (gamma=0, backward compatible) */
        for (size_t i = 0; i < n; i++) {
            int v = accData[i] + nextData[i];
            accData[i] = v > 255 ? 255 : (unsigned char)v;
        }
    }

    /* Write result as raw */
    if (raw_write(outPath, accData, W, H, bands) != 0) {
        free(accData); free(nextData);
        return 1;
    }

    free(accData);
    free(nextData);

    printf("{\"status\":\"ok\",\"width\":%u,\"height\":%u,\"gamma\":%.2f}\n", W, H, gamma);
    return 0;
}

/* ---- encode mode ---- */

static int do_encode(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: imgpipe --encode input.raw out.jpeg [--quality=90]\n");
        return 1;
    }
    const char *inPath = argv[2];
    const char *outPath = argv[3];
    int quality = getArgInt(argc, argv, "--quality", 90);

    /* Load raw image and wrap in VipsImage for encoding */
    unsigned int W, H, bands;
    unsigned char *data = raw_read(inPath, &W, &H, &bands);
    if (!data) return 1;

    VipsImage *img = vips_image_new_from_memory_copy(data, (size_t)W * H * bands,
                                                      W, H, bands, VIPS_FORMAT_UCHAR);
    free(data);
    if (!img) {
        fprintf(stderr, "vips_image_new_from_memory_copy failed\n");
        return 1;
    }

    /* Determine format from output extension */
    const char *ext = strrchr(outPath, '.');
    int isJpeg = ext && (strcmp(ext, ".jpeg") == 0 || strcmp(ext, ".jpg") == 0);

    if (isJpeg) {
        if (vips_jpegsave(img, outPath, "Q", quality, NULL)) {
            fprintf(stderr, "vips_jpegsave failed: %s\n", vips_error_buffer());
            g_object_unref(img);
            return 1;
        }
    } else {
        if (vips_pngsave(img, outPath, "compression", 6, NULL)) {
            fprintf(stderr, "vips_pngsave failed: %s\n", vips_error_buffer());
            g_object_unref(img);
            return 1;
        }
    }

    /* Report file size */
    FILE *f = fopen(outPath, "rb");
    long fsize = 0;
    if (f) { fseek(f, 0, SEEK_END); fsize = ftell(f); fclose(f); }

    g_object_unref(img);
    printf("{\"status\":\"ok\",\"file_size\":%ld,\"format\":\"%s\"}\n",
           fsize, isJpeg ? "jpeg" : "png");
    return 0;
}

/* ---- Main ---- */

int main(int argc, char **argv) {
    if (VIPS_INIT(argv[0])) {
        fprintf(stderr, "VIPS_INIT failed: %s\n", vips_error_buffer());
        return 1;
    }

    /* Suppress vips warnings to stderr */
    vips_leak_set(0);

    if (argc < 2) {
        fprintf(stderr, "Usage: imgpipe --roots2image|--reduce|--encode ...\n");
        vips_shutdown();
        return 1;
    }

    int ret;
    if (strcmp(argv[1], "--roots2image") == 0)
        ret = do_roots2image(argc, argv);
    else if (strcmp(argv[1], "--reduce") == 0)
        ret = do_reduce(argc, argv);
    else if (strcmp(argv[1], "--encode") == 0)
        ret = do_encode(argc, argv);
    else {
        fprintf(stderr, "Unknown mode: %s\n", argv[1]);
        ret = 1;
    }

    vips_shutdown();
    return ret;
}
