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

#include <defines.h>
#include <asserts.h>
#include <vulkan/vulkan.h>
#include <renderer_types.h>

#define FRAME_DESCRIPTOR_COUNT 8  // one per frame
#define UI_SHADER_STAGE_COUNT 2  // vertex and fragment shaders
#define UI_SHADER_OBJECT_DESCRIPTOR_COUNT 2
#define UI_SHADER_OBJECT_SAMPLER_COUNT 1
#define UI_SHADER_MAX_COUNT 1024
#define MATERIAL_SHADER_STAGE_COUNT 2  // vertex and fragment shaders
#define MATERIAL_SHADER_OBJECT_DESCRIPTOR_COUNT 2
#define MATERIAL_SHADER_OBJECT_SAMPLER_COUNT 1
#define MATERIAL_SHADER_MAX_MATERIALS 1024
#define GEOMETRY_MAX_COUNT 4096
#define VK_CHECK(e) KASSERT(e == VK_SUCCESS)

typedef struct {
  u64 total_size;
  VkBuffer handle;
  VkBufferUsageFlagBits usage;
  b8 is_locked;
  VkDeviceMemory memory;
  i32 memory_index;
  u32 memory_property_flags;
} vulkan_buffer;

typedef struct {
  VkSurfaceCapabilitiesKHR capabilities;
  u32 format_count;
  VkSurfaceFormatKHR *formats;
  u32 present_mode_count;
  VkPresentModeKHR *present_modes;
} vulkan_swapchain_support_info;

typedef struct {
  VkPhysicalDevice physical_device;
  VkDevice logical_device;
  vulkan_swapchain_support_info swapchain_support;
  i32 graphics;
  i32 present;
  i32 compute;
  i32 transfer;
  VkQueue graphics_queue;
  VkQueue present_queue;
  VkQueue compute_queue;
  VkQueue transfer_queue;
  VkCommandPool graphics_command_pool;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memory;
  VkFormat depth_format;
  b8 supports_device_local_host_visible;
} vulkan_device;

typedef struct {
  VkImage handle;
  VkDeviceMemory memory;
  VkImageView view;
  u32 width;
  u32 height;
} vulkan_image;

typedef enum {
  READY,
  RECORDING,
  IN_RENDER_PASS,
  RECORDING_ENDED,
  SUBMITTED,
  NOT_ALLOCATED
} vulkan_renderpass_state;

typedef struct {
  VkRenderPass handle;
  Vector4 render_area;
  Vector4 clear_color;
  f32 depth;
  u32 stencil;
  u8 clear_flags;
  b8 prev_pass;
  b8 next_pass;
  vulkan_renderpass_state state;
} vulkan_renderpass;

typedef struct {
  VkSwapchainKHR handle;
  VkSurfaceFormatKHR image_format;
  u8 max_frames_in_flight;
  u32 image_count;
  VkImage *images;
  VkImageView *views;
  vulkan_image depth_attachment;
  VkFramebuffer framebuffers[FRAME_DESCRIPTOR_COUNT];
} vulkan_swapchain;

typedef enum {
  COMMAND_BUFFER_STATE_READY,
  COMMAND_BUFFER_STATE_RECORDING,
  COMMAND_BUFFER_STATE_IN_RENDER_PASS,
  COMMAND_BUFFER_STATE_RECORDING_ENDED,
  COMMAND_BUFFER_STATE_SUBMITTED,
  COMMAND_BUFFER_STATE_NOT_ALLOCATED
} vulkan_command_buffer_state;

typedef struct {
  VkCommandBuffer handle;
  vulkan_command_buffer_state state;
} vulkan_command_buffer;

typedef struct {
  VkShaderModuleCreateInfo create_info;
  VkShaderModule handle;
  VkPipelineShaderStageCreateInfo shader_stage_create_info;
} vulkan_shader_stage;

typedef struct {
  VkPipeline handle;
  VkPipelineLayout pipeline_layout;
} vulkan_pipeline;

typedef struct {
  u32 generations[FRAME_DESCRIPTOR_COUNT];
  u32 ids[FRAME_DESCRIPTOR_COUNT];
} vulkan_descriptor_state;

typedef struct {
  VkDescriptorSet descriptor_sets[FRAME_DESCRIPTOR_COUNT];
  vulkan_descriptor_state descriptor_states[MATERIAL_SHADER_OBJECT_DESCRIPTOR_COUNT];
} vulkan_material_shader_instance_state;

typedef struct {
  u32 id;
  u32 generation;
  u32 vertex_count;
  u32 vertex_element_size;
  u32 vertex_buffer_offset;
  u32 index_count;
  u32 index_element_size;
  u32 index_buffer_offset;
} vulkan_geometry_data;

typedef struct {
  Matrix4 proj;
  Matrix4 view;
  Matrix4 padding0;
  Matrix4 padding1;
} vulkan_material_shader_global_ubo;

typedef struct {
  Vector4 diffuse_color;
  // 48 (16*3) bytes first section padding
  Vector4 padding0;
  Vector4 padding1;
  Vector4 padding2;
  // 192 (64*3) bytes second section padding
  Matrix4 padding3;
  Matrix4 padding4;
  Matrix4 padding5;
} vulkan_material_shader_instance_ubo;

typedef struct {
  vulkan_shader_stage stages[MATERIAL_SHADER_STAGE_COUNT];
  vulkan_pipeline pipeline;
  VkDescriptorPool global_descriptor_pool;
  VkDescriptorPool object_descriptor_pool;
  VkDescriptorSet global_descriptor_sets[FRAME_DESCRIPTOR_COUNT];
  VkDescriptorSetLayout global_descriptor_set_layout;
  VkDescriptorSetLayout object_descriptor_set_layout;
  vulkan_material_shader_global_ubo global_ubo;
  vulkan_buffer global_uniform_buffer;
  vulkan_buffer object_uniform_buffer;
  // TODO: manage this with a free list
  u32 object_uniform_buffer_index;
  texture_use sampler_uses[MATERIAL_SHADER_OBJECT_SAMPLER_COUNT];
  // TODO: make dynamic
  vulkan_material_shader_instance_state instance_states[MATERIAL_SHADER_MAX_MATERIALS];
} vulkan_material_shader;

typedef struct {
  VkDescriptorSet descriptor_sets[FRAME_DESCRIPTOR_COUNT];
  vulkan_descriptor_state descriptor_states[UI_SHADER_OBJECT_DESCRIPTOR_COUNT];
} vulkan_ui_shader_instance_state;

typedef struct {
  Matrix4 proj;
  Matrix4 view;
  Matrix4 padding0;
  Matrix4 padding1;
} vulkan_ui_shader_global_ubo;

typedef struct {
  Vector4 diffuse_color;
  // 48 (16*3) bytes first section padding
  Vector4 padding0;
  Vector4 padding1;
  Vector4 padding2;
  // 192 (64*3) bytes second section padding
  Matrix4 padding3;
  Matrix4 padding4;
  Matrix4 padding5;
} vulkan_ui_shader_instance_ubo;

typedef struct {
  vulkan_shader_stage stages[UI_SHADER_STAGE_COUNT];
  VkDescriptorPool global_descriptor_pool;
  VkDescriptorSetLayout global_descriptor_set_layout;
  VkDescriptorSet global_descriptor_sets[FRAME_DESCRIPTOR_COUNT];
  vulkan_ui_shader_global_ubo global_ubo;
  vulkan_buffer global_uniform_buffer;
  VkDescriptorPool object_descriptor_pool;
  VkDescriptorSetLayout object_descriptor_set_layout;
  vulkan_buffer object_uniform_buffer;
  u32 object_uniform_buffer_index;
  texture_use sampler_uses[UI_SHADER_OBJECT_SAMPLER_COUNT];
  vulkan_ui_shader_instance_state instance_states[UI_SHADER_MAX_COUNT];
  vulkan_pipeline pipeline;
} vulkan_ui_shader;

typedef struct {
  VkInstance instance;
  VkAllocationCallbacks *allocator;
  VkSurfaceKHR surface;
#if defined(_DEBUG)
  VkDebugUtilsMessengerEXT debug_messenger;
#endif
  vulkan_device device;
  u32 framebuffer_width;
  u32 framebuffer_height;
  u64 framebuffer_size_generation;
  u64 framebuffer_size_last_generation;
  vulkan_swapchain swapchain;
  vulkan_renderpass main_renderpass;
  vulkan_renderpass ui_renderpass;
  vulkan_buffer object_vertex_buffer;
  vulkan_buffer object_index_buffer;
  vulkan_command_buffer *graphics_command_buffers;
  VkSemaphore *image_available_semaphores;
  VkSemaphore *queue_complete_semaphores;
  u32 in_flight_fence_count;
  VkFence in_flight_fences[FRAME_DESCRIPTOR_COUNT];
  VkFence *images_in_flight[FRAME_DESCRIPTOR_COUNT];
  u32 image_index;
  u32 current_frame;
  b8 recreating_swapchain;
  vulkan_material_shader material_shader;
  vulkan_ui_shader ui_shader;
  u64 geometry_vertex_offset;
  u64 geometry_index_offset;
  f32 frame_delta_time;
  vulkan_geometry_data geometries[GEOMETRY_MAX_COUNT];
  VkFramebuffer world_framebuffers[FRAME_DESCRIPTOR_COUNT];
  i32 (*find_memory_index)(u32 type_filter, u32 property_flags);
} vulkan_context;

typedef struct {
  vulkan_image image;
  VkSampler sampler;
} vulkan_texture_data;
