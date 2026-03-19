/*
 * bilevel: project polynomial roots to a tile-local bitset and write 1-bit PNG.
 *
 * Reads one or more .bin stripe files (f32 root pairs), projects each root to
 * pixel coordinates, checks if it falls in the target tile, and sets a bit in
 * a tile-local bitset. After all stripes, converts the bitset to a 1-bit PNG
 * via libvips.
 *
 * No RGB data, no palettes, no matching. Just occupancy bits.
 *
 * Usage:
 *   bilevel --full_w=W --full_h=H --tile_col=C --tile_row=R
 *           --tile_w=TW --tile_h=TH --tile_size=TS
 *           --center_re=X --center_im=Y --scale=S --degree=D
 *           [--rotation=R] --output=tile.png
 *           stripe0.bin stripe1.bin ...
 *
 * Stripes are processed sequentially — only one is in memory at a time.
 *
 * Build (dynamic, needs libvips from Lambda layer):
 *   gcc -O3 -o bilevel bilevel.c \
 *     -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
 *     -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm \
 *     -Wl,-rpath,/opt/lib
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <vips/vips.h>

/* ---- Arg parsing (same pattern as roots2pix) ---- */

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

/* ---- Main ---- */

int main(int argc, char **argv) {
    if (VIPS_INIT(argv[0])) {
        fprintf(stderr, "vips_init failed\n");
        return 1;
    }

    int fullW = getArgInt(argc, argv, "--full_w", 4096);
    int fullH = getArgInt(argc, argv, "--full_h", 4096);
    int tileCol = getArgInt(argc, argv, "--tile_col", 0);
    int tileRow = getArgInt(argc, argv, "--tile_row", 0);
    int tileW = getArgInt(argc, argv, "--tile_w", 4096);
    int tileH = getArgInt(argc, argv, "--tile_h", 4096);
    int tileSize = getArgInt(argc, argv, "--tile_size", 4096);
    double centerRe = getArgDouble(argc, argv, "--center_re", 0.0);
    double centerIm = getArgDouble(argc, argv, "--center_im", 0.0);
    double scale = getArgDouble(argc, argv, "--scale", 100.0);
    double rotation = getArgDouble(argc, argv, "--rotation", 0.0);
    double cosA = cos(rotation), sinA = sin(rotation);
    int degree = getArgInt(argc, argv, "--degree", 25);
    const char *outPath = getArgStr(argc, argv, "--output", "/tmp/tile.png");

    /* Tile pixel offset in full image */
    int ox = tileCol * tileSize;
    int oy = tileRow * tileSize;

    /* Allocate bitset */
    size_t nPixels = (size_t)tileW * tileH;
    size_t bitsetBytes = (nPixels + 7) / 8;
    uint8_t *bitset = calloc(1, bitsetBytes);
    if (!bitset) {
        fprintf(stderr, "Cannot allocate bitset (%zu bytes)\n", bitsetBytes);
        return 1;
    }

    double halfW = fullW / 2.0;
    double halfH = fullH / 2.0;
    int stride = degree * 2;
    long totalPlotted = 0, totalClipped = 0, totalDeduped = 0;
    int stripesProcessed = 0;

    /* Collect .bin file paths (non --flag args) */
    const char *binPaths[8192];
    int nBins = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-')
            binPaths[nBins++] = argv[i];
    }

    /* Process each stripe .bin sequentially */
    for (int b = 0; b < nBins; b++) {
        FILE *fin = fopen(binPaths[b], "rb");
        if (!fin) {
            fprintf(stderr, "Cannot open %s\n", binPaths[b]);
            continue;
        }
        fseek(fin, 0, SEEK_END);
        long fileSize = ftell(fin);
        fseek(fin, 0, SEEK_SET);

        long nPoints = fileSize / (stride * sizeof(float));
        if (nPoints <= 0) { fclose(fin); continue; }

        float *roots = malloc(fileSize);
        if (!roots) {
            fprintf(stderr, "Cannot allocate %ld bytes for %s\n", fileSize, binPaths[b]);
            fclose(fin);
            continue;
        }
        fread(roots, 1, fileSize, fin);
        fclose(fin);

        /* Project roots, set bits for those in our tile */
        for (long p = 0; p < nPoints; p++) {
            float *step = roots + p * stride;
            for (int i = 0; i < degree; i++) {
                double re = step[i * 2], im = step[i * 2 + 1];

                /* Viewport transform with rotation */
                double dx = re - centerRe, dy = im - centerIm;
                double rx = dx * cosA - dy * sinA;
                double ry = dx * sinA + dy * cosA;
                int px = (int)(halfW + rx * scale);
                int py = (int)(halfH - ry * scale);

                /* Check if pixel falls in our tile */
                if (px < ox || px >= ox + tileW || py < oy || py >= oy + tileH) {
                    totalClipped++;
                    continue;
                }

                /* Tile-local coordinates */
                int lx = px - ox;
                int ly = py - oy;
                size_t bitIdx = (size_t)ly * tileW + lx;

                /* Dedup: skip if already set */
                size_t byteIdx = bitIdx >> 3;
                uint8_t mask = 1u << (bitIdx & 7);
                if (bitset[byteIdx] & mask) {
                    totalDeduped++;
                    continue;
                }
                bitset[byteIdx] |= mask;
                totalPlotted++;
            }
        }

        free(roots);
        stripesProcessed++;
    }

    /* Convert bitset to uchar image buffer (0 or 255) */
    unsigned char *imgBuf = malloc(nPixels);
    if (!imgBuf) {
        fprintf(stderr, "Cannot allocate image buffer (%zu bytes)\n", nPixels);
        free(bitset);
        vips_shutdown();
        return 1;
    }
    for (size_t i = 0; i < nPixels; i++) {
        imgBuf[i] = (bitset[i >> 3] & (1u << (i & 7))) ? 255 : 0;
    }
    free(bitset);

    /* Create libvips image from buffer */
    VipsImage *img = vips_image_new_from_memory(imgBuf, nPixels, tileW, tileH, 1, VIPS_FORMAT_UCHAR);
    if (!img) {
        fprintf(stderr, "vips_image_new_from_memory failed: %s\n", vips_error_buffer());
        free(imgBuf);
        vips_shutdown();
        return 1;
    }

    /* Threshold > 0 to get boolean for 1-bit PNG */
    VipsImage *thresh;
    if (vips_more_const1(img, &thresh, 0, NULL)) {
        fprintf(stderr, "vips_more_const1 failed: %s\n", vips_error_buffer());
        g_object_unref(img);
        free(imgBuf);
        vips_shutdown();
        return 1;
    }

    /* Write 1-bit PNG */
    if (vips_pngsave(thresh, outPath, "compression", 9, "bitdepth", 1, NULL)) {
        fprintf(stderr, "vips_pngsave failed: %s\n", vips_error_buffer());
        g_object_unref(thresh);
        g_object_unref(img);
        free(imgBuf);
        vips_shutdown();
        return 1;
    }

    g_object_unref(thresh);
    g_object_unref(img);
    free(imgBuf);

    /* Report file size */
    FILE *f = fopen(outPath, "rb");
    long fsize = 0;
    if (f) { fseek(f, 0, SEEK_END); fsize = ftell(f); fclose(f); }

    /* JSON metadata to stdout */
    printf("{\"tile_col\":%d,\"tile_row\":%d,\"tile_w\":%d,\"tile_h\":%d,"
           "\"pixels_set\":%ld,\"pixels_clipped\":%ld,\"pixels_deduped\":%ld,"
           "\"stripes\":%d,\"file_size\":%ld}\n",
           tileCol, tileRow, tileW, tileH,
           totalPlotted, totalClipped, totalDeduped,
           stripesProcessed, fsize);

    vips_shutdown();
    return 0;
}
