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


#include <expect.h>
#include <defines.h>
#include <test_manager.h>
#include <linear_allocator.h>
#include <linear_allocator_test.h>

u8 linear_allocator_test_create_destroy(void) {
  linear_allocator allocator;
  linear_allocator_create(sizeof(u64), 0, &allocator);

  should_not_be(0, allocator.memory);
  should_be(sizeof(u64), allocator.total_size);
  should_be(0, allocator.allocated);

  linear_allocator_destroy(&allocator);

  should_be(0, allocator.memory);
  should_be(0, allocator.total_size);
  should_be(0, allocator.allocated);

  return true;
}

u8 linear_allocator_test_single_alloc_all(void) {
  linear_allocator allocator;
  linear_allocator_create(sizeof(u64), 0, &allocator);

  void *block = linear_allocator_alloc(&allocator, sizeof(u64));
  should_not_be(0, block);
  should_be(sizeof(u64), allocator.allocated);

  linear_allocator_destroy(&allocator);
  return true;
}

u8 linear_allocator_test_multi_alloc_all(void) {
  linear_allocator allocator;
  u64 max_allocs = 1024;
  linear_allocator_create(sizeof(u64) * max_allocs, 0, &allocator);

  void *block;
  for (u64 i = 0; i < max_allocs; ++i) {
    block = linear_allocator_alloc(&allocator, sizeof(u64));
    should_not_be(0, block);
    should_be(sizeof(u64) * (i + 1), allocator.allocated);
  }

  linear_allocator_destroy(&allocator);
  return true;
}

u8 linear_allocator_test_multi_alloc_overflow(void) {
  linear_allocator allocator;
  u64 max_allocs = 3;
  linear_allocator_create(sizeof(u64) * max_allocs, 0, &allocator);

  // Allocate all reserved memory
  void *block;
  for (u64 i = 0; i < max_allocs; ++i) {
    block = linear_allocator_alloc(&allocator, sizeof(u64));
    should_not_be(0, block);
    should_be(sizeof(u64) * (i + 1), allocator.allocated);
  }

  // Allocate more memory than reserved amount
  block = linear_allocator_alloc(&allocator, sizeof(u64));
  should_be(0, block);
  should_be(sizeof(u64) * max_allocs, allocator.allocated);

  linear_allocator_destroy(&allocator);
  return true;
}

u8 linear_allocator_test_multi_alloc_all_free(void) {
  linear_allocator allocator;
  u64 max_allocs = 1024;
  linear_allocator_create(sizeof(u64) * max_allocs, 0, &allocator);

  void *block;
  for (u64 i = 0; i < max_allocs; ++i) {
    block = linear_allocator_alloc(&allocator, sizeof(u64));
    should_not_be(0, block);
    should_be(sizeof(u64) * (i + 1), allocator.allocated);
  }

  linear_allocator_free(&allocator);
  should_be(0, allocator.allocated);

  linear_allocator_destroy(&allocator);
  return true;
}

void linear_allocator_test_register(void) {
  REGISTER_TEST(linear_allocator_test_create_destroy);
  REGISTER_TEST(linear_allocator_test_single_alloc_all);
  REGISTER_TEST(linear_allocator_test_multi_alloc_all);
  REGISTER_TEST(linear_allocator_test_multi_alloc_overflow);
  REGISTER_TEST(linear_allocator_test_multi_alloc_all_free);
}
