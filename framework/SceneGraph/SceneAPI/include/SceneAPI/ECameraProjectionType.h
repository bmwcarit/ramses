//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ECAMERAPROJECTIONTYPE_H
#define RAMSES_ECAMERAPROJECTIONTYPE_H

#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    enum ECameraProjectionType
    {
        ECameraProjectionType_Perspective = 0,
        ECameraProjectionType_Orthographic,
        ECameraProjectionType_Renderer

    };

    inline const Char* EnumToString(ECameraProjectionType cameraProjectionType)
    {
        switch (cameraProjectionType)
        {
        case ECameraProjectionType_Perspective: return "ECameraProjectionType_Perspective";
        case ECameraProjectionType_Orthographic: return "ECameraProjectionType_Orthographic";
        case ECameraProjectionType_Renderer: return "ECameraProjectionType_Renderer";
        }
        return "";
    }
}

#endif //RAMSES_ECAMERAPROJECTIONTYPE_H
