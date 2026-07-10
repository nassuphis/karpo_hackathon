#!/bin/bash
# wall_dz docker gate (deepzoom-speed.md §7.1): compile the composite-wall
# pyramid tool against the real libvips layer on arm64 amazonlinux and prove
# it produces an OpenSeadragon-compatible jpg pyramid with correct geometry.
# Required whenever wall_dz.c or the layer recipe changes.
#
# Coverage (code-review-28 F15/F19/F20): the synthesized cells are a genuine
# MIX of pixel formats — grayscale, sRGB, RGBA (with alpha), and CMYK — to
# prove the band-normalisation path actually flattens/joins them instead of
# failing arrayjoin on mismatched bands. A second pass with a tiny JPEG cap
# exercises the large-wall flat-JPEG skip without allocating a 65k² image.
set -euo pipefail
cd "$(dirname "$0")/.."

LAYER_BUILD="$PWD/lambda/layer-build"
if [ ! -d "$LAYER_BUILD/lib" ]; then
    echo "ERROR: lambda/layer-build missing. Run lambda/build-libvips-layer.sh first."
    exit 1
fi

docker run --rm --platform linux/arm64 \
    -v "$PWD/lambda:/src" \
    -v "$LAYER_BUILD:/opt" \
    public.ecr.aws/amazonlinux/amazonlinux:2023 \
    bash -c '
        set -euo pipefail
        dnf install -y gcc glib2-devel 2>&1 | tail -1
        VIPS_CFLAGS="-I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include"
        VIPS_LIBS="-L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm -Wl,--disable-new-dtags -Wl,-rpath,/opt/lib:/opt/vipsdeps"
        gcc -O3 -o /tmp/wall_dz /src/wall_dz.c $VIPS_CFLAGS $VIPS_LIBS
        export LD_LIBRARY_PATH=/opt/lib

        # Synthesize 5 cells in DIFFERENT pixel formats so the gate proves the
        # normalise path (flatten alpha -> sRGB -> exactly 3 bands), not just
        # the all-grayscale happy path. Cell 2 is a 500px RGBA PNG (small-N
        # preview + alpha) — the exact case that broke arrayjoin before F15.
        cat > /tmp/make_cells.c <<CELLS
#include <vips/vips.h>
#include <stdio.h>
static VipsImage *noise(int sz, double mean) {
    VipsImage *n, *c;
    if (vips_gaussnoise(&n, sz, sz, "mean", mean, "sigma", 40.0, NULL)) return NULL;
    if (vips_cast_uchar(n, &c, NULL)) return NULL;
    g_object_unref(n);
    return c;
}
static int band3(int sz, VipsInterpretation interp, int nbands, const char *path) {
    VipsImage *b[4], *joined, *out;
    for (int i = 0; i < nbands; i++) { b[i] = noise(sz, 70.0 + 35.0 * i); if (!b[i]) return 1; }
    if (vips_bandjoin(b, &joined, nbands, NULL)) return 1;
    if (vips_copy(joined, &out, "interpretation", interp, NULL)) return 1;
    return vips_image_write_to_file(out, path, NULL);
}
int main(int argc, char **argv) {
    if (VIPS_INIT(argv[0])) return 1;
    /* 0: grayscale jpg */
    if (vips_image_write_to_file(noise(512, 128.0), "/tmp/cell_0.jpg", NULL)) return 1;
    /* 1: sRGB (3 bands) jpg */
    if (band3(512, VIPS_INTERPRETATION_sRGB, 3, "/tmp/cell_1.jpg")) return 1;
    /* 2: RGBA (4 bands, alpha) 500px PNG — jpg cannot hold alpha */
    if (band3(500, VIPS_INTERPRETATION_sRGB, 4, "/tmp/cell_2.png")) return 1;
    /* 3: CMYK (4 bands) jpg */
    if (band3(512, VIPS_INTERPRETATION_CMYK, 4, "/tmp/cell_3.jpg")) return 1;
    /* 4: grayscale jpg */
    if (vips_image_write_to_file(noise(512, 160.0), "/tmp/cell_4.jpg", NULL)) return 1;
    vips_shutdown();
    return 0;
}
CELLS
        gcc -O2 -o /tmp/make_cells /tmp/make_cells.c $VIPS_CFLAGS $VIPS_LIBS
        /tmp/make_cells || { echo "FATAL: cell generation failed"; exit 1; }
        # cell 2 is the RGBA png; the rest are jpg
        printf "%s\n" /tmp/cell_0.jpg /tmp/cell_1.jpg /tmp/cell_2.png /tmp/cell_3.jpg /tmp/cell_4.jpg > /tmp/wall_list.txt

        echo "--- pass 1: mixed formats (gray/sRGB/RGBA/CMYK) must all normalise + join (F15) ---"
        OUT=$(/tmp/wall_dz /tmp/wall_list.txt 2 /tmp/wall) || { echo "FATAL: wall_dz failed on mixed formats"; exit 1; }
        echo "  wall_dz: $OUT"
        # 5 cells across=2 -> 3 rows: 1024 x 1536 EXACTLY even with a 500px RGBA
        # cell (proves normalise-to-512 + flatten kept the grid uniform)
        echo "$OUT" | grep -q "\"width\":1024" || { echo "FATAL: wrong width (mixed-format normalise failed?): $OUT"; exit 1; }
        echo "$OUT" | grep -q "\"height\":1536" || { echo "FATAL: wrong height: $OUT"; exit 1; }
        echo "$OUT" | grep -q "\"flat_jpeg\":true" || { echo "FATAL: flat_jpeg should be true for a small wall: $OUT"; exit 1; }
        [ -f /tmp/wall.dzi ] || { echo "FATAL: no .dzi"; exit 1; }
        [ -s /tmp/wall.jpg ] || { echo "FATAL: no flat wall.jpg composite"; exit 1; }
        grep -q "Format=\"jpg\"" /tmp/wall.dzi || { echo "FATAL: dzi is not jpg-tiled"; exit 1; }
        grep -q "TileSize=\"256\"" /tmp/wall.dzi || { echo "FATAL: dzi tile size wrong"; exit 1; }
        TILES=$(find /tmp/wall_files -name "*.jpg" | wc -l)
        [ "$TILES" -gt 10 ] || { echo "FATAL: too few tiles ($TILES)"; exit 1; }
        DEEP=$(ls /tmp/wall_files | sort -n | tail -1)
        DEEP_TILES=$(ls /tmp/wall_files/$DEEP | wc -l)
        [ "$DEEP_TILES" -eq 24 ] || { echo "FATAL: deepest level has $DEEP_TILES tiles, want 24"; exit 1; }
        echo "  mixed-format wall OK: $TILES tiles, deepest level 4x6"

        echo "--- pass 2: wall past the JPEG cap skips the flat jpg but still makes the DZI (F19) ---"
        # Recompile with a tiny cap so the 1024x1536 wall is treated as oversize
        # WITHOUT allocating a real 65k-px image.
        gcc -O3 -DJPEG_MAX_PX=1000 -o /tmp/wall_dz_cap /src/wall_dz.c $VIPS_CFLAGS $VIPS_LIBS
        rm -rf /tmp/wallcap*
        OUT2=$(/tmp/wall_dz_cap /tmp/wall_list.txt 2 /tmp/wallcap) || { echo "FATAL: wall_dz (capped) failed"; exit 1; }
        echo "  wall_dz(cap): $OUT2"
        echo "$OUT2" | grep -q "\"flat_jpeg\":false" || { echo "FATAL: oversize wall should report flat_jpeg:false: $OUT2"; exit 1; }
        [ ! -f /tmp/wallcap.jpg ] || { echo "FATAL: oversize wall must NOT write wall.jpg"; exit 1; }
        [ -f /tmp/wallcap.dzi ] || { echo "FATAL: oversize wall must STILL produce the DZI"; exit 1; }
        grep -q "Format=\"jpg\"" /tmp/wallcap.dzi || { echo "FATAL: capped dzi is not jpg-tiled"; exit 1; }
        echo "  oversize-wall path OK: flat jpg skipped, DZI intact"
    '

echo "=== wall_dz Docker Gate PASSED ==="
