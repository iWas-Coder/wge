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

void report_assertion_failure(const char *expression, const char *message, const char *file, i32 line) {
  log_output(LOG_LEVEL_FATAL, "Assertion failure :: {%s} (%s) -> %s:%d\n", expression, message, file, line);
}
