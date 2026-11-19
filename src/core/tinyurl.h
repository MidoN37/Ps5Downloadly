#ifndef PS5_TINYURL_H
#define PS5_TINYURL_H

#include <stddef.h>

#define TINYURL_BASE_DOMAIN "tinyurl.com"
#define TINYURL_BASE_PREFIX "https://tinyurl.com/"

typedef struct {
    char clean_code[64];        /* e.g. "XsDrt12" */
    char full_request_url[256]; /* e.g. "https://tinyurl.com/XsDrt12" */
    char resolved_url[2048];    /* Target direct download link */
    char detected_filename[256];/* Inferred from Content-Disposition or URL path */
    long long content_length;   /* In bytes (-1 if chunked / unknown) */
    int http_status_code;       /* 200, 301, 302, etc. */
    int is_resumable;           /* 1 if server supports Accept-Ranges: bytes */
    char error_msg[256];
} tinyurl_info_t;

/* Sanitizes user input: strips whitespace, leading slashes, or full tinyurl URL */
void tinyurl_sanitize_code(const char *input, char *output, size_t max_len);

/* Resolves shortcode into direct download URL and metadata */
int tinyurl_resolve(const char *short_code, tinyurl_info_t *out_info);

#endif /* PS5_TINYURL_H */
