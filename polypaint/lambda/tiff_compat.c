/*
 * tiff_compat: convert tiled bilevel TIFF to strip-based for Preview compatibility.
 *
 * Reads tiled or strip CCITT G4 TIFF via libvips (handles any layout),
 * writes strip-based CCITT G4 TIFF via libtiff.
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

    /* Read input with libvips — handles tiled, strip, any compression */
    VipsImage *img = vips_image_new_from_file(inPath, NULL);
    if (!img) {
        fprintf(stderr, "Cannot read %s: %s\n", inPath, vips_error_buffer());
        vips_shutdown();
        return 1;
    }

    int w = img->Xsize, h = img->Ysize;

    /* Get pixel data as uchar (0 or 255 for bilevel) */
    size_t sz;
    unsigned char *pixels = (unsigned char *)vips_image_write_to_memory(img, &sz);
    g_object_unref(img);
    if (!pixels) {
        fprintf(stderr, "Cannot read pixels\n");
        vips_shutdown();
        return 1;
    }

    /* Write strip-based CCITT G4 TIFF with libtiff */
    TIFF *tif = TIFFOpen(outPath, "w");
    if (!tif) {
        fprintf(stderr, "Cannot create %s\n", outPath);
        g_free(pixels);
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
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, (uint32_t)h);

    /* Pack uchar pixels to 1-bit and write scanline by scanline */
    int rowBytes = (w + 7) / 8;
    uint8_t *rowBuf = calloc(1, rowBytes);
    for (int y = 0; y < h; y++) {
        memset(rowBuf, 0, rowBytes);
        unsigned char *row = pixels + (size_t)y * w;
        for (int x = 0; x < w; x++) {
            if (row[x]) {
                rowBuf[x / 8] |= (1 << (7 - (x % 8)));
            }
        }
        if (TIFFWriteScanline(tif, rowBuf, y, 0) < 0) {
            fprintf(stderr, "Write error at row %d\n", y);
            free(rowBuf);
            g_free(pixels);
            TIFFClose(tif);
            vips_shutdown();
            return 1;
        }
    }

    free(rowBuf);
    g_free(pixels);
    TIFFClose(tif);

    /* Report */
    FILE *f = fopen(outPath, "rb");
    long fsize = 0;
    if (f) { fseek(f, 0, SEEK_END); fsize = ftell(f); fclose(f); }

    printf("{\"width\":%d,\"height\":%d,\"file_size\":%ld}\n", w, h, fsize);

    vips_shutdown();
    return 0;
}
