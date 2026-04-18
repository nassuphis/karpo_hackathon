/*
 * score_raw_render: render greyscale.raw score bytes directly to a final
 * color image using a 256-entry equalization LUT and a 256x3 palette LUT.
 *
 * Usage:
 *   score_raw_render input.raw out.jpeg --width=W --height=H --eq_lut=eq.bin \
 *     --palette=<name> --background_color=RRGGBB [--quality=90]
 *     [--preview=preview.png] [--preview_max=512]
 *
 * Input raw format: row-major uchar bytes, one byte per pixel, no header.
 * eq.bin must contain exactly 256 bytes. Entry 0 is used for background.
 *
 * Build (dynamic, needs libvips from Lambda layer):
 *   gcc -O3 -o score_raw_render score_raw_render.c \
 *     -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
 *     -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm \
 *     -Wl,-rpath,/opt/lib
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vips/vips.h>

#include "palette_lut.h"

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

static int parse_hex_rgb(const char *value, unsigned char *r, unsigned char *g, unsigned char *b) {
    char buf[7];
    int idx = 0;
    const char *s = value ? value : "";
    if (*s == '#') s++;
    while (*s && idx < 6) {
        if (!isxdigit((unsigned char)*s)) return 0;
        buf[idx++] = *s++;
    }
    if (idx != 6 || *s != '\0') return 0;
    buf[6] = '\0';
    char pair[3];
    pair[2] = '\0';
    pair[0] = buf[0]; pair[1] = buf[1]; *r = (unsigned char)strtoul(pair, NULL, 16);
    pair[0] = buf[2]; pair[1] = buf[3]; *g = (unsigned char)strtoul(pair, NULL, 16);
    pair[0] = buf[4]; pair[1] = buf[5]; *b = (unsigned char)strtoul(pair, NULL, 16);
    return 1;
}

static int load_exact_file(const char *path, unsigned char *buf, size_t expected) {
    FILE *fh = fopen(path, "rb");
    if (!fh) return 0;
    size_t got = fread(buf, 1, expected, fh);
    int extra = fgetc(fh);
    fclose(fh);
    return got == expected && extra == EOF;
}

static VipsImage *load_eq_lut_image(const char *path) {
    unsigned char eqBytes[256];
    if (!load_exact_file(path, eqBytes, sizeof(eqBytes))) {
        fprintf(stderr, "Failed to load 256-byte equalization LUT from %s\n", path);
        return NULL;
    }
    VipsImage *lut = vips_image_new_from_memory_copy(eqBytes, sizeof(eqBytes), 256, 1, 1, VIPS_FORMAT_UCHAR);
    if (!lut) {
        fprintf(stderr, "vips_image_new_from_memory_copy(eq_lut) failed: %s\n", vips_error_buffer());
        return NULL;
    }
    return lut;
}

static VipsImage *build_palette_lut_image(const char *paletteName, const char *backgroundColor) {
    const PaletteDef *pal = findPalette(paletteName);
    if (!pal) {
        fprintf(stderr, "Unknown palette: %s\n", paletteName);
        return NULL;
    }
    unsigned char bgR = 0, bgG = 0, bgB = 0;
    if (!parse_hex_rgb(backgroundColor, &bgR, &bgG, &bgB)) {
        fprintf(stderr, "Invalid background color: %s\n", backgroundColor);
        return NULL;
    }
    unsigned char palBytes[256 * 3];
    palBytes[0] = bgR;
    palBytes[1] = bgG;
    palBytes[2] = bgB;
    for (int i = 1; i < 256; i++) {
        double t = (double)(i - 1) / 254.0;
        unsigned char r, g, b;
        paletteRGB(pal, t, &r, &g, &b);
        palBytes[i * 3 + 0] = r;
        palBytes[i * 3 + 1] = g;
        palBytes[i * 3 + 2] = b;
    }
    VipsImage *lut = vips_image_new_from_memory_copy(palBytes, sizeof(palBytes), 256, 1, 3, VIPS_FORMAT_UCHAR);
    if (!lut) {
        fprintf(stderr, "vips_image_new_from_memory_copy(palette_lut) failed: %s\n", vips_error_buffer());
        return NULL;
    }
    return lut;
}

static int save_image(VipsImage *img, const char *outPath, int quality) {
    const char *ext = strrchr(outPath, '.');
    int isJpeg = ext && (strcmp(ext, ".jpeg") == 0 || strcmp(ext, ".jpg") == 0);
    if (isJpeg) {
        if (vips_jpegsave(img, outPath, "Q", quality, NULL)) {
            fprintf(stderr, "vips_jpegsave failed: %s\n", vips_error_buffer());
            return 0;
        }
    } else {
        if (vips_pngsave(img, outPath, "compression", 6, NULL)) {
            fprintf(stderr, "vips_pngsave failed: %s\n", vips_error_buffer());
            return 0;
        }
    }
    return 1;
}

static long file_size_bytes(const char *path) {
    FILE *fh = fopen(path, "rb");
    long size = 0;
    if (!fh) return 0;
    fseek(fh, 0, SEEK_END);
    size = ftell(fh);
    fclose(fh);
    return size;
}

int main(int argc, char **argv) {
    if (VIPS_INIT(argv[0])) {
        fprintf(stderr, "VIPS_INIT failed: %s\n", vips_error_buffer());
        return 1;
    }
    vips_leak_set(0);

    if (argc < 3) {
        fprintf(stderr, "Usage: score_raw_render input.raw out.jpeg --width=W --height=H --eq_lut=eq.bin --palette=<name> --background_color=RRGGBB [--quality=90] [--preview=preview.png] [--preview_max=512]\n");
        vips_shutdown();
        return 2;
    }

    const char *inPath = argv[1];
    const char *outPath = argv[2];
    const char *eqLutPath = getArg(argc, argv, "--eq_lut");
    const char *paletteName = getArg(argc, argv, "--palette");
    const char *backgroundColor = getArg(argc, argv, "--background_color");
    const char *previewPath = getArg(argc, argv, "--preview");
    int width = getArgInt(argc, argv, "--width", 0);
    int height = getArgInt(argc, argv, "--height", 0);
    int quality = getArgInt(argc, argv, "--quality", 90);
    int previewMax = getArgInt(argc, argv, "--preview_max", 512);

    if (!eqLutPath || !paletteName || !backgroundColor || width <= 0 || height <= 0) {
        fprintf(stderr, "Missing required args: --width, --height, --eq_lut, --palette, --background_color\n");
        vips_shutdown();
        return 2;
    }

    VipsImage *raw = NULL;
    VipsImage *eqLut = NULL;
    VipsImage *equalized = NULL;
    VipsImage *paletteLut = NULL;
    VipsImage *rgb = NULL;
    VipsImage *preview = NULL;
    int exitCode = 1;

    eqLut = load_eq_lut_image(eqLutPath);
    if (!eqLut) goto cleanup;

    paletteLut = build_palette_lut_image(paletteName, backgroundColor);
    if (!paletteLut) goto cleanup;

    if (vips_rawload(inPath, &raw, width, height, 1, NULL)) {
        fprintf(stderr, "vips_rawload failed: %s\n", vips_error_buffer());
        goto cleanup;
    }
    if (vips_maplut(raw, &equalized, eqLut, NULL)) {
        fprintf(stderr, "vips_maplut(equalization) failed: %s\n", vips_error_buffer());
        goto cleanup;
    }
    if (vips_maplut(equalized, &rgb, paletteLut, NULL)) {
        fprintf(stderr, "vips_maplut(palette) failed: %s\n", vips_error_buffer());
        goto cleanup;
    }
    if (!save_image(rgb, outPath, quality)) goto cleanup;

    if (previewPath && *previewPath) {
        int maxDim = width > height ? width : height;
        if (previewMax > 0 && maxDim > previewMax) {
            double scale = (double)previewMax / (double)maxDim;
            if (vips_resize(rgb, &preview, scale, NULL)) {
                fprintf(stderr, "vips_resize(preview) failed: %s\n", vips_error_buffer());
                goto cleanup;
            }
        } else {
            preview = rgb;
            g_object_ref(preview);
        }
        if (vips_pngsave(preview, previewPath, "compression", 6, NULL)) {
            fprintf(stderr, "vips_pngsave(preview) failed: %s\n", vips_error_buffer());
            goto cleanup;
        }
    }

    printf(
        "{\"status\":\"ok\",\"file_size\":%ld,\"preview_file_size\":%ld}\n",
        file_size_bytes(outPath),
        previewPath && *previewPath ? file_size_bytes(previewPath) : 0L
    );
    exitCode = 0;

cleanup:
    if (preview) g_object_unref(preview);
    if (rgb) g_object_unref(rgb);
    if (paletteLut) g_object_unref(paletteLut);
    if (equalized) g_object_unref(equalized);
    if (eqLut) g_object_unref(eqLut);
    if (raw) g_object_unref(raw);
    vips_shutdown();
    return exitCode;
}
