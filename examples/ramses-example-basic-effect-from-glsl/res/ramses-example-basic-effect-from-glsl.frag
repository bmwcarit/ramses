//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 100

uniform highp vec4 color;
void main(void)
{
    mediump float fc0 = 1.0;
    mediump float fcr = fc0 > 2.0 ? 1.0 : 0.1;
    highp vec4 lighterColor = vec4(color.xyz * 2.0 + vec3(0.2), 1.0);
    gl_FragColor = (lighterColor + vec4(cos(color.x), sin(color.y), tan(color.x), 0.0) * 0.25) * fcr ;
}
