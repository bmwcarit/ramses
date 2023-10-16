//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 100

precision highp float;

uniform sampler2D u_texture;

varying lowp vec2 v_texcoord;

void main(void)
{
    vec4 color = texture2D(u_texture, v_texcoord);
    gl_FragColor = color;
}
