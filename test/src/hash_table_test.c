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
#include <hash_table.h>
#include <test_manager.h>
#include <hash_table_test.h>

typedef struct {
  b8 b;
  u64 u;
  f32 f;
} hash_table_test_struct;

u8 hash_table_test_create_destroy(void) {
  hash_table table;
  u64 element_size = sizeof(u64);
  u64 element_count = 3;
  u64 memory[element_count];

  hash_table_create(element_size,
                    element_count,
                    memory,
                    false,
                    &table);
  should_not_be(0, table.memory);
  should_be(element_size, table.element_size);
  should_be(element_count, table.element_count);

  hash_table_destroy(&table);
  should_be(0, table.memory);
  should_be(0, table.element_size);
  should_be(0, table.element_count);

  return true;
}

u8 hash_table_test_set_get(void) {
  hash_table table;
  u64 element_size = sizeof(u64);
  u64 element_count = 3;
  u64 memory[element_count];

  hash_table_create(element_size,
                    element_count,
                    memory,
                    false,
                    &table);
  should_not_be(0, table.memory);
  should_be(element_size, table.element_size);
  should_be(element_count, table.element_count);

  u64 x = 23;
  hash_table_set(&table, "x", &x);
  u64 returned_x = 0;
  hash_table_get(&table, "x", &returned_x);
  should_be(x, returned_x);

  hash_table_destroy(&table);
  should_be(0, table.memory);
  should_be(0, table.element_size);
  should_be(0, table.element_count);

  return true;
}

u8 hash_table_test_set_get_ptr(void) {
  hash_table table;
  u64 element_size = sizeof(hash_table_test_struct *);
  u64 element_count = 3;
  hash_table_test_struct *memory[element_count];

  hash_table_create(element_size,
                    element_count,
                    memory,
                    true,
                    &table);
  should_not_be(0, table.memory);
  should_be(element_size, table.element_size);
  should_be(element_count, table.element_count);

  hash_table_test_struct ht;
  hash_table_test_struct *x = &ht;
  x->b = true;
  x->u = 63;
  x->f = 3.1415f;
  hash_table_set_ptr(&table, "x", (void **) &x);
  hash_table_test_struct *returned_x = 0;
  hash_table_get_ptr(&table, "x", (void **) &returned_x);

  should_be(x->b, returned_x->b);
  should_be(x->u, returned_x->u);
  float_should_be(x->f, returned_x->f);

  hash_table_destroy(&table);
  should_be(0, table.memory);
  should_be(0, table.element_size);
  should_be(0, table.element_count);

  return true;
}

u8 hash_table_test_set_get_nonexistant(void) {
  hash_table table;
  u64 element_size = sizeof(u64);
  u64 element_count = 3;
  u64 memory[element_count];

  hash_table_create(element_size,
                    element_count,
                    memory,
                    false,
                    &table);
  should_not_be(0, table.memory);
  should_be(element_size, table.element_size);
  should_be(element_count, table.element_count);

  u64 x = 23;
  hash_table_set(&table, "x", &x);

  u64 returned_x = 0;
  hash_table_get(&table, "y", &returned_x);
  should_be(0, returned_x);

  hash_table_destroy(&table);
  should_be(0, table.memory);
  should_be(0, table.element_size);
  should_be(0, table.element_count);

  return true;
}

u8 hash_table_test_set_get_ptr_nonexistant(void) {
  hash_table table;
  u64 element_size = sizeof(hash_table_test_struct *);
  u64 element_count = 3;
  hash_table_test_struct *memory[element_count];

  hash_table_create(element_size,
                    element_count,
                    memory,
                    true,
                    &table);
  should_not_be(0, table.memory);
  should_be(element_size, table.element_size);
  should_be(element_count, table.element_count);

  hash_table_test_struct ht;
  hash_table_test_struct *x = &ht;
  x->b = true;
  x->u = 63;
  x->f = 3.1415f;
  b8 result = hash_table_set_ptr(&table, "x", (void **) &x);
  should_be_true(result);

  hash_table_test_struct *returned_x = 0;
  result = hash_table_get_ptr(&table, "y", (void **) &returned_x);
  should_be_false(result);
  should_be(0, returned_x);

  hash_table_destroy(&table);
  should_be(0, table.memory);
  should_be(0, table.element_size);
  should_be(0, table.element_count);

  return true;
}

u8 hash_table_test_set_unset_ptr(void) {
  hash_table table;
  u64 element_size = sizeof(hash_table_test_struct *);
  u64 element_count = 3;
  hash_table_test_struct *memory[element_count];

  hash_table_create(element_size,
                    element_count,
                    memory,
                    true,
                    &table);
  should_not_be(0, table.memory);
  should_be(element_size, table.element_size);
  should_be(element_count, table.element_count);

  hash_table_test_struct ht;
  hash_table_test_struct *x = &ht;
  x->b = true;
  x->u = 63;
  x->f = 3.1415f;

  b8 result = hash_table_set_ptr(&table, "x", (void **) &x);
  should_be_true(result);

  hash_table_test_struct *returned_x = 0;
  hash_table_get_ptr(&table, "x", (void **) &returned_x);
  should_be(x->b, returned_x->b);
  should_be(x->u, returned_x->u);
  float_should_be(x->f, returned_x->f);

  result = hash_table_set_ptr(&table, "x", 0);
  should_be_true(result);

  hash_table_test_struct *returned_x_2 = 0;
  result = hash_table_get_ptr(&table, "x", (void **) &returned_x_2);
  should_be_false(result);
  should_be(0, returned_x_2);

  hash_table_destroy(&table);
  should_be(0, table.memory);
  should_be(0, table.element_size);
  should_be(0, table.element_count);

  return true;
}

u8 hash_table_test_call_non_ptr_funcs_on_ptr_table(void) {
  hash_table table;
  u64 element_size = sizeof(hash_table_test_struct *);
  u64 element_count = 3;
  hash_table_test_struct *memory[element_count];

  hash_table_create(element_size,
                    element_count,
                    memory,
                    true,
                    &table);
  should_not_be(0, table.memory);
  should_be(element_size, table.element_size);
  should_be(element_count, table.element_count);

  hash_table_test_struct x = {
    .b = true,
    .u = 63,
    .f = 3.1415f
  };
  b8 result = hash_table_set(&table, "x", &x);
  should_be_false(result);
  hash_table_test_struct *returned_x = 0;
  result = hash_table_get(&table, "x", (void **) &returned_x);
  should_be_false(result);

  hash_table_destroy(&table);
  should_be(0, table.memory);
  should_be(0, table.element_size);
  should_be(0, table.element_count);

  return true;
}

u8 hash_table_test_call_ptr_funcs_on_non_ptr_table(void) {
  hash_table table;
  u64 element_size = sizeof(hash_table_test_struct);
  u64 element_count = 3;
  hash_table_test_struct memory[element_count];

  hash_table_create(element_size,
                    element_count,
                    memory,
                    false,
                    &table);
  should_not_be(0, table.memory);
  should_be(element_size, table.element_size);
  should_be(element_count, table.element_count);

  hash_table_test_struct ht;
  hash_table_test_struct *x = &ht;
  x->b = true;
  x->u = 63;
  x->f = 3.1415f;

  b8 result = hash_table_set_ptr(&table, "x", (void **) &x);
  should_be_false(result);
  hash_table_test_struct *returned_x = 0;
  result = hash_table_get_ptr(&table, "x", (void **) &returned_x);
  should_be_false(result);

  hash_table_destroy(&table);
  should_be(0, table.memory);
  should_be(0, table.element_size);
  should_be(0, table.element_count);

  return true;
}

u8 hash_table_test_set_get_update_ptr(void) {
  hash_table table;
  u64 element_size = sizeof(hash_table_test_struct *);
  u64 element_count = 3;
  hash_table_test_struct *memory[element_count];

  hash_table_create(element_size,
                    element_count,
                    memory,
                    true,
                    &table);
  should_not_be(0, table.memory);
  should_be(element_size, table.element_size);
  should_be(element_count, table.element_count);

  hash_table_test_struct ht;
  hash_table_test_struct *x = &ht;
  x->b = true;
  x->u = 63;
  x->f = 3.1415f;
  hash_table_set_ptr(&table, "x", (void **) &x);
  hash_table_test_struct *returned_x = 0;
  hash_table_get_ptr(&table, "x", (void **) &returned_x);

  should_be(x->b, returned_x->b);
  should_be(x->u, returned_x->u);
  float_should_be(x->f, returned_x->f);

  // Update
  returned_x->b = false;
  returned_x->u = 99;
  returned_x->f = 6.69f;

  hash_table_test_struct *returned_x_2 = 0;
  hash_table_get_ptr(&table, "x", (void **) &returned_x_2);
  should_be_false(returned_x_2->b);
  should_be(99, returned_x_2->u);
  float_should_be(6.69f, returned_x->f);

  hash_table_destroy(&table);
  should_be(0, table.memory);
  should_be(0, table.element_size);
  should_be(0, table.element_count);

  return true;
}

void hash_table_test_register(void) {
  REGISTER_TEST(hash_table_test_create_destroy);
  REGISTER_TEST(hash_table_test_set_get);
  REGISTER_TEST(hash_table_test_set_get_ptr);
  REGISTER_TEST(hash_table_test_set_get_nonexistant);
  REGISTER_TEST(hash_table_test_set_get_ptr_nonexistant);
  REGISTER_TEST(hash_table_test_set_unset_ptr);
  REGISTER_TEST(hash_table_test_call_non_ptr_funcs_on_ptr_table);
  REGISTER_TEST(hash_table_test_call_ptr_funcs_on_non_ptr_table);
  REGISTER_TEST(hash_table_test_set_get_update_ptr);
}
