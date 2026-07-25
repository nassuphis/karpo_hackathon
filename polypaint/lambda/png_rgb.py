"""Minimal pure-stdlib PNG decoder for the bake pipeline.

The splat bake needs the per-solve colors that live in a palette PNG this
codebase wrote itself (score_raw_render/vips: 8-bit, non-interlaced, RGB or
RGBA; greyscale tolerated for tiny fixtures). Lambdas here are deliberately
dependency-free — no PIL/numpy — and a PNG is just zlib-deflated, per-line
filtered scanlines, so ~90 lines of stdlib cover our own output. Anything
outside that envelope fails loudly rather than guessing.
"""
import struct
import zlib

_SIGNATURE = b"\x89PNG\r\n\x1a\n"


def decode_png_rgb(data):
    """Decode a PNG into (width, height, rgb_bytes) with rgb_bytes row-major
    RGB8 (alpha stripped, greyscale expanded). Raises RuntimeError on
    anything our own pipeline would never produce."""
    if not data.startswith(_SIGNATURE):
        raise RuntimeError("not a PNG (bad signature)")
    pos = len(_SIGNATURE)
    width = height = None
    bit_depth = color_type = interlace = None
    idat = bytearray()
    while pos + 8 <= len(data):
        length, ctype = struct.unpack_from(">I4s", data, pos)
        pos += 8
        chunk = data[pos:pos + length]
        pos += length + 4                     # skip CRC
        if ctype == b"IHDR":
            width, height, bit_depth, color_type, _comp, _filt, interlace = \
                struct.unpack(">IIBBBBB", chunk)
        elif ctype == b"IDAT":
            idat += chunk
        elif ctype == b"IEND":
            break
    if width is None:
        raise RuntimeError("PNG carries no IHDR")
    if bit_depth != 8:
        raise RuntimeError(f"unsupported PNG bit depth {bit_depth} (need 8)")
    if interlace != 0:
        raise RuntimeError("interlaced PNG unsupported")
    channels = {0: 1, 2: 3, 6: 4}.get(color_type)
    if channels is None:
        raise RuntimeError(f"unsupported PNG color type {color_type} (need grey/RGB/RGBA)")

    raw = zlib.decompress(bytes(idat))
    stride = width * channels
    expected = (stride + 1) * height
    if len(raw) != expected:
        raise RuntimeError(f"PNG scanline data {len(raw)} != expected {expected}")

    out = bytearray(stride * height)
    prev = bytearray(stride)
    for row in range(height):
        base = row * (stride + 1)
        ftype = raw[base]
        line = bytearray(raw[base + 1:base + 1 + stride])
        if ftype == 0:                        # None
            pass
        elif ftype == 1:                      # Sub
            for i in range(channels, stride):
                line[i] = (line[i] + line[i - channels]) & 0xFF
        elif ftype == 2:                      # Up
            for i in range(stride):
                line[i] = (line[i] + prev[i]) & 0xFF
        elif ftype == 3:                      # Average
            for i in range(stride):
                left = line[i - channels] if i >= channels else 0
                line[i] = (line[i] + ((left + prev[i]) >> 1)) & 0xFF
        elif ftype == 4:                      # Paeth
            for i in range(stride):
                left = line[i - channels] if i >= channels else 0
                up = prev[i]
                ul = prev[i - channels] if i >= channels else 0
                p = left + up - ul
                pa, pb, pc = abs(p - left), abs(p - up), abs(p - ul)
                if pa <= pb and pa <= pc:
                    pred = left
                elif pb <= pc:
                    pred = up
                else:
                    pred = ul
                line[i] = (line[i] + pred) & 0xFF
        else:
            raise RuntimeError(f"unsupported PNG filter type {ftype}")
        out[row * stride:(row + 1) * stride] = line
        prev = line

    if channels == 3:
        return width, height, bytes(out)
    rgb = bytearray(width * height * 3)
    if channels == 4:
        for px in range(width * height):
            rgb[px * 3:px * 3 + 3] = out[px * 4:px * 4 + 3]
    else:                                     # greyscale
        for px in range(width * height):
            g = out[px]
            rgb[px * 3] = g
            rgb[px * 3 + 1] = g
            rgb[px * 3 + 2] = g
    return width, height, bytes(rgb)


def encode_png_rgb(width, height, rgb):
    """Tiny filter-0 RGB8 encoder — test fixtures and round-trip pins only."""
    if len(rgb) != width * height * 3:
        raise RuntimeError("encode_png_rgb: rgb size mismatch")
    raw = bytearray()
    for row in range(height):
        raw.append(0)
        raw += rgb[row * width * 3:(row + 1) * width * 3]

    def chunk(ctype, body):
        blob = ctype + body
        return struct.pack(">I", len(body)) + blob + struct.pack(">I", zlib.crc32(blob) & 0xFFFFFFFF)

    ihdr = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    return (_SIGNATURE + chunk(b"IHDR", ihdr)
            + chunk(b"IDAT", zlib.compress(bytes(raw)))
            + chunk(b"IEND", b""))
