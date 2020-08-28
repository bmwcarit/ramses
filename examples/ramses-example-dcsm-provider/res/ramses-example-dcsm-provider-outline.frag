//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 100

varying lowp vec2 v_texcoord;
precision highp float;

void main(void)
{
    const float  offset = 1.0 / 128.0;
    if ( (v_texcoord.x < offset )|| (v_texcoord.x > 1.0-offset))
    {
        if ((mod (gl_FragCoord.y,5.0) < 3.0))
        {
            discard;
        }
        else
        {
            gl_FragColor = vec4(1.0, 1.0, 1.0, 0.8);
        }
    }
    else if( (v_texcoord.y < offset )|| (v_texcoord.y > 1.0-offset))
    {
        if ((mod (gl_FragCoord.x,5.0) < 3.0))
        {
            discard;
        }
        else
        {
            gl_FragColor = vec4(1.0, 1.0, 1.0, 0.8);
        }
    }
    else {
        discard;
    }
}
