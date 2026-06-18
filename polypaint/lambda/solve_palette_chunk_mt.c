/*
 * solve_palette_chunk_mt — exact palette chunk scorer with native threading and
 * optional sectioned range GET input or multispan logical section input.
 *
 * Usage:
 *   solve_palette_chunk_mt input.bin \
 *     --degree=D --metric=proximity \
 *     --clip_lo=X --clip_hi=Y --cuts=c1,...,c9 --omega=1 \
 *     --step_count=S \
 *     --scores_out=file.bin --bins_out=file.bin \
 *     [--threads=4] [--input_mode=tmpfile|sectioned|multispan_sectioned] [--retries=2] \
 *     [--url=PRESIGNED_URL --input_size=BYTES] [--input_manifest=file.json] \
 *     [--score_coeff_manifest=file.json] [--score_params_manifest=file.json] \
 *     [--root_xforms=file.json]
 *
 * Input .bin format: raw float32 roots, one solve after another, degree complex
 * roots each.
 *
 * Output:
 *   - scores_out: float32[count * output_channel_count]
 *   - bins_out: uint8[count * output_channel_count]. Legacy implicit scalar
 *     writes 0..9 palette bins; explicit program outputs write normalized
 *     color bytes 0..255.
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

#include "multispan_reader.h"
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
    const float *sharedCoeffBuf;
    const float *sharedParamBuf;
    const MultiSpanReader *inputReader;
    const MultiSpanReader *scoreCoeffReader;
    const MultiSpanReader *scoreParamReader;
    const char *url;
    const char *scoreCoeffsUrl;
    const char *inputMode;
    int degree;
    int scoreCoeffDegree;
    int scoreCoeffStride;
    int scoreParamDegree;
    int scoreParamStride;
    int solvePreludeRows;
    int scoreCoeffPreludeRows;
    int scoreParamPreludeRows;
    int usesSolveLag;
    int usesCoeffLag;
    int usesParamLag;
    long solveStart;
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
    SolveScoreProgram program;
    int useProgram;
    RootXformEntry *rtChain;
    int nRt;
    double clipLo;
    double clipHi;
    double omega;
    int omegaEnabled;
    double cuts[9];
    int outputChannelCount;
    double scoreOutputClipLos[SOLVE_SCORE_MAX_OUTPUT_CHANNELS];
    double scoreOutputClipHis[SOLVE_SCORE_MAX_OUTPUT_CHANNELS];
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
    return solve_score_eval_program(xformed, degree, program);
}

static double score_program_xformed_with_sources(const float *roots, int degree,
                                                const float *coeffRoots, int coeffDegree,
                                                const float *paramValues, int paramDegree,
                                                const SolveScoreProgram *program,
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

static unsigned char u_to_bin(double u, const double cuts[9]) {
    if (u < 0) u = 0;
    if (u > 1) u = 1;
    unsigned char bin = 9;
    for (int c = 0; c < 9; c++) {
        if (u <= cuts[c]) {
            bin = (unsigned char)c;
            break;
        }
    }
    return bin;
}

static void compute_scores_for_roots(const float *roots, const float *coeffRoots, const float *paramValues,
                                     long solveCount, long solveStart,
                                     ChunkWorkerArgs *arg) {
    float wkRe[MAXDEG], wkIm[MAXDEG];
    float prevWkRe[MAXDEG], prevWkIm[MAXDEG];
    float stepBuf[MAXDEG * 2];
    float prevStepBuf[MAXDEG * 2];
    float currentMetricBuffer[SOLVE_SCORE_MAX_METRIC_SLOTS];
    float recentMetricBuffer[SOLVE_SCORE_MAX_METRIC_SLOTS];
    int stride = arg->degree * 2;
    int coeffStride = arg->scoreCoeffStride;
    long sourceRows = arg->solveBytes > 0 ? (long)(arg->sectionBytes / (size_t)arg->solveBytes) : 0;
    long coeffRows = arg->scoreCoeffSolveBytes > 0 ? (long)(arg->scoreCoeffSectionBytes / (size_t)arg->scoreCoeffSolveBytes) : 0;
    long paramRows = arg->scoreParamStride > 0
        ? (long)(arg->scoreParamSectionBytes / ((size_t)arg->scoreParamStride * sizeof(float)))
        : 0;
    int usesLag = arg->useProgram && solve_score_program_uses_lag(&arg->program);
    int recentInitialized = 0;
    long computeStart = monotonic_ms();
    for (long s = 0; s < solveCount; s++) {
        long outIdx = solveStart + s;
        long sourceLocalIdx = usesLag ? (arg->solvePreludeRows + outIdx - arg->sourceReadStart) : s;
        if (sourceLocalIdx < 0 || sourceLocalIdx >= sourceRows) {
            snprintf(arg->error, sizeof(arg->error), "source local index out of range");
            arg->failed = 1;
            break;
        }
        const float *solveRoots = roots + sourceLocalIdx * stride;
        const float *solveCoeffRoots = NULL;
        const float *solveParamValues = NULL;
        if (coeffRoots) {
            long coeffLocalIdx = usesLag ? (arg->scoreCoeffPreludeRows + outIdx - arg->scoreCoeffReadStart) : s;
            if (coeffLocalIdx < 0 || coeffLocalIdx >= coeffRows) {
                snprintf(arg->error, sizeof(arg->error), "coeff local index out of range");
                arg->failed = 1;
                break;
            }
            solveCoeffRoots = coeffRoots + coeffLocalIdx * coeffStride;
        }
        if (paramValues) {
            long paramLocalIdx = usesLag ? (arg->scoreParamPreludeRows + outIdx - arg->scoreParamReadStart) : s;
            if (paramLocalIdx < 0 || paramLocalIdx >= paramRows) {
                snprintf(arg->error, sizeof(arg->error), "param local index out of range");
                arg->failed = 1;
                break;
            }
            solveParamValues = paramValues + paramLocalIdx * arg->scoreParamStride;
        }
        double score = 0.0;
        if (arg->useProgram) {
            const float *step = prepare_step(solveRoots, arg->degree, arg->rtChain, arg->nRt, stepBuf, wkRe, wkIm);
            if (!solve_score_eval_metric_slots(
                    step, arg->degree, solveCoeffRoots, arg->scoreCoeffDegree,
                    solveParamValues, arg->scoreParamDegree, &arg->program,
                    currentMetricBuffer)) {
                snprintf(arg->error, sizeof(arg->error), "solve-score metric evaluation failed");
                arg->failed = 1;
                break;
            }
            if (usesLag && !recentInitialized) {
                memcpy(recentMetricBuffer, currentMetricBuffer, sizeof(float) * (size_t)arg->program.metricCount);
                const float *prevStep = step;
                const float *prevCoeffRoots = solveCoeffRoots;
                const float *prevParamValues = solveParamValues;
                long prevSourceLocalIdx = sourceLocalIdx - 1;
                long prevCoeffLocalIdx = arg->scoreCoeffPreludeRows + outIdx - arg->scoreCoeffReadStart - 1;
                long prevParamLocalIdx = arg->scoreParamPreludeRows + outIdx - arg->scoreParamReadStart - 1;
                if (arg->usesSolveLag && prevSourceLocalIdx >= 0) {
                    const float *prevRaw = roots + prevSourceLocalIdx * stride;
                    prevStep = prepare_step(prevRaw, arg->degree, arg->rtChain, arg->nRt, prevStepBuf, prevWkRe, prevWkIm);
                }
                if (arg->usesCoeffLag && coeffRoots && prevCoeffLocalIdx >= 0 && prevCoeffLocalIdx < coeffRows) {
                    prevCoeffRoots = coeffRoots + prevCoeffLocalIdx * coeffStride;
                }
                if (arg->usesParamLag && paramValues && prevParamLocalIdx >= 0 && prevParamLocalIdx < paramRows) {
                    prevParamValues = paramValues + prevParamLocalIdx * arg->scoreParamStride;
                }
                if (!solve_score_eval_lagged_metric_slots(
                        prevStep, arg->degree, prevCoeffRoots, arg->scoreCoeffDegree,
                        prevParamValues, arg->scoreParamDegree, &arg->program,
                        recentMetricBuffer)) {
                    snprintf(arg->error, sizeof(arg->error), "solve-score lag metric evaluation failed");
                    arg->failed = 1;
                    break;
                }
                recentInitialized = 1;
            }
            double outputValues[SOLVE_SCORE_MAX_OUTPUT_CHANNELS];
            int gotOutputs = 0;
            if (!solve_score_eval_program_outputs_from_buffers(
                    currentMetricBuffer,
                    usesLag ? recentMetricBuffer : NULL,
                    &arg->program,
                    outputValues,
                    SOLVE_SCORE_MAX_OUTPUT_CHANNELS,
                    &gotOutputs) ||
                gotOutputs != arg->outputChannelCount) {
                snprintf(arg->error, sizeof(arg->error), "solve-score program evaluation failed");
                arg->failed = 1;
                break;
            }

            score = outputValues[0];
            if (solve_score_program_has_explicit_outputs(&arg->program)) {
                for (int ch = 0; ch < arg->outputChannelCount; ch++) {
                    double u = outputValues[ch];
                    if (solve_score_program_output_is_normalized(&arg->program, ch)) {
                        double range = arg->scoreOutputClipHis[ch] - arg->scoreOutputClipLos[ch];
                        if (isfinite(range) && range > 1e-12) {
                            u = (u - arg->scoreOutputClipLos[ch]) / range;
                        }
                    }
                    u = solve_score_clamp_unit(u);
                    int rawByte = (int)llround(u * 255.0);
                    if (rawByte < 0) rawByte = 0;
                    if (rawByte > 255) rawByte = 255;
                    arg->binsOut[outIdx * arg->outputChannelCount + ch] = (unsigned char)rawByte;
                    arg->scoresOut[outIdx * arg->outputChannelCount + ch] = (float)outputValues[ch];
                }
            } else {
                if (!isfinite(score)) {
                    snprintf(arg->error, sizeof(arg->error), "solve-score program evaluation failed");
                    arg->failed = 1;
                    break;
                }
                arg->scoresOut[outIdx] = (float)score;
                arg->binsOut[outIdx] = u_to_bin(score, arg->cuts);
            }
        } else {
            score = (arg->nRt > 0)
                ? score_xformed(solveRoots, arg->degree, arg->metric, arg->rtChain, arg->nRt, wkRe, wkIm)
                : compute_solve_metric_score(solveRoots, arg->degree, arg->metric);
            arg->scoresOut[outIdx] = (float)score;
            arg->binsOut[outIdx] = score_to_bin(score, arg->clipLo, arg->clipHi, arg->omega, arg->omegaEnabled, arg->cuts);
        }
        if (usesLag) {
            memcpy(recentMetricBuffer, currentMetricBuffer, sizeof(float) * (size_t)arg->program.metricCount);
        }
    }
    arg->computeMs = monotonic_ms() - computeStart;
}

static void *chunk_worker_main(void *arg_) {
    ChunkWorkerArgs *arg = (ChunkWorkerArgs *)arg_;
    if (arg->solveCount <= 0) return NULL;

    if (strcmp(arg->inputMode, "multispan_sectioned") == 0) {
        unsigned char *rootBytes = NULL;
        unsigned char *coeffBytes = NULL;
        unsigned char *paramBytes = NULL;
        long inputBytesDownloaded = 0;
        long coeffBytesDownloaded = 0;
        long paramBytesDownloaded = 0;
        long dlStart = monotonic_ms();

        rootBytes = (unsigned char *)malloc(arg->sectionBytes > 0 ? arg->sectionBytes : 1);
        if (!rootBytes) {
            snprintf(arg->error, sizeof(arg->error),
                     "Out of memory for multispan root buffer (%zu bytes)", arg->sectionBytes);
            arg->failed = 1;
            multispan_reader_thread_cleanup();
            return NULL;
        }
        if (!arg->inputReader) {
            snprintf(arg->error, sizeof(arg->error), "multispan input requires inputReader");
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
            coeffBytes = (unsigned char *)malloc(arg->scoreCoeffSectionBytes > 0 ? arg->scoreCoeffSectionBytes : 1);
            if (!coeffBytes) {
                snprintf(arg->error, sizeof(arg->error),
                         "Out of memory for multispan coeff buffer (%zu bytes)", arg->scoreCoeffSectionBytes);
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
            paramBytes = (unsigned char *)malloc(arg->scoreParamSectionBytes > 0 ? arg->scoreParamSectionBytes : 1);
            if (!paramBytes) {
                snprintf(arg->error, sizeof(arg->error),
                         "Out of memory for multispan param buffer (%zu bytes)", arg->scoreParamSectionBytes);
                arg->failed = 1;
                free(paramBytes);
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
        compute_scores_for_roots(
            (const float *)(void *)rootBytes,
            coeffBytes ? (const float *)(void *)coeffBytes : NULL,
            paramBytes ? (const float *)(void *)paramBytes : NULL,
            arg->solveCount,
            arg->solveStart,
            arg
        );
        free(paramBytes);
        free(coeffBytes);
        free(rootBytes);
        multispan_reader_thread_cleanup();
        return NULL;
    }

    if (strcmp(arg->inputMode, "sectioned") == 0) {
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

        if (arg->scoreCoeffsUrl && arg->scoreCoeffSectionBytes > 0) {
            coeffDl.expected = arg->scoreCoeffSectionBytes;
            coeffDl.data = malloc(coeffDl.expected > 0 ? coeffDl.expected : 1);
            if (!coeffDl.data) {
                snprintf(arg->error, sizeof(arg->error),
                         "Out of memory for coeff section buffer (%zu bytes)", coeffDl.expected);
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
        CURLcode rc = CURLE_OK;
        CURLcode coeffRc = CURLE_OK;
        long coeffHttpStatus = 0;
        long dlStart = monotonic_ms();
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
        if (coeffCurl && coeffRc != CURLE_OK) {
            snprintf(arg->error, sizeof(arg->error),
                     "coeff range GET failed for bytes %s after %d attempt%s: %s",
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
        const float *coeffRoots = coeffCurl ? (const float *)(void *)coeffDl.data : NULL;
        const float *paramValues = arg->sharedParamBuf;
        if (paramValues) paramValues += arg->solveStart * arg->scoreParamStride;
        compute_scores_for_roots(
            (const float *)(void *)dl.data,
            coeffRoots,
            paramValues,
            arg->solveCount,
            arg->solveStart,
            arg
        );
        if (coeffCurl) curl_easy_cleanup(coeffCurl);
        curl_easy_cleanup(curl);
        free(coeffDl.data);
        free(dl.data);
        return NULL;
    }

    const float *roots = arg->sharedBuf + (long)arg->solveStart * (arg->degree * 2);
    const float *coeffRoots = arg->sharedCoeffBuf
        ? (arg->sharedCoeffBuf + (long)arg->solveStart * arg->scoreCoeffStride)
        : NULL;
        const float *paramValues = arg->sharedParamBuf;
        if (paramValues) paramValues += arg->solveStart * arg->scoreParamStride;
        compute_scores_for_roots(roots, coeffRoots, paramValues, arg->solveCount, arg->solveStart, arg);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: solve_palette_chunk_mt input.bin --degree=D --metric=... "
                "--clip_lo=X --clip_hi=Y --cuts=c1,...,c9 --step_count=S "
                "--scores_out=file.bin --bins_out=file.bin [--threads=4] "
                "[--input_mode=tmpfile|sectioned|multispan_sectioned] "
                "[--url=URL --input_size=BYTES] [--input_manifest=file.json] "
                "[--score_coeff_manifest=file.json] [--score_params_manifest=file.json] "
                "[--retries=2] [--root_xforms=file.json]\n");
        return 1;
    }

    const char *inPath = argv[1];
    int degree = getArgInt(argc, argv, "--degree", 0);
    const char *metricStr = getArgStr(argc, argv, "--metric", "proximity");
    const char *scoreMetricsCsv = getArgStr(argc, argv, "--score_metrics", NULL);
    const char *scoreSourcesCsv = getArgStr(argc, argv, "--score_sources", NULL);
    const char *scoreClipLosCsv = getArgStr(argc, argv, "--score_clip_los", NULL);
    const char *scoreClipHisCsv = getArgStr(argc, argv, "--score_clip_his", NULL);
    const char *scoreProgramSpec = getArgStr(argc, argv, "--score_program", NULL);
    const char *scoreOutputClipLosCsv = getArgStr(argc, argv, "--score_output_clip_los", NULL);
    const char *scoreOutputClipHisCsv = getArgStr(argc, argv, "--score_output_clip_his", NULL);
    int requestedOutputChannelCount = getArgInt(argc, argv, "--score_output_channel_count", 1);
    const char *scoreCoeffsFile = getArgStr(argc, argv, "--score_coeffs_file", NULL);
    const char *scoreCoeffsUrl = getArgStr(argc, argv, "--score_coeffs_url", NULL);
    const char *scoreCoeffManifest = getArgStr(argc, argv, "--score_coeff_manifest", NULL);
    const char *scoreParamsFile = getArgStr(argc, argv, "--score_params_file", NULL);
    const char *scoreParamsManifest = getArgStr(argc, argv, "--score_params_manifest", NULL);
    long long scoreCoeffInputSize = getArgLongLong(argc, argv, "--score_coeff_input_size", -1);
    int scoreCoeffDegree = getArgInt(argc, argv, "--score_coeff_degree", 0);
    double clipLo = getArgDouble(argc, argv, "--clip_lo", 0.0);
    double clipHi = getArgDouble(argc, argv, "--clip_hi", 0.0);
    double omega = getArgDouble(argc, argv, "--omega", 1.0);
    int omegaEnabled = getArgInt(argc, argv, "--omega_enabled", 1);
    int stepCount = getArgInt(argc, argv, "--step_count", -1);
    int solvePreludeRows = getArgInt(argc, argv, "--prelude_rows", 0);
    int scoreCoeffPreludeRows = getArgInt(argc, argv, "--score_coeff_prelude_rows", 0);
    int scoreParamPreludeRows = getArgInt(argc, argv, "--score_param_prelude_rows", 0);
    int threads = getArgInt(argc, argv, "--threads", 1);
    int retries = getArgInt(argc, argv, "--retries", 2);
    const char *cutsStr = getArgStr(argc, argv, "--cuts", NULL);
    const char *scoresOutPath = getArgStr(argc, argv, "--scores_out", NULL);
    const char *binsOutPath = getArgStr(argc, argv, "--bins_out", NULL);
    const char *rtPath = getArgStr(argc, argv, "--root_xforms", NULL);
    const char *inputMode = getArgStr(argc, argv, "--input_mode", "tmpfile");
    const char *url = getArgStr(argc, argv, "--url", NULL);
    long long inputSize = getArgLongLong(argc, argv, "--input_size", -1);
    const char *inputManifest = getArgStr(argc, argv, "--input_manifest", NULL);

    if (degree < 2 || degree > MAXDEG) {
        fprintf(stderr, "Invalid degree: %d\n", degree);
        return 1;
    }
    if (!scoresOutPath || !binsOutPath) {
        fprintf(stderr, "Missing --scores_out or --bins_out\n");
        return 1;
    }
    if (!scoreProgramSpec && clipHi - clipLo < 1e-12) {
        fprintf(stderr, "Invalid clip range: lo=%.6g hi=%.6g\n", clipLo, clipHi);
        return 1;
    }
    if (strcmp(inputMode, "tmpfile") != 0 &&
        strcmp(inputMode, "sectioned") != 0 &&
        strcmp(inputMode, "multispan_sectioned") != 0) {
        fprintf(stderr, "Invalid input_mode: %s\n", inputMode);
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
        fprintf(stderr, "Invalid metric: %s\n", metricStr);
        return 1;
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
        metric = scoreProgram.metrics[0];
        useScoreProgram = 1;
    }

    int outputChannelCount = useScoreProgram ? solve_score_program_output_count(&scoreProgram) : 1;
    if (requestedOutputChannelCount > 0) {
        outputChannelCount = requestedOutputChannelCount;
    }
    if (outputChannelCount < 1 || outputChannelCount > SOLVE_SCORE_MAX_OUTPUT_CHANNELS) {
        fprintf(stderr, "Invalid score_output_channel_count: %d\n", outputChannelCount);
        return 1;
    }
    if (!useScoreProgram && outputChannelCount != 1) {
        fprintf(stderr, "legacy metric palette chunk requires score_output_channel_count=1\n");
        return 1;
    }
    if (useScoreProgram && outputChannelCount != solve_score_program_output_count(&scoreProgram)) {
        fprintf(stderr, "score output channel count mismatch: expected %d, got %d\n",
                solve_score_program_output_count(&scoreProgram), outputChannelCount);
        return 1;
    }
    double scoreOutputClipLos[SOLVE_SCORE_MAX_OUTPUT_CHANNELS];
    double scoreOutputClipHis[SOLVE_SCORE_MAX_OUTPUT_CHANNELS];
    for (int i = 0; i < SOLVE_SCORE_MAX_OUTPUT_CHANNELS; i++) {
        scoreOutputClipLos[i] = 0.0;
        scoreOutputClipHis[i] = 1.0;
    }
    if (scoreOutputClipLosCsv || scoreOutputClipHisCsv) {
        char scoreErr[256] = {0};
        if (!scoreOutputClipLosCsv || !scoreOutputClipHisCsv) {
            fprintf(stderr, "score output channel clip arrays require both --score_output_clip_los and --score_output_clip_his\n");
            return 1;
        }
        int loCount = parse_solve_score_double_csv(
            scoreOutputClipLosCsv, scoreOutputClipLos, SOLVE_SCORE_MAX_OUTPUT_CHANNELS, scoreErr, sizeof(scoreErr)
        );
        if (loCount <= 0) {
            fprintf(stderr, "Invalid score output clip lows: %s\n", scoreErr[0] ? scoreErr : "unknown error");
            return 1;
        }
        int hiCount = parse_solve_score_double_csv(
            scoreOutputClipHisCsv, scoreOutputClipHis, SOLVE_SCORE_MAX_OUTPUT_CHANNELS, scoreErr, sizeof(scoreErr)
        );
        if (hiCount <= 0) {
            fprintf(stderr, "Invalid score output clip highs: %s\n", scoreErr[0] ? scoreErr : "unknown error");
            return 1;
        }
        if (loCount != outputChannelCount || hiCount != outputChannelCount) {
            fprintf(stderr, "score output clip array length mismatch: expected %d, got lows=%d highs=%d\n",
                    outputChannelCount, loCount, hiCount);
            return 1;
        }
    }
    if (useScoreProgram) {
        for (int ch = 0; ch < outputChannelCount; ch++) {
            if (solve_score_program_output_is_normalized(&scoreProgram, ch)) {
                if (!isfinite(scoreOutputClipLos[ch]) || !isfinite(scoreOutputClipHis[ch])) {
                    fprintf(stderr, "score output channel %d clip bounds must be finite\n", ch);
                    return 1;
                }
                if (scoreOutputClipHis[ch] - scoreOutputClipLos[ch] <= 1e-12) {
                    fprintf(stderr, "solve_score_output_normalize: degenerate channel %d range [%g,%g], using identity\n",
                            ch, scoreOutputClipLos[ch], scoreOutputClipHis[ch]);
                    scoreOutputClipLos[ch] = 0.0;
                    scoreOutputClipHis[ch] = 1.0;
                }
            }
        }
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
    } else if (strcmp(inputMode, "multispan_sectioned") == 0) {
        char manifestErr[256] = {0};
        if (!inputManifest || !*inputManifest) {
            fprintf(stderr, "multispan_sectioned input requires --input_manifest\n");
            return 1;
        }
        CURLcode curlRc = curl_global_init(CURL_GLOBAL_ALL);
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
        totalSolves = (long)(inputReader.logicalSize / (unsigned long long)solveBytes);
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
        REMOTE_CLEANUP();
        return 1;
    }
    if (stepCount <= 0) {
        fprintf(stderr, "Invalid step_count: %d\n", stepCount);
        free(buf);
        REMOTE_CLEANUP();
        return 1;
    }

    int scoreProgramUsesCoeffSources = solve_score_program_uses_coeff_sources(useScoreProgram ? &scoreProgram : NULL);
    int scoreProgramUsesParamSources = solve_score_program_uses_param_sources(useScoreProgram ? &scoreProgram : NULL);
    int scoreProgramUsesLag = useScoreProgram ? solve_score_program_uses_lag(&scoreProgram) : 0;
    int scoreProgramUsesSolveLag = useScoreProgram ? solve_score_program_uses_lag_source(&scoreProgram, SOLVE_SCORE_SOURCE_SOLVE) : 0;
    int scoreProgramUsesCoeffLag = useScoreProgram ? solve_score_program_uses_lag_source(&scoreProgram, SOLVE_SCORE_SOURCE_COEFF) : 0;
    int scoreProgramUsesParamLag = useScoreProgram ? solve_score_program_uses_lag_source(&scoreProgram, SOLVE_SCORE_SOURCE_PARAM) : 0;
    if (scoreProgramUsesLag && strcmp(inputMode, "multispan_sectioned") != 0) {
        fprintf(stderr, "lagged solve-score programs require input_mode=multispan_sectioned\n");
        free(buf);
        REMOTE_CLEANUP();
        return 1;
    }
    if (!scoreProgramUsesLag && (solvePreludeRows || scoreCoeffPreludeRows || scoreParamPreludeRows)) {
        fprintf(stderr, "prelude rows require a lagged solve-score program\n");
        free(buf);
        REMOTE_CLEANUP();
        return 1;
    }
    if (scoreProgramUsesSolveLag != (solvePreludeRows > 0) && !(scoreProgramUsesSolveLag && solvePreludeRows == 0)) {
        fprintf(stderr, "solve prelude rows must match solve-source lag requirements\n");
        free(buf);
        REMOTE_CLEANUP();
        return 1;
    }
    if (scoreProgramUsesLag && (long)totalSolves != (long)stepCount + solvePreludeRows) {
        fprintf(stderr, "input solve count mismatch: got %ld rows expected %d scored + %d prelude\n",
                totalSolves, stepCount, solvePreludeRows);
        free(buf);
        REMOTE_CLEANUP();
        return 1;
    }
    if (!scoreProgramUsesCoeffSources && scoreCoeffPreludeRows) {
        fprintf(stderr, "coeff prelude rows require coeff-source metrics\n");
        free(buf);
        REMOTE_CLEANUP();
        return 1;
    }
    if (!scoreProgramUsesParamSources && scoreParamPreludeRows) {
        fprintf(stderr, "param prelude rows require param-source metrics\n");
        free(buf);
        REMOTE_CLEANUP();
        return 1;
    }
    float *scoreCoeffRows = NULL;
    int scoreCoeffStride = scoreCoeffDegree * 2;
    long scoreCoeffSolveBytes = (long)scoreCoeffStride * (long)sizeof(float);
    if (scoreProgramUsesCoeffSources) {
        if (scoreCoeffDegree < 1 || scoreCoeffDegree > MAXDEG) {
            fprintf(stderr, "Invalid score_coeff_degree: %d (must be 1-%d)\n", scoreCoeffDegree, MAXDEG);
            free(buf);
            REMOTE_CLEANUP();
            return 1;
        }
        if (scoreProgramUsesCoeffLag != (scoreCoeffPreludeRows > 0) && !(scoreProgramUsesCoeffLag && scoreCoeffPreludeRows == 0)) {
            fprintf(stderr, "coeff prelude rows must match coeff-source lag requirements\n");
            free(buf);
            REMOTE_CLEANUP();
            return 1;
        }
        if (strcmp(inputMode, "sectioned") == 0) {
            if (!scoreCoeffsUrl || !*scoreCoeffsUrl) {
                fprintf(stderr, "sectioned score program with coeff sources requires --score_coeffs_url\n");
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            if (scoreCoeffInputSize <= 0) {
                fprintf(stderr, "sectioned score program with coeff sources requires --score_coeff_input_size\n");
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            if ((scoreCoeffInputSize % scoreCoeffSolveBytes) != 0) {
                fprintf(stderr, "Invalid score_coeff_input_size %lld for coeff_degree=%d (solve_bytes=%ld)\n",
                        scoreCoeffInputSize, scoreCoeffDegree, scoreCoeffSolveBytes);
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            long coeffPoints = (long)(scoreCoeffInputSize / scoreCoeffSolveBytes);
            if (coeffPoints != stepCount) {
                fprintf(stderr, "sectioned score coeff solve count mismatch: got %ld expected %d\n", coeffPoints, stepCount);
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
        } else if (strcmp(inputMode, "multispan_sectioned") == 0) {
            char manifestErr[256] = {0};
            if (!scoreCoeffManifest || !*scoreCoeffManifest) {
                fprintf(stderr, "multispan_sectioned score program with coeff sources requires --score_coeff_manifest\n");
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            if (!multispan_reader_open(&scoreCoeffReader, scoreCoeffManifest, retries, manifestErr, sizeof(manifestErr))) {
                fprintf(stderr, "failed to open coeff manifest: %s\n", manifestErr[0] ? manifestErr : "unknown error");
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            if ((scoreCoeffReader.logicalSize % (unsigned long long)scoreCoeffSolveBytes) != 0ULL) {
                fprintf(stderr, "coeff manifest logical_size=%llu is not aligned to solve_bytes=%ld\n",
                        scoreCoeffReader.logicalSize, scoreCoeffSolveBytes);
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            long coeffPoints = (long)(scoreCoeffReader.logicalSize / (unsigned long long)scoreCoeffSolveBytes);
            if (coeffPoints != (long)stepCount + scoreCoeffPreludeRows) {
                fprintf(stderr, "multispan coeff solve count mismatch: got %ld expected %d scored + %d prelude\n",
                        coeffPoints, stepCount, scoreCoeffPreludeRows);
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
        } else {
            if (!scoreCoeffsFile || !*scoreCoeffsFile) {
                fprintf(stderr, "score program with coeff sources requires --score_coeffs_file\n");
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            FILE *fc = fopen(scoreCoeffsFile, "rb");
            if (!fc) {
                fprintf(stderr, "Cannot open %s\n", scoreCoeffsFile);
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            fseek(fc, 0, SEEK_END);
            long coeffFileSize = ftell(fc);
            fseek(fc, 0, SEEK_SET);
            long coeffPoints = coeffFileSize / scoreCoeffSolveBytes;
            if (coeffPoints != stepCount) {
                fprintf(stderr, "score coeffs size mismatch: got %ld solves expected %d\n", coeffPoints, stepCount);
                fclose(fc);
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            scoreCoeffRows = malloc(coeffFileSize > 0 ? (size_t)coeffFileSize : 1);
            if (!scoreCoeffRows) {
                fprintf(stderr, "Out of memory for score coeff rows\n");
                fclose(fc);
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            if ((long)fread(scoreCoeffRows, 1, (size_t)coeffFileSize, fc) != coeffFileSize) {
                fprintf(stderr, "Short read from %s\n", scoreCoeffsFile);
                fclose(fc);
                free(scoreCoeffRows);
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            fclose(fc);
        }
    }
    float *scoreParamRows = NULL;
    int scoreParamStride = 4;
    int scoreParamDegree = 2;
    if (scoreProgramUsesParamSources) {
        long paramSolveBytes = (long)scoreParamStride * (long)sizeof(float);
        if (scoreProgramUsesParamLag != (scoreParamPreludeRows > 0) && !(scoreProgramUsesParamLag && scoreParamPreludeRows == 0)) {
            fprintf(stderr, "param prelude rows must match param-source lag requirements\n");
            free(scoreCoeffRows);
            free(buf);
            REMOTE_CLEANUP();
            return 1;
        }
        if (strcmp(inputMode, "multispan_sectioned") == 0) {
            char manifestErr[256] = {0};
            if (!scoreParamsManifest || !*scoreParamsManifest) {
                fprintf(stderr, "multispan_sectioned score program with param sources requires --score_params_manifest\n");
                free(scoreCoeffRows);
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            if (!multispan_reader_open(&scoreParamReader, scoreParamsManifest, retries, manifestErr, sizeof(manifestErr))) {
                fprintf(stderr, "failed to open param manifest: %s\n", manifestErr[0] ? manifestErr : "unknown error");
                free(scoreCoeffRows);
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            if ((scoreParamReader.logicalSize % (unsigned long long)paramSolveBytes) != 0ULL) {
                fprintf(stderr, "param manifest logical_size=%llu is not aligned to solve_bytes=%ld\n",
                        scoreParamReader.logicalSize, paramSolveBytes);
                free(scoreCoeffRows);
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            long paramPoints = (long)(scoreParamReader.logicalSize / (unsigned long long)paramSolveBytes);
            if (paramPoints != (long)stepCount + scoreParamPreludeRows) {
                fprintf(stderr, "multispan params size mismatch: got %ld solves expected %d scored + %d prelude\n",
                        paramPoints, stepCount, scoreParamPreludeRows);
                free(scoreCoeffRows);
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
        } else {
            if (!scoreParamsFile || !*scoreParamsFile) {
                fprintf(stderr, "score program with param sources requires --score_params_file\n");
                free(scoreCoeffRows);
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            FILE *fp = fopen(scoreParamsFile, "rb");
            if (!fp) {
                fprintf(stderr, "Cannot open %s\n", scoreParamsFile);
                free(scoreCoeffRows);
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            fseek(fp, 0, SEEK_END);
            long paramFileSize = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            long paramPoints = paramFileSize / paramSolveBytes;
            if (paramPoints != stepCount) {
                fprintf(stderr, "score params size mismatch: got %ld solves expected %d\n", paramPoints, stepCount);
                fclose(fp);
                free(scoreCoeffRows);
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            scoreParamRows = malloc(paramFileSize > 0 ? (size_t)paramFileSize : 1);
            if (!scoreParamRows) {
                fprintf(stderr, "Out of memory for score param rows\n");
                fclose(fp);
                free(scoreCoeffRows);
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            if ((long)fread(scoreParamRows, 1, (size_t)paramFileSize, fp) != paramFileSize) {
                fprintf(stderr, "Short read from %s\n", scoreParamsFile);
                fclose(fp);
                free(scoreParamRows);
                free(scoreCoeffRows);
                free(buf);
                REMOTE_CLEANUP();
                return 1;
            }
            fclose(fp);
        }
    }

    RootXformEntry rtChain[MAX_RT_CHAIN];
    int nRt = 0;
    if (rtPath) {
        nRt = parse_root_xform_file(rtPath, rtChain, MAX_RT_CHAIN);
        if (nRt == 0) {
            fprintf(stderr, "Failed to parse root transforms from %s\n", rtPath);
            free(buf);
            REMOTE_CLEANUP();
            return 1;
        }
    }

    threads = clamp_threads(threads, stepCount);
    if (strcmp(inputMode, "sectioned") == 0) {
        CURLcode curlRc = curl_global_init(CURL_GLOBAL_ALL);
        if (curlRc != CURLE_OK) {
            fprintf(stderr, "curl_global_init failed: %s\n", curl_easy_strerror(curlRc));
            free(buf);
            REMOTE_CLEANUP();
            return 1;
        }
        remoteCurlInitialized = 1;
    }

    size_t outputValueCount = (size_t)stepCount * (size_t)outputChannelCount;
    float *scoresOut = malloc(outputValueCount * sizeof(float));
    unsigned char *binsOut = malloc(outputValueCount);
    if (!scoresOut || !binsOut) {
        fprintf(stderr, "Out of memory for outputs\n");
        free(scoresOut);
        free(binsOut);
        free(buf);
        REMOTE_CLEANUP();
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
        REMOTE_CLEANUP();
        return 1;
    }

    long base = stepCount / threads;
    long rem = stepCount % threads;
    long solveStart = 0;
    for (int t = 0; t < threads; t++) {
        long solveCount = base + (t < rem ? 1 : 0);
        ChunkWorkerArgs *arg = &args[t];
        arg->sharedBuf = buf;
        arg->sharedCoeffBuf = scoreCoeffRows;
        arg->sharedParamBuf = scoreParamRows;
        arg->inputReader = strcmp(inputMode, "multispan_sectioned") == 0 ? &inputReader : NULL;
        arg->scoreCoeffReader = scoreProgramUsesCoeffSources && strcmp(inputMode, "multispan_sectioned") == 0 ? &scoreCoeffReader : NULL;
        arg->scoreParamReader = scoreProgramUsesParamSources && strcmp(inputMode, "multispan_sectioned") == 0 ? &scoreParamReader : NULL;
        arg->url = url;
        arg->scoreCoeffsUrl = scoreProgramUsesCoeffSources ? scoreCoeffsUrl : NULL;
        arg->inputMode = inputMode;
        arg->degree = degree;
        arg->scoreCoeffDegree = scoreProgramUsesCoeffSources ? scoreCoeffDegree : 0;
        arg->scoreCoeffStride = scoreCoeffStride;
        arg->scoreParamDegree = scoreProgramUsesParamSources ? scoreParamDegree : 0;
        arg->scoreParamStride = scoreParamStride;
        arg->solvePreludeRows = solvePreludeRows;
        arg->scoreCoeffPreludeRows = scoreCoeffPreludeRows;
        arg->scoreParamPreludeRows = scoreParamPreludeRows;
        arg->usesSolveLag = scoreProgramUsesSolveLag;
        arg->usesCoeffLag = scoreProgramUsesCoeffLag;
        arg->usesParamLag = scoreProgramUsesParamLag;
        arg->solveStart = solveStart;
        arg->solveCount = solveCount;
        arg->solveBytes = solveBytes;
        arg->scoreCoeffSolveBytes = scoreCoeffSolveBytes;
        long sourceCurrentStart = solvePreludeRows + solveStart;
        long sourceReadStart = sourceCurrentStart;
        if (scoreProgramUsesSolveLag && sourceCurrentStart > 0) sourceReadStart -= 1;
        long sourceReadEnd = solvePreludeRows + solveStart + solveCount;
        arg->sourceReadStart = sourceReadStart;
        arg->sectionBytes = (size_t)((unsigned long long)(sourceReadEnd - sourceReadStart) * (unsigned long long)solveBytes);
        arg->byteStart = (unsigned long long)sourceReadStart * (unsigned long long)solveBytes;
        arg->byteEnd = arg->byteStart + (unsigned long long)arg->sectionBytes - 1ULL;
        if (scoreProgramUsesCoeffSources) {
            long coeffCurrentStart = scoreCoeffPreludeRows + solveStart;
            long coeffReadStart = coeffCurrentStart;
            if (scoreProgramUsesCoeffLag && coeffCurrentStart > 0) coeffReadStart -= 1;
            long coeffReadEnd = scoreCoeffPreludeRows + solveStart + solveCount;
            arg->scoreCoeffReadStart = coeffReadStart;
            arg->scoreCoeffSectionBytes = (size_t)((unsigned long long)(coeffReadEnd - coeffReadStart) * (unsigned long long)scoreCoeffSolveBytes);
            arg->scoreCoeffByteStart = (unsigned long long)coeffReadStart * (unsigned long long)scoreCoeffSolveBytes;
            arg->scoreCoeffByteEnd = arg->scoreCoeffByteStart + (unsigned long long)arg->scoreCoeffSectionBytes - 1ULL;
        }
        if (scoreProgramUsesParamSources) {
            long paramCurrentStart = scoreParamPreludeRows + solveStart;
            long paramReadStart = paramCurrentStart;
            if (scoreProgramUsesParamLag && paramCurrentStart > 0) paramReadStart -= 1;
            long paramReadEnd = scoreParamPreludeRows + solveStart + solveCount;
            arg->scoreParamReadStart = paramReadStart;
            arg->scoreParamSectionBytes = (size_t)((unsigned long long)(paramReadEnd - paramReadStart) * (unsigned long long)scoreParamStride * (unsigned long long)sizeof(float));
            arg->scoreParamByteStart = (unsigned long long)paramReadStart * (unsigned long long)scoreParamStride * (unsigned long long)sizeof(float);
            arg->scoreParamByteEnd = arg->scoreParamByteStart + (unsigned long long)arg->scoreParamSectionBytes - 1ULL;
        }
        arg->metric = metric;
        arg->program = scoreProgram;
        arg->useProgram = useScoreProgram;
        arg->rtChain = rtChain;
        arg->nRt = nRt;
        arg->clipLo = clipLo;
        arg->clipHi = clipHi;
        arg->omega = omega;
        arg->omegaEnabled = omegaEnabled;
        memcpy(arg->cuts, cuts, sizeof(cuts));
        arg->outputChannelCount = outputChannelCount;
        memcpy(arg->scoreOutputClipLos, scoreOutputClipLos, sizeof(scoreOutputClipLos));
        memcpy(arg->scoreOutputClipHis, scoreOutputClipHis, sizeof(scoreOutputClipHis));
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
            REMOTE_CLEANUP();
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
            REMOTE_CLEANUP();
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
        REMOTE_CLEANUP();
        return 1;
    }
    if (fwrite(scoresOut, sizeof(float), outputValueCount, sf) != outputValueCount) {
        fprintf(stderr, "Short write to %s\n", scoresOutPath);
        fclose(sf);
        free(threadIds);
        free(args);
        free(scoresOut);
        free(binsOut);
        free(buf);
        REMOTE_CLEANUP();
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
        REMOTE_CLEANUP();
        return 1;
    }
    if (fwrite(binsOut, 1, outputValueCount, bf) != outputValueCount) {
        fprintf(stderr, "Short write to %s\n", binsOutPath);
        fclose(bf);
        free(threadIds);
        free(args);
        free(scoresOut);
        free(binsOut);
        free(buf);
        REMOTE_CLEANUP();
        return 1;
    }
    fclose(bf);

    double minScore = scoresOut[0];
    double maxScore = scoresOut[0];
    for (size_t i = 1; i < outputValueCount; i++) {
        if (scoresOut[i] < minScore) minScore = scoresOut[i];
        if (scoresOut[i] > maxScore) maxScore = scoresOut[i];
    }

    printf("{\"mode\":\"palette_chunk\",\"metric\":\"%s\",\"n_samples\":%d,"
           "\"output_channel_count\":%d,"
           "\"omega\":%.15g,\"omega_enabled\":%s,\"clip_lo\":%.15g,\"clip_hi\":%.15g,"
           "\"min_score\":%.15g,\"max_score\":%.15g,"
           "\"threads\":%d,\"input_mode\":\"%s\",\"retries\":%d,"
           "\"download_ms\":%ld,\"compute_ms\":%ld,\"bytes_downloaded\":%ld}\n",
           solve_metric_name(metric), stepCount, outputChannelCount, omega, omegaEnabled ? "true" : "false",
           clipLo, clipHi, minScore, maxScore, threads, inputMode, retries,
           totalDownloadMs, totalComputeMs, totalBytesDownloaded);

    free(threadIds);
    free(args);
    free(scoresOut);
    free(binsOut);
    free(scoreCoeffRows);
    free(scoreParamRows);
    free(buf);
    REMOTE_CLEANUP();
    return 0;
}
