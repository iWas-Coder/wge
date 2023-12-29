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


#pragma once

#include <vulkan_types.h>

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
                                   vulkan_pipeline *out_pipeline);

void vulkan_pipeline_destroy(vulkan_context *context, vulkan_pipeline *pipeline);

void vulkan_pipeline_bind(vulkan_command_buffer *command_buffer,
                          VkPipelineBindPoint bind_point,
                          vulkan_pipeline *pipeline);
