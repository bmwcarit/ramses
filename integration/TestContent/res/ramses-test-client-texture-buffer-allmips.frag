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

in vec2 v_texcoord;
out vec4 fragColor;

void main(void)
{
    // sample from different mip levels based on tex coords
    // 1 2
    // 0 1
    int mipLevel = int(round(v_texcoord.x) + round(v_texcoord.y));

    // We have a platform with a bug which causes textureSize() to return random (wrong) values when used in combination with mips
    // Hence, calculate mip size manually (sizes statically defined by test: 4x4, 2x2 and 1x1 respectively)
    int mipSize = int(exp2(float(2 - mipLevel)) + 0.5f);

    ivec2 texCoords = ivec2(v_texcoord * vec2(mipSize));
    fragColor = texelFetch(u_texture, texCoords, mipLevel);
}
