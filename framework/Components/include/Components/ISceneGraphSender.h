//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ISCENEGRAPHSENDER_H
#define RAMSES_ISCENEGRAPHSENDER_H

#include "SceneAPI/SceneId.h"

namespace ramses_internal
{
    class Guid;
    class SceneActionCollection;

    class ISceneGraphSender
    {
    public:
        virtual ~ISceneGraphSender() {}
        virtual void sendPublishScene        (SceneId sceneId, EScenePublicationMode publicationMode, const String& name) = 0;
        virtual void sendUnpublishScene      (SceneId sceneId, EScenePublicationMode publicationMode) = 0;
        virtual void sendCreateScene         (const Guid& to, const SceneInfo& sceneInfo, EScenePublicationMode publicationMode) = 0;
        virtual void sendSceneActionList     (const std::vector<Guid>& to, SceneActionCollection&& sceneAction, SceneId sceneId, EScenePublicationMode mode) = 0;
    };
}

#endif
