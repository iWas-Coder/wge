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

#include <kmath.h>
#include <logger.h>

#define should_be(expected, actual)                             \
  {                                                             \
    if (expected != actual) {                                    \
      KERROR("%s:%d :: FAILED -> got '%lld', expected '%lld'",  \
             __FILE__,                                          \
             __LINE__,                                          \
             actual,                                            \
             expected);                                         \
      return false;                                             \
    }                                                           \
  }

#define should_not_be(expected, actual)                                 \
  {                                                                     \
    if (expected == actual) {                                            \
      KERROR("%s:%d :: FAILED -> got '%lld == %lld', expected not to",  \
             __FILE__,                                                  \
             __LINE__,                                                  \
             actual,                                                    \
             expected);                                                 \
      return false;                                                     \
    }                                                                   \
  }

#define float_should_be(expected, actual)                       \
  {                                                             \
    if (kabs(expected - actual) > 0.001f) {                     \
      KERROR("%s:%d :: FAILED -> got '%f', expected '%f'",      \
             __FILE__,                                          \
             __LINE__,                                          \
             actual,                                            \
             expected);                                         \
      return false;                                             \
    }                                                           \
  }

#define float_should_not_be(expected, actual)                   \
  {                                                             \
    if (kabs(expected - actual) <= 0.001f) {                     \
      KERROR("%s:%d :: FAILED -> got '%f', expected '%f'",      \
             __FILE__,                                          \
             __LINE__,                                          \
             actual,                                            \
             expected);                                         \
      return false;                                             \
    }                                                           \
  }

#define should_be_true(actual)                                  \
  {                                                             \
    if (actual != true) {                                        \
      KERROR("%s:%d :: FAILED -> got 'false', expected 'true'", \
             __FILE__,                                          \
             __LINE__);                                         \
      return false;                                             \
    }                                                           \
  }

#define should_be_false(actual)                                 \
  {                                                             \
    if (actual != false) {                                       \
      KERROR("%s:%d :: FAILED -> got 'true', expected 'false'", \
             __FILE__,                                          \
             __LINE__);                                         \
      return false;                                             \
    }                                                           \
  }
