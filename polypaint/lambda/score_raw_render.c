/*
 * score_raw_render: render greyscale.raw score bytes directly to a final
 * color image using a 256-entry equalization LUT and a 256x3 palette LUT.
 *
 * Usage:
 *   score_raw_render input.raw out.jpeg --pix=N --eq_lut=eq.bin \
 *     --palette=<name> --background_color=RRGGBB [--quality=90]
 *     [--preview=preview.png] [--preview_max=512]
 *
 * Input raw format: row-major uchar bytes, one or three bytes per pixel, no header.
 * eq.bin must contain exactly 256 bytes. Entry 0 is used for background.
 *
 * Build (dynamic, needs libvips from Lambda layer):
 *   gcc -O3 -o score_raw_render score_raw_render.c \
 *     -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
 *     -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm \
 *     -Wl,-rpath,/opt/lib
 */

#include <ctype.h>
#include <math.h>
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

static int load_exact_file(const char *path, unsigned char *buf, size_t expected);
static int parse_hex_rgb(const char *value, unsigned char *r, unsigned char *g, unsigned char *b);

static int copy_rgb_raw_to_image(const char *inPath, VipsImage **out, int width, int height,
                                 const char *backgroundColor, int zeroBackground) {
    if (zeroBackground) {
        size_t expected = (size_t)width * (size_t)height * 3u;
        unsigned char *rgb = (unsigned char *)malloc(expected);
        unsigned char bgR = 0, bgG = 0, bgB = 0;
        if (!rgb) {
            fprintf(stderr, "out of memory loading RGB raw\n");
            return 0;
        }
        if (!parse_hex_rgb(backgroundColor ? backgroundColor : "000000", &bgR, &bgG, &bgB)) {
            fprintf(stderr, "Invalid background color: %s\n", backgroundColor ? backgroundColor : "");
            free(rgb);
            return 0;
        }
        if (!load_exact_file(inPath, rgb, expected)) {
            fprintf(stderr, "Failed to load RGB raw bytes from %s\n", inPath);
            free(rgb);
            return 0;
        }
        for (size_t i = 0; i < expected; i += 3) {
            if (rgb[i + 0] == 0 && rgb[i + 1] == 0 && rgb[i + 2] == 0) {
                rgb[i + 0] = bgR;
                rgb[i + 1] = bgG;
                rgb[i + 2] = bgB;
            }
        }
        *out = vips_image_new_from_memory_copy(rgb, expected, width, height, 3, VIPS_FORMAT_UCHAR);
        free(rgb);
        if (!*out) {
            fprintf(stderr, "vips_image_new_from_memory_copy(RGB raw) failed: %s\n", vips_error_buffer());
            return 0;
        }
        return 1;
    }
    if (vips_rawload(inPath, out, width, height, 3, NULL)) {
        fprintf(stderr, "vips_rawload RGB failed: %s\n", vips_error_buffer());
        return 0;
    }
    return 1;
}

static const char *normalize_interpretation(const char *value, int channels) {
    const char *v = value && *value ? value : (channels == 1 ? "scalar_lut" : "rgb");
    return v;
}

static unsigned char unit_to_byte(double value) {
    return (unsigned char)lrint(fmax(0.0, fmin(255.0, value * 255.0)));
}

static void hsv_unit_to_rgb(double h, double s, double v,
                            unsigned char *rOut, unsigned char *gOut, unsigned char *bOut) {
    double r = v, g = v, b = v;
    h = h - floor(h);
    s = fmax(0.0, fmin(1.0, s));
    v = fmax(0.0, fmin(1.0, v));
    if (s > 0.0) {
        double sector = h * 6.0;
        int i = (int)floor(sector);
        double f = sector - (double)i;
        double p = v * (1.0 - s);
        double q = v * (1.0 - s * f);
        double t = v * (1.0 - s * (1.0 - f));
        switch (i % 6) {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            default: r = v; g = p; b = q; break;
        }
    }
    *rOut = unit_to_byte(r);
    *gOut = unit_to_byte(g);
    *bOut = unit_to_byte(b);
}

static void hsv_byte_to_rgb(unsigned char hByte, unsigned char sByte, unsigned char vByte,
                            unsigned char *rOut, unsigned char *gOut, unsigned char *bOut) {
    hsv_unit_to_rgb(
        (double)hByte / 255.0,
        (double)sByte / 255.0,
        (double)vByte / 255.0,
        rOut,
        gOut,
        bOut
    );
}

static void rgb_to_hsv_unit(unsigned char rByte, unsigned char gByte, unsigned char bByte,
                            double *hOut, double *sOut, double *vOut) {
    double r = (double)rByte / 255.0;
    double g = (double)gByte / 255.0;
    double b = (double)bByte / 255.0;
    double maxv = fmax(r, fmax(g, b));
    double minv = fmin(r, fmin(g, b));
    double delta = maxv - minv;
    double h = 0.0;
    if (delta > 0.0) {
        if (maxv == r) {
            h = fmod((g - b) / delta, 6.0) / 6.0;
        } else if (maxv == g) {
            h = (((b - r) / delta) + 2.0) / 6.0;
        } else {
            h = (((r - g) / delta) + 4.0) / 6.0;
        }
        if (h < 0.0) h += 1.0;
    }
    *hOut = h;
    *sOut = maxv <= 0.0 ? 0.0 : delta / maxv;
    *vOut = maxv;
}

static void paletteHSV(const PaletteDef *pal, double t,
                       double *hOut, double *sOut, double *vOut) {
    const RGB *colors = pal ? pal->colors : PAL_INFERNO;
    int n = (pal && pal->n_colors > 1) ? pal->n_colors : 16;
    if (t <= 0.0) {
        rgb_to_hsv_unit(colors[0].r, colors[0].g, colors[0].b, hOut, sOut, vOut);
        return;
    }
    if (t >= 1.0) {
        rgb_to_hsv_unit(colors[n - 1].r, colors[n - 1].g, colors[n - 1].b, hOut, sOut, vOut);
        return;
    }
    double idx = t * (double)(n - 1);
    int lo = (int)idx;
    double f = idx - lo;
    int hi = lo + 1;
    double h0, s0, v0, h1, s1, v1;
    rgb_to_hsv_unit(colors[lo].r, colors[lo].g, colors[lo].b, &h0, &s0, &v0);
    rgb_to_hsv_unit(colors[hi].r, colors[hi].g, colors[hi].b, &h1, &s1, &v1);
    double dh = h1 - h0;
    if (dh > 0.5) dh -= 1.0;
    if (dh < -0.5) dh += 1.0;
    double h = h0 + f * dh;
    h = h - floor(h);
    *hOut = h;
    *sOut = s0 * (1.0 - f) + s1 * f;
    *vOut = v0 * (1.0 - f) + v1 * f;
}

static int convert_hsv_raw_to_image(const char *inPath, VipsImage **out, int width, int height,
                                    const char *backgroundColor, int zeroBackground) {
    size_t expected = (size_t)width * (size_t)height * 3u;
    unsigned char *hsv = (unsigned char *)malloc(expected);
    unsigned char *rgb = (unsigned char *)malloc(expected);
    unsigned char bgR = 0, bgG = 0, bgB = 0;
    if (!hsv || !rgb) {
        fprintf(stderr, "out of memory converting HSV raw\n");
        free(hsv);
        free(rgb);
        return 0;
    }
    if (zeroBackground && !parse_hex_rgb(backgroundColor ? backgroundColor : "000000", &bgR, &bgG, &bgB)) {
        fprintf(stderr, "Invalid background color: %s\n", backgroundColor ? backgroundColor : "");
        free(hsv);
        free(rgb);
        return 0;
    }
    if (!load_exact_file(inPath, hsv, expected)) {
        fprintf(stderr, "Failed to load HSV raw bytes from %s\n", inPath);
        free(hsv);
        free(rgb);
        return 0;
    }
    for (size_t i = 0; i < expected; i += 3) {
        if (zeroBackground && hsv[i + 0] == 0 && hsv[i + 1] == 0 && hsv[i + 2] == 0) {
            rgb[i + 0] = bgR;
            rgb[i + 1] = bgG;
            rgb[i + 2] = bgB;
            continue;
        }
        hsv_byte_to_rgb(hsv[i + 0], hsv[i + 1], hsv[i + 2], &rgb[i + 0], &rgb[i + 1], &rgb[i + 2]);
    }
    *out = vips_image_new_from_memory_copy(rgb, expected, width, height, 3, VIPS_FORMAT_UCHAR);
    free(hsv);
    free(rgb);
    if (!*out) {
        fprintf(stderr, "vips_image_new_from_memory_copy(HSV RGB) failed: %s\n", vips_error_buffer());
        return 0;
    }
    return 1;
}

static int convert_palette_component_lut_raw_to_image(const char *inPath, VipsImage **out, int width, int height,
                                                      const char *paletteName, const char *backgroundColor,
                                                      int hsvSpace, int zeroBackground) {
    size_t expected = (size_t)width * (size_t)height * 3u;
    unsigned char *coords = (unsigned char *)malloc(expected);
    unsigned char *rgb = (unsigned char *)malloc(expected);
    if (!coords || !rgb) {
        fprintf(stderr, "out of memory converting palette LUT raw\n");
        free(coords);
        free(rgb);
        return 0;
    }
    if (!load_exact_file(inPath, coords, expected)) {
        fprintf(stderr, "Failed to load palette LUT raw bytes from %s\n", inPath);
        free(coords);
        free(rgb);
        return 0;
    }

    const PaletteDef *pal = findPalette(paletteName);
    unsigned char bgR = 0, bgG = 0, bgB = 0;
    if (!parse_hex_rgb(backgroundColor ? backgroundColor : "000000", &bgR, &bgG, &bgB)) {
        fprintf(stderr, "Invalid background color: %s\n", backgroundColor ? backgroundColor : "");
        free(coords);
        free(rgb);
        return 0;
    }
    unsigned char rLut[256], gLut[256], bLut[256];
    double hLut[256], sLut[256], vLut[256];
    for (int i = 0; i < 256; i++) {
        double t = (double)i / 255.0;
        if (hsvSpace) {
            paletteHSV(pal, t, &hLut[i], &sLut[i], &vLut[i]);
        } else {
            paletteRGB(pal, t, &rLut[i], &gLut[i], &bLut[i]);
        }
    }

    for (size_t i = 0; i < expected; i += 3) {
        unsigned char c0 = coords[i + 0];
        unsigned char c1 = coords[i + 1];
        unsigned char c2 = coords[i + 2];
        if (zeroBackground && c0 == 0 && c1 == 0 && c2 == 0) {
            rgb[i + 0] = bgR;
            rgb[i + 1] = bgG;
            rgb[i + 2] = bgB;
            continue;
        }
        if (hsvSpace) {
            hsv_unit_to_rgb(hLut[c0], sLut[c1], vLut[c2], &rgb[i + 0], &rgb[i + 1], &rgb[i + 2]);
        } else {
            rgb[i + 0] = rLut[c0];
            rgb[i + 1] = gLut[c1];
            rgb[i + 2] = bLut[c2];
        }
    }
    *out = vips_image_new_from_memory_copy(rgb, expected, width, height, 3, VIPS_FORMAT_UCHAR);
    free(coords);
    free(rgb);
    if (!*out) {
        fprintf(stderr, "vips_image_new_from_memory_copy(palette LUT RGB) failed: %s\n", vips_error_buffer());
        return 0;
    }
    return 1;
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
        fprintf(stderr, "Usage: score_raw_render input.raw out.jpeg --pix=N --eq_lut=eq.bin --palette=<name> --background_color=RRGGBB [--channels=1|3] [--interpretation=scalar_lut|rgb|hsv|rgb_lut|hsv_lut] [--zero_background=0|1] [--quality=90] [--preview=preview.png] [--preview_max=512]\n");
        vips_shutdown();
        return 2;
    }

    const char *inPath = argv[1];
    const char *outPath = argv[2];
    const char *eqLutPath = getArg(argc, argv, "--eq_lut");
    const char *paletteName = getArg(argc, argv, "--palette");
    const char *backgroundColor = getArg(argc, argv, "--background_color");
    const char *previewPath = getArg(argc, argv, "--preview");
    const char *interpretationArg = getArg(argc, argv, "--interpretation");
    const char *widthArg = getArg(argc, argv, "--width");
    const char *heightArg = getArg(argc, argv, "--height");
    int pix = getArgInt(argc, argv, "--pix", 0);
    int channels = getArgInt(argc, argv, "--channels", 1);
    int zeroBackground = getArgInt(argc, argv, "--zero_background", 1) ? 1 : 0;
    int quality = getArgInt(argc, argv, "--quality", 90);
    int previewMax = getArgInt(argc, argv, "--preview_max", 512);
    const char *interpretation = normalize_interpretation(interpretationArg, channels);

    if (widthArg || heightArg) {
        fprintf(stderr, "score_raw_render no longer accepts --width or --height; pass --pix for square output\n");
        vips_shutdown();
        return 2;
    }
    if (pix <= 0 || (channels != 1 && channels != 3)) {
        fprintf(stderr, "Missing/invalid required args: --pix and --channels must be valid (channels 1 or 3)\n");
        vips_shutdown();
        return 2;
    }
    if (channels == 1 && (!eqLutPath || !paletteName || !backgroundColor)) {
        fprintf(stderr, "Missing required scalar args: --eq_lut, --palette, --background_color\n");
        vips_shutdown();
        return 2;
    }
    if (channels == 1 && strcmp(interpretation, "scalar_lut") != 0) {
        fprintf(stderr, "channels=1 requires --interpretation=scalar_lut, got %s\n", interpretation);
        vips_shutdown();
        return 2;
    }
    if (
        channels == 3
        && strcmp(interpretation, "rgb") != 0
        && strcmp(interpretation, "hsv") != 0
        && strcmp(interpretation, "rgb_lut") != 0
        && strcmp(interpretation, "hsv_lut") != 0
    ) {
        fprintf(stderr, "channels=3 requires --interpretation=rgb, hsv, rgb_lut, or hsv_lut, got %s\n", interpretation);
        vips_shutdown();
        return 2;
    }
    if (channels == 3 && (strcmp(interpretation, "rgb_lut") == 0 || strcmp(interpretation, "hsv_lut") == 0) && !paletteName) {
        fprintf(stderr, "channels=3 interpretation=%s requires --palette\n", interpretation);
        vips_shutdown();
        return 2;
    }
    int width = pix;
    int height = pix;

    VipsImage *raw = NULL;
    VipsImage *eqLut = NULL;
    VipsImage *equalized = NULL;
    VipsImage *paletteLut = NULL;
    VipsImage *rgb = NULL;
    VipsImage *preview = NULL;
    int exitCode = 1;

    if (channels == 3) {
        if (strcmp(interpretation, "hsv") == 0) {
            if (!convert_hsv_raw_to_image(inPath, &rgb, width, height, backgroundColor, zeroBackground)) goto cleanup;
        } else if (strcmp(interpretation, "rgb_lut") == 0) {
            if (!convert_palette_component_lut_raw_to_image(inPath, &rgb, width, height, paletteName, backgroundColor, 0, zeroBackground)) goto cleanup;
        } else if (strcmp(interpretation, "hsv_lut") == 0) {
            if (!convert_palette_component_lut_raw_to_image(inPath, &rgb, width, height, paletteName, backgroundColor, 1, zeroBackground)) goto cleanup;
        } else {
            if (!copy_rgb_raw_to_image(inPath, &rgb, width, height, backgroundColor, zeroBackground)) goto cleanup;
        }
    } else {
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
