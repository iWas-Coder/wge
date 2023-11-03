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
#include <vulkan_types.h>
#include <vulkan_device.h>
#include <vulkan_backend.h>
#include <vulkan_platform.h>

static vulkan_context context;

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

b8 vulkan_renderer_backend_initialize(renderer_backend *backend,
                                      const char *application_name,
                                      struct platform_state *plat_state) {
  (void) backend;     // Unused parameter
  (void) plat_state;  // Unused parameter

  // TODO: custom Vulkan allocator
  context.allocator = 0;

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
    b8 found = FALSE;
    for (u32 j = 0; j < available_layer_count; ++j) {
      if (kstrcmp(vk_layers[i], available_layers[j].layerName)) {
        found = TRUE;
        break;
      }
    }
    if (!found) {
      KFATAL("Missing layer: %s", vk_layers[i]);
      return FALSE;
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
  if (!platform_create_vulkan_surface(plat_state, (struct vulkan_context *) &context)) {
    KERROR("Failed to create surface");
    return FALSE;
  }
  KDEBUG("Vulkan surface created");

  // Create device
  if (!vulkan_device_create(&context)) {
    KERROR("Failed to create device");
    return FALSE;
  }

  KINFO("Vulkan renderer initialized");
  return TRUE;
}

void vulkan_renderer_backend_shutdown(renderer_backend *backend) {
  (void) backend;  // Unused parameter

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
  if (context.debug_messenger) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
    func(context.instance, context.debug_messenger, context.allocator);
    KDEBUG("Vulkan debugger destroyed");
  }

  // Destroy instance
  vkDestroyInstance(context.instance, context.allocator);
  KDEBUG("Vulkan instance destroyed");
}

void vulkan_renderer_backend_on_resized(renderer_backend *backend, u16 width, u16 height) {
  (void) backend;  // Unused parameter
  (void) width;    // Unused parameter
  (void) height;   // Unused parameter
}

b8 vulkan_renderer_backend_begin_frame(renderer_backend *backend, f32 delta_time) {
  (void) backend;     // Unused parameter
  (void) delta_time;  // Unused parameter

  return TRUE;
}

b8 vulkan_renderer_backend_end_frame(renderer_backend *backend, f32 delta_time) {
  (void) backend;     // Unused parameter
  (void) delta_time;  // Unused parameter

  return TRUE;
}
