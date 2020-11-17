//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEAPI_SCENEVERSIONTAG_H
#define RAMSES_SCENEAPI_SCENEVERSIONTAG_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Common/StronglyTypedValue.h"

namespace ramses_internal
{
    struct SceneVersionTagTag {};
    using SceneVersionTag = StronglyTypedValue<UInt64, std::numeric_limits<uint64_t>::max(), SceneVersionTagTag>;
}

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::SceneVersionTag)

#endif
