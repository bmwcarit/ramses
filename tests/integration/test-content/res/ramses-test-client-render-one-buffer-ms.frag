//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 310 es

precision highp float;

uniform highp sampler2DMS textureSampler;
uniform highp int sampleCount;

in lowp vec2 v_texcoord;
out vec4 fragColor;

void main(void)
{
    vec4 color = vec4(0.0);

    for (int i = 0; i < sampleCount; i++)
        color += texelFetch(textureSampler, ivec2(v_texcoord * vec2(textureSize(textureSampler))), i);

    color /= float(sampleCount);
    fragColor = color;
}
