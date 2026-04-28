/*
 * Local-only Coeff Program benchmark spike.
 *
 * Includes sweep_cli.c in this translation unit so the benchmark can call the
 * existing static coefficient-transform functions without changing shipped
 * binary entry points.
 */
#define main sweep_cli_embedded_main
#include "sweep_cli.c"
#undef main

#define COEFF_BENCH_MAX_VECTOR_STACK 64
#define COEFF_BENCH_MAX_VECTOR_LEN 256
#define COEFF_BENCH_MAX_REPS 25

typedef struct {
    double stack_re[COEFF_BENCH_MAX_VECTOR_STACK][COEFF_BENCH_MAX_VECTOR_LEN];
    double stack_im[COEFF_BENCH_MAX_VECTOR_STACK][COEFF_BENCH_MAX_VECTOR_LEN];
    uint16_t stack_len[COEFF_BENCH_MAX_VECTOR_STACK];
    uint16_t stack_depth;
    uint16_t stack_head;

    double poly_re[COEFF_BENCH_MAX_VECTOR_LEN];
    double poly_im[COEFF_BENCH_MAX_VECTOR_LEN];
    uint16_t poly_len;
} CoeffBenchWorkspace;

static void coeff_bench_fill_base(double *re, double *im, int n) {
    for (int i = 0; i < n; i++) {
        re[i] = (double)(i + 1);
        im[i] = (double)(i + 1);
    }
}

static void coeff_bench_copy(double *dst_re, double *dst_im,
                             const double *src_re, const double *src_im,
                             int n) {
    memcpy(dst_re, src_re, (size_t)n * sizeof(double));
    memcpy(dst_im, src_im, (size_t)n * sizeof(double));
}

static void coeff_bench_mix_vec(uint64_t *h, const double *re, const double *im, int n) {
    for (int i = 0; i < n; i++) {
        bench_mix_u64(h, re[i]);
        bench_mix_u64(h, im[i]);
    }
}

static uint64_t coeff_bench_legacy_rev(long evals, int n,
                                       const double *base_re, const double *base_im,
                                       const CtEntry *entry) {
    double re[COEFF_BENCH_MAX_VECTOR_LEN], im[COEFF_BENCH_MAX_VECTOR_LEN];
    uint64_t h = 0xcbf29ce484222325ULL;
    for (long row = 0; row < evals; row++) {
        int len = n;
        coeff_bench_copy(re, im, base_re, base_im, n);
        if (dispatchCt(entry, re, im, &len) != 0) return 0;
        coeff_bench_mix_vec(&h, re, im, len);
    }
    return h;
}

static uint64_t coeff_bench_program_poly_rev(long evals, int n,
                                             const double *base_re, const double *base_im,
                                             CoeffTransform fn,
                                             CoeffBenchWorkspace *ws) {
    uint64_t h = 0xcbf29ce484222325ULL;
    for (long row = 0; row < evals; row++) {
        int len = n;
        coeff_bench_copy(ws->poly_re, ws->poly_im, base_re, base_im, n);
        fn(ws->poly_re, ws->poly_im, &len);
        ws->poly_len = (uint16_t)len;
        coeff_bench_mix_vec(&h, ws->poly_re, ws->poly_im, len);
    }
    return h;
}

static int coeff_bench_push_cf(CoeffBenchWorkspace *ws,
                               const double *re, const double *im, int n) {
    if (ws->stack_depth >= COEFF_BENCH_MAX_VECTOR_STACK) return 0;
    uint16_t slot = ws->stack_head;
    coeff_bench_copy(ws->stack_re[slot], ws->stack_im[slot], re, im, n);
    ws->stack_len[slot] = (uint16_t)n;
    ws->stack_head = (uint16_t)((ws->stack_head + 1) % COEFF_BENCH_MAX_VECTOR_STACK);
    ws->stack_depth++;
    return 1;
}

static int coeff_bench_pop_slot(CoeffBenchWorkspace *ws, uint16_t *slot) {
    if (ws->stack_depth == 0) return 0;
    ws->stack_head = (uint16_t)((ws->stack_head + COEFF_BENCH_MAX_VECTOR_STACK - 1) %
                                COEFF_BENCH_MAX_VECTOR_STACK);
    ws->stack_depth--;
    *slot = ws->stack_head;
    return 1;
}

static uint64_t coeff_bench_program_stack_rev(long evals, int n,
                                              const double *base_re, const double *base_im,
                                              CoeffTransform fn,
                                              CoeffBenchWorkspace *ws) {
    uint64_t h = 0xcbf29ce484222325ULL;
    for (long row = 0; row < evals; row++) {
        uint16_t slot = 0;
        memset(ws, 0, sizeof(*ws));
        if (!coeff_bench_push_cf(ws, base_re, base_im, n)) return 0;
        if (!coeff_bench_pop_slot(ws, &slot)) return 0;
        int len = ws->stack_len[slot];
        fn(ws->stack_re[slot], ws->stack_im[slot], &len);
        coeff_bench_copy(ws->poly_re, ws->poly_im, ws->stack_re[slot], ws->stack_im[slot], len);
        ws->poly_len = (uint16_t)len;
        coeff_bench_mix_vec(&h, ws->poly_re, ws->poly_im, len);
    }
    return h;
}

static long coeff_bench_time_legacy_us(long evals, int n,
                                       const double *base_re, const double *base_im,
                                       const CtEntry *entry,
                                       uint64_t *checksum) {
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    *checksum = coeff_bench_legacy_rev(evals, n, base_re, base_im, entry);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    return bench_elapsed_us(t0, t1);
}

static long coeff_bench_time_program_poly_us(long evals, int n,
                                             const double *base_re, const double *base_im,
                                             CoeffTransform fn,
                                             CoeffBenchWorkspace *ws,
                                             uint64_t *checksum) {
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    *checksum = coeff_bench_program_poly_rev(evals, n, base_re, base_im, fn, ws);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    return bench_elapsed_us(t0, t1);
}

static long coeff_bench_time_program_stack_us(long evals, int n,
                                              const double *base_re, const double *base_im,
                                              CoeffTransform fn,
                                              CoeffBenchWorkspace *ws,
                                              uint64_t *checksum) {
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    *checksum = coeff_bench_program_stack_rev(evals, n, base_re, base_im, fn, ws);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    return bench_elapsed_us(t0, t1);
}

static void coeff_bench_print_case(const char *name, long evals, long median_us,
                                   long min_us, uint64_t checksum,
                                   long baseline_us, uint64_t baseline_checksum) {
    double rows_per_sec = (double)evals * 1000000.0 / (double)median_us;
    double ns_per_eval = (double)median_us * 1000.0 / (double)evals;
    double penalty = baseline_us > 0
        ? 100.0 * ((double)median_us - (double)baseline_us) / (double)baseline_us
        : 0.0;
    printf("{\"name\":\"%s\",\"median_us\":%ld,\"min_us\":%ld,"
           "\"ns_per_eval\":%.2f,\"evals_per_sec\":%.0f,"
           "\"penalty_vs_legacy_pct\":%.2f,\"checksum_match\":%s,"
           "\"checksum\":\"%016llx\"}",
           name, median_us, min_us, ns_per_eval, rows_per_sec, penalty,
           checksum == baseline_checksum ? "true" : "false",
           (unsigned long long)checksum);
}

int main(int argc, char **argv) {
    long evals = 10000000L;
    int reps = 3;
    int n = 35;
    if (argc > 1) {
        long parsed = strtol(argv[1], NULL, 10);
        if (parsed > 0) evals = parsed;
    }
    if (argc > 2) {
        int parsed = (int)strtol(argv[2], NULL, 10);
        if (parsed > 0 && parsed <= COEFF_BENCH_MAX_REPS) reps = parsed;
    }
    if (argc > 3) {
        int parsed = (int)strtol(argv[3], NULL, 10);
        if (parsed > 0 && parsed <= COEFF_BENCH_MAX_VECTOR_LEN) n = parsed;
    }

    double base_re[COEFF_BENCH_MAX_VECTOR_LEN], base_im[COEFF_BENCH_MAX_VECTOR_LEN];
    coeff_bench_fill_base(base_re, base_im, n);

    CtEntry rev_entry;
    memset(&rev_entry, 0, sizeof(rev_entry));
    snprintf(rev_entry.name, sizeof(rev_entry.name), "rev");
    CoeffTransform rev_fn = lookupCoeffTransform("rev");
    if (!rev_fn) {
        fprintf(stderr, "rev transform not found\n");
        return 1;
    }

    CoeffBenchWorkspace *ws = calloc(1, sizeof(*ws));
    if (!ws) {
        fprintf(stderr, "workspace allocation failed\n");
        return 1;
    }

    long legacy_times[COEFF_BENCH_MAX_REPS];
    long program_poly_times[COEFF_BENCH_MAX_REPS];
    long program_stack_times[COEFF_BENCH_MAX_REPS];
    uint64_t legacy_checksum = 0;
    uint64_t program_poly_checksum = 0;
    uint64_t program_stack_checksum = 0;

    for (int r = 0; r < reps; r++) {
        legacy_times[r] = coeff_bench_time_legacy_us(
            evals, n, base_re, base_im, &rev_entry, &legacy_checksum);
        program_poly_times[r] = coeff_bench_time_program_poly_us(
            evals, n, base_re, base_im, rev_fn, ws, &program_poly_checksum);
        program_stack_times[r] = coeff_bench_time_program_stack_us(
            evals, n, base_re, base_im, rev_fn, ws, &program_stack_checksum);
    }

    long legacy_median = bench_median(legacy_times, reps);
    long program_poly_median = bench_median(program_poly_times, reps);
    long program_stack_median = bench_median(program_stack_times, reps);

    long legacy_min = bench_min_long(legacy_times, reps);
    long program_poly_min = bench_min_long(program_poly_times, reps);
    long program_stack_min = bench_min_long(program_stack_times, reps);

    printf("{\"mode\":\"coeff_program_bench\",\"transform\":\"rev\","
           "\"evals\":%ld,\"reps\":%d,\"n_coeffs\":%d,"
           "\"base_vector\":\"1+1i..%d+%di\","
           "\"max_vector_stack\":%d,\"max_vector_len\":%d,"
           "\"workspace_bytes_per_thread\":%zu,"
           "\"cases\":[",
           evals, reps, n, n, n,
           COEFF_BENCH_MAX_VECTOR_STACK, COEFF_BENCH_MAX_VECTOR_LEN,
           sizeof(*ws));
    coeff_bench_print_case("legacy_dispatchCt", evals, legacy_median, legacy_min,
                           legacy_checksum, legacy_median, legacy_checksum);
    printf(",");
    coeff_bench_print_case("program_poly_to_poly", evals, program_poly_median, program_poly_min,
                           program_poly_checksum, legacy_median, legacy_checksum);
    printf(",");
    coeff_bench_print_case("program_push_cf_pop_emit", evals, program_stack_median, program_stack_min,
                           program_stack_checksum, legacy_median, legacy_checksum);
    printf("]}\n");

    free(ws);
    return 0;
}
