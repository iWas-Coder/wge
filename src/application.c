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


#include <logger.h>
#include <asserts.h>
#include <kmemory.h>
#include <platform.h>
#include <game_types.h>
#include <application.h>

typedef struct {
  game *game_inst;
  b8 is_running;
  b8 is_suspended;
  platform_state platform;
  i16 width;
  i16 height;
  f64 last_time;
} application_state;

static application_state app_state;
static b8 initialized = FALSE;

b8 application_create(game *game_inst) {
  if (initialized) {
    KERROR("Tried to create the application multiple times");
    return FALSE;
  }

  app_state.game_inst = game_inst;

  // Initialize all subsystems
  initialize_logging();

  KFATAL("A test message: %f", 3.14f);
  KERROR("A test message: %f", 3.14f);
  KWARN("A test message: %f", 3.14f);
  KINFO("A test message: %f", 3.14f);
  KDEBUG("A test message: %f", 3.14f);
  KTRACE("A test message: %f", 3.14f);

  KASSERT(3.14f == 3.14f);

  app_state.is_running = TRUE;
  app_state.is_suspended = FALSE;

  if (!platform_startup(&app_state.platform,
                        game_inst->app_config.name,
                        game_inst->app_config.start_pos_x,
                        game_inst->app_config.start_pos_y,
                        game_inst->app_config.start_width,
                        game_inst->app_config.start_height)) return FALSE;

  // Game initialization
  if (!app_state.game_inst->initialize(app_state.game_inst)) {
    KFATAL("Game initialization failed");
    return FALSE;
  }
  app_state.game_inst->on_resize(app_state.game_inst, app_state.width, app_state.height);

  initialized = TRUE;
  return TRUE;
}

b8 application_run(void) {
  KINFO(get_memory_usage_str());

  while (app_state.is_running) {
    if (!platform_pump_messages(&app_state.platform)) app_state.is_running = FALSE;

    if (!app_state.is_suspended) {
      if (!app_state.game_inst->update(app_state.game_inst, (f32) 0)) {
        KFATAL("Game update failed. Shutting down the engine...");
        app_state.is_running = FALSE;
        break;
      }
      if (!app_state.game_inst->render(app_state.game_inst, (f32) 0)) {
        KFATAL("Game render failed. Shutting down the engine...");
        app_state.is_running = FALSE;
        break;
      }
    }
  }
  app_state.is_running = FALSE;

  platform_shutdown(&app_state.platform);
  return TRUE;
}
