//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Common/StronglyTypedValue.h"

#include <cstdint>

namespace ramses::internal
{
    struct SceneVersionTagTag {};
    using SceneVersionTag = StronglyTypedValue<uint64_t, std::numeric_limits<uint64_t>::max(), SceneVersionTagTag>;
}

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::internal::SceneVersionTag)
