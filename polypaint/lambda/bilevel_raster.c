/*
 * bilevel_raster: project one stripe's roots into per-tile bitset files.
 *
 * Reads a .bin stripe file (f32 root pairs), projects each root to pixel
 * coordinates, determines which tile it belongs to, sets a bit in that
 * tile's bitset. Writes non-empty tile bitsets to output files.
 *
 * No RGB. No color. No matching. Just occupancy bits.
 * One stripe in, many tile bitset files out.
 *
 * Usage:
 *   bilevel_raster stripe.bin /tmp/bits
 *       --width=W --height=H --tile_size=TS
 *       --n_tile_cols=C --n_tile_rows=R
 *       --center_re=X --center_im=Y --scale=S --degree=D
 *       [--rotation=R]
 *
 * Output: {outPrefix}_t0000.bits ... (only non-empty tiles)
 * Each .bits file: raw packed bitset, ceil(tile_w * tile_h / 8) bytes.
 *
 * Build (static):
 *   aarch64-linux-musl-gcc -O3 -static -o bilevel_raster bilevel_raster.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "root_xforms.h"

#define MAX_TILES 4096

/* ---- Arg parsing ---- */

static const char *getArg(int argc, char **argv, const char *key) {
    int klen = strlen(key);
    for (int i = 1; i < argc; i++)
        if (strncmp(argv[i], key, klen) == 0 && argv[i][klen] == '=')
            return argv[i] + klen + 1;
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

/* ---- Main ---- */

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: bilevel_raster stripe.bin outPrefix [options]\n");
        return 1;
    }
    const char *binPath = argv[1];
    const char *outPrefix = argv[2];

    int W = getArgInt(argc, argv, "--width", 4096);
    int H = getArgInt(argc, argv, "--height", 4096);
    int tileSize = getArgInt(argc, argv, "--tile_size", 4096);
    int nTileCols = getArgInt(argc, argv, "--n_tile_cols", 1);
    int nTileRows = getArgInt(argc, argv, "--n_tile_rows", 1);
    double centerRe = getArgDouble(argc, argv, "--center_re", 0.0);
    double centerIm = getArgDouble(argc, argv, "--center_im", 0.0);
    double scale = getArgDouble(argc, argv, "--scale", 100.0);
    double rotation = getArgDouble(argc, argv, "--rotation", 0.0);
    double cosA = cos(rotation), sinA = sin(rotation);
    int degree = getArgInt(argc, argv, "--degree", 25);
    const char *rtPath = getArgStr(argc, argv, "--root_xforms", NULL);

    /* Parse root transform chain */
    RootXformEntry rtChain[MAX_RT_CHAIN];
    int nRt = parse_root_xform_file(rtPath, rtChain, MAX_RT_CHAIN);

    int nTiles = nTileCols * nTileRows;
    if (nTiles > MAX_TILES) {
        fprintf(stderr, "Too many tiles: %d > %d\n", nTiles, MAX_TILES);
        return 1;
    }

    /* Compute per-tile dimensions (edge tiles may be smaller) */
    int tileW[MAX_TILES], tileH[MAX_TILES];
    for (int t = 0; t < nTiles; t++) {
        int tc = t % nTileCols;
        int tr = t / nTileCols;
        tileW[t] = (tc < nTileCols - 1) ? tileSize : W - tc * tileSize;
        tileH[t] = (tr < nTileRows - 1) ? tileSize : H - tr * tileSize;
    }

    /* Allocate per-tile bitsets */
    uint8_t *tileBits[MAX_TILES];
    size_t tileBitBytes[MAX_TILES];
    for (int t = 0; t < nTiles; t++) {
        tileBitBytes[t] = ((size_t)tileW[t] * tileH[t] + 7) / 8;
        tileBits[t] = calloc(1, tileBitBytes[t]);
        if (!tileBits[t]) {
            fprintf(stderr, "Cannot allocate bitset for tile %d (%zu bytes)\n", t, tileBitBytes[t]);
            return 1;
        }
    }

    /* Read stripe .bin */
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

    /* Project roots into tile bitsets */
    double halfW = W / 2.0, halfH = H / 2.0;
    long rootsPlotted = 0, rootsClipped = 0, rootsDeduped = 0;

    /* Working buffers for root transforms (deinterleaved re/im) */
    float *wkRe = NULL, *wkIm = NULL;
    if (nRt > 0) {
        wkRe = malloc(degree * sizeof(float));
        wkIm = malloc(degree * sizeof(float));
    }

    for (long p = 0; p < nPoints; p++) {
        float *step = roots + p * stride;

        /* Apply root transforms if any */
        if (nRt > 0) {
            for (int i = 0; i < degree; i++) {
                wkRe[i] = step[i * 2];
                wkIm[i] = step[i * 2 + 1];
            }
            apply_root_xforms(rtChain, nRt, wkRe, wkIm, degree);
        }

        for (int i = 0; i < degree; i++) {
            double re = nRt > 0 ? wkRe[i] : step[i * 2];
            double im = nRt > 0 ? wkIm[i] : step[i * 2 + 1];

            /* Viewport transform with rotation */
            double dx = re - centerRe, dy = im - centerIm;
            double rx = dx * cosA - dy * sinA;
            double ry = dx * sinA + dy * cosA;
            int px = (int)(halfW + rx * scale);
            int py = (int)(halfH - ry * scale);

            if (px < 0 || px >= W || py < 0 || py >= H) {
                rootsClipped++;
                continue;
            }

            /* Determine tile */
            int tc = px / tileSize;
            int tr = py / tileSize;
            int tileId = tr * nTileCols + tc;
            if (tileId < 0 || tileId >= nTiles) { rootsClipped++; continue; }

            /* Tile-local coordinates */
            int lx = px - tc * tileSize;
            int ly = py - tr * tileSize;
            size_t bitIdx = (size_t)ly * tileW[tileId] + lx;

            /* Dedup */
            size_t byteIdx = bitIdx >> 3;
            uint8_t mask = 1u << (bitIdx & 7);
            if (tileBits[tileId][byteIdx] & mask) {
                rootsDeduped++;
                continue;
            }
            tileBits[tileId][byteIdx] |= mask;
            rootsPlotted++;
        }
    }
    free(roots);
    if (wkRe) free(wkRe);
    if (wkIm) free(wkIm);

    /* Write non-empty tile bitsets */
    int tilesWritten = 0;
    for (int t = 0; t < nTiles; t++) {
        /* Check if any bits set */
        int empty = 1;
        for (size_t b = 0; b < tileBitBytes[t]; b++) {
            if (tileBits[t][b]) { empty = 0; break; }
        }
        if (empty) { free(tileBits[t]); continue; }

        char path[512];
        snprintf(path, sizeof(path), "%s_t%04d.bits", outPrefix, t);
        FILE *fout = fopen(path, "wb");
        if (fout) {
            fwrite(tileBits[t], 1, tileBitBytes[t], fout);
            fclose(fout);
            tilesWritten++;
        }
        free(tileBits[t]);
    }

    printf("{\"roots_plotted\":%ld,\"roots_clipped\":%ld,\"roots_deduped\":%ld,"
           "\"tiles_written\":%d,\"n_points\":%ld}\n",
           rootsPlotted, rootsClipped, rootsDeduped, tilesWritten, nPoints);

    return 0;
}
