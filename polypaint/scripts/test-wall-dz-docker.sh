#!/bin/bash
# wall_dz docker gate (deepzoom-speed.md §7.1): compile the composite-wall
# pyramid tool against the real libvips layer on arm64 amazonlinux and prove
# it produces an OpenSeadragon-compatible jpg pyramid with correct geometry.
# Required whenever wall_dz.c or the layer recipe changes.
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

        # synthesize 5 distinct jpg cells with vips (via a tiny C tool); cell 2
        # is 500px (a small-N render preview) to exercise the normalise-to-512
        # path in wall_dz — the wall must still come out uniform 1024x1536
        cat > /tmp/make_cells.c <<CELLS
#include <vips/vips.h>
int main(int argc, char **argv) {
    if (VIPS_INIT(argv[0])) return 1;
    for (int i = 0; i < 5; i++) {
        VipsImage *noise, *cell;
        int sz = (i == 2) ? 500 : 512;
        if (vips_gaussnoise(&noise, sz, sz, "mean", 90.0 + 30.0 * i, "sigma", 40.0, NULL)) return 1;
        if (vips_cast_uchar(noise, &cell, NULL)) return 1;
        char path[64];
        snprintf(path, sizeof(path), "/tmp/cell_%d.jpg", i);
        if (vips_image_write_to_file(cell, path, NULL)) return 1;
        g_object_unref(noise); g_object_unref(cell);
    }
    vips_shutdown();
    return 0;
}
CELLS
        gcc -O2 -o /tmp/make_cells /tmp/make_cells.c $VIPS_CFLAGS $VIPS_LIBS
        /tmp/make_cells || { echo "FATAL: cell generation failed"; exit 1; }
        for i in 0 1 2 3 4; do echo /tmp/cell_$i.jpg; done > /tmp/wall_list.txt

        OUT=$(/tmp/wall_dz /tmp/wall_list.txt 2 /tmp/wall) || { echo "FATAL: wall_dz failed"; exit 1; }
        echo "  wall_dz: $OUT"
        # 5 cells across=2 -> 3 rows: 1024 x 1536 EXACTLY even though cell 2 is
        # 500px (proves normalise-to-512 kept the grid uniform)
        echo "$OUT" | grep -q "\"width\":1024" || { echo "FATAL: wrong width (500px cell not normalised?): $OUT"; exit 1; }
        echo "$OUT" | grep -q "\"height\":1536" || { echo "FATAL: wrong height: $OUT"; exit 1; }
        [ -f /tmp/wall.dzi ] || { echo "FATAL: no .dzi"; exit 1; }
        [ -s /tmp/wall.jpg ] || { echo "FATAL: no flat wall.jpg composite"; exit 1; }
        grep -q "Format=\"jpg\"" /tmp/wall.dzi || { echo "FATAL: dzi is not jpg-tiled"; exit 1; }
        grep -q "TileSize=\"256\"" /tmp/wall.dzi || { echo "FATAL: dzi tile size wrong"; exit 1; }
        TILES=$(find /tmp/wall_files -name "*.jpg" | wc -l)
        [ "$TILES" -gt 10 ] || { echo "FATAL: too few tiles ($TILES)"; exit 1; }
        # deepest level must be a 4x6 grid of 256px tiles
        DEEP=$(ls /tmp/wall_files | sort -n | tail -1)
        DEEP_TILES=$(ls /tmp/wall_files/$DEEP | wc -l)
        [ "$DEEP_TILES" -eq 24 ] || { echo "FATAL: deepest level has $DEEP_TILES tiles, want 24"; exit 1; }
        echo "  wall_dz gate: $TILES tiles, deepest level 4x6 OK"
    '

echo "=== wall_dz Docker Gate PASSED ==="
