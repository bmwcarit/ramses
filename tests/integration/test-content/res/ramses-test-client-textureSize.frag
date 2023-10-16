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
uniform ivec2 texSizeFromApplication;
out vec4 fragColor;

in vec2 v_texCoords;
flat in ivec2 v_texSize;

void main()
{
    ivec2 size = textureSize(u_texture, 0);

    // Ensure that textureSize() returns the same result as in the vertex stage
    if (v_texSize != size)
    {
        discard;
    }

    // ...and for the value passed from the application
    if (texSizeFromApplication != size)
    {
        discard;
    }

    fragColor = texture(u_texture, v_texCoords);
}
