//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 100

uniform sampler2D textureSampler;

varying lowp vec2 v_texcoord;

void main(void)
{
    gl_FragColor = vec4(texture2D(textureSampler, v_texcoord).rgb, 1.0);
}
