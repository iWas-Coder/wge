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

void vulkan_image_create(vulkan_context *context,
                         VkImageType image_type,
                         u32 width,
                         u32 height,
                         VkFormat format,
                         VkImageTiling tiling,
                         VkImageUsageFlags usage,
                         VkMemoryPropertyFlags memory_flags,
                         b32 create_view,
                         VkImageAspectFlags view_aspect_flags,
                         vulkan_image *out_image) {
  out_image->width  = width;
  out_image->height = height;

  // Image create info
  VkImageCreateInfo image_create_info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .imageType = image_type,
    .extent.width = width,
    .extent.height = height,
    .extent.depth = 1,
    .mipLevels = 4,
    .arrayLayers = 1,
    .format = format,
    .tiling = tiling,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .usage = usage,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE
  };

  // Create image
  VK_CHECK(vkCreateImage(context->device.logical_device,
                         &image_create_info,
                         context->allocator,
                         &out_image->handle));

  // Memory requirements
  VkMemoryRequirements memory_requirements;
  vkGetImageMemoryRequirements(context->device.logical_device,
                               out_image->handle,
                               &memory_requirements);
  i32 memory_type = context->find_memory_index(memory_requirements.memoryTypeBits,
                                              memory_flags);
  if (memory_type == -1) KERROR("Image not valid :: Required memory type not found");

  // Allocate memory
  VkMemoryAllocateInfo memory_allocate_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .allocationSize = memory_requirements.size,
    .memoryTypeIndex = memory_type
  };
  VK_CHECK(vkAllocateMemory(context->device.logical_device,
                            &memory_allocate_info,
                            context->allocator,
                            &out_image->memory));
  VK_CHECK(vkBindImageMemory(context->device.logical_device,
                             out_image->handle,
                             out_image->memory,
                             0));

  // Create view
  if (create_view) {
    out_image->view = 0;
    vulkan_image_view_create(context, format, out_image, view_aspect_flags);
  }
}

void vulkan_image_view_create(vulkan_context *context,
                              VkFormat format,
                              vulkan_image *image,
                              VkImageAspectFlags aspect_flags) {
  VkImageViewCreateInfo view_create_info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .image = image->handle,
    .viewType = VK_IMAGE_VIEW_TYPE_2D,
    .format = format,
    .subresourceRange.aspectMask = aspect_flags,
    .subresourceRange.baseMipLevel = 0,
    .subresourceRange.levelCount = 1,
    .subresourceRange.baseArrayLayer = 0,
    .subresourceRange.layerCount = 1
  };
  VK_CHECK(vkCreateImageView(context->device.logical_device,
                             &view_create_info,
                             context->allocator,
                             &image->view));
}

void vulkan_image_destroy(vulkan_context *context, vulkan_image *image) {
  if (image->view) {
    vkDestroyImageView(context->device.logical_device, image->view, context->allocator);
    image->view = 0;
  }
  if (image->memory) {
    vkFreeMemory(context->device.logical_device, image->memory, context->allocator);
    image->memory = 0;
  }
  if (image->handle) {
    vkDestroyImage(context->device.logical_device, image->handle, context->allocator);
    image->handle = 0;
  }
}
