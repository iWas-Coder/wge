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


#pragma once

#include <renderer_backend.h>

b8 vulkan_renderer_backend_initialize(renderer_backend *backend, const char *application_name);
void vulkan_renderer_backend_shutdown(renderer_backend *backend);
void vulkan_renderer_backend_on_resized(renderer_backend *backend, u16 width, u16 height);
b8 vulkan_renderer_backend_begin_frame(renderer_backend *backend, f32 delta_time);
void vulkan_renderer_backend_update(Matrix4 proj,
                                    Matrix4 view,
                                    Vector3 view_pos,
                                    Vector4 ambient_color,
                                    i32 mode);
b8 vulkan_renderer_backend_end_frame(renderer_backend *backend, f32 delta_time);
void vulkan_renderer_backend_update_object(Matrix4 model);
