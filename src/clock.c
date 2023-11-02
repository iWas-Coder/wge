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


#include <clock.h>
#include <platform.h>

void clock_start(clock *clock) {
  clock->start = platform_get_absolute_time();
  clock->elapsed = 0;
}

void clock_stop(clock *clock) {
  clock->start = 0;
}

void clock_update(clock *clock) {
  if (clock->start) clock->elapsed = platform_get_absolute_time() - clock->start;
}
