#ifndef POLYPAINT_VIEW_PROJECTION_H
#define POLYPAINT_VIEW_PROJECTION_H

#include <math.h>

typedef struct {
    double min_re;
    double min_im;
    double inv_re_span;
    double inv_im_span;
    double x_center;
    double y_center;
    double scale;
} ViewIsometricProjection;

/*
 * Prepare the invariant viewport normalization and uniform unit-cube fit.
 * A square output has small horizontal margins rather than stretched
 * geometry, preserving the conventional 30-degree isometric axes.
 */
static inline int view_isometric_projection_init(
        ViewIsometricProjection *projection,
        double min_re,
        double max_re,
        double min_im,
        double max_im,
        int width,
        int height) {
    if (!projection
            || width < 2 || height < 2
            || !isfinite(min_re) || !isfinite(max_re)
            || !isfinite(min_im) || !isfinite(max_im)
            || !(max_re > min_re) || !(max_im > min_im)) {
        return 0;
    }

    const double x_extent = (double)(width - 1);
    const double y_extent = (double)(height - 1);
    projection->min_re = min_re;
    projection->min_im = min_im;
    projection->inv_re_span = 1.0 / (max_re - min_re);
    projection->inv_im_span = 1.0 / (max_im - min_im);
    projection->x_center = 0.5 * x_extent;
    projection->y_center = 0.5 * y_extent;
    projection->scale = 0.5 * fmin(x_extent, y_extent);
    return 1;
}

static inline int view_project_isometric(
        const ViewIsometricProjection *projection,
        double re,
        double im,
        double t,
        double *px,
        double *py) {
    if (!projection || !px || !py
            || !isfinite(re) || !isfinite(im) || !isfinite(t)) {
        return 0;
    }

    const double x = (re - projection->min_re) * projection->inv_re_span;
    const double y = (im - projection->min_im) * projection->inv_im_span;
    const double iso_x = (x - y) * 0.86602540378443864676; /* cos(30 deg) */
    const double iso_y = (x + y) * 0.5 - t;                /* sin(30 deg) */

    *px = projection->x_center + projection->scale * iso_x;
    *py = projection->y_center + projection->scale * iso_y;
    return isfinite(*px) && isfinite(*py);
}

#endif
