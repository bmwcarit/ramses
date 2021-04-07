//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEWORK_SCENEFILEHANDLE_H
#define RAMSES_FRAMEWORK_SCENEFILEHANDLE_H

#include "Common/StronglyTypedValue.h"

namespace ramses_internal
{
    struct SceneFileHandleTag;
    using SceneFileHandle = StronglyTypedValue<uint64_t, 0, SceneFileHandleTag>;
}

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::SceneFileHandle);

#endif
