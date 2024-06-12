//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 320 es

precision highp float;

uniform int         u_multiplexer;
uniform sampler2D   u_texture1;
uniform sampler2D   u_texture2;
uniform sampler2D   u_texture3;

in vec2 v_texcoord;
out vec4 fragColor;

void main(void)
{
    vec4 color = vec4(0.0);
    if(u_multiplexer == 1)
        color = texture(u_texture1, v_texcoord);
    if(u_multiplexer == 2)
        color = texture(u_texture2, v_texcoord);
    if(u_multiplexer == 3)
        color = texture(u_texture3, v_texcoord);

    fragColor = color;
}
