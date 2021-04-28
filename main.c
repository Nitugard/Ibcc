#include <stdio.h>
#include "Device/Device.h"
#include "Os/Allocator.h"
#include "Graphics/Graphics.h"
#include <Interface/Text.h>
#include <Model/Model.h>
#include <Asset/Asset.h>
#include <Scene/Scene.h>
#include <Draw/Draw.h>
#include <Interface/Sprite.h>
#include <Shaders/DepthShader.h>

scene_handle active_scene;

void escape()
{
    device_window_close();
}

void toggle_wireframe()
{
    scene_wireframe_toggle(active_scene);
}

void toggle_bounding_box()
{
    scene_bounding_box_toggle(active_scene);
}


int main()
{


    device_desc device_desc = {
            .height = 720,
            .width = 1280,
            .name = "Device",
            .fullscreen = true,
            .msaa = 0,
    };

    device_init();
    device_cursor_state cursor_desc = {.centered = true, .visible = false};
    device_handle handle = device_new(&device_desc);
    device_set_current(handle);
    device_window_cursor_set(&cursor_desc);
    device_joystick_trigger_register(JOYSTICK_TRIGGER_MENU, JOYSTICK_TRIGGER_PRESS, escape);
    device_joystick_trigger_register(JOYSTICK_TRIGGER_1, JOYSTICK_TRIGGER_PRESS, toggle_wireframe);
    device_joystick_trigger_register(JOYSTICK_TRIGGER_2, JOYSTICK_TRIGGER_PRESS, toggle_bounding_box);
    int32_t width, height;
    device_window_dimensions_get(&width, &height);

    asset_init();
    gfx_init();
    mdl_init();
    sprite_init(8);

    gfx_texture_desc tex_desc = {
            .data = 0,
            .width =  width ,
            .height = height,
            .type = GFX_TEXTURE_TYPE_SRGB,
            .mipmaps = false,
            .filter = GFX_TEXTURE_FILTER_NEAREST
    };
    gfx_texture_desc depth_tex = {
            .data = 0,
            .width =  width ,
            .height = height ,
            .type = GFX_TEXTURE_TYPE_DEPTH,
            .mipmaps = false,
            .filter = GFX_TEXTURE_FILTER_NEAREST
    };

    gfx_texture_handle fbo_color_texture = gfx_texture_create(&tex_desc);
    gfx_texture_handle depth_color_texture = gfx_texture_create(&depth_tex);

    gfx_framebuffer_desc fbo_desc = {
            .color_attachments = {
                    {
                        .enabled = true,
                        .texture_handle = fbo_color_texture,
                    }
            },
            .depth_stencil_attachment = {
                    .texture_handle = depth_color_texture,
                    .enabled = true,
            }
    };
    gfx_framebuffer_handle fbo_handle = gfx_framebuffer_create(&fbo_desc);


    gfx_pass_desc pass_desc = {
            .pass_options = GFX_PASS_OPTION_DEPTH_TEST | GFX_PASS_OPTION_CULL_BACK,
            .actions = GFX_PASS_ACTION_CLEAR_STENCIL | GFX_PASS_ACTION_CLEAR_DEPTH | GFX_PASS_ACTION_CLEAR_COLOR,
            .fbo_handle = fbo_handle,
            .clear_color = {0.2f, 0.8f, 1, 1}
    };

    gfx_pass_desc default_pass = {
            .pass_options = GFX_PASS_OPTION_FRAMEBUFFER_SRGB | GFX_PASS_OPTION_CULL_BACK,
            .actions = GFX_PASS_ACTION_CLEAR_STENCIL | GFX_PASS_ACTION_CLEAR_COLOR,
            .fbo_handle = 0,
            .clear_color = {0, 0, 0, 1}
    };

    gfx_shader_handle depth_shader = gfx_shader_create(&sprite_shader_desc);


    text_init();
    text_handle lbl1 = text_new(50); text_update(lbl1, width-350, 15, 1, "Dragutin Sredojevic");
    text_handle fps = text_new(50);
    text_handle memory = text_new(50);
    mdl_handle mdl_handle = (struct mdl_data *) asset_load("./Data/sponza_tex_single.gltf");
    scene_lighting_settings lighting_settings = SCENE_DEFAULT_LIGHTING;
    scene_sun_settings sun_settings = SCENE_DEFAULT_SUN;

    active_scene = scene_new(mdl_handle, sun_settings, lighting_settings);
    asset_unload((asset_hndl) mdl_handle);
    char fps_buf[50];
    dw_desc desc = {};
    dw_handle dw = dw_new(&desc);

    sprite_desc fb1 = {.gfx_texture_handle = fbo_color_texture, .width = width, .height = height, .scale= 1, .offset_y = 0, .offset_x = 0};
    sprite_desc fb2 = {.gfx_texture_handle = depth_color_texture, .width = 300, .height = 200, .scale= 1, .offset_y = 0, .offset_x = 0, .custom_shader = {.enabled = true, .gfx_shader_handle = depth_shader}};
    sprite_new(fb1);
    sprite_new(fb2);

    dw_grid(dw, 10);
    dw_apply(dw);

    while(device_window_valid()) {

        device_update_events();


        gfx_begin_pass(&pass_desc);

        scene_draw(active_scene);
        scene_camera_projection projection = scene_camera_projection_get();
        dw_draw(dw, projection.projection, projection.view);
        gfx_begin_pass(&default_pass);
        sprintf(fps_buf, "Instances: %i Fps: %i", gfx_draw_call_count_get(), (int)(1.0 /device_dt_get()));
        text_update(fps, 0, height-30, 1, fps_buf);
        sprintf(fps_buf, "Heap allocations: %i Heap alloc size: %i", os_get_tracked_allocations_length(), os_get_tracked_allocations_size());
        text_update(memory, 0, height-60, 1, fps_buf);
        sprite_draw();
        text_draw_screen(fps);
        text_draw_screen(memory);
        text_draw_screen(lbl1);
        device_refresh();

    }
    text_delete(memory);
    text_delete(fps);
    text_delete(lbl1);
    text_terminate();
    dw_delete(dw);
    gfx_terminate();
    asset_terminate();
    scene_delete(active_scene);
    device_delete(handle);
    device_terminate();
    return 0;
}
