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

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_texcoord;

layout(set = 0, binding = 0) uniform global_uniform_obj {
  mat4 proj;
  mat4 view;
} global_ubo;
layout(push_constant) uniform push_constants {
  mat4 model;
} u_push_constants;

layout(location = 0) out int out_mode;
layout(location = 1) out struct dto {
  vec2 texcoord;
} out_dto;

void main(void) {
  out_dto.texcoord = in_texcoord;
  gl_Position = global_ubo.proj * global_ubo.view * u_push_constants.model * vec4(in_pos, 1.0);
}
