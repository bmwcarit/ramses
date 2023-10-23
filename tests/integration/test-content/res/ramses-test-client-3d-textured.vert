//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 300 es

uniform highp mat4 mvpMatrix;

in vec3 a_position;
in vec3 a_texcoord;

out vec3 v_texcoord;

void main()
{
    v_texcoord = a_texcoord;
    gl_Position = mvpMatrix * vec4(a_position, 1.0);
}
