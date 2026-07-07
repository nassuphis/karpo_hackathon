/*
 * wall_dz: composite mosaic-wall previews into one DeepZoom pyramid.
 *
 * Reads a list file (one image path per line, already in baked wall order),
 * arrayjoins them into a grid `across` columns wide, and dzsaves an
 * OpenSeadragon-compatible pyramid with JPEG tiles (deepzoom-speed.md §7.1).
 *
 * Usage: wall_dz listfile across outputBase
 *
 * Output: {outputBase}.dzi + {outputBase}_files/level/col_row.jpg
 * Prints: {"width":W,"height":H,"count":N,"across":A} on success.
 *
 * Inputs are the wall's uniform 512px preview jpgs; RANDOM access decodes
 * each (~786 KB) so dzsave can pull lower pyramid levels — ~1.3 GB for a
 * 1,651-tile wall, which is why the deepzoom-export lambda gets 8 GB.
 *
 * Build (dynamic, needs libvips from Lambda layer) — same recipe as
 * dz_export.c in deploy.sh's layer-build block.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vips/vips.h>

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: wall_dz listfile across outputBase\n");
        return 1;
    }
    if (VIPS_INIT(argv[0])) {
        fprintf(stderr, "vips_init failed\n");
        return 1;
    }

    const char *listPath = argv[1];
    int across = atoi(argv[2]);
    const char *outBase = argv[3];
    if (across <= 0) {
        fprintf(stderr, "across must be positive, got %s\n", argv[2]);
        return 1;
    }

    FILE *lf = fopen(listPath, "r");
    if (!lf) {
        fprintf(stderr, "Cannot open list file %s\n", listPath);
        return 1;
    }

    int cap = 4096;
    int count = 0;
    VipsImage **images = g_malloc0(sizeof(VipsImage *) * cap);
    char line[4096];
    while (fgets(line, sizeof(line), lf)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';
        if (len == 0)
            continue;
        if (count >= cap) {
            cap *= 2;
            images = g_realloc(images, sizeof(VipsImage *) * cap);
        }
        VipsImage *img = vips_image_new_from_file(line, "access", VIPS_ACCESS_RANDOM, NULL);
        if (!img) {
            fprintf(stderr, "Cannot read %s: %s\n", line, vips_error_buffer());
            fclose(lf);
            return 1;
        }
        /* arrayjoin requires matching band counts; previews are RGB but a
         * placeholder or odd artifact could differ. */
        if (img->Bands != 3) {
            VipsImage *rgb;
            if (vips_colourspace(img, &rgb, VIPS_INTERPRETATION_sRGB, NULL)) {
                fprintf(stderr, "Cannot normalise %s: %s\n", line, vips_error_buffer());
                fclose(lf);
                return 1;
            }
            g_object_unref(img);
            img = rgb;
        }
        images[count++] = img;
    }
    fclose(lf);
    if (count == 0) {
        fprintf(stderr, "List file %s is empty\n", listPath);
        return 1;
    }

    VipsImage *wall;
    if (vips_arrayjoin(images, &wall, count, "across", across, NULL)) {
        fprintf(stderr, "vips_arrayjoin failed: %s\n", vips_error_buffer());
        return 1;
    }
    for (int i = 0; i < count; i++)
        g_object_unref(images[i]);
    g_free(images);

    /* flat full-resolution composite alongside the pyramid — the Save Wall
     * button downloads it and it's print-grade (well under JPEG's 65,535px
     * cap). Written first so a dzsave failure can't leave a half wall.jpg. */
    char jpgPath[4096];
    snprintf(jpgPath, sizeof(jpgPath), "%s.jpg", outBase);
    if (vips_jpegsave(wall, jpgPath, "Q", 90, NULL)) {
        fprintf(stderr, "vips_jpegsave failed: %s\n", vips_error_buffer());
        g_object_unref(wall);
        vips_shutdown();
        return 1;
    }

    if (vips_dzsave(wall, outBase,
                    "layout", VIPS_FOREIGN_DZ_LAYOUT_DZ,
                    "suffix", ".jpg[Q=88]",
                    "tile-size", 256,
                    "overlap", 0,
                    NULL)) {
        fprintf(stderr, "vips_dzsave failed: %s\n", vips_error_buffer());
        g_object_unref(wall);
        vips_shutdown();
        return 1;
    }

    printf("{\"width\":%d,\"height\":%d,\"count\":%d,\"across\":%d}\n",
           wall->Xsize, wall->Ysize, count, across);

    g_object_unref(wall);
    vips_shutdown();
    return 0;
}
