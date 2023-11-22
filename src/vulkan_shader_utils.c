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
#include <filesystem.h>
#include <vulkan_shader_utils.h>

#define FILENAME_MAX_LEN 512

b8 create_shader_module(vulkan_context *context,
                        const char *name,
                        const char *type,
                        VkShaderStageFlagBits shader_stage_flag,
                        u32 stage_idx,
                        vulkan_shader_stage *shader_stages) {
  char filename[FILENAME_MAX_LEN];
  kstrfmt(filename, "assets/shaders/%s.%s.spv", name, type);

  // Open file
  file_handle handle;
  if (!filesystem_open(filename, FILE_MODE_READ, true, &handle)) {
    KERROR("create_shader_module :: unable to open file '%s'", filename);
    return false;
  }
  // Read entire file
  u64 size = 0;
  u8 *file_buf = 0;
  if (!filesystem_read_all(&handle, &file_buf, &size)) {
    KERROR("create_shader_module :: unable to read file '%s'", filename);
    return false;
  }
  // Save buffer to 'create info' struct and close file
  kzero_memory(&shader_stages[stage_idx].create_info, sizeof(VkShaderModuleCreateInfo));
  shader_stages[stage_idx].create_info = (VkShaderModuleCreateInfo) {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = size,
    .pCode = (u32 *) file_buf
  };
  filesystem_close(&handle);

  VK_CHECK(vkCreateShaderModule(context->device.logical_device,
                                &shader_stages[stage_idx].create_info,
                                context->allocator,
                                &shader_stages[stage_idx].handle));

  kzero_memory(&shader_stages[stage_idx].shader_stage_create_info,
               sizeof(VkPipelineShaderStageCreateInfo));
  shader_stages[stage_idx].shader_stage_create_info = (VkPipelineShaderStageCreateInfo) {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = shader_stage_flag,
    .module = shader_stages[stage_idx].handle,
    .pName = "main"
  };

  if (file_buf) {
    kfree(file_buf, sizeof(u8) * size, MEMORY_TAG_STRING);
    file_buf = 0;
  }
  return true;
}
