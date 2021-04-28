/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */

#ifndef DEVICE_H
#define DEVICE_H

#include <stdbool.h>
#include <stdint.h>

#ifndef API
#define API
#endif

typedef enum device_joystick_trigger_type {
    JOYSTICK_TRIGGER_MENU,
    JOYSTICK_TRIGGER_DESELECT,
    JOYSTICK_TRIGGER_SELECT,
    JOYSTICK_TRIGGER_RISE,
    JOYSTICK_TRIGGER_FALL,
    JOYSTICK_TRIGGER_0, //Key E
    JOYSTICK_TRIGGER_1, //Key F
    JOYSTICK_TRIGGER_2, //Key Q
    JOYSTICK_TRIGGER_COUNT
} device_joystick_trigger_type;

typedef enum device_joystick_trigger_state {
    JOYSTICK_TRIGGER_PRESS = 1,
    JOYSTICK_TRIGGER_RELEASE = 2,
} device_joystick_trigger_state;

typedef struct device_desc {
    int32_t width;
    int32_t height;
    const char *name;
    bool fullscreen;
    int32_t msaa;
} device_desc;

typedef struct device_joystick_pointer {
    double x, y;
    double dx, dy;
} device_joystick_pointer;

typedef struct device_joystick_axis {
    double value;
    double raw;
} device_joystick_axis;

typedef struct device_cursor_state {
    bool centered;
    bool visible;
} device_cursor_state;

typedef struct device_joystick {
    device_joystick_pointer pointer;
    device_joystick_axis horiz;
    device_joystick_axis vert;
} device_joystick;

typedef struct device_data *device_handle;
typedef struct device_joystick_callback_data *device_joystick_callback_handle;
typedef void (*device_joystick_callback)();


/*
 * Device management.
 */

API bool device_init();

API void device_terminate();

API device_handle device_new(device_desc const *desc);

API void device_delete(device_handle handle);

API void device_set_current(device_handle handle);

API void device_update_events();

API void device_refresh();

API void device_window_close();

API void device_window_cursor_set(device_cursor_state const *state);

API bool device_window_valid();

API void device_window_dimensions_get(int32_t* width, int32_t* height);


/*
 * Events
 */

API double device_dt_get();

API double device_time_get();

API device_joystick device_joystick_get();

API void device_joystick_trigger_register(
        device_joystick_trigger_type trigger_type,
        device_joystick_trigger_state trigger_state_flags,
        device_joystick_callback callback);

API void device_joystick_trigger_unregister(device_joystick_callback callback);


#endif //DEVICE_H
