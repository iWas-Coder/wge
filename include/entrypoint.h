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

#include <stdlib.h>
#include <logger.h>
#include <kmemory.h>
#include <game_types.h>
#include <application.h>

extern b8 create_game(game *out_game);

// App's main entrypoint
int main(void) {
  initialize_memory();

  game game_inst;

  if (!create_game(&game_inst)) {
    KFATAL("Could not create game");
    return EXIT_FAILURE;
  }
  if (!game_inst.render     ||
      !game_inst.update     ||
      !game_inst.initialize ||
      !game_inst.on_resize) {
    KFATAL("Game's function pointers need to be assigned");
    return EXIT_FAILURE;
  }


  // Initialization
  if (!application_create(&game_inst)) {
    KINFO("Application failed to create");
    return EXIT_FAILURE;
  }

  // Main loop
  if (!application_run()) {
    KINFO("Application closed abruptly");
    return EXIT_FAILURE;
  }

  shutdown_memory();

  return EXIT_SUCCESS;
}
