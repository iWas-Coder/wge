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
#include <kstring.h>
#include <kmemory.h>
#include <hash_table.h>
#include <texture_system.h>
#include <renderer_frontend.h>

// TODO: move to resource system
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define FILE_PATH_SIZE 512

typedef struct {
  texture_system_config config;
  texture fallback_texture;
  texture *registered_textures;
  hash_table registered_texture_table;
} texture_system_state;

typedef struct {
  u64 ref_count;
  u32 handle;
  b8 auto_release;
} texture_ref;

static texture_system_state *state_ptr = 0;

b8 create_fallback_textures(texture_system_state *state) {
  // Fallback texture creation (256x256 blue+white checkerboard pattern)
  const i32 texture_size = 256;
  const i32 channels = 4;
  const i32 pixel_count = texture_size * texture_size;
  u8 *pixels = kallocate(sizeof(u8) * pixel_count * channels, MEMORY_TAG_TEXTURE);
  // u8 pixels[pixel_count * channels];
  // set all pixels to white
  kset_memory(pixels, texture_size - 1, sizeof(u8) * pixel_count * channels);
  for (u64 i = 0; i < texture_size; ++i) {
    for (u64 j = 0; j < texture_size; ++j) {
      u64 idx = (i * texture_size) + j;
      u64 idx_bpp = idx * channels;
      // set alternating pixels to blue (setting red/green channels to 0)
      if (i % 2 && j % 2) {
        pixels[idx_bpp]     = 0;
        pixels[idx_bpp + 1] = 0;
      }
      else if (!(j % 2)) {
        pixels[idx_bpp]     = 0;
        pixels[idx_bpp + 1] = 0;
      }
    }
  }
  renderer_create_texture(FALLBACK_TEXTURE_NAME,
                          texture_size,
                          texture_size,
                          channels,
                          pixels,
                          false,
                          &state->fallback_texture);

  state->fallback_texture.generation = INVALID_ID;
  KTRACE("Fallback texture created");
  return true;
}

void destroy_fallback_textures(texture_system_state *state) {
  if (state) renderer_destroy_texture(&state->fallback_texture);
}

// TODO: move to resource system
b8 load_texture(const char *name, texture *t) {
  const i32 required_channel_count = 4;
  char *format_str = "assets/textures/%s.%s";
  stbi_set_flip_vertically_on_load(true);
  char full_file_path[FILE_PATH_SIZE];

  kstrfmt(full_file_path, format_str, name, "png");

  texture t_tmp;
  u8 *data = stbi_load(full_file_path,
                       (i32 *) &t_tmp.width,
                       (i32 *) &t_tmp.height,
                       (i32 *) &t_tmp.channel_count,
                       required_channel_count);
  t_tmp.channel_count = required_channel_count;

  if (!data) {
    if (stbi_failure_reason()) KWARN("load_texture :: failed to load file '%s': %s",
                                     full_file_path,
                                     stbi_failure_reason());
    return false;
  }

  u32 current_gen = t->generation;
  t->generation = INVALID_ID;
  u64 total_size = t_tmp.width * t_tmp.height * t_tmp.channel_count;
  b32 has_transparency = false;
  for (u64 i = 0; i < total_size; i += required_channel_count) {
    if (data[i + 3] < 255) {
      has_transparency = true;
      break;
    }
  }
  if (stbi_failure_reason()) KWARN("load_texture :: failed to load file '%s': %s",
                                   full_file_path,
                                   stbi_failure_reason());

  // Create texture
  renderer_create_texture(name,
                          t_tmp.width,
                          t_tmp.height,
                          t_tmp.channel_count,
                          data,
                          has_transparency,
                          &t_tmp);
  texture old = *t;
  *t = t_tmp;
  renderer_destroy_texture(&old);
  if (current_gen == INVALID_ID) t->generation = 0;
  else t->generation = current_gen + 1;

  stbi_image_free(data);
  return true;
}

b8 texture_system_initialize(u64 *memory_requirements,
                             void *state,
                             texture_system_config config) {
  if (config.max_texture_count == 0) {
    KFATAL("texture_system_initialize :: `max_texture_count` must be > 0");
    return false;
  }
  u64 struct_requirements = sizeof(texture_system_state);
  u64 array_requirements = sizeof(texture) * config.max_texture_count;
  u64 hash_table_requirements = sizeof(texture_ref) * config.max_texture_count;
  *memory_requirements = struct_requirements + array_requirements + hash_table_requirements;
  if (!state) return true;

  state_ptr = state;
  state_ptr->config = config;
  void *array_block = (void *) ((u64 *) state + struct_requirements);
  state_ptr->registered_textures = array_block;
  void *hash_table_block = (void *) ((u64 *) array_block + array_requirements);
  hash_table_create(sizeof(texture_ref),
                    config.max_texture_count,
                    hash_table_block,
                    false,
                    &state_ptr->registered_texture_table);

  texture_ref invalid_ref = {
    .auto_release = false,
    .handle = INVALID_ID,
    .ref_count = 0
  };
  hash_table_fill(&state_ptr->registered_texture_table, &invalid_ref);
  for (u32 i = 0; i < state_ptr->config.max_texture_count; ++i) {
    state_ptr->registered_textures[i].id = INVALID_ID;
    state_ptr->registered_textures[i].generation = INVALID_ID;
  }

  create_fallback_textures(state_ptr);
  return true;
}

void texture_system_shutdown(void *state) {
  (void) state;  // Unused parameter

  if (!state_ptr) return;
  for (u32 i = 0; i < state_ptr->config.max_texture_count; ++i) {
    texture *t = &state_ptr->registered_textures[i];
    if (t->generation != INVALID_ID) renderer_destroy_texture(t);
  }
  destroy_fallback_textures(state_ptr);
  state_ptr = 0;
}

texture *texture_system_get(const char *name, b8 auto_release) {
  if (kstrcmpi(name, FALLBACK_TEXTURE_NAME)) {
    KWARN("texture_system_get :: called for fallback texture (use `texture_system_get_fallback` instead)");
    return &state_ptr->fallback_texture;
  }

  texture_ref ref;
  if (!state_ptr || !hash_table_get(&state_ptr->registered_texture_table,
                                    name,
                                    &ref)) {
    KERROR("texture_system_get :: failed to get texture '%s'", name);
    return 0;
  }

  if (!ref.ref_count) ref.auto_release = auto_release;
  ++ref.ref_count;
  if (ref.handle == INVALID_ID) {
    u32 count = state_ptr->config.max_texture_count;
    texture *t = 0;
    for (u32 i = 0; i < count; ++i) {
      if (state_ptr->registered_textures[i].id == INVALID_ID) {
        ref.handle = i;
        t = &state_ptr->registered_textures[i];
        break;
      }
    }
    if (!t || ref.handle == INVALID_ID) {
      KFATAL("texture_system_get :: texture system is full (adjust config to allow more textures)");
      return 0;
    }
    if (!load_texture(name, t)) {
      KERROR("texture_system_get :: failed to load texture '%s'", name);
      return 0;
    }

    t->id = ref.handle;
    KTRACE("Texture '%s' created (does not exists yet). `ref_count` -> %i",
           name,
           ref.ref_count);
  }
  else KTRACE("Texture '%s' already exists. `ref_count` -> %i",
              name,
              ref.ref_count);
  // Update entry
  hash_table_set(&state_ptr->registered_texture_table, name, &ref);
  return &state_ptr->registered_textures[ref.handle];
}

texture *texture_system_get_fallback(void) {
  if (state_ptr) return &state_ptr->fallback_texture;
  KERROR("texture_system_get_fallback :: called before texture system init");
  return 0;
}

void texture_system_release(const char *name) {
  if (kstrcmpi(name, FALLBACK_TEXTURE_NAME)) return;

  texture_ref ref;
  if (!state_ptr || !hash_table_get(&state_ptr->registered_texture_table,
                                    name,
                                    &ref)) {
    KERROR("texture_system_release :: failed to release texture '%s'", name);
    return;
  }

  if (!ref.ref_count) {
    KWARN("Tried to release non-existant texture ('%s')", name);
    return;
  }
  --ref.ref_count;
  if (!ref.ref_count && ref.auto_release) {
    texture *t = &state_ptr->registered_textures[ref.handle];
    renderer_destroy_texture(t);
    kzero_memory(t, sizeof(texture));
    t->id = INVALID_ID;
    t->generation = INVALID_ID;
    ref.handle = INVALID_ID;
    ref.auto_release = false;
    KTRACE("Texture '%s' released (`ref_count` -> 0 && `auto_release` -> true)", name);
  }
  else KTRACE("Texture '%s' released (`ref_count` -> %i && `auto_release` -> %s)",
              name,
              ref.ref_count,
              ref.auto_release ? "true" : "false");
  // Update entry
  hash_table_set(&state_ptr->registered_texture_table, name, &ref);
}
