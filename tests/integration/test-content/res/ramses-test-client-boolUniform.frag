//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 300 es

precision highp float;

uniform bool makeSemiTransparent;
uniform bool colorCh[3];

out vec4 fragColor;

void main(void)
{
    float alpha = 1.0;
    if(makeSemiTransparent)
    {
        alpha = 0.5;
    }

    vec3 color = vec3(0.0);
    if (colorCh[0])
        color.r = 1.0;

    if (colorCh[1])
        color.g = 1.0;

    if (colorCh[2])
        color.b = 1.0;

    fragColor = vec4(color, alpha);
}
