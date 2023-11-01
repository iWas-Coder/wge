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


#include <kstring.h>
#include <kmemory.h>

#include <string.h>

u64 kstrlen(const char *str) {
  return strlen(str);
}

char *kstrdup(const char *str) {
  u64 length = kstrlen(str);
  char *dup = kallocate(length + 1, MEMORY_TAG_STRING);
  kcopy_memory(dup, str, length + 1);
  return dup;
}
