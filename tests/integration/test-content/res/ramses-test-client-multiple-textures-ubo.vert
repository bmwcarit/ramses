//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 320 es

layout(std140, binding=1) uniform modelBlock_t
{
    highp mat4 modelMat;
} modelUbo;

layout(std140, binding=2) uniform cameraBlock_t
{
    highp mat4 projMat;
    highp mat4 viewMat;
    highp vec3 cameraPos;
} cameraUbo;

in vec3 a_position;
in vec2 a_texcoord;

out vec2 v_texcoord;

void main()
{
    gl_Position = cameraUbo.projMat * cameraUbo.viewMat * modelUbo.modelMat * vec4(a_position, 1.0);
    v_texcoord = a_texcoord;
}
