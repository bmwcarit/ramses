//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Scene/Scene.h"

namespace ramses::internal
{
    class ClientScene;
    class SceneActionCollectionCreator;

    /**
        Describes a scene in the form of SceneActions to a SceneActionCollectionCreator
    */
    class SceneDescriber
    {
    public:
        template <typename T>
        static void describeScene(const T& source, SceneActionCollectionCreator& collector);

    private:
        static void RecreateNodes(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreateCameras(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreateTransformNodes(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreateTransformations(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreateRenderables(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreateStates(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreateDataLayouts(const ClientScene& source, SceneActionCollectionCreator& collector);
        static void RecreateDataLayouts(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreateDataInstances(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreateRenderGroups(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreateRenderPasses(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreateBlitPasses(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreatePickableObjects(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreateDataBuffers(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreateTextureBuffers(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreateTextureSamplers(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreateRenderBuffersAndTargets(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreateDataSlots(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreateSceneVersionTag(const IScene& source, SceneActionCollectionCreator& collector);
        static void RecreateSceneReferences(const IScene& source, SceneActionCollectionCreator& collector);
    };
}
