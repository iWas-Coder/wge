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

void vulkan_framebuffer_create(vulkan_context *context,
                               vulkan_renderpass *renderpass,
                               u32 width,
                               u32 height,
                               u32 attachment_count,
                               VkImageView *attachments,
                               vulkan_framebuffer *out_framebuffer);

void vulkan_framebuffer_destroy(vulkan_context *context, vulkan_framebuffer *framebuffer);
