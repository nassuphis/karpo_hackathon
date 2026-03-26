/*
 * solve_score.h — shared solve-level metric computations.
 *
 * Used by both solve_proximity_stats.c (prepass) and roots2pix.c (raster).
 * Single implementation of each metric to avoid drift between binaries.
 *
 * Metrics:
 *   proximity  — min pairwise distance (near-collision proxy)
 *   crowding   — mean pairwise distance (global clustering)
 *   spread     — RMS radius from centroid
 *   anisotropy — eigenvalue ratio of root cloud covariance
 *   area       — geometric mean of covariance eigenvalues
 */

#ifndef SOLVE_SCORE_H
#define SOLVE_SCORE_H

#include <math.h>
#include <string.h>

#define SOLVE_SCORE_EPS2 1e-300

enum SolveMetric {
    SOLVE_METRIC_PROXIMITY = 0,
    SOLVE_METRIC_CROWDING = 1,
    SOLVE_METRIC_SPREAD = 2,
    SOLVE_METRIC_ANISOTROPY = 3,
    SOLVE_METRIC_AREA = 4,
};

static int parse_solve_metric(const char *s, enum SolveMetric *out) {
    if (!s) return 0;
    if (strcmp(s, "proximity") == 0)  { *out = SOLVE_METRIC_PROXIMITY; return 1; }
    if (strcmp(s, "crowding") == 0)   { *out = SOLVE_METRIC_CROWDING; return 1; }
    if (strcmp(s, "spread") == 0)     { *out = SOLVE_METRIC_SPREAD; return 1; }
    if (strcmp(s, "anisotropy") == 0) { *out = SOLVE_METRIC_ANISOTROPY; return 1; }
    if (strcmp(s, "area") == 0)       { *out = SOLVE_METRIC_AREA; return 1; }
    return 0;
}

static const char *solve_metric_name(enum SolveMetric m) {
    switch (m) {
        case SOLVE_METRIC_PROXIMITY:  return "proximity";
        case SOLVE_METRIC_CROWDING:   return "crowding";
        case SOLVE_METRIC_SPREAD:     return "spread";
        case SOLVE_METRIC_ANISOTROPY: return "anisotropy";
        case SOLVE_METRIC_AREA:       return "area";
    }
    return "unknown";
}

/*
 * Compute a solve-level metric score from root positions.
 * roots: interleaved (re, im) float pairs, length = degree * 2.
 * degree: number of roots.
 * metric: which metric to compute.
 * Returns: scalar score (larger = more extreme in the metric's direction).
 */
static double compute_solve_metric_score(const float *roots, int degree, enum SolveMetric metric) {
    if (degree < 2) return 0.0;

    if (metric == SOLVE_METRIC_PROXIMITY) {
        /* d2_min = min_{i<j} |r_i - r_j|^2 */
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

    if (metric == SOLVE_METRIC_CROWDING) {
        /* Mean of -0.5*log10(d2_ij) over all i<j pairs */
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

    /* Metrics that need centroid */
    double mean_re = 0, mean_im = 0;
    for (int i = 0; i < degree; i++) {
        mean_re += roots[i * 2];
        mean_im += roots[i * 2 + 1];
    }
    mean_re /= degree;
    mean_im /= degree;

    if (metric == SOLVE_METRIC_SPREAD) {
        /* RMS radius: r2_mean = (1/degree) * sum(dx^2 + dy^2) */
        double r2_sum = 0;
        for (int i = 0; i < degree; i++) {
            double dx = roots[i * 2] - mean_re;
            double dy = roots[i * 2 + 1] - mean_im;
            r2_sum += dx * dx + dy * dy;
        }
        double r2_mean = r2_sum / degree;
        return 0.5 * log10(r2_mean > SOLVE_SCORE_EPS2 ? r2_mean : SOLVE_SCORE_EPS2);
    }

    /* anisotropy and area both need the 2x2 covariance matrix */
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
    double disc = sqrt(disc_sq > 0 ? disc_sq : 0);  /* clamp negative from FP noise */
    double lambda_max = 0.5 * (trace + disc);
    double lambda_min = 0.5 * (trace - disc);

    if (metric == SOLVE_METRIC_ANISOTROPY) {
        return log10((lambda_max + SOLVE_SCORE_EPS2) / (lambda_min + SOLVE_SCORE_EPS2));
    }

    if (metric == SOLVE_METRIC_AREA) {
        double product = lambda_max * lambda_min;
        return 0.5 * log10(product > SOLVE_SCORE_EPS2 ? product : SOLVE_SCORE_EPS2);
    }

    return 0.0;
}

#endif /* SOLVE_SCORE_H */
