//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 310 es

precision highp float;

uniform sampler2D textureSampler;

layout(std140,binding=1) uniform ub_t
{
     highp vec4 color;
} uniformBlock;

in lowp vec2 v_texcoord;
out vec4 fragColor;

void main(void)
{
    fragColor = uniformBlock.color * texture(textureSampler, v_texcoord);
}
