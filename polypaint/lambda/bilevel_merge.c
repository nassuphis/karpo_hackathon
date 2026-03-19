/*
 * bilevel_merge: merge per-stripe tile bitsets or stitch tile PNGs.
 *
 * Mode "merge": OR multiple .bits files → 1-bit tile PNG
 *   bilevel_merge merge --tile_w=TW --tile_h=TH --output=tile.png
 *       bits1.bits bits2.bits ...
 *
 * Mode "stitch": join tile PNGs into final image PNG
 *   bilevel_merge stitch --n_cols=C --n_rows=R --output=final.png
 *       tile0.png tile1.png ...
 *
 * Build (dynamic, needs libvips):
 *   gcc -O3 -o bilevel_merge bilevel_merge.c \
 *     -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
 *     -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm \
 *     -Wl,-rpath,/opt/lib
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <vips/vips.h>

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
static const char *getArgStr(int argc, char **argv, const char *key, const char *def) {
    const char *v = getArg(argc, argv, key);
    return v ? v : def;
}

/* ---- Merge: OR bitsets → 1-bit PNG ---- */

static int do_merge(int argc, char **argv) {
    int tileW = getArgInt(argc, argv, "--tile_w", 4096);
    int tileH = getArgInt(argc, argv, "--tile_h", 4096);
    const char *outPath = getArgStr(argc, argv, "--output", "/tmp/tile.png");

    size_t nPixels = (size_t)tileW * tileH;
    size_t bitsetBytes = (nPixels + 7) / 8;
    uint8_t *bitset = calloc(1, bitsetBytes);
    if (!bitset) {
        fprintf(stderr, "Cannot allocate bitset (%zu bytes)\n", bitsetBytes);
        return 1;
    }

    /* Collect .bits file paths (non --flag, non mode args) */
    int nFiles = 0;
    for (int i = 2; i < argc; i++) {
        if (argv[i][0] == '-') continue;
        FILE *f = fopen(argv[i], "rb");
        if (!f) continue;
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        fseek(f, 0, SEEK_SET);
        if (sz > 0) {
            uint8_t *buf = malloc(sz);
            if (buf) {
                fread(buf, 1, sz, f);
                /* OR into accumulated bitset */
                size_t limit = sz < (long)bitsetBytes ? sz : bitsetBytes;
                for (size_t b = 0; b < limit; b++)
                    bitset[b] |= buf[b];
                free(buf);
            }
        }
        fclose(f);
        nFiles++;
    }

    /* Count set pixels */
    long pixelsSet = 0;
    for (size_t b = 0; b < bitsetBytes; b++) {
        uint8_t v = bitset[b];
        while (v) { pixelsSet += v & 1; v >>= 1; }
    }

    /* Convert bitset to uchar buffer (0 or 255) */
    unsigned char *imgBuf = malloc(nPixels);
    if (!imgBuf) {
        fprintf(stderr, "Cannot allocate image buffer\n");
        free(bitset);
        return 1;
    }
    for (size_t i = 0; i < nPixels; i++)
        imgBuf[i] = (bitset[i >> 3] & (1u << (i & 7))) ? 255 : 0;
    free(bitset);

    /* Create libvips image and write 1-bit PNG */
    VipsImage *img = vips_image_new_from_memory(imgBuf, nPixels, tileW, tileH, 1, VIPS_FORMAT_UCHAR);
    if (!img) {
        fprintf(stderr, "vips_image_new_from_memory failed: %s\n", vips_error_buffer());
        free(imgBuf);
        return 1;
    }

    VipsImage *thresh;
    if (vips_more_const1(img, &thresh, 0, NULL)) {
        fprintf(stderr, "vips_more_const1 failed: %s\n", vips_error_buffer());
        g_object_unref(img);
        free(imgBuf);
        return 1;
    }

    if (vips_pngsave(thresh, outPath, "compression", 9, "bitdepth", 1, NULL)) {
        fprintf(stderr, "vips_pngsave failed: %s\n", vips_error_buffer());
        g_object_unref(thresh);
        g_object_unref(img);
        free(imgBuf);
        return 1;
    }

    g_object_unref(thresh);
    g_object_unref(img);
    free(imgBuf);

    FILE *fout = fopen(outPath, "rb");
    long fsize = 0;
    if (fout) { fseek(fout, 0, SEEK_END); fsize = ftell(fout); fclose(fout); }

    printf("{\"mode\":\"merge\",\"files_merged\":%d,\"pixels_set\":%ld,\"file_size\":%ld}\n",
           nFiles, pixelsSet, fsize);
    return 0;
}

/* ---- Stitch: join tile PNGs → final PNG ---- */

static int do_stitch(int argc, char **argv) {
    int nCols = getArgInt(argc, argv, "--n_cols", 1);
    int nRows = getArgInt(argc, argv, "--n_rows", 1);
    const char *outPath = getArgStr(argc, argv, "--output", "/tmp/final.png");

    int nTiles = nCols * nRows;

    /* Collect tile PNG paths */
    const char *paths[4096];
    int nPaths = 0;
    for (int i = 2; i < argc && nPaths < nTiles; i++) {
        if (argv[i][0] == '-') continue;
        paths[nPaths++] = argv[i];
    }
    if (nPaths != nTiles) {
        fprintf(stderr, "Expected %d tile paths, got %d\n", nTiles, nPaths);
        return 1;
    }

    /* Load all tile images — fail hard on missing tiles */
    VipsImage **tiles = malloc(nTiles * sizeof(VipsImage *));
    for (int t = 0; t < nTiles; t++) {
        tiles[t] = vips_image_new_from_file(paths[t], NULL);
        if (!tiles[t]) {
            fprintf(stderr, "Cannot load tile %d (%s): %s\n", t, paths[t], vips_error_buffer());
            for (int j = 0; j < t; j++) g_object_unref(tiles[j]);
            free(tiles);
            return 1;
        }
    }

    /* Join into grid: arrayjoin with across=nCols */
    VipsImage *joined;
    if (vips_arrayjoin(tiles, &joined, nTiles, "across", nCols, NULL)) {
        fprintf(stderr, "vips_arrayjoin failed: %s\n", vips_error_buffer());
        for (int t = 0; t < nTiles; t++) g_object_unref(tiles[t]);
        free(tiles);
        return 1;
    }

    /* Save as 1-bit PNG */
    if (vips_pngsave(joined, outPath, "compression", 9, "bitdepth", 1, NULL)) {
        fprintf(stderr, "vips_pngsave failed: %s\n", vips_error_buffer());
        g_object_unref(joined);
        for (int t = 0; t < nTiles; t++) g_object_unref(tiles[t]);
        free(tiles);
        return 1;
    }

    g_object_unref(joined);
    for (int t = 0; t < nTiles; t++) g_object_unref(tiles[t]);
    free(tiles);

    FILE *fout = fopen(outPath, "rb");
    long fsize = 0;
    if (fout) { fseek(fout, 0, SEEK_END); fsize = ftell(fout); fclose(fout); }

    printf("{\"mode\":\"stitch\",\"tiles\":%d,\"file_size\":%ld}\n", nTiles, fsize);
    return 0;
}

/* ---- Main ---- */

int main(int argc, char **argv) {
    if (VIPS_INIT(argv[0])) {
        fprintf(stderr, "vips_init failed\n");
        return 1;
    }

    if (argc < 2) {
        fprintf(stderr, "Usage: bilevel_merge merge|stitch [options] files...\n");
        vips_shutdown();
        return 1;
    }

    int ret;
    if (strcmp(argv[1], "merge") == 0)
        ret = do_merge(argc, argv);
    else if (strcmp(argv[1], "stitch") == 0)
        ret = do_stitch(argc, argv);
    else {
        fprintf(stderr, "Unknown mode: %s (expected merge or stitch)\n", argv[1]);
        ret = 1;
    }

    vips_shutdown();
    return ret;
}
