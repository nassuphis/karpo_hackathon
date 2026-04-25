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

#include "multispan_reader.h"
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

static const float *prepare_step(const float *raw, int degree,
                                 RootXformEntry *rtChain, int nRt,
                                 float *stepBuf, float *wkRe, float *wkIm) {
    if (nRt <= 0) return raw;
    for (int i = 0; i < degree; i++) {
        wkRe[i] = raw[i * 2];
        wkIm[i] = raw[i * 2 + 1];
    }
    apply_root_xforms(rtChain, nRt, wkRe, wkIm, degree);
    for (int i = 0; i < degree; i++) {
        stepBuf[i * 2] = wkRe[i];
        stepBuf[i * 2 + 1] = wkIm[i];
    }
    return stepBuf;
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

static double score_program_xformed_with_sources(const float *roots, int degree,
                                                const float *coeffRoots, int coeffDegree,
                                                const float *paramValues, int paramDegree,
                                                const SolveScoreProgram *program,
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
    return solve_score_eval_program_with_sources(
        xformed, degree, coeffRoots, coeffDegree, paramValues, paramDegree, program
    );
}

static int solve_score_program_uses_coeff_sources(const SolveScoreProgram *program) {
    if (!program) return 0;
    for (int i = 0; i < program->metricCount; i++) {
        if (program->metricSources[i] == SOLVE_SCORE_SOURCE_COEFF) return 1;
    }
    return 0;
}

static int solve_score_program_uses_param_sources(const SolveScoreProgram *program) {
    if (!program) return 0;
    for (int i = 0; i < program->metricCount; i++) {
        if (program->metricSources[i] == SOLVE_SCORE_SOURCE_PARAM) return 1;
    }
    return 0;
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

static double eval_score_or_program_with_sources(const float *roots, int degree,
                                                 const float *coeffRoots, int coeffDegree,
                                                 const float *paramValues, int paramDegree,
                                                 enum SolveMetric metric, const SolveScoreProgram *program,
                                                 RootXformEntry *rtChain, int nRt,
                                                 float *wkRe, float *wkIm) {
    if (program && (solve_score_program_uses_coeff_sources(program) || solve_score_program_uses_param_sources(program))) {
        if (nRt > 0) {
            return score_program_xformed_with_sources(
                roots, degree, coeffRoots, coeffDegree, paramValues, paramDegree, program, rtChain, nRt, wkRe, wkIm
            );
        }
        return solve_score_eval_program_with_sources(
            roots, degree, coeffRoots, coeffDegree, paramValues, paramDegree, program
        );
    }
    return eval_score_or_program(roots, degree, metric, program, rtChain, nRt, wkRe, wkIm);
}

typedef struct {
    unsigned char *data;
    size_t expected;
    size_t size;
    int overflow;
} DownloadBuffer;

typedef struct {
    const char *url;
    const char *scoreCoeffsUrl;
    const char *inputMode;
    const float *scoreParamRows;
    const MultiSpanReader *inputReader;
    const MultiSpanReader *scoreCoeffReader;
    const MultiSpanReader *scoreParamReader;
    int degree;
    int scoreCoeffDegree;
    int scoreParamDegree;
    int solvePreludeRows;
    int scoreCoeffPreludeRows;
    int scoreParamPreludeRows;
    int usesSolveLag;
    int usesCoeffLag;
    int usesParamLag;
    long startSolve;
    long solveCount;
    long sourceReadStart;
    long scoreCoeffReadStart;
    long scoreParamReadStart;
    long solveBytes;
    long scoreCoeffSolveBytes;
    size_t sectionBytes;
    size_t scoreCoeffSectionBytes;
    size_t scoreParamSectionBytes;
    unsigned long long byteStart;
    unsigned long long byteEnd;
    unsigned long long scoreCoeffByteStart;
    unsigned long long scoreCoeffByteEnd;
    unsigned long long scoreParamByteStart;
    unsigned long long scoreParamByteEnd;
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
    if (strcmp(arg->inputMode, "multispan_sectioned") == 0) {
        unsigned char *rootBytes = NULL;
        unsigned char *coeffBytes = NULL;
        unsigned char *paramBytes = NULL;
        long inputBytesDownloaded = 0;
        long coeffBytesDownloaded = 0;
        long paramBytesDownloaded = 0;
        float wkRe[MAXDEG], wkIm[MAXDEG];

        long dlStart = monotonic_ms();
        rootBytes = malloc(arg->sectionBytes > 0 ? arg->sectionBytes : 1);
        if (!rootBytes) {
            snprintf(arg->error, sizeof(arg->error), "Out of memory for section buffer (%zu bytes)", arg->sectionBytes);
            arg->failed = 1;
            multispan_reader_thread_cleanup();
            return NULL;
        }
        if (!arg->inputReader) {
            snprintf(arg->error, sizeof(arg->error), "multispan_sectioned requires inputReader");
            arg->failed = 1;
            free(rootBytes);
            multispan_reader_thread_cleanup();
            return NULL;
        }
        if (!multispan_reader_read_exact(
                arg->inputReader,
                arg->byteStart,
                arg->sectionBytes,
                rootBytes,
                &inputBytesDownloaded,
                arg->error,
                sizeof(arg->error))) {
            arg->failed = 1;
            free(rootBytes);
            multispan_reader_thread_cleanup();
            return NULL;
        }

        if (arg->scoreCoeffReader && arg->scoreCoeffSectionBytes > 0) {
            coeffBytes = malloc(arg->scoreCoeffSectionBytes > 0 ? arg->scoreCoeffSectionBytes : 1);
            if (!coeffBytes) {
                snprintf(arg->error, sizeof(arg->error), "Out of memory for coeff section buffer (%zu bytes)", arg->scoreCoeffSectionBytes);
                arg->failed = 1;
                free(rootBytes);
                multispan_reader_thread_cleanup();
                return NULL;
            }
            if (!multispan_reader_read_exact(
                    arg->scoreCoeffReader,
                    arg->scoreCoeffByteStart,
                    arg->scoreCoeffSectionBytes,
                    coeffBytes,
                    &coeffBytesDownloaded,
                    arg->error,
                    sizeof(arg->error))) {
                arg->failed = 1;
                free(coeffBytes);
                free(rootBytes);
                multispan_reader_thread_cleanup();
                return NULL;
            }
        }

        if (arg->scoreParamReader && arg->scoreParamSectionBytes > 0) {
            paramBytes = malloc(arg->scoreParamSectionBytes > 0 ? arg->scoreParamSectionBytes : 1);
            if (!paramBytes) {
                snprintf(arg->error, sizeof(arg->error), "Out of memory for param section buffer (%zu bytes)", arg->scoreParamSectionBytes);
                arg->failed = 1;
                free(coeffBytes);
                free(rootBytes);
                multispan_reader_thread_cleanup();
                return NULL;
            }
            if (!multispan_reader_read_exact(
                    arg->scoreParamReader,
                    arg->scoreParamByteStart,
                    arg->scoreParamSectionBytes,
                    paramBytes,
                    &paramBytesDownloaded,
                    arg->error,
                    sizeof(arg->error))) {
                arg->failed = 1;
                free(paramBytes);
                free(coeffBytes);
                free(rootBytes);
                multispan_reader_thread_cleanup();
                return NULL;
            }
        }

        arg->downloadMs = monotonic_ms() - dlStart;
        arg->bytesDownloaded = inputBytesDownloaded + coeffBytesDownloaded + paramBytesDownloaded;

        long sourceRows = arg->solveBytes > 0 ? (long)(arg->sectionBytes / (size_t)arg->solveBytes) : 0;
        long coeffRows = arg->scoreCoeffSolveBytes > 0 ? (long)(arg->scoreCoeffSectionBytes / (size_t)arg->scoreCoeffSolveBytes) : 0;
        long paramRows = arg->scoreParamSectionBytes > 0 ? (long)(arg->scoreParamSectionBytes / (4u * sizeof(float))) : 0;
        int usesLag = arg->program && solve_score_program_uses_lag(arg->program);
        int recentInitialized = 0;
        float prevWkRe[MAXDEG], prevWkIm[MAXDEG];
        float stepBuf[MAXDEG * 2];
        float prevStepBuf[MAXDEG * 2];
        float currentMetricBuffer[SOLVE_SCORE_MAX_METRIC_SLOTS];
        float recentMetricBuffer[SOLVE_SCORE_MAX_METRIC_SLOTS];
        long computeStart = monotonic_ms();
        for (long s = 0; s < arg->solveCount; s++) {
            long globalIdx = arg->startSolve + s;
            long sourceLocalIdx = usesLag ? (arg->solvePreludeRows + globalIdx - arg->sourceReadStart) : s;
            if (sourceLocalIdx < 0 || sourceLocalIdx >= sourceRows) {
                snprintf(arg->error, sizeof(arg->error), "source local index out of range");
                arg->failed = 1;
                break;
            }
            const float *roots = (const float *)(void *)(rootBytes + (size_t)sourceLocalIdx * (size_t)arg->solveBytes);
            const float *coeffRoots = NULL;
            const float *paramValues = NULL;
            if (coeffBytes) {
                long coeffLocalIdx = usesLag ? (arg->scoreCoeffPreludeRows + globalIdx - arg->scoreCoeffReadStart) : s;
                if (coeffLocalIdx < 0 || coeffLocalIdx >= coeffRows) {
                    snprintf(arg->error, sizeof(arg->error), "coeff local index out of range");
                    arg->failed = 1;
                    break;
                }
                coeffRoots = (const float *)(void *)(coeffBytes + (size_t)coeffLocalIdx * (size_t)arg->scoreCoeffSolveBytes);
            }
            if (paramBytes) {
                long paramLocalIdx = usesLag ? (arg->scoreParamPreludeRows + globalIdx - arg->scoreParamReadStart) : s;
                if (paramLocalIdx < 0 || paramLocalIdx >= paramRows) {
                    snprintf(arg->error, sizeof(arg->error), "param local index out of range");
                    arg->failed = 1;
                    break;
                }
                paramValues = (const float *)(void *)(paramBytes + (size_t)paramLocalIdx * 4u * sizeof(float));
            }
            double u;
            if (usesLag) {
                const float *step = prepare_step(roots, arg->degree, arg->rtChain, arg->nRt, stepBuf, wkRe, wkIm);
                if (!solve_score_eval_metric_slots(
                        step, arg->degree, coeffRoots, arg->scoreCoeffDegree,
                        paramValues, arg->scoreParamDegree, arg->program,
                        currentMetricBuffer)) {
                    snprintf(arg->error, sizeof(arg->error), "solve-score metric evaluation failed");
                    arg->failed = 1;
                    break;
                }
                if (!recentInitialized) {
                    memcpy(recentMetricBuffer, currentMetricBuffer, sizeof(float) * (size_t)arg->program->metricCount);
                    const float *prevStep = step;
                    const float *prevCoeffRoots = coeffRoots;
                    const float *prevParamValues = paramValues;
                    long prevSourceLocalIdx = sourceLocalIdx - 1;
                    long prevCoeffLocalIdx = arg->scoreCoeffPreludeRows + globalIdx - arg->scoreCoeffReadStart - 1;
                    long prevParamLocalIdx = arg->scoreParamPreludeRows + globalIdx - arg->scoreParamReadStart - 1;
                    if (arg->usesSolveLag && prevSourceLocalIdx >= 0) {
                        const float *prevRaw = (const float *)(void *)(rootBytes + (size_t)prevSourceLocalIdx * (size_t)arg->solveBytes);
                        prevStep = prepare_step(prevRaw, arg->degree, arg->rtChain, arg->nRt, prevStepBuf, prevWkRe, prevWkIm);
                    }
                    if (arg->usesCoeffLag && coeffBytes && prevCoeffLocalIdx >= 0 && prevCoeffLocalIdx < coeffRows) {
                        prevCoeffRoots = (const float *)(void *)(coeffBytes + (size_t)prevCoeffLocalIdx * (size_t)arg->scoreCoeffSolveBytes);
                    }
                    if (arg->usesParamLag && paramBytes && prevParamLocalIdx >= 0 && prevParamLocalIdx < paramRows) {
                        prevParamValues = (const float *)(void *)(paramBytes + (size_t)prevParamLocalIdx * 4u * sizeof(float));
                    }
                    if (!solve_score_eval_lagged_metric_slots(
                            prevStep, arg->degree, prevCoeffRoots, arg->scoreCoeffDegree,
                            prevParamValues, arg->scoreParamDegree, arg->program,
                            recentMetricBuffer)) {
                        snprintf(arg->error, sizeof(arg->error), "solve-score lag metric evaluation failed");
                        arg->failed = 1;
                        break;
                    }
                    recentInitialized = 1;
                }
                u = solve_score_eval_program_from_buffers(currentMetricBuffer, recentMetricBuffer, arg->program);
                if (!isfinite(u)) {
                    snprintf(arg->error, sizeof(arg->error), "solve-score program evaluation failed");
                    arg->failed = 1;
                    break;
                }
            } else if (arg->program) {
                u = eval_score_or_program_with_sources(
                    roots, arg->degree, coeffRoots, arg->scoreCoeffDegree, paramValues, arg->scoreParamDegree,
                    arg->metric, arg->program, arg->rtChain, arg->nRt, wkRe, wkIm
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
            if (usesLag) {
                memcpy(recentMetricBuffer, currentMetricBuffer, sizeof(float) * (size_t)arg->program->metricCount);
            }
        }
        arg->computeMs = monotonic_ms() - computeStart;

        free(paramBytes);
        free(coeffBytes);
        free(rootBytes);
        multispan_reader_thread_cleanup();
        return NULL;
    }

    DownloadBuffer dl = {0};
    DownloadBuffer coeffDl = {0};
    CURL *curl = NULL;
    CURL *coeffCurl = NULL;
    char rangeBuf[96];
    char coeffRangeBuf[96];
    char curlErr[CURL_ERROR_SIZE] = {0};
    char coeffCurlErr[CURL_ERROR_SIZE] = {0};

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

    if (arg->scoreCoeffsUrl && arg->scoreCoeffSectionBytes > 0) {
        coeffDl.expected = arg->scoreCoeffSectionBytes;
        coeffDl.data = malloc(coeffDl.expected > 0 ? coeffDl.expected : 1);
        if (!coeffDl.data) {
            snprintf(arg->error, sizeof(arg->error), "Out of memory for coeff section buffer (%zu bytes)", coeffDl.expected);
            arg->failed = 1;
            curl_easy_cleanup(curl);
            free(dl.data);
            return NULL;
        }
        coeffCurl = curl_easy_init();
        if (!coeffCurl) {
            snprintf(arg->error, sizeof(arg->error), "curl_easy_init failed for coeff section");
            arg->failed = 1;
            curl_easy_cleanup(curl);
            free(coeffDl.data);
            free(dl.data);
            return NULL;
        }
        snprintf(coeffRangeBuf, sizeof(coeffRangeBuf), "%llu-%llu",
                 (unsigned long long)arg->scoreCoeffByteStart, (unsigned long long)arg->scoreCoeffByteEnd);
        curl_easy_setopt(coeffCurl, CURLOPT_URL, arg->scoreCoeffsUrl);
        curl_easy_setopt(coeffCurl, CURLOPT_RANGE, coeffRangeBuf);
        curl_easy_setopt(coeffCurl, CURLOPT_WRITEFUNCTION, write_section_cb);
        curl_easy_setopt(coeffCurl, CURLOPT_WRITEDATA, &coeffDl);
        curl_easy_setopt(coeffCurl, CURLOPT_ERRORBUFFER, coeffCurlErr);
        curl_easy_setopt(coeffCurl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(coeffCurl, CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(coeffCurl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(coeffCurl, CURLOPT_ACCEPT_ENCODING, "identity");
        curl_easy_setopt(coeffCurl, CURLOPT_TCP_KEEPALIVE, 1L);
    }

    int attempts = arg->retries + 1;
    long dlStart = monotonic_ms();
    CURLcode rc = CURLE_OK;
    CURLcode coeffRc = CURLE_OK;
    long coeffHttpStatus = 0;
    for (int attempt = 0; attempt < attempts; attempt++) {
        dl.size = 0;
        dl.overflow = 0;
        curlErr[0] = '\0';
        arg->httpStatus = 0;
        rc = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &arg->httpStatus);
        int coeffOk = 1;
        if (coeffCurl) {
            coeffDl.size = 0;
            coeffDl.overflow = 0;
            coeffCurlErr[0] = '\0';
            coeffHttpStatus = 0;
            coeffRc = curl_easy_perform(coeffCurl);
            curl_easy_getinfo(coeffCurl, CURLINFO_RESPONSE_CODE, &coeffHttpStatus);
            coeffOk = (
                coeffRc == CURLE_OK &&
                (coeffHttpStatus == 206L || coeffHttpStatus == 200L) &&
                !coeffDl.overflow &&
                coeffDl.size == coeffDl.expected
            );
        }
        if (rc == CURLE_OK &&
            (arg->httpStatus == 206L || arg->httpStatus == 200L) &&
            !dl.overflow &&
            dl.size == dl.expected &&
            coeffOk) {
            break;
        }
        int rootRetryable = retryable_range_failure(rc, arg->httpStatus);
        int coeffRetryable = coeffCurl ? retryable_range_failure(coeffRc, coeffHttpStatus) : 0;
        if (attempt + 1 >= attempts || (!rootRetryable && !coeffRetryable)) {
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
        if (coeffCurl) curl_easy_cleanup(coeffCurl);
        free(coeffDl.data);
        free(dl.data);
        return NULL;
    }
    if (arg->httpStatus != 206L && arg->httpStatus != 200L) {
        snprintf(arg->error, sizeof(arg->error), "unexpected HTTP status %ld for bytes %s after %d attempt%s",
                 arg->httpStatus, rangeBuf, attempts, attempts == 1 ? "" : "s");
        arg->failed = 1;
        curl_easy_cleanup(curl);
        if (coeffCurl) curl_easy_cleanup(coeffCurl);
        free(coeffDl.data);
        free(dl.data);
        return NULL;
    }
    if (dl.overflow) {
        snprintf(arg->error, sizeof(arg->error), "range GET overflow for bytes %s after %d attempt%s",
                 rangeBuf, attempts, attempts == 1 ? "" : "s");
        arg->failed = 1;
        curl_easy_cleanup(curl);
        if (coeffCurl) curl_easy_cleanup(coeffCurl);
        free(coeffDl.data);
        free(dl.data);
        return NULL;
    }
    if (dl.size != dl.expected) {
        snprintf(arg->error, sizeof(arg->error), "short range GET for bytes %s after %d attempt%s: got %zu of %zu bytes",
                 rangeBuf, attempts, attempts == 1 ? "" : "s", dl.size, dl.expected);
        arg->failed = 1;
        curl_easy_cleanup(curl);
        if (coeffCurl) curl_easy_cleanup(coeffCurl);
        free(coeffDl.data);
        free(dl.data);
        return NULL;
    }
    if (coeffCurl && coeffRc != CURLE_OK) {
        snprintf(arg->error, sizeof(arg->error), "coeff range GET failed for bytes %s after %d attempt%s: %s",
                 coeffRangeBuf, attempts, attempts == 1 ? "" : "s",
                 coeffCurlErr[0] ? coeffCurlErr : curl_easy_strerror(coeffRc));
        arg->failed = 1;
        curl_easy_cleanup(coeffCurl);
        curl_easy_cleanup(curl);
        free(coeffDl.data);
        free(dl.data);
        return NULL;
    }
    if (coeffCurl && coeffDl.size != coeffDl.expected) {
        snprintf(arg->error, sizeof(arg->error),
                 "short coeff range GET for bytes %s after %d attempt%s: got %zu of %zu bytes",
                 coeffRangeBuf, attempts, attempts == 1 ? "" : "s", coeffDl.size, coeffDl.expected);
        arg->failed = 1;
        curl_easy_cleanup(coeffCurl);
        curl_easy_cleanup(curl);
        free(coeffDl.data);
        free(dl.data);
        return NULL;
    }
    arg->bytesDownloaded = (long)dl.size + (long)coeffDl.size;

    float wkRe[MAXDEG], wkIm[MAXDEG];
    long computeStart = monotonic_ms();
    for (long s = 0; s < arg->solveCount; s++) {
        const float *roots = (const float *)(void *)(dl.data + (size_t)s * (size_t)arg->solveBytes);
        const float *coeffRoots = coeffCurl
            ? (const float *)(void *)(coeffDl.data + (size_t)s * (size_t)arg->scoreCoeffSolveBytes)
            : NULL;
        const float *paramValues = arg->scoreParamRows
            ? (arg->scoreParamRows + (size_t)s * 4u)
            : NULL;
        double u;
        if (arg->program) {
            u = eval_score_or_program_with_sources(
                roots, arg->degree, coeffRoots, arg->scoreCoeffDegree, paramValues, arg->scoreParamDegree,
                arg->metric, arg->program, arg->rtChain, arg->nRt, wkRe, wkIm
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

    if (coeffCurl) curl_easy_cleanup(coeffCurl);
    curl_easy_cleanup(curl);
    free(coeffDl.data);
    free(dl.data);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: solve_proximity_hist_sectioned [--input_mode=sectioned|multispan_sectioned] "
                "(--url=URL --input_size=BYTES | --input_manifest=file.json) --degree=D "
                "[--metric=%s] --clip_lo=X --clip_hi=Y "
                "[--hist_bins=100] [--omega=1] [--threads=2] [--root_xforms=file.json]\n",
                SOLVE_SCORE_METRIC_LIST_TEXT);
        return 1;
    }

    const char *inputMode = getArgStr(argc, argv, "--input_mode", "sectioned");
    const char *url = getArgStr(argc, argv, "--url", NULL);
    const char *inputManifest = getArgStr(argc, argv, "--input_manifest", NULL);
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
    long requestedStepCount = getArgLongLong(argc, argv, "--step_count", -1);
    int solvePreludeRows = getArgInt(argc, argv, "--prelude_rows", 0);
    int scoreCoeffPreludeRows = getArgInt(argc, argv, "--score_coeff_prelude_rows", 0);
    int scoreParamPreludeRows = getArgInt(argc, argv, "--score_param_prelude_rows", 0);
    const char *scoreMetricsCsv = getArgStr(argc, argv, "--score_metrics", NULL);
    const char *scoreSourcesCsv = getArgStr(argc, argv, "--score_sources", NULL);
    const char *scoreClipLosCsv = getArgStr(argc, argv, "--score_clip_los", NULL);
    const char *scoreClipHisCsv = getArgStr(argc, argv, "--score_clip_his", NULL);
    const char *scoreProgramSpec = getArgStr(argc, argv, "--score_program", NULL);
    const char *scoreCoeffsUrl = getArgStr(argc, argv, "--score_coeffs_url", NULL);
    const char *scoreCoeffManifest = getArgStr(argc, argv, "--score_coeff_manifest", NULL);
    const char *scoreParamsFile = getArgStr(argc, argv, "--score_params_file", NULL);
    const char *scoreParamsManifest = getArgStr(argc, argv, "--score_params_manifest", NULL);
    long long scoreCoeffInputSize = getArgLongLong(argc, argv, "--score_coeff_input_size", -1);
    int scoreCoeffDegree = getArgInt(argc, argv, "--score_coeff_degree", 0);

    if (strcmp(inputMode, "sectioned") != 0 && strcmp(inputMode, "multispan_sectioned") != 0) {
        fprintf(stderr, "Invalid input_mode: %s\n", inputMode);
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
    if (solvePreludeRows < 0 || solvePreludeRows > 1 ||
        scoreCoeffPreludeRows < 0 || scoreCoeffPreludeRows > 1 ||
        scoreParamPreludeRows < 0 || scoreParamPreludeRows > 1) {
        fprintf(stderr, "prelude row counts must be 0 or 1 in v1\n");
        return 1;
    }

    enum SolveMetric metric;
    if (!parse_solve_metric(metricStr, &metric)) {
        fprintf(stderr, "Invalid metric: %s (use %s)\n", metricStr, SOLVE_SCORE_METRIC_LIST_TEXT);
        return 1;
    }

    long solveBytes = (long)degree * 2L * (long)sizeof(float);
    long nSolves = 0;
    MultiSpanReader inputReader;
    MultiSpanReader scoreCoeffReader;
    MultiSpanReader scoreParamReader;
    int remoteCurlInitialized = 0;
    memset(&inputReader, 0, sizeof(inputReader));
    memset(&scoreCoeffReader, 0, sizeof(scoreCoeffReader));
    memset(&scoreParamReader, 0, sizeof(scoreParamReader));

#define REMOTE_CLEANUP() \
    do { \
        multispan_reader_close(&scoreParamReader); \
        multispan_reader_close(&scoreCoeffReader); \
        multispan_reader_close(&inputReader); \
        if (remoteCurlInitialized) { \
            curl_global_cleanup(); \
            remoteCurlInitialized = 0; \
        } \
    } while (0)

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
        if (!parse_solve_score_program_args_ex(
                scoreMetricsCsv, scoreSourcesCsv, scoreClipLosCsv, scoreClipHisCsv, scoreProgramSpec,
                &scoreProgram, scoreErr, sizeof(scoreErr))) {
            fprintf(stderr, "Invalid score program: %s\n", scoreErr[0] ? scoreErr : "unknown error");
            return 1;
        }
        useScoreProgram = 1;
    }

    if (strcmp(inputMode, "sectioned") == 0) {
        if (!url || !*url) {
            fprintf(stderr, "Missing --url\n");
            return 1;
        }
        if (inputSize <= 0) {
            fprintf(stderr, "Missing or invalid --input_size: %lld\n", inputSize);
            return 1;
        }
        if (inputSize % solveBytes != 0) {
            fprintf(stderr, "Invalid input_size %lld for degree=%d (solve_bytes=%ld)\n", inputSize, degree, solveBytes);
            return 1;
        }
        nSolves = (long)(inputSize / solveBytes);
        if (nSolves <= 0) {
            fprintf(stderr, "No solves in input_size=%lld for degree=%d\n", inputSize, degree);
            return 1;
        }
    } else {
        char manifestErr[256] = {0};
        CURLcode curlRc;
        if (!inputManifest || !*inputManifest) {
            fprintf(stderr, "multispan_sectioned requires --input_manifest\n");
            return 1;
        }
        curlRc = curl_global_init(CURL_GLOBAL_DEFAULT);
        if (curlRc != CURLE_OK) {
            fprintf(stderr, "curl_global_init failed: %s\n", curl_easy_strerror(curlRc));
            return 1;
        }
        remoteCurlInitialized = 1;
        if (!multispan_reader_open(&inputReader, inputManifest, retries, manifestErr, sizeof(manifestErr))) {
            fprintf(stderr, "failed to open input manifest: %s\n", manifestErr[0] ? manifestErr : "unknown error");
            REMOTE_CLEANUP();
            return 1;
        }
        if ((inputReader.logicalSize % (unsigned long long)solveBytes) != 0ULL) {
            fprintf(stderr, "input manifest logical_size=%llu is not aligned to solve_bytes=%ld\n",
                    inputReader.logicalSize, solveBytes);
            REMOTE_CLEANUP();
            return 1;
        }
        nSolves = (long)(inputReader.logicalSize / (unsigned long long)solveBytes);
        if (nSolves <= 0) {
            fprintf(stderr, "No solves in input manifest logical_size=%llu for degree=%d\n",
                    inputReader.logicalSize, degree);
            REMOTE_CLEANUP();
            return 1;
        }
    }

    int scoreProgramUsesCoeffSources = solve_score_program_uses_coeff_sources(useScoreProgram ? &scoreProgram : NULL);
    int scoreProgramUsesParamSources = solve_score_program_uses_param_sources(useScoreProgram ? &scoreProgram : NULL);
    int scoreProgramUsesLag = useScoreProgram ? solve_score_program_uses_lag(&scoreProgram) : 0;
    int scoreProgramUsesSolveLag = useScoreProgram ? solve_score_program_uses_lag_source(&scoreProgram, SOLVE_SCORE_SOURCE_SOLVE) : 0;
    int scoreProgramUsesCoeffLag = useScoreProgram ? solve_score_program_uses_lag_source(&scoreProgram, SOLVE_SCORE_SOURCE_COEFF) : 0;
    int scoreProgramUsesParamLag = useScoreProgram ? solve_score_program_uses_lag_source(&scoreProgram, SOLVE_SCORE_SOURCE_PARAM) : 0;
    long scoredSolves = requestedStepCount > 0 ? (long)requestedStepCount : nSolves - solvePreludeRows;
    if (scoredSolves <= 0) {
        fprintf(stderr, "Invalid scored solve count: %ld\n", scoredSolves);
        REMOTE_CLEANUP();
        return 1;
    }
    if (scoreProgramUsesLag && strcmp(inputMode, "multispan_sectioned") != 0) {
        fprintf(stderr, "lagged solve-score programs require input_mode=multispan_sectioned\n");
        REMOTE_CLEANUP();
        return 1;
    }
    if (!scoreProgramUsesLag && (solvePreludeRows || scoreCoeffPreludeRows || scoreParamPreludeRows)) {
        fprintf(stderr, "prelude rows require a lagged solve-score program\n");
        REMOTE_CLEANUP();
        return 1;
    }
    if (!scoreProgramUsesCoeffSources && scoreCoeffPreludeRows) {
        fprintf(stderr, "coeff prelude rows require coeff-source metrics\n");
        REMOTE_CLEANUP();
        return 1;
    }
    if (!scoreProgramUsesParamSources && scoreParamPreludeRows) {
        fprintf(stderr, "param prelude rows require param-source metrics\n");
        REMOTE_CLEANUP();
        return 1;
    }
    if (scoreProgramUsesSolveLag != (solvePreludeRows > 0) && !(scoreProgramUsesSolveLag && solvePreludeRows == 0)) {
        fprintf(stderr, "solve prelude rows must match solve-source lag requirements\n");
        REMOTE_CLEANUP();
        return 1;
    }
    if (scoreProgramUsesLag && nSolves != scoredSolves + solvePreludeRows) {
        fprintf(stderr, "input solve count mismatch: got %ld rows expected %ld scored + %d prelude\n",
                nSolves, scoredSolves, solvePreludeRows);
        REMOTE_CLEANUP();
        return 1;
    }
    long scoreCoeffSolveBytes = (long)(scoreCoeffDegree * 2) * (long)sizeof(float);
    if (scoreProgramUsesCoeffSources) {
        if (scoreCoeffDegree < 1 || scoreCoeffDegree > MAXDEG) {
            fprintf(stderr, "Invalid score_coeff_degree: %d (must be 1-%d)\n", scoreCoeffDegree, MAXDEG);
            REMOTE_CLEANUP();
            return 1;
        }
        if (scoreProgramUsesCoeffLag != (scoreCoeffPreludeRows > 0) && !(scoreProgramUsesCoeffLag && scoreCoeffPreludeRows == 0)) {
            fprintf(stderr, "coeff prelude rows must match coeff-source lag requirements\n");
            REMOTE_CLEANUP();
            return 1;
        }
        if (strcmp(inputMode, "sectioned") == 0) {
            if (!scoreCoeffsUrl || !*scoreCoeffsUrl) {
                fprintf(stderr, "score program with coeff sources requires --score_coeffs_url\n");
                REMOTE_CLEANUP();
                return 1;
            }
            if (scoreCoeffInputSize <= 0) {
                fprintf(stderr, "score program with coeff sources requires --score_coeff_input_size\n");
                REMOTE_CLEANUP();
                return 1;
            }
            if ((scoreCoeffInputSize % scoreCoeffSolveBytes) != 0) {
                fprintf(stderr, "Invalid score_coeff_input_size %lld for coeff_degree=%d (solve_bytes=%ld)\n",
                        scoreCoeffInputSize, scoreCoeffDegree, scoreCoeffSolveBytes);
                REMOTE_CLEANUP();
                return 1;
            }
            long coeffPoints = (long)(scoreCoeffInputSize / scoreCoeffSolveBytes);
            if (coeffPoints != nSolves) {
                fprintf(stderr, "score coeff solve count mismatch: got %ld expected %ld\n", coeffPoints, nSolves);
                REMOTE_CLEANUP();
                return 1;
            }
        } else {
            char manifestErr[256] = {0};
            if (!scoreCoeffManifest || !*scoreCoeffManifest) {
                fprintf(stderr, "multispan_sectioned score program with coeff sources requires --score_coeff_manifest\n");
                REMOTE_CLEANUP();
                return 1;
            }
            if (!multispan_reader_open(&scoreCoeffReader, scoreCoeffManifest, retries, manifestErr, sizeof(manifestErr))) {
                fprintf(stderr, "failed to open coeff manifest: %s\n", manifestErr[0] ? manifestErr : "unknown error");
                REMOTE_CLEANUP();
                return 1;
            }
            if ((scoreCoeffReader.logicalSize % (unsigned long long)scoreCoeffSolveBytes) != 0ULL) {
                fprintf(stderr, "coeff manifest logical_size=%llu is not aligned to solve_bytes=%ld\n",
                        scoreCoeffReader.logicalSize, scoreCoeffSolveBytes);
                REMOTE_CLEANUP();
                return 1;
            }
            long coeffPoints = (long)(scoreCoeffReader.logicalSize / (unsigned long long)scoreCoeffSolveBytes);
            if (coeffPoints != scoredSolves + scoreCoeffPreludeRows) {
                fprintf(stderr, "multispan coeff solve count mismatch: got %ld expected %ld scored + %d prelude\n",
                        coeffPoints, scoredSolves, scoreCoeffPreludeRows);
                REMOTE_CLEANUP();
                return 1;
            }
        }
    }
    int scoreParamStride = 4;
    int scoreParamDegree = 2;
    long scoreParamSolveBytes = (long)scoreParamStride * (long)sizeof(float);
    float *scoreParamRows = NULL;
    if (scoreProgramUsesParamSources) {
        if (scoreProgramUsesParamLag != (scoreParamPreludeRows > 0) && !(scoreProgramUsesParamLag && scoreParamPreludeRows == 0)) {
            fprintf(stderr, "param prelude rows must match param-source lag requirements\n");
            REMOTE_CLEANUP();
            return 1;
        }
        if (strcmp(inputMode, "multispan_sectioned") == 0) {
            char manifestErr[256] = {0};
            if (!scoreParamsManifest || !*scoreParamsManifest) {
                fprintf(stderr, "multispan_sectioned score program with param sources requires --score_params_manifest\n");
                REMOTE_CLEANUP();
                return 1;
            }
            if (!multispan_reader_open(&scoreParamReader, scoreParamsManifest, retries, manifestErr, sizeof(manifestErr))) {
                fprintf(stderr, "failed to open param manifest: %s\n", manifestErr[0] ? manifestErr : "unknown error");
                REMOTE_CLEANUP();
                return 1;
            }
            if ((scoreParamReader.logicalSize % (unsigned long long)scoreParamSolveBytes) != 0ULL) {
                fprintf(stderr, "param manifest logical_size=%llu is not aligned to solve_bytes=%ld\n",
                        scoreParamReader.logicalSize, scoreParamSolveBytes);
                REMOTE_CLEANUP();
                return 1;
            }
            long paramPoints = (long)(scoreParamReader.logicalSize / (unsigned long long)scoreParamSolveBytes);
            if (paramPoints != scoredSolves + scoreParamPreludeRows) {
                fprintf(stderr, "multispan params size mismatch: got %ld solves expected %ld scored + %d prelude\n",
                        paramPoints, scoredSolves, scoreParamPreludeRows);
                REMOTE_CLEANUP();
                return 1;
            }
        } else {
            if (!scoreParamsFile || !*scoreParamsFile) {
                fprintf(stderr, "score program with param sources requires --score_params_file\n");
                REMOTE_CLEANUP();
                return 1;
            }
            FILE *fp = fopen(scoreParamsFile, "rb");
            if (!fp) {
                fprintf(stderr, "Cannot open %s\n", scoreParamsFile);
                REMOTE_CLEANUP();
                return 1;
            }
            fseek(fp, 0, SEEK_END);
            long paramFileSize = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            long paramPoints = paramFileSize / scoreParamSolveBytes;
            if (paramPoints != nSolves) {
                fprintf(stderr, "score params size mismatch: got %ld solves expected %ld\n", paramPoints, nSolves);
                fclose(fp);
                REMOTE_CLEANUP();
                return 1;
            }
            scoreParamRows = malloc((size_t)paramFileSize);
            if (!scoreParamRows) {
                fprintf(stderr, "Out of memory for score params\n");
                fclose(fp);
                REMOTE_CLEANUP();
                return 1;
            }
            if ((long)fread(scoreParamRows, 1, (size_t)paramFileSize, fp) != paramFileSize) {
                fprintf(stderr, "Short read from %s\n", scoreParamsFile);
                fclose(fp);
                free(scoreParamRows);
                REMOTE_CLEANUP();
                return 1;
            }
            fclose(fp);
        }
    }

    int threads = clamp_threads(requestedThreads, scoredSolves);
    HistSectionArgs *args = calloc((size_t)threads, sizeof(HistSectionArgs));
    pthread_t *workers = calloc((size_t)threads, sizeof(pthread_t));
    long *hist = calloc((size_t)histBins, sizeof(long));
    if (!args || !workers || !hist) {
        fprintf(stderr, "Out of memory for section workers\n");
        free(args);
        free(workers);
        free(hist);
        REMOTE_CLEANUP();
        return 1;
    }

    if (!remoteCurlInitialized) {
        CURLcode curlRc = curl_global_init(CURL_GLOBAL_DEFAULT);
        if (curlRc != CURLE_OK) {
            fprintf(stderr, "curl_global_init failed: %s\n", curl_easy_strerror(curlRc));
            free(scoreParamRows);
            free(args);
            free(workers);
            free(hist);
            REMOTE_CLEANUP();
            return 1;
        }
        remoteCurlInitialized = 1;
    }

    long wallStart = monotonic_ms();
    long base = scoredSolves / threads;
    long extra = scoredSolves % threads;
    long startSolve = 0;
    for (int i = 0; i < threads; i++) {
        long solveCount = base + (i < extra ? 1 : 0);
        args[i].url = url;
        args[i].inputMode = inputMode;
        args[i].scoreCoeffsUrl = scoreProgramUsesCoeffSources ? scoreCoeffsUrl : NULL;
        args[i].scoreParamRows = (scoreProgramUsesParamSources && strcmp(inputMode, "multispan_sectioned") != 0)
            ? (scoreParamRows + (size_t)startSolve * (size_t)scoreParamStride)
            : NULL;
        args[i].inputReader = strcmp(inputMode, "multispan_sectioned") == 0 ? &inputReader : NULL;
        args[i].scoreCoeffReader = scoreProgramUsesCoeffSources && strcmp(inputMode, "multispan_sectioned") == 0 ? &scoreCoeffReader : NULL;
        args[i].scoreParamReader = scoreProgramUsesParamSources && strcmp(inputMode, "multispan_sectioned") == 0 ? &scoreParamReader : NULL;
        args[i].degree = degree;
        args[i].scoreCoeffDegree = scoreProgramUsesCoeffSources ? scoreCoeffDegree : 0;
        args[i].scoreParamDegree = scoreProgramUsesParamSources ? scoreParamDegree : 0;
        args[i].solvePreludeRows = solvePreludeRows;
        args[i].scoreCoeffPreludeRows = scoreCoeffPreludeRows;
        args[i].scoreParamPreludeRows = scoreParamPreludeRows;
        args[i].usesSolveLag = scoreProgramUsesSolveLag;
        args[i].usesCoeffLag = scoreProgramUsesCoeffLag;
        args[i].usesParamLag = scoreProgramUsesParamLag;
        args[i].startSolve = startSolve;
        args[i].solveCount = solveCount;
        args[i].solveBytes = solveBytes;
        args[i].scoreCoeffSolveBytes = scoreCoeffSolveBytes;
        long sourceCurrentStart = solvePreludeRows + startSolve;
        long sourceReadStart = sourceCurrentStart;
        if (scoreProgramUsesSolveLag && sourceCurrentStart > 0) sourceReadStart -= 1;
        long sourceReadEnd = solvePreludeRows + startSolve + solveCount;
        args[i].sourceReadStart = sourceReadStart;
        args[i].sectionBytes = (size_t)(sourceReadEnd - sourceReadStart) * (size_t)solveBytes;
        args[i].byteStart = (unsigned long long)sourceReadStart * (unsigned long long)solveBytes;
        args[i].byteEnd = args[i].byteStart + (unsigned long long)args[i].sectionBytes - 1ULL;
        if (scoreProgramUsesCoeffSources) {
            long coeffCurrentStart = scoreCoeffPreludeRows + startSolve;
            long coeffReadStart = coeffCurrentStart;
            if (scoreProgramUsesCoeffLag && coeffCurrentStart > 0) coeffReadStart -= 1;
            long coeffReadEnd = scoreCoeffPreludeRows + startSolve + solveCount;
            args[i].scoreCoeffReadStart = coeffReadStart;
            args[i].scoreCoeffSectionBytes = (size_t)(coeffReadEnd - coeffReadStart) * (size_t)scoreCoeffSolveBytes;
            args[i].scoreCoeffByteStart = (unsigned long long)coeffReadStart * (unsigned long long)scoreCoeffSolveBytes;
            args[i].scoreCoeffByteEnd = args[i].scoreCoeffByteStart + (unsigned long long)args[i].scoreCoeffSectionBytes - 1ULL;
        }
        if (scoreProgramUsesParamSources) {
            long paramCurrentStart = scoreParamPreludeRows + startSolve;
            long paramReadStart = paramCurrentStart;
            if (scoreProgramUsesParamLag && paramCurrentStart > 0) paramReadStart -= 1;
            long paramReadEnd = scoreParamPreludeRows + startSolve + solveCount;
            args[i].scoreParamReadStart = paramReadStart;
            args[i].scoreParamSectionBytes = (size_t)(paramReadEnd - paramReadStart) * (size_t)scoreParamSolveBytes;
            args[i].scoreParamByteStart = (unsigned long long)paramReadStart * (unsigned long long)scoreParamSolveBytes;
            args[i].scoreParamByteEnd = args[i].scoreParamByteStart + (unsigned long long)args[i].scoreParamSectionBytes - 1ULL;
        }
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
            free(scoreParamRows);
            free(args);
            free(workers);
            free(hist);
            REMOTE_CLEANUP();
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

    REMOTE_CLEANUP();

    if (failed) {
        fprintf(stderr, "%s\n", errorMsg);
        free(scoreParamRows);
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
           metricName, scoredSolves, degree, threads, histBins, retries, omega,
           omegaEnabled ? "true" : "false", clipLo, clipHi,
           downloadMs, computeMs, wallMs, bytesDownloaded);
    for (int i = 0; i < histBins; i++) {
        if (i > 0) printf(",");
        printf("%ld", hist[i]);
    }
    printf("]}\n");

    free(scoreParamRows);
    free(args);
    free(workers);
    free(hist);
    return 0;
}
