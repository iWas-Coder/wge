#
# GNU WGE --- Wildebeest Game Engine™
# Copyright (C) 2023 Wasym A. Alonso
#
# This file is part of WGE.
#
# WGE is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# WGE is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with WGE.  If not, see <http://www.gnu.org/licenses/>.
#


##################
# === MACROS === #
##################
# Version info
VERSION       = 0
PATCHLEVEL    = 1
SUBLEVEL      = 0
EXTRAVERSION  =
NAME = Greased Wildebeest

# Dev version string appended if no EXTRAVERSION is defined
#   - If Git repo is present => '-git+<COMMIT_HASH>'
#   - If not                 => '-dev'
ifndef EXTRAVERSION
  EXTRAVERSION += $(or $(and $(wildcard .git/), -git+$$(git rev-parse --short HEAD)), -dev)
endif
# Version formatting
ifeq ($(SUBLEVEL), 0)
  FULL_VERSION = $(VERSION).$(PATCHLEVEL)$(EXTRAVERSION)
else
  FULL_VERSION = $(VERSION).$(PATCHLEVEL).$(SUBLEVEL)$(EXTRAVERSION)
endif

# Verbose control
Q = @
# Pretty Printing Output (PPO) [17C]
PPO_INSTALL = INSTALL
PPO_MKDIR   = MKDIR
PPO_CLEAN   = CLEAN
PPO_GLSLC   = GLSLC
PPO_SYNC    = SYNC
PPO_GZIP    = GZIP
PPO_TAR     = TAR
PPO_ZIP     = ZIP
PPO_GEN     = GEN
PPO_CC      = CC
PPO_LD      = LD
PPO_LN      = LN

# Architecture selector
ARCH ?= x86_64
ifneq ($(ARCH), x86_64)
  $(error Specified arch is not supported. See 'make help' for more details)
endif

# Platform selector
TARGET ?= linux
ifeq ($(TARGET), linux)
  BUILD_DIR      = $(BUILD_DIR_LINUX)
  TEST_BUILD_DIR = $(TEST_BUILD_DIR_LINUX)
  WGE_OUT        = $(WGE_OUT_LINUX)
  TEST_OUT       = $(TEST_OUT_LINUX)
  CC             = $(CC_LINUX)
  CPPFLAGS       = $(CPPFLAGS_LINUX)
  CFLAGS         = $(CFLAGS_LINUX)
  CFLAGS_TEST    = $(CFLAGS_LINUX_TEST)
  LDFLAGS        = $(LDFLAGS_LINUX)
  LDFLAGS_TEST   = $(LDFLAGS_LINUX_TEST)
else ifeq ($(TARGET), windows)
  BUILD_DIR      = $(BUILD_DIR_WIN)
  TEST_BUILD_DIR = $(TEST_BUILD_DIR_WIN)
  WGE_OUT        = $(WGE_OUT_WIN)
  TEST_OUT       = $(TEST_OUT_WIN)
  CC             = $(CC_WIN)
  CPPFLAGS       = $(CPPFLAGS_WIN)
  CFLAGS         = $(CFLAGS_WIN)
  CFLAGS_TEST    = $(CFLAGS_WIN_TEST)
  LDFLAGS        = $(LDFLAGS_WIN)
  LDFLAGS_TEST   = $(LDFLAGS_WIN_TEST)
else
  $(error Specified target is not supported. See 'make help' for more details)
endif

# Dependency check
# DEPS      := git $(CC)
# DEPS_EXEC := $(foreach dep, $(DEPS),                                    \
#              $(if $(shell command -v $(dep) 2>/dev/null),               \
#              $(info checking for $(dep)... $(shell command -v $(dep))), \
#              $(error checking for $(dep)... no)))

# Prefix (install) default path
PREFIX ?= /usr/local

# C default standard
CSTD ?= gnu17
ifneq (gnu, $(findstring gnu, $(CSTD)))
  $(error Specified C standard is not supported. See 'make help' for more details)
endif

# Directories
SRC_DIR              = src
HDR_DIR              = include
TEST_DIR             = test
SHADERS_DIR          = shaders
VENDOR_DIR           = vendor
BUILD_DIR_PARENT     = build
BUILD_DIR_LINUX      = $(BUILD_DIR_PARENT)/linux
BUILD_DIR_WIN        = $(BUILD_DIR_PARENT)/windows
TEST_BUILD_DIR_LINUX = $(BUILD_DIR_PARENT)/$(TEST_DIR)/linux
TEST_BUILD_DIR_WIN   = $(BUILD_DIR_PARENT)/$(TEST_DIR)/windows
SHADERS_BUILD_DIR    = $(BUILD_DIR_PARENT)/shaders

# Files
ETAGS_XREF    = TAGS
HDRS         := $(wildcard $(HDR_DIR)/*.h)
TEST_HDRS    := $(wildcard $(TEST_DIR)/$(HDR_DIR)/*.h)
SRCS         := $(wildcard $(SRC_DIR)/*.c)
TEST_SRCS    := $(wildcard $(TEST_DIR)/$(SRC_DIR)/*.c)
SHADERS_SRCS := $(wildcard $(SHADERS_DIR)/*.glsl)
OBJS         := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
TEST_OBJS    := $(patsubst $(TEST_DIR)/$(SRC_DIR)/%.c, $(TEST_BUILD_DIR)/%.o, $(TEST_SRCS))
SHADERS_SPVS := $(patsubst $(SHADERS_DIR)/%.glsl, $(SHADERS_BUILD_DIR)/%.spv, $(SHADERS_SRCS))

# Build flags: Common
GLSL_CC = glslc
CPPFLAGS_COMMON = -I $(HDR_DIR) -I $(TEST_DIR)/$(HDR_DIR) -I $(VENDOR_DIR)
### DEBUG version
ifndef RELEASE
  CPP_MACROS_COMMON   = -DKEXPORT -D_DEBUG
  define CFLAGS_COMMON
    -std=$(CSTD)                    \
    -Wall -Wextra -pedantic -Werror \
    -fanalyzer                      \
    -ggdb                           \
    -Og
  endef
  LDFLAGS_COMMON      = -Wl,--build-id -shared
  LDFLAGS_COMMON_TEST = -Wl,--build-id
### RELEASE version
else
  RELEASE_OPTS        = -O3 -fipa-pta -fuse-linker-plugin -flto=auto
  CPP_MACROS_COMMON   = -DKEXPORT
  define CFLAGS_COMMON
    -std=$(CSTD)                    \
    -Wall -Wextra -pedantic -Werror \
    -fanalyzer                      \
    -pipe                           \
    -march=$(subst _,-,$(ARCH))     \
    -mtune=generic                  \
    $(RELEASE_OPTS)
  endef
  LDFLAGS_COMMON      = -Wl,--build-id -shared -s $(RELEASE_OPTS)
  LDFLAGS_COMMON_TEST = -Wl,--build-id -s $(RELEASE_OPTS)
endif
### Disable specific compiler warnings
CFLAGS_COMMON += -Wno-gnu-zero-variadic-macro-arguments
CFLAGS_COMMON += -Wno-language-extension-token

# Build flags: Linux
CC_LINUX           = gcc
CPP_MACROS_LINUX   = $(CPP_MACROS_COMMON)
CPPFLAGS_LINUX     = $(CPP_MACROS_LINUX) $(CPPFLAGS_COMMON)
CFLAGS_LINUX       = $(CFLAGS_COMMON) -fPIC
CFLAGS_LINUX_TEST  = $(CFLAGS_COMMON)
LDFLAGS_LINUX      = $(LDFLAGS_COMMON)
LDFLAGS_LINUX_TEST = $(LDFLAGS_COMMON_TEST) -L. -lwge -lxcb -lX11 -lX11-xcb -lvulkan

# Build flags: Windows
CC_WIN           = x86_64-w64-mingw32-gcc
CPP_MACROS_WIN   = $(CPP_MACROS_COMMON) -D_CRT_SECURE_NO_WARNINGS
CPPFLAGS_WIN     = $(CPP_MACROS_WIN) $(CPPFLAGS_COMMON) -I /usr/include
CFLAGS_WIN       = $(CFLAGS_COMMON)
CFLAGS_WIN_TEST  = $(CFLAGS_COMMON)
LDFLAGS_WIN      = $(LDFLAGS_COMMON)
LDFLAGS_WIN_TEST = $(LDFLAGS_COMMON_TEST) -L. -lwge -luser32 -lvulkan-1

# Build configuration
CFG_FILE = .config
DIFF_CFG = __diff_cfg__

# Build output
OUTS = $(CFG_FILE) $(WGE_OUT) $(TEST_OUT)
### 'wge' target output
WGE_OUT_LINUX = libwge.so
WGE_OUT_WIN   = wge.dll
### 'check' target output
TEST_OUT_LINUX = $(TEST_DIR)/test
TEST_OUT_WIN   = $(TEST_DIR)/test.exe
### 'shaders' target output
SHADERS_OUT = $(SHADERS_SPVS)

# Build targets
TGTS     = wge shaders
DIR_TGTS = $(BUILD_DIR) $(TEST_BUILD_DIR) $(SHADERS_BUILD_DIR)
ALL_TGTS = $(TGTS) $(ETAGS_XREF)


###################
# === TARGETS === #
###################
.PHONY: all $(TGTS) check install dist clean mrproper version help $(DIFF_CFG)

all: $(ALL_TGTS)
	@:

wge: $(BUILD_DIR) $(DIFF_CFG) $(WGE_OUT)
	@echo "Engine: $(WGE_OUT) is ready  ($(FULL_VERSION))"

check: $(TEST_BUILD_DIR) $(DIFF_CFG) $(TEST_OUT)
	@LD_LIBRARY_PATH=$$(pwd):$$LD_LIBRARY_PATH ./$(TEST_OUT)

shaders: $(SHADERS_BUILD_DIR) $(SHADERS_OUT)
	@:

# ********************** 'TAGS' ********************** #
$(ETAGS_XREF): $(SRCS) $(HDRS)
	@if [ -x "$$(command -v etags)" ]; then  \
	  echo "  $(PPO_GEN)     $@";            \
	  etags $$(find -type f -name "*.[ch]"); \
	fi
# **************************************************** #

# ******************* directories ******************** #
$(BUILD_DIR):
	@echo "  $(PPO_MKDIR)   $@"
	@mkdir -p $@

$(TEST_BUILD_DIR):
	@echo "  $(PPO_MKDIR)   $@"
	@mkdir -p $@

$(SHADERS_BUILD_DIR):
	@printf "  $(PPO_MKDIR)   $@"
	@mkdir -p $@
# **************************************************** #

# ***************** '.config': check ***************** #
$(DIFF_CFG):
	@if [ -f $(CFG_FILE) ]; then                                                    \
	  if [ "$(ARCH)" != "$$(sed -n '1p' $(CFG_FILE) | tr -d '\n')" ]; then           \
	    echo "  $(PPO_SYNC)    $(ARCH)";                                            \
	    sed -i "s/$$(sed -n '1p' $(CFG_FILE) | tr -d '\n')/$(ARCH)/" $(CFG_FILE);   \
	  fi;                                                                           \
	  if [ "$(TARGET)" != "$$(sed -n '2p' $(CFG_FILE) | tr -d '\n')" ]; then         \
	    echo "  $(PPO_SYNC)    $(TARGET)";                                          \
	    sed -i "s/$$(sed -n '2p' $(CFG_FILE) | tr -d '\n')/$(TARGET)/" $(CFG_FILE); \
	  fi;                                                                           \
	  if [ -z "$(RELEASE)" ]; then                                                  \
	    if [ "debug" != "$$(sed -n '3p' $(CFG_FILE) | tr -d '\n')" ]; then           \
	      echo "  $(PPO_SYNC)    debug";                                            \
	      sed -i "s/$$(sed -n '3p' $(CFG_FILE) | tr -d '\n')/debug/" $(CFG_FILE);   \
	    fi;                                                                         \
	  else                                                                          \
	    if [ "release" != "$$(sed -n '3p' $(CFG_FILE) | tr -d '\n')" ]; then         \
	      echo "  $(PPO_SYNC)    release";                                          \
	      sed -i "s/$$(sed -n '3p' $(CFG_FILE) | tr -d '\n')/release/" $(CFG_FILE); \
	    fi;                                                                         \
	  fi;                                                                           \
	  if [ "$(PREFIX)" != "$$(sed -n '4p' $(CFG_FILE) | tr -d '\n')" ]; then         \
	    echo "  $(PPO_SYNC)    $(PREFIX)";                                          \
	    sed -i "s/$$(sed -n '4p' $(CFG_FILE) | tr -d '\n')/$(PREFIX)/" $(CFG_FILE); \
	  fi;                                                                           \
	  if [ "$(CSTD)" != "$$(sed -n '5p' $(CFG_FILE) | tr -d '\n')" ]; then           \
	    echo "  $(PPO_SYNC)    $(CSTD)";                                            \
	    sed -i "s/$$(sed -n '5p' $(CFG_FILE) | tr -d '\n')/$(CSTD)/" $(CFG_FILE);   \
	  fi;                                                                           \
	else                                                                            \
	  make --no-print-directory $(CFG_FILE);                                        \
	fi
# **************************************************** #

# ***************** '.config': check ***************** #
$(CFG_FILE):
	@echo "  $(PPO_GEN)     $@"
	@echo "  $(PPO_SYNC)    $(ARCH)"
	@printf "$(ARCH)\n"    > $@
	@echo "  $(PPO_SYNC)    $(TARGET)"
	@printf "$(TARGET)\n" >> $@
	@if [ -z "$(RELEASE)" ]; then      \
	  echo "  $(PPO_SYNC)    debug";   \
	  printf "debug\n"    >> $@;       \
	else                               \
	  echo "  $(PPO_SYNC)    release"; \
	  printf "release\n"  >> $@;       \
	fi
	@echo "  $(PPO_SYNC)    $(PREFIX)"
	@printf "$(PREFIX)\n" >> $@
	@echo "  $(PPO_SYNC)    $(CSTD)"
	@printf "$(CSTD)\n"   >> $@
# **************************************************** #

# ******************* 'wge': link ******************** #
$(WGE_OUT): $(OBJS)
	@echo "  $(PPO_LD)      $@"
	@$(CC) $^ $(LDFLAGS) -o $@
# **************************************************** #

# ****************** 'check': link ******************* #
$(TEST_OUT): $(TEST_OBJS)
	@echo "  $(PPO_LD)      $@"
	@$(CC) $^ $(LDFLAGS_TEST) -o $@
# **************************************************** #

# ************ 'wge': compile & assembly ************* #
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(CFG_FILE)
	@echo "  $(PPO_CC)      $@"
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c -MD $< -o $@

-include $(BUILD_DIR)/*.d
# **************************************************** #

# *********** 'check': compile & assembly ************ #
$(TEST_BUILD_DIR)/%.o: $(TEST_DIR)/$(SRC_DIR)/%.c $(CFG_FILE)
	@echo "  $(PPO_CC)      $@"
	@$(CC) $(CPPFLAGS) $(CFLAGS_TEST) -c -MD $< -o $@

-include $(TEST_BUILD_DIR)/*.d
# **************************************************** #

# **************** 'shaders': compile **************** #
$(SHADERS_BUILD_DIR)/%.vert.spv: $(SHADERS_DIR)/%.vert.glsl
	@echo "  $(PPO_GLSLC)   $@"
	@$(GLSL_CC) -fshader-stage=vert $< -o $@

$(SHADERS_BUILD_DIR)/%.frag.spv: $(SHADERS_DIR)/%.frag.glsl
	@echo "  $(PPO_GLSLC)   $@"
	@$(GLSL_CC) -fshader-stage=frag $< -o $@
# **************************************************** #

install:
	$(warning This functionality is not implemented yet (dry run).)
	@echo "  $(PPO_MKDIR)   $(PREFIX)/$(HDR_DIR)/wge-$(FULL_VERSION)"
	@echo "  $(PPO_LN)      $(PREFIX)/$(HDR_DIR)/wge"
	@for i in $$(find $(HDR_DIR) -type f); do \
	  echo "  $(PPO_INSTALL) $$i";            \
	done
	@echo "  $(PPO_INSTALL) $(WGE_OUT)"

dist:
	$(warning This functionality is not implemented yet (dry run).)
	@echo 

	@echo "Files selected:"
	@echo "  + $(WGE_OUT)"
	@echo "  + $(HDR_DIR)/*"
	@echo "  + $(SHADERS_BUILD_DIR)/*"
	@echo "Generated:"
	@echo "  => wge-$(FULL_VERSION)-$(TARGET)-$(ARCH).tar.gz"
	@echo "  => wge-$(FULL_VERSION)-$(TARGET)-$(ARCH).zip"

clean:
	@if [ -d $(BUILD_DIR_PARENT) ]; then           \
	  echo "  $(PPO_CLEAN)   $(BUILD_DIR_PARENT)"; \
	  rm -r $(BUILD_DIR_PARENT);                   \
	fi

mrproper: clean
	@for i in $(OUTS); do            \
	  if [ -e $$i ]; then            \
	    echo "  $(PPO_CLEAN)   $$i"; \
	    rm $$i;                      \
	  fi                             \
	done

version:
	@echo $(FULL_VERSION)

help:
	@echo "Configuration"
	@echo "============="
	@echo "  ARCH     :: <TBD> 'x86_64' (default)"
	@echo "  TARGET   :: 'linux' (default), 'windows'"
	@echo "  RELEASE  :: Set the environment for a release build (e.g. RELEASE=1)"
	@echo "  PREFIX   :: <TBD> '/usr/local' (default)"
	@echo "  CSTD     :: C standard to use, only GNU dialects accepted ('gnu17' by default)"
	@echo
	@echo "Targets"
	@echo "======="
	@echo "  all      :: Build all targets marked with [*]"
	@echo "* wge      :: Build the bare engine"
	@echo "  check    :: Run all defined unit tests under the 'test' directory"
	@echo "* shaders  :: Build all internal shaders under the 'shaders' directory"
	@echo "  install  :: <TBD> Install WGE to the system"
	@echo "  dist     :: <TBD> Creates archive packages for distribution ('.tar.gz', '.zip')"
	@echo "  clean    :: Remove the 'build' directory (where the '.o' object files live)"
	@echo "  mrproper :: Remove and cleans everything"
	@echo "  version  :: Shows the current checkout version string"
	@echo "  help     :: Shows this help and usage panel"
	@echo
	@echo "Execute 'make' or 'make all' to build all targets marked with [*]"
	@echo "For further info see the ./README.org file"
