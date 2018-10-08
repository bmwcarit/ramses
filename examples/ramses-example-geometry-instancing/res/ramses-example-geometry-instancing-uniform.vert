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

uniform highp mat4 mvpMatrix;
uniform vec3 translations[10];
flat out int instanceID;

void main()
{
    instanceID = gl_InstanceID;
    vec3 final_position = a_position + translations[instanceID];
    gl_Position = mvpMatrix * vec4(final_position, 1.0);
}
