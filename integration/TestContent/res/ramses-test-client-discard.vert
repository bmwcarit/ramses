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
varying highp vec2 v_position;

void main()
{
    vec4 position = mvpMatrix * vec4(a_position, 1.0);
    v_position = position.xy / position.w;
    gl_Position = position;
}
