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

#include <defines.h>

typedef struct {
  void *handle;
  b8 is_valid;
} file_handle;

typedef enum {
  FILE_MODE_READ  = 0x1,
  FILE_MODE_WRITE = 0x2
} file_modes;

KAPI b8 filesystem_exists(const char *path);
KAPI b8 filesystem_open(const char *path,
                        file_modes mode,
                        b8 binary,
                        file_handle *out_handle);
KAPI void filesystem_close(file_handle *handle);
KAPI b8 filesystem_read_line(file_handle *handle, char **line);
KAPI b8 filesystem_write_line(file_handle *handle, const char *text);
KAPI b8 filesystem_read(file_handle *handle,
                        u64 size,
                        void *out_data,
                        u64 *out_bytes_read);
KAPI b8 filesystem_read_all(file_handle *handle,
                            u8 **out_data,
                            u64 *out_bytes_read);
KAPI b8 filesystem_write(file_handle *handle,
                         u64 size,
                         const void *data,
                         u64 *out_bytes_written);
