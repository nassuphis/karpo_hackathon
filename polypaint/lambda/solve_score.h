/*
 * solve_score.h — shared solve-level metric computations.
 *
 * Used by solve_proximity_stats.c (clip prepass) and roots2pix_mt.c (fused raster).
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
 *   max_re           — maximum real component
 *   min_re           — minimum real component
 *   max_im           — maximum imaginary component
 *   min_im           — minimum imaginary component
 *   min_mod          — minimum non-zero modulus (0 if all values are zero)
 *   max_mod          — maximum modulus
 *   min_angular_separation — smallest wrapped angular gap between non-zero roots
 *
 * Metrics (v4, parameter-row metrics; source=pm only):
 *   t1_re, t1_im, t1_abs, t1_phase
 *   t2_re, t2_im, t2_abs, t2_phase
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
#define SOLVE_SCORE_FILTER_MAXDEG 1024

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
    SOLVE_METRIC_MAX_RE = 15,
    SOLVE_METRIC_MIN_RE = 16,
    SOLVE_METRIC_MAX_IM = 17,
    SOLVE_METRIC_MIN_IM = 18,
    SOLVE_METRIC_MIN_MOD = 19,
    SOLVE_METRIC_MAX_MOD = 20,
    SOLVE_METRIC_MIN_ANGULAR_SEPARATION = 21,
    SOLVE_METRIC_T1_RE = 22,
    SOLVE_METRIC_T1_IM = 23,
    SOLVE_METRIC_T1_ABS = 24,
    SOLVE_METRIC_T1_PHASE = 25,
    SOLVE_METRIC_T2_RE = 26,
    SOLVE_METRIC_T2_IM = 27,
    SOLVE_METRIC_T2_ABS = 28,
    SOLVE_METRIC_T2_PHASE = 29,
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
    if (strcmp(s, "max_re") == 0)               { *out = SOLVE_METRIC_MAX_RE; return 1; }
    if (strcmp(s, "min_re") == 0)               { *out = SOLVE_METRIC_MIN_RE; return 1; }
    if (strcmp(s, "max_im") == 0)               { *out = SOLVE_METRIC_MAX_IM; return 1; }
    if (strcmp(s, "min_im") == 0)               { *out = SOLVE_METRIC_MIN_IM; return 1; }
    if (strcmp(s, "min_mod") == 0)              { *out = SOLVE_METRIC_MIN_MOD; return 1; }
    if (strcmp(s, "max_mod") == 0)              { *out = SOLVE_METRIC_MAX_MOD; return 1; }
    if (strcmp(s, "min_angular_separation") == 0) { *out = SOLVE_METRIC_MIN_ANGULAR_SEPARATION; return 1; }
    if (strcmp(s, "t1_re") == 0)                { *out = SOLVE_METRIC_T1_RE; return 1; }
    if (strcmp(s, "t1_im") == 0)                { *out = SOLVE_METRIC_T1_IM; return 1; }
    if (strcmp(s, "t1_abs") == 0)               { *out = SOLVE_METRIC_T1_ABS; return 1; }
    if (strcmp(s, "t1_phase") == 0)             { *out = SOLVE_METRIC_T1_PHASE; return 1; }
    if (strcmp(s, "t2_re") == 0)                { *out = SOLVE_METRIC_T2_RE; return 1; }
    if (strcmp(s, "t2_im") == 0)                { *out = SOLVE_METRIC_T2_IM; return 1; }
    if (strcmp(s, "t2_abs") == 0)               { *out = SOLVE_METRIC_T2_ABS; return 1; }
    if (strcmp(s, "t2_phase") == 0)             { *out = SOLVE_METRIC_T2_PHASE; return 1; }
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
        case SOLVE_METRIC_MAX_RE:               return "max_re";
        case SOLVE_METRIC_MIN_RE:               return "min_re";
        case SOLVE_METRIC_MAX_IM:               return "max_im";
        case SOLVE_METRIC_MIN_IM:               return "min_im";
        case SOLVE_METRIC_MIN_MOD:              return "min_mod";
        case SOLVE_METRIC_MAX_MOD:              return "max_mod";
        case SOLVE_METRIC_MIN_ANGULAR_SEPARATION: return "min_angular_separation";
        case SOLVE_METRIC_T1_RE:                return "t1_re";
        case SOLVE_METRIC_T1_IM:                return "t1_im";
        case SOLVE_METRIC_T1_ABS:               return "t1_abs";
        case SOLVE_METRIC_T1_PHASE:             return "t1_phase";
        case SOLVE_METRIC_T2_RE:                return "t2_re";
        case SOLVE_METRIC_T2_IM:                return "t2_im";
        case SOLVE_METRIC_T2_ABS:               return "t2_abs";
        case SOLVE_METRIC_T2_PHASE:             return "t2_phase";
    }
    return "unknown";
}

static int solve_metric_is_param_metric(enum SolveMetric metric) {
    switch (metric) {
        case SOLVE_METRIC_T1_RE:
        case SOLVE_METRIC_T1_IM:
        case SOLVE_METRIC_T1_ABS:
        case SOLVE_METRIC_T1_PHASE:
        case SOLVE_METRIC_T2_RE:
        case SOLVE_METRIC_T2_IM:
        case SOLVE_METRIC_T2_ABS:
        case SOLVE_METRIC_T2_PHASE:
            return 1;
        default:
            return 0;
    }
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

static int count_finite_roots(const float *roots, int degree) {
    int count = 0;
    for (int i = 0; i < degree; i++) {
        if (isfinite((double)roots[i * 2]) && isfinite((double)roots[i * 2 + 1])) {
            count++;
        }
    }
    return count;
}

static int solve_metric_min_roots(enum SolveMetric metric) {
    switch (metric) {
        case SOLVE_METRIC_CENTROID_RE:
        case SOLVE_METRIC_CENTROID_IM:
        case SOLVE_METRIC_CENTROID_DIST:
        case SOLVE_METRIC_DIST_UNIT_CIRCLE:
        case SOLVE_METRIC_ASYMMETRY_RE:
        case SOLVE_METRIC_MAX_RE:
        case SOLVE_METRIC_MIN_RE:
        case SOLVE_METRIC_MAX_IM:
        case SOLVE_METRIC_MIN_IM:
        case SOLVE_METRIC_MIN_MOD:
        case SOLVE_METRIC_MAX_MOD:
            return 1;
        case SOLVE_METRIC_AREA:
            return 3;
        case SOLVE_METRIC_MIN_ANGULAR_SEPARATION:
            return 2;
        default:
            return 2;
    }
}

static const char *solve_metric_validity_policy_name(void) {
    return "finite_only_min_roots";
}

static double apply_solve_score_omega(double u, double omega, double phase) {
    if (!isfinite(u)) return 0.0;
    if (u < 0.0) u = 0.0;
    if (u > 1.0) u = 1.0;
    if (!isfinite(omega)) omega = 1.0;
    if (omega < 1.0) omega = 1.0;
    if (omega > 10.0) omega = 10.0;
    if (!isfinite(phase)) phase = 0.0;
    return 0.5 * (cos(omega * 2.0 * M_PI * u + phase) + 1.0);
}

static double apply_solve_score_transfer(double u, int omegaEnabled, double omega) {
    if (!isfinite(u)) return 0.0;
    if (u < 0.0) u = 0.0;
    if (u > 1.0) u = 1.0;
    if (!omegaEnabled) return u;
    return apply_solve_score_omega(u, omega, 0.0);
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

static double wrapped_angle_0_2pi(double re, double im) {
    double angle = atan2(im, re);
    if (angle < 0.0) angle += 2.0 * M_PI;
    return angle;
}

static double compute_param_metric_score(const float *params, int paramDegree, enum SolveMetric metric);

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
    if (solve_metric_is_param_metric(metric)) {
        if (!roots) return 0.0;
        for (int i = 0; i < degree * 2 && i < 4; i++) {
            if (!isfinite((double)roots[i])) return 0.0;
        }
        return compute_param_metric_score(roots, degree, metric);
    }
    int minRoots = solve_metric_min_roots(metric);
    int finiteDegree = degree;
    const float *useRoots = roots;
    float stackRoots[SOLVE_SCORE_FILTER_MAXDEG * 2];
    float *ownedRoots = NULL;

    if (!roots_all_finite(roots, degree)) {
        finiteDegree = count_finite_roots(roots, degree);
        if (finiteDegree < minRoots) return 0.0;
        ownedRoots = (finiteDegree <= SOLVE_SCORE_FILTER_MAXDEG)
            ? stackRoots
            : (float *)malloc((size_t)finiteDegree * 2 * sizeof(float));
        if (!ownedRoots) return 0.0;
        int out = 0;
        for (int i = 0; i < degree; i++) {
            double re = roots[i * 2];
            double im = roots[i * 2 + 1];
            if (!isfinite(re) || !isfinite(im)) continue;
            ownedRoots[out * 2] = (float)re;
            ownedRoots[out * 2 + 1] = (float)im;
            out++;
        }
        useRoots = ownedRoots;
    } else if (degree < minRoots) {
        return 0.0;
    }

    degree = finiteDegree;
    roots = useRoots;

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
        double result = -0.5 * log10(d2_min > SOLVE_SCORE_EPS2 ? d2_min : SOLVE_SCORE_EPS2);
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return result;
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
        double result = sum / M;
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return result;
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
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
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
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return result;
    }

    /* ── Centroid-based metrics ── */
    double mean_re, mean_im;
    compute_centroid(roots, degree, &mean_re, &mean_im);

    if (metric == SOLVE_METRIC_CENTROID_RE) {
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return mean_re;
    }

    if (metric == SOLVE_METRIC_CENTROID_IM) {
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return mean_im;
    }

    if (metric == SOLVE_METRIC_CENTROID_DIST) {
        double result = log10(hypot(mean_re, mean_im) + SOLVE_SCORE_EPS);
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return result;
    }

    if (metric == SOLVE_METRIC_DIST_UNIT_CIRCLE) {
        double sum = 0.0;
        for (int i = 0; i < degree; i++) {
            double re = roots[i * 2];
            double im = roots[i * 2 + 1];
            sum += fabs(hypot(re, im) - 1.0);
        }
        double result = log10(sum / degree + SOLVE_SCORE_EPS);
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return result;
    }

    if (metric == SOLVE_METRIC_ASYMMETRY_RE) {
        double mean_abs_re = 0.0;
        for (int i = 0; i < degree; i++) {
            mean_abs_re += fabs((double)roots[i * 2]);
        }
        mean_abs_re /= degree;
        double result = fabs(mean_re) / (mean_abs_re + SOLVE_SCORE_EPS);
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return result;
    }

    if (metric == SOLVE_METRIC_MAX_RE) {
        double best = roots[0];
        for (int i = 1; i < degree; i++) {
            double re = roots[i * 2];
            if (re > best) best = re;
        }
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return best;
    }

    if (metric == SOLVE_METRIC_MIN_RE) {
        double best = roots[0];
        for (int i = 1; i < degree; i++) {
            double re = roots[i * 2];
            if (re < best) best = re;
        }
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return best;
    }

    if (metric == SOLVE_METRIC_MAX_IM) {
        double best = roots[1];
        for (int i = 1; i < degree; i++) {
            double im = roots[i * 2 + 1];
            if (im > best) best = im;
        }
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return best;
    }

    if (metric == SOLVE_METRIC_MIN_IM) {
        double best = roots[1];
        for (int i = 1; i < degree; i++) {
            double im = roots[i * 2 + 1];
            if (im < best) best = im;
        }
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return best;
    }

    if (metric == SOLVE_METRIC_MIN_MOD) {
        double min_mod = 0.0;
        int found_nonzero = 0;
        for (int i = 0; i < degree; i++) {
            double re = roots[i * 2];
            double im = roots[i * 2 + 1];
            if (re == 0.0 && im == 0.0) continue;
            double mod = hypot(re, im);
            if (!found_nonzero || mod < min_mod) {
                min_mod = mod;
                found_nonzero = 1;
            }
        }
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return found_nonzero ? min_mod : 0.0;
    }

    if (metric == SOLVE_METRIC_MAX_MOD) {
        double max_mod = 0.0;
        for (int i = 0; i < degree; i++) {
            double re = roots[i * 2];
            double im = roots[i * 2 + 1];
            double mod = hypot(re, im);
            if (mod > max_mod) max_mod = mod;
        }
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return max_mod;
    }

    if (metric == SOLVE_METRIC_MIN_ANGULAR_SEPARATION) {
        double angle_stack[1024];
        double *angles = degree <= 1024 ? angle_stack : (double *)malloc((size_t)degree * sizeof(double));
        if (!angles) {
            if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
            return 0.0;
        }
        int angle_count = 0;
        for (int i = 0; i < degree; i++) {
            double re = roots[i * 2];
            double im = roots[i * 2 + 1];
            if (re == 0.0 && im == 0.0) continue;
            angles[angle_count++] = wrapped_angle_0_2pi(re, im);
        }
        if (angle_count < 2) {
            if (angles != angle_stack) free(angles);
            if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
            return 0.0;
        }
        qsort(angles, (size_t)angle_count, sizeof(double), _ss_dbl_cmp);
        double min_gap = 2.0 * M_PI;
        for (int i = 1; i < angle_count; i++) {
            double gap = angles[i] - angles[i - 1];
            if (gap < min_gap) min_gap = gap;
        }
        double wrap_gap = 2.0 * M_PI - angles[angle_count - 1] + angles[0];
        if (wrap_gap < min_gap) min_gap = wrap_gap;
        if (angles != angle_stack) free(angles);
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return min_gap;
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
        double result = 0.5 * log10(r2_mean > SOLVE_SCORE_EPS2 ? r2_mean : SOLVE_SCORE_EPS2);
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return result;
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
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
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
        double result = log10(lambda_max + SOLVE_SCORE_EPS2) - log10(lambda_min + SOLVE_SCORE_EPS2);
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return result;
    }

    if (metric == SOLVE_METRIC_AREA) {
        double product = lambda_max * lambda_min;
        double result = 0.5 * log10(product > SOLVE_SCORE_EPS2 ? product : SOLVE_SCORE_EPS2);
        if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
        return result;
    }

    if (ownedRoots && ownedRoots != stackRoots) free(ownedRoots);
    return 0.0;
}

/* ── Solve-score RPN program helpers ──────────────────────────────────── */

#define SOLVE_SCORE_MAX_METRIC_SLOTS 16
#define SOLVE_SCORE_MAX_PROGRAM_TOKENS 32

enum SolveScoreProgramOp {
    SOLVE_SCORE_OP_PUSH_METRIC = 1,
    SOLVE_SCORE_OP_AVG = 2,
    SOLVE_SCORE_OP_MIN = 3,
    SOLVE_SCORE_OP_MAX = 4,
    SOLVE_SCORE_OP_MUL = 5,
    SOLVE_SCORE_OP_WEIGHTED_SUM = 6,
    SOLVE_SCORE_OP_ABS_DIFF = 7,
    SOLVE_SCORE_OP_GEOMETRIC_MEAN = 8,
    SOLVE_SCORE_OP_OMEGA_COSINE = 9,
    SOLVE_SCORE_OP_SAWTOOTH = 10,
    SOLVE_SCORE_OP_FLIP = 11,
};

enum SolveScoreMetricSource {
    SOLVE_SCORE_SOURCE_SOLVE = 0,
    SOLVE_SCORE_SOURCE_COEFF = 1,
    SOLVE_SCORE_SOURCE_PARAM = 2,
};

static int solve_metric_supports_source(enum SolveMetric metric, enum SolveScoreMetricSource source) {
    if (solve_metric_is_param_metric(metric)) {
        return source == SOLVE_SCORE_SOURCE_PARAM;
    }
    switch (metric) {
        case SOLVE_METRIC_MAX_RE:
        case SOLVE_METRIC_MIN_RE:
        case SOLVE_METRIC_MAX_IM:
        case SOLVE_METRIC_MIN_IM:
        case SOLVE_METRIC_MIN_MOD:
        case SOLVE_METRIC_MAX_MOD:
            return source == SOLVE_SCORE_SOURCE_SOLVE ||
                   source == SOLVE_SCORE_SOURCE_COEFF ||
                   source == SOLVE_SCORE_SOURCE_PARAM;
        default:
            return source == SOLVE_SCORE_SOURCE_SOLVE || source == SOLVE_SCORE_SOURCE_COEFF;
    }
}

typedef struct {
    enum SolveScoreProgramOp op;
    int metricSlot;
    int lagDepth;
    double a;
    double b;
} SolveScoreProgramToken;

typedef struct {
    int metricCount;
    enum SolveMetric metrics[SOLVE_SCORE_MAX_METRIC_SLOTS];
    enum SolveScoreMetricSource metricSources[SOLVE_SCORE_MAX_METRIC_SLOTS];
    double clipLo[SOLVE_SCORE_MAX_METRIC_SLOTS];
    double clipHi[SOLVE_SCORE_MAX_METRIC_SLOTS];
    int tokenCount;
    SolveScoreProgramToken tokens[SOLVE_SCORE_MAX_PROGRAM_TOKENS];
} SolveScoreProgram;

static double solve_score_clamp_unit(double v) {
    if (!isfinite(v)) return 0.0;
    if (v < 0.0) return 0.0;
    if (v > 1.0) return 1.0;
    return v;
}

static int parse_solve_score_metric_csv(const char *s, enum SolveMetric *out, int maxMetrics,
                                        char *err, size_t errCap) {
    if (!s || !*s) {
        snprintf(err, errCap, "missing score metrics");
        return 0;
    }
    char *copy = strdup(s);
    if (!copy) {
        snprintf(err, errCap, "out of memory parsing score metrics");
        return 0;
    }
    int n = 0;
    char *save = NULL;
    for (char *tok = strtok_r(copy, ",", &save); tok; tok = strtok_r(NULL, ",", &save)) {
        if (n >= maxMetrics) {
            snprintf(err, errCap, "too many score metrics (max %d)", maxMetrics);
            free(copy);
            return 0;
        }
        if (!parse_solve_metric(tok, &out[n])) {
            snprintf(err, errCap, "invalid score metric '%s'", tok);
            free(copy);
            return 0;
        }
        n++;
    }
    free(copy);
    if (n <= 0) {
        snprintf(err, errCap, "missing score metrics");
        return 0;
    }
    return n;
}

static int parse_solve_score_source_csv(const char *s, enum SolveScoreMetricSource *out, int expectedCount,
                                        char *err, size_t errCap) {
    if (!s || !*s) {
        for (int i = 0; i < expectedCount; i++) out[i] = SOLVE_SCORE_SOURCE_SOLVE;
        return expectedCount;
    }
    char *copy = strdup(s);
    if (!copy) {
        snprintf(err, errCap, "out of memory parsing score sources");
        return 0;
    }
    int n = 0;
    char *save = NULL;
    for (char *tok = strtok_r(copy, ",", &save); tok; tok = strtok_r(NULL, ",", &save)) {
        if (n >= expectedCount) {
            snprintf(err, errCap, "too many score sources");
            free(copy);
            return 0;
        }
        if (strcmp(tok, "slv") == 0) out[n] = SOLVE_SCORE_SOURCE_SOLVE;
        else if (strcmp(tok, "cf") == 0) out[n] = SOLVE_SCORE_SOURCE_COEFF;
        else if (strcmp(tok, "pm") == 0) out[n] = SOLVE_SCORE_SOURCE_PARAM;
        else {
            snprintf(err, errCap, "invalid score source '%s'", tok);
            free(copy);
            return 0;
        }
        n++;
    }
    free(copy);
    if (n != expectedCount) {
        snprintf(err, errCap, "score metric / source list length mismatch");
        return 0;
    }
    return n;
}

static int parse_solve_score_double_csv(const char *s, double *out, int maxValues,
                                        char *err, size_t errCap) {
    if (!s || !*s) {
        snprintf(err, errCap, "missing score numeric list");
        return 0;
    }
    char *copy = strdup(s);
    if (!copy) {
        snprintf(err, errCap, "out of memory parsing numeric list");
        return 0;
    }
    int n = 0;
    char *save = NULL;
    for (char *tok = strtok_r(copy, ",", &save); tok; tok = strtok_r(NULL, ",", &save)) {
        if (n >= maxValues) {
            snprintf(err, errCap, "too many numeric values (max %d)", maxValues);
            free(copy);
            return 0;
        }
        char *end = NULL;
        double v = strtod(tok, &end);
        if (!end || *end != '\0' || !isfinite(v)) {
            snprintf(err, errCap, "invalid numeric value '%s'", tok);
            free(copy);
            return 0;
        }
        out[n++] = v;
    }
    free(copy);
    if (n <= 0) {
        snprintf(err, errCap, "missing numeric values");
        return 0;
    }
    return n;
}

static int parse_solve_score_metric_token(const char *tok, int metricCount, int *slotOut, int *lagOut,
                                          char *err, size_t errCap) {
    if (!tok || tok[0] != 'm' || tok[1] == '\0') {
        snprintf(err, errCap, "invalid metric slot token '%s'", tok ? tok : "");
        return 0;
    }
    const char *cursor = tok + 1;
    if (*cursor < '0' || *cursor > '9') {
        snprintf(err, errCap, "invalid metric slot token '%s'", tok);
        return 0;
    }
    char *end = NULL;
    long slot = strtol(cursor, &end, 10);
    if (slot < 0 || slot >= metricCount) {
        snprintf(err, errCap, "invalid metric slot token '%s'", tok);
        return 0;
    }
    int lag = 0;
    if (*end == '\0') {
        lag = 0; /* legacy current-token form */
    } else if (*end == '-') {
        const char *lagText = end + 1;
        if (strcmp(lagText, "0") == 0) {
            lag = 0;
        } else if (strcmp(lagText, "1") == 0) {
            lag = 1;
        } else {
            snprintf(err, errCap, "unsupported metric lag token '%s'", tok);
            return 0;
        }
    } else {
        snprintf(err, errCap, "invalid metric slot token '%s'", tok);
        return 0;
    }
    *slotOut = (int)slot;
    *lagOut = lag;
    return 1;
}

static int parse_solve_score_program_spec(const char *spec, int metricCount,
                                          SolveScoreProgramToken *tokens, int maxTokens,
                                          char *err, size_t errCap) {
    if (!spec || !*spec) {
        snprintf(err, errCap, "missing score program");
        return 0;
    }
    char *copy = strdup(spec);
    if (!copy) {
        snprintf(err, errCap, "out of memory parsing score program");
        return 0;
    }
    int n = 0;
    int stackDepth = 0;
    char *save = NULL;
    for (char *tok = strtok_r(copy, ";", &save); tok; tok = strtok_r(NULL, ";", &save)) {
        if (n >= maxTokens) {
            snprintf(err, errCap, "too many program tokens (max %d)", maxTokens);
            free(copy);
            return 0;
        }
        SolveScoreProgramToken token;
        memset(&token, 0, sizeof(token));
        if (strcmp(tok, "avg") == 0) {
            token.op = SOLVE_SCORE_OP_AVG;
            stackDepth -= 1;
        } else if (strcmp(tok, "min") == 0) {
            token.op = SOLVE_SCORE_OP_MIN;
            stackDepth -= 1;
        } else if (strcmp(tok, "max") == 0) {
            token.op = SOLVE_SCORE_OP_MAX;
            stackDepth -= 1;
        } else if (strcmp(tok, "mul") == 0) {
            token.op = SOLVE_SCORE_OP_MUL;
            stackDepth -= 1;
        } else if (strcmp(tok, "abs_diff") == 0) {
            token.op = SOLVE_SCORE_OP_ABS_DIFF;
            stackDepth -= 1;
        } else if (strcmp(tok, "geometric_mean") == 0) {
            token.op = SOLVE_SCORE_OP_GEOMETRIC_MEAN;
            stackDepth -= 1;
        } else if (strncmp(tok, "weighted_sum:", 13) == 0) {
            char *params = tok + 13;
            char *mid = strchr(params, ':');
            if (!mid) {
                snprintf(err, errCap, "weighted_sum requires two weights");
                free(copy);
                return 0;
            }
            *mid = '\0';
            char *endA = NULL;
            char *endB = NULL;
            double a = strtod(params, &endA);
            double b = strtod(mid + 1, &endB);
            if (!endA || *endA != '\0' || !endB || *endB != '\0' || !isfinite(a) || !isfinite(b)) {
                snprintf(err, errCap, "weighted_sum requires two numeric weights");
                free(copy);
                return 0;
            }
            if (!(fabs(a) > 0.0 || fabs(b) > 0.0)) {
                snprintf(err, errCap, "weighted_sum requires at least one non-zero weight");
                free(copy);
                return 0;
            }
            token.op = SOLVE_SCORE_OP_WEIGHTED_SUM;
            token.a = a;
            token.b = b;
            stackDepth -= 1;
        } else if (strncmp(tok, "omega_cosine:", 13) == 0) {
            char *params = tok + 13;
            char *mid = strchr(params, ':');
            char *endOmega = NULL;
            char *endPhase = NULL;
            double omega = 0.0;
            double phase = 0.0;
            if (mid) {
                *mid = '\0';
                omega = strtod(params, &endOmega);
                phase = strtod(mid + 1, &endPhase);
                if (!endOmega || *endOmega != '\0' || !endPhase || *endPhase != '\0' || !isfinite(omega) || !isfinite(phase)) {
                    snprintf(err, errCap, "omega_cosine requires numeric omega and phase parameters");
                    free(copy);
                    return 0;
                }
            } else {
                omega = strtod(params, &endOmega);
                if (!endOmega || *endOmega != '\0' || !isfinite(omega)) {
                    snprintf(err, errCap, "omega_cosine requires a numeric omega parameter");
                    free(copy);
                    return 0;
                }
            }
            if (omega < 1.0 || omega > 10.0) {
                snprintf(err, errCap, "omega_cosine omega must be in [1, 10]");
                free(copy);
                return 0;
            }
            token.op = SOLVE_SCORE_OP_OMEGA_COSINE;
            token.a = omega;
            token.b = phase;
        } else if (strncmp(tok, "sawtooth:", 9) == 0) {
            char *end = NULL;
            double mult = strtod(tok + 9, &end);
            if (!end || *end != '\0' || !isfinite(mult)) {
                snprintf(err, errCap, "sawtooth requires one numeric multiplier");
                free(copy);
                return 0;
            }
            token.op = SOLVE_SCORE_OP_SAWTOOTH;
            token.a = mult;
        } else if (strcmp(tok, "flip") == 0) {
            token.op = SOLVE_SCORE_OP_FLIP;
        } else if (tok[0] == 'm' && tok[1] != '\0') {
            int slot = 0;
            int lag = 0;
            if (!parse_solve_score_metric_token(tok, metricCount, &slot, &lag, err, errCap)) {
                free(copy);
                return 0;
            }
            token.op = SOLVE_SCORE_OP_PUSH_METRIC;
            token.metricSlot = slot;
            token.lagDepth = lag;
            stackDepth += 1;
        } else {
            snprintf(err, errCap, "invalid score program token '%s'", tok);
            free(copy);
            return 0;
        }
        if (token.op != SOLVE_SCORE_OP_PUSH_METRIC && stackDepth < 1) {
            snprintf(err, errCap, "invalid stack underflow at token '%s'", tok);
            free(copy);
            return 0;
        }
        tokens[n++] = token;
    }
    free(copy);
    if (n <= 0) {
        snprintf(err, errCap, "missing score program tokens");
        return 0;
    }
    if (stackDepth != 1) {
        snprintf(err, errCap, "score program must end with stack depth 1");
        return 0;
    }
    return n;
}

static int parse_solve_score_program_args_ex(const char *metricsCsv, const char *sourcesCsv,
                                             const char *clipLoCsv, const char *clipHiCsv,
                                             const char *programSpec,
                                             SolveScoreProgram *out, char *err, size_t errCap) {
    memset(out, 0, sizeof(*out));
    int metricCount = parse_solve_score_metric_csv(metricsCsv, out->metrics, SOLVE_SCORE_MAX_METRIC_SLOTS, err, errCap);
    if (metricCount <= 0) return 0;
    int sourceCount = parse_solve_score_source_csv(sourcesCsv, out->metricSources, metricCount, err, errCap);
    if (sourceCount <= 0) return 0;
    int loCount = parse_solve_score_double_csv(clipLoCsv, out->clipLo, SOLVE_SCORE_MAX_METRIC_SLOTS, err, errCap);
    if (loCount <= 0) return 0;
    int hiCount = parse_solve_score_double_csv(clipHiCsv, out->clipHi, SOLVE_SCORE_MAX_METRIC_SLOTS, err, errCap);
    if (hiCount <= 0) return 0;
    if (loCount != metricCount || hiCount != metricCount) {
        snprintf(err, errCap, "score metric / clip list length mismatch");
        return 0;
    }
    for (int i = 0; i < metricCount; i++) {
        if (!solve_metric_supports_source(out->metrics[i], out->metricSources[i])) {
            snprintf(
                err, errCap, "metric %s does not support source %s",
                solve_metric_name(out->metrics[i]),
                out->metricSources[i] == SOLVE_SCORE_SOURCE_PARAM ? "pm"
                    : (out->metricSources[i] == SOLVE_SCORE_SOURCE_COEFF ? "cf" : "slv")
            );
            return 0;
        }
    }
    for (int i = 0; i < metricCount; i++) {
        if (!(out->clipHi[i] - out->clipLo[i] >= 1e-12)) {
            snprintf(err, errCap, "invalid clip range for metric slot %d", i);
            return 0;
        }
    }
    int tokenCount = parse_solve_score_program_spec(
        programSpec, metricCount, out->tokens, SOLVE_SCORE_MAX_PROGRAM_TOKENS, err, errCap
    );
    if (tokenCount <= 0) return 0;
    out->metricCount = metricCount;
    out->tokenCount = tokenCount;
    return 1;
}

static int parse_solve_score_program_args(const char *metricsCsv, const char *clipLoCsv,
                                          const char *clipHiCsv, const char *programSpec,
                                          SolveScoreProgram *out, char *err, size_t errCap) {
    return parse_solve_score_program_args_ex(
        metricsCsv, NULL, clipLoCsv, clipHiCsv, programSpec, out, err, errCap
    );
}

static void solve_score_program_from_legacy(enum SolveMetric metric, double clipLo, double clipHi,
                                            double omega, int omegaEnabled, SolveScoreProgram *out) {
    memset(out, 0, sizeof(*out));
    out->metricCount = 1;
    out->metrics[0] = metric;
    out->metricSources[0] = SOLVE_SCORE_SOURCE_SOLVE;
    out->clipLo[0] = clipLo;
    out->clipHi[0] = clipHi;
    out->tokenCount = omegaEnabled ? 2 : 1;
    out->tokens[0].op = SOLVE_SCORE_OP_PUSH_METRIC;
    out->tokens[0].metricSlot = 0;
    if (omegaEnabled) {
        out->tokens[1].op = SOLVE_SCORE_OP_OMEGA_COSINE;
        out->tokens[1].a = omega;
        out->tokens[1].b = 0.0;
    }
}

static double compute_param_metric_score(const float *params, int paramDegree, enum SolveMetric metric) {
    if (!params || paramDegree < 1) return 0.0;
    double t1_re = (paramDegree >= 1) ? params[0] : 0.0;
    double t1_im = (paramDegree >= 1) ? params[1] : 0.0;
    double t2_re = (paramDegree >= 2) ? params[2] : 0.0;
    double t2_im = (paramDegree >= 2) ? params[3] : 0.0;
    switch (metric) {
        case SOLVE_METRIC_MAX_RE: return t1_re > t2_re ? t1_re : t2_re;
        case SOLVE_METRIC_MIN_RE: return t1_re < t2_re ? t1_re : t2_re;
        case SOLVE_METRIC_MAX_IM: return t1_im > t2_im ? t1_im : t2_im;
        case SOLVE_METRIC_MIN_IM: return t1_im < t2_im ? t1_im : t2_im;
        case SOLVE_METRIC_MIN_MOD: {
            double t1_mod = hypot(t1_re, t1_im);
            double t2_mod = hypot(t2_re, t2_im);
            if (t1_mod == 0.0 && t2_mod == 0.0) return 0.0;
            if (t1_mod == 0.0) return t2_mod;
            if (t2_mod == 0.0) return t1_mod;
            return t1_mod < t2_mod ? t1_mod : t2_mod;
        }
        case SOLVE_METRIC_MAX_MOD: {
            double t1_mod = hypot(t1_re, t1_im);
            double t2_mod = hypot(t2_re, t2_im);
            return t1_mod > t2_mod ? t1_mod : t2_mod;
        }
        case SOLVE_METRIC_T1_RE: return t1_re;
        case SOLVE_METRIC_T1_IM: return t1_im;
        case SOLVE_METRIC_T1_ABS: return hypot(t1_re, t1_im);
        case SOLVE_METRIC_T1_PHASE: return wrapped_angle_0_2pi(t1_re, t1_im);
        case SOLVE_METRIC_T2_RE: return t2_re;
        case SOLVE_METRIC_T2_IM: return t2_im;
        case SOLVE_METRIC_T2_ABS: return hypot(t2_re, t2_im);
        case SOLVE_METRIC_T2_PHASE: return wrapped_angle_0_2pi(t2_re, t2_im);
        default: return 0.0;
    }
}

static int solve_score_program_uses_lag(const SolveScoreProgram *program) {
    if (!program) return 0;
    for (int i = 0; i < program->tokenCount; i++) {
        if (program->tokens[i].op == SOLVE_SCORE_OP_PUSH_METRIC && program->tokens[i].lagDepth > 0) return 1;
    }
    return 0;
}

static int solve_score_program_uses_lag_source(const SolveScoreProgram *program, enum SolveScoreMetricSource source) {
    if (!program) return 0;
    for (int i = 0; i < program->tokenCount; i++) {
        const SolveScoreProgramToken *token = &program->tokens[i];
        if (token->op != SOLVE_SCORE_OP_PUSH_METRIC || token->lagDepth <= 0) continue;
        if (token->metricSlot >= 0 && token->metricSlot < program->metricCount &&
            program->metricSources[token->metricSlot] == source) {
            return 1;
        }
    }
    return 0;
}

static int solve_score_metric_slot_uses_lag(const SolveScoreProgram *program, int slot) {
    if (!program || slot < 0 || slot >= program->metricCount) return 0;
    for (int i = 0; i < program->tokenCount; i++) {
        const SolveScoreProgramToken *token = &program->tokens[i];
        if (token->op == SOLVE_SCORE_OP_PUSH_METRIC && token->metricSlot == slot && token->lagDepth > 0) {
            return 1;
        }
    }
    return 0;
}

static float solve_score_eval_metric_slot_normalized(const float *roots, int degree,
                                                     const float *coeffRoots, int coeffDegree,
                                                     const float *paramValues, int paramDegree,
                                                     const SolveScoreProgram *program,
                                                     int slot) {
    double score = 0.0;
    if (!program || slot < 0 || slot >= program->metricCount) return 0.0f;
    if (program->metricSources[slot] == SOLVE_SCORE_SOURCE_PARAM) {
        score = compute_param_metric_score(paramValues, paramDegree, program->metrics[slot]);
    } else {
        const float *metricRoots = roots;
        int metricDegree = degree;
        if (program->metricSources[slot] == SOLVE_SCORE_SOURCE_COEFF) {
            metricRoots = coeffRoots;
            metricDegree = coeffDegree;
        }
        score = (!metricRoots || metricDegree <= 0)
            ? 0.0
            : compute_solve_metric_score(metricRoots, metricDegree, program->metrics[slot]);
    }
    double range = program->clipHi[slot] - program->clipLo[slot];
    double u = (score - program->clipLo[slot]) / range;
    return (float)solve_score_clamp_unit(u);
}

static int solve_score_eval_metric_slots(const float *roots, int degree,
                                         const float *coeffRoots, int coeffDegree,
                                         const float *paramValues, int paramDegree,
                                         const SolveScoreProgram *program,
                                         float *outMetricBuffer) {
    if (!program || !outMetricBuffer) return 0;
    for (int i = 0; i < program->metricCount; i++) {
        outMetricBuffer[i] = solve_score_eval_metric_slot_normalized(
            roots, degree, coeffRoots, coeffDegree, paramValues, paramDegree, program, i
        );
    }
    return 1;
}

static int solve_score_eval_lagged_metric_slots(const float *roots, int degree,
                                                const float *coeffRoots, int coeffDegree,
                                                const float *paramValues, int paramDegree,
                                                const SolveScoreProgram *program,
                                                float *outMetricBuffer) {
    if (!program || !outMetricBuffer) return 0;
    for (int i = 0; i < program->metricCount; i++) {
        if (!solve_score_metric_slot_uses_lag(program, i)) continue;
        outMetricBuffer[i] = solve_score_eval_metric_slot_normalized(
            roots, degree, coeffRoots, coeffDegree, paramValues, paramDegree, program, i
        );
    }
    return 1;
}

static double solve_score_eval_program_from_buffers(const float *currentMetricBuffer,
                                                    const float *recentMetricBuffer,
                                                    const SolveScoreProgram *program) {
    if (!program || !currentMetricBuffer) return NAN;

    double stack[SOLVE_SCORE_MAX_PROGRAM_TOKENS];
    int sp = 0;
    for (int i = 0; i < program->tokenCount; i++) {
        const SolveScoreProgramToken *token = &program->tokens[i];
        switch (token->op) {
            case SOLVE_SCORE_OP_PUSH_METRIC:
                if (token->metricSlot < 0 || token->metricSlot >= program->metricCount) return NAN;
                if (token->lagDepth == 0) {
                    stack[sp++] = currentMetricBuffer[token->metricSlot];
                } else if (token->lagDepth == 1) {
                    if (!recentMetricBuffer) return NAN;
                    stack[sp++] = recentMetricBuffer[token->metricSlot];
                } else {
                    return NAN;
                }
                break;
            case SOLVE_SCORE_OP_AVG: {
                double b = stack[--sp];
                double a = stack[sp - 1];
                stack[sp - 1] = solve_score_clamp_unit(0.5 * (a + b));
                break;
            }
            case SOLVE_SCORE_OP_MIN: {
                double b = stack[--sp];
                double a = stack[sp - 1];
                stack[sp - 1] = a < b ? a : b;
                break;
            }
            case SOLVE_SCORE_OP_MAX: {
                double b = stack[--sp];
                double a = stack[sp - 1];
                stack[sp - 1] = a > b ? a : b;
                break;
            }
            case SOLVE_SCORE_OP_MUL: {
                double b = stack[--sp];
                double a = stack[sp - 1];
                stack[sp - 1] = solve_score_clamp_unit(a * b);
                break;
            }
            case SOLVE_SCORE_OP_WEIGHTED_SUM: {
                double b = stack[--sp];
                double a = stack[sp - 1];
                stack[sp - 1] = solve_score_clamp_unit(token->a * a + token->b * b);
                break;
            }
            case SOLVE_SCORE_OP_ABS_DIFF: {
                double b = stack[--sp];
                double a = stack[sp - 1];
                stack[sp - 1] = solve_score_clamp_unit(fabs(a - b));
                break;
            }
            case SOLVE_SCORE_OP_GEOMETRIC_MEAN: {
                double b = stack[--sp];
                double a = stack[sp - 1];
                stack[sp - 1] = solve_score_clamp_unit(sqrt(solve_score_clamp_unit(a) * solve_score_clamp_unit(b)));
                break;
            }
            case SOLVE_SCORE_OP_OMEGA_COSINE:
                stack[sp - 1] = apply_solve_score_omega(stack[sp - 1], token->a, token->b);
                break;
            case SOLVE_SCORE_OP_SAWTOOTH: {
                double x = stack[sp - 1] * token->a;
                stack[sp - 1] = solve_score_clamp_unit(x - floor(x));
                break;
            }
            case SOLVE_SCORE_OP_FLIP:
                stack[sp - 1] = solve_score_clamp_unit(1.0 - stack[sp - 1]);
                break;
            default:
                return 0.0;
        }
    }
    if (sp != 1) return 0.0;
    return solve_score_clamp_unit(stack[0]);
}

static double solve_score_eval_program_with_sources(const float *roots, int degree,
                                                    const float *coeffRoots, int coeffDegree,
                                                    const float *paramValues, int paramDegree,
                                                    const SolveScoreProgram *program) {
    float metricVals[SOLVE_SCORE_MAX_METRIC_SLOTS];
    if (solve_score_program_uses_lag(program)) return NAN;
    if (!solve_score_eval_metric_slots(roots, degree, coeffRoots, coeffDegree, paramValues, paramDegree, program, metricVals)) {
        return NAN;
    }
    return solve_score_eval_program_from_buffers(metricVals, NULL, program);
}

static double solve_score_eval_program(const float *roots, int degree, const SolveScoreProgram *program) {
    return solve_score_eval_program_with_sources(roots, degree, NULL, 0, NULL, 0, program);
}

#endif /* SOLVE_SCORE_H */
