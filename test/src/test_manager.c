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
#include <logger.h>
#include <darray.h>
#include <kstring.h>
#include <test_manager.h>

#define CPP_STRINGIFY(x) (#x)

typedef struct {
  PFN_test func;
  char *desc;
} test_entry;

static test_entry *tests;

void test_manager_init(void) {
  tests = darray_create(test_entry);
}

void test_manager_register(PFN_test func, char *desc) {
  test_entry e = {
    .func = func,
    .desc = desc
  };
  darray_push(tests, e);
}

void test_manager_run(void) {
  u32 passed  = 0;
  u32 skipped = 0;
  u32 failed  = 0;
  u32 n = darray_length(tests);
  clock total_time;

  clock_start(&total_time);
  for (u32 i = 0; i < n; ++i) {
    clock test_time;
    clock_start(&test_time);
    u8 result = tests[i].func();
    clock_update(&test_time);
    if (result == true) {
      KINFO("(%d/%d) %s :: PASSED", i + 1, n, tests[i].desc);
      ++passed;
    }
    else if (result == PASS) {
      KWARN("(%d/%d) %s :: SKIPPED", i + 1, n, tests[i].desc);
      ++skipped;
    }
    else {
      KERROR("(%d/%d) %s :: FAILED", i + 1, n, tests[i].desc);
      ++failed;
    }
    clock_update(&total_time);
    // char status[STATUS_MAX_LEN];
    // kstrfmt(status, failed ? "FAILED" : "PASSED");
    // KINFO("Ran %d/%d (%d skipped) %s :: %.6fs / %.6fs", i + 1, n, skipped, status, test_time.elapsed, total_time.elapsed);
  }
  clock_stop(&total_time);
  if (failed) {
    KERROR("=========== %d failed, %d passed (%d skipped) in %.2fs ===========",
           failed, passed, skipped, total_time.elapsed);
  }
  else {
    KINFO("=========== %d passed (%d skipped) in %.2fs ===========",
          passed, skipped, total_time.elapsed);
  }
}
