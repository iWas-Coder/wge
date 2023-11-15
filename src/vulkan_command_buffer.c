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


#include <kmemory.h>
#include <vulkan_command_buffer.h>

void vulkan_command_buffer_allocate(vulkan_context *context,
                                    VkCommandPool pool,
                                    b8 is_primary,
                                    vulkan_command_buffer *out_command_buffer) {
  kzero_memory(out_command_buffer, sizeof(out_command_buffer));
  VkCommandBufferAllocateInfo allocate_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = pool,
    .level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
    .commandBufferCount = 1,
    .pNext = 0
  };
  out_command_buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
  VK_CHECK(vkAllocateCommandBuffers(context->device.logical_device,
                                    &allocate_info,
                                    &out_command_buffer->handle));
  out_command_buffer->state = COMMAND_BUFFER_STATE_READY;
}

void vulkan_command_buffer_free(vulkan_context *context,
                                VkCommandPool pool,
                                vulkan_command_buffer *command_buffer) {
  vkFreeCommandBuffers(context->device.logical_device,
                       pool,
                       1,
                       &command_buffer->handle);
  command_buffer->handle = 0;
  command_buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

void vulkan_command_buffer_begin(vulkan_command_buffer *command_buffer,
                                 b8 is_single_use,
                                 b8 is_renderpass_continue,
                                 b8 is_simultaneous_use) {
  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = 0
  };
  if (is_single_use) begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  if (is_renderpass_continue) begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
  if (is_simultaneous_use) begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

  VK_CHECK(vkBeginCommandBuffer(command_buffer->handle, &begin_info));
  command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;
}

void vulkan_command_buffer_end(vulkan_command_buffer *command_buffer) {
  VK_CHECK(vkEndCommandBuffer(command_buffer->handle));
  command_buffer->state = COMMAND_BUFFER_STATE_RECORDING_ENDED;
}

void vulkan_command_buffer_update(vulkan_command_buffer *command_buffer) {
  command_buffer->state = COMMAND_BUFFER_STATE_SUBMITTED;
}

void vulkan_command_buffer_reset(vulkan_command_buffer *command_buffer) {
  command_buffer->state = COMMAND_BUFFER_STATE_READY;
}

void vulkan_command_buffer_oneshot_start(vulkan_context *context,
                                         VkCommandPool pool,
                                         vulkan_command_buffer *out_command_buffer) {
  vulkan_command_buffer_allocate(context, pool, TRUE, out_command_buffer);
  vulkan_command_buffer_begin(out_command_buffer, TRUE, FALSE, FALSE);
}

void vulkan_command_buffer_oneshot_end(vulkan_context *context,
                                       VkCommandPool pool,
                                       vulkan_command_buffer *command_buffer,
                                       VkQueue queue) {
  vulkan_command_buffer_end(command_buffer);
  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &command_buffer->handle
  };
  VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, 0));
  VK_CHECK(vkQueueWaitIdle(queue));
  vulkan_command_buffer_free(context, pool, command_buffer);
}
