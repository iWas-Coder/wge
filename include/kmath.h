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
#include <kmemory.h>
#include <math_types.h>

#define K_PI      3.14159265358979323846f  // π
#define K_2PI     2.0f * K_PI              // 2π
#define K_PI_2    0.5f * K_PI              // π/2
#define K_PI_4    0.25f * K_PI             // π/4
#define K_1_PI    1.0f / K_PI              // 1/π
#define K_1_2PI   1.0f / K_2PI             // 1/2π
#define K_SQRT2   1.41421356237309504880f  // √2
#define K_1_SQRT2 1.0f / K_SQRT2           // 1/√2
#define K_SQRT3   1.73205080756887729352f  // √3
#define K_1_SQRT3 1.0f / K_SQRT3           // 1/√3

#define K_DEG2RAD K_PI / 180.0f            // π/180
#define K_RAD2DEG 180.0f / K_PI            // 180/π

#define K_INF 1e30f
#define K_FLOAT_EPSILON 1.192092896e-7f

#define TIME_MS_IN_S 1e3f
#define TIME_S_IN_MS 1e-3f

KINLINE b8 is_power_of_2(u64 value) {
  return (value != 0) && ((value & (value - 1)) == 0);
}

KINLINE f32 deg_to_rad(f32 deg) {
  return deg * K_DEG2RAD;
}

KINLINE f32 rad_to_deg(f32 rad) {
  return rad * K_RAD2DEG;
}

KAPI f32 kabs(f32 x);

KAPI f32 ksqrt(f32 x);

KAPI f32 ksin(f32 x);

KAPI f32 kcos(f32 x);

KAPI f32 ktan(f32 x);

KAPI f32 karccos(f32 x);

KAPI i32 krandom(void);

KAPI f32 krandom_f(void);

KAPI i32 krandom_range(i32 min, i32 max);

KAPI f32 krandom_range_f(f32 min, f32 max);

KINLINE Vector2 vec2_create(f32 x, f32 y) {
  return (Vector2) {{ x, y }};
}

KINLINE Vector2 vec2_zero(void) {
  return (Vector2) {{ 0.0f, 0.0f }};
}

KINLINE Vector2 vec2_right(void) {
  return (Vector2) {{ 1.0f, 0.0f }};
}

KINLINE Vector2 vec2_up(void) {
  return (Vector2) {{ 0.0f, 1.0f }};
}

KINLINE Vector2 vec2_left(void) {
  return (Vector2) {{ -1.0f, 0.0f }};
}

KINLINE Vector2 vec2_down(void) {
  return (Vector2) {{ 0.0f, -1.0f }};
}

KINLINE Vector2 vec2_one(void) {
  return (Vector2) {{ 1.0f, 1.0f }};
}

KINLINE Vector2 vec2_add(Vector2 u, Vector2 v) {
  return (Vector2) {{ u.x + v.x, u.y + v.y }};
}

KINLINE Vector2 vec2_sub(Vector2 u, Vector2 v) {
  return (Vector2) {{ u.x - v.x, u.y - v.y }};
}

KINLINE Vector2 vec2_mult(Vector2 u, Vector2 v) {
  return (Vector2) {{ u.x * v.x, u.y * v.y }};
}

KINLINE Vector2 vec2_mult_scalar(Vector2 v, f32 k) {
  return (Vector2) {{ v.x * k, v.y * k }};
}

KINLINE f32 vec2_dot(Vector2 u, Vector2 v) {
  return (u.x * v.x) + (u.y * v.y);
}

KINLINE Vector2 vec2_div(Vector2 u, Vector2 v) {
  return (Vector2) {{ u.x / v.x, u.y / v.y }};
}

KINLINE f32 vec2_squared_len(Vector2 v) {
  return (v.x * v.x) + (v.y * v.y);
}

KINLINE f32 vec2_len(Vector2 v) {
  return ksqrt(vec2_squared_len(v));
}

KINLINE void vec2_normalize_set(Vector2 *v) {
  const f32 len = vec2_len(*v);
  v->x /= len;
  v->y /= len;
}

KINLINE Vector2 vec2_normalize_get(Vector2 v) {
  vec2_normalize_set(&v);
  return v;
}

KINLINE b8 vec2_cmp(Vector2 u, Vector2 v, f32 epsilon) {
  if (kabs(u.x - v.x) > epsilon) return false;
  if (kabs(u.y - v.y) > epsilon) return false;
  return true;
}

KINLINE f32 vec2_distance(Vector2 u, Vector2 v) {
  return vec2_len(vec2_sub(u, v));
}

KINLINE Vector3 vec3_create(f32 x, f32 y, f32 z) {
  return (Vector3) {{{ x, y, z }}};
}

KINLINE Vector3 vec3_zero(void) {
  return (Vector3) {{{ 0.0f, 0.0f, 0.0f }}};
}

KINLINE Vector3 vec3_right(void) {
  return (Vector3) {{{ 1.0f, 0.0f, 0.0f }}};
}

KINLINE Vector3 vec3_up(void) {
  return (Vector3) {{{ 0.0f, 1.0f, 0.0f }}};
}

KINLINE Vector3 vec3_backward(void) {
  return (Vector3) {{{ 0.0f, 0.0f, 1.0f }}};
}

KINLINE Vector3 vec3_left(void) {
  return (Vector3) {{{ -1.0f, 0.0f, 0.0f }}};
}

KINLINE Vector3 vec3_down(void) {
  return (Vector3) {{{ 0.0f, -1.0f, 0.0f }}};
}

KINLINE Vector3 vec3_forward(void) {
  return (Vector3) {{{ 0.0f, 0.0f, -1.0f }}};
}

KINLINE Vector3 vec3_one(void) {
  return (Vector3) {{{ 1.0f, 1.0f, 1.0f }}};
}

KINLINE Vector3 vec3_add(Vector3 u, Vector3 v) {
  return (Vector3) {{{ u.x + v.x, u.y + v.y, u.z + v.z }}};
}

KINLINE Vector3 vec3_sub(Vector3 u, Vector3 v) {
  return (Vector3) {{{ u.x - v.x, u.y - v.y, u.z - v.z }}};
}

KINLINE Vector3 vec3_mult(Vector3 u, Vector3 v) {
  return (Vector3) {{{ u.x * v.x, u.y * v.y, u.z * v.z }}};
}

KINLINE Vector3 vec3_mult_scalar(Vector3 v, f32 k) {
  return (Vector3) {{{ v.x * k, v.y * k, v.z * k }}};
}

KINLINE f32 vec3_dot(Vector3 u, Vector3 v) {
  return (u.x * v.x) + (u.y * v.y) + (u.z * v.z);
}

KINLINE Vector3 vec3_cross(Vector3 u, Vector3 v) {
  return (Vector3) {{{
        (u.y * v.z) - (u.z * v.y),
        (u.z * v.x) - (u.x * v.z),
        (u.x * v.y) - (u.y * v.x)
      }}};
}

KINLINE Vector3 vec3_div(Vector3 u, Vector3 v) {
  return (Vector3) {{{ u.x / v.x, u.y / v.y, u.z / v.z }}};
}

KINLINE f32 vec3_squared_len(Vector3 v) {
  return (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
}

KINLINE f32 vec3_len(Vector3 v) {
  return ksqrt(vec3_squared_len(v));
}

KINLINE void vec3_normalize_set(Vector3 *v) {
  const f32 len = vec3_len(*v);
  v->x /= len;
  v->y /= len;
  v->z /= len;
}

KINLINE Vector3 vec3_normalize_get(Vector3 v) {
  vec3_normalize_set(&v);
  return v;
}

KINLINE b8 vec3_cmp(Vector3 u, Vector3 v, f32 epsilon) {
  if (kabs(u.x - v.x) > epsilon) return false;
  if (kabs(u.y - v.y) > epsilon) return false;
  if (kabs(u.z - v.z) > epsilon) return false;
  return true;
}

KINLINE f32 vec3_distance(Vector3 u, Vector3 v) {
  return vec3_len(vec3_sub(u, v));
}

KINLINE Vector4 vec4_create(f32 x, f32 y, f32 z, f32 w) {
#if defined(KUSE_SIMD)
  return (Vector4) { .data = _mm_setr_ps(x, y, z, w) };
#else
  return (Vector4) {{ x, y, z, w }};
#endif
}

KINLINE Vector4 vec3_to_vec4(Vector3 v, f32 w) {
#if defined(KUSE_SIMD)
  return (Vector4) { .data = _mm_setr_ps(v.x, v.y, v.z, w) };
#else
  return (Vector4) {{ v.x, v.y, v.z, w }};
#endif
}

KINLINE Vector3 vec4_to_vec3(Vector4 v) {
  return (Vector3) {{{ v.x, v.y, v.z }}};
}

KINLINE Vector4 vec4_zero(void) {
  return (Vector4) {{ 0.0f, 0.0f, 0.0f, 0.0f }};
}

KINLINE Vector4 vec4_one(void) {
  return (Vector4) {{ 1.0f, 1.0f, 1.0f, 1.0f }};
}

KINLINE Vector4 vec4_add(Vector4 u, Vector4 v) {
  return (Vector4) {{ u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w }};
}

KINLINE Vector4 vec4_sub(Vector4 u, Vector4 v) {
  return (Vector4) {{ u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w }};
}

KINLINE Vector4 vec4_mult(Vector4 u, Vector4 v) {
  return (Vector4) {{ u.x * v.x, u.y * v.y, u.z * v.z, u.w * v.w }};
}

KINLINE f32 vec4_dot(Vector4 u, Vector4 v) {
  return (u.x * v.x) + (u.y * v.y) + (u.z * v.z) + (u.w * v.w);
}

KINLINE f32 vec4_dot_f32(f32 x1, f32 y1, f32 z1, f32 w1,
                         f32 x2, f32 y2, f32 z2, f32 w2) {
  return (x1 * x2) + (y1 * y2) + (z1 * z2) + (w1 * w2);
}

KINLINE Vector4 vec4_div(Vector4 u, Vector4 v) {
  return (Vector4) {{ u.x / v.x, u.y / v.y, u.z / v.z, u.w / v.w }};
}

KINLINE f32 vec4_squared_len(Vector4 v) {
  return (v.x * v.x) + (v.y * v.y) + (v.z * v.z) + (v.w * v.w);
}

KINLINE f32 vec4_len(Vector4 v) {
  return ksqrt(vec4_squared_len(v));
}

KINLINE void vec4_normalize_set(Vector4 *v) {
  const f32 len = vec4_len(*v);
  v->x /= len;
  v->y /= len;
  v->z /= len;
  v->w /= len;
}

KINLINE Vector4 vec4_normalize_get(Vector4 v) {
  vec4_normalize_set(&v);
  return v;
}

KINLINE Matrix4 mat4_id(void) {
  Matrix4 m;
  kzero_memory(m.data, sizeof(f32) * 16);
  m.data[0]  = 1.0f;
  m.data[5]  = 1.0f;
  m.data[10] = 1.0f;
  m.data[15] = 1.0f;
  return m;
}

KINLINE Matrix4 mat4_mult(Matrix4 a, Matrix4 b) {
  Matrix4 m = mat4_id();
  f32 *m_ptr = m.data;
  const f32 *a_ptr = a.data;
  const f32 *b_ptr = b.data;

  for (i32 i = 0; i < 4; ++i) {
    for (i32 j = 0; j < 4; ++j) {
      *m_ptr =
        (a_ptr[0] * b_ptr[0 + j]) +
        (a_ptr[1] * b_ptr[4 + j]) +
        (a_ptr[2] * b_ptr[8 + j]) +
        (a_ptr[3] * b_ptr[12 + j]);
      ++m_ptr;
    }
    a_ptr += 4;
  }
  return m;
}

KINLINE Matrix4 mat4_ortho_proj(f32 left,
                                f32 right,
                                f32 bottom,
                                f32 top,
                                f32 near_clip,
                                f32 far_clip) {
  Matrix4 m = mat4_id();
  f32 i = 1.0f / (left - right);
  f32 j = 1.0f / (bottom - top);
  f32 k = 1.0f / (near_clip - far_clip);

  m.data[0]  = -2.0f * i;
  m.data[5]  = -2.0f * j;
  m.data[10] = 2.0f * k;
  m.data[12] = (left + right) * i;
  m.data[13] = (bottom + top) * j;
  m.data[14] = (near_clip + far_clip) * k;

  return m;
}

KINLINE Matrix4 mat4_persp_proj(f32 fov,
                                f32 aspect_ratio,
                                f32 near_clip,
                                f32 far_clip) {
  f32 half_tan_fov = ktan(fov * 0.5f);
  Matrix4 m;
  kzero_memory(m.data, sizeof(f32) * 16);

  m.data[0]  = 1.0f / (aspect_ratio * half_tan_fov);
  m.data[5]  = 1.0f / half_tan_fov;
  m.data[10] = -((2.0f * far_clip * near_clip) / (far_clip - near_clip));
  m.data[11] = -1.0f;
  m.data[14] = -((2.0f * far_clip * near_clip) / (far_clip - near_clip));

  return m;
}

KINLINE Matrix4 mat4_lookat(Vector3 position, Vector3 target, Vector3 up) {
  Matrix4 m;
  Vector3 z = {{{
        target.x - position.x,
        target.y - position.y,
        target.z - position.z
      }}};
  vec3_normalize_set(&z);
  Vector3 x = vec3_normalize_get(vec3_cross(z, up));
  Vector3 y = vec3_cross(x, z);

  m.data[0]  = x.x;
  m.data[1]  = y.x;
  m.data[2]  = -z.x;
  m.data[3]  = 0.0f;
  m.data[4]  = x.y;
  m.data[5]  = y.y;
  m.data[6]  = -z.y;
  m.data[7]  = 0.0f;
  m.data[8]  = x.z;
  m.data[9]  = y.z;
  m.data[10] = -z.z;
  m.data[11] = 0.0f;
  m.data[12] = -vec3_dot(x, position);
  m.data[13] = -vec3_dot(y, position);
  m.data[14] = vec3_dot(z, position);
  m.data[15] = 1.0f;

  return m;
}

KINLINE Matrix4 mat4_transpose(Matrix4 m) {
  Matrix4 t  = mat4_id();
  t.data[0]  = m.data[0];
  t.data[1]  = m.data[4];
  t.data[2]  = m.data[8];
  t.data[3]  = m.data[12];
  t.data[4]  = m.data[1];
  t.data[5]  = m.data[5];
  t.data[6]  = m.data[9];
  t.data[7]  = m.data[13];
  t.data[8]  = m.data[2];
  t.data[9]  = m.data[6];
  t.data[10] = m.data[10];
  t.data[11] = m.data[14];
  t.data[12] = m.data[3];
  t.data[13] = m.data[7];
  t.data[14] = m.data[11];
  t.data[15] = m.data[15];
  return t;
}

KINLINE Matrix4 mat4_inv(Matrix4 matrix) {
  const f32 *m = matrix.data;

  f32 t0  = m[10] * m[15];
  f32 t1  = m[14] * m[11];
  f32 t2  = m[6] * m[15];
  f32 t3  = m[14] * m[7];
  f32 t4  = m[6] * m[11];
  f32 t5  = m[10] * m[7];
  f32 t6  = m[2] * m[15];
  f32 t7  = m[14] * m[3];
  f32 t8  = m[2] * m[11];
  f32 t9  = m[10] * m[3];
  f32 t10 = m[2] * m[7];
  f32 t11 = m[6] * m[3];
  f32 t12 = m[8] * m[13];
  f32 t13 = m[12] * m[9];
  f32 t14 = m[4] * m[13];
  f32 t15 = m[12] * m[5];
  f32 t16 = m[4] * m[9];
  f32 t17 = m[8] * m[5];
  f32 t18 = m[0] * m[13];
  f32 t19 = m[12] * m[1];
  f32 t20 = m[0] * m[9];
  f32 t21 = m[8] * m[1];
  f32 t22 = m[0] * m[5];
  f32 t23 = m[4] * m[1];

  Matrix4 inv;
  f32 *i = inv.data;

  i[0] = ((t0 * m[5]) + (t3 * m[9]) + (t4 * m[13]))  - ((t1 * m[5]) + (t2 * m[9]) + (t5 * m[13]));
  i[1] = ((t1 * m[1]) + (t6 * m[9]) + (t9 * m[13]))  - ((t0 * m[1]) + (t7 * m[9]) + (t8 * m[13]));
  i[2] = ((t2 * m[1]) + (t7 * m[5]) + (t10 * m[13])) - ((t3 * m[1]) + (t6 * m[5]) + (t11 * m[13]));
  i[3] = ((t5 * m[1]) + (t8 * m[5]) + (t11 * m[9]))  - ((t4 * m[1]) + (t9 * m[5]) + (t10 * m[9]));

  f32 d = 1.0f / ((m[0] * i[0]) + (m[4] * i[1]) + (m[8] * i[2]) + (m[12] * i[3]));

  i[0]  = d * i[0];
  i[1]  = d * i[1];
  i[2]  = d * i[2];
  i[3]  = d * i[3];
  i[4]  = d * (((t1 * m[4]) + (t2 * m[8]) + (t5 * m[12])) - ((t0 * m[4]) + (t3 * m[8]) + (t4 * m[12])));
  i[5]  = d * (((t0 * m[0]) + (t7 * m[8]) + (t8 * m[12])) - ((t1 * m[0]) + (t6 * m[8]) + (t9 * m[12])));
  i[6]  = d * (((t3 * m[0]) + (t6 * m[4]) + (t11 * m[12])) - ((t2 * m[0]) + (t7 * m[4]) + (t10 * m[12])));
  i[7]  = d * (((t4 * m[0]) + (t9 * m[4]) + (t10 * m[8])) - ((t5 * m[0]) + (t8 * m[4]) + (t11 * m[8])));
  i[8]  = d * (((t12 * m[7]) + (t15 * m[11]) + (t16 * m[15])) - ((t13 * m[7]) + (t14 * m[11]) + (t17 * m[15])));
  i[9]  = d * (((t13 * m[3]) + (t18 * m[11]) + (t21 * m[15])) - ((t12 * m[3]) + (t19 * m[11]) + (t20 * m[15])));
  i[10] = d * (((t14 * m[3]) + (t19 * m[7]) + (t22 * m[15])) - ((t15 * m[3]) + (t18 * m[7]) + (t23 * m[15])));
  i[11] = d * (((t17 * m[3]) + (t20 * m[7]) + (t23 * m[11])) - ((t16 * m[3]) + (t21 * m[7]) + (t22 * m[11])));
  i[12] = d * (((t14 * m[10]) + (t17 * m[14]) + (t13 * m[6])) - ((t16 * m[14]) + (t12 * m[6]) + (t15 * m[10])));
  i[13] = d * (((t20 * m[14]) + (t12 * m[2]) + (t19 * m[10])) - ((t18 * m[10]) + (t21 * m[14]) + (t13 * m[2])));
  i[14] = d * (((t18 * m[6]) + (t23 * m[14]) + (t15 * m[2])) - ((t22 * m[14]) + (t14 * m[2]) + (t19 * m[6])));
  i[15] = d * (((t22 * m[10]) + (t16 * m[2]) + (t21 * m[6])) - ((t20 * m[6]) + (t23 * m[10]) + (t17 * m[2])));

  return inv;
}

KINLINE Matrix4 mat4_translation(Vector3 position) {
  Matrix4 m = mat4_id();
  m.data[12] = position.x;
  m.data[13] = position.y;
  m.data[14] = position.z;
  return m;
}

KINLINE Matrix4 mat4_scale(Vector3 scale) {
  Matrix4 m = mat4_id();
  m.data[0]  = scale.x;
  m.data[5]  = scale.y;
  m.data[10] = scale.z;
  return m;
}

KINLINE Matrix4 mat4_euler_x(f32 angle) {
  Matrix4 m = mat4_id();
  f32 cos_angle = kcos(angle);
  f32 sin_angle = ksin(angle);

  m.data[5]  = cos_angle;
  m.data[6]  = sin_angle;
  m.data[9]  = -sin_angle;
  m.data[10] = cos_angle;

  return m;
}

KINLINE Matrix4 mat4_euler_y(f32 angle) {
  Matrix4 m = mat4_id();
  f32 cos_angle = kcos(angle);
  f32 sin_angle = ksin(angle);

  m.data[0]  = cos_angle;
  m.data[2]  = -sin_angle;
  m.data[8]  = sin_angle;
  m.data[10] = cos_angle;

  return m;
}

KINLINE Matrix4 mat4_euler_z(f32 angle) {
  Matrix4 m = mat4_id();
  f32 cos_angle = kcos(angle);
  f32 sin_angle = ksin(angle);

  m.data[0]  = cos_angle;
  m.data[1]  = sin_angle;
  m.data[4]  = -sin_angle;
  m.data[5] = cos_angle;

  return m;
}

KINLINE Matrix4 mat4_euler(f32 angle_x, f32 angle_y, f32 angle_z) {
  Matrix4 x = mat4_euler_x(angle_x);
  Matrix4 y = mat4_euler_y(angle_y);
  Matrix4 z = mat4_euler_z(angle_z);
  return mat4_mult(mat4_mult(x, y), z);
}

KINLINE Vector3 mat4_forward(Matrix4 m) {
  Vector3 v;
  v.x = -m.data[2];
  v.y = -m.data[6];
  v.z = -m.data[10];
  return vec3_normalize_get(v);
}

KINLINE Vector3 mat4_backward(Matrix4 m) {
  Vector3 v;
  v.x = m.data[2];
  v.y = m.data[6];
  v.z = m.data[10];
  return vec3_normalize_get(v);
}

KINLINE Vector3 mat4_up(Matrix4 m) {
  Vector3 v;
  v.x = m.data[1];
  v.y = m.data[5];
  v.z = m.data[9];
  return vec3_normalize_get(v);
}

KINLINE Vector3 mat4_down(Matrix4 m) {
  Vector3 v;
  v.x = -m.data[1];
  v.y = -m.data[5];
  v.z = -m.data[9];
  return vec3_normalize_get(v);
}

KINLINE Vector3 mat4_left(Matrix4 m) {
  Vector3 v;
  v.x = -m.data[0];
  v.y = -m.data[4];
  v.z = -m.data[8];
  return vec3_normalize_get(v);
}

KINLINE Vector3 mat4_right(Matrix4 m) {
  Vector3 v;
  v.x = m.data[0];
  v.y = m.data[4];
  v.z = m.data[8];
  return vec3_normalize_get(v);
}

KINLINE Quaternion quat_id(void) {
  return (Quaternion) {{ 0.0f, 0.0f, 0.0f, 1.0f }};
}

KINLINE f32 quat_normal(Quaternion q) {
  return ksqrt((q.x * q.x) +
               (q.y * q.y) +
               (q.z * q.z) +
               (q.w * q.w));
}

KINLINE Quaternion quat_normalize(Quaternion q) {
  f32 n = quat_normal(q);
  return (Quaternion) {{ q.x / n, q.y / n, q.z / n, q.w / n }};
}

KINLINE Quaternion quat_conjugate(Quaternion q) {
  return (Quaternion) {{ -q.x, -q.y, -q.z, q.w }};
}

KINLINE Quaternion quat_inv(Quaternion q) {
  return quat_normalize(quat_conjugate(q));
}

KINLINE Quaternion quat_mult(Quaternion a, Quaternion b) {
  return (Quaternion) {{
      (a.x * b.w)  + (a.y * b.z) - (a.z * b.y) + (a.w * b.x),
      (-a.x * b.z) + (a.y * b.w) + (a.z * b.x) + (a.w * b.y),
      (a.x * b.y)  - (a.y * b.x) + (a.z * b.w) + (a.w * b.z),
      (-a.x * b.x) - (a.y * b.y) - (a.z * b.z) + (a.w * b.w)
    }};
}

KINLINE f32 quat_dot(Quaternion a, Quaternion b) {
  return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

KINLINE Matrix4 quat_to_mat4(Quaternion q) {
  Matrix4 m = mat4_id();
  Quaternion n = quat_normalize(q);

  m.data[0]  = 1.0f - (2.0f * n.y * n.y) - (2.0f * n.z * n.z);
  m.data[1]  = (2.0f * n.x * n.y) - (2.0f * n.z * n.w);
  m.data[2]  = (2.0f * n.x * n.z) + (2.0f * n.y * n.w);
  m.data[4]  = (2.0f * n.x * n.y) + (2.0f * n.z * n.w);
  m.data[5]  = 1.0f - (2.0f * n.x * n.x) - (2.0f * n.z * n.z);
  m.data[6]  = (2.0f * n.y * n.z) - (2.0f * n.x * n.w);
  m.data[8]  = (2.0f * n.x * n.z) - (2.0f * n.y * n.w);
  m.data[9]  = (2.0f * n.y * n.z) + (2.0f * n.x * n.w);
  m.data[10] = 1.0f - (2.0f * n.x * n.x) - (2.0f * n.y * n.y);

  return m;
}

KINLINE Matrix4 quat_to_mat4_center(Quaternion q, Vector3 center) {
  Matrix4 m;
  m.data[0] = (q.x * q.x) - (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
  m.data[1] = 2.0f * ((q.x * q.y) + (q.z * q.w));
  m.data[2] = 2.0f * ((q.x * q.z) - (q.y * q.w));
  m.data[3] = center.x - (center.x * m.data[0]) - (center.y * m.data[1]) - (center.z * m.data[2]);
  m.data[4] = 2.0f * ((q.x * q.y) - (q.z * q.w));
  m.data[5] = -(q.x * q.x) + (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
  m.data[6] = 2.0f * ((q.y * q.z) + (q.x * q.w));
  m.data[7] = center.y - (center.x * m.data[4]) - (center.y * m.data[5]) - (center.z * m.data[6]);
  m.data[8] = 2.0f * ((q.x * q.z) + (q.y * q.w));
  m.data[9] = 2.0f * ((q.y * q.z) - (q.x * q.w));
  m.data[10] = -(q.x * q.x) - (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
  m.data[11] = center.z - (center.x * m.data[8]) - (center.y * m.data[9]) - (center.z * m.data[10]);
  m.data[12] = 0.0f;
  m.data[13] = 0.0f;
  m.data[14] = 0.0f;
  m.data[15] = 1.0f;
  return m;
}

KINLINE Quaternion euler_to_quat(Vector3 v, f32 angle, b8 normalize) {
  f32 sin_angle = ksin(0.5f * angle);
  f32 cos_angle = kcos(0.5f * angle);
  Quaternion q = {{ sin_angle * v.x, sin_angle * v.y, sin_angle * v.z, cos_angle }};
  if (normalize) return quat_normalize(q);
  return q;
}

KINLINE Quaternion quat_slerp(Quaternion a, Quaternion b, f32 percent) {
  Quaternion a_n = quat_normalize(a);
  Quaternion b_n = quat_normalize(b);
  f32 dot = quat_dot(a_n, b_n);

  if (dot < 0.0f) {
    b_n.x = -b_n.x;
    b_n.y = -b_n.y;
    b_n.z = -b_n.z;
    b_n.w = -b_n.w;
    dot = -dot;
  }

  const f32 threshold = 0.9995f;
  if (dot > threshold) return quat_normalize((Quaternion) {{
        a_n.x + ((b_n.x - a_n.x) * percent),
        a_n.y + ((b_n.y - a_n.y) * percent),
        a_n.z + ((b_n.z - a_n.z) * percent),
        a_n.w + ((b_n.w - a_n.w) * percent)
      }});

  f32 theta_0 = karccos(dot);
  f32 sin_theta_0 = ksin(theta_0);
  f32 theta = theta_0 * percent;
  f32 sin_theta = ksin(theta);
  f32 s0 = kcos(theta) - (dot * sin_theta / sin_theta_0);
  f32 s1 = sin_theta / sin_theta_0;

  return (Quaternion) {{
      (a_n.x * s0) + (b_n.x * s1),
      (a_n.y * s0) + (b_n.y * s1),
      (a_n.z * s0) + (b_n.z * s1),
      (a_n.w * s0) + (b_n.w * s1)
    }};
}
