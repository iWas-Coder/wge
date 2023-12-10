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
#include <hash_table.h>

#define HASH_MULT 97

u64 hash(const char *name, u32 element_count) {
  u64 hash = 0;
  for (const u8 *us = (const u8 *) name; *us; ++us) {
    hash = hash * HASH_MULT + *us;
  }
  hash %= element_count;
  return hash;
}

void hash_table_create(u64 element_size,
                       u32 element_count,
                       void *memory,
                       b8 is_pointer_type,
                       hash_table *out_hash_table) {
  if (!memory || !out_hash_table) {
    KERROR("hash_table_create :: `memory` and `out_hash_table` are required");
    return;
  }
  if (!element_count || !element_size) {
    KERROR("hash_table_create :: `element_count` and `element_size` must be > 0");
    return;
  }
  // TODO: allocate the memory here
  out_hash_table->memory = memory;
  out_hash_table->element_count = element_count;
  out_hash_table->element_size = element_size;
  out_hash_table->is_pointer_type = is_pointer_type;
  kzero_memory(out_hash_table->memory, element_size * element_count);
}

void hash_table_destroy(hash_table *table) {
  if (table) kzero_memory(table, sizeof(hash_table));
}

b8 hash_table_set(hash_table *table,
                  const char *name,
                  void *value) {
  if (!table || !name || !value) {
    KERROR("hash_table_set :: `table`, `name` and `value` are required");
    return false;
  }
  if (table->is_pointer_type) {
    KERROR("`hash_table_set` called with a pointer-type hash table (use `hash_table_set_ptr` instead)");
    return false;
  }
  u64 h = hash(name, table->element_count);
  kcopy_memory((void *) ((u8 *) table->memory + (table->element_size * h)),
               value,
               table->element_size);
  return true;
}

b8 hash_table_set_ptr(hash_table *table,
                      const char *name,
                      void **value) {
  if (!table || !name) {
    KERROR("hash_table_set_ptr :: `table` and `name` are required");
    return false;
  }
  if (!table->is_pointer_type) {
    KERROR("`hash_table_set_ptr` called without a pointer-type hash table (use `hash_table_set` instead)");
    return false;
  }
  u64 h = hash(name, table->element_count);
  ((void **) table->memory)[h] = value ? *value : 0;
  return true;
}

b8 hash_table_get(hash_table *table,
                  const char *name,
                  void *out_value) {
  if (!table || !name || !out_value) {
    KERROR("hash_table_get :: `table`, `name` and `out_value` are required");
    return false;
  }
  if (table->is_pointer_type) {
    KERROR("`hash_table_get` called with a pointer-type hash table (use `hash_table_get_ptr` instead)");
    return false;
  }
  u64 h = hash(name, table->element_count);
  kcopy_memory(out_value,
               (void *) ((u8 *) table->memory + (table->element_size * h)),
               table->element_size);
  return true;
}

b8 hash_table_get_ptr(hash_table *table,
                      const char *name,
                      void **out_value) {
  if (!table || !name || !out_value) {
    KERROR("hash_table_get_ptr :: `table`, `name` and `out_value` are required");
    return false;
  }
  if (!table->is_pointer_type) {
    KERROR("`hash_table_get_ptr` called without a pointer-type hash table (use `hash_table_get` instead)");
    return false;
  }
  u64 h = hash(name, table->element_count);
  *out_value = ((void **) table->memory)[h];
  return *out_value != 0;
}

b8 hash_table_fill(hash_table *table,
                   void *value) {
  if (!table || !value) {
    KERROR("hash_table_fill :: `table` and `value` are required");
    return false;
  }
  if (table->is_pointer_type) {
    KERROR("`hash_table_fill` is not compatible with a pointer-type hash table");
    return false;
  }
  for (u32 i = 0; i < table->element_count; ++i) {
    kcopy_memory((void *) ((u8 *) table->memory + (table->element_size * i)),
                 value,
                 table->element_size);
  }
  return true;
}
