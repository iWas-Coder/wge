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


#include <math.h>
#include <kmath.h>
#include <stdlib.h>
#include <platform.h>

static b8 rand_seeded = FALSE;

f32 kabs(f32 x) {
  return fabsf(x);
}

f32 ksqrt(f32 x) {
  return sqrtf(x);
}

f32 ksin(f32 x) {
  return sinf(x);
}

f32 kcos(f32 x) {
  return cosf(x);
}

f32 ktan(f32 x) {
  return tanf(x);
}

f32 karccos(f32 x) {
  return acosf(x);
}

i32 krandom(void) {
  if (!rand_seeded) {
    srand((u32) platform_get_absolute_time());
    rand_seeded = TRUE;
  }
  return rand();
}

f32 krandom_f(void) {
  return (f32) krandom() / (f32) RAND_MAX;
}

i32 krandom_range(i32 min, i32 max) {
  if (!rand_seeded) {
    srand((u32) platform_get_absolute_time());
    rand_seeded = TRUE;
  }
  return (rand() % (max - min + 1)) + min;
}

f32 krandom_range_f(f32 min, f32 max) {
  return min + ((f32) krandom() / ((f32) RAND_MAX / (max - min)));
}
