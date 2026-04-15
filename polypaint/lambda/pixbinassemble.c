/*
 * pixbinassemble: assemble pixel-bin fragments into a dense uint8 tile.
 *
 * Sparse mode reads packed 8-byte entries from stdin:
 *   [local_pixel_idx:uint32, bin:uint32]
 * Dense layers mode reads concatenated dense uint8 tile buffers from stdin.
 * Every non-empty byte in each layer overwrites the output tile.
 *
 * Uses "last wins" overwrite to match pixassemble/final image semantics.
 * Outputs a dense headerless tile buffer:
 *   - 1 byte per pixel
 *   - 0..9 = winning palette bin
 *   - empty value (default 255) = background / no winner
 *
 * Build (static):
 *   aarch64-linux-musl-gcc -O3 -static -o pixbinassemble pixbinassemble.c -lm
 */

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

static int isArgValue(int argc, char **argv, const char *key, const char *expected) {
    const char *v = getArg(argc, argv, key);
    return v && strcmp(v, expected) == 0;
}

int main(int argc, char **argv) {
    int tileW = getArgInt(argc, argv, "--tile_w", 0);
    int tileH = getArgInt(argc, argv, "--tile_h", 0);
    int emptyValue = getArgInt(argc, argv, "--empty", 255);
    const char *outPath = getArg(argc, argv, "--output");
    int denseLayers = isArgValue(argc, argv, "--input_format", "dense_layers");

    if (tileW <= 0 || tileH <= 0 || !outPath) {
        fprintf(stderr,
                "Usage: pixbinassemble --tile_w=TW --tile_h=TH --output=tile.bin "
                "[--empty=255] [--input_format=sparse|dense_layers]\n");
        return 1;
    }
    if (emptyValue < 0 || emptyValue > 255) {
        fprintf(stderr, "Invalid --empty=%d\n", emptyValue);
        return 1;
    }

    size_t npix = (size_t)tileW * (size_t)tileH;
    uint8_t *tile = malloc(npix);
    if (!tile) {
        fprintf(stderr, "Cannot allocate tile buffer: %zu bytes\n", npix);
        return 1;
    }
    memset(tile, emptyValue, npix);

    #define READ_BUF_ENTRIES 65536
    uint32_t buf[READ_BUF_ENTRIES * 2];
    long entriesRead = 0;
    long layersRead = 0;

    if (denseLayers) {
        uint8_t *layer = malloc(npix);
        if (!layer) {
            fprintf(stderr, "Cannot allocate dense layer buffer: %zu bytes\n", npix);
            free(tile);
            return 1;
        }
        for (;;) {
            size_t total = 0;
            while (total < npix) {
                size_t got = fread(layer + total, 1, npix - total, stdin);
                if (got == 0) break;
                total += got;
            }
            if (total == 0) break;
            if (total != npix) {
                fprintf(stderr, "Truncated dense layer: got %zu bytes, expected %zu\n", total, npix);
                free(layer);
                free(tile);
                return 1;
            }
            for (size_t i = 0; i < npix; i++) {
                if (layer[i] != (uint8_t)emptyValue) {
                    tile[i] = layer[i];
                }
            }
            layersRead += 1;
        }
        free(layer);
    } else {
        for (;;) {
            size_t got = fread(buf, 8, READ_BUF_ENTRIES, stdin);
            if (got == 0) break;
            for (size_t i = 0; i < got; i++) {
                uint32_t pixIdx = buf[i * 2];
                uint32_t bin = buf[i * 2 + 1];
                if (pixIdx < npix) {
                    tile[pixIdx] = (bin <= 255u) ? (uint8_t)bin : (uint8_t)emptyValue;
                }
            }
            entriesRead += (long)got;
        }
    }

    FILE *fout = fopen(outPath, "wb");
    if (!fout) {
        fprintf(stderr, "Cannot create %s\n", outPath);
        free(tile);
        return 1;
    }
    fwrite(tile, 1, npix, fout);
    fclose(fout);
    free(tile);

    printf("{\"tile_w\":%d,\"tile_h\":%d,\"entries\":%ld,\"layers\":%ld,\"pixels\":%zu,\"empty\":%d,\"input_format\":\"%s\"}\n",
           tileW, tileH, entriesRead, layersRead, npix, emptyValue, denseLayers ? "dense_layers" : "sparse");
    return 0;
}
