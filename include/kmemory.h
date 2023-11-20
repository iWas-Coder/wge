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

#define MEM_USE_PRINT_BUF_SIZE 8000
#define MEM_PRINT_UNIT_SYM_SIZE 3 + 1  // 3 chars + '\0'
#define MEM_B_IN_KIB (1 << 10)  // 2^10 B
#define MEM_B_IN_MIB (1 << 20)  // 2^20 B
#define MEM_B_IN_GIB (1 << 30)  // 2^30 B

typedef enum {
  MEMORY_TAG_UNKNOWN,
  MEMORY_TAG_ARRAY,
  MEMORY_TAG_LINEAR_ALLOCATOR,
  MEMORY_TAG_DARRAY,
  MEMORY_TAG_DICT,
  MEMORY_TAG_RING_QUEUE,
  MEMORY_TAG_BST,
  MEMORY_TAG_STRING,
  MEMORY_TAG_APPLICATION,
  MEMORY_TAG_JOB,
  MEMORY_TAG_TEXTURE,
  MEMORY_TAG_MATERIAL_INSTANCE,
  MEMORY_TAG_RENDERER,
  MEMORY_TAG_GAME,
  MEMORY_TAG_TRANSFORM,
  MEMORY_TAG_ENTITY,
  MEMORY_TAG_ENTITY_NODE,
  MEMORY_TAG_SCENE,
  // Tag that marks the end of enum
  MEMORY_TAG_MAX_TAGS
} memory_tag;

KAPI void memory_system_initialize(u64 *memory_requirements, void *state);
KAPI void memory_system_shutdown(void *state);

KAPI void *kallocate(u64 size, memory_tag tag);
KAPI void kfree(void *block, u64 size, memory_tag tag);
KAPI void *kzero_memory(void *block, u64 size);
KAPI void *kcopy_memory(void *dest, const void *source, u64 size);
KAPI void *kset_memory(void *dest, i32 value, u64 size);

KAPI char *get_memory_usage_str(void);

KAPI u64 get_memory_alloc_count(void);
