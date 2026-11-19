#include "tinyurl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void tinyurl_sanitize_code(const char *input, char *output, size_t max_len) {
    if (!input || !output || max_len == 0) return;
    
    // Trim leading whitespace
    while (isspace((unsigned char)*input)) input++;
    
    // Check if user accidentally pasted full URL
    const char *prefix1 = "https://tinyurl.com/";
    const char *prefix2 = "http://tinyurl.com/";
    const char *prefix3 = "tinyurl.com/";
    
    if (strncmp(input, prefix1, strlen(prefix1)) == 0) {
        input += strlen(prefix1);
    } else if (strncmp(input, prefix2, strlen(prefix2)) == 0) {
        input += strlen(prefix2);
    } else if (strncmp(input, prefix3, strlen(prefix3)) == 0) {
        input += strlen(prefix3);
    }
    
    // Trim leading slashes
    while (*input == '/') input++;
    
    // Copy alphanumeric/hyphen/underscore characters
    size_t idx = 0;
    while (*input && idx + 1 < max_len && !isspace((unsigned char)*input) && *input != '?' && *input != '#') {
        output[idx++] = *input++;
    }
    output[idx] = '\0';
}

static void extract_filename_from_url(const char *url, char *filename, size_t max_len) {
    if (!url || !filename || max_len == 0) return;
    
    // Find last '/'
    const char *slash = strrchr(url, '/');
    const char *candidate = slash ? slash + 1 : url;
    
    // Copy until '?', '#', or end of string
    size_t i = 0;
    while (candidate[i] && candidate[i] != '?' && candidate[i] != '#' && i + 1 < max_len) {
        filename[i] = candidate[i];
        i++;
    }
    filename[i] = '\0';
    
    // If empty or invalid, fallback
    if (strlen(filename) == 0 || strcmp(filename, "/") == 0) {
        snprintf(filename, max_len, "downloaded_file.bin");
    }
}

static void parse_content_disposition(const char *header_val, char *filename, size_t max_len) {
    const char *fn_tag = "filename=";
    const char *found = strstr(header_val, fn_tag);
    if (!found) return;
    
    found += strlen(fn_tag);
    if (*found == '"' || *found == '\'') found++;
    
    size_t i = 0;
    while (found[i] && found[i] != '"' && found[i] != '\'' && found[i] != ';' && found[i] != '\r' && found[i] != '\n' && i + 1 < max_len) {
        filename[i] = found[i];
        i++;
    }
    filename[i] = '\0';
}

int tinyurl_resolve(const char *short_code, tinyurl_info_t *out_info) {
    if (!short_code || !out_info) return -1;
    
    memset(out_info, 0, sizeof(tinyurl_info_t));
    tinyurl_sanitize_code(short_code, out_info->clean_code, sizeof(out_info->clean_code));
    
    if (strlen(out_info->clean_code) == 0) {
        snprintf(out_info->error_msg, sizeof(out_info->error_msg), "Error: TinyURL code cannot be empty.");
        return -1;
    }
    
    snprintf(out_info->full_request_url, sizeof(out_info->full_request_url), 
             "%s%s", TINYURL_BASE_PREFIX, out_info->clean_code);
    out_info->content_length = -1;
    out_info->is_resumable = 1; /* Default to checking on download */

    extract_filename_from_url(out_info->full_request_url, out_info->detected_filename, sizeof(out_info->detected_filename));
    parse_content_disposition("", out_info->detected_filename, sizeof(out_info->detected_filename));

    return 0;
}
