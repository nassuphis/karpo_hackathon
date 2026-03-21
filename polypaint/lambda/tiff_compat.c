/*
 * tiff_compat: convert tiled bilevel TIFF to strip-based for Preview compatibility.
 *
 * Streaming: reads one tile row at a time via libvips sequential access,
 * packs to 1-bit, writes scanlines. Peak memory: one tile row of pixels.
 *
 * Auto-selects classic TIFF vs BigTIFF based on image dimensions.
 *
 * Usage: tiff_compat input.tif output.tif
 *
 * Build (dynamic, needs libvips + libtiff from Lambda layer):
 *   gcc -O3 -o tiff_compat tiff_compat.c \
 *     -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
 *     -L/opt/lib -lvips -ltiff -lgobject-2.0 -lglib-2.0 -lm -Wl,-rpath,/opt/lib
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <vips/vips.h>
#include <tiffio.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: tiff_compat input.tif output.tif\n");
        return 1;
    }
    if (VIPS_INIT(argv[0])) {
        fprintf(stderr, "vips_init failed\n");
        return 1;
    }

    const char *inPath = argv[1];
    const char *outPath = argv[2];

    /* Open with sequential access — libvips streams rows on demand */
    VipsImage *img = vips_image_new_from_file(inPath, "access", VIPS_ACCESS_SEQUENTIAL, NULL);
    if (!img) {
        fprintf(stderr, "Cannot read %s: %s\n", inPath, vips_error_buffer());
        vips_shutdown();
        return 1;
    }

    int w = img->Xsize, h = img->Ysize;

    /* Auto-select BigTIFF for large images (>4 GB potential uncompressed) */
    int64_t rawBits = (int64_t)w * h;
    const char *tiffMode = (rawBits > (int64_t)4 * 1024 * 1024 * 1024 * 8) ? "w8" : "w";

    /* Create output strip-based TIFF */
    TIFF *tif = TIFFOpen(outPath, tiffMode);
    if (!tif) {
        fprintf(stderr, "Cannot create %s\n", outPath);
        g_object_unref(img);
        vips_shutdown();
        return 1;
    }

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, (uint32_t)w);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, (uint32_t)h);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 1);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_CCITTFAX4);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(tif, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, (uint32_t)1);

    /* Prepare a VipsRegion for streaming row access */
    VipsRegion *region = vips_region_new(img);
    if (!region) {
        fprintf(stderr, "Cannot create region\n");
        TIFFClose(tif);
        g_object_unref(img);
        vips_shutdown();
        return 1;
    }

    int rowBytes = (w + 7) / 8;
    uint8_t *rowBuf = calloc(1, rowBytes);
    int error = 0;

    for (int y = 0; y < h && !error; y++) {
        /* Request one row from libvips (streams from source) */
        VipsRect r = { 0, y, w, 1 };
        if (vips_region_prepare(region, &r) < 0) {
            fprintf(stderr, "vips_region_prepare failed at row %d: %s\n", y, vips_error_buffer());
            error = 1;
            break;
        }
        unsigned char *px = VIPS_REGION_ADDR(region, 0, y);

        /* Pack to 1-bit MSB-first */
        memset(rowBuf, 0, rowBytes);
        for (int x = 0; x < w; x++) {
            if (px[x]) {
                rowBuf[x / 8] |= (1 << (7 - (x % 8)));
            }
        }

        if (TIFFWriteScanline(tif, rowBuf, y, 0) < 0) {
            fprintf(stderr, "TIFFWriteScanline failed at row %d\n", y);
            error = 1;
        }
    }

    free(rowBuf);
    g_object_unref(region);
    g_object_unref(img);
    TIFFClose(tif);

    if (error) {
        vips_shutdown();
        return 1;
    }

    FILE *f = fopen(outPath, "rb");
    long fsize = 0;
    if (f) { fseek(f, 0, SEEK_END); fsize = ftell(f); fclose(f); }

    printf("{\"width\":%d,\"height\":%d,\"file_size\":%ld,\"bigtiff\":%s}\n",
           w, h, fsize, tiffMode[1] == '8' ? "true" : "false");

    vips_shutdown();
    return 0;
}
