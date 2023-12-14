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
#include <texture_system.h>
#include <renderer_backend.h>
#include <renderer_frontend.h>

// TEMPORARY include
#include <event.h>

#define PROJ_MATRIX_ASPECT_RATIO (16 / 9.0f)

typedef struct {
  renderer_backend backend;
  Matrix4 proj;
  Matrix4 view;
  f32 near_clip;
  f32 far_clip;
  // TEMPORARY texture to test things out
  texture *test_diffuse;
} renderer_system_state;

static renderer_system_state *state_ptr;

// TEMPORARY function to handle event which swaps the texture while app is running
b8 event_on_debug(u16 code, void *sender, void *listener_inst, event_context data) {
  (void) code;           // Unused parameter
  (void) sender;         // Unused parameter
  (void) listener_inst;  // Unused parameter
  (void) data;           // Unused parameter

  const char *names[] = {
    "cobblestone",
    "paving",
    "paving2"
  };
  static u8 choice = 2;
  const char *old_name = names[choice];
  ++choice;
  choice %= 3;

  state_ptr->test_diffuse = texture_system_get(names[choice], true);
  texture_system_release(old_name);
  return true;
}

b8 renderer_system_initialize(u64 *memory_requirements,
                              void *state,
                              const char *application_name) {
  *memory_requirements = sizeof(renderer_system_state);
  if (!state) return true;
  state_ptr = state;

  // TEMPORARY register for the debug event to swap textures
  event_register(EVENT_CODE_DEBUG0, state_ptr, event_on_debug);

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

  return true;
}

void renderer_system_shutdown(void *state) {
  (void) state;  // Unused parameter

  if (!state_ptr) return;
  // TEMPORARY unregister for the debug event to swap textures
  event_unregister(EVENT_CODE_DEBUG0, state_ptr, event_on_debug);

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
    angle += 0.0001f;
    Quaternion rot = euler_to_quat(vec3_forward(), angle, false);

    Matrix4 model = quat_to_mat4_center(rot, vec3_zero());
    if (!state_ptr->test_diffuse) state_ptr->test_diffuse = texture_system_get_fallback();
    geometry_render_data data = {
      .object_id = 0,
      .model = model,
      .textures[0] = state_ptr->test_diffuse  // TEMPORARY texture
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
                             i32 width,
                             i32 height,
                             i32 channel_count,
                             const u8 *pixels,
                             b8 has_transparency,
                             texture *out_texture) {
  state_ptr->backend.create_texture(name,
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
