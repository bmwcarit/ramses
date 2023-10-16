//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Scene/DataLayoutCachedScene.h"
#include "internal/SceneReferencing/SceneReferenceAction.h"
#include "internal/Core/Utils/StatisticCollection.h"

namespace ramses::internal
{
    // The client scene is just a wrapper for concrete implementation of a low level scene
    // together with some additional data used in client side logic
    class ClientScene final : public DataLayoutCachedScene
    {
    public:
        explicit ClientScene(const SceneInfo& sceneInfo = {})
            : DataLayoutCachedScene(sceneInfo)
        {
        }

        StatisticCollectionScene& getStatisticCollection()
        {
            return m_statisticCollection;
        }

    private:
        StatisticCollectionScene m_statisticCollection;
    };
}
