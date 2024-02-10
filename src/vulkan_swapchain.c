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
#include <vulkan_image.h>
#include <vulkan_device.h>
#include <vulkan_swapchain.h>

static void create(vulkan_context *context, u32 width, u32 height, vulkan_swapchain *swapchain) {
  // Query swapchain support for the first time
  vulkan_device_query_swapchain_support(context->device.physical_device,
                                        context->surface,
                                        &context->device.swapchain_support);

  VkExtent2D swapchain_extent = { width, height };

  // Select surface format
  b8 found = false;
  for (u32 i = 0; i < context->device.swapchain_support.format_count; ++i) {
    VkSurfaceFormatKHR format = context->device.swapchain_support.formats[i];
    if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      swapchain->image_format = format;
      found = true;
      break;
    }
  }
  if (!found) swapchain->image_format = context->device.swapchain_support.formats[0];

  // Select present mode. Available values:
  /*
    FIFO
    ====
    Wait for the next VBLANK period to update the current image.
    i.e. "VSync enabled".

    FIFO_RELAXED
    ============
    Wait for the next VBLANK period to update the current image,
    unless a VBLANK period has already passed since the last update
    of the current image, in that case do not wait for another VBLANK
    period for the update.
    i.e. IMMEDIATE when fps < refresh-rate; FIFO when fps >= refresh-rate.

    MAILBOX
    =======
    Wait for the next VBLANK period to update the current image, but in the next
    update a newer image may be presented (previously rendered images may not be presented).
    i.e. FIFO with uncapped frame rendering.

    IMMEDIATE
    =========
    Do not wait for a VBLANK period to update the current image.
    i.e. "VSync disabled".
  */
  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;  // FIFO by default
  for (u32 i = 0; i < context->device.swapchain_support.present_mode_count; ++i) {
    VkPresentModeKHR mode = context->device.swapchain_support.present_modes[i];
    if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {  // IMMEDIATE if available
      present_mode = mode;
      break;
    }
  }
  // Re-query swapchain support
  vulkan_device_query_swapchain_support(context->device.physical_device,
                                        context->surface,
                                        &context->device.swapchain_support);
  if (context->device.swapchain_support.capabilities.currentExtent.width != UINT32_MAX) {
    swapchain_extent = context->device.swapchain_support.capabilities.currentExtent;
  }

  // Clamp the extent to allowed values
  VkExtent2D min = context->device.swapchain_support.capabilities.minImageExtent;
  VkExtent2D max = context->device.swapchain_support.capabilities.maxImageExtent;
  swapchain_extent.width = KCLAMP(swapchain_extent.width, min.width, max.width);
  swapchain_extent.height = KCLAMP(swapchain_extent.height, min.height, max.height);

  // Get image count
  u32 image_count = context->device.swapchain_support.capabilities.maxImageCount + 1;
  if (context->device.swapchain_support.capabilities.maxImageCount > 0 &&
      image_count > context->device.swapchain_support.capabilities.maxImageCount) {
    image_count = context->device.swapchain_support.capabilities.maxImageCount;
  }

  swapchain->max_frames_in_flight = image_count - 1;  // double of triple buffering

  // Swapchain create info
  VkSwapchainCreateInfoKHR swapchain_create_info = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = context->surface,
    .minImageCount = image_count,
    .imageFormat = swapchain->image_format.format,
    .imageColorSpace = swapchain->image_format.colorSpace,
    .imageExtent = swapchain_extent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,  // color buffer
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices = 0,
    .preTransform = context->device.swapchain_support.capabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = present_mode,
    .clipped = VK_TRUE,
    .oldSwapchain = 0
  };
  if (context->device.graphics != context->device.present) {
    u32 queueFamilyIndices[] = {
      (u32) context->device.graphics,
      (u32) context->device.present
    };
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_create_info.queueFamilyIndexCount = 2;
    swapchain_create_info.pQueueFamilyIndices = queueFamilyIndices;
  }

  // Create swapchain
  VK_CHECK(vkCreateSwapchainKHR(context->device.logical_device,
                                &swapchain_create_info,
                                context->allocator,
                                &swapchain->handle));

  // Images
  context->current_frame = 0;
  swapchain->image_count = 0;
  VK_CHECK(vkGetSwapchainImagesKHR(context->device.logical_device,
                                   swapchain->handle,
                                   &swapchain->image_count,
                                   0));
  if (!swapchain->images) swapchain->images = (VkImage *) kallocate(sizeof(VkImage) * swapchain->image_count,
                                                                  MEMORY_TAG_RENDERER);
  if (!swapchain->views) swapchain->views = (VkImageView *) kallocate(sizeof(VkImageView) * swapchain->image_count,
                                                                    MEMORY_TAG_RENDERER);
  VK_CHECK(vkGetSwapchainImagesKHR(context->device.logical_device,
                                   swapchain->handle,
                                   &swapchain->image_count,
                                   swapchain->images));

  // Views
  for (u32 i = 0; i < swapchain->image_count; ++i) {
    VkImageViewCreateInfo view_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = swapchain->images[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = swapchain->image_format.format,
      .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel = 0,
      .subresourceRange.levelCount = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount = 1
    };
    VK_CHECK(vkCreateImageView(context->device.logical_device,
                               &view_info,
                               context->allocator,
                               &swapchain->views[i]));
  }

  // Depth
  if (!vulkan_device_detect_depth_format(&context->device)) {
    context->device.depth_format = VK_FORMAT_UNDEFINED;
    KFATAL("No supported format found");
  }

  // Create depth's image and view
  vulkan_image_create(context,
                      VK_IMAGE_TYPE_2D,
                      swapchain_extent.width,
                      swapchain_extent.height,
                      context->device.depth_format,
                      VK_IMAGE_TILING_OPTIMAL,
                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                      true,
                      VK_IMAGE_ASPECT_DEPTH_BIT,
                      &swapchain->depth_attachment);

  KINFO("Vulkan swapchain created");
}

static void destroy(vulkan_context *context, vulkan_swapchain *swapchain) {
  vkDeviceWaitIdle(context->device.logical_device);
  vulkan_image_destroy(context, &swapchain->depth_attachment);
  for (u32 i = 0; i < swapchain->image_count; ++i) {
    vkDestroyImageView(context->device.logical_device,
                       swapchain->views[i],
                       context->allocator);
  }
  vkDestroySwapchainKHR(context->device.logical_device,
                        swapchain->handle,
                        context->allocator);
}

void vulkan_swapchain_create(vulkan_context *context, u32 width, u32 height, vulkan_swapchain *swapchain) {
  create(context, width, height, swapchain);
}

void vulkan_swapchain_recreate(vulkan_context *context, u32 width, u32 height, vulkan_swapchain *swapchain) {
  destroy(context, swapchain);
  create(context, width, height, swapchain);
}

void vulkan_swapchain_destroy(vulkan_context *context, vulkan_swapchain *swapchain) {
  destroy(context, swapchain);
}

b8 vulkan_swapchain_get_next_image_index(vulkan_context *context,
                                         vulkan_swapchain *swapchain,
                                         u64 timeout_ns,
                                         VkSemaphore image_available_semaphore,
                                         VkFence fence,
                                         u32 *out_image_index) {
  VkResult result = vkAcquireNextImageKHR(context->device.logical_device,
                                          swapchain->handle,
                                          timeout_ns,
                                          image_available_semaphore,
                                          fence,
                                          out_image_index);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    vulkan_swapchain_recreate(context,
                              context->framebuffer_width,
                              context->framebuffer_height,
                              swapchain);
    return false;
  }
  else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    KFATAL("Get swapchain image failed");
    return false;
  }
  return true;
}

void vulkan_swapchain_present(vulkan_context *context,
                              vulkan_swapchain *swapchain,
                              VkQueue graphics_queue,
                              VkQueue present_queue,
                              VkSemaphore render_complete_semaphore,
                              u32 present_image_index) {
  (void) graphics_queue;  // Unused parameter

  VkPresentInfoKHR present_info = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &render_complete_semaphore,
    .swapchainCount = 1,
    .pSwapchains = &swapchain->handle,
    .pImageIndices = &present_image_index,
    .pResults = 0
  };
  VkResult result = vkQueuePresentKHR(present_queue, &present_info);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    vulkan_swapchain_recreate(context,
                              context->framebuffer_width,
                              context->framebuffer_height,
                              swapchain);
  }
  else if (result != VK_SUCCESS) KFATAL("Present swapchain image failed");

  // Increment the current frame counter looping it through 'max_frames_in_flight' as limit
  context->current_frame = (context->current_frame + 1) % swapchain->max_frames_in_flight;
}
