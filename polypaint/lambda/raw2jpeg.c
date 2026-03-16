/*
 * raw2jpeg: encode raw pixel buffer to JPEG or PNG via libvips.
 *
 * Usage:
 *   raw2jpeg input.raw out.jpeg [--quality=90]
 *   raw2jpeg input.raw out.png
 *
 * Raw format: 12-byte header (uint32 W, H, bands) + pixel data.
 * Output format determined by file extension (.jpeg/.jpg → JPEG, else PNG).
 *
 * Build (dynamic, needs libvips from Lambda layer):
 *   gcc -O3 -o raw2jpeg raw2jpeg.c \
 *     -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
 *     -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm \
 *     -Wl,-rpath,/opt/lib
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vips/vips.h>

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

/* ---- Main ---- */

int main(int argc, char **argv) {
    if (VIPS_INIT(argv[0])) {
        fprintf(stderr, "VIPS_INIT failed: %s\n", vips_error_buffer());
        return 1;
    }
    vips_leak_set(0);

    if (argc < 3) {
        fprintf(stderr, "Usage: raw2jpeg input.raw out.jpeg [--quality=90]\n");
        vips_shutdown();
        return 1;
    }

    const char *inPath = argv[1];
    const char *outPath = argv[2];
    int quality = getArgInt(argc, argv, "--quality", 90);

    /* Read 12-byte header to get dimensions */
    unsigned int W, H, bands;
    FILE *hf = fopen(inPath, "rb");
    if (!hf) {
        fprintf(stderr, "Cannot open %s\n", inPath);
        vips_shutdown();
        return 1;
    }
    if (fread(&W, 4, 1, hf) != 1 || fread(&H, 4, 1, hf) != 1 || fread(&bands, 4, 1, hf) != 1) {
        fprintf(stderr, "Bad raw header in %s\n", inPath);
        fclose(hf);
        vips_shutdown();
        return 1;
    }
    fclose(hf);

    /* Load via vips_rawload — streams from disk, no full-image malloc.
     * offset=12 skips the 12-byte header (W, H, bands). */
    VipsImage *img;
    if (vips_rawload(inPath, &img, (int)W, (int)H, (int)bands,
                     "offset", (guint64)12, NULL)) {
        fprintf(stderr, "vips_rawload failed: %s\n", vips_error_buffer());
        vips_shutdown();
        return 1;
    }

    /* Determine format from output extension */
    const char *ext = strrchr(outPath, '.');
    int isJpeg = ext && (strcmp(ext, ".jpeg") == 0 || strcmp(ext, ".jpg") == 0);

    if (isJpeg) {
        if (vips_jpegsave(img, outPath, "Q", quality, NULL)) {
            fprintf(stderr, "vips_jpegsave failed: %s\n", vips_error_buffer());
            g_object_unref(img);
            vips_shutdown();
            return 1;
        }
    } else {
        if (vips_pngsave(img, outPath, "compression", 6, NULL)) {
            fprintf(stderr, "vips_pngsave failed: %s\n", vips_error_buffer());
            g_object_unref(img);
            vips_shutdown();
            return 1;
        }
    }

    /* Report file size */
    FILE *f = fopen(outPath, "rb");
    long fsize = 0;
    if (f) { fseek(f, 0, SEEK_END); fsize = ftell(f); fclose(f); }

    g_object_unref(img);
    printf("{\"status\":\"ok\",\"file_size\":%ld,\"format\":\"%s\"}\n",
           fsize, isJpeg ? "jpeg" : "png");

    vips_shutdown();
    return 0;
}
