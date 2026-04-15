/*
 * roots2pix_mt: multithreaded color raster for supported Color modes.
 *
 * This is the native MT replacement for the earlier subprocess fan-out path.
 * It keeps the same external .pix / .pbx contracts used by finalize and fast
 * Color RePalette, but computes them in one process with pthread workers.
 *
 * Supported modes:
 *   - solve_score
 *   - saved_palette
 *   - constant
 *   - rainbow with match=none
 *
 * Unsupported here:
 *   - proximity
 *   - rainbow with greedy / hungarian
 *
 * Build (sectioned mode needs libcurl at runtime):
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

#include "palette_lut.h"
#include "root_xforms.h"
#include "solve_score.h"

#define MAXDEG 256
#define MAX_TILES 4096

enum ColorMode {
    COLOR_RAINBOW = 0,
    COLOR_CONSTANT = 1,
    COLOR_SOLVE_SCORE = 2,
    COLOR_SAVED_PALETTE = 3,
};

enum InputMode {
    INPUT_TMPFILE = 0,
    INPUT_SECTIONED = 1,
};

typedef struct {
    uint32_t *data;
    size_t len;
    size_t cap;
} U32Vec;

typedef struct {
    unsigned char *data;
    size_t expected;
    size_t size;
    int overflow;
} DownloadBuffer;

typedef struct {
    int id;
    long start;
    long end;
    int degree;
    int stride;
    int W;
    int H;
    int tileSize;
    int nTileCols;
    int nTileRows;
    int nTiles;
    double centerRe;
    double centerIm;
    double scale;
    double cosA;
    double sinA;
    double halfW;
    double halfH;
    enum ColorMode colorMode;
    enum InputMode inputMode;
    enum SolveMetric solveMetric;
    SolveScoreProgram solveScoreProgram;
    int useScoreProgram;
    double solveScoreClipLo;
    double solveScoreClipHi;
    double solveScoreOmega;
    int solveScoreOmegaEnabled;
    double solveScoreCuts[9];
    int nSolveScoreCuts;
    uint32_t constRGB;
    int emitPixelBins;
    int skipPixOutput;
    const float *roots;
    const float *scoreCoeffRows;
    const float *scoreParamRows;
    int scoreCoeffDegree;
    int scoreCoeffStride;
    int scoreParamDegree;
    int scoreParamStride;
    const char *url;
    const char *scoreCoeffsUrl;
    long solveBytes;
    long scoreCoeffSolveBytes;
    size_t sectionBytes;
    size_t scoreCoeffSectionBytes;
    unsigned long long byteStart;
    unsigned long long byteEnd;
    unsigned long long scoreCoeffByteStart;
    unsigned long long scoreCoeffByteEnd;
    const uint8_t *solveBins;
    RootXformEntry *rtChain;
    int nRt;
    uint64_t **tileBits;
    int *tileW;
    U32Vec *pixVecs;
    U32Vec *pbxVecs;
    unsigned char rbPalR[MAXDEG];
    unsigned char rbPalG[MAXDEG];
    unsigned char rbPalB[MAXDEG];
    unsigned char ssPalR[10];
    unsigned char ssPalG[10];
    unsigned char ssPalB[10];
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

static void sleep_ms(long ms) {
    if (ms <= 0) return;
    struct timespec ts;
    ts.tv_sec = ms / 1000L;
    ts.tv_nsec = (ms % 1000L) * 1000000L;
    nanosleep(&ts, NULL);
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

static int download_section(const char *url, unsigned long long byteStart, unsigned long long byteEnd,
                            unsigned char *out, size_t expected, int retries, char *errBuf, size_t errBufLen) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        snprintf(errBuf, errBufLen, "curl_easy_init failed");
        return 0;
    }

    DownloadBuffer dl = {
        .data = out,
        .expected = expected,
        .size = 0,
        .overflow = 0,
    };
    char rangeBuf[96];
    char curlErr[CURL_ERROR_SIZE] = {0};
    snprintf(rangeBuf, sizeof(rangeBuf), "%llu-%llu",
             (unsigned long long)byteStart, (unsigned long long)byteEnd);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_RANGE, rangeBuf);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_section_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dl);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErr);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "identity");
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

    int attempts = retries + 1;
    CURLcode rc = CURLE_OK;
    long httpStatus = 0;
    for (int attempt = 0; attempt < attempts; attempt++) {
        dl.size = 0;
        dl.overflow = 0;
        curlErr[0] = '\0';
        httpStatus = 0;
        rc = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatus);
        if (rc == CURLE_OK &&
            (httpStatus == 206L || httpStatus == 200L) &&
            !dl.overflow &&
            dl.size == expected) {
            break;
        }
        if (attempt + 1 >= attempts || !retryable_range_failure(rc, httpStatus)) {
            break;
        }
        sleep_ms(150L * (attempt + 1));
    }
    curl_easy_cleanup(curl);

    if (rc != CURLE_OK) {
        snprintf(errBuf, errBufLen, "range GET failed for bytes %s after %d attempt%s: %s",
                 rangeBuf, attempts, attempts == 1 ? "" : "s",
                 curlErr[0] ? curlErr : curl_easy_strerror(rc));
        return 0;
    }
    if (httpStatus != 206L && httpStatus != 200L) {
        snprintf(errBuf, errBufLen, "unexpected HTTP status %ld for bytes %s after %d attempt%s",
                 httpStatus, rangeBuf, attempts, attempts == 1 ? "" : "s");
        return 0;
    }
    if (dl.overflow) {
        snprintf(errBuf, errBufLen, "range GET overflow for bytes %s after %d attempt%s",
                 rangeBuf, attempts, attempts == 1 ? "" : "s");
        return 0;
    }
    if (dl.size != expected) {
        snprintf(errBuf, errBufLen, "short range GET for bytes %s after %d attempt%s: got %zu of %zu bytes",
                 rangeBuf, attempts, attempts == 1 ? "" : "s", dl.size, expected);
        return 0;
    }
    return 1;
}

static void rainbowRGB(int index, int total,
                       unsigned char *r, unsigned char *g, unsigned char *b) {
    double hue = (double)index / (total > 0 ? total : 1);
    double h6 = hue * 6.0;
    int hi = (int)h6;
    double f = h6 - hi;
    double q = 1.0 - f;
    switch (hi % 6) {
        case 0: *r = 255; *g = (unsigned char)(f * 255); *b = 0; break;
        case 1: *r = (unsigned char)(q * 255); *g = 255; *b = 0; break;
        case 2: *r = 0; *g = 255; *b = (unsigned char)(f * 255); break;
        case 3: *r = 0; *g = (unsigned char)(q * 255); *b = 255; break;
        case 4: *r = (unsigned char)(f * 255); *g = 0; *b = 255; break;
        case 5: *r = 255; *g = 0; *b = (unsigned char)(q * 255); break;
    }
}

static int vec_push2(U32Vec *vec, uint32_t a, uint32_t b) {
    if (vec->len + 2 > vec->cap) {
        size_t newCap = vec->cap ? vec->cap * 2 : 2048;
        while (newCap < vec->len + 2) newCap *= 2;
        uint32_t *newData = realloc(vec->data, newCap * sizeof(uint32_t));
        if (!newData) return 0;
        vec->data = newData;
        vec->cap = newCap;
    }
    vec->data[vec->len++] = a;
    vec->data[vec->len++] = b;
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

static int claim_pixel(uint64_t *tileWords, uint32_t pix_idx) {
    size_t wordIdx = (size_t)(pix_idx >> 6);
    uint64_t bit = 1ULL << (pix_idx & 63);
    uint64_t old = __atomic_fetch_or(&tileWords[wordIdx], bit, __ATOMIC_RELAXED);
    return (old & bit) == 0;
}

static void worker_fail(WorkerArgs *arg, const char *msg) {
    arg->error = 1;
    strncpy(arg->error_msg, msg, sizeof(arg->error_msg) - 1);
    arg->error_msg[sizeof(arg->error_msg) - 1] = '\0';
}

static void free_worker_storage(WorkerArgs *args, int nWorkers, int nTiles) {
    if (!args) return;
    for (int i = 0; i < nWorkers; i++) {
        if (args[i].pixVecs) {
            for (int t = 0; t < nTiles; t++) free(args[i].pixVecs[t].data);
        }
        if (args[i].pbxVecs) {
            for (int t = 0; t < nTiles; t++) free(args[i].pbxVecs[t].data);
        }
        free(args[i].pixVecs);
        free(args[i].pbxVecs);
    }
}

static void *worker_main(void *arg_) {
    WorkerArgs *arg = (WorkerArgs *)arg_;
    float stepBuf[MAXDEG * 2];
    float wkRe[MAXDEG];
    float wkIm[MAXDEG];
    unsigned char *sectionBuf = NULL;
    unsigned char *coeffSectionBuf = NULL;
    const float *sectionRoots = NULL;
    const float *sectionCoeffRoots = NULL;
    long localSolves = arg->end - arg->start;
    long long nativeStartUs = 0;
    int nativeStarted = 0;

    if (arg->inputMode == INPUT_SECTIONED) {
        if (arg->sectionBytes > 0) {
            sectionBuf = malloc(arg->sectionBytes);
            if (!sectionBuf) {
                worker_fail(arg, "section buffer alloc failed");
                goto cleanup;
            }
            long long dlStartUs = monotonic_us();
            if (!download_section(arg->url, arg->byteStart, arg->byteEnd,
                                  sectionBuf, arg->sectionBytes,
                                  arg->retries,
                                  arg->error_msg, sizeof(arg->error_msg))) {
                arg->error = 1;
                goto cleanup;
            }
            if (arg->scoreCoeffsUrl && arg->scoreCoeffSectionBytes > 0) {
                coeffSectionBuf = malloc(arg->scoreCoeffSectionBytes);
                if (!coeffSectionBuf) {
                    worker_fail(arg, "score coeff section buffer alloc failed");
                    goto cleanup;
                }
                if (!download_section(arg->scoreCoeffsUrl, arg->scoreCoeffByteStart, arg->scoreCoeffByteEnd,
                                      coeffSectionBuf, arg->scoreCoeffSectionBytes,
                                      arg->retries,
                                      arg->error_msg, sizeof(arg->error_msg))) {
                    arg->error = 1;
                    goto cleanup;
                }
                sectionCoeffRoots = (const float *)(const void *)coeffSectionBuf;
            }
            arg->downloadUs = (long)(monotonic_us() - dlStartUs);
            sectionRoots = (const float *)(const void *)sectionBuf;
        }
    }

    nativeStartUs = monotonic_us();
    nativeStarted = 1;
    for (long p = arg->start; p < arg->end; p++) {
        const float *rawStep = NULL;
        if (arg->inputMode == INPUT_SECTIONED) {
            long localIdx = p - arg->start;
            if (localIdx < 0 || localIdx >= localSolves) {
                worker_fail(arg, "section local index out of range");
                goto cleanup;
            }
            rawStep = sectionRoots + localIdx * arg->stride;
        } else {
            rawStep = arg->roots + p * arg->stride;
        }
        const float *step = prepare_step(rawStep, arg->degree, arg->rtChain, arg->nRt, stepBuf, wkRe, wkIm);
        const float *coeffStep = NULL;
        const float *paramStep = NULL;
        if (arg->scoreCoeffDegree > 0) {
            if (arg->inputMode == INPUT_SECTIONED) {
                long localIdx = p - arg->start;
                coeffStep = sectionCoeffRoots ? (sectionCoeffRoots + localIdx * arg->scoreCoeffStride) : NULL;
            } else {
                coeffStep = arg->scoreCoeffRows ? (arg->scoreCoeffRows + p * arg->scoreCoeffStride) : NULL;
            }
        }
        if (arg->scoreParamDegree > 0) {
            if (arg->inputMode == INPUT_SECTIONED) {
                long localIdx = p - arg->start;
                paramStep = arg->scoreParamRows ? (arg->scoreParamRows + localIdx * arg->scoreParamStride) : NULL;
            } else {
                paramStep = arg->scoreParamRows ? (arg->scoreParamRows + p * arg->scoreParamStride) : NULL;
            }
        }

        uint32_t solveRGB = 0;
        uint8_t solveBin = 255;

        if (arg->colorMode == COLOR_SOLVE_SCORE) {
            double u;
            if (arg->useScoreProgram) {
                u = solve_score_eval_program_with_sources(
                    step, arg->degree, coeffStep, arg->scoreCoeffDegree, paramStep, arg->scoreParamDegree,
                    &arg->solveScoreProgram
                );
            } else {
                double score = compute_solve_metric_score(step, arg->degree, arg->solveMetric);
                double ssRange = arg->solveScoreClipHi - arg->solveScoreClipLo;
                u = (score - arg->solveScoreClipLo) / ssRange;
                if (u < 0) u = 0;
                if (u > 1) u = 1;
                u = apply_solve_score_transfer(u, arg->solveScoreOmegaEnabled, arg->solveScoreOmega);
            }
            int bin = 9;
            for (int c = 0; c < arg->nSolveScoreCuts; c++) {
                if (u <= arg->solveScoreCuts[c]) { bin = c; break; }
            }
            solveBin = (uint8_t)bin;
            solveRGB = ((uint32_t)arg->ssPalR[bin] << 16) |
                       ((uint32_t)arg->ssPalG[bin] << 8) |
                       arg->ssPalB[bin];
        } else if (arg->colorMode == COLOR_SAVED_PALETTE) {
            uint8_t bin = arg->solveBins[p];
            if (bin > 9) {
                worker_fail(arg, "saved_palette bin out of range");
                goto cleanup;
            }
            solveBin = bin;
            solveRGB = ((uint32_t)arg->ssPalR[bin] << 16) |
                       ((uint32_t)arg->ssPalG[bin] << 8) |
                       arg->ssPalB[bin];
        }

        for (int r = 0; r < arg->degree; r++) {
            double re = step[r * 2];
            double im = step[r * 2 + 1];
            double dx = re - arg->centerRe;
            double dy = im - arg->centerIm;
            double rx = dx * arg->cosA - dy * arg->sinA;
            double ry = dx * arg->sinA + dy * arg->cosA;
            double pxf = arg->halfW + rx * arg->scale;
            double pyf = arg->halfH - ry * arg->scale;
            if (!isfinite(pxf) || !isfinite(pyf)) {
                arg->rootsClipped++;
                continue;
            }
            int px = (int)pxf;
            int py = (int)pyf;
            if (px < 0 || px >= arg->W || py < 0 || py >= arg->H) {
                arg->rootsClipped++;
                continue;
            }

            int tileCol = px / arg->tileSize;
            int tileRow = py / arg->tileSize;
            int tileId = tileRow * arg->nTileCols + tileCol;
            uint32_t localX = (uint32_t)(px - tileCol * arg->tileSize);
            uint32_t localY = (uint32_t)(py - tileRow * arg->tileSize);
            uint32_t pixIdx = localY * (uint32_t)arg->tileW[tileId] + localX;

            if (!claim_pixel(arg->tileBits[tileId], pixIdx)) {
                arg->rootsDeduped++;
                continue;
            }

            uint32_t rgb = solveRGB;
            if (arg->colorMode == COLOR_CONSTANT) {
                rgb = arg->constRGB;
            } else if (arg->colorMode == COLOR_RAINBOW) {
                rgb = ((uint32_t)arg->rbPalR[r] << 16) |
                      ((uint32_t)arg->rbPalG[r] << 8) |
                       arg->rbPalB[r];
            }

            if (!arg->skipPixOutput) {
                if (!vec_push2(&arg->pixVecs[tileId], pixIdx, rgb)) {
                    worker_fail(arg, "pix vec alloc failed");
                    goto cleanup;
                }
            }
            if (arg->emitPixelBins && (arg->colorMode == COLOR_SOLVE_SCORE || arg->colorMode == COLOR_SAVED_PALETTE)) {
                if (!vec_push2(&arg->pbxVecs[tileId], pixIdx, (uint32_t)solveBin)) {
                    worker_fail(arg, "pbx vec alloc failed");
                    goto cleanup;
                }
            }
            arg->rootsPlotted++;
        }
    }
cleanup:
    if (nativeStarted) arg->nativeUs = (long)(monotonic_us() - nativeStartUs);
    free(coeffSectionBuf);
    free(sectionBuf);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: roots2pix_mt stripe.bin|ignored /tmp/pix "
                "--width=W --height=H --center_re=X --center_im=Y --scale=S "
                "--degree=D --tile_size=T --n_tile_cols=C --n_tile_rows=R "
                "[--input_mode=tmpfile|sectioned] [--url=URL] [--input_size=BYTES] [--retries=N] "
                "[--threads=N] [--color=rainbow|solve_score|saved_palette|constant] "
                "[--match=none] [--palette=<name>] [--constant_color=RRGGBB] "
                "[--solve_metric=proximity|crowding|spread|anisotropy|area|clusteriness|shelliness|outlierness|nn_variation|real_axis_proximity|centroid_re|centroid_im|centroid_dist|dist_unit_circle|asymmetry_re|min_mod|max_mod|min_angular_separation] "
                "[--solve_score_clip_lo=X --solve_score_clip_hi=Y --solve_score_cuts=c1,...,c9] "
                "[--solve_score_omega=W] [--solve_score_omega_enabled=0|1] "
                "[--solve_bins_file=file.bin] [--pixel_bin_prefix=/tmp/pixbin] [--skip_pix_output=0|1] [--root_xforms=file.json]\n");
        return 1;
    }

    const char *binPath = argv[1];
    const char *outPrefix = argv[2];
    const char *inputModeStr = getArgStr(argc, argv, "--input_mode", "tmpfile");
    const char *url = getArgStr(argc, argv, "--url", NULL);
    long long inputSize = getArgLongLong(argc, argv, "--input_size", -1);
    int W = getArgInt(argc, argv, "--width", 4096);
    int H = getArgInt(argc, argv, "--height", 4096);
    double centerRe = getArgDouble(argc, argv, "--center_re", 0.0);
    double centerIm = getArgDouble(argc, argv, "--center_im", 0.0);
    double scale = getArgDouble(argc, argv, "--scale", 100.0);
    double rotation = getArgDouble(argc, argv, "--rotation", 0.0);
    double cosA = cos(rotation), sinA = sin(rotation);
    int degree = getArgInt(argc, argv, "--degree", 25);
    int tileSize = getArgInt(argc, argv, "--tile_size", 4096);
    int nTileCols = getArgInt(argc, argv, "--n_tile_cols", 1);
    int nTileRows = getArgInt(argc, argv, "--n_tile_rows", 1);
    int retries = getArgInt(argc, argv, "--retries", 2);
    int requestedThreads = getArgInt(argc, argv, "--threads", 1);
    const char *colorStr = getArgStr(argc, argv, "--color", "rainbow");
    const char *matchStr = getArgStr(argc, argv, "--match", "none");
    const char *palName = getArgStr(argc, argv, "--palette", "inferno");
    const char *solveBinsPath = getArgStr(argc, argv, "--solve_bins_file", NULL);
    const char *pixelBinPrefix = getArgStr(argc, argv, "--pixel_bin_prefix", NULL);
    int skipPixOutput = getArgInt(argc, argv, "--skip_pix_output", 0);
    const char *constColorStr = getArgStr(argc, argv, "--constant_color", "ffffff");
    const char *rtPath = getArgStr(argc, argv, "--root_xforms", NULL);
    enum InputMode inputMode = INPUT_TMPFILE;
    if (strcmp(inputModeStr, "sectioned") == 0) inputMode = INPUT_SECTIONED;
    else if (strcmp(inputModeStr, "tmpfile") != 0) {
        fprintf(stderr, "Unsupported input mode: %s\n", inputModeStr);
        return 1;
    }

    enum ColorMode colorMode = COLOR_RAINBOW;
    if (strcmp(colorStr, "solve_score") == 0 || strcmp(colorStr, "solve_proximity") == 0) colorMode = COLOR_SOLVE_SCORE;
    else if (strcmp(colorStr, "saved_palette") == 0) colorMode = COLOR_SAVED_PALETTE;
    else if (strcmp(colorStr, "constant") == 0) colorMode = COLOR_CONSTANT;
    else if (strcmp(colorStr, "rainbow") == 0) colorMode = COLOR_RAINBOW;
    else {
        fprintf(stderr, "Unsupported color mode for roots2pix_mt: %s\n", colorStr);
        return 1;
    }

    if (strcmp(matchStr, "none") != 0 && colorMode == COLOR_RAINBOW) {
        fprintf(stderr, "roots2pix_mt only supports --match=none in rainbow mode\n");
        return 1;
    }
    if (degree < 1 || degree > MAXDEG) {
        fprintf(stderr, "Invalid degree: %d\n", degree);
        return 1;
    }
    if (W < 1 || H < 1) {
        fprintf(stderr, "Invalid dimensions: %dx%d\n", W, H);
        return 1;
    }
    if (retries < 0 || retries > 10) {
        fprintf(stderr, "Invalid retries: %d\n", retries);
        return 1;
    }

    int nTiles = nTileCols * nTileRows;
    if (nTiles < 1 || nTiles > MAX_TILES) {
        fprintf(stderr, "Invalid tile grid: %dx%d\n", nTileCols, nTileRows);
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

    enum SolveMetric solveMetric = SOLVE_METRIC_PROXIMITY;
    const char *solveMetricStr = getArgStr(argc, argv, "--solve_metric", "proximity");
    if ((colorMode == COLOR_SOLVE_SCORE) && !parse_solve_metric(solveMetricStr, &solveMetric)) {
        fprintf(stderr, "ERROR: unknown solve_metric '%s'\n", solveMetricStr);
        return 1;
    }
    const char *scoreMetricsCsv = getArgStr(argc, argv, "--score_metrics", NULL);
    const char *scoreSourcesCsv = getArgStr(argc, argv, "--score_sources", NULL);
    const char *scoreClipLosCsv = getArgStr(argc, argv, "--score_clip_los", NULL);
    const char *scoreClipHisCsv = getArgStr(argc, argv, "--score_clip_his", NULL);
    const char *scoreProgramSpec = getArgStr(argc, argv, "--score_program", NULL);
    const char *scoreCoeffsFile = getArgStr(argc, argv, "--score_coeffs_file", NULL);
    const char *scoreCoeffsUrl = getArgStr(argc, argv, "--score_coeffs_url", NULL);
    const char *scoreParamsFile = getArgStr(argc, argv, "--score_params_file", NULL);
    long long scoreCoeffInputSize = getArgLongLong(argc, argv, "--score_coeff_input_size", -1);
    int scoreCoeffDegree = getArgInt(argc, argv, "--score_coeff_degree", 0);

    double solveScoreClipLo = getArgDouble(argc, argv, "--solve_score_clip_lo",
                               getArgDouble(argc, argv, "--solve_prox_clip_lo", 0));
    double solveScoreClipHi = getArgDouble(argc, argv, "--solve_score_clip_hi",
                               getArgDouble(argc, argv, "--solve_prox_clip_hi", 0));
    double solveScoreOmega = getArgDouble(argc, argv, "--solve_score_omega", 1.0);
    int solveScoreOmegaEnabled = getArgInt(argc, argv, "--solve_score_omega_enabled", 1);
    SolveScoreProgram solveScoreProgram;
    int useScoreProgram = 0;
    double solveScoreCuts[9] = {0};
    int nSolveScoreCuts = 0;
    {
        const char *cutsStr = getArgStr(argc, argv, "--solve_score_cuts",
                               getArgStr(argc, argv, "--solve_prox_cuts", NULL));
        if (cutsStr) {
            char tmp[256];
            strncpy(tmp, cutsStr, sizeof(tmp) - 1);
            tmp[sizeof(tmp) - 1] = '\0';
            char *tok = strtok(tmp, ",");
            while (tok && nSolveScoreCuts < 9) {
                solveScoreCuts[nSolveScoreCuts++] = atof(tok);
                tok = strtok(NULL, ",");
            }
        }
    }
    if (scoreMetricsCsv || scoreClipLosCsv || scoreClipHisCsv || scoreProgramSpec) {
        char scoreErr[256] = {0};
        if (!scoreMetricsCsv || !scoreClipLosCsv || !scoreClipHisCsv || !scoreProgramSpec) {
            fprintf(stderr, "solve_score program requires --score_metrics, --score_clip_los, --score_clip_his, and --score_program together\n");
            return 1;
        }
        if (!parse_solve_score_program_args_ex(
                scoreMetricsCsv, scoreSourcesCsv, scoreClipLosCsv, scoreClipHisCsv, scoreProgramSpec,
                &solveScoreProgram, scoreErr, sizeof(scoreErr))) {
            fprintf(stderr, "Invalid solve_score program: %s\n", scoreErr[0] ? scoreErr : "unknown error");
            return 1;
        }
        solveMetric = solveScoreProgram.metrics[0];
        useScoreProgram = 1;
    }
    if (colorMode == COLOR_SOLVE_SCORE) {
        if (nSolveScoreCuts != 9) {
            fprintf(stderr, "solve_score requires exactly 9 cuts (got %d)\n", nSolveScoreCuts);
            return 1;
        }
        if (!useScoreProgram && solveScoreClipHi - solveScoreClipLo < 1e-12) {
            fprintf(stderr, "solve_score requires valid clip range\n");
            return 1;
        }
    }

    unsigned int constHex = 0xffffff;
    sscanf(constColorStr, "%x", &constHex);
    uint32_t constRGB = (((constHex >> 16) & 0xffu) << 16) |
                        (((constHex >> 8) & 0xffu) << 8) |
                        (constHex & 0xffu);

    int stride = degree * 2;
    long solveBytes = stride * (long)sizeof(float);
    long fileSize = 0;
    long nPoints = 0;
    float *roots = NULL;
    int scoreProgramUsesCoeffSources = 0;
    int scoreProgramUsesParamSources = 0;
    float *scoreCoeffRows = NULL;
    int scoreCoeffStride = scoreCoeffDegree * 2;
    long scoreCoeffSolveBytes = scoreCoeffStride * (long)sizeof(float);

    if (inputMode == INPUT_TMPFILE) {
        FILE *fin = fopen(binPath, "rb");
        if (!fin) {
            fprintf(stderr, "Cannot open %s\n", binPath);
            return 1;
        }
        fseek(fin, 0, SEEK_END);
        fileSize = ftell(fin);
        fseek(fin, 0, SEEK_SET);
        nPoints = fileSize / solveBytes;
        if (nPoints <= 0) {
            fprintf(stderr, "Empty root file\n");
            fclose(fin);
            return 1;
        }
        roots = malloc((size_t)fileSize);
        if (!roots) {
            fprintf(stderr, "Cannot allocate %ld bytes\n", fileSize);
            fclose(fin);
            return 1;
        }
        if ((long)fread(roots, 1, (size_t)fileSize, fin) != fileSize) {
            fprintf(stderr, "Short read\n");
            fclose(fin);
            free(roots);
            return 1;
        }
        fclose(fin);
    } else {
        if (!url || !*url) {
            fprintf(stderr, "sectioned input requires --url\n");
            return 1;
        }
        if (inputSize <= 0) {
            fprintf(stderr, "sectioned input requires --input_size\n");
            return 1;
        }
        if ((inputSize % solveBytes) != 0) {
            fprintf(stderr, "Invalid input_size %lld for degree=%d (solve_bytes=%ld)\n", inputSize, degree, solveBytes);
            return 1;
        }
        fileSize = (long)inputSize;
        nPoints = fileSize / solveBytes;
        if (nPoints <= 0) {
            fprintf(stderr, "Empty sectioned input\n");
            return 1;
        }
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
            free(roots);
            return 1;
        }
        if (inputMode == INPUT_TMPFILE) {
            if (!scoreCoeffsFile || !*scoreCoeffsFile) {
                fprintf(stderr, "solve_score program with coeff sources requires --score_coeffs_file\n");
                free(roots);
                return 1;
            }
            FILE *fc = fopen(scoreCoeffsFile, "rb");
            if (!fc) {
                fprintf(stderr, "Cannot open %s\n", scoreCoeffsFile);
                free(roots);
                return 1;
            }
            fseek(fc, 0, SEEK_END);
            long coeffFileSize = ftell(fc);
            fseek(fc, 0, SEEK_SET);
            long coeffPoints = coeffFileSize / scoreCoeffSolveBytes;
            if (coeffPoints != nPoints) {
                fprintf(stderr, "score coeffs size mismatch: got %ld solves expected %ld\n", coeffPoints, nPoints);
                fclose(fc);
                free(roots);
                return 1;
            }
            scoreCoeffRows = malloc((size_t)coeffFileSize);
            if (!scoreCoeffRows) {
                fprintf(stderr, "Cannot allocate %ld bytes for score coeffs\n", coeffFileSize);
                fclose(fc);
                free(roots);
                return 1;
            }
            if ((long)fread(scoreCoeffRows, 1, (size_t)coeffFileSize, fc) != coeffFileSize) {
                fprintf(stderr, "Short read from %s\n", scoreCoeffsFile);
                fclose(fc);
                free(scoreCoeffRows);
                free(roots);
                return 1;
            }
            fclose(fc);
        } else {
            if (!scoreCoeffsUrl || !*scoreCoeffsUrl) {
                fprintf(stderr, "sectioned solve_score program with coeff sources requires --score_coeffs_url\n");
                free(roots);
                return 1;
            }
            if (scoreCoeffInputSize <= 0) {
                fprintf(stderr, "sectioned solve_score program with coeff sources requires --score_coeff_input_size\n");
                free(roots);
                return 1;
            }
            if ((scoreCoeffInputSize % scoreCoeffSolveBytes) != 0) {
                fprintf(stderr, "Invalid score_coeff_input_size %lld for coeff_degree=%d (solve_bytes=%ld)\n",
                        scoreCoeffInputSize, scoreCoeffDegree, scoreCoeffSolveBytes);
                free(roots);
                return 1;
            }
            long coeffPoints = (long)(scoreCoeffInputSize / scoreCoeffSolveBytes);
            if (coeffPoints != nPoints) {
                fprintf(stderr, "sectioned score coeff solve count mismatch: got %ld expected %ld\n", coeffPoints, nPoints);
                free(roots);
                return 1;
            }
        }
    }
    float *scoreParamRows = NULL;
    int scoreParamStride = 4;
    int scoreParamDegree = 2;
    if (scoreProgramUsesParamSources) {
        if (!scoreParamsFile || !*scoreParamsFile) {
            fprintf(stderr, "solve_score program with param sources requires --score_params_file\n");
            free(roots);
            return 1;
        }
        FILE *fp = fopen(scoreParamsFile, "rb");
        if (!fp) {
            fprintf(stderr, "Cannot open %s\n", scoreParamsFile);
            free(roots);
            return 1;
        }
        fseek(fp, 0, SEEK_END);
        long paramFileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        long paramSolveBytes = (long)scoreParamStride * (long)sizeof(float);
        long paramPoints = paramFileSize / paramSolveBytes;
        if (paramPoints != nPoints) {
            fprintf(stderr, "score params size mismatch: got %ld solves expected %ld\n", paramPoints, nPoints);
            fclose(fp);
            free(roots);
            return 1;
        }
        scoreParamRows = malloc((size_t)paramFileSize);
        if (!scoreParamRows) {
            fprintf(stderr, "Cannot allocate %ld bytes for score params\n", paramFileSize);
            fclose(fp);
            free(roots);
            return 1;
        }
        if ((long)fread(scoreParamRows, 1, (size_t)paramFileSize, fp) != paramFileSize) {
            fprintf(stderr, "Short read from %s\n", scoreParamsFile);
            fclose(fp);
            free(scoreParamRows);
            free(roots);
            return 1;
        }
        fclose(fp);
    }

    uint8_t *solveBins = NULL;
    if (colorMode == COLOR_SAVED_PALETTE) {
        if (!solveBinsPath) {
            fprintf(stderr, "saved_palette requires --solve_bins_file\n");
            free(roots);
            return 1;
        }
        FILE *fb = fopen(solveBinsPath, "rb");
        if (!fb) {
            fprintf(stderr, "Cannot open %s\n", solveBinsPath);
            free(roots);
            return 1;
        }
        fseek(fb, 0, SEEK_END);
        long binSize = ftell(fb);
        fseek(fb, 0, SEEK_SET);
        if (binSize != nPoints) {
            fprintf(stderr, "saved_palette bins size mismatch: got %ld expected %ld\n", binSize, nPoints);
            fclose(fb);
            free(roots);
            return 1;
        }
        solveBins = malloc((size_t)nPoints);
        if (!solveBins) {
            fprintf(stderr, "Cannot allocate solve bins\n");
            fclose(fb);
            free(roots);
            return 1;
        }
        if ((long)fread(solveBins, 1, (size_t)nPoints, fb) != nPoints) {
            fprintf(stderr, "Short read from %s\n", solveBinsPath);
            fclose(fb);
            free(solveBins);
            free(roots);
            return 1;
        }
        fclose(fb);
    }

    int tileW[MAX_TILES];
    int tileH[MAX_TILES];
    size_t tileWordCount[MAX_TILES];
    uint64_t *tileBits[MAX_TILES] = {0};
    unsigned char rbPalR[MAXDEG], rbPalG[MAXDEG], rbPalB[MAXDEG];
    unsigned char ssPalR[10] = {0}, ssPalG[10] = {0}, ssPalB[10] = {0};
    int emitPixelBins = pixelBinPrefix &&
        (colorMode == COLOR_SOLVE_SCORE || colorMode == COLOR_SAVED_PALETTE);
    if (skipPixOutput && !emitPixelBins) {
        fprintf(stderr, "--skip_pix_output requires --pixel_bin_prefix in solve_score/saved_palette mode\n");
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

    for (int t = 0; t < nTiles; t++) {
        int tc = t % nTileCols;
        int tr = t / nTileCols;
        tileW[t] = (tc < nTileCols - 1) ? tileSize : (W - tc * tileSize);
        tileH[t] = (tr < nTileRows - 1) ? tileSize : (H - tr * tileSize);
        if (tileW[t] <= 0 || tileH[t] <= 0) {
            fprintf(stderr, "Invalid tile %d geometry\n", t);
            goto cleanup;
        }
        tileWordCount[t] = (((size_t)tileW[t] * (size_t)tileH[t]) + 63u) / 64u;
        tileBits[t] = calloc(tileWordCount[t], sizeof(uint64_t));
        if (!tileBits[t]) {
            fprintf(stderr, "Cannot allocate tile bitset %d\n", t);
            goto cleanup;
        }
    }

    for (int i = 0; i < degree; i++) {
        rainbowRGB(i, degree, &rbPalR[i], &rbPalG[i], &rbPalB[i]);
    }
    if (colorMode == COLOR_SOLVE_SCORE || colorMode == COLOR_SAVED_PALETTE) {
        const PaletteDef *proxPal = findPalette(palName);
        if (!proxPal) {
            fprintf(stderr, "Unknown palette: %s\n", palName);
            goto cleanup;
        }
        for (int b = 0; b < 10; b++) {
            paletteRGB(proxPal, (b + 0.5) / 10.0, &ssPalR[b], &ssPalG[b], &ssPalB[b]);
        }
    }

    if (inputMode == INPUT_SECTIONED) {
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
        args[i].tileSize = tileSize;
        args[i].nTileCols = nTileCols;
        args[i].nTileRows = nTileRows;
        args[i].nTiles = nTiles;
        args[i].centerRe = centerRe;
        args[i].centerIm = centerIm;
        args[i].scale = scale;
        args[i].cosA = cosA;
        args[i].sinA = sinA;
        args[i].halfW = W / 2.0;
        args[i].halfH = H / 2.0;
        args[i].colorMode = colorMode;
        args[i].inputMode = inputMode;
        args[i].solveMetric = solveMetric;
        args[i].useScoreProgram = useScoreProgram;
        args[i].solveScoreProgram = solveScoreProgram;
        args[i].solveScoreClipLo = solveScoreClipLo;
        args[i].solveScoreClipHi = solveScoreClipHi;
        args[i].solveScoreOmega = solveScoreOmega;
        args[i].solveScoreOmegaEnabled = solveScoreOmegaEnabled;
        memcpy(args[i].solveScoreCuts, solveScoreCuts, sizeof(solveScoreCuts));
        args[i].nSolveScoreCuts = nSolveScoreCuts;
        args[i].constRGB = constRGB;
        args[i].emitPixelBins = emitPixelBins;
        args[i].skipPixOutput = skipPixOutput;
        args[i].roots = roots;
        args[i].scoreCoeffRows = scoreCoeffRows;
        args[i].scoreParamRows = scoreParamRows;
        args[i].scoreCoeffDegree = scoreProgramUsesCoeffSources ? scoreCoeffDegree : 0;
        args[i].scoreCoeffStride = scoreCoeffStride;
        args[i].scoreParamDegree = scoreProgramUsesParamSources ? scoreParamDegree : 0;
        args[i].scoreParamStride = scoreParamStride;
        args[i].url = url;
        args[i].scoreCoeffsUrl = scoreProgramUsesCoeffSources ? scoreCoeffsUrl : NULL;
        args[i].retries = retries;
        args[i].solveBytes = solveBytes;
        args[i].scoreCoeffSolveBytes = scoreCoeffSolveBytes;
        args[i].sectionBytes = (size_t)width * (size_t)solveBytes;
        args[i].scoreCoeffSectionBytes = (size_t)width * (size_t)scoreCoeffSolveBytes;
        args[i].byteStart = (unsigned long long)start * (unsigned long long)solveBytes;
        args[i].byteEnd = args[i].sectionBytes > 0
            ? args[i].byteStart + (unsigned long long)args[i].sectionBytes - 1ULL
            : args[i].byteStart;
        args[i].scoreCoeffByteStart = (unsigned long long)start * (unsigned long long)scoreCoeffSolveBytes;
        args[i].scoreCoeffByteEnd = args[i].scoreCoeffSectionBytes > 0
            ? args[i].scoreCoeffByteStart + (unsigned long long)args[i].scoreCoeffSectionBytes - 1ULL
            : args[i].scoreCoeffByteStart;
        args[i].solveBins = solveBins;
        args[i].rtChain = rtChain;
        args[i].nRt = nRt;
        args[i].tileBits = tileBits;
        args[i].tileW = tileW;
        args[i].pixVecs = skipPixOutput ? NULL : calloc((size_t)nTiles, sizeof(U32Vec));
        args[i].pbxVecs = emitPixelBins ? calloc((size_t)nTiles, sizeof(U32Vec)) : NULL;
        memcpy(args[i].rbPalR, rbPalR, sizeof(rbPalR));
        memcpy(args[i].rbPalG, rbPalG, sizeof(rbPalG));
        memcpy(args[i].rbPalB, rbPalB, sizeof(rbPalB));
        memcpy(args[i].ssPalR, ssPalR, sizeof(ssPalR));
        memcpy(args[i].ssPalG, ssPalG, sizeof(ssPalG));
        memcpy(args[i].ssPalB, ssPalB, sizeof(ssPalB));
        if ((!skipPixOutput && !args[i].pixVecs) || (emitPixelBins && !args[i].pbxVecs)) {
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
    int tilesWithData = 0;
    for (int t = 0; t < nTiles; t++) {
        size_t tilePixU32 = 0;
        size_t tilePbxU32 = 0;
        for (int i = 0; i < threads; i++) {
            if (!skipPixOutput) tilePixU32 += args[i].pixVecs[t].len;
            if (emitPixelBins) tilePbxU32 += args[i].pbxVecs[t].len;
        }
        if (tilePixU32 > 0) {
            snprintf(pathBuf, sizeof(pathBuf), "%s_t%04d.pix", outPrefix, t);
            FILE *fout = fopen(pathBuf, "wb");
            if (!fout) {
                fprintf(stderr, "Cannot create %s\n", pathBuf);
                goto cleanup;
            }
            for (int i = 0; i < threads; i++) {
                if (args[i].pixVecs[t].len > 0) {
                    fwrite(args[i].pixVecs[t].data, sizeof(uint32_t), args[i].pixVecs[t].len, fout);
                }
            }
            fclose(fout);
            tilesWithData++;
            totalEntries += (long)(tilePixU32 / 2u);
        }
        if (emitPixelBins && tilePbxU32 > 0) {
            snprintf(pathBuf, sizeof(pathBuf), "%s_t%04d.pbx", pixelBinPrefix, t);
            FILE *fb = fopen(pathBuf, "wb");
            if (!fb) {
                fprintf(stderr, "Cannot create %s\n", pathBuf);
                goto cleanup;
            }
            for (int i = 0; i < threads; i++) {
                if (args[i].pbxVecs[t].len > 0) {
                    fwrite(args[i].pbxVecs[t].data, sizeof(uint32_t), args[i].pbxVecs[t].len, fb);
                }
            }
            fclose(fb);
            if (skipPixOutput) {
                tilesWithData++;
                totalEntries += (long)(tilePbxU32 / 2u);
            }
        }
    }

    if (rootsDeduped > 0) {
        fprintf(stderr, "dedup: %ld unique, %ld skipped\n", rootsPlotted, rootsDeduped);
    }

    printf("{\"roots_plotted\":%ld,\"roots_clipped\":%ld,\"n_points\":%ld,"
           "\"degree\":%d,\"threads\":%d,\"color\":\"%s\",\"match\":\"%s\","
           "\"n_tiles\":%d,\"tiles_with_data\":%d,\"total_entries\":%ld,"
           "\"input_mode\":\"%s\",\"retries\":%d,\"download_us\":%ld,\"native_us\":%ld",
           rootsPlotted, rootsClipped, nPoints, degree, threads, colorStr, matchStr,
           nTiles, tilesWithData, totalEntries,
           inputMode == INPUT_SECTIONED ? "sectioned" : "tmpfile",
           retries,
           totalDownloadUs, totalNativeUs);
    if (colorMode == COLOR_SOLVE_SCORE) {
        printf(",\"palette\":\"%s\",\"solve_score\":true,\"solve_metric\":\"%s\",\"solve_score_omega\":%.15g,\"solve_score_omega_enabled\":%s",
               palName, solve_metric_name(solveMetric), solveScoreOmega, solveScoreOmegaEnabled ? "true" : "false");
    } else if (colorMode == COLOR_SAVED_PALETTE) {
        printf(",\"palette\":\"%s\",\"saved_palette\":true", palName);
    } else if (colorMode == COLOR_CONSTANT) {
        printf(",\"constant_color\":\"%s\"", constColorStr);
    }
    printf(",\"skip_pix_output\":%s", skipPixOutput ? "true" : "false");
    printf("}\n");
    exitCode = 0;

cleanup:
    if (workers && !workersJoined) {
        for (int i = 0; i < workersStarted; i++) pthread_join(workers[i], NULL);
    }
    free_worker_storage(args, workersPrepared, nTiles);
    for (int t = 0; t < nTiles; t++) free(tileBits[t]);
    free(workers);
    free(args);
    free(solveBins);
    free(scoreCoeffRows);
    free(scoreParamRows);
    free(roots);
    if (curlInitialized) curl_global_cleanup();
    return exitCode;
}
