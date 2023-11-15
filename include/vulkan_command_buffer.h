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

void vulkan_command_buffer_allocate(vulkan_context *context,
                                    VkCommandPool pool,
                                    b8 is_primary,
                                    vulkan_command_buffer *out_command_buffer);

void vulkan_command_buffer_free(vulkan_context *context,
                                VkCommandPool pool,
                                vulkan_command_buffer *command_buffer);

void vulkan_command_buffer_begin(vulkan_command_buffer *command_buffer,
                                 b8 is_single_use,
                                 b8 is_renderpass_continue,
                                 b8 is_simultaneous_use);

void vulkan_command_buffer_end(vulkan_command_buffer *command_buffer);

void vulkan_command_buffer_update(vulkan_command_buffer *command_buffer);

void vulkan_command_buffer_reset(vulkan_command_buffer *command_buffer);

void vulkan_command_buffer_oneshot_start(vulkan_context *context,
                                         VkCommandPool pool,
                                         vulkan_command_buffer *out_command_buffer);

void vulkan_command_buffer_oneshot_end(vulkan_context *context,
                                       VkCommandPool pool,
                                       vulkan_command_buffer *command_buffer,
                                       VkQueue queue);
