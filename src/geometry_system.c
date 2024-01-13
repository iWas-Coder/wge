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
#include <geometry_system.h>
#include <material_system.h>
#include <renderer_frontend.h>

typedef struct {
  u64 ref_count;
  geometry geometry;
  b8 auto_release;
} geometry_ref;

typedef struct {
  geometry_system_config config;
  geometry fallback_geometry;
  geometry fallback_2D_geometry;
  geometry_ref *registered_geometries;
} geometry_system_state;

static geometry_system_state *state_ptr = 0;

b8 create_fallback_geometries(geometry_system_state *state) {
  const f32 factor = 10.0f;

  vertex_3d vertex_list[] = {
    {
      .position.x = -0.5f * factor,
      .position.y = -0.5f * factor,
      .texcoord.x = 0.0f,
      .texcoord.y = 0.0f
    },
    {
      .position.x = 0.5f * factor,
      .position.y = 0.5f * factor,
      .texcoord.x = 1.0f,
      .texcoord.y = 1.0f
    },
    {
      .position.x = -0.5f * factor,
      .position.y = 0.5f * factor,
      .texcoord.x = 0.0f,
      .texcoord.y = 1.0f
    },
    {
      .position.x = 0.5f * factor,
      .position.y = -0.5f * factor,
      .texcoord.x = 1.0f,
      .texcoord.y = 0.0f
    }};
  u32 index_list[] = {0, 1, 2, 0, 3, 1};

  if (!renderer_create_geometry(&state->fallback_geometry,
                                sizeof(vertex_3d),
                                4,
                                vertex_list,
                                sizeof(u32),
                                6,
                                index_list)) {
    KFATAL("create_fallback_geometry :: failed to create fallback geometry");
    return false;
  }
  state->fallback_geometry.material = material_system_get_fallback();

  vertex_2d vertex_2d_list[] = {
    {
      .position.x = -0.5f * factor,
      .position.y = -0.5f * factor,
      .texcoord.x = 0.0f,
      .texcoord.y = 0.0f
    },
    {
      .position.x = 0.5f * factor,
      .position.y = 0.5f * factor,
      .texcoord.x = 1.0f,
      .texcoord.y = 1.0f
    },
    {
      .position.x = -0.5f * factor,
      .position.y = 0.5f * factor,
      .texcoord.x = 0.0f,
      .texcoord.y = 1.0f
    },
    {
      .position.x = 0.5f * factor,
      .position.y = -0.5f * factor,
      .texcoord.x = 1.0f,
      .texcoord.y = 0.0f
    }
  };
  u32 index_2d_list[] = {2, 1, 0, 3, 0, 1};

  if (!renderer_create_geometry(&state->fallback_2D_geometry,
                                sizeof(vertex_2d),
                                4,
                                vertex_2d_list,
                                sizeof(u32),
                                6,
                                index_2d_list)) {
    KFATAL("create_fallback_geometry :: failed to create fallback 2D geometry");
    return false;
  }
  state->fallback_2D_geometry.material = material_system_get_fallback();

  return true;
}

b8 create_geometry(geometry_system_state *state, geometry_config cfg, geometry *g) {
  if (!renderer_create_geometry(g,
                                cfg.vertex_size,
                                cfg.vertex_count,
                                cfg.vertices,
                                cfg.index_size,
                                cfg.index_count,
                                cfg.indices)) {
    state->registered_geometries[g->id].ref_count = 0;
    state->registered_geometries[g->id].auto_release = false;
    g->id = INVALID_ID;
    g->generation = INVALID_ID;
    g->internal_id = INVALID_ID;
    return false;
  }
  if (kstrlen(cfg.material_name)) {
    g->material = material_system_get(cfg.material_name);
    if (!g->material) g->material = material_system_get_fallback();
  }
  return true;
}

void destroy_geometry(geometry_system_state *state, geometry *g) {
  (void) state;  // Unused parameter
  
  renderer_destroy_geometry(g);
  g->internal_id = INVALID_ID;
  g->generation = INVALID_ID;
  g->id = INVALID_ID;
  kstrrm(g->name);
  if (g->material && kstrlen(g->material->name)) {
    material_system_release(g->material->name);
    g->material = 0;
  }
}

b8 geometry_system_initialize(u64 *memory_requirements,
                              void *state,
                              geometry_system_config config) {
  if (!config.max_geometry_count) {
    KFATAL("geometry_system_initialize :: `max_geometry_count` must be > 0");
    return false;
  }
  u64 struct_requirements = sizeof(geometry_system_state);
  u64 array_requirements = sizeof(geometry_ref) * config.max_geometry_count;
  *memory_requirements = struct_requirements + array_requirements;
  if (!state) return true;

  state_ptr = state;
  state_ptr->config = config;
  void *array_block = (void *) ((u64 *) state + struct_requirements);
  state_ptr->registered_geometries = array_block;

  for (u32 i = 0; i < state_ptr->config.max_geometry_count; ++i) {
    state_ptr->registered_geometries[i].geometry.id =  INVALID_ID;
    state_ptr->registered_geometries[i].geometry.generation = INVALID_ID;
    state_ptr->registered_geometries[i].geometry.internal_id = INVALID_ID;
  }

  if (!create_fallback_geometries(state_ptr)) {
    KFATAL("geometry_system_initialize :: Fallback geometries creation failed");
    return false;
  }
  return true;
}

void geometry_system_shutdown(void *state) {
  (void) state;  // Unused parameter
  // nothing to do here :D
}

geometry *geometry_system_get(u32 id) {
  if (id != INVALID_ID && state_ptr->registered_geometries[id].geometry.id != INVALID_ID) {
    ++state_ptr->registered_geometries[id].ref_count;
    return &state_ptr->registered_geometries[id].geometry;
  }
  KERROR("geometry_system_get :: invalid geometry id");
  return 0;
}

geometry *geometry_system_get_from_cfg(geometry_config cfg, b8 auto_release) {
  geometry *g = 0;
  for (u32 i = 0; i < state_ptr->config.max_geometry_count; ++i) {
    if (state_ptr->registered_geometries[i].geometry.id != INVALID_ID) continue;
    state_ptr->registered_geometries[i].auto_release = auto_release;
    state_ptr->registered_geometries[i].ref_count = 1;
    g = &state_ptr->registered_geometries[i].geometry;
    g->id = i;
    break;
  }
  if (!g) {
    KERROR("geometry_system_get_from_cfg :: geometry system is full (adjust config to allow more geometries)");
    return 0;
  }
  if (!create_geometry(state_ptr, cfg, g)) {
    KERROR("geometry_system_get_from_cfg :: failed to create geometry");
    return 0;
  }
  return g;
}

geometry *geometry_system_get_fallback(void) {
  if (state_ptr) return &state_ptr->fallback_geometry;
  KFATAL("geometry_system_get_fallback :: called before geometry system init");
  return 0;
}

geometry *geometry_system_get_fallback_2D(void) {
  if (state_ptr) return &state_ptr->fallback_2D_geometry;
  KFATAL("geometry_system_get_fallback_2D :: called before geometry system init");
  return 0;
}

void geometry_system_release(geometry *geometry) {
  if (!geometry || geometry->id == INVALID_ID) {
    KWARN("geometry_system_release :: invalid geometry ID (nothing was done)");
    return;
  }
  geometry_ref *ref = &state_ptr->registered_geometries[geometry->id];
  if (ref->geometry.id != geometry->id) {
    KFATAL("geometry_system_release :: geometry ID mismatch");
    return;
  }
  if (ref->ref_count) --ref->ref_count;
  if (!ref->ref_count && ref->auto_release) {
    destroy_geometry(state_ptr, &ref->geometry);
    ref->ref_count = 0;
    ref->auto_release = false;
  }
}

geometry_config geometry_system_gen_plane_cfg(f32 width,
                                              f32 height,
                                              u32 x_segment_count,
                                              u32 y_segment_count,
                                              f32 tile_x,
                                              f32 tile_y,
                                              const char *name,
                                              const char *material_name) {
  if (!width) {
    KWARN("geometry_system_gen_plane_cfg :: `width` must be != 0 (auto set to 1)");
    width = 1.0f;
  }
  if (!height) {
    KWARN("geometry_system_gen_plane_cfg :: `height` must be != 0 (auto set to 1)");
    height = 1.0f;
  }
  if (!x_segment_count) {
    KWARN("geometry_system_gen_plane_cfg :: `x_segment_count` must be > 0 (auto set to 1)");
    x_segment_count = 1;
  }
  if (!y_segment_count) {
    KWARN("geometry_system_gen_plane_cfg :: `y_segment_count` must be > 0 (auto set to 1)");
    y_segment_count = 1;
  }
  if (!tile_x) {
    KWARN("geometry_system_gen_plane_cfg :: `tile_x` must be != 0 (auto set to 1)");
    tile_x = 1.0f;
  }
  if (!tile_y) {
    KWARN("geometry_system_gen_plane_cfg :: `tile_y` must be != 0 (auto set to 1)");
    tile_y = 1.0f;
  }

  geometry_config cfg = {
    .vertex_size = sizeof(vertex_3d),
    .vertex_count = x_segment_count * y_segment_count * 4,
    .vertices = kallocate(sizeof(vertex_3d) * cfg.vertex_count, MEMORY_TAG_ARRAY),
    .index_size = sizeof(u32),
    .index_count = x_segment_count * y_segment_count * 6,
    .indices = kallocate(sizeof(u32) * cfg.index_count, MEMORY_TAG_ARRAY)
  };

  f32 seg_width = width / x_segment_count;
  f32 seg_height = height / y_segment_count;
  f32 half_width = width / 2.0f;
  f32 half_height = height / 2.0f;
  for (u32 y = 0; y < y_segment_count; ++y) {
    for (u32 x = 0; x < x_segment_count; ++x) {
      f32 min_x = (x * seg_width) - half_width;
      f32 min_y = (y * seg_height) - half_height;
      f32 max_x = min_x + seg_width;
      f32 max_y = min_y + seg_height;
      f32 min_uvx = (x / (f32) x_segment_count) * tile_x;
      f32 min_uvy = (y / (f32) y_segment_count) * tile_y;
      f32 max_uvx = ((x + 1) / (f32) x_segment_count) * tile_x;
      f32 max_uvy = ((y + 1) / (f32) y_segment_count) * tile_y;

      u32 v_offset = ((y * x_segment_count) + x) * 4;
      vertex_3d *v0 = &((vertex_3d *) cfg.vertices)[v_offset];
      vertex_3d *v1 = &((vertex_3d *) cfg.vertices)[v_offset + 1];
      vertex_3d *v2 = &((vertex_3d *) cfg.vertices)[v_offset + 2];
      vertex_3d *v3 = &((vertex_3d *) cfg.vertices)[v_offset + 3];

      v0->position.x = min_x;
      v0->position.y = min_y;
      v0->texcoord.x = min_uvx;
      v0->texcoord.y = min_uvy;
      
      v1->position.x = max_x;
      v1->position.y = max_y;
      v1->texcoord.x = max_uvx;
      v1->texcoord.y = max_uvy;

      v2->position.x = min_x;
      v2->position.y = max_y;
      v2->texcoord.x = min_uvx;
      v2->texcoord.y = max_uvy;

      v3->position.x = max_x;
      v3->position.y = min_y;
      v3->texcoord.x = max_uvx;
      v3->texcoord.y = min_uvy;

      u32 i_offset = ((y * x_segment_count) + x) * 6;
      ((u32 *) cfg.indices)[i_offset]     = v_offset;
      ((u32 *) cfg.indices)[i_offset + 1] = v_offset + 1;
      ((u32 *) cfg.indices)[i_offset + 2] = v_offset + 2;
      ((u32 *) cfg.indices)[i_offset + 3] = v_offset;
      ((u32 *) cfg.indices)[i_offset + 4] = v_offset + 3;
      ((u32 *) cfg.indices)[i_offset + 5] = v_offset + 1;
    }
  }

  // Geometry name
  if (name && kstrlen(name)) kstrncp(cfg.name, name, GEOMETRY_NAME_MAX_LEN);
  else kstrncp(cfg.name, FALLBACK_GEOMETRY_NAME, GEOMETRY_NAME_MAX_LEN);

  // Material name
  if (material_name && kstrlen(material_name)) kstrncp(cfg.material_name,
                                                       material_name,
                                                       MATERIAL_NAME_MAX_LEN);
  else kstrncp(cfg.material_name, FALLBACK_MATERIAL_NAME, MATERIAL_NAME_MAX_LEN);

  return cfg;
}
