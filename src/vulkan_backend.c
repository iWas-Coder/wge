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
#include <application.h>
#include <vulkan_types.h>
#include <vulkan_utils.h>
#include <vulkan_fence.h>
#include <vulkan_device.h>
#include <vulkan_backend.h>
#include <vulkan_platform.h>
#include <vulkan_swapchain.h>
#include <vulkan_renderpass.h>
#include <vulkan_framebuffer.h>
#include <vulkan_object_shader.h>
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
  if (!vulkan_object_shader_create(&context, &context.object_shader)) {
    KERROR("Failed to create the builtin shader");
    return false;
  }

  KINFO("Vulkan renderer initialized");
  return true;
}

void vulkan_renderer_backend_shutdown(renderer_backend *backend) {
  (void) backend;  // Unused parameter
  // Wait until device is idle
  vkDeviceWaitIdle(context.device.logical_device);

  vulkan_object_shader_destroy(&context, &context.object_shader);

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
  (void) delta_time;  // Unused parameter

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
