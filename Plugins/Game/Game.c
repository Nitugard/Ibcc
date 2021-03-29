
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include <Device/Device.h>
#include <Graphics/Graphics.h>
#include <Game/ShaderTest.h>
#include <Plugin/Plugin.h>

void input_callback(char a)
{
    fprintf(stdout, "%c", a);
}

plg_desc req_plugins[] = {{.name = "Graphics", .min_version = 1},
                            {.name = "Device", .min_version = 1}};

void plg_on_start(plg_info* info) {

    info->name = "Game";
    info->req_plugins = req_plugins;
    info->req_plugins_count = sizeof(req_plugins) / sizeof(plg_desc);
    info->version = 1;
}

bool plg_on_load()
{
    srand(time(NULL));

    device_cursor_state cursorState = {
            .visible = 1,
            .confined = 0,
            .wrap = 1,
            .centered = 0,
    };

    device_events_input_set_callback(input_callback);
    device_events_input_set_callback(input_callback);
    device_window_cursor_set_state(&cursorState);

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


    while (device_window_valid()) {
        if (device_events_get_key(DEVICE_KEY_ESCAPE) == DEVICE_PRESS_ACTION) {
            device_window_close();
        }

        for(int i=0; i<12; ++i) {

            float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
            colors[i] = scale;
        }

        gfx_draw(pipeline);
        gfx_buffer_update(buffer_c, &buffer_c_desc);

        device_window_mouse_update();
        device_window_cursor_update();
        device_window_refresh();
        device_events_poll();
    }

    //cleanup resources
    gfx_shader_destroy(shader);
    return true;
}
