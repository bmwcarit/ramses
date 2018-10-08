//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 300 es

uniform highp mat4 mvpMatrix;
uniform sampler2D u_texture;

in vec3 a_position;
in vec2 a_texCoords;

out vec2 v_texCoords;
flat out ivec2 v_texSize;

void main()
{
    gl_Position = mvpMatrix * vec4(a_position, 1.0);
    v_texCoords = a_texCoords;
    v_texSize = textureSize(u_texture, 0);
}
