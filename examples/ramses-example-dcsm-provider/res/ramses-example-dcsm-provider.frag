//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 100

uniform sampler2D textureSampler;

varying lowp vec2 v_texcoord;

uniform highp float transparency;

void main(void)
{
    gl_FragColor = texture2D(textureSampler, v_texcoord) * vec4(1.0, 1.0, 1.0, transparency);
}
