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
    out_renderer_backend->initialize       = vulkan_renderer_backend_initialize;
    out_renderer_backend->shutdown         = vulkan_renderer_backend_shutdown;
    out_renderer_backend->resized          = vulkan_renderer_backend_on_resized;
    out_renderer_backend->begin_frame      = vulkan_renderer_backend_begin_frame;
    out_renderer_backend->update_world     = vulkan_renderer_backend_update_world;
    out_renderer_backend->update_ui        = vulkan_renderer_backend_update_ui;
    out_renderer_backend->end_frame        = vulkan_renderer_backend_end_frame;
    out_renderer_backend->begin_renderpass = vulkan_renderer_backend_begin_renderpass;
    out_renderer_backend->end_renderpass   = vulkan_renderer_backend_end_renderpass;
    out_renderer_backend->draw_geometry    = vulkan_renderer_backend_draw_geometry;
    out_renderer_backend->create_geometry  = vulkan_renderer_backend_create_geometry;
    out_renderer_backend->destroy_geometry = vulkan_renderer_backend_destroy_geometry;
    out_renderer_backend->create_texture   = vulkan_renderer_backend_create_texture;
    out_renderer_backend->destroy_texture  = vulkan_renderer_backend_destroy_texture;
    out_renderer_backend->create_material  = vulkan_renderer_backend_create_material;
    out_renderer_backend->destroy_material = vulkan_renderer_backend_destroy_material;
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
  renderer_backend->initialize       = 0;
  renderer_backend->shutdown         = 0;
  renderer_backend->resized          = 0;
  renderer_backend->begin_frame      = 0;
  renderer_backend->update_world     = 0;
  renderer_backend->update_ui        = 0;
  renderer_backend->end_frame        = 0;
  renderer_backend->begin_renderpass = 0;
  renderer_backend->end_renderpass   = 0;
  renderer_backend->draw_geometry    = 0;
  renderer_backend->create_geometry  = 0;
  renderer_backend->destroy_geometry = 0;
  renderer_backend->create_texture   = 0;
  renderer_backend->destroy_texture  = 0;
  renderer_backend->create_material  = 0;
  renderer_backend->destroy_material = 0;
}
