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
  void *memory;
} free_list;

KAPI void free_list_create(u32 total_size,
                           u64 *memory_requirements,
                           void *memory,
                           free_list *out_list);

KAPI void free_list_destroy(free_list *list);

KAPI b8 free_list_alloc(free_list *list, u32 size, u32 *out_offset);

KAPI b8 free_list_free(free_list *list, u32 size, u32 offset);

KAPI void free_list_clear(free_list *list);

KAPI u64 free_list_get_free_space(free_list *list);
