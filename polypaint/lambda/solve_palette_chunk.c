/*
 * solve_palette_chunk — compute exact solve-score scores and final bins for one root chunk.
 *
 * Usage:
 *   solve_palette_chunk input.bin \
 *     --degree=D --metric=proximity \
 *     --clip_lo=X --clip_hi=Y --cuts=c1,...,c9 --omega=1 \
 *     --step_count=S \
 *     --scores_out=file.bin --bins_out=file.bin \
 *     [--root_xforms=file.json]
 *
 * Input .bin format: raw float32 roots, one solve after another, degree complex roots each.
 * Output:
 *   - scores_out: float32[count]
 *   - bins_out: uint8[count] in range 0..9
 *   - stdout: small JSON metadata
 *
 * Build:
 *   aarch64-linux-musl-gcc -O3 -static -o solve_palette_chunk solve_palette_chunk.c -lm
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static const char *getArgStr(int argc, char **argv, const char *key, const char *def) {
    const char *v = getArg(argc, argv, key);
    return v ? v : def;
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

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: solve_palette_chunk input.bin --degree=D --metric=... "
                "--clip_lo=X --clip_hi=Y --cuts=c1,...,c9 --step_count=S "
                "--scores_out=file.bin --bins_out=file.bin [--root_xforms=file.json]\n");
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
    const char *scoreCoeffsFile = getArgStr(argc, argv, "--score_coeffs_file", NULL);
    const char *scoreParamsFile = getArgStr(argc, argv, "--score_params_file", NULL);
    int scoreCoeffDegree = getArgInt(argc, argv, "--score_coeff_degree", 0);
    double clipLo = getArgDouble(argc, argv, "--clip_lo", 0.0);
    double clipHi = getArgDouble(argc, argv, "--clip_hi", 0.0);
    double omega = getArgDouble(argc, argv, "--omega", 1.0);
    int omegaEnabled = getArgInt(argc, argv, "--omega_enabled", 1);
    int stepCount = getArgInt(argc, argv, "--step_count", -1);
    const char *cutsStr = getArgStr(argc, argv, "--cuts", NULL);
    const char *scoresOut = getArgStr(argc, argv, "--scores_out", NULL);
    const char *binsOut = getArgStr(argc, argv, "--bins_out", NULL);
    const char *rtPath = getArgStr(argc, argv, "--root_xforms", NULL);

    if (degree < 2 || degree > MAXDEG) {
        fprintf(stderr, "Invalid degree: %d\n", degree);
        return 1;
    }
    if (!scoresOut || !binsOut) {
        fprintf(stderr, "Missing --scores_out or --bins_out\n");
        return 1;
    }
    if (!scoreProgramSpec && clipHi - clipLo < 1e-12) {
        fprintf(stderr, "Invalid clip range: lo=%.6g hi=%.6g\n", clipLo, clipHi);
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
        if (solve_score_program_uses_lag(&scoreProgram)) {
            fprintf(stderr, "Lagged solve-score refs are supported only by fused color raster in v1\n");
            return 1;
        }
        metric = scoreProgram.metrics[0];
        useScoreProgram = 1;
    }

    double cuts[9];
    int nCuts = parseCuts(cutsStr, cuts);
    if (nCuts != 9) {
        fprintf(stderr, "Expected exactly 9 cuts, got %d\n", nCuts);
        return 1;
    }

    FILE *f = fopen(inPath, "rb");
    if (!f) {
        fprintf(stderr, "Cannot open %s\n", inPath);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    int stride = degree * 2;
    long totalSolves = fileSize / (stride * (long)sizeof(float));
    if (stepCount < 0) stepCount = (int)totalSolves;
    if ((long)stepCount > totalSolves) {
        fprintf(stderr, "step_count=%d exceeds total solves=%ld\n", stepCount, totalSolves);
        fclose(f);
        return 1;
    }

    long bytesWanted = (long)stepCount * stride * (long)sizeof(float);
    float *buf = malloc(bytesWanted);
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

    FILE *sf = fopen(scoresOut, "wb");
    if (!sf) {
        fprintf(stderr, "Cannot open %s\n", scoresOut);
        free(buf);
        return 1;
    }
    FILE *bf = fopen(binsOut, "wb");
    if (!bf) {
        fprintf(stderr, "Cannot open %s\n", binsOut);
        fclose(sf);
        free(buf);
        return 1;
    }

    float wkRe[MAXDEG], wkIm[MAXDEG];
    int scoreProgramUsesCoeffSources = 0;
    int scoreProgramUsesParamSources = 0;
    for (int i = 0; i < scoreProgram.metricCount; i++) {
        if (scoreProgram.metricSources[i] == SOLVE_SCORE_SOURCE_COEFF) {
            scoreProgramUsesCoeffSources = 1;
        }
        if (scoreProgram.metricSources[i] == SOLVE_SCORE_SOURCE_PARAM) {
            scoreProgramUsesParamSources = 1;
        }
    }
    float *scoreCoeffRows = NULL;
    int scoreCoeffStride = scoreCoeffDegree * 2;
    if (scoreProgramUsesCoeffSources) {
        if (!scoreCoeffsFile || !*scoreCoeffsFile) {
            fprintf(stderr, "score program with coeff sources requires --score_coeffs_file\n");
            fclose(sf);
            fclose(bf);
            free(buf);
            return 1;
        }
        if (scoreCoeffDegree < 1 || scoreCoeffDegree > MAXDEG) {
            fprintf(stderr, "Invalid score_coeff_degree: %d (must be 1-%d)\n", scoreCoeffDegree, MAXDEG);
            fclose(sf);
            fclose(bf);
            free(buf);
            return 1;
        }
        FILE *fc = fopen(scoreCoeffsFile, "rb");
        if (!fc) {
            fprintf(stderr, "Cannot open %s\n", scoreCoeffsFile);
            fclose(sf);
            fclose(bf);
            free(buf);
            return 1;
        }
        fseek(fc, 0, SEEK_END);
        long coeffFileSize = ftell(fc);
        fseek(fc, 0, SEEK_SET);
        long coeffSolveBytes = scoreCoeffStride * (long)sizeof(float);
        long coeffPoints = coeffFileSize / coeffSolveBytes;
        if (coeffPoints != stepCount) {
            fprintf(stderr, "score coeffs size mismatch: got %ld solves expected %d\n", coeffPoints, stepCount);
            fclose(fc);
            fclose(sf);
            fclose(bf);
            free(buf);
            return 1;
        }
        scoreCoeffRows = malloc(coeffFileSize);
        if (!scoreCoeffRows) {
            fprintf(stderr, "Out of memory for score coeff rows\n");
            fclose(fc);
            fclose(sf);
            fclose(bf);
            free(buf);
            return 1;
        }
        if ((long)fread(scoreCoeffRows, 1, coeffFileSize, fc) != coeffFileSize) {
            fprintf(stderr, "Short read from %s\n", scoreCoeffsFile);
            fclose(fc);
            fclose(sf);
            fclose(bf);
            free(scoreCoeffRows);
            free(buf);
            return 1;
        }
        fclose(fc);
    }
    float *scoreParamRows = NULL;
    int scoreParamStride = 4;
    int scoreParamDegree = 2;
    if (scoreProgramUsesParamSources) {
        if (!scoreParamsFile || !*scoreParamsFile) {
            fprintf(stderr, "score program with param sources requires --score_params_file\n");
            fclose(sf);
            fclose(bf);
            free(scoreCoeffRows);
            free(buf);
            return 1;
        }
        FILE *fp = fopen(scoreParamsFile, "rb");
        if (!fp) {
            fprintf(stderr, "Cannot open %s\n", scoreParamsFile);
            fclose(sf);
            fclose(bf);
            free(scoreCoeffRows);
            free(buf);
            return 1;
        }
        fseek(fp, 0, SEEK_END);
        long paramFileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        long paramSolveBytes = scoreParamStride * (long)sizeof(float);
        long paramPoints = paramFileSize / paramSolveBytes;
        if (paramPoints != stepCount) {
            fprintf(stderr, "score params size mismatch: got %ld solves expected %d\n", paramPoints, stepCount);
            fclose(fp);
            fclose(sf);
            fclose(bf);
            free(scoreCoeffRows);
            free(buf);
            return 1;
        }
        scoreParamRows = malloc(paramFileSize);
        if (!scoreParamRows) {
            fprintf(stderr, "Out of memory for score param rows\n");
            fclose(fp);
            fclose(sf);
            fclose(bf);
            free(scoreCoeffRows);
            free(buf);
            return 1;
        }
        if ((long)fread(scoreParamRows, 1, paramFileSize, fp) != paramFileSize) {
            fprintf(stderr, "Short read from %s\n", scoreParamsFile);
            fclose(fp);
            fclose(sf);
            fclose(bf);
            free(scoreParamRows);
            free(scoreCoeffRows);
            free(buf);
            return 1;
        }
        fclose(fp);
    }
    double minScore = 0.0, maxScore = 0.0;

    for (int s = 0; s < stepCount; s++) {
        float *roots = buf + (long)s * stride;
        const float *coeffRoots = scoreCoeffRows ? (scoreCoeffRows + (long)s * scoreCoeffStride) : NULL;
        const float *paramValues = scoreParamRows ? (scoreParamRows + (long)s * scoreParamStride) : NULL;
        double score;
        if (nRt > 0) {
            float xf[MAXDEG * 2];
            for (int k = 0; k < degree; k++) {
                wkRe[k] = roots[k * 2];
                wkIm[k] = roots[k * 2 + 1];
            }
            apply_root_xforms(rtChain, nRt, wkRe, wkIm, degree);
            for (int k = 0; k < degree; k++) {
                xf[k * 2] = wkRe[k];
                xf[k * 2 + 1] = wkIm[k];
            }
            score = useScoreProgram ? solve_score_eval_program_with_sources(
                xf, degree, coeffRoots, scoreCoeffDegree, paramValues, scoreParamDegree, &scoreProgram
            )
                                    : compute_solve_metric_score(xf, degree, metric);
        } else {
            score = useScoreProgram ? solve_score_eval_program_with_sources(
                roots, degree, coeffRoots, scoreCoeffDegree, paramValues, scoreParamDegree, &scoreProgram
            )
                                    : compute_solve_metric_score(roots, degree, metric);
        }

        if (s == 0) {
            minScore = maxScore = score;
        } else {
            if (score < minScore) minScore = score;
            if (score > maxScore) maxScore = score;
        }

        double u;
        if (useScoreProgram) {
            u = score;
        } else {
            double range = clipHi - clipLo;
            u = (score - clipLo) / range;
            if (u < 0) u = 0;
            if (u > 1) u = 1;
            u = apply_solve_score_transfer(u, omegaEnabled, omega);
        }

        uint8_t bin = 9;
        for (int c = 0; c < 9; c++) {
            if (u <= cuts[c]) {
                bin = (uint8_t)c;
                break;
            }
        }

        float sf32 = (float)score;
        fwrite(&sf32, sizeof(float), 1, sf);
        fwrite(&bin, 1, 1, bf);
    }

    fclose(sf);
    fclose(bf);
    free(scoreParamRows);
    free(scoreCoeffRows);
    free(buf);

    printf("{\"mode\":\"palette_chunk\",\"metric\":\"%s\",\"n_samples\":%d,"
           "\"omega\":%.15g,\"omega_enabled\":%s,\"clip_lo\":%.15g,\"clip_hi\":%.15g,\"min_score\":%.15g,\"max_score\":%.15g}\n",
           solve_metric_name(metric), stepCount, omega, omegaEnabled ? "true" : "false", clipLo, clipHi, minScore, maxScore);
    return 0;
}
