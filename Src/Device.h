/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBCWEB_DEVICE_H
#define IBCWEB_DEVICE_H

#include <stdbool.h>
#include <stdint.h>

#define LOG_BUFFER_SIZE 1024

#ifndef IBC_API
#define IBC_API extern
#endif

typedef enum device_joystick_trigger_type {
    JOYSTICK_TRIGGER_MENU,
    JOYSTICK_TRIGGER_DESELECT,
    JOYSTICK_TRIGGER_SELECT,
    JOYSTICK_TRIGGER_RISE,
    JOYSTICK_TRIGGER_FALL,
    JOYSTICK_TRIGGER_0,
    JOYSTICK_TRIGGER_1,
    JOYSTICK_TRIGGER_2,
    JOYSTICK_TRIGGER_COUNT
} device_joystick_trigger_type;

typedef enum device_joystick_trigger_state {
    JOYSTICK_TRIGGER_PRESS = 1,
    JOYSTICK_TRIGGER_RELEASE = 2,
} device_joystick_trigger_state;

typedef struct device_joystick_pointer {
    float x, y;
    float dx, dy;
    double scroll_y, scroll_x;
    double scroll_dx, scroll_dy;

} device_joystick_pointer;

typedef struct device_joystick_axis {
    float value;
    float raw;
} device_joystick_axis;

typedef struct device_mouse_state{
    bool lmb_press, lmb_release;
    bool rmb_press, rmb_release;
    bool mmb_press, mmb_release;
} device_mouse_state;

typedef struct device_joystick {
    device_joystick_pointer pointer;
    device_joystick_axis horiz;
    device_joystick_axis vert;
    device_mouse_state mouse;
} device_joystick;


typedef void(*device_log_callback)(char const* log, bool error);
typedef struct device_data *device_handle;
typedef void *device_file_handle;

/*
 * Device management.
 */

IBC_API bool device_init(int32_t version_major, int32_t version_minor);
IBC_API void device_terminate();
IBC_API void device_log_callback_set(device_log_callback callback);
IBC_API device_handle device_new(const char* name, int32_t width, int32_t height, bool vsync, bool fullscreen, bool resizable);
IBC_API void device_delete(device_handle handle);
IBC_API void device_set_current(device_handle handle);
IBC_API void device_update_events();
IBC_API void device_refresh();
IBC_API void device_window_close();
IBC_API void device_window_cursor_set(bool centered, bool visible);
IBC_API bool device_window_valid();
IBC_API void device_window_dimensions_get(int32_t* width, int32_t* height);
IBC_API void* device_window_handle();


/*
 * File
 */

IBC_API device_file_handle device_file_open(const char* path, const char* mode);
IBC_API void device_file_size(device_file_handle handle, int32_t* size);
IBC_API int32_t device_file_read(device_file_handle handle, int32_t offset, int32_t length, void* buffer);
IBC_API void device_file_write(device_file_handle handle, int32_t offset, int32_t length, void const* buffer);
IBC_API void device_file_close(device_file_handle handle);
IBC_API void* device_file_read_text(const char* path);

/*
 * Events
 */

IBC_API double device_dt_get();
IBC_API double device_time_get();
IBC_API device_joystick device_joystick_get();

#endif //IBCWEB_DEVICE_H
