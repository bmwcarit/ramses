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
    enum ESceneState
    {
        ESceneState_Unknown = 0,
        ESceneState_Published,              // Scene has been announced from client
        ESceneState_SubscriptionRequested,  // User requested scene subscription and request was sent to client
        ESceneState_SubscriptionPending,    // Scene info was received from client and waiting for content (first flush on client side)
        ESceneState_Subscribed,             // Scene content arrived from client with content last flushed on client side
        ESceneState_MapRequested,           // User requested a scene to be mapped on a display, waiting for all pending (partial) flushes to be applied
        ESceneState_MappingAndUploading,    // Scene is mapped internally to a display and is uploading its resources
        ESceneState_Mapped,                 // Scene is ready to be rendered
        ESceneState_Rendered,               // Scene is rendered
        ESceneState_NUMBER_OF_ELEMENTS
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
        "Rendered"
    };

    ENUM_TO_STRING(ESceneState, SceneStateNames, ESceneState_NUMBER_OF_ELEMENTS);
}
#endif
