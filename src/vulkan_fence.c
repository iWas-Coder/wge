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
#include <vulkan_fence.h>

void vulkan_fence_create(vulkan_context *context,
                         b8 create_signaled,
                         vulkan_fence *out_fence) {
  out_fence->is_signaled = create_signaled;
  VkFenceCreateInfo fence_create_info = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
  };
  if (out_fence->is_signaled) fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  VK_CHECK(vkCreateFence(context->device.logical_device,
                         &fence_create_info,
                         context->allocator,
                         &out_fence->handle));
}

void vulkan_fence_destroy(vulkan_context *context, vulkan_fence *fence) {
  if (fence->handle) {
    vkDestroyFence(context->device.logical_device,
                   fence->handle,
                   context->allocator);
    fence->handle = 0;
  }
  fence->is_signaled = false;
}

b8 vulkan_fence_wait(vulkan_context *context,
                     vulkan_fence *fence,
                     u64 timeout) {
  if (fence->is_signaled) return true;

  VkResult result = vkWaitForFences(context->device.logical_device,
                                    1,
                                    &fence->handle,
                                    true,
                                    timeout);
  switch (result) {
  case VK_SUCCESS:
    fence->is_signaled = true;
    return true;
  case VK_TIMEOUT:
    KWARN("vulkan_fence_wait :: Timed out");
    break;
  case VK_ERROR_DEVICE_LOST:
    KERROR("vulkan_fence_wait :: Device lost");
    break;
  case VK_ERROR_OUT_OF_HOST_MEMORY:
    KERROR("vulkan_fence_wait :: Out of host memory");
    break;
  case VK_ERROR_OUT_OF_DEVICE_MEMORY:
    KERROR("vulkan_fence_wait :: Out of device memory");
    break;
  default:
    KERROR("vulkan_fence_wait :: Unknown error");
    break;
  }
  return false;
}

void vulkan_fence_reset(vulkan_context *context, vulkan_fence *fence) {
  if (fence->is_signaled) {
    VK_CHECK(vkResetFences(context->device.logical_device,
                           1,
                           &fence->handle));
    fence->is_signaled = false;
  }
}
