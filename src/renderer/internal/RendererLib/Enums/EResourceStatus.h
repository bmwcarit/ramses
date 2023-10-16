//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LoggingUtils.h"

namespace ramses::internal
{
    enum class EResourceStatus
    {
        Registered = 0,   ///< Resource is known to renderer but there is no information/data available
        Provided,         ///< Resource is registered and data has been provided, not uploaded yet
        ScheduledForUpload, ///< Resource is marked for upload asynchronously
        Uploaded,         ///< Resource has been uploaded and is ready for rendering
        Broken            ///< Resource failed to be uploaded, it is freed from system memory and will not be attempted to upload again
    };

    const std::array ResourceStatusNames =
    {
        "Registered",
        "Provided",
        "ScheduledForUpload",
        "Uploaded",
        "Broken"
    };
}

MAKE_ENUM_CLASS_PRINTABLE(ramses::internal::EResourceStatus,
                                        "EResourceStatus",
                                        ramses::internal::ResourceStatusNames,
                                        ramses::internal::EResourceStatus::Broken);
