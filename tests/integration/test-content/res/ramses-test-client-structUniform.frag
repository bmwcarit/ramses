//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 300 es

precision highp float;

struct simpleStruct
{
    float red;
    float green;
    float blue;
    float alpha;
};

struct complexStruct
{
    simpleStruct data[2];
};

uniform complexStruct inputVar;

out vec4 fragColor;

void main(void)
{
    float red = inputVar.data[0].red;
    // This is just for avoiding the compiler optimizing away the unused uniforms
    float blah = inputVar.data[0].blue + inputVar.data[0].alpha + inputVar.data[1].red + inputVar.data[1].green;
    if (blah > 9.0)
        red = 0.0;

    fragColor = vec4(red, inputVar.data[0].green, inputVar.data[1].blue, inputVar.data[1].alpha);
}
