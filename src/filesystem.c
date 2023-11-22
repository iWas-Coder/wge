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


#include <stdio.h>
#include <logger.h>
#include <string.h>
#include <kmemory.h>
#include <sys/stat.h>
#include <filesystem.h>

#define FS_READ_LINE_BUF_SIZE 32000  // 32K character limit

KAPI b8 filesystem_exists(const char *path) {
  struct stat buf;
  return stat(path, &buf) == 0;
}

KAPI b8 filesystem_open(const char *path,
                        file_modes mode,
                        b8 binary,
                        file_handle *out_handle) {
  out_handle->is_valid = false;
  out_handle->handle = 0;

  const char *buf;
  if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0) buf = binary ? "w+b" : "w+";
  else if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0) buf = binary ? "rb" : "r";
  else if ((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0) buf = binary ? "wb" : "w";
  else {
    KERROR("filesystem_open :: invalid mode (file: '%s')", path);
    return false;
  }

  FILE *fd = fopen(path, buf);
  if (!fd) {
    KERROR("filesystem_open :: could not open the file ('%s')", path);
    return false;
  }
  out_handle->handle = fd;
  out_handle->is_valid = true;
  return true;
}

KAPI void filesystem_close(file_handle *handle) {
  if (!handle->handle) return;
  fclose((FILE *) handle->handle);
  handle->handle = 0;
  handle->is_valid = false;
}

KAPI b8 filesystem_read_line(file_handle *handle, char **line) {
  if (!handle->handle) return false;
  char buf[FS_READ_LINE_BUF_SIZE];
  if (fgets(buf, FS_READ_LINE_BUF_SIZE, (FILE *) handle->handle) != 0) {
    u64 len = strlen(buf);
    *line = kallocate((sizeof(char) * len) + 1, MEMORY_TAG_STRING);
    strcpy(*line, buf);
    return true;
  }
  return false;
}

KAPI b8 filesystem_write_line(file_handle *handle, const char *text) {
  if (!handle->handle) return false;
  i32 result = fputs(text, (FILE *) handle->handle);
  if (result != EOF) result = fputc('\n', (FILE *) handle->handle);
  fflush((FILE *) handle->handle);
  return result != EOF;
}

KAPI b8 filesystem_read(file_handle *handle,
                        u64 size,
                        void *out_data,
                        u64 *out_bytes_read) {
  if (!handle->handle || !out_data) return false;
  *out_bytes_read = fread(out_data, 1, size, (FILE *) handle->handle);
  if (*out_bytes_read != size) return false;
  return true;
}

KAPI b8 filesystem_read_all(file_handle *handle,
                            u8 **out_data,
                            u64 *out_bytes_read) {
  if (!handle->handle) return false;
  fseek((FILE *) handle->handle, 0, SEEK_END);
  u64 size = ftell((FILE *) handle->handle);
  rewind((FILE *) handle->handle);
  *out_data = kallocate(sizeof(u8) * size, MEMORY_TAG_STRING);
  *out_bytes_read = fread(*out_data, 1, size, (FILE *) handle->handle);
  if (*out_bytes_read != size) return false;
  return true;
}

KAPI b8 filesystem_write(file_handle *handle,
                         u64 size,
                         const void *data,
                         u64 *out_bytes_written) {
  if (!handle->handle) return false;
  *out_bytes_written = fwrite(data, 1, size, (FILE *) handle->handle);
  if (*out_bytes_written != size) return false;
  fflush((FILE *) handle->handle);
  return true;
}
