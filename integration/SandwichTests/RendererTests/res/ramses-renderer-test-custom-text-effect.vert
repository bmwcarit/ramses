//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 100

precision highp float;

uniform highp mat4 mvpMatrix;

attribute vec2 a_customPosition;
attribute vec2 a_customTexCoord;

varying vec2 v_texcoord;
varying vec2 v_position;

void main()
{
    gl_Position = mvpMatrix * vec4(a_customPosition, 0.0, 1.0);
    v_texcoord = a_customTexCoord;
    v_position = vec2(a_customPosition.x / 640.0, a_customPosition.y / 100.0);
}
