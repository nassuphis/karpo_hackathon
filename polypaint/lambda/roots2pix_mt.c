/*
 * roots2pix_mt: multithreaded fused solve-score raster.
 *
 * This is the native raster for the fused Color path. It emits fused
 * raw-score global fragments. Legacy scalar records are u32le_u8_v1; explicit
 * multi-output programs use u32le_pixel_idx_plus_u8_channels_v1.
 *
 * Supported color mode:
 *   - solve_score
 *
 * Build (multispan mode needs libcurl at runtime):
 *   gcc -O3 -pthread -o roots2pix_mt roots2pix_mt.c -lcurl -lm -Wl,-rpath,'$ORIGIN/lib'
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

#define MAXDEG 256
typedef struct {
    unsigned char *data;
    size_t len;
    size_t cap;
} ByteVec;

typedef struct {
    int id;
    long start;
    long end;
    int degree;
    int stride;
    int W;
    int H;
    double minRe;
    double maxRe;
    double minIm;
    double maxIm;
    double centerRe;
    double centerIm;
    double xScale;
    double yScale;
    double cosA;
    double sinA;
    SolveScoreProgram solveScoreProgram;
    int scoreOutputNormalize;
    double scoreOutputClipLo;
    double scoreOutputClipHi;
    int outputChannelCount;
    double scoreOutputClipLos[SOLVE_SCORE_MAX_OUTPUT_CHANNELS];
    double scoreOutputClipHis[SOLVE_SCORE_MAX_OUTPUT_CHANNELS];
    int emitPaletteBins;
    long long paletteStepStart;
    int paletteGridN;
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
    MultiSpanReader *inputReader;
    MultiSpanReader *scoreCoeffReader;
    MultiSpanReader *scoreParamReader;
    long solveBytes;
    long scoreCoeffSolveBytes;
    long scoreParamSolveBytes;
    size_t sectionBytes;
    size_t scoreCoeffSectionBytes;
    size_t scoreParamSectionBytes;
    unsigned long long byteStart;
    unsigned long long scoreCoeffByteStart;
    unsigned long long scoreParamByteStart;
    long sourceReadStart;
    long scoreCoeffReadStart;
    long scoreParamReadStart;
    RootXformEntry *rtChain;
    int nRt;
    uint64_t *pixelBits;
    ByteVec fragmentByteVec;
    ByteVec paletteFragmentByteVec;
    unsigned char *stepScores;
    long stepScoreCount;
    int stepScoreChannels;
    long rootsPlotted;
    long rootsClipped;
    long rootsDeduped;
    long downloadUs;
    long nativeUs;
    int retries;
    int error;
    char error_msg[256];
} WorkerArgs;

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

static int option_matches(const char *arg, const char *key) {
    int klen = (int)strlen(key);
    return strncmp(arg, key, klen) == 0 && arg[klen] == '=';
}

static int reject_unknown_options(int argc, char **argv, int firstFlagIndex, const char **allowed, int nAllowed) {
    for (int i = firstFlagIndex; i < argc; i++) {
        if (argv[i][0] != '-') continue;
        int known = 0;
        for (int j = 0; j < nAllowed; j++) {
            if (option_matches(argv[i], allowed[j])) {
                known = 1;
                break;
            }
        }
        if (!known) {
            fprintf(stderr, "Unknown roots2pix_mt option: %s\n", argv[i]);
            return 1;
        }
    }
    return 0;
}

static int clamp_threads(int requested, long n_items) {
    int threads = requested < 1 ? 1 : requested;
    if (n_items > 0 && threads > (int)n_items) threads = (int)n_items;
    if (threads < 1) threads = 1;
    return threads;
}

static long long monotonic_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000LL + (long long)(ts.tv_nsec / 1000LL);
}

static int bytevec_push_u32le_channels(ByteVec *vec, uint32_t pixIdx, const uint8_t *channels, int channelCount) {
    if (!channels || channelCount < 1 || channelCount > SOLVE_SCORE_MAX_OUTPUT_CHANNELS) return 0;
    size_t recordSize = 4u + (size_t)channelCount;
    if (vec->len + recordSize > vec->cap) {
        size_t newCap = vec->cap ? vec->cap * 2 : 4096;
        while (newCap < vec->len + recordSize) newCap *= 2;
        unsigned char *newData = realloc(vec->data, newCap);
        if (!newData) return 0;
        vec->data = newData;
        vec->cap = newCap;
    }
    vec->data[vec->len + 0] = (unsigned char)(pixIdx & 0xFFu);
    vec->data[vec->len + 1] = (unsigned char)((pixIdx >> 8) & 0xFFu);
    vec->data[vec->len + 2] = (unsigned char)((pixIdx >> 16) & 0xFFu);
    vec->data[vec->len + 3] = (unsigned char)((pixIdx >> 24) & 0xFFu);
    for (int i = 0; i < channelCount; i++) {
        vec->data[vec->len + 4u + (size_t)i] = channels[i];
    }
    vec->len += recordSize;
    return 1;
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

static int claim_pixel(uint64_t *pixelWords, uint32_t pix_idx) {
    size_t wordIdx = (size_t)(pix_idx >> 6);
    uint64_t bit = 1ULL << (pix_idx & 63);
    uint64_t old = __atomic_fetch_or(&pixelWords[wordIdx], bit, __ATOMIC_RELAXED);
    return (old & bit) == 0;
}

static void worker_fail(WorkerArgs *arg, const char *msg) {
    arg->error = 1;
    strncpy(arg->error_msg, msg, sizeof(arg->error_msg) - 1);
    arg->error_msg[sizeof(arg->error_msg) - 1] = '\0';
}

static int write_suffix_path(char *dst, size_t dstSize, const char *prefix, const char *suffix) {
    if (!dst || dstSize == 0 || !prefix || !suffix) return 0;
    int written = snprintf(dst, dstSize, "%s%s", prefix, suffix);
    return written > 0 && (size_t)written < dstSize;
}

static void free_worker_storage(WorkerArgs *args, int nWorkers) {
    if (!args) return;
    for (int i = 0; i < nWorkers; i++) {
        free(args[i].fragmentByteVec.data);
        free(args[i].paletteFragmentByteVec.data);
        free(args[i].stepScores);
    }
}

static void *worker_main(void *arg_) {
    WorkerArgs *arg = (WorkerArgs *)arg_;
    float stepBuf[MAXDEG * 2];
    float prevStepBuf[MAXDEG * 2];
    float wkRe[MAXDEG];
    float wkIm[MAXDEG];
    float prevWkRe[MAXDEG];
    float prevWkIm[MAXDEG];
    float currentMetricBuffer[SOLVE_SCORE_MAX_METRIC_SLOTS];
    float recentMetricBuffer[SOLVE_SCORE_MAX_METRIC_SLOTS];
    double outputValues[SOLVE_SCORE_MAX_OUTPUT_CHANNELS];
    uint8_t outputBytes[SOLVE_SCORE_MAX_OUTPUT_CHANNELS];
    unsigned char *sectionBuf = NULL;
    unsigned char *coeffSectionBuf = NULL;
    unsigned char *paramSectionBuf = NULL;
    const float *sectionRoots = NULL;
    const float *sectionCoeffRoots = NULL;
    const float *sectionParamRows = NULL;
    long localSolves = arg->end - arg->start;
    long sourceRows = 0;
    long coeffRows = 0;
    long paramRows = 0;
    int usesLag = solve_score_program_uses_lag(&arg->solveScoreProgram);
    int recentInitialized = 0;
    long long nativeStartUs = 0;
    int nativeStarted = 0;

    {
        long inputBytesDownloaded = 0;
        long coeffBytesDownloaded = 0;
        long paramBytesDownloaded = 0;
        if (arg->sectionBytes > 0) {
            sectionBuf = malloc(arg->sectionBytes > 0 ? arg->sectionBytes : 1);
            if (!sectionBuf) {
                worker_fail(arg, "multispan section buffer alloc failed");
                multispan_reader_thread_cleanup();
                goto cleanup;
            }
            if (!arg->inputReader) {
                worker_fail(arg, "multispan input requires inputReader");
                multispan_reader_thread_cleanup();
                goto cleanup;
            }
            long long dlStartUs = monotonic_us();
            if (!multispan_reader_read_exact(
                    arg->inputReader,
                    arg->byteStart,
                    arg->sectionBytes,
                    sectionBuf,
                    &inputBytesDownloaded,
                    arg->error_msg,
                    sizeof(arg->error_msg))) {
                arg->error = 1;
                multispan_reader_thread_cleanup();
                goto cleanup;
            }
            if (arg->scoreCoeffReader && arg->scoreCoeffSectionBytes > 0) {
                coeffSectionBuf = malloc(arg->scoreCoeffSectionBytes > 0 ? arg->scoreCoeffSectionBytes : 1);
                if (!coeffSectionBuf) {
                    worker_fail(arg, "multispan coeff section buffer alloc failed");
                    multispan_reader_thread_cleanup();
                    goto cleanup;
                }
                if (!multispan_reader_read_exact(
                        arg->scoreCoeffReader,
                        arg->scoreCoeffByteStart,
                        arg->scoreCoeffSectionBytes,
                        coeffSectionBuf,
                        &coeffBytesDownloaded,
                        arg->error_msg,
                        sizeof(arg->error_msg))) {
                    arg->error = 1;
                    multispan_reader_thread_cleanup();
                    goto cleanup;
                }
                sectionCoeffRoots = (const float *)(const void *)coeffSectionBuf;
            }
            if (arg->scoreParamReader && arg->scoreParamSectionBytes > 0) {
                paramSectionBuf = malloc(arg->scoreParamSectionBytes > 0 ? arg->scoreParamSectionBytes : 1);
                if (!paramSectionBuf) {
                    worker_fail(arg, "multispan param section buffer alloc failed");
                    multispan_reader_thread_cleanup();
                    goto cleanup;
                }
                if (!multispan_reader_read_exact(
                        arg->scoreParamReader,
                        arg->scoreParamByteStart,
                        arg->scoreParamSectionBytes,
                        paramSectionBuf,
                        &paramBytesDownloaded,
                        arg->error_msg,
                        sizeof(arg->error_msg))) {
                    arg->error = 1;
                    multispan_reader_thread_cleanup();
                    goto cleanup;
                }
                sectionParamRows = (const float *)(const void *)paramSectionBuf;
            }
            arg->downloadUs = (long)(monotonic_us() - dlStartUs);
            arg->downloadUs += 0;
            sectionRoots = (const float *)(const void *)sectionBuf;
        }
    }
    sourceRows = arg->solveBytes > 0 ? (long)(arg->sectionBytes / (size_t)arg->solveBytes) : 0;
    coeffRows = arg->scoreCoeffSolveBytes > 0 ? (long)(arg->scoreCoeffSectionBytes / (size_t)arg->scoreCoeffSolveBytes) : 0;
    paramRows = arg->scoreParamSolveBytes > 0 ? (long)(arg->scoreParamSectionBytes / (size_t)arg->scoreParamSolveBytes) : 0;

    nativeStartUs = monotonic_us();
    nativeStarted = 1;
    for (long p = arg->start; p < arg->end; p++) {
        long localIdx = p - arg->start;
        long sourceLocalIdx = arg->solvePreludeRows + p - arg->sourceReadStart;
        if (localIdx < 0 || localIdx >= localSolves || sourceLocalIdx < 0 || sourceLocalIdx >= sourceRows) {
            worker_fail(arg, "section local index out of range");
            goto cleanup;
        }
        const float *rawStep = sectionRoots + sourceLocalIdx * arg->stride;
        const float *step = prepare_step(rawStep, arg->degree, arg->rtChain, arg->nRt, stepBuf, wkRe, wkIm);
        const float *coeffStep = NULL;
        const float *paramStep = NULL;
        if (arg->scoreCoeffDegree > 0) {
            long coeffLocalIdx = arg->scoreCoeffPreludeRows + p - arg->scoreCoeffReadStart;
            if (coeffLocalIdx < 0 || coeffLocalIdx >= coeffRows) {
                worker_fail(arg, "coeff section local index out of range");
                goto cleanup;
            }
            coeffStep = sectionCoeffRoots ? (sectionCoeffRoots + coeffLocalIdx * arg->scoreCoeffStride) : NULL;
        }
        if (arg->scoreParamDegree > 0) {
            long paramLocalIdx = arg->scoreParamPreludeRows + p - arg->scoreParamReadStart;
            if (paramLocalIdx < 0 || paramLocalIdx >= paramRows) {
                worker_fail(arg, "param section local index out of range");
                goto cleanup;
            }
            paramStep = sectionParamRows ? (sectionParamRows + paramLocalIdx * arg->scoreParamStride) : NULL;
        }

        uint8_t solveBin = 255;

        if (!solve_score_eval_metric_slots(
                step, arg->degree, coeffStep, arg->scoreCoeffDegree, paramStep, arg->scoreParamDegree,
                &arg->solveScoreProgram, currentMetricBuffer)) {
            worker_fail(arg, "solve-score metric evaluation failed");
            goto cleanup;
        }
        if (usesLag && !recentInitialized) {
            memcpy(recentMetricBuffer, currentMetricBuffer, sizeof(float) * (size_t)arg->solveScoreProgram.metricCount);
            const float *prevStep = step;
            const float *prevCoeffStep = coeffStep;
            const float *prevParamStep = paramStep;
            long prevSourceLocalIdx = sourceLocalIdx - 1;
            long prevCoeffLocalIdx = arg->scoreCoeffPreludeRows + p - arg->scoreCoeffReadStart - 1;
            long prevParamLocalIdx = arg->scoreParamPreludeRows + p - arg->scoreParamReadStart - 1;
            if (arg->usesSolveLag && prevSourceLocalIdx >= 0) {
                const float *prevRawStep = sectionRoots + prevSourceLocalIdx * arg->stride;
                prevStep = prepare_step(prevRawStep, arg->degree, arg->rtChain, arg->nRt, prevStepBuf, prevWkRe, prevWkIm);
            }
            if (arg->usesCoeffLag && sectionCoeffRoots && prevCoeffLocalIdx >= 0 && prevCoeffLocalIdx < coeffRows) {
                prevCoeffStep = sectionCoeffRoots + prevCoeffLocalIdx * arg->scoreCoeffStride;
            }
            if (arg->usesParamLag && sectionParamRows && prevParamLocalIdx >= 0 && prevParamLocalIdx < paramRows) {
                prevParamStep = sectionParamRows + prevParamLocalIdx * arg->scoreParamStride;
            }
            if (!solve_score_eval_lagged_metric_slots(
                    prevStep, arg->degree,
                    prevCoeffStep, arg->scoreCoeffDegree,
                    prevParamStep, arg->scoreParamDegree,
                    &arg->solveScoreProgram, recentMetricBuffer)) {
                worker_fail(arg, "solve-score lag metric evaluation failed");
                goto cleanup;
            }
            recentInitialized = 1;
        }
        int gotOutputs = 0;
        if (!solve_score_eval_program_outputs_from_buffers(
                currentMetricBuffer,
                usesLag ? recentMetricBuffer : NULL,
                &arg->solveScoreProgram,
                outputValues,
                SOLVE_SCORE_MAX_OUTPUT_CHANNELS,
                &gotOutputs) ||
            gotOutputs != arg->outputChannelCount) {
            worker_fail(arg, "solve-score program evaluation failed");
            goto cleanup;
        }
        if (solve_score_program_has_explicit_outputs(&arg->solveScoreProgram)) {
            for (int ch = 0; ch < arg->outputChannelCount; ch++) {
                double u = outputValues[ch];
                if (solve_score_program_output_is_normalized(&arg->solveScoreProgram, ch)) {
                    double range = arg->scoreOutputClipHis[ch] - arg->scoreOutputClipLos[ch];
                    if (isfinite(range) && range > 1e-12) {
                        u = (u - arg->scoreOutputClipLos[ch]) / range;
                    }
                }
                u = solve_score_clamp_unit(u);
                int rawByte = (int)llround(u * 255.0);
                if (rawByte < 0) rawByte = 0;
                if (rawByte > 255) rawByte = 255;
                outputBytes[ch] = (uint8_t)rawByte;
            }
            solveBin = outputBytes[0];
        } else {
            double u = outputValues[0];
            if (arg->scoreOutputNormalize) {
                double range = arg->scoreOutputClipHi - arg->scoreOutputClipLo;
                if (isfinite(range) && range > 1e-12) {
                    u = solve_score_clamp_unit((u - arg->scoreOutputClipLo) / range);
                }
            }
            u = solve_score_clamp_unit(u);
            int rawByte = 1 + (int)llround(u * 254.0);
            if (rawByte < 1) rawByte = 1;
            if (rawByte > 255) rawByte = 255;
            solveBin = (uint8_t)rawByte;
            outputBytes[0] = solveBin;
        }

        if (arg->emitPaletteBins) {
            long long globalStep = arg->paletteStepStart + p;
            long long pass0Steps = (long long)arg->paletteGridN * (long long)arg->paletteGridN;
            if (globalStep >= 0 && globalStep < pass0Steps) {
                int row = (int)(globalStep / (long long)arg->paletteGridN);
                int j = (int)(globalStep % (long long)arg->paletteGridN);
                int col = (row & 1) ? (arg->paletteGridN - 1 - j) : j;
                uint32_t palettePixIdx = (uint32_t)row * (uint32_t)arg->paletteGridN + (uint32_t)col;
                if (!bytevec_push_u32le_channels(
                        &arg->paletteFragmentByteVec,
                        palettePixIdx,
                        outputBytes,
                        arg->outputChannelCount)) {
                    worker_fail(arg, "palette fragment vec alloc failed");
                    goto cleanup;
                }
            }
        }
        if (arg->stepScores) {
            long localIdx = p - arg->start;
            if (localIdx < 0 || localIdx >= arg->stepScoreCount) {
                worker_fail(arg, "step score local index out of range");
                goto cleanup;
            }
            size_t base = (size_t)localIdx * (size_t)arg->stepScoreChannels;
            for (int ch = 0; ch < arg->stepScoreChannels; ch++) {
                arg->stepScores[base + (size_t)ch] = outputBytes[ch];
            }
        }

        for (int r = 0; r < arg->degree; r++) {
            double re = step[r * 2];
            double im = step[r * 2 + 1];
            double dx = re - arg->centerRe;
            double dy = im - arg->centerIm;
            double rotRe = arg->centerRe + (dx * arg->cosA - dy * arg->sinA);
            double rotIm = arg->centerIm + (dx * arg->sinA + dy * arg->cosA);
            double pxf = (rotRe - arg->minRe) * arg->xScale;
            double pyf = (arg->maxIm - rotIm) * arg->yScale;
            if (!isfinite(pxf) || !isfinite(pyf)) {
                arg->rootsClipped++;
                continue;
            }
            int px = (int)floor(pxf);
            int py = (int)floor(pyf);
            if (px < 0 || px >= arg->W || py < 0 || py >= arg->H) {
                arg->rootsClipped++;
                continue;
            }

            uint32_t globalPixIdx = (uint32_t)py * (uint32_t)arg->W + (uint32_t)px;
            if (!claim_pixel(arg->pixelBits, globalPixIdx)) {
                arg->rootsDeduped++;
                continue;
            }

            if (!bytevec_push_u32le_channels(
                    &arg->fragmentByteVec,
                    globalPixIdx,
                    outputBytes,
                    arg->outputChannelCount)) {
                worker_fail(arg, "fragment vec alloc failed");
                goto cleanup;
            }
            arg->rootsPlotted++;
        }
        if (usesLag) {
            memcpy(recentMetricBuffer, currentMetricBuffer, sizeof(float) * (size_t)arg->solveScoreProgram.metricCount);
        }
    }
cleanup:
    if (nativeStarted) arg->nativeUs = (long)(monotonic_us() - nativeStartUs);
    multispan_reader_thread_cleanup();
    free(paramSectionBuf);
    free(coeffSectionBuf);
    free(sectionBuf);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: roots2pix_mt /tmp/pix "
                "--pix=N --min_re=A --max_re=B --min_im=C --max_im=D "
                "--degree=D "
                "--input_manifest=file.json [--retries=N] "
                "--step_count=N [--prelude_rows=0|1] [--score_coeff_prelude_rows=0|1] [--score_param_prelude_rows=0|1] "
                "[--threads=N] "
                "--score_metrics=csv --score_clip_los=csv --score_clip_his=csv --score_program=spec "
                "[--score_sources=csv] "
                "[--score_output_normalize=0|1 --score_output_clip_lo=X --score_output_clip_hi=Y] "
                "[--score_output_clip_los=csv --score_output_clip_his=csv] "
                "[--score_coeff_manifest=file.json] [--score_params_manifest=file.json] "
                "--fragment_prefix=/tmp/fused_fragment "
                "[--associated_palette_fragment_prefix=/tmp/palette_fragment] [--palette_grid_n=N] [--palette_step_start=STEP] "
                "[--step_scores_output=/tmp/step_scores.bin] "
                "[--root_xforms=file.json]\n");
        return 1;
    }
    const char *allowedOptions[] = {
        "--input_manifest", "--pix",
        "--min_re", "--max_re", "--min_im", "--max_im",
        "--rotation", "--degree", "--retries", "--threads", "--step_count",
        "--prelude_rows", "--score_coeff_prelude_rows", "--score_param_prelude_rows",
        "--fragment_prefix", "--associated_palette_fragment_prefix", "--step_scores_output",
        "--palette_grid_n", "--palette_step_start", "--root_xforms",
        "--score_metrics", "--score_sources",
        "--score_clip_los", "--score_clip_his", "--score_program",
        "--score_output_normalize", "--score_output_clip_lo", "--score_output_clip_hi",
        "--score_output_clip_los", "--score_output_clip_his",
        "--score_coeff_manifest",
        "--score_params_manifest",
        "--score_coeff_degree",
    };
    if (reject_unknown_options(argc, argv, 2, allowedOptions, (int)(sizeof(allowedOptions) / sizeof(allowedOptions[0])))) {
        return 1;
    }

    const char *outPrefix = argv[1];
    (void)outPrefix;
    const char *inputManifest = getArgStr(argc, argv, "--input_manifest", NULL);
    int pix = getArgInt(argc, argv, "--pix", 0);
    const char *minReArg = getArg(argc, argv, "--min_re");
    const char *maxReArg = getArg(argc, argv, "--max_re");
    const char *minImArg = getArg(argc, argv, "--min_im");
    const char *maxImArg = getArg(argc, argv, "--max_im");
    double rotation = getArgDouble(argc, argv, "--rotation", 0.0);
    double cosA = cos(rotation), sinA = sin(rotation);
    int degree = getArgInt(argc, argv, "--degree", 25);
    int retries = getArgInt(argc, argv, "--retries", 2);
    int requestedThreads = getArgInt(argc, argv, "--threads", 1);
    long requestedStepCount = getArgLongLong(argc, argv, "--step_count", 0);
    int solvePreludeRows = getArgInt(argc, argv, "--prelude_rows", 0);
    int scoreCoeffPreludeRows = getArgInt(argc, argv, "--score_coeff_prelude_rows", 0);
    int scoreParamPreludeRows = getArgInt(argc, argv, "--score_param_prelude_rows", 0);
    const char *fragmentPrefix = getArgStr(argc, argv, "--fragment_prefix", NULL);
    const char *paletteFragmentPrefix = getArgStr(argc, argv, "--associated_palette_fragment_prefix", NULL);
    const char *stepScoresOutputPath = getArgStr(argc, argv, "--step_scores_output", NULL);
    int paletteGridN = getArgInt(argc, argv, "--palette_grid_n", 0);
    long long paletteStepStart = getArgLongLong(argc, argv, "--palette_step_start", 0);
    const char *rtPath = getArgStr(argc, argv, "--root_xforms", NULL);
    if (degree < 1 || degree > MAXDEG) {
        fprintf(stderr, "Invalid degree: %d\n", degree);
        return 1;
    }
    if (pix < 1) {
        fprintf(stderr, "Invalid pix: %d\n", pix);
        return 1;
    }
    if (requestedStepCount < 1) {
        fprintf(stderr, "roots2pix_mt requires --step_count >= 1\n");
        return 1;
    }
    if (solvePreludeRows < 0 || solvePreludeRows > 1 ||
        scoreCoeffPreludeRows < 0 || scoreCoeffPreludeRows > 1 ||
        scoreParamPreludeRows < 0 || scoreParamPreludeRows > 1) {
        fprintf(stderr, "prelude row counts must be 0 or 1 in v1\n");
        return 1;
    }
    int W = pix;
    int H = pix;
    double minRe = 0.0, maxRe = 0.0, minIm = 0.0, maxIm = 0.0;
    if (minReArg || maxReArg || minImArg || maxImArg) {
        if (!minReArg || !maxReArg || !minImArg || !maxImArg) {
            fprintf(stderr, "Exact viewport requires --min_re, --max_re, --min_im, and --max_im together\n");
            return 1;
        }
        minRe = atof(minReArg);
        maxRe = atof(maxReArg);
        minIm = atof(minImArg);
        maxIm = atof(maxImArg);
        if (!(maxRe > minRe) || !(maxIm > minIm)) {
            fprintf(stderr, "Invalid exact viewport bounds\n");
            return 1;
        }
    } else {
        fprintf(stderr, "Exact viewport requires --min_re, --max_re, --min_im, and --max_im\n");
        return 1;
    }
    double centerRe = (minRe + maxRe) / 2.0;
    double centerIm = (minIm + maxIm) / 2.0;
    double xScale = (double)W / (maxRe - minRe);
    double yScale = (double)H / (maxIm - minIm);
    if (retries < 0 || retries > 10) {
        fprintf(stderr, "Invalid retries: %d\n", retries);
        return 1;
    }

    RootXformEntry rtChain[MAX_RT_CHAIN];
    int nRt = 0;
    if (rtPath) {
        nRt = parse_root_xform_file(rtPath, rtChain, MAX_RT_CHAIN);
        if (nRt == 0) {
            fprintf(stderr, "Failed to parse root transforms from %s\n", rtPath);
            return 1;
        }
    }

    const char *scoreMetricsCsv = getArgStr(argc, argv, "--score_metrics", NULL);
    const char *scoreSourcesCsv = getArgStr(argc, argv, "--score_sources", NULL);
    const char *scoreClipLosCsv = getArgStr(argc, argv, "--score_clip_los", NULL);
    const char *scoreClipHisCsv = getArgStr(argc, argv, "--score_clip_his", NULL);
    const char *scoreProgramSpec = getArgStr(argc, argv, "--score_program", NULL);
    const char *scoreCoeffManifest = getArgStr(argc, argv, "--score_coeff_manifest", NULL);
    const char *scoreParamsManifest = getArgStr(argc, argv, "--score_params_manifest", NULL);
    int scoreCoeffDegree = getArgInt(argc, argv, "--score_coeff_degree", 0);
    int scoreOutputNormalize = getArgInt(argc, argv, "--score_output_normalize", 0);
    double scoreOutputClipLo = getArgDouble(argc, argv, "--score_output_clip_lo", 0.0);
    double scoreOutputClipHi = getArgDouble(argc, argv, "--score_output_clip_hi", 1.0);
    const char *scoreOutputClipLosCsv = getArgStr(argc, argv, "--score_output_clip_los", NULL);
    const char *scoreOutputClipHisCsv = getArgStr(argc, argv, "--score_output_clip_his", NULL);
    if (scoreOutputNormalize && (!isfinite(scoreOutputClipLo) || !isfinite(scoreOutputClipHi))) {
        fprintf(stderr, "score output clip bounds must be finite when normalization is enabled\n");
        return 1;
    }
    if (scoreOutputNormalize && scoreOutputClipHi - scoreOutputClipLo <= 1e-12) {
        fprintf(stderr, "solve_score_output_normalize: degenerate range [%g,%g], using identity\n",
                scoreOutputClipLo, scoreOutputClipHi);
        scoreOutputNormalize = 0;
    }
    SolveScoreProgram solveScoreProgram;
    double scoreOutputClipLos[SOLVE_SCORE_MAX_OUTPUT_CHANNELS];
    double scoreOutputClipHis[SOLVE_SCORE_MAX_OUTPUT_CHANNELS];
    {
        char scoreErr[256] = {0};
        if (!scoreMetricsCsv || !scoreClipLosCsv || !scoreClipHisCsv || !scoreProgramSpec) {
            fprintf(stderr, "roots2pix_mt requires --score_metrics, --score_clip_los, --score_clip_his, and --score_program\n");
            return 1;
        }
        if (!parse_solve_score_program_args_ex(
                scoreMetricsCsv, scoreSourcesCsv, scoreClipLosCsv, scoreClipHisCsv, scoreProgramSpec,
                &solveScoreProgram, scoreErr, sizeof(scoreErr))) {
            fprintf(stderr, "Invalid solve_score program: %s\n", scoreErr[0] ? scoreErr : "unknown error");
            return 1;
        }
        if (solveScoreProgram.metricCount < 1 || solveScoreProgram.tokenCount < 1) {
            fprintf(stderr, "Invalid solve_score program: missing metric slots or program tokens\n");
            return 1;
        }
        for (int i = 0; i < SOLVE_SCORE_MAX_OUTPUT_CHANNELS; i++) {
            scoreOutputClipLos[i] = scoreOutputClipLo;
            scoreOutputClipHis[i] = scoreOutputClipHi;
        }
        if (scoreOutputClipLosCsv || scoreOutputClipHisCsv) {
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
            if (loCount != solveScoreProgram.outputCount || hiCount != solveScoreProgram.outputCount) {
                fprintf(stderr, "score output clip array length mismatch: expected %d, got lows=%d highs=%d\n",
                        solveScoreProgram.outputCount, loCount, hiCount);
                return 1;
            }
        }
        for (int ch = 0; ch < solveScoreProgram.outputCount; ch++) {
            if (solve_score_program_output_is_normalized(&solveScoreProgram, ch)) {
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

    if (scoreMetricsCsv || scoreClipLosCsv || scoreClipHisCsv || scoreProgramSpec) {
        /* required-and-validated above */
    } else {
        return 1;
    }

    int stride = degree * 2;
    long solveBytes = stride * (long)sizeof(float);
    long nPoints = 0;
    MultiSpanReader inputReader = {0};
    MultiSpanReader scoreCoeffReader = {0};
    MultiSpanReader scoreParamReader = {0};
    char manifestErr[256] = {0};
    int scoreProgramUsesCoeffSources = 0;
    int scoreProgramUsesParamSources = 0;
    int scoreProgramUsesLag = 0;
    int scoreProgramUsesSolveLag = 0;
    int scoreProgramUsesCoeffLag = 0;
    int scoreProgramUsesParamLag = 0;
    int scoreCoeffStride = scoreCoeffDegree * 2;
    long scoreCoeffSolveBytes = scoreCoeffStride * (long)sizeof(float);

    if (!inputManifest || !*inputManifest) {
        fprintf(stderr, "multispan_sectioned input requires --input_manifest\n");
        return 1;
    }
    if (!multispan_reader_open(&inputReader, inputManifest, retries, manifestErr, sizeof(manifestErr))) {
        fprintf(stderr, "failed to open input manifest: %s\n", manifestErr[0] ? manifestErr : "unknown error");
        return 1;
    }
    if ((inputReader.logicalSize % (unsigned long long)solveBytes) != 0ULL) {
        fprintf(stderr, "input manifest logical_size=%llu is not aligned to solve_bytes=%ld\n",
                inputReader.logicalSize, solveBytes);
        multispan_reader_close(&inputReader);
        return 1;
    }
    long inputRows = (long)(inputReader.logicalSize / (unsigned long long)solveBytes);
    if (inputRows <= 0) {
        fprintf(stderr, "Empty multispan input\n");
        multispan_reader_close(&inputReader);
        return 1;
    }
    nPoints = requestedStepCount;

    for (int i = 0; i < solveScoreProgram.metricCount; i++) {
        if (solveScoreProgram.metricSources[i] == SOLVE_SCORE_SOURCE_COEFF) {
            scoreProgramUsesCoeffSources = 1;
        }
        if (solveScoreProgram.metricSources[i] == SOLVE_SCORE_SOURCE_PARAM) {
            scoreProgramUsesParamSources = 1;
        }
    }
    scoreProgramUsesLag = solve_score_program_uses_lag(&solveScoreProgram);
    scoreProgramUsesSolveLag = solve_score_program_uses_lag_source(&solveScoreProgram, SOLVE_SCORE_SOURCE_SOLVE);
    scoreProgramUsesCoeffLag = solve_score_program_uses_lag_source(&solveScoreProgram, SOLVE_SCORE_SOURCE_COEFF);
    scoreProgramUsesParamLag = solve_score_program_uses_lag_source(&solveScoreProgram, SOLVE_SCORE_SOURCE_PARAM);
    if (!scoreProgramUsesLag && (solvePreludeRows || scoreCoeffPreludeRows || scoreParamPreludeRows)) {
        fprintf(stderr, "prelude rows require a lagged solve-score program\n");
        multispan_reader_close(&inputReader);
        return 1;
    }
    if (!scoreProgramUsesCoeffSources && scoreCoeffPreludeRows) {
        fprintf(stderr, "coeff prelude rows require coeff-source metrics\n");
        multispan_reader_close(&inputReader);
        return 1;
    }
    if (!scoreProgramUsesParamSources && scoreParamPreludeRows) {
        fprintf(stderr, "param prelude rows require param-source metrics\n");
        multispan_reader_close(&inputReader);
        return 1;
    }
    if (scoreProgramUsesSolveLag != (solvePreludeRows > 0) && !(scoreProgramUsesSolveLag && solvePreludeRows == 0)) {
        fprintf(stderr, "solve prelude rows must match solve-source lag requirements\n");
        multispan_reader_close(&inputReader);
        return 1;
    }
    if (inputRows != nPoints + solvePreludeRows) {
        fprintf(stderr, "input manifest solve count mismatch: got %ld rows expected %ld scored + %d prelude\n",
                inputRows, nPoints, solvePreludeRows);
        multispan_reader_close(&inputReader);
        return 1;
    }
    if (scoreProgramUsesCoeffSources) {
        if (scoreCoeffDegree < 1 || scoreCoeffDegree > MAXDEG) {
            fprintf(stderr, "Invalid score_coeff_degree: %d (must be 1-%d)\n", scoreCoeffDegree, MAXDEG);
            multispan_reader_close(&inputReader);
            return 1;
        }
        if (!scoreCoeffManifest || !*scoreCoeffManifest) {
            fprintf(stderr, "multispan_sectioned solve_score program with coeff sources requires --score_coeff_manifest\n");
            multispan_reader_close(&inputReader);
            return 1;
        }
        if (!multispan_reader_open(&scoreCoeffReader, scoreCoeffManifest, retries, manifestErr, sizeof(manifestErr))) {
            fprintf(stderr, "failed to open coeff manifest: %s\n", manifestErr[0] ? manifestErr : "unknown error");
            multispan_reader_close(&inputReader);
            return 1;
        }
        if ((scoreCoeffReader.logicalSize % (unsigned long long)scoreCoeffSolveBytes) != 0ULL) {
            fprintf(stderr, "coeff manifest logical_size=%llu is not aligned to solve_bytes=%ld\n",
                    scoreCoeffReader.logicalSize, scoreCoeffSolveBytes);
            multispan_reader_close(&scoreCoeffReader);
            multispan_reader_close(&inputReader);
            return 1;
        }
        long coeffPoints = (long)(scoreCoeffReader.logicalSize / (unsigned long long)scoreCoeffSolveBytes);
        if (scoreProgramUsesCoeffLag != (scoreCoeffPreludeRows > 0) && !(scoreProgramUsesCoeffLag && scoreCoeffPreludeRows == 0)) {
            fprintf(stderr, "coeff prelude rows must match coeff-source lag requirements\n");
            multispan_reader_close(&scoreCoeffReader);
            multispan_reader_close(&inputReader);
            return 1;
        }
        if (coeffPoints != nPoints + scoreCoeffPreludeRows) {
            fprintf(stderr, "multispan score coeff solve count mismatch: got %ld expected %ld scored + %d prelude\n",
                    coeffPoints, nPoints, scoreCoeffPreludeRows);
            multispan_reader_close(&scoreCoeffReader);
            multispan_reader_close(&inputReader);
            return 1;
        }
    }
    int scoreParamStride = 4;
    int scoreParamDegree = 2;
    if (scoreProgramUsesParamSources) {
        long paramSolveBytes = (long)scoreParamStride * (long)sizeof(float);
        if (!scoreParamsManifest || !*scoreParamsManifest) {
            fprintf(stderr, "multispan_sectioned solve_score program with param sources requires --score_params_manifest\n");
            multispan_reader_close(&scoreCoeffReader);
            multispan_reader_close(&inputReader);
            return 1;
        }
        if (!multispan_reader_open(&scoreParamReader, scoreParamsManifest, retries, manifestErr, sizeof(manifestErr))) {
            fprintf(stderr, "failed to open param manifest: %s\n", manifestErr[0] ? manifestErr : "unknown error");
            multispan_reader_close(&scoreCoeffReader);
            multispan_reader_close(&inputReader);
            return 1;
        }
        if ((scoreParamReader.logicalSize % (unsigned long long)paramSolveBytes) != 0ULL) {
            fprintf(stderr, "param manifest logical_size=%llu is not aligned to solve_bytes=%ld\n",
                    scoreParamReader.logicalSize, paramSolveBytes);
            multispan_reader_close(&scoreParamReader);
            multispan_reader_close(&scoreCoeffReader);
            multispan_reader_close(&inputReader);
            return 1;
        }
        long paramPoints = (long)(scoreParamReader.logicalSize / (unsigned long long)paramSolveBytes);
        if (scoreProgramUsesParamLag != (scoreParamPreludeRows > 0) && !(scoreProgramUsesParamLag && scoreParamPreludeRows == 0)) {
            fprintf(stderr, "param prelude rows must match param-source lag requirements\n");
            multispan_reader_close(&scoreParamReader);
            multispan_reader_close(&scoreCoeffReader);
            multispan_reader_close(&inputReader);
            return 1;
        }
        if (paramPoints != nPoints + scoreParamPreludeRows) {
            fprintf(stderr, "multispan score params size mismatch: got %ld solves expected %ld scored + %d prelude\n",
                    paramPoints, nPoints, scoreParamPreludeRows);
            multispan_reader_close(&scoreParamReader);
            multispan_reader_close(&scoreCoeffReader);
            multispan_reader_close(&inputReader);
            return 1;
        }
    }

    uint64_t *pixelBits = NULL;
    int emitFragments = fragmentPrefix && *fragmentPrefix;
    int emitPaletteBins = paletteFragmentPrefix && *paletteFragmentPrefix;
    int emitStepScores = stepScoresOutputPath && *stepScoresOutputPath;
    if (!emitFragments) {
        fprintf(stderr, "roots2pix_mt requires --fragment_prefix for fused fragment output\n");
        return 1;
    }
    if (emitPaletteBins) {
        if (paletteGridN < 1) {
            fprintf(stderr, "--palette_grid_n must be >= 1 when --associated_palette_fragment_prefix is set\n");
            return 1;
        }
    }
    if (emitStepScores && solveScoreProgram.outputCount != 1 && solveScoreProgram.outputCount != 3) {
        fprintf(stderr, "step_scores_output requires one or three solve-score output channels\n");
        return 1;
    }
    int threads = clamp_threads(requestedThreads, nPoints);
    WorkerArgs *args = NULL;
    pthread_t *workers = NULL;
    int workersPrepared = 0;
    int workersStarted = 0;
    int workersJoined = 0;
    int curlInitialized = 0;
    int exitCode = 1;
    long rootsPlotted = 0;
    long rootsClipped = 0;
    long rootsDeduped = 0;
    long totalDownloadUs = 0;
    long totalNativeUs = 0;
    char workerErrorMsg[256] = {0};
    uint64_t totalPixels = (uint64_t)(uint32_t)W * (uint64_t)(uint32_t)H;
    if (totalPixels == 0 || totalPixels > UINT32_MAX) {
        fprintf(stderr, "pix is too large for u32 pixel indexes: %d\n", pix);
        goto cleanup;
    }
    size_t pixelWordCount = (size_t)((totalPixels + 63u) / 64u);
    pixelBits = calloc(pixelWordCount, sizeof(uint64_t));
    if (!pixelBits) {
        fprintf(stderr, "Cannot allocate pixel bitset\n");
        goto cleanup;
    }

    {
        CURLcode curlRc = curl_global_init(CURL_GLOBAL_DEFAULT);
        if (curlRc != CURLE_OK) {
            fprintf(stderr, "curl_global_init failed: %s\n", curl_easy_strerror(curlRc));
            goto cleanup;
        }
        curlInitialized = 1;
    }

    args = calloc((size_t)threads, sizeof(WorkerArgs));
    workers = calloc((size_t)threads, sizeof(pthread_t));
    if (!args || !workers) {
        fprintf(stderr, "Out of memory for raster threads\n");
        goto cleanup;
    }

    long base = nPoints / threads;
    long extra = nPoints % threads;
    long start = 0;
    for (int i = 0; i < threads; i++) {
        long width = base + (i < extra ? 1 : 0);
        args[i].id = i;
        args[i].start = start;
        args[i].end = start + width;
        args[i].degree = degree;
        args[i].stride = stride;
        args[i].W = W;
        args[i].H = H;
        args[i].minRe = minRe;
        args[i].maxRe = maxRe;
        args[i].minIm = minIm;
        args[i].maxIm = maxIm;
        args[i].centerRe = centerRe;
        args[i].centerIm = centerIm;
        args[i].xScale = xScale;
        args[i].yScale = yScale;
        args[i].cosA = cosA;
        args[i].sinA = sinA;
        args[i].solveScoreProgram = solveScoreProgram;
        args[i].scoreOutputNormalize = scoreOutputNormalize;
        args[i].scoreOutputClipLo = scoreOutputClipLo;
        args[i].scoreOutputClipHi = scoreOutputClipHi;
        args[i].outputChannelCount = solveScoreProgram.outputCount;
        for (int ch = 0; ch < SOLVE_SCORE_MAX_OUTPUT_CHANNELS; ch++) {
            args[i].scoreOutputClipLos[ch] = scoreOutputClipLos[ch];
            args[i].scoreOutputClipHis[ch] = scoreOutputClipHis[ch];
        }
        args[i].emitPaletteBins = emitPaletteBins;
        args[i].paletteStepStart = paletteStepStart;
        args[i].paletteGridN = paletteGridN;
        args[i].scoreCoeffDegree = scoreProgramUsesCoeffSources ? scoreCoeffDegree : 0;
        args[i].scoreCoeffStride = scoreCoeffStride;
        args[i].scoreParamDegree = scoreProgramUsesParamSources ? scoreParamDegree : 0;
        args[i].scoreParamStride = scoreParamStride;
        args[i].retries = retries;
        args[i].solveBytes = solveBytes;
        args[i].scoreCoeffSolveBytes = scoreCoeffSolveBytes;
        args[i].scoreParamSolveBytes = (long)scoreParamStride * (long)sizeof(float);
        args[i].solvePreludeRows = solvePreludeRows;
        args[i].scoreCoeffPreludeRows = scoreCoeffPreludeRows;
        args[i].scoreParamPreludeRows = scoreParamPreludeRows;
        args[i].usesSolveLag = scoreProgramUsesSolveLag;
        args[i].usesCoeffLag = scoreProgramUsesCoeffLag;
        args[i].usesParamLag = scoreProgramUsesParamLag;
        long sourceCurrentStart = solvePreludeRows + start;
        long sourceReadStart = sourceCurrentStart;
        if (scoreProgramUsesSolveLag && sourceCurrentStart > 0) sourceReadStart -= 1;
        long sourceReadEnd = solvePreludeRows + start + width;
        args[i].sourceReadStart = sourceReadStart;
        args[i].sectionBytes = (size_t)(sourceReadEnd - sourceReadStart) * (size_t)solveBytes;
        args[i].byteStart = (unsigned long long)sourceReadStart * (unsigned long long)solveBytes;
        if (scoreProgramUsesCoeffSources) {
            long coeffCurrentStart = scoreCoeffPreludeRows + start;
            long coeffReadStart = coeffCurrentStart;
            if (scoreProgramUsesCoeffLag && coeffCurrentStart > 0) coeffReadStart -= 1;
            long coeffReadEnd = scoreCoeffPreludeRows + start + width;
            args[i].scoreCoeffReadStart = coeffReadStart;
            args[i].scoreCoeffSectionBytes = (size_t)(coeffReadEnd - coeffReadStart) * (size_t)scoreCoeffSolveBytes;
            args[i].scoreCoeffByteStart = (unsigned long long)coeffReadStart * (unsigned long long)scoreCoeffSolveBytes;
        }
        if (scoreProgramUsesParamSources) {
            long paramCurrentStart = scoreParamPreludeRows + start;
            long paramReadStart = paramCurrentStart;
            if (scoreProgramUsesParamLag && paramCurrentStart > 0) paramReadStart -= 1;
            long paramReadEnd = scoreParamPreludeRows + start + width;
            args[i].scoreParamReadStart = paramReadStart;
            args[i].scoreParamSectionBytes = (size_t)(paramReadEnd - paramReadStart) * (size_t)args[i].scoreParamSolveBytes;
            args[i].scoreParamByteStart = (unsigned long long)paramReadStart * (unsigned long long)args[i].scoreParamSolveBytes;
        }
        args[i].rtChain = rtChain;
        args[i].nRt = nRt;
        args[i].inputReader = &inputReader;
        args[i].scoreCoeffReader = scoreProgramUsesCoeffSources ? &scoreCoeffReader : NULL;
        args[i].scoreParamReader = scoreProgramUsesParamSources ? &scoreParamReader : NULL;
        args[i].pixelBits = pixelBits;
        args[i].stepScoreChannels = solveScoreProgram.outputCount;
        args[i].stepScores = emitStepScores
            ? calloc((size_t)(width > 0 ? width : 1) * (size_t)args[i].stepScoreChannels, sizeof(unsigned char))
            : NULL;
        args[i].stepScoreCount = width;
        if (emitStepScores && !args[i].stepScores) {
            fprintf(stderr, "Out of memory for worker vectors\n");
            goto cleanup;
        }
        workersPrepared++;
        start += width;
    }

    for (int i = 0; i < threads; i++) {
        if (pthread_create(&workers[i], NULL, worker_main, &args[i]) != 0) {
            fprintf(stderr, "pthread_create failed for worker %d\n", i);
            goto cleanup;
        }
        workersStarted++;
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(workers[i], NULL);
        rootsPlotted += args[i].rootsPlotted;
        rootsClipped += args[i].rootsClipped;
        rootsDeduped += args[i].rootsDeduped;
        totalDownloadUs += args[i].downloadUs;
        totalNativeUs += args[i].nativeUs;
        if (args[i].error && !workerErrorMsg[0]) {
            strncpy(workerErrorMsg, args[i].error_msg, sizeof(workerErrorMsg) - 1);
            workerErrorMsg[sizeof(workerErrorMsg) - 1] = '\0';
        }
    }
    workersJoined = 1;
    if (workerErrorMsg[0]) {
        fprintf(stderr, "roots2pix_mt worker failed: %s\n", workerErrorMsg);
        goto cleanup;
    }

    char pathBuf[512];
    long totalEntries = 0;
    int fragmentsWithData = 0;
    size_t totalFragmentBytes = 0;
    size_t fragmentRecordSize = 4u + (size_t)solveScoreProgram.outputCount;
    for (int i = 0; i < threads; i++) {
        totalFragmentBytes += args[i].fragmentByteVec.len;
    }
    if (totalFragmentBytes > 0) fragmentsWithData = 1;
    totalEntries = (long)(fragmentRecordSize > 0 ? totalFragmentBytes / fragmentRecordSize : 0u);
    if (emitFragments) {
        if (totalFragmentBytes > 0) {
            if (!write_suffix_path(pathBuf, sizeof(pathBuf), fragmentPrefix, ".frag")) {
                fprintf(stderr, "Cannot build fused fragment path from %s\n", fragmentPrefix);
                goto cleanup;
            }
            FILE *fb = fopen(pathBuf, "wb");
            if (!fb) {
                fprintf(stderr, "Cannot create %s\n", pathBuf);
                goto cleanup;
            }
            for (int i = 0; i < threads; i++) {
                if (args[i].fragmentByteVec.len > 0) {
                    fwrite(args[i].fragmentByteVec.data, 1, args[i].fragmentByteVec.len, fb);
                }
            }
            fclose(fb);
        }
    }
    if (emitPaletteBins) {
        size_t totalPaletteBytes = 0;
        for (int i = 0; i < threads; i++) {
            totalPaletteBytes += args[i].paletteFragmentByteVec.len;
        }
        if (totalPaletteBytes > 0) {
            if (!write_suffix_path(pathBuf, sizeof(pathBuf), paletteFragmentPrefix, ".frag")) {
                fprintf(stderr, "Cannot build palette fused fragment path from %s\n", paletteFragmentPrefix);
                goto cleanup;
            }
            FILE *fb = fopen(pathBuf, "wb");
            if (!fb) {
                fprintf(stderr, "Cannot create %s\n", pathBuf);
                goto cleanup;
            }
            for (int i = 0; i < threads; i++) {
                if (args[i].paletteFragmentByteVec.len > 0) {
                    fwrite(args[i].paletteFragmentByteVec.data, 1, args[i].paletteFragmentByteVec.len, fb);
                }
            }
            fclose(fb);
        }
    }
    if (emitStepScores) {
        FILE *fs = fopen(stepScoresOutputPath, "wb");
        if (!fs) {
            fprintf(stderr, "Cannot create %s\n", stepScoresOutputPath);
            goto cleanup;
        }
        for (int i = 0; i < threads; i++) {
            if (args[i].stepScoreCount > 0) {
                fwrite(
                    args[i].stepScores,
                    1,
                    (size_t)args[i].stepScoreCount * (size_t)args[i].stepScoreChannels,
                    fs
                );
            }
        }
        fclose(fs);
    }

    if (rootsDeduped > 0) {
        fprintf(stderr, "dedup: %ld unique, %ld skipped\n", rootsPlotted, rootsDeduped);
    }

    printf("{\"roots_plotted\":%ld,\"roots_clipped\":%ld,\"n_points\":%ld,"
           "\"degree\":%d,\"threads\":%d,"
           "\"fragments_with_data\":%d,\"total_entries\":%ld,"
           "\"fragment_channels\":%d,\"fragment_record_size_bytes\":%zu,"
           "\"input_mode\":\"multispan_sectioned\",\"retries\":%d,\"download_us\":%ld,\"native_us\":%ld",
           rootsPlotted, rootsClipped, nPoints, degree, threads,
           fragmentsWithData, totalEntries,
           solveScoreProgram.outputCount, fragmentRecordSize,
           retries, totalDownloadUs, totalNativeUs);
    printf(",\"solve_score\":true");
    if (emitStepScores) {
        printf(",\"step_score_channels\":%d", solveScoreProgram.outputCount);
    }
    printf("}\n");
    exitCode = 0;

cleanup:
    if (workers && !workersJoined) {
        for (int i = 0; i < workersStarted; i++) pthread_join(workers[i], NULL);
    }
    free_worker_storage(args, workersPrepared);
    free(pixelBits);
    free(workers);
    free(args);
    multispan_reader_close(&scoreParamReader);
    multispan_reader_close(&scoreCoeffReader);
    multispan_reader_close(&inputReader);
    if (curlInitialized) curl_global_cleanup();
    return exitCode;
}
