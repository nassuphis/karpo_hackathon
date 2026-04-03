# Image Text Overlay — Embedded Bitmap Font

Status: proposed.

## Goal

Burn parameter labels directly into rendered images (JPEG/PNG) using a tiny embedded bitmap font in C. No external dependencies, no Lambda layers, no ImageMagick.

Example label:

```
poly_450 N=2000 CM crowding q=5% reef 4096x4096
```

## Approach

Embed an 8x8 monospace bitmap font covering printable ASCII (32–126) as a static `uint8_t` array in a shared C header. Each glyph is 8 bytes — one byte per row, MSB-first. Total: 95 glyphs × 8 bytes = 760 bytes of static data.

## Font Data

New file: `lambda/font8x8.h`

```c
#ifndef FONT8X8_H
#define FONT8X8_H

/* 8x8 bitmap font for printable ASCII 32..126.
 * Each glyph: 8 bytes, one per row, MSB = leftmost pixel. */
static const unsigned char FONT8X8[95][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* 32 ' ' */
    {0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x00}, /* 33 '!' */
    /* ... 93 more glyphs ... */
};

#endif
```

Source: any public-domain 8x8 bitmap font (e.g. IBM CGA/EGA, cp437). The generator should emit the array from a known font bitmap.

## Rendering API

New file: `lambda/text_overlay.h`

```c
#ifndef TEXT_OVERLAY_H
#define TEXT_OVERLAY_H

#include "font8x8.h"

/* Draw a single character into an RGB888 buffer.
 *
 * buf:    RGB888 pixel buffer (3 bytes per pixel, row-major)
 * stride: bytes per row (width * 3)
 * bx, by: top-left pixel position of the glyph
 * ch:     ASCII character to draw
 * r,g,b:  foreground color
 * W, H:   buffer dimensions (for bounds checking)
 */
static void draw_char(unsigned char *buf, int stride, int bx, int by,
                      char ch, unsigned char r, unsigned char g, unsigned char b,
                      int W, int H) {
    int idx = (int)ch - 32;
    if (idx < 0 || idx > 94) return;
    const unsigned char *glyph = FONT8X8[idx];
    for (int row = 0; row < 8; row++) {
        int py = by + row;
        if (py < 0 || py >= H) continue;
        unsigned char bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (!(bits & (0x80 >> col))) continue;
            int px = bx + col;
            if (px < 0 || px >= W) continue;
            int off = py * stride + px * 3;
            buf[off] = r;
            buf[off + 1] = g;
            buf[off + 2] = b;
        }
    }
}

/* Draw a null-terminated string into an RGB888 buffer.
 *
 * x, y: top-left pixel position of the first character
 * Advances 8 pixels per character. No line wrapping.
 */
static void draw_text(unsigned char *buf, int stride, int x, int y,
                      const char *text, unsigned char r, unsigned char g, unsigned char b,
                      int W, int H) {
    for (int i = 0; text[i]; i++) {
        draw_char(buf, stride, x + i * 8, y, text[i], r, g, b, W, H);
    }
}

/* Draw text with a 1px dark outline for readability on any background.
 *
 * Draws the string 8 times offset by ±1 pixel in dark, then once in foreground.
 */
static void draw_text_outlined(unsigned char *buf, int stride, int x, int y,
                               const char *text,
                               unsigned char fr, unsigned char fg, unsigned char fb,
                               unsigned char br, unsigned char bg, unsigned char bb,
                               int W, int H) {
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            draw_text(buf, stride, x + dx, y + dy, text, br, bg, bb, W, H);
        }
    }
    draw_text(buf, stride, x, y, text, fr, fg, fb, W, H);
}

#endif
```

## Integration Points

The text overlay should be applied in the C binaries that produce raw RGB buffers, just before the buffer is written to disk or piped to the encoder.

### `palette_bins_render`

Currently writes an N×N raw RGB buffer from palette bin data.

Change: after rendering all pixels, call `draw_text_outlined` at the bottom-left corner with the label string.

Label source: new CLI argument `--label="..."`.

### `pixassemble`

Assembles tile .pix files into a single raw RGB tile buffer.

Change: after assembly, if `--label` is provided, draw the text. Only makes sense for the first tile (tile 0) or for a post-assembly step.

However, tiles are assembled per-tile, not per-image. The label should go on the final composed image. Since there's no single-image composition step in the current pipeline (tiles go straight to encode), this is not the right place for color renders.

### `raw2jpeg` / `encode`

These read a raw RGB buffer, encode to JPEG/PNG, and upload. The raw buffer is available before encoding.

Change: if `--label` is provided, read the raw buffer, draw text, then encode. This works for both palette images (via `raw2jpeg`) and color renders (via the encode Lambda).

**Recommended primary integration point: `raw2jpeg`**, since it's used by both palette finalize and color encode, and it already has the raw buffer in memory.

### Encode Lambda (color renders)

The encode Lambda calls `pixassemble` per tile, then uploads. For multi-tile images, the label would need to go on tile 0 (top-left or bottom-left). The `pixassemble` binary could accept `--label` and only draw it when assembling tile 0.

Alternative: add the label after all tiles are assembled, in a lightweight post-composition step. But this adds complexity.

**Simplest approach**: pass `--label` to `pixassemble` only for the tile that covers the bottom-left corner (tile index where tile_row == n_tile_rows - 1 and tile_col == 0).

## Label Content

The label string is built by the Lambda handler from render parameters:

```
{function} N={N} {solver} {color_mode} {metric} q={q}% w={omega} {palette} {pix}x{pix}
```

Examples:

```
poly_450 N=2000 CM solve_score crowding q=5.0% w=1 reef 4096x4096
giga_19 N=4000 AE proximity inferno 8192x8192
poly_1 N=500 AE rainbow 2048x2048
```

For palette images:

```
crowding q=5.0% w=1 tri_redgold 2000x2000
```

The handler constructs the label and passes it as a CLI argument. The C binary does not need to know what the fields mean — it just draws the string.

## Label Placement

Bottom-left corner, 8 pixels from the edge:

```
x = 8
y = H - 16
```

This keeps the label out of the visual center and readable on dark backgrounds (which most renders have). The outlined variant ensures readability on light backgrounds too.

For palette images (N×N, typically 2000–8000): 8x8 font is small but legible at full zoom. For large renders (8192+), it's subtle but visible when zoomed in.

## Scaling

For images larger than 4096, a 2x scaled font (16x16) may be more readable:

```c
static void draw_text_2x(unsigned char *buf, int stride, int x, int y,
                         const char *text, unsigned char r, unsigned char g, unsigned char b,
                         int W, int H) {
    for (int i = 0; text[i]; i++) {
        int idx = (int)text[i] - 32;
        if (idx < 0 || idx > 94) continue;
        const unsigned char *glyph = FONT8X8[idx];
        for (int row = 0; row < 8; row++) {
            unsigned char bits = glyph[row];
            for (int col = 0; col < 8; col++) {
                if (!(bits & (0x80 >> col))) continue;
                for (int sy = 0; sy < 2; sy++) {
                    for (int sx = 0; sx < 2; sx++) {
                        int px = x + i * 16 + col * 2 + sx;
                        int py = y + row * 2 + sy;
                        if (px >= 0 && px < W && py >= 0 && py < H) {
                            int off = py * stride + px * 3;
                            buf[off] = r; buf[off+1] = g; buf[off+2] = b;
                        }
                    }
                }
            }
        }
    }
}
```

Selection rule: use 2x if `min(W, H) > 4096`, else 1x.

## File Changes

### New files

- `lambda/font8x8.h` — static glyph data (760 bytes + boilerplate)
- `lambda/text_overlay.h` — `draw_text`, `draw_text_outlined`, `draw_text_2x`

### Modified files

- `lambda/palette_bins_render.c` — `#include "text_overlay.h"`, accept `--label`, draw after render
- `lambda/raw2jpeg` or encode path — accept `--label`, draw before encode

### Handler changes

- `lambda/handler_palette_finalize.py` — build label string from palette params, pass `--label` to `palette_bins_render`
- `lambda/handler_finalize.py` or `lambda/handler_raster.py` — build label string, pass to encode/pixassemble

### No Lambda layer changes

Everything is compiled into the existing binaries. No new dependencies.

## Non-Goals

- Arbitrary font selection
- Multiline text layout
- Unicode support
- Anti-aliased rendering
- TrueType / FreeType
- Runtime font loading

The 8x8 bitmap font is permanent and sufficient for parameter labels.
