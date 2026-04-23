/*
 * roots2pix_mt: multithreaded fused solve-score raster.
 *
 * This is the native raster for the fused Color path. It emits fused
 * raw-score global u32le_u8_v1 fragments only.
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
    int emitPaletteBins;
    long long paletteStepStart;
    int paletteGridN;
    int scoreCoeffDegree;
    int scoreCoeffStride;
    int scoreParamDegree;
    int scoreParamStride;
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
    RootXformEntry *rtChain;
    int nRt;
    uint64_t *pixelBits;
    ByteVec pbxByteVec;
    ByteVec palettePbxByteVec;
    unsigned char *stepScores;
    long stepScoreCount;
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

static int bytevec_push_u32le_u8(ByteVec *vec, uint32_t pixIdx, uint8_t score) {
    if (vec->len + 5 > vec->cap) {
        size_t newCap = vec->cap ? vec->cap * 2 : 4096;
        while (newCap < vec->len + 5) newCap *= 2;
        unsigned char *newData = realloc(vec->data, newCap);
        if (!newData) return 0;
        vec->data = newData;
        vec->cap = newCap;
    }
    vec->data[vec->len + 0] = (unsigned char)(pixIdx & 0xFFu);
    vec->data[vec->len + 1] = (unsigned char)((pixIdx >> 8) & 0xFFu);
    vec->data[vec->len + 2] = (unsigned char)((pixIdx >> 16) & 0xFFu);
    vec->data[vec->len + 3] = (unsigned char)((pixIdx >> 24) & 0xFFu);
    vec->data[vec->len + 4] = score;
    vec->len += 5;
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
        free(args[i].pbxByteVec.data);
        free(args[i].palettePbxByteVec.data);
        free(args[i].stepScores);
    }
}

static void *worker_main(void *arg_) {
    WorkerArgs *arg = (WorkerArgs *)arg_;
    float stepBuf[MAXDEG * 2];
    float wkRe[MAXDEG];
    float wkIm[MAXDEG];
    unsigned char *sectionBuf = NULL;
    unsigned char *coeffSectionBuf = NULL;
    unsigned char *paramSectionBuf = NULL;
    const float *sectionRoots = NULL;
    const float *sectionCoeffRoots = NULL;
    const float *sectionParamRows = NULL;
    long localSolves = arg->end - arg->start;
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

    nativeStartUs = monotonic_us();
    nativeStarted = 1;
    for (long p = arg->start; p < arg->end; p++) {
        long localIdx = p - arg->start;
        if (localIdx < 0 || localIdx >= localSolves) {
            worker_fail(arg, "section local index out of range");
            goto cleanup;
        }
        const float *rawStep = sectionRoots + localIdx * arg->stride;
        const float *step = prepare_step(rawStep, arg->degree, arg->rtChain, arg->nRt, stepBuf, wkRe, wkIm);
        const float *coeffStep = NULL;
        const float *paramStep = NULL;
        if (arg->scoreCoeffDegree > 0) {
            coeffStep = sectionCoeffRoots ? (sectionCoeffRoots + localIdx * arg->scoreCoeffStride) : NULL;
        }
        if (arg->scoreParamDegree > 0) {
            paramStep = sectionParamRows ? (sectionParamRows + localIdx * arg->scoreParamStride) : NULL;
        }

        uint8_t solveBin = 255;

        double u = solve_score_eval_program_with_sources(
            step, arg->degree, coeffStep, arg->scoreCoeffDegree, paramStep, arg->scoreParamDegree,
            &arg->solveScoreProgram
        );
        {
            int rawByte = 1 + (int)llround(u * 254.0);
            if (rawByte < 1) rawByte = 1;
            if (rawByte > 255) rawByte = 255;
            solveBin = (uint8_t)rawByte;
        }

        if (arg->emitPaletteBins) {
            long long globalStep = arg->paletteStepStart + p;
            long long pass0Steps = (long long)arg->paletteGridN * (long long)arg->paletteGridN;
            if (globalStep >= 0 && globalStep < pass0Steps) {
                int row = (int)(globalStep / (long long)arg->paletteGridN);
                int j = (int)(globalStep % (long long)arg->paletteGridN);
                int col = (row & 1) ? (arg->paletteGridN - 1 - j) : j;
                uint32_t palettePixIdx = (uint32_t)row * (uint32_t)arg->paletteGridN + (uint32_t)col;
                if (!bytevec_push_u32le_u8(&arg->palettePbxByteVec, palettePixIdx, solveBin)) {
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
            arg->stepScores[localIdx] = solveBin;
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

            if (!bytevec_push_u32le_u8(&arg->pbxByteVec, globalPixIdx, solveBin)) {
                worker_fail(arg, "fragment vec alloc failed");
                goto cleanup;
            }
            arg->rootsPlotted++;
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
                "[--threads=N] "
                "--score_metrics=csv --score_clip_los=csv --score_clip_his=csv --score_program=spec "
                "[--score_sources=csv] "
                "[--score_coeff_manifest=file.json] [--score_params_manifest=file.json] "
                "[--pixel_bin_prefix=/tmp/pixbin] "
                "[--palette_bin_prefix=/tmp/palette_pixbin] [--palette_grid_n=N] [--palette_step_start=STEP] "
                "[--step_scores_output=/tmp/step_scores.bin] "
                "[--root_xforms=file.json]\n");
        return 1;
    }

    const char *outPrefix = argv[1];
    (void)outPrefix;
    const char *urlArg = getArg(argc, argv, "--url");
    const char *inputSizeArg = getArg(argc, argv, "--input_size");
    const char *inputModeArg = getArg(argc, argv, "--input_mode");
    const char *colorArg = getArg(argc, argv, "--color");
    const char *matchArg = getArg(argc, argv, "--match");
    const char *paletteArg = getArg(argc, argv, "--palette");
    const char *inputManifest = getArgStr(argc, argv, "--input_manifest", NULL);
    const char *widthArg = getArg(argc, argv, "--width");
    const char *heightArg = getArg(argc, argv, "--height");
    int pix = getArgInt(argc, argv, "--pix", 0);
    const char *minReArg = getArg(argc, argv, "--min_re");
    const char *maxReArg = getArg(argc, argv, "--max_re");
    const char *minImArg = getArg(argc, argv, "--min_im");
    const char *maxImArg = getArg(argc, argv, "--max_im");
    const char *centerReArg = getArg(argc, argv, "--center_re");
    const char *centerImArg = getArg(argc, argv, "--center_im");
    const char *scaleArg = getArg(argc, argv, "--scale");
    double rotation = getArgDouble(argc, argv, "--rotation", 0.0);
    double cosA = cos(rotation), sinA = sin(rotation);
    int degree = getArgInt(argc, argv, "--degree", 25);
    const char *tileSizeArg = getArg(argc, argv, "--tile_size");
    const char *nTileColsArg = getArg(argc, argv, "--n_tile_cols");
    const char *nTileRowsArg = getArg(argc, argv, "--n_tile_rows");
    int retries = getArgInt(argc, argv, "--retries", 2);
    int requestedThreads = getArgInt(argc, argv, "--threads", 1);
    const char *pixelBinPrefix = getArgStr(argc, argv, "--pixel_bin_prefix", NULL);
    const char *paletteBinPrefix = getArgStr(argc, argv, "--palette_bin_prefix", NULL);
    const char *stepScoresOutputPath = getArgStr(argc, argv, "--step_scores_output", NULL);
    int paletteGridN = getArgInt(argc, argv, "--palette_grid_n", 0);
    long long paletteStepStart = getArgLongLong(argc, argv, "--palette_step_start", 0);
    const char *rtPath = getArgStr(argc, argv, "--root_xforms", NULL);
    if (inputModeArg) {
        fprintf(stderr, "roots2pix_mt no longer accepts --input_mode; multispan_sectioned is implicit\n");
        return 1;
    }
    if (urlArg || inputSizeArg) {
        fprintf(stderr, "roots2pix_mt no longer supports direct sectioned input; pass --input_manifest\n");
        return 1;
    }
    if (colorArg || matchArg || paletteArg) {
        fprintf(stderr, "roots2pix_mt no longer accepts --color, --match, or --palette; it emits raw-score fragments only\n");
        return 1;
    }
    if (degree < 1 || degree > MAXDEG) {
        fprintf(stderr, "Invalid degree: %d\n", degree);
        return 1;
    }
    if (widthArg || heightArg) {
        fprintf(stderr, "roots2pix_mt no longer accepts --width or --height; pass --pix for square output\n");
        return 1;
    }
    if (tileSizeArg || nTileColsArg || nTileRowsArg) {
        fprintf(stderr, "roots2pix_mt no longer accepts tile args; it writes sparse global fragments\n");
        return 1;
    }
    if (pix < 1) {
        fprintf(stderr, "Invalid pix: %d\n", pix);
        return 1;
    }
    int W = pix;
    int H = pix;
    double minRe = 0.0, maxRe = 0.0, minIm = 0.0, maxIm = 0.0;
    if (centerReArg || centerImArg || scaleArg) {
        fprintf(stderr, "Legacy viewport args are no longer supported; pass --min_re, --max_re, --min_im, and --max_im\n");
        return 1;
    }
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

    const char *solveMetricArg = getArg(argc, argv, "--solve_metric");
    const char *scoreMetricsCsv = getArgStr(argc, argv, "--score_metrics", NULL);
    const char *scoreSourcesCsv = getArgStr(argc, argv, "--score_sources", NULL);
    const char *scoreClipLosCsv = getArgStr(argc, argv, "--score_clip_los", NULL);
    const char *scoreClipHisCsv = getArgStr(argc, argv, "--score_clip_his", NULL);
    const char *scoreProgramSpec = getArgStr(argc, argv, "--score_program", NULL);
    const char *scoreCoeffsFileArg = getArg(argc, argv, "--score_coeffs_file");
    const char *scoreCoeffsUrlArg = getArg(argc, argv, "--score_coeffs_url");
    const char *scoreCoeffManifest = getArgStr(argc, argv, "--score_coeff_manifest", NULL);
    const char *scoreParamsFileArg = getArg(argc, argv, "--score_params_file");
    const char *scoreParamsManifest = getArgStr(argc, argv, "--score_params_manifest", NULL);
    const char *scoreCoeffInputSizeArg = getArg(argc, argv, "--score_coeff_input_size");
    int scoreCoeffDegree = getArgInt(argc, argv, "--score_coeff_degree", 0);
    if (scoreCoeffsFileArg || scoreCoeffsUrlArg || scoreCoeffInputSizeArg || scoreParamsFileArg) {
        fprintf(stderr, "roots2pix_mt no longer supports direct coeff/param inputs; pass manifest-backed sources\n");
        return 1;
    }
    if (
        solveMetricArg ||
        getArg(argc, argv, "--solve_score_clip_lo") ||
        getArg(argc, argv, "--solve_score_clip_hi") ||
        getArg(argc, argv, "--solve_score_omega") ||
        getArg(argc, argv, "--solve_score_omega_enabled")
    ) {
        fprintf(stderr, "roots2pix_mt no longer accepts legacy single-metric solve-score args; pass score program args instead\n");
        return 1;
    }
    SolveScoreProgram solveScoreProgram;
    if (
        getArg(argc, argv, "--solve_score_cuts") ||
        getArg(argc, argv, "--solve_prox_cuts") ||
        getArg(argc, argv, "--solve_score_raw_bytes") ||
        getArg(argc, argv, "--skip_pix_output") ||
        getArg(argc, argv, "--solve_prox_clip_lo") ||
        getArg(argc, argv, "--solve_prox_clip_hi")
    ) {
        fprintf(stderr, "Legacy raster output args are no longer supported; roots2pix_mt emits fused raw-score fragments only\n");
        return 1;
    }
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
    nPoints = (long)(inputReader.logicalSize / (unsigned long long)solveBytes);
    if (nPoints <= 0) {
        fprintf(stderr, "Empty multispan input\n");
        multispan_reader_close(&inputReader);
        return 1;
    }

    for (int i = 0; i < solveScoreProgram.metricCount; i++) {
        if (solveScoreProgram.metricSources[i] == SOLVE_SCORE_SOURCE_COEFF) {
            scoreProgramUsesCoeffSources = 1;
        }
        if (solveScoreProgram.metricSources[i] == SOLVE_SCORE_SOURCE_PARAM) {
            scoreProgramUsesParamSources = 1;
        }
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
        if (coeffPoints != nPoints) {
            fprintf(stderr, "multispan score coeff solve count mismatch: got %ld expected %ld\n", coeffPoints, nPoints);
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
        if (paramPoints != nPoints) {
            fprintf(stderr, "multispan score params size mismatch: got %ld solves expected %ld\n", paramPoints, nPoints);
            multispan_reader_close(&scoreParamReader);
            multispan_reader_close(&scoreCoeffReader);
            multispan_reader_close(&inputReader);
            return 1;
        }
    }

    uint64_t *pixelBits = NULL;
    int emitPixelBins = pixelBinPrefix && *pixelBinPrefix;
    int emitPaletteBins = paletteBinPrefix && *paletteBinPrefix;
    int emitStepScores = stepScoresOutputPath && *stepScoresOutputPath;
    if (!emitPixelBins) {
        fprintf(stderr, "roots2pix_mt requires --pixel_bin_prefix for fused fragment output\n");
        return 1;
    }
    if (emitPaletteBins) {
        if (paletteGridN < 1) {
            fprintf(stderr, "--palette_grid_n must be >= 1 when --palette_bin_prefix is set\n");
            return 1;
        }
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
        args[i].sectionBytes = (size_t)width * (size_t)solveBytes;
        args[i].scoreCoeffSectionBytes = (size_t)width * (size_t)scoreCoeffSolveBytes;
        args[i].scoreParamSectionBytes = (size_t)width * (size_t)args[i].scoreParamSolveBytes;
        args[i].byteStart = (unsigned long long)start * (unsigned long long)solveBytes;
        args[i].scoreCoeffByteStart = (unsigned long long)start * (unsigned long long)scoreCoeffSolveBytes;
        args[i].scoreParamByteStart = (unsigned long long)start * (unsigned long long)args[i].scoreParamSolveBytes;
        args[i].rtChain = rtChain;
        args[i].nRt = nRt;
        args[i].inputReader = &inputReader;
        args[i].scoreCoeffReader = scoreProgramUsesCoeffSources ? &scoreCoeffReader : NULL;
        args[i].scoreParamReader = scoreProgramUsesParamSources ? &scoreParamReader : NULL;
        args[i].pixelBits = pixelBits;
        args[i].stepScores = emitStepScores ? calloc((size_t)(width > 0 ? width : 1), sizeof(unsigned char)) : NULL;
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
    size_t totalPbxBytes = 0;
    for (int i = 0; i < threads; i++) {
        totalPbxBytes += args[i].pbxByteVec.len;
    }
    if (totalPbxBytes > 0) fragmentsWithData = 1;
    totalEntries = (long)(totalPbxBytes / 5u);
    if (emitPixelBins) {
        if (totalPbxBytes > 0) {
            if (!write_suffix_path(pathBuf, sizeof(pathBuf), pixelBinPrefix, ".frag")) {
                fprintf(stderr, "Cannot build fused fragment path from %s\n", pixelBinPrefix);
                goto cleanup;
            }
            FILE *fb = fopen(pathBuf, "wb");
            if (!fb) {
                fprintf(stderr, "Cannot create %s\n", pathBuf);
                goto cleanup;
            }
            for (int i = 0; i < threads; i++) {
                if (args[i].pbxByteVec.len > 0) {
                    fwrite(args[i].pbxByteVec.data, 1, args[i].pbxByteVec.len, fb);
                }
            }
            fclose(fb);
        }
    }
    if (emitPaletteBins) {
        size_t totalPaletteBytes = 0;
        for (int i = 0; i < threads; i++) {
            totalPaletteBytes += args[i].palettePbxByteVec.len;
        }
        if (totalPaletteBytes > 0) {
            if (!write_suffix_path(pathBuf, sizeof(pathBuf), paletteBinPrefix, ".frag")) {
                fprintf(stderr, "Cannot build palette fused fragment path from %s\n", paletteBinPrefix);
                goto cleanup;
            }
            FILE *fb = fopen(pathBuf, "wb");
            if (!fb) {
                fprintf(stderr, "Cannot create %s\n", pathBuf);
                goto cleanup;
            }
            for (int i = 0; i < threads; i++) {
                if (args[i].palettePbxByteVec.len > 0) {
                    fwrite(args[i].palettePbxByteVec.data, 1, args[i].palettePbxByteVec.len, fb);
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
                fwrite(args[i].stepScores, 1, (size_t)args[i].stepScoreCount, fs);
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
           "\"input_mode\":\"multispan_sectioned\",\"retries\":%d,\"download_us\":%ld,\"native_us\":%ld",
           rootsPlotted, rootsClipped, nPoints, degree, threads,
           fragmentsWithData, totalEntries, retries, totalDownloadUs, totalNativeUs);
    printf(",\"solve_score\":true");
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
