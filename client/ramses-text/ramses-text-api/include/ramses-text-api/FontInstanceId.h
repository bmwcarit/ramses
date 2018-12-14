//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FONTINSTANCEID_H
#define RAMSES_FONTINSTANCEID_H

#include "ramses-framework-api/StronglyTypedValue.h"
#include <stdint.h>
#include <limits>
#include <functional>

namespace ramses
{
    /**
    * @brief An empty struct to make FontId a strong type
    */
    struct FontIdTag {};

    /**
    * @brief A strongly typed integer to distinguish between different fonts
    */
    using FontId = StronglyTypedValue<uint32_t, FontIdTag>;

    /**
    * @brief A constant value representing an invalid FontId
    */
    static const FontId InvalidFontId(std::numeric_limits<FontId::BaseType>::max());

    /**
    * @brief An empty struct to make FontInstanceId a strong type
    */
    struct FontInstanceIdTag {};

    /**
    * @brief A strongly typed integer to distinguish between different font instances
    */
    using FontInstanceId = StronglyTypedValue<uint32_t, FontInstanceIdTag>;

    /**
    * @brief A constant value representing an invalid FontInstanceId
    */
    static const FontInstanceId InvalidFontInstanceId(std::numeric_limits<FontInstanceId::BaseType>::max());
}

#endif
