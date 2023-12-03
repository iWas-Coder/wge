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

#include <renderer_types.h>

// Forward declarations
struct platform_state;
struct static_mesh_data;

b8 renderer_system_initialize(u64 *memory_requirements,
                              void *state,
                              const char *application_name);
void renderer_system_shutdown(void *state);
void renderer_on_resized(u16 width, u16 height);
b8 renderer_draw_frame(render_packet *packet);
// This function should not have external visibility, but it does for now :D
KAPI void renderer_set_view(Matrix4 view);
