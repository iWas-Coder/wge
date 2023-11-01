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
#include <kstring.h>
#include <platform.h>

#include <stdio.h>
#include <stdarg.h>

b8 initialize_logging(void) {
  // TODO: create log file
  return TRUE;
}

void shutdown_logging(void) {
  // TODO: cleanup logging (write queued entries)
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

  // Read all arguments and construct the log message ('tmp')
  char tmp[LOG_MSG_SIZE] = {0};
  __builtin_va_list arg_ptr;
  va_start(arg_ptr, message);
  vsnprintf(tmp, LOG_MSG_SIZE, message, arg_ptr);
  va_end(arg_ptr);

  // Construct the final log message ('out_message') by prepending
  // the corresponding loglevel tag to the raw log message ('tmp')
  char out_message[LOG_MSG_SIZE + kstrlen(level_strings[level])];
  sprintf(out_message, "%s%s\n", level_strings[level], tmp);

  // printf("%s", out_message);
  if (is_error) platform_console_write_error(out_message, level);
  else platform_console_write(out_message, level);
}
