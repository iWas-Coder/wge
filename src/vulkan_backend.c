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
#include <darray.h>
#include <kstring.h>
#include <kmemory.h>
#include <math_types.h>
#include <application.h>
#include <vulkan_types.h>
#include <vulkan_utils.h>
#include <vulkan_fence.h>
#include <vulkan_image.h>
#include <vulkan_device.h>
#include <vulkan_buffer.h>
#include <vulkan_backend.h>
#include <vulkan_platform.h>
#include <material_system.h>
#include <vulkan_swapchain.h>
#include <vulkan_renderpass.h>
#include <vulkan_framebuffer.h>
#include <vulkan_material_shader.h>
#include <vulkan_command_buffer.h>

static vulkan_context context;
static u32 cached_framebuffer_width = 0;
static u32 cached_framebuffer_height = 0;

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_types, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
  (void) message_types;  // Unused parameter
  (void) user_data;      // Unused parameter

  switch (message_severity) {
  default:
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    KERROR(callback_data->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    KWARN(callback_data->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    KINFO(callback_data->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    KTRACE(callback_data->pMessage);
    break;
  }
  return VK_FALSE;
}

i32 find_memory_index(u32 type_filter, u32 property_flags) {
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(context.device.physical_device, &memory_properties);
  for (u32 i = 0; i < memory_properties.memoryTypeCount; ++i) {
    if (type_filter & (1 << i) &&
        (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) return i;
  }

  KWARN("Suitable memory type not found");
  return -1;
}

b8 create_buffers(vulkan_context *context) {
  VkMemoryPropertyFlagBits memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  const u64 vertex_buf_size = sizeof(vertex_3d) * 1024 * 1024;
  if (!vulkan_buffer_create(context,
                            vertex_buf_size,
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            memory_property_flags,
                            true,
                            &context->object_vertex_buffer)) {
    KERROR("create_buffers :: vertex buffer creation failed");
    return false;
  }
  context->geometry_vertex_offset = 0;

  const u64 index_buf_size = sizeof(u32) * 1024 * 1024;
  if (!vulkan_buffer_create(context,
                            index_buf_size,
                            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            memory_property_flags,
                            true,
                            &context->object_index_buffer)) {
    KERROR("create_buffers :: index buffer creation failed");
    return false;
  }
  context->geometry_index_offset = 0;

  return true;
}

void create_command_buffers(renderer_backend *backend) {
  (void) backend;  // Unused parameter

  if (!context.graphics_command_buffers) {
    context.graphics_command_buffers = darray_reserve(vulkan_command_buffer,
                                                      context.swapchain.image_count);
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
      kzero_memory(&context.graphics_command_buffers[i], sizeof(vulkan_command_buffer));
    }
  }

  for (u32 i = 0; i < context.swapchain.image_count; ++i) {
    if (context.graphics_command_buffers[i].handle) {
      vulkan_command_buffer_free(&context,
                                 context.device.graphics_command_pool,
                                 &context.graphics_command_buffers[i]);
    }
    kzero_memory(&context.graphics_command_buffers[i], sizeof(vulkan_command_buffer));
    vulkan_command_buffer_allocate(&context,
                                   context.device.graphics_command_pool,
                                   true,
                                   &context.graphics_command_buffers[i]);
  }
  KDEBUG("Vulkan command buffers created");
}

void regenerate_framebuffers(renderer_backend *backend,
                             vulkan_swapchain *swapchain,
                             vulkan_renderpass *renderpass) {
  (void) backend;  // Unused parameter

  u32 attachment_count = 2;
  for (u32 i = 0; i < swapchain->image_count; ++i) {
    VkImageView attachments[] = {
      swapchain->views[i],
      swapchain->depth_attachment.view
    };
    vulkan_framebuffer_create(&context,
                              renderpass,
                              context.framebuffer_width,
                              context.framebuffer_height,
                              attachment_count,
                              attachments,
                              &context.swapchain.framebuffers[i]);
  }
}

b8 recreate_swapchain(renderer_backend *backend) {
  if (context.recreating_swapchain) {
    KDEBUG("recreate_swapchain :: Already recreating");
    return false;
  }
  if (context.framebuffer_width == 0 || context.framebuffer_height == 0) {
    KDEBUG("recreate_swapchain :: Window is too small to be drawn to");
    return false;
  }
  context.recreating_swapchain = true;

  vkDeviceWaitIdle(context.device.logical_device);
  for (u32 i = 0; i < context.swapchain.image_count; ++i) context.images_in_flight[i] = 0;
  vulkan_device_query_swapchain_support(context.device.physical_device,
                                        context.surface,
                                        &context.device.swapchain_support);
  vulkan_device_detect_depth_format(&context.device);

  // Recreate swapchain
  vulkan_swapchain_recreate(&context,
                            cached_framebuffer_width,
                            cached_framebuffer_height,
                            &context.swapchain);
  // Sync framebuffer size with the cached sizes
  context.framebuffer_width = cached_framebuffer_width;
  context.framebuffer_height = cached_framebuffer_height;
  cached_framebuffer_width = 0;
  cached_framebuffer_height = 0;
  context.main_renderpass.w = context.framebuffer_width;
  context.main_renderpass.h = context.framebuffer_height;
  // Sync framebuffer size generation
  context.framebuffer_size_last_generation = context.framebuffer_size_generation;

  // Cleanup framebuffers and command buffers
  for (u32 i = 0; i < context.swapchain.image_count; ++i) {
    vulkan_command_buffer_free(&context,
                               context.device.graphics_command_pool,
                               &context.graphics_command_buffers[i]);
  }
  for (u32 i = 0; i < context.swapchain.image_count; ++i) {
    vulkan_framebuffer_destroy(&context, &context.swapchain.framebuffers[i]);
  }

  context.main_renderpass.x = 0;
  context.main_renderpass.y = 0;
  context.main_renderpass.w = context.framebuffer_width;
  context.main_renderpass.h = context.framebuffer_height;

  // Recreate framebuffers and command buffers
  regenerate_framebuffers(backend, &context.swapchain, &context.main_renderpass);
  create_command_buffers(backend);

  context.recreating_swapchain = false;
  return true;
}

void upload_data_range(vulkan_context *context,
                       VkCommandPool pool,
                       VkFence fence,
                       VkQueue queue,
                       vulkan_buffer *buffer,
                       u64 offset,
                       u64 size,
                       const void *data) {
  VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  vulkan_buffer tmp_buf;
  // Create temporal buffer
  vulkan_buffer_create(context,
                       size,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       flags,
                       true,
                       &tmp_buf);
  // Load data into temporal buffer
  vulkan_buffer_load(context, &tmp_buf, 0, size, 0, data);
  // Copy data from temporal buffer to device local buffer
  vulkan_buffer_copy(context,
                     pool,
                     fence,
                     queue,
                     tmp_buf.handle,
                     0,
                     buffer->handle,
                     offset,
                     size);
  // Destroy temporal buffer
  vulkan_buffer_destroy(context, &tmp_buf);
}

void free_data_range(vulkan_buffer *buffer, u64 offset, u64 size) {
  (void) buffer;  // Unused parameter
  (void) offset;  // Unused parameter
  (void) size;    // Unused parameter
  
  // TODO: implement this with a free list
}

b8 vulkan_renderer_backend_initialize(renderer_backend *backend, const char *application_name) {
  // Set the `find_memory_index` function pointer to its definition
  context.find_memory_index = find_memory_index;

  // TODO: custom Vulkan allocator
  context.allocator = 0;

  // Set framebuffer dimensions
  application_set_framebuffer_size(&cached_framebuffer_width, &cached_framebuffer_height);
  context.framebuffer_width = cached_framebuffer_width ? cached_framebuffer_width : 800;
  context.framebuffer_height = cached_framebuffer_height ? cached_framebuffer_height : 600;
  cached_framebuffer_width = 0;
  cached_framebuffer_height = 0;

  VkApplicationInfo app_info = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .apiVersion = VK_API_VERSION_1_2,
    .pApplicationName = application_name,
    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    .pEngineName = "Wildebeest Game Engine™",
    .engineVersion = VK_MAKE_VERSION(1, 0, 0)
  };

  // Extensions
  const char **vk_extensions = darray_create(const char *);
  darray_push(vk_extensions, &VK_KHR_SURFACE_EXTENSION_NAME);
  platform_get_required_extension_names(&vk_extensions);
#if defined(_DEBUG)
  darray_push(vk_extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  KDEBUG("Extension list:");
  u32 length = darray_length(vk_extensions);
  for (u32 i = 0; i < length; ++i) KDEBUG(vk_extensions[i]);
#endif

  // Validation layers
  const char **vk_layers = 0;
  u32 vk_layers_count = 0;
#if defined(_DEBUG)
  KDEBUG("Validation layers list:");
  vk_layers = darray_create(const char *);
  darray_push(vk_layers, &"VK_LAYER_KHRONOS_validation");
  vk_layers_count = darray_length(vk_layers);

  // Get list of available ones
  u32 available_layer_count = 0;
  VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, 0));
  VkLayerProperties *available_layers = darray_reserve(VkLayerProperties, available_layer_count);
  VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers));

  // Verify if all required ones are available
  for (u32 i = 0; i < vk_layers_count; ++i) {
    KDEBUG("%s", vk_layers[i]);
    b8 found = false;
    for (u32 j = 0; j < available_layer_count; ++j) {
      if (kstrcmp(vk_layers[i], available_layers[j].layerName)) {
        found = true;
        break;
      }
    }
    if (!found) {
      KFATAL("Missing layer: %s", vk_layers[i]);
      return false;
    }
  }
  KDEBUG("Validation layers list verified");
#endif

  VkInstanceCreateInfo create_info = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &app_info,
    .enabledExtensionCount = darray_length(vk_extensions),
    .ppEnabledExtensionNames = vk_extensions,
    .enabledLayerCount = vk_layers_count,
    .ppEnabledLayerNames = vk_layers
  };

  VK_CHECK(vkCreateInstance(&create_info, context.allocator, &context.instance));
  KINFO("Vulkan instance created");

  // Vulkan debugger
#if defined(_DEBUG)
  u32 log_severity = (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT);
  VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .messageSeverity = log_severity,
    .messageType = (VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT),
    .pfnUserCallback = vk_debug_callback,
    .pUserData = 0
  };
  PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");

  KASSERT_MSG(func, "Failed to create the debug messenger");
  VK_CHECK(func(context.instance,
                &debug_create_info,
                context.allocator,
                &context.debug_messenger));
  KDEBUG("Vulkan debugger created");
#endif

  // Surface creation
  if (!platform_create_vulkan_surface((struct vulkan_context *) &context)) {
    KERROR("Failed to create surface");
    return false;
  }
  KDEBUG("Vulkan surface created");

  // Create device
  if (!vulkan_device_create(&context)) {
    KERROR("Failed to create device");
    return false;
  }

  // Create swapchain
  vulkan_swapchain_create(&context,
                          context.framebuffer_width,
                          context.framebuffer_height,
                          &context.swapchain);

  // Create renderpass
  vulkan_renderpass_create(&context,
                           &context.main_renderpass,
                           0, 0,
                           context.framebuffer_width,
                           context.framebuffer_height,
                           0.0f, 0.0f, 0.2f, 1.0f,
                           1.0f, 0);

  // Create swapchain framebuffers
  context.swapchain.framebuffers = darray_reserve(vulkan_framebuffer,
                                                  context.swapchain.image_count);
  regenerate_framebuffers(backend, &context.swapchain, &context.main_renderpass);

  // Create command buffers
  create_command_buffers(backend);

  // Create semaphores & fences
  context.image_available_semaphores = darray_reserve(VkSemaphore,
                                                      context.swapchain.max_frames_in_flight);
  context.queue_complete_semaphores = darray_reserve(VkSemaphore,
                                                     context.swapchain.max_frames_in_flight);
  context.in_flight_fences = darray_reserve(vulkan_fence,
                                            context.swapchain.max_frames_in_flight);
  for (u8 i = 0; i < context.swapchain.max_frames_in_flight; ++i) {
    VkSemaphoreCreateInfo semaphore_create_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    // Semaphores
    vkCreateSemaphore(context.device.logical_device,
                      &semaphore_create_info,
                      context.allocator,
                      &context.image_available_semaphores[i]);
    vkCreateSemaphore(context.device.logical_device,
                      &semaphore_create_info,
                      context.allocator,
                      &context.queue_complete_semaphores[i]);
    // Fences
    vulkan_fence_create(&context, true, &context.in_flight_fences[i]);
  }
  context.images_in_flight = darray_reserve(vulkan_fence, context.swapchain.image_count);
  for (u32 i = 0; i < context.swapchain.image_count; ++i) context.images_in_flight[i] = 0;

  // Create builtin shaders
  if (!vulkan_material_shader_create(&context, &context.material_shader)) {
    KERROR("Failed to create the builtin shader");
    return false;
  }

  // Create vertex and index buffers
  create_buffers(&context);

  // Mark all geometries as invalid
  for (u32 i = 0; i < GEOMETRY_MAX_COUNT; ++i) context.geometries[i].id = INVALID_ID;

  KINFO("Vulkan renderer initialized");
  return true;
}

void vulkan_renderer_backend_shutdown(renderer_backend *backend) {
  (void) backend;  // Unused parameter
  // Wait until device is idle
  vkDeviceWaitIdle(context.device.logical_device);

  // Destroy vertex and index buffers
  vulkan_buffer_destroy(&context, &context.object_vertex_buffer);
  vulkan_buffer_destroy(&context, &context.object_index_buffer);

  vulkan_material_shader_destroy(&context, &context.material_shader);

  // Destroy semaphores & fences
  for (u8 i = 0; i < context.swapchain.max_frames_in_flight; ++i) {
    // Semaphores
    if (context.image_available_semaphores[i]) {
      vkDestroySemaphore(context.device.logical_device,
                         context.image_available_semaphores[i],
                         context.allocator);
      context.image_available_semaphores[i] = 0;
    }
    if (context.queue_complete_semaphores[i]) {
      vkDestroySemaphore(context.device.logical_device,
                         context.queue_complete_semaphores[i],
                         context.allocator);
      context.queue_complete_semaphores[i] = 0;
    }
    // Fences
    vulkan_fence_destroy(&context, &context.in_flight_fences[i]);
  }
  darray_destroy(context.image_available_semaphores);
  darray_destroy(context.queue_complete_semaphores);
  darray_destroy(context.in_flight_fences);
  darray_destroy(context.images_in_flight);
  context.image_available_semaphores = 0;
  context.queue_complete_semaphores = 0;
  context.in_flight_fences = 0;
  context.images_in_flight = 0;

  // Destroy command buffers
  for (u32 i = 0; i < context.swapchain.image_count; ++i) {
    if (context.graphics_command_buffers[i].handle) {
      vulkan_command_buffer_free(&context,
                                 context.device.graphics_command_pool,
                                 &context.graphics_command_buffers[i]);
      context.graphics_command_buffers[i].handle = 0;
    }
  }
  darray_destroy(context.graphics_command_buffers);
  context.graphics_command_buffers = 0;

  // Destroy swapchain framebuffers
  for (u32 i = 0; i < context.swapchain.image_count; ++i) {
    vulkan_framebuffer_destroy(&context, &context.swapchain.framebuffers[i]);
  }

  // Destroy renderpass
  vulkan_renderpass_destroy(&context, &context.main_renderpass);

  // Destroy swapchain
  vulkan_swapchain_destroy(&context, &context.swapchain);

  // Destroy device
  vulkan_device_destroy(&context);
  KDEBUG("Vulkan device destroyed");

  // Destroy surface
  if (context.surface) {
    vkDestroySurfaceKHR(context.instance,
                        context.surface,
                        context.allocator);
    context.surface = 0;
    KDEBUG("Vulkan surface destroyed");
  }

  // Destroy debugger
#if defined(_DEBUG)
  if (context.debug_messenger) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
    func(context.instance, context.debug_messenger, context.allocator);
    KDEBUG("Vulkan debugger destroyed");
  }
#endif

  // Destroy instance
  vkDestroyInstance(context.instance, context.allocator);
  KDEBUG("Vulkan instance destroyed");
}

void vulkan_renderer_backend_on_resized(renderer_backend *backend, u16 width, u16 height) {
  (void) backend;  // Unused parameter

  cached_framebuffer_width = width;
  cached_framebuffer_height = height;
  ++context.framebuffer_size_generation;
  KINFO("vulkan_renderer_backend_on_resized :: %ix%i (%llu)", width, height, context.framebuffer_size_generation);
}

b8 vulkan_renderer_backend_begin_frame(renderer_backend *backend, f32 delta_time) {
  context.frame_delta_time = delta_time;
  vulkan_device *device = &context.device;

  // Swapchain recreation
  if (context.recreating_swapchain) {
    VkResult result = vkDeviceWaitIdle(device->logical_device);
    if (!vulkan_result_is_success(result)) {
      KERROR("vulkan_renderer_backend_begin_frame (1) :: %s", vulkan_result_string(result, true));
      return false;
    }
    KINFO("Swapchain recreated");
    return false;
  }

  if (context.framebuffer_size_generation != context.framebuffer_size_last_generation) {
    VkResult result = vkDeviceWaitIdle(device->logical_device);
    if (!vulkan_result_is_success(result)) {
      KERROR("vulkan_renderer_backend_begin_frame (2) :: %s", vulkan_result_string(result, true));
      return false;
    }
    if (!recreate_swapchain(backend)) return false;
    KINFO("Swapchain resized");
    return false;
  }

  if (!vulkan_fence_wait(&context,
                         &context.in_flight_fences[context.current_frame],
                         UINT64_MAX)) {
    KWARN("vulkan_fence_wait :: in-flight fence wait operation failed");
    return false;
  }

  if (!vulkan_swapchain_get_next_image_index(&context,
                                             &context.swapchain,
                                             UINT64_MAX,
                                             context.image_available_semaphores[context.current_frame],
                                             0,
                                             &context.image_index)) return false;

  vulkan_command_buffer *command_buffer = &context.graphics_command_buffers[context.image_index];
  vulkan_command_buffer_reset(command_buffer);
  vulkan_command_buffer_begin(command_buffer, false, false, false);

  VkViewport viewport = {
    .x = 0.0f,
    .y = (f32) context.framebuffer_height,
    .width = (f32) context.framebuffer_width,
    .height = - (f32) context.framebuffer_height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f
  };
  VkRect2D scissor = {
    .offset.x = 0,
    .offset.y = 0,
    .extent.width = context.framebuffer_width,
    .extent.height = context.framebuffer_height
  };

  vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
  vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);

  context.main_renderpass.w = context.framebuffer_width;
  context.main_renderpass.h = context.framebuffer_height;

  // Begin the main renderpass
  vulkan_renderpass_begin(command_buffer,
                          &context.main_renderpass,
                          context.swapchain.framebuffers[context.image_index].handle);

  return true;
}

void vulkan_renderer_backend_update(Matrix4 proj,
                                    Matrix4 view,
                                    Vector3 view_pos,
                                    Vector4 ambient_color,
                                    i32 mode) {
  (void) view_pos;       // Unused parameter
  (void) ambient_color;  // Unused parameter
  (void) mode;           // Unused parameter

  vulkan_material_shader_use(&context, &context.material_shader);
  context.material_shader.global_ubo.proj = proj;
  context.material_shader.global_ubo.view = view;
  vulkan_material_shader_update(&context, &context.material_shader, context.frame_delta_time);
}

b8 vulkan_renderer_backend_end_frame(renderer_backend *backend, f32 delta_time) {
  (void) backend;     // Unused parameter
  (void) delta_time;  // Unused parameter

  vulkan_command_buffer *command_buffer = &context.graphics_command_buffers[context.image_index];

  // End the main renderpass
  vulkan_renderpass_end(command_buffer, &context.main_renderpass);
  // End the command buffer
  vulkan_command_buffer_end(command_buffer);

  if (context.images_in_flight[context.image_index] != VK_NULL_HANDLE) {
    vulkan_fence_wait(&context,
                      context.images_in_flight[context.image_index],
                      UINT64_MAX);
  }
  context.images_in_flight[context.image_index] = &context.in_flight_fences[context.current_frame];

  vulkan_fence_reset(&context, &context.in_flight_fences[context.current_frame]);

  VkPipelineStageFlags flags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &command_buffer->handle,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &context.queue_complete_semaphores[context.current_frame],
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &context.image_available_semaphores[context.current_frame],
    .pWaitDstStageMask = flags
  };
  VkResult result = vkQueueSubmit(context.device.graphics_queue,
                                  1,
                                  &submit_info,
                                  context.in_flight_fences[context.current_frame].handle);
  if (result != VK_SUCCESS) {
    KERROR("vulkan_renderer_backend_end_frame :: %s", vulkan_result_string(result, true));
    return false;
  }
  vulkan_command_buffer_update(command_buffer);

  // Present frame to display
  vulkan_swapchain_present(&context,
                           &context.swapchain,
                           context.device.graphics_queue,
                           context.device.present_queue,
                           context.queue_complete_semaphores[context.current_frame],
                           context.image_index);

  return true;
}

void vulkan_renderer_backend_draw_geometry(geometry_render_data data) {
  if (data.geometry && data.geometry->internal_id == INVALID_ID) return;

  vulkan_geometry_data *buf_data = &context.geometries[data.geometry->internal_id];
  vulkan_command_buffer *command_buffer = &context.graphics_command_buffers[context.image_index];

  vulkan_material_shader_use(&context, &context.material_shader);
  vulkan_material_shader_set_model(&context,
                                   &context.material_shader,
                                   data.model);
  material *m = 0;
  if (data.geometry->material) m = data.geometry->material;
  else m = material_system_get_fallback();
  vulkan_material_shader_apply_material(&context,
                                        &context.material_shader,
                                        m);

  // Bind vertex buffer
  VkDeviceSize offsets[] = { buf_data->vertex_buffer_offset };
  vkCmdBindVertexBuffers(command_buffer->handle,
                         0,
                         1,
                         &context.object_vertex_buffer.handle,
                         offsets);
  if (buf_data->index_count) {
    // Bind index buffer
    vkCmdBindIndexBuffer(command_buffer->handle,
                         context.object_index_buffer.handle,
                         buf_data->index_buffer_offset,
                         VK_INDEX_TYPE_UINT32);
    // Draw indexed data
    vkCmdDrawIndexed(command_buffer->handle,
                     buf_data->index_count,
                     1,
                     0,
                     0,
                     0);
  }
  // Draw non-indexed data
  else vkCmdDraw(command_buffer->handle,
                 buf_data->vertex_count,
                 1,
                 0,
                 0);
}

b8 vulkan_renderer_backend_create_geometry(geometry *geometry,
                                           u32 vertex_count,
                                           const vertex_3d *vertices,
                                           u32 index_count,
                                           const u32 *indices) {
  if (!vertex_count || !vertices) {
    KERROR("vulkan_renderer_backend_create_geometry :: vertex data is required");
    return false;
  }

  b8 is_reupload = geometry->internal_id != INVALID_ID;
  vulkan_geometry_data *internal_data = 0;
  vulkan_geometry_data old_range;
  if (is_reupload) {
    internal_data = &context.geometries[geometry->internal_id];
    old_range = (vulkan_geometry_data) {
      .index_buffer_offset = internal_data->index_buffer_offset,
      .index_count = internal_data->index_count,
      .index_size = internal_data->index_size,
      .vertex_buffer_offset = internal_data->vertex_buffer_offset,
      .vertex_count = internal_data->vertex_count,
      .vertex_size = internal_data->vertex_size
    };
  }
  else {
    for (u32 i = 0; i < GEOMETRY_MAX_COUNT; ++i) {
      if (context.geometries[i].id != INVALID_ID) continue;
      geometry->internal_id = i;
      context.geometries[i].id = i;
      internal_data = &context.geometries[i];
      break;
    }
  }

  if (!internal_data) {
    KFATAL("vulkan_renderer_backend_create_geometry :: geometry system is full (adjust config to allow more geometries)");
    return false;
  }

  VkCommandPool pool = context.device.graphics_command_pool;
  VkQueue queue = context.device.graphics_queue;

  // Vertex data
  internal_data->vertex_buffer_offset = context.geometry_vertex_offset;
  internal_data->vertex_count = vertex_count;
  internal_data->vertex_size = sizeof(vertex_3d) * vertex_count;
  upload_data_range(&context,
                    pool,
                    0,
                    queue,
                    &context.object_vertex_buffer,
                    internal_data->vertex_buffer_offset,
                    internal_data->vertex_size,
                    vertices);
  context.geometry_vertex_offset += internal_data->vertex_size;

  // Index data
  if (index_count && indices) {
    internal_data->index_buffer_offset = context.geometry_index_offset;
    internal_data->index_count = index_count;
    internal_data->index_size = sizeof(u32) * index_count;
    upload_data_range(&context,
                      pool,
                      0,
                      queue,
                      &context.object_index_buffer,
                      internal_data->index_buffer_offset,
                      internal_data->index_size,
                      indices);
    context.geometry_index_offset += internal_data->index_size;
  }

  if (internal_data->generation == INVALID_ID) internal_data->generation = 0;
  else ++internal_data->generation;

  if (is_reupload) {
    free_data_range(&context.object_vertex_buffer,
                    old_range.vertex_buffer_offset,
                    old_range.vertex_size);
    if (old_range.index_size) free_data_range(&context.object_index_buffer,
                                              old_range.index_buffer_offset,
                                              old_range.index_size);
  }

  return true;
}

void vulkan_renderer_backend_destroy_geometry(geometry *geometry) {
  if (!geometry || geometry->internal_id == INVALID_ID) return;
  vkDeviceWaitIdle(context.device.logical_device);
  vulkan_geometry_data *internal_data = &context.geometries[geometry->internal_id];

  // Vertex data
  free_data_range(&context.object_vertex_buffer,
                  internal_data->vertex_buffer_offset,
                  internal_data->vertex_size);
  // Index data
  if (internal_data->index_size) free_data_range(&context.object_index_buffer,
                                                internal_data->index_buffer_offset,
                                                internal_data->index_size);

  kzero_memory(internal_data, sizeof(vulkan_geometry_data));
  internal_data->id = INVALID_ID;
  internal_data->generation = INVALID_ID;
}

void vulkan_renderer_backend_create_texture(const u8 *pixels, texture *t) {
  // Internal data
  t->data = (vulkan_texture_data *) kallocate(sizeof(vulkan_texture_data), MEMORY_TAG_TEXTURE);
  vulkan_texture_data *data = (vulkan_texture_data *) t->data;
  VkDeviceSize image_size = t->width * t->height * t->channel_count;
  VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;

  // Create buffer
  VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  VkMemoryPropertyFlags mem_prop_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  vulkan_buffer buf;
  vulkan_buffer_create(&context,
                       image_size,
                       usage,
                       mem_prop_flags,
                       true,
                       &buf);
  vulkan_buffer_load(&context,
                     &buf,
                     0,
                     image_size,
                     0,
                     pixels);

  // Create image
  vulkan_image_create(&context,
                      VK_IMAGE_TYPE_2D,
                      t->width,
                      t->height,
                      image_format,
                      VK_IMAGE_TILING_OPTIMAL,
                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                      true,
                      VK_IMAGE_ASPECT_COLOR_BIT,
                      &data->image);

  // Create command buffer
  vulkan_command_buffer cmd_buf;
  VkCommandPool pool = context.device.graphics_command_pool;
  VkQueue queue = context.device.graphics_queue;
  vulkan_command_buffer_oneshot_start(&context, pool, &cmd_buf);

  // Transition layout from undefined to transfer_dst
  vulkan_image_transition_layout(&context,
                                 &cmd_buf,
                                 &data->image,
                                 image_format,
                                 VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // Copy data from buffer
  vulkan_image_copy(&context, &data->image, buf.handle, &cmd_buf);

  // Transition layout from transfer_dst to shader_read_only
  vulkan_image_transition_layout(&context,
                                 &cmd_buf,
                                 &data->image,
                                 image_format,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  // End command buffer
  vulkan_command_buffer_oneshot_end(&context, pool, &cmd_buf, queue);
  vulkan_buffer_destroy(&context, &buf);

  // Create texture sampler
  VkSamplerCreateInfo sampler_info = {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .magFilter = VK_FILTER_LINEAR,
    .minFilter = VK_FILTER_LINEAR,
    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .anisotropyEnable = VK_TRUE,
    .maxAnisotropy = 16,
    .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
    .unnormalizedCoordinates = VK_FALSE,
    .compareEnable = VK_FALSE,
    .compareOp = VK_COMPARE_OP_ALWAYS,
    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
    .mipLodBias = 0,
    .minLod = 0,
    .maxLod = 0
  };
  VkResult result = vkCreateSampler(context.device.logical_device,
                                    &sampler_info,
                                    context.allocator,
                                    &data->sampler);
  if (!vulkan_result_is_success(result)) {
    KERROR("Failed to create texture sampler: %s", vulkan_result_string(result, true));
    return;
  }
  ++t->generation;
}

void vulkan_renderer_backend_destroy_texture(texture *in_texture) {
  vkDeviceWaitIdle(context.device.logical_device);

  vulkan_texture_data *data = (vulkan_texture_data *) in_texture->data;
  if (data) {
    vulkan_image_destroy(&context, &data->image);
    kzero_memory(&data->image, sizeof(vulkan_image));
    vkDestroySampler(context.device.logical_device,
                     data->sampler,
                     context.allocator);
    data->sampler = 0;
    kfree(in_texture->data, sizeof(vulkan_texture_data), MEMORY_TAG_TEXTURE);
  }
  kzero_memory(in_texture, sizeof(texture));
}

b8 vulkan_renderer_backend_create_material(material *material) {
  if (!material) {
    KERROR("vulkan_renderer_backend_create_material :: `material` is required");
    return false;
  }
  if (!vulkan_material_shader_get_resources(&context,
                                            &context.material_shader,
                                            material)) {
    KERROR("vulkan_renderer_backend_create_material :: failed to get shader resources");
    return false;
  }
  KTRACE("vulkan_renderer_backend_create_material :: material created successfully");
  return true;
}

void vulkan_renderer_backend_destroy_material(material *material) {
  if (!material) {
    KWARN("vulkan_renderer_backend_destroy_material :: `material` is required (nothing was done)");
    return;
  }
  if (material->internal_id == INVALID_ID) {
    KWARN("vulkan_renderer_backend_destroy_material :: `internal_id` is invalid (nothing was done)");
    return;
  }
  vulkan_material_shader_release_resources(&context,
                                           &context.material_shader,
                                           material);
}
