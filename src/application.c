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


#include <event.h>
#include <input.h>
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

b8 application_on_event(u16 code, void *sender, void *listener_inst, event_context context);
b8 application_on_event(u16 code, void *sender, void *listener_inst, event_context context) {
  (void) sender;         // Unused parameter
  (void) listener_inst;  // Unused parameter
  (void) context;        // Unused parameter

  if (code == EVENT_CODE_APPLICATION_QUIT) {
    KINFO("Quit event received. Shutting down the engine...");
    app_state.is_running = FALSE;
    return TRUE;
  }
  return FALSE;
}

b8 application_on_key(u16 code, void *sender, void *listener_inst, event_context context);
b8 application_on_key(u16 code, void *sender, void *listener_inst, event_context context) {
  (void) sender;         // Unused parameter
  (void) listener_inst;  // Unused parameter

  // Key pressed
  if (code == EVENT_CODE_KEY_PRESSED) {
    u16 key_code = context.data.u16[0];
    if (key_code == KEY_ESCAPE) {
      event_context data = {0};
      event_fire(EVENT_CODE_APPLICATION_QUIT, 0, data);
      return TRUE;
    }
    else KDEBUG("'%c' key pressed", key_code);
  }
  // Key released
  else if (code == EVENT_CODE_KEY_RELEASED) {
    u16 key_code = context.data.u16[0];
    KDEBUG("'%c' key released", key_code);
  }
  return FALSE;
}

b8 application_create(game *game_inst) {
  if (initialized) {
    KERROR("Tried to create the application multiple times");
    return FALSE;
  }

  app_state.game_inst = game_inst;

  // Initialize all subsystems
  initialize_logging();
  input_initialize();

  app_state.is_running = TRUE;
  app_state.is_suspended = FALSE;

  if (!event_initialize()) {
    KERROR("Event system initialization failed");
    return FALSE;
  }

  event_register(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
  event_register(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
  event_register(EVENT_CODE_KEY_RELEASED, 0, application_on_key);

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

      // Input is the last thing to be updated before the frame ends
      input_update(0);
    }
  }
  app_state.is_running = FALSE;

  event_unregister(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
  event_unregister(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
  event_unregister(EVENT_CODE_KEY_RELEASED, 0, application_on_key);

  event_shutdown();
  input_shutdown();
  platform_shutdown(&app_state.platform);
  return TRUE;
}
