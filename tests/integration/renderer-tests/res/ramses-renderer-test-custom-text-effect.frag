//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 100

precision highp float;

uniform sampler2D u_customTexture;
uniform vec4 u_customColor;

varying vec2 v_texcoord;

varying vec2 v_position;

void main(void)
{
    float g = (sin(length(v_position) * 20.0) + 1.0) * 0.5;
    float a = texture2D(u_customTexture, v_texcoord).r;
    gl_FragColor = vec4(u_customColor.rgb, a);
    gl_FragColor.g = g;
}
