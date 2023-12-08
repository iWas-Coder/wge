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


#include <kmath.h>
#include <logger.h>
#include <kmemory.h>
#include <vulkan_buffer.h>
#include <vulkan_pipeline.h>
#include <vulkan_shader_utils.h>
#include <vulkan_object_shader.h>

#define ATTRIBUTE_COUNT 2
#define BUILTIN_SHADER_NAME_OBJECT "builtin"

b8 vulkan_object_shader_create(vulkan_context *context, vulkan_object_shader *out_shader) {
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
  // Global Descriptors
  VkDescriptorSetLayoutBinding global_ubo_layout_binding = {
    .binding = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .pImmutableSamplers = 0,
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
  };
  VkDescriptorSetLayoutCreateInfo global_layout_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .bindingCount = 1,
    .pBindings = &global_ubo_layout_binding,
  };
  VK_CHECK(vkCreateDescriptorSetLayout(context->device.logical_device,
                                       &global_layout_info,
                                       context->allocator,
                                       &out_shader->global_descriptor_set_layout));
  // Global descriptor pool
  VkDescriptorPoolSize global_pool_size = {
    .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .descriptorCount = context->swapchain.image_count
  };
  VkDescriptorPoolCreateInfo global_pool_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .poolSizeCount = 1,
    .pPoolSizes = &global_pool_size,
    .maxSets = context->swapchain.image_count
  };
  VK_CHECK(vkCreateDescriptorPool(context->device.logical_device,
                                  &global_pool_info,
                                  context->allocator,
                                  &out_shader->global_descriptor_pool));

  // Local (object) descriptors
  const u32 local_sampler_count = 1;
  VkDescriptorType descriptor_types[] = {
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         // Binding 0: uniform buffer
    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER  // Binding 1: diffuse sampler layout
  };
  VkDescriptorSetLayoutBinding bindings[OBJECT_SHADER_OBJECT_DESCRIPTOR_NUMBER];
  kzero_memory(&bindings, sizeof(VkDescriptorSetLayoutBinding) * OBJECT_SHADER_OBJECT_DESCRIPTOR_NUMBER);
  for (u32 i = 0; i < OBJECT_SHADER_OBJECT_DESCRIPTOR_NUMBER; ++i) {
    bindings[i] = (VkDescriptorSetLayoutBinding) {
      .binding = i,
      .descriptorCount = 1,
      .descriptorType = descriptor_types[i],
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
  }
  VkDescriptorSetLayoutCreateInfo layout_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .bindingCount = OBJECT_SHADER_OBJECT_DESCRIPTOR_NUMBER,
    .pBindings = bindings
  };
  VK_CHECK(vkCreateDescriptorSetLayout(context->device.logical_device,
                                       &layout_info,
                                       0,
                                       &out_shader->object_descriptor_set_layout));

  // Local (object) descriptor pool
  VkDescriptorPoolSize object_pool_sizes[] = {
    {
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = OBJECT_SHADER_MAX_OBJECTS
    },
    {
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = local_sampler_count * OBJECT_SHADER_MAX_OBJECTS
    }
  };
  VkDescriptorPoolCreateInfo object_pool_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .poolSizeCount = OBJECT_SHADER_OBJECT_DESCRIPTOR_NUMBER,
    .pPoolSizes = object_pool_sizes,
    .maxSets = OBJECT_SHADER_MAX_OBJECTS
  };
  VK_CHECK(vkCreateDescriptorPool(context->device.logical_device,
                                  &object_pool_info,
                                  context->allocator,
                                  &out_shader->object_descriptor_pool));

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
  VkVertexInputAttributeDescription attribute_descriptions[ATTRIBUTE_COUNT];
  VkFormat formats[] = {
    VK_FORMAT_R32G32B32_SFLOAT,
    VK_FORMAT_R32G32_SFLOAT
  };
  u64 sizes[] = {
    sizeof(Vector3),
    sizeof(Vector2)
  };
  for (u32 i = 0; i < ATTRIBUTE_COUNT; ++i) {
    attribute_descriptions[i] = (VkVertexInputAttributeDescription) {
      .binding = 0,
      .location = i,
      .format = formats[i],
      .offset = offset
    };
    offset += sizes[i];
  }

  // Descriptor set layouts
  const i32 descriptor_set_layout_count = 2;
  VkDescriptorSetLayout layouts[] = {
    out_shader->global_descriptor_set_layout,
    out_shader->object_descriptor_set_layout
  };

  VkPipelineShaderStageCreateInfo stage_create_infos[OBJECT_SHADER_STAGE_COUNT];
  kzero_memory(stage_create_infos, sizeof(stage_create_infos));
  for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
    stage_create_infos[i] = out_shader->stages[i].shader_stage_create_info;
  }

  // Create pipeline
  if (!vulkan_graphics_pipeline_create(context,
                                       &context->main_renderpass,
                                       ATTRIBUTE_COUNT,
                                       attribute_descriptions,
                                       descriptor_set_layout_count,
                                       layouts,
                                       OBJECT_SHADER_STAGE_COUNT,
                                       stage_create_infos,
                                       viewport,
                                       scissor,
                                       false,
                                       &out_shader->pipeline)) {
    KERROR("vulkan_object_shader_create :: failed to create graphics pipeline");
    return false;
  }

  // Create uniform buffer
  if (!vulkan_buffer_create(context,
                            sizeof(global_uniform_object) * OBJECT_SHADER_DESCRIPTOR_COUNT,
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            true,
                            &out_shader->global_uniform_buffer)) {
    KERROR("vulkan_object_shader_create :: failed to create object shader's buffer");
    return false;
  }

  VkDescriptorSetLayout global_layouts[OBJECT_SHADER_DESCRIPTOR_COUNT];
  for (u32 i = 0; i < OBJECT_SHADER_DESCRIPTOR_COUNT; ++i) {
    global_layouts[i] = out_shader->global_descriptor_set_layout;
  }
  VkDescriptorSetAllocateInfo alloc_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = out_shader->global_descriptor_pool,
    .descriptorSetCount = OBJECT_SHADER_DESCRIPTOR_COUNT,
    .pSetLayouts = global_layouts
  };
  VK_CHECK(vkAllocateDescriptorSets(context->device.logical_device,
                                    &alloc_info,
                                    out_shader->global_descriptor_sets));

  // Create object uniform buffer
  if (!vulkan_buffer_create(context,
                            sizeof(local_uniform_object),
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            true,
                            &out_shader->object_uniform_buffer)) {
    KERROR("vulkan_object_shader_create :: Failed material instance buffer creation");
    return false;
  }

  return true;
}

void vulkan_object_shader_destroy(vulkan_context *context, vulkan_object_shader *shader) {
  vkDestroyDescriptorPool(context->device.logical_device,
                          shader->object_descriptor_pool,
                          context->allocator);
  vkDestroyDescriptorSetLayout(context->device.logical_device,
                               shader->object_descriptor_set_layout,
                               context->allocator);

  // Destroy uniform buffers
  vulkan_buffer_destroy(context, &shader->global_uniform_buffer);
  vulkan_buffer_destroy(context, &shader->object_uniform_buffer);

  // Destroy pipeline
  vulkan_pipeline_destroy(context, &shader->pipeline);

  // Destroy global descriptor pool
  vkDestroyDescriptorPool(context->device.logical_device,
                          shader->global_descriptor_pool,
                          context->allocator);

  // Destroy descriptor set layouts
  vkDestroyDescriptorSetLayout(context->device.logical_device,
                               shader->global_descriptor_set_layout,
                               context->allocator);

  // Destroy shader modules
  for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
    vkDestroyShaderModule(context->device.logical_device,
                          shader->stages[i].handle,
                          context->allocator);
    shader->stages[i].handle = 0;
  }
}

void vulkan_object_shader_use(vulkan_context *context, vulkan_object_shader *shader) {
  u32 img_idx = context->image_index;
  vulkan_pipeline_bind(&context->graphics_command_buffers[img_idx],
                       VK_PIPELINE_BIND_POINT_GRAPHICS,
                       &shader->pipeline);
}

void vulkan_object_shader_update(vulkan_context *context,
                                 vulkan_object_shader *shader,
                                 f32 delta_time) {
  (void) delta_time;  // Unused parameter

  u32 image_idx = context->image_index;
  VkCommandBuffer command_buffer = context->graphics_command_buffers[image_idx].handle;
  VkDescriptorSet global_descriptor = shader->global_descriptor_sets[image_idx];

  u32 range = sizeof(global_uniform_object);
  u64 offset = 0;
  // Copy data to buffer
  vulkan_buffer_load(context,
                     &shader->global_uniform_buffer,
                     offset,
                     range,
                     0,
                     &shader->global_ubo);
  VkDescriptorBufferInfo buffer_info = {
    .buffer = shader->global_uniform_buffer.handle,
    .offset = offset,
    .range = range
  };
  // Update descriptor sets
  VkWriteDescriptorSet descriptor_write = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = global_descriptor,
    .dstBinding = 0,
    .dstArrayElement = 0,
    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .descriptorCount = 1,
    .pBufferInfo = &buffer_info
  };
  vkUpdateDescriptorSets(context->device.logical_device,
                         1,
                         &descriptor_write,
                         0,
                         0);

  // Bind the descriptor set (global) to be updated
  vkCmdBindDescriptorSets(command_buffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          shader->pipeline.pipeline_layout,
                          0,
                          1,
                          &global_descriptor,
                          0,
                          0);
}

void vulkan_object_shader_update_object(vulkan_context *context,
                                        vulkan_object_shader *shader,
                                        geometry_render_data data) {
  u32 image_idx = context->image_index;
  VkCommandBuffer command_buffer = context->graphics_command_buffers[image_idx].handle;

  vkCmdPushConstants(command_buffer,
                     shader->pipeline.pipeline_layout,
                     VK_SHADER_STAGE_VERTEX_BIT,
                     0,
                     sizeof(Matrix4),
                     &data.model);

  // Get material data
  vulkan_object_shader_object_state *object_state = &shader->object_states[data.object_id];
  VkDescriptorSet object_descriptor_set = object_state->descriptor_sets[image_idx];

  VkWriteDescriptorSet descriptor_writes[OBJECT_SHADER_OBJECT_DESCRIPTOR_NUMBER];
  kzero_memory(descriptor_writes, sizeof(VkWriteDescriptorSet) * OBJECT_SHADER_OBJECT_DESCRIPTOR_NUMBER);

  u32 descriptor_count = 0, descriptor_idx = 0;

  // Descriptor 0: uniform buffer
  u32 range = sizeof(local_uniform_object);
  u64 offset = sizeof(local_uniform_object) * data.object_id;
  local_uniform_object obo;

  static f32 accumulator = 0.0f;
  accumulator += context->frame_delta_time;
  f32 s = (ksin(accumulator) + 1.0f) / 2.0f;
  obo.diffuse_color = vec4_create(s, s, s, 1.0f);

  vulkan_buffer_load(context,
                     &shader->object_uniform_buffer,
                     offset,
                     range,
                     0,
                     &obo);

  // If the descriptor has not been updated yet
  if (object_state->descriptor_states[descriptor_idx].generations[image_idx] == INVALID_ID) {
    VkDescriptorBufferInfo buffer_info = {
      .buffer = shader->object_uniform_buffer.handle,
      .offset = offset,
      .range = range
    };
    descriptor_writes[descriptor_count] = (VkWriteDescriptorSet) {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = object_descriptor_set,
      .dstBinding = descriptor_idx,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .pBufferInfo = &buffer_info
    };
    ++descriptor_count;
    object_state->descriptor_states[descriptor_idx].generations[image_idx] = 1;
  }
  ++descriptor_idx;

  const u32 sampler_count = 1;
  VkDescriptorImageInfo image_infos[sampler_count];
  for (u32 i = 0; i < sampler_count; ++i) {
    texture *t = data.textures[i];
    u32 *descriptor_gen = &object_state->descriptor_states[descriptor_idx].generations[image_idx];
    // If descriptor needs to be updated
    if (t && (*descriptor_gen != t->generation || *descriptor_gen == INVALID_ID)) {
      vulkan_texture_data *internal_data = (vulkan_texture_data *) t->data;
      // Assign view & sampler
      image_infos[i] = (VkDescriptorImageInfo) {
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .imageView = internal_data->image.view,
        .sampler = internal_data->sampler
      };
      descriptor_writes[descriptor_count] = (VkWriteDescriptorSet) {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = object_descriptor_set,
        .dstBinding = descriptor_idx,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .pImageInfo = &image_infos[i]
      };
      ++descriptor_count;
      // Sync frame generation if not using default/fallback texture
      if (t->generation != INVALID_ID) *descriptor_gen = t->generation;
      ++descriptor_idx;
    }
  }
  if (descriptor_count > 0) vkUpdateDescriptorSets(context->device.logical_device,
                                                   descriptor_count,
                                                   descriptor_writes,
                                                   0,
                                                   0);
  vkCmdBindDescriptorSets(command_buffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          shader->pipeline.pipeline_layout,
                          1,
                          1,
                          &object_descriptor_set,
                          0,
                          0);
}

b8 vulkan_object_shader_get_resources(vulkan_context *context,
                                      vulkan_object_shader *shader,
                                      u32 *out_object_id) {
  *out_object_id = shader->object_uniform_buffer_index;
  ++shader->object_uniform_buffer_index;

  vulkan_object_shader_object_state *object_state = &shader->object_states[*out_object_id];
  for (u32 i = 0; i < OBJECT_SHADER_OBJECT_DESCRIPTOR_NUMBER; ++i) {
    for (u32 j = 0; j < OBJECT_SHADER_DESCRIPTOR_COUNT; ++j) {
      object_state->descriptor_states[i].generations[j] = INVALID_ID;
    }
  }

  // Allocate descriptor sets
  VkDescriptorSetLayout layouts[OBJECT_SHADER_DESCRIPTOR_COUNT];
  for (u32 i = 0; i < OBJECT_SHADER_DESCRIPTOR_COUNT; ++i) {
    layouts[i] = shader->object_descriptor_set_layout;
  }
  VkDescriptorSetAllocateInfo alloc_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = shader->object_descriptor_pool,
    .descriptorSetCount = OBJECT_SHADER_DESCRIPTOR_COUNT,
    .pSetLayouts = layouts
  };
  VkResult result = vkAllocateDescriptorSets(context->device.logical_device,
                                             &alloc_info,
                                             object_state->descriptor_sets);
  if (result != VK_SUCCESS) {
    KERROR("vulkan_object_shader_get_resources :: Failed descriptor sets allocation");
    return false;
  }
  return true;
}

void vulkan_object_shader_release_resources(vulkan_context *context,
                                            vulkan_object_shader *shader,
                                            u32 object_id) {
  vulkan_object_shader_object_state *object_state = &shader->object_states[object_id];

  VkResult result = vkFreeDescriptorSets(context->device.logical_device,
                                         shader->object_descriptor_pool,
                                         OBJECT_SHADER_DESCRIPTOR_COUNT,
                                         object_state->descriptor_sets);
  if (result != VK_SUCCESS) {
    KERROR("vulkan_object_shader_release_resources :: Failed descriptor sets free");
  }

  for (u32 i = 0; i < OBJECT_SHADER_OBJECT_DESCRIPTOR_NUMBER; ++i) {
    for (u32 j = 0; j < OBJECT_SHADER_DESCRIPTOR_COUNT; ++j) {
      object_state->descriptor_states[i].generations[j] = INVALID_ID;
    }
  }
}
