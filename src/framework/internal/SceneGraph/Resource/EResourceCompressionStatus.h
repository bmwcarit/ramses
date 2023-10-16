//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LoggingUtils.h"

namespace ramses::internal
{
    enum class EResourceCompressionStatus  //enum to support multiple compression formats in the future
    {
        Uncompressed = 0,
        Compressed
    };

    const std::array EResourceCompressionStatusNames =
    {
        "Uncompressed",
        "Compressed",
    };
}

MAKE_ENUM_CLASS_PRINTABLE(ramses::internal::EResourceCompressionStatus,
                                        "EResourceCompressionStatus",
                                        ramses::internal::EResourceCompressionStatusNames,
                                        ramses::internal::EResourceCompressionStatus::Compressed);
