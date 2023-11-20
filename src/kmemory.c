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


#include <stdio.h>
#include <kstring.h>
#include <logger.h>
#include <kmemory.h>
#include <platform.h>

typedef struct {
  u64 total_allocated;
  u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
} memory_stats;

typedef struct {
  memory_stats stats;
  u64 alloc_count;
} memory_system_state;

static memory_system_state *state_ptr;

static const char *memory_tag_strings[] = {
  "UNKNOWN          ",
  "ARRAY            ",
  "LINEAR_ALLOCATOR ",
  "DARRAY           ",
  "DICT             ",
  "RING_QUEUE       ",
  "BST              ",
  "STRING           ",
  "APPLICATION      ",
  "JOB              ",
  "TEXTURE          ",
  "MATERIAL_INSTANCE",
  "RENDERER         ",
  "GAME             ",
  "TRANSFORM        ",
  "ENTITY           ",
  "ENTITY_NODE      ",
  "SCENE            "
};

void memory_system_initialize(u64 *memory_requirements, void *state) {
  *memory_requirements = sizeof(memory_system_state);
  if (!state) return;
  state_ptr = state;
  state_ptr->alloc_count = 0;

  platform_zero_memory(&state_ptr->stats, sizeof(state_ptr->stats));
}

void memory_system_shutdown(void *state) {
  (void) state;  // Unused parameter

  state_ptr = 0;
}

void *kallocate(u64 size, memory_tag tag) {
  if (tag == MEMORY_TAG_UNKNOWN) {
    KWARN("kallocate :: used `MEMORY_TAG_UNKNOWN`, must be re-classified");
  }
  if (state_ptr) {
    state_ptr->stats.total_allocated += size;
    state_ptr->stats.tagged_allocations[tag] += size;
    ++state_ptr->alloc_count;
  }

  // TODO: Memory alignment
  void *block = platform_allocate(size, false);
  platform_zero_memory(block, size);
  return block;
}

void kfree(void *block, u64 size, memory_tag tag) {
  if (tag == MEMORY_TAG_UNKNOWN) {
    KWARN("kfree :: used `MEMORY_TAG_UNKNOWN`, must be re-classified");
  }
  if (state_ptr) {
    state_ptr->stats.total_allocated -= size;
    state_ptr->stats.tagged_allocations[tag] -= size;
  }

  // TODO: Memory alignment
  platform_free(block, false);
}

void *kzero_memory(void *block, u64 size) {
  return platform_zero_memory(block, size);
}

void *kcopy_memory(void *dest, const void *source, u64 size) {
  return platform_copy_memory(dest, source, size);
}

void *kset_memory(void *dest, i32 value, u64 size) {
  return platform_set_memory(dest, value, size);
}

char *get_memory_usage_str(void) {
  char buf[MEM_USE_PRINT_BUF_SIZE] = "USED_MEM (tagged):\n";
  u64 offset = kstrlen(buf);

  for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
    float amount;
    char unit[MEM_PRINT_UNIT_SYM_SIZE] = "XiB";

    if (state_ptr->stats.tagged_allocations[i] >= MEM_B_IN_GIB) {
      unit[0] = 'G';
      amount = state_ptr->stats.tagged_allocations[i] / (float) MEM_B_IN_GIB;
    }
    else if (state_ptr->stats.tagged_allocations[i] >= MEM_B_IN_MIB) {
      unit[0] = 'M';
      amount = state_ptr->stats.tagged_allocations[i] / (float) MEM_B_IN_MIB;
    }
    else if (state_ptr->stats.tagged_allocations[i] >= MEM_B_IN_KIB) {
      unit[0] = 'K';
      amount = state_ptr->stats.tagged_allocations[i] / (float) MEM_B_IN_KIB;
    }
    else {
      unit[0] = 'B';
      unit[1] = 0;
      amount = (float) state_ptr->stats.tagged_allocations[i];
    }

    offset += snprintf(buf + offset,
                       MEM_USE_PRINT_BUF_SIZE,
                       "  %s: %.2f %s\n",
                       memory_tag_strings[i],
                       amount,
                       unit);
  }
  return kstrdup(buf);
}

u64 get_memory_alloc_count(void) {
  if (state_ptr) return state_ptr->alloc_count;
  return 0;
}
