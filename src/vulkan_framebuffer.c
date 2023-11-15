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
#include <vulkan_framebuffer.h>

void vulkan_framebuffer_create(vulkan_context *context,
                               vulkan_renderpass *renderpass,
                               u32 width,
                               u32 height,
                               u32 attachment_count,
                               VkImageView *attachments,
                               vulkan_framebuffer *out_framebuffer) {
  out_framebuffer->attachments = kallocate(attachment_count * sizeof(VkImageView),
                                          MEMORY_TAG_RENDERER);
  for (u32 i = 0; i < attachment_count; ++i) out_framebuffer->attachments[i] = attachments[i];
  out_framebuffer->renderpass = renderpass;
  out_framebuffer->attachment_count = attachment_count;

  VkFramebufferCreateInfo framebuffer_create_info = {
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .renderPass = renderpass->handle,
    .attachmentCount = attachment_count,
    .pAttachments = out_framebuffer->attachments,
    .width = width,
    .height = height,
    .layers = 1
  };
  VK_CHECK(vkCreateFramebuffer(context->device.logical_device,
                               &framebuffer_create_info,
                               context->allocator,
                               &out_framebuffer->handle));
}

void vulkan_framebuffer_destroy(vulkan_context *context, vulkan_framebuffer *framebuffer) {
  vkDestroyFramebuffer(context->device.logical_device,
                       framebuffer->handle,
                       context->allocator);
  if (framebuffer->attachments) {
    kfree(framebuffer->attachments,
          framebuffer->attachment_count * sizeof(VkImageView),
          MEMORY_TAG_RENDERER);
    framebuffer->attachments = 0;
  }
  framebuffer->handle = 0;
  framebuffer->attachment_count = 0;
  framebuffer->renderpass = 0;
}
