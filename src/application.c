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
#include <clock.h>
#include <logger.h>
#include <asserts.h>
#include <kmemory.h>
#include <platform.h>
#include <game_types.h>
#include <application.h>
#include <texture_system.h>
#include <linear_allocator.h>
#include <renderer_frontend.h>

#define FRAMERATE 60
#define TIME_MS_IN_S 1e3
#define SYSTEMS_ALLOCATOR_SIZE 64 * 1024 * 1024  // 64MB
#define TEXTURE_SYSTEM_MAX_COUNT 65536

typedef struct {
  game *game_inst;
  b8 is_running;
  b8 is_suspended;
  i16 width;
  i16 height;
  clock clock;
  f64 last_time;
  linear_allocator systems_allocator;
  u64 event_system_memory_requirements;
  void *event_system_state;
  u64 memory_system_memory_requirements;
  void *memory_system_state;
  u64 logging_system_memory_requirements;
  void *logging_system_state;
  u64 input_system_memory_requirements;
  void *input_system_state;
  u64 platform_system_memory_requirements;
  void *platform_system_state;
  u64 renderer_system_memory_requirements;
  void *renderer_system_state;
  u64 texture_system_memory_requirements;
  void *texture_system_state;
} application_state;

static application_state *app_state;

void application_set_framebuffer_size(u32 *width, u32 *height) {
  *width = app_state->width;
  *height = app_state->height;
}

b8 application_on_event(u16 code, void *sender, void *listener_inst, event_context context) {
  (void) sender;         // Unused parameter
  (void) listener_inst;  // Unused parameter
  (void) context;        // Unused parameter

  if (code == EVENT_CODE_APPLICATION_QUIT) {
    KINFO("Quit event received. Shutting down the engine...");
    app_state->is_running = false;
    return true;
  }
  return false;
}

b8 application_on_key(u16 code, void *sender, void *listener_inst, event_context context) {
  (void) sender;         // Unused parameter
  (void) listener_inst;  // Unused parameter

  u16 key_code = context.data.u16[0];

  // Key pressed
  if (code == EVENT_CODE_KEY_PRESSED) {
    if (key_code == KEY_ESCAPE) {
      event_context data = {0};
      event_fire(EVENT_CODE_APPLICATION_QUIT, 0, data);
      return true;
    }
    else { KDEBUG("'%c' key pressed", key_code); }
  }
  // Key released
  else if (code == EVENT_CODE_KEY_RELEASED) {
    KDEBUG("'%c' key released", key_code);
  }
  return false;
}

b8 application_on_resized(u16 code, void *sender, void *listener_inst, event_context context) {
  (void) sender;         // Unused parameter
  (void) listener_inst;  // Unused parameter

  if (code == EVENT_CODE_RESIZED) {
    u16 width = context.data.u16[0];
    u16 height = context.data.u16[1];
    if (width != app_state->width || height != app_state->height) {
      KDEBUG("Window resized :: %hux%hu -> %hux%hu",
             app_state->width, app_state->height,
             width, height);
      app_state->width = width;
      app_state->height = height;

      // Minimize window
      if (!width || !height) {
        KINFO("Application suspended (due to window minimize)");
        app_state->is_suspended = true;
        return true;
      }
      // Restore window
      else if (app_state->is_suspended) {
        KINFO("Application resumed (due to window restore)");
        app_state->is_suspended = false;
      }
      app_state->game_inst->on_resize(app_state->game_inst, width, height);
      renderer_on_resized(width, height);
    }
  }
  return false;
}

b8 application_create(game *game_inst) {
  if (game_inst->app_state) {
    KERROR("Tried to create the application multiple times");
    return false;
  }

  game_inst->app_state = kallocate(sizeof(application_state), MEMORY_TAG_APPLICATION);
  app_state = game_inst->app_state;
  app_state->game_inst = game_inst;
  app_state->is_running = false;
  app_state->is_suspended = false;

  linear_allocator_create(SYSTEMS_ALLOCATOR_SIZE, 0, &app_state->systems_allocator);

  // Initialize event system
  event_system_initialize(&app_state->event_system_memory_requirements, 0);
  app_state->event_system_state = linear_allocator_alloc(&app_state->systems_allocator,
                                                        app_state->event_system_memory_requirements);
  event_system_initialize(&app_state->event_system_memory_requirements, app_state->event_system_state);

  // Initialize memory system
  memory_system_initialize(&app_state->memory_system_memory_requirements, 0);
  app_state->memory_system_state = linear_allocator_alloc(&app_state->systems_allocator,
                                                         app_state->memory_system_memory_requirements);
  memory_system_initialize(&app_state->memory_system_memory_requirements, app_state->memory_system_state);

  // Initialize logging system
  initialize_logging(&app_state->logging_system_memory_requirements, 0);
  app_state->logging_system_state = linear_allocator_alloc(&app_state->systems_allocator,
                                                          app_state->logging_system_memory_requirements);
  if (!initialize_logging(&app_state->logging_system_memory_requirements, app_state->logging_system_state)) {
    KERROR("Logging system initialization failed. Shutting down the engine...");
    return false;
  }

  // Initialize input system
  input_system_initialize(&app_state->input_system_memory_requirements, 0);
  app_state->input_system_state = linear_allocator_alloc(&app_state->systems_allocator,
                                                        app_state->input_system_memory_requirements);
  input_system_initialize(&app_state->input_system_memory_requirements,
                          app_state->input_system_state);

  // Engine-level events registration
  event_register(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
  event_register(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
  event_register(EVENT_CODE_KEY_RELEASED, 0, application_on_key);
  event_register(EVENT_CODE_RESIZED, 0, application_on_resized);

  // Initialize platform system
  platform_system_startup(&app_state->platform_system_memory_requirements,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0);
  app_state->platform_system_state = linear_allocator_alloc(&app_state->systems_allocator,
                                                           app_state->platform_system_memory_requirements);
  if (!platform_system_startup(&app_state->platform_system_memory_requirements,
                               app_state->platform_system_state,
                               game_inst->app_config.name,
                               game_inst->app_config.start_pos_x,
                               game_inst->app_config.start_pos_y,
                               game_inst->app_config.start_width,
                               game_inst->app_config.start_height)) return false;

  // Initialize renderer system
  renderer_system_initialize(&app_state->renderer_system_memory_requirements,
                             0,
                             0);
  app_state->renderer_system_state = linear_allocator_alloc(&app_state->systems_allocator,
                                                           app_state->renderer_system_memory_requirements);
  if (!renderer_system_initialize(&app_state->renderer_system_memory_requirements,
                                  app_state->renderer_system_state,
                                  game_inst->app_config.name)) {
    KFATAL("Renderer initialization failed. Shutting down the engine...");
    return false;
  }

  // Initialize texture system
  texture_system_config texture_system_cfg = {
    .max_texture_count = TEXTURE_SYSTEM_MAX_COUNT
  };
  texture_system_initialize(&app_state->texture_system_memory_requirements,
                            0,
                            texture_system_cfg);
  app_state->texture_system_state = linear_allocator_alloc(&app_state->systems_allocator,
                                                          app_state->texture_system_memory_requirements);
  if (!texture_system_initialize(&app_state->texture_system_memory_requirements,
                                 app_state->texture_system_state,
                                 texture_system_cfg)) {
    KFATAL("Texture system initialization failed. Shutting down the engine...");
    return false;
  }

  // Game initialization
  if (!app_state->game_inst->initialize(app_state->game_inst)) {
    KFATAL("Game initialization failed. Shutting down the engine...");
    return false;
  }
  app_state->game_inst->on_resize(app_state->game_inst, app_state->width, app_state->height);

  return true;
}

b8 application_run(void) {
  app_state->is_running = true;
  clock_start(&app_state->clock);
  clock_update(&app_state->clock);
  app_state->last_time = app_state->clock.elapsed;

  f64 running_time = 0;
  u8 frame_count = 0;
  f64 target_frame_time = 1.0f / FRAMERATE;

  KINFO(get_memory_usage_str());

  while (app_state->is_running) {
    if (!platform_pump_messages()) app_state->is_running = false;

    if (!app_state->is_suspended) {
      clock_update(&app_state->clock);
      f64 current_time = app_state->clock.elapsed;
      f64 delta = current_time - app_state->last_time;
      f64 frame_start_time = platform_get_absolute_time();

      if (!app_state->game_inst->update(app_state->game_inst, (f32) delta)) {
        KFATAL("Game update failed. Shutting down the engine...");
        app_state->is_running = false;
        break;
      }
      if (!app_state->game_inst->render(app_state->game_inst, (f32) delta)) {
        KFATAL("Game render failed. Shutting down the engine...");
        app_state->is_running = false;
        break;
      }

      render_packet packet;
      packet.delta_time = delta;
      renderer_draw_frame(&packet);

      f64 frame_end_time = platform_get_absolute_time();
      f64 frame_elapsed_time = frame_end_time - frame_start_time;
      running_time += frame_elapsed_time;
      f64 remaining_time = target_frame_time - frame_elapsed_time;
      if (remaining_time > 0) {
        u64 remaining_ms = remaining_time * TIME_MS_IN_S;
        // Framelimiter turned ON if this is set to TRUE
        b8 framelimit = false;
        if (remaining_ms > 0 && framelimit) platform_sleep(remaining_ms - 1);
        ++frame_count;
      }

      // Input is the last thing to be updated before the frame ends
      input_update(delta);

      (void) running_time;  // Unused parameter
      (void) frame_count;   // Unused parameter

      app_state->last_time = current_time;
    }
  }
  app_state->is_running = false;

  // Engine-level events unregistration
  event_unregister(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
  event_unregister(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
  event_unregister(EVENT_CODE_KEY_RELEASED, 0, application_on_key);
  event_unregister(EVENT_CODE_RESIZED, 0, application_on_resized);

  input_system_shutdown(app_state->input_system_state);
  texture_system_shutdown(app_state->texture_system_state);
  renderer_system_shutdown(app_state->renderer_system_state);
  platform_system_shutdown(app_state->platform_system_state);
  memory_system_shutdown(app_state->memory_system_state);
  event_system_shutdown(app_state->event_system_state);

  return true;
}
