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

void vulkan_swapchain_create(vulkan_context *context, u32 width, u32 height, vulkan_swapchain *swapchain);

void vulkan_swapchain_recreate(vulkan_context *context, u32 width, u32 height, vulkan_swapchain *swapchain);

void vulkan_swapchain_destroy(vulkan_context *context, vulkan_swapchain *swapchain);

b8 vulkan_swapchain_get_next_image_index(vulkan_context *context,
                                         vulkan_swapchain *swapchain,
                                         u64 timeout_ns,
                                         VkSemaphore image_available_semaphore,
                                         VkFence fence,
                                         u32 *out_image_index);

void vulkan_swapchain_present(vulkan_context *context,
                              vulkan_swapchain *swapchain,
                              VkQueue graphics_queue,
                              VkQueue present_queue,
                              VkSemaphore render_complete_semaphore,
                              u32 present_image_index);
