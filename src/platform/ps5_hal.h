#ifndef PS5_HAL_H
#define PS5_HAL_H

#include <stdint.h>
#include "../ui/ui.h"

/* Standard Sony Pad Button definitions */
#define SCE_PAD_BUTTON_OPTIONS   0x00000008
#define SCE_PAD_BUTTON_UP        0x00000010
#define SCE_PAD_BUTTON_RIGHT     0x00000020
#define SCE_PAD_BUTTON_DOWN      0x00000040
#define SCE_PAD_BUTTON_LEFT      0x00000080
#define SCE_PAD_BUTTON_TRIANGLE  0x00001000
#define SCE_PAD_BUTTON_CIRCLE    0x00002000
#define SCE_PAD_BUTTON_CROSS     0x00004000
#define SCE_PAD_BUTTON_SQUARE    0x00008000
#define SCE_PAD_BUTTON_L1        0x00000400
#define SCE_PAD_BUTTON_R1        0x00000800

typedef struct {
    uint32_t buttons;
    uint32_t buttons_pressed; /* newly pressed this frame */
    int left_stick_x;
    int left_stick_y;
} ps5_pad_state_t;

int ps5_hal_init(ps5_ui_context_t *ui_ctx);
int ps5_hal_poll_input(ps5_pad_state_t *out_state);
void ps5_hal_flip_framebuffer(void);
void ps5_hal_cleanup(void);

#endif /* PS5_HAL_H */
