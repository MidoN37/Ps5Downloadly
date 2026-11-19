#include "downloader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

const char* PS5_STORAGE_DIRS[] = {
    "/data/homebrew",
    "/data",
    "/user/home",
    "/mnt/usb0",
    "/mnt/usb1"
};
const int PS5_STORAGE_DIRS_COUNT = 5;

void downloader_task_init(download_task_t *task) {
    if (!task) return;
    memset(task, 0, sizeof(download_task_t));
    snprintf(task->save_directory, sizeof(task->save_directory), "%s", DEFAULT_PS5_DOWNLOAD_DIR);
    task->state = DL_STATE_IDLE;
    snprintf(task->status_message, sizeof(task->status_message), "Ready. Enter TinyURL code to start.");
}

void downloader_set_directory(download_task_t *task, const char *dir_path) {
    if (!task || !dir_path) return;
    snprintf(task->save_directory, sizeof(task->save_directory), "%s", dir_path);
}

void downloader_format_bytes(uint64_t bytes, char *buffer, size_t buf_size) {
    if (!buffer || buf_size == 0) return;
    const double KB = 1024.0;
    const double MB = KB * 1024.0;
    const double GB = MB * 1024.0;
    
    if (bytes >= (uint64_t)GB) {
        snprintf(buffer, buf_size, "%.2f GB", (double)bytes / GB);
    } else if (bytes >= (uint64_t)MB) {
        snprintf(buffer, buf_size, "%.2f MB", (double)bytes / MB);
    } else if (bytes >= (uint64_t)KB) {
        snprintf(buffer, buf_size, "%.2f KB", (double)bytes / KB);
    } else {
        snprintf(buffer, buf_size, "%llu B", (unsigned long long)bytes);
    }
}

void downloader_format_eta(int seconds, char *buffer, size_t buf_size) {
    if (!buffer || buf_size == 0) return;
    if (seconds < 0) {
        snprintf(buffer, buf_size, "--:--");
        return;
    }
    
    int hrs = seconds / 3600;
    int mins = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    if (hrs > 0) {
        snprintf(buffer, buf_size, "%02dh %02dm %02ds", hrs, mins, secs);
    } else {
        snprintf(buffer, buf_size, "%02dm %02ds", mins, secs);
    }
}

int downloader_prepare(download_task_t *task, const char *short_code) {
    if (!task || !short_code) return -1;
    
    tinyurl_sanitize_code(short_code, task->short_code, sizeof(task->short_code));
    if (strlen(task->short_code) == 0) {
        snprintf(task->status_message, sizeof(task->status_message), "Invalid code.");
        task->state = DL_STATE_ERROR;
        return -1;
    }
    
    task->state = DL_STATE_RESOLVING;
    snprintf(task->status_message, sizeof(task->status_message), "Resolving tinyurl.com/%s...", task->short_code);
    return 0;
}

void downloader_pause(download_task_t *task) {
    if (!task) return;
    if (task->state == DL_STATE_DOWNLOADING) {
        task->request_pause = 1;
        task->state = DL_STATE_PAUSED;
        snprintf(task->status_message, sizeof(task->status_message), "Download paused.");
    }
}

void downloader_resume(download_task_t *task) {
    if (!task) return;
    if (task->state == DL_STATE_PAUSED) {
        task->request_pause = 0;
        task->state = DL_STATE_DOWNLOADING;
        snprintf(task->status_message, sizeof(task->status_message), "Resuming download...");
    }
}

void downloader_cancel(download_task_t *task) {
    if (!task) return;
    task->request_cancel = 1;
    task->state = DL_STATE_IDLE;
    snprintf(task->status_message, sizeof(task->status_message), "Download cancelled.");
}
