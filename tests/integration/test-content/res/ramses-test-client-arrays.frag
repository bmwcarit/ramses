//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#version 100

uniform int caseNumber;

uniform highp vec4 colorVec4Array[3];
uniform highp int colorIntArray[3];

uniform int index;

void main(void)
{
    if (caseNumber == 0)
    {
        gl_FragColor = colorVec4Array[1];
    }
    else if (caseNumber == 1)
    {
        gl_FragColor = vec4(0.0, 0.0, float(colorIntArray[1]), 0.0);
    }
    else if (caseNumber == 2)
    {
        gl_FragColor = vec4(1.0, float(colorIntArray[index]), 0.0, 0.0);
    }
}
