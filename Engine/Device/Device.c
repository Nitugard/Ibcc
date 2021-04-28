/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Device.h"

#include <GLFW/glfw3.h>

#include <Os/Platform.h>
#include <Os/Log.h>
#include <Os/Allocator.h>
#include <Containers/Array.h>
#include <stdlib.h>

typedef struct device_delta_time{
    double t;
    double dt;
    bool init;
}device_delta_time;

typedef struct device_joystick_callback_data{
    device_joystick_callback callback;
    device_joystick_trigger_state flags;
} device_joystick_callback_data;

typedef struct device_data {
    GLFWwindow *win_h;
    int32_t width, height;
    device_cursor_state cursor_state;

    device_joystick joystick;
    device_delta_time time;
    arr_handle joystick_callbacks;

} device_data;

device_handle active_device;

//void glfw_character_callback(GLFWwindow *window, uint32_t codepoint);
//void glfw_cursor_pos_callback(GLFWwindow *window, double x, double y);

void joystick_invoke_callbacks(device_joystick_trigger_type trigger, device_joystick_trigger_state state) {
    arr_handle *callbacks = (arr_handle *) arr_get(active_device->joystick_callbacks, trigger);
    for (int32_t j = 0; j < arr_size(*callbacks); ++j) {
        struct device_joystick_callback_data* data = (device_joystick_callback_data *) arr_get(*callbacks, j);
        if((data->flags & state) != 0)
            data->callback();
    }
}

void glfw_key_callback(GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods){
    enum device_joystick_trigger_state flags = 0;
    switch (action) {
        case GLFW_PRESS: flags |= JOYSTICK_TRIGGER_PRESS; break;
        case GLFW_RELEASE: flags |= JOYSTICK_TRIGGER_RELEASE; break;
        default: break;
    }

    switch (key) {
        case GLFW_KEY_ESCAPE: joystick_invoke_callbacks(JOYSTICK_TRIGGER_MENU, flags); break;
        case GLFW_MOUSE_BUTTON_LEFT: joystick_invoke_callbacks(JOYSTICK_TRIGGER_SELECT, flags); break;
        case GLFW_MOUSE_BUTTON_RIGHT: joystick_invoke_callbacks(JOYSTICK_TRIGGER_DESELECT, flags); break;
        case GLFW_KEY_SPACE: joystick_invoke_callbacks(JOYSTICK_TRIGGER_RISE, flags); break;
        case GLFW_KEY_LEFT_SHIFT: joystick_invoke_callbacks(JOYSTICK_TRIGGER_FALL, flags); break;
        case GLFW_KEY_E: joystick_invoke_callbacks(JOYSTICK_TRIGGER_0, flags); break;
        case GLFW_KEY_F: joystick_invoke_callbacks(JOYSTICK_TRIGGER_1, flags); break;
        case GLFW_KEY_Q: joystick_invoke_callbacks(JOYSTICK_TRIGGER_2, flags); break;
        default: break;
    }

}


double clamp(double x, double min, double max) {
    if (x > max) return max;
    if (x < min) return min;
    return x;
}

double move_towards(double val, double target, double dt)
{
    return (target - val) * dt + val;
}

void device_joystick_update() {

    double x, y;
    glfwGetCursorPos(active_device->win_h, &x, &y);
    active_device->joystick.pointer.dx = (x - active_device->joystick.pointer.x) ;
    active_device->joystick.pointer.dy = (y - active_device->joystick.pointer.y);
    active_device->joystick.pointer.x = x;
    active_device->joystick.pointer.y = y;

    active_device->joystick.vert.raw = 0;
    active_device->joystick.horiz.raw = 0;

    if (glfwGetKey(active_device->win_h, GLFW_KEY_W) == GLFW_PRESS)
        active_device->joystick.vert.raw += 1;
    if (glfwGetKey(active_device->win_h, GLFW_KEY_S) == GLFW_PRESS)
        active_device->joystick.vert.raw += -1;

    if (glfwGetKey(active_device->win_h, GLFW_KEY_A) == GLFW_PRESS)
        active_device->joystick.horiz.raw += 1;
    if (glfwGetKey(active_device->win_h, GLFW_KEY_D) == GLFW_PRESS)
        active_device->joystick.horiz.raw += -1;


    active_device->joystick.horiz.value = clamp(
            move_towards(active_device->joystick.horiz.value, active_device->joystick.horiz.raw, device_dt_get() * 10), -1, 1);
    active_device->joystick.vert.value = clamp(
            move_towards(active_device->joystick.vert.value, active_device->joystick.vert.raw, device_dt_get() * 10), -1, 1);
}


void device_window_cursor_update() {

    device_cursor_state state = active_device->cursor_state;
    device_joystick_pointer *pointer_state = &(active_device->joystick.pointer);
    int32_t width = active_device->width;
    int32_t height = active_device->height;
    double x, y;
    glfwGetCursorPos(active_device->win_h, &x, &y);

    if (state.centered) {
        x = width / 2.0;
        y = height / 2.0;
    }

    pointer_state->x = x;
    pointer_state->y = y;
    glfwSetCursorPos(active_device->win_h, pointer_state->x, pointer_state->y);
}


void device_delta_time_update()
{
    if(!active_device->time.init)
        active_device->time.init = true;
    else
        active_device->time.dt = glfwGetTime() - active_device->time.t;

    active_device->time.t = glfwGetTime();
}


GLFWcursor *glfw_blank_cursor() {
    unsigned char pixels[4];
    os_memset(pixels, 0, sizeof(pixels));
    GLFWimage image;
    image.width = 1;
    image.height = 1;
    image.pixels = pixels;
    return glfwCreateCursor(&image, 0, 0);
}

bool device_init() {


    os_allocator_init();

    if (!glfwInit()) {
        LOG_ERROR("Failed to initialize glfw\n");
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

    LOG_INFO("Device initialized\n");
    return true;
}

void device_terminate() {
    glfwTerminate();

    uint32_t tracked_allocations = os_get_tracked_allocations_length();
    if(tracked_allocations > 0) {
        const os_proxy_header **allocations = malloc(sizeof(void *) * tracked_allocations);
        os_get_tracked_allocations(allocations);
        for (unsigned int i = 0; i < tracked_allocations; ++i) {
            const struct os_proxy_header *data = allocations[i];
            if (data != 0)
                LOG_ERROR("Leak detected, size: %i\n%s:%i\n", data->size, data->file, data->line);
        }
        free(allocations);
        LOG_ERROR("Number of leaks %i, leak size: %i\n", tracked_allocations, os_get_tracked_allocations_size());
    }

    os_allocator_terminate();
}

device_handle device_new(const device_desc *desc) {

    glfwWindowHint(GLFW_SAMPLES, desc->msaa);

    const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int32_t w, h;
    GLFWmonitor* monitor = 0;
    if(desc->fullscreen)
    {
        w = mode->width;
        h = mode->height;
        monitor = glfwGetPrimaryMonitor();
    }
    else{
        w = desc->width;
        h = desc->height;
    }
    GLFWwindow *window = glfwCreateWindow(w, h, desc->name , monitor, NULL);
    if(window == 0) return 0;
    glfwMakeContextCurrent(window);

    LOG_INFO("Glfw %s\n", glfwGetVersionString());

    glfwSetKeyCallback(window, glfw_key_callback);
    //glfwSetCharCallback(window, glfw_character_callback);
    //glfwSetCursorPosCallback(window, glfw_cursor_pos_callback);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    arr_handle joystick_callbacks = arr_new(sizeof(arr_handle), JOYSTICK_TRIGGER_COUNT);
    arr_resize(joystick_callbacks, JOYSTICK_TRIGGER_COUNT);
    arr_handle callbacks_array;
    for(uint32_t i=0; i<JOYSTICK_TRIGGER_COUNT; ++i) {
        callbacks_array = arr_new(sizeof(device_joystick_callback_data), 1);
        arr_set(joystick_callbacks, i, &callbacks_array);
    }

    device_handle handle = OS_MALLOC(sizeof(device_data));
    os_memset(handle, 0, sizeof(device_data));

    handle->win_h = window;
    handle->width = w;
    handle->height = h;
    handle->joystick_callbacks = joystick_callbacks;
    return handle;

}

void device_delete(device_handle handle) {
    glfwMakeContextCurrent(0);
    glfwDestroyWindow(handle->win_h);
    for(int i=0; i<JOYSTICK_TRIGGER_COUNT; ++i) {
        arr_handle *callbacks = (arr_handle *) arr_get(active_device->joystick_callbacks, i);
        arr_delete(*callbacks);
    }
    arr_delete(active_device->joystick_callbacks);

    OS_FREE(handle);
}

void device_set_current(device_handle handle) {
    active_device = handle;
}

void device_refresh() {
    ASSERT(active_device != 0);

    glfwSwapBuffers(active_device->win_h);
}

bool device_window_valid() {
    if (active_device->win_h == NULL)
        return false;
    if (glfwWindowShouldClose(active_device->win_h))
        return false;

    return true;
}

void device_window_close() {
    glfwSetWindowShouldClose(active_device->win_h, true);
}

void device_window_cursor_set(const device_cursor_state *state) {
    if (!state->visible) {
        glfwSetCursor(active_device->win_h, glfw_blank_cursor());
    } else {
        glfwSetCursor(active_device->win_h, NULL);
    }
    active_device->cursor_state = *state;
}


device_joystick device_joystick_get() {
    return active_device->joystick;
}

double device_dt_get() {
    return active_device->time.dt;
}

double device_time_get() {
   return glfwGetTime();
}

void device_joystick_trigger_register(device_joystick_trigger_type trigger_type,
                                                                 device_joystick_trigger_state trigger_state_flags,
                                                                 device_joystick_callback callback) {

    device_joystick_callback_data data = {.callback = callback, .flags = trigger_state_flags};
    arr_handle *callbacks = (arr_handle *) arr_get(active_device->joystick_callbacks, trigger_type);
    for(int32_t j=0; j<arr_size(*callbacks); ++j) {
        if (*((device_joystick_callback_data *) arr_get(*callbacks, j))->callback == callback) {
            LOG_ERROR("Joystick callback %i already registered\n", callback);
            return;
        }
    }
    arr_add(*callbacks, &data);
    LOG_INFO("Joystick callback %i registered\n", callback);
}

void device_joystick_trigger_unregister(device_joystick_callback callback) {
    for(int i=0; i<JOYSTICK_TRIGGER_COUNT; ++i) {
        arr_handle *callbacks = (arr_handle *) arr_get(active_device->joystick_callbacks, i);
        for (int32_t j = 0; j < arr_size(*callbacks); ++j) {
            if (*((device_joystick_callback_data *) arr_get(*callbacks, j))->callback == callback) {
                arr_remove_swap(*callbacks, j);
                LOG_INFO("Joystick callback %i removed\n", callback);
                break;
            }
        }
    }

}

void device_window_dimensions_get(int32_t *width, int32_t *height) {
    *width = active_device->width;
    *height = active_device->height;
}

void device_update_events() {

    device_delta_time_update();
    device_joystick_update();
    device_window_cursor_update();

    glfwPollEvents();
}
