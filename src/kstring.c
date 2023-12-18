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
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <kstring.h>
#include <kmemory.h>

#ifndef _MSC_VER
#include <strings.h>
#endif

u64 kstrlen(const char *str) {
  return strlen(str);
}

char *kstrdup(const char *str) {
  u64 length = kstrlen(str);
  char *dup = kallocate(length + 1, MEMORY_TAG_STRING);
  kcopy_memory(dup, str, length + 1);
  return dup;
}

b8 kstrcmp(const char *s1, const char *s2) {
  return strcmp(s1, s2) == 0;
}

b8 kstrcmpi(const char *s1, const char *s2) {
#if defined(__GNUC__)
  return strcasecmp(s1, s2) == 0;
#elif defined(_MSC_VER)
  return _strcmpi(s1, s2) == 0;
#endif
}

i32 kstrfmt(char *dest, const char *format, ...) {
  if (!dest) return -1;
  __builtin_va_list arg_ptr;
  va_start(arg_ptr, format);
  i32 n_bytes = kstrfmt_v(dest, format, arg_ptr);
  va_end(arg_ptr);
  return n_bytes;
}

i32 kstrfmt_v(char *dest, const char *format, void *va_list) {
  if (!dest) return -1;
  char buf[FORMAT_BUF_SIZE];
  i32 n_bytes = vsnprintf(buf, FORMAT_BUF_SIZE, format, va_list);
  buf[n_bytes] = 0;
  kcopy_memory(dest, buf, n_bytes + 1);
  return n_bytes;
}

char *kstrcp(char *dest, const char *src) {
  return strcpy(dest, src);
}

char *kstrncp(char *dest, const char *src, i64 len) {
  return strncpy(dest, src, len);
}

char *kstrtr(char *str) {
  while (isspace((unsigned char) *str)) ++str;
  if (*str) {
    char *p = str;
    while (*p) ++p;
    while (isspace((unsigned char) *--p));
    p[1] = '\0';
  }
  return str;
}

void kstrsub(char *dest, const char *src, u64 start, u64 len) {
  if (!len) return;
  u64 src_len = kstrlen(src);
  if (start >= src_len) {
    dest[0] = 0;
    return;
  }
  if (len > 0) {
    for (u64 i = start, j = 0; j < len && src[i]; ++i, ++j) dest[j] = src[i];
    dest[start + len] = 0;
  }
  else {
    u64 j = 0;
    for (u64 i = start; src[i]; ++i, ++j) dest[j] = src[i];
    dest[start + j] = 0;
  }
}

i32 kstridx(char *str, char c) {
  if (!str) return -1;
  u32 len = kstrlen(str);
  if (len > 0) {
    for (u32 i = 0; i < len; ++i) {
      if (str[i] == c) return i;
    }
  }
  return -1;
}

b8 str_to_vec4(char *str, Vector4 *v) {
  if (!str) return false;
  kzero_memory(v, sizeof(Vector4));
  i32 result = sscanf(str,
                      "%f %f %f %f",
                      &v->x,
                      &v->y,
                      &v->z,
                      &v->w);
  return result != -1;
}

b8 str_to_vec3(char *str, Vector3 *v) {
  if (!str) return false;
  kzero_memory(v, sizeof(Vector3));
  i32 result = sscanf(str,
                      "%f %f %f",
                      &v->x,
                      &v->y,
                      &v->z);
  return result != -1;
}

b8 str_to_vec2(char *str, Vector2 *v) {
  if (!str) return false;
  kzero_memory(v, sizeof(Vector2));
  i32 result = sscanf(str,
                      "%f %f",
                      &v->x,
                      &v->y);
  return result != -1;
}

b8 str_to_f32(char *str, f32 *f) {
  if (!str) return false;
  *f = 0;
  i32 result = sscanf(str, "%f", f);
  return result != -1;
}

b8 str_to_f64(char *str, f64 *f) {
  if (!str) return false;
  *f = 0;
  i32 result = sscanf(str, "%lf", f);
  return result != -1;
}

b8 str_to_i8(char *str, i8 *i) {
  if (!str) return false;
  *i = 0;
  i32 result = sscanf(str, "%c", i);
  return result != -1;
}

b8 str_to_i16(char *str, i16 *i) {
  if (!str) return false;
  *i = 0;
  i32 result = sscanf(str, "%hi", i);
  return result != -1;
}

b8 str_to_i32(char *str, i32 *i) {
  if (!str) return false;
  *i = 0;
  i32 result = sscanf(str, "%i", i);
  return result != -1;
}

b8 str_to_i64(char *str, i64 *i) {
  if (!str) return false;
  *i = 0;
  i32 result = sscanf(str, "%lli", i);
  return result != -1;
}

b8 str_to_u8(char *str, u8 *u) {
  if (!str) return false;
  *u = 0;
  i32 result = sscanf(str, "%hhu", u);
  return result != -1;
}

b8 str_to_u16(char *str, u16 *u) {
  if (!str) return false;
  *u = 0;
  i32 result = sscanf(str, "%hu", u);
  return result != -1;
}

b8 str_to_u32(char *str, u32 *u) {
  if (!str) return false;
  *u = 0;
  i32 result = sscanf(str, "%u", u);
  return result != -1;
}

b8 str_to_u64(char *str, u64 *u) {
  if (!str) return false;
  *u = 0;
  i32 result = sscanf(str, "%llu", u);
  return result != -1;
}

b8 str_to_bool(char *str, b8 *b) {
  (void) b;  // Unused parameter

  if (!str) return false;
  return kstrcmp(str, "1") || kstrcmpi(str, "true");
}
