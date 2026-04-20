/*
 * raw_to_bilevel: threshold a greyscale.raw score image into a 1-bit bilevel
 * TIFF plus optional preview PNG.
 *
 * Usage:
 *   raw_to_bilevel input.raw out.tif
 *       --width=W --height=H --threshold=T
 *       [--preview=preview.png] [--preview_size=1024]
 *
 * Pixels with value > threshold are foreground; the rest are background.
 */

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

int main(int argc, char **argv) {
    if (VIPS_INIT(argv[0])) {
        fprintf(stderr, "vips_init failed\n");
        return 1;
    }

    if (argc < 3) {
        fprintf(stderr, "Usage: raw_to_bilevel input.raw out.tif --width=W --height=H --threshold=T [--preview=preview.png] [--preview_size=1024]\n");
        vips_shutdown();
        return 1;
    }

    const char *inPath = argv[1];
    const char *outPath = argv[2];
    int width = getArgInt(argc, argv, "--width", 0);
    int height = getArgInt(argc, argv, "--height", 0);
    int threshold = getArgInt(argc, argv, "--threshold", 0);
    const char *previewPath = getArg(argc, argv, "--preview");
    int previewSize = getArgInt(argc, argv, "--preview_size", 1024);

    if (width <= 0 || height <= 0) {
        fprintf(stderr, "width and height must be > 0\n");
        vips_shutdown();
        return 1;
    }
    if (threshold < 0 || threshold > 255) {
        fprintf(stderr, "threshold must be in [0,255]\n");
        vips_shutdown();
        return 1;
    }

    VipsImage *raw = NULL;
    VipsImage *thresh = NULL;
    if (vips_rawload(inPath, &raw, width, height, 1, NULL)) {
        fprintf(stderr, "vips_rawload failed: %s\n", vips_error_buffer());
        vips_shutdown();
        return 1;
    }
    if (vips_more_const1(raw, &thresh, threshold, NULL)) {
        fprintf(stderr, "vips_more_const1 failed: %s\n", vips_error_buffer());
        g_object_unref(raw);
        vips_shutdown();
        return 1;
    }

    if (vips_tiffsave(thresh, outPath,
                      /* 1-bit output is already compact; avoid Fax4 CPU cost on large noisy masks. */
                      "compression", VIPS_FOREIGN_TIFF_COMPRESSION_NONE,
                      "bitdepth", 1,
                      NULL)) {
        fprintf(stderr, "vips_tiffsave failed: %s\n", vips_error_buffer());
        g_object_unref(thresh);
        g_object_unref(raw);
        vips_shutdown();
        return 1;
    }

    long previewFsize = 0;
    if (previewPath && previewSize > 0) {
        int maxDim = width > height ? width : height;
        double scale = (double)previewSize / (double)maxDim;
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

    printf(
        "{\"width\":%d,\"height\":%d,\"threshold\":%d,\"file_size\":%ld,\"preview_size\":%ld}\n",
        width,
        height,
        threshold,
        fsize,
        previewFsize
    );

    g_object_unref(thresh);
    g_object_unref(raw);
    vips_shutdown();
    return 0;
}
