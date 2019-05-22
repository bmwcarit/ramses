//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ESCENESTATE_H
#define RAMSES_ESCENESTATE_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Utils/LoggingUtils.h"
#include <assert.h>

namespace ramses_internal
{
    enum class ESceneState
    {
        Unknown = 0,
        Published,              // Scene has been announced from client
        SubscriptionRequested,  // (renderer internal state) User requested scene subscription and request was sent to client
        SubscriptionPending,    // (renderer internal state) Scene info was received from client and waiting for content (first flush on client side)
        Subscribed,             // Scene content arrived from client with content last flushed on client side
        MapRequested,           // (renderer internal state) User requested a scene to be mapped on a display, waiting for all pending (partial) flushes to be applied
        MappingAndUploading,    // (renderer internal state) Scene is mapped internally to a display and is uploading its resources
        Mapped,                 // Scene is ready to be rendered
        RenderRequested,        // (renderer internal state) Scene is requested to be rendered
        Rendered,               // Scene is rendered
        NUMBER_OF_ELEMENTS
    };

    static const char* SceneStateNames[] =
    {
        "Unknown",
        "Published",
        "SubscriptionRequested",
        "SubscriptionPending",
        "Subscribed",
        "MapRequested",
        "MappingAndUploading",
        "Mapped",
        "RenderRequested",
        "Rendered"
    };

    ENUM_TO_STRING(ESceneState, SceneStateNames, ESceneState::NUMBER_OF_ELEMENTS);

    static inline bool SceneStateIsAtLeast(ESceneState state, ESceneState minState)
    {
        return state >= minState;
    }
}

#endif
