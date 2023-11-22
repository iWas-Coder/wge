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
#include <math_types.h>
#include <vulkan_pipeline.h>
#include <vulkan_shader_utils.h>
#include <vulkan_object_shader.h>

#define BUILTIN_SHADER_NAME_OBJECT "builtin"

b8 vulkan_object_shader_create(vulkan_context *context,
                               vulkan_object_shader *out_shader) {
  char *stage_type_names[] = { "vert", "frag" };
  VkShaderStageFlagBits stage_types[] = {
    VK_SHADER_STAGE_VERTEX_BIT,
    VK_SHADER_STAGE_FRAGMENT_BIT
  };
  for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
    if (!create_shader_module(context,
                              BUILTIN_SHADER_NAME_OBJECT,
                              stage_type_names[i],
                              stage_types[i],
                              i,
                              out_shader->stages)) {
      KERROR("Unable to create %s shader module for `%s`",
             stage_type_names[i],
             BUILTIN_SHADER_NAME_OBJECT);
      return false;
    }
  }
  // TODO: Descriptors

  // Prepare pipeline creation
  VkViewport viewport = {
    .x = 0.0f,
    .y = (f32) context->framebuffer_height,
    .width = (f32) context->framebuffer_width,
    .height = - (f32) context->framebuffer_height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f
  };
  VkRect2D scissor = {
    .offset.x = 0,
    .offset.y = 0,
    .extent.width = context->framebuffer_width,
    .extent.height = context->framebuffer_height
  };
  u32 offset = 0;
  const i32 attribute_count = 1;
  VkVertexInputAttributeDescription attribute_descriptions[attribute_count];
  VkFormat formats[] = {
    VK_FORMAT_R32G32B32_SFLOAT
  };
  u64 sizes[] = {
    sizeof(Vector3)
  };
  for (u32 i = 0; i < attribute_count; ++i) {
    attribute_descriptions[i] = (VkVertexInputAttributeDescription) {
      .binding = 0,
      .location = i,
      .format = formats[i],
      .offset = offset
    };
    offset += sizes[i];
  }
  // TODO: Descriptors - set layouts
  VkPipelineShaderStageCreateInfo stage_create_infos[OBJECT_SHADER_STAGE_COUNT];
  kzero_memory(stage_create_infos, sizeof(stage_create_infos));
  for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
    stage_create_infos[i] = out_shader->stages[i].shader_stage_create_info;
  }

  // Create pipeline
  if (!vulkan_graphics_pipeline_create(context,
                                       &context->main_renderpass,
                                       attribute_count,
                                       attribute_descriptions,
                                       0,
                                       0,
                                       OBJECT_SHADER_STAGE_COUNT,
                                       stage_create_infos,
                                       viewport,
                                       scissor,
                                       false,
                                       &out_shader->pipeline)) {
    KERROR("vulkan_object_shader_create :: failed to create graphics pipeline");
    return false;
  }

  return true;
}

void vulkan_object_shader_destroy(vulkan_context *context,
                                  vulkan_object_shader *shader) {
  // Destroy pipeline
  vulkan_pipeline_destroy(context, &shader->pipeline);
  // Destroy shader modules
  for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
    vkDestroyShaderModule(context->device.logical_device,
                          shader->stages[i].handle,
                          context->allocator);
    shader->stages[i].handle = 0;
  }
}

void vulkan_object_shader_use(vulkan_context *context,
                              vulkan_object_shader *shader) {
  (void) context;  // Unused parameter
  (void) shader;   // Unused parameter
}
