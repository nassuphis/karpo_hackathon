/*
 * pixel_bins_render: render one tile raw image from a dense uint8 pixel-bin map.
 *
 * Usage:
 *   pixel_bins_render tile_bins.bin tile.raw --tile_w=TW --tile_h=TH
 *       --palette=<name> [--background_color=RRGGBB] [--empty=255]
 *
 * Input:
 *   - exactly TW*TH bytes
 *   - each byte is:
 *       0..9   final visible palette bin at that pixel
 *       empty  background / no plotted root (default 255)
 *
 * Output:
 *   - raw2jpeg-compatible 12-byte header [width,height,bands=3]
 *   - followed by RGB pixel data
 *
 * Build:
 *   aarch64-linux-musl-gcc -O3 -static -o pixel_bins_render pixel_bins_render.c -lm
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "palette_lut.h"

static const char *getArg(int argc, char **argv, const char *key) {
    size_t klen = strlen(key);
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

static const char *getArgStr(int argc, char **argv, const char *key, const char *def) {
    const char *v = getArg(argc, argv, key);
    return v ? v : def;
}

static int parseHexColor(const char *s, unsigned char *r, unsigned char *g, unsigned char *b) {
    if (!s) return 0;
    if (*s == '#') s++;
    if (strlen(s) != 6) return 0;
    unsigned int rv = 0, gv = 0, bv = 0;
    if (sscanf(s, "%2x%2x%2x", &rv, &gv, &bv) != 3) return 0;
    *r = (unsigned char)rv;
    *g = (unsigned char)gv;
    *b = (unsigned char)bv;
    return 1;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr,
                "Usage: pixel_bins_render tile_bins.bin tile.raw --tile_w=TW --tile_h=TH "
                "--palette=<name> [--background_color=RRGGBB] [--empty=255]\n");
        return 1;
    }

    const char *inPath = argv[1];
    const char *outPath = argv[2];
    int tileW = getArgInt(argc, argv, "--tile_w", 0);
    int tileH = getArgInt(argc, argv, "--tile_h", 0);
    int emptyValue = getArgInt(argc, argv, "--empty", 255);
    const char *palName = getArgStr(argc, argv, "--palette", "inferno");
    const char *bgColor = getArgStr(argc, argv, "--background_color", "000000");

    if (tileW <= 0 || tileH <= 0) {
        fprintf(stderr, "Invalid tile size %dx%d\n", tileW, tileH);
        return 1;
    }
    if (emptyValue < 0 || emptyValue > 255) {
        fprintf(stderr, "Invalid --empty=%d\n", emptyValue);
        return 1;
    }

    const PaletteDef *pal = findPalette(palName);
    if (!pal) {
        fprintf(stderr, "Invalid palette: %s\n", palName);
        return 1;
    }

    unsigned char bgR = 0, bgG = 0, bgB = 0;
    if (!parseHexColor(bgColor, &bgR, &bgG, &bgB)) {
        fprintf(stderr, "Invalid background_color: %s\n", bgColor);
        return 1;
    }

    size_t npix = (size_t)tileW * (size_t)tileH;
    FILE *fin = fopen(inPath, "rb");
    if (!fin) {
        fprintf(stderr, "Cannot open %s\n", inPath);
        return 1;
    }
    fseek(fin, 0, SEEK_END);
    long fileSize = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    if ((size_t)fileSize != npix) {
        fprintf(stderr, "Bin tile size mismatch: got %ld expected %zu\n", fileSize, npix);
        fclose(fin);
        return 1;
    }

    uint8_t *bins = malloc(npix);
    if (!bins) {
        fprintf(stderr, "Out of memory allocating %zu bytes\n", npix);
        fclose(fin);
        return 1;
    }
    if (fread(bins, 1, npix, fin) != npix) {
        fprintf(stderr, "Short read\n");
        free(bins);
        fclose(fin);
        return 1;
    }
    fclose(fin);

    FILE *fout = fopen(outPath, "wb");
    if (!fout) {
        fprintf(stderr, "Cannot create %s\n", outPath);
        free(bins);
        return 1;
    }

    uint32_t header[3] = { (uint32_t)tileW, (uint32_t)tileH, 3 };
    fwrite(header, sizeof(uint32_t), 3, fout);

    unsigned char rgb[3];
    long colored = 0;
    for (size_t i = 0; i < npix; i++) {
        uint8_t bin = bins[i];
        if (bin == (uint8_t)emptyValue) {
            rgb[0] = bgR;
            rgb[1] = bgG;
            rgb[2] = bgB;
        } else {
            if (bin > 9) bin = 9;
            double t = ((double)bin + 0.5) / 10.0;
            paletteRGB(pal, t, &rgb[0], &rgb[1], &rgb[2]);
            colored++;
        }
        fwrite(rgb, 1, 3, fout);
    }

    fclose(fout);
    free(bins);
    printf("{\"tile_w\":%d,\"tile_h\":%d,\"palette\":\"%s\",\"colored_pixels\":%ld}\n",
           tileW, tileH, palName, colored);
    return 0;
}
