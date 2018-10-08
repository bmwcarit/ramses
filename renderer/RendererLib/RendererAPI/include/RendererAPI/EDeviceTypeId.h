//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EDEVICETYPEID_H
#define RAMSES_EDEVICETYPEID_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Common/BitForgeMacro.h"

namespace ramses_internal
{
    enum EDeviceTypeId
    {
        EDeviceTypeId_INVALID                       = 0,
        EDeviceTypeId_GL_ES_3_0                     = BIT(0),
        EDeviceTypeId_GL_4_2_CORE                   = BIT(1),
        EDeviceTypeId_GL_4_5                        = BIT(2),
        EDeviceTypeId_ALL                           = BIT(3) - 1
    };

    inline Bool IsRendererDeviceEnabled(UInt32 deviceMask, EDeviceTypeId deviceIDToCheck)
    {
        return (deviceMask & deviceIDToCheck) != 0;
    }

    inline const Char* EnumToString(EDeviceTypeId deviceTypeId)
    {
        switch (deviceTypeId)
        {
        case EDeviceTypeId_INVALID:
            return "EDeviceTypeId_INVALID";
        case EDeviceTypeId_GL_4_2_CORE:
            return "EDeviceTypeId_GL_4_2_CORE";
        case EDeviceTypeId_GL_4_5:
            return "EDeviceTypeId_GL_4_5";
        case EDeviceTypeId_GL_ES_3_0:
            return "EDeviceTypeId_GL_ES_3_0";
        default:
            break;
        }

        return "EDeviceTypeId UNKNOWN";
    }
}

#endif
