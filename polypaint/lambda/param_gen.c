/*
 * param_gen: generate transformed parameter pairs as raw f32 output.
 *
 * Uses the EXACT same param transform pipeline as sweep_cli.c.
 * Reads JSON spec from stdin, writes raw f32 pairs (z1r, z1i, z2r, z2i)
 * to output file.
 *
 * Usage:
 *   echo '{"n1":500,"n2":500,"param_transforms":[["unit_circle"]]}' | param_gen output.bin
 *
 * Output: raw f32 pairs, 4 floats per grid point (z1r, z1i, z2r, z2i).
 * Total size: n1 * n2 * 4 * sizeof(float) bytes.
 *
 * JSON metadata to stdout: {"n_points":..., "data_bytes":...}
 *
 * Build (static):
 *   aarch64-linux-musl-gcc -O3 -static -o param_gen param_gen.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ---- Minimal JSON parsing (extracted from sweep_cli.c) ---- */

static const char *skip(const char *p) {
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    return p;
}

static const char *findKey(const char *json, const char *key) {
    char needle[128];
    snprintf(needle, sizeof(needle), "\"%s\"", key);
    const char *p = strstr(json, needle);
    if (!p) return NULL;
    p += strlen(needle);
    p = skip(p);
    if (*p == ':') p++;
    return skip(p);
}

static int jsonInt(const char *json, const char *key, int def) {
    const char *v = findKey(json, key);
    return v ? atoi(v) : def;
}

/* ---- RNG (same as sweep_cli) ---- */

static uint64_t rng_state = 0x12345678ABCDEF01ULL;
static double rng_uniform(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 7;
    rng_state ^= rng_state << 17;
    return (rng_state >> 11) * (1.0 / (double)(1ULL << 53));
}

/* ---- Parameter transforms (copied from sweep_cli.c) ---- */

typedef void (*ParamTransform)(double *z1r, double *z1i, double *z2r, double *z2i);

static void pt_none(double *z1r, double *z1i, double *z2r, double *z2i) {
    (void)z1r; (void)z1i; (void)z2r; (void)z2i;
}
static void pt_unit_circle(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a1 = 2.0 * M_PI * (*z1r), a2 = 2.0 * M_PI * (*z2r);
    *z1r = cos(a1); *z1i = sin(a1);
    *z2r = cos(a2); *z2i = sin(a2);
}
static void pt_square(double *z1r, double *z1i, double *z2r, double *z2i) {
    double r, i;
    r = (*z1r)*(*z1r) - (*z1i)*(*z1i); i = 2.0*(*z1r)*(*z1i);
    *z1r = r; *z1i = i;
    r = (*z2r)*(*z2r) - (*z2i)*(*z2i); i = 2.0*(*z2r)*(*z2i);
    *z2r = r; *z2i = i;
}
static void pt_cube(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a, b, r, i;
    a = *z1r; b = *z1i;
    r = a*a*a - 3.0*a*b*b; i = 3.0*a*a*b - b*b*b;
    *z1r = r; *z1i = i;
    a = *z2r; b = *z2i;
    r = a*a*a - 3.0*a*b*b; i = 3.0*a*a*b - b*b*b;
    *z2r = r; *z2i = i;
}
static void pt_reciprocal(double *z1r, double *z1i, double *z2r, double *z2i) {
    double d;
    d = (*z1r)*(*z1r) + (*z1i)*(*z1i);
    if (d > 1e-30) { *z1r = (*z1r)/d; *z1i = -(*z1i)/d; } else { *z1r = 0; *z1i = 0; }
    d = (*z2r)*(*z2r) + (*z2i)*(*z2i);
    if (d > 1e-30) { *z2r = (*z2r)/d; *z2i = -(*z2i)/d; } else { *z2r = 0; *z2i = 0; }
}
static void pt_conjugate(double *z1r, double *z1i, double *z2r, double *z2i) {
    *z1i = -(*z1i); *z2i = -(*z2i);
}
static void pt_swap(double *z1r, double *z1i, double *z2r, double *z2i) {
    double tr = *z1r, ti = *z1i;
    *z1r = *z2r; *z1i = *z2i;
    *z2r = tr; *z2i = ti;
}
static void pt_add_sub(double *z1r, double *z1i, double *z2r, double *z2i) {
    double ar = *z1r, ai = *z1i, br = *z2r, bi = *z2i;
    *z1r = ar + br; *z1i = ai + bi;
    *z2r = ar - br; *z2i = ai - bi;
}
static void pt_mul_div(double *z1r, double *z1i, double *z2r, double *z2i) {
    double ar = *z1r, ai = *z1i, br = *z2r, bi = *z2i;
    *z1r = ar*br - ai*bi; *z1i = ar*bi + ai*br;
    double d = br*br + bi*bi;
    if (d > 1e-30) { *z2r = (ar*br + ai*bi)/d; *z2i = (ai*br - ar*bi)/d; }
    else { *z2r = 0; *z2i = 0; }
}
static void pt_moebius(double *z1r, double *z1i, double *z2r, double *z2i) {
    double d, ar, ai;
    ar = *z1r + 2.0; ai = *z1i; d = ar*ar + ai*ai;
    if (d > 1e-30) { *z1r = ar/d; *z1i = -ai/d; } else { *z1r = 0; *z1i = 0; }
    ar = *z2r + 2.0; ai = *z2i; d = ar*ar + ai*ai;
    if (d > 1e-30) { *z2r = ar/d; *z2i = -ai/d; } else { *z2r = 0; *z2i = 0; }
}
static void pt_shift1(double *z1r, double *z1i, double *z2r, double *z2i) {
    *z1r += 1.0; *z2r += 1.0; (void)z1i; (void)z2i;
}
static void pt_scale10(double *z1r, double *z1i, double *z2r, double *z2i) {
    *z1r *= 10.0; *z1i *= 10.0; *z2r *= 10.0; *z2i *= 10.0;
}
static void pt_negate(double *z1r, double *z1i, double *z2r, double *z2i) {
    *z1r = -(*z1r); *z1i = -(*z1i); *z2r = -(*z2r); *z2i = -(*z2i);
}
static void pt_zz(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a = *z1r, b = *z2r;
    *z1r = a; *z1i = b; *z2r = b; *z2i = a;
}
static void pt_exp(double *z1r, double *z1i, double *z2r, double *z2i) {
    double e, r, i;
    e = exp(*z1r); r = e * cos(*z1i); i = e * sin(*z1i); *z1r = r; *z1i = i;
    e = exp(*z2r); r = e * cos(*z2i); i = e * sin(*z2i); *z2r = r; *z2i = i;
}

/* Complex helpers */
static inline void c_mul_h(double ar, double ai, double br, double bi, double *rr, double *ri) {
    *rr = ar*br - ai*bi; *ri = ar*bi + ai*br;
}
static inline void c_div_h(double ar, double ai, double br, double bi, double *rr, double *ri) {
    double d = br*br + bi*bi;
    if (d < 1e-30) { *rr = 0; *ri = 0; return; }
    *rr = (ar*br + ai*bi)/d; *ri = (ai*br - ar*bi)/d;
}
static inline void c_sin_h(double ar, double ai, double *rr, double *ri) {
    *rr = sin(ar)*cosh(ai); *ri = cos(ar)*sinh(ai);
}
static inline void c_cos_h(double ar, double ai, double *rr, double *ri) {
    *rr = cos(ar)*cosh(ai); *ri = -sin(ar)*sinh(ai);
}

/* coeff transforms used as param transforms */
static void pt_coeff2(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a1r=*z1r, a1i=*z1i, a2r=*z2r, a2i=*z2i, pr, pi;
    c_mul_h(a1r, a1i, a2r, a2i, &pr, &pi);
    *z1r = a1r + a2r; *z1i = a1i + a2i; *z2r = pr; *z2i = pi;
}
static void pt_coeff3(double *z1r, double *z1i, double *z2r, double *z2i) {
    c_div_h(1,0, *z1r+2, *z1i, z1r, z1i);
    c_div_h(1,0, *z2r+2, *z2i, z2r, z2i);
}
static void pt_coeff3a(double *z1r, double *z1i, double *z2r, double *z2i) {
    c_div_h(1,0, *z1r+1, *z1i, z1r, z1i);
    c_div_h(1,0, *z2r+1, *z2i, z2r, z2i);
}
static void pt_coeff4(double *z1r, double *z1i, double *z2r, double *z2i) {
    double rr, ri;
    c_cos_h(*z1r, *z1i, &rr, &ri); *z1r = rr; *z1i = ri;
    c_sin_h(*z2r, *z2i, &rr, &ri); *z2r = rr; *z2i = ri;
}
static void pt_coeff5(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a1r=*z1r, a1i=*z1i, a2r=*z2r, a2i=*z2i, rr, ri;
    c_div_h(1,0, a2r, a2i, &rr, &ri); *z1r = a1r+rr; *z1i = a1i+ri;
    c_div_h(1,0, a1r, a1i, &rr, &ri); *z2r = a2r+rr; *z2i = a2i+ri;
}
static void pt_coeff5a(double *z1r, double *z1i, double *z2r, double *z2i) {
    double a1r=*z1r, a1i=*z1i, a2r=*z2r, a2i=*z2i, rr, ri;
    c_div_h(1,0, a1r, a1i, &rr, &ri); *z1r = a1r+rr; *z1i = a1i+ri;
    c_div_h(1,0, a2r, a2i, &rr, &ri); *z2r = a2r+rr; *z2i = a2i+ri;
}
static void pt_coeff6(double *z1r, double *z1i, double *z2r, double *z2i) {
    double tr, ti, s2r, s2i, c3r, c3i;
    tr=*z1r; ti=*z1i; c_mul_h(tr,ti,tr,ti,&s2r,&s2i); c_mul_h(s2r,s2i,tr,ti,&c3r,&c3i);
    c_div_h(c3r, c3i+1, c3r, c3i-1, z1r, z1i);
    tr=*z2r; ti=*z2i; c_mul_h(tr,ti,tr,ti,&s2r,&s2i); c_mul_h(s2r,s2i,tr,ti,&c3r,&c3i);
    c_div_h(c3r, c3i+1, c3r, c3i-1, z2r, z2i);
}
static void pt_coeff7(double *z1r, double *z1i, double *z2r, double *z2i) {
    double sr, si, cr, ci, tr, ti;
    tr=*z1r; ti=*z1i; c_sin_h(tr,ti,&sr,&si); c_cos_h(tr,ti,&cr,&ci);
    c_div_h(tr+sr, ti+si, tr+cr, ti+ci, z1r, z1i);
    tr=*z2r; ti=*z2i; c_sin_h(tr,ti,&sr,&si); c_cos_h(tr,ti,&cr,&ci);
    c_div_h(tr+sr, ti+si, tr+cr, ti+ci, z2r, z2i);
}

static ParamTransform lookupParamTransform(const char *name) {
    if (strcmp(name, "none") == 0)        return pt_none;
    if (strcmp(name, "unit_circle") == 0) return pt_unit_circle;
    if (strcmp(name, "square") == 0)      return pt_square;
    if (strcmp(name, "cube") == 0)        return pt_cube;
    if (strcmp(name, "reciprocal") == 0)  return pt_reciprocal;
    if (strcmp(name, "conjugate") == 0)   return pt_conjugate;
    if (strcmp(name, "swap") == 0)        return pt_swap;
    if (strcmp(name, "add_sub") == 0)     return pt_add_sub;
    if (strcmp(name, "mul_div") == 0)     return pt_mul_div;
    if (strcmp(name, "moebius") == 0)     return pt_moebius;
    if (strcmp(name, "shift1") == 0)      return pt_shift1;
    if (strcmp(name, "scale10") == 0)     return pt_scale10;
    if (strcmp(name, "negate") == 0)      return pt_negate;
    if (strcmp(name, "zz") == 0)          return pt_zz;
    if (strcmp(name, "exp") == 0)         return pt_exp;
    if (strcmp(name, "coeff2") == 0)     return pt_coeff2;
    if (strcmp(name, "coeff3") == 0)     return pt_coeff3;
    if (strcmp(name, "coeff3a") == 0)    return pt_coeff3a;
    if (strcmp(name, "coeff4") == 0)     return pt_coeff4;
    if (strcmp(name, "coeff5") == 0)     return pt_coeff5;
    if (strcmp(name, "coeff5a") == 0)    return pt_coeff5a;
    if (strcmp(name, "coeff6") == 0)     return pt_coeff6;
    if (strcmp(name, "coeff7") == 0)     return pt_coeff7;
    return NULL;
}

/* ---- PtEntry parsing and dispatch (from sweep_cli.c) ---- */

#define MAX_PT_ARGS 4
#define MAX_PT_CHAIN 16

typedef struct {
    char name[64];
    double args[MAX_PT_ARGS];
    int nArgs;
} PtEntry;

static int parsePtChain(const char *p, PtEntry *entries, int maxCount) {
    p = skip(p);
    if (*p != '[') return 0;
    p++;
    int count = 0;
    while (count < maxCount) {
        p = skip(p);
        if (*p == ']') break;
        if (*p == ',') { p++; p = skip(p); }
        if (*p != '[') break;
        p++;
        p = skip(p);
        if (*p != '"') break;
        p++;
        int i = 0;
        while (*p && *p != '"' && i < 63) entries[count].name[i++] = *p++;
        entries[count].name[i] = '\0';
        if (*p == '"') p++;
        entries[count].nArgs = 0;
        while (entries[count].nArgs < MAX_PT_ARGS) {
            p = skip(p);
            if (*p == ']') break;
            if (*p == ',') { p++; p = skip(p); }
            if (*p == '"') {
                p++;
                char tmp[64]; int j = 0;
                while (*p && *p != '"' && j < 63) tmp[j++] = *p++;
                tmp[j] = '\0';
                if (*p == '"') p++;
                entries[count].args[entries[count].nArgs++] = atof(tmp);
            } else if (*p == '-' || (*p >= '0' && *p <= '9')) {
                entries[count].args[entries[count].nArgs++] = atof(p);
                while (*p && *p != ',' && *p != ']') p++;
            } else break;
        }
        p = skip(p);
        if (*p == ']') p++;
        count++;
    }
    return count;
}

static void dispatchPt(const PtEntry *e, double *z1r, double *z1i, double *z2r, double *z2i, int gridN) {
    if (strcmp(e->name, "sdith") == 0) {
        double d = e->nArgs > 0 ? e->args[0] : 1.0;
        if (d <= 0.0) d = 1.0;
        double w = 1.0 / (d * gridN);
        *z1r += w * (rng_uniform() - 0.5);
        *z2r += w * (rng_uniform() - 0.5);
        return;
    }
    if (strcmp(e->name, "radd") == 0) {
        double v = e->nArgs > 0 ? e->args[0] : 0.0;
        *z1r += v; *z2r += v; return;
    }
    if (strcmp(e->name, "iadd") == 0) {
        double v = e->nArgs > 0 ? e->args[0] : 0.0;
        *z1i += v; *z2i += v; return;
    }
    if (strcmp(e->name, "add") == 0) {
        double v = e->nArgs > 0 ? e->args[0] : 0.0;
        *z1r += v; *z1i += v; *z2r += v; *z2i += v; return;
    }
    if (strcmp(e->name, "cadd") == 0) {
        double re = e->nArgs > 0 ? e->args[0] : 0.0;
        double im = e->nArgs > 1 ? e->args[1] : 0.0;
        *z1r += re; *z1i += im; *z2r += re; *z2i += im; return;
    }
    if (strcmp(e->name, "rscale") == 0) {
        double v = e->nArgs > 0 ? e->args[0] : 1.0;
        *z1r *= v; *z2r *= v; return;
    }
    if (strcmp(e->name, "iscale") == 0) {
        double v = e->nArgs > 0 ? e->args[0] : 1.0;
        *z1i *= v; *z2i *= v; return;
    }
    if (strcmp(e->name, "scale") == 0) {
        double v = e->nArgs > 0 ? e->args[0] : 1.0;
        *z1r *= v; *z1i *= v; *z2r *= v; *z2i *= v; return;
    }
    if (strcmp(e->name, "rtheta") == 0) {
        double r1 = *z1r, r2 = *z2r;
        double a1 = 2.0 * M_PI * *z2r, a2 = 2.0 * M_PI * *z1r;
        *z1r = r1 * cos(a1); *z1i = r1 * sin(a1);
        *z2r = r2 * cos(a2); *z2i = r2 * sin(a2);
        return;
    }
    ParamTransform fn = lookupParamTransform(e->name);
    if (fn) fn(z1r, z1i, z2r, z2i);
    else fprintf(stderr, "Unknown param transform: %s\n", e->name);
}

/* ---- Main ---- */

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: echo '{...}' | param_gen output.bin\n");
        return 1;
    }
    const char *outPath = argv[1];

    /* Read JSON from stdin */
    char buf[65536];
    int len = fread(buf, 1, sizeof(buf) - 1, stdin);
    buf[len] = '\0';

    int n1 = jsonInt(buf, "n1", 500);
    int n2 = jsonInt(buf, "n2", 500);

    /* Parse param_transforms */
    PtEntry ptChain[MAX_PT_CHAIN];
    int nPt = 0;
    const char *ptJson = findKey(buf, "param_transforms");
    if (ptJson) nPt = parsePtChain(ptJson, ptChain, MAX_PT_CHAIN);

    /* Generate transformed params and write as f32 */
    long nPoints = (long)n1 * n2;
    FILE *fout = fopen(outPath, "wb");
    if (!fout) { fprintf(stderr, "Cannot open %s\n", outPath); return 1; }

    for (int i1 = 0; i1 < n1; i1++) {
        for (int i2 = 0; i2 < n2; i2++) {
            double z1r = (double)i1 / n1, z1i = 0;
            double z2r = (double)i2 / n2, z2i = 0;

            for (int t = 0; t < nPt; t++)
                dispatchPt(&ptChain[t], &z1r, &z1i, &z2r, &z2i, n1);

            float out[4] = { (float)z1r, (float)z1i, (float)z2r, (float)z2i };
            fwrite(out, sizeof(float), 4, fout);
        }
    }
    fclose(fout);

    long dataBytes = nPoints * 4 * sizeof(float);
    printf("{\"n_points\":%ld,\"data_bytes\":%ld,\"n1\":%d,\"n2\":%d,\"n_transforms\":%d}\n",
           nPoints, dataBytes, n1, n2, nPt);
    return 0;
}
