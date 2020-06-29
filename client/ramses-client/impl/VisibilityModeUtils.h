//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VISIBILITYMODEUTILS_H
#define RAMSES_VISIBILITYMODEUTILS_H

#include "ramses-client-api/EVisibilityMode.h"
#include "SceneAPI/Renderable.h"

namespace ramses
{
    class VisibilityModeUtils
    {
    public:
        static ramses::EVisibilityMode ConvertToHL(ramses_internal::EVisibilityMode mode);
        static ramses_internal::EVisibilityMode ConvertToLL(EVisibilityMode mode);
        static const char* ToString(EVisibilityMode mode);
    };
}

#endif
