//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 300 es
precision highp float;

layout(location = 0) out vec4 color1;
layout(location = 1) out vec4 color2;

void main(void)
{
    color1 = vec4(1.0, 0.0, 0.0, 1.0);
    color2 = vec4(0.0, 0.0, 1.0, 1.0);
}
