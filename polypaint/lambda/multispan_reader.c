#include "multispan_reader.h"

#include <curl/curl.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define MSR_THREAD_LOCAL _Thread_local
#elif defined(__GNUC__) || defined(__clang__)
#define MSR_THREAD_LOCAL __thread
#else
#error "multispan_reader requires thread-local storage support"
#endif

typedef struct {
    unsigned char *data;
    size_t expected;
    size_t size;
    int overflow;
} DownloadBuffer;

static MSR_THREAD_LOCAL CURL *msr_tls_curl = NULL;

static void msr_set_error(char *errBuf, size_t errBufLen, const char *msg) {
    if (!errBuf || errBufLen == 0) return;
    snprintf(errBuf, errBufLen, "%s", msg ? msg : "unknown error");
}

static void msr_set_errorf(char *errBuf, size_t errBufLen, const char *fmt, ...) {
    va_list ap;
    if (!errBuf || errBufLen == 0) return;
    va_start(ap, fmt);
    vsnprintf(errBuf, errBufLen, fmt, ap);
    va_end(ap);
}

static void msr_skip_ws(const char **p) {
    while (**p && isspace((unsigned char)**p)) (*p)++;
}

static int msr_expect_char(const char **p, char expected, char *errBuf, size_t errBufLen) {
    msr_skip_ws(p);
    if (**p != expected) {
        msr_set_errorf(errBuf, errBufLen, "expected '%c', got '%c'", expected, **p ? **p : '?');
        return 0;
    }
    (*p)++;
    return 1;
}

static int msr_parse_string(const char **p, char **out, char *errBuf, size_t errBufLen) {
    size_t cap = 32;
    size_t len = 0;
    char *buf = NULL;
    msr_skip_ws(p);
    if (**p != '"') {
        msr_set_error(errBuf, errBufLen, "expected string");
        return 0;
    }
    (*p)++;
    buf = (char *)malloc(cap);
    if (!buf) {
        msr_set_error(errBuf, errBufLen, "out of memory parsing string");
        return 0;
    }
    while (**p && **p != '"') {
        unsigned char ch = (unsigned char)**p;
        if (ch == '\\') {
            (*p)++;
            ch = (unsigned char)**p;
            if (!ch) {
                free(buf);
                msr_set_error(errBuf, errBufLen, "unterminated escape");
                return 0;
            }
            switch (ch) {
                case '"':
                case '\\':
                case '/':
                    break;
                case 'b':
                    ch = '\b';
                    break;
                case 'f':
                    ch = '\f';
                    break;
                case 'n':
                    ch = '\n';
                    break;
                case 'r':
                    ch = '\r';
                    break;
                case 't':
                    ch = '\t';
                    break;
                default:
                    free(buf);
                    msr_set_errorf(errBuf, errBufLen, "unsupported escape \\%c", ch);
                    return 0;
            }
        }
        if (len + 2 > cap) {
            cap *= 2;
            char *grown = (char *)realloc(buf, cap);
            if (!grown) {
                free(buf);
                msr_set_error(errBuf, errBufLen, "out of memory growing string");
                return 0;
            }
            buf = grown;
        }
        buf[len++] = (char)ch;
        (*p)++;
    }
    if (**p != '"') {
        free(buf);
        msr_set_error(errBuf, errBufLen, "unterminated string");
        return 0;
    }
    (*p)++;
    buf[len] = '\0';
    *out = buf;
    return 1;
}

static int msr_parse_ull(const char **p, unsigned long long *out, char *errBuf, size_t errBufLen) {
    char *end = NULL;
    unsigned long long value = 0;
    msr_skip_ws(p);
    if (!isdigit((unsigned char)**p)) {
        msr_set_error(errBuf, errBufLen, "expected unsigned integer");
        return 0;
    }
    value = strtoull(*p, &end, 10);
    if (end == *p) {
        msr_set_error(errBuf, errBufLen, "invalid unsigned integer");
        return 0;
    }
    *p = end;
    *out = value;
    return 1;
}

static int msr_parse_int(const char **p, int *out, char *errBuf, size_t errBufLen) {
    unsigned long long value = 0;
    if (!msr_parse_ull(p, &value, errBuf, errBufLen)) return 0;
    *out = (int)value;
    return 1;
}

static int msr_skip_string(const char **p, char *errBuf, size_t errBufLen) {
    char *tmp = NULL;
    int ok = msr_parse_string(p, &tmp, errBuf, errBufLen);
    free(tmp);
    return ok;
}

static int msr_skip_value(const char **p, char *errBuf, size_t errBufLen);

static int msr_skip_array(const char **p, char *errBuf, size_t errBufLen) {
    if (!msr_expect_char(p, '[', errBuf, errBufLen)) return 0;
    msr_skip_ws(p);
    if (**p == ']') {
        (*p)++;
        return 1;
    }
    while (**p) {
        if (!msr_skip_value(p, errBuf, errBufLen)) return 0;
        msr_skip_ws(p);
        if (**p == ']') {
            (*p)++;
            return 1;
        }
        if (!msr_expect_char(p, ',', errBuf, errBufLen)) return 0;
    }
    msr_set_error(errBuf, errBufLen, "unterminated array");
    return 0;
}

static int msr_skip_object(const char **p, char *errBuf, size_t errBufLen) {
    if (!msr_expect_char(p, '{', errBuf, errBufLen)) return 0;
    msr_skip_ws(p);
    if (**p == '}') {
        (*p)++;
        return 1;
    }
    while (**p) {
        if (!msr_skip_string(p, errBuf, errBufLen)) return 0;
        if (!msr_expect_char(p, ':', errBuf, errBufLen)) return 0;
        if (!msr_skip_value(p, errBuf, errBufLen)) return 0;
        msr_skip_ws(p);
        if (**p == '}') {
            (*p)++;
            return 1;
        }
        if (!msr_expect_char(p, ',', errBuf, errBufLen)) return 0;
    }
    msr_set_error(errBuf, errBufLen, "unterminated object");
    return 0;
}

static int msr_skip_literal(const char **p, const char *literal, char *errBuf, size_t errBufLen) {
    size_t len = strlen(literal);
    if (strncmp(*p, literal, len) != 0) {
        msr_set_errorf(errBuf, errBufLen, "expected literal %s", literal);
        return 0;
    }
    *p += len;
    return 1;
}

static int msr_skip_number(const char **p, char *errBuf, size_t errBufLen) {
    char *end = NULL;
    msr_skip_ws(p);
    (void)strtod(*p, &end);
    if (end == *p) {
        msr_set_error(errBuf, errBufLen, "invalid number");
        return 0;
    }
    *p = end;
    return 1;
}

static int msr_skip_value(const char **p, char *errBuf, size_t errBufLen) {
    msr_skip_ws(p);
    switch (**p) {
        case '"':
            return msr_skip_string(p, errBuf, errBufLen);
        case '{':
            return msr_skip_object(p, errBuf, errBufLen);
        case '[':
            return msr_skip_array(p, errBuf, errBufLen);
        case 't':
            return msr_skip_literal(p, "true", errBuf, errBufLen);
        case 'f':
            return msr_skip_literal(p, "false", errBuf, errBufLen);
        case 'n':
            return msr_skip_literal(p, "null", errBuf, errBufLen);
        default:
            return msr_skip_number(p, errBuf, errBufLen);
    }
}

static int msr_parse_source_object(const char **p, MultiSpanSource *out, char *errBuf, size_t errBufLen) {
    memset(out, 0, sizeof(*out));
    if (!msr_expect_char(p, '{', errBuf, errBufLen)) return 0;
    while (**p) {
        char *key = NULL;
        msr_skip_ws(p);
        if (**p == '}') {
            (*p)++;
            return out->url != NULL;
        }
        if (!msr_parse_string(p, &key, errBuf, errBufLen)) return 0;
        if (!msr_expect_char(p, ':', errBuf, errBufLen)) {
            free(key);
            return 0;
        }
        if (strcmp(key, "id") == 0) {
            if (!msr_parse_int(p, &out->id, errBuf, errBufLen)) {
                free(key);
                return 0;
            }
        } else if (strcmp(key, "url") == 0) {
            if (!msr_parse_string(p, &out->url, errBuf, errBufLen)) {
                free(key);
                return 0;
            }
        } else if (strcmp(key, "key") == 0) {
            if (!msr_parse_string(p, &out->key, errBuf, errBufLen)) {
                free(key);
                return 0;
            }
        } else {
            if (!msr_skip_value(p, errBuf, errBufLen)) {
                free(key);
                return 0;
            }
        }
        free(key);
        msr_skip_ws(p);
        if (**p == '}') {
            (*p)++;
            return out->url != NULL;
        }
        if (!msr_expect_char(p, ',', errBuf, errBufLen)) return 0;
    }
    msr_set_error(errBuf, errBufLen, "unterminated source object");
    return 0;
}

static int msr_parse_span_object(const char **p, MultiSpanSpan *out, char *errBuf, size_t errBufLen) {
    memset(out, 0, sizeof(*out));
    if (!msr_expect_char(p, '{', errBuf, errBufLen)) return 0;
    while (**p) {
        char *key = NULL;
        msr_skip_ws(p);
        if (**p == '}') {
            (*p)++;
            return 1;
        }
        if (!msr_parse_string(p, &key, errBuf, errBufLen)) return 0;
        if (!msr_expect_char(p, ':', errBuf, errBufLen)) {
            free(key);
            return 0;
        }
        if (strcmp(key, "source_id") == 0) {
            if (!msr_parse_int(p, &out->sourceId, errBuf, errBufLen)) {
                free(key);
                return 0;
            }
        } else if (strcmp(key, "logical_byte_start") == 0) {
            if (!msr_parse_ull(p, &out->logicalByteStart, errBuf, errBufLen)) {
                free(key);
                return 0;
            }
        } else if (strcmp(key, "byte_start") == 0) {
            if (!msr_parse_ull(p, &out->byteStart, errBuf, errBufLen)) {
                free(key);
                return 0;
            }
        } else if (strcmp(key, "byte_length") == 0) {
            if (!msr_parse_ull(p, &out->byteLength, errBuf, errBufLen)) {
                free(key);
                return 0;
            }
        } else {
            if (!msr_skip_value(p, errBuf, errBufLen)) {
                free(key);
                return 0;
            }
        }
        free(key);
        msr_skip_ws(p);
        if (**p == '}') {
            (*p)++;
            return 1;
        }
        if (!msr_expect_char(p, ',', errBuf, errBufLen)) return 0;
    }
    msr_set_error(errBuf, errBufLen, "unterminated span object");
    return 0;
}

static char *msr_read_file(const char *path, size_t *sizeOut, char *errBuf, size_t errBufLen) {
    FILE *f = fopen(path, "rb");
    char *buf = NULL;
    size_t size = 0;
    if (!f) {
        msr_set_errorf(errBuf, errBufLen, "cannot open manifest %s", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (fileSize < 0) {
        fclose(f);
        msr_set_errorf(errBuf, errBufLen, "cannot stat manifest %s", path);
        return NULL;
    }
    buf = (char *)malloc((size_t)fileSize + 1);
    if (!buf) {
        fclose(f);
        msr_set_error(errBuf, errBufLen, "out of memory reading manifest");
        return NULL;
    }
    size = fread(buf, 1, (size_t)fileSize, f);
    fclose(f);
    if (size != (size_t)fileSize) {
        free(buf);
        msr_set_error(errBuf, errBufLen, "short read on manifest");
        return NULL;
    }
    buf[size] = '\0';
    if (sizeOut) *sizeOut = size;
    return buf;
}

static size_t msr_write_section_cb(char *ptr, size_t size, size_t nmemb, void *userdata) {
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

static void msr_sleep_ms(long ms) {
    if (ms <= 0) return;
    struct timespec ts;
    ts.tv_sec = ms / 1000L;
    ts.tv_nsec = (ms % 1000L) * 1000000L;
    nanosleep(&ts, NULL);
}

static int msr_retryable_range_failure(CURLcode rc, long httpStatus) {
    /* HTTP response present: retry only transient statuses, never a
     * permanent 4xx even though FAILONERROR reports CURLE_HTTP_RETURNED_ERROR
     * (CR28 F16). */
    if (httpStatus != 0L) {
        return httpStatus == 429L || httpStatus == 500L ||
               httpStatus == 502L || httpStatus == 503L || httpStatus == 504L;
    }
    return rc == CURLE_OPERATION_TIMEDOUT ||
           rc == CURLE_COULDNT_CONNECT ||
           rc == CURLE_COULDNT_RESOLVE_HOST ||
           rc == CURLE_RECV_ERROR ||
           rc == CURLE_SEND_ERROR ||
           rc == CURLE_GOT_NOTHING;
}

static int msr_starts_with(const char *text, const char *prefix) {
    size_t n = strlen(prefix);
    return text && strncmp(text, prefix, n) == 0;
}

static int msr_read_exact_file_range(const char *url,
                                     const char *key,
                                     unsigned long long byteStart,
                                     unsigned long long byteLength,
                                     unsigned char *dst,
                                     char *errBuf,
                                     size_t errBufLen) {
    const char *path = url;
    FILE *f = NULL;
    size_t remaining = 0;
    unsigned char *cursor = dst;
    if (!url || !dst) {
        msr_set_error(errBuf, errBufLen, "file range read requires url and dst");
        return 0;
    }
    if (msr_starts_with(url, "file://")) {
        path = url + 7;
    }
    if (!path || !*path) {
        msr_set_errorf(errBuf, errBufLen, "empty file path for %s", key ? key : "local source");
        return 0;
    }
    if ((unsigned long long)((size_t)byteLength) != byteLength) {
        msr_set_errorf(errBuf, errBufLen, "file range too large for %s: %llu bytes",
                       key ? key : path, byteLength);
        return 0;
    }
    f = fopen(path, "rb");
    if (!f) {
        msr_set_errorf(errBuf, errBufLen, "cannot open local source %s: %s",
                       key ? key : path, strerror(errno));
        return 0;
    }
    if (fseeko(f, (off_t)byteStart, SEEK_SET) != 0) {
        msr_set_errorf(errBuf, errBufLen, "cannot seek local source %s to byte %llu: %s",
                       key ? key : path, byteStart, strerror(errno));
        fclose(f);
        return 0;
    }
    remaining = (size_t)byteLength;
    while (remaining > 0) {
        size_t n = fread(cursor, 1, remaining, f);
        if (n == 0) {
            if (ferror(f)) {
                msr_set_errorf(errBuf, errBufLen, "failed reading local source %s: %s",
                               key ? key : path, strerror(errno));
            } else {
                msr_set_errorf(errBuf, errBufLen,
                               "short local read for %s at byte %llu: missing %zu bytes",
                               key ? key : path, byteStart, remaining);
            }
            fclose(f);
            return 0;
        }
        cursor += n;
        remaining -= n;
    }
    fclose(f);
    return 1;
}

static CURL *msr_thread_curl_handle(char *errBuf, size_t errBufLen) {
    if (!msr_tls_curl) {
        msr_tls_curl = curl_easy_init();
        if (!msr_tls_curl) {
            msr_set_error(errBuf, errBufLen, "curl_easy_init failed");
            return NULL;
        }
    }
    return msr_tls_curl;
}

static int msr_download_exact_range(const char *url,
                                    const char *key,
                                    unsigned long long byteStart,
                                    unsigned long long byteLength,
                                    int retries,
                                    unsigned char *dst,
                                    char *errBuf,
                                    size_t errBufLen) {
    CURL *curl = NULL;
    CURLcode rc = CURLE_OK;
    long httpStatus = 0;
    int attempts = retries + 1;
    char rangeBuf[128];
    char curlErr[CURL_ERROR_SIZE] = {0};
    DownloadBuffer dl;

    memset(&dl, 0, sizeof(dl));
    dl.data = dst;
    dl.expected = (size_t)byteLength;

    snprintf(rangeBuf, sizeof(rangeBuf), "%llu-%llu",
             byteStart,
             byteStart + byteLength - 1ULL);

    curl = msr_thread_curl_handle(errBuf, errBufLen);
    if (!curl) {
        return 0;
    }

    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_RANGE, rangeBuf);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, msr_write_section_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dl);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErr);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "identity");
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

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
            dl.size == dl.expected) {
            return 1;
        }
        if (attempt + 1 >= attempts || !msr_retryable_range_failure(rc, httpStatus)) break;
        msr_sleep_ms(150L * (attempt + 1));
    }

    if (rc != CURLE_OK) {
        msr_set_errorf(
            errBuf, errBufLen,
            "range GET failed for %s bytes %s after %d attempt%s: %s",
            key ? key : url,
            rangeBuf,
            attempts,
            attempts == 1 ? "" : "s",
            curlErr[0] ? curlErr : curl_easy_strerror(rc)
        );
    } else if (httpStatus != 206L && httpStatus != 200L) {
        msr_set_errorf(
            errBuf, errBufLen,
            "unexpected HTTP status %ld for %s bytes %s",
            httpStatus,
            key ? key : url,
            rangeBuf
        );
    } else if (dl.overflow) {
        msr_set_errorf(
            errBuf, errBufLen,
            "range GET overflow for %s bytes %s",
            key ? key : url,
            rangeBuf
        );
    } else {
        msr_set_errorf(
            errBuf, errBufLen,
            "short range GET for %s bytes %s: got %zu of %zu bytes",
            key ? key : url,
            rangeBuf,
            dl.size,
            dl.expected
        );
    }
    return 0;
}

static int msr_validate_reader(const MultiSpanReader *reader, char *errBuf, size_t errBufLen) {
    unsigned long long cursor = 0;
    if (!reader) {
        msr_set_error(errBuf, errBufLen, "reader is null");
        return 0;
    }
    if (reader->logicalSize == 0) {
        msr_set_error(errBuf, errBufLen, "manifest logical_size must be > 0");
        return 0;
    }
    if (reader->rowBytes == 0) {
        msr_set_error(errBuf, errBufLen, "manifest row_bytes must be > 0");
        return 0;
    }
    if (reader->sourceCount <= 0 || reader->spanCount <= 0) {
        msr_set_error(errBuf, errBufLen, "manifest missing sources or spans");
        return 0;
    }
    for (int i = 0; i < reader->sourceCount; i++) {
        if (!reader->sources[i].url || !reader->sources[i].url[0]) {
            msr_set_errorf(errBuf, errBufLen, "source %d missing url", i);
            return 0;
        }
    }
    for (int i = 0; i < reader->spanCount; i++) {
        const MultiSpanSpan *span = &reader->spans[i];
        if (span->sourceId < 0 || span->sourceId >= reader->sourceCount) {
            msr_set_errorf(errBuf, errBufLen, "span %d references invalid source_id %d", i, span->sourceId);
            return 0;
        }
        if (span->byteLength == 0) {
            msr_set_errorf(errBuf, errBufLen, "span %d has zero byte_length", i);
            return 0;
        }
        if (i > 0 && span->logicalByteStart < reader->spans[i - 1].logicalByteStart) {
            msr_set_errorf(errBuf, errBufLen, "spans must be sorted by logical_byte_start (span %d)", i);
            return 0;
        }
    }
    for (int i = 0; i < reader->spanCount; i++) {
        const MultiSpanSpan *span = &reader->spans[i];
        unsigned long long spanEnd = 0;
        if (span->logicalByteStart != cursor) {
            if (span->logicalByteStart > cursor) {
                msr_set_errorf(errBuf, errBufLen,
                               "manifest has gap before logical byte %llu", cursor);
            } else {
                msr_set_errorf(errBuf, errBufLen,
                               "manifest spans overlap at logical byte %llu", span->logicalByteStart);
            }
            return 0;
        }
        spanEnd = span->logicalByteStart + span->byteLength;
        if (spanEnd < span->logicalByteStart) {
            msr_set_errorf(errBuf, errBufLen, "span %d logical range overflow", i);
            return 0;
        }
        if (spanEnd > reader->logicalSize) {
            msr_set_errorf(errBuf, errBufLen,
                           "span %d exceeds logical_size (%llu > %llu)",
                           i, spanEnd, reader->logicalSize);
            return 0;
        }
        cursor = spanEnd;
    }
    if (cursor != reader->logicalSize) {
        msr_set_errorf(errBuf, errBufLen,
                       "manifest coverage ends at %llu, expected logical_size %llu",
                       cursor, reader->logicalSize);
        return 0;
    }
    return 1;
}

int multispan_reader_open(MultiSpanReader *reader, const char *manifestPath, int retries,
                          char *errBuf, size_t errBufLen) {
    size_t rawSize = 0;
    char *raw = NULL;
    const char *p = NULL;
    if (!reader || !manifestPath) {
        msr_set_error(errBuf, errBufLen, "multispan_reader_open requires reader and manifestPath");
        return 0;
    }
    memset(reader, 0, sizeof(*reader));
    reader->retries = retries;
    raw = msr_read_file(manifestPath, &rawSize, errBuf, errBufLen);
    if (!raw) return 0;
    p = raw;
    if (!msr_expect_char(&p, '{', errBuf, errBufLen)) {
        free(raw);
        return 0;
    }
    while (*p) {
        char *key = NULL;
        msr_skip_ws(&p);
        if (*p == '}') {
            p++;
            break;
        }
        if (!msr_parse_string(&p, &key, errBuf, errBufLen)) {
            free(raw);
            return 0;
        }
        if (!msr_expect_char(&p, ':', errBuf, errBufLen)) {
            free(key);
            free(raw);
            return 0;
        }
        if (strcmp(key, "logical_size") == 0) {
            if (!msr_parse_ull(&p, &reader->logicalSize, errBuf, errBufLen)) {
                free(key);
                free(raw);
                return 0;
            }
        } else if (strcmp(key, "row_bytes") == 0) {
            if (!msr_parse_ull(&p, &reader->rowBytes, errBuf, errBufLen)) {
                free(key);
                free(raw);
                return 0;
            }
        } else if (strcmp(key, "solve_start") == 0) {
            unsigned long long value = 0;
            if (!msr_parse_ull(&p, &value, errBuf, errBufLen)) {
                free(key);
                free(raw);
                return 0;
            }
            reader->solveStart = (long long)value;
        } else if (strcmp(key, "solve_count") == 0) {
            unsigned long long value = 0;
            if (!msr_parse_ull(&p, &value, errBuf, errBufLen)) {
                free(key);
                free(raw);
                return 0;
            }
            reader->solveCount = (long long)value;
        } else if (strcmp(key, "sources") == 0) {
            if (!msr_expect_char(&p, '[', errBuf, errBufLen)) {
                free(key);
                free(raw);
                return 0;
            }
            msr_skip_ws(&p);
            if (*p != ']') {
                while (*p) {
                    MultiSpanSource source;
                    MultiSpanSource *grown = NULL;
                    if (!msr_parse_source_object(&p, &source, errBuf, errBufLen)) {
                        free(key);
                        free(raw);
                        return 0;
                    }
                    grown = (MultiSpanSource *)realloc(reader->sources, sizeof(MultiSpanSource) * (reader->sourceCount + 1));
                    if (!grown) {
                        free(source.url);
                        free(source.key);
                        free(key);
                        free(raw);
                        msr_set_error(errBuf, errBufLen, "out of memory growing sources");
                        return 0;
                    }
                    reader->sources = grown;
                    reader->sources[reader->sourceCount++] = source;
                    msr_skip_ws(&p);
                    if (*p == ']') break;
                    if (!msr_expect_char(&p, ',', errBuf, errBufLen)) {
                        free(key);
                        free(raw);
                        return 0;
                    }
                }
            }
            if (!msr_expect_char(&p, ']', errBuf, errBufLen)) {
                free(key);
                free(raw);
                return 0;
            }
        } else if (strcmp(key, "spans") == 0) {
            if (!msr_expect_char(&p, '[', errBuf, errBufLen)) {
                free(key);
                free(raw);
                return 0;
            }
            msr_skip_ws(&p);
            if (*p != ']') {
                while (*p) {
                    MultiSpanSpan span;
                    MultiSpanSpan *grown = NULL;
                    if (!msr_parse_span_object(&p, &span, errBuf, errBufLen)) {
                        free(key);
                        free(raw);
                        return 0;
                    }
                    grown = (MultiSpanSpan *)realloc(reader->spans, sizeof(MultiSpanSpan) * (reader->spanCount + 1));
                    if (!grown) {
                        free(key);
                        free(raw);
                        msr_set_error(errBuf, errBufLen, "out of memory growing spans");
                        return 0;
                    }
                    reader->spans = grown;
                    reader->spans[reader->spanCount++] = span;
                    msr_skip_ws(&p);
                    if (*p == ']') break;
                    if (!msr_expect_char(&p, ',', errBuf, errBufLen)) {
                        free(key);
                        free(raw);
                        return 0;
                    }
                }
            }
            if (!msr_expect_char(&p, ']', errBuf, errBufLen)) {
                free(key);
                free(raw);
                return 0;
            }
        } else {
            if (!msr_skip_value(&p, errBuf, errBufLen)) {
                free(key);
                free(raw);
                return 0;
            }
        }
        free(key);
        msr_skip_ws(&p);
        if (*p == '}') {
            p++;
            break;
        }
        if (!msr_expect_char(&p, ',', errBuf, errBufLen)) {
            free(raw);
            return 0;
        }
    }
    free(raw);
    if (!msr_validate_reader(reader, errBuf, errBufLen)) {
        multispan_reader_close(reader);
        return 0;
    }
    return 1;
}

int multispan_reader_read_exact(const MultiSpanReader *reader,
                                unsigned long long logicalOffset,
                                size_t length,
                                unsigned char *dst,
                                long *bytesDownloaded,
                                char *errBuf,
                                size_t errBufLen) {
    unsigned long long requestEnd = 0;
    unsigned long long cursor = 0;
    long totalDownloaded = 0;
    if (bytesDownloaded) *bytesDownloaded = 0;
    if (!reader || !dst) {
        msr_set_error(errBuf, errBufLen, "multispan_reader_read_exact requires reader and dst");
        return 0;
    }
    if (length == 0) {
        msr_set_error(errBuf, errBufLen, "multispan_reader_read_exact requires non-zero length");
        return 0;
    }
    requestEnd = logicalOffset + (unsigned long long)length;
    if (requestEnd > reader->logicalSize || requestEnd < logicalOffset) {
        msr_set_errorf(errBuf, errBufLen,
                       "logical read out of range: offset=%llu length=%zu logical_size=%llu",
                       logicalOffset, length, reader->logicalSize);
        return 0;
    }
    cursor = logicalOffset;
    for (int i = 0; i < reader->spanCount && cursor < requestEnd; i++) {
        const MultiSpanSpan *span = &reader->spans[i];
        unsigned long long spanStart = span->logicalByteStart;
        unsigned long long spanEnd = spanStart + span->byteLength;
        if (spanEnd <= cursor || spanStart >= requestEnd) continue;
        if (spanStart > cursor) {
            msr_set_errorf(errBuf, errBufLen,
                           "gap in logical coverage before offset %llu", cursor);
            return 0;
        }
        if (span->sourceId < 0 || span->sourceId >= reader->sourceCount) {
            msr_set_errorf(errBuf, errBufLen,
                           "span references invalid source_id %d", span->sourceId);
            return 0;
        }
        unsigned long long overlapStart = cursor;
        unsigned long long overlapEnd = spanEnd < requestEnd ? spanEnd : requestEnd;
        unsigned long long partLen = overlapEnd - overlapStart;
        unsigned long long physicalStart = span->byteStart + (overlapStart - spanStart);
        const MultiSpanSource *source = &reader->sources[span->sourceId];
        if (msr_starts_with(source->url, "file://")) {
            if (!msr_read_exact_file_range(
                    source->url,
                    source->key,
                    physicalStart,
                    partLen,
                    dst + (size_t)(overlapStart - logicalOffset),
                    errBuf,
                    errBufLen)) {
                return 0;
            }
        } else {
            if (!msr_download_exact_range(
                    source->url,
                    source->key,
                    physicalStart,
                    partLen,
                    reader->retries,
                    dst + (size_t)(overlapStart - logicalOffset),
                    errBuf,
                    errBufLen)) {
                return 0;
            }
        }
        totalDownloaded += (long)partLen;
        cursor = overlapEnd;
    }
    if (cursor != requestEnd) {
        msr_set_errorf(errBuf, errBufLen,
                       "logical range not fully covered: offset=%llu length=%zu covered_until=%llu",
                       logicalOffset, length, cursor);
        return 0;
    }
    if (bytesDownloaded) *bytesDownloaded = totalDownloaded;
    return 1;
}

void multispan_reader_thread_cleanup(void) {
    if (msr_tls_curl) {
        curl_easy_cleanup(msr_tls_curl);
        msr_tls_curl = NULL;
    }
}

void multispan_reader_close(MultiSpanReader *reader) {
    if (!reader) return;
    for (int i = 0; i < reader->sourceCount; i++) {
        free(reader->sources[i].url);
        free(reader->sources[i].key);
    }
    free(reader->sources);
    free(reader->spans);
    memset(reader, 0, sizeof(*reader));
}
