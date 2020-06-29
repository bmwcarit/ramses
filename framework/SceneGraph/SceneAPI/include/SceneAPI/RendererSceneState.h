//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEWORK_RENDERERSCENESTATE_H
#define RAMSES_FRAMEWORK_RENDERERSCENESTATE_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    enum class RendererSceneState : uint32_t
    {
        Unavailable,
        Available,
        Ready,
        Rendered,
    };

    static const char* RendererSceneStateNames[] =
    {
        "Unavailable",
        "Available",
        "Ready",
        "Rendered",
    };
    ENUM_TO_STRING(RendererSceneState, RendererSceneStateNames, 4);
}

#endif
