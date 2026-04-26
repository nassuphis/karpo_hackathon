/*
 * assemble_greyscale: assemble sparse score fragments into a dense raw image.
 *
 * Input fragments contain repeated records:
 *   [pixel_idx:uint32 little-endian][channel_byte:uint8]...
 *
 * Fragments may be passed as local file paths or via --url-manifest=<path>,
 * one presigned URL per line.
 *
 * For legacy scalar mode, score_byte == 0 is invalid unless --allow-zero=1.
 * pixel_idx must be < pix * pix.
 * Repeated writes are accepted with "any arrival wins" semantics.
 */

#include <ctype.h>
#include <curl/curl.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t *data;
    size_t size;
    size_t cap;
} DownloadBuffer;

typedef struct {
    int width;
    int height;
    int channels;
    int allow_zero;
    size_t record_size;
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

typedef struct {
    AssembleState *st;
    CURL *curl;
} WorkerCtx;

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

static int is_url(const char *path) {
    return path && (
        strncmp(path, "http://", 7) == 0 ||
        strncmp(path, "https://", 8) == 0
    );
}

static size_t write_download_cb(char *ptr, size_t size, size_t nmemb, void *userdata) {
    DownloadBuffer *dl = (DownloadBuffer *)userdata;
    size_t n = size * nmemb;
    if (n == 0) return 0;
    if (dl->size + n > dl->cap) {
        size_t newCap = dl->cap ? dl->cap * 2 : 65536;
        while (newCap < dl->size + n) newCap *= 2;
        uint8_t *grown = (uint8_t *)realloc(dl->data, newCap);
        if (!grown) return 0;
        dl->data = grown;
        dl->cap = newCap;
    }
    memcpy(dl->data + dl->size, ptr, n);
    dl->size += n;
    return n;
}

static int download_url_bytes(WorkerCtx *ctx, const char *url, uint8_t **outData, size_t *outSize) {
    char curlErr[CURL_ERROR_SIZE] = {0};
    long httpStatus = 0;
    CURLcode rc;
    DownloadBuffer dl;
    memset(&dl, 0, sizeof(dl));

    if (!ctx->curl) {
        ctx->curl = curl_easy_init();
        if (!ctx->curl) {
            return 0;
        }
    }

    curl_easy_reset(ctx->curl);
    curl_easy_setopt(ctx->curl, CURLOPT_URL, url);
    curl_easy_setopt(ctx->curl, CURLOPT_WRITEFUNCTION, write_download_cb);
    curl_easy_setopt(ctx->curl, CURLOPT_WRITEDATA, &dl);
    curl_easy_setopt(ctx->curl, CURLOPT_ERRORBUFFER, curlErr);
    curl_easy_setopt(ctx->curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(ctx->curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(ctx->curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(ctx->curl, CURLOPT_ACCEPT_ENCODING, "identity");
    curl_easy_setopt(ctx->curl, CURLOPT_TCP_KEEPALIVE, 1L);

    rc = curl_easy_perform(ctx->curl);
    curl_easy_getinfo(ctx->curl, CURLINFO_RESPONSE_CODE, &httpStatus);
    if (rc != CURLE_OK) {
        free(dl.data);
        set_error(
            ctx->st,
            "assemble_greyscale: failed to download %s (%lld %lld)",
            url,
            (long long)rc,
            (long long)httpStatus
        );
        return 0;
    }

    *outData = dl.data;
    *outSize = dl.size;
    return 1;
}

static int load_local_bytes(AssembleState *st, const char *path, uint8_t **outData, size_t *outSize) {
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
    *outData = data;
    *outSize = (size_t)(size > 0 ? size : 0);
    return 1;
}

static int process_fragment_bytes(AssembleState *st, const char *path, uint8_t *data, size_t size) {
    if (st->record_size < 5) {
        set_error(st, "assemble_greyscale: invalid record size for %s (%lld)", path, (long long)st->record_size, 0);
        return 0;
    }
    if ((size % st->record_size) != 0) {
        set_error(
            st,
            "assemble_greyscale: fragment %s size %lld is not divisible by record size %lld",
            path,
            (long long)size,
            (long long)st->record_size
        );
        return 0;
    }
    for (size_t off = 0; off < size; off += st->record_size) {
        uint32_t pixel_idx =
            ((uint32_t)data[off]) |
            ((uint32_t)data[off + 1] << 8) |
            ((uint32_t)data[off + 2] << 16) |
            ((uint32_t)data[off + 3] << 24);
        if (!st->allow_zero && st->channels == 1 && data[off + 4] == 0) {
            set_error(st, "assemble_greyscale: fragment %s has invalid zero score at pair %lld", path, (long long)(off / st->record_size), 0);
            return 0;
        }
        if ((size_t)pixel_idx >= st->npix) {
            set_error(
                st,
                "assemble_greyscale: fragment %s pixel_idx %lld out of bounds for npix=%lld",
                path,
                (long long)pixel_idx,
                (long long)st->npix
            );
            return 0;
        }
        memcpy(st->buf + ((size_t)pixel_idx * (size_t)st->channels), data + off + 4, (size_t)st->channels);
    }
    return 1;
}

static int process_fragment_source(WorkerCtx *ctx, const char *path) {
    uint8_t *data = NULL;
    size_t size = 0;
    int ok = 0;

    if (is_url(path)) {
        if (!download_url_bytes(ctx, path, &data, &size)) {
            return 0;
        }
    } else {
        if (!load_local_bytes(ctx->st, path, &data, &size)) {
            return 0;
        }
    }

    ok = process_fragment_bytes(ctx->st, path, data, size);
    free(data);
    return ok;
}

static void *worker_main(void *arg) {
    WorkerCtx *ctx = (WorkerCtx *)arg;
    AssembleState *st = ctx->st;
    for (;;) {
        int idx = next_job(st);
        if (idx < 0) break;
        if (!process_fragment_source(ctx, st->paths[idx])) {
            break;
        }
    }
    if (ctx->curl) {
        curl_easy_cleanup(ctx->curl);
        ctx->curl = NULL;
    }
    return NULL;
}

static int write_histogram_json(const char *path, int width, int height, int channels, const uint8_t *buf, size_t npix) {
    unsigned long long hist[256];
    memset(hist, 0, sizeof(hist));
    unsigned long long background = 0;
    for (size_t i = 0; i < npix; i++) {
        const uint8_t *px = buf + i * (size_t)channels;
        hist[px[0]] += 1ULL;
        int any = 0;
        for (int ch = 0; ch < channels; ch++) {
            if (px[ch] != 0) {
                any = 1;
                break;
            }
        }
        if (!any) background++;
    }
    FILE *fh = fopen(path, "wb");
    if (!fh) return 0;
    fprintf(
        fh,
        "{\"version\":1,\"width\":%d,\"height\":%d,\"channels\":%d,\"histogram_channel\":0,\"background_pixels\":%llu,\"nonzero_pixels\":%llu,\"histogram\":[",
        width,
        height,
        channels,
        background,
        (unsigned long long)(npix - (size_t)background)
    );
    for (int i = 0; i < 256; i++) {
        if (i) fputc(',', fh);
        fprintf(fh, "%llu", hist[i]);
    }
    fputs("]}", fh);
    fclose(fh);
    return 1;
}

static char *trim_line(char *line) {
    char *start = line;
    char *end;
    while (*start && isspace((unsigned char)*start)) start++;
    end = start + strlen(start);
    while (end > start && isspace((unsigned char)end[-1])) {
        end--;
    }
    *end = '\0';
    return start;
}

static int append_path(char ***paths, int *count, int *cap, const char *value) {
    if (*count >= *cap) {
        int nextCap = (*cap > 0) ? (*cap * 2) : 16;
        char **grown = (char **)realloc(*paths, (size_t)nextCap * sizeof(char *));
        if (!grown) return 0;
        *paths = grown;
        *cap = nextCap;
    }
    (*paths)[*count] = strdup(value);
    if (!(*paths)[*count]) return 0;
    (*count)++;
    return 1;
}

static int load_url_manifest(const char *manifestPath, char ***paths, int *count, int *cap) {
    FILE *fh = fopen(manifestPath, "r");
    if (!fh) return 0;
    char line[8192];
    while (fgets(line, sizeof(line), fh)) {
        char *trimmed = trim_line(line);
        if (!trimmed[0] || trimmed[0] == '#') continue;
        if (!append_path(paths, count, cap, trimmed)) {
            fclose(fh);
            return 0;
        }
    }
    fclose(fh);
    return 1;
}

int main(int argc, char **argv) {
    const char *outPath = getArg(argc, argv, "--output");
    const char *histPath = getArg(argc, argv, "--hist-output");
    const char *urlManifest = getArg(argc, argv, "--url-manifest");
    const char *widthArg = getArg(argc, argv, "--width");
    const char *heightArg = getArg(argc, argv, "--height");
    int pix = getArgInt(argc, argv, "--pix", 0);
    int channels = getArgInt(argc, argv, "--channels", 1);
    int allow_zero = getArgInt(argc, argv, "--allow-zero", 0);
    int workers = getArgInt(argc, argv, "--workers", 1);
    char **paths = NULL;
    int n_paths = 0;
    int cap_paths = 0;
    CURLcode curlRc;

    if (widthArg || heightArg) {
        fprintf(stderr, "assemble_greyscale no longer accepts --width or --height; pass --pix for square output\n");
        return 2;
    }
    if (!outPath || pix <= 0) {
        fprintf(stderr, "Usage: assemble_greyscale --pix=N --output=raw.bin [--channels=N] [--allow-zero=0|1] [--hist-output=hist.json] [--workers=N] [--url-manifest=urls.txt] frag0 [frag1 ...]\n");
        return 2;
    }
    if (channels < 1 || channels > 8) {
        fprintf(stderr, "assemble_greyscale: --channels must be in [1,8], got %d\n", channels);
        return 2;
    }
    int width = pix;
    int height = pix;
    if (workers <= 0) workers = 1;

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--", 2) == 0) continue;
        if (!append_path(&paths, &n_paths, &cap_paths, argv[i])) {
            fprintf(stderr, "assemble_greyscale: out of memory\n");
            return 3;
        }
    }
    if (urlManifest && *urlManifest) {
        if (!load_url_manifest(urlManifest, &paths, &n_paths, &cap_paths)) {
            fprintf(stderr, "assemble_greyscale: cannot read url manifest %s\n", urlManifest);
            return 3;
        }
    }

    curlRc = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (curlRc != CURLE_OK) {
        fprintf(stderr, "assemble_greyscale: curl_global_init failed: %s\n", curl_easy_strerror(curlRc));
        return 3;
    }

    size_t npix = (size_t)width * (size_t)height;
    size_t raw_size = npix * (size_t)channels;
    uint8_t *buf = (uint8_t *)calloc(raw_size ? raw_size : 1, 1);
    if (!buf) {
        fprintf(stderr, "assemble_greyscale: cannot allocate %zu bytes\n", raw_size);
        curl_global_cleanup();
        return 4;
    }

    AssembleState st;
    memset(&st, 0, sizeof(st));
    st.width = width;
    st.height = height;
    st.channels = channels;
    st.allow_zero = allow_zero ? 1 : 0;
    st.record_size = 4u + (size_t)channels;
    st.npix = npix;
    st.buf = buf;
    st.paths = paths;
    st.n_paths = n_paths;
    pthread_mutex_init(&st.queue_mu, NULL);
    pthread_mutex_init(&st.err_mu, NULL);

    pthread_t *threads = (pthread_t *)calloc((size_t)workers, sizeof(pthread_t));
    WorkerCtx *ctxs = (WorkerCtx *)calloc((size_t)workers, sizeof(WorkerCtx));
    if (!threads || !ctxs) {
        fprintf(stderr, "assemble_greyscale: cannot allocate worker state\n");
        free(ctxs);
        free(threads);
        free(buf);
        for (int i = 0; i < n_paths; i++) free(paths[i]);
        free(paths);
        curl_global_cleanup();
        return 5;
    }

    for (int i = 0; i < workers; i++) {
        ctxs[i].st = &st;
        ctxs[i].curl = NULL;
        if (pthread_create(&threads[i], NULL, worker_main, &ctxs[i]) != 0) {
            fprintf(stderr, "assemble_greyscale: pthread_create failed\n");
            free(ctxs);
            free(threads);
            free(buf);
            for (int j = 0; j < n_paths; j++) free(paths[j]);
            free(paths);
            curl_global_cleanup();
            return 6;
        }
    }
    for (int i = 0; i < workers; i++) pthread_join(threads[i], NULL);

    pthread_mutex_destroy(&st.queue_mu);
    pthread_mutex_destroy(&st.err_mu);
    free(ctxs);
    free(threads);

    if (st.failed) {
        fprintf(stderr, "%s\n", st.error);
        free(buf);
        for (int i = 0; i < n_paths; i++) free(paths[i]);
        free(paths);
        curl_global_cleanup();
        return 7;
    }

    FILE *out = fopen(outPath, "wb");
    if (!out) {
        fprintf(stderr, "assemble_greyscale: cannot create %s\n", outPath);
        free(buf);
        for (int i = 0; i < n_paths; i++) free(paths[i]);
        free(paths);
        curl_global_cleanup();
        return 8;
    }
    if (raw_size > 0 && fwrite(buf, 1, raw_size, out) != raw_size) {
        fprintf(stderr, "assemble_greyscale: short write to %s\n", outPath);
        fclose(out);
        free(buf);
        for (int i = 0; i < n_paths; i++) free(paths[i]);
        free(paths);
        curl_global_cleanup();
        return 9;
    }
    fclose(out);

    if (histPath && *histPath) {
        if (!write_histogram_json(histPath, width, height, channels, buf, npix)) {
            fprintf(stderr, "assemble_greyscale: cannot write histogram %s\n", histPath);
            free(buf);
            for (int i = 0; i < n_paths; i++) free(paths[i]);
            free(paths);
            curl_global_cleanup();
            return 10;
        }
    }

    free(buf);
    for (int i = 0; i < n_paths; i++) free(paths[i]);
    free(paths);
    curl_global_cleanup();
    return 0;
}
