
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include <Device/Device.h>
#include <Graphics/Graphics.h>
#include <Game/Shaders/UnlitShader.h>
#include <Plugin/Plugin.h>
#include <Asset/Asset.h>
#include <Log/Log.h>
#include <Os/Time.h>
#include <Os/Allocator.h>
#include <Game/Primitives/Cube.h>

#include <SoftFloat/SoftMatrix.h>

void input_callback(char a)
{
    fprintf(stdout, "%c", a);
}

plg_desc req_plugins[] = {{.name = "Graphics", .min_version = 1},
                          {.name = "Device", .min_version = 1},
                          {.name = "Asset", .min_version = 1}
                          };

void plg_on_start(plg_info* info) {

    info->name = "Game";
    info->req_plugins = req_plugins;
    info->req_plugins_count = sizeof(req_plugins) / sizeof(plg_desc);
    info->version = 1;
}

bool plg_on_load()
{
    srand(time(NULL));

    SH_MVP_T mvp;
    device_cursor_state cursorState = {
            .visible = 1,
            .confined = 0,
            .wrap = 0,
            .centered = 0,
    };

    device_events_input_set_callback(input_callback);
    device_events_input_set_callback(input_callback);
    device_window_cursor_set_state(&cursorState);

    //init resources
    gfx_shader_handle shader = gfx_shader_create(&unlit_shader_desc);

    gfx_buffer_desc buffer_v_desc ={
            .size = sizeof(cube_vertices),
            .type = VERTEX,
            .update_mode = STATIC_DRAW,
            .data = cube_vertices,
    };

    gfx_buffer_desc buffer_i_desc ={
            .size = sizeof(cube_indices),
            .type = INDEX,
            .update_mode = STATIC_DRAW,
            .data = cube_indices,
    };

    gfx_buffer_desc buffer_c_desc ={
            .size = sizeof(cube_colors),
            .type = VERTEX,
            .update_mode = STREAM_DRAW,
            .data = cube_colors,
    };

    gfx_buffer_desc buffer_u_desc ={
            .size = sizeof(SH_MVP_T),
            .type = UNIFORM,
            .update_mode = STREAM_DRAW,
            .data = &mvp,
    };

    gfx_buffer_handle buffer_v = gfx_buffer_create(&buffer_v_desc);
    gfx_buffer_handle buffer_c = gfx_buffer_create(&buffer_c_desc);
    gfx_buffer_handle buffer_u = gfx_buffer_create(&buffer_u_desc);
    gfx_buffer_handle buffer_i = gfx_buffer_create(&buffer_i_desc);


    gfx_pipeline_desc pipeline_desc = {
            .contiguous_buffer = true,
            .shader = shader,
            .attrs = {
                    [position_attr] = {
                            .enabled = true,
                            .buffer = buffer_v,
                    },
                    [color_attr] = {
                            .enabled = true,
                            .buffer = buffer_c,
                    },

            },
            .uniform_blocks = {
                    [0] = {.name = "matrices", .enabled = true, .buffer = buffer_u }
            },
            .index_buffer = buffer_i
    };

    gfx_pass_action default_pass = {};
    gfx_pipeline_handle pipeline = gfx_pipeline_create(&pipeline_desc);
    gfx_color color = { 0.2, 0.8, 1, 1};
    default_pass.action = GFX_ACTION_CLEAR;
    default_pass.value = color;
    while (device_window_valid()) {
        if (device_events_get_key(DEVICE_KEY_ESCAPE) == DEVICE_PRESS_ACTION) {
            device_window_close();
        }

        sf_vec3 co = sf_vec3_new(sf_new(0), sf_new(0), sf_new(-10));
        sf_mat4 m4p = sf_mat_perspective(sf_new(60), sf_new_fraction(800, 600), sf_new_fraction(1, 10), sf_new(100));
        sf_mat4 m4t = sf_mat_translate(&co);
        sf_mat4 m4r = sf_mat_rotate_y(sf_new_fraction(device_get_time() * 100, 100));

        sf_mat4 m4 = SF_MAT_MUL(&m4p, &m4t, &m4r);

        sf_array_to_float_array(m4.data, 16, mvp.projection);

        gfx_begin_default_pass(&default_pass);
        gfx_apply_pipeline(pipeline);
        gfx_buffer_update(buffer_c, &buffer_c_desc);
        gfx_buffer_update(buffer_u, &buffer_u_desc);

        gfx_draw_triangles_indexed(CUBE_TRIANGLES);
        gfx_end_pass();

        device_window_mouse_update();
        device_window_cursor_update();
        device_window_refresh();
        device_events_poll();
    }

    //cleanup resources
    gfx_shader_destroy(shader);
    gfx_buffer_destroy(buffer_v);
    gfx_buffer_destroy(buffer_c);
    gfx_buffer_destroy(buffer_u);
    gfx_buffer_destroy(buffer_i);
    gfx_pipeline_destroy(pipeline);
    return true;
}


void plg_on_stop()
{

}