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

b8 vulkan_buffer_create(vulkan_context *context,
                        u64 size,
                        VkBufferUsageFlagBits usage,
                        u32 memory_property_flags,
                        b8 bind_on_create,
                        vulkan_buffer *out_buffer);

void vulkan_buffer_destroy(vulkan_context *context, vulkan_buffer *buffer);

void *vulkan_buffer_lock(vulkan_context *context,
                         vulkan_buffer *buffer,
                         u64 offset,
                         u64 size,
                         u32 flags);
void vulkan_buffer_unlock(vulkan_context *context, vulkan_buffer *buffer);

void vulkan_buffer_load(vulkan_context *context,
                        vulkan_buffer *buffer,
                        u64 offset,
                        u64 size,
                        u32 flags,
                        const void *data);

void vulkan_buffer_copy(vulkan_context *context,
                        VkCommandPool pool,
                        VkFence fence,
                        VkQueue queue,
                        VkBuffer src,
                        u64 src_offset,
                        VkBuffer dst,
                        u64 dst_offset,
                        u64 size);

b8 vulkan_buffer_resize(vulkan_context *context,
                        u64 new_size,
                        vulkan_buffer *buffer,
                        VkQueue queue,
                        VkCommandPool pool);

void vulkan_buffer_bind(vulkan_context *context,
                        vulkan_buffer *buffer,
                        u64 offset);
