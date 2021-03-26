
#include <stdio.h>
#include "./Core/Device/Device.h"
#include "./Core/Graphics/Graphics.h"
#include "./Core/Graphics/ShaderTest.h"

void input_callback(char a)
{
    fprintf(stdout, "%c", a);
}

int main() {

    device_wnd_desc     wndDesc     = {
            .width = 600,
            .height = 400,
            .name = "Physics Engine",
    };
    device_cursor_state cursorState = {
            .visible = 1,
            .confined = 0,
            .wrap = 1,
            .centered = 0,
    };
    device_init();
    device_wnd_hndl wndHndl = device_window_create(&wndDesc);
    device_events_input_set_callback(wndHndl, input_callback);
    device_events_input_set_callback(wndHndl, input_callback);
    device_window_cursor_set_state(wndHndl, &cursorState);

    //init resources
    gfx_shader_handle shader = gfx_shader_create(&gl_shader_desc);

    while (device_window_valid(wndHndl)) {
        if (device_events_get_key(wndHndl, DEVICE_KEY_ESCAPE) == DEVICE_PRESS_ACTION) {
            device_window_close(wndHndl);
        }

        device_window_mouse_update(wndHndl);
        device_window_cursor_update(wndHndl);
        device_window_refresh(wndHndl);
        device_events_poll();
    }

    //cleanup resources
    gfx_shader_destroy(shader);

    device_window_destroy(wndHndl);
    device_terminate();

}