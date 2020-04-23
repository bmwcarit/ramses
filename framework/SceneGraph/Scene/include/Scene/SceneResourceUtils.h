//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENERESOURCEUTILS_H
#define RAMSES_SCENERESOURCEUTILS_H

#include "Scene/SceneResourceChanges.h"

namespace ramses_internal
{
    class IScene;

    namespace SceneResourceUtils
    {
        void GetAllSceneResourcesFromScene(SceneResourceActionVector& actions, const IScene& scene, size_t& usedDataByteSize);
        void GetAllClientResourcesFromScene(ResourceContentHashVector& resources, const IScene& scene);

        void DiffClientResources(ResourceContentHashVector const& old, ResourceContentHashVector const& curr, SceneResourceChanges& changes);
    }
}

#endif
