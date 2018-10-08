//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EDATASLOTTYPE_H
#define RAMSES_EDATASLOTTYPE_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    enum EDataSlotType
    {
        EDataSlotType_TransformationProvider = 0,
        EDataSlotType_TransformationConsumer,
        EDataSlotType_DataProvider,
        EDataSlotType_DataConsumer,
        EDataSlotType_TextureProvider,
        EDataSlotType_TextureConsumer,
        EDataSlotType_Undefined,
        EDataSlotType_NUMBER_OF_ELEMENTS
    };

    static const char* DataSlotTypeNames[] =
    {
        "EDataSlotType_TransformationProvider",
        "EDataSlotType_TransformationConsumer",
        "EDataSlotType_DataProvider",
        "EDataSlotType_DataConsumer",
        "EDataSlotType_TextureProvider",
        "EDataSlotType_TextureConsumer",
        "EDataSlotType_Undefined"
    };

    ENUM_TO_STRING(EDataSlotType, DataSlotTypeNames, EDataSlotType_NUMBER_OF_ELEMENTS);
}

#endif
