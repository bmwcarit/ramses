//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 100

uniform highp vec4 color;
uniform highp vec2 redgreen_offset;

varying highp float factor;

void main(void)
{
    gl_FragColor = color * factor + vec4(redgreen_offset.x, redgreen_offset.y, 0.0, 0.0);
}
