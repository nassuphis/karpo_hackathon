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
 * Inputs are preview jpgs; each is normalised to CELL_PX square before the
 * join so the grid is uniform even when a preview is not 512px (small-N
 * renders are <=512 and never upscaled at preview time, so an N=500 render
 * lands at 500px). Uniform cells keep the viewer's click-to-tile mapping
 * correct. RANDOM access decodes each so dzsave can pull lower pyramid
 * levels — ~1.3 GB for a 1,651-tile wall (why the lambda gets 8 GB).
 *
 * Build (dynamic, needs libvips from Lambda layer) — same recipe as
 * dz_export.c in deploy.sh's layer-build block.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vips/vips.h>

#define CELL_PX 512   /* must match handler_wall_pyramid.CELL_PX / viewer cell */
#ifndef JPEG_MAX_PX
#define JPEG_MAX_PX 65500  /* libjpeg's 65,535px dimension cap, with headroom */
#endif

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
        /* arrayjoin requires matching band counts AND that alpha is resolved.
         * vips_colourspace(sRGB) PRESERVES alpha, so an RGBA/GA preview would
         * stay 4/2 bands and break the join (code-review-28 F15). Normalise in
         * three explicit steps: flatten alpha against a chosen background,
         * convert any colourspace (grayscale/CMYK/...) to sRGB, then force
         * exactly three bands. */
        if (vips_image_hasalpha(img)) {
            VipsImage *flat;
            double bg[3] = {0.0, 0.0, 0.0};   /* explicit black backdrop */
            VipsArrayDouble *bgArr = vips_array_double_new(bg, 3);
            int rc = vips_flatten(img, &flat, "background", bgArr, NULL);
            vips_area_unref(VIPS_AREA(bgArr));
            if (rc) {
                fprintf(stderr, "Cannot flatten alpha of %s: %s\n", line, vips_error_buffer());
                fclose(lf);
                return 1;
            }
            g_object_unref(img);
            img = flat;
        }
        if (img->Type != VIPS_INTERPRETATION_sRGB || img->Bands != 3) {
            VipsImage *rgb;
            if (vips_colourspace(img, &rgb, VIPS_INTERPRETATION_sRGB, NULL)) {
                fprintf(stderr, "Cannot normalise %s: %s\n", line, vips_error_buffer());
                fclose(lf);
                return 1;
            }
            g_object_unref(img);
            img = rgb;
        }
        if (img->Bands != 3) {
            /* CMYK->sRGB and odd inputs can still land off 3 bands; keep RGB. */
            VipsImage *three;
            if (vips_extract_band(img, &three, 0, "n", 3, NULL)) {
                fprintf(stderr, "Cannot extract RGB bands of %s: %s\n", line, vips_error_buffer());
                fclose(lf);
                return 1;
            }
            g_object_unref(img);
            img = three;
        }
        /* Normalise every tile to a CELL_PX square so the grid stays uniform
         * even when a preview is not 512px (small-N renders are <=512 and
         * never upscaled at preview time). Fill + centre-crop keeps the tile
         * filling its cell; a clean scale for the common square previews. */
        if (img->Xsize != CELL_PX || img->Ysize != CELL_PX) {
            VipsImage *sq;
            if (vips_thumbnail_image(img, &sq, CELL_PX,
                                     "height", CELL_PX,
                                     "crop", VIPS_INTERESTING_CENTRE, NULL)) {
                fprintf(stderr, "Cannot resize %s to %dpx: %s\n",
                        line, CELL_PX, vips_error_buffer());
                fclose(lf);
                return 1;
            }
            g_object_unref(img);
            img = sq;
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

    /* Optional flat full-resolution composite alongside the pyramid — the Save
     * Wall button downloads it. A big wall exceeds JPEG's dimension cap, and
     * the whole point of the DZI is images larger than that, so the flat
     * export must be OPTIONAL: skip it above the limit rather than aborting and
     * starving DZI generation (code-review-28 F19). Written before dzsave so a
     * dzsave failure can't leave a half wall.jpg. */
    int wrote_jpeg = 0;
    if (wall->Xsize <= JPEG_MAX_PX && wall->Ysize <= JPEG_MAX_PX) {
        char jpgPath[4096];
        snprintf(jpgPath, sizeof(jpgPath), "%s.jpg", outBase);
        if (vips_jpegsave(wall, jpgPath, "Q", 90, NULL)) {
            fprintf(stderr, "vips_jpegsave failed: %s\n", vips_error_buffer());
            g_object_unref(wall);
            vips_shutdown();
            return 1;
        }
        wrote_jpeg = 1;
    } else {
        fprintf(stderr,
                "wall %dx%d exceeds JPEG %dpx cap; skipping flat wall.jpg, "
                "DZI still generated\n",
                wall->Xsize, wall->Ysize, JPEG_MAX_PX);
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

    printf("{\"width\":%d,\"height\":%d,\"count\":%d,\"across\":%d,\"flat_jpeg\":%s}\n",
           wall->Xsize, wall->Ysize, count, across, wrote_jpeg ? "true" : "false");

    g_object_unref(wall);
    vips_shutdown();
    return 0;
}
