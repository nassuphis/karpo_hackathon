/* Root transform prepared-vs-unprepared parity probe (CR32 F12).
 *
 * Parses a root_transforms JSON chain (argv[1]) the way every consumer binary
 * does — parse_root_xform_json, which runs rt_prepare_chain — then applies it
 * twice to identical root sets:
 *   A) the prepared entries exactly as parsed (prep_fn dispatch), and
 *   B) a copy with prep_fn zeroed, forcing the legacy per-row fallback that
 *      hand-built chains use.
 * The two float32 results must be bit-identical for every transform, arity,
 * default form, pole, and non-finite input. Exit 0 and {"match":1} on parity.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "root_xforms.h"

#define PROBE_DEGREE 40

static void fill_roots(float *re, float *im) {
    for (int i = 0; i < PROBE_DEGREE; i++) {
        /* deterministic spread crossing the unit circle, both signs */
        double theta = (2.0 * M_PI * i) / PROBE_DEGREE;
        double radius = 0.25 + 0.05 * i;
        re[i] = (float)(radius * cos(theta));
        im[i] = (float)(radius * sin(theta));
    }
    /* policy boundaries: NaN, inf, exact zero, huge, tiny, signed zero */
    re[3] = NAN;          im[3] = 0.5f;
    re[7] = INFINITY;     im[7] = -1.0f;
    re[11] = 0.0f;        im[11] = 0.0f;
    re[13] = 1e30f;       im[13] = -1e30f;
    re[17] = 1e-30f;      im[17] = 1e-30f;
    re[19] = -0.0f;       im[19] = -0.0f;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <chain.json>\n", argv[0]);
        return 2;
    }
    FILE *fh = fopen(argv[1], "rb");
    if (!fh) {
        fprintf(stderr, "cannot open %s\n", argv[1]);
        return 2;
    }
    char buf[65536];
    size_t got = fread(buf, 1, sizeof(buf) - 1, fh);
    fclose(fh);
    buf[got] = '\0';

    RootXformEntry prepared[MAX_RT_CHAIN];
    int n = parse_root_xform_json(buf, prepared, MAX_RT_CHAIN);
    if (n < 0) {
        fprintf(stderr, "parse failed\n");
        return 2;
    }

    RootXformEntry legacy[MAX_RT_CHAIN];
    memcpy(legacy, prepared, sizeof(legacy));
    for (int i = 0; i < n; i++) legacy[i].prep_fn = 0;

    float reA[PROBE_DEGREE], imA[PROBE_DEGREE];
    float reB[PROBE_DEGREE], imB[PROBE_DEGREE];
    fill_roots(reA, imA);
    fill_roots(reB, imB);

    apply_root_xforms(prepared, n, reA, imA, PROBE_DEGREE);
    apply_root_xforms(legacy, n, reB, imB, PROBE_DEGREE);

    int match = 1;
    for (int i = 0; i < PROBE_DEGREE; i++) {
        uint32_t ra, rb, ia, ib;
        memcpy(&ra, &reA[i], 4); memcpy(&rb, &reB[i], 4);
        memcpy(&ia, &imA[i], 4); memcpy(&ib, &imB[i], 4);
        if (ra != rb || ia != ib) {
            fprintf(stderr,
                    "mismatch at root %d: prepared (%.9g, %.9g) legacy (%.9g, %.9g)\n",
                    i, (double)reA[i], (double)imA[i], (double)reB[i], (double)imB[i]);
            match = 0;
        }
    }
    printf("{\"n_entries\":%d,\"match\":%d}\n", n, match);
    return match ? 0 : 1;
}
