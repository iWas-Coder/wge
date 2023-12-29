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

typedef enum {
  RENDERPASS_CLEAR_NONE_FLAG           = 0x0,
  RENDERPASS_CLEAR_COLOR_BUFFER_FLAG   = 0x1,
  RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG   = 0x2,
  RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG = 0x4
} renderpass_clear_flag;

void vulkan_renderpass_create(vulkan_context *context,
                              vulkan_renderpass *out_renderpass,
                              Vector4 render_area,
                              Vector4 clear_color,
                              f32 depth,
                              u32 stencil,
                              u8 clear_flags,
                              b8 prev_pass,
                              b8 next_pass);

void vulkan_renderpass_destroy(vulkan_context *context,
                               vulkan_renderpass *renderpass);

void vulkan_renderpass_begin(vulkan_command_buffer *command_buffer,
                             vulkan_renderpass *renderpass,
                             VkFramebuffer framebuffer);

void vulkan_renderpass_end(vulkan_command_buffer *command_buffer,
                           vulkan_renderpass *renderpass);
