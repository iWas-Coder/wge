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

typedef union {
  f32 elements[2];
  struct {
    union { f32 x, r, s, u; };
    union { f32 y, g, t, v; };
  };
} Vector2;

typedef struct {
  union {
    f32 elements[3];
    struct{
      union { f32 x, r, s, u; };
      union { f32 y, g, t, v; };
      union { f32 z, b, p, w; };
    };
  };
} Vector3;

typedef union {
#if defined(KUSE_SIMD)
  alignas(16) __m128 data;
#endif
  f32 elements[4];
  union {
    struct {
      union { f32 x, r, s; };
      union { f32 y, g, t; };
      union { f32 z, b, p; };
      union { f32 w, a, q; };
    };
  };
} Vector4;

typedef Vector4 Quaternion;

typedef union {
  f32 data[16];
#if defined(KUSE_SIMD)
  alignas(16) Vector4 rows[4];
#endif
} Matrix4;

typedef struct {
  Vector3 position;
} vertex_3d;
