/*
 * autolevels_render: Preview-like autolevel postprocess for saved render images.
 *
 * Usage:
 *   autolevels_render input.{jpeg|png} output.{jpeg|png} [options]
 *
 * Options:
 *   --bins=256
 *   --clip-low=0.0
 *   --clip-high=1.0
 *   --peak-factor=0.0
 *   --gamma=1.0
 *   --auto-gamma=none|median
 *   --target=0.5
 *   --sigmoid-strength=0.0
 *   --sigmoid-mid=0.5
 *   --vibrance=0.0
 *   --pooled-rgb=0.0         (fraction; >1 treated as percent)
 *   --quality=90
 *   --jpeg-subsample=auto|on|off
 *   --jpeg-optimize-coding
 *   --jpeg-interlace
 *   --background-color=000000
 *   --background-threshold=4
 *   --exclude-background=1
 *
 * Input is expected to be a saved render image (RGB or RGBA, uchar-ish).
 * Output image format is determined by output filename extension.
 *
 * Build (dynamic, needs libvips from Lambda layer):
 *   gcc -O3 -o autolevels_render autolevels_render.c \
 *     -I/opt/include -I/opt/include/glib-2.0 -I/opt/lib/glib-2.0/include \
 *     -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include \
 *     -L/opt/lib -lvips -lgobject-2.0 -lglib-2.0 -lm \
 *     -Wl,-rpath,/opt/lib
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <vips/vips.h>

typedef struct {
    int bins;
    double clip_low;
    double clip_high;
    double peak_factor;
    double gamma;
    char auto_gamma[16];
    double target;
    double sigmoid_strength;
    double sigmoid_mid;
    double vibrance;
    double pooled_rgb;
    int pooled_rgb_enabled;
    int quality;
    VipsForeignSubsample subsample_mode;
    int jpeg_optimize_coding;
    int jpeg_interlace;
    int background_r;
    int background_g;
    int background_b;
    int background_threshold;
    int exclude_background;
} Config;

typedef struct {
    double black;
    double white;
    int black_bin;
    int white_bin;
    double gamma_use;
    int median_bin;
    int final_lo_bin;
    int final_hi_bin;
    double final_lo;
    double final_hi;
    int used_final_stretch;
    int r_min_bin;
    int r_max_bin;
    int g_min_bin;
    int g_max_bin;
    int b_min_bin;
    int b_max_bin;
    size_t included_pixels;
    size_t excluded_pixels;
} DebugInfo;

static const char *get_arg(int argc, char **argv, const char *key) {
    size_t klen = strlen(key);
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], key, klen) == 0 && argv[i][klen] == '=') {
            return argv[i] + klen + 1;
        }
    }
    return NULL;
}

static int has_flag(int argc, char **argv, const char *flag) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], flag) == 0) return 1;
    }
    return 0;
}

static int parse_int_arg(int argc, char **argv, const char *key, int def) {
    const char *v = get_arg(argc, argv, key);
    return v ? atoi(v) : def;
}

static double parse_double_arg(int argc, char **argv, const char *key, double def) {
    const char *v = get_arg(argc, argv, key);
    return v ? atof(v) : def;
}

static VipsForeignSubsample parse_subsample_mode(const char *s) {
    if (!s || strcmp(s, "auto") == 0) return VIPS_FOREIGN_SUBSAMPLE_AUTO;
    if (strcmp(s, "on") == 0) return VIPS_FOREIGN_SUBSAMPLE_ON;
    if (strcmp(s, "off") == 0) return VIPS_FOREIGN_SUBSAMPLE_OFF;
    return VIPS_FOREIGN_SUBSAMPLE_ON;
}

static double clamp01(double x) {
    if (x < 0.0) return 0.0;
    if (x > 1.0) return 1.0;
    return x;
}

static int parse_hex_color(const char *s, int *r, int *g, int *b) {
    if (!s) return -1;
    if (*s == '#') s++;
    if (strlen(s) != 6) return -1;
    char *end = NULL;
    long value = strtol(s, &end, 16);
    if (!end || *end != '\0') return -1;
    if (value < 0 || value > 0xFFFFFFL) return -1;
    *r = (int) ((value >> 16) & 0xFF);
    *g = (int) ((value >> 8) & 0xFF);
    *b = (int) (value & 0xFF);
    return 0;
}

static int quantile_bin_from_cdf(const double *cdf, int bins, double target) {
    if (target <= 0.0) return 0;
    if (target >= 1.0) return bins - 1;
    for (int i = 0; i < bins; i++) {
        if (cdf[i] >= target) return i;
    }
    return bins - 1;
}

static void hist_to_pooled_pdf(const uint64_t hist[3][256], int bins, double *pooled) {
    double totals[3] = {0.0, 0.0, 0.0};
    double pooled_sum = 0.0;
    for (int c = 0; c < 3; c++) {
        for (int i = 0; i < bins; i++) totals[c] += (double) hist[c][i];
    }
    for (int i = 0; i < bins; i++) {
        double pr = totals[0] > 0.0 ? ((double) hist[0][i]) / totals[0] : 0.0;
        double pg = totals[1] > 0.0 ? ((double) hist[1][i]) / totals[1] : 0.0;
        double pb = totals[2] > 0.0 ? ((double) hist[2][i]) / totals[2] : 0.0;
        pooled[i] = (pr + pg + pb) / 3.0;
        pooled_sum += pooled[i];
    }
    if (pooled_sum > 0.0) {
        for (int i = 0; i < bins; i++) pooled[i] /= pooled_sum;
    }
}

static void peak_limit_pdf(double *pdf, int bins, double factor) {
    if (factor <= 0.0) return;
    double avg = 1.0 / (double) bins;
    double limit = factor * avg;
    double excess = 0.0;
    for (int i = 0; i < bins; i++) {
        if (pdf[i] > limit) {
            excess += pdf[i] - limit;
            pdf[i] = limit;
        }
    }
    double add = excess / (double) bins;
    double sum = 0.0;
    for (int i = 0; i < bins; i++) {
        pdf[i] += add;
        sum += pdf[i];
    }
    if (sum > 0.0) {
        for (int i = 0; i < bins; i++) pdf[i] /= sum;
    }
}

static void cdf_from_pdf(const double *pdf, int bins, double *cdf) {
    double sum = 0.0;
    for (int i = 0; i < bins; i++) {
        sum += pdf[i];
        cdf[i] = sum;
    }
}

static void hist_nonzero_extent(const uint64_t *hist, int bins, int *min_bin, int *max_bin) {
    int lo = -1, hi = -1;
    for (int i = 0; i < bins; i++) {
        if (hist[i] > 0) {
            lo = i;
            break;
        }
    }
    for (int i = bins - 1; i >= 0; i--) {
        if (hist[i] > 0) {
            hi = i;
            break;
        }
    }
    if (lo < 0 || hi < 0) {
        lo = 0;
        hi = 0;
    }
    *min_bin = lo;
    *max_bin = hi;
}

static void compute_endpoints(const double *pdf, int bins, double clip_low_pct, double clip_high_pct, DebugInfo *dbg) {
    double cdf[256];
    double low_target = clip_low_pct / 100.0;
    double high_target = 1.0 - (clip_high_pct / 100.0);
    if (low_target < 0.0) low_target = 0.0;
    if (high_target > 1.0) high_target = 1.0;
    cdf_from_pdf(pdf, bins, cdf);
    dbg->black_bin = quantile_bin_from_cdf(cdf, bins, low_target);
    dbg->white_bin = quantile_bin_from_cdf(cdf, bins, high_target);
    dbg->black = dbg->black_bin / (double) (bins - 1);
    dbg->white = dbg->white_bin / (double) (bins - 1);
    if (dbg->white <= dbg->black) dbg->white = dbg->black + 1e-6;
}

static void maybe_compute_auto_gamma(const double *pdf, int bins, Config *cfg, DebugInfo *dbg) {
    dbg->gamma_use = cfg->gamma;
    dbg->median_bin = -1;
    if (strcmp(cfg->auto_gamma, "median") != 0) return;

    double cdf[256];
    cdf_from_pdf(pdf, bins, cdf);
    dbg->median_bin = quantile_bin_from_cdf(cdf, bins, 0.5);

    double x = dbg->median_bin / (double) (bins - 1);
    double denom = dbg->white - dbg->black;
    if (denom < 1e-9) denom = 1e-9;
    double s = (x - dbg->black) / denom;
    if (s < 1e-6) s = 1e-6;
    if (s > 1.0 - 1e-6) s = 1.0 - 1e-6;

    double t = cfg->target;
    if (t < 1e-6) t = 1e-6;
    if (t > 1.0 - 1e-6) t = 1.0 - 1e-6;

    double g = log(t) / log(s);
    if (!isfinite(g)) g = 1.0;
    if (g < 0.5) g = 0.5;
    if (g > 2.0) g = 2.0;
    dbg->gamma_use = g;
}

static void build_curve_lut(unsigned char lut[256], const Config *cfg, const DebugInfo *dbg) {
    double s0 = 0.0, s1 = 1.0, denom_s = 1.0;
    if (cfg->sigmoid_strength > 0.0) {
        s0 = 1.0 / (1.0 + exp(cfg->sigmoid_strength * (cfg->sigmoid_mid - 0.0)));
        s1 = 1.0 / (1.0 + exp(cfg->sigmoid_strength * (cfg->sigmoid_mid - 1.0)));
        denom_s = s1 - s0;
        if (fabs(denom_s) < 1e-6) denom_s = (denom_s < 0.0 ? -1e-6 : 1e-6);
    }

    double denom = dbg->white - dbg->black;
    if (denom < 1e-6) denom = 1e-6;
    for (int i = 0; i < 256; i++) {
        double x = i / 255.0;
        double v = clamp01((x - dbg->black) / denom);
        if (fabs(dbg->gamma_use - 1.0) > 1e-12) v = pow(v, dbg->gamma_use);
        if (cfg->sigmoid_strength > 0.0) {
            double ss = 1.0 / (1.0 + exp(cfg->sigmoid_strength * (cfg->sigmoid_mid - v)));
            v = clamp01((ss - s0) / denom_s);
        }
        int out = (int) floor(v * 255.0 + 0.5);
        if (out < 0) out = 0;
        if (out > 255) out = 255;
        lut[i] = (unsigned char) out;
    }
}

static void apply_curve_lut(const unsigned char *src, unsigned char *dst, size_t pixels, int bands, const unsigned char lut[256]) {
    for (size_t p = 0; p < pixels; p++) {
        size_t off = p * (size_t) bands;
        dst[off + 0] = lut[src[off + 0]];
        dst[off + 1] = lut[src[off + 1]];
        dst[off + 2] = lut[src[off + 2]];
        if (bands == 4) dst[off + 3] = src[off + 3];
    }
}

static void apply_vibrance(unsigned char *buf, size_t pixels, int bands, double amount) {
    if (fabs(amount) < 1e-12) return;
    for (size_t p = 0; p < pixels; p++) {
        size_t off = p * (size_t) bands;
        double r = buf[off + 0] / 255.0;
        double g = buf[off + 1] / 255.0;
        double b = buf[off + 2] / 255.0;
        double maxv = r;
        if (g > maxv) maxv = g;
        if (b > maxv) maxv = b;
        double minv = r;
        if (g < minv) minv = g;
        if (b < minv) minv = b;
        double delta = maxv - minv;
        double sat = delta / (maxv + 1e-6);
        sat = clamp01(sat);
        double boost = 1.0 + amount * (1.0 - sat);
        double mean = (r + g + b) / 3.0;
        r = clamp01(mean + (r - mean) * boost);
        g = clamp01(mean + (g - mean) * boost);
        b = clamp01(mean + (b - mean) * boost);
        buf[off + 0] = (unsigned char) floor(r * 255.0 + 0.5);
        buf[off + 1] = (unsigned char) floor(g * 255.0 + 0.5);
        buf[off + 2] = (unsigned char) floor(b * 255.0 + 0.5);
    }
}

static int pixel_is_background(const unsigned char *ref, size_t off, const Config *cfg) {
    if (!cfg->exclude_background) return 0;
    int dr = abs((int) ref[off + 0] - cfg->background_r);
    int dg = abs((int) ref[off + 1] - cfg->background_g);
    int db = abs((int) ref[off + 2] - cfg->background_b);
    int maxd = dr;
    if (dg > maxd) maxd = dg;
    if (db > maxd) maxd = db;
    return maxd <= cfg->background_threshold;
}

static size_t compute_hist_from_buffer(const unsigned char *buf, const unsigned char *ref_buf, size_t pixels, int bands, const Config *cfg, uint64_t hist[3][256]) {
    memset(hist, 0, sizeof(uint64_t) * 3 * 256);
    size_t included = 0;
    for (size_t p = 0; p < pixels; p++) {
        size_t off = p * (size_t) bands;
        if (ref_buf && pixel_is_background(ref_buf, off, cfg)) continue;
        hist[0][buf[off + 0]]++;
        hist[1][buf[off + 1]]++;
        hist[2][buf[off + 2]]++;
        included++;
    }
    if (included == 0 && ref_buf) {
        for (size_t p = 0; p < pixels; p++) {
            size_t off = p * (size_t) bands;
            hist[0][buf[off + 0]]++;
            hist[1][buf[off + 1]]++;
            hist[2][buf[off + 2]]++;
        }
        return pixels;
    }
    return included;
}

static void build_linear_stretch_lut(unsigned char lut[256], double lo, double hi) {
    double denom = hi - lo;
    if (denom < 1e-6) denom = 1e-6;
    for (int i = 0; i < 256; i++) {
        double x = i / 255.0;
        double v = clamp01((x - lo) / denom);
        int out = (int) floor(v * 255.0 + 0.5);
        if (out < 0) out = 0;
        if (out > 255) out = 255;
        lut[i] = (unsigned char) out;
    }
}

static void maybe_apply_final_stretch(unsigned char *buf, const unsigned char *ref_buf, size_t pixels, int bands, const Config *cfg, DebugInfo *dbg) {
    dbg->used_final_stretch = 0;
    dbg->final_lo_bin = 0;
    dbg->final_hi_bin = 255;
    dbg->final_lo = 0.0;
    dbg->final_hi = 1.0;
    if (!cfg->pooled_rgb_enabled || cfg->pooled_rgb == 0.0) return;

    double q = cfg->pooled_rgb;
    if (q > 1.0) q /= 100.0;
    if (q < 0.0) q = 0.0;
    if (q > 0.49) q = 0.49;
    if (q <= 0.0) return;

    uint64_t hist[3][256];
    double pooled[256];
    double cdf[256];
    unsigned char lut[256];
    compute_hist_from_buffer(buf, ref_buf, pixels, bands, cfg, hist);
    hist_to_pooled_pdf(hist, 256, pooled);
    cdf_from_pdf(pooled, 256, cdf);
    dbg->final_lo_bin = quantile_bin_from_cdf(cdf, 256, q);
    dbg->final_hi_bin = quantile_bin_from_cdf(cdf, 256, 1.0 - q);
    dbg->final_lo = dbg->final_lo_bin / 255.0;
    dbg->final_hi = dbg->final_hi_bin / 255.0;
    if (dbg->final_hi <= dbg->final_lo) dbg->final_hi = dbg->final_lo + 1e-6;
    build_linear_stretch_lut(lut, dbg->final_lo, dbg->final_hi);
    for (size_t p = 0; p < pixels; p++) {
        size_t off = p * (size_t) bands;
        buf[off + 0] = lut[buf[off + 0]];
        buf[off + 1] = lut[buf[off + 1]];
        buf[off + 2] = lut[buf[off + 2]];
    }
    dbg->used_final_stretch = 1;
}

static int save_output(VipsImage *img, const char *out_path, const Config *cfg) {
    const char *ext = strrchr(out_path, '.');
    int is_jpeg = ext && (!strcmp(ext, ".jpg") || !strcmp(ext, ".jpeg"));
    if (is_jpeg) {
        VipsImage *rgb = img;
        if (img->Bands == 4) {
            if (vips_extract_band(img, &rgb, 0, "n", 3, NULL)) {
                fprintf(stderr, "vips_extract_band failed: %s\n", vips_error_buffer());
                return -1;
            }
        }
        int rc = vips_jpegsave(rgb, out_path,
                               "Q", cfg->quality,
                               "subsample_mode", cfg->subsample_mode,
                               "optimize_coding", cfg->jpeg_optimize_coding,
                               "interlace", cfg->jpeg_interlace,
                               NULL);
        if (rgb != img) g_object_unref(rgb);
        if (rc) {
            fprintf(stderr, "vips_jpegsave failed: %s\n", vips_error_buffer());
            return -1;
        }
        return 0;
    }
    if (vips_pngsave(img, out_path, "compression", 6, NULL)) {
        fprintf(stderr, "vips_pngsave failed: %s\n", vips_error_buffer());
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: autolevels_render input output [options]\n");
        return 1;
    }
    if (VIPS_INIT(argv[0])) {
        fprintf(stderr, "VIPS_INIT failed: %s\n", vips_error_buffer());
        return 1;
    }
    vips_leak_set(0);

    Config cfg;
    cfg.bins = parse_int_arg(argc, argv, "--bins", 256);
    if (cfg.bins != 256) {
        fprintf(stderr, "Only bins=256 is supported in the native renderer.\n");
        vips_shutdown();
        return 1;
    }
    cfg.clip_low = parse_double_arg(argc, argv, "--clip-low", 0.0);
    cfg.clip_high = parse_double_arg(argc, argv, "--clip-high", 1.0);
    cfg.peak_factor = parse_double_arg(argc, argv, "--peak-factor", 0.0);
    cfg.gamma = parse_double_arg(argc, argv, "--gamma", 1.0);
    snprintf(cfg.auto_gamma, sizeof(cfg.auto_gamma), "%s", get_arg(argc, argv, "--auto-gamma") ? get_arg(argc, argv, "--auto-gamma") : "none");
    cfg.target = parse_double_arg(argc, argv, "--target", 0.5);
    cfg.sigmoid_strength = parse_double_arg(argc, argv, "--sigmoid-strength", 0.0);
    cfg.sigmoid_mid = parse_double_arg(argc, argv, "--sigmoid-mid", 0.5);
    cfg.vibrance = parse_double_arg(argc, argv, "--vibrance", 0.0);
    cfg.pooled_rgb = parse_double_arg(argc, argv, "--pooled-rgb", 0.0);
    cfg.pooled_rgb_enabled = has_flag(argc, argv, "--pooled-rgb=0") ? 1 : 0;
    if (get_arg(argc, argv, "--pooled-rgb")) cfg.pooled_rgb_enabled = 1;
    cfg.quality = parse_int_arg(argc, argv, "--quality", 90);
    cfg.subsample_mode = parse_subsample_mode(get_arg(argc, argv, "--jpeg-subsample"));
    cfg.jpeg_optimize_coding = has_flag(argc, argv, "--jpeg-optimize-coding");
    cfg.jpeg_interlace = has_flag(argc, argv, "--jpeg-interlace");
    cfg.background_threshold = parse_int_arg(argc, argv, "--background-threshold", 4);
    if (cfg.background_threshold < 0) cfg.background_threshold = 0;
    if (cfg.background_threshold > 255) cfg.background_threshold = 255;
    cfg.exclude_background = parse_int_arg(argc, argv, "--exclude-background", 1) != 0;
    const char *background_color = get_arg(argc, argv, "--background-color");
    if (!background_color) background_color = "000000";
    if (parse_hex_color(background_color, &cfg.background_r, &cfg.background_g, &cfg.background_b) != 0) {
        fprintf(stderr, "Invalid --background-color value: %s\n", background_color);
        vips_shutdown();
        return 1;
    }

    const char *in_path = argv[1];
    const char *out_path = argv[2];

    VipsImage *img = vips_image_new_from_file(in_path, "access", VIPS_ACCESS_SEQUENTIAL, NULL);
    if (!img) {
        fprintf(stderr, "Cannot read %s: %s\n", in_path, vips_error_buffer());
        vips_shutdown();
        return 1;
    }

    VipsImage *u8 = img;
    if (img->BandFmt != VIPS_FORMAT_UCHAR) {
        if (vips_cast(img, &u8, VIPS_FORMAT_UCHAR, NULL)) {
            fprintf(stderr, "vips_cast failed: %s\n", vips_error_buffer());
            g_object_unref(img);
            vips_shutdown();
            return 1;
        }
    }

    VipsImage *trim = u8;
    if (u8->Bands > 4) {
        if (vips_extract_band(u8, &trim, 0, "n", 4, NULL)) {
            fprintf(stderr, "vips_extract_band failed: %s\n", vips_error_buffer());
            if (u8 != img) g_object_unref(u8);
            g_object_unref(img);
            vips_shutdown();
            return 1;
        }
    }

    if (trim->Bands != 3 && trim->Bands != 4) {
        fprintf(stderr, "Expected RGB/RGBA input, got %d bands.\n", trim->Bands);
        if (trim != u8) g_object_unref(trim);
        if (u8 != img) g_object_unref(u8);
        g_object_unref(img);
        vips_shutdown();
        return 1;
    }

    VipsImage *mem = vips_image_copy_memory(trim);
    if (!mem) {
        fprintf(stderr, "vips_image_copy_memory failed: %s\n", vips_error_buffer());
        if (trim != u8) g_object_unref(trim);
        if (u8 != img) g_object_unref(u8);
        g_object_unref(img);
        vips_shutdown();
        return 1;
    }

    size_t in_len = 0;
    unsigned char *in_buf = (unsigned char *) vips_image_write_to_memory(mem, &in_len);
    if (!in_buf) {
        fprintf(stderr, "vips_image_write_to_memory failed: %s\n", vips_error_buffer());
        g_object_unref(mem);
        if (trim != u8) g_object_unref(trim);
        if (u8 != img) g_object_unref(u8);
        g_object_unref(img);
        vips_shutdown();
        return 1;
    }

    const int width = mem->Xsize;
    const int height = mem->Ysize;
    const int bands = mem->Bands;
    size_t pixels = (size_t) width * (size_t) height;
    size_t out_len = pixels * (size_t) bands;
    unsigned char *out_buf = (unsigned char *) malloc(out_len);
    if (!out_buf) {
        fprintf(stderr, "malloc failed for output buffer\n");
        g_free(in_buf);
        g_object_unref(mem);
        if (trim != u8) g_object_unref(trim);
        if (u8 != img) g_object_unref(u8);
        g_object_unref(img);
        vips_shutdown();
        return 1;
    }

    uint64_t hist[3][256];
    double pooled[256];
    DebugInfo dbg;
    unsigned char lut[256];
    memset(&dbg, 0, sizeof(dbg));

    dbg.included_pixels = compute_hist_from_buffer(in_buf, in_buf, pixels, bands, &cfg, hist);
    dbg.excluded_pixels = pixels >= dbg.included_pixels ? pixels - dbg.included_pixels : 0;
    hist_nonzero_extent(hist[0], 256, &dbg.r_min_bin, &dbg.r_max_bin);
    hist_nonzero_extent(hist[1], 256, &dbg.g_min_bin, &dbg.g_max_bin);
    hist_nonzero_extent(hist[2], 256, &dbg.b_min_bin, &dbg.b_max_bin);
    hist_to_pooled_pdf(hist, 256, pooled);
    peak_limit_pdf(pooled, 256, cfg.peak_factor);
    compute_endpoints(pooled, 256, cfg.clip_low, cfg.clip_high, &dbg);
    maybe_compute_auto_gamma(pooled, 256, &cfg, &dbg);
    build_curve_lut(lut, &cfg, &dbg);
    apply_curve_lut(in_buf, out_buf, pixels, bands, lut);
    apply_vibrance(out_buf, pixels, bands, cfg.vibrance);
    maybe_apply_final_stretch(out_buf, in_buf, pixels, bands, &cfg, &dbg);

    VipsImage *out_img = vips_image_new_from_memory_copy(out_buf, out_len, width, height, bands, VIPS_FORMAT_UCHAR);
    if (!out_img) {
        fprintf(stderr, "vips_image_new_from_memory_copy failed: %s\n", vips_error_buffer());
        free(out_buf);
        g_free(in_buf);
        g_object_unref(mem);
        if (trim != u8) g_object_unref(trim);
        if (u8 != img) g_object_unref(u8);
        g_object_unref(img);
        vips_shutdown();
        return 1;
    }

    if (save_output(out_img, out_path, &cfg) != 0) {
        g_object_unref(out_img);
        free(out_buf);
        g_free(in_buf);
        g_object_unref(mem);
        if (trim != u8) g_object_unref(trim);
        if (u8 != img) g_object_unref(u8);
        g_object_unref(img);
        vips_shutdown();
        return 1;
    }

    FILE *f = fopen(out_path, "rb");
    long fsize = 0;
    if (f) {
        fseek(f, 0, SEEK_END);
        fsize = ftell(f);
        fclose(f);
    }

    printf("{\"width\":%d,\"height\":%d,\"bands\":%d,\"file_size\":%ld,"
           "\"black_bin\":%d,\"white_bin\":%d,\"black\":%.6f,\"white\":%.6f,"
           "\"gamma\":%.6f,\"final_stretch\":%s,\"final_lo_bin\":%d,\"final_hi_bin\":%d,"
           "\"r_min_bin\":%d,\"r_max_bin\":%d,\"g_min_bin\":%d,\"g_max_bin\":%d,"
           "\"b_min_bin\":%d,\"b_max_bin\":%d,\"included_pixels\":%zu,\"excluded_pixels\":%zu}\n",
           width, height, bands, fsize,
           dbg.black_bin, dbg.white_bin, dbg.black, dbg.white, dbg.gamma_use,
           dbg.used_final_stretch ? "true" : "false",
           dbg.final_lo_bin, dbg.final_hi_bin,
           dbg.r_min_bin, dbg.r_max_bin, dbg.g_min_bin, dbg.g_max_bin,
           dbg.b_min_bin, dbg.b_max_bin, dbg.included_pixels, dbg.excluded_pixels);

    g_object_unref(out_img);
    free(out_buf);
    g_free(in_buf);
    g_object_unref(mem);
    if (trim != u8) g_object_unref(trim);
    if (u8 != img) g_object_unref(u8);
    g_object_unref(img);
    vips_shutdown();
    return 0;
}
