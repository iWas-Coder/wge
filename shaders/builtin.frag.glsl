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


#version 450
// #extension GL_ARG_separate_shader_objects : enable

layout(location = 0) flat in int in_mode;
layout(location = 1) in struct dto {
  vec2 texcoord;
} in_dto;

layout(set = 1, binding = 0) uniform local_uniform_object {
  vec4 diffuse_color;
} object_ubo;
layout(set = 1, binding = 1) uniform sampler2D diffuse_sampler;

layout(location = 0) out vec4 out_color;

void main(void) {
  out_color = object_ubo.diffuse_color * texture(diffuse_sampler, in_dto.texcoord);
}
