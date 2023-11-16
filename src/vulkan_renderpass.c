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
#include <vulkan_renderpass.h>

void vulkan_renderpass_create(vulkan_context *context,
                              vulkan_renderpass *out_renderpass,
                              f32 x, f32 y, f32 w, f32 h,
                              f32 r, f32 g, f32 b, f32 a,
                              f32 depth,
                              u32 stencil) {
  // Copy dimensions
  out_renderpass->x = x;
  out_renderpass->y = y;
  out_renderpass->w = w;
  out_renderpass->h = h;
  // Copy RGBA info
  out_renderpass->r = r;
  out_renderpass->g = g;
  out_renderpass->b = b;
  out_renderpass->a = a;
  // Copy depth & stencil info
  out_renderpass->depth = depth;
  out_renderpass->stencil = stencil;

  // Color attachment
  VkAttachmentDescription color_attachment = {
    .format = context->swapchain.image_format.format,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    .flags = 0
  };
  VkAttachmentReference color_attachment_ref = {
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
  };

  // Depth attachment
  VkAttachmentDescription depth_attachment = {
    .format = context->device.depth_format,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
  };
  VkAttachmentReference depth_attachment_ref = {
    .attachment = 1,
    .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
  };

  // Subpass
  VkSubpassDescription subpass = {
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .colorAttachmentCount = 1,
    .pColorAttachments = &color_attachment_ref,
    .pDepthStencilAttachment = &depth_attachment_ref,
    .inputAttachmentCount = 0,
    .pInputAttachments = 0,
    .pResolveAttachments = 0,
    .preserveAttachmentCount = 0,
    .pPreserveAttachments = 0
  };

  // Array of attachments
  u32 attachment_count = 2;
  VkAttachmentDescription attachments[] = {
    color_attachment,
    depth_attachment
  };

  // Renderpass dependency
  VkSubpassDependency dependency = {
    .srcSubpass = VK_SUBPASS_EXTERNAL,
    .dstSubpass = 0,
    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .srcAccessMask = 0,
    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    .dependencyFlags = 0
  };

  // Renderpass create info
  VkRenderPassCreateInfo renderpass_create_info = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = attachment_count,
    .pAttachments = attachments,
    .subpassCount = 1,
    .pSubpasses = &subpass,
    .dependencyCount = 1,
    .pDependencies = &dependency,
    .pNext = 0,
    .flags = 0
  };

  // Create renderpass
  VK_CHECK(vkCreateRenderPass(context->device.logical_device,
                              &renderpass_create_info,
                              context->allocator,
                              &out_renderpass->handle));
}

void vulkan_renderpass_destroy(vulkan_context *context,
                               vulkan_renderpass *renderpass) {
  if (renderpass && renderpass->handle) {
    vkDestroyRenderPass(context->device.logical_device,
                        renderpass->handle,
                        context->allocator);
    renderpass->handle = 0;
  }
}

void vulkan_renderpass_begin(vulkan_command_buffer *command_buffer,
                             vulkan_renderpass *renderpass,
                             VkFramebuffer framebuffer) {
  // Clear values array
  VkClearValue clear_values[] = {
    (VkClearValue) { .color.float32 = {
        renderpass->r,
        renderpass->g,
        renderpass->b,
        renderpass->a
      }},
    (VkClearValue) {
      .depthStencil.depth = renderpass->depth,
      .depthStencil.stencil = renderpass->stencil
    }
  };

  // Begin info
  VkRenderPassBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .renderPass = renderpass->handle,
    .framebuffer = framebuffer,
    .renderArea.offset.x = renderpass->x,
    .renderArea.offset.y = renderpass->y,
    .renderArea.extent.width = renderpass->w,
    .renderArea.extent.height = renderpass->h,
    .clearValueCount = 2,
    .pClearValues = clear_values
  };

  // Command
  vkCmdBeginRenderPass(command_buffer->handle,
                       &begin_info,
                       VK_SUBPASS_CONTENTS_INLINE);
  command_buffer->state = COMMAND_BUFFER_STATE_IN_RENDER_PASS;
}

void vulkan_renderpass_end(vulkan_command_buffer *command_buffer,
                           vulkan_renderpass *renderpass) {
  (void) renderpass;  // Unused parameter

  vkCmdEndRenderPass(command_buffer->handle);
  command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;
}
