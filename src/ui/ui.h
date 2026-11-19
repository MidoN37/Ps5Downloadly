#ifndef PS5_UI_H
#define PS5_UI_H

#include <stdint.h>
#include "keyboard.h"
#include "../core/downloader.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

/* PS5 Palette (ARGB 32-bit format: 0xAARRGGBB) */
#define COLOR_BG            0xFF0B0E14  /* Deep Midnight */
#define COLOR_PANEL         0xFF141C2B  /* Obsidian Blue */
#define COLOR_PANEL_BORDER  0xFF223048  /* Subtle Divider */
#define COLOR_ACCENT_CYAN   0xFF00F0FF  /* Neon PS5 Cyan Glow */
#define COLOR_ACCENT_BLUE   0xFF0070D1  /* PlayStation Vibrant Blue */
#define COLOR_FOCUS         0xFF2D5FFF  /* Active Focus Glow */
#define COLOR_TEXT_WHITE    0xFFFFFFFF  /* Crisp White */
#define COLOR_TEXT_MUTED    0xFF8E9DAF  /* Dim Slate */
#define COLOR_SUCCESS       0xFF00E676  /* Mint Green */
#define COLOR_WARNING       0xFFFFB300  /* Amber Warning */
#define COLOR_ERROR         0xFFFF5252  /* Coral Red */

typedef enum {
    MODAL_NONE = 0,
    MODAL_PATH_SELECTOR
} modal_state_t;

typedef struct {
    uint32_t *framebuffer;
    int width;
    int height;
    
    ps5_keyboard_t keyboard;
    download_task_t download_task;
    
    modal_state_t active_modal;
    int selected_path_index;
    
    int anim_tick;
    int is_path_selector_open;
} ps5_ui_context_t;

void ui_init(ps5_ui_context_t *ctx, uint32_t *fb, int w, int h);
void ui_update(ps5_ui_context_t *ctx);
void ui_render(ps5_ui_context_t *ctx);

/* Modal triggers */
void ui_open_path_selector(ps5_ui_context_t *ctx);
void ui_close_path_selector(ps5_ui_context_t *ctx);
void ui_path_selector_next(ps5_ui_context_t *ctx);
void ui_path_selector_prev(ps5_ui_context_t *ctx);
void ui_path_selector_confirm(ps5_ui_context_t *ctx);

#endif /* PS5_UI_H */
