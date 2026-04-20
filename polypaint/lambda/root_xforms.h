/*
 * root_xforms.h — shared root transform engine for render-time geometry transforms.
 *
 * Applied in-flight during rasterization. No data saved back to S3.
 * Used by both roots2pix_mt.c (fused color) and bilevel_raster.c (bilevel).
 *
 * Usage:
 *   RootXformEntry chain[MAX_RT_CHAIN];
 *   int nrt = parse_root_xform_file("/tmp/rt.json", chain, MAX_RT_CHAIN);
 *   // per sample:
 *   apply_root_xforms(chain, nrt, roots_re, roots_im, degree);
 */

#ifndef ROOT_XFORMS_H
#define ROOT_XFORMS_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_RT_CHAIN 16
#define MAX_RT_ARGS 8

typedef struct {
    char name[64];
    double args[MAX_RT_ARGS];
    int n_args;
} RootXformEntry;

/* ---- Parsing ---- */

static const char *rt_skip(const char *p) {
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    return p;
}

/* Parse root_transforms JSON: [["rotate_roots","0.5"],["pull_unit_circle","0.75","1.0"]] */
static int parse_root_xform_json(const char *p, RootXformEntry *entries, int maxCount) {
    p = rt_skip(p);
    if (*p != '[') return 0;
    p++;
    int count = 0;
    while (count < maxCount) {
        p = rt_skip(p);
        if (*p == ']') break;
        if (*p == ',') { p++; p = rt_skip(p); }
        if (*p != '[') break;
        p++;
        p = rt_skip(p);
        if (*p != '"') break;
        p++;
        int i = 0;
        while (*p && *p != '"' && i < 63) entries[count].name[i++] = *p++;
        entries[count].name[i] = '\0';
        if (*p == '"') p++;
        entries[count].n_args = 0;
        while (entries[count].n_args < MAX_RT_ARGS) {
            p = rt_skip(p);
            if (*p == ']') break;
            if (*p == ',') { p++; p = rt_skip(p); }
            if (*p == '"') {
                p++;
                char tmp[64]; int j = 0;
                while (*p && *p != '"' && j < 63) tmp[j++] = *p++;
                tmp[j] = '\0';
                if (*p == '"') p++;
                entries[count].args[entries[count].n_args++] = atof(tmp);
            } else if (*p == '-' || (*p >= '0' && *p <= '9')) {
                entries[count].args[entries[count].n_args++] = atof(p);
                while (*p && *p != ',' && *p != ']') p++;
            } else break;
        }
        p = rt_skip(p);
        if (*p == ']') p++;
        count++;
    }
    return count;
}

/* Parse from a JSON file */
static int parse_root_xform_file(const char *path, RootXformEntry *entries, int maxCount) {
    if (!path || !path[0]) return 0;
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0 || sz > 65536) { fclose(f); return 0; }
    char *buf = malloc(sz + 1);
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);
    int n = parse_root_xform_json(buf, entries, maxCount);
    free(buf);
    return n;
}

/* ---- Transforms ---- */

/* Undefined / pole outputs are encoded as NaN so downstream rasterizers can clip them. */
static void rt_mark_undefined(float *re, float *im, int idx) {
    re[idx] = NAN;
    im[idx] = NAN;
}

/* rotate_roots(turns): multiply all roots by exp(i * 2*pi*turns).
 * Positive = CW, negative = CCW. 1 = full turn. */
static void rt_rotate_roots(float *re, float *im, int degree, double turns) {
    double theta = 2.0 * M_PI * turns;
    double c = cos(theta), s = sin(theta);
    for (int k = 0; k < degree; k++) {
        double r = re[k], i = im[k];
        re[k] = (float)(r * c - i * s);
        im[k] = (float)(r * s + i * c);
    }
}

/* pull_unit_circle(sigma, alpha): radial deformation toward unit circle.
 * r' = r - alpha * d * exp(-(d/sigma)^2), d = r - 1. */
static void rt_pull_unit_circle(float *re, float *im, int degree, double sigma, double alpha) {
    if (sigma < 1e-10) sigma = 1e-10;
    double inv_sig2 = 1.0 / (sigma * sigma);
    for (int k = 0; k < degree; k++) {
        double r_re = re[k], r_im = im[k];
        double r = sqrt(r_re * r_re + r_im * r_im);
        if (r < 1e-30) continue;
        double d = r - 1.0;
        double rprime = r - alpha * d * exp(-d * d * inv_sig2);
        double scale = rprime / r;
        re[k] = (float)(r_re * scale);
        im[k] = (float)(r_im * scale);
    }
}

/* roots_toline(): Cayley transform w = i*(1+z)/(1-z).
 * Maps unit circle → real line, unit disk interior → upper half-plane. */
static void rt_roots_toline(float *re, float *im, int degree) {
    for (int k = 0; k < degree; k++) {
        double zr = re[k], zi = im[k];
        /* num = 1 + z */
        double nr = 1.0 + zr, ni = zi;
        /* den = 1 - z */
        double dr = 1.0 - zr, di = -zi;
        /* num/den */
        double d2 = dr * dr + di * di;
        if (d2 < 1e-30) { rt_mark_undefined(re, im, k); continue; }  /* pole at z=1 */
        double qr = (nr * dr + ni * di) / d2;
        double qi = (ni * dr - nr * di) / d2;
        /* i * (num/den) = -qi + i*qr */
        re[k] = (float)(-qi);
        im[k] = (float)(qr);
    }
}

/* line_to_unit_circle(): inverse Cayley transform z = (w-i)/(w+i).
 * Maps real line → unit circle, upper half-plane → unit disk. */
static void rt_line_to_unit_circle(float *re, float *im, int degree) {
    for (int k = 0; k < degree; k++) {
        double wr = re[k], wi = im[k];
        /* num = w - i */
        double nr = wr, ni = wi - 1.0;
        /* den = w + i */
        double dr = wr, di = wi + 1.0;
        double d2 = dr * dr + di * di;
        if (d2 < 1e-30) { rt_mark_undefined(re, im, k); continue; }  /* pole at w=-i */
        re[k] = (float)((nr * dr + ni * di) / d2);
        im[k] = (float)((ni * dr - nr * di) / d2);
    }
}

/* invert_roots(): z' = 1 / z. */
static void rt_invert_roots(float *re, float *im, int degree) {
    for (int k = 0; k < degree; k++) {
        double zr = re[k], zi = im[k];
        double d2 = zr * zr + zi * zi;
        if (d2 < 1e-30) { rt_mark_undefined(re, im, k); continue; }  /* pole at z=0 */
        re[k] = (float)(zr / d2);
        im[k] = (float)(-zi / d2);
    }
}

/* add_complex(a,b): z' = z + (a + i*b). */
static void rt_add_complex(float *re, float *im, int degree, double a, double b) {
    for (int k = 0; k < degree; k++) {
        re[k] = (float)(re[k] + a);
        im[k] = (float)(im[k] + b);
    }
}

/* mul_complex(a,b): z' = z * (a + i*b). */
static void rt_mul_complex(float *re, float *im, int degree, double a, double b) {
    for (int k = 0; k < degree; k++) {
        double zr = re[k], zi = im[k];
        re[k] = (float)(zr * a - zi * b);
        im[k] = (float)(zr * b + zi * a);
    }
}

/* moebius(a,b,c,d): real-coefficient Mobius transform z' = (a z + b)/(c z + d). */
static void rt_moebius(float *re, float *im, int degree, double a, double b, double c, double d) {
    for (int k = 0; k < degree; k++) {
        double zr = re[k], zi = im[k];
        /* num = a z + b */
        double nr = a * zr + b;
        double ni = a * zi;
        /* den = c z + d */
        double dr = c * zr + d;
        double di = c * zi;
        double den2 = dr * dr + di * di;
        if (den2 < 1e-30) { rt_mark_undefined(re, im, k); continue; }  /* pole where c z + d = 0 */
        re[k] = (float)((nr * dr + ni * di) / den2);
        im[k] = (float)((ni * dr - nr * di) / den2);
    }
}

/* pull_towards_center(alpha, sigma): Gaussian radial shrink toward origin.
 * r' = r * (1 - alpha * exp(-(r/sigma)^2)). */
static void rt_pull_towards_center(float *re, float *im, int degree, double alpha, double sigma) {
    if (sigma < 1e-10) sigma = 1e-10;
    double inv_sig2 = 1.0 / (sigma * sigma);
    for (int k = 0; k < degree; k++) {
        double x = re[k], y = im[k];
        double r = sqrt(x * x + y * y);
        if (r < 1e-30) continue;
        double shrink = alpha * exp(-r * r * inv_sig2);
        double s = 1.0 - shrink;
        re[k] = (float)(x * s);
        im[k] = (float)(y * s);
    }
}

/* ---- Dispatch ---- */

static void apply_root_xforms(const RootXformEntry *entries, int n_entries,
                               float *re, float *im, int degree) {
    for (int t = 0; t < n_entries; t++) {
        const RootXformEntry *e = &entries[t];
        if (strcmp(e->name, "rotate_roots") == 0) {
            double turns = e->n_args > 0 ? e->args[0] : 0.0;
            rt_rotate_roots(re, im, degree, turns);
        } else if (strcmp(e->name, "pull_unit_circle") == 0) {
            double sigma = e->n_args > 0 ? e->args[0] : 0.75;
            double alpha = e->n_args > 1 ? e->args[1] : 1.0;
            rt_pull_unit_circle(re, im, degree, sigma, alpha);
        } else if (strcmp(e->name, "roots_toline") == 0) {
            rt_roots_toline(re, im, degree);
        } else if (strcmp(e->name, "line_to_unit_circle") == 0) {
            rt_line_to_unit_circle(re, im, degree);
        } else if (strcmp(e->name, "invert_roots") == 0) {
            rt_invert_roots(re, im, degree);
        } else if (strcmp(e->name, "add_complex") == 0) {
            double a = e->n_args > 0 ? e->args[0] : 0.0;
            double b = e->n_args > 1 ? e->args[1] : 0.0;
            rt_add_complex(re, im, degree, a, b);
        } else if (strcmp(e->name, "mul_complex") == 0) {
            double a = e->n_args > 0 ? e->args[0] : 1.0;
            double b = e->n_args > 1 ? e->args[1] : 0.0;
            rt_mul_complex(re, im, degree, a, b);
        } else if (strcmp(e->name, "moebius") == 0) {
            double a = e->n_args > 0 ? e->args[0] : 1.0;
            double b = e->n_args > 1 ? e->args[1] : 0.0;
            double c = e->n_args > 2 ? e->args[2] : 0.0;
            double d = e->n_args > 3 ? e->args[3] : 1.0;
            rt_moebius(re, im, degree, a, b, c, d);
        } else if (strcmp(e->name, "pull_towards_center") == 0) {
            double alpha = e->n_args > 0 ? e->args[0] : 1.0;
            double sigma = e->n_args > 1 ? e->args[1] : 0.75;
            rt_pull_towards_center(re, im, degree, alpha, sigma);
        }
        /* unknown transforms silently ignored */
    }
}

#endif /* ROOT_XFORMS_H */
