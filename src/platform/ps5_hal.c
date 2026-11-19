#include "ps5_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#ifdef __PS5__
#include <ps5/kernel.h>

typedef int32_t SceUserServiceUserId;

typedef struct {
    uint8_t x;
    uint8_t y;
} ScePadAnalogStick;

typedef struct {
    uint32_t buttons;
    ScePadAnalogStick leftStick;
    ScePadAnalogStick rightStick;
    uint8_t analogButtons[8];
    uint8_t padding[16];
} ScePadData;

#define SCE_PAD_PORT_TYPE_STANDARD 0
#define SCE_VIDEO_OUT_BUS_TYPE_MAIN 0
#define SCE_VIDEO_OUT_FLIP_MODE_VSYNC 1

int sceUserServiceInitialize(void *params);
int sceUserServiceGetInitialUser(SceUserServiceUserId *userId);
int scePadInit(void);
int scePadOpen(SceUserServiceUserId userId, int type, int index, void *param);
int scePadReadState(int handle, ScePadData *data);
int scePadClose(int handle);
int sceVideoOutOpen(int userId, int type, int index, void *param);
int sceVideoOutSubmitFlip(int handle, int bufferIndex, int flipMode, int64_t flipArg);
int sceVideoOutClose(int handle);

static int g_video_handle = -1;
static int g_pad_handle = -1;
static uint32_t *g_fb_ptr = NULL;
static uint32_t g_prev_buttons = 0;

int ps5_hal_init(ps5_ui_context_t *ui_ctx) {
    // 1. Initialize User Service
    sceUserServiceInitialize(NULL);
    SceUserServiceUserId userId = 0;
    sceUserServiceGetInitialUser(&userId);

    // 2. Initialize DualSense Pad
    scePadInit();
    g_pad_handle = scePadOpen(userId, SCE_PAD_PORT_TYPE_STANDARD, 0, NULL);
    if (g_pad_handle < 0) {
        printf("[PS5-HAL] Notice: DualSense pad handle: %d\n", g_pad_handle);
    }

    // 3. Setup Video Out Direct Framebuffer
    g_video_handle = sceVideoOutOpen(0, SCE_VIDEO_OUT_BUS_TYPE_MAIN, 0, NULL);
    
    // Allocate 1080p RGBA framebuffer (1920 * 1080 * 4 bytes)
    size_t fb_size = SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t);
    g_fb_ptr = (uint32_t*)malloc(fb_size);
    if (!g_fb_ptr) {
        printf("[PS5-HAL] Error: failed to allocate framebuffer.\n");
        return -1;
    }
    memset(g_fb_ptr, 0, fb_size);

    ui_init(ui_ctx, g_fb_ptr, SCREEN_WIDTH, SCREEN_HEIGHT);
    return 0;
}

int ps5_hal_poll_input(ps5_pad_state_t *out_state) {
    if (!out_state || g_pad_handle < 0) return -1;
    
    ScePadData pad_data;
    memset(&pad_data, 0, sizeof(pad_data));
    int ret = scePadReadState(g_pad_handle, &pad_data);
    if (ret == 0) {
        uint32_t cur = pad_data.buttons;
        out_state->buttons = cur;
        out_state->buttons_pressed = cur & (~g_prev_buttons);
        g_prev_buttons = cur;
        
        out_state->left_stick_x = (int)pad_data.leftStick.x - 128;
        out_state->left_stick_y = (int)pad_data.leftStick.y - 128;
        return 0;
    }
    return -1;
}

void ps5_hal_flip_framebuffer(void) {
    if (g_video_handle >= 0 && g_fb_ptr) {
        sceVideoOutSubmitFlip(g_video_handle, 0, SCE_VIDEO_OUT_FLIP_MODE_VSYNC, 0);
    }
}

void ps5_hal_cleanup(void) {
    if (g_pad_handle >= 0) {
        scePadClose(g_pad_handle);
    }
    if (g_video_handle >= 0) {
        sceVideoOutClose(g_video_handle);
    }
    if (g_fb_ptr) {
        free(g_fb_ptr);
        g_fb_ptr = NULL;
    }
}

#else
/* Host / Simulator fallback */
static uint32_t *g_mock_fb = NULL;

int ps5_hal_init(ps5_ui_context_t *ui_ctx) {
    size_t fb_size = SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t);
    g_mock_fb = (uint32_t*)malloc(fb_size);
    if (!g_mock_fb) return -1;
    memset(g_mock_fb, 0, fb_size);
    ui_init(ui_ctx, g_mock_fb, SCREEN_WIDTH, SCREEN_HEIGHT);
    return 0;
}

int ps5_hal_poll_input(ps5_pad_state_t *out_state) {
    if (!out_state) return -1;
    memset(out_state, 0, sizeof(ps5_pad_state_t));
    return 0;
}

void ps5_hal_flip_framebuffer(void) {
}

void ps5_hal_cleanup(void) {
    if (g_mock_fb) {
        free(g_mock_fb);
        g_mock_fb = NULL;
    }
}
#endif
