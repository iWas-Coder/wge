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
