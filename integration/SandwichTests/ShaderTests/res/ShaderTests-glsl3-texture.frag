//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 300 es

uniform sampler2D textureSampler;

in lowp vec2 v_texcoord;

out highp vec4 fragmentColor;

void main(void)
{
    fragmentColor = texture(textureSampler, v_texcoord);
}
