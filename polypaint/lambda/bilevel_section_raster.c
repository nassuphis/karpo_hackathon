/*
 * bilevel_section_raster: project one logical solve section into a sparse
 * occupancy fragment.
 *
 * Reads a .bin section file (f32 root pairs), projects each root to pixel
 * coordinates, clips to the viewport, dedups on the full image grid, and writes
 * one sparse u32le_u8_v1 fragment file where each pair is:
 *   little-endian uint32 pixel_idx
 *   uint8 score byte (=1 for bilevel occupancy)
 *
 * Usage:
 *   bilevel_section_raster section.bin out.frag
 *       --width=W --height=H
 *       --min_re=A --max_re=B --min_im=C --max_im=D --degree=D
 *       [--rotation=R] [--root_xforms=chain.json]
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "root_xforms.h"

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

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: bilevel_section_raster section.bin out.frag [options]\n");
        return 1;
    }

    const char *binPath = argv[1];
    const char *outPath = argv[2];
    int W = getArgInt(argc, argv, "--width", 0);
    int H = getArgInt(argc, argv, "--height", 0);
    const char *minReArg = getArg(argc, argv, "--min_re");
    const char *maxReArg = getArg(argc, argv, "--max_re");
    const char *minImArg = getArg(argc, argv, "--min_im");
    const char *maxImArg = getArg(argc, argv, "--max_im");
    const char *centerReArg = getArg(argc, argv, "--center_re");
    const char *centerImArg = getArg(argc, argv, "--center_im");
    const char *scaleArg = getArg(argc, argv, "--scale");
    double rotation = getArgDouble(argc, argv, "--rotation", 0.0);
    double cosA = cos(rotation), sinA = sin(rotation);
    int degree = getArgInt(argc, argv, "--degree", 0);
    const char *rtPath = getArgStr(argc, argv, "--root_xforms", NULL);

    if (W <= 0 || H <= 0 || degree <= 0) {
        fprintf(stderr, "width, height, and degree must be > 0\n");
        return 1;
    }
    double minRe = 0.0, maxRe = 0.0, minIm = 0.0, maxIm = 0.0;
    if (minReArg || maxReArg || minImArg || maxImArg) {
        if (centerReArg || centerImArg || scaleArg) {
            fprintf(stderr, "Do not mix exact viewport bounds with legacy center/scale args\n");
            return 1;
        }
        if (!minReArg || !maxReArg || !minImArg || !maxImArg) {
            fprintf(stderr, "Exact viewport requires --min_re, --max_re, --min_im, and --max_im together\n");
            return 1;
        }
        minRe = atof(minReArg);
        maxRe = atof(maxReArg);
        minIm = atof(minImArg);
        maxIm = atof(maxImArg);
        if (!(maxRe > minRe) || !(maxIm > minIm)) {
            fprintf(stderr, "Invalid exact viewport bounds\n");
            return 1;
        }
    } else if (centerReArg || centerImArg || scaleArg) {
        if (!centerReArg || !centerImArg || !scaleArg) {
            fprintf(stderr, "Legacy viewport requires --center_re, --center_im, and --scale together\n");
            return 1;
        }
        double centerRe = atof(centerReArg);
        double centerIm = atof(centerImArg);
        double scale = atof(scaleArg);
        if (!(scale > 0.0)) {
            fprintf(stderr, "Invalid scale\n");
            return 1;
        }
        minRe = centerRe - ((double)W / 2.0) / scale;
        maxRe = centerRe + ((double)W / 2.0) / scale;
        minIm = centerIm - ((double)H / 2.0) / scale;
        maxIm = centerIm + ((double)H / 2.0) / scale;
    } else {
        fprintf(stderr, "Viewport requires exact bounds or legacy center/scale args\n");
        return 1;
    }
    double centerRe = (minRe + maxRe) / 2.0;
    double centerIm = (minIm + maxIm) / 2.0;
    double xScale = (double)W / (maxRe - minRe);
    double yScale = (double)H / (maxIm - minIm);

    RootXformEntry rtChain[MAX_RT_CHAIN];
    int nRt = parse_root_xform_file(rtPath, rtChain, MAX_RT_CHAIN);

    FILE *fin = fopen(binPath, "rb");
    if (!fin) {
        fprintf(stderr, "Cannot open %s\n", binPath);
        return 1;
    }
    if (fseek(fin, 0, SEEK_END) != 0) {
        fprintf(stderr, "Cannot seek %s\n", binPath);
        fclose(fin);
        return 1;
    }
    long fileSize = ftell(fin);
    if (fileSize < 0) {
        fprintf(stderr, "Cannot stat %s\n", binPath);
        fclose(fin);
        return 1;
    }
    if (fseek(fin, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Cannot rewind %s\n", binPath);
        fclose(fin);
        return 1;
    }

    int stride = degree * 2;
    long solveBytes = (long)stride * (long)sizeof(float);
    if (solveBytes <= 0 || (fileSize % solveBytes) != 0) {
        fprintf(stderr, "Input size %ld is not divisible by solve row bytes %ld\n", fileSize, solveBytes);
        fclose(fin);
        return 1;
    }

    long nPoints = fileSize / solveBytes;
    float *roots = NULL;
    if (fileSize > 0) {
        roots = (float *)malloc((size_t)fileSize);
        if (!roots) {
            fprintf(stderr, "Cannot allocate %ld bytes for roots\n", fileSize);
            fclose(fin);
            return 1;
        }
        if ((long)fread(roots, 1, (size_t)fileSize, fin) != fileSize) {
            fprintf(stderr, "Short read from %s\n", binPath);
            free(roots);
            fclose(fin);
            return 1;
        }
    }
    fclose(fin);

    size_t nPixels = (size_t)W * (size_t)H;
    size_t bitsetBytes = (nPixels + 7u) / 8u;
    uint8_t *bitset = (uint8_t *)calloc(1, bitsetBytes > 0 ? bitsetBytes : 1u);
    if (!bitset) {
        fprintf(stderr, "Cannot allocate %zu-byte bitset\n", bitsetBytes);
        free(roots);
        return 1;
    }
    uint32_t *pixelHits = NULL;
    size_t pixelHitCount = 0;
    size_t pixelHitCap = 0;

    float *wkRe = NULL;
    float *wkIm = NULL;
    if (nRt > 0) {
        wkRe = (float *)malloc((size_t)degree * sizeof(float));
        wkIm = (float *)malloc((size_t)degree * sizeof(float));
        if (!wkRe || !wkIm) {
            fprintf(stderr, "Cannot allocate root-transform working buffers\n");
            free(wkRe);
            free(wkIm);
            free(bitset);
            free(roots);
            return 1;
        }
    }

    long rootsPlotted = 0;
    long rootsClipped = 0;
    long rootsDeduped = 0;

    for (long p = 0; p < nPoints; p++) {
        float *step = roots + p * stride;
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

            double dx = re - centerRe;
            double dy = im - centerIm;
            double rotRe = centerRe + (dx * cosA - dy * sinA);
            double rotIm = centerIm + (dx * sinA + dy * cosA);
            double pxf = (rotRe - minRe) * xScale;
            double pyf = (maxIm - rotIm) * yScale;
            if (!isfinite(pxf) || !isfinite(pyf)) {
                rootsClipped++;
                continue;
            }

            int px = (int)floor(pxf);
            int py = (int)floor(pyf);
            if (px < 0 || px >= W || py < 0 || py >= H) {
                rootsClipped++;
                continue;
            }

            size_t bitIdx = (size_t)py * (size_t)W + (size_t)px;
            if (bitIdx > 0xFFFFFFFFu) {
                fprintf(stderr, "pixel index %zu exceeds u32 fragment encoding\n", bitIdx);
                free(wkRe);
                free(wkIm);
                free(bitset);
                free(pixelHits);
                free(roots);
                return 1;
            }
            size_t byteIdx = bitIdx >> 3;
            uint8_t mask = (uint8_t)(1u << (bitIdx & 7u));
            if (bitset[byteIdx] & mask) {
                rootsDeduped++;
                continue;
            }
            bitset[byteIdx] |= mask;
            if (pixelHitCount == pixelHitCap) {
                size_t nextCap = pixelHitCap ? pixelHitCap * 2u : 1024u;
                uint32_t *grown = (uint32_t *)realloc(pixelHits, nextCap * sizeof(uint32_t));
                if (!grown) {
                    fprintf(stderr, "Cannot grow sparse fragment hit buffer\n");
                    free(wkRe);
                    free(wkIm);
                    free(bitset);
                    free(pixelHits);
                    free(roots);
                    return 1;
                }
                pixelHits = grown;
                pixelHitCap = nextCap;
            }
            pixelHits[pixelHitCount++] = (uint32_t)bitIdx;
            rootsPlotted++;
        }
    }

    FILE *fout = fopen(outPath, "wb");
    if (!fout) {
        fprintf(stderr, "Cannot create %s\n", outPath);
        free(wkRe);
        free(wkIm);
        free(bitset);
        free(pixelHits);
        free(roots);
        return 1;
    }
    for (size_t i = 0; i < pixelHitCount; i++) {
        uint32_t pixelIdx = pixelHits[i];
        uint8_t rec[5];
        rec[0] = (uint8_t)(pixelIdx & 0xFFu);
        rec[1] = (uint8_t)((pixelIdx >> 8) & 0xFFu);
        rec[2] = (uint8_t)((pixelIdx >> 16) & 0xFFu);
        rec[3] = (uint8_t)((pixelIdx >> 24) & 0xFFu);
        rec[4] = 1u;
        if (fwrite(rec, 1, sizeof(rec), fout) != sizeof(rec)) {
            fprintf(stderr, "Short write to %s\n", outPath);
            fclose(fout);
            free(wkRe);
            free(wkIm);
            free(bitset);
            free(pixelHits);
            free(roots);
            return 1;
        }
    }
    fclose(fout);

    printf(
        "{\"roots_plotted\":%ld,\"roots_clipped\":%ld,\"roots_deduped\":%ld,"
        "\"n_points\":%ld,\"width\":%d,\"height\":%d,\"file_size\":%zu}\n",
        rootsPlotted,
        rootsClipped,
        rootsDeduped,
        nPoints,
        W,
        H,
        pixelHitCount * 5u
    );

    free(wkRe);
    free(wkIm);
    free(bitset);
    free(pixelHits);
    free(roots);
    return 0;
}
