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

#include <vulkan_types.h>
#include <renderer_types.h>

b8 vulkan_material_shader_create(vulkan_context *context, vulkan_material_shader *out_shader);

void vulkan_material_shader_destroy(vulkan_context *context, vulkan_material_shader *shader);

void vulkan_material_shader_use(vulkan_context *context, vulkan_material_shader *shader);

void vulkan_material_shader_update(vulkan_context *context,
                                   vulkan_material_shader *shader,
                                   f32 delta_time);

void vulkan_material_shader_set_model(vulkan_context *context,
                                      vulkan_material_shader *shader,
                                      Matrix4 model);

void vulkan_material_shader_apply_material(vulkan_context *context,
                                           vulkan_material_shader *shader,
                                           material *material);

b8 vulkan_material_shader_get_resources(vulkan_context *context,
                                        vulkan_material_shader *shader,
                                        material *material);

void vulkan_material_shader_release_resources(vulkan_context *context,
                                              vulkan_material_shader *shader,
                                              material *material);
