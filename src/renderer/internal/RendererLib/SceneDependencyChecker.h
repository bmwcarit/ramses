//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/RendererLib/Types.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"

namespace ramses::internal
{

    class SceneDependencyChecker
    {
    public:
        SceneDependencyChecker();

        bool addDependency(SceneId providerScene, SceneId consumerScene);
        void removeDependency(SceneId providerScene, SceneId consumerScene);
        bool hasDependencyAsConsumer(SceneId scene) const;
        void removeScene(SceneId scene);
        const SceneIdVector& getDependentScenesInOrder() const;
        bool isEmpty() const;

    private:
        bool hasDependencyAsConsumerToProvider(SceneId consumerScene, SceneId providerScene) const;
        void updateSceneOrder() const;

        using ConsumerToProvidersMap = HashMap<SceneId, SceneIdVector>;
        ConsumerToProvidersMap m_consumerToProvidersMap;

        mutable SceneIdVector m_sceneOrderList;
        mutable bool m_dirty{false};
    };
}
