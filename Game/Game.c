
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "Game.h"
#include <Device/Device.h>
#include <Graphics/Graphics.h>
#include <Shaders/UnlitShader.h>
#include <Asset/Asset.h>
#include <Os/Log.h>
#include <Os/Time.h>
#include <Os/Allocator.h>
#include <Asset/Texture/Texture.h>
#include <Asset/Model/Model.h>
#include <Math/Math.h>
#include <stddef.h>
#include "Controller/Controller.h"
#include "Render/Draw/Draw.h"


void input_callback(char a)
{
}

bool game_init()
{
    asset_init();
    device_init();
    gfx_init();

    srand(time(NULL));

    SH_MVP_T mvp;
    mvp.light_pos[1] = 1;

    device_cursor_state cursorState = {
            .visible = 0,
            .confined = 0,
            .wrap = 0,
            .centered = 1,
    };

    controller_camera_init_desc camera_desc = {
            .pos = mm_vec3_new(0, 1, 10),
            .rot_euler = mm_vec3_new(0, 180, 0),
            .first_person = true,
            .movement_speed = 10,
            .rotation_speed = 30
    };


    device_events_input_set_callback(input_callback);
    device_events_input_set_callback(input_callback);
    device_window_cursor_set_state(&cursorState);

    //init resources
    gfx_shader_handle shader = gfx_shader_create(&unlit_shader_desc);
    mdl_data_handle obj_handle = (mdl_data_handle) asset_load("./Assets/demongirl.obj");
    controller_camera_handle camera = controller_camera_new(&camera_desc);

    gfx_buffer_desc buffer_v_desc ={
            .size = obj_handle->buffer_size,
            .type = VERTEX,
            .update_mode = STATIC_DRAW,
            .data = obj_handle->buffer,
    };

    gfx_buffer_desc buffer_u_desc ={
            .size = sizeof(SH_MVP_T),
            .type = UNIFORM,
            .update_mode = STREAM_DRAW,
            .data = &mvp,
    };

    gfx_buffer_handle buffer_v = gfx_buffer_create(&buffer_v_desc);
    gfx_buffer_handle buffer_u = gfx_buffer_create(&buffer_u_desc);

    dw_desc draw_desc = {
            .cam_buffer = buffer_u
    };

    dw_handle draw_handle = dw_new(&draw_desc);
    for(int i=0; i<=10; ++i) {
        dw_vector_origin(draw_handle,
                         mm_vec3_new(i - 5, 0, 5),
                         mm_vec3_new(i - 5, 0, -5),
                         mm_vec3_new(0.5, 0.5, 0.5));
        dw_vector_origin(draw_handle,
                         mm_vec3_new(+5, 0, i - 5),
                         mm_vec3_new(-5, 0, i - 5),
                         mm_vec3_new(0.5, 0.5, 0.5));
    }
    dw_vector(draw_handle, mm_vec3_new(0,100,0), mm_vec3_new(1,0,0));
    dw_vector(draw_handle, mm_vec3_new(100,0,0), mm_vec3_new(0,1,0));
    dw_vector(draw_handle, mm_vec3_new(0,0,100), mm_vec3_new(0,0,1));

    float identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    gfx_pipeline_desc pipeline_desc = {
            .shader = shader,
            .attrs = {
                    [position_attr] = {
                            .buffer = buffer_v,
                            .stride = sizeof(vertex_t),
                            .offset = 0,
                    },
                    [normal_attr] = {
                            .buffer = buffer_v,
                            .stride = sizeof(vertex_t),
                            .offset = offsetof(vertex_t, color),
                    },

            },
            .uniform_blocks = {
                    [0] = {.name = "matrices", .buffer = buffer_u }
            },
            .uniforms = {
                    [0] = {.name = "model", .buffer = identity, .offset = 0}
            }
    };


    gfx_pass_action default_pass = {};
    gfx_pipeline_handle pipeline = gfx_pipeline_create(&pipeline_desc);
    gfx_color color = { 0.2f, 0.8f, 1, 1};
    default_pass.action = GFX_ACTION_CLEAR;
    default_pass.value = color;
    uint32_t dw_pos = dw_get_position(draw_handle);
    while (device_window_valid()) {

        if (device_events_get_key(DEVICE_KEY_ESCAPE) == DEVICE_PRESS_ACTION) {
            device_window_close();
        }

        controller_camera_update(camera, (float)device_events_get_dt());
        controller_camera_update_mvp(camera, mvp.projection, mvp.view, mvp.cam_pos);

        mvp.light_pos[0] = mm_sin(device_get_time()) * 2;
        mvp.light_pos[2] = mm_cos(device_get_time()) * 2;

        gfx_buffer_update(buffer_u, &buffer_u_desc);

        gfx_update_uniforms(pipeline, 0, -1);
        gfx_begin_default_pass(&default_pass);
        gfx_apply_pipeline(pipeline);
        gfx_draw_triangles(0, obj_handle->vertices);
        dw_bind(draw_handle);
        dw_clear(draw_handle, dw_pos);
        dw_vector(draw_handle, mm_vec3_new(mvp.light_pos[0], mvp.light_pos[1], mvp.light_pos[2]), mm_vec3_new(0,0,0));
        gfx_end_pass();

        device_window_mouse_update();
        device_window_cursor_update();
        device_window_refresh();
        device_events_poll();
    }

    //cleanup resources
    gfx_shader_destroy(shader);
    gfx_buffer_destroy(buffer_v);
    gfx_buffer_destroy(buffer_u);
    gfx_pipeline_destroy(pipeline);
    controller_camera_destroy(camera);
    asset_unload((asset_hndl) obj_handle);

    gfx_terminate();
    device_terminate();
    asset_terminate();

    return true;
}


void game_terminate()
{

}