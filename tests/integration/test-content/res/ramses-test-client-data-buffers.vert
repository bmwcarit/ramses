//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 100

uniform highp mat4 mvpMatrix;
uniform int caseNumber;

attribute vec3 a_positionFloatX;
attribute vec3 a_positionFloatY;
attribute vec3 a_positionVec2;
attribute vec3 a_positionVec3;
attribute vec3 a_positionVec4;

void main()
{
    gl_Position = mvpMatrix * vec4(a_positionVec3, 1.0);
}
