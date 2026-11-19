#ifndef PS5_DOWNLOADER_H
#define PS5_DOWNLOADER_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include "tinyurl.h"

typedef enum {
    DL_STATE_IDLE = 0,
    DL_STATE_RESOLVING,
    DL_STATE_DOWNLOADING,
    DL_STATE_PAUSED,
    DL_STATE_COMPLETED,
    DL_STATE_ERROR
} download_state_t;

#define DEFAULT_PS5_DOWNLOAD_DIR "/data/homebrew"

typedef struct {
    char short_code[64];
    char target_url[2048];
    char save_directory[256];
    char filename[256];
    char full_dest_path[512];
    
    uint64_t total_size;
    uint64_t current_downloaded;
    uint64_t resumed_offset;
    
    double speed_bytes_per_sec;
    int eta_seconds;
    float progress_percentage;
    
    download_state_t state;
    char status_message[256];
    
    int request_pause;
    int request_cancel;
    
    time_t start_time;
    time_t last_calc_time;
    uint64_t last_calc_bytes;
} download_task_t;

/* Common PS5 jailbreak directories for selector */
extern const char* PS5_STORAGE_DIRS[];
extern const int PS5_STORAGE_DIRS_COUNT;

/* Initializes a task with default parameters */
void downloader_task_init(download_task_t *task);

/* Sets the target directory (e.g. /data/homebrew) */
void downloader_set_directory(download_task_t *task, const char *dir_path);

/* Prepares download from tinyurl shortcode */
int downloader_prepare(download_task_t *task, const char *short_code);

/* Starts or resumes download thread */
int downloader_start(download_task_t *task);

/* Pauses current download */
void downloader_pause(download_task_t *task);

/* Resumes paused download */
void downloader_resume(download_task_t *task);

/* Cancels current download */
void downloader_cancel(download_task_t *task);

/* Formats byte counts into human readable string (e.g. "1.42 GB") */
void downloader_format_bytes(uint64_t bytes, char *buffer, size_t buf_size);

/* Formats seconds into ETA string (e.g. "02m 45s") */
void downloader_format_eta(int seconds, char *buffer, size_t buf_size);

#endif /* PS5_DOWNLOADER_H */
