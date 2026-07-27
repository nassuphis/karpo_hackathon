#ifndef POLYPAINT_VIEW_CAMERA_PROJECTION_H
#define POLYPAINT_VIEW_CAMERA_PROJECTION_H

#include <math.h>

typedef struct {
    double model_view[16];
    double projection[16];
    int vertical;
    int slices;
    double effective_tlo;
    double effective_thi;
    double point_world_size;
    double point_scale;
    double point_min_fraction;
    double point_max_fraction;
} ViewCameraProjection;

enum {
    VIEW_CAMERA_ACCEPTED = 0,
    VIEW_CAMERA_REJECT_T_RANGE = 1,
    VIEW_CAMERA_REJECT_BEHIND = 2,
    VIEW_CAMERA_REJECT_CLIP = 3,
    VIEW_CAMERA_REJECT_INVALID = 4,
};

static inline void view_camera_mul4(
        const double matrix[16],
        const double in[4],
        double out[4]) {
    for (int row = 0; row < 4; row++) {
        out[row] = matrix[row] * in[0]
            + matrix[4 + row] * in[1]
            + matrix[8 + row] * in[2]
            + matrix[12 + row] * in[3];
    }
}

static inline double view_camera_quantize_t(double t, int slices) {
    if (slices <= 0) return t;
    int level = (int)floor(t * (double)slices);
    if (level < 0) level = 0;
    if (level >= slices) level = slices - 1;
    return slices > 1
        ? (double)level / (double)(slices - 1)
        : 0.5;
}

static inline int view_camera_project(
        const ViewCameraProjection *camera,
        double local_x,
        double local_t,
        double local_z,
        int pix,
        double *px,
        double *py,
        float *depth,
        int *point_side,
        int *reject_reason) {
    if (reject_reason) *reject_reason = VIEW_CAMERA_REJECT_INVALID;
    if (!camera || !px || !py || !depth || !point_side || pix < 1) return 0;
    double t = view_camera_quantize_t(local_t, camera->slices);
    if (!isfinite(t)
            || t < camera->effective_tlo
            || t > camera->effective_thi) {
        if (reject_reason) *reject_reason = VIEW_CAMERA_REJECT_T_RANGE;
        return 0;
    }

    const double local[4] = {local_x, t - 0.5, local_z, 1.0};
    double view[4];
    double clip[4];
    view_camera_mul4(camera->model_view, local, view);
    view_camera_mul4(camera->projection, view, clip);
    if (!isfinite(view[2]) || !isfinite(clip[0]) || !isfinite(clip[1])
            || !isfinite(clip[2]) || !isfinite(clip[3])
            || !(clip[3] > 0.0)) {
        if (reject_reason) *reject_reason = VIEW_CAMERA_REJECT_BEHIND;
        return 0;
    }
    if (clip[0] < -clip[3] || clip[0] > clip[3]
            || clip[1] < -clip[3] || clip[1] > clip[3]
            || clip[2] < -clip[3] || clip[2] > clip[3]) {
        if (reject_reason) *reject_reason = VIEW_CAMERA_REJECT_CLIP;
        return 0;
    }

    const double camera_depth = -view[2];
    if (!isfinite(camera_depth) || !(camera_depth > 0.0)) return 0;
    const float camera_depth_f32 = (float)camera_depth;
    if (!isfinite(camera_depth_f32) || !(camera_depth_f32 > 0.0f)) return 0;

    const double ndc_x = clip[0] / clip[3];
    const double ndc_y = clip[1] / clip[3];
    *px = (ndc_x * 0.5 + 0.5) * (double)pix;
    *py = (0.5 - ndc_y * 0.5) * (double)pix;
    *depth = camera_depth_f32;

    double fraction = camera->point_world_size * camera->point_scale / camera_depth;
    if (fraction < camera->point_min_fraction) fraction = camera->point_min_fraction;
    if (fraction > camera->point_max_fraction) fraction = camera->point_max_fraction;
    if (!isfinite(fraction) || !(fraction > 0.0)) return 0;
    int side = (int)ceil((double)pix * fraction);
    if (side < 1) side = 1;
    if (side > pix) side = pix;
    *point_side = side;
    if (reject_reason) *reject_reason = VIEW_CAMERA_ACCEPTED;
    return 1;
}

#endif
