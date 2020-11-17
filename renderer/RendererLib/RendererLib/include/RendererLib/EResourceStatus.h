//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ERESOURCESTATUS_H
#define RAMSES_ERESOURCESTATUS_H

#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    enum class EResourceStatus
    {
        Registered = 0,   ///< Resource is known to renderer but there is no information/data available
        Provided,         ///< Resource is registered and data has been provided, not uploaded yet
        Uploaded,         ///< Resource has been uploaded and is ready for rendering
        Broken            ///< Resource failed to be uploaded, it is freed from system memory and will not be attempted to upload again
    };

    static constexpr const char* ResourceStatusNames[] =
    {
        "EResourceStatus_Registered",
        "EResourceStatus_Provided",
        "EResourceStatus_Uploaded",
        "EResourceStatus_Broken"
    };
}

MAKE_ENUM_CLASS_PRINTABLE_NO_EXTRA_LAST(ramses_internal::EResourceStatus,
    ramses_internal::ResourceStatusNames,
    ramses_internal::EResourceStatus::Broken);

#endif
