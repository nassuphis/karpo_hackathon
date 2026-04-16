#ifndef MULTISPAN_READER_H
#define MULTISPAN_READER_H

#include <stddef.h>

typedef struct {
    int id;
    char *url;
    char *key;
} MultiSpanSource;

typedef struct {
    int sourceId;
    unsigned long long logicalByteStart;
    unsigned long long byteStart;
    unsigned long long byteLength;
} MultiSpanSpan;

typedef struct {
    MultiSpanSource *sources;
    int sourceCount;
    MultiSpanSpan *spans;
    int spanCount;
    unsigned long long logicalSize;
    unsigned long long rowBytes;
    long long solveStart;
    long long solveCount;
    int retries;
} MultiSpanReader;

int multispan_reader_open(MultiSpanReader *reader, const char *manifestPath, int retries,
                          char *errBuf, size_t errBufLen);

int multispan_reader_read_exact(const MultiSpanReader *reader,
                                unsigned long long logicalOffset,
                                size_t length,
                                unsigned char *dst,
                                long *bytesDownloaded,
                                char *errBuf,
                                size_t errBufLen);

void multispan_reader_thread_cleanup(void);
void multispan_reader_close(MultiSpanReader *reader);

#endif
