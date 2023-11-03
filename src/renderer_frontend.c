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


#include <logger.h>
#include <kmemory.h>
#include <renderer_backend.h>
#include <renderer_frontend.h>

static renderer_backend *backend = 0;

b8 renderer_initialize(const char *application_name, struct platform_state *plat_state) {
  backend = kallocate(sizeof(renderer_backend), MEMORY_TAG_RENDERER);
  renderer_backend_create(RENDERER_BACKEND_TYPE_VULKAN, plat_state, backend);
  backend->frame_number = 0;

  if (!backend->initialize(backend, application_name, plat_state)) {
    KFATAL("Renderer backend initialization failed. Shutting down the engine...");
    return FALSE;
  }
  return TRUE;
}

void renderer_shutdown(void) {
  backend->shutdown(backend);
  kfree(backend, sizeof(renderer_backend), MEMORY_TAG_RENDERER);
}

void renderer_on_resized(u16 width, u16 height) {
  (void) width;  // Unused parameter
  (void) height;  // Unused parameter
}

b8 renderer_begin_frame(f32 delta_time) {
  return backend->begin_frame(backend, delta_time);
}

b8 renderer_end_frame(f32 delta_time) {
  b8 result = backend->end_frame(backend, delta_time);
  ++(backend->frame_number);
  return result;
}

b8 renderer_draw_frame(render_packet *packet) {
  if (renderer_begin_frame(packet->delta_time)) {
    b8 result = renderer_end_frame(packet->delta_time);
    if (!result) {
      KERROR("`renderer_draw_frame` failed. Shutting down the engine...");
      return FALSE;
    }
  }
  return TRUE;
}
