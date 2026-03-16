/*
 * pixassemble: assemble tile-bucketed .pix entries into a .raw tile image.
 *
 * Reads packed 8-byte entries from stdin: [local_pixel_idx:uint32, rgb:uint32]
 * Uses "last wins" overwrite — the last entry for a given pixel determines its color.
 * Outputs a .raw file (12-byte header: uint32 W, H, bands=3 + pixel data).
 *
 * Usage:
 *   pixassemble --tile_w=TW --tile_h=TH --output=tile.raw
 *   (reads from stdin)
 *
 * Build (static):
 *   aarch64-linux-musl-gcc -O3 -static -o pixassemble pixassemble.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

int main(int argc, char **argv) {
    int TW = getArgInt(argc, argv, "--tile_w", 0);
    int TH = getArgInt(argc, argv, "--tile_h", 0);
    const char *outPath = getArg(argc, argv, "--output");

    if (TW <= 0 || TH <= 0 || !outPath) {
        fprintf(stderr, "Usage: pixassemble --tile_w=TW --tile_h=TH --output=tile.raw\n"
                        "  Reads packed 8-byte entries [pixel_idx:u32, rgb:u32] from stdin.\n");
        return 1;
    }

    size_t npix = (size_t)TW * TH;

    /* Allocate tile buffer — packed RGB as uint32, 0 = black background */
    uint32_t *tile = calloc(npix, sizeof(uint32_t));
    if (!tile) {
        fprintf(stderr, "Cannot allocate tile buffer: %zu MB\n",
                npix * sizeof(uint32_t) / (1024 * 1024));
        return 1;
    }

    /* Read entries from stdin in bulk — last wins overwrite */
    #define READ_BUF_ENTRIES 65536
    uint32_t buf[READ_BUF_ENTRIES * 2];
    long entries_read = 0;

    for (;;) {
        size_t got = fread(buf, 8, READ_BUF_ENTRIES, stdin);
        if (got == 0) break;
        for (size_t i = 0; i < got; i++) {
            uint32_t pix_idx = buf[i * 2];
            uint32_t rgb = buf[i * 2 + 1];
            if (pix_idx < npix) {
                tile[pix_idx] = rgb;
            }
        }
        entries_read += got;
    }

    /* Convert packed uint32 RGB to 3-byte RGB for .raw output */
    unsigned char *pixels = malloc(npix * 3);
    if (!pixels) {
        fprintf(stderr, "Cannot allocate pixel output: %zu MB\n",
                npix * 3 / (1024 * 1024));
        free(tile);
        return 1;
    }

    for (size_t i = 0; i < npix; i++) {
        pixels[i * 3 + 0] = (tile[i] >> 16) & 0xff;
        pixels[i * 3 + 1] = (tile[i] >> 8)  & 0xff;
        pixels[i * 3 + 2] =  tile[i]         & 0xff;
    }

    free(tile);

    /* Write .raw (12-byte header + pixels) */
    FILE *fout = fopen(outPath, "wb");
    if (!fout) {
        fprintf(stderr, "Cannot create %s\n", outPath);
        free(pixels);
        return 1;
    }

    uint32_t header[3] = { (uint32_t)TW, (uint32_t)TH, 3 };
    fwrite(header, 4, 3, fout);
    fwrite(pixels, 1, npix * 3, fout);
    fclose(fout);

    free(pixels);

    /* Output metadata as JSON to stdout */
    fprintf(stdout, "{\"tile_w\":%d,\"tile_h\":%d,\"entries\":%ld,\"pixels\":%zu}\n",
            TW, TH, entries_read, npix);

    return 0;
}
