//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 320 es

layout(points) in;
layout(triangle_strip, max_vertices = 3) out;

uniform highp float x_multiplier;

void main() {
    vec4 positionIn = gl_in[0].gl_Position;

    gl_Position = positionIn + vec4(-0.2, -0.3, 0.0, 0.0);
    EmitVertex();
    gl_Position = positionIn + vec4(0.2 * x_multiplier, -0.3, 0.0, 0.0);
    EmitVertex();
    gl_Position = positionIn + vec4(0.0, 0.6, 0.0, 0.0);
    EmitVertex();
    EndPrimitive();
}
