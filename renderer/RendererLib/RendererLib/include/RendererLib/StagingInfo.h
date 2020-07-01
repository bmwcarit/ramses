//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STAGINGINFO_H
#define RAMSES_STAGINGINFO_H

#include "SceneAPI/SceneSizeInformation.h"
#include "SceneAPI/SceneVersionTag.h"
#include "Scene/SceneActionCollection.h"
#include "Scene/SceneResourceChanges.h"
#include "SceneReferencing/SceneReferenceAction.h"
#include "Components/FlushTimeInformation.h"
#include <deque>

namespace ramses_internal
{
    class ResourceCachedScene;

    struct PendingFlush
    {
        SceneActionCollection sceneActions;
        UInt64                flushIndex = 0u;
        FlushTimeInformation  timeInfo;
        SceneVersionTag       versionTag;

    };
    typedef std::deque<PendingFlush> PendingFlushes;

    struct PendingData
    {
        bool                      allPendingFlushesApplied = false;
        PendingFlushes            pendingFlushes;
        // client resources that are needed in addition to resources in use by renderer scene
        ResourceContentHashVector clientResourcesNeeded;
        // client resources that will not be needed anymore after pending flushes are applied
        ResourceContentHashVector clientResourcesUnneeded;
        // client resources that were newly needed and then unneeded within the scope of pending flushes,
        // they were requested and need to be therefore unrequested after pending flushes are applied
        ResourceContentHashVector clientResourcesPendingUnneeded;
        // scene resource actions to execute after pending flushes are applied
        SceneResourceActionVector sceneResourceActions;
        // scene reference actions to execute after pending flushes are applied
        SceneReferenceActionVector sceneReferenceActions;

        static void Clear(PendingData& pendingData)
        {
            pendingData.allPendingFlushesApplied = false;
            pendingData.pendingFlushes.clear();
            pendingData.clientResourcesNeeded.clear();
            pendingData.clientResourcesUnneeded.clear();
            pendingData.clientResourcesPendingUnneeded.clear();
            pendingData.sceneResourceActions.clear();
            pendingData.sceneReferenceActions.clear();
        }
    };

    struct StagingInfo
    {
        SceneSizeInformation      sizeInformation;
        // client resources referenced by renderer scene (without pending flushes)
        ResourceContentHashVector clientResourcesInUse;
        PendingData               pendingData;
        SceneVersionTag           lastAppliedVersionTag;
    };
}

#endif
