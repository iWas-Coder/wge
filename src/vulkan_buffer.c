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
#include <vulkan_utils.h>
#include <vulkan_buffer.h>
#include <vulkan_device.h>
#include <vulkan_command_buffer.h>

b8 vulkan_buffer_create(vulkan_context *context,
                        u64 size,
                        VkBufferUsageFlagBits usage,
                        u32 memory_property_flags,
                        b8 bind_on_create,
                        vulkan_buffer *out_buffer) {
  kzero_memory(out_buffer, sizeof(vulkan_buffer));
  out_buffer->total_size = size;
  out_buffer->usage = usage;
  out_buffer->memory_property_flags = memory_property_flags;

  VkBufferCreateInfo buffer_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = size,
    .usage = usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE
  };
  VK_CHECK(vkCreateBuffer(context->device.logical_device,
                          &buffer_info,
                          context->allocator,
                          &out_buffer->handle));

  VkMemoryRequirements requirements;
  vkGetBufferMemoryRequirements(context->device.logical_device,
                                out_buffer->handle,
                                &requirements);
  out_buffer->memory_index = context->find_memory_index(requirements.memoryTypeBits,
                                                      out_buffer->memory_property_flags);
  if (out_buffer->memory_index == -1) {
    KERROR("vulkan_buffer_create :: required memory type idx not found");
    return false;
  }

  VkMemoryAllocateInfo allocate_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = requirements.size,
    .memoryTypeIndex = (u32) out_buffer->memory_index
  };
  VkResult result = vkAllocateMemory(context->device.logical_device,
                                     &allocate_info,
                                     context->allocator,
                                     &out_buffer->memory);
  if (result != VK_SUCCESS) {
    KERROR("vulkan_buffer_create :: required memory allocation failed (%i)", result);
    return false;
  }
  if (bind_on_create) vulkan_buffer_bind(context, out_buffer, 0);

  return true;
}

void vulkan_buffer_destroy(vulkan_context *context, vulkan_buffer *buffer) {
  if (buffer->memory) {
    vkFreeMemory(context->device.logical_device,
                 buffer->memory,
                 context->allocator);
    buffer->memory = 0;
  }
  if (buffer->handle) {
    vkDestroyBuffer(context->device.logical_device,
                    buffer->handle,
                    context->allocator);
    buffer->handle = 0;
  }
  buffer->total_size = 0;
  buffer->usage = 0;
  buffer->is_locked = false;
}

void *vulkan_buffer_lock(vulkan_context *context,
                         vulkan_buffer *buffer,
                         u64 offset,
                         u64 size,
                         u32 flags) {
  void *data;
  VK_CHECK(vkMapMemory(context->device.logical_device,
                       buffer->memory,
                       offset,
                       size,
                       flags,
                       &data));
  return data;
}

void vulkan_buffer_unlock(vulkan_context *context, vulkan_buffer *buffer) {
  vkUnmapMemory(context->device.logical_device, buffer->memory);
}

void vulkan_buffer_load(vulkan_context *context,
                        vulkan_buffer *buffer,
                        u64 offset,
                        u64 size,
                        u32 flags,
                        const void *data) {
  void *data_ptr;
  VK_CHECK(vkMapMemory(context->device.logical_device,
                       buffer->memory,
                       offset,
                       size,
                       flags,
                       &data_ptr));
  kcopy_memory(data_ptr, data, size);
  vkUnmapMemory(context->device.logical_device, buffer->memory);
}

void vulkan_buffer_copy(vulkan_context *context,
                        VkCommandPool pool,
                        VkFence fence,
                        VkQueue queue,
                        VkBuffer src,
                        u64 src_offset,
                        VkBuffer dst,
                        u64 dst_offset,
                        u64 size) {
  (void) fence;  // Unused parameter

  vkQueueWaitIdle(queue);

  vulkan_command_buffer tmp_cmd_buf;
  vulkan_command_buffer_oneshot_start(context, pool, &tmp_cmd_buf);
  VkBufferCopy copy_region = {
    .srcOffset = src_offset,
    .dstOffset = dst_offset,
    .size = size
  };
  vkCmdCopyBuffer(tmp_cmd_buf.handle,
                  src,
                  dst,
                  1,
                  &copy_region);
  vulkan_command_buffer_oneshot_end(context, pool, &tmp_cmd_buf, queue);
}

b8 vulkan_buffer_resize(vulkan_context *context,
                        u64 new_size,
                        vulkan_buffer *buffer,
                        VkQueue queue,
                        VkCommandPool pool) {
  VkBufferCreateInfo buffer_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = new_size,
    .usage = buffer->usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE
  };
  VkBuffer new_buffer;
  VK_CHECK(vkCreateBuffer(context->device.logical_device,
                          &buffer_info,
                          context->allocator,
                          &new_buffer));

  VkMemoryRequirements requirements;
  vkGetBufferMemoryRequirements(context->device.logical_device,
                                new_buffer,
                                &requirements);

  VkMemoryAllocateInfo allocate_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = requirements.size,
    .memoryTypeIndex = (u32) buffer->memory_index
  };
  VkDeviceMemory new_memory;
  VkResult result = vkAllocateMemory(context->device.logical_device,
                                     &allocate_info,
                                     context->allocator,
                                     &new_memory);
  if (result != VK_SUCCESS) {
    KERROR("vulkan_buffer_resize :: required memory allocation failed (%i)", result);
    return false;
  }
  VK_CHECK(vkBindBufferMemory(context->device.logical_device,
                              new_buffer,
                              new_memory,
                              0));
  vulkan_buffer_copy(context,
                     pool,
                     0,
                     queue,
                     buffer->handle,
                     0,
                     new_buffer,
                     0,
                     buffer->total_size);

  vkDeviceWaitIdle(context->device.logical_device);

  if (buffer->memory) {
    vkFreeMemory(context->device.logical_device,
                 buffer->memory,
                 context->allocator);
    buffer->memory = 0;
  }
  if (buffer->handle) {
    vkDestroyBuffer(context->device.logical_device,
                    buffer->handle,
                    context->allocator);
    buffer->handle = 0;
  }
  buffer->total_size = new_size;
  buffer->memory = new_memory;
  buffer->handle = new_buffer;

  return true;
}

void vulkan_buffer_bind(vulkan_context *context,
                        vulkan_buffer *buffer,
                        u64 offset) {
  VK_CHECK(vkBindBufferMemory(context->device.logical_device,
                              buffer->handle,
                              buffer->memory,
                              offset));
}
