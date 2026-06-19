#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "solve_score.h"

static int parse_float_csv(const char *text, float **out, int *count_out) {
    *out = NULL;
    *count_out = 0;
    if (!text || strcmp(text, "-") == 0 || text[0] == '\0') return 1;
    char *copy = strdup(text);
    if (!copy) return 0;
    int count = 1;
    for (const char *p = text; *p; p++) {
        if (*p == ',') count++;
    }
    float *values = (float *)calloc((size_t)count, sizeof(float));
    if (!values) {
        free(copy);
        return 0;
    }
    int n = 0;
    char *save = NULL;
    for (char *tok = strtok_r(copy, ",", &save); tok; tok = strtok_r(NULL, ",", &save)) {
        char *end = NULL;
        double v = strtod(tok, &end);
        if (!end || *end != '\0') {
            free(values);
            free(copy);
            return 0;
        }
        values[n++] = (float)v;
    }
    free(copy);
    *out = values;
    *count_out = n;
    return 1;
}

static void print_csv_double(const double *values, int count) {
    for (int i = 0; i < count; i++) {
        if (i) printf(",");
        printf("%.17g", values[i]);
    }
}

static void print_csv_float(const float *values, int count) {
    for (int i = 0; i < count; i++) {
        if (i) printf(",");
        printf("%.17g", (double)values[i]);
    }
}

int main(int argc, char **argv) {
    if (argc != 10) {
        fprintf(stderr, "usage: %s metrics sources lo hi program roots coeff param recent\n", argv[0]);
        return 2;
    }
    SolveScoreProgram program;
    char err[256] = "";
    if (!parse_solve_score_program_args_ex(argv[1], argv[2], argv[3], argv[4], argv[5],
                                           &program, err, sizeof(err))) {
        fprintf(stderr, "%s\n", err[0] ? err : "parse failed");
        return 3;
    }

    float *roots = NULL, *coeff = NULL, *param = NULL, *recent = NULL;
    int roots_n = 0, coeff_n = 0, param_n = 0, recent_n = 0;
    if (!parse_float_csv(argv[6], &roots, &roots_n) ||
        !parse_float_csv(argv[7], &coeff, &coeff_n) ||
        !parse_float_csv(argv[8], &param, &param_n) ||
        !parse_float_csv(argv[9], &recent, &recent_n)) {
        fprintf(stderr, "invalid float csv\n");
        free(roots);
        free(coeff);
        free(param);
        free(recent);
        return 4;
    }
    if (roots_n % 2 != 0 || coeff_n % 2 != 0 || param_n % 2 != 0) {
        fprintf(stderr, "root/coeff/param csv lengths must be even\n");
        free(roots);
        free(coeff);
        free(param);
        free(recent);
        return 5;
    }
    if (recent && recent_n < program.metricCount) {
        fprintf(stderr, "recent metric csv length must match metric count\n");
        free(roots);
        free(coeff);
        free(param);
        free(recent);
        return 6;
    }

    float metrics[SOLVE_SCORE_MAX_METRIC_SLOTS];
    if (!solve_score_eval_metric_slots(
            roots, roots_n / 2,
            coeff_n ? coeff : NULL, coeff_n / 2,
            param_n ? param : NULL, param_n / 2,
            &program, metrics)) {
        fprintf(stderr, "metric eval failed\n");
        free(roots);
        free(coeff);
        free(param);
        free(recent);
        return 7;
    }

    double outputs[SOLVE_SCORE_MAX_OUTPUT_CHANNELS];
    int out_count = 0;
    if (!solve_score_eval_program_outputs_from_buffers(
            metrics,
            recent_n ? recent : NULL,
            &program,
            outputs,
            SOLVE_SCORE_MAX_OUTPUT_CHANNELS,
            &out_count)) {
        fprintf(stderr, "program eval failed\n");
        free(roots);
        free(coeff);
        free(param);
        free(recent);
        return 8;
    }

    printf("{\"metrics\":\"");
    print_csv_float(metrics, program.metricCount);
    printf("\",\"outputs\":\"");
    print_csv_double(outputs, out_count);
    printf("\",\"output_count\":%d}\n", out_count);

    free(roots);
    free(coeff);
    free(param);
    free(recent);
    return 0;
}
