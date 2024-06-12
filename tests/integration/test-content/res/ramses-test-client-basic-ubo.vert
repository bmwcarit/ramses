//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 310 es

layout(std140, binding=0) uniform general_t
{
    int variant;
} generalUbo;

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

layout(std140, binding=3) uniform modelCameraBlock_t
{
    highp mat4 mvpMat;
    highp mat4 mvMat;
    highp mat4 normalMat;
} modelCameraUbo;

in vec3 a_position;

void main()
{
    if (generalUbo.variant == 1)
        gl_Position = cameraUbo.projMat * cameraUbo.viewMat * modelUbo.modelMat * vec4(a_position, 1.0);
    if (generalUbo.variant == 2)
        gl_Position = modelCameraUbo.mvpMat * vec4(a_position, 1.0);
    if (generalUbo.variant == 3)
        gl_Position = cameraUbo.projMat * modelCameraUbo.mvMat * vec4(a_position, 1.0);
}
