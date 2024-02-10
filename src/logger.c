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


#include <stdarg.h>
#include <logger.h>
#include <kstring.h>
#include <kmemory.h>
#include <platform.h>
#include <filesystem.h>

typedef struct {
  file_handle logfile_handle;
} logger_system_state;

static logger_system_state *state_ptr;

static void write_to_logfile(const char *msg) {
  if (!state_ptr || !state_ptr->logfile_handle.is_valid) return;
  u64 len = kstrlen(msg);
  u64 written = 0;
  if (!filesystem_write(&state_ptr->logfile_handle, len, msg, &written)) {
    platform_console_write_error("[ERROR]: Unable to write to logfile (wge.log)",
                                 LOG_LEVEL_ERROR);
  }
}

b8 initialize_logging(u64 *memory_requirements, void *state) {
  *memory_requirements = sizeof(logger_system_state);
  if (!state) return true;

  state_ptr = state;

  // Create log file
  if (!filesystem_open("wge.log", FILE_MODE_WRITE, false, &state_ptr->logfile_handle)) {
    platform_console_write_error("[ERROR]: Unable to open logfile (wge.log) for writing",
                                 LOG_LEVEL_ERROR);
    return false;
  }

  return true;
}

void shutdown_logging(void *state) {
  (void) state;  // Unused parameter

  // TODO: cleanup logging (write queued entries)

  state_ptr = 0;
}

void log_output(log_level level, const char *message, ...) {
  b8 is_error = level < LOG_LEVEL_WARN;
  const char *level_strings[] = {
    "[FATAL]: ",
    "[ERROR]: ",
    "[WARN]: ",
    "[INFO]: ",
    "[DEBUG]: ",
    "[TRACE]: "
  };
  char out_message[LOG_MSG_SIZE];
  kzero_memory(out_message, sizeof(out_message));

  // Read all arguments and construct the log message
  __builtin_va_list arg_ptr;
  va_start(arg_ptr, message);
  kstrfmt_v(out_message, message, arg_ptr);
  va_end(arg_ptr);

  // Prepend the corresponding loglevel to the log message
  kstrfmt(out_message, "%s%s\n", level_strings[level], out_message);

  if (is_error) platform_console_write_error(out_message, level);
  else platform_console_write(out_message, level);

  // Write the msg also to the logfile
  write_to_logfile(out_message);
}
