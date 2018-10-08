//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 300 es

precision highp float;

uniform sampler2D u_texture;
uniform int u_mip;

in vec2 v_texcoord;
out vec4 fragColor;

void main(void)
{
    ivec2 texCoords = ivec2(v_texcoord * vec2(textureSize(u_texture, u_mip)));
    fragColor = texelFetch(u_texture, texCoords, u_mip);
}
