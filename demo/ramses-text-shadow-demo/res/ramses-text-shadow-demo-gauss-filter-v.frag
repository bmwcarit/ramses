//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 300 es

const int maxKernelSize = 32;

precision highp float;
precision highp int;

uniform sampler2D u_texture;

uniform int u_kernelSize;
uniform int u_maxKernelSize;

in vec2 v_position;

out vec4 fragmentColor;

uniform float u_kernel[maxKernelSize + 1];

float clampedTexelFetch(ivec2 p)
{
    ivec2 ts = textureSize(u_texture, 0);
    if (p.x >= 0 && p.y >= 0 && p.x < ts.x && p.y  < ts.y)
    {
        return texelFetch(u_texture, p, 0).r;
    }
    else
    {
        return 0.0;
    }
}

void main(void)
{
    ivec2 p = ivec2(v_position);
    p.y -= u_maxKernelSize;

    fragmentColor.r = clampedTexelFetch(p) * u_kernel[0];

    for (int i = 1; i <= u_kernelSize; i++)
    {
        fragmentColor.r += clampedTexelFetch(ivec2(p.x, p.y - i)) * u_kernel[i];
        fragmentColor.r += clampedTexelFetch(ivec2(p.x, p.y + i)) * u_kernel[i];
    }
}
