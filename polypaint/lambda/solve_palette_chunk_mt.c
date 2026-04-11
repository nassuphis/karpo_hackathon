/*
 * solve_palette_chunk_mt — exact palette chunk scorer with native threading and
 * optional sectioned range GET input.
 *
 * Usage:
 *   solve_palette_chunk_mt input.bin \
 *     --degree=D --metric=proximity \
 *     --clip_lo=X --clip_hi=Y --cuts=c1,...,c9 --omega=1 \
 *     --step_count=S \
 *     --scores_out=file.bin --bins_out=file.bin \
 *     [--threads=4] [--input_mode=tmpfile|sectioned] [--retries=2] \
 *     [--url=PRESIGNED_URL --input_size=BYTES] [--root_xforms=file.json]
 *
 * Input .bin format: raw float32 roots, one solve after another, degree complex
 * roots each.
 *
 * Output:
 *   - scores_out: float32[count]
 *   - bins_out: uint8[count] in range 0..9
 *   - stdout: compact JSON metadata with perf counters
 *
 * Build (Amazon Linux / Lambda-compatible):
 *   gcc -O3 -pthread -o solve_palette_chunk_mt solve_palette_chunk_mt.c \
 *     -lcurl -lm -Wl,-rpath,'$ORIGIN/lib'
 */

#include <curl/curl.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "root_xforms.h"
#include "solve_score.h"

#define MAXDEG 1024

static const char *getArg(int argc, char **argv, const char *key) {
    size_t klen = strlen(key);
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

static long long getArgLongLong(int argc, char **argv, const char *key, long long def) {
    const char *v = getArg(argc, argv, key);
    return v ? atoll(v) : def;
}

static double getArgDouble(int argc, char **argv, const char *key, double def) {
    const char *v = getArg(argc, argv, key);
    return v ? atof(v) : def;
}

static const char *getArgStr(int argc, char **argv, const char *key, const char *def) {
    const char *v = getArg(argc, argv, key);
    return v ? v : def;
}

static long monotonic_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long)(ts.tv_sec * 1000L + ts.tv_nsec / 1000000L);
}

static int clamp_threads(int requested, long n_items) {
    int threads = requested < 1 ? 1 : requested;
    if (n_items > 0 && threads > (int)n_items) threads = (int)n_items;
    if (threads < 1) threads = 1;
    return threads;
}

static int parseCuts(const char *s, double cuts[9]) {
    if (!s || !*s) return 0;
    char *copy = strdup(s);
    if (!copy) return 0;
    int n = 0;
    char *save = NULL;
    for (char *tok = strtok_r(copy, ",", &save); tok && n < 9; tok = strtok_r(NULL, ",", &save)) {
        cuts[n++] = atof(tok);
    }
    free(copy);
    return n;
}

static void sleep_ms(long ms) {
    if (ms <= 0) return;
    struct timespec ts;
    ts.tv_sec = ms / 1000L;
    ts.tv_nsec = (ms % 1000L) * 1000000L;
    nanosleep(&ts, NULL);
}

static int retryable_range_failure(CURLcode rc, long httpStatus) {
    if (httpStatus == 429L || httpStatus == 500L || httpStatus == 502L ||
        httpStatus == 503L || httpStatus == 504L) return 1;
    return rc == CURLE_HTTP_RETURNED_ERROR ||
           rc == CURLE_OPERATION_TIMEDOUT ||
           rc == CURLE_COULDNT_CONNECT ||
           rc == CURLE_COULDNT_RESOLVE_HOST ||
           rc == CURLE_RECV_ERROR ||
           rc == CURLE_SEND_ERROR ||
           rc == CURLE_GOT_NOTHING;
}

typedef struct {
    unsigned char *data;
    size_t expected;
    size_t size;
    int overflow;
} DownloadBuffer;

typedef struct {
    const float *sharedBuf;
    const char *url;
    const char *inputMode;
    int degree;
    long solveStart;
    long solveCount;
    long solveBytes;
    size_t sectionBytes;
    unsigned long long byteStart;
    unsigned long long byteEnd;
    enum SolveMetric metric;
    RootXformEntry *rtChain;
    int nRt;
    double clipLo;
    double clipHi;
    double omega;
    int omegaEnabled;
    double cuts[9];
    int retries;
    float *scoresOut;
    unsigned char *binsOut;
    long downloadMs;
    long computeMs;
    long bytesDownloaded;
    long httpStatus;
    int failed;
    char error[256];
} ChunkWorkerArgs;

static size_t write_section_cb(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total = size * nmemb;
    DownloadBuffer *buf = (DownloadBuffer *)userdata;
    if (buf->size + total > buf->expected) {
        buf->overflow = 1;
        return 0;
    }
    memcpy(buf->data + buf->size, ptr, total);
    buf->size += total;
    return total;
}

static double score_xformed(const float *roots, int degree, enum SolveMetric metric,
                            RootXformEntry *rtChain, int nRt,
                            float *wkRe, float *wkIm) {
    float xformed[MAXDEG * 2];
    for (int k = 0; k < degree; k++) {
        wkRe[k] = roots[k * 2];
        wkIm[k] = roots[k * 2 + 1];
    }
    apply_root_xforms(rtChain, nRt, wkRe, wkIm, degree);
    for (int k = 0; k < degree; k++) {
        xformed[k * 2] = wkRe[k];
        xformed[k * 2 + 1] = wkIm[k];
    }
    return compute_solve_metric_score(xformed, degree, metric);
}

static unsigned char score_to_bin(double score, double clipLo, double clipHi,
                                  double omega, int omegaEnabled, const double cuts[9]) {
    double range = clipHi - clipLo;
    double u = (score - clipLo) / range;
    if (u < 0) u = 0;
    if (u > 1) u = 1;
    u = apply_solve_score_transfer(u, omegaEnabled, omega);
    unsigned char bin = 9;
    for (int c = 0; c < 9; c++) {
        if (u <= cuts[c]) {
            bin = (unsigned char)c;
            break;
        }
    }
    return bin;
}

static void compute_scores_for_roots(const float *roots, long solveCount, long solveStart,
                                     ChunkWorkerArgs *arg) {
    float wkRe[MAXDEG], wkIm[MAXDEG];
    int stride = arg->degree * 2;
    long computeStart = monotonic_ms();
    for (long s = 0; s < solveCount; s++) {
        const float *solveRoots = roots + s * stride;
        double score = (arg->nRt > 0)
            ? score_xformed(solveRoots, arg->degree, arg->metric, arg->rtChain, arg->nRt, wkRe, wkIm)
            : compute_solve_metric_score(solveRoots, arg->degree, arg->metric);
        long outIdx = solveStart + s;
        arg->scoresOut[outIdx] = (float)score;
        arg->binsOut[outIdx] = score_to_bin(score, arg->clipLo, arg->clipHi, arg->omega, arg->omegaEnabled, arg->cuts);
    }
    arg->computeMs = monotonic_ms() - computeStart;
}

static void *chunk_worker_main(void *arg_) {
    ChunkWorkerArgs *arg = (ChunkWorkerArgs *)arg_;
    if (arg->solveCount <= 0) return NULL;

    if (strcmp(arg->inputMode, "sectioned") == 0) {
        DownloadBuffer dl = {0};
        CURL *curl = NULL;
        char rangeBuf[96];
        char curlErr[CURL_ERROR_SIZE] = {0};

        dl.expected = arg->sectionBytes;
        dl.data = malloc(dl.expected > 0 ? dl.expected : 1);
        if (!dl.data) {
            snprintf(arg->error, sizeof(arg->error),
                     "Out of memory for section buffer (%zu bytes)", dl.expected);
            arg->failed = 1;
            return NULL;
        }

        curl = curl_easy_init();
        if (!curl) {
            snprintf(arg->error, sizeof(arg->error), "curl_easy_init failed");
            arg->failed = 1;
            free(dl.data);
            return NULL;
        }

        snprintf(rangeBuf, sizeof(rangeBuf), "%llu-%llu",
                 (unsigned long long)arg->byteStart, (unsigned long long)arg->byteEnd);
        curl_easy_setopt(curl, CURLOPT_URL, arg->url);
        curl_easy_setopt(curl, CURLOPT_RANGE, rangeBuf);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_section_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dl);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErr);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "identity");
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

        int attempts = arg->retries + 1;
        CURLcode rc = CURLE_OK;
        long dlStart = monotonic_ms();
        for (int attempt = 0; attempt < attempts; attempt++) {
            dl.size = 0;
            dl.overflow = 0;
            curlErr[0] = '\0';
            arg->httpStatus = 0;
            rc = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &arg->httpStatus);
            if (rc == CURLE_OK &&
                (arg->httpStatus == 206L || arg->httpStatus == 200L) &&
                !dl.overflow &&
                dl.size == dl.expected) {
                break;
            }
            if (attempt + 1 >= attempts || !retryable_range_failure(rc, arg->httpStatus)) {
                break;
            }
            sleep_ms(150L * (attempt + 1));
        }
        arg->downloadMs = monotonic_ms() - dlStart;
        if (rc != CURLE_OK) {
            snprintf(arg->error, sizeof(arg->error),
                     "range GET failed for bytes %s after %d attempt%s: %s",
                     rangeBuf, attempts, attempts == 1 ? "" : "s",
                     curlErr[0] ? curlErr : curl_easy_strerror(rc));
            arg->failed = 1;
            curl_easy_cleanup(curl);
            free(dl.data);
            return NULL;
        }
        if (arg->httpStatus != 206L && arg->httpStatus != 200L) {
            snprintf(arg->error, sizeof(arg->error),
                     "unexpected HTTP status %ld for bytes %s after %d attempt%s",
                     arg->httpStatus, rangeBuf, attempts, attempts == 1 ? "" : "s");
            arg->failed = 1;
            curl_easy_cleanup(curl);
            free(dl.data);
            return NULL;
        }
        if (dl.overflow) {
            snprintf(arg->error, sizeof(arg->error),
                     "range GET overflow for bytes %s after %d attempt%s",
                     rangeBuf, attempts, attempts == 1 ? "" : "s");
            arg->failed = 1;
            curl_easy_cleanup(curl);
            free(dl.data);
            return NULL;
        }
        if (dl.size != dl.expected) {
            snprintf(arg->error, sizeof(arg->error),
                     "short range GET for bytes %s after %d attempt%s: got %zu of %zu bytes",
                     rangeBuf, attempts, attempts == 1 ? "" : "s", dl.size, dl.expected);
            arg->failed = 1;
            curl_easy_cleanup(curl);
            free(dl.data);
            return NULL;
        }
        arg->bytesDownloaded = (long)dl.size;
        compute_scores_for_roots((const float *)(void *)dl.data, arg->solveCount, arg->solveStart, arg);
        curl_easy_cleanup(curl);
        free(dl.data);
        return NULL;
    }

    const float *roots = arg->sharedBuf + (long)arg->solveStart * (arg->degree * 2);
    compute_scores_for_roots(roots, arg->solveCount, arg->solveStart, arg);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: solve_palette_chunk_mt input.bin --degree=D --metric=... "
                "--clip_lo=X --clip_hi=Y --cuts=c1,...,c9 --step_count=S "
                "--scores_out=file.bin --bins_out=file.bin [--threads=4] "
                "[--input_mode=tmpfile|sectioned] [--url=URL --input_size=BYTES] "
                "[--retries=2] [--root_xforms=file.json]\n");
        return 1;
    }

    const char *inPath = argv[1];
    int degree = getArgInt(argc, argv, "--degree", 0);
    const char *metricStr = getArgStr(argc, argv, "--metric", "proximity");
    double clipLo = getArgDouble(argc, argv, "--clip_lo", 0.0);
    double clipHi = getArgDouble(argc, argv, "--clip_hi", 0.0);
    double omega = getArgDouble(argc, argv, "--omega", 1.0);
    int omegaEnabled = getArgInt(argc, argv, "--omega_enabled", 1);
    int stepCount = getArgInt(argc, argv, "--step_count", -1);
    int threads = getArgInt(argc, argv, "--threads", 1);
    int retries = getArgInt(argc, argv, "--retries", 2);
    const char *cutsStr = getArgStr(argc, argv, "--cuts", NULL);
    const char *scoresOutPath = getArgStr(argc, argv, "--scores_out", NULL);
    const char *binsOutPath = getArgStr(argc, argv, "--bins_out", NULL);
    const char *rtPath = getArgStr(argc, argv, "--root_xforms", NULL);
    const char *inputMode = getArgStr(argc, argv, "--input_mode", "tmpfile");
    const char *url = getArgStr(argc, argv, "--url", NULL);
    long long inputSize = getArgLongLong(argc, argv, "--input_size", -1);

    if (degree < 2 || degree > MAXDEG) {
        fprintf(stderr, "Invalid degree: %d\n", degree);
        return 1;
    }
    if (!scoresOutPath || !binsOutPath) {
        fprintf(stderr, "Missing --scores_out or --bins_out\n");
        return 1;
    }
    if (clipHi - clipLo < 1e-12) {
        fprintf(stderr, "Invalid clip range: lo=%.6g hi=%.6g\n", clipLo, clipHi);
        return 1;
    }
    if (strcmp(inputMode, "tmpfile") != 0 && strcmp(inputMode, "sectioned") != 0) {
        fprintf(stderr, "Invalid input_mode: %s\n", inputMode);
        return 1;
    }
    if (retries < 0 || retries > 10) {
        fprintf(stderr, "Invalid retries: %d\n", retries);
        return 1;
    }

    enum SolveMetric metric;
    if (!parse_solve_metric(metricStr, &metric)) {
        fprintf(stderr, "Invalid metric: %s\n", metricStr);
        return 1;
    }

    double cuts[9];
    int nCuts = parseCuts(cutsStr, cuts);
    if (nCuts != 9) {
        fprintf(stderr, "Expected exactly 9 cuts, got %d\n", nCuts);
        return 1;
    }

    int solveStride = degree * 2;
    long solveBytes = (long)solveStride * (long)sizeof(float);
    long totalSolves = 0;
    float *buf = NULL;

    if (strcmp(inputMode, "sectioned") == 0) {
        if (!url || !*url) {
            fprintf(stderr, "sectioned input requires --url\n");
            return 1;
        }
        if (inputSize <= 0) {
            fprintf(stderr, "sectioned input requires positive --input_size\n");
            return 1;
        }
        totalSolves = (long)(inputSize / solveBytes);
    } else {
        FILE *f = fopen(inPath, "rb");
        if (!f) {
            fprintf(stderr, "Cannot open %s\n", inPath);
            return 1;
        }
        fseek(f, 0, SEEK_END);
        long fileSize = ftell(f);
        fseek(f, 0, SEEK_SET);
        totalSolves = fileSize / solveBytes;
        if (stepCount < 0) stepCount = (int)totalSolves;
        if ((long)stepCount > totalSolves) {
            fprintf(stderr, "step_count=%d exceeds total solves=%ld\n", stepCount, totalSolves);
            fclose(f);
            return 1;
        }
        long bytesWanted = (long)stepCount * solveBytes;
        buf = malloc(bytesWanted > 0 ? bytesWanted : 1);
        if (!buf) {
            fprintf(stderr, "Out of memory\n");
            fclose(f);
            return 1;
        }
        if ((long)fread(buf, 1, bytesWanted, f) != bytesWanted) {
            fprintf(stderr, "Short read\n");
            free(buf);
            fclose(f);
            return 1;
        }
        fclose(f);
    }

    if (stepCount < 0) stepCount = (int)totalSolves;
    if ((long)stepCount > totalSolves) {
        fprintf(stderr, "step_count=%d exceeds total solves=%ld\n", stepCount, totalSolves);
        free(buf);
        return 1;
    }
    if (stepCount <= 0) {
        fprintf(stderr, "Invalid step_count: %d\n", stepCount);
        free(buf);
        return 1;
    }

    RootXformEntry rtChain[MAX_RT_CHAIN];
    int nRt = 0;
    if (rtPath) {
        nRt = parse_root_xform_file(rtPath, rtChain, MAX_RT_CHAIN);
        if (nRt == 0) {
            fprintf(stderr, "Failed to parse root transforms from %s\n", rtPath);
            free(buf);
            return 1;
        }
    }

    threads = clamp_threads(threads, stepCount);
    if (strcmp(inputMode, "sectioned") == 0) {
        CURLcode curlRc = curl_global_init(CURL_GLOBAL_ALL);
        if (curlRc != CURLE_OK) {
            fprintf(stderr, "curl_global_init failed: %s\n", curl_easy_strerror(curlRc));
            free(buf);
            return 1;
        }
    }

    float *scoresOut = malloc((size_t)stepCount * sizeof(float));
    unsigned char *binsOut = malloc((size_t)stepCount);
    if (!scoresOut || !binsOut) {
        fprintf(stderr, "Out of memory for outputs\n");
        free(scoresOut);
        free(binsOut);
        free(buf);
        if (strcmp(inputMode, "sectioned") == 0) curl_global_cleanup();
        return 1;
    }

    pthread_t *threadIds = calloc((size_t)threads, sizeof(pthread_t));
    ChunkWorkerArgs *args = calloc((size_t)threads, sizeof(ChunkWorkerArgs));
    if (!threadIds || !args) {
        fprintf(stderr, "Out of memory for thread state\n");
        free(threadIds);
        free(args);
        free(scoresOut);
        free(binsOut);
        free(buf);
        if (strcmp(inputMode, "sectioned") == 0) curl_global_cleanup();
        return 1;
    }

    long base = stepCount / threads;
    long rem = stepCount % threads;
    long solveStart = 0;
    for (int t = 0; t < threads; t++) {
        long solveCount = base + (t < rem ? 1 : 0);
        ChunkWorkerArgs *arg = &args[t];
        arg->sharedBuf = buf;
        arg->url = url;
        arg->inputMode = inputMode;
        arg->degree = degree;
        arg->solveStart = solveStart;
        arg->solveCount = solveCount;
        arg->solveBytes = solveBytes;
        arg->sectionBytes = (size_t)((unsigned long long)solveCount * (unsigned long long)solveBytes);
        arg->byteStart = (unsigned long long)solveStart * (unsigned long long)solveBytes;
        arg->byteEnd = arg->byteStart + (unsigned long long)arg->sectionBytes - 1ULL;
        arg->metric = metric;
        arg->rtChain = rtChain;
        arg->nRt = nRt;
        arg->clipLo = clipLo;
        arg->clipHi = clipHi;
        arg->omega = omega;
        arg->omegaEnabled = omegaEnabled;
        memcpy(arg->cuts, cuts, sizeof(cuts));
        arg->retries = retries;
        arg->scoresOut = scoresOut;
        arg->binsOut = binsOut;
        solveStart += solveCount;
    }

    for (int t = 0; t < threads; t++) {
        if (pthread_create(&threadIds[t], NULL, chunk_worker_main, &args[t]) != 0) {
            fprintf(stderr, "pthread_create failed\n");
            for (int j = 0; j < t; j++) pthread_join(threadIds[j], NULL);
            free(threadIds);
            free(args);
            free(scoresOut);
            free(binsOut);
            free(buf);
            if (strcmp(inputMode, "sectioned") == 0) curl_global_cleanup();
            return 1;
        }
    }

    long totalDownloadMs = 0;
    long totalComputeMs = 0;
    long totalBytesDownloaded = 0;
    for (int t = 0; t < threads; t++) {
        pthread_join(threadIds[t], NULL);
        totalDownloadMs += args[t].downloadMs;
        totalComputeMs += args[t].computeMs;
        totalBytesDownloaded += args[t].bytesDownloaded;
        if (args[t].failed) {
            fprintf(stderr, "%s\n", args[t].error[0] ? args[t].error : "palette chunk worker failed");
            free(threadIds);
            free(args);
            free(scoresOut);
            free(binsOut);
            free(buf);
            if (strcmp(inputMode, "sectioned") == 0) curl_global_cleanup();
            return 1;
        }
    }

    FILE *sf = fopen(scoresOutPath, "wb");
    if (!sf) {
        fprintf(stderr, "Cannot open %s\n", scoresOutPath);
        free(threadIds);
        free(args);
        free(scoresOut);
        free(binsOut);
        free(buf);
        if (strcmp(inputMode, "sectioned") == 0) curl_global_cleanup();
        return 1;
    }
    if (fwrite(scoresOut, sizeof(float), (size_t)stepCount, sf) != (size_t)stepCount) {
        fprintf(stderr, "Short write to %s\n", scoresOutPath);
        fclose(sf);
        free(threadIds);
        free(args);
        free(scoresOut);
        free(binsOut);
        free(buf);
        if (strcmp(inputMode, "sectioned") == 0) curl_global_cleanup();
        return 1;
    }
    fclose(sf);

    FILE *bf = fopen(binsOutPath, "wb");
    if (!bf) {
        fprintf(stderr, "Cannot open %s\n", binsOutPath);
        free(threadIds);
        free(args);
        free(scoresOut);
        free(binsOut);
        free(buf);
        if (strcmp(inputMode, "sectioned") == 0) curl_global_cleanup();
        return 1;
    }
    if (fwrite(binsOut, 1, (size_t)stepCount, bf) != (size_t)stepCount) {
        fprintf(stderr, "Short write to %s\n", binsOutPath);
        fclose(bf);
        free(threadIds);
        free(args);
        free(scoresOut);
        free(binsOut);
        free(buf);
        if (strcmp(inputMode, "sectioned") == 0) curl_global_cleanup();
        return 1;
    }
    fclose(bf);

    double minScore = scoresOut[0];
    double maxScore = scoresOut[0];
    for (int i = 1; i < stepCount; i++) {
        if (scoresOut[i] < minScore) minScore = scoresOut[i];
        if (scoresOut[i] > maxScore) maxScore = scoresOut[i];
    }

    printf("{\"mode\":\"palette_chunk\",\"metric\":\"%s\",\"n_samples\":%d,"
           "\"omega\":%.15g,\"omega_enabled\":%s,\"clip_lo\":%.15g,\"clip_hi\":%.15g,"
           "\"min_score\":%.15g,\"max_score\":%.15g,"
           "\"threads\":%d,\"input_mode\":\"%s\",\"retries\":%d,"
           "\"download_ms\":%ld,\"compute_ms\":%ld,\"bytes_downloaded\":%ld}\n",
           solve_metric_name(metric), stepCount, omega, omegaEnabled ? "true" : "false",
           clipLo, clipHi, minScore, maxScore, threads, inputMode, retries,
           totalDownloadMs, totalComputeMs, totalBytesDownloaded);

    free(threadIds);
    free(args);
    free(scoresOut);
    free(binsOut);
    free(buf);
    if (strcmp(inputMode, "sectioned") == 0) curl_global_cleanup();
    return 0;
}
