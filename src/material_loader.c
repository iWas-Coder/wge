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
#include <resource_types.h>
#include <material_loader.h>

#define FILE_PATH_SIZE 512
#define MATERIAL_FILE_LINE_MAX_SIZE 512
#define MATERIAL_FILE_VAR_MAX_SIZE 64
#define MATERIAL_FILE_VALUE_MAX_SIZE (MATERIAL_FILE_LINE_MAX_SIZE - MATERIAL_FILE_VAR_MAX_SIZE - 2)
#define MATERIAL_FILE_EXTENSION_NAME "wmt"

static b8 material_loader_load(resource_loader *self, const char *name, resource *out_resource) {
  if (!self || !name || !out_resource) return false;

  char *format_str = "%s/%s/%s%s";
  char full_file_path[FILE_PATH_SIZE];
  kstrfmt(full_file_path,
          format_str,
          resource_system_get_base_path(),
          self->type_path,
          name,
          "." MATERIAL_FILE_EXTENSION_NAME);

  file_handle fd;
  if (!filesystem_open(full_file_path, FILE_MODE_READ, false, &fd)) {
    KERROR("material_loader_load :: unable to open material file '%s'", full_file_path);
    return false;
  }

  out_resource->full_path = kstrdup(full_file_path);

  material_config *resource_data = kallocate(sizeof(material_config),
                                             MEMORY_TAG_MATERIAL_INSTANCE);
  resource_data->type = MATERIAL_TYPE_WORLD;
  resource_data->auto_release = true;
  resource_data->diffuse_color = vec4_one();  // white
  resource_data->diffuse_map_name[0] = 0;
  kstrncp(resource_data->name, name, MATERIAL_NAME_MAX_LEN);

  char line_buf[MATERIAL_FILE_LINE_MAX_SIZE] = "";
  char *p = &line_buf[0];
  u64 line_len = 0;
  u32 line_num = 1;
  while (filesystem_read_line(&fd,
                              MATERIAL_FILE_LINE_MAX_SIZE - 1,
                              &p,
                              &line_len)) {
    char *trimmed_line = kstrtr(line_buf);
    line_len = kstrlen(trimmed_line);
    // Skip blank lines and comments
    if (!line_len || trimmed_line[0] == '#') {
      ++line_num;
      continue;
    }
    // Parse the line
    i32 equal_idx = kstridx(trimmed_line, '=');
    if (equal_idx == -1) {
      KWARN("load_cfg_file :: `%s:%ui` -> skipping line due to '=' char not found",
            full_file_path,
            line_num);
      ++line_num;
      continue;
    }
    // Store variable
    char raw_var[MATERIAL_FILE_VAR_MAX_SIZE];
    kzero_memory(raw_var, sizeof(char) * MATERIAL_FILE_VAR_MAX_SIZE);
    kstrsub(raw_var, trimmed_line, 0, equal_idx);
    char *trimmed_var = kstrtr(raw_var);
    // Store value
    char raw_value[MATERIAL_FILE_VALUE_MAX_SIZE];
    kzero_memory(raw_value, sizeof(char) * MATERIAL_FILE_VALUE_MAX_SIZE);
    kstrsub(raw_value, trimmed_line, equal_idx + 1, -1);
    char *trimmed_value = kstrtr(raw_value);

    KTRACE("material_loader :: trimmed line -> %s", trimmed_line);
    KTRACE("material_loader :: raw var -> %s", raw_var);
    KTRACE("material_loader :: raw value -> %s", raw_value);

    // Process variable against value
    if (kstrcmpi(trimmed_var, "version")) {
      // TODO: handle version
    }
    else if (kstrcmpi(trimmed_var, "name")) {
      kstrncp(resource_data->name, trimmed_value, MATERIAL_NAME_MAX_LEN);
    }
    else if (kstrcmpi(trimmed_var, "diffuse_map_name")) {
      kstrncp(resource_data->diffuse_map_name, trimmed_value, TEXTURE_NAME_MAX_LEN);
    }
    else if (kstrcmpi(trimmed_var, "diffuse_color")) {
      if (!str_to_vec4(trimmed_value, &resource_data->diffuse_color)) {
        KWARN("material_loader_load :: `%s:%ui` -> `diffuse_color` parse error (using white as fallback)",
              full_file_path,
              line_num);
      }
    }
    else if (kstrcmpi(trimmed_var, "type")) {
      if (kstrcmpi(trimmed_value, "ui")) resource_data->type = MATERIAL_TYPE_UI;
    }

    // Cleanup
    kzero_memory(line_buf, sizeof(char) * MATERIAL_FILE_LINE_MAX_SIZE);
    ++line_num;
  }

  filesystem_close(&fd);
  out_resource->data = resource_data;
  out_resource->data_size = sizeof(material_config);
  out_resource->name = name;

  return true;
}

static void material_loader_unload(resource_loader *self, resource *resource) {
  if (!resource_unload(self, resource, MEMORY_TAG_MATERIAL_INSTANCE)) {
    KWARN("material_loader_unload :: `self` and `resource` are required");
  }
}

resource_loader material_loader_create(void) {
  return (resource_loader) {
    .type = RESOURCE_TYPE_MATERIAL,
    .custom_type = 0,
    .load = material_loader_load,
    .unload = material_loader_unload,
    .type_path = "materials"
  };
}
