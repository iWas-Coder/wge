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
#include <vulkan_device.h>

typedef struct {
  b8 discrete_gpu;
  b8 graphics;
  b8 present;
  b8 compute;
  b8 transfer;
  b8 sampler_anisotropy;
  const char **extensions;
} vulkan_physical_device_requirements;

typedef struct {
  i32 graphics;
  i32 present;
  i32 compute;
  i32 transfer;
} vulkan_physical_device_queue_family_info;

void vulkan_device_query_swapchain_support(VkPhysicalDevice physical_device,
                                           VkSurfaceKHR surface,
                                           vulkan_swapchain_support_info *out_support_info) {
  // Surface capabilities
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device,
                                                     surface,
                                                     &out_support_info->capabilities));
  // Surface formats
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
                                                surface,
                                                &out_support_info->format_count,
                                                0));
  if (out_support_info->format_count) {
    if (!out_support_info->formats) {
      out_support_info->formats = kallocate(sizeof(VkSurfaceFormatKHR) * out_support_info->format_count,
                                           MEMORY_TAG_RENDERER);
    }
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
                                                  surface,
                                                  &out_support_info->format_count,
                                                  out_support_info->formats));
  }
  // Present
  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                                                     surface,
                                                     &out_support_info->present_mode_count,
                                                     0));
  if (out_support_info->present_mode_count) {
    if (!out_support_info->present_modes) {
      out_support_info->present_modes = kallocate(sizeof(VkPresentModeKHR) * out_support_info->present_mode_count,
                                                 MEMORY_TAG_RENDERER);
    }
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                                                       surface,
                                                       &out_support_info->present_mode_count,
                                                       out_support_info->present_modes));
  }
}

b8 physical_device_meets_requirements(VkPhysicalDevice device,
                                      VkSurfaceKHR surface,
                                      const VkPhysicalDeviceProperties* properties,
                                      const VkPhysicalDeviceFeatures* features,
                                      const vulkan_physical_device_requirements* requirements,
                                      vulkan_physical_device_queue_family_info* out_queue_info,
                                      vulkan_swapchain_support_info* out_swapchain_support) {
  out_queue_info->graphics = -1;
  out_queue_info->present  = -1;
  out_queue_info->compute  = -1;
  out_queue_info->transfer = -1;

  // Discrete GPU check
  if (requirements->discrete_gpu) {
    if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      KINFO("Device skipped (no discrete GPU)");
      return FALSE;
    }
  }

  u32 queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);
  VkQueueFamilyProperties queue_families[queue_family_count];
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

  KINFO("Graphics | Present | Compute | Transfer | Name");
  u8 min_transfer_score = 255;
  for (u32 i = 0; i < queue_family_count; ++i) {
    u8 current_transfer_score = 0;
    // Graphics
    if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      out_queue_info->graphics = i;
      ++current_transfer_score;
    }
    // Compute
    if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      out_queue_info->compute = i;
      ++current_transfer_score;
    }
    // Transfer
    if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
      // Increase the likelihood that it is a dedicated transfer queue
      if (current_transfer_score <= min_transfer_score) {
        min_transfer_score = current_transfer_score;
        out_queue_info->transfer = i;
      }
    }
    // Present
    VkBool32 supports_present = VK_FALSE;
    VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_present));
    if (supports_present) out_queue_info->present = i;
  }
  KINFO("       %d |       %d |       %d |        %d | %s",
        out_queue_info->graphics != -1,
        out_queue_info->present != -1,
        out_queue_info->compute != -1,
        out_queue_info->transfer != -1,
        properties->deviceName);

  // Check if requirements are met
  if ((!requirements->graphics || (requirements->graphics && out_queue_info->graphics != -1)) &&
      (!requirements->present  || (requirements->present  && out_queue_info->present  != -1)) &&
      (!requirements->compute  || (requirements->compute  && out_queue_info->compute  != -1)) &&
      (!requirements->transfer || (requirements->transfer && out_queue_info->transfer != -1))) {
    KINFO("Device meets requirements");
    KTRACE("Graphics Family Index: %i", out_queue_info->graphics);
    KTRACE("Present Family Index:  %i", out_queue_info->present);
    KTRACE("Transfer Family Index: %i", out_queue_info->transfer);
    KTRACE("Compute Family Index:  %i", out_queue_info->compute);

    // Query swapchain support
    vulkan_device_query_swapchain_support(device, surface, out_swapchain_support);
    if (out_swapchain_support->format_count < 1 ||
        out_swapchain_support->present_mode_count < 1) {
      if (out_swapchain_support->formats) {
        kfree(out_swapchain_support->formats,
              sizeof(VkSurfaceFormatKHR) * out_swapchain_support->format_count,
              MEMORY_TAG_RENDERER);
      }
      if (out_swapchain_support->present_modes) {
        kfree(out_swapchain_support->present_modes,
              sizeof(VkPresentModeKHR) * out_swapchain_support->present_mode_count,
              MEMORY_TAG_RENDERER);
      }
      KINFO("Device skipped (no swapchain)");
      return FALSE;
    }

    // Device extensions
    if (requirements->extensions) {
      u32 available_extension_count = 0;
      VkExtensionProperties *available_extensions = 0;
      VK_CHECK(vkEnumerateDeviceExtensionProperties(device,
                                                    0,
                                                    &available_extension_count,
                                                    0));
      if (available_extension_count) {
        available_extensions = kallocate(sizeof(VkExtensionProperties) * available_extension_count,
                                         MEMORY_TAG_RENDERER);
        VK_CHECK(vkEnumerateDeviceExtensionProperties(device,
                                                      0,
                                                      &available_extension_count,
                                                      available_extensions));
        u32 required_extension_count = darray_length(requirements->extensions);
        for (u32 i = 0; i < required_extension_count; ++i) {
          b8 found = FALSE;
          for (u32 j = 0; i < available_extension_count; ++j) {
            if (kstrcmp(requirements->extensions[i], available_extensions[j].extensionName)) {
              found = TRUE;
              break;
            }
          }
          if (!found) {
            KINFO("Device skipped (no extension: %s)", requirements->extensions[i]);
            kfree(available_extensions,
                  sizeof(VkExtensionProperties) * available_extension_count,
                  MEMORY_TAG_RENDERER);
            return FALSE;
          }
        }
      }
      kfree(available_extensions,
            sizeof(VkExtensionProperties) * available_extension_count,
            MEMORY_TAG_RENDERER);
    }

    // Sampler anisotropy
    if (requirements->sampler_anisotropy && !features->samplerAnisotropy) {
      KINFO("Device skipped (no samplerAnisotropy)");
      return FALSE;
    }

    // All requirements met
    return TRUE;
  }
  return FALSE;
}

b8 select_physical_device(vulkan_context *context) {
  u32 c = 0;
  VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &c, 0));
  if (!c) {
    KFATAL("No devices supporting Vulkan were found");
    return FALSE;
  }
  VkPhysicalDevice physical_devices[c];
  VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &c, physical_devices));

  for (u32 i = 0; i < c; ++i) {
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceProperties(physical_devices[i], &properties);
    vkGetPhysicalDeviceFeatures(physical_devices[i], &features);
    vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory);

    vulkan_physical_device_requirements requirements = {
      .discrete_gpu = TRUE,
      .graphics = TRUE,
      .present = TRUE,
      .compute = TRUE,
      .transfer = TRUE,
      .sampler_anisotropy = TRUE,
      .extensions = darray_create(const char *)
    };
    darray_push(requirements.extensions, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    vulkan_physical_device_queue_family_info queue_info = {0};
    b8 result = physical_device_meets_requirements(physical_devices[i],
                                                   context->surface,
                                                   &properties,
                                                   &features,
                                                   &requirements,
                                                   &queue_info,
                                                   &context->device.swapchain_support);
    if (result) {
      // Device name and type
      KINFO("Selected device: '%s'", properties.deviceName);
      switch (properties.deviceType) {
      default:
      case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        KINFO("Device type: 'Unknown'");
        break;
      case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        KINFO("Device type: 'Integrated GPU'");
        break;
      case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        KINFO("Device type: 'Discrete GPU'");
        break;
      case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        KINFO("Device type: 'Virtual GPU'");
        break;
      case VK_PHYSICAL_DEVICE_TYPE_CPU:
        KINFO("Device type: 'CPU'");
        break;
      }
      // GPU driver and Vulkan API versions
      KINFO("GPU driver version: %d.%d.%d",
            VK_VERSION_MAJOR(properties.driverVersion),
            VK_VERSION_MINOR(properties.driverVersion),
            VK_VERSION_PATCH(properties.driverVersion));
      KINFO("Vulkan API version: %d.%d.%d",
            VK_VERSION_MAJOR(properties.apiVersion),
            VK_VERSION_MINOR(properties.apiVersion),
            VK_VERSION_PATCH(properties.apiVersion));
      // Memory
      for (u32 j = 0; j < memory.memoryHeapCount; ++j) {
        f32 memory_size_gib = ((f32) memory.memoryHeaps[j].size / MEM_B_IN_GIB);
        if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
          KINFO("Dedicated GPU memory: %.2f GiB", memory_size_gib);
        }
        else KINFO("Shared system memory: %.2f GiB", memory_size_gib);
      }

      // Saving queried info to the device struct
      context->device = (vulkan_device) {
        .physical_device = physical_devices[i],
        .graphics = queue_info.graphics,
        .present = queue_info.present,
        .compute = queue_info.compute,
        .transfer = queue_info.transfer,
        .properties = properties,
        .features = features,
        .memory = memory
      };
      break;
    }
  }
  // Ensure a device was selected
  if (!context->device.physical_device) {
    KERROR("No physical devices were found meeting the requirements");
    return FALSE;
  }
  KINFO("Physical device selected");
  return TRUE;
}

b8 vulkan_device_create(vulkan_context *context) {
  // Select physical device
  if (!select_physical_device(context)) return FALSE;

  // Create logical device
  b8 present_shares_graphics_queue = (context->device.graphics == context->device.present);
  b8 transfer_shares_graphics_queue = (context->device.graphics == context->device.transfer);
  u32 index_count = 1;
  if (!present_shares_graphics_queue) ++index_count;
  if (!transfer_shares_graphics_queue) ++index_count;
  u32 indices[index_count];
  u8 idx = 0;
  indices[idx] = context->device.graphics;
  if (!present_shares_graphics_queue) indices[++idx] = context->device.present;
  if (!transfer_shares_graphics_queue) indices[++idx] = context->device.transfer;

  // Queue create info array
  VkDeviceQueueCreateInfo queue_create_infos[index_count];
  for (u32 i = 0; i < index_count; ++i) {
    f32 *queue_priority = darray_create(f32);
    darray_push(queue_priority, 1.0f);
    queue_create_infos[i] = (VkDeviceQueueCreateInfo) {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = indices[i],
      .queueCount = 1,
      .pQueuePriorities = queue_priority,
      .flags = 0,
      .pNext = 0
    };
    if ((i32) indices[i] == context->device.graphics) {
      queue_create_infos[i].queueCount = 2;
      darray_push(queue_priority, 1.0f);
    }
  }

  // Device features
  VkPhysicalDeviceFeatures device_features = {
    .samplerAnisotropy = VK_TRUE
  };
  // Device create info
  const char *extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
  VkDeviceCreateInfo device_create_info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .queueCreateInfoCount = index_count,
    .pQueueCreateInfos = queue_create_infos,
    .pEnabledFeatures = &device_features,
    .enabledExtensionCount = 1,
    .ppEnabledExtensionNames = &extension_names,
    .enabledLayerCount = 0,
    .ppEnabledLayerNames = 0
  };

  VK_CHECK(vkCreateDevice(context->device.physical_device,
                          &device_create_info,
                          context->allocator,
                          &context->device.logical_device));
  KINFO("Logical device created");

  // Obtain queue handlers
  vkGetDeviceQueue(context->device.logical_device,
                   context->device.graphics,
                   0,
                   &context->device.graphics_queue);
  vkGetDeviceQueue(context->device.logical_device,
                   context->device.present,
                   0,
                   &context->device.present_queue);
  vkGetDeviceQueue(context->device.logical_device,
                   context->device.compute,
                   0,
                   &context->device.compute_queue);
  vkGetDeviceQueue(context->device.logical_device,
                   context->device.transfer,
                   0,
                   &context->device.transfer_queue);
  KINFO("Queue handlers obtained");

  // Create command pool (for graphics queue)
  VkCommandPoolCreateInfo pool_create_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .queueFamilyIndex = context->device.graphics,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
  };
  VK_CHECK(vkCreateCommandPool(context->device.logical_device,
                               &pool_create_info,
                               context->allocator,
                               &context->device.graphics_command_pool));
  KINFO("Graphics command pool created");

  return TRUE;
}

void vulkan_device_destroy(vulkan_context *context) {
  // Unset queue handlers
  context->device.graphics_queue = 0;
  context->device.present_queue  = 0;
  context->device.compute_queue  = 0;
  context->device.transfer_queue = 0;

  // Destroy graphics command pool
  vkDestroyCommandPool(context->device.logical_device,
                       context->device.graphics_command_pool,
                       context->allocator);

  // Destroy logical device
  if (context->device.logical_device) {
    vkDestroyDevice(context->device.logical_device, context->allocator);
    context->device.logical_device = 0;
  }
  KINFO("Logical device destroyed");

  // Releasing selected physical device
  context->device.physical_device = 0;
  if (context->device.swapchain_support.formats) {
    kfree(context->device.swapchain_support.formats,
          sizeof(VkSurfaceFormatKHR) * context->device.swapchain_support.format_count,
          MEMORY_TAG_RENDERER);
    context->device.swapchain_support.formats = 0;
    context->device.swapchain_support.format_count = 0;
  }
  if (context->device.swapchain_support.present_modes) {
    kfree(context->device.swapchain_support.present_modes,
          sizeof(VkPresentModeKHR) * context->device.swapchain_support.present_mode_count,
          MEMORY_TAG_RENDERER);
    context->device.swapchain_support.present_modes = 0;
    context->device.swapchain_support.present_mode_count = 0;
  }
  kzero_memory(&context->device.swapchain_support.capabilities,
               sizeof(context->device.swapchain_support.capabilities));
  context->device.graphics = -1;
  context->device.present  = -1;
  context->device.compute  = -1;
  context->device.transfer = -1;
  KINFO("Released physical device resources");
}

b8 vulkan_device_detect_depth_format(vulkan_device *device) {
  const u64 candidate_count = 3;
  VkFormat candidates[] = {
    VK_FORMAT_D32_SFLOAT,          // 32 bits for depth
    VK_FORMAT_D32_SFLOAT_S8_UINT,  // 32 bits for depth and 8 bits for stencil
    VK_FORMAT_D24_UNORM_S8_UINT    // 24 bits for depth (norm.) and 8 bits for stencil
  };
  u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
  for (u64 i = 0; i < candidate_count; ++i) {
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(device->physical_device, candidates[i], &properties);
    if (((properties.linearTilingFeatures | properties.optimalTilingFeatures) & flags) == flags) {
      device->depth_format = candidates[i];
      return TRUE;
    }
  }
  return FALSE;
}
