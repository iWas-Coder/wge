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

#include <application.h>

typedef struct game {
  void *state;
  void *app_state;
  application_config app_config;
  b8 (*initialize)(struct game *game_inst);
  b8 (*update)(struct game *game_inst, f32 delta_time);
  b8 (*render)(struct game *game_inst, f32 delta_time);
  void (*on_resize)(struct game *game_inst, u32 width, u32 height);
} game;
