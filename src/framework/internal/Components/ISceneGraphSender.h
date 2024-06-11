//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/SceneId.h"

#include <string_view>

namespace ramses::internal
{
    class Guid;
    struct SceneUpdate;
    class StatisticCollectionScene;

    class ISceneGraphSender
    {
    public:
        virtual ~ISceneGraphSender() = default;
        virtual void sendPublishScene        (const SceneInfo& sceneInfo) = 0;
        virtual void sendUnpublishScene      (SceneId sceneId, EScenePublicationMode publicationMode) = 0;
        virtual void sendCreateScene         (const Guid& to, const SceneInfo& sceneInfo) = 0;
        virtual void sendSceneUpdate         (const std::vector<Guid>& to, SceneUpdate&& sceneUpdate, SceneId sceneId, EScenePublicationMode mode, StatisticCollectionScene& sceneStatistics) = 0;
    };
}
