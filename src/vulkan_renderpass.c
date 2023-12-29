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
                              Vector4 render_area,
                              Vector4 clear_color,
                              f32 depth,
                              u32 stencil,
                              u8 clear_flags,
                              b8 prev_pass,
                              b8 next_pass) {
  // Copy dimensions
  out_renderpass->render_area = render_area;
  // Copy RGBA info
  out_renderpass->clear_color = clear_color;
  // Copy depth & stencil info
  out_renderpass->depth = depth;
  out_renderpass->stencil = stencil;
  // Copy flags
  out_renderpass->clear_flags = clear_flags;
  // Copy boolean indicators for prev/next passes
  out_renderpass->prev_pass = prev_pass;
  out_renderpass->next_pass = next_pass;

  // Color attachment
  b8 do_clear_color = (out_renderpass->clear_flags &
                       RENDERPASS_CLEAR_COLOR_BUFFER_FLAG) != 0;
  VkAttachmentDescription color_attachment = {
    .format = context->swapchain.image_format.format,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = do_clear_color ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = prev_pass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = next_pass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    .flags = 0
  };
  VkAttachmentReference color_attachment_ref = {
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
  };

  // Subpass
  VkSubpassDescription subpass = {
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .colorAttachmentCount = 1,
    .pColorAttachments = &color_attachment_ref,
    .inputAttachmentCount = 0,
    .pInputAttachments = 0,
    .pResolveAttachments = 0,
    .preserveAttachmentCount = 0,
    .pPreserveAttachments = 0
  };

  // Depth attachment
  u32 attachment_count = 2;
  VkAttachmentDescription attachments[attachment_count];
  b8 do_clear_depth = (out_renderpass->clear_flags &
                       RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG) != 0;
  if (do_clear_depth) {
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
    // Set depth attachment ref to subpass
    subpass.pDepthStencilAttachment = &depth_attachment_ref;
    // Set array of attachments
    attachments[0] = color_attachment;
    attachments[1] = depth_attachment;
  }
  else {
    subpass.pDepthStencilAttachment = 0;
    --attachment_count;
    attachments[0] = color_attachment;
    kzero_memory(&attachments[attachment_count], sizeof(VkAttachmentDescription));
  }

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
  // Begin info
  VkRenderPassBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .renderPass = renderpass->handle,
    .framebuffer = framebuffer,
    .renderArea.offset.x = renderpass->render_area.x,
    .renderArea.offset.y = renderpass->render_area.y,
    .renderArea.extent.width = renderpass->render_area.z,
    .renderArea.extent.height = renderpass->render_area.w,
    .clearValueCount = 0,
    .pClearValues = 0
  };

  VkClearValue clear_values[2];
  kzero_memory(clear_values, sizeof(VkClearValue) * 2);
  // Check if clear color
  if ((renderpass->clear_flags &
       RENDERPASS_CLEAR_COLOR_BUFFER_FLAG) != 0) {
    kcopy_memory(clear_values[begin_info.clearValueCount].color.float32,
                 renderpass->clear_color.elements,
                 sizeof(f32) * 4);
    ++begin_info.clearValueCount;
  }
  // Check if clear depth
  if ((renderpass->clear_flags &
       RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG) != 0) {
    kcopy_memory(clear_values[begin_info.clearValueCount].color.float32,
                 renderpass->clear_color.elements,
                 sizeof(f32) * 4);
    clear_values[begin_info.clearValueCount].depthStencil.depth = renderpass->depth;
    b8 do_clear_stencil = (renderpass->clear_flags &
                           RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG) != 0;
    clear_values[begin_info.clearValueCount].depthStencil.stencil = do_clear_stencil ? renderpass->stencil : 0;
    ++begin_info.clearValueCount;
  }
  // Set clear values if the count has incremented from 0 within the previous checks
  begin_info.pClearValues = begin_info.clearValueCount > 0 ? clear_values : 0;

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
