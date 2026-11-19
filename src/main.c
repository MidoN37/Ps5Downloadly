#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "platform/ps5_hal.h"
#include "core/tinyurl.h"
#include "core/downloader.h"
#include "ui/ui.h"

int main(int argc, char **argv) {
    printf("[PS5DOWNLOADLY] Starting PS5 Downloader Homebrew...\n");
    
    ps5_ui_context_t ui_ctx;
    if (ps5_hal_init(&ui_ctx) != 0) {
        printf("[PS5DOWNLOADLY] Failed to initialize hardware abstraction layer.\n");
        return -1;
    }

    int running = 1;
    ps5_pad_state_t pad_state;

    while (running) {
        // 1. Poll Controller Inputs
        if (ps5_hal_poll_input(&pad_state) == 0) {
            uint32_t pressed = pad_state.buttons_pressed;

            if (ui_ctx.is_path_selector_open) {
                // Modal Navigation
                if (pressed & SCE_PAD_BUTTON_UP) {
                    ui_path_selector_prev(&ui_ctx);
                } else if (pressed & SCE_PAD_BUTTON_DOWN) {
                    ui_path_selector_next(&ui_ctx);
                } else if (pressed & SCE_PAD_BUTTON_CROSS) {
                    ui_path_selector_confirm(&ui_ctx);
                } else if (pressed & SCE_PAD_BUTTON_CIRCLE) {
                    ui_close_path_selector(&ui_ctx);
                }
            } else {
                // Main Screen Navigation
                if (pressed & SCE_PAD_BUTTON_UP) {
                    keyboard_move_up(&ui_ctx.keyboard);
                } else if (pressed & SCE_PAD_BUTTON_DOWN) {
                    keyboard_move_down(&ui_ctx.keyboard);
                } else if (pressed & SCE_PAD_BUTTON_LEFT) {
                    keyboard_move_left(&ui_ctx.keyboard);
                } else if (pressed & SCE_PAD_BUTTON_RIGHT) {
                    keyboard_move_right(&ui_ctx.keyboard);
                } else if (pressed & SCE_PAD_BUTTON_CROSS) {
                    kb_action_t act = keyboard_press_cross(&ui_ctx.keyboard);
                    if (act == KB_ACTION_SUBMIT) {
                        downloader_prepare(&ui_ctx.download_task, ui_ctx.keyboard.buffer);
                    }
                } else if (pressed & SCE_PAD_BUTTON_SQUARE) {
                    // Context-aware Square: Pause/Resume if downloading, otherwise Backspace
                    if (ui_ctx.download_task.state == DL_STATE_DOWNLOADING) {
                        downloader_pause(&ui_ctx.download_task);
                    } else if (ui_ctx.download_task.state == DL_STATE_PAUSED) {
                        downloader_resume(&ui_ctx.download_task);
                    } else {
                        keyboard_press_square(&ui_ctx.keyboard);
                    }
                } else if (pressed & SCE_PAD_BUTTON_TRIANGLE) {
                    ui_open_path_selector(&ui_ctx);
                } else if (pressed & SCE_PAD_BUTTON_OPTIONS) {
                    if (strlen(ui_ctx.keyboard.buffer) > 0) {
                        downloader_prepare(&ui_ctx.download_task, ui_ctx.keyboard.buffer);
                    }
                } else if (pressed & SCE_PAD_BUTTON_CIRCLE) {
                    if (ui_ctx.download_task.state == DL_STATE_DOWNLOADING) {
                        downloader_cancel(&ui_ctx.download_task);
                    } else {
                        // Exit requested
                        running = 0;
                    }
                }
            }
        }

        // 2. Update UI & Animation State
        ui_update(&ui_ctx);

        // 3. Render Graphics
        ui_render(&ui_ctx);

        // 4. Flip Framebuffer (Vsync)
        ps5_hal_flip_framebuffer();
        
        // Target ~60 FPS
        usleep(16666);
    }

    ps5_hal_cleanup();
    printf("[PS5DOWNLOADLY] Exiting safely.\n");
    return 0;
}
