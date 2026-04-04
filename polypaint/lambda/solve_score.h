/*
 * solve_score.h — shared solve-level metric computations.
 *
 * Used by both solve_proximity_stats.c (prepass) and roots2pix.c (raster).
 * Single implementation of each metric to avoid drift between binaries.
 *
 * Metrics (v1):
 *   proximity  — min pairwise distance (near-collision proxy)
 *   crowding   — mean pairwise distance (global clustering)
 *   spread     — RMS radius from centroid
 *   anisotropy — eigenvalue ratio of root cloud covariance
 *   area       — geometric mean of covariance eigenvalues
 *
 * Metrics (v2):
 *   clusteriness       — max NN score minus median NN score
 *   shelliness         — thin-shell detector (CV of radii)
 *   outlierness        — max radius / median radius
 *   nn_variation        — stddev of NN scores
 *   real_axis_proximity — median |im| closeness to real axis
 *
 * Metrics (v3):
 *   centroid_re      — real part of root centroid
 *   centroid_im      — imaginary part of root centroid
 *   centroid_dist    — distance of centroid from origin
 *   dist_unit_circle — mean distance of roots from the unit circle
 *   asymmetry_re     — left/right imbalance across the imaginary axis
 */

#ifndef SOLVE_SCORE_H
#define SOLVE_SCORE_H

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ── Constants ────────────────────────────────────────────────────────── */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SOLVE_SCORE_EPS2 1e-300
#define SOLVE_SCORE_EPS  1e-150

/* ── Enum ─────────────────────────────────────────────────────────────── */

enum SolveMetric {
    SOLVE_METRIC_PROXIMITY = 0,
    SOLVE_METRIC_CROWDING = 1,
    SOLVE_METRIC_SPREAD = 2,
    SOLVE_METRIC_ANISOTROPY = 3,
    SOLVE_METRIC_AREA = 4,
    SOLVE_METRIC_CLUSTERINESS = 5,
    SOLVE_METRIC_SHELLINESS = 6,
    SOLVE_METRIC_OUTLIERNESS = 7,
    SOLVE_METRIC_NN_VARIATION = 8,
    SOLVE_METRIC_REAL_AXIS_PROXIMITY = 9,
    SOLVE_METRIC_CENTROID_RE = 10,
    SOLVE_METRIC_CENTROID_IM = 11,
    SOLVE_METRIC_CENTROID_DIST = 12,
    SOLVE_METRIC_DIST_UNIT_CIRCLE = 13,
    SOLVE_METRIC_ASYMMETRY_RE = 14,
};

/* ── Parser ───────────────────────────────────────────────────────────── */

static int parse_solve_metric(const char *s, enum SolveMetric *out) {
    if (!s) return 0;
    if (strcmp(s, "proximity") == 0)            { *out = SOLVE_METRIC_PROXIMITY; return 1; }
    if (strcmp(s, "crowding") == 0)             { *out = SOLVE_METRIC_CROWDING; return 1; }
    if (strcmp(s, "spread") == 0)               { *out = SOLVE_METRIC_SPREAD; return 1; }
    if (strcmp(s, "anisotropy") == 0)           { *out = SOLVE_METRIC_ANISOTROPY; return 1; }
    if (strcmp(s, "area") == 0)                 { *out = SOLVE_METRIC_AREA; return 1; }
    if (strcmp(s, "clusteriness") == 0)         { *out = SOLVE_METRIC_CLUSTERINESS; return 1; }
    if (strcmp(s, "shelliness") == 0)           { *out = SOLVE_METRIC_SHELLINESS; return 1; }
    if (strcmp(s, "outlierness") == 0)          { *out = SOLVE_METRIC_OUTLIERNESS; return 1; }
    if (strcmp(s, "nn_variation") == 0)         { *out = SOLVE_METRIC_NN_VARIATION; return 1; }
    if (strcmp(s, "real_axis_proximity") == 0)  { *out = SOLVE_METRIC_REAL_AXIS_PROXIMITY; return 1; }
    if (strcmp(s, "centroid_re") == 0)          { *out = SOLVE_METRIC_CENTROID_RE; return 1; }
    if (strcmp(s, "centroid_im") == 0)          { *out = SOLVE_METRIC_CENTROID_IM; return 1; }
    if (strcmp(s, "centroid_dist") == 0)        { *out = SOLVE_METRIC_CENTROID_DIST; return 1; }
    if (strcmp(s, "dist_unit_circle") == 0)     { *out = SOLVE_METRIC_DIST_UNIT_CIRCLE; return 1; }
    if (strcmp(s, "asymmetry_re") == 0)         { *out = SOLVE_METRIC_ASYMMETRY_RE; return 1; }
    return 0;
}

/* ── Serializer ───────────────────────────────────────────────────────── */

static const char *solve_metric_name(enum SolveMetric m) {
    switch (m) {
        case SOLVE_METRIC_PROXIMITY:            return "proximity";
        case SOLVE_METRIC_CROWDING:             return "crowding";
        case SOLVE_METRIC_SPREAD:               return "spread";
        case SOLVE_METRIC_ANISOTROPY:           return "anisotropy";
        case SOLVE_METRIC_AREA:                 return "area";
        case SOLVE_METRIC_CLUSTERINESS:         return "clusteriness";
        case SOLVE_METRIC_SHELLINESS:           return "shelliness";
        case SOLVE_METRIC_OUTLIERNESS:          return "outlierness";
        case SOLVE_METRIC_NN_VARIATION:         return "nn_variation";
        case SOLVE_METRIC_REAL_AXIS_PROXIMITY:  return "real_axis_proximity";
        case SOLVE_METRIC_CENTROID_RE:          return "centroid_re";
        case SOLVE_METRIC_CENTROID_IM:          return "centroid_im";
        case SOLVE_METRIC_CENTROID_DIST:        return "centroid_dist";
        case SOLVE_METRIC_DIST_UNIT_CIRCLE:     return "dist_unit_circle";
        case SOLVE_METRIC_ASYMMETRY_RE:         return "asymmetry_re";
    }
    return "unknown";
}

/* ── Low-level math helpers ───────────────────────────────────────────── */

static int _ss_dbl_cmp(const void *a, const void *b) {
    double da = *(const double *)a, db = *(const double *)b;
    return (da > db) - (da < db);
}

static int roots_all_finite(const float *roots, int degree) {
    for (int i = 0; i < degree; i++) {
        if (!isfinite((double)roots[i * 2]) || !isfinite((double)roots[i * 2 + 1])) {
            return 0;
        }
    }
    return 1;
}

static double apply_solve_score_omega(double u, double omega) {
    if (!isfinite(u)) return 0.0;
    if (u < 0.0) u = 0.0;
    if (u > 1.0) u = 1.0;
    if (!isfinite(omega)) omega = 1.0;
    if (omega < 1.0) omega = 1.0;
    if (omega > 10.0) omega = 10.0;
    return 0.5 * (cos(omega * 2.0 * M_PI * u) + 1.0);
}

/* Exact median: sort + middle element(s). Modifies values[] in-place. */
static double median_inplace(double *values, int n) {
    if (n <= 0) return 0.0;
    qsort(values, n, sizeof(double), _ss_dbl_cmp);
    if (n % 2 == 1) return values[n / 2];
    return 0.5 * (values[n / 2 - 1] + values[n / 2]);
}

static double mean_of(const double *values, int n) {
    if (n <= 0) return 0.0;
    double sum = 0;
    for (int i = 0; i < n; i++) sum += values[i];
    return sum / n;
}

static double stddev_of(const double *values, int n, double mean) {
    if (n <= 1) return 0.0;
    double sum = 0;
    for (int i = 0; i < n; i++) {
        double d = values[i] - mean;
        sum += d * d;
    }
    return sqrt(sum / n);
}

/* Compute centroid of interleaved roots. */
static void compute_centroid(const float *roots, int degree, double *mean_re, double *mean_im) {
    double sr = 0, si = 0;
    for (int i = 0; i < degree; i++) {
        sr += roots[i * 2];
        si += roots[i * 2 + 1];
    }
    *mean_re = sr / degree;
    *mean_im = si / degree;
}

/* Compute radii from centroid for each root. rho_out must have length >= degree. */
static void compute_radii_from_centroid(const float *roots, int degree,
                                        double mean_re, double mean_im,
                                        double *rho_out) {
    for (int i = 0; i < degree; i++) {
        double dx = roots[i * 2] - mean_re;
        double dy = roots[i * 2 + 1] - mean_im;
        rho_out[i] = sqrt(dx * dx + dy * dy);
    }
}

/* Compute per-root nearest-neighbor scores: s1_i = -0.5*log10(min_{j!=i} d2_ij).
 * s1_out must have length >= degree. */
static void compute_nearest_neighbor_scores(const float *roots, int degree, double *s1_out) {
    for (int i = 0; i < degree; i++) {
        double ri_re = roots[i * 2], ri_im = roots[i * 2 + 1];
        double d2_min = 1e300;
        for (int j = 0; j < degree; j++) {
            if (j == i) continue;
            double dr = ri_re - roots[j * 2];
            double di = ri_im - roots[j * 2 + 1];
            double d2 = dr * dr + di * di;
            if (d2 < d2_min) d2_min = d2;
        }
        s1_out[i] = -0.5 * log10(d2_min > SOLVE_SCORE_EPS2 ? d2_min : SOLVE_SCORE_EPS2);
    }
}

/* ── Main metric function ─────────────────────────────────────────────── */

/*
 * Compute a solve-level metric score from root positions.
 * roots: interleaved (re, im) float pairs, length = degree * 2.
 * degree: number of roots.
 * metric: which metric to compute.
 * Returns: scalar score (larger = more extreme in the metric's direction).
 */
static double compute_solve_metric_score(const float *roots, int degree, enum SolveMetric metric) {
    if (degree <= 0) return 0.0;
    if (!roots_all_finite(roots, degree)) return 0.0;

    /* ── proximity: -0.5*log10(min d2) ── */
    if (metric == SOLVE_METRIC_PROXIMITY) {
        if (degree < 2) return 0.0;
        double d2_min = 1e300;
        for (int i = 0; i < degree; i++) {
            double ri_re = roots[i * 2], ri_im = roots[i * 2 + 1];
            for (int j = i + 1; j < degree; j++) {
                double dr = ri_re - roots[j * 2];
                double di = ri_im - roots[j * 2 + 1];
                double d2 = dr * dr + di * di;
                if (d2 < d2_min) d2_min = d2;
            }
        }
        return -0.5 * log10(d2_min > SOLVE_SCORE_EPS2 ? d2_min : SOLVE_SCORE_EPS2);
    }

    /* ── crowding: mean(-0.5*log10(d2)) over all i<j ── */
    if (metric == SOLVE_METRIC_CROWDING) {
        if (degree < 2) return 0.0;
        int M = degree * (degree - 1) / 2;
        double sum = 0.0;
        for (int i = 0; i < degree; i++) {
            double ri_re = roots[i * 2], ri_im = roots[i * 2 + 1];
            for (int j = i + 1; j < degree; j++) {
                double dr = ri_re - roots[j * 2];
                double di = ri_im - roots[j * 2 + 1];
                double d2 = dr * dr + di * di;
                sum += -0.5 * log10(d2 > SOLVE_SCORE_EPS2 ? d2 : SOLVE_SCORE_EPS2);
            }
        }
        return sum / M;
    }

    /* ── NN-based metrics: clusteriness, nn_variation ── */
    if (metric == SOLVE_METRIC_CLUSTERINESS || metric == SOLVE_METRIC_NN_VARIATION) {
        if (degree < 2) return 0.0;
        double s1[1024];
        double *s1_buf = degree <= 1024 ? s1 : (double *)malloc(degree * sizeof(double));
        compute_nearest_neighbor_scores(roots, degree, s1_buf);

        double result;
        if (metric == SOLVE_METRIC_CLUSTERINESS) {
            /* score = s1_max - median(s1) */
            double s1_max = s1_buf[0];
            for (int i = 1; i < degree; i++)
                if (s1_buf[i] > s1_max) s1_max = s1_buf[i];
            /* median_inplace sorts s1_buf */
            double s1_med = median_inplace(s1_buf, degree);
            result = s1_max - s1_med;
        } else {
            /* nn_variation: stddev(s1) */
            double s1_mean = mean_of(s1_buf, degree);
            result = stddev_of(s1_buf, degree, s1_mean);
        }

        if (s1_buf != s1) free(s1_buf);
        return result;
    }

    /* ── real_axis_proximity: -log10(median(|im|) + EPS) ── */
    if (metric == SOLVE_METRIC_REAL_AXIS_PROXIMITY) {
        double abs_im[1024];
        double *buf = degree <= 1024 ? abs_im : (double *)malloc(degree * sizeof(double));
        for (int i = 0; i < degree; i++)
            buf[i] = fabs(roots[i * 2 + 1]);
        double im_med = median_inplace(buf, degree);
        double result = -log10(im_med + SOLVE_SCORE_EPS);
        if (buf != abs_im) free(buf);
        return result;
    }

    /* ── Centroid-based metrics ── */
    double mean_re, mean_im;
    compute_centroid(roots, degree, &mean_re, &mean_im);

    if (metric == SOLVE_METRIC_CENTROID_RE) {
        return mean_re;
    }

    if (metric == SOLVE_METRIC_CENTROID_IM) {
        return mean_im;
    }

    if (metric == SOLVE_METRIC_CENTROID_DIST) {
        return log10(hypot(mean_re, mean_im) + SOLVE_SCORE_EPS);
    }

    if (metric == SOLVE_METRIC_DIST_UNIT_CIRCLE) {
        double sum = 0.0;
        for (int i = 0; i < degree; i++) {
            double re = roots[i * 2];
            double im = roots[i * 2 + 1];
            sum += fabs(hypot(re, im) - 1.0);
        }
        return log10(sum / degree + SOLVE_SCORE_EPS);
    }

    if (metric == SOLVE_METRIC_ASYMMETRY_RE) {
        double mean_abs_re = 0.0;
        for (int i = 0; i < degree; i++) {
            mean_abs_re += fabs((double)roots[i * 2]);
        }
        mean_abs_re /= degree;
        return fabs(mean_re) / (mean_abs_re + SOLVE_SCORE_EPS);
    }

    /* ── spread: 0.5*log10(RMS_radius^2) ── */
    if (metric == SOLVE_METRIC_SPREAD) {
        if (degree < 2) return 0.0;
        double r2_sum = 0;
        for (int i = 0; i < degree; i++) {
            double dx = roots[i * 2] - mean_re;
            double dy = roots[i * 2 + 1] - mean_im;
            r2_sum += dx * dx + dy * dy;
        }
        double r2_mean = r2_sum / degree;
        return 0.5 * log10(r2_mean > SOLVE_SCORE_EPS2 ? r2_mean : SOLVE_SCORE_EPS2);
    }

    /* ── Radii-based metrics: shelliness, outlierness ── */
    if (metric == SOLVE_METRIC_SHELLINESS || metric == SOLVE_METRIC_OUTLIERNESS) {
        if (degree < 2) return 0.0;
        double rho[1024];
        double *rho_buf = degree <= 1024 ? rho : (double *)malloc(degree * sizeof(double));
        compute_radii_from_centroid(roots, degree, mean_re, mean_im, rho_buf);

        double result;
        if (metric == SOLVE_METRIC_SHELLINESS) {
            /* score = -log10(rho_std / (rho_mean + EPS) + EPS) */
            double rho_mean = mean_of(rho_buf, degree);
            double rho_std = stddev_of(rho_buf, degree, rho_mean);
            result = -log10(rho_std / (rho_mean + SOLVE_SCORE_EPS) + SOLVE_SCORE_EPS);
        } else {
            /* outlierness: log10((rho_max + EPS) / (rho_med + EPS)) */
            double rho_max = rho_buf[0];
            for (int i = 1; i < degree; i++)
                if (rho_buf[i] > rho_max) rho_max = rho_buf[i];
            double rho_med = median_inplace(rho_buf, degree);
            result = log10((rho_max + SOLVE_SCORE_EPS) / (rho_med + SOLVE_SCORE_EPS));
        }

        if (rho_buf != rho) free(rho_buf);
        return result;
    }

    /* ── Covariance-based metrics: anisotropy, area ── */
    if (degree < 2) return 0.0;
    double Sxx = 0, Syy = 0, Sxy = 0;
    for (int i = 0; i < degree; i++) {
        double dx = roots[i * 2] - mean_re;
        double dy = roots[i * 2 + 1] - mean_im;
        Sxx += dx * dx;
        Syy += dy * dy;
        Sxy += dx * dy;
    }
    Sxx /= degree;
    Syy /= degree;
    Sxy /= degree;

    double trace = Sxx + Syy;
    double det = Sxx * Syy - Sxy * Sxy;
    double disc_sq = trace * trace - 4.0 * det;
    double disc = sqrt(disc_sq > 0 ? disc_sq : 0);
    double lambda_max = 0.5 * (trace + disc);
    double lambda_min = 0.5 * (trace - disc);

    /* Covariance eigenvalues are mathematically non-negative.
     * Clamp tiny negative values caused by floating-point roundoff
     * before using them in log-domain metrics such as anisotropy. */
    if (lambda_max < 0.0) lambda_max = 0.0;
    if (lambda_min < 0.0) lambda_min = 0.0;

    if (metric == SOLVE_METRIC_ANISOTROPY) {
        return log10(lambda_max + SOLVE_SCORE_EPS2) - log10(lambda_min + SOLVE_SCORE_EPS2);
    }

    if (metric == SOLVE_METRIC_AREA) {
        double product = lambda_max * lambda_min;
        return 0.5 * log10(product > SOLVE_SCORE_EPS2 ? product : SOLVE_SCORE_EPS2);
    }

    return 0.0;
}

#endif /* SOLVE_SCORE_H */
