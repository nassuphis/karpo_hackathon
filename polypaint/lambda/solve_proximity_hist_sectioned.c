/*
 * solve_proximity_hist_sectioned — solve-score histogram worker with native
 * sectioned HTTP range GETs.
 *
 * One thread owns one solve-aligned section end to end:
 *   - download byte range from a presigned URL
 *   - decode solves in that section
 *   - compute metric histogram locally
 *
 * Output: JSON to stdout, compatible with the existing hist artifact contract,
 * plus timing fields for download/compute/wall.
 *
 * Build (Amazon Linux / Lambda-compatible):
 *   gcc -O3 -pthread -o solve_proximity_hist_sectioned solve_proximity_hist_sectioned.c \
 *     -lcurl -lm -Wl,-rpath,'$ORIGIN/lib'
 */

#include <curl/curl.h>
#include <math.h>
#include <pthread.h>
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

static double getArgDouble(int argc, char **argv, const char *key, double def) {
    const char *v = getArg(argc, argv, key);
    return v ? atof(v) : def;
}

static long long getArgLongLong(int argc, char **argv, const char *key, long long def) {
    const char *v = getArg(argc, argv, key);
    return v ? atoll(v) : def;
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

static double score_xformed(const float *roots, int degree, enum SolveMetric metric,
                            RootXformEntry *rtChain, int nRt,
                            float *wkRe, float *wkIm) {
    for (int k = 0; k < degree; k++) {
        wkRe[k] = roots[k * 2];
        wkIm[k] = roots[k * 2 + 1];
    }
    apply_root_xforms(rtChain, nRt, wkRe, wkIm, degree);
    float xformed[MAXDEG * 2];
    for (int k = 0; k < degree; k++) {
        xformed[k * 2] = wkRe[k];
        xformed[k * 2 + 1] = wkIm[k];
    }
    return compute_solve_metric_score(xformed, degree, metric);
}

static double score_program_xformed(const float *roots, int degree, const SolveScoreProgram *program,
                                    RootXformEntry *rtChain, int nRt,
                                    float *wkRe, float *wkIm) {
    for (int k = 0; k < degree; k++) {
        wkRe[k] = roots[k * 2];
        wkIm[k] = roots[k * 2 + 1];
    }
    apply_root_xforms(rtChain, nRt, wkRe, wkIm, degree);
    float xformed[MAXDEG * 2];
    for (int k = 0; k < degree; k++) {
        xformed[k * 2] = wkRe[k];
        xformed[k * 2 + 1] = wkIm[k];
    }
    return solve_score_eval_program(xformed, degree, program);
}

static double eval_score_or_program(const float *roots, int degree,
                                    enum SolveMetric metric, const SolveScoreProgram *program,
                                    RootXformEntry *rtChain, int nRt,
                                    float *wkRe, float *wkIm) {
    if (program) {
        if (nRt > 0) return score_program_xformed(roots, degree, program, rtChain, nRt, wkRe, wkIm);
        return solve_score_eval_program(roots, degree, program);
    }
    if (nRt > 0) return score_xformed(roots, degree, metric, rtChain, nRt, wkRe, wkIm);
    return compute_solve_metric_score(roots, degree, metric);
}

typedef struct {
    unsigned char *data;
    size_t expected;
    size_t size;
    int overflow;
} DownloadBuffer;

typedef struct {
    const char *url;
    int degree;
    long startSolve;
    long solveCount;
    long solveBytes;
    size_t sectionBytes;
    unsigned long long byteStart;
    unsigned long long byteEnd;
    enum SolveMetric metric;
    const SolveScoreProgram *program;
    RootXformEntry *rtChain;
    int nRt;
    double clipLo;
    double clipHi;
    double omega;
    int omegaEnabled;
    int histBins;
    long *hist;
    long downloadMs;
    long computeMs;
    long bytesDownloaded;
    int retries;
    int failed;
    long httpStatus;
    char error[256];
} HistSectionArgs;

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

static void *hist_section_worker_main(void *arg_) {
    HistSectionArgs *arg = (HistSectionArgs *)arg_;
    DownloadBuffer dl = {0};
    CURL *curl = NULL;
    char rangeBuf[96];
    char curlErr[CURL_ERROR_SIZE] = {0};

    dl.expected = arg->sectionBytes;
    dl.data = malloc(dl.expected > 0 ? dl.expected : 1);
    if (!dl.data) {
        snprintf(arg->error, sizeof(arg->error), "Out of memory for section buffer (%zu bytes)", dl.expected);
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
    long dlStart = monotonic_ms();
    CURLcode rc = CURLE_OK;
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
        snprintf(arg->error, sizeof(arg->error), "range GET failed for bytes %s after %d attempt%s: %s",
                 rangeBuf, attempts, attempts == 1 ? "" : "s",
                 curlErr[0] ? curlErr : curl_easy_strerror(rc));
        arg->failed = 1;
        curl_easy_cleanup(curl);
        free(dl.data);
        return NULL;
    }
    if (arg->httpStatus != 206L && arg->httpStatus != 200L) {
        snprintf(arg->error, sizeof(arg->error), "unexpected HTTP status %ld for bytes %s after %d attempt%s",
                 arg->httpStatus, rangeBuf, attempts, attempts == 1 ? "" : "s");
        arg->failed = 1;
        curl_easy_cleanup(curl);
        free(dl.data);
        return NULL;
    }
    if (dl.overflow) {
        snprintf(arg->error, sizeof(arg->error), "range GET overflow for bytes %s after %d attempt%s",
                 rangeBuf, attempts, attempts == 1 ? "" : "s");
        arg->failed = 1;
        curl_easy_cleanup(curl);
        free(dl.data);
        return NULL;
    }
    if (dl.size != dl.expected) {
        snprintf(arg->error, sizeof(arg->error), "short range GET for bytes %s after %d attempt%s: got %zu of %zu bytes",
                 rangeBuf, attempts, attempts == 1 ? "" : "s", dl.size, dl.expected);
        arg->failed = 1;
        curl_easy_cleanup(curl);
        free(dl.data);
        return NULL;
    }
    arg->bytesDownloaded = (long)dl.size;

    float wkRe[MAXDEG], wkIm[MAXDEG];
    long computeStart = monotonic_ms();
    for (long s = 0; s < arg->solveCount; s++) {
        const float *roots = (const float *)(void *)(dl.data + (size_t)s * (size_t)arg->solveBytes);
        double u;
        if (arg->program) {
            u = eval_score_or_program(
                roots, arg->degree, arg->metric, arg->program, arg->rtChain, arg->nRt, wkRe, wkIm
            );
        } else {
            const double range = arg->clipHi - arg->clipLo;
            double score = eval_score_or_program(
                roots, arg->degree, arg->metric, NULL, arg->rtChain, arg->nRt, wkRe, wkIm
            );
            u = (score - arg->clipLo) / range;
            if (u < 0) u = 0;
            if (u > 1) u = 1;
            u = apply_solve_score_transfer(u, arg->omegaEnabled, arg->omega);
        }
        int h = (int)(u * arg->histBins);
        if (h >= arg->histBins) h = arg->histBins - 1;
        arg->hist[h]++;
    }
    arg->computeMs = monotonic_ms() - computeStart;

    curl_easy_cleanup(curl);
    free(dl.data);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: solve_proximity_hist_sectioned --url=URL --input_size=BYTES --degree=D "
                "[--metric=proximity|crowding|spread|anisotropy|area|clusteriness|shelliness|outlierness|nn_variation|real_axis_proximity|centroid_re|centroid_im|centroid_dist|dist_unit_circle|asymmetry_re] "
                "--clip_lo=X --clip_hi=Y [--hist_bins=100] [--omega=1] [--threads=2] [--root_xforms=file.json]\n");
        return 1;
    }

    const char *url = getArgStr(argc, argv, "--url", NULL);
    long long inputSize = getArgLongLong(argc, argv, "--input_size", -1);
    int degree = getArgInt(argc, argv, "--degree", 0);
    const char *metricStr = getArgStr(argc, argv, "--metric", "proximity");
    double clipLo = getArgDouble(argc, argv, "--clip_lo", 0.0);
    double clipHi = getArgDouble(argc, argv, "--clip_hi", 0.0);
    int histBins = getArgInt(argc, argv, "--hist_bins", 100);
    double omega = getArgDouble(argc, argv, "--omega", 1.0);
    int omegaEnabled = getArgInt(argc, argv, "--omega_enabled", 1);
    int requestedThreads = getArgInt(argc, argv, "--threads", 2);
    int retries = getArgInt(argc, argv, "--retries", 2);
    const char *scoreMetricsCsv = getArgStr(argc, argv, "--score_metrics", NULL);
    const char *scoreClipLosCsv = getArgStr(argc, argv, "--score_clip_los", NULL);
    const char *scoreClipHisCsv = getArgStr(argc, argv, "--score_clip_his", NULL);
    const char *scoreProgramSpec = getArgStr(argc, argv, "--score_program", NULL);

    if (!url || !*url) {
        fprintf(stderr, "Missing --url\n");
        return 1;
    }
    if (inputSize <= 0) {
        fprintf(stderr, "Missing or invalid --input_size: %lld\n", inputSize);
        return 1;
    }
    if (degree < 1 || degree > MAXDEG) {
        fprintf(stderr, "Invalid degree: %d (must be 1-%d)\n", degree, MAXDEG);
        return 1;
    }
    if (!scoreProgramSpec && clipHi - clipLo < 1e-12) {
        fprintf(stderr, "Invalid clip range: lo=%.15g hi=%.15g\n", clipLo, clipHi);
        return 1;
    }
    if (histBins < 1 || histBins > 10000) {
        fprintf(stderr, "Invalid hist_bins: %d\n", histBins);
        return 1;
    }
    if (retries < 0 || retries > 10) {
        fprintf(stderr, "Invalid retries: %d\n", retries);
        return 1;
    }

    enum SolveMetric metric;
    if (!parse_solve_metric(metricStr, &metric)) {
        fprintf(stderr, "Invalid metric: %s (use proximity|crowding|spread|anisotropy|area|clusteriness|shelliness|outlierness|nn_variation|real_axis_proximity|centroid_re|centroid_im|centroid_dist|dist_unit_circle|asymmetry_re)\n", metricStr);
        return 1;
    }

    long solveBytes = (long)degree * 2L * (long)sizeof(float);
    if (inputSize % solveBytes != 0) {
        fprintf(stderr, "Invalid input_size %lld for degree=%d (solve_bytes=%ld)\n", inputSize, degree, solveBytes);
        return 1;
    }
    long nSolves = (long)(inputSize / solveBytes);
    if (nSolves <= 0) {
        fprintf(stderr, "No solves in input_size=%lld for degree=%d\n", inputSize, degree);
        return 1;
    }

    const char *rtPath = getArgStr(argc, argv, "--root_xforms", NULL);
    RootXformEntry rtChain[MAX_RT_CHAIN];
    int nRt = 0;
    if (rtPath) {
        nRt = parse_root_xform_file(rtPath, rtChain, MAX_RT_CHAIN);
        if (nRt == 0) {
            fprintf(stderr, "Failed to parse root transforms from %s\n", rtPath);
            return 1;
        }
    }

    SolveScoreProgram scoreProgram;
    int useScoreProgram = 0;
    if (scoreMetricsCsv || scoreClipLosCsv || scoreClipHisCsv || scoreProgramSpec) {
        char scoreErr[256] = {0};
        if (!scoreMetricsCsv || !scoreClipLosCsv || !scoreClipHisCsv || !scoreProgramSpec) {
            fprintf(stderr, "score program requires --score_metrics, --score_clip_los, --score_clip_his, and --score_program together\n");
            return 1;
        }
        if (!parse_solve_score_program_args(
                scoreMetricsCsv, scoreClipLosCsv, scoreClipHisCsv, scoreProgramSpec,
                &scoreProgram, scoreErr, sizeof(scoreErr))) {
            fprintf(stderr, "Invalid score program: %s\n", scoreErr[0] ? scoreErr : "unknown error");
            return 1;
        }
        useScoreProgram = 1;
    }

    int threads = clamp_threads(requestedThreads, nSolves);
    HistSectionArgs *args = calloc((size_t)threads, sizeof(HistSectionArgs));
    pthread_t *workers = calloc((size_t)threads, sizeof(pthread_t));
    long *hist = calloc((size_t)histBins, sizeof(long));
    if (!args || !workers || !hist) {
        fprintf(stderr, "Out of memory for section workers\n");
        free(args);
        free(workers);
        free(hist);
        return 1;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    long wallStart = monotonic_ms();
    long base = nSolves / threads;
    long extra = nSolves % threads;
    long startSolve = 0;
    for (int i = 0; i < threads; i++) {
        long solveCount = base + (i < extra ? 1 : 0);
        args[i].url = url;
        args[i].degree = degree;
        args[i].startSolve = startSolve;
        args[i].solveCount = solveCount;
        args[i].solveBytes = solveBytes;
        args[i].sectionBytes = (size_t)solveCount * (size_t)solveBytes;
        args[i].byteStart = (unsigned long long)startSolve * (unsigned long long)solveBytes;
        args[i].byteEnd = args[i].byteStart + (unsigned long long)args[i].sectionBytes - 1ULL;
        args[i].metric = metric;
        args[i].program = useScoreProgram ? &scoreProgram : NULL;
        args[i].rtChain = rtChain;
        args[i].nRt = nRt;
        args[i].clipLo = clipLo;
        args[i].clipHi = clipHi;
        args[i].omega = omega;
        args[i].omegaEnabled = omegaEnabled;
        args[i].histBins = histBins;
        args[i].retries = retries;
        args[i].hist = calloc((size_t)histBins, sizeof(long));
        if (!args[i].hist) {
            fprintf(stderr, "Out of memory for hist bins\n");
            for (int j = 0; j < i; j++) free(args[j].hist);
            free(args);
            free(workers);
            free(hist);
            curl_global_cleanup();
            return 1;
        }
        pthread_create(&workers[i], NULL, hist_section_worker_main, &args[i]);
        startSolve += solveCount;
    }

    int failed = 0;
    char errorMsg[256] = {0};
    long downloadMs = 0;
    long computeMs = 0;
    long bytesDownloaded = 0;
    for (int i = 0; i < threads; i++) {
        pthread_join(workers[i], NULL);
        if (args[i].failed && !failed) {
            failed = 1;
            snprintf(errorMsg, sizeof(errorMsg), "%s", args[i].error[0] ? args[i].error : "unknown section worker error");
        }
        downloadMs += args[i].downloadMs;
        computeMs += args[i].computeMs;
        bytesDownloaded += args[i].bytesDownloaded;
        if (!args[i].failed) {
            for (int h = 0; h < histBins; h++) hist[h] += args[i].hist[h];
        }
        free(args[i].hist);
    }
    long wallMs = monotonic_ms() - wallStart;

    curl_global_cleanup();

    if (failed) {
        fprintf(stderr, "%s\n", errorMsg);
        free(args);
        free(workers);
        free(hist);
        return 1;
    }

    const char *metricName = solve_metric_name(metric);
    printf("{\"mode\":\"hist\",\"metric\":\"%s\",\"n_solves\":%ld,\"degree\":%d,\"threads\":%d,"
           "\"hist_bins\":%d,\"retries\":%d,\"omega\":%.15g,\"omega_enabled\":%s,\"clip_lo\":%.15g,\"clip_hi\":%.15g,"
           "\"download_ms\":%ld,\"compute_ms\":%ld,\"wall_ms\":%ld,\"bytes_downloaded\":%ld,"
           "\"hist\":[",
           metricName, nSolves, degree, threads, histBins, retries, omega,
           omegaEnabled ? "true" : "false", clipLo, clipHi,
           downloadMs, computeMs, wallMs, bytesDownloaded);
    for (int i = 0; i < histBins; i++) {
        if (i > 0) printf(",");
        printf("%ld", hist[i]);
    }
    printf("]}\n");

    free(args);
    free(workers);
    free(hist);
    return 0;
}
