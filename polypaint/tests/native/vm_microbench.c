#define _POSIX_C_SOURCE 200809L

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "root_xforms.h"
#include "solve_score.h"

#define REPS 9
#define ROOT_DEGREE 35

static volatile double bench_sink = 0.0;

static uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static int cmp_u64(const void *a, const void *b) {
    const uint64_t aa = *(const uint64_t *)a;
    const uint64_t bb = *(const uint64_t *)b;
    return (aa > bb) - (aa < bb);
}

static double median_ns_per_call(uint64_t values[REPS], long calls) {
    qsort(values, REPS, sizeof(values[0]), cmp_u64);
    return (double)values[REPS / 2] / (double)calls;
}

static void init_roots(float *re, float *im, int degree) {
    for (int i = 0; i < degree; i++) {
        const double theta = 2.0 * M_PI * (double)i / (double)degree;
        const double radius = 0.65 + 0.013 * (double)i;
        re[i] = (float)(radius * cos(theta));
        im[i] = (float)(radius * sin(theta));
    }
}

static double bench_root_chain(const RootXformEntry *chain, int n_chain, long calls) {
    float base_re[ROOT_DEGREE], base_im[ROOT_DEGREE];
    float re[ROOT_DEGREE], im[ROOT_DEGREE];
    uint64_t elapsed[REPS];
    init_roots(base_re, base_im, ROOT_DEGREE);
    for (int rep = 0; rep < REPS; rep++) {
        const uint64_t t0 = now_ns();
        for (long call = 0; call < calls; call++) {
            memcpy(re, base_re, sizeof(re));
            memcpy(im, base_im, sizeof(im));
            re[0] += (float)((call & 1023L) * 1e-9);
            apply_root_xforms(chain, n_chain, re, im, ROOT_DEGREE);
            bench_sink += re[call % ROOT_DEGREE] + im[(call + 7) % ROOT_DEGREE];
        }
        elapsed[rep] = now_ns() - t0;
    }
    return median_ns_per_call(elapsed, calls);
}

static SolveScoreProgram program_pass(void) {
    SolveScoreProgram p;
    memset(&p, 0, sizeof(p));
    p.metricCount = 2;
    p.tokenCount = 2;
    p.tokens[0] = (SolveScoreProgramToken){SOLVE_SCORE_OP_PUSH_METRIC, 0, 0, 0.0, 0.0};
    p.tokens[1] = (SolveScoreProgramToken){SOLVE_SCORE_OP_EMIT, 0, 0, 0.0, 0.0};
    p.outputCount = 1;
    p.hasExplicitOutputs = 1;
    p.outputOps[0] = SOLVE_SCORE_OP_EMIT;
    return p;
}

static SolveScoreProgram program_arithmetic(void) {
    SolveScoreProgram p;
    memset(&p, 0, sizeof(p));
    p.metricCount = 2;
    p.tokenCount = 7;
    p.tokens[0] = (SolveScoreProgramToken){SOLVE_SCORE_OP_PUSH_METRIC, 0, 0, 0.0, 0.0};
    p.tokens[1] = (SolveScoreProgramToken){SOLVE_SCORE_OP_PUSH_METRIC, 1, 0, 0.0, 0.0};
    p.tokens[2] = (SolveScoreProgramToken){SOLVE_SCORE_OP_WEIGHTED_SUM, 0, 0, 0.6, 0.4};
    p.tokens[3] = (SolveScoreProgramToken){SOLVE_SCORE_OP_DUP, 0, 0, 0.0, 0.0};
    p.tokens[4] = (SolveScoreProgramToken){SOLVE_SCORE_OP_FLIP, 0, 0, 0.0, 0.0};
    p.tokens[5] = (SolveScoreProgramToken){SOLVE_SCORE_OP_AVG, 0, 0, 0.0, 0.0};
    p.tokens[6] = (SolveScoreProgramToken){SOLVE_SCORE_OP_EMIT, 0, 0, 0.0, 0.0};
    p.outputCount = 1;
    p.hasExplicitOutputs = 1;
    p.outputOps[0] = SOLVE_SCORE_OP_EMIT;
    return p;
}

static SolveScoreProgram program_transcendental(void) {
    SolveScoreProgram p;
    memset(&p, 0, sizeof(p));
    p.metricCount = 1;
    p.tokenCount = 7;
    p.tokens[0] = (SolveScoreProgramToken){SOLVE_SCORE_OP_PUSH_METRIC, 0, 0, 0.0, 0.0};
    p.tokens[1] = (SolveScoreProgramToken){SOLVE_SCORE_OP_SIN, 0, 0, 0.0, 0.0};
    p.tokens[2] = (SolveScoreProgramToken){SOLVE_SCORE_OP_COS, 0, 0, 0.0, 0.0};
    p.tokens[3] = (SolveScoreProgramToken){SOLVE_SCORE_OP_EXP, 0, 0, 0.0, 0.0};
    p.tokens[4] = (SolveScoreProgramToken){SOLVE_SCORE_OP_POW, 0, 0, 1.7, 0.0};
    p.tokens[5] = (SolveScoreProgramToken){SOLVE_SCORE_OP_OMEGA_COSINE, 0, 0, 3.0, 0.125};
    p.tokens[6] = (SolveScoreProgramToken){SOLVE_SCORE_OP_EMIT, 0, 0, 0.0, 0.0};
    p.outputCount = 1;
    p.hasExplicitOutputs = 1;
    p.outputOps[0] = SOLVE_SCORE_OP_EMIT;
    return p;
}

static SolveScoreProgram program_long(void) {
    SolveScoreProgram p;
    memset(&p, 0, sizeof(p));
    p.metricCount = 4;
    int n = 0;
    p.tokens[n++] = (SolveScoreProgramToken){SOLVE_SCORE_OP_PUSH_METRIC, 0, 0, 0.0, 0.0};
    for (int i = 0; i < 14; i++) {
        p.tokens[n++] = (SolveScoreProgramToken){SOLVE_SCORE_OP_PUSH_METRIC, (i + 1) % 4, 0, 0.0, 0.0};
        p.tokens[n++] = (SolveScoreProgramToken){SOLVE_SCORE_OP_ADD, 0, 0, 0.0, 0.0};
    }
    p.tokens[n++] = (SolveScoreProgramToken){SOLVE_SCORE_OP_CLAMP, 0, 0, 0.0, 0.0};
    p.tokens[n++] = (SolveScoreProgramToken){SOLVE_SCORE_OP_EMIT, 0, 0, 0.0, 0.0};
    p.tokenCount = n;
    p.outputCount = 1;
    p.hasExplicitOutputs = 1;
    p.outputOps[0] = SOLVE_SCORE_OP_EMIT;
    return p;
}

static double bench_solve_vm(SolveScoreProgram *program, long calls) {
    float current[SOLVE_SCORE_MAX_METRIC_SLOTS] = {0};
    float recent[SOLVE_SCORE_MAX_METRIC_SLOTS] = {0};
    double outputs[SOLVE_SCORE_MAX_OUTPUT_CHANNELS];
    int out_count = 0;
    uint64_t elapsed[REPS];
    for (int i = 0; i < SOLVE_SCORE_MAX_METRIC_SLOTS; i++) {
        current[i] = (float)(0.1 + 0.03 * i);
        recent[i] = (float)(0.2 + 0.02 * i);
    }
    for (int rep = 0; rep < REPS; rep++) {
        const uint64_t t0 = now_ns();
        for (long call = 0; call < calls; call++) {
            current[0] = (float)(0.1 + (call & 1023L) * 1e-7);
            if (!solve_score_eval_program_outputs_from_buffers(
                    current, recent, program, outputs,
                    SOLVE_SCORE_MAX_OUTPUT_CHANNELS, &out_count)) {
                abort();
            }
            bench_sink += outputs[0] + (double)out_count;
        }
        elapsed[rep] = now_ns() - t0;
    }
    return median_ns_per_call(elapsed, calls);
}

/* CR31 F4: five slots (proximity, crowding, clusteriness, nn_variation, and a
 * duplicate-quantile proximity) evaluated the way production does — through
 * solve_score_eval_metric_slots. Before the feature cache this ran the O(d^2)
 * pair loop three times plus two NN passes; after, exactly one traversal. */
static double bench_metric_bundle(long calls) {
    float roots[ROOT_DEGREE * 2];
    float outBuf[SOLVE_SCORE_MAX_METRIC_SLOTS];
    uint64_t elapsed[REPS];
    for (int i = 0; i < ROOT_DEGREE; i++) {
        const double theta = 2.0 * M_PI * (double)i / (double)ROOT_DEGREE;
        roots[2 * i] = (float)((0.6 + 0.01 * i) * cos(theta));
        roots[2 * i + 1] = (float)((0.6 + 0.01 * i) * sin(theta));
    }
    SolveScoreProgram prog;
    memset(&prog, 0, sizeof(prog));
    prog.metricCount = 5;
    prog.metrics[0] = SOLVE_METRIC_PROXIMITY;
    prog.metrics[1] = SOLVE_METRIC_CROWDING;
    prog.metrics[2] = SOLVE_METRIC_CLUSTERINESS;
    prog.metrics[3] = SOLVE_METRIC_NN_VARIATION;
    prog.metrics[4] = SOLVE_METRIC_PROXIMITY;   /* duplicate, different clip */
    for (int i = 0; i < prog.metricCount; i++) {
        prog.metricSources[i] = SOLVE_SCORE_SOURCE_SOLVE;
        prog.clipLo[i] = 0.0;
        prog.clipHi[i] = 4.0 + i;
    }
    for (int rep = 0; rep < REPS; rep++) {
        const uint64_t t0 = now_ns();
        for (long call = 0; call < calls; call++) {
            roots[0] += (float)((call & 1L) ? 1e-7 : -1e-7);
            solve_score_eval_metric_slots(roots, ROOT_DEGREE, NULL, 0, NULL, 0, &prog, outBuf);
            bench_sink += outBuf[0] + outBuf[3];
        }
        elapsed[rep] = now_ns() - t0;
    }
    return median_ns_per_call(elapsed, calls);
}

static double bench_metric(enum SolveMetric metric, long calls) {
    float roots[ROOT_DEGREE * 2];
    uint64_t elapsed[REPS];
    for (int i = 0; i < ROOT_DEGREE; i++) {
        const double theta = 2.0 * M_PI * (double)i / (double)ROOT_DEGREE;
        roots[2 * i] = (float)((0.6 + 0.01 * i) * cos(theta));
        roots[2 * i + 1] = (float)((0.6 + 0.01 * i) * sin(theta));
    }
    for (int rep = 0; rep < REPS; rep++) {
        const uint64_t t0 = now_ns();
        for (long call = 0; call < calls; call++) {
            roots[0] += (float)((call & 1L) ? 1e-7 : -1e-7);
            bench_sink += compute_solve_metric_score(roots, ROOT_DEGREE, metric);
        }
        elapsed[rep] = now_ns() - t0;
    }
    return median_ns_per_call(elapsed, calls);
}

int main(void) {
    RootXformEntry rotate = {.fn_index = RT_FN_ROTATE_ROOTS, .n_args = 1, .args = {0.125}};
    RootXformEntry pull = {.fn_index = RT_FN_PULL_UNIT_CIRCLE, .n_args = 2, .args = {0.75, 1.0}};
    RootXformEntry affine[3] = {
        {.fn_index = RT_FN_ROTATE_ROOTS, .n_args = 1, .args = {0.125}},
        {.fn_index = RT_FN_MUL_COMPLEX, .n_args = 2, .args = {0.9, 0.1}},
        {.fn_index = RT_FN_ADD_COMPLEX, .n_args = 2, .args = {0.2, -0.1}},
    };
    RootXformEntry sixteen[MAX_RT_CHAIN];
    for (int i = 0; i < MAX_RT_CHAIN; i++) {
        sixteen[i] = affine[i % 3];
    }

    SolveScoreProgram pass = program_pass();
    /* CR31 F4 acceptance case: the four pair-family metrics from one source,
     * plus a duplicate-quantile proximity slot — one traversal expected. */
    SolveScoreProgram arithmetic = program_arithmetic();
    SolveScoreProgram transcendental = program_transcendental();
    SolveScoreProgram long_program = program_long();

    const long root_calls = 200000;
    const long vm_calls = 3000000;
    const long metric_calls = 100000;
    const double root_identity = bench_root_chain(NULL, 0, root_calls);

    printf("{\n");
    printf("  \"root_degree\": %d,\n", ROOT_DEGREE);
    printf("  \"root_copy_only_ns\": %.3f,\n", root_identity);
    printf("  \"root_rotate_ns\": %.3f,\n", bench_root_chain(&rotate, 1, root_calls));
    printf("  \"root_affine3_ns\": %.3f,\n", bench_root_chain(affine, 3, root_calls));
    printf("  \"root_pull_ns\": %.3f,\n", bench_root_chain(&pull, 1, root_calls));
    printf("  \"root_affine16_ns\": %.3f,\n", bench_root_chain(sixteen, MAX_RT_CHAIN, root_calls));
    printf("  \"solve_vm_pass_2tok_ns\": %.3f,\n", bench_solve_vm(&pass, vm_calls));
    printf("  \"solve_vm_arithmetic_7tok_ns\": %.3f,\n", bench_solve_vm(&arithmetic, vm_calls));
    printf("  \"solve_vm_transcendental_7tok_ns\": %.3f,\n", bench_solve_vm(&transcendental, vm_calls));
    printf("  \"solve_vm_long_31tok_ns\": %.3f,\n", bench_solve_vm(&long_program, vm_calls));
    printf("  \"metric_max_re_ns\": %.3f,\n", bench_metric(SOLVE_METRIC_MAX_RE, metric_calls));
    printf("  \"metric_proximity_ns\": %.3f,\n", bench_metric(SOLVE_METRIC_PROXIMITY, metric_calls));
    printf("  \"metric_clusteriness_ns\": %.3f,\n", bench_metric(SOLVE_METRIC_CLUSTERINESS, metric_calls));
    printf("  \"metric_min_angular_ns\": %.3f,\n", bench_metric(SOLVE_METRIC_MIN_ANGULAR_SEPARATION, metric_calls));
    printf("  \"metric_bundle_pair4_ns\": %.3f,\n", bench_metric_bundle(metric_calls));
    printf("  \"sink\": %.17g\n", bench_sink);
    printf("}\n");
    return 0;
}
