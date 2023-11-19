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
#include <linear_allocator.h>

void linear_allocator_create(u64 total_size, void *memory, linear_allocator *out_allocator) {
  if (!out_allocator) return;
  out_allocator->total_size = total_size;
  out_allocator->allocated = 0;
  out_allocator->owns_memory = (memory == 0);
  if (memory) out_allocator->memory = memory;
  else out_allocator->memory = kallocate(total_size, MEMORY_TAG_LINEAR_ALLOCATOR);
}

void linear_allocator_destroy(linear_allocator *allocator) {
  if (!allocator) return;
  allocator->allocated = 0;
  if (allocator->owns_memory && allocator->memory) kfree(allocator->memory,
                                                         allocator->total_size,
                                                         MEMORY_TAG_LINEAR_ALLOCATOR);
  allocator->memory = 0;
  allocator->total_size = 0;
  allocator->owns_memory = false;
}

void *linear_allocator_alloc(linear_allocator *allocator, u64 size) {
  if (!allocator || !allocator->memory) {
    KERROR("linear_allocator_alloc :: allocator not initialized");
    return 0;
  }
  if ((allocator->allocated + size) > allocator->total_size) {
    u64 left = allocator->total_size - allocator->allocated;
    KERROR("linear_allocator_alloc :: wanted to alloc %lluB, only %lluB left", size, left);
    return 0;
  }
  void *block = ((b8 *) allocator->memory) + allocator->allocated;
  allocator->allocated += size;
  return block;
}

void linear_allocator_free(linear_allocator *allocator) {
  if (!allocator || !allocator->memory) return;
  allocator->allocated = 0;
  kzero_memory(allocator->memory, allocator->total_size);
}
