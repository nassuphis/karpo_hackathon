#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static const char *get_arg(int argc, char **argv, const char *key) {
    size_t key_len = strlen(key);
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], key, key_len) == 0 && argv[i][key_len] == '=') {
            return argv[i] + key_len + 1;
        }
    }
    return NULL;
}

static int parse_positive_int(const char *value, const char *label) {
    char *end = NULL;
    long parsed = strtol(value, &end, 10);
    if (!value || !*value || !end || *end != '\0' || parsed <= 0 || parsed > 1000000L) {
        fprintf(stderr, "%s must be a positive integer, got %s\n", label, value ? value : "(null)");
        return 0;
    }
    return (int)parsed;
}

static uint64_t parse_positive_u64(const char *value, const char *label) {
    char *end = NULL;
    unsigned long long parsed = strtoull(value, &end, 10);
    if (!value || !*value || !end || *end != '\0' || parsed == 0ULL) {
        fprintf(stderr, "%s must be a positive integer, got %s\n", label, value ? value : "(null)");
        return 0ULL;
    }
    return (uint64_t)parsed;
}

int main(int argc, char **argv) {
    const char *input_path = get_arg(argc, argv, "--input");
    const char *output_path = get_arg(argc, argv, "--output");
    const char *grid_n_str = get_arg(argc, argv, "--grid-n");
    const char *step_count_str = get_arg(argc, argv, "--step-count");

    if (!input_path || !output_path || !grid_n_str || !step_count_str) {
        fprintf(
            stderr,
            "Usage: step_scores_to_palette_raw --input=step_scores.raw --output=palette.raw "
            "--grid-n=N --step-count=COUNT\n"
        );
        return 1;
    }

    int grid_n = parse_positive_int(grid_n_str, "grid-n");
    uint64_t step_count = parse_positive_u64(step_count_str, "step-count");
    if (grid_n <= 0 || step_count == 0ULL) {
        return 1;
    }

    uint64_t palette_pixels = (uint64_t)grid_n * (uint64_t)grid_n;
    if (palette_pixels == 0ULL || palette_pixels > (uint64_t)SIZE_MAX) {
        fprintf(stderr, "grid-n is too large: %d\n", grid_n);
        return 1;
    }
    if (step_count < palette_pixels) {
        fprintf(
            stderr,
            "step-count must be at least grid-n^2 (%llu), got %llu\n",
            (unsigned long long)palette_pixels,
            (unsigned long long)step_count
        );
        return 1;
    }

    struct stat st;
    if (stat(input_path, &st) != 0) {
        fprintf(stderr, "stat(%s) failed: %s\n", input_path, strerror(errno));
        return 1;
    }
    if ((uint64_t)st.st_size < palette_pixels) {
        fprintf(
            stderr,
            "input is shorter than grid-n^2 bytes: need %llu, got %lld\n",
            (unsigned long long)palette_pixels,
            (long long)st.st_size
        );
        return 1;
    }
    if ((uint64_t)st.st_size < step_count) {
        fprintf(
            stderr,
            "input is shorter than advertised step-count: need %llu, got %lld\n",
            (unsigned long long)step_count,
            (long long)st.st_size
        );
        return 1;
    }

    FILE *fin = fopen(input_path, "rb");
    if (!fin) {
        fprintf(stderr, "fopen(%s) failed: %s\n", input_path, strerror(errno));
        return 1;
    }

    size_t palette_size = (size_t)palette_pixels;
    unsigned char *step_scores = (unsigned char *)malloc(palette_size);
    unsigned char *palette = (unsigned char *)calloc(palette_size, 1);
    if (!step_scores || !palette) {
        fprintf(stderr, "out of memory allocating %zu-byte buffers\n", palette_size);
        fclose(fin);
        free(step_scores);
        free(palette);
        return 1;
    }

    size_t nread = fread(step_scores, 1, palette_size, fin);
    fclose(fin);
    if (nread != palette_size) {
        fprintf(stderr, "failed to read first %zu step-score bytes, got %zu\n", palette_size, nread);
        free(step_scores);
        free(palette);
        return 1;
    }

    for (uint64_t s = 0; s < palette_pixels; s++) {
        uint64_t row = s / (uint64_t)grid_n;
        uint64_t j = s % (uint64_t)grid_n;
        uint64_t col = (row & 1ULL) ? ((uint64_t)grid_n - 1ULL - j) : j;
        palette[row * (uint64_t)grid_n + col] = step_scores[s];
    }

    FILE *fout = fopen(output_path, "wb");
    if (!fout) {
        fprintf(stderr, "fopen(%s) failed: %s\n", output_path, strerror(errno));
        free(step_scores);
        free(palette);
        return 1;
    }
    size_t nwritten = fwrite(palette, 1, palette_size, fout);
    fclose(fout);
    free(step_scores);
    free(palette);
    if (nwritten != palette_size) {
        fprintf(stderr, "failed to write %zu bytes to %s\n", palette_size, output_path);
        return 1;
    }
    return 0;
}
