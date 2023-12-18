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
#include <math_types.h>

#define FORMAT_BUF_SIZE 32000  // 32K character limit

KAPI u64 kstrlen(const char *str);

KAPI char *kstrdup(const char *str);

KAPI b8 kstrcmp(const char *s1, const char *s2);

KAPI b8 kstrcmpi(const char *s1, const char *s2);

KAPI i32 kstrfmt(char *dest, const char *format, ...);

KAPI i32 kstrfmt_v(char *dest, const char *format, void *va_list);

KAPI char *kstrcp(char *dest, const char *src);

KAPI char *kstrncp(char *dest, const char *src, i64 len);

KAPI char *kstrtr(char *str);

KAPI void kstrsub(char *dest, const char *src, u64 start, u64 len);

KAPI i32 kstridx(char *str, char c);

KAPI b8 str_to_vec4(char *str, Vector4 *v);

KAPI b8 str_to_vec3(char *str, Vector3 *v);

KAPI b8 str_to_vec2(char *str, Vector2 *v);

KAPI b8 str_to_f32(char *str, f32 *f);

KAPI b8 str_to_f64(char *str, f64 *f);

KAPI b8 str_to_i8(char *str, i8 *i);

KAPI b8 str_to_i16(char *str, i16 *i);

KAPI b8 str_to_i32(char *str, i32 *i);

KAPI b8 str_to_i64(char *str, i64 *i);

KAPI b8 str_to_u8(char *str, u8 *u);

KAPI b8 str_to_u16(char *str, u16 *u);

KAPI b8 str_to_u32(char *str, u32 *u);

KAPI b8 str_to_u64(char *str, u64 *u);

KAPI b8 str_to_bool(char *str, b8 *b);
