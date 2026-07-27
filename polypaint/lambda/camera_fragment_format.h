#ifndef POLYPAINT_CAMERA_FRAGMENT_FORMAT_H
#define POLYPAINT_CAMERA_FRAGMENT_FORMAT_H

#include <stdint.h>
#include <string.h>

#define CAMERA_FRAGMENT_ENCODING "u32le_f32depth_u8_channels_v1"

static inline void camera_fragment_write_u32le(uint8_t *dst, uint32_t value) {
    dst[0] = (uint8_t)(value & 0xffu);
    dst[1] = (uint8_t)((value >> 8) & 0xffu);
    dst[2] = (uint8_t)((value >> 16) & 0xffu);
    dst[3] = (uint8_t)((value >> 24) & 0xffu);
}

static inline uint32_t camera_fragment_read_u32le(const uint8_t *src) {
    return ((uint32_t)src[0])
        | ((uint32_t)src[1] << 8)
        | ((uint32_t)src[2] << 16)
        | ((uint32_t)src[3] << 24);
}

static inline void camera_fragment_write_f32le(uint8_t *dst, float value) {
    uint32_t bits = 0;
    memcpy(&bits, &value, sizeof(bits));
    camera_fragment_write_u32le(dst, bits);
}

static inline float camera_fragment_read_f32le(const uint8_t *src) {
    uint32_t bits = camera_fragment_read_u32le(src);
    float value = 0.0f;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

#endif
