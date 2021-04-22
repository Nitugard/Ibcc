
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "Game.h"
#include <Device/Device.h>
#include <Graphics/Graphics.h>
#include <Shaders/BlinnPhongShader.h>
#include <Asset/Asset.h>
#include <Os/Log.h>
#include <Os/Time.h>
#include <Os/Allocator.h>
#include <Asset/Model/Model.h>
#include <Math/Math.h>
#include <stddef.h>
#include "Controller/Controller.h"
#include "Render/Draw/Draw.h"
#include "Scene/Scene.h"

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
            .pos = mm_vec3_new(-20, 10, 0),
            .rot_euler = mm_vec3_new(45, 90, 0),
            .first_person = true,
            .movement_speed = 10,
            .rotation_speed = 30
    };


    device_events_input_set_callback(input_callback);
    device_events_input_set_callback(input_callback);
    device_window_cursor_set_state(&cursorState);

    //init resources
    gfx_shader_handle shader = gfx_shader_create(&unlit_shader_desc);
    controller_camera_handle camera = controller_camera_new(&camera_desc);


    gfx_buffer_desc buffer_u_desc ={
            .size = sizeof(SH_MVP_T),
            .type = UNIFORM,
            .update_mode = DYNAMIC_DRAW,
            .data = &mvp,
    };

    gfx_buffer_handle buffer_u = gfx_buffer_create(&buffer_u_desc);

    dw_desc draw_desc = {
            .cam_buffer = buffer_u
    };

    dw_handle draw_handle = dw_new(&draw_desc);
    dw_grid(draw_handle, 20);

    xmdl_data_handle model = (xmdl_data_handle) asset_load("./Assets/untitled.xmdl");
    scene_handle scene = scene_new(buffer_u, *model);


    gfx_pass_action default_pass;
    gfx_color color = { 0.2f, 0.8f, 1, 1};
    default_pass.action = GFX_ACTION_CLEAR;
    default_pass.value = color;
    dw_apply(draw_handle);
    while (device_window_valid()) {

        if (device_events_get_key(DEVICE_KEY_ESCAPE) == DEVICE_PRESS_ACTION) {
            device_window_close();
        }

        controller_camera_update(camera, (float)device_events_get_dt());
        controller_camera_update_mvp(camera, mvp.projection, mvp.view, mvp.cam_pos);

        mvp.light_pos[0] = mm_sin(device_get_time()) * 2;
        mvp.light_pos[2] = mm_cos(device_get_time()) * 2;

        dw_clear_all(draw_handle);
        dw_cube(draw_handle, mm_vec3_new(mvp.light_pos[0], mvp.light_pos[1], mvp.light_pos[2]), mm_vec3_new(1,1,1), 0.2f);
        gfx_buffer_update(buffer_u, &buffer_u_desc);

        gfx_begin_default_pass(&default_pass);
        scene_draw(scene);
        dw_bind(draw_handle);
        gfx_end_pass();

        device_window_mouse_update();
        device_window_cursor_update();
        device_window_refresh();
        device_events_poll();
    }

    //cleanup resources
    gfx_shader_destroy(shader);
    gfx_buffer_destroy(buffer_u);
    dw_delete(draw_handle);
    controller_camera_destroy(camera);

    gfx_terminate();
    device_terminate();
    asset_terminate();

    return true;
}


void game_terminate()
{

}