//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 100

precision highp float;

uniform sampler2D u_texture;

varying vec2 v_texcoord;

void main(void)
{
    float a = texture2D(u_texture, v_texcoord).r;
    gl_FragColor = vec4(1.0, 1.0, 1.0, a);
}
