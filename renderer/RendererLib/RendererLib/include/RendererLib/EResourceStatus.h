//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ERESOURCESTATUS_H
#define RAMSES_ERESOURCESTATUS_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Utils/LoggingUtils.h"
#include <assert.h>

namespace ramses_internal
{
    enum EResourceStatus
    {
        EResourceStatus_Unknown = 0,   ///< Resource is not known to renderer
        EResourceStatus_Registered,    ///< Resource has been registered, not requested yet
        EResourceStatus_Requested,     ///< Resource has been requested from provider, not provided yet
        EResourceStatus_Provided,      ///< Resource has been provided, not uploaded yet
        EResourceStatus_Uploaded,      ///< Resource has been uploaded and is ready for rendering
        EResourceStatus_Broken,        ///< Resource failed to be uploaded, it is freed from system memory and will not be attempted to upload again

        EResourceStatus_NUMBER_OF_ELEMENTS
    };

    static const char* ResourceStatusNames[] =
    {
        "EResourceStatus_Unknown",
        "EResourceStatus_Registered",
        "EResourceStatus_Requested",
        "EResourceStatus_Provided",
        "EResourceStatus_Uploaded",
        "EResourceStatus_Broken"
    };

    ENUM_TO_STRING(EResourceStatus, ResourceStatusNames, EResourceStatus_NUMBER_OF_ELEMENTS);
}
#endif
