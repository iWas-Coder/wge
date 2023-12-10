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

#define FORMAT_BUF_SIZE 32000  // 32K character limit

KAPI u64 kstrlen(const char *str);

KAPI char *kstrdup(const char *str);

KAPI b8 kstrcmp(const char *s1, const char *s2);

KAPI b8 kstrcmpi(const char *s1, const char *s2);

KAPI i32 kstrfmt(char *dest, const char *format, ...);

KAPI i32 kstrfmt_v(char *dest, const char *format, void *va_list);
