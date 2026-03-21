/*
 * dz_export: generate DeepZoom tile pyramid from a TIFF via libvips dzsave.
 *
 * Reads TIFF with sequential access, writes .dzi + tile directory.
 * OpenSeadragon-compatible output.
 *
 * Usage: dz_export input.tif /tmp/dz/image
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
#include <vips/vips.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: dz_export input.tif outputBase\n");
        return 1;
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

    if (vips_dzsave(img, outBase,
                    "layout", VIPS_FOREIGN_DZ_LAYOUT_DZ,
                    "suffix", ".png",
                    "tile-size", 256,
                    "overlap", 0,
                    NULL)) {
        fprintf(stderr, "vips_dzsave failed: %s\n", vips_error_buffer());
        g_object_unref(img);
        vips_shutdown();
        return 1;
    }

    g_object_unref(img);

    printf("{\"width\":%d,\"height\":%d}\n", w, h);

    vips_shutdown();
    return 0;
}
