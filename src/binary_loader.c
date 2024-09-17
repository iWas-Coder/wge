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


#include <kmath.h>
#include <logger.h>
#include <kmemory.h>
#include <kstring.h>
#include <filesystem.h>
#include <loader_utils.h>
#include <binary_loader.h>
#include <resource_types.h>

#define FILE_PATH_SIZE 512

static b8 binary_loader_load(resource_loader *self, const char *name, resource *out_resource) {
  if (!self || !name || !out_resource) return false;

  char *format_str = "%s/%s/%s%s";
  char full_file_path[FILE_PATH_SIZE];
  kstrfmt(full_file_path,
          format_str,
          resource_system_get_base_path(),
          self->type_path,
          name,
          "");

  file_handle fd;
  if (!filesystem_open(full_file_path, FILE_MODE_READ, true, &fd)) {
    KERROR("binary_loader_load :: unable to open file '%s'", full_file_path);
    return false;
  }

  out_resource->full_path = kstrdup(full_file_path);

  u64 file_size = 0;
  if (!filesystem_sizeof(&fd, &file_size)) {
    KERROR("binary_loader_load :: unable to read file '%s'", full_file_path);
    filesystem_close(&fd);
    return false;
  }

  u8 *resource_data = kallocate(sizeof(u8) * file_size, MEMORY_TAG_ARRAY);
  u64 read_size = 0;
  if (!filesystem_read_bytes_all(&fd, resource_data, &read_size)) {
    KERROR("binary_loader_load :: unable to read file '%s'", full_file_path);
    filesystem_close(&fd);
    return false;
  }

  filesystem_close(&fd);

  out_resource->data = resource_data;
  out_resource->data_size = read_size;
  out_resource->name = name;
  return true;
}

static void binary_loader_unload(resource_loader *self, resource *resource) {
  if (!resource_unload(self, resource, MEMORY_TAG_ARRAY)) {
    KWARN("binary_loader_unload :: `self` and `resource` are required");
  }
}

resource_loader binary_loader_create(void) {
  return (resource_loader) {
    .type = RESOURCE_TYPE_BINARY,
    .custom_type = 0,
    .load = binary_loader_load,
    .unload = binary_loader_unload,
    .type_path = ""
  };
}
