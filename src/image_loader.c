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
#include <kmemory.h>
#include <kstring.h>
#include <image_loader.h>
#include <loader_utils.h>
#include <resource_types.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define FILE_PATH_SIZE 512

static b8 image_loader_load(resource_loader *self, const char *name, resource *out_resource) {
  if (!self || !name || !out_resource) return false;

  char *format_str = "%s/%s/%s%s";
  const i32 required_channel_count = 4;
  stbi_set_flip_vertically_on_load(true);
  char full_file_path[FILE_PATH_SIZE];

  // TODO: support other image format/extensions
  kstrfmt(full_file_path,
          format_str,
          resource_system_get_base_path(),
          self->type_path,
          name,
          ".png");

  i32 width;
  i32 height;
  i32 channel_count;

  u8 *data = stbi_load(full_file_path,
                       &width,
                       &height,
                       &channel_count,
                       required_channel_count);

  const char *fail_reason = stbi_failure_reason();
  if (fail_reason) {
    KERROR("image_loader_load :: failed to load file '%s': %s",
           full_file_path,
           fail_reason);
    stbi__err(0, 0);
    if (data) stbi_image_free(data);
    return false;
  }
  if (!data) {
    KERROR("image_loader_load :: failed to load file '%s'", full_file_path);
    return false;
  }

  out_resource->full_path = kstrdup(full_file_path);
  image_resource_data *resource_data = kallocate(sizeof(image_resource_data),
                                                 MEMORY_TAG_TEXTURE);
  resource_data->pixels = data;
  resource_data->width = width;
  resource_data->height = height;
  resource_data->channel_count = required_channel_count;
  out_resource->data = resource_data;
  out_resource->data_size = sizeof(image_resource_data);
  out_resource->name = name;

  return true;
}

static void image_loader_unload(resource_loader *self, resource *resource) {
  if (!resource_unload(self, resource, MEMORY_TAG_TEXTURE)) {
    KWARN("image_loader_unload :: `self` and `resource` are required");
  }
}

resource_loader image_loader_create(void) {
  return (resource_loader) {
    .type = RESOURCE_TYPE_IMAGE,
    .custom_type = 0,
    .load = image_loader_load,
    .unload = image_loader_unload,
    .type_path = "textures"
  };
}
