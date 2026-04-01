/*
 * palette_bins_render — render an RGB raw image from a full-resolution palette bin grid.
 *
 * Usage:
 *   palette_bins_render bins.bin output.raw --n=N --palette=<name>
 *
 * bins.bin format:
 *   - exactly N*N bytes
 *   - each byte is a bin index 0..9
 *
 * output.raw format:
 *   - raw2jpeg-compatible 12-byte header [width,height,bands] as uint32
 *   - followed by RGB bytes
 *
 * Build:
 *   aarch64-linux-musl-gcc -O3 -static -o palette_bins_render palette_bins_render.c -lm
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

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: palette_bins_render bins.bin output.raw --n=N --palette=name\n");
        return 1;
    }

    const char *inPath = argv[1];
    const char *outPath = argv[2];
    int n = getArgInt(argc, argv, "--n", 0);
    const char *palName = getArgStr(argc, argv, "--palette", "inferno");

    if (n < 1) {
        fprintf(stderr, "Invalid --n=%d\n", n);
        return 1;
    }

    int found = 0;
    for (int i = 0; PALETTES[i].name; i++) {
        if (strcmp(PALETTES[i].name, palName) == 0) {
            found = 1;
            break;
        }
    }
    if (!found) {
        fprintf(stderr, "Invalid palette: %s\n", palName);
        return 1;
    }
    const PaletteDef *pal = findPalette(palName);

    FILE *fin = fopen(inPath, "rb");
    if (!fin) {
        fprintf(stderr, "Cannot open %s\n", inPath);
        return 1;
    }
    fseek(fin, 0, SEEK_END);
    long fileSize = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    long expected = (long)n * (long)n;
    if (fileSize != expected) {
        fprintf(stderr, "Bin file size mismatch: got %ld expected %ld\n", fileSize, expected);
        fclose(fin);
        return 1;
    }

    uint8_t *bins = malloc(expected);
    if (!bins) {
        fprintf(stderr, "Out of memory\n");
        fclose(fin);
        return 1;
    }
    if ((long)fread(bins, 1, expected, fin) != expected) {
        fprintf(stderr, "Short read\n");
        free(bins);
        fclose(fin);
        return 1;
    }
    fclose(fin);

    FILE *fout = fopen(outPath, "wb");
    if (!fout) {
        fprintf(stderr, "Cannot open %s\n", outPath);
        free(bins);
        return 1;
    }

    uint32_t hdr[3] = { (uint32_t)n, (uint32_t)n, 3 };
    fwrite(hdr, sizeof(uint32_t), 3, fout);

    unsigned char rgb[3];
    for (long i = 0; i < expected; i++) {
        uint8_t bin = bins[i];
        if (bin > 9) bin = 9;
        double t = ((double)bin + 0.5) / 10.0;
        paletteRGB(pal, t, &rgb[0], &rgb[1], &rgb[2]);
        fwrite(rgb, 1, 3, fout);
    }

    fclose(fout);
    free(bins);
    printf("{\"mode\":\"palette_bins_render\",\"n\":%d,\"palette\":\"%s\"}\n", n, palName);
    return 0;
}
