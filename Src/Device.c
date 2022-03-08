/*
 *  Copyright (C) 2021-2022 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#include "Device.h"

#include "Allocator.h"
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#ifndef CORE_ASSERT
#include "assert.h"
#define CORE_ASSERT(e) assert(e)
#endif

typedef struct device_delta_time{
    float t;
    float dt;
    bool init;
}device_delta_time;

typedef struct device_data {
    GLFWwindow *win_h;
    int32_t width, height;
    bool cursor_centered;
    bool cursor_visible;

    bool joystick_init;
    device_joystick joystick;
    device_delta_time time;
} device_data;

device_handle active_device;
device_log_callback device_log;

char log_buffer[LOG_BUFFER_SIZE];

#define LOG_ERROR(format, ...) if(device_log != 0) { sprintf(log_buffer, format, __VA_ARGS__); device_log(log_buffer, true);}
#define LOG(format, ...)  if(device_log != 0) { sprintf(log_buffer, format, __VA_ARGS__); device_log(log_buffer, false);}
#define LOGG(format)  if(device_log != 0) device_log(format, true);
#define LOGG_ERROR(format) if(device_log != 0) device_log(format, true);


void glfw_key_callback(GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods){
    enum device_joystick_trigger_state flags = 0;
    switch (action) {
        case GLFW_PRESS: flags |= JOYSTICK_TRIGGER_PRESS; break;
        case GLFW_RELEASE: flags |= JOYSTICK_TRIGGER_RELEASE; break;
        default: break;
    }
}

void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    active_device->joystick.pointer.scroll_x += xoffset;
    active_device->joystick.pointer.scroll_y += yoffset;
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
    if(active_device->joystick_init) {
        active_device->joystick.pointer.dx = (float) (x - active_device->joystick.pointer.x);
        active_device->joystick.pointer.dy = (float) (y - active_device->joystick.pointer.y);
    }
    active_device->joystick.pointer.x = (float)x;
    active_device->joystick.pointer.y = (float)y;

    active_device->joystick.vert.raw = 0;
    active_device->joystick.horiz.raw = 0;


    if(active_device->joystick_init) {
        active_device->joystick.pointer.scroll_dx = active_device->joystick.pointer.scroll_x;
        active_device->joystick.pointer.scroll_dy = active_device->joystick.pointer.scroll_y;
    }

    active_device->joystick.pointer.scroll_x = 0;
    active_device->joystick.pointer.scroll_y = 0;


    active_device->joystick.mouse.lmb_press = glfwGetMouseButton(active_device->win_h, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    active_device->joystick.mouse.rmb_press = glfwGetMouseButton(active_device->win_h, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    active_device->joystick.mouse.mmb_press = glfwGetMouseButton(active_device->win_h, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    active_device->joystick.mouse.lmb_release = glfwGetMouseButton(active_device->win_h, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE;
    active_device->joystick.mouse.rmb_release = glfwGetMouseButton(active_device->win_h, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE;
    active_device->joystick.mouse.mmb_release = glfwGetMouseButton(active_device->win_h, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_RELEASE;


    if (glfwGetKey(active_device->win_h, GLFW_KEY_W) == GLFW_PRESS)
        active_device->joystick.vert.raw += 1;
    if (glfwGetKey(active_device->win_h, GLFW_KEY_S) == GLFW_PRESS)
        active_device->joystick.vert.raw += -1;

    if (glfwGetKey(active_device->win_h, GLFW_KEY_A) == GLFW_PRESS)
        active_device->joystick.horiz.raw += 1;
    if (glfwGetKey(active_device->win_h, GLFW_KEY_D) == GLFW_PRESS)
        active_device->joystick.horiz.raw += -1;


    active_device->joystick.horiz.value = (float)clamp(
            move_towards(active_device->joystick.horiz.value, active_device->joystick.horiz.raw, device_dt_get() * 10), -1, 1);
    active_device->joystick.vert.value = (float)clamp(
            move_towards(active_device->joystick.vert.value, active_device->joystick.vert.raw, device_dt_get() * 10), -1, 1);

    active_device->joystick_init = true;
}

void* device_window_handle(){
    return active_device->win_h;
}

void device_window_cursor_update() {

    device_joystick_pointer *pointer_state = &(active_device->joystick.pointer);
    int32_t width = active_device->width;
    int32_t height = active_device->height;
    double x, y;
    glfwGetCursorPos(active_device->win_h, &x, &y);

    if (active_device->cursor_centered) {
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

void device_log_callback_set(device_log_callback callback) {
    device_log = callback;
}

bool device_init(int32_t version_major, int32_t version_minor) {
    os_allocator_init();

    if (!glfwInit()) {
        return false;
    }


    LOG("Device initialized: %s\n", glfwGetVersionString());

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, version_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, version_minor);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_DOUBLEBUFFER, true);
    device_log = 0;

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
                LOG("Leak detected, size: %i\n%s:%i\n", data->size, data->file, data->line);
        }
        free(allocations);
        LOG("Number of leaks %i, leak size: %i\n", tracked_allocations, os_get_tracked_allocations_size());
    }

    os_allocator_terminate();
}

device_handle device_new(const char* name, int32_t width, int32_t height, bool vsync, bool fs, bool resizable) {
    glfwWindowHint(GLFW_RESIZABLE, resizable);
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *vidmode = glfwGetVideoMode(monitor);
    LOG("Device monitor width: %i, height: %i, rgb bits: %i %i %i refresh rate: %i\n", vidmode->width, vidmode->height,
           vidmode->redBits, vidmode->greenBits, vidmode->blueBits, vidmode->refreshRate);
    LOG("Vsync: %i, Fullscreen: %i, Resizable: %i", vsync, fs, resizable);
    GLFWwindow *window;
    if(width < 0 || height < 0){
       width = vidmode->width;
       height = vidmode->height;
    }

    if (fs) {
        window = glfwCreateWindow(width, height, name, monitor, 0);
    } else {
        window = glfwCreateWindow(width, height, name, 0, 0);
    }

    if (window == 0)
    {
        LOGG_ERROR("Device creation failed\n");
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetScrollCallback(window, glfw_scroll_callback);

    if (vsync) {
        glfwSwapInterval(1);
    }

    device_handle handle = OS_MALLOC(sizeof(device_data));
    os_memset(handle, 0, sizeof(device_data));

    handle->win_h = window;
    handle->width = width;
    handle->height = height;

    LOGG("Device created\n");
    return handle;
}

void device_delete(device_handle handle) {
    glfwMakeContextCurrent(0);
    glfwDestroyWindow(handle->win_h);
    OS_FREE(handle);
    LOGG("Device destroyed\n");
}

void device_set_current(device_handle handle) {
    glfwMakeContextCurrent(handle->win_h);
    active_device = handle;
}

void device_refresh() {
    glfwMakeContextCurrent(active_device->win_h);
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

void device_window_cursor_set(bool centered, bool visible) {
    if (!visible) {
        glfwSetCursor(active_device->win_h, glfw_blank_cursor());
    } else {
        glfwSetCursor(active_device->win_h, NULL);
    }
    active_device->cursor_centered = centered;
    active_device->cursor_visible = visible;
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



/*
 * File
 */

device_file_handle device_file_open(const char* path, const char* mode){
    return fopen(path, mode);
}

void device_file_size(device_file_handle handle, int32_t* size){
    CORE_ASSERT(handle != 0 && "File handle is invalid");
    fseek(handle, 0, SEEK_END);
    *size = ftell(handle);
    fseek(handle, 0, SEEK_SET);
    return;
}

int32_t device_file_read(device_file_handle handle, int32_t offset, int32_t length, void* buffer){
    CORE_ASSERT(handle != 0 && "File handle is invalid");
    return fread(buffer, sizeof(char), length, handle);
}

void device_file_write(device_file_handle handle, int32_t offset, int32_t length, void const* buffer){
    CORE_ASSERT(handle != 0 && "File handle is invalid");
    fwrite(buffer, offset, length, handle);
}

void device_file_close(device_file_handle handle){
    CORE_ASSERT(handle != 0 && "File handle is invalid");
    fclose(handle);
}

void* device_file_read_text(const char* path){
    void* file = device_file_open(path, "r");
    int32_t size;
    device_file_size(file, &size);
    char* buffer = OS_MALLOC((size+1) * sizeof(char));
    int32_t new_len = device_file_read(file, 0, size, buffer);
    buffer[new_len] = '\0';
    device_file_close(file);
    return buffer;
}

