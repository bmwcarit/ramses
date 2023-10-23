//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Common/StronglyTypedValue.h"

namespace ramses::internal
{
    struct SceneFileHandleTag;
    using SceneFileHandle = StronglyTypedValue<uint64_t, 0, SceneFileHandleTag>;
}

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::internal::SceneFileHandle);
