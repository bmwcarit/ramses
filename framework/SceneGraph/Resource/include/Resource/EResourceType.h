//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ERESOURCETYPE_H
#define RAMSES_ERESOURCETYPE_H

#include <assert.h>
#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    enum EResourceType
    {
        EResourceType_Invalid               = 0x00000000,

        EResourceType_VertexArray,
        EResourceType_IndexArray,
        EResourceType_Texture2D,
        EResourceType_Texture3D,
        EResourceType_TextureCube,
        EResourceType_Effect,
        EResourceType_NUMBER_OF_ELEMENTS
    };

    static const char* ResourceTypeNames[] =
    {
        "EResourceType_Invalid",
        "EResourceType_VertexArray",
        "EResourceType_IndexArray",
        "EResourceType_Texture2D",
        "EResourceType_Texture3D",
        "EResourceType_TextureCube",
        "EResourceType_Effect"
    };

    ENUM_TO_STRING(EResourceType, ResourceTypeNames, EResourceType_NUMBER_OF_ELEMENTS);
}

#endif
