/*
 * roots2pix_mt: multithreaded color raster for supported Color modes.
 *
 * This is the native MT replacement for the earlier subprocess fan-out path.
 * It keeps the same external .pix / .pbx contracts used by finalize and fast
 * Color RePalette, but computes them in one process with pthread workers.
 *
 * Supported modes:
 *   - solve_score
 *   - saved_palette
 *   - constant
 *   - rainbow with match=none
 *
 * Unsupported here:
 *   - proximity
 *   - rainbow with greedy / hungarian
 *
 * Build:
 *   aarch64-linux-musl-gcc -O3 -static -pthread -o roots2pix_mt roots2pix_mt.c -lm
 */

#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "palette_lut.h"
#include "root_xforms.h"
#include "solve_score.h"

#define MAXDEG 256
#define MAX_TILES 4096

enum ColorMode {
    COLOR_RAINBOW = 0,
    COLOR_CONSTANT = 1,
    COLOR_SOLVE_SCORE = 2,
    COLOR_SAVED_PALETTE = 3,
};

typedef struct {
    uint32_t *data;
    size_t len;
    size_t cap;
} U32Vec;

typedef struct {
    int id;
    long start;
    long end;
    int degree;
    int stride;
    int W;
    int H;
    int tileSize;
    int nTileCols;
    int nTileRows;
    int nTiles;
    double centerRe;
    double centerIm;
    double scale;
    double cosA;
    double sinA;
    double halfW;
    double halfH;
    enum ColorMode colorMode;
    enum SolveMetric solveMetric;
    double solveScoreClipLo;
    double solveScoreClipHi;
    double solveScoreOmega;
    int solveScoreOmegaEnabled;
    double solveScoreCuts[9];
    int nSolveScoreCuts;
    uint32_t constRGB;
    int emitPixelBins;
    const float *roots;
    const uint8_t *solveBins;
    RootXformEntry *rtChain;
    int nRt;
    uint64_t **tileBits;
    int *tileW;
    U32Vec *pixVecs;
    U32Vec *pbxVecs;
    unsigned char rbPalR[MAXDEG];
    unsigned char rbPalG[MAXDEG];
    unsigned char rbPalB[MAXDEG];
    unsigned char ssPalR[10];
    unsigned char ssPalG[10];
    unsigned char ssPalB[10];
    long rootsPlotted;
    long rootsClipped;
    long rootsDeduped;
    int error;
    char error_msg[256];
} WorkerArgs;

static const char *getArg(int argc, char **argv, const char *key) {
    int klen = (int)strlen(key);
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], key, klen) == 0 && argv[i][klen] == '=') {
            return argv[i] + klen + 1;
        }
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

static int clamp_threads(int requested, long n_items) {
    int threads = requested < 1 ? 1 : requested;
    if (n_items > 0 && threads > (int)n_items) threads = (int)n_items;
    if (threads < 1) threads = 1;
    return threads;
}

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

static int vec_push2(U32Vec *vec, uint32_t a, uint32_t b) {
    if (vec->len + 2 > vec->cap) {
        size_t newCap = vec->cap ? vec->cap * 2 : 2048;
        while (newCap < vec->len + 2) newCap *= 2;
        uint32_t *newData = realloc(vec->data, newCap * sizeof(uint32_t));
        if (!newData) return 0;
        vec->data = newData;
        vec->cap = newCap;
    }
    vec->data[vec->len++] = a;
    vec->data[vec->len++] = b;
    return 1;
}

static const float *prepare_step(const float *raw, int degree,
                                 RootXformEntry *rtChain, int nRt,
                                 float *stepBuf, float *wkRe, float *wkIm) {
    if (nRt <= 0) return raw;
    for (int i = 0; i < degree; i++) {
        wkRe[i] = raw[i * 2];
        wkIm[i] = raw[i * 2 + 1];
    }
    apply_root_xforms(rtChain, nRt, wkRe, wkIm, degree);
    for (int i = 0; i < degree; i++) {
        stepBuf[i * 2] = wkRe[i];
        stepBuf[i * 2 + 1] = wkIm[i];
    }
    return stepBuf;
}

static int claim_pixel(uint64_t *tileWords, uint32_t pix_idx) {
    size_t wordIdx = (size_t)(pix_idx >> 6);
    uint64_t bit = 1ULL << (pix_idx & 63);
    uint64_t old = __atomic_fetch_or(&tileWords[wordIdx], bit, __ATOMIC_RELAXED);
    return (old & bit) == 0;
}

static void worker_fail(WorkerArgs *arg, const char *msg) {
    arg->error = 1;
    strncpy(arg->error_msg, msg, sizeof(arg->error_msg) - 1);
    arg->error_msg[sizeof(arg->error_msg) - 1] = '\0';
}

static void *worker_main(void *arg_) {
    WorkerArgs *arg = (WorkerArgs *)arg_;
    float stepBuf[MAXDEG * 2];
    float wkRe[MAXDEG];
    float wkIm[MAXDEG];
    double ssRange = arg->solveScoreClipHi - arg->solveScoreClipLo;

    for (long p = arg->start; p < arg->end; p++) {
        const float *rawStep = arg->roots + p * arg->stride;
        const float *step = prepare_step(rawStep, arg->degree, arg->rtChain, arg->nRt, stepBuf, wkRe, wkIm);

        uint32_t solveRGB = 0;
        uint8_t solveBin = 255;

        if (arg->colorMode == COLOR_SOLVE_SCORE) {
            double score = compute_solve_metric_score(step, arg->degree, arg->solveMetric);
            double u = (score - arg->solveScoreClipLo) / ssRange;
            if (u < 0) u = 0;
            if (u > 1) u = 1;
            u = apply_solve_score_transfer(u, arg->solveScoreOmegaEnabled, arg->solveScoreOmega);
            int bin = 9;
            for (int c = 0; c < arg->nSolveScoreCuts; c++) {
                if (u <= arg->solveScoreCuts[c]) { bin = c; break; }
            }
            solveBin = (uint8_t)bin;
            solveRGB = ((uint32_t)arg->ssPalR[bin] << 16) |
                       ((uint32_t)arg->ssPalG[bin] << 8) |
                       arg->ssPalB[bin];
        } else if (arg->colorMode == COLOR_SAVED_PALETTE) {
            uint8_t bin = arg->solveBins[p];
            if (bin > 9) {
                worker_fail(arg, "saved_palette bin out of range");
                return NULL;
            }
            solveBin = bin;
            solveRGB = ((uint32_t)arg->ssPalR[bin] << 16) |
                       ((uint32_t)arg->ssPalG[bin] << 8) |
                       arg->ssPalB[bin];
        }

        for (int r = 0; r < arg->degree; r++) {
            double re = step[r * 2];
            double im = step[r * 2 + 1];
            double dx = re - arg->centerRe;
            double dy = im - arg->centerIm;
            double rx = dx * arg->cosA - dy * arg->sinA;
            double ry = dx * arg->sinA + dy * arg->cosA;
            int px = (int)(arg->halfW + rx * arg->scale);
            int py = (int)(arg->halfH - ry * arg->scale);
            if (px < 0 || px >= arg->W || py < 0 || py >= arg->H) {
                arg->rootsClipped++;
                continue;
            }

            int tileCol = px / arg->tileSize;
            int tileRow = py / arg->tileSize;
            int tileId = tileRow * arg->nTileCols + tileCol;
            uint32_t localX = (uint32_t)(px - tileCol * arg->tileSize);
            uint32_t localY = (uint32_t)(py - tileRow * arg->tileSize);
            uint32_t pixIdx = localY * (uint32_t)arg->tileW[tileId] + localX;

            if (!claim_pixel(arg->tileBits[tileId], pixIdx)) {
                arg->rootsDeduped++;
                continue;
            }

            uint32_t rgb = solveRGB;
            if (arg->colorMode == COLOR_CONSTANT) {
                rgb = arg->constRGB;
            } else if (arg->colorMode == COLOR_RAINBOW) {
                rgb = ((uint32_t)arg->rbPalR[r] << 16) |
                      ((uint32_t)arg->rbPalG[r] << 8) |
                       arg->rbPalB[r];
            }

            if (!vec_push2(&arg->pixVecs[tileId], pixIdx, rgb)) {
                worker_fail(arg, "pix vec alloc failed");
                return NULL;
            }
            if (arg->emitPixelBins && (arg->colorMode == COLOR_SOLVE_SCORE || arg->colorMode == COLOR_SAVED_PALETTE)) {
                if (!vec_push2(&arg->pbxVecs[tileId], pixIdx, (uint32_t)solveBin)) {
                    worker_fail(arg, "pbx vec alloc failed");
                    return NULL;
                }
            }
            arg->rootsPlotted++;
        }
    }
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: roots2pix_mt stripe.bin /tmp/pix "
                "--width=W --height=H --center_re=X --center_im=Y --scale=S "
                "--degree=D --tile_size=T --n_tile_cols=C --n_tile_rows=R "
                "[--threads=N] [--color=rainbow|solve_score|saved_palette|constant] "
                "[--match=none] [--palette=<name>] [--constant_color=RRGGBB] "
                "[--solve_metric=proximity|crowding|spread|anisotropy|area|clusteriness|shelliness|outlierness|nn_variation|real_axis_proximity|centroid_re|centroid_im|centroid_dist|dist_unit_circle|asymmetry_re] "
                "[--solve_score_clip_lo=X --solve_score_clip_hi=Y --solve_score_cuts=c1,...,c9] "
                "[--solve_score_omega=W] [--solve_score_omega_enabled=0|1] "
                "[--solve_bins_file=file.bin] [--pixel_bin_prefix=/tmp/pixbin] [--root_xforms=file.json]\n");
        return 1;
    }

    const char *binPath = argv[1];
    const char *outPrefix = argv[2];
    int W = getArgInt(argc, argv, "--width", 4096);
    int H = getArgInt(argc, argv, "--height", 4096);
    double centerRe = getArgDouble(argc, argv, "--center_re", 0.0);
    double centerIm = getArgDouble(argc, argv, "--center_im", 0.0);
    double scale = getArgDouble(argc, argv, "--scale", 100.0);
    double rotation = getArgDouble(argc, argv, "--rotation", 0.0);
    double cosA = cos(rotation), sinA = sin(rotation);
    int degree = getArgInt(argc, argv, "--degree", 25);
    int tileSize = getArgInt(argc, argv, "--tile_size", 4096);
    int nTileCols = getArgInt(argc, argv, "--n_tile_cols", 1);
    int nTileRows = getArgInt(argc, argv, "--n_tile_rows", 1);
    int requestedThreads = getArgInt(argc, argv, "--threads", 1);
    const char *colorStr = getArgStr(argc, argv, "--color", "rainbow");
    const char *matchStr = getArgStr(argc, argv, "--match", "none");
    const char *palName = getArgStr(argc, argv, "--palette", "inferno");
    const char *solveBinsPath = getArgStr(argc, argv, "--solve_bins_file", NULL);
    const char *pixelBinPrefix = getArgStr(argc, argv, "--pixel_bin_prefix", NULL);
    const char *constColorStr = getArgStr(argc, argv, "--constant_color", "ffffff");
    const char *rtPath = getArgStr(argc, argv, "--root_xforms", NULL);

    enum ColorMode colorMode = COLOR_RAINBOW;
    if (strcmp(colorStr, "solve_score") == 0 || strcmp(colorStr, "solve_proximity") == 0) colorMode = COLOR_SOLVE_SCORE;
    else if (strcmp(colorStr, "saved_palette") == 0) colorMode = COLOR_SAVED_PALETTE;
    else if (strcmp(colorStr, "constant") == 0) colorMode = COLOR_CONSTANT;
    else if (strcmp(colorStr, "rainbow") == 0) colorMode = COLOR_RAINBOW;
    else {
        fprintf(stderr, "Unsupported color mode for roots2pix_mt: %s\n", colorStr);
        return 1;
    }

    if (strcmp(matchStr, "none") != 0) {
        fprintf(stderr, "roots2pix_mt only supports --match=none\n");
        return 1;
    }
    if (degree < 1 || degree > MAXDEG) {
        fprintf(stderr, "Invalid degree: %d\n", degree);
        return 1;
    }
    if (W < 1 || H < 1) {
        fprintf(stderr, "Invalid dimensions: %dx%d\n", W, H);
        return 1;
    }

    int nTiles = nTileCols * nTileRows;
    if (nTiles < 1 || nTiles > MAX_TILES) {
        fprintf(stderr, "Invalid tile grid: %dx%d\n", nTileCols, nTileRows);
        return 1;
    }

    RootXformEntry rtChain[MAX_RT_CHAIN];
    int nRt = 0;
    if (rtPath) {
        nRt = parse_root_xform_file(rtPath, rtChain, MAX_RT_CHAIN);
        if (nRt == 0) {
            fprintf(stderr, "Failed to parse root transforms from %s\n", rtPath);
            return 1;
        }
    }

    enum SolveMetric solveMetric = SOLVE_METRIC_PROXIMITY;
    const char *solveMetricStr = getArgStr(argc, argv, "--solve_metric", "proximity");
    if ((colorMode == COLOR_SOLVE_SCORE) && !parse_solve_metric(solveMetricStr, &solveMetric)) {
        fprintf(stderr, "ERROR: unknown solve_metric '%s'\n", solveMetricStr);
        return 1;
    }

    double solveScoreClipLo = getArgDouble(argc, argv, "--solve_score_clip_lo",
                               getArgDouble(argc, argv, "--solve_prox_clip_lo", 0));
    double solveScoreClipHi = getArgDouble(argc, argv, "--solve_score_clip_hi",
                               getArgDouble(argc, argv, "--solve_prox_clip_hi", 0));
    double solveScoreOmega = getArgDouble(argc, argv, "--solve_score_omega", 1.0);
    int solveScoreOmegaEnabled = getArgInt(argc, argv, "--solve_score_omega_enabled", 1);
    double solveScoreCuts[9] = {0};
    int nSolveScoreCuts = 0;
    {
        const char *cutsStr = getArgStr(argc, argv, "--solve_score_cuts",
                               getArgStr(argc, argv, "--solve_prox_cuts", NULL));
        if (cutsStr) {
            char tmp[256];
            strncpy(tmp, cutsStr, sizeof(tmp) - 1);
            tmp[sizeof(tmp) - 1] = '\0';
            char *tok = strtok(tmp, ",");
            while (tok && nSolveScoreCuts < 9) {
                solveScoreCuts[nSolveScoreCuts++] = atof(tok);
                tok = strtok(NULL, ",");
            }
        }
    }
    if (colorMode == COLOR_SOLVE_SCORE) {
        if (nSolveScoreCuts != 9) {
            fprintf(stderr, "solve_score requires exactly 9 cuts (got %d)\n", nSolveScoreCuts);
            return 1;
        }
        if (solveScoreClipHi - solveScoreClipLo < 1e-12) {
            fprintf(stderr, "solve_score requires valid clip range\n");
            return 1;
        }
    }

    unsigned int constHex = 0xffffff;
    sscanf(constColorStr, "%x", &constHex);
    uint32_t constRGB = (((constHex >> 16) & 0xffu) << 16) |
                        (((constHex >> 8) & 0xffu) << 8) |
                        (constHex & 0xffu);

    FILE *fin = fopen(binPath, "rb");
    if (!fin) {
        fprintf(stderr, "Cannot open %s\n", binPath);
        return 1;
    }
    fseek(fin, 0, SEEK_END);
    long fileSize = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    int stride = degree * 2;
    long nPoints = fileSize / (stride * (long)sizeof(float));
    if (nPoints <= 0) {
        fprintf(stderr, "Empty root file\n");
        fclose(fin);
        return 1;
    }

    float *roots = malloc((size_t)fileSize);
    if (!roots) {
        fprintf(stderr, "Cannot allocate %ld bytes\n", fileSize);
        fclose(fin);
        return 1;
    }
    if ((long)fread(roots, 1, (size_t)fileSize, fin) != fileSize) {
        fprintf(stderr, "Short read\n");
        fclose(fin);
        free(roots);
        return 1;
    }
    fclose(fin);

    uint8_t *solveBins = NULL;
    if (colorMode == COLOR_SAVED_PALETTE) {
        if (!solveBinsPath) {
            fprintf(stderr, "saved_palette requires --solve_bins_file\n");
            free(roots);
            return 1;
        }
        FILE *fb = fopen(solveBinsPath, "rb");
        if (!fb) {
            fprintf(stderr, "Cannot open %s\n", solveBinsPath);
            free(roots);
            return 1;
        }
        fseek(fb, 0, SEEK_END);
        long binSize = ftell(fb);
        fseek(fb, 0, SEEK_SET);
        if (binSize != nPoints) {
            fprintf(stderr, "saved_palette bins size mismatch: got %ld expected %ld\n", binSize, nPoints);
            fclose(fb);
            free(roots);
            return 1;
        }
        solveBins = malloc((size_t)nPoints);
        if (!solveBins) {
            fprintf(stderr, "Cannot allocate solve bins\n");
            fclose(fb);
            free(roots);
            return 1;
        }
        if ((long)fread(solveBins, 1, (size_t)nPoints, fb) != nPoints) {
            fprintf(stderr, "Short read from %s\n", solveBinsPath);
            fclose(fb);
            free(solveBins);
            free(roots);
            return 1;
        }
        fclose(fb);
    }

    int tileW[MAX_TILES];
    int tileH[MAX_TILES];
    size_t tileWordCount[MAX_TILES];
    uint64_t *tileBits[MAX_TILES];
    for (int t = 0; t < nTiles; t++) {
        int tc = t % nTileCols;
        int tr = t / nTileCols;
        tileW[t] = (tc < nTileCols - 1) ? tileSize : (W - tc * tileSize);
        tileH[t] = (tr < nTileRows - 1) ? tileSize : (H - tr * tileSize);
        if (tileW[t] <= 0 || tileH[t] <= 0) {
            fprintf(stderr, "Invalid tile %d geometry\n", t);
            free(solveBins);
            free(roots);
            return 1;
        }
        tileWordCount[t] = (((size_t)tileW[t] * (size_t)tileH[t]) + 63u) / 64u;
        tileBits[t] = calloc(tileWordCount[t], sizeof(uint64_t));
        if (!tileBits[t]) {
            fprintf(stderr, "Cannot allocate tile bitset %d\n", t);
            free(solveBins);
            free(roots);
            return 1;
        }
    }

    unsigned char rbPalR[MAXDEG], rbPalG[MAXDEG], rbPalB[MAXDEG];
    for (int i = 0; i < degree; i++) {
        rainbowRGB(i, degree, &rbPalR[i], &rbPalG[i], &rbPalB[i]);
    }
    const PaletteDef *proxPal = findPalette(palName);
    unsigned char ssPalR[10], ssPalG[10], ssPalB[10];
    for (int b = 0; b < 10; b++) {
        paletteRGB(proxPal, (b + 0.5) / 10.0, &ssPalR[b], &ssPalG[b], &ssPalB[b]);
    }

    int emitPixelBins = pixelBinPrefix &&
        (colorMode == COLOR_SOLVE_SCORE || colorMode == COLOR_SAVED_PALETTE);
    int threads = clamp_threads(requestedThreads, nPoints);

    WorkerArgs *args = calloc((size_t)threads, sizeof(WorkerArgs));
    pthread_t *workers = calloc((size_t)threads, sizeof(pthread_t));
    if (!args || !workers) {
        fprintf(stderr, "Out of memory for raster threads\n");
        free(workers);
        free(args);
        free(solveBins);
        free(roots);
        return 1;
    }

    long base = nPoints / threads;
    long extra = nPoints % threads;
    long start = 0;
    for (int i = 0; i < threads; i++) {
        long width = base + (i < extra ? 1 : 0);
        args[i].id = i;
        args[i].start = start;
        args[i].end = start + width;
        args[i].degree = degree;
        args[i].stride = stride;
        args[i].W = W;
        args[i].H = H;
        args[i].tileSize = tileSize;
        args[i].nTileCols = nTileCols;
        args[i].nTileRows = nTileRows;
        args[i].nTiles = nTiles;
        args[i].centerRe = centerRe;
        args[i].centerIm = centerIm;
        args[i].scale = scale;
        args[i].cosA = cosA;
        args[i].sinA = sinA;
        args[i].halfW = W / 2.0;
        args[i].halfH = H / 2.0;
        args[i].colorMode = colorMode;
        args[i].solveMetric = solveMetric;
        args[i].solveScoreClipLo = solveScoreClipLo;
        args[i].solveScoreClipHi = solveScoreClipHi;
        args[i].solveScoreOmega = solveScoreOmega;
        args[i].solveScoreOmegaEnabled = solveScoreOmegaEnabled;
        memcpy(args[i].solveScoreCuts, solveScoreCuts, sizeof(solveScoreCuts));
        args[i].nSolveScoreCuts = nSolveScoreCuts;
        args[i].constRGB = constRGB;
        args[i].emitPixelBins = emitPixelBins;
        args[i].roots = roots;
        args[i].solveBins = solveBins;
        args[i].rtChain = rtChain;
        args[i].nRt = nRt;
        args[i].tileBits = tileBits;
        args[i].tileW = tileW;
        args[i].pixVecs = calloc((size_t)nTiles, sizeof(U32Vec));
        args[i].pbxVecs = emitPixelBins ? calloc((size_t)nTiles, sizeof(U32Vec)) : NULL;
        memcpy(args[i].rbPalR, rbPalR, sizeof(rbPalR));
        memcpy(args[i].rbPalG, rbPalG, sizeof(rbPalG));
        memcpy(args[i].rbPalB, rbPalB, sizeof(rbPalB));
        memcpy(args[i].ssPalR, ssPalR, sizeof(ssPalR));
        memcpy(args[i].ssPalG, ssPalG, sizeof(ssPalG));
        memcpy(args[i].ssPalB, ssPalB, sizeof(ssPalB));
        if (!args[i].pixVecs || (emitPixelBins && !args[i].pbxVecs)) {
            fprintf(stderr, "Out of memory for worker vectors\n");
            return 1;
        }
        pthread_create(&workers[i], NULL, worker_main, &args[i]);
        start += width;
    }

    long rootsPlotted = 0;
    long rootsClipped = 0;
    long rootsDeduped = 0;
    int workerError = 0;
    char workerErrorMsg[256] = {0};
    for (int i = 0; i < threads; i++) {
        pthread_join(workers[i], NULL);
        rootsPlotted += args[i].rootsPlotted;
        rootsClipped += args[i].rootsClipped;
        rootsDeduped += args[i].rootsDeduped;
        if (args[i].error && !workerError) {
            workerError = 1;
            strncpy(workerErrorMsg, args[i].error_msg, sizeof(workerErrorMsg) - 1);
        }
    }
    if (workerError) {
        fprintf(stderr, "roots2pix_mt worker failed: %s\n", workerErrorMsg);
        return 1;
    }

    char pathBuf[512];
    long totalEntries = 0;
    int tilesWithData = 0;
    for (int t = 0; t < nTiles; t++) {
        size_t tilePixU32 = 0;
        size_t tilePbxU32 = 0;
        for (int i = 0; i < threads; i++) {
            tilePixU32 += args[i].pixVecs[t].len;
            if (emitPixelBins) tilePbxU32 += args[i].pbxVecs[t].len;
        }
        if (tilePixU32 > 0) {
            snprintf(pathBuf, sizeof(pathBuf), "%s_t%04d.pix", outPrefix, t);
            FILE *fout = fopen(pathBuf, "wb");
            if (!fout) {
                fprintf(stderr, "Cannot create %s\n", pathBuf);
                return 1;
            }
            for (int i = 0; i < threads; i++) {
                if (args[i].pixVecs[t].len > 0) {
                    fwrite(args[i].pixVecs[t].data, sizeof(uint32_t), args[i].pixVecs[t].len, fout);
                }
            }
            fclose(fout);
            tilesWithData++;
            totalEntries += (long)(tilePixU32 / 2u);
        }
        if (emitPixelBins && tilePbxU32 > 0) {
            snprintf(pathBuf, sizeof(pathBuf), "%s_t%04d.pbx", pixelBinPrefix, t);
            FILE *fb = fopen(pathBuf, "wb");
            if (!fb) {
                fprintf(stderr, "Cannot create %s\n", pathBuf);
                return 1;
            }
            for (int i = 0; i < threads; i++) {
                if (args[i].pbxVecs[t].len > 0) {
                    fwrite(args[i].pbxVecs[t].data, sizeof(uint32_t), args[i].pbxVecs[t].len, fb);
                }
            }
            fclose(fb);
        }
    }

    if (rootsDeduped > 0) {
        fprintf(stderr, "dedup: %ld unique, %ld skipped\n", rootsPlotted, rootsDeduped);
    }

    printf("{\"roots_plotted\":%ld,\"roots_clipped\":%ld,\"n_points\":%ld,"
           "\"degree\":%d,\"threads\":%d,\"color\":\"%s\",\"match\":\"%s\","
           "\"n_tiles\":%d,\"tiles_with_data\":%d,\"total_entries\":%ld",
           rootsPlotted, rootsClipped, nPoints, degree, threads, colorStr, matchStr,
           nTiles, tilesWithData, totalEntries);
    if (colorMode == COLOR_SOLVE_SCORE) {
        printf(",\"palette\":\"%s\",\"solve_score\":true,\"solve_metric\":\"%s\",\"solve_score_omega\":%.15g,\"solve_score_omega_enabled\":%s",
               palName, solve_metric_name(solveMetric), solveScoreOmega, solveScoreOmegaEnabled ? "true" : "false");
    } else if (colorMode == COLOR_SAVED_PALETTE) {
        printf(",\"palette\":\"%s\",\"saved_palette\":true", palName);
    } else if (colorMode == COLOR_CONSTANT) {
        printf(",\"constant_color\":\"%s\"", constColorStr);
    }
    printf("}\n");

    for (int i = 0; i < threads; i++) {
        for (int t = 0; t < nTiles; t++) {
            free(args[i].pixVecs[t].data);
            if (emitPixelBins) free(args[i].pbxVecs[t].data);
        }
        free(args[i].pixVecs);
        free(args[i].pbxVecs);
    }
    for (int t = 0; t < nTiles; t++) free(tileBits[t]);
    free(workers);
    free(args);
    free(solveBins);
    free(roots);
    return 0;
}
