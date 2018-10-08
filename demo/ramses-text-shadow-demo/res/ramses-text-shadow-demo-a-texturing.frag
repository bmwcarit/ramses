//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 300 es

precision highp float;

uniform sampler2D textureSampler;
uniform vec3 u_color;

in vec2 v_texcoord;
out vec4 fragmentColor;

void main(void)
{
    fragmentColor = vec4(u_color, texture(textureSampler, v_texcoord).r);
}
