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


#include <clock.h>
#include <expect.h>
#include <unistd.h>
#include <clock_test.h>
#include <test_manager.h>

u8 clock_test_1s_elapsed(void) {
  clock clk;
  clock_start(&clk);
  float_should_be(0, clk.elapsed);
  float_should_not_be(0, clk.start);
  sleep(1);
  clock_update(&clk);
  clock_stop(&clk);
  float_should_be(1, clk.elapsed);
  float_should_be(0, clk.start);
  return true;
}

void clock_test_register(void) {
  REGISTER_TEST(clock_test_1s_elapsed);
}
