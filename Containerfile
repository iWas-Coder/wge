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


FROM debian:sid-slim
RUN apt-get update && apt-get full-upgrade -y
RUN apt-get install -y                             \
                       git                         \
                       xcb                         \
                       clang                       \
                       glslc                       \
                       mingw-w64                   \
                       libx11-dev                  \
                       libvulkan1                  \
                       glslang-dev                 \
                       gcc-multilib                \
                       vulkan-tools                \
                       glslang-tools               \
                       libvulkan-dev               \
                       libx11-xcb-dev              \
                       libxcb-xkb-dev              \
                       build-essential             \
                       libxkbcommon-x11-dev        \
                       vulkan-validationlayers     \
                       vulkan-validationlayers-dev
WORKDIR /wge
ENTRYPOINT ["make"]
