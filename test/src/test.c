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
#include <clock_test.h>
#include <test_manager.h>
#include <kstring_test.h>
#include <free_list_test.h>
#include <hash_table_test.h>
#include <linear_allocator_test.h>

int main(void) {
  test_manager_init();

  clock_test_register();
  kstring_test_register();
  free_list_test_register();
  hash_table_test_register();
  linear_allocator_test_register();

  test_manager_run();
  return 0;
}
