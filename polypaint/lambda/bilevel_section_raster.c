/*
 * bilevel_section_raster: project one logical solve section into a single
 * full-frame packed bitset.
 *
 * Reads a .bin section file (f32 root pairs), projects each root to pixel
 * coordinates, clips to the viewport, dedups on the full image grid, and writes
 * one packed row-major bitset file.
 *
 * Usage:
 *   bilevel_section_raster section.bin out.bits
 *       --width=W --height=H
 *       --center_re=X --center_im=Y --scale=S --degree=D
 *       [--rotation=R] [--root_xforms=chain.json]
 *
 * Output bits use the same local packing convention as the old tile .bits
 * files: bit i lives at byte (i >> 3), mask (1 << (i & 7)).
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
        fprintf(stderr, "Usage: bilevel_section_raster section.bin out.bits [options]\n");
        return 1;
    }

    const char *binPath = argv[1];
    const char *outPath = argv[2];
    int W = getArgInt(argc, argv, "--width", 0);
    int H = getArgInt(argc, argv, "--height", 0);
    double centerRe = getArgDouble(argc, argv, "--center_re", 0.0);
    double centerIm = getArgDouble(argc, argv, "--center_im", 0.0);
    double scale = getArgDouble(argc, argv, "--scale", 100.0);
    double rotation = getArgDouble(argc, argv, "--rotation", 0.0);
    double cosA = cos(rotation), sinA = sin(rotation);
    int degree = getArgInt(argc, argv, "--degree", 0);
    const char *rtPath = getArgStr(argc, argv, "--root_xforms", NULL);

    if (W <= 0 || H <= 0 || degree <= 0) {
        fprintf(stderr, "width, height, and degree must be > 0\n");
        return 1;
    }

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

    double halfW = W / 2.0;
    double halfH = H / 2.0;
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
            double rx = dx * cosA - dy * sinA;
            double ry = dx * sinA + dy * cosA;
            double pxf = halfW + rx * scale;
            double pyf = halfH - ry * scale;
            if (!isfinite(pxf) || !isfinite(pyf)) {
                rootsClipped++;
                continue;
            }

            int px = (int)pxf;
            int py = (int)pyf;
            if (px < 0 || px >= W || py < 0 || py >= H) {
                rootsClipped++;
                continue;
            }

            size_t bitIdx = (size_t)py * (size_t)W + (size_t)px;
            size_t byteIdx = bitIdx >> 3;
            uint8_t mask = (uint8_t)(1u << (bitIdx & 7u));
            if (bitset[byteIdx] & mask) {
                rootsDeduped++;
                continue;
            }
            bitset[byteIdx] |= mask;
            rootsPlotted++;
        }
    }

    FILE *fout = fopen(outPath, "wb");
    if (!fout) {
        fprintf(stderr, "Cannot create %s\n", outPath);
        free(wkRe);
        free(wkIm);
        free(bitset);
        free(roots);
        return 1;
    }
    if (bitsetBytes > 0 && fwrite(bitset, 1, bitsetBytes, fout) != bitsetBytes) {
        fprintf(stderr, "Short write to %s\n", outPath);
        fclose(fout);
        free(wkRe);
        free(wkIm);
        free(bitset);
        free(roots);
        return 1;
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
        bitsetBytes
    );

    free(wkRe);
    free(wkIm);
    free(bitset);
    free(roots);
    return 0;
}
