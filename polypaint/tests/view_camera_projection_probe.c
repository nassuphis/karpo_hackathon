#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "view_camera_projection.h"

static int parse_matrix(const char *text, double out[16]) {
    const char *cursor = text;
    char *end = NULL;
    for (int idx = 0; idx < 16; idx++) {
        out[idx] = strtod(cursor, &end);
        if (end == cursor) return 0;
        if (idx < 15) {
            if (*end != ',') return 0;
            cursor = end + 1;
        } else if (*end != '\0') {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char **argv) {
    if (argc != 14) {
        fprintf(stderr, "expected 13 arguments\n");
        return 2;
    }
    ViewCameraProjection camera;
    memset(&camera, 0, sizeof(camera));
    if (!parse_matrix(argv[1], camera.model_view)
            || !parse_matrix(argv[2], camera.projection)) {
        fprintf(stderr, "invalid matrix\n");
        return 2;
    }
    double x = strtod(argv[3], NULL);
    double t = strtod(argv[4], NULL);
    double z = strtod(argv[5], NULL);
    int pix = atoi(argv[6]);
    camera.slices = atoi(argv[7]);
    camera.effective_tlo = strtod(argv[8], NULL);
    camera.effective_thi = strtod(argv[9], NULL);
    camera.point_world_size = strtod(argv[10], NULL);
    camera.point_scale = strtod(argv[11], NULL);
    camera.point_min_fraction = strtod(argv[12], NULL);
    camera.point_max_fraction = strtod(argv[13], NULL);

    double px = 0.0;
    double py = 0.0;
    float depth = 0.0f;
    int point_side = 0;
    int reason = VIEW_CAMERA_REJECT_INVALID;
    int accepted = view_camera_project(
        &camera,
        x,
        t,
        z,
        pix,
        &px,
        &py,
        &depth,
        &point_side,
        &reason
    );
    printf(
        "{\"accepted\":%s,\"reason\":%d,\"px\":%.17g,\"py\":%.17g,"
        "\"depth\":%.9g,\"point_side\":%d,\"quantized_t\":%.17g}\n",
        accepted ? "true" : "false",
        reason,
        px,
        py,
        (double)depth,
        point_side,
        view_camera_quantize_t(t, camera.slices)
    );
    return 0;
}
