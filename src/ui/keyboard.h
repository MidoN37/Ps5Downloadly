#ifndef PS5_KEYBOARD_H
#define PS5_KEYBOARD_H

#include <stddef.h>

#define KB_MAX_ROWS 5
#define KB_MAX_COLS 12
#define KB_BUFFER_SIZE 64

typedef enum {
    KB_ACTION_NONE = 0,
    KB_ACTION_CHAR,
    KB_ACTION_SHIFT,
    KB_ACTION_SPACE,
    KB_ACTION_BACKSPACE,
    KB_ACTION_CLEAR,
    KB_ACTION_SUBMIT,
    KB_ACTION_CANCEL
} kb_action_t;

typedef struct {
    int selected_row;
    int selected_col;
    int is_shift_active;
    
    char buffer[KB_BUFFER_SIZE];
    size_t cursor_pos;
    
    int is_submitted;
    int is_cancelled;
} ps5_keyboard_t;

void keyboard_init(ps5_keyboard_t *kb);
void keyboard_move_up(ps5_keyboard_t *kb);
void keyboard_move_down(ps5_keyboard_t *kb);
void keyboard_move_left(ps5_keyboard_t *kb);
void keyboard_move_right(ps5_keyboard_t *kb);

kb_action_t keyboard_press_cross(ps5_keyboard_t *kb);    /* Select key */
void keyboard_press_square(ps5_keyboard_t *kb);          /* Backspace */
void keyboard_press_triangle(ps5_keyboard_t *kb);        /* Toggle Shift / Clear */
void keyboard_press_circle(ps5_keyboard_t *kb);          /* Cancel */
void keyboard_press_options(ps5_keyboard_t *kb);         /* Submit */

const char* keyboard_get_key_label(int row, int col, int is_shift);

#endif /* PS5_KEYBOARD_H */
