
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "./Core/Device/Device.h"
#include "./Core/Graphics/Graphics.h"
#include "./Core/Graphics/ShaderTest.h"


void input_callback(char a)
{
    fprintf(stdout, "%c", a);
}

int main() {
    srand(time(NULL));

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
    gfx_shader_handle shader = gfx_shader_create(&unlit_shader_shader_desc);
    float vertices[] = {
                        -0.5f, -0.5f, 0.0f, 0,
                        0.5f, -0.5f, 0.0f, 0,
                        0.0f,  0.5f, 0.0f, 0,
    };
    float colors[] = {
            1, 0, 0, 1,
            0, 1, 0, 1,
            0, 0, 1, 1,
    };

    gfx_buffer_desc buffer_v_desc ={
            .size = sizeof(vertices),
            .type = VERTEX,
            .update_mode = STATIC_DRAW,
            .data = vertices,
    };

    gfx_buffer_desc buffer_c_desc ={
            .size = sizeof(colors),
            .type = VERTEX,
            .update_mode = STREAM_DRAW,
            .data = colors,
    };
    gfx_buffer_handle buffer_v = gfx_buffer_create(&buffer_v_desc);
    gfx_buffer_handle buffer_c = gfx_buffer_create(&buffer_c_desc);


    gfx_pipeline_desc pipeline_desc = {
            .contiguous_buffer = true,
            .shader = shader,
            .attrs = {
                    [unlit_shader_POSITION_attr] = {
                        .enabled = true,
                        .buffer = buffer_v,
                    },
                    [unlit_shader_COLOR_attr] = {
                            .enabled = true,
                            .buffer = buffer_c,
                    }
            }
    };
    gfx_pipeline_handle pipeline = gfx_pipeline_create(&pipeline_desc);


    while (device_window_valid(wndHndl)) {
        if (device_events_get_key(wndHndl, DEVICE_KEY_ESCAPE) == DEVICE_PRESS_ACTION) {
            device_window_close(wndHndl);
        }

        for(int i=0; i<12; ++i) {

            float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
            colors[i] = scale;
        }

        gfx_draw(pipeline);
        gfx_buffer_update(buffer_c, &buffer_c_desc);

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

