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

varying highp vec3 v_normal;
varying highp vec3 v_position;

uniform highp float u_float;
uniform highp vec2 u_vec2;

uniform highp samplerCube cubeTex;

void main(void)
{
    highp vec3 ray = normalize(v_position.xyz - vec3(0.0,0.0,0.0));

    highp mat4 modelViewMatrix  = viewMatrix * modelMatrix;
    highp mat3 rotationPart = mat3(modelViewMatrix);
    highp vec3 normal = normalize(reflect(ray, normalize(rotationPart * v_normal)));

    gl_FragColor = textureCube(cubeTex, normalize(normal)) + vec4(u_vec2, u_float, 1.0);
}
