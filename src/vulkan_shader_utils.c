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
#include <kstring.h>
#include <kmemory.h>
#include <resource_system.h>
#include <vulkan_shader_utils.h>

#define FILENAME_MAX_LEN 512

b8 create_shader_module(vulkan_context *context,
                        const char *name,
                        const char *type,
                        VkShaderStageFlagBits shader_stage_flag,
                        u32 stage_idx,
                        vulkan_shader_stage *shader_stages) {
  char filename[FILENAME_MAX_LEN];
  kstrfmt(filename, "shaders/%s.%s.spv", name, type);

  // Read binary resource
  resource binary_resource;
  if (!resource_system_load(filename,
                            RESOURCE_TYPE_BINARY,
                            &binary_resource)) {
    KERROR("create_shader_module :: failed to read shader module '%s'", filename);
    return false;
  }

  // Save buffer to 'create info' struct and close file
  kzero_memory(&shader_stages[stage_idx].create_info,
               sizeof(VkShaderModuleCreateInfo));
  shader_stages[stage_idx].create_info = (VkShaderModuleCreateInfo) {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = binary_resource.data_size,
    .pCode = (u32 *) binary_resource.data
  };

  VK_CHECK(vkCreateShaderModule(context->device.logical_device,
                                &shader_stages[stage_idx].create_info,
                                context->allocator,
                                &shader_stages[stage_idx].handle));

  resource_system_unload(&binary_resource);

  kzero_memory(&shader_stages[stage_idx].shader_stage_create_info,
               sizeof(VkPipelineShaderStageCreateInfo));
  shader_stages[stage_idx].shader_stage_create_info = (VkPipelineShaderStageCreateInfo) {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = shader_stage_flag,
    .module = shader_stages[stage_idx].handle,
    .pName = "main"
  };

  return true;
}
