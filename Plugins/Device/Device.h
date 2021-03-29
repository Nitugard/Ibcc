/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

 //Todo: resizing, fullscreen, framebuffer


#ifndef FIXEDPHYSICS_DEVICE_H
#define FIXEDPHYSICS_DEVICE_H

#include "Common.h"


typedef struct device_wnd_desc{
    i32 width;
    i32 height;
    bool fullscreen;
    bool async;
    const char* name;
} device_wnd_desc;

typedef enum device_key {
    DEVICE_KEY_UNKNOWN,
    DEVICE_KEY_SPACE,
    DEVICE_KEY_APOSTROPHE,
    DEVICE_KEY_COMMA,
    DEVICE_KEY_MINUS,
    DEVICE_KEY_PERIOD,
    DEVICE_KEY_SLASH,
    DEVICE_KEY_0,
    DEVICE_KEY_1,
    DEVICE_KEY_2,
    DEVICE_KEY_3,
    DEVICE_KEY_4,
    DEVICE_KEY_5,
    DEVICE_KEY_6,
    DEVICE_KEY_7,
    DEVICE_KEY_8,
    DEVICE_KEY_9,
    DEVICE_KEY_SEMICOLON,
    DEVICE_KEY_EQUAL,
    DEVICE_KEY_A,
    DEVICE_KEY_B,
    DEVICE_KEY_C,
    DEVICE_KEY_D,
    DEVICE_KEY_E,
    DEVICE_KEY_F,
    DEVICE_KEY_G,
    DEVICE_KEY_H,
    DEVICE_KEY_I,
    DEVICE_KEY_J,
    DEVICE_KEY_K,
    DEVICE_KEY_L,
    DEVICE_KEY_M,
    DEVICE_KEY_N,
    DEVICE_KEY_O,
    DEVICE_KEY_P,
    DEVICE_KEY_Q,
    DEVICE_KEY_R,
    DEVICE_KEY_S,
    DEVICE_KEY_T,
    DEVICE_KEY_U,
    DEVICE_KEY_V,
    DEVICE_KEY_W,
    DEVICE_KEY_X,
    DEVICE_KEY_Y,
    DEVICE_KEY_Z,
    DEVICE_KEY_LEFT_BRACKET,
    DEVICE_KEY_BACKSLASH,
    DEVICE_KEY_RIGHT_BRACKET,
    DEVICE_KEY_GRAVE_ACCENT,
    DEVICE_KEY_WORLD_1,
    DEVICE_KEY_WORLD_2,
    DEVICE_KEY_ESCAPE,
    DEVICE_KEY_ENTER,
    DEVICE_KEY_TAB,
    DEVICE_KEY_BACKSPACE,
    DEVICE_KEY_INSERT,
    DEVICE_KEY_DELETE,
    DEVICE_KEY_RIGHT,
    DEVICE_KEY_LEFT,
    DEVICE_KEY_DOWN,
    DEVICE_KEY_UP,
    DEVICE_KEY_PAGE_UP,
    DEVICE_KEY_PAGE_DOWN,
    DEVICE_KEY_HOME,
    DEVICE_KEY_END,
    DEVICE_KEY_CAPS_LOCK,
    DEVICE_KEY_SCROLL_LOCK,
    DEVICE_KEY_NUM_LOCK,
    DEVICE_KEY_PRINT_SCREEN,
    DEVICE_KEY_PAUSE,
    DEVICE_KEY_F1,
    DEVICE_KEY_F2,
    DEVICE_KEY_F3,
    DEVICE_KEY_F4,
    DEVICE_KEY_F5,
    DEVICE_KEY_F6,
    DEVICE_KEY_F7,
    DEVICE_KEY_F8,
    DEVICE_KEY_F9,
    DEVICE_KEY_F10,
    DEVICE_KEY_F11,
    DEVICE_KEY_F12,
    DEVICE_KEY_F13,
    DEVICE_KEY_F14,
    DEVICE_KEY_F15,
    DEVICE_KEY_F16,
    DEVICE_KEY_F17,
    DEVICE_KEY_F18,
    DEVICE_KEY_F19,
    DEVICE_KEY_F20,
    DEVICE_KEY_F21,
    DEVICE_KEY_F22,
    DEVICE_KEY_F23,
    DEVICE_KEY_F24,
    DEVICE_KEY_F25,
    DEVICE_KEY_KP_0,
    DEVICE_KEY_KP_1,
    DEVICE_KEY_KP_2,
    DEVICE_KEY_KP_3,
    DEVICE_KEY_KP_4,
    DEVICE_KEY_KP_5,
    DEVICE_KEY_KP_6,
    DEVICE_KEY_KP_7,
    DEVICE_KEY_KP_8,
    DEVICE_KEY_KP_9,
    DEVICE_KEY_KP_DECIMAL,
    DEVICE_KEY_KP_DIVIDE,
    DEVICE_KEY_KP_MULTIPLY,
    DEVICE_KEY_KP_SUBTRACT,
    DEVICE_KEY_KP_ADD,
    DEVICE_KEY_KP_ENTER,
    DEVICE_KEY_KP_EQUAL,
    DEVICE_KEY_LEFT_SHIFT,
    DEVICE_KEY_LEFT_CONTROL,
    DEVICE_KEY_LEFT_ALT,
    DEVICE_KEY_LEFT_SUPER,
    DEVICE_KEY_RIGHT_SHIFT,
    DEVICE_KEY_RIGHT_CONTROL,
    DEVICE_KEY_RIGHT_ALT,
    DEVICE_KEY_RIGHT_SUPER,
    DEVICE_KEY_MENU,
    DEVICE_MOUSE_BUTTON_1,
    DEVICE_MOUSE_BUTTON_2,
    DEVICE_MOUSE_BUTTON_3,
    DEVICE_MOUSE_BUTTON_4,
    DEVICE_MOUSE_BUTTON_5,
    DEVICE_MOUSE_BUTTON_6,
    DEVICE_MOUSE_BUTTON_7,
    DEVICE_MOUSE_BUTTON_8,
} device_key;
typedef enum device_key_state {
    DEVICE_UNKNOWN_ACTION,
    DEVICE_PRESS_ACTION,
    DEVICE_RELEASE_ACTION,
    DEVICE_HOLD_ACTION
} device_key_state;

typedef struct device_mouse_state{
    double x, y;
    double dx, dy;
} device_mouse_state;

typedef struct device_cursor_state{
    bool confined;
    bool centered;
    bool visible;
    bool wrap;
} device_cursor_state;

API void device_events_poll(void);

API void device_events_keys_set_callback(void(*)(device_key, device_key_state));
API void device_events_mouse_set_callback(void(*)(device_mouse_state));
API void device_events_input_set_callback(void(*)(char));

API device_key_state device_events_get_key(device_key);
API device_mouse_state device_events_get_mouse();

API f64 device_get_time();

API void device_window_resize(i32, i32);
API void device_window_close();
API void device_window_refresh();
API bool device_window_valid();

API void device_window_cursor_set_state(device_cursor_state const*);
API device_cursor_state device_window_cursor_get_state();

API void device_window_mouse_update();
API void device_window_cursor_update();

#endif //FIXEDPHYSICS_DEVICE_H
