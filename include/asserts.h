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

#define KASSERTIONS_ENABLED

#ifdef KASSERTIONS_ENABLED
#if _MSC_VER
// MSVC-specific halt function
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
// GCC-specific halt function
#define debugBreak() __builtin_trap()
#endif

KAPI void report_assertion_failure(const char *expression, const char *message, const char *file, i32 line);

#define KASSERT(expr)                                           \
  {                                                             \
    if (expr) {}                                                \
    else {                                                      \
      report_assertion_failure(#expr, "", __FILE__, __LINE__);  \
      debugBreak();                                             \
    }                                                           \
  }

#define KASSERT_MSG(expr, message)                                      \
  {                                                                     \
    if (expr) {}                                                        \
    else {                                                              \
      report_assertion_failure(#expr, message, __FILE__, __LINE__);     \
      debugBreak();                                                     \
    }                                                                   \
  }

#ifdef _DEBUG
#define KASSERT_DEBUG(expr)                                     \
  {                                                             \
    if (expr) {}                                                \
    else {                                                      \
      report_assertion_failure(#expr, "", __FILE__, __LINE__);  \
      debugBreak();                                             \
    }                                                           \
  }
#else
#define KASSERT_DEBUG(expr)
#endif

#else
#define KASSERT(expr)
#define KASSERT_MSG(expr, message)
#define KASSERT_DEBUG(expr)
#endif
