/*
 * roots2pix: render polynomial root positions to tile-bucketed .pix files.
 *
 * Reads f32 root positions from .bin file, projects each root to a pixel,
 * determines which 2D tile it belongs to, and emits packed 8-byte entries:
 *   [local_pixel_idx:uint32, rgb:uint32]
 * into per-tile output files.
 *
 * No canvas allocation. Memory: .bin file + tile output buffers (~512 KB each).
 *
 * Usage:
 *   roots2pix stripe.bin /tmp/pix --width=W --height=H
 *            --center_re=X --center_im=Y --scale=S --degree=D
 *            --tile_size=4096 --n_tile_cols=13 --n_tile_rows=13
 *            [--color=rainbow|proximity|constant]
 *            [--match=none|greedy|hungarian]
 *            [--palette=inferno|viridis|magma|plasma|turbo|cividis|warm|cool]
 *            [--constant_color=RRGGBB]
 *
 * Output: {outPrefix}_t0000.pix ... {outPrefix}_t{nTiles-1}.pix
 *
 * Build (static):
 *   aarch64-linux-musl-gcc -O3 -static -o roots2pix roots2pix.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#define MAXDEG 256
#define MAX_TILES 4096  /* up to 64x64 grid */
#define BUF_ENTRIES (128 * 1024)  /* 128K uint32s = 64K entries = 512 KB per tile */

/* ---- RGB type and palette definitions (16-step) ---- */

typedef struct { unsigned char r, g, b; } RGB;

static const RGB PAL_INFERNO[16] = {
    {0,0,4}, {16,11,53}, {43,15,95}, {72,12,119},
    {101,14,118}, {126,34,102}, {148,56,81}, {168,81,60},
    {186,108,41}, {203,137,25}, {217,169,13}, {228,201,27},
    {235,232,68}, {247,249,115}, {252,254,164}, {252,255,164}
};

static const RGB PAL_VIRIDIS[16] = {
    {68,1,84}, {72,20,103}, {71,40,120}, {63,57,131},
    {55,72,137}, {46,87,140}, {38,102,141}, {31,116,140},
    {26,131,137}, {27,146,130}, {40,161,119}, {65,175,102},
    {102,187,79}, {149,198,50}, {201,206,22}, {253,231,37}
};

static const RGB PAL_MAGMA[16] = {
    {0,0,4}, {13,7,49}, {38,11,93}, {65,8,123},
    {93,14,126}, {119,31,114}, {142,52,98}, {163,75,82},
    {184,101,68}, {204,130,56}, {221,162,47}, {234,196,53},
    {242,228,82}, {249,249,121}, {253,254,168}, {252,253,191}
};

static const RGB PAL_PLASMA[16] = {
    {13,8,135}, {47,5,146}, {79,2,150}, {107,2,145},
    {132,9,133}, {153,21,117}, {171,38,98}, {187,58,79},
    {201,81,59}, {213,107,39}, {223,135,22}, {231,165,11},
    {237,196,8}, {240,225,15}, {243,249,40}, {240,249,33}
};

static const RGB PAL_TURBO[16] = {
    {48,18,59}, {57,68,148}, {43,118,196}, {28,163,206},
    {20,200,178}, {44,222,128}, {96,237,79}, {156,240,43},
    {208,230,30}, {242,204,20}, {255,170,14}, {252,130,15},
    {236,89,16}, {210,49,14}, {175,18,8}, {122,4,3}
};

static const RGB PAL_CIVIDIS[16] = {
    {0,32,76}, {0,46,96}, {23,60,108}, {48,73,113},
    {72,85,116}, {93,97,119}, {113,110,121}, {132,122,119},
    {151,134,115}, {170,147,108}, {189,160,98}, {208,174,84},
    {226,189,65}, {242,205,43}, {254,222,19}, {253,238,6}
};

static const RGB PAL_WARM[16] = {
    {110,64,170}, {138,60,162}, {163,62,143}, {182,72,121},
    {196,87,97}, {208,107,75}, {216,130,56}, {222,155,42},
    {225,180,36}, {225,205,41}, {220,226,56}, {208,243,81},
    {190,252,108}, {168,254,139}, {145,253,168}, {122,250,196}
};

static const RGB PAL_COOL[16] = {
    {110,64,170}, {100,82,192}, {88,101,207}, {75,119,215},
    {62,137,217}, {52,154,213}, {44,170,203}, {40,185,188},
    {42,199,168}, {53,211,145}, {71,222,119}, {96,230,91},
    {126,236,65}, {160,240,44}, {194,241,32}, {228,238,29}
};

typedef struct { const char *name; const RGB *colors; } PalEntry;
static const PalEntry PALETTES[] = {
    {"inferno", PAL_INFERNO}, {"viridis", PAL_VIRIDIS},
    {"magma",   PAL_MAGMA},   {"plasma",  PAL_PLASMA},
    {"turbo",   PAL_TURBO},   {"cividis", PAL_CIVIDIS},
    {"warm",    PAL_WARM},    {"cool",    PAL_COOL},
    {NULL, NULL}
};

static const RGB *findPalette(const char *name) {
    if (!name) return PAL_INFERNO;
    for (int i = 0; PALETTES[i].name; i++)
        if (strcmp(PALETTES[i].name, name) == 0)
            return PALETTES[i].colors;
    return PAL_INFERNO;
}

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

/* ---- Rainbow palette (HSL) ---- */

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

/* ---- Root matching: greedy ---- */

static void greedyMatch(const float *prevStep, const float *currStep,
                        int degree, int *perm) {
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

/* ---- Root matching: Hungarian (Kuhn-Munkres O(n^3)) ---- */

static void hungarianMatch(const float *prevStep, const float *currStep,
                           int degree, int *perm) {
    double cost[MAXDEG * MAXDEG];
    for (int i = 0; i < degree; i++)
        for (int j = 0; j < degree; j++) {
            double dr = currStep[j * 2] - prevStep[i * 2];
            double di = currStep[j * 2 + 1] - prevStep[i * 2 + 1];
            cost[i * degree + j] = dr * dr + di * di;
        }

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

    for (int j = 1; j <= n; j++)
        perm[p[j] - 1] = j - 1;
}

/* ---- Tile output buffering ---- */

static FILE *tileFiles[MAX_TILES];
static uint32_t *tileBuf[MAX_TILES];
static int tileBufPos[MAX_TILES];
static int tileW[MAX_TILES];  /* actual width of each tile (edge tiles may be smaller) */

static void flush_tile(int t) {
    if (tileBufPos[t] > 0) {
        fwrite(tileBuf[t], 4, tileBufPos[t], tileFiles[t]);
        tileBufPos[t] = 0;
    }
}

static inline void emit_pixel(int tile_id, uint32_t pix_idx, uint32_t rgb) {
    tileBuf[tile_id][tileBufPos[tile_id]++] = pix_idx;
    tileBuf[tile_id][tileBufPos[tile_id]++] = rgb;
    if (tileBufPos[tile_id] >= BUF_ENTRIES)
        flush_tile(tile_id);
}

/* ---- Main ---- */

enum ColorMode { COLOR_RAINBOW = 0, COLOR_PROXIMITY = 1, COLOR_CONSTANT = 2 };
enum MatchMode { MATCH_NONE = 0, MATCH_GREEDY = 1, MATCH_HUNGARIAN = 2 };

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: roots2pix stripe.bin /tmp/pix "
                "--width=W --height=H --center_re=X --center_im=Y --scale=S "
                "--degree=D --tile_size=T --n_tile_cols=C --n_tile_rows=R "
                "[--color=rainbow|proximity|constant] "
                "[--match=none|greedy|hungarian] [--palette=inferno|...] "
                "[--constant_color=RRGGBB]\n");
        return 1;
    }
    const char *binPath = argv[1];
    const char *outPrefix = argv[2];
    int W = getArgInt(argc, argv, "--width", 4096);
    int H = getArgInt(argc, argv, "--height", 4096);
    double centerRe = getArgDouble(argc, argv, "--center_re", 0.0);
    double centerIm = getArgDouble(argc, argv, "--center_im", 0.0);
    double scale = getArgDouble(argc, argv, "--scale", 100.0);
    int degree = getArgInt(argc, argv, "--degree", 25);
    int tileSize = getArgInt(argc, argv, "--tile_size", 4096);
    int nTileCols = getArgInt(argc, argv, "--n_tile_cols", 1);
    int nTileRows = getArgInt(argc, argv, "--n_tile_rows", 1);
    const char *colorStr = getArgStr(argc, argv, "--color", "rainbow");
    const char *matchStr = getArgStr(argc, argv, "--match", "none");
    const char *palName = getArgStr(argc, argv, "--palette", "inferno");
    const char *constColorStr = getArgStr(argc, argv, "--constant_color", "ffffff");

    enum ColorMode colorMode = COLOR_RAINBOW;
    if (strcmp(colorStr, "proximity") == 0) colorMode = COLOR_PROXIMITY;
    else if (strcmp(colorStr, "constant") == 0) colorMode = COLOR_CONSTANT;

    unsigned int constHex = 0xffffff;
    sscanf(constColorStr, "%x", &constHex);
    unsigned char constR = (constHex >> 16) & 0xff;
    unsigned char constG = (constHex >> 8) & 0xff;
    unsigned char constB = constHex & 0xff;

    enum MatchMode matchMode = MATCH_NONE;
    if (strcmp(matchStr, "greedy") == 0) matchMode = MATCH_GREEDY;
    else if (strcmp(matchStr, "hungarian") == 0) matchMode = MATCH_HUNGARIAN;

    const RGB *proxPal = findPalette(palName);

    if (W < 1 || W > 65536 || H < 1 || H > 65536) {
        fprintf(stderr, "Invalid dimensions: %dx%d\n", W, H);
        return 1;
    }
    if (degree < 1 || degree > MAXDEG) {
        fprintf(stderr, "Invalid degree: %d\n", degree);
        return 1;
    }

    int nTiles = nTileCols * nTileRows;
    if (nTiles < 1 || nTiles > MAX_TILES) {
        fprintf(stderr, "Invalid tile grid: %dx%d = %d tiles (max %d)\n",
                nTileCols, nTileRows, nTiles, MAX_TILES);
        return 1;
    }

    /* Compute actual tile widths/heights (edge tiles may be smaller) */
    int tileH[MAX_TILES];
    for (int t = 0; t < nTiles; t++) {
        int tc = t % nTileCols;
        int tr = t / nTileCols;
        tileW[t] = (tc < nTileCols - 1) ? tileSize : (W - tc * tileSize);
        tileH[t] = (tr < nTileRows - 1) ? tileSize : (H - tr * tileSize);
        if (tileW[t] <= 0 || tileH[t] <= 0) {
            fprintf(stderr, "Invalid tile %d: w=%d h=%d\n", t, tileW[t], tileH[t]);
            return 1;
        }
    }

    /* Open tile output files and allocate buffers */
    char pathBuf[512];
    for (int t = 0; t < nTiles; t++) {
        snprintf(pathBuf, sizeof(pathBuf), "%s_t%04d.pix", outPrefix, t);
        tileFiles[t] = fopen(pathBuf, "wb");
        if (!tileFiles[t]) {
            fprintf(stderr, "Cannot create %s\n", pathBuf);
            return 1;
        }
        tileBuf[t] = malloc(BUF_ENTRIES * sizeof(uint32_t));
        if (!tileBuf[t]) {
            fprintf(stderr, "Cannot allocate tile buffer %d\n", t);
            return 1;
        }
        tileBufPos[t] = 0;
    }

    /* Read binary root data */
    FILE *fin = fopen(binPath, "rb");
    if (!fin) { fprintf(stderr, "Cannot open %s\n", binPath); return 1; }
    fseek(fin, 0, SEEK_END);
    long fileSize = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    int stride = degree * 2;
    long nPoints = fileSize / (stride * sizeof(float));
    if (nPoints <= 0) { fprintf(stderr, "Empty root file\n"); fclose(fin); return 1; }

    float *roots = malloc(fileSize);
    if (!roots) { fprintf(stderr, "Cannot allocate %ld bytes\n", fileSize); fclose(fin); return 1; }
    fread(roots, 1, fileSize, fin);
    fclose(fin);

    /* Build rainbow palette */
    unsigned char rbPalR[MAXDEG], rbPalG[MAXDEG], rbPalB[MAXDEG];
    for (int i = 0; i < degree; i++)
        rainbowRGB(i, degree, &rbPalR[i], &rbPalG[i], &rbPalB[i]);

    long rootsPlotted = 0, rootsClipped = 0;
    double halfW = W / 2.0, halfH = H / 2.0;

    if (colorMode == COLOR_PROXIMITY) {
        /* --- Proximity coloring: two-pass --- */
        /* Pass 1: find global min/max nearest-neighbor distance */
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

        /* Pass 2: emit pixels with proximity-based color */
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

                /* Tile bucketing */
                int tile_col = px / tileSize;
                int tile_row = py / tileSize;
                int tile_id = tile_row * nTileCols + tile_col;
                uint32_t local_x = px - tile_col * tileSize;
                uint32_t local_y = py - tile_row * tileSize;
                uint32_t pix_idx = local_y * (uint32_t)tileW[tile_id] + local_x;
                uint32_t rgb = ((uint32_t)cr << 16) | ((uint32_t)cg << 8) | cb;
                emit_pixel(tile_id, pix_idx, rgb);
                rootsPlotted++;
            }
        }
    } else if (colorMode == COLOR_CONSTANT) {
        uint32_t constRGB = ((uint32_t)constR << 16) | ((uint32_t)constG << 8) | constB;

        for (long p = 0; p < nPoints; p++) {
            float *step = roots + p * stride;
            for (int r = 0; r < degree; r++) {
                double re = step[r * 2];
                double im = step[r * 2 + 1];
                int px = (int)(halfW + (re - centerRe) * scale);
                int py = (int)(halfH - (im - centerIm) * scale);
                if (px >= 0 && px < W && py >= 0 && py < H) {
                    int tile_col = px / tileSize;
                    int tile_row = py / tileSize;
                    int tile_id = tile_row * nTileCols + tile_col;
                    uint32_t local_x = px - tile_col * tileSize;
                    uint32_t local_y = py - tile_row * tileSize;
                    uint32_t pix_idx = local_y * (uint32_t)tileW[tile_id] + local_x;
                    emit_pixel(tile_id, pix_idx, constRGB);
                    rootsPlotted++;
                } else {
                    rootsClipped++;
                }
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

            if (matchMode != MATCH_NONE && havePrev) {
                if (matchMode == MATCH_HUNGARIAN)
                    hungarianMatch(prevStep, step, degree, perm);
                else
                    greedyMatch(prevStep, step, degree, perm);

                int newColorMap[MAXDEG];
                for (int i = 0; i < degree; i++)
                    newColorMap[perm[i]] = colorMap[i];
                memcpy(colorMap, newColorMap, degree * sizeof(int));
            }

            for (int r = 0; r < degree; r++) {
                double re = step[r * 2];
                double im = step[r * 2 + 1];
                int px = (int)(halfW + (re - centerRe) * scale);
                int py = (int)(halfH - (im - centerIm) * scale);
                if (px >= 0 && px < W && py >= 0 && py < H) {
                    int ci = colorMap[r];
                    int tile_col = px / tileSize;
                    int tile_row = py / tileSize;
                    int tile_id = tile_row * nTileCols + tile_col;
                    uint32_t local_x = px - tile_col * tileSize;
                    uint32_t local_y = py - tile_row * tileSize;
                    uint32_t pix_idx = local_y * (uint32_t)tileW[tile_id] + local_x;
                    uint32_t rgb = ((uint32_t)rbPalR[ci] << 16) |
                                   ((uint32_t)rbPalG[ci] << 8) |
                                    rbPalB[ci];
                    emit_pixel(tile_id, pix_idx, rgb);
                    rootsPlotted++;
                } else {
                    rootsClipped++;
                }
            }

            if (matchMode != MATCH_NONE) {
                memcpy(prevStep, step, stride * sizeof(float));
                havePrev = 1;
            }
        }
    }

    /* Flush remaining buffers and close files */
    long totalEntries = 0;
    int tilesWithData = 0;
    for (int t = 0; t < nTiles; t++) {
        flush_tile(t);
        long pos = ftell(tileFiles[t]);
        if (pos > 0) tilesWithData++;
        totalEntries += pos / 8;
        fclose(tileFiles[t]);
        free(tileBuf[t]);
    }

    free(roots);

    /* Remove empty tile files */
    for (int t = 0; t < nTiles; t++) {
        snprintf(pathBuf, sizeof(pathBuf), "%s_t%04d.pix", outPrefix, t);
        /* Check file size; remove if empty */
        FILE *check = fopen(pathBuf, "rb");
        if (check) {
            fseek(check, 0, SEEK_END);
            long sz = ftell(check);
            fclose(check);
            if (sz == 0) remove(pathBuf);
        }
    }

    /* Output metadata as JSON */
    printf("{\"roots_plotted\":%ld,\"roots_clipped\":%ld,\"n_points\":%ld,"
           "\"degree\":%d,\"color\":\"%s\",\"match\":\"%s\","
           "\"n_tiles\":%d,\"tiles_with_data\":%d,\"total_entries\":%ld",
           rootsPlotted, rootsClipped, nPoints, degree, colorStr, matchStr,
           nTiles, tilesWithData, totalEntries);
    if (colorMode == COLOR_PROXIMITY)
        printf(",\"palette\":\"%s\"", palName);
    else if (colorMode == COLOR_CONSTANT)
        printf(",\"constant_color\":\"%s\"", constColorStr);
    printf("}\n");

    return 0;
}
