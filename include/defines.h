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

// Unsigned integer types
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// Signed integer types
typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

// Floating point types
typedef float f32;
typedef double f64;

// Boolean types
typedef char b8;
typedef int b32;

// Static assertions definition
#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif  // defined(__clang__) || defined(__gcc__)

// Check size of unsigned integer types
STATIC_ASSERT(sizeof(u8) == 1, "ERROR: u8 :: 1B");
STATIC_ASSERT(sizeof(u16) == 2, "ERROR: u16 :: 2B");
STATIC_ASSERT(sizeof(u32) == 4, "ERROR: u32 :: 4B");
STATIC_ASSERT(sizeof(u64) == 8, "ERROR: u64 :: 8B");

// Check size of signed integer types
STATIC_ASSERT(sizeof(i8) == 1, "ERROR: i8 :: 1B");
STATIC_ASSERT(sizeof(i16) == 2, "ERROR: i16 :: 2B");
STATIC_ASSERT(sizeof(i32) == 4, "ERROR: i32 :: 4B");
STATIC_ASSERT(sizeof(i64) == 8, "ERROR: i64 :: 8B");

// Check size of floating point types
STATIC_ASSERT(sizeof(f32) == 4, "ERROR: f32 :: 4B");
STATIC_ASSERT(sizeof(f64) == 8, "ERROR: f64 :: 8B");

// Boolean values
#define TRUE 1
#define FALSE 0

// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
// Windows
#define KPLATFORM_WINDOWS 1
#ifndef _WIN64
#error "ERROR: 64-bit is required on Windows"
#endif  // _WIN64
#elif defined(__linux__) || defined(__gnu_linux__)
// Linux | GNU/Linux
#define KPLATFORM_LINUX 1
#if defined(__ANDROID__)
// Android
#define KPLATFORM_ANDROID 1
#endif  // defined(__ANDROID__)
#elif defined(__unix__)
// General UNIX-like OS
#define KPLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
// POSIX compliant OS
#elif __APPLE__
// General Apple OS
#define KPLATFORM_APPLE 1
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
// iOS simulator
#define KPLATFORM_IOS 1
#define KPLATFORM_IOS_SIMULATOR 1
#elif TARGET_OS_IPHONE
// iOS
#define KPLATFORM_IOS 1
#elif TARGET_OS_MAC
// macOS
#else
#error "ERROR: Unknown Apple platform"
#endif
#else
#error "ERROR: Unknown platform"
#endif

#ifdef KEXPORT
// Exports
#ifdef _MSC_VER
#define KAPI __declspec(dllimport)
#else
#define KAPI __attribute__((visibility("default")))
#endif  // _MSC_VER
#else
// Imports
#ifdef _MSC_VER
#define KAPI __declspec(dllimport)
#else
#define KAPI
#endif  // _MSC_VER
#endif  // KEXPORT
