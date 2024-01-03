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
#include <loader_utils.h>

b8 resource_unload(struct resource_loader *self, resource *resource, memory_tag tag) {
  if (!self || !resource) {
    KWARN("resource_unload :: `self` and `resource` are required");
    return false;
  }
  if (kstrlen(resource->full_path)) kfree(resource->full_path,
                                         sizeof(char) * kstrlen(resource->full_path) + 1,
                                         MEMORY_TAG_STRING);
  if (resource->data) {
    kfree(resource->data, resource->data_size, tag);
    resource->data = 0;
    resource->data_size = 0;
    resource->loader_id = INVALID_ID;
  }
  return true;
}
