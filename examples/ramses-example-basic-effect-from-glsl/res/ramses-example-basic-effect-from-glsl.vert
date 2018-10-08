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
uniform vec4 u_transformations[2];

void main()
{
    //scaling
    vec3 scaling = u_transformations[0].xyz;
    vec3 res = a_position * scaling;

    float shearing[2];
    shearing[0] = u_transformations[1].x;
    shearing[1] = u_transformations[1].y;

    //shearing 1
    res = vec3(res.x + shearing[0] * res.y, res.y, res.z);
    //shearing 2
    res.x = res.x + res.y * shearing[1];

    mat3 rotMat = mat3(cos(0.78), sin(0.78), 0.0, -sin(0.78), cos(0.78), 0.0, 0.0, 0.0, 1.0);
    res = rotMat * res;

    gl_Position = mvpMatrix * vec4(res, 1.0);
}
