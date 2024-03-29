//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Scene/ResourceChanges.h"

namespace ramses::internal
{
    class RendererCachedScene;
    class IRendererResourceManager;
    class FrameTimer;

    class PendingSceneResourcesUtils
    {
    public:
        static void ConsolidateSceneResourceActions(const SceneResourceActionVector& newActions, SceneResourceActionVector& currentActionsInOut);
        static bool ApplySceneResourceActions(const SceneResourceActionVector& actions, const RendererCachedScene& scene, IRendererResourceManager& resourceManager, const FrameTimer* frameTimer = nullptr);

    private:
        static bool RemoveSceneResourceActionIfContained(SceneResourceActionVector& actions, MemoryHandle handle, ESceneResourceAction action);
        static bool ContainsSceneResourceAction(const SceneResourceActionVector& actions, MemoryHandle handle, ESceneResourceAction action);
    };
}
