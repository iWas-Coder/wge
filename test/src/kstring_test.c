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
#include <kstring.h>
#include <kstring_test.h>
#include <test_manager.h>

u8 kstring_test_kstrlen(void) {
  should_be(13, kstrlen("hello, world!"));
  return true;
}

u8 kstring_test_kstrdup(void) {
  char *str = "hello, world!";
  char *str_cp = kstrdup(str);
  should_be_true(kstrcmp(str, str_cp));
  return true;
}

u8 kstring_test_kstrcmp(void) {
  should_be_true(kstrcmp("hello, world!", "hello, world!"));
  should_be_false(kstrcmp("hello, world!", "hello, dev!"));
  should_be_false(kstrcmp("hello, world!", "Hello, World!"));
  return true;
}

u8 kstring_test_kstrcmpi(void) {
  should_be_true(kstrcmpi("hello, world!", "hello, world!"));
  should_be_false(kstrcmpi("hello, world!", "hello, dev!"));
  should_be_true(kstrcmpi("hello, world!", "Hello, World!"));
  return true;
}

u8 kstring_test_kstrfmt(void) {
  u32 len = 13;
  char dest[len + 1];
  kzero_memory(dest, sizeof(char) * (len + 1));
  i32 n = kstrfmt(dest, "Hello, %s!", "World");
  should_be_true(kstrcmp(dest, "Hello, World!"));
  should_be(len, (u32) n);
  return true;
}

u8 kstring_test_kstrcp(void) {
  const char *src = "Hello, World!";
  char dest[14];
  kzero_memory(dest, sizeof(char) * 14);
  should_be_true(kstrcmp(kstrcp(dest, src), src));
  should_be_true(kstrcmp(dest, src));
  return true;
}

u8 kstring_test_kstrncp(void) {
  const char *src = "Hello, World!";
  char dest[14];
  kzero_memory(dest, sizeof(char) * 14);
  should_be_true(kstrcmp(kstrncp(dest, src, 5), "Hello"));
  should_be_true(kstrcmp(dest, "Hello"));
  should_be_false(kstrcmp(dest, src));
  return true;
}

u8 kstring_test_kstrtr(void) {
  char str[] = "  Hello, World!  ";
  should_be_true(kstrcmp(kstrtr(str), "Hello, World!"));
  return true;
}

u8 kstring_test_kstrsub_start(void) {
  const char *str = "Hello John, how are you?";
  u32 len = 5;
  char dest[len + 1];
  kzero_memory(dest, sizeof(char) * (len + 1));
  kstrsub(dest, str, 0, len);
  should_be_true(kstrcmp(dest, "Hello"));
  return true;
}

u8 kstring_test_kstrsub_mid(void) {
  const char *str = "Hello John, how are you?";
  u32 len = 4;
  char dest[len + 1];
  kzero_memory(dest, sizeof(char) * (len + 1));
  kstrsub(dest, str, 6, len);
  should_be_true(kstrcmp(dest, "John"));
  return true;
}

u8 kstring_test_kstrsub_end(void) {
  const char *str = "Hello John, how are you?";
  u32 len = 12;
  char dest[len + 1];
  kzero_memory(dest, sizeof(char) * (len + 1));
  kstrsub(dest, str, len, len);
  should_be_true(kstrcmp(dest, "how are you?"));
  return true;
}

u8 kstring_test_kstridx(void) {
  char *str = "Hello, World!";
  should_be(0, kstridx(str, 'H'));
  should_be(5, kstridx(str, ','));
  should_be(12, kstridx(str, '!'));
  return true;
}

u8 kstring_test_kstrrm(void) {
  char str[] = "Hello, World!";
  should_be_true(kstrcmp(kstrrm(str), ""));
  return true;
}

void kstring_test_register(void) {
  REGISTER_TEST(kstring_test_kstrlen);
  REGISTER_TEST(kstring_test_kstrdup);
  REGISTER_TEST(kstring_test_kstrcmp);
  REGISTER_TEST(kstring_test_kstrcmpi);
  REGISTER_TEST(kstring_test_kstrfmt);
  REGISTER_TEST(kstring_test_kstrcp);
  REGISTER_TEST(kstring_test_kstrncp);
  REGISTER_TEST(kstring_test_kstrtr);
  REGISTER_TEST(kstring_test_kstrsub_start);
  REGISTER_TEST(kstring_test_kstrsub_mid);
  REGISTER_TEST(kstring_test_kstrsub_end);
  REGISTER_TEST(kstring_test_kstridx);
  REGISTER_TEST(kstring_test_kstrrm);
}
