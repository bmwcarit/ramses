//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 300 es

precision mediump float;

in vec3 a_position;
in vec3 translation;
in vec4 color;

out vec4 vColor;

uniform highp mat4 mvpMatrix;

void main()
{
    vec3 final_position = a_position + translation;
    gl_Position = mvpMatrix * vec4(final_position, 1.0);
    vColor = color;
}
