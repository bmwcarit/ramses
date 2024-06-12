//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 310 es

precision highp float;

layout(std140,binding=1) uniform modelCameraBlock_t
{
    mat4 mvpMatrix;
    mat4 mvMatrix;
    mat4 normalMatrix;
} modelCameraBlock;

in vec3 a_position;

void main()
{
    gl_Position = modelCameraBlock.mvpMatrix * vec4(a_position, 1.0);
}
