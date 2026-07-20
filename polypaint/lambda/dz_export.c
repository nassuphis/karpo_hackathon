/*
 * dz_export: generate a DeepZoom tile pyramid from an image via libvips dzsave.
 *
 * Reads the source with sequential access, writes .dzi + tile directory.
 * OpenSeadragon-compatible output.
 *
 * Usage: dz_export input-image /tmp/dz/image [--bilevel]
 *
 * Output: {outBase}.dzi + {outBase}_files/level/col_row.png
 *
 * Build (dynamic, needs libvips from Lambda layer):
 *   gcc -O3 -o dz_export dz_export.c \
 *     -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
 *     -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm -Wl,-rpath,/opt/lib
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vips/vips.h>

int main(int argc, char **argv) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: dz_export input-image outputBase [--bilevel]\n");
        return 1;
    }
    int bilevel = 0;
    if (argc == 4) {
        if (strcmp(argv[3], "--bilevel") != 0) {
            fprintf(stderr, "Unknown option: %s\n", argv[3]);
            return 1;
        }
        bilevel = 1;
    }
    if (VIPS_INIT(argv[0])) {
        fprintf(stderr, "vips_init failed\n");
        return 1;
    }

    const char *inPath = argv[1];
    const char *outBase = argv[2];

    VipsImage *img = vips_image_new_from_file(inPath, "access", VIPS_ACCESS_SEQUENTIAL, NULL);
    if (!img) {
        fprintf(stderr, "Cannot read %s: %s\n", inPath, vips_error_buffer());
        vips_shutdown();
        return 1;
    }

    int w = img->Xsize, h = img->Ysize;

    const char *suffix = bilevel
        ? ".png[bitdepth=1,compression=6]"
        : ".png";
    if (vips_dzsave(img, outBase,
                    "layout", VIPS_FOREIGN_DZ_LAYOUT_DZ,
                    "suffix", suffix,
                    "tile-size", 256,
                    "overlap", 0,
                    NULL)) {
        fprintf(stderr, "vips_dzsave failed: %s\n", vips_error_buffer());
        g_object_unref(img);
        vips_shutdown();
        return 1;
    }

    g_object_unref(img);

    printf("{\"width\":%d,\"height\":%d,\"bitdepth\":%d}\n",
           w, h, bilevel ? 1 : 8);

    vips_shutdown();
    return 0;
}
