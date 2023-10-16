//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/StronglyTypedValue.h"
#include <cstdint>
#include <limits>

namespace ramses
{
    /**
    * @brief An empty struct to make FontId a strong type
    */
    struct FontIdTag {};

    /**
    * @ingroup TextAPI
    * @brief A strongly typed integer to distinguish between different fonts
    */
    using FontId = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), FontIdTag>;

    /**
    * @ingroup TextAPI
    * @brief An empty struct to make FontInstanceId a strong type
    */
    struct FontInstanceIdTag {};

    /**
    * @ingroup TextAPI
    * @brief A strongly typed integer to distinguish between different font instances
    */
    using FontInstanceId = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), FontInstanceIdTag>;
}
