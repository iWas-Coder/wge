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
#include <kmemory.h>
#include <free_list.h>
#include <test_manager.h>
#include <free_list_test.h>

u8 free_list_test_create_destroy(void) {
  free_list fl;

  u64 memory_requirements = 0;
  u64 total_size = 40;  // It should throw a warning saying that it is inefficient.
  free_list_create(total_size, &memory_requirements, 0, 0);

  void *block = kallocate(memory_requirements, MEMORY_TAG_APPLICATION);
  free_list_create(total_size, &memory_requirements, block, &fl);
  should_not_be(0, fl.memory);
  should_be(total_size, free_list_get_free_space(&fl));

  free_list_destroy(&fl);
  should_be(0, fl.memory);
  kfree(block, memory_requirements, MEMORY_TAG_APPLICATION);

  return true;
}

u8 free_list_test_alloc_free(void) {
  free_list fl;

  u64 memory_requirements = 0;
  u64 total_size = 512;
  free_list_create(total_size, &memory_requirements, 0, 0);

  void *block = kallocate(memory_requirements, MEMORY_TAG_APPLICATION);
  free_list_create(total_size, &memory_requirements, block, &fl);

  u32 offset = INVALID_ID;
  should_be_true(free_list_alloc(&fl, 64, &offset));
  should_be(0, offset);
  should_be(total_size - 64, free_list_get_free_space(&fl));

  should_be_true(free_list_free(&fl, 64, offset));
  should_be(total_size, free_list_get_free_space(&fl));

  free_list_destroy(&fl);
  should_be(0, fl.memory);
  kfree(block, memory_requirements, MEMORY_TAG_APPLICATION);

  return true;
}

u8 free_list_test_alloc_free_multi(void) {
  free_list fl;

  u64 memory_requirements = 0;
  u64 total_size = 512;
  free_list_create(total_size, &memory_requirements, 0, 0);

  void *block = kallocate(memory_requirements, MEMORY_TAG_APPLICATION);
  free_list_create(total_size, &memory_requirements, block, &fl);

  u32
    offset1 = INVALID_ID,
    offset2 = INVALID_ID,
    offset3 = INVALID_ID,
    offset4 = INVALID_ID;

  should_be_true(free_list_alloc(&fl, 64, &offset1));
  should_be(0, offset1);
  should_be_true(free_list_alloc(&fl, 64, &offset2));
  should_be(64, offset2);
  should_be_true(free_list_alloc(&fl, 64, &offset3));
  should_be(128, offset3);
  should_be(total_size - 192, free_list_get_free_space(&fl));

  should_be_true(free_list_free(&fl, 64, offset2));
  should_be(total_size - 128, free_list_get_free_space(&fl));

  should_be_true(free_list_alloc(&fl, 64, &offset4));
  should_be(offset2, offset4);
  should_be(total_size - 192, free_list_get_free_space(&fl));

  should_be_true(free_list_free(&fl, 64, offset1));
  should_be(total_size - 128, free_list_get_free_space(&fl));
  should_be_true(free_list_free(&fl, 64, offset3));
  should_be(total_size - 64, free_list_get_free_space(&fl));
  should_be_true(free_list_free(&fl, 64, offset4));
  should_be(total_size, free_list_get_free_space(&fl));

  free_list_destroy(&fl);
  should_be(0, fl.memory);
  kfree(block, memory_requirements, MEMORY_TAG_APPLICATION);

  return true;
}

u8 free_list_test_alloc_free_multi_diff_sizes(void) {
  free_list fl;

  u64 memory_requirements = 0;
  u64 total_size = 512;
  free_list_create(total_size, &memory_requirements, 0, 0);

  void *block = kallocate(memory_requirements, MEMORY_TAG_APPLICATION);
  free_list_create(total_size, &memory_requirements, block, &fl);

  u32
    offset1 = INVALID_ID,
    offset2 = INVALID_ID,
    offset3 = INVALID_ID,
    offset4 = INVALID_ID;

  should_be_true(free_list_alloc(&fl, 64, &offset1));
  should_be(0, offset1);
  should_be_true(free_list_alloc(&fl, 32, &offset2));
  should_be(64, offset2);
  should_be_true(free_list_alloc(&fl, 64, &offset3));
  should_be(96, offset3);
  should_be(total_size - 160, free_list_get_free_space(&fl));

  should_be_true(free_list_free(&fl, 32, offset2));
  should_be(total_size - 128, free_list_get_free_space(&fl));

  should_be_true(free_list_alloc(&fl, 64, &offset4));
  should_be(160, offset4);
  should_be(total_size - 192, free_list_get_free_space(&fl));

  should_be_true(free_list_free(&fl, 64, offset1));
  should_be(total_size - 128, free_list_get_free_space(&fl));
  should_be_true(free_list_free(&fl, 64, offset3));
  should_be(total_size - 64, free_list_get_free_space(&fl));
  should_be_true(free_list_free(&fl, 64, offset4));
  should_be(total_size, free_list_get_free_space(&fl));

  free_list_destroy(&fl);
  should_be(0, fl.memory);
  kfree(block, memory_requirements, MEMORY_TAG_APPLICATION);

  return true;
}

u8 free_list_test_alloc_full_and_fail_to_alloc_more(void) {
  free_list fl;

  u64 memory_requirements = 0;
  u64 total_size = 512;
  free_list_create(total_size, &memory_requirements, 0, 0);

  void *block = kallocate(memory_requirements, MEMORY_TAG_APPLICATION);
  free_list_create(total_size, &memory_requirements, block, &fl);

  u32 offset1 = INVALID_ID, offset2 = INVALID_ID;

  should_be_true(free_list_alloc(&fl, total_size, &offset1));
  should_be(0, offset1);
  should_be(0, free_list_get_free_space(&fl));

  should_be_false(free_list_alloc(&fl, 64, &offset2));
  should_be(INVALID_ID, offset2);
  should_be(0, free_list_get_free_space(&fl));

  free_list_destroy(&fl);
  should_be(0, fl.memory);
  kfree(block, memory_requirements, MEMORY_TAG_APPLICATION);

  return true;
}

void free_list_test_register(void) {
  REGISTER_TEST(free_list_test_create_destroy);
  REGISTER_TEST(free_list_test_alloc_free);
  REGISTER_TEST(free_list_test_alloc_free_multi);
  REGISTER_TEST(free_list_test_alloc_free_multi_diff_sizes);
  REGISTER_TEST(free_list_test_alloc_full_and_fail_to_alloc_more);
}
