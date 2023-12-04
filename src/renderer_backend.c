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


#include <vulkan_backend.h>
#include <renderer_backend.h>

b8 renderer_backend_create(renderer_backend_type type,
                           renderer_backend *out_renderer_backend) {
  switch (type) {
  case RENDERER_BACKEND_TYPE_VULKAN:
    out_renderer_backend->initialize      = vulkan_renderer_backend_initialize;
    out_renderer_backend->shutdown        = vulkan_renderer_backend_shutdown;
    out_renderer_backend->resized         = vulkan_renderer_backend_on_resized;
    out_renderer_backend->begin_frame     = vulkan_renderer_backend_begin_frame;
    out_renderer_backend->update          = vulkan_renderer_backend_update;
    out_renderer_backend->end_frame       = vulkan_renderer_backend_end_frame;
    out_renderer_backend->update_object   = vulkan_renderer_backend_update_object;
    out_renderer_backend->create_texture  = vulkan_renderer_backend_create_texture;
    out_renderer_backend->destroy_texture = vulkan_renderer_backend_destroy_texture;
    return true;
  case RENDERER_BACKEND_TYPE_OPENGL:
    // TODO
    break;
  case RENDERER_BACKEND_TYPE_DIRECTX:
    // TODO
    break;
  }

  return false;
}
void renderer_backend_destroy(renderer_backend *renderer_backend) {
  renderer_backend->initialize      = 0;
  renderer_backend->shutdown        = 0;
  renderer_backend->resized         = 0;
  renderer_backend->begin_frame     = 0;
  renderer_backend->update          = 0;
  renderer_backend->end_frame       = 0;
  renderer_backend->update_object   = 0;
  renderer_backend->create_texture  = 0;
  renderer_backend->destroy_texture = 0;
}
