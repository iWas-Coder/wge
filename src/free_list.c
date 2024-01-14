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
#include <free_list.h>

#define FREE_LIST_MIN_MEM_TO_USE ((sizeof(internal_state) + sizeof(free_list_node)) * sizeof(void *))

typedef struct free_list_node {
  u32 offset;
  u32 size;
  struct free_list_node *next;
} free_list_node;

typedef struct {
  u32 total_size;
  u32 max_entries;
  free_list_node *head;
  free_list_node *nodes;
} internal_state;

free_list_node *get_node(free_list *list) {
  internal_state *state = list->memory;
  for (u32 i = 1; i < state->max_entries; ++i) {
    if (state->nodes[i].offset == INVALID_ID) return &state->nodes[i];
  }
  return 0;
}

void return_node(free_list *list, free_list_node *node) {
  (void) list;  // Unused parameter
  node->offset = INVALID_ID;
  node->size = INVALID_ID;
  node->next = 0;
}

void free_list_create(u32 total_size,
                      u64 *memory_requirements,
                      void *memory,
                      free_list *out_list) {
  u32 max_entries = (total_size / sizeof(void *));
  *memory_requirements = sizeof(internal_state) + (max_entries * sizeof(free_list_node));
  if (!memory) return;

  if (total_size < FREE_LIST_MIN_MEM_TO_USE) {
    KWARN("free_list_create :: Inefficient usage due to `total_size` being smaller than FREE_LIST_MIN_MEM_TO_USE (%i B)", FREE_LIST_MIN_MEM_TO_USE);
  }

  out_list->memory = memory;
  kzero_memory(out_list->memory, *memory_requirements);

  internal_state *state = out_list->memory;
  state->total_size = total_size;
  state->max_entries = max_entries;
  state->nodes = (void *) ((u8 *) out_list->memory + sizeof(internal_state));
  state->head = state->nodes;
  state->head->offset = 0;
  state->head->next = 0;
  state->head->size = total_size;

  for (u32 i = 1; i < state->max_entries; ++i) {
    state->nodes[i].offset = INVALID_ID;
    state->nodes[i].size = INVALID_ID;
  }
}

void free_list_destroy(free_list *list) {
  if (!list || !list->memory) return;
  kzero_memory(list->memory,
               sizeof(internal_state) + (((internal_state *) list->memory)->max_entries * sizeof(free_list_node)));
  list->memory = 0;
}

b8 free_list_alloc(free_list *list, u32 size, u32 *out_offset) {
  if (!list || !list->memory || !out_offset) return false;

  internal_state *state = list->memory;
  free_list_node *node = state->head;
  free_list_node *prev = 0;
  while (node) {
    if (node->size == size) {
      // Return the node (exact match)
      *out_offset = node->offset;
      free_list_node *n = 0;
      if (prev) {
        prev->next = node->next;
        n = node;
      }
      else {
        // Reassign head and return the previous head node (node is the head)
        n = state->head;
        state->head = node->next;
      }
      return_node(list, n);
      return true;
    }
    else if (node->size > size) {
      // Node is larger (move the offset)
      *out_offset = node->offset;
      node->size -= size;
      node->offset += size;
      return true;
    }
    prev = node;
    node = node->next;
  }

  KWARN("free_list_alloc :: no block found with enough free space (requested: %u B, available: %llu B)",
        size,
        free_list_get_free_space(list));
  return false;
}

b8 free_list_free(free_list *list, u32 size, u32 offset) {
  if (!list || !list->memory || !size) return false;

  internal_state *state = list->memory;
  free_list_node *node = state->head;
  free_list_node *prev = 0;
  while (node) {
    if (node->offset == offset) {
      // Can be joined with this node
      node->size += size;
      if (node->next &&
          node->next->offset == node->offset + node->size) {
        node->size += node->next->size;
        free_list_node *n = node->next;
        node->next = node->next->next;
        return_node(list, n);
      }
      return true;
    }
    else if (node->offset > offset) {
      // Need a new node
      free_list_node *n = get_node(list);
      if (!n) {
        KWARN("free_list_free :: early return due to `get_node` returning NULL");
        return false;
      }
      n->offset = offset;
      n->size = size;
      if (prev) {
        prev->next = n;
        n->next = node;
      }
      else {
        n->next = node;
        state->head = n;
      }
      // Check next node if it could be merged
      if (n->next &&
          n->offset + n->size == n->next->offset) {
        n->size += n->next->size;
        free_list_node *merged = n->next;
        n->next = merged->next;
        return_node(list, merged);
      }
      if (prev &&
          prev->offset + prev->size == n->offset) {
        prev->size += n->size;
        free_list_node *merged = n;
        prev->next = merged->next;
        return_node(list, merged);
      }
      return true;
    }
    prev = node;
    node = node->next;
  }

  KWARN("free_list_free :: unable to free block (possible data corruption)");
  return false;
}

void free_list_clear(free_list *list) {
  if (!list || !list->memory) return;
  internal_state *state = list->memory;
  for (u32 i = 1; i < state->max_entries; ++i) {
    state->nodes[i].offset = INVALID_ID;
    state->nodes[i].size = INVALID_ID;
  }
  state->head->offset = 0;
  state->head->next = 0;
  state->head->size = state->total_size;
}

u64 free_list_get_free_space(free_list *list) {
  if (!list || !list->memory) return 0;
  u64 total = 0;
  free_list_node *node = ((internal_state *) list->memory)->head;
  while (node) {
    total += node->size;
    node = node->next;
  }
  return total;
}
