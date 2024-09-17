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

#include <defines.h>

typedef struct {
  u64 total_size;
  u64 allocated;
  void *memory;
  b8 owns_memory;
} linear_allocator;

KAPI void linear_allocator_create(u64 total_size, void *memory, linear_allocator *out_allocator);

KAPI void linear_allocator_destroy(linear_allocator *allocator);

KAPI void *linear_allocator_alloc(linear_allocator *allocator, u64 size);

KAPI void linear_allocator_free(linear_allocator *allocator);
