/*
 * assemble_greyscale: assemble sparse u32le_u8_v1 score fragments into
 * a dense greyscale raw image.
 *
 * Input files contain repeated 5-byte pairs:
 *   [pixel_idx:uint32 little-endian][score_byte:uint8]
 *
 * score_byte == 0 is invalid. pixel_idx must be < width * height.
 * Repeated writes are accepted with "any arrival wins" semantics.
 *
 * Usage:
 *   assemble_greyscale --width=W --height=H --output=raw.bin \
 *     [--hist-output=hist.json] [--workers=N] frag0.frag [frag1.frag ...]
 */

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int width;
    int height;
    size_t npix;
    uint8_t *buf;
    char **paths;
    int n_paths;
    int next_idx;
    int failed;
    char error[512];
    pthread_mutex_t queue_mu;
    pthread_mutex_t err_mu;
} AssembleState;

static const char *getArg(int argc, char **argv, const char *key) {
    int klen = (int)strlen(key);
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], key, klen) == 0 && argv[i][klen] == '=') {
            return argv[i] + klen + 1;
        }
    }
    return NULL;
}

static int getArgInt(int argc, char **argv, const char *key, int def) {
    const char *v = getArg(argc, argv, key);
    return v ? atoi(v) : def;
}

static void set_error(AssembleState *st, const char *fmt, const char *path, long long a, long long b) {
    pthread_mutex_lock(&st->err_mu);
    if (!st->failed) {
        st->failed = 1;
        snprintf(st->error, sizeof(st->error), fmt, path, a, b);
    }
    pthread_mutex_unlock(&st->err_mu);
}

static int next_job(AssembleState *st) {
    int idx = -1;
    pthread_mutex_lock(&st->queue_mu);
    if (!st->failed && st->next_idx < st->n_paths) {
        idx = st->next_idx++;
    }
    pthread_mutex_unlock(&st->queue_mu);
    return idx;
}

static int process_fragment_file(AssembleState *st, const char *path) {
    FILE *fh = fopen(path, "rb");
    if (!fh) {
        set_error(st, "assemble_greyscale: cannot open %s (%lld)", path, (long long)errno, 0);
        return 0;
    }
    if (fseek(fh, 0, SEEK_END) != 0) {
        fclose(fh);
        set_error(st, "assemble_greyscale: cannot seek %s (%lld)", path, (long long)errno, 0);
        return 0;
    }
    long size = ftell(fh);
    if (size < 0) {
        fclose(fh);
        set_error(st, "assemble_greyscale: cannot stat %s (%lld)", path, (long long)errno, 0);
        return 0;
    }
    if ((size % 5) != 0) {
        fclose(fh);
        set_error(st, "assemble_greyscale: fragment %s size %lld is not divisible by 5", path, size, 0);
        return 0;
    }
    if (fseek(fh, 0, SEEK_SET) != 0) {
        fclose(fh);
        set_error(st, "assemble_greyscale: cannot rewind %s (%lld)", path, (long long)errno, 0);
        return 0;
    }
    uint8_t *data = NULL;
    if (size > 0) {
        data = (uint8_t *)malloc((size_t)size);
        if (!data) {
            fclose(fh);
            set_error(st, "assemble_greyscale: out of memory reading %s (%lld bytes)", path, size, 0);
            return 0;
        }
        if (fread(data, 1, (size_t)size, fh) != (size_t)size) {
            free(data);
            fclose(fh);
            set_error(st, "assemble_greyscale: short read on %s (%lld bytes)", path, size, 0);
            return 0;
        }
    }
    fclose(fh);

    for (long off = 0; off < size; off += 5) {
        uint32_t pixel_idx =
            ((uint32_t)data[off]) |
            ((uint32_t)data[off + 1] << 8) |
            ((uint32_t)data[off + 2] << 16) |
            ((uint32_t)data[off + 3] << 24);
        uint8_t score = data[off + 4];
        if (score == 0) {
            free(data);
            set_error(st, "assemble_greyscale: fragment %s has invalid zero score at pair %lld", path, off / 5, 0);
            return 0;
        }
        if ((size_t)pixel_idx >= st->npix) {
            free(data);
            set_error(
                st,
                "assemble_greyscale: fragment %s pixel_idx %lld out of bounds for npix=%lld",
                path,
                (long long)pixel_idx,
                (long long)st->npix
            );
            return 0;
        }
        st->buf[pixel_idx] = score;
    }

    free(data);
    return 1;
}

static void *worker_main(void *arg) {
    AssembleState *st = (AssembleState *)arg;
    for (;;) {
        int idx = next_job(st);
        if (idx < 0) break;
        if (!process_fragment_file(st, st->paths[idx])) {
            break;
        }
    }
    return NULL;
}

static int write_histogram_json(const char *path, int width, int height, const uint8_t *buf, size_t npix) {
    unsigned long long hist[256];
    memset(hist, 0, sizeof(hist));
    for (size_t i = 0; i < npix; i++) hist[buf[i]] += 1ULL;
    FILE *fh = fopen(path, "wb");
    if (!fh) return 0;
    fprintf(
        fh,
        "{\"version\":1,\"width\":%d,\"height\":%d,\"background_pixels\":%llu,\"nonzero_pixels\":%llu,\"histogram\":[",
        width,
        height,
        hist[0],
        (unsigned long long)(npix - (size_t)hist[0])
    );
    for (int i = 0; i < 256; i++) {
        if (i) fputc(',', fh);
        fprintf(fh, "%llu", hist[i]);
    }
    fputs("]}", fh);
    fclose(fh);
    return 1;
}

int main(int argc, char **argv) {
    const char *outPath = getArg(argc, argv, "--output");
    const char *histPath = getArg(argc, argv, "--hist-output");
    int width = getArgInt(argc, argv, "--width", 0);
    int height = getArgInt(argc, argv, "--height", 0);
    int workers = getArgInt(argc, argv, "--workers", 1);
    char **paths = NULL;
    int n_paths = 0;

    if (!outPath || width <= 0 || height <= 0) {
        fprintf(stderr, "Usage: assemble_greyscale --width=W --height=H --output=raw.bin [--hist-output=hist.json] [--workers=N] frag0 [frag1 ...]\n");
        return 2;
    }
    if (workers <= 0) workers = 1;

    paths = (char **)calloc((size_t)argc, sizeof(char *));
    if (!paths) {
        fprintf(stderr, "assemble_greyscale: out of memory\n");
        return 3;
    }
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--", 2) == 0) continue;
        paths[n_paths++] = argv[i];
    }

    size_t npix = (size_t)width * (size_t)height;
    uint8_t *buf = (uint8_t *)calloc(npix ? npix : 1, 1);
    if (!buf) {
        fprintf(stderr, "assemble_greyscale: cannot allocate %zu bytes\n", npix);
        free(paths);
        return 4;
    }

    AssembleState st;
    memset(&st, 0, sizeof(st));
    st.width = width;
    st.height = height;
    st.npix = npix;
    st.buf = buf;
    st.paths = paths;
    st.n_paths = n_paths;
    pthread_mutex_init(&st.queue_mu, NULL);
    pthread_mutex_init(&st.err_mu, NULL);

    pthread_t *threads = (pthread_t *)calloc((size_t)workers, sizeof(pthread_t));
    if (!threads) {
        fprintf(stderr, "assemble_greyscale: cannot allocate worker threads\n");
        free(buf);
        free(paths);
        return 5;
    }

    for (int i = 0; i < workers; i++) {
        if (pthread_create(&threads[i], NULL, worker_main, &st) != 0) {
            fprintf(stderr, "assemble_greyscale: pthread_create failed\n");
            free(threads);
            free(buf);
            free(paths);
            return 6;
        }
    }
    for (int i = 0; i < workers; i++) pthread_join(threads[i], NULL);

    pthread_mutex_destroy(&st.queue_mu);
    pthread_mutex_destroy(&st.err_mu);
    free(threads);
    free(paths);

    if (st.failed) {
        fprintf(stderr, "%s\n", st.error);
        free(buf);
        return 7;
    }

    FILE *out = fopen(outPath, "wb");
    if (!out) {
        fprintf(stderr, "assemble_greyscale: cannot create %s\n", outPath);
        free(buf);
        return 8;
    }
    if (npix > 0 && fwrite(buf, 1, npix, out) != npix) {
        fclose(out);
        fprintf(stderr, "assemble_greyscale: short write to %s\n", outPath);
        free(buf);
        return 9;
    }
    fclose(out);

    if (histPath && !write_histogram_json(histPath, width, height, buf, npix)) {
        fprintf(stderr, "assemble_greyscale: cannot write histogram %s\n", histPath);
        free(buf);
        return 10;
    }

    free(buf);
    return 0;
}
