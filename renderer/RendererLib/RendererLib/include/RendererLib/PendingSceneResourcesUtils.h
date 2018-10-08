//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PENDINGSCENERESOURCESUTILS_H
#define RAMSES_PENDINGSCENERESOURCESUTILS_H

#include "Scene/SceneResourceChanges.h"

namespace ramses_internal
{
    class IScene;
    class IRendererResourceManager;

    class PendingSceneResourcesUtils
    {
    public:
        static SceneResourceActionVector ConsolidateSceneResourceActions(const SceneResourceActionVector& newActions, const SceneResourceActionVector* oldActions = nullptr);
        static void ApplySceneResourceActions(const SceneResourceActionVector& actions, const IScene& scene, IRendererResourceManager& resourceManager);

    private:
        static Bool RemoveSceneResourceActionIfContained(SceneResourceActionVector& actions, MemoryHandle handle, ESceneResourceAction action);
        static Bool ContainsSceneResourceAction(const SceneResourceActionVector& actions, MemoryHandle handle, ESceneResourceAction action);
    };
}

#endif
