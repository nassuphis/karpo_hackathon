/*
 * palette_custom_probe — exercises palette_lut.h's custom hex-stop
 * palette parsing exactly as the render binaries see it (findPalette +
 * paletteRGB). Driven by tests/test_palette_custom.py.
 *
 * Usage: palette_custom_probe <palette_name> <t0> [t1 ...]
 * Prints: name=<resolved name> n=<stops> then one "r,g,b" line per t.
 * Or:    palette_custom_probe --valid <palette_name>
 * Prints: valid=<0|1>  (paletteNameIsValid — the strict binaries' check)
 */
#include <stdio.h>
#include <stdlib.h>
#include "../lambda/palette_lut.h"

int main(int argc, char **argv) {
    if (argc == 3 && strcmp(argv[1], "--valid") == 0) {
        printf("valid=%d\n", paletteNameIsValid(argv[2]));
        return 0;
    }
    if (argc < 3) {
        fprintf(stderr, "usage: %s <palette> <t...>\n", argv[0]);
        return 2;
    }
    const PaletteDef *pal = findPalette(argv[1]);
    printf("name=%s n=%d\n", pal->name, pal->n_colors);
    for (int i = 2; i < argc; i++) {
        double t = atof(argv[i]);
        unsigned char r, g, b;
        paletteRGB(pal, t, &r, &g, &b);
        printf("%d,%d,%d\n", r, g, b);
    }
    return 0;
}
