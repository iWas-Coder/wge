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
  u64 element_size;
  u32 element_count;
  void *memory;
  b8 is_pointer_type;
} hash_table;

KAPI void hash_table_create(u64 element_size,
                            u32 element_count,
                            void *memory,
                            b8 is_pointer_type,
                            hash_table *out_hash_table);

KAPI void hash_table_destroy(hash_table *table);

KAPI b8 hash_table_set(hash_table *table,
                       const char *name,
                       void *value);

KAPI b8 hash_table_set_ptr(hash_table *table,
                           const char *name,
                           void **value);

KAPI b8 hash_table_get(hash_table *table,
                       const char *name,
                       void *out_value);

KAPI b8 hash_table_get_ptr(hash_table *table,
                           const char *name,
                           void **out_value);

KAPI b8 hash_table_fill(hash_table *table,
                        void *value);
