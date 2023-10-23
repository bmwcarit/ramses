//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 100

uniform highp mat4 viewMatrix;
uniform highp mat4 modelMatrix;
uniform highp mat4 mvpMatrix;

attribute vec3 a_position;
attribute vec3 a_normal;

varying vec3 v_normal;
varying vec3 v_position;

void main()
{
    gl_Position = mvpMatrix * vec4(a_position, 1.0);

    vec4 globalPosition = viewMatrix * modelMatrix * vec4(a_position, 1.0);

    v_normal = a_normal;
    v_position = normalize(vec3(globalPosition));
}
