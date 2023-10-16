//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/SceneSizeInformation.h"
#include "internal/SceneGraph/SceneAPI/SceneVersionTag.h"
#include "internal/SceneGraph/Scene/SceneActionCollection.h"
#include "internal/SceneGraph/Scene/ResourceChanges.h"
#include "internal/SceneReferencing/SceneReferenceAction.h"
#include "internal/Components/FlushTimeInformation.h"
#include "internal/Components/ManagedResource.h"

namespace ramses::internal
{
    class ResourceCachedScene;

    struct PendingFlush
    {
        SceneActionCollection     sceneActions;
        uint64_t                  flushIndex = 0u;
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
