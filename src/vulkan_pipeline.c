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
#include <vulkan_utils.h>
#include <vulkan_pipeline.h>

b8 vulkan_graphics_pipeline_create(vulkan_context *context,
                                   vulkan_renderpass *renderpass,
                                   u32 stride,
                                   u32 attribute_count,
                                   VkVertexInputAttributeDescription *attributes,
                                   u32 descriptor_set_layout_count,
                                   VkDescriptorSetLayout *descriptor_set_layouts,
                                   u32 stage_count,
                                   VkPipelineShaderStageCreateInfo *stages,
                                   VkViewport viewport,
                                   VkRect2D scissor,
                                   b8 is_wireframe,
                                   b8 depth_test_enabled,
                                   vulkan_pipeline *out_pipeline) {
  // Dynamic state
  const u32 dynamic_state_count = 3;
  VkDynamicState dynamic_states[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
    VK_DYNAMIC_STATE_LINE_WIDTH
  };

  // Create info structs creation
  VkPipelineViewportStateCreateInfo viewport_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .pViewports = &viewport,
    .scissorCount = 1,
    .pScissors = &scissor
  };
  VkPipelineRasterizationStateCreateInfo rasterizer_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = is_wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL,
    .lineWidth = 1.0f,
    .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = VK_FALSE,
    .depthBiasConstantFactor = 0.0f,
    .depthBiasClamp = 0.0f,
    .depthBiasSlopeFactor = 0.0f
  };
  VkPipelineMultisampleStateCreateInfo multisampling_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .sampleShadingEnable = VK_FALSE,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    .minSampleShading = 1.0f,
    .pSampleMask = 0,
    .alphaToCoverageEnable = VK_FALSE,
    .alphaToOneEnable = VK_FALSE
  };
  VkPipelineDepthStencilStateCreateInfo depth_stencil = {0};
  if (depth_test_enabled) {
    depth_stencil = (VkPipelineDepthStencilStateCreateInfo) {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_TRUE,
      .depthWriteEnable = VK_TRUE,
      .depthCompareOp = VK_COMPARE_OP_LESS,
      .depthBoundsTestEnable = VK_FALSE,
      .stencilTestEnable = VK_FALSE
    };
  }
  VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
    .blendEnable = VK_TRUE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .colorBlendOp = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .alphaBlendOp = VK_BLEND_OP_ADD,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
  };
  VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_COPY,
    .attachmentCount = 1,
    .pAttachments = &color_blend_attachment_state
  };
  VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = dynamic_state_count,
    .pDynamicStates = dynamic_states
  };
  VkVertexInputBindingDescription binding_description = {
    .binding = 0,
    .stride = stride,
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
  };
  VkPipelineVertexInputStateCreateInfo vertex_input_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &binding_description,
    .vertexAttributeDescriptionCount = attribute_count,
    .pVertexAttributeDescriptions = attributes
  };
  VkPipelineInputAssemblyStateCreateInfo input_assembly = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE
  };
  VkPushConstantRange push_constant = {
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    .offset = sizeof(Matrix4) * 0,
    .size = sizeof(Matrix4) * 2
  };
  VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = descriptor_set_layout_count,
    .pSetLayouts = descriptor_set_layouts,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges = &push_constant
  };

  // Create pipeline layout
  VK_CHECK(vkCreatePipelineLayout(context->device.logical_device,
                                  &pipeline_layout_create_info,
                                  context->allocator,
                                  &out_pipeline->pipeline_layout));

  VkGraphicsPipelineCreateInfo pipeline_create_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .stageCount = stage_count,
    .pStages = stages,
    .pVertexInputState = &vertex_input_info,
    .pInputAssemblyState = &input_assembly,
    .pViewportState = &viewport_state,
    .pRasterizationState = &rasterizer_create_info,
    .pMultisampleState = &multisampling_create_info,
    .pDepthStencilState = depth_test_enabled ? &depth_stencil : 0,
    .pColorBlendState = &color_blend_state_create_info,
    .pDynamicState = &dynamic_state_create_info,
    .pTessellationState = 0,
    .layout = out_pipeline->pipeline_layout,
    .renderPass = renderpass->handle,
    .subpass = 0,
    .basePipelineHandle = VK_NULL_HANDLE,
    .basePipelineIndex = -1
  };

  // Create graphics pipeline
  VkResult result = vkCreateGraphicsPipelines(context->device.logical_device,
                                              VK_NULL_HANDLE,
                                              1,
                                              &pipeline_create_info,
                                              context->allocator,
                                              &out_pipeline->handle);
  if (vulkan_result_is_success(result)) {
    KDEBUG("Graphics pipeline created");
    return true;
  }
  KERROR("vulkan_graphics_pipeline_create :: %s", vulkan_result_string(result, true));
  return false;
}

void vulkan_pipeline_destroy(vulkan_context *context, vulkan_pipeline *pipeline) {
  if (!pipeline) return;
  if (pipeline->handle) {
    vkDestroyPipeline(context->device.logical_device,
                      pipeline->handle,
                      context->allocator);
    pipeline->handle = 0;
  }
  if (pipeline->pipeline_layout) {
    vkDestroyPipelineLayout(context->device.logical_device,
                            pipeline->pipeline_layout,
                            context->allocator);
    pipeline->pipeline_layout = 0;
  }
}

void vulkan_pipeline_bind(vulkan_command_buffer *command_buffer,
                          VkPipelineBindPoint bind_point,
                          vulkan_pipeline *pipeline) {
  vkCmdBindPipeline(command_buffer->handle, bind_point, pipeline->handle);
}
