/*
 * GNU WGE --- Wildebeest Game Engine™
 * Copyright (C) 2023 Wasym A. Alonso
 *
 * This file is part of WGE.
 *
 * WGE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * WGE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with WGE.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <kmath.h>
#include <logger.h>
#include <kmemory.h>
#include <kstring.h>
#include <texture_system.h>
#include <material_system.h>
#include <renderer_backend.h>
#include <renderer_frontend.h>

#define PROJ_MATRIX_ASPECT_RATIO (16 / 9.0f)

typedef struct {
  renderer_backend backend;
  Matrix4 proj;
  Matrix4 view;
  Matrix4 ui_proj;
  Matrix4 ui_view;
  f32 near_clip;
  f32 far_clip;
} renderer_system_state;

static renderer_system_state *state_ptr;

b8 renderer_system_initialize(u64 *memory_requirements,
                              void *state,
                              const char *application_name) {
  *memory_requirements = sizeof(renderer_system_state);
  if (!state) return true;
  state_ptr = state;

  renderer_backend_create(RENDERER_BACKEND_TYPE_VULKAN,
                          &state_ptr->backend);
  state_ptr->backend.frame_number = 0;

  if (!state_ptr->backend.initialize(&state_ptr->backend, application_name)) {
    KFATAL("Renderer backend initialization failed. Shutting down the engine...");
    return false;
  }

  // World projection and view properties
  state_ptr->near_clip = 0.1f;
  state_ptr->far_clip = 1000.0f;
  state_ptr->proj = mat4_persp_proj(deg_to_rad(45.0f),
                                   PROJ_MATRIX_ASPECT_RATIO,
                                   state_ptr->near_clip,
                                   state_ptr->far_clip);
  state_ptr->view = mat4_inv(mat4_translation((Vector3) {{{ 0, 0, -30.0f }}}));

  // UI projection and view properties
  state_ptr->ui_proj = mat4_ortho_proj(0,
                                      1280.0f,
                                      720.0f,
                                      0,
                                      -100.0f,
                                      100.0f);
  state_ptr->ui_view = mat4_inv(mat4_id());

  return true;
}

void renderer_system_shutdown(void *state) {
  (void) state;  // Unused parameter

  if (!state_ptr) return;
  state_ptr->backend.shutdown(&state_ptr->backend);
  state_ptr = 0;
}

void renderer_on_resized(u16 width, u16 height) {
  if (state_ptr) {
    state_ptr->proj = mat4_persp_proj(deg_to_rad(45.0f),
                                     (f32) width / height,
                                     state_ptr->near_clip,
                                     state_ptr->far_clip);
    state_ptr->ui_proj = mat4_ortho_proj(0,
                                        (f32) width,
                                        (f32) height,
                                        0,
                                        -100.0f,
                                        100.0f);
    state_ptr->backend.resized(&state_ptr->backend, width, height);
  }
  else KWARN("renderer_on_resized :: Renderer backend does not exist");
}

b8 renderer_draw_frame(render_packet *packet) {
  // Begin frame
  if (state_ptr->backend.begin_frame(&state_ptr->backend, packet->delta_time)) {
    // Begin world renderpass
    if (!state_ptr->backend.begin_renderpass(&state_ptr->backend, BUILTIN_RENDERPASS_WORLD)) {
      KERROR("renderer_draw_frame :: World's `begin_renderpass` failed");
      return false;
    }

    // Update world global state
    state_ptr->backend.update_world(state_ptr->proj, state_ptr->view, vec3_zero(), vec4_one(), 0);

    // Draw world geometries
    for (u32 i = 0; i < packet->geometry_count; ++i) {
      state_ptr->backend.draw_geometry(packet->geometries[i]);
    }

    // End world renderpass
    if (!state_ptr->backend.end_renderpass(&state_ptr->backend, BUILTIN_RENDERPASS_WORLD)) {
      KERROR("renderer_draw_frame :: World's `end_renderpass` failed");
      return false;
    }

    // Begin UI renderpass
    if (!state_ptr->backend.begin_renderpass(&state_ptr->backend, BUILTIN_RENDERPASS_UI)) {
      KERROR("renderer_draw_frame :: UI's `begin_renderpass` failed");
      return false;
    }

    // Update UI global state
    state_ptr->backend.update_ui(state_ptr->ui_proj, state_ptr->ui_view, 0);

    // Draw UI geometries
    for (u32 i = 0; i < packet->ui_geometry_count; ++i) {
      state_ptr->backend.draw_geometry(packet->ui_geometries[i]);
    }

    // End UI renderpass
    if (!state_ptr->backend.end_renderpass(&state_ptr->backend, BUILTIN_RENDERPASS_UI)) {
      KERROR("renderer_draw_frame :: UI's `end_renderpass` failed");
      return false;
    }

    // End frame
    b8 result = state_ptr->backend.end_frame(&state_ptr->backend, packet->delta_time);
    ++state_ptr->backend.frame_number;
    if (!result) {
      KERROR("`renderer_draw_frame` failed. Shutting down the engine...");
      return false;
    }
  }
  return true;
}

void renderer_set_view(Matrix4 view) {
  state_ptr->view = view;
}

void renderer_create_texture(const u8 *pixels, texture *texture) {
  state_ptr->backend.create_texture(pixels, texture);
}

void renderer_destroy_texture(texture *texture) {
  state_ptr->backend.destroy_texture(texture);
}

b8 renderer_create_material(material *material) {
  return state_ptr->backend.create_material(material);
}

void renderer_destroy_material(material *material) {
  state_ptr->backend.destroy_material(material);
}

b8 renderer_create_geometry(geometry *geometry,
                            u32 vertex_count,
                            const vertex_3d *vertices,
                            u32 index_count,
                            const u32 *indices) {
  return state_ptr->backend.create_geometry(geometry,
                                           vertex_count,
                                           vertices,
                                           index_count,
                                           indices);
}

void renderer_destroy_geometry(geometry *geometry) {
  state_ptr->backend.destroy_geometry(geometry);
}
