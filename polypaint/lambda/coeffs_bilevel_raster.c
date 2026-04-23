/*
 * coeffs_bilevel_raster: project one logical coefficient section into a sparse
 * occupancy fragment.
 *
 * Reads a contiguous coeffs .bin slice (f32 complex coefficient pairs) and
 * writes one sparse u32le_u8_v1 fragment file:
 *
 *   uint32 little-endian global pixel index
 *   uint8 score byte (=1 for occupancy)
 *
 * No RGB. No color. No matching. Just coefficient occupancy.
 *
 * Usage:
 *   coeffs_bilevel_raster coeffs.bin out.frag
 *       --pix=N
 *       --min_re=A --max_re=B --min_im=C --max_im=D
 *       --n_coeffs=D [--rotation=R]
 *
 * Build:
 *   aarch64-linux-musl-gcc -O3 -static -o coeffs_bilevel_raster coeffs_bilevel_raster.c -lm
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: coeffs_bilevel_raster coeffs.bin out.frag [options]\n");
        return 1;
    }

    const char *binPath = argv[1];
    const char *outPath = argv[2];

    const char *widthArg = getArg(argc, argv, "--width");
    const char *heightArg = getArg(argc, argv, "--height");
    const char *tileSizeArg = getArg(argc, argv, "--tile_size");
    const char *nTileColsArg = getArg(argc, argv, "--n_tile_cols");
    const char *nTileRowsArg = getArg(argc, argv, "--n_tile_rows");
    const char *centerReArg = getArg(argc, argv, "--center_re");
    const char *centerImArg = getArg(argc, argv, "--center_im");
    const char *scaleArg = getArg(argc, argv, "--scale");
    const char *minReArg = getArg(argc, argv, "--min_re");
    const char *maxReArg = getArg(argc, argv, "--max_re");
    const char *minImArg = getArg(argc, argv, "--min_im");
    const char *maxImArg = getArg(argc, argv, "--max_im");

    int pix = getArgInt(argc, argv, "--pix", 0);
    int nCoeffs = getArgInt(argc, argv, "--n_coeffs", 0);
    double rotation = getArgDouble(argc, argv, "--rotation", 0.0);

    if (widthArg || heightArg) {
        fprintf(stderr, "coeffs_bilevel_raster no longer accepts --width or --height; pass --pix for square output\n");
        return 1;
    }
    if (tileSizeArg || nTileColsArg || nTileRowsArg) {
        fprintf(stderr, "coeffs_bilevel_raster no longer accepts tile args; it writes sparse global fragments\n");
        return 1;
    }
    if (centerReArg || centerImArg || scaleArg) {
        fprintf(stderr, "Legacy viewport args are no longer supported; pass --min_re, --max_re, --min_im, and --max_im\n");
        return 1;
    }
    if (pix <= 0 || nCoeffs <= 0) {
        fprintf(stderr, "pix and n_coeffs must be > 0\n");
        return 1;
    }
    if (!minReArg || !maxReArg || !minImArg || !maxImArg) {
        fprintf(stderr, "Exact viewport requires --min_re, --max_re, --min_im, and --max_im\n");
        return 1;
    }

    int W = pix;
    int H = pix;
    double minRe = atof(minReArg);
    double maxRe = atof(maxReArg);
    double minIm = atof(minImArg);
    double maxIm = atof(maxImArg);
    if (!(maxRe > minRe) || !(maxIm > minIm)) {
        fprintf(stderr, "Invalid exact viewport bounds\n");
        return 1;
    }
    size_t npix = (size_t)W * (size_t)H;
    if (npix > 0xFFFFFFFFu) {
        fprintf(stderr, "pix=%d exceeds u32 fragment encoding\n", pix);
        return 1;
    }

    double centerRe = (minRe + maxRe) / 2.0;
    double centerIm = (minIm + maxIm) / 2.0;
    double xScale = (double)W / (maxRe - minRe);
    double yScale = (double)H / (maxIm - minIm);
    double cosA = cos(rotation);
    double sinA = sin(rotation);

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

    int stride = nCoeffs * 2;
    size_t rowBytes = (size_t)stride * sizeof(float);
    if (rowBytes == 0 || ((size_t)fileSize % rowBytes) != 0) {
        fprintf(stderr, "Coeff file size %ld is not divisible by row bytes %zu\n", fileSize, rowBytes);
        fclose(fin);
        return 1;
    }
    long nPoints = (long)((size_t)fileSize / rowBytes);
    if (nPoints <= 0) {
        fprintf(stderr, "Empty coeff file\n");
        fclose(fin);
        return 1;
    }

    float *coeffs = (float *)malloc((size_t)fileSize);
    if (!coeffs) {
        fprintf(stderr, "Cannot allocate %ld bytes\n", fileSize);
        fclose(fin);
        return 1;
    }
    if (fread(coeffs, 1, (size_t)fileSize, fin) != (size_t)fileSize) {
        fprintf(stderr, "Short read from %s\n", binPath);
        free(coeffs);
        fclose(fin);
        return 1;
    }
    fclose(fin);

    size_t bitBytes = (npix + 7u) / 8u;
    uint8_t *bitset = (uint8_t *)calloc(1, bitBytes);
    if (!bitset) {
        fprintf(stderr, "Cannot allocate full-image bitset (%zu bytes)\n", bitBytes);
        free(coeffs);
        return 1;
    }

    uint32_t *pixelHits = NULL;
    size_t pixelHitCount = 0;
    size_t pixelHitCap = 0;
    long coeffsPlotted = 0;
    long coeffsClipped = 0;
    long coeffsDeduped = 0;

    for (long p = 0; p < nPoints; p++) {
        float *step = coeffs + p * stride;
        for (int i = 0; i < nCoeffs; i++) {
            double re = step[i * 2];
            double im = step[i * 2 + 1];
            double dx = re - centerRe;
            double dy = im - centerIm;
            double rotRe = centerRe + (dx * cosA - dy * sinA);
            double rotIm = centerIm + (dx * sinA + dy * cosA);
            double pxf = (rotRe - minRe) * xScale;
            double pyf = (maxIm - rotIm) * yScale;
            if (!isfinite(pxf) || !isfinite(pyf)) {
                coeffsClipped++;
                continue;
            }

            int px = (int)floor(pxf);
            int py = (int)floor(pyf);
            if (px < 0 || px >= W || py < 0 || py >= H) {
                coeffsClipped++;
                continue;
            }

            size_t bitIdx = (size_t)py * (size_t)W + (size_t)px;
            size_t byteIdx = bitIdx >> 3;
            uint8_t mask = (uint8_t)(1u << (bitIdx & 7u));
            if (bitset[byteIdx] & mask) {
                coeffsDeduped++;
                continue;
            }
            bitset[byteIdx] |= mask;

            if (pixelHitCount == pixelHitCap) {
                size_t nextCap = pixelHitCap ? pixelHitCap * 2u : 1024u;
                uint32_t *grown = (uint32_t *)realloc(pixelHits, nextCap * sizeof(uint32_t));
                if (!grown) {
                    fprintf(stderr, "Cannot grow sparse fragment hit buffer\n");
                    free(pixelHits);
                    free(bitset);
                    free(coeffs);
                    return 1;
                }
                pixelHits = grown;
                pixelHitCap = nextCap;
            }
            pixelHits[pixelHitCount++] = (uint32_t)bitIdx;
            coeffsPlotted++;
        }
    }

    FILE *fout = fopen(outPath, "wb");
    if (!fout) {
        fprintf(stderr, "Cannot create %s\n", outPath);
        free(pixelHits);
        free(bitset);
        free(coeffs);
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
            free(pixelHits);
            free(bitset);
            free(coeffs);
            return 1;
        }
    }
    if (fclose(fout) != 0) {
        fprintf(stderr, "Cannot close %s\n", outPath);
        free(pixelHits);
        free(bitset);
        free(coeffs);
        return 1;
    }

    printf("{\"coeffs_plotted\":%ld,\"coeffs_clipped\":%ld,\"coeffs_deduped\":%ld,"
           "\"pixels_set\":%zu,\"file_size\":%zu,\"n_points\":%ld}\n",
           coeffsPlotted, coeffsClipped, coeffsDeduped, pixelHitCount, pixelHitCount * 5u, nPoints);

    free(pixelHits);
    free(bitset);
    free(coeffs);
    return 0;
}
