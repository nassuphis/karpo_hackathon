/*
 * png_export: convert bilevel TIFF to 1-bit PNG via libvips.
 *
 * Reads TIFF with sequential access (streaming), writes PNG.
 * For bilevel input, uses bitdepth=1 to keep output compact.
 *
 * Usage: png_export input.tif output.png
 *
 * Build (dynamic, needs libvips from Lambda layer):
 *   gcc -O3 -o png_export png_export.c \
 *     -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
 *     -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm -Wl,-rpath,/opt/lib
 */

#include <stdio.h>
#include <stdlib.h>
#include <vips/vips.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: png_export input.tif output.png\n");
        return 1;
    }
    if (VIPS_INIT(argv[0])) {
        fprintf(stderr, "vips_init failed\n");
        return 1;
    }

    const char *inPath = argv[1];
    const char *outPath = argv[2];

    /* Sequential access for streaming read */
    VipsImage *img = vips_image_new_from_file(inPath, "access", VIPS_ACCESS_SEQUENTIAL, NULL);
    if (!img) {
        fprintf(stderr, "Cannot read %s: %s\n", inPath, vips_error_buffer());
        vips_shutdown();
        return 1;
    }

    int w = img->Xsize, h = img->Ysize;

    /* Threshold to boolean for clean bilevel output */
    VipsImage *thresh;
    if (vips_more_const1(img, &thresh, 0, NULL)) {
        fprintf(stderr, "vips_more_const1 failed: %s\n", vips_error_buffer());
        g_object_unref(img);
        vips_shutdown();
        return 1;
    }

    /* Write 1-bit PNG */
    if (vips_pngsave(thresh, outPath, "compression", 6, "bitdepth", 1, NULL)) {
        fprintf(stderr, "vips_pngsave failed: %s\n", vips_error_buffer());
        g_object_unref(thresh);
        g_object_unref(img);
        vips_shutdown();
        return 1;
    }

    g_object_unref(thresh);
    g_object_unref(img);

    FILE *f = fopen(outPath, "rb");
    long fsize = 0;
    if (f) { fseek(f, 0, SEEK_END); fsize = ftell(f); fclose(f); }

    printf("{\"width\":%d,\"height\":%d,\"file_size\":%ld}\n", w, h, fsize);

    vips_shutdown();
    return 0;
}
