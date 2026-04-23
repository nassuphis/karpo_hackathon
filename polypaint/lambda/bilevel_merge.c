/*
 * bilevel_merge: OR full-frame 1-bit section bitsets and write a bilevel TIFF.
 *
 * Usage:
 *   bilevel_merge assemble --pix=N --output=final.tif [--preview=preview.png]
 *       section0.bits section1.bits ...
 *
 * Build (dynamic, needs libvips):
 *   gcc -O3 -o bilevel_merge bilevel_merge.c \
 *     -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
 *     -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm \
 *     -Wl,-rpath,/opt/lib
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vips/vips.h>

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

static const char *getArgStr(int argc, char **argv, const char *key, const char *def) {
    const char *v = getArg(argc, argv, key);
    return v ? v : def;
}

static int option_matches(const char *arg, const char *key) {
    int klen = (int)strlen(key);
    return strncmp(arg, key, klen) == 0 && arg[klen] == '=';
}

static int reject_unknown_options(int argc, char **argv) {
    const char *allowed[] = {"--width", "--height", "--pix", "--output", "--preview", "--preview_size"};
    for (int i = 2; i < argc; i++) {
        if (argv[i][0] != '-') continue;
        int known = 0;
        for (int j = 0; j < (int)(sizeof(allowed) / sizeof(allowed[0])); j++) {
            if (option_matches(argv[i], allowed[j])) {
                known = 1;
                break;
            }
        }
        if (!known) {
            fprintf(stderr, "Unknown bilevel_merge option: %s\n", argv[i]);
            return 1;
        }
    }
    return 0;
}

static int save_bitset_tiff_preview(
    const uint8_t *bitset,
    int width,
    int height,
    const char *outPath,
    const char *previewPath,
    int previewSize,
    long *fileSizeOut,
    long *previewSizeOut,
    long *pixelsSetOut
) {
    size_t nPixels = (size_t)width * (size_t)height;
    size_t bitsetBytes = (nPixels + 7) / 8;
    long pixelsSet = 0;
    for (size_t b = 0; b < bitsetBytes; b++) {
        uint8_t v = bitset[b];
        while (v) {
            pixelsSet += v & 1;
            v >>= 1;
        }
    }

    unsigned char *imgBuf = malloc(nPixels > 0 ? nPixels : 1);
    if (!imgBuf) {
        fprintf(stderr, "Cannot allocate image buffer\n");
        return 1;
    }
    for (size_t i = 0; i < nPixels; i++) {
        imgBuf[i] = (bitset[i >> 3] & (1u << (i & 7))) ? 255 : 0;
    }

    VipsImage *img = vips_image_new_from_memory(imgBuf, nPixels, width, height, 1, VIPS_FORMAT_UCHAR);
    if (!img) {
        fprintf(stderr, "vips_image_new_from_memory failed: %s\n", vips_error_buffer());
        free(imgBuf);
        return 1;
    }

    VipsImage *thresh = NULL;
    if (vips_more_const1(img, &thresh, 0, NULL)) {
        fprintf(stderr, "vips_more_const1 failed: %s\n", vips_error_buffer());
        g_object_unref(img);
        free(imgBuf);
        return 1;
    }

    if (vips_tiffsave(thresh, outPath,
                      "compression", VIPS_FOREIGN_TIFF_COMPRESSION_CCITTFAX4,
                      "bitdepth", 1, NULL)) {
        fprintf(stderr, "vips_tiffsave failed: %s\n", vips_error_buffer());
        g_object_unref(thresh);
        g_object_unref(img);
        free(imgBuf);
        return 1;
    }

    long previewFsize = 0;
    if (previewPath) {
        double scale = (double)previewSize / (width > height ? width : height);
        if (scale >= 1.0) scale = 1.0;
        VipsImage *small = NULL;
        if (vips_resize(thresh, &small, scale, NULL) == 0) {
            if (vips_pngsave(small, previewPath, "compression", 6, NULL) == 0) {
                FILE *pf = fopen(previewPath, "rb");
                if (pf) {
                    fseek(pf, 0, SEEK_END);
                    previewFsize = ftell(pf);
                    fclose(pf);
                }
            }
            g_object_unref(small);
        }
    }

    FILE *fout = fopen(outPath, "rb");
    long fsize = 0;
    if (fout) {
        fseek(fout, 0, SEEK_END);
        fsize = ftell(fout);
        fclose(fout);
    }

    g_object_unref(thresh);
    g_object_unref(img);
    free(imgBuf);

    if (fileSizeOut) *fileSizeOut = fsize;
    if (previewSizeOut) *previewSizeOut = previewFsize;
    if (pixelsSetOut) *pixelsSetOut = pixelsSet;
    return 0;
}

static int do_assemble(int argc, char **argv) {
    const char *widthArg = getArg(argc, argv, "--width");
    const char *heightArg = getArg(argc, argv, "--height");
    int pix = getArgInt(argc, argv, "--pix", 0);
    const char *outPath = getArgStr(argc, argv, "--output", "/tmp/final.tif");
    const char *previewPath = getArgStr(argc, argv, "--preview", NULL);
    int previewSize = getArgInt(argc, argv, "--preview_size", 1024);

    if (widthArg || heightArg) {
        fprintf(stderr, "assemble no longer accepts --width or --height; pass --pix for square output\n");
        return 1;
    }
    if (pix <= 0) {
        fprintf(stderr, "assemble requires --pix\n");
        return 1;
    }

    size_t nPixels = (size_t)pix * (size_t)pix;
    size_t bitsetBytes = (nPixels + 7) / 8;
    uint8_t *bitset = calloc(1, bitsetBytes > 0 ? bitsetBytes : 1);
    if (!bitset) {
        fprintf(stderr, "Cannot allocate %zu-byte full-frame bitset\n", bitsetBytes);
        return 1;
    }

    int nFiles = 0;
    for (int i = 2; i < argc; i++) {
        if (argv[i][0] == '-') continue;
        FILE *f = fopen(argv[i], "rb");
        if (!f) {
            fprintf(stderr, "Cannot open %s\n", argv[i]);
            free(bitset);
            return 1;
        }
        if (fseek(f, 0, SEEK_END) != 0) {
            fclose(f);
            free(bitset);
            fprintf(stderr, "Cannot seek %s\n", argv[i]);
            return 1;
        }
        long sz = ftell(f);
        if (sz < 0) {
            fclose(f);
            free(bitset);
            fprintf(stderr, "Cannot stat %s\n", argv[i]);
            return 1;
        }
        if ((size_t)sz != bitsetBytes) {
            fclose(f);
            free(bitset);
            fprintf(stderr, "Unexpected bitset size for %s: got %ld expected %zu\n", argv[i], sz, bitsetBytes);
            return 1;
        }
        if (fseek(f, 0, SEEK_SET) != 0) {
            fclose(f);
            free(bitset);
            fprintf(stderr, "Cannot rewind %s\n", argv[i]);
            return 1;
        }
        uint8_t *buf = malloc(bitsetBytes > 0 ? bitsetBytes : 1);
        if (!buf) {
            fclose(f);
            free(bitset);
            fprintf(stderr, "Cannot allocate temporary section buffer\n");
            return 1;
        }
        if (bitsetBytes > 0 && fread(buf, 1, bitsetBytes, f) != bitsetBytes) {
            free(buf);
            fclose(f);
            free(bitset);
            fprintf(stderr, "Short read from %s\n", argv[i]);
            return 1;
        }
        fclose(f);
        for (size_t b = 0; b < bitsetBytes; b++) bitset[b] |= buf[b];
        free(buf);
        nFiles++;
    }

    long fsize = 0;
    long previewFsize = 0;
    long pixelsSet = 0;
    int rc = save_bitset_tiff_preview(
        bitset,
        pix,
        pix,
        outPath,
        previewPath,
        previewSize,
        &fsize,
        &previewFsize,
        &pixelsSet
    );
    free(bitset);
    if (rc != 0) return rc;

    printf("{\"mode\":\"assemble\",\"sections\":%d,\"width\":%d,\"height\":%d,"
           "\"pixels_set\":%ld,\"file_size\":%ld,\"preview_size\":%ld}\n",
           nFiles, pix, pix, pixelsSet, fsize, previewFsize);
    return 0;
}

int main(int argc, char **argv) {
    if (VIPS_INIT(argv[0])) {
        fprintf(stderr, "vips_init failed\n");
        return 1;
    }

    if (argc < 2) {
        fprintf(stderr, "Usage: bilevel_merge assemble [options] files...\n");
        vips_shutdown();
        return 1;
    }
    if (strcmp(argv[1], "assemble") != 0) {
        fprintf(stderr, "Unknown mode: %s (expected assemble)\n", argv[1]);
        vips_shutdown();
        return 1;
    }
    if (reject_unknown_options(argc, argv)) {
        vips_shutdown();
        return 1;
    }

    int ret = do_assemble(argc, argv);
    vips_shutdown();
    return ret;
}
