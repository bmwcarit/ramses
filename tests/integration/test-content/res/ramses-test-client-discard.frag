//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 100

varying highp vec2 v_position;
uniform highp vec4 color;

void main(void)
{
    if(v_position.x > 0.0)
    {
        if(v_position.y > 0.25)
        {
            // set from input
            gl_FragColor = color;
        }
        else
        {
            // blue
            gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
        }
    }
    else
    {
        if(v_position.y > 0.25)
        {
            // white
            gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
            // but should be black because of discard
            discard;
        }
        else
        {
            // green
            gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
        }
    }
}
