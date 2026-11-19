#include "keyboard.h"
#include <string.h>
#include <ctype.h>

static const char* KB_LOWER[KB_MAX_ROWS][KB_MAX_COLS] = {
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "_"},
    {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "", ""},
    {"a", "s", "d", "f", "g", "h", "j", "k", "l", "", "", ""},
    {"z", "x", "c", "v", "b", "n", "m", ".", "", "", "", ""},
    {"SHIFT", "⌫ BACK", "CLEAR", "START ➔", "", "", "", "", "", "", "", ""}
};

static const char* KB_UPPER[KB_MAX_ROWS][KB_MAX_COLS] = {
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "_"},
    {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "", ""},
    {"A", "S", "D", "F", "G", "H", "J", "K", "L", "", "", ""},
    {"Z", "X", "C", "V", "B", "N", "M", ".", "", "", "", ""},
    {"shift", "⌫ BACK", "CLEAR", "START ➔", "", "", "", "", "", "", "", ""}
};

static const int ROW_COL_COUNTS[KB_MAX_ROWS] = {12, 10, 9, 8, 4};

void keyboard_init(ps5_keyboard_t *kb) {
    if (!kb) return;
    memset(kb, 0, sizeof(ps5_keyboard_t));
    kb->selected_row = 1; /* Start on 'q' */
    kb->selected_col = 0;
    kb->is_shift_active = 0;
    kb->cursor_pos = 0;
}

const char* keyboard_get_key_label(int row, int col, int is_shift) {
    if (row < 0 || row >= KB_MAX_ROWS || col < 0 || col >= ROW_COL_COUNTS[row]) {
        return "";
    }
    return is_shift ? KB_UPPER[row][col] : KB_LOWER[row][col];
}

void keyboard_move_up(ps5_keyboard_t *kb) {
    if (!kb) return;
    kb->selected_row = (kb->selected_row - 1 + KB_MAX_ROWS) % KB_MAX_ROWS;
    if (kb->selected_col >= ROW_COL_COUNTS[kb->selected_row]) {
        kb->selected_col = ROW_COL_COUNTS[kb->selected_row] - 1;
    }
}

void keyboard_move_down(ps5_keyboard_t *kb) {
    if (!kb) return;
    kb->selected_row = (kb->selected_row + 1) % KB_MAX_ROWS;
    if (kb->selected_col >= ROW_COL_COUNTS[kb->selected_row]) {
        kb->selected_col = ROW_COL_COUNTS[kb->selected_row] - 1;
    }
}

void keyboard_move_left(ps5_keyboard_t *kb) {
    if (!kb) return;
    int max_c = ROW_COL_COUNTS[kb->selected_row];
    kb->selected_col = (kb->selected_col - 1 + max_c) % max_c;
}

void keyboard_move_right(ps5_keyboard_t *kb) {
    if (!kb) return;
    int max_c = ROW_COL_COUNTS[kb->selected_row];
    kb->selected_col = (kb->selected_col + 1) % max_c;
}

kb_action_t keyboard_press_cross(ps5_keyboard_t *kb) {
    if (!kb) return KB_ACTION_NONE;
    
    int r = kb->selected_row;
    int c = kb->selected_col;
    
    if (r == 4) {
        if (c == 0) { /* Shift */
            kb->is_shift_active = !kb->is_shift_active;
            return KB_ACTION_SHIFT;
        } else if (c == 1) { /* Backspace */
            keyboard_press_square(kb);
            return KB_ACTION_BACKSPACE;
        } else if (c == 2) { /* Clear */
            keyboard_init(kb);
            return KB_ACTION_CLEAR;
        } else if (c == 3) { /* Start / Submit */
            keyboard_press_options(kb);
            return KB_ACTION_SUBMIT;
        }
    }
    
    /* Standard character key */
    const char *label = keyboard_get_key_label(r, c, kb->is_shift_active);
    if (strlen(label) == 1 && kb->cursor_pos + 1 < KB_BUFFER_SIZE) {
        kb->buffer[kb->cursor_pos++] = label[0];
        kb->buffer[kb->cursor_pos] = '\0';
        return KB_ACTION_CHAR;
    }
    return KB_ACTION_NONE;
}

void keyboard_press_square(ps5_keyboard_t *kb) {
    if (!kb || kb->cursor_pos == 0) return;
    kb->cursor_pos--;
    kb->buffer[kb->cursor_pos] = '\0';
}

void keyboard_press_triangle(ps5_keyboard_t *kb) {
    if (!kb) return;
    kb->is_shift_active = !kb->is_shift_active;
}

void keyboard_press_circle(ps5_keyboard_t *kb) {
    if (!kb) return;
    kb->is_cancelled = 1;
}

void keyboard_press_options(ps5_keyboard_t *kb) {
    if (!kb) return;
    if (kb->cursor_pos > 0) {
        kb->is_submitted = 1;
    }
}
