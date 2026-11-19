#include "ui.h"
#include <string.h>
#include <stdio.h>

/* Built-in minimal 8x12 bitmap font for standalone PS5 framebuffer rendering */
static const unsigned char FONT_DEFAULT[128][12] = {
    [' '] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    ['-'] = {0x00,0x00,0x00,0x00,0x00,0x7E,0x7E,0x00,0x00,0x00,0x00,0x00},
    ['_'] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x7E,0x00},
    ['.'] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00},
    ['/'] = {0x02,0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00,0x00,0x00,0x00},
    [':'] = {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00},
    ['0'] = {0x3C,0x66,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00,0x00,0x00,0x00},
    ['1'] = {0x18,0x38,0x18,0x18,0x18,0x18,0x18,0x7E,0x00,0x00,0x00,0x00},
    ['2'] = {0x3C,0x66,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00,0x00,0x00,0x00},
    ['3'] = {0x3C,0x66,0x06,0x1C,0x06,0x06,0x66,0x3C,0x00,0x00,0x00,0x00},
    ['4'] = {0x0C,0x1C,0x34,0x64,0x7E,0x04,0x04,0x0E,0x00,0x00,0x00,0x00},
    ['5'] = {0x7E,0x60,0x7C,0x06,0x06,0x06,0x66,0x3C,0x00,0x00,0x00,0x00},
    ['6'] = {0x1C,0x30,0x60,0x7C,0x66,0x66,0x66,0x3C,0x00,0x00,0x00,0x00},
    ['7'] = {0x7E,0x06,0x0C,0x18,0x30,0x30,0x30,0x30,0x00,0x00,0x00,0x00},
    ['8'] = {0x3C,0x66,0x66,0x3C,0x66,0x66,0x66,0x3C,0x00,0x00,0x00,0x00},
    ['9'] = {0x3C,0x66,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00,0x00,0x00,0x00},
    ['A'] = {0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x66,0x00,0x00,0x00,0x00},
    ['B'] = {0x7C,0x66,0x66,0x7C,0x66,0x66,0x66,0x7C,0x00,0x00,0x00,0x00},
    ['C'] = {0x3C,0x66,0x60,0x60,0x60,0x60,0x66,0x3C,0x00,0x00,0x00,0x00},
    ['D'] = {0x78,0x6C,0x66,0x66,0x66,0x66,0x6C,0x78,0x00,0x00,0x00,0x00},
    ['E'] = {0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x7E,0x00,0x00,0x00,0x00},
    ['F'] = {0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x60,0x00,0x00,0x00,0x00},
    ['G'] = {0x3C,0x66,0x60,0x6E,0x66,0x66,0x66,0x3A,0x00,0x00,0x00,0x00},
    ['H'] = {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x66,0x00,0x00,0x00,0x00},
    ['I'] = {0x3C,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00},
    ['J'] = {0x0E,0x06,0x06,0x06,0x06,0x66,0x66,0x3C,0x00,0x00,0x00,0x00},
    ['K'] = {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x66,0x00,0x00,0x00,0x00},
    ['L'] = {0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00,0x00,0x00,0x00},
    ['M'] = {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x63,0x00,0x00,0x00,0x00},
    ['N'] = {0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x66,0x00,0x00,0x00,0x00},
    ['O'] = {0x3C,0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00,0x00,0x00,0x00},
    ['P'] = {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x60,0x00,0x00,0x00,0x00},
    ['Q'] = {0x3C,0x66,0x66,0x66,0x66,0x6A,0x3C,0x0E,0x00,0x00,0x00,0x00},
    ['R'] = {0x7C,0x66,0x66,0x7C,0x78,0x6C,0x66,0x66,0x00,0x00,0x00,0x00},
    ['S'] = {0x3C,0x66,0x60,0x3C,0x06,0x06,0x66,0x3C,0x00,0x00,0x00,0x00},
    ['T'] = {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00},
    ['U'] = {0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00,0x00,0x00,0x00},
    ['V'] = {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x18,0x00,0x00,0x00,0x00},
    ['W'] = {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x63,0x00,0x00,0x00,0x00},
    ['X'] = {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x66,0x00,0x00,0x00,0x00},
    ['Y'] = {0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00},
    ['Z'] = {0x7E,0x06,0x0C,0x18,0x30,0x60,0x60,0x7E,0x00,0x00,0x00,0x00},
};

static void draw_pixel(ps5_ui_context_t *ctx, int x, int y, uint32_t color) {
    if (!ctx || !ctx->framebuffer || x < 0 || x >= ctx->width || y < 0 || y >= ctx->height) return;
    ctx->framebuffer[y * ctx->width + x] = color;
}

static void draw_rect(ps5_ui_context_t *ctx, int x, int y, int w, int h, uint32_t color) {
    for (int j = y; j < y + h; j++) {
        for (int i = x; i < x + w; i++) {
            draw_pixel(ctx, i, j, color);
        }
    }
}

static void draw_rect_outline(ps5_ui_context_t *ctx, int x, int y, int w, int h, int thickness, uint32_t color) {
    draw_rect(ctx, x, y, w, thickness, color);
    draw_rect(ctx, x, y + h - thickness, w, thickness, color);
    draw_rect(ctx, x, y, thickness, h, color);
    draw_rect(ctx, x + w - thickness, y, thickness, h, color);
}

static void draw_char(ps5_ui_context_t *ctx, int x, int y, char c, int scale, uint32_t color) {
    unsigned char uc = (unsigned char)c;
    if (uc >= 'a' && uc <= 'z') uc -= 32; /* Use uppercase glyphs if available */
    if (uc > 127) return;

    for (int row = 0; row < 12; row++) {
        unsigned char bits = FONT_DEFAULT[uc][row];
        for (int col = 0; col < 8; col++) {
            if ((bits >> (7 - col)) & 1) {
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        draw_pixel(ctx, x + col * scale + sx, y + row * scale + sy, color);
                    }
                }
            }
        }
    }
}

static void draw_string(ps5_ui_context_t *ctx, int x, int y, const char *str, int scale, uint32_t color) {
    if (!str) return;
    int cur_x = x;
    while (*str) {
        draw_char(ctx, cur_x, y, *str, scale, color);
        cur_x += 8 * scale + scale;
        str++;
    }
}

void ui_init(ps5_ui_context_t *ctx, uint32_t *fb, int w, int h) {
    if (!ctx) return;
    memset(ctx, 0, sizeof(ps5_ui_context_t));
    ctx->framebuffer = fb;
    ctx->width = w;
    ctx->height = h;
    keyboard_init(&ctx->keyboard);
    downloader_task_init(&ctx->download_task);
    ctx->selected_path_index = 0;
}

void ui_update(ps5_ui_context_t *ctx) {
    if (!ctx) return;
    ctx->anim_tick++;
}

void ui_open_path_selector(ps5_ui_context_t *ctx) {
    if (!ctx) return;
    ctx->is_path_selector_open = 1;
    ctx->active_modal = MODAL_PATH_SELECTOR;
}

void ui_close_path_selector(ps5_ui_context_t *ctx) {
    if (!ctx) return;
    ctx->is_path_selector_open = 0;
    ctx->active_modal = MODAL_NONE;
}

void ui_path_selector_next(ps5_ui_context_t *ctx) {
    if (!ctx) return;
    ctx->selected_path_index = (ctx->selected_path_index + 1) % PS5_STORAGE_DIRS_COUNT;
}

void ui_path_selector_prev(ps5_ui_context_t *ctx) {
    if (!ctx) return;
    ctx->selected_path_index = (ctx->selected_path_index - 1 + PS5_STORAGE_DIRS_COUNT) % PS5_STORAGE_DIRS_COUNT;
}

void ui_path_selector_confirm(ps5_ui_context_t *ctx) {
    if (!ctx) return;
    downloader_set_directory(&ctx->download_task, PS5_STORAGE_DIRS[ctx->selected_path_index]);
    ui_close_path_selector(ctx);
}

void ui_render(ps5_ui_context_t *ctx) {
    if (!ctx || !ctx->framebuffer) return;

    /* 1. Background fill */
    draw_rect(ctx, 0, 0, ctx->width, ctx->height, COLOR_BG);

    /* 2. Top Header */
    draw_rect(ctx, 60, 40, ctx->width - 120, 70, COLOR_PANEL);
    draw_rect_outline(ctx, 60, 40, ctx->width - 120, 70, 2, COLOR_PANEL_BORDER);
    draw_string(ctx, 90, 56, "PS5DOWNLOADLY", 2, COLOR_ACCENT_CYAN);
    draw_string(ctx, 420, 68, "| High-Speed Direct Downloader", 1, COLOR_TEXT_MUTED);

    /* 3. TinyURL Input Card */
    int card_y = 130;
    draw_rect(ctx, 60, card_y, ctx->width - 120, 110, COLOR_PANEL);
    draw_rect_outline(ctx, 60, card_y, ctx->width - 120, 110, 2, COLOR_PANEL_BORDER);
    
    draw_string(ctx, 90, card_y + 15, "ENTER SHORTCODE (Only the code after tinyurl.com/)", 1, COLOR_TEXT_MUTED);
    
    /* Input box */
    int box_x = 90, box_y = card_y + 40, box_w = 1100, box_h = 50;
    draw_rect(ctx, box_x, box_y, box_w, box_h, 0xFF0D141F);
    draw_rect_outline(ctx, box_x, box_y, box_w, box_h, 2, COLOR_FOCUS);
    
    draw_string(ctx, box_x + 15, box_y + 16, "https://tinyurl.com/", 2, COLOR_TEXT_MUTED);
    int prefix_len = 20 * 18;
    draw_string(ctx, box_x + 15 + prefix_len, box_y + 16, ctx->keyboard.buffer, 2, COLOR_ACCENT_CYAN);
    
    /* Blinking Cursor */
    if ((ctx->anim_tick / 30) % 2 == 0) {
        int cur_x = box_x + 15 + prefix_len + (int)strlen(ctx->keyboard.buffer) * 18;
        draw_rect(ctx, cur_x, box_y + 14, 4, 24, COLOR_ACCENT_CYAN);
    }

    /* 4. Save Directory Selector Card */
    int dir_card_x = 1240, dir_card_w = ctx->width - 120 - 1180;
    draw_rect(ctx, dir_card_x, card_y, dir_card_w, 110, COLOR_PANEL);
    draw_rect_outline(ctx, dir_card_x, card_y, dir_card_w, 2, COLOR_PANEL_BORDER);
    draw_string(ctx, dir_card_x + 20, card_y + 15, "SAVE LOCATION (Press △ to Change)", 1, COLOR_TEXT_MUTED);
    
    draw_rect(ctx, dir_card_x + 20, card_y + 45, dir_card_w - 40, 42, 0xFF0D141F);
    draw_rect_outline(ctx, dir_card_x + 20, card_y + 45, dir_card_w - 40, 1, COLOR_ACCENT_BLUE);
    draw_string(ctx, dir_card_x + 35, card_y + 57, ctx->download_task.save_directory, 1, COLOR_TEXT_WHITE);

    /* 5. Virtual Keyboard Grid */
    int kb_y = 260;
    draw_rect(ctx, 60, kb_y, ctx->width - 120, 360, COLOR_PANEL);
    draw_rect_outline(ctx, 60, kb_y, ctx->width - 120, 360, 2, COLOR_PANEL_BORDER);
    
    for (int r = 0; r < KB_MAX_ROWS; r++) {
        for (int c = 0; c < KB_MAX_COLS; c++) {
            const char *label = keyboard_get_key_label(r, c, ctx->keyboard.is_shift_active);
            if (strlen(label) == 0) continue;
            
            int key_w = (r == 4) ? 220 : 64;
            int key_h = 50;
            int key_x = 90 + c * (key_w + 14);
            int key_y = kb_y + 20 + r * (key_h + 16);
            
            int is_focused = (ctx->keyboard.selected_row == r && ctx->keyboard.selected_col == c);
            uint32_t k_bg = is_focused ? COLOR_FOCUS : 0xFF192437;
            uint32_t k_border = is_focused ? COLOR_ACCENT_CYAN : COLOR_PANEL_BORDER;
            
            draw_rect(ctx, key_x, key_y, key_w, key_h, k_bg);
            draw_rect_outline(ctx, key_x, key_y, key_w, key_h, is_focused ? 3 : 1, k_border);
            
            int text_x = key_x + (key_w - (int)strlen(label) * 12) / 2;
            int text_y = key_y + (key_h - 14) / 2;
            draw_string(ctx, text_x, text_y, label, 1, is_focused ? COLOR_TEXT_WHITE : COLOR_TEXT_MUTED);
        }
    }

    /* 6. Active Download & Progress Card */
    int dl_y = 640;
    draw_rect(ctx, 60, dl_y, ctx->width - 120, 270, COLOR_PANEL);
    draw_rect_outline(ctx, 60, dl_y, ctx->width - 120, 270, 2, COLOR_PANEL_BORDER);
    
    draw_string(ctx, 90, dl_y + 20, "STATUS & DOWNLOAD MONITOR", 1, COLOR_TEXT_MUTED);
    draw_string(ctx, 90, dl_y + 45, ctx->download_task.status_message, 2, COLOR_TEXT_WHITE);
    
    /* Progress bar */
    int pb_x = 90, pb_y = dl_y + 90, pb_w = ctx->width - 180, pb_h = 36;
    draw_rect(ctx, pb_x, pb_y, pb_w, pb_h, 0xFF0A0F1A);
    draw_rect_outline(ctx, pb_x, pb_y, pb_w, pb_h, 2, COLOR_PANEL_BORDER);
    
    float pct = ctx->download_task.progress_percentage;
    if (pct > 100.0f) pct = 100.0f;
    int fill_w = (int)((pb_w - 4) * (pct / 100.0f));
    if (fill_w > 0) {
        draw_rect(ctx, pb_x + 2, pb_y + 2, fill_w, pb_h - 4, COLOR_ACCENT_CYAN);
    }
    
    /* Metrics Row */
    char metrics_buf[256];
    char size_cur[32], size_tot[32], eta_str[32];
    downloader_format_bytes(ctx->download_task.current_downloaded, size_cur, sizeof(size_cur));
    downloader_format_bytes(ctx->download_task.total_size, size_tot, sizeof(size_tot));
    downloader_format_eta(ctx->download_task.eta_seconds, eta_str, sizeof(eta_str));
    
    snprintf(metrics_buf, sizeof(metrics_buf), 
             "PROGRESS: %.1f%%  |  TRANSFERRED: %s / %s  |  SPEED: %.2f MB/s  |  ETA: %s",
             pct, size_cur, size_tot, ctx->download_task.speed_bytes_per_sec / (1024.0 * 1024.0), eta_str);
    draw_string(ctx, 90, dl_y + 145, metrics_buf, 1, COLOR_TEXT_MUTED);

    /* 7. Bottom Controller Legend Bar */
    int foot_y = ctx->height - 70;
    draw_rect(ctx, 0, foot_y, ctx->width, 70, 0xFF080B10);
    draw_rect_outline(ctx, 0, foot_y, ctx->width, 1, COLOR_PANEL_BORDER);
    
    draw_string(ctx, 90, foot_y + 25, "(X) Enter Key", 1, COLOR_ACCENT_CYAN);
    draw_string(ctx, 300, foot_y + 25, "(Square) Backspace / Pause", 1, COLOR_TEXT_WHITE);
    draw_string(ctx, 650, foot_y + 25, "(Triangle) Change Save Path", 1, COLOR_SUCCESS);
    draw_string(ctx, 1020, foot_y + 25, "(Options) Start Download", 1, COLOR_ACCENT_BLUE);
    draw_string(ctx, 1380, foot_y + 25, "(O) Cancel", 1, COLOR_ERROR);

    /* 8. Path Selector Modal overlay if opened */
    if (ctx->is_path_selector_open) {
        /* Semi-transparent dimmed background */
        draw_rect(ctx, 0, 0, ctx->width, ctx->height, 0xCC000000);
        
        int modal_w = 800, modal_h = 420;
        int modal_x = (ctx->width - modal_w) / 2;
        int modal_y = (ctx->height - modal_h) / 2;
        
        draw_rect(ctx, modal_x, modal_y, modal_w, modal_h, COLOR_PANEL);
        draw_rect_outline(ctx, modal_x, modal_y, modal_w, modal_h, 3, COLOR_ACCENT_CYAN);
        
        draw_string(ctx, modal_x + 40, modal_y + 30, "CHOOSE SAVE DESTINATION", 2, COLOR_ACCENT_CYAN);
        draw_string(ctx, modal_x + 40, modal_y + 70, "Select storage path on jailbroken PS5:", 1, COLOR_TEXT_MUTED);
        
        for (int i = 0; i < PS5_STORAGE_DIRS_COUNT; i++) {
            int item_y = modal_y + 110 + i * 50;
            int is_sel = (i == ctx->selected_path_index);
            
            draw_rect(ctx, modal_x + 40, item_y, modal_w - 80, 42, is_sel ? COLOR_FOCUS : 0xFF0E1521);
            draw_rect_outline(ctx, modal_x + 40, item_y, modal_w - 80, 1, is_sel ? COLOR_ACCENT_CYAN : COLOR_PANEL_BORDER);
            
            char line[128];
            snprintf(line, sizeof(line), "%s %s %s", 
                     is_sel ? ">" : " ", 
                     PS5_STORAGE_DIRS[i], 
                     (i == 0) ? "(Default)" : "");
            draw_string(ctx, modal_x + 60, item_y + 14, line, 1, is_sel ? COLOR_TEXT_WHITE : COLOR_TEXT_MUTED);
        }
        
        draw_string(ctx, modal_x + 40, modal_y + 375, "(D-Pad Up/Down) Select  |  (X) Confirm  |  (O) Cancel", 1, COLOR_TEXT_MUTED);
    }
}
