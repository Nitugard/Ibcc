/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Device.h"

#include <GLFW/glfw3.h>
#include <Os/Plugin.h>
#include <Os/Log.h>
#include <Os/Allocator.h>

#ifndef DEVICE_ASSERT

#include <assert.h>

#define DEVICE_ASSERT(e) ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif

typedef struct device_wnd {
    GLFWwindow *handle;
    int32_t width, height;
    device_cursor_state cursor_state;
    device_mouse_state mouse_state;

    void (*device_events_keyboard_callback)(device_key, device_key_state);

    void (*device_events_input_callback)(char);

    void (*device_events_mouse_callback)(device_mouse_state);
} device_wnd;

typedef struct device_timer {
    int64_t start_time;
} device_timer;

device_wnd wnd;

void glfw_character_callback(GLFWwindow *window, uint32_t codepoint);

void glfw_key_callback(GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods);

void glfw_cursor_pos_callback(GLFWwindow *window, double x, double y);


plg_desc req_plugins[] = {};

void plg_on_start(plg_info *info) {

    info->name = "Device";
    info->req_plugins = req_plugins;
    info->req_plugins_count = sizeof(req_plugins) / sizeof(plg_desc);
    info->version = 1;
}

//todo: device terminate
bool plg_on_load() {
    if (!glfwInit()) {
        LOG_ERROR("Failed to initialize device\n");
        return false;
    }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_CENTER_CURSOR, true);
    glfwWindowHint(GLFW_DOUBLEBUFFER, true);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Device", NULL, NULL);
    glfwMakeContextCurrent(window);
    DEVICE_ASSERT(window != NULL);

    if (window == NULL) {
        LOG_ERROR("Failed to create window\n");
        return false;
    }

    LOG_INFO("Glfw %s\n", glfwGetVersionString());

    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetCharCallback(window, glfw_character_callback);
    glfwSetCursorPosCallback(window, glfw_cursor_pos_callback);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    os_memset(&wnd, 0, sizeof(device_wnd));
    wnd.handle = window;
    wnd.device_events_input_callback = NULL;
    wnd.device_events_keyboard_callback = NULL;
    wnd.width = 800;
    wnd.height = 600;
    return true;
}

void plg_on_stop() {
    glfwDestroyWindow(wnd.handle);
    glfwTerminate();
}

struct device_mouse_state device_mouse_calc_state(double x, double y, double x1, double y1) {
    device_mouse_state state;
    state.dx = x1 - x;
    state.dy = y1 - y;
    state.x = x1;
    state.y = y1;
    return state;
}

double clamp(double x, double min, double max) {
    if (x > max) return max;
    if (x < min) return min;
    return x;
}

//workaround
GLFWcursor *glfw_blank_cursor() {
    int32_t w = 1;
    int32_t h = 1;;
    char pixels[w * h * 4];
    os_memset(pixels, 0, sizeof(pixels));
    GLFWimage image;
    image.width = w;
    image.height = h;
    image.pixels = pixels;
    return glfwCreateCursor(&image, 0, 0);
}

device_key glfw_key_to_device_key(int32_t key) {
    switch (key) {
        case GLFW_KEY_UNKNOWN :
            return DEVICE_KEY_UNKNOWN;
        case GLFW_KEY_SPACE :
            return DEVICE_KEY_SPACE;
        case GLFW_KEY_APOSTROPHE :
            return DEVICE_KEY_APOSTROPHE;
        case GLFW_KEY_COMMA :
            return DEVICE_KEY_COMMA;
        case GLFW_KEY_MINUS :
            return DEVICE_KEY_MINUS;
        case GLFW_KEY_PERIOD :
            return DEVICE_KEY_PERIOD;
        case GLFW_KEY_SLASH :
            return DEVICE_KEY_SLASH;
        case GLFW_KEY_0 :
            return DEVICE_KEY_0;
        case GLFW_KEY_1 :
            return DEVICE_KEY_1;
        case GLFW_KEY_2 :
            return DEVICE_KEY_2;
        case GLFW_KEY_3 :
            return DEVICE_KEY_3;
        case GLFW_KEY_4 :
            return DEVICE_KEY_4;
        case GLFW_KEY_5 :
            return DEVICE_KEY_5;
        case GLFW_KEY_6 :
            return DEVICE_KEY_6;
        case GLFW_KEY_7 :
            return DEVICE_KEY_7;
        case GLFW_KEY_8 :
            return DEVICE_KEY_8;
        case GLFW_KEY_9 :
            return DEVICE_KEY_9;
        case GLFW_KEY_SEMICOLON :
            return DEVICE_KEY_SEMICOLON;
        case GLFW_KEY_EQUAL :
            return DEVICE_KEY_EQUAL;
        case GLFW_KEY_A :
            return DEVICE_KEY_A;
        case GLFW_KEY_B :
            return DEVICE_KEY_B;
        case GLFW_KEY_C :
            return DEVICE_KEY_C;
        case GLFW_KEY_D :
            return DEVICE_KEY_D;
        case GLFW_KEY_E :
            return DEVICE_KEY_E;
        case GLFW_KEY_F :
            return DEVICE_KEY_F;
        case GLFW_KEY_G :
            return DEVICE_KEY_G;
        case GLFW_KEY_H :
            return DEVICE_KEY_H;
        case GLFW_KEY_I :
            return DEVICE_KEY_I;
        case GLFW_KEY_J :
            return DEVICE_KEY_J;
        case GLFW_KEY_K :
            return DEVICE_KEY_K;
        case GLFW_KEY_L :
            return DEVICE_KEY_L;
        case GLFW_KEY_M :
            return DEVICE_KEY_M;
        case GLFW_KEY_N :
            return DEVICE_KEY_N;
        case GLFW_KEY_O :
            return DEVICE_KEY_O;
        case GLFW_KEY_P :
            return DEVICE_KEY_P;
        case GLFW_KEY_Q :
            return DEVICE_KEY_Q;
        case GLFW_KEY_R :
            return DEVICE_KEY_R;
        case GLFW_KEY_S :
            return DEVICE_KEY_S;
        case GLFW_KEY_T :
            return DEVICE_KEY_T;
        case GLFW_KEY_U :
            return DEVICE_KEY_U;
        case GLFW_KEY_V :
            return DEVICE_KEY_V;
        case GLFW_KEY_W :
            return DEVICE_KEY_W;
        case GLFW_KEY_X :
            return DEVICE_KEY_X;
        case GLFW_KEY_Y :
            return DEVICE_KEY_Y;
        case GLFW_KEY_Z :
            return DEVICE_KEY_Z;
        case GLFW_KEY_LEFT_BRACKET :
            return DEVICE_KEY_LEFT_BRACKET;
        case GLFW_KEY_BACKSLASH :
            return DEVICE_KEY_BACKSLASH;
        case GLFW_KEY_RIGHT_BRACKET :
            return DEVICE_KEY_RIGHT_BRACKET;
        case GLFW_KEY_GRAVE_ACCENT :
            return DEVICE_KEY_GRAVE_ACCENT;
        case GLFW_KEY_WORLD_1 :
            return DEVICE_KEY_WORLD_1;
        case GLFW_KEY_WORLD_2 :
            return DEVICE_KEY_WORLD_2;
        case GLFW_KEY_ESCAPE :
            return DEVICE_KEY_ESCAPE;
        case GLFW_KEY_ENTER :
            return DEVICE_KEY_ENTER;
        case GLFW_KEY_TAB :
            return DEVICE_KEY_TAB;
        case GLFW_KEY_BACKSPACE :
            return DEVICE_KEY_BACKSPACE;
        case GLFW_KEY_INSERT :
            return DEVICE_KEY_INSERT;
        case GLFW_KEY_DELETE :
            return DEVICE_KEY_DELETE;
        case GLFW_KEY_RIGHT :
            return DEVICE_KEY_RIGHT;
        case GLFW_KEY_LEFT :
            return DEVICE_KEY_LEFT;
        case GLFW_KEY_DOWN :
            return DEVICE_KEY_DOWN;
        case GLFW_KEY_UP :
            return DEVICE_KEY_UP;
        case GLFW_KEY_PAGE_UP :
            return DEVICE_KEY_PAGE_UP;
        case GLFW_KEY_PAGE_DOWN :
            return DEVICE_KEY_PAGE_DOWN;
        case GLFW_KEY_HOME :
            return DEVICE_KEY_HOME;
        case GLFW_KEY_END :
            return DEVICE_KEY_END;
        case GLFW_KEY_CAPS_LOCK :
            return DEVICE_KEY_CAPS_LOCK;
        case GLFW_KEY_SCROLL_LOCK :
            return DEVICE_KEY_SCROLL_LOCK;
        case GLFW_KEY_NUM_LOCK :
            return DEVICE_KEY_NUM_LOCK;
        case GLFW_KEY_PRINT_SCREEN :
            return DEVICE_KEY_PRINT_SCREEN;
        case GLFW_KEY_PAUSE :
            return DEVICE_KEY_PAUSE;
        case GLFW_KEY_F1 :
            return DEVICE_KEY_F1;
        case GLFW_KEY_F2 :
            return DEVICE_KEY_F2;
        case GLFW_KEY_F3 :
            return DEVICE_KEY_F3;
        case GLFW_KEY_F4 :
            return DEVICE_KEY_F4;
        case GLFW_KEY_F5 :
            return DEVICE_KEY_F5;
        case GLFW_KEY_F6 :
            return DEVICE_KEY_F6;
        case GLFW_KEY_F7 :
            return DEVICE_KEY_F7;
        case GLFW_KEY_F8 :
            return DEVICE_KEY_F8;
        case GLFW_KEY_F9 :
            return DEVICE_KEY_F9;
        case GLFW_KEY_F10 :
            return DEVICE_KEY_F10;
        case GLFW_KEY_F11 :
            return DEVICE_KEY_F11;
        case GLFW_KEY_F12 :
            return DEVICE_KEY_F12;
        case GLFW_KEY_F13 :
            return DEVICE_KEY_F13;
        case GLFW_KEY_F14 :
            return DEVICE_KEY_F14;
        case GLFW_KEY_F15 :
            return DEVICE_KEY_F15;
        case GLFW_KEY_F16 :
            return DEVICE_KEY_F16;
        case GLFW_KEY_F17 :
            return DEVICE_KEY_F17;
        case GLFW_KEY_F18 :
            return DEVICE_KEY_F18;
        case GLFW_KEY_F19 :
            return DEVICE_KEY_F19;
        case GLFW_KEY_F20 :
            return DEVICE_KEY_F20;
        case GLFW_KEY_F21 :
            return DEVICE_KEY_F21;
        case GLFW_KEY_F22 :
            return DEVICE_KEY_F22;
        case GLFW_KEY_F23 :
            return DEVICE_KEY_F23;
        case GLFW_KEY_F24 :
            return DEVICE_KEY_F24;
        case GLFW_KEY_F25 :
            return DEVICE_KEY_F25;
        case GLFW_KEY_KP_0 :
            return DEVICE_KEY_KP_0;
        case GLFW_KEY_KP_1 :
            return DEVICE_KEY_KP_1;
        case GLFW_KEY_KP_2 :
            return DEVICE_KEY_KP_2;
        case GLFW_KEY_KP_3 :
            return DEVICE_KEY_KP_3;
        case GLFW_KEY_KP_4 :
            return DEVICE_KEY_KP_4;
        case GLFW_KEY_KP_5 :
            return DEVICE_KEY_KP_5;
        case GLFW_KEY_KP_6 :
            return DEVICE_KEY_KP_6;
        case GLFW_KEY_KP_7 :
            return DEVICE_KEY_KP_7;
        case GLFW_KEY_KP_8 :
            return DEVICE_KEY_KP_8;
        case GLFW_KEY_KP_9 :
            return DEVICE_KEY_KP_9;
        case GLFW_KEY_KP_DECIMAL :
            return DEVICE_KEY_KP_DECIMAL;
        case GLFW_KEY_KP_DIVIDE :
            return DEVICE_KEY_KP_DIVIDE;
        case GLFW_KEY_KP_MULTIPLY :
            return DEVICE_KEY_KP_MULTIPLY;
        case GLFW_KEY_KP_SUBTRACT :
            return DEVICE_KEY_KP_SUBTRACT;
        case GLFW_KEY_KP_ADD :
            return DEVICE_KEY_KP_ADD;
        case GLFW_KEY_KP_ENTER :
            return DEVICE_KEY_KP_ENTER;
        case GLFW_KEY_KP_EQUAL :
            return DEVICE_KEY_KP_EQUAL;
        case GLFW_KEY_LEFT_SHIFT :
            return DEVICE_KEY_LEFT_SHIFT;
        case GLFW_KEY_LEFT_CONTROL :
            return DEVICE_KEY_LEFT_CONTROL;
        case GLFW_KEY_LEFT_ALT :
            return DEVICE_KEY_LEFT_ALT;
        case GLFW_KEY_LEFT_SUPER :
            return DEVICE_KEY_LEFT_SUPER;
        case GLFW_KEY_RIGHT_SHIFT :
            return DEVICE_KEY_RIGHT_SHIFT;
        case GLFW_KEY_RIGHT_CONTROL :
            return DEVICE_KEY_RIGHT_CONTROL;
        case GLFW_KEY_RIGHT_ALT :
            return DEVICE_KEY_RIGHT_ALT;
        case GLFW_KEY_RIGHT_SUPER :
            return DEVICE_KEY_RIGHT_SUPER;
        case GLFW_KEY_MENU :
            return DEVICE_KEY_MENU;
        case GLFW_MOUSE_BUTTON_1 :
            return DEVICE_MOUSE_BUTTON_1;
        case GLFW_MOUSE_BUTTON_2 :
            return DEVICE_MOUSE_BUTTON_2;
        case GLFW_MOUSE_BUTTON_3 :
            return DEVICE_MOUSE_BUTTON_3;
        case GLFW_MOUSE_BUTTON_4 :
            return DEVICE_MOUSE_BUTTON_4;
        case GLFW_MOUSE_BUTTON_5 :
            return DEVICE_MOUSE_BUTTON_5;
        case GLFW_MOUSE_BUTTON_6 :
            return DEVICE_MOUSE_BUTTON_6;
        case GLFW_MOUSE_BUTTON_7 :
            return DEVICE_MOUSE_BUTTON_7;
        case GLFW_MOUSE_BUTTON_8 :
            return DEVICE_MOUSE_BUTTON_8;
        default:
            return DEVICE_KEY_UNKNOWN;
    }
}

int32_t device_key_to_glfw_key(device_key key) {

    switch (key) {
        case DEVICE_KEY_UNKNOWN :
            return GLFW_KEY_UNKNOWN;
        case DEVICE_KEY_SPACE :
            return GLFW_KEY_SPACE;
        case DEVICE_KEY_APOSTROPHE :
            return GLFW_KEY_APOSTROPHE;
        case DEVICE_KEY_COMMA :
            return GLFW_KEY_COMMA;
        case DEVICE_KEY_MINUS :
            return GLFW_KEY_MINUS;
        case DEVICE_KEY_PERIOD :
            return GLFW_KEY_PERIOD;
        case DEVICE_KEY_SLASH :
            return GLFW_KEY_SLASH;
        case DEVICE_KEY_0 :
            return GLFW_KEY_0;
        case DEVICE_KEY_1 :
            return GLFW_KEY_1;
        case DEVICE_KEY_2 :
            return GLFW_KEY_2;
        case DEVICE_KEY_3 :
            return GLFW_KEY_3;
        case DEVICE_KEY_4 :
            return GLFW_KEY_4;
        case DEVICE_KEY_5 :
            return GLFW_KEY_5;
        case DEVICE_KEY_6 :
            return GLFW_KEY_6;
        case DEVICE_KEY_7 :
            return GLFW_KEY_7;
        case DEVICE_KEY_8 :
            return GLFW_KEY_8;
        case DEVICE_KEY_9 :
            return GLFW_KEY_9;
        case DEVICE_KEY_SEMICOLON :
            return GLFW_KEY_SEMICOLON;
        case DEVICE_KEY_EQUAL :
            return GLFW_KEY_EQUAL;
        case DEVICE_KEY_A :
            return GLFW_KEY_A;
        case DEVICE_KEY_B :
            return GLFW_KEY_B;
        case DEVICE_KEY_C :
            return GLFW_KEY_C;
        case DEVICE_KEY_D :
            return GLFW_KEY_D;
        case DEVICE_KEY_E :
            return GLFW_KEY_E;
        case DEVICE_KEY_F :
            return GLFW_KEY_F;
        case DEVICE_KEY_G :
            return GLFW_KEY_G;
        case DEVICE_KEY_H :
            return GLFW_KEY_H;
        case DEVICE_KEY_I :
            return GLFW_KEY_I;
        case DEVICE_KEY_J :
            return GLFW_KEY_J;
        case DEVICE_KEY_K :
            return GLFW_KEY_K;
        case DEVICE_KEY_L :
            return GLFW_KEY_L;
        case DEVICE_KEY_M :
            return GLFW_KEY_M;
        case DEVICE_KEY_N :
            return GLFW_KEY_N;
        case DEVICE_KEY_O :
            return GLFW_KEY_O;
        case DEVICE_KEY_P :
            return GLFW_KEY_P;
        case DEVICE_KEY_Q :
            return GLFW_KEY_Q;
        case DEVICE_KEY_R :
            return GLFW_KEY_R;
        case DEVICE_KEY_S :
            return GLFW_KEY_S;
        case DEVICE_KEY_T :
            return GLFW_KEY_T;
        case DEVICE_KEY_U :
            return GLFW_KEY_U;
        case DEVICE_KEY_V :
            return GLFW_KEY_V;
        case DEVICE_KEY_W :
            return GLFW_KEY_W;
        case DEVICE_KEY_X :
            return GLFW_KEY_X;
        case DEVICE_KEY_Y :
            return GLFW_KEY_Y;
        case DEVICE_KEY_Z :
            return GLFW_KEY_Z;
        case DEVICE_KEY_LEFT_BRACKET :
            return GLFW_KEY_LEFT_BRACKET;
        case DEVICE_KEY_BACKSLASH :
            return GLFW_KEY_BACKSLASH;
        case DEVICE_KEY_RIGHT_BRACKET :
            return GLFW_KEY_RIGHT_BRACKET;
        case DEVICE_KEY_GRAVE_ACCENT :
            return GLFW_KEY_GRAVE_ACCENT;
        case DEVICE_KEY_WORLD_1 :
            return GLFW_KEY_WORLD_1;
        case DEVICE_KEY_WORLD_2 :
            return GLFW_KEY_WORLD_2;
        case DEVICE_KEY_ESCAPE :
            return GLFW_KEY_ESCAPE;
        case DEVICE_KEY_ENTER :
            return GLFW_KEY_ENTER;
        case DEVICE_KEY_TAB :
            return GLFW_KEY_TAB;
        case DEVICE_KEY_BACKSPACE :
            return GLFW_KEY_BACKSPACE;
        case DEVICE_KEY_INSERT :
            return GLFW_KEY_INSERT;
        case DEVICE_KEY_DELETE :
            return GLFW_KEY_DELETE;
        case DEVICE_KEY_RIGHT :
            return GLFW_KEY_RIGHT;
        case DEVICE_KEY_LEFT :
            return GLFW_KEY_LEFT;
        case DEVICE_KEY_DOWN :
            return GLFW_KEY_DOWN;
        case DEVICE_KEY_UP :
            return GLFW_KEY_UP;
        case DEVICE_KEY_PAGE_UP :
            return GLFW_KEY_PAGE_UP;
        case DEVICE_KEY_PAGE_DOWN :
            return GLFW_KEY_PAGE_DOWN;
        case DEVICE_KEY_HOME :
            return GLFW_KEY_HOME;
        case DEVICE_KEY_END :
            return GLFW_KEY_END;
        case DEVICE_KEY_CAPS_LOCK :
            return GLFW_KEY_CAPS_LOCK;
        case DEVICE_KEY_SCROLL_LOCK :
            return GLFW_KEY_SCROLL_LOCK;
        case DEVICE_KEY_NUM_LOCK :
            return GLFW_KEY_NUM_LOCK;
        case DEVICE_KEY_PRINT_SCREEN :
            return GLFW_KEY_PRINT_SCREEN;
        case DEVICE_KEY_PAUSE :
            return GLFW_KEY_PAUSE;
        case DEVICE_KEY_F1 :
            return GLFW_KEY_F1;
        case DEVICE_KEY_F2 :
            return GLFW_KEY_F2;
        case DEVICE_KEY_F3 :
            return GLFW_KEY_F3;
        case DEVICE_KEY_F4 :
            return GLFW_KEY_F4;
        case DEVICE_KEY_F5 :
            return GLFW_KEY_F5;
        case DEVICE_KEY_F6 :
            return GLFW_KEY_F6;
        case DEVICE_KEY_F7 :
            return GLFW_KEY_F7;
        case DEVICE_KEY_F8 :
            return GLFW_KEY_F8;
        case DEVICE_KEY_F9 :
            return GLFW_KEY_F9;
        case DEVICE_KEY_F10 :
            return GLFW_KEY_F10;
        case DEVICE_KEY_F11 :
            return GLFW_KEY_F11;
        case DEVICE_KEY_F12 :
            return GLFW_KEY_F12;
        case DEVICE_KEY_F13 :
            return GLFW_KEY_F13;
        case DEVICE_KEY_F14 :
            return GLFW_KEY_F14;
        case DEVICE_KEY_F15 :
            return GLFW_KEY_F15;
        case DEVICE_KEY_F16 :
            return GLFW_KEY_F16;
        case DEVICE_KEY_F17 :
            return GLFW_KEY_F17;
        case DEVICE_KEY_F18 :
            return GLFW_KEY_F18;
        case DEVICE_KEY_F19 :
            return GLFW_KEY_F19;
        case DEVICE_KEY_F20 :
            return GLFW_KEY_F20;
        case DEVICE_KEY_F21 :
            return GLFW_KEY_F21;
        case DEVICE_KEY_F22 :
            return GLFW_KEY_F22;
        case DEVICE_KEY_F23 :
            return GLFW_KEY_F23;
        case DEVICE_KEY_F24 :
            return GLFW_KEY_F24;
        case DEVICE_KEY_F25 :
            return GLFW_KEY_F25;
        case DEVICE_KEY_KP_0 :
            return GLFW_KEY_KP_0;
        case DEVICE_KEY_KP_1 :
            return GLFW_KEY_KP_1;
        case DEVICE_KEY_KP_2 :
            return GLFW_KEY_KP_2;
        case DEVICE_KEY_KP_3 :
            return GLFW_KEY_KP_3;
        case DEVICE_KEY_KP_4 :
            return GLFW_KEY_KP_4;
        case DEVICE_KEY_KP_5 :
            return GLFW_KEY_KP_5;
        case DEVICE_KEY_KP_6 :
            return GLFW_KEY_KP_6;
        case DEVICE_KEY_KP_7 :
            return GLFW_KEY_KP_7;
        case DEVICE_KEY_KP_8 :
            return GLFW_KEY_KP_8;
        case DEVICE_KEY_KP_9 :
            return GLFW_KEY_KP_9;
        case DEVICE_KEY_KP_DECIMAL :
            return GLFW_KEY_KP_DECIMAL;
        case DEVICE_KEY_KP_DIVIDE :
            return GLFW_KEY_KP_DIVIDE;
        case DEVICE_KEY_KP_MULTIPLY :
            return GLFW_KEY_KP_MULTIPLY;
        case DEVICE_KEY_KP_SUBTRACT :
            return GLFW_KEY_KP_SUBTRACT;
        case DEVICE_KEY_KP_ADD :
            return GLFW_KEY_KP_ADD;
        case DEVICE_KEY_KP_ENTER :
            return GLFW_KEY_KP_ENTER;
        case DEVICE_KEY_KP_EQUAL :
            return GLFW_KEY_KP_EQUAL;
        case DEVICE_KEY_LEFT_SHIFT :
            return GLFW_KEY_LEFT_SHIFT;
        case DEVICE_KEY_LEFT_CONTROL :
            return GLFW_KEY_LEFT_CONTROL;
        case DEVICE_KEY_LEFT_ALT :
            return GLFW_KEY_LEFT_ALT;
        case DEVICE_KEY_LEFT_SUPER :
            return GLFW_KEY_LEFT_SUPER;
        case DEVICE_KEY_RIGHT_SHIFT :
            return GLFW_KEY_RIGHT_SHIFT;
        case DEVICE_KEY_RIGHT_CONTROL :
            return GLFW_KEY_RIGHT_CONTROL;
        case DEVICE_KEY_RIGHT_ALT :
            return GLFW_KEY_RIGHT_ALT;
        case DEVICE_KEY_RIGHT_SUPER :
            return GLFW_KEY_RIGHT_SUPER;
        case DEVICE_KEY_MENU :
            return GLFW_KEY_MENU;
        case DEVICE_MOUSE_BUTTON_1 :
            return GLFW_MOUSE_BUTTON_1;
        case DEVICE_MOUSE_BUTTON_2 :
            return GLFW_MOUSE_BUTTON_2;
        case DEVICE_MOUSE_BUTTON_3 :
            return GLFW_MOUSE_BUTTON_3;
        case DEVICE_MOUSE_BUTTON_4 :
            return GLFW_MOUSE_BUTTON_4;
        case DEVICE_MOUSE_BUTTON_5 :
            return GLFW_MOUSE_BUTTON_5;
        case DEVICE_MOUSE_BUTTON_6 :
            return GLFW_MOUSE_BUTTON_6;
        case DEVICE_MOUSE_BUTTON_7 :
            return GLFW_MOUSE_BUTTON_7;
        case DEVICE_MOUSE_BUTTON_8 :
            return GLFW_MOUSE_BUTTON_8;
        default:
            return GLFW_KEY_UNKNOWN;
    }
}

device_key_state glfw_key_action_to_device_state(int32_t action) {
    switch (action) {
        case GLFW_PRESS:
            return DEVICE_PRESS_ACTION;
        case GLFW_REPEAT:
            return DEVICE_HOLD_ACTION;
        case GLFW_RELEASE:
            return DEVICE_RELEASE_ACTION;
        default:
            return DEVICE_UNKNOWN_ACTION;
    }
}

void glfw_character_callback(GLFWwindow *window, uint32_t codepoint) {
    if (wnd.device_events_input_callback != NULL) {
        wnd.device_events_input_callback((char) codepoint);
    }
}

void glfw_key_callback(GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {

    if (wnd.device_events_keyboard_callback != NULL) {
        wnd.device_events_keyboard_callback(glfw_key_to_device_key(key), glfw_key_action_to_device_state(action));
    }
}

void glfw_cursor_pos_callback(GLFWwindow *window, double x, double y) {
    if (wnd.device_events_mouse_callback != NULL) {
        wnd.device_events_mouse_callback(wnd.mouse_state);
    }
}

device_key_state device_events_get_key(device_key key) {
    return glfw_key_action_to_device_state(glfwGetKey(wnd.handle, device_key_to_glfw_key(key)));
}

device_mouse_state device_events_get_mouse() {
    return wnd.mouse_state;
}

void device_window_refresh() {
    glfwSwapBuffers(wnd.handle);
}

void device_events_poll(void) {

    glfwPollEvents();
}

bool device_window_valid() {
    if (wnd.handle == NULL)
        return false;
    if (glfwWindowShouldClose(wnd.handle))
        return false;

    return true;
}


void device_events_keys_set_callback(void (*callback)(device_key, device_key_state)) {

    wnd.device_events_keyboard_callback = callback;
}

void device_events_input_set_callback(void (*callback)(char)) {
    wnd.device_events_input_callback = callback;
}


void device_window_close() {
    glfwSetWindowShouldClose(wnd.handle, true);
}

void device_window_cursor_set_state(const device_cursor_state *state) {

//    if(wnd->cursor_state.centered != state->centered);
//    if(wnd->cursor_state.wrap != state->wrap);
//    if(wnd->cursor_state.confined != state->confined);

    if (!state->visible) {
        glfwSetCursor(wnd.handle, glfw_blank_cursor());
    } else {
        glfwSetCursor(wnd.handle, NULL);
    }


    wnd.cursor_state = *state;
}


device_cursor_state device_window_cursor_get_state() {
    return wnd.cursor_state;
}

void device_events_mouse_set_callback(void (*callback)(device_mouse_state)) {
    wnd.device_events_mouse_callback = callback;
}


void device_window_cursor_update() {


    device_cursor_state state = wnd.cursor_state;
    device_mouse_state *mouse_state = &(wnd.mouse_state);
    int32_t width = wnd.width;
    int32_t height = wnd.height;
    double x, y;
    glfwGetCursorPos(wnd.handle, &x, &y);

    if (state.centered) {
        x = width / 2.0;
        y = height / 2.0;
    } else {
        if (x < 0 || x > width || y < 0 || y > height) {
            if (state.confined) {
                x = clamp(x, 0, width);
                y = clamp(y, 0, height);
            } else if (state.wrap) {
                while (x > width) x -= width;
                while (x < 0) x += width;
                while (y > height) y -= height;
                while (y < 0) y += height;
            }
        }
    }

    //mouse state delta calculations should not be affected by this transformation!
    mouse_state->x = x;
    mouse_state->y = y;
    glfwSetCursorPos(wnd.handle, mouse_state->x, mouse_state->y);

}

void device_window_mouse_update() {

    double x, y;

    glfwGetCursorPos(wnd.handle, &x, &y);
    wnd.mouse_state = device_mouse_calc_state(wnd.mouse_state.x, wnd.mouse_state.y, x, y);
}

double device_get_time() {
    return glfwGetTime();
}

