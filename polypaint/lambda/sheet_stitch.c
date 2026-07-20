/*
 * sheet_stitch: join raw 8-bit bilevel sheet tiles and write a 1-bit PNG.
 *
 * The input list contains one raw tile path per rendered frame, in row-major
 * order. Each file is exactly tile_px * tile_px bytes with pixels restricted
 * to 0 or 255. libvips keeps the join lazy: source tiles live on /tmp and the
 * final image is streamed through pngsave rather than materialized as a full
 * Python canvas.
 *
 * Usage:
 *   sheet_stitch list.txt output.png tile_px cols rows margin_px bg
 *
 * Build (dynamic, needs the standard libvips Lambda layer):
 *   gcc -O3 -o sheet_stitch sheet_stitch.c \
 *     -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
 *     -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm \
 *     -Wl,-rpath,/opt/lib
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <vips/vips.h>

#define MAX_TILES 4096
#define PATH_BUF 4096

static int parse_int(const char *text, const char *name, int lo, int hi, int *out) {
    char *end = NULL;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno || !end || *end != '\0' || value < lo || value > hi) {
        fprintf(stderr, "%s must be an integer in %d..%d, got %s\n",
                name, lo, hi, text);
        return -1;
    }
    *out = (int)value;
    return 0;
}

static void unref_images(VipsImage **images, int count) {
    if (!images)
        return;
    for (int i = 0; i < count; i++) {
        if (images[i])
            g_object_unref(images[i]);
    }
    g_free(images);
}

int main(int argc, char **argv) {
    if (argc != 8) {
        fprintf(stderr,
                "Usage: sheet_stitch list.txt output.png tile_px cols rows margin_px bg\n");
        return 2;
    }
    if (VIPS_INIT(argv[0])) {
        fprintf(stderr, "VIPS_INIT failed: %s\n", vips_error_buffer());
        return 1;
    }
    vips_leak_set(0);
    /* The graph is evaluated once. Disabling the operation cache prevents a
     * large sheet from retaining decoded regions that pngsave will not revisit. */
    vips_cache_set_max(0);

    const char *list_path = argv[1];
    const char *output_path = argv[2];
    int tile_px, cols, rows, margin_px, bg;
    if (parse_int(argv[3], "tile_px", 1, 1000000, &tile_px) ||
        parse_int(argv[4], "cols", 1, MAX_TILES, &cols) ||
        parse_int(argv[5], "rows", 1, MAX_TILES, &rows) ||
        parse_int(argv[6], "margin_px", 0, 1000000, &margin_px) ||
        parse_int(argv[7], "bg", 0, 255, &bg)) {
        vips_shutdown();
        return 2;
    }
    if (bg != 0 && bg != 255) {
        fprintf(stderr, "bg must be bilevel 0 or 255, got %d\n", bg);
        vips_shutdown();
        return 2;
    }
    if ((long long)cols * rows > MAX_TILES) {
        fprintf(stderr, "grid has too many cells: %d x %d > %d\n",
                cols, rows, MAX_TILES);
        vips_shutdown();
        return 2;
    }
    long long canvas_w64 = (long long)cols * tile_px
        + ((long long)cols + 1) * margin_px;
    long long canvas_h64 = (long long)rows * tile_px
        + ((long long)rows + 1) * margin_px;
    if (canvas_w64 > INT_MAX || canvas_h64 > INT_MAX) {
        fprintf(stderr, "output geometry exceeds libvips integer dimensions: %lld x %lld\n",
                canvas_w64, canvas_h64);
        vips_shutdown();
        return 2;
    }
    int canvas_w = (int)canvas_w64;
    int canvas_h = (int)canvas_h64;

    FILE *list = fopen(list_path, "r");
    if (!list) {
        fprintf(stderr, "Cannot open tile list %s: %s\n",
                list_path, strerror(errno));
        vips_shutdown();
        return 1;
    }

    VipsImage **images = g_malloc0(sizeof(VipsImage *) * MAX_TILES);
    int count = 0;
    char path[PATH_BUF];
    while (fgets(path, sizeof(path), list)) {
        size_t len = strlen(path);
        if (len == sizeof(path) - 1 && path[len - 1] != '\n') {
            fprintf(stderr, "Tile path exceeds the %d-byte input limit\n",
                    PATH_BUF - 1);
            fclose(list);
            unref_images(images, count);
            vips_shutdown();
            return 2;
        }
        while (len > 0 && (path[len - 1] == '\n' || path[len - 1] == '\r'))
            path[--len] = '\0';
        if (len == 0)
            continue;
        if (count >= MAX_TILES || count >= cols * rows) {
            fprintf(stderr, "Tile list has more entries than the %d x %d grid\n",
                    cols, rows);
            fclose(list);
            unref_images(images, count);
            vips_shutdown();
            return 2;
        }

        struct stat st;
        long long expected = (long long)tile_px * tile_px;
        int stat_rc = stat(path, &st);
        if (stat_rc || (long long)st.st_size != expected) {
            fprintf(stderr, "Bad raw tile %s: expected %lld bytes, got %lld\n",
                    path, expected, stat_rc ? -1LL : (long long)st.st_size);
            fclose(list);
            unref_images(images, count);
            vips_shutdown();
            return 1;
        }

        VipsImage *raw = NULL;
        VipsImage *bilevel = NULL;
        if (vips_rawload(path, &raw, tile_px, tile_px, 1, NULL)) {
            fprintf(stderr, "vips_rawload failed for %s: %s\n",
                    path, vips_error_buffer());
            fclose(list);
            unref_images(images, count);
            vips_shutdown();
            return 1;
        }
        /* Normalize defensively. Worker output is already 0/255, but this
         * guarantees a binary result if an old or malformed tile contains an
         * intermediate gray value. */
        if (vips_more_const1(raw, &bilevel, 0.0, NULL)) {
            fprintf(stderr, "vips_more_const1 failed for %s: %s\n",
                    path, vips_error_buffer());
            g_object_unref(raw);
            fclose(list);
            unref_images(images, count);
            vips_shutdown();
            return 1;
        }
        g_object_unref(raw);
        images[count++] = bilevel;
    }
    if (ferror(list)) {
        fprintf(stderr, "Failed while reading tile list %s: %s\n",
                list_path, strerror(errno));
        fclose(list);
        unref_images(images, count);
        vips_shutdown();
        return 1;
    }
    fclose(list);

    if (count == 0) {
        fprintf(stderr, "Tile list %s is empty\n", list_path);
        unref_images(images, count);
        vips_shutdown();
        return 2;
    }
    if (count <= (rows - 1) * cols || count > rows * cols) {
        fprintf(stderr,
                "Tile count %d does not fill the declared final row of %d x %d grid\n",
                count, cols, rows);
        unref_images(images, count);
        vips_shutdown();
        return 2;
    }

    double bg_value = (double)bg;
    VipsArrayDouble *background = vips_array_double_new(&bg_value, 1);
    VipsImage *joined = NULL;
    if (vips_arrayjoin(images, &joined, count,
                       "across", cols,
                       "shim", margin_px,
                       "background", background,
                       NULL)) {
        fprintf(stderr, "vips_arrayjoin failed: %s\n", vips_error_buffer());
        vips_area_unref(VIPS_AREA(background));
        unref_images(images, count);
        vips_shutdown();
        return 1;
    }
    unref_images(images, count);

    VipsImage *canvas = joined;
    if (margin_px > 0) {
        if (vips_embed(joined, &canvas,
                       margin_px, margin_px, canvas_w, canvas_h,
                       "extend", VIPS_EXTEND_BACKGROUND,
                       "background", background,
                       NULL)) {
            fprintf(stderr, "vips_embed failed: %s\n", vips_error_buffer());
            vips_area_unref(VIPS_AREA(background));
            g_object_unref(joined);
            vips_shutdown();
            return 1;
        }
        g_object_unref(joined);
    }
    vips_area_unref(VIPS_AREA(background));

    VipsImage *output = NULL;
    if (vips_more_const1(canvas, &output, 0.0, NULL)) {
        fprintf(stderr, "final bilevel threshold failed: %s\n", vips_error_buffer());
        g_object_unref(canvas);
        vips_shutdown();
        return 1;
    }
    g_object_unref(canvas);

    if (output->Xsize != canvas_w || output->Ysize != canvas_h
            || output->Bands != 1 || output->BandFmt != VIPS_FORMAT_UCHAR) {
        fprintf(stderr,
                "unexpected output geometry/format: %dx%dx%d format=%d, expected %dx%dx1 uchar\n",
                output->Xsize, output->Ysize, output->Bands,
                (int)output->BandFmt, canvas_w, canvas_h);
        g_object_unref(output);
        vips_shutdown();
        return 1;
    }

    if (vips_pngsave(output, output_path,
                     "compression", 6,
                     "bitdepth", 1,
                     NULL)) {
        fprintf(stderr, "vips_pngsave(bitdepth=1) failed: %s\n", vips_error_buffer());
        g_object_unref(output);
        vips_shutdown();
        return 1;
    }
    g_object_unref(output);

    struct stat out_stat;
    if (stat(output_path, &out_stat) != 0 || out_stat.st_size <= 0) {
        fprintf(stderr, "Cannot verify output PNG %s: %s\n",
                output_path, errno ? strerror(errno) : "empty file");
        vips_shutdown();
        return 1;
    }
    long long file_size = (long long)out_stat.st_size;
    printf("{\"width\":%d,\"height\":%d,\"tiles\":%d,"
           "\"cols\":%d,\"rows\":%d,\"bitdepth\":1,\"file_size\":%lld}\n",
           canvas_w, canvas_h, count, cols, rows, file_size);

    vips_shutdown();
    return 0;
}
