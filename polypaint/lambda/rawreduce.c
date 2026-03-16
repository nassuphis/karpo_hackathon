/*
 * rawreduce: gamma-correct additive merge of two raw pixel buffers.
 *
 * Usage:
 *   rawreduce acc.raw next.raw out.raw [--gamma=2.2]
 *
 * Raw format: 12-byte header (uint32 W, H, bands) + pixel data.
 * gamma=0 disables gamma correction (raw saturating add).
 *
 * Build (static, no libvips):
 *   aarch64-linux-musl-gcc -O3 -static -o rawreduce rawreduce.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ---- Parse --key=value from argv ---- */

static const char *getArg(int argc, char **argv, const char *key) {
    int klen = strlen(key);
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], key, klen) == 0 && argv[i][klen] == '=')
            return argv[i] + klen + 1;
    }
    return NULL;
}

static double getArgDouble(int argc, char **argv, const char *key, double def) {
    const char *v = getArg(argc, argv, key);
    return v ? atof(v) : def;
}

/* ---- Raw image I/O (12-byte header: uint32 W, H, bands + pixel data) ---- */

static unsigned char *raw_read(const char *path,
                               unsigned int *w, unsigned int *h, unsigned int *bands) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", path); return NULL; }
    if (fread(w, 4, 1, f) != 1 || fread(h, 4, 1, f) != 1 || fread(bands, 4, 1, f) != 1) {
        fprintf(stderr, "Bad raw header in %s\n", path);
        fclose(f); return NULL;
    }
    size_t n = (size_t)*w * *h * *bands;
    unsigned char *data = malloc(n);
    if (!data) { fprintf(stderr, "Cannot allocate %zu bytes\n", n); fclose(f); return NULL; }
    if (fread(data, 1, n, f) != n) {
        fprintf(stderr, "Short read in %s\n", path);
        free(data); fclose(f); return NULL;
    }
    fclose(f);
    return data;
}

static int raw_write(const char *path, const unsigned char *data,
                     unsigned int w, unsigned int h, unsigned int bands) {
    FILE *f = fopen(path, "wb");
    if (!f) { fprintf(stderr, "Cannot create %s\n", path); return -1; }
    fwrite(&w, 4, 1, f);
    fwrite(&h, 4, 1, f);
    fwrite(&bands, 4, 1, f);
    fwrite(data, 1, (size_t)w * h * bands, f);
    fclose(f);
    return 0;
}

/* ---- Gamma LUTs ---- */

static float srgb2lin[256];
static unsigned char lin2srgb[4096];

static void buildGammaLUT(double gamma) {
    for (int i = 0; i < 256; i++)
        srgb2lin[i] = (float)pow(i / 255.0, gamma);
    double inv_gamma = 1.0 / gamma;
    for (int i = 0; i < 4096; i++) {
        double v = pow(i / 4095.0, inv_gamma) * 255.0;
        lin2srgb[i] = v > 255.0 ? 255 : (unsigned char)(v + 0.5);
    }
}

/* ---- Main ---- */

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: rawreduce acc.raw next.raw out.raw [--gamma=2.2]\n");
        return 1;
    }
    const char *accPath = argv[1];
    const char *nextPath = argv[2];
    const char *outPath = argv[3];
    double gamma = getArgDouble(argc, argv, "--gamma", 2.2);

    /* Load both raw images */
    unsigned int W, H, bands;
    unsigned int W2, H2, bands2;
    unsigned char *accData = raw_read(accPath, &W, &H, &bands);
    if (!accData) return 1;
    unsigned char *nextData = raw_read(nextPath, &W2, &H2, &bands2);
    if (!nextData) { free(accData); return 1; }

    if (W != W2 || H != H2) {
        fprintf(stderr, "Image dimension mismatch: %ux%u vs %ux%u\n", W, H, W2, H2);
        free(accData); free(nextData);
        return 1;
    }

    size_t n = (size_t)W * H * bands;

    if (gamma > 0.01) {
        /* Gamma-correct blending via LUTs */
        buildGammaLUT(gamma);
        for (size_t i = 0; i < n; i++) {
            float sum = srgb2lin[accData[i]] + srgb2lin[nextData[i]];
            if (sum >= 1.0f) {
                accData[i] = 255;
            } else {
                int idx = (int)(sum * 4095.0f + 0.5f);
                if (idx > 4095) idx = 4095;
                accData[i] = lin2srgb[idx];
            }
        }
    } else {
        /* Raw saturating add (gamma=0) */
        for (size_t i = 0; i < n; i++) {
            int v = accData[i] + nextData[i];
            accData[i] = v > 255 ? 255 : (unsigned char)v;
        }
    }

    /* Write result */
    if (raw_write(outPath, accData, W, H, bands) != 0) {
        free(accData); free(nextData);
        return 1;
    }

    free(accData);
    free(nextData);

    printf("{\"status\":\"ok\",\"width\":%u,\"height\":%u,\"gamma\":%.2f}\n", W, H, gamma);
    return 0;
}
