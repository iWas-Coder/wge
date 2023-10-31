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


#include <darray.h>
#include <logger.h>
#include <kmemory.h>

void *_darray_create(u64 length, u64 stride) {
  u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
  u64 array_size = length * stride;
  u64 *array = kallocate(header_size + array_size, MEMORY_TAG_DARRAY);
  kset_memory(array, 0, header_size + array_size);
  array[DARRAY_CAPACITY] = length;
  array[DARRAY_LENGTH] = 0;
  array[DARRAY_STRIDE] = stride;
  return (void *) (array + DARRAY_FIELD_LENGTH);
}

void _darray_destroy(void *array) {
  u64 *header = (u64 *) array - DARRAY_FIELD_LENGTH;
  u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
  u64 total_size = header_size + (header[DARRAY_CAPACITY] * header[DARRAY_STRIDE]);
  kfree(header, total_size, MEMORY_TAG_DARRAY);
}

u64 _darray_field_get(void *array, u64 field) {
  u64 *header = (u64 *) array - DARRAY_FIELD_LENGTH;
  return header[field];
}

void _darray_field_set(void *array, u64 field, u64 value) {
  u64 *header = (u64 *) array - DARRAY_FIELD_LENGTH;
  header[field] = value;
}

void *_darray_resize(void *array) {
  u64 length = darray_length(array);
  u64 stride = darray_stride(array);
  void *resized_array = _darray_create(DARRAY_RESIZE_FACTOR * darray_capacity(array), stride);
  kcopy_memory(resized_array, array, length * stride);
  _darray_field_set(resized_array, DARRAY_LENGTH, length);
  _darray_destroy(array);
  return resized_array;
}

void *_darray_push(void *array, const void *value_ptr) {
  u64 length = darray_length(array);
  u64 stride = darray_stride(array);
  if (length >= darray_capacity(array)) array = _darray_resize(array);
  u64 addr = (u64) array;
  addr += (length * stride);
  kcopy_memory((void *) addr, value_ptr, stride);
  _darray_field_set(array, DARRAY_LENGTH, length + 1);
  return array;
}

void _darray_pop(void *array, void *dest) {
  u64 length = darray_length(array);
  u64 stride = darray_stride(array);
  u64 addr = (u64) array;
  addr += (stride * (length - 1));
  kcopy_memory(dest, (void *) addr, stride);
  _darray_field_set(array, DARRAY_LENGTH, length - 1);
}

void *_darray_insert_at(void *array, u64 index, void *value_ptr) {
  u64 length = darray_length(array);
  u64 stride = darray_stride(array);

  if (index >= length) {
    KERROR("Index out of bounds :: length: %i | index: %i", length, index);
    return array;
  }

  if (length >= darray_capacity(array)) array = _darray_resize(array);
  u64 addr = (u64) array;

  if (index != length - 1) {
    // Push everything out as a block by 1 index
    kcopy_memory((void *) (addr + (stride * (index + 1))),
                 (void *) (addr + (stride * index)),
                 stride * (length - index));
  }
  kcopy_memory((void *) (addr + (stride * index)), value_ptr, stride);
  _darray_field_set(array, DARRAY_LENGTH, length + 1);
  return array;
}

void *_darray_pop_at(void *array, u64 index, void *dest) {
  u64 length = darray_length(array);
  u64 stride = darray_stride(array);

  if (index >= length) {
    KERROR("Index out of bounds :: length: %i | index: %i", length, index);
    return array;
  }

  u64 addr = (u64) array;
  kcopy_memory(dest, (void *) (addr + (index * stride)), stride);

  if (index != length - 1) {
    // Pull everything in as a block by 1 index
    kcopy_memory((void *) (addr + (stride * index)),
                 (void *) (addr + (stride * (index + 1))),
                 stride * (length - index));
  }
  _darray_field_set(array, DARRAY_LENGTH, length - 1);
  return array;
}
