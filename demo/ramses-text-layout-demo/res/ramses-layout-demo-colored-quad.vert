//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 100

uniform highp mat4 mvpMatrix;

attribute vec3 a_position;
uniform highp vec4 u_boxSize;

void main()
{
    gl_Position = mvpMatrix * vec4(u_boxSize.xy + (a_position.xy * u_boxSize.zw), 0.0, 1.0);
}
