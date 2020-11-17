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
#include "Scene/ResourceChanges.h"
#include "SceneReferencing/SceneReferenceAction.h"
#include "Components/FlushTimeInformation.h"
#include "Components/ManagedResource.h"

namespace ramses_internal
{
    class ResourceCachedScene;

    struct PendingFlush
    {
        SceneActionCollection     sceneActions;
        UInt64                    flushIndex = 0u;
        FlushTimeInformation      timeInfo;
        SceneVersionTag           versionTag;

        ManagedResourceVector     resourceDataToProvide;
        ResourceContentHashVector resourcesAdded;
        ResourceContentHashVector resourcesRemoved;
    };
    using PendingFlushes = std::vector<PendingFlush>;

    struct PendingData
    {
        bool                      allPendingFlushesApplied = false;
        PendingFlushes            pendingFlushes;

        // scene resource actions to execute after pending flushes are applied
        SceneResourceActionVector sceneResourceActions;
        // scene reference actions to execute after pending flushes are applied
        SceneReferenceActionVector sceneReferenceActions;

        static void Clear(PendingData& pendingData)
        {
            pendingData.allPendingFlushesApplied = false;
            pendingData.pendingFlushes.clear();
            pendingData.sceneResourceActions.clear();
            pendingData.sceneReferenceActions.clear();
        }
    };

    struct StagingInfo
    {
        SceneSizeInformation      sizeInformation;
        PendingData               pendingData;
        SceneVersionTag           lastAppliedVersionTag;
        ManagedResourceVector     resourcesToUploadOnceMapping;
    };
}

#endif
