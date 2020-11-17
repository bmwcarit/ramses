//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEDEPENDENCYCHECKER_H
#define RAMSES_SCENEDEPENDENCYCHECKER_H

#include "SceneAPI/SceneId.h"
#include "RendererAPI/Types.h"
#include "Collections/HashMap.h"

namespace ramses_internal
{

    class SceneDependencyChecker
    {
    public:
        SceneDependencyChecker();

        Bool addDependency(SceneId providerScene, SceneId consumerScene);
        void removeDependency(SceneId providerScene, SceneId consumerScene);
        Bool hasDependencyAsConsumer(SceneId scene) const;
        void removeScene(SceneId scene);
        const SceneIdVector& getDependentScenesInOrder() const;
        Bool isEmpty() const;

    private:
        Bool hasDependencyAsConsumerToProvider(SceneId consumerScene, SceneId providerScene) const;
        void updateSceneOrder() const;

        using ConsumerToProvidersMap = HashMap<SceneId, SceneIdVector>;
        ConsumerToProvidersMap m_consumerToProvidersMap;

        mutable SceneIdVector m_sceneOrderList;
        mutable Bool m_dirty;
    };
}

#endif
