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
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "camera_fragment_format.h"

/* Deterministic collision policy (CR28 F1): when two fragments carry the same
 * global pixel, the one from the LOWEST source ordinal wins, independent of
 * thread timing. A per-pixel owner (uint16 ordinal, 0xFFFF = unclaimed) plus a
 * bank of striped mutexes makes concurrent overlapping writes race-free while
 * keeping non-overlapping writes contended only by hash bucket. */
#define AG_WRITE_STRIPES 64
#define AG_OWNER_NONE 0xFFFFu
#define AG_MAX_FRAGMENTS 65534   /* ordinal must fit uint16 below AG_OWNER_NONE */

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
    int camera_mode;
    size_t record_size;
    size_t npix;
    uint8_t *buf;
    float *depth;
    uint16_t *owner;                 /* per-pixel winning fragment ordinal */
    char **paths;
    int n_paths;
    int next_idx;
    atomic_int failed;               /* read cross-mutex in next_job */
    char error[512];
    pthread_mutex_t queue_mu;
    pthread_mutex_t err_mu;
    pthread_mutex_t write_mu[AG_WRITE_STRIPES];
    uint64_t camera_bytes_received;
    uint64_t camera_records_seen;
    uint64_t camera_depth_replacements;
    uint64_t camera_fragments_processed;
    size_t camera_peak_carry_bytes;
    long long camera_merge_us;
} AssembleState;

typedef struct {
    AssembleState *st;
    CURL *curl;
} WorkerCtx;

typedef struct {
    AssembleState *st;
    const char *path;
    uint8_t carry[8 + 8];
    size_t carry_size;
} CameraStreamCtx;

static const char *getArg(int argc, char **argv, const char *key) {
    int klen = (int)strlen(key);
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], key, klen) == 0 && argv[i][klen] == '=') {
            return argv[i] + klen + 1;
        }
    }
    return NULL;
}

static long long monotonic_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000LL + (long long)ts.tv_nsec / 1000LL;
}

static int getArgInt(int argc, char **argv, const char *key, int def) {
    const char *v = getArg(argc, argv, key);
    return v ? atoi(v) : def;
}

static void set_error(AssembleState *st, const char *fmt, const char *path, long long a, long long b) {
    pthread_mutex_lock(&st->err_mu);
    if (!atomic_load(&st->failed)) {
        atomic_store(&st->failed, 1);
        snprintf(st->error, sizeof(st->error), fmt, path, a, b);
    }
    pthread_mutex_unlock(&st->err_mu);
}

/* Set the stop flag without recording an error (used to abort remaining work
 * during a partial thread-start failure, F2). */
static void request_stop(AssembleState *st) {
    atomic_store(&st->failed, 1);
}

static int next_job(AssembleState *st) {
    int idx = -1;
    pthread_mutex_lock(&st->queue_mu);
    if (!atomic_load(&st->failed) && st->next_idx < st->n_paths) {
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

/* S3 returns 503 SlowDown when a prefix is hit too hard; the AWS SDKs retry
 * it automatically, and this fragment downloader must too, or a single
 * throttled section fails the whole render (2026-07 incident). Mirrors the
 * range-GET retry policy in multispan_reader.c / solve_proximity_hist_sectioned.c. */
#define AG_DOWNLOAD_ATTEMPTS 6

static void ag_sleep_ms(long ms) {
    if (ms <= 0) return;
    struct timespec ts;
    ts.tv_sec = ms / 1000L;
    ts.tv_nsec = (ms % 1000L) * 1000000L;
    nanosleep(&ts, NULL);
}

static int ag_retryable_failure(CURLcode rc, long httpStatus) {
    /* When an HTTP response arrived (status != 0), retry ONLY transient
     * statuses — a permanent 4xx (403/404) must not be retried even though
     * CURLOPT_FAILONERROR reports it as CURLE_HTTP_RETURNED_ERROR (CR28 F16). */
    if (httpStatus != 0L) {
        return httpStatus == 429L || httpStatus == 500L ||
               httpStatus == 502L || httpStatus == 503L || httpStatus == 504L;
    }
    /* No HTTP status: a transport-level failure — retry the transient ones. */
    return rc == CURLE_OPERATION_TIMEDOUT ||
           rc == CURLE_COULDNT_CONNECT ||
           rc == CURLE_COULDNT_RESOLVE_HOST ||
           rc == CURLE_RECV_ERROR ||
           rc == CURLE_SEND_ERROR ||
           rc == CURLE_GOT_NOTHING ||
           rc == CURLE_PARTIAL_FILE;
}

static int download_url_bytes(WorkerCtx *ctx, const char *url, uint8_t **outData, size_t *outSize) {
    char curlErr[CURL_ERROR_SIZE] = {0};
    long httpStatus = 0;
    int attempts_made = 0;
    CURLcode rc = CURLE_OK;

    if (!ctx->curl) {
        ctx->curl = curl_easy_init();
        if (!ctx->curl) {
            return 0;
        }
    }

    for (int attempt = 0; attempt < AG_DOWNLOAD_ATTEMPTS; attempt++) {
        DownloadBuffer dl;
        memset(&dl, 0, sizeof(dl));
        curlErr[0] = 0;
        httpStatus = 0;

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
        /* Deadlines so retry count actually bounds wall time (CR28 F16):
         * 10s to connect, 120s total per attempt, and abort a stalled
         * transfer (<1 byte/s for 30s). */
        curl_easy_setopt(ctx->curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(ctx->curl, CURLOPT_TIMEOUT, 120L);
        curl_easy_setopt(ctx->curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
        curl_easy_setopt(ctx->curl, CURLOPT_LOW_SPEED_TIME, 30L);

        rc = curl_easy_perform(ctx->curl);
        curl_easy_getinfo(ctx->curl, CURLINFO_RESPONSE_CODE, &httpStatus);
        if (rc == CURLE_OK) {
            *outData = dl.data;
            *outSize = dl.size;
            return 1;
        }

        free(dl.data);
        attempts_made = attempt + 1;
        if (attempts_made >= AG_DOWNLOAD_ATTEMPTS || !ag_retryable_failure(rc, httpStatus)) {
            break;
        }
        /* backoff with a small linear ramp (150/300/450/... ms); S3 SlowDown
         * clears quickly once the request rate drops */
        ag_sleep_ms(150L * (attempt + 1));
    }

    /* Diagnostics lead: presigned URLs (~1.5 KB) overflow the 512-byte error
     * buffer, so anything formatted after the URL is truncated away (this hid
     * the curl rc during the 2026-06 incident). Report ACTUAL attempts, not
     * the constant max (CR28 F16). */
    char detail[512];
    snprintf(detail, sizeof(detail),
             "assemble_greyscale: download failed after %d attempt(s) (curl %d %s; http %ld; %s): %s",
             attempts_made, (int)rc, curl_easy_strerror(rc), httpStatus,
             curlErr[0] ? curlErr : "no detail", url);
    set_error(ctx->st, "%s", detail, 0, 0);
    return 0;
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

static int process_fragment_bytes(AssembleState *st, int frag_ord, const char *path, uint8_t *data, size_t size) {
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
        /* Deterministic, race-free write (CR28 F1): lowest source ordinal
         * wins each pixel, under the pixel's stripe lock. */
        int stripe = (int)(pixel_idx & (AG_WRITE_STRIPES - 1));
        pthread_mutex_lock(&st->write_mu[stripe]);
        if ((uint32_t)frag_ord < (uint32_t)st->owner[pixel_idx]) {
            st->owner[pixel_idx] = (uint16_t)frag_ord;
            memcpy(st->buf + ((size_t)pixel_idx * (size_t)st->channels),
                   data + off + 4, (size_t)st->channels);
        }
        pthread_mutex_unlock(&st->write_mu[stripe]);
    }
    return 1;
}

static int process_camera_record(
        AssembleState *st,
        const char *path,
        const uint8_t *record) {
    uint32_t pixel_idx = camera_fragment_read_u32le(record);
    float depth = camera_fragment_read_f32le(record + 4);
    st->camera_records_seen++;
    if ((size_t)pixel_idx >= st->npix) {
        set_error(
            st,
            "assemble_greyscale: camera fragment %s pixel_idx %lld out of bounds for npix=%lld",
            path,
            (long long)pixel_idx,
            (long long)st->npix
        );
        return 0;
    }
    if (!isfinite(depth) || !(depth > 0.0f)) {
        set_error(
            st,
            "assemble_greyscale: camera fragment %s has invalid depth at pixel %lld",
            path,
            (long long)pixel_idx,
            0
        );
        return 0;
    }
    if (depth < st->depth[pixel_idx]) {
        st->depth[pixel_idx] = depth;
        st->camera_depth_replacements++;
        memcpy(
            st->buf + (size_t)pixel_idx * (size_t)st->channels,
            record + 8,
            (size_t)st->channels
        );
    }
    return 1;
}

static size_t write_camera_stream_cb(char *ptr, size_t size, size_t nmemb, void *userdata) {
    CameraStreamCtx *stream = (CameraStreamCtx *)userdata;
    size_t total = size * nmemb;
    stream->st->camera_bytes_received += (uint64_t)total;
    size_t consumed = 0;
    while (consumed < total) {
        size_t need = stream->st->record_size - stream->carry_size;
        size_t take = total - consumed < need ? total - consumed : need;
        memcpy(stream->carry + stream->carry_size, ptr + consumed, take);
        stream->carry_size += take;
        if (stream->carry_size > stream->st->camera_peak_carry_bytes) {
            stream->st->camera_peak_carry_bytes = stream->carry_size;
        }
        consumed += take;
        if (stream->carry_size == stream->st->record_size) {
            if (!process_camera_record(stream->st, stream->path, stream->carry)) {
                return 0;
            }
            stream->carry_size = 0;
        }
    }
    return total;
}

static int process_camera_url(AssembleState *st, const char *url) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        set_error(st, "assemble_greyscale: cannot initialize camera download for %s", url, 0, 0);
        return 0;
    }
    char curlErr[CURL_ERROR_SIZE] = {0};
    CURLcode rc = CURLE_OK;
    long httpStatus = 0;
    int attempts_made = 0;
    for (int attempt = 0; attempt < AG_DOWNLOAD_ATTEMPTS; attempt++) {
        CameraStreamCtx stream = {
            .st = st,
            .path = url,
            .carry_size = 0,
        };
        curlErr[0] = 0;
        httpStatus = 0;
        curl_easy_reset(curl);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_camera_stream_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stream);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErr);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "identity");
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        /* The enclosing assembler process owns the 600-second deadline.
         * A separate 120-second object cap rejected large, healthy streams
         * that admission had correctly priced inside that budget. */
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30L);
        rc = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatus);
        attempts_made = attempt + 1;
        if (rc == CURLE_OK && stream.carry_size == 0 && !atomic_load(&st->failed)) {
            curl_easy_cleanup(curl);
            return 1;
        }
        if (rc == CURLE_OK && stream.carry_size != 0 && !atomic_load(&st->failed)) {
            set_error(
                st,
                "assemble_greyscale: camera fragment %s ends with %lld partial bytes",
                url,
                (long long)stream.carry_size,
                0
            );
        }
        if (atomic_load(&st->failed)
                || attempts_made >= AG_DOWNLOAD_ATTEMPTS
                || !ag_retryable_failure(rc, httpStatus)) {
            break;
        }
        ag_sleep_ms(150L * (attempt + 1));
    }
    curl_easy_cleanup(curl);
    if (!atomic_load(&st->failed)) {
        char detail[512];
        snprintf(
            detail,
            sizeof(detail),
            "assemble_greyscale: camera download failed after %d attempt(s) "
            "(curl %d %s; http %ld; %s): %s",
            attempts_made,
            (int)rc,
            curl_easy_strerror(rc),
            httpStatus,
            curlErr[0] ? curlErr : "no detail",
            url
        );
        set_error(st, "%s", detail, 0, 0);
    }
    return 0;
}

static int process_camera_local(AssembleState *st, const char *path) {
    FILE *fh = fopen(path, "rb");
    if (!fh) {
        set_error(st, "assemble_greyscale: cannot open %s (%lld)", path, (long long)errno, 0);
        return 0;
    }
    uint8_t buffer[64 * 1024];
    CameraStreamCtx stream = {
        .st = st,
        .path = path,
        .carry_size = 0,
    };
    size_t got = 0;
    while ((got = fread(buffer, 1, sizeof(buffer), fh)) > 0) {
        if (write_camera_stream_cb((char *)buffer, 1, got, &stream) != got) {
            fclose(fh);
            return 0;
        }
    }
    if (ferror(fh)) {
        fclose(fh);
        set_error(st, "assemble_greyscale: cannot read %s (%lld)", path, (long long)errno, 0);
        return 0;
    }
    fclose(fh);
    if (stream.carry_size != 0) {
        set_error(
            st,
            "assemble_greyscale: camera fragment %s ends with %lld partial bytes",
            path,
            (long long)stream.carry_size,
            0
        );
        return 0;
    }
    return 1;
}

static int process_camera_source(AssembleState *st, const char *path) {
    return is_url(path)
        ? process_camera_url(st, path)
        : process_camera_local(st, path);
}

static int process_fragment_source(WorkerCtx *ctx, int frag_ord, const char *path) {
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

    ok = process_fragment_bytes(ctx->st, frag_ord, path, data, size);
    free(data);
    return ok;
}

static void *worker_main(void *arg) {
    WorkerCtx *ctx = (WorkerCtx *)arg;
    AssembleState *st = ctx->st;
    for (;;) {
        int idx = next_job(st);
        if (idx < 0) break;
        if (!process_fragment_source(ctx, idx, st->paths[idx])) {
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
    const char *fragmentEncoding = getArg(argc, argv, "--fragment_encoding");
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
    int camera_mode = fragmentEncoding
        && strcmp(fragmentEncoding, CAMERA_FRAGMENT_ENCODING) == 0;
    if (fragmentEncoding && *fragmentEncoding
            && !camera_mode
            && strcmp(fragmentEncoding, "u32le_u8_v1") != 0
            && strcmp(fragmentEncoding, "u32le_pixel_idx_plus_u8_channels_v1") != 0) {
        fprintf(
            stderr,
            "assemble_greyscale: unsupported --fragment_encoding=%s\n",
            fragmentEncoding
        );
        return 2;
    }
    if (camera_mode && !allow_zero) {
        fprintf(stderr, "assemble_greyscale: camera fragments require --allow-zero=1\n");
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
    if (n_paths > AG_MAX_FRAGMENTS) {
        fprintf(stderr, "assemble_greyscale: %d fragments exceeds max %d\n", n_paths, AG_MAX_FRAGMENTS);
        for (int i = 0; i < n_paths; i++) free(paths[i]);
        free(paths);
        return 2;
    }
    /* never spawn more workers than fragments (F2) */
    if (workers > n_paths && n_paths > 0) workers = n_paths;

    curlRc = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (curlRc != CURLE_OK) {
        fprintf(stderr, "assemble_greyscale: curl_global_init failed: %s\n", curl_easy_strerror(curlRc));
        return 3;
    }

    size_t npix = (size_t)width * (size_t)height;
    size_t raw_size = npix * (size_t)channels;
    uint8_t *buf = (uint8_t *)calloc(raw_size ? raw_size : 1, 1);
    /* Legacy fragments need an ordinal plane. Camera fragments need a
     * nearest-depth plane and are processed in ordinal order, so equal-depth
     * ties retain the earlier source without a second ownership plane. */
    uint16_t *owner = camera_mode
        ? NULL
        : (uint16_t *)malloc((npix ? npix : 1) * sizeof(uint16_t));
    float *depth = camera_mode
        ? (float *)malloc((npix ? npix : 1) * sizeof(float))
        : NULL;
    if (!buf || (camera_mode ? !depth : !owner)) {
        fprintf(stderr, "assemble_greyscale: cannot allocate %zu bytes\n", raw_size);
        free(buf); free(owner); free(depth);
        for (int i = 0; i < n_paths; i++) free(paths[i]);
        free(paths);
        curl_global_cleanup();
        return 4;
    }
    if (owner) {
        memset(owner, 0xFF, (npix ? npix : 1) * sizeof(uint16_t));
    }
    if (depth) {
        for (size_t i = 0; i < npix; i++) depth[i] = INFINITY;
    }

    AssembleState st;
    memset(&st, 0, sizeof(st));
    st.owner = owner;
    st.depth = depth;
    st.width = width;
    st.height = height;
    st.channels = channels;
    st.allow_zero = allow_zero ? 1 : 0;
    st.camera_mode = camera_mode;
    st.record_size = (camera_mode ? 8u : 4u) + (size_t)channels;
    st.npix = npix;
    st.buf = buf;
    st.paths = paths;
    st.n_paths = n_paths;
    pthread_mutex_init(&st.queue_mu, NULL);
    pthread_mutex_init(&st.err_mu, NULL);
    for (int i = 0; i < AG_WRITE_STRIPES; i++) pthread_mutex_init(&st.write_mu[i], NULL);

    int start_failed = 0;
    pthread_t *threads = NULL;
    WorkerCtx *ctxs = NULL;
    if (camera_mode) {
        /* Camera fragments can be enormous. Stream one source at a time so
         * memory is O(output pixels), and preserve source order so equal
         * float32 depths deterministically keep the lower ordinal. */
        long long camera_merge_start_us = monotonic_us();
        for (int i = 0; i < n_paths && !atomic_load(&st.failed); i++) {
            if (!process_camera_source(&st, paths[i])) break;
            st.camera_fragments_processed++;
        }
        st.camera_merge_us = monotonic_us() - camera_merge_start_us;
    } else {
        threads = (pthread_t *)calloc((size_t)workers, sizeof(pthread_t));
        ctxs = (WorkerCtx *)calloc((size_t)workers, sizeof(WorkerCtx));
        if (!threads || !ctxs) {
            fprintf(stderr, "assemble_greyscale: cannot allocate worker state\n");
            start_failed = 1;
        }

        /* Safe partial thread startup (CR28 F2): on a mid-loop
         * pthread_create failure, signal stop and JOIN the threads already
         * created before freeing shared state. */
        int created = 0;
        if (!start_failed) {
            for (int i = 0; i < workers; i++) {
                ctxs[i].st = &st;
                ctxs[i].curl = NULL;
                if (pthread_create(&threads[i], NULL, worker_main, &ctxs[i]) != 0) {
                    fprintf(stderr, "assemble_greyscale: pthread_create failed\n");
                    request_stop(&st);
                    start_failed = 1;
                    break;
                }
                created++;
            }
        }
        for (int i = 0; i < created; i++) pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&st.queue_mu);
    pthread_mutex_destroy(&st.err_mu);
    for (int i = 0; i < AG_WRITE_STRIPES; i++) pthread_mutex_destroy(&st.write_mu[i]);
    free(ctxs);
    free(threads);
    if (start_failed) {
        free(buf); free(owner); free(depth);
        for (int i = 0; i < n_paths; i++) free(paths[i]);
        free(paths);
        curl_global_cleanup();
        return 6;
    }

    if (st.failed) {
        fprintf(stderr, "%s\n", st.error);
        free(buf); free(owner); free(depth);
        for (int i = 0; i < n_paths; i++) free(paths[i]);
        free(paths);
        curl_global_cleanup();
        return 7;
    }

    FILE *out = fopen(outPath, "wb");
    if (!out) {
        fprintf(stderr, "assemble_greyscale: cannot create %s\n", outPath);
        free(buf); free(owner); free(depth);
        for (int i = 0; i < n_paths; i++) free(paths[i]);
        free(paths);
        curl_global_cleanup();
        return 8;
    }
    if (raw_size > 0 && fwrite(buf, 1, raw_size, out) != raw_size) {
        fprintf(stderr, "assemble_greyscale: short write to %s\n", outPath);
        fclose(out);
        free(buf); free(owner); free(depth);
        for (int i = 0; i < n_paths; i++) free(paths[i]);
        free(paths);
        curl_global_cleanup();
        return 9;
    }
    fclose(out);

    if (histPath && *histPath) {
        if (!write_histogram_json(histPath, width, height, channels, buf, npix)) {
            fprintf(stderr, "assemble_greyscale: cannot write histogram %s\n", histPath);
            free(buf); free(owner); free(depth);
            for (int i = 0; i < n_paths; i++) free(paths[i]);
            free(paths);
            curl_global_cleanup();
            return 10;
        }
    }

    if (camera_mode) {
        printf(
            "{\"camera_fragment_transfer_bytes\":%llu"
            ",\"camera_fragment_records_seen\":%llu"
            ",\"camera_final_depth_replacements\":%llu"
            ",\"camera_fragments_processed\":%llu"
            ",\"camera_stream_carry_peak_bytes\":%zu"
            ",\"camera_depth_buffer_bytes\":%zu"
            ",\"camera_merge_us\":%lld}\n",
            (unsigned long long)st.camera_bytes_received,
            (unsigned long long)st.camera_records_seen,
            (unsigned long long)st.camera_depth_replacements,
            (unsigned long long)st.camera_fragments_processed,
            st.camera_peak_carry_bytes,
            npix * sizeof(float),
            st.camera_merge_us
        );
    }

    free(buf); free(owner); free(depth);
    for (int i = 0; i < n_paths; i++) free(paths[i]);
    free(paths);
    curl_global_cleanup();
    return 0;
}
