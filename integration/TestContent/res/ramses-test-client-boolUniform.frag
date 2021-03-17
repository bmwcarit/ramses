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

out vec4 fragColor;

void main(void)
{
    float alpha = 1.0;
    if(makeSemiTransparent)
    {
        alpha = 0.5;
    }

    fragColor = vec4(1.0, 0.0, 0.0, alpha);
}
