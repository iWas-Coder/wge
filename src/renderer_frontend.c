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
#include <renderer_backend.h>
#include <renderer_frontend.h>

#define PROJ_MATRIX_ASPECT_RATIO (16 / 9.0f)

typedef struct {
  renderer_backend backend;
  Matrix4 proj;
  Matrix4 view;
  f32 near_clip;
  f32 far_clip;
  texture fallback_texture;
} renderer_system_state;

static renderer_system_state *state_ptr;

b8 renderer_system_initialize(u64 *memory_requirements,
                              void *state,
                              const char *application_name) {
  *memory_requirements = sizeof(renderer_system_state);
  if (!state) return true;
  state_ptr = state;

  renderer_backend_create(RENDERER_BACKEND_TYPE_VULKAN, &state_ptr->backend);
  state_ptr->backend.frame_number = 0;

  if (!state_ptr->backend.initialize(&state_ptr->backend, application_name)) {
    KFATAL("Renderer backend initialization failed. Shutting down the engine...");
    return false;
  }

  state_ptr->near_clip = 0.1f;
  state_ptr->far_clip = 1000.0f;
  state_ptr->proj = mat4_persp_proj(deg_to_rad(45.0f),
                                   PROJ_MATRIX_ASPECT_RATIO,
                                   state_ptr->near_clip,
                                   state_ptr->far_clip);
  state_ptr->view = mat4_inv(mat4_translation((Vector3) {{{ 0, 0, -30.0f }}}));

  // renderer_create_fallback_texture();

  return true;
}

void renderer_system_shutdown(void *state) {
  (void) state;  // Unused parameter

  if (!state_ptr) return;
  // renderer_destroy_texture(&state_ptr->fallback_texture);
  state_ptr->backend.shutdown(&state_ptr->backend);
  state_ptr = 0;
}

void renderer_on_resized(u16 width, u16 height) {
  if (state_ptr) {
    state_ptr->proj = mat4_persp_proj(deg_to_rad(45.0f),
                                     (f32) width / height,
                                     state_ptr->near_clip,
                                     state_ptr->far_clip);
    state_ptr->backend.resized(&state_ptr->backend, width, height);
  }
  else KWARN("renderer_on_resized :: Renderer backend does not exist");
}

b8 renderer_begin_frame(f32 delta_time) {
  if (!state_ptr) return false;
  return state_ptr->backend.begin_frame(&state_ptr->backend, delta_time);
}

b8 renderer_end_frame(f32 delta_time) {
  if (!state_ptr) return false;
  b8 result = state_ptr->backend.end_frame(&state_ptr->backend, delta_time);
  ++(state_ptr->backend.frame_number);
  return result;
}

b8 renderer_draw_frame(render_packet *packet) {
  if (renderer_begin_frame(packet->delta_time)) {
    state_ptr->backend.update(state_ptr->proj, state_ptr->view, vec3_zero(), vec4_one(), 0);

    // Object rotation per frame
    static f32 angle = 0.01f;
    angle += 0.001f;
    Quaternion rot = euler_to_quat(vec3_forward(), angle, false);

    Matrix4 model = quat_to_mat4_center(rot, vec3_zero());
    geometry_render_data data = {
      .object_id = 0,
      .model = model,
      .textures[0] = &state_ptr->fallback_texture
    };
    state_ptr->backend.update_object(data);

    b8 result = renderer_end_frame(packet->delta_time);
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

void renderer_create_texture(const char *name,
                             b8 auto_release,
                             i32 width,
                             i32 height,
                             i32 channel_count,
                             const u8 *pixels,
                             b8 has_transparency,
                             texture *out_texture) {
  state_ptr->backend.create_texture(name,
                                   auto_release,
                                   width,
                                   height,
                                   channel_count,
                                   pixels,
                                   has_transparency,
                                   out_texture);
}

void renderer_destroy_texture(texture *texture) {
  state_ptr->backend.destroy_texture(texture);
}

void renderer_create_fallback_texture(void) {
  // Fallback texture creation (256x256 blue+white checkerboard pattern)
  const i32 texture_size = 256;
  const i32 channels = 4;
  const i32 pixel_count = texture_size * texture_size;
  u8 *pixels = kallocate(sizeof(u8) * pixel_count * channels, MEMORY_TAG_TEXTURE);
  // u8 pixels[pixel_count * channels];
  // set all pixels to white
  kset_memory(pixels, texture_size - 1, sizeof(u8) * pixel_count * channels);
  for (u64 i = 0; i < texture_size; ++i) {
    for (u64 j = 0; j < texture_size; ++j) {
      u64 idx = (i * texture_size) + j;
      u64 idx_bpp = idx * channels;
      // set alternating pixels to blue (setting red/green channels to 0)
      if (i % 2 && j % 2) {
        pixels[idx_bpp]     = 0;
        pixels[idx_bpp + 1] = 0;
      }
      else if (!(j % 2)) {
        pixels[idx_bpp]     = 0;
        pixels[idx_bpp + 1] = 0;
      }
    }
  }
  KTRACE("Creating fallback texture...");
  renderer_create_texture("fallback",
                          false,
                          texture_size,
                          texture_size,
                          channels,
                          pixels,
                          false,
                          &state_ptr->fallback_texture);
  KTRACE("Fallback texture created");
}
